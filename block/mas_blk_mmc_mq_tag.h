/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: mas block MQ tag
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

#ifndef __MAS_BLK_MMC_MQ_TAG_H__
#define __MAS_BLK_MMC_MQ_TAG_H__
#include <linux/blk_types.h>

#include "blk-mq-tag.h"

void mmc_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn,
	const void *priv);
void mmc_tagset_tag_busy_iter(
	struct blk_mq_hw_ctx *hctx, busy_iter_fn *fn, void *priv);

struct blk_mq_tags *mmc_tagset_init_tags(
	unsigned int total_tags, unsigned int reserved_tags,
	unsigned int high_prio_tags, int node, int alloc_policy);
unsigned int mmc_tagset_bt_get(
	struct blk_mq_alloc_data *data, struct sbitmap_queue *bt,
	struct blk_mq_hw_ctx *hctx);
void mmc_mq_inc_vip_wait_cnt(struct blk_mq_alloc_data *data);
void mmc_mq_dec_vip_wait_cnt(struct blk_mq_alloc_data *data);
void mmc_reset_vip_wait_cnt(struct blk_mq_alloc_data *data);
int mmc_mq_vip_tag_wait_cnt(struct blk_mq_alloc_data *data);
#endif
