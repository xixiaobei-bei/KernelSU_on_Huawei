#define pr_fmt(fmt) "[BLK-IO]" fmt
#include "hisi-blk-iosched-interface.h"

void __cfi_ufs_mq_sync_io_dispatch_work_fn(struct work_struct *work)
{
        ufs_mq_sync_io_dispatch_work_fn(work);
}
void __cfi_ufs_mq_sync_burst_check_timer_expire(unsigned long data)
{
        ufs_mq_sync_burst_check_timer_expire(data);
}
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
        ufs_mq_io_guard_work_fn(work);
}

blk_qc_t __cif_ufs_mq_make_request(struct request_queue *q, struct bio *bio)
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
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn, void *priv)
{
        ufs_tagset_all_tag_busy_iter(tags, fn, priv);
}

static inline int __cfi_ufs_mq_iosched_init(struct request_queue *q)
{
	return ufs_mq_iosched_init(q);
}

static inline int __cfi_ufs_mq_iosched_exit(struct request_queue *q)
{
	return ufs_mq_iosched_exit(q);
}

static inline int __cfi_ufs_mq_req_alloc_prep(
	struct blk_mq_alloc_data *data, unsigned long ioflag, bool fs_submit)
{
	return ufs_mq_req_alloc_prep(data, ioflag, fs_submit);
}

static inline int __cfi_ufs_mq_req_init(
	struct request_queue *q, struct blk_mq_ctx *ctx, struct request *rq)
{
	return ufs_mq_req_init(q, ctx, rq);
}

static inline int __cfi_ufs_mq_req_complete(
	struct request *rq, struct request_queue *q, bool succ_done)
{
	return ufs_mq_req_complete(rq, q, succ_done);
}

static inline int __cfi_ufs_mq_req_deinit(
	struct request_queue *q, struct request *rq)
{
	return ufs_mq_req_deinit(q, rq);
}

static inline int __cfi_ufs_mq_req_insert(
	struct request *req, struct request_queue *q, bool run_list)
{
	return ufs_mq_req_insert(req, q, run_list);
}

static inline int __cfi_ufs_mq_req_requeue(
	struct request *req, struct request_queue *q)
{
	return ufs_mq_req_requeue(req, q);
}

static inline int __cfi_ufs_mq_req_timeout_handler(
	struct request *req, bool reserved)
{
	return ufs_mq_req_timeout_handler(req, reserved);
}

static inline void __cfi_ufs_mq_ctx_put(struct blk_mq_ctx *ctx)
{
        ufs_mq_ctx_put(ctx);
}

static inline int __cfi_ufs_mq_hctx_get_by_req(
	struct request *rq, struct blk_mq_hw_ctx **hctx)
{
	return ufs_mq_hctx_get_by_req(rq, hctx);
}

static inline int __cfi_ufs_mq_hctx_free_in_ctx_map(
	struct request_queue *q, struct blk_mq_hw_ctx *hctx)
{
	return ufs_mq_hctx_free_in_ctx_map(q, hctx);
}

static inline unsigned int __cfi_ufs_mq_tag_get(struct blk_mq_alloc_data *data)
{
	return ufs_mq_tag_get(data);
}

static inline int __cfi_ufs_mq_tag_put(struct blk_mq_hw_ctx *hctx,
	struct blk_mq_ctx *ctx, unsigned int tag, struct request *rq)
{
	return ufs_mq_tag_put(hctx, ctx, tag, rq);
}

static inline int __cfi_ufs_mq_tag_update_depth(
	struct request_queue *q, struct blk_mq_tags *tags, unsigned int tdepth)
{
	return ufs_mq_tag_update_depth(q, tags, tdepth);
}

static inline int __cfi_ufs_mq_tag_busy_iter(
	struct request_queue *q, busy_iter_fn *fn, void *priv)
{
	return ufs_mq_tag_busy_iter(q, fn, priv);
}

static inline int __cfi_ufs_mq_tag_wakeup_all(struct blk_mq_tags *tags)
{
	return ufs_mq_tag_wakeup_all(tags);
}

static inline int __cfi_ufs_mq_exec_queue(struct request_queue *q)
{
	return ufs_mq_exec_queue(q);
}

static inline int __cfi_ufs_mq_run_hw_queue(struct request_queue *q)
{
	return ufs_mq_run_hw_queue(q);
}

static inline int __cfi_ufs_mq_run_delay_queue(struct request_queue *q)
{
	return ufs_mq_run_delay_queue(q);
}

static inline int __cfi_ufs_mq_run_requeue(struct request_queue *q)
{
	return ufs_mq_run_requeue(q);
}

static inline int __cfi_ufs_mq_poll_enable(
	struct request_queue *q, blk_qc_t cookie, bool *enable)
{
	return ufs_mq_poll_enable(q, cookie, enable);
}

static inline void __cfi_ufs_mq_status_dump(struct request_queue *q,
	unsigned char *prefix, enum blk_dump_scenario scene)
{
        ufs_mq_status_dump(q, prefix, scene);
}

static inline void __cfi_ufs_mq_dump_request(struct request_queue *q,
	unsigned char *prefix, enum blk_dump_scenario scene)
{
        ufs_mq_dump_request(q, prefix, scene);
}
extern struct hisi_ufs_mq_priv hisi_ufs_mq;
struct blk_queue_ops hisi_ufs_blk_queue_ops = {
	.io_scheduler_strategy = IOSCHED_HISI_UFS_MQ,
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
	.mq_hctx_free_in_ctx_map_fn = ufs_mq_hctx_free_in_ctx_map,
	.mq_tag_get_fn = __cfi_ufs_mq_tag_get,
	.mq_tag_put_fn = __cfi_ufs_mq_tag_put,
	.mq_tag_update_depth_fn = __cfi_ufs_mq_tag_update_depth,
	.mq_tag_busy_iter_fn = __cfi_ufs_mq_tag_busy_iter,
	.mq_tag_wakeup_all_fn = __cfi_ufs_mq_tag_wakeup_all,
	.mq_exec_queue_fn = __cfi_ufs_mq_exec_queue,
	.mq_run_hw_queue_fn = __cfi_ufs_mq_run_hw_queue,
	.mq_run_delay_queue_fn = __cfi_ufs_mq_run_delay_queue,
	.mq_run_requeue_fn = __cfi_ufs_mq_run_requeue,
	.blk_poll_enable_fn = __cfi_ufs_mq_poll_enable,
	.blk_status_dump_fn = __cfi_ufs_mq_status_dump,
	.blk_req_dump_fn = __cfi_ufs_mq_dump_request,
	.io_scheduler_private_data = &hisi_ufs_mq,
};

struct blk_tagset_ops hisi_ufs_blk_tagset_ops = {
	.tagset_init_tags_fn = __cfi_ufs_tagset_init_tags,
	.tagset_free_tags_fn = __cfi_ufs_tagset_free_tags,
	.tagset_all_tag_busy_iter_fn = __cfi_ufs_tagset_all_tag_busy_iter,
	.queue_ops = &hisi_ufs_blk_queue_ops,
};

