/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: mas block core interface
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

#ifndef __MAS_BLK_CORE_INTERFACE_H__
#define __MAS_BLK_CORE_INTERFACE_H__
#include <linux/blkdev.h>
#include <linux/version.h>
#include <linux/timer.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
extern void _cfi_mas_blk_queue_usr_ctrl_recovery_timer_expire(
	struct timer_list *timer);
#else
extern void _cfi_mas_blk_queue_usr_ctrl_recovery_timer_expire(
	unsigned long data);
#endif
extern void mas_blk_queue_usr_ctrl_recovery_timer_expire(unsigned long data);
extern ssize_t __cfi_mas_queue_status_show(
	struct request_queue *q, char *page);
extern ssize_t mas_queue_status_show(
	const struct request_queue *q, char *page, unsigned long len);
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
extern ssize_t __cfi_mas_queue_io_prio_sim_show(
	struct request_queue *q, char *page);
extern ssize_t mas_queue_io_prio_sim_show(
	const struct request_queue *q, char *page);
extern ssize_t __cfi_mas_queue_io_prio_sim_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t mas_queue_io_prio_sim_store(
	struct request_queue *q, const char *page, size_t count);
#endif
#endif
