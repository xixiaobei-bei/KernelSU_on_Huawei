/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description:  hyperhold helper function in block
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

void blk_dev_health_query_register(
	struct request_queue *q, lld_query_health_fn func)
{
	q->health_query = func;
}

int blk_dev_health_query(struct block_device *bi_bdev, u8 *pre_eol_info,
	u8 *life_time_est_a, u8 *life_time_est_b)
{
	struct request_queue *q = NULL;

	if ((!bi_bdev) || (!pre_eol_info) || (!life_time_est_a) ||
		(!life_time_est_b))
		return -EPERM;

	q = bdev_get_queue(bi_bdev);
	if (q && q->health_query)
		return q->health_query(q, pre_eol_info, life_time_est_a,
			life_time_est_b);

	return -EPERM;
}

