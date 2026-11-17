/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description:  inline crypt support in block
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
#include <linux/bio.h>
#include <linux/blk-mq.h>
#include <linux/blkdev.h>
#include <linux/kernel.h>

#include "blk.h"
#include "mas_blk.h"

/*
 * This interface will be called to set the inline crypto support on request
 * queue
 */
void blk_queue_set_inline_crypto_flag(
	struct request_queue *q, bool enable)
{
	if (enable)
		q->inline_crypt_support = 1;
	else
		q->inline_crypt_support = 0;
}

static int blk_queue_support_crypto(const struct request_queue *q)
{
	return q->inline_crypt_support;
}

#define MAS_BLK_CI_KEY_LEN 64
#define MAS_BLK_CI_KEY_LEN_2 48
void mas_blk_inline_crypto_init_request_from_bio(
	struct request *req, const struct bio *bio)
{
	if (unlikely(!blk_queue_support_crypto(req->q)))
		return;

	if (!bio->ci_key)
		return;

	if (unlikely(bio->ci_key_len != MAS_BLK_CI_KEY_LEN &&
		     bio->ci_key_len != MAS_BLK_CI_KEY_LEN_2)) {
		pr_err("%s: init rq key len not %d or %d\n",
			__func__, MAS_BLK_CI_KEY_LEN, MAS_BLK_CI_KEY_LEN_2);
		mas_blk_rdr_panic("Invalid ci_key_len!");
	}
	req->ci_key = bio->ci_key;
	req->ci_key_len = bio->ci_key_len;
	req->ci_key_index = bio->ci_key_index;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	req->ci_metadata = bio->ci_metadata;
#endif
}

bool mas_blk_inline_crypto_bio_merge_allow(
	const struct request *rq, const struct bio *bio)
{
	struct bio *prev = NULL;
	int ret;

	if (!blk_queue_support_crypto(rq->q))
		return true;

	/*
	 * check if both the bio->key & last merged request->key
	 * do not exist, wo shall tell block that this bio may merge to the rq.
	 */
	if (!bio->ci_key && !rq->ci_key)
		return true;

	/*
	 * check if the bio->key or last merged request->key
	 * does not exist, but the other's was existing,
	 * we shall tell block that the bio should not be merged to the rq.
	 */
	if (!bio->ci_key || !rq->ci_key)
		return false;

	if (bio->ci_key_len != rq->ci_key_len)
		return false;

	if (bio->ci_key_len != MAS_BLK_CI_KEY_LEN &&
	    bio->ci_key_len != MAS_BLK_CI_KEY_LEN_2)
		pr_err("%s: key len invalid!\n", __func__);

	if (bio->ci_key != rq->ci_key)
		return false;

	prev = rq->biotail;
	ret = blk_try_merge((struct request *)rq, (struct bio *)bio);
	switch (ret) {
	case ELEVATOR_BACK_MERGE:
		if (prev->index + prev->bi_vcnt != bio->index)
			return false;
		break;
	case ELEVATOR_FRONT_MERGE:
		if (bio->index + bio->bi_vcnt !=
			rq->bio->index)
			return false;
		break;
	default:
		return false;
	}

	return true;
}

void mas_blk_inline_crypto_bio_split_pre(struct bio *bio, struct bio *split)
{
	if (!bio->ci_key)
		return;

	if (bio->ci_key_len != MAS_BLK_CI_KEY_LEN &&
	    bio->ci_key_len != MAS_BLK_CI_KEY_LEN_2)
		pr_err("%s: Invalid ci_key_len!\n", __func__);

	split->ci_key = bio->ci_key;
	split->ci_key_len = bio->ci_key_len;
	split->ci_key_index = bio->ci_key_index;
	split->index = bio->index;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	split->ci_metadata = bio->ci_metadata;
#endif
}

void mas_blk_inline_crypto_bio_split_post(struct bio *bio)
{
	if (!bio->ci_key)
		return;

	if (bio->ci_key_len != MAS_BLK_CI_KEY_LEN &&
	    bio->ci_key_len != MAS_BLK_CI_KEY_LEN_2)
		pr_err("%s: init key len invalid!\n", __func__);
	bio->index = bio_page(bio)->index;
}

void mas_blk_inline_crypto_req_init(struct request *rq)
{
	rq->ci_key = NULL;
	rq->ci_key_len = 0;
	rq->ci_key_index = -1;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	rq->ci_metadata = NULL;
#endif
}

