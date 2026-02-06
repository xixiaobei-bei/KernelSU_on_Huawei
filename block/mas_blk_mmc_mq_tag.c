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

#define pr_fmt(fmt) "[BLK-IO]" fmt

#include <linux/types.h>
#include <linux/sbitmap.h>
#include <linux/blk-mq.h>

#include "blk-mq.h"
#include "blk-mq-tag.h"
#include "mas_blk_mmc_mq_tag.h"

struct mmc_bt_iter_data {
	struct blk_mq_hw_ctx *hctx;
	busy_iter_fn *fn;
	void *data;
	bool reserved;
	bool prior;
};

struct mmc_bt_tags_iter_data {
	struct blk_mq_tags *tags;
	busy_tag_iter_fn *fn;
	void *data;
	bool reserved;
	bool prior;
};

static bool mmc_tagset_bt_iter(
		struct sbitmap *bitmap, unsigned int bitnr, void *data)
{
	struct mmc_bt_iter_data *iter_data = data;
	struct blk_mq_hw_ctx *hctx = iter_data->hctx;
	struct blk_mq_tags *tags = hctx->tags;
	struct request *rq = NULL;

	if (iter_data->reserved)
		bitnr += tags->reserved_tags_id_offset;
	if (iter_data->prior)
		bitnr += tags->high_prio_tags_id_offset;

	rq = tags->rqs[bitnr];

	/*
	 * We can hit rq == NULL here, because the tagging functions
	 * test and set the bit before assining ->rqs[].
	 */
	if (rq && rq->q == hctx->queue && iter_data->fn)
		iter_data->fn(hctx, rq, iter_data->data, iter_data->reserved);

	return true;
}

static inline void mmc_tagset_bt_for_each(
	struct sbitmap_queue *bt, struct mmc_bt_iter_data *iter_data)
{
	sbitmap_for_each_set(&bt->sb, mmc_tagset_bt_iter, iter_data);
}

void mmc_tagset_tag_busy_iter(
	struct blk_mq_hw_ctx *hctx, busy_iter_fn *fn, void *priv)
{
	struct mmc_bt_iter_data iter_data;
	struct blk_mq_tags *tags = hctx->tags;

	iter_data.hctx = hctx;
	iter_data.fn = fn;
	iter_data.data = priv;
	iter_data.reserved = false;
	iter_data.prior = false;

	mmc_tagset_bt_for_each(&tags->bitmap_tags, &iter_data);

	if (tags->nr_high_prio_tags) {
		iter_data.prior = true;
		mmc_tagset_bt_for_each(&tags->highprio_tags, &iter_data);
	}

	if (tags->nr_reserved_tags) {
		iter_data.reserved = true;
		iter_data.prior = false;
		mmc_tagset_bt_for_each(&tags->breserved_tags, &iter_data);
	}
}

static bool mmc_tagset_bt_tags_iter(
	struct sbitmap *bitmap, unsigned int bitnr, void *data)
{
	struct mmc_bt_tags_iter_data *iter_data = data;
	struct blk_mq_tags *tags = iter_data->tags;
	struct request *rq = NULL;

	if (iter_data->reserved)
		bitnr += tags->reserved_tags_id_offset;
	if (iter_data->prior)
		bitnr += tags->high_prio_tags_id_offset;

	rq = tags->rqs[bitnr];

	if (iter_data->fn)
		iter_data->fn(rq, iter_data->data, iter_data->reserved);
	return true;
}

static inline void mmc_tagset_bt_tags_for_each(
	struct sbitmap_queue *bt, const struct mmc_bt_tags_iter_data *iter_data)
{
	if (iter_data->tags->rqs)
		sbitmap_for_each_set(
			&bt->sb, mmc_tagset_bt_tags_iter, (void *)iter_data);
}

static inline bool mmc_sbitmap_is_fg(struct blk_mq_alloc_data *data)
{
	return (data->io_flag & REQ_FG_META) && !(data->io_flag & REQ_VIP);
}

#define FG_WAIT_TAG_TIMEOUT_MS 500ull
static inline bool mmc_sbitmap_fg_starved(struct blk_mq_alloc_data *data)
{
	if (!data->wait_tag_start)
		return false;

	return ktime_after(ktime_get(),
		ktime_add_ms(data->wait_tag_start, FG_WAIT_TAG_TIMEOUT_MS));
}

static int mmc_sbitmap_queue_get(
	struct blk_mq_alloc_data *data, struct sbitmap_queue *bt)
{
	if (unlikely(mmc_sbitmap_is_fg(data) && mmc_mq_vip_tag_wait_cnt(data) &&
		     !mmc_sbitmap_fg_starved(data)))
		return -1;

	return __sbitmap_queue_get(bt);
}

static int mmc_try_get_tag(struct blk_mq_alloc_data *data,
	struct sbitmap_queue *bt, struct blk_mq_hw_ctx *hctx)
{
	int tag;

	tag = mmc_sbitmap_queue_get(data, bt);
	if (tag != -1)
		return tag;

	/*
	 * We're out of tags on this hardware queue, kick any
	 * pending IO submits before going to sleep waiting for
	 * some to complete.
	 */
	blk_mq_run_hw_queue(hctx, false);

	/*
	 * Retry tag allocation after running the hardware queue,
	 * as running the queue may also have found completions.
	 */
	return mmc_sbitmap_queue_get(data, bt);
}

static unsigned int mmc_tagset_bt_get_slowpath(
	struct blk_mq_alloc_data *data, struct sbitmap_queue *bt,
	struct blk_mq_hw_ctx *hctx)
{
	struct sbq_wait_state *bs = NULL;
	DEFINE_WAIT(wait);
	int tag;

	if (data->io_flag & REQ_VIP)
		mmc_mq_inc_vip_wait_cnt(data);

	if (mmc_sbitmap_is_fg(data))
		data->wait_tag_start = ktime_get();

	bs = &bt->ws[0];
	do {
		prepare_to_wait(&bs->wait, &wait, TASK_UNINTERRUPTIBLE);

		tag = mmc_try_get_tag(data, bt, hctx);
		if (tag != -1)
			break;

		blk_mq_put_ctx(data->ctx);
		if (mmc_sbitmap_is_fg(data)) {
			if (!io_schedule_timeout(1))
				mmc_reset_vip_wait_cnt(data);
		} else {
			io_schedule();
		}

		data->ctx = blk_mq_get_ctx(data->q);
		data->hctx = blk_mq_map_queue(data->q, data->ctx->cpu);
		if (!data->hctx)
			break;

		if (likely(hctx == data->hctx))
			goto finish_wait;

		if (likely(data->io_flag & REQ_SYNC)) {
			if (likely(!(data->io_flag & REQ_FG_META)))
				bt = &data->hctx->tags->bitmap_tags;
			else
				bt = &data->hctx->tags->highprio_tags;
		} else {
			bt = &data->hctx->tags->breserved_tags;
		}
		hctx = data->hctx;
finish_wait:
		finish_wait(&bs->wait, &wait);
	} while (1);

	if (data->io_flag & REQ_VIP)
		mmc_mq_dec_vip_wait_cnt(data);

	finish_wait(&bs->wait, &wait);

	return (unsigned int)tag;
}

unsigned int mmc_tagset_bt_get(
	struct blk_mq_alloc_data *data, struct sbitmap_queue *bt,
	struct blk_mq_hw_ctx *hctx)
{
	int tag;

	if (!hctx)
		return BLK_MQ_TAG_FAIL;

	tag = mmc_sbitmap_queue_get(data, bt);
	if (likely(tag != -1))
		return (unsigned int)tag;

	return mmc_tagset_bt_get_slowpath(data, bt, hctx);
}

static int mmc_tagset_init_bitmap_tags(
	struct blk_mq_tags *tags, int node, int alloc_policy)
{
	unsigned int depth = tags->nr_tags - tags->nr_reserved_tags -
			     tags->nr_high_prio_tags;
	bool round_robin = alloc_policy == BLK_TAG_ALLOC_RR;

	pr_err("depth = %d, reserved = %d, high prio = %d \r\n", depth,
		tags->nr_reserved_tags, tags->nr_high_prio_tags);

	if (sbitmap_queue_init_node(&tags->bitmap_tags, depth, -1, round_robin,
		    GFP_KERNEL, node))
		goto enomem;

	if (sbitmap_queue_init_node(&tags->breserved_tags,
		    tags->nr_reserved_tags, -1, round_robin, GFP_KERNEL, node))
		goto enomem1;

	if (sbitmap_queue_init_node(&tags->highprio_tags,
		    tags->nr_high_prio_tags, -1, round_robin, GFP_KERNEL, node))
		goto enomem2;

	tags->tags_id_offset = 0;
	tags->reserved_tags_id_offset = depth;
	tags->high_prio_tags_id_offset = tags->nr_reserved_tags + depth; //21
	return 0;

enomem2:
	sbitmap_queue_free(&tags->breserved_tags);
enomem1:
	sbitmap_queue_free(&tags->bitmap_tags);
enomem:
	pr_err("%s: error nomem\n", __func__);
	return -ENOMEM;
}

/* caller make sure input params are valid */
void mmc_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn, const void *priv)
{
	struct mmc_bt_tags_iter_data iter_data;

	iter_data.tags = tags;
	iter_data.fn = fn;
	iter_data.data = (void *)priv;

	iter_data.reserved = false;
	iter_data.prior = false;
	mmc_tagset_bt_tags_for_each(&tags->bitmap_tags, &iter_data);

	if (tags->nr_high_prio_tags) {
		iter_data.prior = true;
		mmc_tagset_bt_tags_for_each(&tags->highprio_tags, &iter_data);
	}

	if (tags->nr_reserved_tags) {
		iter_data.reserved = true;
		iter_data.prior = false;
		mmc_tagset_bt_tags_for_each(&tags->breserved_tags, &iter_data);
	}
}

void mmc_tagset_free_tags(struct blk_mq_tags *tags)
{
	sbitmap_queue_free(&tags->bitmap_tags);
	sbitmap_queue_free(&tags->breserved_tags);
	sbitmap_queue_free(&tags->highprio_tags);
	kfree(tags);
}

int mmc_mq_tag_wakeup_all(struct blk_mq_tags *tags)
{
	sbitmap_queue_wake_all(&tags->bitmap_tags);
	sbitmap_queue_wake_all(&tags->breserved_tags);
	sbitmap_queue_wake_all(&tags->highprio_tags);
	return 0;
}

struct blk_mq_tags *mmc_tagset_init_tags(
	unsigned int total_tags, unsigned int reserved_tags,
	unsigned int high_prio_tags, int node, int alloc_policy)
{
	struct blk_mq_tags *tags = NULL;
	int ret;

	tags = kzalloc_node(sizeof(*tags), GFP_KERNEL, node);
	if (!tags)
		return NULL;

	tags->nr_tags = total_tags;
	tags->nr_reserved_tags = reserved_tags;
	tags->nr_high_prio_tags = high_prio_tags;

	ret = mmc_tagset_init_bitmap_tags(tags, node, alloc_policy);
	if (ret) {
		pr_err("Failed to init tagset bitmaps!\n");
		kfree(tags);
		return NULL;
	}

	return tags;
}
