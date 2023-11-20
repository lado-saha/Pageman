# Linux Memory Manager ```Pageman``` 

|Project Title | Linux Memory Manager |
|--|--|
|Name           |LADO SAHA|
|ID             |21P296|
|Class          |3GI|
|Subject| Operating System|
|Teacher Name   |Pr. Djiotio  |
|Date           |19 Nov 2023 |

## Table of Contents
- #### [Abstract](#abstract)
- #### [Introduction](#introduction)
- #### [Methodology](#methodology)
- #### [Implementation](#implementation)
- ####  [Results](#results)
- #### [Evaluation](#evaluation)
- #### [Conclusion](#conclusion)
- #### [References](#references)
- #### [Appendices](#appendices)

## Abstract/Summary

## Introduction

## Methodology

## Implementation
The implementation was done on a linux machine with the following specs
|Computer|Lenovo Thinkpad T460s|
|--|--|
|Firmware version| N1CET89W (1.57)|
|Memory|12.0GiB|
|Processor|Intel® Core™ i7-6600U × 4|
|Graphics|Intel® HD Graphics 520 (SKL GT2)|
|Disk Capacity|2TB|
|Linux Distro|Ubuntu 23.04|
|OS Type|64-bit|
|Kernel Version| Linux 6.2.0-20-generic|

The project was carried out on a custom built **linux kernel version 6.5.3.**

### 0. Prerequisites
The following steps were carried out to setup the system
 ```bash
# The following libraries were installed during the setup process

# Updating the system libraries
sudo update
sudo upgrade

# Installing linux headers for compilation
sudo apt install build-essential dkms linux-headers-$(uname -r)

# Installing C, git, make and perl
sudo apt install git gcc make perl 

# Additional libraries for compilation
sudo apt-get install libncurses-dev flex bison openssl libssl-dev dkms libelf-dev libudev-dev libpci-dev libiberty-dev autoconf
sudo apt install dwarves
```
### 1. Building a Linux kernel from source
- Download and extraction of the kernel 
``` bash
# Changing to the home directory and creating our working directory manager
cd ~
mkdir manager

# Download the kernel archive file
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.5.3.tar.xz

# Extract the extraction 
tar -xf manager/linux-6.5.3.tar.xz

# Navigating to the kernel code 
cd manager/linux-6.5.3
```
- Building the kernel image and modules
```bash
# Copying the current system modules in an attempt to build a clone
sudo lsmod > /tmp/lsmod.now

# After putting our terminal in fullscreen, we edit the configuration of our kernel 
make menuconfig
```
The last command opens a minimalist menu driven interface to tweak some of the kernel configurations. 

---
NB: The following keys are used in the menuconfig (non exhaustive)
|Keyboard Key | Meaning | Visual Effect|
|--|---|--|
|y| Enable| * |
|n| Disable| n |
|m| Enable as module| m |
|Esc(x2)| Navigate back | |
|Up, Down| Scroll Up, Down| |
|Left, Right| Navigate about the menu| Highlights the Exit, Save, Load options|
---
The table below contains the different configurations which were done to the kernel 

|Feature| Info |Path in menu |Precise config option |New value|
|--|--|--|--|--|
|Kernel config Support| Allows us to see the current kernel config details | General Setup/ Kernel .config support | CONFIG_IKCONFIG |y|
| |Allows us to see current kernel configuration details via the **procfs**|General Setup/ Enable access to .config through the /proc/config.gz | CONFIG_IKCONFIG_PROC | n|
|Kernel profiling| Kernel profiling support | General Setup / Profiling support| CONFIG_PROFILING | n|
|HAM RADIO | Support for the HAM radio | Networking support/ Amateur Radio support| CONFIG_HAMRAIO|y|
|Userspace IO | UIO support | Device Drivers / Userspace I/O Drivers | CONFIG_UIO | m|
| |UIO platform driver with generic IRQ(Interrupt handler Request) handling | Device Drivers / Userspace I/O Drivers / Userspace I/O platform driver with generic IRQ handling| CONFIG_UIO_PDRV_GENIRQ|m|
|MS-DOS filesystem support |Mount NTFS drives| File systems / DOS/FAT/NT Filesystems / MSDOS fs support |CONFIG_MSDOS_FS | m|
|Security LSMs | Turn off kernel LSMs (Not safe for production environments) | Security options / Enable different security models | CONFIG_SECURITY | n|
|Kernel debug: stack utilization info | Have full debuf info about the memory | Kernel hacking / Memory Debugging / Stack utilization instrumentation | CONFIG_DEBUG_STACK-USAGE| y |

- After applying all the configuration above we saved and left the menu. We then proceseded by disabling some securities and were finally ready to build the kernel. Th e building process took 45 minutes and was very CPU and memory Intensive 


> ⚠️ **Warning:** Any interruption like sleeping or shutdown during the compilation process may lead to a complete corruption of the current system GUI and internal components . Thus we made sure our computer could not sleep and was well powered.

```bash
# ~/manger/linux-6.5.3
# Disabling some debian specific security keys 
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --disable SYSTEM_TRUSTED_KEYS

# Building the kernel and specifying the modules to install 
# For parallel compilations, we used -j4 which allocates 4 cores 
make LSMOD=/tmp/lsmod.now -j4
```
### 3. Setting up the Editing environment 
- We used the [neovim](https://github.com/neovim/neovim/wiki/Installing-Neovim#linux) editor. The basic installation was done using the following commands.
```bash
# Navigate to the home directory
cd ~
# Installing the neovim image
curl -LO https://github.com/neovim/neovim/releases/latest/download/nvim.appimage
# Giving the permissions 
chmod u+x nvim.appimage
./nvim.appimage
#
# if the ./nvim.appimage fails, we can try
./nvim.appimage --appimage-extract
./squashfs-root/AppRun --version

# Optional: exposing nvim globally.
sudo mv squashfs-root /
sudo ln -s /squashfs-root/AppRun /usr/bin/nvim

# Finally, we can run the editor using the command below from any where
nvim
```
After installing neovim, some setup process was needed to make it suitable for editing C code, and the resource below was of great help 

[![The perfect Neovim Setup for C++](https://img.youtube.com/vi/lsFoZIg-oDs/0.jpg)](https://www.youtube.com/watch?v=lsFoZIg-oDs)

> Another option could have been the Code editor [vscode](https://code.visualstudio.com/docs/setup/linux). In that case, we could have installed the C/C++ extensions from Microsoft and the Linux Kernel extensions.

- After setting up the code editor for vanilla C code, we had make sure it understood we were dealing with Linux kernel code and could include Kernel header files. This was done beacuse the linux kernel does not use the standard gcc library. The setup was as follows. 
> The [StackOverflow thread](https://stackoverflow.com/questions/33676829/vim-configuration-for-linux-kernel-development) was of great help
```bash
# Navigate to the kernel code 
cd ~/manager/linux-6.5.3 

# Required for code editor to navigate through the kernel code 
sudo apt install cscope exuberant-ctags

# Creating an index database for easy navigation and code intelisense 
# We index only indexed the x86_64 architecture directories since we had no need for cross platform and our computer was an x86 64bit computer 
make O=. ARCH=x86_64 COMPILED_SOURCE=1 cscope tags

# Next, we generated the following json file to inform our editor about some compiling options.
python3 scripts/clang-tools/gen_compile_commands.py
```
> NB: Eventhough we didnot use vscode, we found the resource [vscode for kernel dev](https://github.com/neilchennc/vscode-linux-kernel) potentially helpful. It mainly consists in editing the ```c_cpp_properties.json```, ```settings.json```, ```tasks.json```, ```.vscode/``` to recognise the kernel header files

### 4. Tweaking the kernel memory manager source code
>This section was done with a lot of precaution since any error could make the compilation fail. We made sure to carefully add some functions and global variables necessary for the wee functioning of ``Pageman``.
- Overall the, the following kernel files were modified
    - ```~/manager/linux-6.5.3/include/linux/gfp.h```
    - ```~/manager/linux-6.5.3/mm/page_alloc.c```
    - ```~/manager/linux-6.5.3/mm/slab_common.c```

For each file, we did specific changes as shown below

- ``gfp.h`` File additions  
```c
...
/* Added fter line 157 in gfp.h*/

/**
 * A global tracker to know if pageman has been loaded or not 
 * Becomes true when pageman is loaded and false when unloaded  
 */
extern bool is_pageman_loaded;
/**
 * A global variables that holds the value of the start of the virtual first page of an allocated or to be deallocated block.
 * When ever an allocation or deallocation is intercepted, we save the this address.
 * NB: This address is a the virtual adddress to the page and not the page address
 */
extern unsigned long gb_start_address;
/**
 * Another global variable that holds the order of an allocated request to the BuddyAllocator
 * Varies from 0 to MAX_ORDER(generally 11).
 */
extern unsigned int gb_order;
/**
 * Another global variable that holds the size in Bytes of an allocation request 
 */
extern size_t gb_size;
/**
 * The size of this array is ideally equals to the total numbers of pages in our RAM. This size is calculated by 
 * dividing the RAM size by the size of a page. 
 * e.g array size for 16GB = (16 * 1024 * 1024 KB)/(4096 KB), assuming our pages are 4KB large.  
 * Since we cannot know the this at compile time, we must set assume a large enough value.
 *
 * In practice, this array stores the number of pages that were allocated for each allocation, which permits us 
 * durring deallocation to know the number of pages which were allocated. The index is the page frame number associated to 
 * start_address of the allocation block and it is initialized to zeroes at compile time 
 *
 * (Warning) There are limits to the size of this array above which the compilation will fail.
 */
extern unsigned long pages_metadata_table[3145728];
/**
 * This is a helper function called by the allocators to set the global variables 
 */
void fit_and_free_injector(unsigned long page_start_addr, unsigned int order, size_t size);
/**
 * This is a helper function called by the deallocators to set the start_address of the block to be deallocated
 */
void free_and_fit_injector(unsigned long page_start_addr);
/**
 * A helper function to know if a page has been allocated by checking if its page frame number(pfn) has a non zero 
 * value in the array 
 *
 * Returns 0 if the page is not currently being allocated
 * Returns 1 if the page is currently in use or has not been deallocated 
 */
extern int get_nr_pages_metadata(struct page *page);
/**
 * A helper function to set the number of pages allocated with by a block starting at that pfn 
 * In summary for a given page, it gets the pfn and uses it as an index to set the number of pages allocated 
 */
extern void set_nr_pages_metadata(struct page *page, unsigned short nr_pages);
....
```
- ``page_alloc.c`` File modifications 

Definitions of the global variables and functions defined in gfp.h 
```c 
...

/* In page_alloc.c after line 394 */

bool is_pageman_loaded = false;
unsigned long gb_start_address = 0;
unsigned int gb_order = 0;
size_t gb_size = 0;
unsigned long pages_metadata_table[3145728] = { 0 };
void fit_and_free_injector(unsigned long page_start_addr, unsigned int order, size_t size)
{
	gb_start_address = page_start_addr;
	gb_size = size;
	gb_order = order;
}

void free_and_fit_injector(unsigned long page_start_addr)
{
	gb_start_address = page_start_addr;
}

int get_nr_pages_metadata(struct page *page)
{
	unsigned long pfn = page_to_pfn(page);
	return pages_metadata_table[pfn];
}

void set_nr_pages_metadata(struct page *page, unsigned short nr_pages)
{
	unsigned long pfn = page_to_pfn(page);
	pages_metadata_table[pfn] = nr_pages;
}
 ....
```
Modification of the ``alloc_pages_exact`` definition at around line 4791

```c 
/* In page_alloc.c */
....
/**
 * alloc_pages_exact - allocate an exact number physically-contiguous pages.
 * @size: the number of bytes to allocate
 * @gfp_mask: GFP flags for the allocation, must not contain __GFP_COMP
 *
 * This function is similar to alloc_pages(), except that it allocates the
 * minimum number of pages to satisfy the request.  alloc_pages() can only
 * allocate memory in power-of-two pages.
 *
 * This function is also limited by MAX_ORDER.
 *
 * Memory allocated by this function must be released by free_pages_exact().
 *
 * Return: pointer to the allocated area or %NULL in case of error.
 */
void *alloc_pages_exact(size_t size, gfp_t gfp_mask)
{
	unsigned int order = get_order(size);
	unsigned long addr;

	if (WARN_ON_ONCE(gfp_mask & (__GFP_COMP | __GFP_HIGHMEM)))
		gfp_mask &= ~(__GFP_COMP | __GFP_HIGHMEM);

	addr = __get_free_pages(gfp_mask, order);

	/*
   * New 
   * We check if the pageman module is loaded before setting the global variables 
   */
	if (is_pageman_loaded) {
		fit_and_free_injector((unsigned long)addr, order, size);
	}
	/* End */

	return make_alloc_exact(addr, order, size);
}
....
```

Modification of the ``alloc_pages_exact_nid`` definition at around line 4791

```c
/* In page_alloc.c*/
....
/**
 * alloc_pages_exact_nid - allocate an exact number of physically-contiguous
 *			   pages on a node.
 * @nid: the preferred node ID where memory should be allocated
 * @size: the number of bytes to allocate
 * @gfp_mask: GFP flags for the allocation, must not contain __GFP_COMP
 *
 * Like alloc_pages_exact(), but try to allocate on node nid first before falling
 * back.
 *
 * Return: pointer to the allocated area or %NULL in case of error.
 */
void *__meminit alloc_pages_exact_nid(int nid, size_t size, gfp_t gfp_mask)
{
	unsigned int order = get_order(size);
	struct page *p;

	if (WARN_ON_ONCE(gfp_mask & (__GFP_COMP | __GFP_HIGHMEM)))
		gfp_mask &= ~(__GFP_COMP | __GFP_HIGHMEM);

	p = alloc_pages_node(nid, gfp_mask, order);
	if (!p)
		return NULL;

	/*
   * New 
   * We check if the pageman module is loaded before setting the global variables 
   */
	if (is_pageman_loaded) {
		fit_and_free_injector((unsigned long)page_address(p), order,
				      size);
	}
	/* End */

	return make_alloc_exact((unsigned long)page_address(p), order, size);
}

....
```
Modification of the ``free_pages_exact`` definition around line 4849
```c 
/* In page_alloc.c */
....

/**
 * free_pages_exact - release memory allocated via alloc_pages_exact()
 * @virt: the value returned by alloc_pages_exact.
 * @size: size of allocation, same value as passed to alloc_pages_exact().
 *
 * Release the memory allocated by a previous call to alloc_pages_exact.
 */
void free_pages_exact(void *virt, size_t size)
{
  /**
   * New 
   * We check if this pageman is loaded and that this allocation was initially done by pageman 
   * other wise, we leave the default behavior 
   */
	if (is_pageman_loaded &&
	    get_nr_pages_metadata(virt_to_page(virt)) != 0) {
		free_and_fit_injector((unsigned long)virt);
	} else {
    /* End */
		unsigned long addr = (unsigned long)virt;
		unsigned long end = addr + PAGE_ALIGN(size);

		while (addr < end) {
			free_page(addr);
			addr += PAGE_SIZE;
		}
	}
}
....
```
- ```slab_common.c``` Modifications 
Firstly we modify the ```__kmalloc_large_node```(after line 1116) which is called when the allocation size of ```kmalloc``` is too large for any preallocated slab.

```c 
/* In slab_common.c File after 1116*/
/*
 * To avoid unnecessary overhead, we pass through large allocation requests
 * directly to the page allocator. We use __GFP_COMP, because we will need to
 * know the allocation order to free the pages properly in kfree.
 */

static void *__kmalloc_large_node(size_t size, gfp_t flags, int node)
{
	struct page *page;
	void *ptr = NULL;
	unsigned int order = get_order(size);

	if (unlikely(flags & GFP_SLAB_BUG_MASK))
		flags = kmalloc_fix_flags(flags);

	flags |= __GFP_COMP;
	page = alloc_pages_node(node, flags, order);
	if (page) {
		ptr = page_address(page);
		/**
     * New 
     * We first check if pageman is loaded, next we make sure the allocation size is not a power of 2 (This is the ideal case)
     * FInally, we make sure the size allocated is not more than the buddy allocator can handle 
     * In case everything is ok, we set the global variables and launch the make_alloc_exact function which will later be intercepted by 
     * pageman and Fit the number of pages
     *
     * Else, we use the default behavior ...
     */
		if (is_pageman_loaded &&
		    DIV_ROUND_UP(size, PAGE_SIZE) != 1 << order &&
		    size < (1 << MAX_ORDER) * PAGE_SIZE) {
			fit_and_free_injector((unsigned long)ptr, order, size);
			return make_alloc_exact((unsigned long)ptr, order,
						size);
		}
		/** End **/
		mod_lruvec_page_state(page, NR_SLAB_UNRECLAIMABLE_B,
				      PAGE_SIZE << order);
	}

	ptr = kasan_kmalloc_large(ptr, size, flags);
	/* As ptr might get tagged, call kmemleak hook after KASAN. */
	kmemleak_alloc(ptr, size, 1, flags);
	kmsan_kmalloc_large(ptr, size, flags);

	return ptr;
}
```
Next, we edit the `kmalloc_large`(after line) and `kmalloc_large_node` allocators wrappers to know how to trace the allocations by setting the pages actually allocated in case pageman was active 
```c
/* In slab_common.c after line 1158 */

void *kmalloc_large(size_t size, gfp_t flags)
{
	void *ret = __kmalloc_large_node(size, flags, NUMA_NO_NODE);

	/** New: 0 if the allocation failed*/
	unsigned long nr_pages = get_nr_pages_metadata(virt_to_page(ret));

	/**
   * New 
   * In case pageman is loaded and the allocation was successfull, we instead trace the  nr_pages actually allocted 
   */
	if (is_pageman_loaded && nr_pages != 0) {
		trace_kmalloc(_RET_IP_, ret, size, nr_pages, flags,
			      NUMA_NO_NODE);
		/*End*/
	} else {
		trace_kmalloc(_RET_IP_, ret, size, PAGE_SIZE << get_order(size),
			      flags, NUMA_NO_NODE);
	}
	return ret;
}
...

```
```c 
/** In slab_common.c */

void *kmalloc_large_node(size_t size, gfp_t flags, int node)
{
	void *ret = __kmalloc_large_node(size, flags, NUMA_NO_NODE);

	/** New: 0 if the allocation failed*/
	unsigned long nr_pages = get_nr_pages_metadata(virt_to_page(ret));
	/**
   * New 
   * In case pageman is loaded and the allocation was successfull, we instead trace the nr_pages actually allocted 
   */

	if (is_pageman_loaded && nr_pages != 0) {
		trace_kmalloc(_RET_IP_, ret, size, nr_pages, flags, node);
	/*End*/
	} else {
		trace_kmalloc(_RET_IP_, ret, size, PAGE_SIZE << get_order(size),
			      flags, node);
	}

	return ret;
}
```
>That marked the end of the modifications in the kernel code. In Summary, introduce hooking points and state savers in the kernel code. All this was in order to make our module function well.
 
## Results

## Evaluation

## Conclusion

## References

## Appendices
This book covers the following exciting features:
* Write high-quality modular kernel code (LKM framework) for 5.x kernels
* Configure and build a kernel from source
* Explore the Linux kernel architecture
* Get to grips with key internals regarding memory management within the kernel
* Understand and work with various dynamic kernel memory alloc/dealloc APIs
Discover key internals aspects regarding CPU scheduling within the kernel
Gain an understanding of kernel concurrency issues
Learn how to work with key kernel synchronization primitives

If you feel this book is for you, get your [copy](https://www.amazon.com/dp/178995343X) today!

<a href="https://www.packtpub.com/?utm_source=github&utm_medium=banner&utm_campaign=GitHubBanner"><img src="https://raw.githubusercontent.com/PacktPublishing/GitHub/master/GitHub.png" 
alt="https://www.packtpub.com/" border="5" /></a>

## Instructions and Navigations
All of the code is organized into folders. For example, ch2.

The code will look like the following:
```C
static int __init miscdrv_init(void)
{
	int ret;
	struct device *dev;
```

**Following is what you need for this book:**
This book is for Linux programmers beginning to find their way with Linux kernel development. Linux kernel and driver developers looking to overcome frequent and common kernel development issues, as well as understand kernel internals, will benefit from this book. A basic understanding of Linux CLI and C programming is required.

With the following software and hardware list you can run all code files present in the book (Chapter 1-13).
### Software and Hardware List
| Chapter | Software required | OS required |
| -------- | ------------------------------------ | ----------------------------------- |
| 1-13 | Oracle VirtualBox 6.1 | Windows and Linux (Any) |

We also provide a PDF file that has color images of the screenshots/diagrams used in this book. [Click here to download it](https://static.packt-cdn.com/downloads/9781789953435_ColorImages.pdf).

### Known Errata
Wrt the PDF doc:

- pg 26:
    - 'sudo apt install git fakeroot ...' ; corrections:
        - change pstree to psmisc
        - to install 'tuna' refer https://tuna.readthedocs.io/en/stable/installation.html 
        - change hexdump to bsdmainutils (the package name)
        - for 'openjdk-14-jre' installation refer https://java.tutorials24x7.com/blog/how-to-install-openjdk-14-on-ubuntu
- pg 99:
    - 'Chapter 9' should be 'Chapter 10'
    - 'Chapter 10' should be 'Chapter 11'
- pg 155:
    - the line '// ch4/helloworld_lkm/hellowworld_lkm.c' has the letter 'w' twice; it should be:
     '// ch4/helloworld_lkm/helloworld_lkm.c'   (thanks to @xuhw21)
- pg 246:
    - 'via the module_parm_cb() macro' should be 'via the module_param_cb() macro'
- pg 291:
    - '(The kernel-mode stack for ' - incomplete sentence; it should be deleted/ignored.
- pg 307:
    - the process view after the sentence '...  and a total of *nine threads*:'
        - the first two columns are shown as 'PID  TGID'; the order is reversed, it should be 'TGID  PID'
- pg 324: in *Figure 7.4*, the third column 'Addr Bits', last 3 rows have errors; the corrections are shown here:

                          `AB    VM-split`

`x86_64:  5 : 56 --> 57 :  64PB:64PB  : corrected (allows for total of 128 PB)`

`Aarch64: 3 : 39 --> 40 : 512G:512G : corrected (allows for total of 1024 GB = 1 TB)`

`Aarch64: 4 : 48 --> 49 : 256T:256T  : corrected (allows for total of 512 T)`

- pg 385:
   - 'On high-end enterprise server class systems running the Itanium (IA-64) processor, MAX_ORDER can be as high as 17 (implying a
largest chunk size on order (17-1), thus of 216 = 65,536 pages = 512 MB chunks of physically contiguous RAM on order 16 of the freelists, for
a 4 KB page size).'
should be:
'On high-end enterprise server class systems running the Itanium (IA-64) processor, MAX_ORDER can be as high as 17 (implying a
largest chunk size on order (17-1), thus of 216 = 65,536 pages = *256 MB* chunks of physically contiguous RAM on order 16 of the freelists, for
a 4 KB page size).'

- pg 388:
    - '... the next available memory chunk is on order 7, of size 256 KB.' should be: '... the next available memory chunk is on order 6, of size 256 KB.

- pg 656: the line
  'In place of atomic64_dec_if_positive(), use atomic64_dec_if_positive()'
  should be
  'In place of atomic_dec_if_positive(), use atomic64_dec_if_positive()'
(thanks to @xuhw21)

- pg 661:
    - '... there is a incorrect reference regarding a folder chp15/kthread_simple/kthread. The correct reference should be ch5/kthread_simple/kthread in part 2 of the book's GitHub [[Repository]](https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2)

- pg 665:
    - '...In info/Tip: 
```
"The material in this section assumes you have at least a base understanding of accessing 
peripheral device (chip) memory and registers; we have covered this in detail in 
Chapter 13, Working with Hardware I/O Memory. Please ensure that you understand it before moving forward. 
```
should be
```
"The material in this section assumes you have at least a base understanding of accessing 
peripheral device (chip) memory and registers; we have covered this in detail in 
Linux Kernel Programming Part 2 - Chapter 3, Working with Hardware I/O Memory. 
Please ensure that you understand it before moving forward."
```
* pg 183 : **Wiring to the console** _should be_ **Writing to the console**

### Related products
* Mastering Linux Device Driver Development [[Packt]](https://www.packtpub.com/product/Mastering-Linux-Device-Driver-Development/9781789342048) [[Amazon]](https://www.amazon.com/Mastering-Linux-Device-Driver-Development/dp/178934204X)

* Hands-On System Programming with Linux [[Packt]](https://www.packtpub.com/in/networking-and-servers/hands-system-programming-linux?utm_source=github&utm_medium=repository&utm_campaign=9781788998475) [[Amazon]](https://www.amazon.com/Hands-System-Programming-Linux-programming-ebook/dp/B079RKKKJ7/ref=sr_1_1?dchild=1&keywords=Hands-On+System+Programming+with+Linux&qid=1614057025&s=books&sr=1-1)

## Get to Know the Author
**Kaiwan N Billimoria**
taught himself BASIC programming on his dad's IBM PC back in 1983. He was programming in C and Assembly on DOS until he discovered the joys of Unix, and by around 1997, Linux!

Kaiwan has worked on many aspects of the Linux system programming stack, including Bash scripting, system programming in C, kernel internals, device drivers, and embedded Linux work. He has actively worked on several commercial/FOSS projects. His contributions include drivers to the mainline Linux OS and many smaller projects hosted on GitHub. His Linux passion feeds well into his passion for teaching these topics to engineers, which he has done for well over two decades now. He's also the author of Hands-On System Programming with Linux. It doesn't hurt that he is a recreational ultrarunner too.

## Other books by the authors
* [Hands-On System Programming with Linux](https://www.packtpub.com/in/networking-and-servers/hands-system-programming-linux?utm_source=github&utm_medium=repository&utm_campaign=9781788998475)
  
* * * * 
### Download a free PDF

 <i>If you have already purchased a print or Kindle version of this book, you can get a DRM-free PDF version at no cost.<br>Simply click on the link to claim your free PDF.</i>
<p align="center"> <a href="https://packt.link/free-ebook/9781789953435">https://packt.link/free-ebook/9781789953435 </a> </p>
