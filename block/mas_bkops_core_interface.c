/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: mas bkops core interface
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>

#include "mas_bkops_core_interface.h"

void __ufs_bkops_idle_work_func(struct work_struct *work)
{
	bkops_idle_work_func(work);
}

enum blk_busyidle_callback_ret __cfi_bkops_io_busyidle_notify_handler(
	struct blk_busyidle_event_node *event_node,
	enum blk_idle_notify_state state)
{
	return bkops_io_busyidle_notify_handler(event_node, state);
}

int __cfi_bkops_pm_callback(
	struct notifier_block *nb, unsigned long action, void *ptr)
{
	return bkops_pm_callback(nb, action, ptr);
}

int __cfi_bkops_notify_reboot(
	struct notifier_block *this, unsigned long code, void *no_use)
{
	return bkops_notify_reboot(this, code, no_use);
}

int __cfi_mas_bkops_manual_gc_proc_show(struct seq_file *m, void *v)
{
	return mas_bkops_manual_gc_proc_show(m, v);
}

int __cfi_mas_bkops_manual_gc_proc_open(
	struct inode *p_inode, struct file *p_file)
{
	return mas_bkops_manual_gc_proc_open(p_inode, p_file);
}

ssize_t __cfi_mas_bkops_manual_gc_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos)
{
	return mas_bkops_manual_gc_proc_write(p_file, userbuf, count, ppos);
}

int __cfi_mas_bkops_status_proc_show(struct seq_file *m, void *v)
{
	return mas_bkops_status_proc_show(m, v);
}

int __cfi_mas_bkops_status_proc_open(
	struct inode *p_inode, struct file *p_file)
{
	return mas_bkops_status_proc_open(p_inode, p_file);
}
