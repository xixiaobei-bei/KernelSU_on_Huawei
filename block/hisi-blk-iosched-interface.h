#ifndef __HISI_BLK_IOSCHED_INTERFACE_H__
#define __HISI_BLK_IOSCHED_INTERFACE_H__

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include "blk.h"
#include "blk-mq.h"
#include "blk-mq-tag.h"


extern struct blk_mq_tags *ufs_tagset_init_tags(unsigned int total_tags,
	unsigned int reserved_tags, unsigned int high_prio_tags, int node,
	int alloc_policy);
extern void ufs_tagset_free_tags(struct blk_mq_tags *tags);
extern void ufs_tagset_all_tag_busy_iter(
	struct blk_mq_tags *tags, busy_tag_iter_fn *fn, void *priv);
extern int ufs_mq_iosched_init(struct request_queue *q);
extern int ufs_mq_iosched_exit(struct request_queue *q);
extern int ufs_mq_req_alloc_prep(
	struct blk_mq_alloc_data *data, unsigned long ioflag, bool fs_submit);
extern int ufs_mq_req_init(
	struct request_queue *q, struct blk_mq_ctx *ctx, struct request *rq);
extern int ufs_mq_req_complete(
	struct request *rq, struct request_queue *q, bool succ_done);
extern int ufs_mq_req_deinit(struct request_queue *q, struct request *rq);
extern int ufs_mq_req_insert(
	struct request *req, struct request_queue *q, bool run_list);
extern int ufs_mq_req_requeue(struct request *req, struct request_queue *q);
extern int ufs_mq_req_timeout_handler(struct request *req, bool reserved);
extern void ufs_mq_ctx_put(struct blk_mq_ctx *ctx);
extern int ufs_mq_hctx_get_by_req(
	struct request *rq, struct blk_mq_hw_ctx **hctx);
extern int ufs_mq_hctx_free_in_ctx_map(
	struct request_queue *q, struct blk_mq_hw_ctx *hctx);
extern unsigned int ufs_mq_tag_get(struct blk_mq_alloc_data *data);
extern int ufs_mq_tag_put(struct blk_mq_hw_ctx *hctx, struct blk_mq_ctx *ctx,
	unsigned int tag, struct request *rq);
extern int ufs_mq_tag_update_depth(
	struct request_queue *q, struct blk_mq_tags *tags, unsigned int tdepth);
extern int ufs_mq_tag_busy_iter(
	struct request_queue *q, busy_iter_fn *fn, void *priv);
extern int ufs_mq_tag_wakeup_all(struct blk_mq_tags *tags);
extern int ufs_mq_exec_queue(struct request_queue *q);
extern int ufs_mq_run_hw_queue(struct request_queue *q);
extern int ufs_mq_run_delay_queue(struct request_queue *q);
extern int ufs_mq_run_requeue(struct request_queue *q);
extern int ufs_mq_poll_enable(
	struct request_queue *q, blk_qc_t cookie, bool *enable);
extern void ufs_mq_status_dump(struct request_queue *q, unsigned char *prefix,
	enum blk_dump_scenario scene);
extern void ufs_mq_dump_request(struct request_queue *q, unsigned char *prefix,
	enum blk_dump_scenario scene);
extern void ufs_mq_async_io_dispatch_work_fn(struct work_struct *work);
extern void __ufs_mq_complete_request_remote(void *data);
extern void ufs_mq_flush_plug_list(struct blk_plug *plug, bool from_schedule);
extern void ufs_mq_sync_burst_check_timer_expire(unsigned long data);
extern void ufs_mq_sync_io_dispatch_work_fn(struct work_struct *work);

extern void __cfi_ufs_mq_async_io_dispatch_work_fn(struct work_struct *work);
extern void __cfi__ufs_mq_complete_request_remote(void *data);
extern void __cfi_ufs_mq_flush_plug_list(struct blk_plug *plug, bool from_schedule);
extern void __cfi_ufs_mq_sync_burst_check_timer_expire(unsigned long data);
extern void __cfi_ufs_mq_sync_io_dispatch_work_fn(struct work_struct *work);
extern void __cfi_ufs_mq_io_guard_work_fn(struct work_struct *work);
extern void ufs_mq_io_guard_work_fn(struct work_struct *work);
extern blk_qc_t __cif_ufs_mq_make_request(struct request_queue *q, struct bio *bio);
extern blk_qc_t ufs_mq_make_request(struct request_queue *q, struct bio *bio);
#endif /* __HISI_BLK_IOSCHED_INTERFACE_H__ */
