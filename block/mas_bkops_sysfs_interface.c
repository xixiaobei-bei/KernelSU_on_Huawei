/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: mas bkops sysfs interface
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

#ifdef CONFIG_MAS_DEBUG_FS
#include "mas_bkops_sysfs_interface.h"

int __cfi_mas_bkops_stat_open(struct inode *inode, struct file *filp)
{
	return mas_bkops_stat_open(inode, filp);
}

ssize_t __cfi_mas_bkops_stat_read(
	struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
	return mas_bkops_stat_read(filp, ubuf, cnt, ppos);
}

int __cfi_mas_bkops_stat_release(struct inode *inode, struct file *file)
{
	return mas_bkops_stat_release(inode, file);
}

int __cfi_mas_bkops_force_query_open(struct inode *inode, struct file *filp)
{
	return mas_bkops_force_query_open(inode, filp);
}

ssize_t __cfi_mas_bkops_force_query_read(
	struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
	return mas_bkops_force_query_read(filp, ubuf, cnt, ppos);
}

int __cfi_mas_bkops_force_query_release(struct inode *inode, struct file *file)
{
	return mas_bkops_force_query_release(inode, file);
}
#endif /* CONFIG_MAS_DEBUG_FS */
