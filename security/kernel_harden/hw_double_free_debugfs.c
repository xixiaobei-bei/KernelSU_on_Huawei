/*
 * hw_double_free_debugfs.c
 *
 * Huawei double free check debugfs interface
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/slub_def.h>
#include <linux/delay.h>
#include <linux/mm_types.h>
#include <asm/memory.h>
#include <asm/sections.h>

#define KMEM_CACHE_SIZE 128
static struct dentry *slub_double_free_root;

#define SLUB_DEBUG_ENTRY(name) \
static int slub_##name##_open(struct inode *inode, struct file *file) \
{ \
	return single_open(file, slub_##name##_show, inode->i_private); \
} \
\
static const struct file_operations slub_##name##_fops = { \
	.owner = THIS_MODULE, \
	.open = slub_##name##_open, \
	.read = seq_read, \
	.llseek = seq_lseek, \
	.release = single_release, \
}

#ifdef CONFIG_HW_SLUB_SANITIZE
static int slub_double_free_show(struct seq_file *m, void *v)
{
	void *freelist = NULL;
	void *object = NULL;

	/* create kmem_cache allocate object which size is 128bytes */
	struct kmem_cache *my_cachep = kmem_cache_create("my_cache",
		KMEM_CACHE_SIZE, 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!my_cachep)
		return -ENOMEM;
	freelist = this_cpu_read(my_cachep->cpu_slab->freelist);
	object = kmem_cache_alloc(my_cachep, GFP_KERNEL);
	if (!object) {
		kmem_cache_destroy(my_cachep);
		my_cachep = NULL;
		return -ENOMEM;
	}
	seq_printf(m, "The freelist object's addr is 0x%pK\n", freelist);
	seq_printf(m, "The allocated object's addr is 0x%pK\n", object);
	kfree(object);
	freelist = (void *)this_cpu_read(my_cachep->cpu_slab->freelist);
	seq_printf(m, "The freelist object's addr is 0x%pK\n", freelist);

#ifdef CONFIG_HW_SLUB_DF
	if (!(my_cachep->flags & SLAB_DOUBLEFREE_CHECK))
#else
	if (!(my_cachep->flags & SLAB_CLEAR))
#endif
		seq_puts(m, "Before object is double freed, Don't detect it\n");
	/* free the object twice to verify kernel heap protect project */
	kfree(object);
	object = NULL;
#ifdef CONFIG_HW_SLUB_DF
	if (my_cachep->flags & SLAB_DOUBLEFREE_CHECK)
#else
	if (my_cachep->flags & SLAB_CLEAR)
#endif
		seq_puts(m, "After object is double freed, Detect it\n");
	kmem_cache_destroy(my_cachep);
	return 0;
}
#endif

#ifdef CONFIG_HW_SLUB_SANITIZE
SLUB_DEBUG_ENTRY(double_free);
#endif

static int __init hwslub_double_free_debugfs_init(void)
{
	struct dentry *debugfs_file = NULL;

	/* create sys/kernel/debug/hwslub_double_free for debugfs */
	slub_double_free_root = debugfs_create_dir("hwslub_double_free", NULL);
	if (!slub_double_free_root)
		return -ENODEV;
#ifdef CONFIG_HW_SLUB_SANITIZE
	debugfs_file = debugfs_create_file("double_free",
					    0444,
					    slub_double_free_root,
					    NULL,
					    &slub_double_free_fops);
	if (!debugfs_file) {
		debugfs_remove(slub_double_free_root);
		slub_double_free_root = NULL;
		return -ENODEV;
	}
#endif
	return 0;
}

static void __exit hwslub_double_free_debugfs_exit(void)
{
	debugfs_remove(slub_double_free_root);
	slub_double_free_root = NULL;
}

module_init(hwslub_double_free_debugfs_init);
module_exit(hwslub_double_free_debugfs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei double free debugfs");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
