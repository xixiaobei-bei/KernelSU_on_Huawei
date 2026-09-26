/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description:  MAS MQ ioscheduler interface
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
#include "mas_blk_iosched_mmc_interface.h"

void __cfi_mmc_mq_io_dispatch_work_fn(struct work_struct *work)
{
	mmc_mq_io_dispatch_work_fn(work);
}


void __cfi__mmc_mq_complete_request_remote(void *data)
{
	__mmc_mq_complete_request_remote(data);
}

void __cfi_mmc_mq_io_guard_work_fn(struct work_struct *work)
{
	mmc_mq_io_guard_work_fn();
}

blk_qc_t __cfi_mmc_mq_make_request(struct request_queue *q, struct bio *bio)
{
	return mmc_mq_make_request(q, bio);
}

static inline struct blk_mq_tags *__cfi_mmc_tagset_init_tags(
	unsigned int total_tags, unsigned int reserved_tags,
	unsigned int high_prio_tags, int node, int alloc_policy)
{
	return mmc_tagset_init_tags(
		total_tags, reserved_tags, high_prio_tags, node, alloc_policy);
}

static inline void __cfi_mmc_tagset_free_tags(struct blk_mq_tags *tags)
{
	mmc_tagset_free_tags(tags);
}

static inline void __cfi_mmc_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn, const void *priv)
{
	mmc_tagset_all_tag_busy_iter(tags, fn, priv);
}

static inline void __cfi_mmc_tagset_tag_busy_iter(
	struct blk_mq_hw_ctx *hctx, busy_iter_fn *fn, void *priv)
{
	mmc_tagset_tag_busy_iter(hctx, fn, priv);
}

static inline int __cfi_mmc_mq_iosched_init(struct request_queue *q)
{
	return mmc_mq_iosched_init(q);
}

static inline void __cfi_mmc_mq_iosched_exit(struct request_queue *q)
{
	mmc_mq_iosched_exit(q);
}

static inline void __cfi_mmc_mq_req_alloc_prep(
	struct blk_mq_alloc_data *data, unsigned long ioflag, bool fs_submit)
{
	mmc_mq_req_alloc_prep(data, ioflag, fs_submit);
}

static inline void __cfi_mmc_mq_req_init(
	struct request_queue *q, struct blk_mq_ctx *ctx, struct request *rq)
{
	mmc_mq_req_init(ctx, rq);
}

static inline void __cfi_mmc_mq_req_complete(
	struct request *rq, struct request_queue *q, bool succ_done)
{
	mmc_mq_req_complete(rq, q, succ_done);
}

static inline void __cfi_mmc_mq_req_deinit(struct request *rq)
{
	mmc_mq_req_deinit(rq);
}

static inline void __cfi_mmc_mq_req_insert(
	struct request *req, struct request_queue *q)
{
	mmc_mq_req_insert(req, q);
}

static inline void __cfi_mmc_mq_req_requeue(
	struct request *req, struct request_queue *q)
{
	mmc_mq_req_requeue(req, q);
}

static inline void __cfi_mmc_mq_req_timeout_handler(struct request *req)
{
	mmc_mq_req_timeout_handler(req);
}

static inline void __cfi_mmc_mq_ctx_put(void)
{
	mmc_mq_ctx_put();
}

static inline void __cfi_mmc_mq_hctx_get_by_req(
	struct request *rq, struct blk_mq_hw_ctx **hctx)
{
	mmc_mq_hctx_get_by_req(rq, hctx);
}

static inline unsigned int __cfi_mmc_mq_tag_get(struct blk_mq_alloc_data *data)
{
	return mmc_mq_tag_get(data);
}

static inline int __cfi_mmc_mq_tag_put(
	struct blk_mq_hw_ctx *hctx, unsigned int tag, struct request *rq)
{
	return mmc_mq_tag_put(hctx, tag, rq);
}

static inline int __cfi_mmc_mq_tag_wakeup_all(struct blk_mq_tags *tags)
{
	return mmc_mq_tag_wakeup_all(tags);
}

static inline void __cfi_mmc_mq_exec_queue(struct request_queue *q)
{
	mmc_mq_exec_queue(q);
}

static inline void __cfi_mmc_mq_run_hw_queue(struct request_queue *q)
{
	mmc_mq_run_hw_queue(q);
}

static inline void __cfi_mmc_mq_run_requeue(struct request_queue *q)
{
	mmc_mq_run_requeue(q);
}

static inline void __cfi_mmc_mq_poll_enable(bool *enable)
{
	mmc_mq_poll_enable(enable);
}

static inline void __cfi_mmc_mq_status_dump(
	struct request_queue *q, enum blk_dump_scene s)
{
	mmc_mq_status_dump(q, s);
}

static inline void __cfi_mmc_mq_dump_request(
	struct request_queue *q, enum blk_dump_scene s)
{
	mmc_mq_dump_request(q, s);
}


struct blk_queue_ops mas_mmc_blk_queue_ops = {
	.io_scheduler_strategy = IOSCHED_MAS_MMC_MQ,
	.mq_iosched_init_fn = __cfi_mmc_mq_iosched_init,
	.mq_iosched_exit_fn =  __cfi_mmc_mq_iosched_exit,
	.mq_req_alloc_prep_fn = __cfi_mmc_mq_req_alloc_prep,
	.mq_req_init_fn = __cfi_mmc_mq_req_init,
	.mq_req_complete_fn = __cfi_mmc_mq_req_complete,
	.mq_req_deinit_fn = __cfi_mmc_mq_req_deinit,
	.mq_req_insert_fn = __cfi_mmc_mq_req_insert,
	.mq_req_requeue_fn = __cfi_mmc_mq_req_requeue,
	.mq_req_timeout_fn = __cfi_mmc_mq_req_timeout_handler,
	.mq_ctx_put_fn = __cfi_mmc_mq_ctx_put,
	.mq_hctx_get_by_req_fn = __cfi_mmc_mq_hctx_get_by_req,
	.mq_tag_get_fn = __cfi_mmc_mq_tag_get,
	.mq_tag_put_fn = __cfi_mmc_mq_tag_put,
	.mq_tag_wakeup_all_fn = __cfi_mmc_mq_tag_wakeup_all,
	.mq_exec_queue_fn = __cfi_mmc_mq_exec_queue,
	.mq_run_hw_queue_fn = __cfi_mmc_mq_run_hw_queue,
	.mq_run_requeue_fn = __cfi_mmc_mq_run_requeue,
	.blk_poll_enable_fn = __cfi_mmc_mq_poll_enable,
	.blk_status_dump_fn = __cfi_mmc_mq_status_dump,
	.blk_req_dump_fn = __cfi_mmc_mq_dump_request,
	.scheduler_priv = &mas_mmc_mq,
	.iosched_name = "MAS MQ",
};

struct blk_tagset_ops mas_mmc_blk_tagset_ops = {
	.tagset_init_tags_fn = __cfi_mmc_tagset_init_tags,
	.tagset_free_tags_fn = __cfi_mmc_tagset_free_tags,
	.tagset_all_tag_busy_iter_fn = __cfi_mmc_tagset_all_tag_busy_iter,
	.tagset_tag_busy_iter_fn = __cfi_mmc_tagset_tag_busy_iter,
	.queue_ops = &mas_mmc_blk_queue_ops,
};

