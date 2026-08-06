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

#ifndef __MAS_BLK_MQ_TAG_H__
#define __MAS_BLK_MQ_TAG_H__
#include <linux/blk_types.h>

#include "blk-mq-tag.h"

void ufs_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn,
	const void *priv);
void ufs_tagset_tag_busy_iter(
	struct blk_mq_hw_ctx *hctx, busy_iter_fn *fn, void *priv);

struct blk_mq_tags *ufs_tagset_init_tags(
	unsigned int total_tags, unsigned int reserved_tags,
	unsigned int high_prio_tags, int node, int alloc_policy);
unsigned int ufs_tagset_bt_get(
	struct blk_mq_alloc_data *data, struct sbitmap_queue *bt,
	struct blk_mq_hw_ctx *hctx);
void ufs_mq_inc_vip_wait_cnt(struct blk_mq_alloc_data *data);
void ufs_mq_dec_vip_wait_cnt(struct blk_mq_alloc_data *data);
void reset_vip_wait_cnt(struct blk_mq_alloc_data *data);
int ufs_mq_vip_tag_wait_cnt(struct blk_mq_alloc_data *data);
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
unsigned int ufs_tagset_bt_get_unistore(
	struct blk_mq_alloc_data *data, bool *highprio);
#endif
#endif
