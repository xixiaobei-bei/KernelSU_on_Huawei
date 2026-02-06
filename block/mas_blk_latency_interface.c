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

#include <linux/kernel.h>
#include <linux/blkdev.h>

#include "mas_blk_latency_interface.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
void __mas_blk_latency_check_timer_expire(struct timer_list *timer)
{
	struct blk_bio_cust *mas_bio = (struct blk_bio_cust *)from_timer(mas_bio,
			timer, latency_expire_timer);
	struct bio *bio = (struct bio *)container_of(mas_bio, struct bio, mas_bio);
	mas_blk_latency_check_timer_expire((unsigned long)bio);
}
#else
void __mas_blk_latency_check_timer_expire(unsigned long data)
{
	mas_blk_latency_check_timer_expire(data);
}
#endif

ssize_t __mas_queue_io_latency_warning_threshold_store(
	struct request_queue *q, const char *page, size_t count)
{
	return mas_queue_io_latency_warning_threshold_store(q, page, count);
}

