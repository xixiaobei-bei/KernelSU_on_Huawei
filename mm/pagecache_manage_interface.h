/*
 * mm_hisi/pagecache_manage_interface.h
 *
 * Copyright(C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
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

#ifndef __HISI_PCH_MANAGE_INTERFACE_H__
#define __HISI_PCH_MANAGE_INTERFACE_H__
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/workqueue.h>

extern void __cfi_pch_timeout_work(struct work_struct *work);
extern void pch_timeout_work(struct work_struct *work);
extern void __cfi_pch_timeout_timer(unsigned long data);
extern void pch_timeout_timer(unsigned long data);
extern int __cfi_pch_emui_bootstat_proc_show(struct seq_file *m, void *v);
extern int pch_emui_bootstat_proc_show(struct seq_file *m, void *v);
extern int __cfi_pch_emui_bootstat_proc_open(struct inode *p_inode, struct file *p_file);
extern int pch_emui_bootstat_proc_open(struct inode *p_inode, struct file *p_file);
extern ssize_t __cfi_pch_emui_bootstat_proc_write(struct file *p_file,
		const char __user *userbuf, size_t count, loff_t *ppos);
extern ssize_t pch_emui_bootstat_proc_write(struct file *p_file,
		const char __user *userbuf, size_t count, loff_t *ppos);
extern int __init __cfi_hisi_page_cache_manage_init(void);
extern int __init hisi_page_cache_manage_init(void);
extern void __exit __cfi_hisi_page_cache_manage_exit(void);
extern void __exit hisi_page_cache_manage_exit(void);

#endif /* __HISI_PCH_MANAGE_INTERFACE_H__ */

