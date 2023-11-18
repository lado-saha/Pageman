#include "asm/ptrace.h"
#include "linux/linkage.h"
#include "linux/panic.h"
#include "linux/stddef.h"
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/ftrace.h>
#include "ftrace_helper.h"

asmlinkage void *(*orig_make_alloc_exact)(const struct pt_regs *);
asmlinkage void *fit_and_free(const struct pt_regs *regs)
{
	unsigned long start_address = gb_start_address;
	size_t size = gb_size;
	unsigned int nr_pages_req = DIV_ROUND_UP(size, PAGE_SIZE),
		     init_order = gb_order, nr_total_pages = 1 << init_order;
	struct timespec64 start_time, end_time;
	s64 elapsed_time;
	unsigned long addr_start, addr_mid, addr_end, size_span, req_span;
	struct page *page_head;
	unsigned int order = init_order - 1;

	if ((void *)start_address == NULL) {
		pr_info("Fit&Free: Allocation Failed\n");
		return NULL;
	}

	if (nr_total_pages - nr_pages_req <= 1) {
		pr_info("Fit&Free: Nothing to fit with %zuBytes and %uPages\n",
			size, nr_pages_req);
		return (void *)start_address;
	}
	// This is a log to benchmark the process
	ktime_get_ts64(&start_time);
	pr_info("Fit&Free: Order From\t\t Mid\t\t To\t\t Fit_pages\t Fit_size\t Half_size\n");

	addr_start = start_address;
	addr_end = start_address + (1 << init_order) * PAGE_SIZE;
	req_span = PAGE_SIZE * nr_pages_req;

	while (1) {
		addr_mid = (addr_start / 2 + addr_end / 2);
		size_span = addr_end - addr_mid;
		pr_info("Fit&Free: %u   0x%lx 0x%lx 0x%lx \t%u \t%lu KB \t%lu KB\n",
			order, addr_start, addr_mid, addr_end, nr_pages_req,
			(req_span / 1024), (size_span / 1024));
		if (req_span > size_span) {
			req_span -= size_span;
			addr_start = addr_mid;
			order--;
		} else if (req_span <= size_span) {
			page_head = virt_to_page((void *)(addr_mid + 1));
			__free_pages(page_head, order);
			if (req_span == size_span) {
				break;
			}
			addr_end = addr_mid;
			order--;
		}
	}
	// This is a log to benchmark the process
	ktime_get_ts64(&end_time);
	elapsed_time = timespec64_sub(end_time, start_time).tv_nsec;
	pr_info("Fit&Free Stats| Fit_size=%lu KB \t Required_pages=%u \tOriginal_pages=%u \tElapse=%llu ns",
		size / 1024, nr_pages_req, nr_total_pages, elapsed_time);
	return (void *)start_address;
}

asmlinkage void (*orig_free_pages_exact)(const struct pt_regs *);
asmlinkage void free_and_fit(const struct pt_regs *regs)
{
	unsigned long start_address = gb_start_address;
	if ((void *)start_address == NULL) {
		pr_info("Free&Fit: Invalid Address\n");
		return;
	}

	pr_info("Free&Fit: Order From\t\t Mid\t\t To\t\t fit_pages\t left_size\t Half_size\n");
	struct page *page_head;
	unsigned long addr_start, addr_end, addr_mid, size_span, req_span;
	struct timespec64 start_time, end_time;
	s64 elapsed_time;
	// This is a log to benchmark the process
	ktime_get_ts64(&start_time);
	unsigned short fit_nr_pages =
		get_nr_pages_metadata(virt_to_page((void *)start_address));
	set_nr_pages_metadata(virt_to_page((void *)start_address), 0);
	req_span = fit_nr_pages * PAGE_SIZE;
	unsigned short init_order = get_order(req_span);
	unsigned short order = init_order - 1;
	unsigned long total_nr_pages = 1 << init_order;
	addr_start = start_address;
	addr_end = start_address + (total_nr_pages * PAGE_SIZE);
	while (1) {
		addr_mid = addr_start / 2 + addr_end / 2;
		size_span = addr_end - addr_mid;
		pr_info("Free&Fit: %u   0x%lx 0x%lx 0x%lx \t%u \t%lu KB \t%lu KB\n",
			order, addr_start, addr_mid, addr_end, fit_nr_pages,
			(req_span / 1024), (size_span / 1024));
		if (req_span < size_span) {
			addr_end = addr_mid;
			order--;
		} else {
			req_span -= size_span;
			page_head = virt_to_page((void *)(addr_start));
			__free_pages(page_head, order);
			if (req_span == size_span) {
				break;
			}
			addr_start = addr_mid;
			order--;
		}
	}

	// This is a log to benchmark the process
	ktime_get_ts64(&end_time);
	elapsed_time = timespec64_sub(end_time, start_time).tv_nsec;
	pr_info("Fit&Free Stats| Free_size=%lu KB \t fit_pages=%u \tMax_pages=%lu \tElapse=%llu ns\n",
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
	pr_info("Pageman(F&F): loaded\n");
	return 0;
}

static void __exit pageman_exit(void)
{
	fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
	pr_info("Pageman(F&F): unloaded\n");
}

module_init(pageman_init);
module_exit(pageman_exit);
