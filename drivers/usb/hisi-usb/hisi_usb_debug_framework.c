/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hiusb debug framework
 * Create: 2020-08-13
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/hisi/usb/chip_usb_debug_framework.h>

#undef pr_fmt
#define pr_fmt(fmt) "[hiusb_debug]%s: " fmt, __func__

#define USB_DEBUG_MODULE_INIT_SUCESS 0x55

struct usb_debug_block_event_info {
	struct work_struct event_work;
	struct kfifo_rec_ptr_1 kfifo;
	spinlock_t buf_lock;
	int init_success;
};

/* record err event info */
struct usb_err_event_type {
	enum usb_debug_event_type event_type;
	unsigned int count;
	unsigned long time_stamp;
};

static struct usb_err_event_type g_usb_event_info[USB_DEBUG_EVENT_MAX];
static struct usb_debug_block_event_info g_block_event_info;
struct dentry *g_usbdebug_root;

#define USB_DEBUG_KFIFO_SIZE 128
#define USB_DEBUG_WRITE_BUF_SIZE 64

static int usb_debug_event_dequeue(unsigned int *event_type)
{
	return kfifo_out_spinlocked(&g_block_event_info.kfifo, event_type, 1,
				&g_block_event_info.buf_lock);
}

static void usb_debug_handle_block_event(struct work_struct *data)
{
	unsigned int event_type = 0;

	while (usb_debug_event_dequeue(&event_type)) {
		/* notify block */
		pr_err("record_event type: %u\n", event_type);
		usb_notify_blockerr_occur(event_type);
	}
}

static void usb_debug_record_errevent(unsigned int event_type)
{
	g_usb_event_info[event_type].count++;
	g_usb_event_info[event_type].time_stamp = jiffies;
}

const char *usb_debug_event_type_string(enum usb_debug_event_type event_type)
{
	static const char *err_type_strings[] = {
		[USB_GADGET_DEVICE_RESET] = "USB_GADGET_DEVICE_RESET",
		[USB_CORE_HOST_ENUM_ERR] = "USB_CORE_HOST_ENUM_ERR",
		[USB_CORE_HOST_RESUME_ERR] = "USB_CORE_HOST_RESUME_ERR",
		[USB_CORE_HOST_TRANS_TIMEOUT] = "USB_CORE_HOST_TRANS_TIMEOUT",
	};

	if (event_type >= ARRAY_SIZE(err_type_strings))
		return "illegal err event type";

	if (err_type_strings[event_type] == NULL)
		return "illegal err event type";

	return err_type_strings[event_type];
}

static int usb_debug_errevent_show(struct seq_file *s, void *d)
{
	unsigned int index;

	for (index = 0; index < USB_DEBUG_EVENT_MAX; ++index)
		seq_printf(s, "event_type:[%s],occur_count:[%u],last_time:[%lu]\n",
				usb_debug_event_type_string(index),
				g_usb_event_info[index].count,
				g_usb_event_info[index].time_stamp);

	return 0;
}

static int usb_debug_errevent_open(struct inode *inode, struct file *file)
{
	return single_open(file, usb_debug_errevent_show, inode->i_private);
}

static void usb_debug_add_err_event(unsigned int event_type)
{
	pr_err("+\n");

	if (event_type >= USB_DEBUG_EVENT_MAX) {
		pr_err("invalid usb error event type\n");
		return;
	}

	usb_debug_event_notify(event_type);
	pr_err("-\n");
}

static ssize_t usb_debug_errevent_write(struct file *file,
			const char __user *buf, size_t size, loff_t *ppos)
{
	int ret;
	unsigned int flag = 0;

	pr_err("+\n");
	if (buf == NULL)
		return -EINVAL;

	if (size >= USB_DEBUG_WRITE_BUF_SIZE) {
		pr_err("set cmd too long!\n");
		return -ENOMEM;
	}

	ret = kstrtouint_from_user(buf, size, 0, &flag);
	if (ret < 0) {
		pr_err("Invalid value\n");
		return ret;
	}

	pr_err("send cmd: %u, %lu\n", flag, size);
	usb_debug_add_err_event(flag);

	pr_err("-\n");
	return size;
}

static const struct file_operations usb_debug_err_event_fops = {
	.open = usb_debug_errevent_open,
	.read = seq_read,
	.write = usb_debug_errevent_write,
	.release = single_release,
};

void usb_debug_event_notify(unsigned int event_type)
{
	int ret;

	pr_err("+\n");

	if (event_type >= USB_DEBUG_EVENT_MAX) {
		pr_err("usb err type is NULL\n");
		return;
	}

	if (g_block_event_info.init_success != USB_DEBUG_MODULE_INIT_SUCESS) {
		pr_err("block event info not init\n");
		return;
	}

	usb_debug_record_errevent(event_type);

	if (in_atomic() || irqs_disabled())
		usb_notify_atomicerr_occur(event_type);

	spin_lock(&g_block_event_info.buf_lock);
	ret = kfifo_in(&g_block_event_info.kfifo, &event_type, 1);
	pr_err("kfifo_in ret = %d\n", ret);
	spin_unlock(&g_block_event_info.buf_lock);

	if (!queue_work(system_power_efficient_wq, &g_block_event_info.event_work))
		pr_err("queue event worker failed\n");

	pr_err("-\n");
}

static void usb_debug_init_err_event(void)
{
	unsigned int index;

	for (index = 0; index < USB_DEBUG_EVENT_MAX; ++index) {
		g_usb_event_info[index].count = 0;
		g_usb_event_info[index].event_type = (enum usb_debug_event_type)index;
		g_usb_event_info[index].time_stamp = 0;
	}
}

int __init usb_debug_framework_init(void)
{
	int ret;
	struct dentry *errevent = NULL;

	pr_err("+");
	/* init usb debug framework event */
	usb_debug_init_err_event();

	/* init usb debugfs */
	g_usbdebug_root = debugfs_create_dir("usbdebug", usb_debug_root);
	if (IS_ERR_OR_NULL(g_usbdebug_root)) {
		pr_err("create usbdebug root failed\n");
		g_usbdebug_root = NULL;
		return -EINVAL;
	}

	errevent = debugfs_create_file("errevent", S_IWUSR | S_IRUSR, g_usbdebug_root, NULL, &usb_debug_err_event_fops);
	if (IS_ERR_OR_NULL(errevent)) {
		pr_err("create errevent debugfs failed\n");
		ret = -EINVAL;
		goto err_init_usbdebug;
	}

	/* init work events and kfifo */
	INIT_WORK(&g_block_event_info.event_work, usb_debug_handle_block_event);
	ret = kfifo_alloc(&g_block_event_info.kfifo, USB_DEBUG_KFIFO_SIZE, GFP_KERNEL);
	if (ret < 0) {
		pr_err("kfifo alloc faied %d\n", ret);
		goto err_init_usbdebug;
	}

	spin_lock_init(&g_block_event_info.buf_lock);
	g_block_event_info.init_success = USB_DEBUG_MODULE_INIT_SUCESS;

	pr_err("-\n");
	return 0;

err_init_usbdebug:
	debugfs_remove_recursive(g_usbdebug_root);
	g_usbdebug_root = NULL;

	return ret;
}
module_init(usb_debug_framework_init);

void __exit usb_debug_framework_exit(void)
{
	/* free debug_nb_list */
	g_block_event_info.init_success = 0;
	cancel_work_sync(&g_block_event_info.event_work);
	kfifo_free(&g_block_event_info.kfifo);
	memset(&g_block_event_info, 0, sizeof(g_block_event_info));
	/* free usbdebug file */
	debugfs_remove_recursive(g_usbdebug_root);
	g_usbdebug_root = NULL;
}
module_exit(usb_debug_framework_exit);

