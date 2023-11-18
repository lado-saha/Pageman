#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0x5e515be6, "ktime_get_ts64" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x6c78ae0, "get_nr_pages_metadata" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0xec578d69, "__free_pages" },
	{ 0xfe62ae8d, "set_nr_pages_metadata" },
	{ 0x365acda7, "set_normalized_timespec64" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x8fa36e7d, "gb_order" },
	{ 0x292f130d, "gb_size" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0xe007de41, "kallsyms_lookup_name" },
	{ 0x505d2266, "ftrace_set_filter_ip" },
	{ 0x5dd86fa2, "register_ftrace_function" },
	{ 0x14dd9f1d, "unregister_ftrace_function" },
	{ 0x470f423b, "is_fandf_loaded" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x6589fc2d, "gb_start_address" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x38eadbf6, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B120D7936321ABCB33D190C");
