/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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
#include "mas_blk_iosched_interface.h"

void __cfi_ufs_mq_sync_io_dispatch_work_fn(struct work_struct *work)
{
	ufs_mq_sync_io_dispatch_work_fn(work);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
void __cfi_ufs_mq_sync_burst_check_timer_expire(struct timer_list *timer)
{
	unsigned long data;
	data = ufs_mq_sync_burst_check_timer_prepare(timer);
	ufs_mq_sync_burst_check_timer_expire(data);
}
#else
void __cfi_ufs_mq_sync_burst_check_timer_expire(unsigned long data)
{
	ufs_mq_sync_burst_check_timer_expire(data);
}
#endif
void __cfi_ufs_mq_flush_plug_list(struct blk_plug *plug, bool from_schedule)
{
	ufs_mq_flush_plug_list(plug, from_schedule);
}

void __cfi__ufs_mq_complete_request_remote(void *data)
{
	__ufs_mq_complete_request_remote(data);
}

void __cfi_ufs_mq_async_io_dispatch_work_fn(struct work_struct *work)
{
	ufs_mq_async_io_dispatch_work_fn(work);
}

void __cfi_ufs_mq_io_guard_work_fn(struct work_struct *work)
{
	ufs_mq_io_guard_work_fn();
}

blk_qc_t __cfi_ufs_mq_make_request(struct request_queue *q, struct bio *bio)
{
	return ufs_mq_make_request(q, bio);
}

static inline struct blk_mq_tags *__cfi_ufs_tagset_init_tags(
	unsigned int total_tags, unsigned int reserved_tags,
	unsigned int high_prio_tags, int node, int alloc_policy)
{
	return ufs_tagset_init_tags(
		total_tags, reserved_tags, high_prio_tags, node, alloc_policy);
}

static inline void __cfi_ufs_tagset_free_tags(struct blk_mq_tags *tags)
{
	ufs_tagset_free_tags(tags);
}

static inline void __cfi_ufs_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn, const void *priv)
{
	ufs_tagset_all_tag_busy_iter(tags, fn, priv);
}

static inline void __cfi_ufs_tagset_tag_busy_iter(
	struct blk_mq_hw_ctx *hctx, busy_iter_fn *fn, void *priv)
{
	ufs_tagset_tag_busy_iter(hctx, fn, priv);
}

static inline int __cfi_ufs_mq_iosched_init(struct request_queue *q)
{
	return ufs_mq_iosched_init(q);
}

static inline void __cfi_ufs_mq_iosched_exit(struct request_queue *q)
{
	ufs_mq_iosched_exit(q);
}

static inline void __cfi_ufs_mq_req_alloc_prep(
	struct blk_mq_alloc_data *data, unsigned long ioflag, bool fs_submit)
{
	ufs_mq_req_alloc_prep(data, ioflag, fs_submit);
}

static inline void __cfi_ufs_mq_req_init(
	struct request_queue *q, struct blk_mq_ctx *ctx, struct request *rq)
{
	ufs_mq_req_init(ctx, rq);
}

static inline void __cfi_ufs_mq_req_complete(
	struct request *rq, struct request_queue *q, bool succ_done)
{
	ufs_mq_req_complete(rq, q, succ_done);
}

static inline void __cfi_ufs_mq_req_deinit(struct request *rq)
{
	ufs_mq_req_deinit(rq);
}

static inline void __cfi_ufs_mq_req_insert(
	struct request *req, struct request_queue *q)
{
	ufs_mq_req_insert(req, q);
}

static inline void __cfi_ufs_mq_req_requeue(
	struct request *req, struct request_queue *q)
{
	ufs_mq_req_requeue(req, q);
}

static inline void __cfi_ufs_mq_req_timeout_handler(struct request *req)
{
	ufs_mq_req_timeout_handler(req);
}

static inline void __cfi_ufs_mq_ctx_put(void)
{
	ufs_mq_ctx_put();
}

static inline void __cfi_ufs_mq_hctx_get_by_req(
	struct request *rq, struct blk_mq_hw_ctx **hctx)
{
	ufs_mq_hctx_get_by_req(rq, hctx);
}

static inline unsigned int __cfi_ufs_mq_tag_get(struct blk_mq_alloc_data *data)
{
	return ufs_mq_tag_get(data);
}

static inline int __cfi_ufs_mq_tag_put(
	struct blk_mq_hw_ctx *hctx, unsigned int tag, struct request *rq)
{
	return ufs_mq_tag_put(hctx, tag, rq);
}

static inline int __cfi_ufs_mq_tag_wakeup_all(struct blk_mq_tags *tags)
{
	return ufs_mq_tag_wakeup_all(tags);
}

static inline void __cfi_ufs_mq_exec_queue(struct request_queue *q)
{
	ufs_mq_exec_queue(q);
}

static inline void __cfi_ufs_mq_run_hw_queue(struct request_queue *q)
{
	ufs_mq_run_hw_queue(q);
}

static inline void __cfi_ufs_mq_run_requeue(struct request_queue *q)
{
	ufs_mq_run_requeue(q);
}

static inline void __cfi_ufs_mq_poll_enable(bool *enable)
{
	ufs_mq_poll_enable(enable);
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
void __cfi_ufs_mq_write_throttle_check_timer_expire(struct timer_list *timer)
{
	unsigned long data;
	data = ufs_mq_write_throttle_check_timer_prepare(timer);
	ufs_mq_write_throttle_check_timer_expire(data);
}
#else
void __cfi_ufs_mq_write_throttle_check_timer_expire(unsigned long data)
{
	ufs_mq_write_throttle_check_timer_expire(data);
}
#endif
static inline void __cfi_ufs_mq_write_throttle_handler(
	struct request_queue *q, bool level)
{
	ufs_mq_write_throttle_handler(q, level);
}

static inline void __cfi_ufs_mq_status_dump(
	struct request_queue *q, enum blk_dump_scene s)
{
	ufs_mq_status_dump(q, s);
}

static inline void __cfi_ufs_mq_dump_request(
	struct request_queue *q, enum blk_dump_scene s)
{
	ufs_mq_dump_request(q, s);
}

static inline void __cfi_ufs_tagset_power_off_proc(struct blk_dev_lld *lld)
{
	ufs_tagset_power_off_proc(lld);
}

struct blk_queue_ops mas_ufs_blk_queue_ops = {
	.io_scheduler_strategy = IOSCHED_MAS_UFS_MQ,
	.mq_iosched_init_fn = __cfi_ufs_mq_iosched_init,
	.mq_iosched_exit_fn = __cfi_ufs_mq_iosched_exit,
	.mq_req_alloc_prep_fn = __cfi_ufs_mq_req_alloc_prep,
	.mq_req_init_fn = __cfi_ufs_mq_req_init,
	.mq_req_complete_fn = __cfi_ufs_mq_req_complete,
	.mq_req_deinit_fn = __cfi_ufs_mq_req_deinit,
	.mq_req_insert_fn = __cfi_ufs_mq_req_insert,
	.mq_req_requeue_fn = __cfi_ufs_mq_req_requeue,
	.mq_req_timeout_fn = __cfi_ufs_mq_req_timeout_handler,
	.mq_ctx_put_fn = __cfi_ufs_mq_ctx_put,
	.mq_hctx_get_by_req_fn = __cfi_ufs_mq_hctx_get_by_req,
	.mq_tag_get_fn = __cfi_ufs_mq_tag_get,
	.mq_tag_put_fn = __cfi_ufs_mq_tag_put,
	.mq_tag_wakeup_all_fn = __cfi_ufs_mq_tag_wakeup_all,
	.mq_exec_queue_fn = __cfi_ufs_mq_exec_queue,
	.mq_run_hw_queue_fn = __cfi_ufs_mq_run_hw_queue,
	.mq_run_requeue_fn = __cfi_ufs_mq_run_requeue,
	.blk_poll_enable_fn = __cfi_ufs_mq_poll_enable,
	.blk_write_throttle_fn = __cfi_ufs_mq_write_throttle_handler,
	.blk_status_dump_fn = __cfi_ufs_mq_status_dump,
	.blk_req_dump_fn = __cfi_ufs_mq_dump_request,
	.scheduler_priv = &mas_ufs_mq,
#ifdef CONFIG_MAS_QOS_MQ
	.iosched_name = "MAS QOS MQ",
#else
	.iosched_name = "MAS MQ",
#endif
};

struct blk_tagset_ops mas_ufs_blk_tagset_ops = {
	.tagset_init_tags_fn = __cfi_ufs_tagset_init_tags,
	.tagset_free_tags_fn = __cfi_ufs_tagset_free_tags,
	.tagset_all_tag_busy_iter_fn = __cfi_ufs_tagset_all_tag_busy_iter,
	.tagset_tag_busy_iter_fn = __cfi_ufs_tagset_tag_busy_iter,
	.tagset_power_off_proc_fn = __cfi_ufs_tagset_power_off_proc,
	.queue_ops = &mas_ufs_blk_queue_ops,
};

