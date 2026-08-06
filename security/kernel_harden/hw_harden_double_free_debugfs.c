/*
 * hw_harden_double_free_debugfs.c
 *
 * Huawei double free check debugfs interface
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
#include <linux/random.h>
#include <asm/memory.h>
#include <asm/sections.h>

static struct dentry *slub_double_free_root;

#ifdef CONFIG_HW_SLUB_DF
#define MAX_LENGTH       16
#define USER_AREA_TEST   1
#define DOUBLE_FREE_TEST 2
#define PRESS_TEST       3
#define PRESS_TEST_TIMES 100000
#define LOOP_TIMES       10000
#define MAX_CACHE_SIZE   4600
#define MAX_STACK_LENGTH 1000
#define KMEM_CACHE_SIZE  128

static int slub_harden_double_free_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

/* Test when the data in object equals magic number hw_random_free */
static int slub_user_area_test(struct kmem_cache *my_cachep)
{
	void *object = NULL;
	unsigned long *canary = NULL;
	unsigned long random_free = my_cachep->hw_random_free;

	object = kmem_cache_alloc(my_cachep, GFP_KERNEL);
	if (!object) {
		pr_err("slub_user_area_test alloc object failed\n");
		return -ENOMEM;
	}
	canary = (unsigned long *)(object + sizeof(void *));
	*canary = (random_free ^ (unsigned long)canary) & CANARY_MASK;
	kfree(object);
	object = NULL;
	return 0;
}

static int slub_double_free_test(struct kmem_cache *my_cachep)
{
	void *object = NULL;
	void *another_object = NULL;

	object = kmem_cache_alloc(my_cachep, GFP_KERNEL);
	if (!object) {
		pr_err("slub_double_free_test alloc object failed\n");
		return -ENOMEM;
	}
	another_object = kmem_cache_alloc(my_cachep, GFP_KERNEL);
	if (!another_object) {
		pr_err("slub_double_free_test alloc object failed\n");
		kfree(object);
		object = NULL;
		return -ENOMEM;
	}
	kfree(object);
	kfree(another_object);
	/* free the object twice to verify kernel double_free */
	kfree(object);
	object = NULL;
	another_object = NULL;
	return 0;
}

static int slub_press_test(void)
{
	unsigned long count;
	unsigned long random = 0;
	unsigned long pos = 0;
	unsigned long loop;
	bool malloc_result = true;

	void **stack = kzalloc(sizeof(void *) * MAX_STACK_LENGTH, GFP_KERNEL);

	if (!stack)
		return -ENOMEM;
	for (loop = 0; loop < LOOP_TIMES; loop++) {
		for (count = 0; count < PRESS_TEST_TIMES; count++) {
			pos = get_random_long() % MAX_STACK_LENGTH;
			if (stack[pos] == NULL) {
				random = get_random_long() % MAX_CACHE_SIZE + 1;
				stack[pos] = kmalloc(random, GFP_KERNEL);
				if (stack[pos] == NULL) {
					pr_err("double free kmalloc fail\n");
					malloc_result = false;
					goto clear;
				}
			} else {
				kfree(stack[pos]);
				stack[pos] = NULL;
			}
		}
		msleep(20); // sleep 20ms
	}
clear:
	for (pos = 0; pos < MAX_STACK_LENGTH; pos++) {
		if (stack[pos] != NULL) {
			kfree(stack[pos]);
			stack[pos] = NULL;
		}
	}
	kfree(stack);
	if (!malloc_result) {
		pr_info("harden double free press test fail\n");
		return -ENOMEM;
	}
	pr_err("harden double free press test succ\n");
	return 0;
}

static ssize_t slub_harden_double_free_write(struct file *file,
					     const char __user *ubuf,
					     size_t count, loff_t *ppos)
{
	long result = -1;
	long ret;
	int test_result = 0;
	char buf[MAX_LENGTH] = {0};
	struct kmem_cache *my_cachep = NULL;
	unsigned int num;

	if ((count == 0) || (count > MAX_LENGTH)) {
		pr_err("double free test parameter failed\n");
		return -EINVAL;
	}

	num = copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count));
	if (num != 0) {
		pr_err("double free test copy data from user failed\n");
		return -EFAULT;
	}

	/* create kmem_cache allocate object which size is 128bytes */
	my_cachep = kmem_cache_create("my_cache", KMEM_CACHE_SIZE, 0,
				       SLAB_HWCACHE_ALIGN, NULL);
	if (!my_cachep)
		return -ENOMEM;
	/* convert the string to decimal number */
	ret = kstrtol(buf, 10, &result);
	if (ret)
		return ret;
	switch (result) {
	case USER_AREA_TEST:
		test_result = slub_user_area_test(my_cachep);
		break;
	case DOUBLE_FREE_TEST:
		test_result = slub_double_free_test(my_cachep);
		break;
	case PRESS_TEST:
		test_result = slub_press_test();
		break;
	default:
		break;
	}
	if (test_result != 0) {
		pr_err("double free slub test failed\n");
		kmem_cache_destroy(my_cachep);
		my_cachep = NULL;
		return -EINVAL;
	}
	kmem_cache_destroy(my_cachep);
	return count;
}

static ssize_t slub_harden_double_free_read(struct file *file,
					    char __user *ubuf,
					    size_t count, loff_t *ppos)
{
	const char *enable = "ENABLE\n";
	const char *disable = "DISABLE\n";
	char buf[MAX_LENGTH] = {0};
	int ret;

	/* create kmem_cache allocate object which size is 128bytes */
	struct kmem_cache *my_cachep = kmem_cache_create("my_cache",
		KMEM_CACHE_SIZE, 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!my_cachep)
		return -ENOMEM;
	if (my_cachep->flags & SLAB_DOUBLEFREE_CHECK)
		strncpy(buf, enable, strlen(enable) + 1);
	else
		strncpy(buf, disable, strlen(disable) + 1);
	ret = simple_read_from_buffer(ubuf, count, ppos, buf, strlen(buf));
	kmem_cache_destroy(my_cachep);
	return ret;
}

static const struct file_operations slub_harden_double_free_fops = {
	.owner = THIS_MODULE,
	.open = slub_harden_double_free_open,
	.read = slub_harden_double_free_read,
	.write = slub_harden_double_free_write,
};

#endif

static int __init hwslub_debugfs_init(void)
{
	struct dentry *debugfs_file = NULL;

	/* create sys/kernel/debug/hwslub_harden_double_free for debugfs */
	slub_double_free_root = debugfs_create_dir("hwslub_harden_double_free",
						    NULL);
	if (!slub_double_free_root)
		return -ENODEV;
#ifdef CONFIG_HW_SLUB_DF
	debugfs_file = debugfs_create_file("harden_double_free",
					    0444,
					    slub_double_free_root,
					    NULL,
					    &slub_harden_double_free_fops);
	if (!debugfs_file) {
		debugfs_remove(slub_double_free_root);
		slub_double_free_root = NULL;
		return -ENODEV;
	}
#endif
	return 0;
}
static void __exit hwslub_debugfs_exit(void)
{
	debugfs_remove(slub_double_free_root);
	slub_double_free_root = NULL;
}

module_init(hwslub_debugfs_init);
module_exit(hwslub_debugfs_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei harden double free debugfs");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
