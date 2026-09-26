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

#ifndef __MAS_BKOPS_CORE_INTERFACE_H__
#define __MAS_BKOPS_CORE_INTERFACE_H__
#include <linux/blkdev.h>
#include <linux/mas_bkops_core.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>

extern void bkops_idle_work_func(const struct work_struct *work);
extern void __ufs_bkops_idle_work_func(struct work_struct *work);
extern enum blk_busyidle_callback_ret bkops_io_busyidle_notify_handler(
	const struct blk_busyidle_event_node *event_node,
	enum blk_idle_notify_state state);
extern enum blk_busyidle_callback_ret __cfi_bkops_io_busyidle_notify_handler(
	struct blk_busyidle_event_node *event_node,
	enum blk_idle_notify_state state);
extern int __cfi_bkops_pm_callback(
	struct notifier_block *nb, unsigned long action, void *ptr);
extern int bkops_pm_callback(
	const struct notifier_block *nb, unsigned long action, const void *ptr);
extern int bkops_notify_reboot(
	const struct notifier_block *this, unsigned long code,
	const void *no_use);
extern int __cfi_bkops_notify_reboot(
	struct notifier_block *this, unsigned long code, void *no_use);
extern int __cfi_mas_bkops_manual_gc_proc_show(struct seq_file *m, void *v);
extern int mas_bkops_manual_gc_proc_show(struct seq_file *m, const void *v);
extern int __cfi_mas_bkops_manual_gc_proc_open(
	struct inode *p_inode, struct file *p_file);
extern int mas_bkops_manual_gc_proc_open(
	const struct inode *p_inode, const struct file *p_file);
extern ssize_t __cfi_mas_bkops_manual_gc_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos);
extern ssize_t mas_bkops_manual_gc_proc_write(const struct file *p_file,
	const char __user *userbuf, size_t count, const loff_t *ppos);
extern int __cfi_mas_bkops_status_proc_show(struct seq_file *m, void *v);
extern int mas_bkops_status_proc_show(struct seq_file *m, const void *v);
extern int __cfi_mas_bkops_status_proc_open(
	struct inode *p_inode, struct file *p_file);
extern int mas_bkops_status_proc_open(
	const struct inode *p_inode, const struct file *p_file);
#endif /* __MAS_BKOPS_CORE_INTERFACE_H__ */
