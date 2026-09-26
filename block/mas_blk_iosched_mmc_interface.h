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

#ifndef __MAS_BLK_IOSCHED_MMC_INTERFACE_H__
#define __MAS_BLK_IOSCHED_MMC_INTERFACE_H__

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>

#include "blk.h"
#include "blk-mq.h"
#include "blk-mq-tag.h"

extern blk_qc_t request_to_qc_t(struct blk_mq_hw_ctx *hctx, struct request *rq);
extern struct blk_tagset_ops mas_mmc_blk_tagset_ops;

extern struct mas_mmc_mq_priv mas_mmc_mq;
extern struct blk_mq_tags *mmc_tagset_init_tags(
	unsigned int total_tags, unsigned int reserved_tags,
	unsigned int high_prio_tags, int node, int alloc_policy);
extern void mmc_tagset_free_tags(const struct blk_mq_tags *tags);
extern void mmc_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn,
	const void *priv);
extern void mmc_tagset_tag_busy_iter(
	struct blk_mq_hw_ctx *hctx, busy_iter_fn *fn, void *priv);
extern int mmc_mq_iosched_init(struct request_queue *q);
extern void mmc_mq_iosched_exit(struct request_queue *q);
extern void mmc_mq_req_alloc_prep(
	struct blk_mq_alloc_data *data, unsigned long ioflag, bool fs_submit);
extern void mmc_mq_req_init(const struct blk_mq_ctx *ctx, struct request *rq);
extern void mmc_mq_req_complete(
	struct request *rq, const struct request_queue *q, bool succ_done);
extern void mmc_mq_req_deinit(struct request *rq);
extern void mmc_mq_req_insert(
	struct request *req, struct request_queue *q);
extern void mmc_mq_req_requeue(
	struct request *req, const struct request_queue *q);
extern void mmc_mq_req_timeout_handler(struct request *req);
extern void mmc_mq_ctx_put(void);
extern void mmc_mq_hctx_get_by_req(
	const struct request *rq, struct blk_mq_hw_ctx **hctx);
extern unsigned int mmc_mq_tag_get(const struct blk_mq_alloc_data *data);
extern int mmc_mq_tag_put(
	const struct blk_mq_hw_ctx *hctx, unsigned int tag,
	const struct request *rq);
extern int mmc_mq_tag_update_depth(void);
extern int mmc_mq_tag_wakeup_all(struct blk_mq_tags *tags);
extern void mmc_mq_exec_queue(const struct request_queue *q);
extern void mmc_mq_run_hw_queue(const struct request_queue *q);
extern void mmc_mq_run_requeue(const struct request_queue *q);
extern void mmc_mq_poll_enable(bool *enable);
extern void mmc_mq_status_dump(
	const struct request_queue *q, enum blk_dump_scene s);
extern void __mmc_mq_complete_request_remote(const void *data);
extern void mmc_mq_io_dispatch_work_fn(const struct work_struct *work);
extern void __cfi__mmc_mq_complete_request_remote(void *data);
extern void __cfi_mmc_mq_io_dispatch_work_fn(struct work_struct *work);
extern void __cfi_mmc_mq_io_guard_work_fn(struct work_struct *work);
extern void mmc_mq_io_guard_work_fn(void);
extern blk_qc_t __cfi_mmc_mq_make_request(
	struct request_queue *q, struct bio *bio);
extern blk_qc_t mmc_mq_make_request(struct request_queue *q, struct bio *bio);
extern void __blk_mq_requeue_request(struct request *rq);
extern struct request *blk_mq_get_request(
	struct request_queue *q, struct bio *bio,
	unsigned int op, struct blk_mq_alloc_data *data);
extern void mmc_mq_dump_request(
	const struct request_queue *q, enum blk_dump_scene s);
extern void mmc_mq_requeue_invalidate_reqs(struct request_queue *q);

#endif /* __MAS_BLK_IOSCHED_MMC_INTERFACE_H__ */
