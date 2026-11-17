/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: mas block io latency interface
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

#ifndef __MAS_BLK_LATENCY_INTERFACE_H__
#define __MAS_BLK_LATENCY_INTERFACE_H__
#include <linux/blk-mq.h>
#include <linux/version.h>
#include <linux/timer.h>

#include "blk.h"
#include "mas_blk_latency_interface.h"

extern const struct bio_delay_stage_config bio_stage_cfg[BIO_PROC_STAGE_MAX];
extern const struct req_delay_stage_config req_stage_cfg[REQ_PROC_STAGE_MAX];

extern void mas_blk_latency_check_timer_expire(unsigned long data);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
extern void __mas_blk_latency_check_timer_expire(struct timer_list *timer);
#else
extern void __mas_blk_latency_check_timer_expire(unsigned long data);
#endif
extern ssize_t mas_queue_io_latency_warning_threshold_store(
	const struct request_queue *q, const char *page, size_t count);
extern ssize_t __mas_queue_io_latency_warning_threshold_store(
	struct request_queue *q, const char *page, size_t count);
ssize_t mas_queue_tz_write_bytes_show(struct request_queue *q, char *page);

#endif /* __MAS_BLK_LATENCY_INTERFACE_H__ */
