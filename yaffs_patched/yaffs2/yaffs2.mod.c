#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xe189b8a7, "struct_module" },
	{ 0x8dc513b8, "per_cpu__current_task" },
	{ 0xcba6c63f, "kmalloc_caches" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xb6f42f30, "__mark_inode_dirty" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xed9fc2f3, "page_address" },
	{ 0x74cc238d, "current_kernel_time" },
	{ 0x20000329, "simple_strtoul" },
	{ 0x971e7480, "generic_file_aio_read" },
	{ 0x7b627719, "remove_proc_entry" },
	{ 0x5d9020ee, "vfs_follow_link" },
	{ 0xda11b20d, "vfs_readlink" },
	{ 0xb1054d4e, "generic_read_dir" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x8a062c33, "generic_file_aio_write" },
	{ 0xef1815ad, "inode_setattr" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x8d3894f2, "_ctype" },
	{ 0x1b7d4074, "printk" },
	{ 0x8a1848ff, "d_rehash" },
	{ 0x7441f98b, "put_mtd_device" },
	{ 0xf65892af, "d_alloc_root" },
	{ 0x5568be43, "lock_kernel" },
	{ 0xa2c58ca1, "kunmap" },
	{ 0x6446534d, "unlock_page" },
	{ 0x5dfe8f1a, "unlock_kernel" },
	{ 0xd5976294, "kmem_cache_alloc" },
	{ 0xef65aa4a, "generic_file_mmap" },
	{ 0x59ffc0dd, "generic_file_sendfile" },
	{ 0xab2a8745, "kmap" },
	{ 0x31f977e9, "bdevname" },
	{ 0x72216fa9, "param_get_uint" },
	{ 0x4292364c, "schedule" },
	{ 0xe518189f, "do_sync_read" },
	{ 0x614b0470, "unlock_new_inode" },
	{ 0x8942d81f, "kill_block_super" },
	{ 0x6345aeb8, "create_proc_entry" },
	{ 0x8b557df6, "get_mtd_device" },
	{ 0xbf1c6598, "inode_change_ok" },
	{ 0xa02babf4, "proc_root" },
	{ 0x7a71da5e, "register_filesystem" },
	{ 0xffd3c7, "init_waitqueue_head" },
	{ 0x91ab30ba, "iput" },
	{ 0x37a0cba, "kfree" },
	{ 0xb6fe27d5, "do_sync_write" },
	{ 0x8abac70a, "param_set_uint" },
	{ 0x67cd9411, "get_sb_bdev" },
	{ 0xa22a4f74, "put_page" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0xd04d493a, "unregister_filesystem" },
	{ 0xe720554, "init_special_inode" },
	{ 0x96b27088, "__down_failed" },
	{ 0xe9246339, "clear_inode" },
	{ 0xd7d3429a, "d_instantiate" },
	{ 0x92ac7bf4, "iget_locked" },
	{ 0x7ea5f3fb, "truncate_inode_pages" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=mtd";


MODULE_INFO(srcversion, "EE0661AD1F9577F8AC2C7B0");
