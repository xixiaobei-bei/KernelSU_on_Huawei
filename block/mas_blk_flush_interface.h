/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description:  mas block flush reduce interface
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

#ifndef __MAS_BLK_FLUSH_INTERFACE_H__
#define __MAS_BLK_FLUSH_INTERFACE_H__
#include <linux/blkdev.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>

extern void __cfi_mas_blk_flush_work_fn(struct work_struct *work);
extern void mas_blk_flush_work_fn(const struct work_struct *work);
extern int __cfi_mas_blk_poweroff_flush_notifier_call(
	struct notifier_block *powerkey_nb, unsigned long event, void *data);
extern int mas_blk_poweroff_flush_notifier_call(
	const struct notifier_block *powerkey_nb, unsigned long event,
	const void *data);
extern int __init __cfi_mas_blk_flush_init(void);
extern int __init mas_blk_flush_init(void);
extern void mas_blk_flush_reduced_queue_unregister(struct request_queue *q);
extern void mas_blk_flush_list_register(struct list_head *lld_list);
extern void mas_blk_flush_list_unregister(struct list_head *lld_list);
#endif /* __MAS_BLK_FLUSH_INTERFACE_H__ */
