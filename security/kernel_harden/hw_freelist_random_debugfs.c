/*
 * hw_freelist_random_debugfs.c
 *
 * Huawei freelist random debugfs interface
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#define OO_MASK               65535
#define KMEM_CACHE_SIZE_ONE   128
#define KMEM_CACHE_SIZE_TWO   192
#define KMEM_CACHE_SIZE_THREE 256
static struct dentry *slub_freelist_root;

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

#ifdef CONFIG_SLAB_FREELIST_RANDOM
static int slub_freelist_random_show_num(struct seq_file *m, void *v,
					 unsigned int num)
{
	unsigned int object_num;
	unsigned int size;

	struct kmem_cache *my_cachep = kmem_cache_create("my_cache", num,
		0, SLAB_HWCACHE_ALIGN, NULL);
	if (!my_cachep)
		return -ENOMEM;
	object_num = my_cachep->oo.x & OO_MASK;
	size = my_cachep->size;
	if (size == 0) {
		kmem_cache_destroy(my_cachep);
		my_cachep = NULL;
		return -ENOMEM;
	}
	seq_printf(m, "The name of kmem_cache is %s\n", my_cachep->name);
	/*
	 * object_num is set when kmem_cache create,
	 * it is the length of random_seq, overflow couldn't happen
	 */
	if (my_cachep->random_seq) {
		unsigned int i;

		for (i = 0; i < object_num; i++)
			seq_printf(m, "%s->random_seq[%d] is %d\n",
				   my_cachep->name, i,
				   my_cachep->random_seq[i]/size);
	}
	kmem_cache_destroy(my_cachep);
	return 0;
}

static int slub_freelist_random_show(struct seq_file *m, void *v)
{
	/*
	 * create kmem_cache allocate object that length
	 * is 128bytes, 192bytes, 256bytes
	 */
	slub_freelist_random_show_num(m, v, KMEM_CACHE_SIZE_ONE);
	slub_freelist_random_show_num(m, v, KMEM_CACHE_SIZE_TWO);
	slub_freelist_random_show_num(m, v, KMEM_CACHE_SIZE_THREE);
	return 0;
}
#endif

#ifdef CONFIG_SLAB_FREELIST_RANDOM
SLUB_DEBUG_ENTRY(freelist_random);
#endif

static int __init hw_freelist_random_debugfs_init(void)
{
	struct dentry *debug_file = NULL;

	/* create sys/kernel/debug/hwslub for debugfs */
	slub_freelist_root = debugfs_create_dir("hwslub", NULL);
	if (!slub_freelist_root)
		return -ENODEV;
#ifdef CONFIG_SLAB_FREELIST_RANDOM
	debug_file = debugfs_create_file("freelist_random",
					  0444,
					  slub_freelist_root,
					  NULL,
					  &slub_freelist_random_fops);
	if (!debug_file) {
		debugfs_remove(slub_freelist_root);
		slub_freelist_root = NULL;
		return -ENODEV;
	}
#endif
	return 0;
}

static void __exit hw_freelist_random_debugfs_exit(void)
{
	debugfs_remove(slub_freelist_root);
	slub_freelist_root = NULL;
}

module_init(hw_freelist_random_debugfs_init);
module_exit(hw_freelist_random_debugfs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei Freelist Random debugfs");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
