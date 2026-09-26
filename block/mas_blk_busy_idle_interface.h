/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: block busy idle interface
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

#ifndef __MAS_BLK_BUSY_IDLE_INTERFACE_H__
#define __MAS_BLK_BUSY_IDLE_INTERFACE_H__
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/timer.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
extern void __cfi_mas_blk_busyidle_handler_latency_check_timer_expire(
	struct timer_list *timer);
#else
extern void __cfi_mas_blk_busyidle_handler_latency_check_timer_expire(
	unsigned long data);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
extern void mas_blk_busyidle_handler_latency_check_timer_expire(
	struct timer_list *timer);
#else
extern void mas_blk_busyidle_handler_latency_check_timer_expire(
	unsigned long data);
#endif
extern int __cfi_mas_blk_busyidle_notify_handler(
	struct notifier_block *nb, unsigned long val, void *v);
extern int mas_blk_busyidle_notify_handler(
	const struct notifier_block *nb, unsigned long val, const void *v);
extern void __cfi_mas_blk_idle_notify_work(struct work_struct *work);
extern void mas_blk_idle_notify_work(const struct work_struct *work);
extern ssize_t __cfi_mas_queue_busyidle_enable_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t __cfi_mas_queue_busyidle_statistic_reset_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t __cfi_mas_queue_busyidle_statistic_show(
	struct request_queue *q, char *page);
extern ssize_t __cfi_mas_queue_hw_idle_enable_show(
	struct request_queue *q, char *page);
extern ssize_t __cfi_mas_queue_idle_state_show(
	struct request_queue *q, char *page);
extern void __cfi_mas_blk_busyidle_end_rq(
	struct request *rq, blk_status_t error);
extern void mas_blk_busyidle_end_rq(
	const struct request *rq, blk_status_t error);
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
extern ssize_t mas_queue_busyidle_enable_store(
	const struct request_queue *q, const char *page, size_t count);
extern ssize_t mas_queue_busyidle_statistic_reset_store(
	const struct request_queue *q, const char *page, size_t count);
extern ssize_t mas_queue_busyidle_statistic_show(
	const struct request_queue *q, char *page, unsigned long len);
extern ssize_t mas_queue_hw_idle_enable_show(
	const struct request_queue *q, char *page, unsigned long len);
extern ssize_t mas_queue_idle_state_show(
	const struct request_queue *q, char *page, unsigned long len);
#endif /* CONFIG_MAS_DEBUG_FS */
#endif /* __MAS_BLK_BUSY_IDLE_INTERFACE_H__ */
