/*
 * memcheck_inject.c
 *
 * inject all kind of kernel leak
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#include "memcheck_common.h"
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/ion.h>
#include <linux/skbuff.h>
#include <linux/zsmalloc.h>
#include <linux/netdevice.h>
#include <chipset_common/hwmemcheck/memcheck.h>

#define MAX_BUF			16
#define FREE_FREQ		2
#define GRO_MAX_HEAD		(MAX_HEADER + 128)

#define LEAK_SLUB_SIZE		(PAGE_SIZE / 2)
#define LEAK_LSLUB_SIZE		(4 * PAGE_SIZE)
#define LEAK_ZSMALLOC_SIZE	(PAGE_SIZE / 2)
#define LEAK_SKB_SIZE	((GRO_MAX_HEAD + NET_SKB_PAD + NET_IP_ALIGN) * 2)

struct zs_pool;
struct leak_item {
	int is_on;
	char *name;
	struct task_struct *task;
	void (*func)(void);
};
static struct dentry *leak_root;

static int stop_inject_thread(struct leak_item *p);
static int create_inject_thread(struct leak_item *p);

#define DEBUGFS_LEAK_TYPE(name)						\
	static void memcheck_inject_##name##_leak(void);		\
	static struct leak_item leak_##name##_item = {			\
		.is_on = 0,						\
		.task = NULL,};						\
	static int attr_##name##_show(struct seq_file *m, void *private)\
	{								\
		if (leak_##name##_item.is_on == 1)			\
			seq_printf(m, "on\n");				\
		else if (leak_##name##_item.is_on == 0)			\
			seq_printf(m, "off\n");				\
		else							\
			seq_printf(m, "error!\n");			\
		return 0;						\
	}								\
	static ssize_t attr_##name##_write(struct file *file,		\
					const char __user *ubuf,	\
					size_t len, loff_t *offp)	\
	{								\
		char buf[MAX_BUF];					\
		size_t cnt = len;					\
									\
		if (cnt > (sizeof(buf) - 1)) {				\
			return -EINVAL;					\
		}							\
		memset(buf, 0, sizeof(buf));				\
		if (copy_from_user(buf, ubuf, cnt))			\
			return -EINVAL;					\
		while ((cnt > 0) && ((buf[cnt-1] == ' ') ||		\
		       (buf[cnt-1] == '\n')))				\
			cnt--;						\
		buf[cnt] = '\0';					\
		if (!strncmp(buf, "on", cnt))				\
			leak_##name##_item.is_on = 1;			\
		else if (!strncmp(buf, "off", cnt))			\
			leak_##name##_item.is_on = 0;			\
		else							\
			return -EINVAL;					\
		leak_##name##_item.func = memcheck_inject_##name##_leak;\
		if (leak_##name##_item.is_on)				\
			create_inject_thread(&leak_##name##_item);	\
		else							\
			stop_inject_thread(&leak_##name##_item);	\
		return len;						\
	}								\
	static int attr_##name##_open(struct inode *inode,		\
					struct file *file)		\
	{								\
		return single_open(file, attr_##name##_show,		\
					inode->i_private);		\
	}								\
	static const struct file_operations fs_##name##_fops = {	\
			.owner = THIS_MODULE,				\
			.open = attr_##name##_open,			\
			.read = seq_read,				\
			.llseek = seq_lseek,				\
			.release = single_release,			\
			.write = attr_##name##_write,			\
	}

#define DEBUGFS_LEAK_VALUE(name, def, min, max)				\
	static size_t memcheck_leak_##name##_val = (def);		\
	static int attr_##name##_val_show(struct seq_file *m,		\
						void *private)		\
	{								\
		seq_printf(m, "%zu\n", memcheck_leak_##name##_val);	\
		return 0;						\
	}								\
	static ssize_t attr_##name##_val_write(struct file *file,	\
						const char __user *ubuf,\
						size_t len, loff_t *off)\
	{								\
		unsigned long temp = (def);				\
		char buf[MAX_BUF];					\
		size_t cnt = len;					\
									\
		if (cnt > (sizeof(buf) - 1))				\
			return -EINVAL;					\
		memset(buf, 0, sizeof(buf));				\
		if (copy_from_user(buf, ubuf, cnt))			\
			return -EINVAL;					\
		while ((cnt > 0) &&					\
		       ((buf[cnt-1] == ' ') || (buf[cnt-1] == '\n')))	\
			cnt--;						\
		buf[cnt] = '\0';					\
		if (kstrtoul(buf, 10, &temp))				\
			return -EINVAL;					\
		if ((temp < (min)) || (temp > (max)))			\
			return -EINVAL;					\
		memcheck_leak_##name##_val = temp;			\
		return len;						\
	}								\
	static int attr_##name##_val_open(struct inode *inode,		\
						struct file *file)	\
	{								\
			return single_open(file, attr_##name##_val_show,\
						inode->i_private);	\
	}								\
	static const struct file_operations fs_##name##_fops = {	\
			.owner = THIS_MODULE,				\
			.open = attr_##name##_val_open,			\
			.read = seq_read,				\
			.llseek = seq_lseek,				\
			.release = single_release,			\
			.write = attr_##name##_val_write,		\
	}

DEBUGFS_LEAK_VALUE(size, 1024 * 1024, 1024, 100 * 1024 * 1024);
DEBUGFS_LEAK_VALUE(interval, 1, 1, 60);

static int inject_thread_func(void *data)
{
	struct leak_item *leak_item = NULL;

	if (!data)
		return -EINVAL;

	leak_item = (struct leak_item *)data;
	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		if (kthread_should_stop())
			break;
		leak_item->func();
		mdelay(memcheck_leak_interval_val * 1000);
	}

	return 0;
}

static int create_inject_thread(struct leak_item *p)
{
	int ret = 0;

	if (!p) {
		memcheck_err("p is null\n");
		return -EINVAL;
	}
	if (p->task) {
		memcheck_err("inject thread is already running\n");
		return -EFAULT;
	}
	p->task = kthread_create(inject_thread_func, p, "memleak_inject");
	if (IS_ERR(p->task)) {
		memcheck_err("kthread_create failed\n");
		ret = PTR_ERR(p->task);
		p->task = NULL;
		return ret;
	}

	wake_up_process(p->task);

	return ret;
}

static int stop_inject_thread(struct leak_item *p)
{
	if (!p) {
		memcheck_err("is null\n");
		return -EINVAL;
	}
	if (!p->task) {
		memcheck_err("inject thread is not running\n");
		return -EFAULT;
	}
	kthread_stop(p->task);
	p->task = NULL;

	return 0;
}

static void memcheck_inject_buddy_leak(void)
{
	int i;
	struct page *p = NULL;
	int num = (memcheck_leak_size_val - 1) / PAGE_SIZE + 1;

	for (i = 0; i < num; i++) {
		p = alloc_pages(GFP_KERNEL, 0);
		if (!p) {
			memcheck_err("alloc_pages failed, i=%d, num=%d\n", i,
				     num);
			return;
		}
	}
	memcheck_info("memcheck_inject_buddy_leak success, num=%d\n", num);
}

static void memcheck_inject_slub_leak(void)
{
	int i;
	void *p = NULL;
	int num = memcheck_leak_size_val / LEAK_SLUB_SIZE + 1;

	for (i = 0; i < num * FREE_FREQ; i++) {
		p = kmalloc(LEAK_SLUB_SIZE, GFP_KERNEL);
		if (!p) {
			memcheck_err("kmalloc failed, i=%d, num=%d\n", i, num);
			return;
		}
		if (i % FREE_FREQ)
			kfree(p);
	}
	memcheck_info("memcheck_inject_slub_leak success, size=%zu, num=%d\n",
		      memcheck_leak_size_val, num);
}

static void memcheck_inject_lslub_leak(void)
{
	int i;
	int num = memcheck_leak_size_val / LEAK_LSLUB_SIZE + 1;

	for (i = 0; i < num; i++) {
		if (!kmalloc(LEAK_LSLUB_SIZE, GFP_KERNEL))
			return;
	}
	memcheck_info("memcheck_inject_lslub_leak success, size=%zu, num=%d\n",
		      memcheck_leak_size_val, num);
}

static void memcheck_inject_vmalloc_leak(void)
{
	if (!vmalloc(memcheck_leak_size_val))
		return;

	memcheck_info("memcheck_inject_vmalloc_leak success, size=%zu\n",
		      memcheck_leak_size_val);
}

static void memcheck_inject_skb_leak(void)
{
	int i;
	/* about 710B per skb */
	int num = memcheck_leak_size_val / LEAK_SKB_SIZE + 1;

	for (i = 0; i < num; i++) {
		struct napi_struct napi;
		struct sk_buff *skbuf = NULL;

		memset(&napi, 0, sizeof(napi));
		skbuf = napi_get_frags(&napi);
		if (!skbuf) {
			memcheck_err("napi_get_frags failed, i=%d, num=%d\n", i,
				     num);
			return;
		}
	}
	memcheck_info("memcheck_inject_skb_leak success, num=%d\n", num);
}

/*
 * This is a samle function usage flow
 * 1、zs_create_pool 2、zs_malloc 3、zs_map_object 4、zs_unmap_object
zs_unmap_objectzs_destroy_pool
 */
static void memcheck_inject_zsmalloc_leak(void)
{
	int i;
	struct zs_pool *pool = NULL;
	int num = memcheck_leak_size_val / LEAK_ZSMALLOC_SIZE + 1;

	pool = zs_create_pool("inject_zsmalloc_pool");
	if (!pool) {
		memcheck_err("zs_create_pool failed\n");
		return;
	}
	for (i = 0; i < num; i++) {
		unsigned long handle = zs_malloc(pool, LEAK_ZSMALLOC_SIZE,
						 GFP_NOIO | __GFP_HIGHMEM);

		if (!handle) {
			memcheck_err("zs_malloc failed, i=%d, num=%d\n", i,
				     num);
			return;
		}
	}
	memcheck_info("memcheck_inject_zsmalloc_leak success, num=%d\n", num);
}

DEBUGFS_LEAK_TYPE(buddy);
DEBUGFS_LEAK_TYPE(slub);
DEBUGFS_LEAK_TYPE(lslub);
DEBUGFS_LEAK_TYPE(vmalloc);
DEBUGFS_LEAK_TYPE(skb);
DEBUGFS_LEAK_TYPE(zsmalloc);

static int __init memcheck_init(void)
{
	leak_root = debugfs_create_dir("memcheck_inject", NULL);

	if (!leak_root)
		return -ENXIO;

	debugfs_create_file("buddy", 0444, leak_root, NULL, &fs_buddy_fops);
	debugfs_create_file("slub", 0444, leak_root, NULL, &fs_slub_fops);
	debugfs_create_file("lslub", 0444, leak_root, NULL, &fs_lslub_fops);
	debugfs_create_file("vmalloc", 0444, leak_root, NULL, &fs_vmalloc_fops);
	debugfs_create_file("skb", 0444, leak_root, NULL, &fs_skb_fops);
	debugfs_create_file("zsmalloc", 0444, leak_root, NULL,
			    &fs_zsmalloc_fops);

	debugfs_create_file("size", 0444, leak_root, NULL, &fs_size_fops);
	debugfs_create_file("interval", 0444, leak_root, NULL,
			    &fs_interval_fops);

	return 0;
}

static void __exit memcheck_exit(void)
{
	debugfs_remove(leak_root);
}

module_init(memcheck_init);
module_exit(memcheck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei Memory Leak Inject debugfs");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

