#include "asm/page.h"
#include "asm/ptrace.h"
#include "linux/linkage.h"
#include "linux/panic.h"
#include "linux/printk.h"
#include "linux/stddef.h"
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/ftrace.h>
#include "ftrace_helper.h"

/*
  * Every kernel module must present it self to the kernel. This is done using different MODULE_<*> macros 
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LADO SAHA");
MODULE_DESCRIPTION(
	"Another way mitigate the internal fragmentation of the Buddy Allocator.");
MODULE_VERSION("0.01");

#define MODULE_NAME "Pageman"
#define MOD_NAME_FIT MODULE_NAME "(Fit): "
#define MOD_NAME_FREE MODULE_NAME "(Free): "

/**
* The fit and free function is responsible for trimming down the number of allocated pages to the minimum possible size, 
* then carefully frees the excess pages in such a way not to completely decompose higher orders to order 0.
* When ever we intercept an allocation call with a size (x) KB, of order (n), and starting virtual address (virt) we proceed as follows:
* 1) Ideal Case: x can be written as a 2^n times PAGE_SIZE e.g x=128KiB, x=1024KiB etc, we donot do anything 
* 2) Worst Case: x cannot be written as 2^n times PAGE_SIZE e,g x=129KiB, x=1000KiB etc we intervene. 
*   2.0) We calculate the pages allocated by the buddy allocator by using the fact that it will always allocate (total_nr_pages) 2^n * PAGE_SIZE 
*   2.1) We calculate the minimum amount of pages which can fit the size (nr_pages_req) by finding the upper bound of x/PAGE_SIZE 
*   2.1) We successively divide the address space orignally spanning from (virt to virt + 2^n * PAGE_SIZE) into 2 equals halfs and compare if the left half can fit
*       the size 
*     2.1.1) In case the size can (not exactly) fit, we free right half to the required order, consider the address space as the left half, 
*         In  case the size is exactly fit to the left half, we just free the right half and mark the left as allocated and **stop**
*     2.1.2) In case the size cannot fit into the left half, we mark the left as allocated then continue on the right half, then reduce the the 
*         size required by subtracting from it the size of the left half allocated. 
*     We decreement the order (n--) and restart the subdivision.
*
*   2.3) We finally stop, save the number of pages allocated in the pages_metadata array at address the page frame number of the address
*     and return the initial address of the memory block and at this point we know that we have allocated the minimum number of pages 
*     required for the size.  
* Complexity O(log2(n)) and in most computers, n = 11 thus 0(1)
*
* @regs Eventhough this is unused, this the pointer to the registers of the hooked function orig_make_alloc_exact
* returns address
*/
asmlinkage void *fit_and_free(const struct pt_regs *regs);

/**
 * This is a pointer to the make_alloc_exact function in the kernel. We do this to be able to call the original behavior when ever needed
 * @pt_regs is the pointer to the registry entry which contains the information about the function. 
 */
asmlinkage void *(*orig_make_alloc_exact)(const struct pt_regs *);

/**
 * This is a pointer to the free_pages_exact function in the kernel which is hooked or intercepted
 */
asmlinkage void (*orig_free_pages_exact)(const struct pt_regs *);

/**
* The free and fit function is responsible for freeing the previously fitted pages. 
* When ever we intercept a deallocation call on an address which was previously fitted(We know this by verifying if the pfn has a non null entry in the page metadata array).
* If this is not the case, we do nothing else we execute this function.
* 1) We get the number of pages allocated from the array and calculate the size (x) KiB of the allocation and required order (n) 
* 2) We calculate the pages allocated which were to be allocated by the buddy allocator by using the fact that it will always allocate (total_nr_pages) 2^n * PAGE_SIZE 
*   2.1) We successively divide the address space orignally spanning from (virt to virt + 2^n * PAGE_SIZE) into 2 equals halfs and compare if the left half can fit
*       the allocated size
*     2.1.1) In case the size can (but not exactly) fit, we just consider the left half and continue 
*         In  case the size can exactly fit to the left half, we just free the left half and  **stop**
*     2.1.2) In case the size cannot fit into the left half, we free the left half to the required order and reduce the size required to be freed by subtracting
*         from it the size of the left half freed and continue 
*     We decreement the order (n--) and restart the subdivision.
*   2.2) We finally stop and reset the value of allocated pages in the array at index page frame number of the current address to 0 
* Complexity O(log2(n)) and in most computers, n = 11 thus 0(1)
*
* @regs Eventhough this is unused, this the pointer to the registers of the hooked function orig_free_pages_exact 
*/
asmlinkage void free_and_fit(const struct pt_regs *regs);

asmlinkage void *fit_and_free(const struct pt_regs *regs)
{
	unsigned long start_address = gb_start_address;

	if ((void *)start_address == NULL) {
		pr_err(MOD_NAME_FIT "Initial allocation failed\n");
		return NULL;
	}

	size_t size = gb_size;
	unsigned int nr_pages_req = DIV_ROUND_UP(size, PAGE_SIZE),
		     init_order = gb_order, total_nr_pages = 1 << init_order;

	if (total_nr_pages == nr_pages_req) {
		pr_info(MOD_NAME_FIT
			"Nothing to fit with %zu KB and %u Pages\n",
			size / 1024, nr_pages_req);
		return (void *)start_address;
	}

	struct timespec64 start_time, end_time;
	s64 elapsed_time;
	unsigned long addr_start, addr_mid, addr_end, size_span, req_span;
	struct page *page_head;
	unsigned int order = init_order - 1;

	// This is a log to benchmark the process
	ktime_get_ts64(&start_time);
	pr_info(MOD_NAME_FIT
		"Order From\t\t Mid\t\t To\t\t Fit_pages\t Fit_size\t Half_size\n");

	addr_start = start_address;
	addr_end = start_address + total_nr_pages * PAGE_SIZE;
	req_span = PAGE_SIZE * nr_pages_req;

	while (1) {
		addr_mid = (addr_start / 2 + addr_end / 2);
		size_span = addr_end - addr_mid;
		pr_info(MOD_NAME_FIT
			"%u   0x%lx 0x%lx 0x%lx \t%u \t%lu KB \t%lu KB\n",
			order, addr_start, addr_mid, addr_end, nr_pages_req,
			(req_span), (size_span));
		if (req_span > size_span) {
			req_span -= size_span;
			addr_start = addr_mid;
			order--;
		} else {
			page_head = virt_to_page((void *)(addr_mid + 1));
			__free_pages(page_head, order);
			if (req_span == size_span) {
				set_nr_pages_metadata(
					virt_to_page((void *)start_address),
					nr_pages_req);
				break;
			}
			addr_end = addr_mid;
			order--;
		}
	}
	// This is a log to benchmark the process
	ktime_get_ts64(&end_time);
	elapsed_time = timespec64_sub(end_time, start_time).tv_nsec;
	pr_info(MOD_NAME_FIT
		"Stats| Fit_size=%lu KB \t Required_pages=%u \tOriginal_pages=%u \tElapse=%llu ns",
		size / 1024, nr_pages_req, total_nr_pages, elapsed_time);
	return (void *)start_address;
}

asmlinkage void (*orig_free_pages_exact)(const struct pt_regs *);
asmlinkage void free_and_fit(const struct pt_regs *regs)
{
	orig_free_pages_exact(regs);
	unsigned long start_address = gb_start_address;
	if ((void *)start_address == NULL) {
		pr_info("Free&Fit: Invalid Address\n");
		return;
	};
	unsigned int fit_nr_pages =
		get_nr_pages_metadata(virt_to_page((void *)start_address));
	if (fit_nr_pages == 0) {
		pr_info("Free&Fit: Not required\n");
		return;
	}

	struct page *page_head;
	unsigned long addr_start, addr_end, addr_mid, size_span, req_span;
	struct timespec64 start_time, end_time;
	s64 elapsed_time;
	// This is a log to benchmark the process
	ktime_get_ts64(&start_time);
	req_span = fit_nr_pages * PAGE_SIZE;
	unsigned short init_order = get_order(req_span);
	unsigned short order = init_order - 1;
	unsigned long total_nr_pages = 1 << init_order;

	if (fit_nr_pages == total_nr_pages) {
		pr_info("Free&Fit: Nothing to Fit\n");
		return;
	}
	pr_info("Free&Fit: Order From\t\t Mid\t\t To\t\t fit_pages\t left_size\t Half_size\n");

	addr_start = start_address;
	addr_end = start_address + (total_nr_pages * PAGE_SIZE);
	while (1) {
		addr_mid = addr_start / 2 + addr_end / 2;
		size_span = addr_end - addr_mid;
		pr_info("Free&Fit: %u   0x%lx 0x%lx 0x%lx \t%u \t%lu KB \t%lu KB\n",
			order, addr_start, addr_mid, addr_end, fit_nr_pages,
			(req_span), (size_span));
		if (req_span < size_span) {
			addr_end = addr_mid;
			order--;
		} else {
			page_head = virt_to_page((void *)(addr_start));
			__free_pages(page_head, order);
			if (req_span == size_span) {
				set_nr_pages_metadata(
					virt_to_page((void *)start_address), 0);
				break;
			}
			req_span -= size_span;
			addr_start = addr_mid;
			order--;
		}
	}

	// This is a log to benchmark the process
	ktime_get_ts64(&end_time);
	elapsed_time = timespec64_sub(end_time, start_time).tv_nsec;
	pr_info("Fit&Free: Stats| Free_size=%lu KB \t fit_pages=%u \tMax_pages=%lu \tElapse=%llu ns\n",
		(fit_nr_pages * PAGE_SIZE) / 1024, fit_nr_pages, total_nr_pages,
		elapsed_time);
}

static struct ftrace_hook hooks[2] = {
	HOOK("make_alloc_exact", fit_and_free, &orig_make_alloc_exact),
	HOOK("free_pages_exact", free_and_fit, &orig_free_pages_exact)
};

static int __init pageman_init(void)
{
	int err;
	err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
	if (err)
		return err;

	is_pageman_loaded = true;
	pr_info("Pageman(F&F): loaded\n");
	return 0;
}

static void __exit pageman_exit(void)
{
	fh_remove_hooks(hooks, ARRAY_SIZE(hooks));

	is_pageman_loaded = false;
	pr_info("Pageman(F&F): unloaded\n");
}

module_init(pageman_init);
module_exit(pageman_exit);
