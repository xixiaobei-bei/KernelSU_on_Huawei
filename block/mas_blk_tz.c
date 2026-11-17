/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description:  turbo zone helper function in block
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
#include <linux/blk-mq.h>
#include "mas_blk.h"

void blk_mq_tagset_tz_query_register(
	struct blk_mq_tag_set *tag_set, lld_tz_query_fn func)
{
	if (tag_set)
		tag_set->lld_func.tz_query = func;
}

int blk_lld_tz_query(
	const struct block_device *bi_bdev, u32 type, u8 *buf, u32 buf_len)
{
	struct blk_dev_lld *lld_fn = NULL;
	struct request_queue *q = NULL;

	if (!bi_bdev || !buf)
		return -EPERM;
	q = bdev_get_queue((struct block_device *)bi_bdev);
	if (!q)
		return -EPERM;

	lld_fn = mas_blk_get_lld(q);
	if (lld_fn && lld_fn->tz_query)
		return lld_fn->tz_query(q, type, buf, buf_len);

	return -EPERM;
}

void blk_mq_tagset_tz_ctrl_register(
	struct blk_mq_tag_set *tag_set, lld_tz_ctrl_fn func)
{
	if (tag_set)
		tag_set->lld_func.tz_ctrl = func;
}

int blk_lld_tz_ctrl(
	const struct block_device *bi_bdev, int desc_id, uint8_t index)
{
	struct blk_dev_lld *lld_fn = NULL;
	struct request_queue *q = NULL;

	if (!bi_bdev)
		return -EPERM;
	q = bdev_get_queue((struct block_device *)bi_bdev);
	if (!q)
		return -EPERM;

	lld_fn = mas_blk_get_lld(q);
	if (lld_fn && lld_fn->tz_ctrl)
		return lld_fn->tz_ctrl(q, desc_id, index);

	return -EPERM;
}

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
ssize_t mas_queue_tz_write_bytes_show(struct request_queue *q, char *page)
{
	unsigned long offset = 0;

	offset += snprintf(page, PAGE_SIZE, "tz_write_bytes: %lu\n",
			   q->mas_queue.tz_write_bytes);

	return (ssize_t)offset;
}
#endif

