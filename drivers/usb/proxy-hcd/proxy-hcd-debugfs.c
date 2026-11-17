/*
 * proxy-hcd-debugfs.c
 *
 * utilityies for proxy-hcd debugging
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

#include "proxy-hcd.h"
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/usb.h>

#include "proxy-hcd-stat.h"

int phcd_seq_print_stat(struct proxy_hcd *phcd, struct seq_file *s)
{
	struct proxy_hcd_stat *phcd_stat = NULL;
	struct proxy_hcd_usb_device_stat *udev_stat = NULL;
	struct proxy_hcd_urb_stat *urb_stat = NULL;
	int i;

	if (phcd == NULL) {
		seq_puts(s, "proxy_hcd destroyed!\n");
		return -EINVAL;
	}

	phcd_stat = &phcd->stat;
	seq_printf(s, "phcd_stat %s: %ld\n", "stat_alloc_dev", (unsigned long)phcd_stat->stat_alloc_dev);
	seq_printf(s, "phcd_stat %s: %ld\n", "stat_free_dev", (unsigned long)phcd_stat->stat_free_dev);
	seq_printf(s, "phcd_stat %s: %ld\n", "stat_hub_control", (unsigned long)phcd_stat->stat_hub_control);
	seq_printf(s, "phcd_stat %s: %ld\n", "stat_hub_status_data", (unsigned long)phcd_stat->stat_hub_status_data);
	seq_printf(s, "phcd_stat %s: %ld\n", "last_hub_control_time", (unsigned long)phcd_stat->last_hub_control_time);
	seq_printf(s, "phcd_stat %s: %ld\n", "last_hub_status_data_time", (unsigned long)phcd_stat->last_hub_status_data_time);
	seq_printf(s, "phcd_stat %s: %ld\n", "stat_bus_suspend", (unsigned long)phcd_stat->stat_bus_suspend);
	seq_printf(s, "phcd_stat %s: %ld\n", "stat_bus_suspend", (unsigned long)phcd_stat->stat_bus_suspend);

	udev_stat = &phcd->phcd_udev.stat;
	seq_printf(s, "urb_stat %s: %ld\n", "stat_urb_complete_pipe_err",
		(unsigned long)udev_stat->stat_urb_complete_pipe_err);

	for (i = 0; i < PROXY_HCD_DEV_MAX_EPS; i++) {
		urb_stat = &phcd->phcd_udev.phcd_eps[i].urb_stat;

		if (urb_stat->stat_urb_enqueue == 0)
			continue;

		seq_printf(s, "--------- ep%d ---------\n", i);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_enqueue",
			(unsigned long)urb_stat->stat_urb_enqueue);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_dequeue",
			(unsigned long)urb_stat->stat_urb_dequeue);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_enqueue_fail",
			(unsigned long)urb_stat->stat_urb_enqueue_fail);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_dequeue_fail",
			(unsigned long)urb_stat->stat_urb_dequeue_fail);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_dequeue_giveback",
			(unsigned long)urb_stat->stat_urb_dequeue_giveback);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_giveback",
			(unsigned long)urb_stat->stat_urb_giveback);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_complete",
			(unsigned long)urb_stat->stat_urb_complete);
		seq_printf(s, "urb_stat.%s: %ld\n", "stat_urb_complete_fail",
			(unsigned long)urb_stat->stat_urb_complete_fail);
	}

	seq_printf(s, "client_ref: live %d, count %lu\n",
		phcd->client->client_ref.live,
		phcd->client->client_ref.count);

	return 0;
}

static int phcd_print_stat_show(struct seq_file *s, void *unused)
{
	struct proxy_hcd *phcd = s->private;

	return phcd_seq_print_stat(phcd, s);
}

static int phcd_print_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, phcd_print_stat_show, inode->i_private);
}

static const struct file_operations phcd_print_stat_fops = {
	.open			= phcd_print_stat_open,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

int phcd_debugfs_init(struct proxy_hcd *phcd)
{
	struct dentry *root = NULL;
	struct dentry *file = NULL;

	root = debugfs_create_dir("proxyhcd", usb_debug_root);
	if (IS_ERR_OR_NULL(root))
		return -ENOMEM;

	file = debugfs_create_file("stat", S_IRUSR, root,
		phcd, &phcd_print_stat_fops);
	if (file == NULL)
		goto file_null_err;

	phcd->debugfs_root = root;
	return 0;

file_null_err:
	debugfs_remove_recursive(root);
	return -ENOMEM;
}

void phcd_debugfs_exit(struct proxy_hcd *phcd)
{
	if (phcd->debugfs_root != NULL)
		debugfs_remove_recursive(phcd->debugfs_root);
}
