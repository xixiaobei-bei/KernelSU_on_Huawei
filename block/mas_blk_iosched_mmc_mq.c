/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: MAS MQ ioscheduler
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
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/writeback.h>
#include <linux/mm_inline.h>
#include <linux/mpage.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <trace/events/block.h>
#include <linux/rbtree.h>
#include <linux/types.h>
#include <linux/hisi/pagecache_manage.h>
#include <linux/hisi/powerkey_event.h>
#include "blk.h"
#include "blk-mq.h"
#include "blk-mq-tag.h"
#include "blk-wbt.h"
#include "mas_blk.h"
#include "mas_blk_iosched_mmc_interface.h"
#include "mas_blk_mmc_mq_tag.h"
#include "mas_blk_flush_interface.h"
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
#include "mas_blk_ft.h"
#endif

#define MAS_MMC_MQ_PLUG_MERGE_ENABLE 0
#define MAS_MMC_MQ_PLUG_MERGE_MAX_SIZE (512 * 1024)

#define IO_HW_PENDING_THRESH 8

#define MAS_BLK_IO_GUARD_PERIOD_MS 2000

#define SEND_READ_REQ_LMT 100
#define SEND_WRITE_REQ_LMT 1

/* default values for starvation time limit */
#define HP_EXPIRE_TIME 200
#define RP_EXPIRE_TIME 500

#define	MAS_REG_STARVATION_TOLLERANCE	5000
#define	MAS_LOW_STARVATION_TOLLERANCE	10000

#define MAS_READ_IDLE_TIME_MSEC 5

enum mas_blk_mq_class_ioprio {
	MAS_MMC_MQ_CLASS_VIP = 0,
	MAS_MMC_MQ_CLASS_SYNC,
	MAS_MMC_MQ_CLASS_ASYNC,
	MAS_MMC_MQ_CLASS_MAX,
};

enum mas_blk_mq_req_prio {
	MAS_MMC_MQ_REQ_VIP_READ = 0,
	MAS_MMC_MQ_REQ_VIP_WRITE,
	MAS_MMC_MQ_REQ_SYNC_READ,
	MAS_MMC_MQ_REQ_SYNC_WRITE,
	MAS_MMC_MQ_REQ_ASYNC_READ,
	MAS_MMC_MQ_REQ_ASYNC_WRITE,
	MAS_MMC_MQ_REQ_MAX_PRIO,
};

enum mas_mmc_mq_async_sched_type {
	/* async io will be sort by submitted time */
	MAS_MMC_MQ_SCHED_ASYNC_FIFO = 0,
};

struct mas_mmc_mq_work {
	ktime_t last_queue_tm;
	ktime_t last_enter_tm;
	ktime_t last_exit_tm;
	struct delayed_work io_dispatch_work;
	struct request_queue *queue;
};

struct mas_queue_params {
	bool idling_enabled;
	int quantum;
	bool is_urgent;
};

static const struct mas_queue_params mas_queues_def[] = {
/* idling_enabled, quantum, is_urgent */
	{false, 10, true},	/* MAS_MMC_MQ_REQ_VIP_READ */
	{false, 1, false},	/* MAS_MMC_MQ_REQ_VIP_WRITE */
	{true, 100, true},	/* MAS_MMC_MQ_REQ_SYNC_READ */
	{false, 1, false},	/* MAS_MMC_MQ_REQ_SYNC_WRITE */
	{false, 1, false},	/* MAS_MMC_MQ_REQ_ASYNC_READ */
	{false, 1, false}	/* MAS_MMC_MQ_REQ_ASYNC_WRITE */
};

struct mas_queue {
	struct list_head	fifo;
	enum mas_blk_mq_class_ioprio	prio;

	unsigned int		nr_dispatched;
	unsigned int		nr_req;
	int			disp_quantum;
};

struct read_idle_data {
	s64	last_rd_insert_time;
	s64	last_rd_send_time;
	s64	read_idle_freq_ms;
	bool	read_begin_idling;
};

/*
 * This struct defines all the variable for mmc mq io-scheduler per lld.
 */
struct mas_mmc_sched_ds_lld {
	struct blk_dev_lld *lld;
	/* IOs with cp flag in low level driver */
	atomic_t cp_io_inflt_cnt;

	/* MQ tag usage statistic */
	atomic_t mq_tag_used_cnt;
	atomic_t mq_resrvd_tag_used_cnt;
	atomic_t mq_prio_tag_used_cnt;
	/* count of VIP IOs that wait for tags */
	atomic_t vip_wait_cnt;

	/* Sync IOs in low level driver */
	atomic_t sync_io_inflt_cnt;
	/* Async IOs in low level driver */
	atomic_t async_io_inflt_cnt;
	/* VIP IOs in low level driver */
	atomic_t vip_io_inflt_cnt;
	/* FG IOs in low level driver */
	atomic_t fg_io_inflt_cnt;

	spinlock_t sync_disp_lock;
	/* List for all requeued high prio IOs */
	/* IO counts in high_prio_sync_dispatch_list */
	atomic_t hp_io_list_cnt;

	/* List for all requeued sync IOs */
	/* IO counts in sync_dispatch_list */
	atomic_t sync_io_list_cnt;
	int reg_prio_starvation_counter;
	int reg_prio_starvation_limit;

	bool async_io_sched_inited;
	/* IO counts in async_dispatch_list */
	atomic_t async_io_list_cnt;
	int low_prio_starvation_counter;
	int low_prio_starvation_limit;

	enum mas_blk_mq_class_ioprio ioprio_class_type;
	struct mas_queue mas_queues[MAS_MMC_MQ_REQ_MAX_PRIO];

	struct read_idle_data rd_idle_data;

	atomic_t ref_cnt;

};

struct mas_mmc_mq_sched {
	struct request_queue *q;
	/* The work will dispatch all the requeue sync IOs */
	struct mas_mmc_mq_work req_dispatch_work;
	struct list_head io_guard_list_node;
	struct mas_mmc_sched_ds_lld *sched_ds_lld;
};

/*
 * This struct defines all the variable for mmc mq async io-scheduler.
 */
struct mas_mmc_mq_async_sched {
	enum mas_mmc_mq_async_sched_type type;
	/* async io sched init interface */
	void (*async_sched_init_fn)(struct mas_mmc_sched_ds_lld *ds_lld);
	/* async io insert request interface */
	void (*async_sched_insert_fn)(
		struct request *rq, struct mas_mmc_sched_ds_lld *ds_lld);
	/* async io seek request interface */
	struct request *(*async_sched_seek_fn)(
		const struct mas_mmc_sched_ds_lld *ds_lld);
	/* async io requeue request interface */
	void (*async_sched_requeue_fn)(
		struct request *rq, struct mas_mmc_sched_ds_lld *ds_lld);
	/* async io request merge interface */
	bool (*async_sched_attempt_merge_fn)(
		struct bio *bio, struct request_queue *q);
	/* interface to query async io sched empty or not */
	bool (*async_sched_is_empty_fn)(const struct mas_mmc_sched_ds_lld *ds_lld);
};

struct mas_mmc_mq_priv {
	struct mas_mmc_mq_async_sched *async_io_sched_strategy;
};

DECLARE_PER_CPU(struct list_head, blk_cpu_done);

static struct workqueue_struct *mas_blk_mq_req_disp_wq;

static struct workqueue_struct *mas_blk_io_guard_wq;
static LIST_HEAD(mas_io_guard_queue_list);
static DEFINE_SPINLOCK(io_guard_queue_list_lock);
static struct delayed_work mas_io_guard_work;

void mmc_mq_dispatch_request(const struct request_queue *q);

static struct mas_mmc_sched_ds_lld *get_sched_ds_lld(
	const struct request_queue *q)
{
	void *queuedata = q->mas_queue.cust_queuedata;

	return queuedata ? ((struct mas_mmc_mq_sched *)queuedata)->sched_ds_lld
		       : NULL;
}

void mmc_mq_inc_vip_wait_cnt(struct blk_mq_alloc_data *data)
{
	struct mas_mmc_sched_ds_lld *ds_lld = get_sched_ds_lld(data->q);
	if(unlikely(!ds_lld))
		return;

	atomic_inc(&ds_lld->vip_wait_cnt);
}

void mmc_mq_dec_vip_wait_cnt(struct blk_mq_alloc_data *data)
{
	struct mas_mmc_sched_ds_lld *ds_lld = get_sched_ds_lld(data->q);
	if(unlikely(!ds_lld))
		return;

	atomic_dec_if_positive(&ds_lld->vip_wait_cnt);
}

void mmc_reset_vip_wait_cnt(struct blk_mq_alloc_data *data)
{
	struct mas_mmc_sched_ds_lld *ds_lld = get_sched_ds_lld(data->q);

	if (atomic_read(&ds_lld->mq_prio_tag_used_cnt))
		return;

	atomic_set(&ds_lld->vip_wait_cnt, 0);
}

int mmc_mq_vip_tag_wait_cnt(struct blk_mq_alloc_data *data)
{
	struct mas_mmc_sched_ds_lld *ds_lld = get_sched_ds_lld(data->q);

	return atomic_read(&ds_lld->vip_wait_cnt);
}

static unsigned int mmc_tagset_get_tag(const struct blk_mq_alloc_data *data)
{
	unsigned int tag;
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;

	tag = mmc_tagset_bt_get((struct blk_mq_alloc_data *)data,
		&data->hctx->tags->bitmap_tags, data->hctx);
	if (likely(tag != BLK_MQ_TAG_FAIL)) {
		ds_lld = get_sched_ds_lld(data->q);
		if(unlikely(!ds_lld))
			return BLK_MQ_TAG_FAIL;
		atomic_inc(&(ds_lld->mq_tag_used_cnt));
	}
	return tag;
}

static unsigned int mmc_tagset_get_reserved_tag(
	const struct blk_mq_alloc_data *data)
{
	unsigned int tag;
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;

	if (unlikely(!data->hctx->tags->nr_reserved_tags)) {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
		mas_blk_rdr_panic("nr_reserved_tags is zero!");
#else
		return BLK_MQ_TAG_FAIL;
#endif
	}

	tag = mmc_tagset_bt_get((struct blk_mq_alloc_data *)data,
		&data->hctx->tags->breserved_tags, data->hctx);
	if (likely(tag != BLK_MQ_TAG_FAIL)) {
		ds_lld = get_sched_ds_lld(data->q);
		if(unlikely(!ds_lld))
			return BLK_MQ_TAG_FAIL;
		atomic_inc(&(ds_lld->mq_resrvd_tag_used_cnt));
		tag += data->hctx->tags->reserved_tags_id_offset;
	}

	return tag;
}

static unsigned int mmc_tagset_get_high_prio_tag(
	const struct blk_mq_alloc_data *data)
{
	unsigned int tag;
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;

	if (unlikely(!data->hctx->tags->nr_high_prio_tags)) {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
		mas_blk_rdr_panic("nr_high_prio_tags is zero!");
#else
		return BLK_MQ_TAG_FAIL;
#endif
	}

	tag = mmc_tagset_bt_get((struct blk_mq_alloc_data *)data,
		&data->hctx->tags->highprio_tags, data->hctx);
	if (likely(tag != BLK_MQ_TAG_FAIL)) {
		ds_lld = get_sched_ds_lld(data->q);
		if(unlikely(!ds_lld))
			return BLK_MQ_TAG_FAIL;
		atomic_inc(&(ds_lld->mq_prio_tag_used_cnt));
		tag += data->hctx->tags->high_prio_tags_id_offset;
	}
	return tag;
}


unsigned int mmc_mq_tag_get(const struct blk_mq_alloc_data *data)
{
	unsigned long flag = data->io_flag & (REQ_SYNC | REQ_FG_META);
	unsigned int tag;
	if (likely(flag == REQ_SYNC))
		tag = mmc_tagset_get_tag(data);
	else if (flag & REQ_FG_META)
		tag = mmc_tagset_get_high_prio_tag(data);
	else
		tag = mmc_tagset_get_reserved_tag(data);

	return tag;
}

int mmc_mq_tag_put(
	const struct blk_mq_hw_ctx *hctx, unsigned int tag,
	const struct request *rq)
{
	struct blk_mq_tags *tags = hctx->tags;
	struct mas_mmc_sched_ds_lld *ds_lld = get_sched_ds_lld(hctx->queue);
	if(unlikely(!ds_lld))
		return BLK_MQ_TAG_FAIL;

	if (unlikely(tag >= tags->high_prio_tags_id_offset)) {
		sbitmap_queue_clear(&tags->highprio_tags,
			(tag - tags->high_prio_tags_id_offset),
			rq->mas_req.mq_ctx_generate->cpu);
		atomic_dec(&(ds_lld->mq_prio_tag_used_cnt));
	} else if (unlikely(tag >= tags->reserved_tags_id_offset)) {
		sbitmap_queue_clear(&tags->breserved_tags,
			(tag - tags->reserved_tags_id_offset),
			rq->mas_req.mq_ctx_generate->cpu);
		atomic_dec(&(ds_lld->mq_resrvd_tag_used_cnt));
	} else {
		sbitmap_queue_clear(&tags->bitmap_tags, tag,
			rq->mas_req.mq_ctx_generate->cpu);
		atomic_dec(&(ds_lld->mq_tag_used_cnt));
	}
	return 0;
}


static inline void mmc_mq_dump_dispatch_work_stat(
	const struct mas_mmc_mq_work *work)
{
	pr_err("lst_queue_t: %lld, lst_in_t: %lld, lst_out_t: %lld\n",
		work->last_queue_tm, work->last_enter_tm, work->last_exit_tm);
}

void mmc_mq_dump_request(const struct request_queue *q, enum blk_dump_scene s)
{
	struct request *pos = NULL;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;
	int i;

	if (!sched_ds || !(sched_ds->sched_ds_lld))
		return;

	ds_lld = sched_ds->sched_ds_lld;
	for (i = (int)MAS_MMC_MQ_REQ_VIP_READ; i < (int)MAS_MMC_MQ_REQ_MAX_PRIO; i++) {
		if (!list_empty(&ds_lld->mas_queues[i].fifo)) {
			pr_err("mas_queues[%d].list:\n",i);
			list_for_each_entry(pos, &ds_lld->mas_queues[i].fifo, queuelist)
				if (pos->q == q)
					mas_blk_dump_request(pos, s);
		}
		mmc_mq_dump_dispatch_work_stat(&sched_ds->req_dispatch_work);
	}

}

static inline bool mmc_mq_rq_is_fg(const struct request *rq)
{
	return (rq->cmd_flags & REQ_FG_META) && !(rq->cmd_flags & REQ_VIP);
}

static inline int mmc_mq_get_hp_inflt(const struct mas_mmc_sched_ds_lld *ds_lld)
{
	return atomic_read(&ds_lld->fg_io_inflt_cnt) + atomic_read(&ds_lld->vip_io_inflt_cnt);
}

static int mmc_mq_fs_io_inflt(
	struct request *rq, struct mas_mmc_sched_ds_lld *ds_lld)
{
	if (unlikely(rq->cmd_flags & REQ_FG_META)) {
		/* Dispatch unconditionally for high priority sync io */
		if (mmc_mq_rq_is_fg(rq))
			atomic_inc(&ds_lld->fg_io_inflt_cnt);
		else
			atomic_inc(&ds_lld->vip_io_inflt_cnt);

		goto end;
	}

	if (likely(rq->cmd_flags & REQ_SYNC))
		atomic_inc(&ds_lld->sync_io_inflt_cnt);
	else
		atomic_inc(&ds_lld->async_io_inflt_cnt);

end:

	return BLK_STS_OK;

}

static void mmc_mq_rq_inflt_add(
	const struct request *rq, struct mas_mmc_sched_ds_lld *ds_lld)
{
	if (unlikely(blk_rq_is_passthrough((struct request *)rq)))
		return;

	if (likely(rq->cmd_flags & REQ_SYNC)) {
		if (unlikely(rq->cmd_flags & REQ_FG_META))
			if (mmc_mq_rq_is_fg(rq))
				atomic_inc(&ds_lld->fg_io_inflt_cnt);
			else
				atomic_inc(&ds_lld->vip_io_inflt_cnt);
		else
			atomic_inc(&ds_lld->sync_io_inflt_cnt);
	} else {
		atomic_inc(&ds_lld->async_io_inflt_cnt);
	}
}

static void mmc_mq_rq_inflt_update(
	const struct request *rq,
	struct mas_mmc_sched_ds_lld *ds_lld, bool update_complete_time)
{
	if (likely(rq->cmd_flags & REQ_SYNC)) {
		if (unlikely(rq->cmd_flags & REQ_FG_META)) {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
			if (unlikely(!mmc_mq_get_hp_inflt(ds_lld)))
				mas_blk_rdr_panic("high_prio_sync_io is 0!");
#endif
			if (mmc_mq_rq_is_fg(rq))
				atomic_dec(&ds_lld->fg_io_inflt_cnt);
			else
				atomic_dec(&ds_lld->vip_io_inflt_cnt);
		} else {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
			if (!atomic_read(&ds_lld->sync_io_inflt_cnt))
				mas_blk_rdr_panic("sync_io_inflt_cnt is 0!");
#endif
			atomic_dec(&ds_lld->sync_io_inflt_cnt);
		}
	} else {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
		if (unlikely(!atomic_read(&ds_lld->async_io_inflt_cnt)))
			mas_blk_rdr_panic("async_io_inflt_cnt is 0!");
#endif
		atomic_dec(&ds_lld->async_io_inflt_cnt);
	}
	
}

/**
 * mmc_mq_requeue_invalidate_tags - invalidate all pending tags
 * @q:  the request queue for the device
 *
 *  Description:
 *   Hardware conditions may dictate a need to stop all pending requests.
 *   In this case, we will safely clear the block side of the tag queue and
 *   requeue all requests to the request queue in the right order.
 **/
void mmc_mq_requeue_invalidate_reqs(struct request_queue *q)
{
	struct list_head *tmp = NULL;
	struct list_head *n = NULL;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;

	lockdep_assert_held(q->queue_lock);

	list_for_each_safe(tmp, n, &q->tag_busy_list) {
		struct request *rq;
		rq = cmdq_list_entry_rq(tmp);
		list_del_init(&rq->cmdq_list);

		/* update request inflt */
		if (!blk_mark_rq_complete(rq))
			mmc_mq_rq_inflt_update(rq, ds_lld, true);

		blk_mq_requeue_request(rq, true);
	}
}

static inline blk_status_t __mmc_mq_queue_rq_internal(
	const struct blk_mq_hw_ctx *hctx, const struct request_queue *q,
	const struct blk_mq_queue_data *bd,
	const struct mas_mmc_sched_ds_lld *ds_lld)
{
	blk_status_t ret;

	if (blk_mq_hctx_stopped((struct blk_mq_hw_ctx *)hctx) || blk_queue_quiesced(q)) {
		return BLK_STS_RESOURCE;
	}

	ret = q->mq_ops->queue_rq((struct blk_mq_hw_ctx *)hctx, bd);
	mas_blk_latency_req_check(bd->rq, REQ_PROC_STAGE_MQ_QUEUE_RQ);
	return ret;
}

static enum mas_blk_mq_class_ioprio mmc_mq_get_req_flag(struct request *req)
{
	if ((req->cmd_flags & REQ_VIP) || (req->cmd_flags & REQ_FG_META))
		return MAS_MMC_MQ_CLASS_VIP;
	if ((req->cmd_flags & REQ_SYNC) || (req_op(req) == REQ_OP_WRITE))
		return MAS_MMC_MQ_CLASS_SYNC;
	else
		return MAS_MMC_MQ_CLASS_ASYNC;
}

static bool mmc_mq_sync_req_pending(
	const struct mas_mmc_sched_ds_lld *ds_lld)
{
	int i;

	for (i = MAS_MMC_MQ_REQ_SYNC_READ; i < MAS_MMC_MQ_REQ_ASYNC_READ; i++) {
		if (!list_empty(&ds_lld->mas_queues[i].fifo))
			return true;
	}
	return false;
}

static bool mmc_mq_async_req_pending(
	const struct mas_mmc_sched_ds_lld *ds_lld)
{
	int i;

	for (i = MAS_MMC_MQ_REQ_ASYNC_READ; i < MAS_MMC_MQ_REQ_MAX_PRIO; i++) {
		if (!list_empty(&ds_lld->mas_queues[i].fifo))
			return true;
	}
	return false;
}

static enum mas_blk_mq_req_prio mmc_mq_get_req_class_prio(struct mas_mmc_sched_ds_lld *ds_lld,
	struct request *req)
{
	enum mas_blk_mq_class_ioprio req_ioprio_class;
	enum mas_blk_mq_req_prio ret;

	req_ioprio_class = mmc_mq_get_req_flag(req);

	switch(req_ioprio_class) {
	case MAS_MMC_MQ_CLASS_VIP:
		atomic_inc(&ds_lld->hp_io_list_cnt);
		if (req_op(req) == REQ_OP_READ)
			ret = MAS_MMC_MQ_REQ_VIP_READ;
		else
			ret = MAS_MMC_MQ_REQ_VIP_WRITE;
		break;

	case MAS_MMC_MQ_CLASS_SYNC:
		atomic_inc(&ds_lld->sync_io_list_cnt);
		if (req_op(req) == REQ_OP_READ)
			ret = MAS_MMC_MQ_REQ_SYNC_READ;
		else
			ret = MAS_MMC_MQ_REQ_SYNC_WRITE;
		break;

	case MAS_MMC_MQ_CLASS_ASYNC:
		atomic_inc(&ds_lld->async_io_list_cnt);
		if (req_op(req) == REQ_OP_READ)
			ret = MAS_MMC_MQ_REQ_ASYNC_READ;
		else
			ret = MAS_MMC_MQ_REQ_ASYNC_WRITE;
		break;
	default:
		ret = MAS_MMC_MQ_REQ_ASYNC_WRITE;

	}

	return ret;
}


static void mmc_mq_dispatch_rw_cnt_set(struct mas_mmc_sched_ds_lld *ds_lld,
	struct request *rq)
{
	enum mas_blk_mq_class_ioprio req_ioprio_class = mmc_mq_get_req_flag(rq);
	enum mas_blk_mq_req_prio req_prio = mmc_mq_get_req_class_prio(ds_lld, rq);

	if (req_ioprio_class == MAS_MMC_MQ_CLASS_VIP) {
		atomic_dec(&ds_lld->hp_io_list_cnt);
		if (mmc_mq_sync_req_pending(ds_lld))
			ds_lld->reg_prio_starvation_counter++;
		if (mmc_mq_async_req_pending(ds_lld))
			ds_lld->low_prio_starvation_counter++;
	} else if (req_ioprio_class == MAS_MMC_MQ_CLASS_SYNC) {
		atomic_dec(&ds_lld->sync_io_list_cnt);
		ds_lld->reg_prio_starvation_counter = 0;
		if (mmc_mq_async_req_pending(ds_lld))
			ds_lld->low_prio_starvation_counter++;
	} else {
		atomic_dec(&ds_lld->async_io_list_cnt);
		ds_lld->low_prio_starvation_counter = 0;
	}

	atomic_inc((atomic_t *)&ds_lld->mas_queues[req_prio].nr_dispatched);
}


/* for all IO except passthrough */
static int __mmc_mq_queue_rq(
	struct request *rq, const struct blk_mq_hw_ctx *hctx,
	const struct mas_mmc_sched_ds_lld *ds_lld,
	const struct blk_mq_queue_data *bd,
	const struct request_queue *q)
{
	int ret;

	ret = mmc_mq_fs_io_inflt(rq, (struct mas_mmc_sched_ds_lld *)ds_lld);
	if (unlikely(ret != BLK_STS_OK))
		goto exit;

	ret = __mmc_mq_queue_rq_internal(hctx, q, bd, ds_lld);
	if (likely(ret == BLK_STS_OK)) {
		return ret;
	}

	rq->mas_req.requeue_reason = REQ_REQUEUE_IO_HW_LIMIT;
	if (mmc_mq_get_hp_inflt(ds_lld) +
			atomic_read(&ds_lld->sync_io_inflt_cnt) +
			atomic_read(&ds_lld->async_io_inflt_cnt) <=
		IO_HW_PENDING_THRESH)
		rq->mas_req.requeue_reason = REQ_REQUEUE_IO_HW_PENDING;

exit:
	mmc_mq_rq_inflt_update(
		rq, (struct mas_mmc_sched_ds_lld *)ds_lld, false);
	return ret;
}

/* for sync and async dispatch */
static int mmc_mq_queue_rq(
	struct request *rq, const struct blk_mq_hw_ctx *hctx,
	const struct mas_mmc_sched_ds_lld *ds_lld,
	const struct blk_mq_queue_data *bd,
	const struct request_queue *q)
{
	if (unlikely(blk_rq_is_passthrough(rq))) {
		rq->cmd_flags |= REQ_VIP | REQ_FG | REQ_SYNC;
		return __mmc_mq_queue_rq_internal(hctx, q, bd, ds_lld);
	}
	return __mmc_mq_queue_rq(rq, hctx, ds_lld, bd, q);
}
static int mmc_mq_check_list_empty(struct mas_mmc_mq_sched *sched_ds)
{
	int i;
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;

	if(unlikely(!sched_ds))
		return false;

	ds_lld = sched_ds->sched_ds_lld;
	if(unlikely(!ds_lld))
		return false;

	for (i = MAS_MMC_MQ_REQ_VIP_READ; i < MAS_MMC_MQ_REQ_MAX_PRIO; i++) {
		if (!list_empty(&ds_lld->mas_queues[i].fifo))
			return true;
	}
	return false;

}

static void mmc_mq_run_delay_sync_list(
	const struct request_queue *q, unsigned long delay_jiffies)
{
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);

	if (!mmc_mq_check_list_empty(sched_ds))
		return;

	queue_delayed_work(mas_blk_mq_req_disp_wq,
		&sched_ds->req_dispatch_work.io_dispatch_work, delay_jiffies);
	sched_ds->req_dispatch_work.last_queue_tm = ktime_get();
}

static inline void mmc_mq_run_sync_list(const struct request_queue *q)
{
	mmc_mq_run_delay_sync_list(q, 0);
}

static void mmc_mq_insert_list(struct request *rq, struct request_queue *q)
{
	unsigned long flags;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;
	enum mas_blk_mq_req_prio req_prio = mmc_mq_get_req_class_prio(ds_lld, rq);
	s64 diff_ms;

	if (rq->cmd_flags & REQ_SYNC)
		mas_blk_latency_req_check(
			(struct request *)rq, REQ_PROC_STAGE_MQ_ADDTO_SYNC_LIST);
	else
		mas_blk_latency_req_check(
			(struct request *)rq, REQ_PROC_STAGE_MQ_ADDTO_ASYNC_LIST);

	if (req_prio == MAS_MMC_MQ_REQ_SYNC_READ) {
		diff_ms = ktime_to_ms(ktime_sub(ktime_get(),
				ds_lld->rd_idle_data.last_rd_insert_time));

		if (diff_ms < ds_lld->rd_idle_data.read_idle_freq_ms) {
			ds_lld->rd_idle_data.read_begin_idling = true;
		} else {
			ds_lld->rd_idle_data.read_begin_idling = false;
		}

		ds_lld->rd_idle_data.last_rd_insert_time = ktime_get();
	}

	spin_lock_irqsave(&ds_lld->sync_disp_lock, flags);
	list_add_tail(&rq->queuelist, &ds_lld->mas_queues[req_prio].fifo);
	spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);
}

static void mmc_mq_requeue_insert_list(struct request *rq, struct request_queue *q)
{
	unsigned long flags;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;
	enum mas_blk_mq_req_prio req_prio = mmc_mq_get_req_class_prio(ds_lld, rq);

	spin_lock_irqsave(&ds_lld->sync_disp_lock, flags);
	list_add(&rq->queuelist, &ds_lld->mas_queues[req_prio].fifo);
	spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);
}


static void mmc_mq_requeue_sync_list(
	struct request *rq, struct request_queue *q)
{
	mmc_mq_requeue_insert_list(rq, q);
	mmc_mq_run_sync_list(q);
}

static bool mmc_mq_bio_is_sync(
	const struct request_queue *q, struct bio *bio,
	unsigned int dispatch_op)
{
	if (dispatch_op == REQ_OP_READ || dispatch_op == REQ_OP_FLUSH ||
		(bio->bi_opf & (REQ_SYNC | REQ_FG_META | REQ_FUA))) {
		bio->bi_opf |= REQ_SYNC;
		return true;
	}
	return false;
}

static inline void mmc_mq_bio_to_request(struct request *rq, struct bio *bio)
{
	blk_init_request_from_bio(rq, bio);
	blk_account_io_start(rq, true);
}

static bool mmc_mq_attempt_merge(
	const struct request_queue *q, const struct bio *bio)
{
	unsigned long flags;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;

	if (unlikely(bio->bi_opf & REQ_SYNC))
		return false;

	if (likely(q->mas_queue_ops &&
		    q->mas_queue_ops->scheduler_priv)) {
		struct mas_mmc_mq_priv *priv = (struct mas_mmc_mq_priv
				*)(q->mas_queue_ops->scheduler_priv);

		if (priv->async_io_sched_strategy
				->async_sched_attempt_merge_fn) {
			bool merged = false;

			spin_lock_irqsave(&ds_lld->sync_disp_lock, flags);
			merged = priv->async_io_sched_strategy
					 ->async_sched_attempt_merge_fn(
						 (struct bio *)bio,
						 (struct request_queue *)q);
			spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);
			return merged;
		}
	}

	return false;
}

static bool mmc_mq_merge_queue_io(struct blk_mq_hw_ctx *hctx,
	struct blk_mq_ctx *ctx, struct request *rq, struct bio *bio)
{

	struct request_queue *q = hctx->queue;

	if (!bio_mergeable(bio)) {
		mmc_mq_bio_to_request(rq, bio);
		spin_lock(&ctx->lock);
insert_rq:
		mmc_mq_req_insert(rq, hctx->queue);

		spin_unlock(&ctx->lock);
		return false;
	}

	spin_lock(&ctx->lock);
	if (!mmc_mq_attempt_merge(q, bio)) {
		mmc_mq_bio_to_request(rq, bio);
		goto insert_rq;
	}
	spin_unlock(&ctx->lock);

	blk_mq_free_request(rq);

	return true;
}

static __always_inline bool mmc_mq_make_flush_request(
	unsigned int dispatch_op, struct request *rq, const struct bio *bio)
{
	if (unlikely((dispatch_op == REQ_OP_FLUSH) ||
		     (bio->bi_opf & REQ_PREFLUSH))) {
#ifdef CONFIG_MAS_IO_DEBUG_TRACE
		trace_mas_io(__func__, "flush+fua", MAS_IO_TRACE_LEN);
#endif
		mmc_mq_bio_to_request(rq, (struct bio *)bio);
		blk_insert_flush(rq);
		return true;
	}
	return false;
}

static void mmc_mq_make_sync_request(struct bio *bio, struct request *rq,
	const struct blk_mq_alloc_data *alloc_data)
{
	mmc_mq_bio_to_request(rq, bio);
	mmc_mq_req_insert(rq, alloc_data->hctx->queue);
	mmc_mq_dispatch_request(alloc_data->hctx->queue);
}

blk_qc_t mmc_mq_make_request(struct request_queue *q, struct bio *bio)
{
	struct blk_mq_alloc_data alloc_data = {.flags = 0};
	struct request *rq = NULL;
	blk_qc_t cookie;
	unsigned int wb_acct = false;
	bool is_sync = mmc_mq_bio_is_sync(q, bio, bio_op(bio));

	blk_queue_split(q, &bio);

	wb_acct = wbt_wait(q->rq_wb, bio, NULL);
	mas_blk_latency_bio_check(bio, BIO_PROC_STAGE_WBT);

	rq = blk_mq_get_request(q, bio, bio->bi_opf, &alloc_data);
	if (unlikely(!rq)) {
		__wbt_done(q->rq_wb, wb_acct);
		if (bio->bi_opf & REQ_NOWAIT)
			bio_wouldblock_error(bio);
		return BLK_QC_T_NONE; /*lint !e501 */
	}

	wbt_track(&rq->issue_stat, wb_acct);

	cookie = request_to_qc_t(alloc_data.hctx, rq);
	if (mmc_mq_make_flush_request(bio_op(bio), rq, bio)) {
		blk_mq_run_hw_queue(alloc_data.hctx, !is_sync);
		blk_mq_put_ctx(alloc_data.ctx);
		goto done;
	}

	if (likely(rq->cmd_flags & REQ_SYNC)) {
		blk_mq_put_ctx(alloc_data.ctx);
		mmc_mq_make_sync_request(
			bio, rq, &alloc_data);
		goto done;
	}

	if (mmc_mq_merge_queue_io(alloc_data.hctx, alloc_data.ctx, rq, bio)) {
		blk_mq_put_ctx(alloc_data.ctx);
	} else {
		blk_mq_put_ctx(alloc_data.ctx);
		mmc_mq_dispatch_request(alloc_data.hctx->queue);
	}

done:
#ifdef CONFIG_MAS_IO_DEBUG_TRACE
	trace_mas_io(__func__, "out", MAS_IO_TRACE_LEN);
#endif
	return cookie;
}

static void mmc_blk_mq_restart_cycle(struct mas_mmc_sched_ds_lld *ds_lld,
				int start_idx, int end_idx)
{
	int i;

	for (i = start_idx; i < end_idx; i++)
		ds_lld->mas_queues[i].nr_dispatched = 0;
}

static struct request *mmc_mq_pick_sync_rq(
	struct mas_mmc_sched_ds_lld *ds_lld)
{
	struct request *rq = NULL;
	enum mas_blk_mq_class_ioprio req_class_type = ds_lld->ioprio_class_type;
	bool restart_flg = true;
	int start_index;
	int end_index;
	int ret = 0;
	int i;

	switch (req_class_type) {
	case MAS_MMC_MQ_CLASS_VIP:
		start_index = MAS_MMC_MQ_REQ_VIP_READ;
		end_index = MAS_MMC_MQ_REQ_SYNC_READ;
		break;
	case MAS_MMC_MQ_CLASS_SYNC:
		start_index = MAS_MMC_MQ_REQ_SYNC_READ;
		end_index = MAS_MMC_MQ_REQ_ASYNC_READ;
		break;
	case MAS_MMC_MQ_CLASS_ASYNC:
		start_index = MAS_MMC_MQ_REQ_ASYNC_READ;
		end_index = MAS_MMC_MQ_REQ_MAX_PRIO;
		break;
	default:
		return NULL;
	}

	i = start_index;
	do {
		if (list_empty(&ds_lld->mas_queues[i].fifo) ||
		    ds_lld->mas_queues[i].nr_dispatched >=
		    ds_lld->mas_queues[i].disp_quantum) {
			i++;
			if (i == end_index && restart_flg) {
				mmc_blk_mq_restart_cycle(ds_lld, start_index, end_index);
				i = start_index;
				restart_flg = false;
			}
		} else {
			ret = i;
			break;
		}
	} while (i < end_index);

	if (ret >= 0) {
		if (unlikely(!list_empty_careful(&ds_lld->mas_queues[ret].fifo))) {
			rq = list_first_entry(&ds_lld->mas_queues[ret].fifo, struct request, queuelist);
			if (rq && (ret == MAS_MMC_MQ_REQ_SYNC_READ))
				ds_lld->rd_idle_data.last_rd_send_time = ktime_get();
		}
	}

	return rq;
}



static int mmc_mq_be_expire_adjust(
	struct mas_mmc_sched_ds_lld *ds_lld)
{
	int i = 0;
	unsigned int max_expire_time = 0;
	int expire_number = 0;
	unsigned int timeout = 0;
	int expire_index = -1;
	unsigned int expire_time = 0;
	unsigned long temp_jiffies = jiffies;
	struct request *check_req = NULL;

	for (i = MAS_MMC_MQ_REQ_VIP_READ; i < MAS_MMC_MQ_REQ_MAX_PRIO; i++) {
		if (list_empty(&ds_lld->mas_queues[i].fifo))
			continue;
		check_req = list_entry_rq(ds_lld->mas_queues[i].fifo.next);
		expire_time = jiffies_to_msecs(temp_jiffies -
			check_req->start_time);
		if (i < MAS_MMC_MQ_REQ_ASYNC_READ && expire_time > HP_EXPIRE_TIME)
			timeout = expire_time - HP_EXPIRE_TIME;
		else if (expire_time > RP_EXPIRE_TIME)
			timeout = expire_time - RP_EXPIRE_TIME;
		if (timeout > 0) {
			expire_number++;
			if (timeout > max_expire_time) {
				max_expire_time = timeout;
				expire_index = i;
			}
			timeout = 0;
		}
	}
	if (expire_number <= 0)
		expire_index = -1;

	return expire_index;
}

static void mmc_mq_check_expire_time(
	struct mas_mmc_sched_ds_lld *ds_lld)
{
	enum mas_blk_mq_class_ioprio req_class_type = ds_lld->ioprio_class_type;

	if (req_class_type == MAS_MMC_MQ_CLASS_VIP ||
		req_class_type == MAS_MMC_MQ_CLASS_SYNC) {
		int expire_index = mmc_mq_be_expire_adjust(ds_lld);
		if (expire_index >= MAS_MMC_MQ_REQ_ASYNC_READ)
			ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_ASYNC;
		else if(expire_index >= MAS_MMC_MQ_REQ_SYNC_READ)
			ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_SYNC;
		else
			ds_lld->ioprio_class_type = req_class_type;
	}
	return;
}


static void mmc_mq_get_class_iopro(
	struct mas_mmc_sched_ds_lld *ds_lld)
{
	int i;
	s64 diff_ms;
	ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_MAX;

	/* First, go over the high priority queues */
	for(i = 0; i< (int)MAS_MMC_MQ_REQ_SYNC_READ; i++) {
		if (!list_empty(&ds_lld->mas_queues[i].fifo)) {
			if (mmc_mq_sync_req_pending(ds_lld) &&
				((ds_lld->reg_prio_starvation_counter >=
				ds_lld->reg_prio_starvation_limit)))
				ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_SYNC;
			else if (mmc_mq_async_req_pending(ds_lld) &&
				((ds_lld->low_prio_starvation_counter >=
				ds_lld->low_prio_starvation_limit)))
				ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_ASYNC;
			else
				ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_VIP;

			goto end;
		}

	}

	diff_ms = ktime_to_ms(ktime_sub(ktime_get(),
			ds_lld->rd_idle_data.last_rd_send_time));

	for(i = (int)MAS_MMC_MQ_REQ_SYNC_READ; i < (int)MAS_MMC_MQ_REQ_ASYNC_READ; i++) {
		if (list_empty(&ds_lld->mas_queues[i].fifo)) {
			if (ds_lld->rd_idle_data.read_begin_idling &&
				(diff_ms < ds_lld->rd_idle_data.read_idle_freq_ms))
				goto end;
		} else {
			if (mmc_mq_async_req_pending(ds_lld) &&
				((ds_lld->low_prio_starvation_counter >=
			     ds_lld->low_prio_starvation_limit)))
				ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_ASYNC;
			else
				ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_SYNC;

			goto end;
		}
	}

	for(i = (int)MAS_MMC_MQ_REQ_ASYNC_READ; i < (int)MAS_MMC_MQ_REQ_MAX_PRIO; i++) {
		if (!list_empty(&ds_lld->mas_queues[i].fifo))
			ds_lld->ioprio_class_type = MAS_MMC_MQ_CLASS_ASYNC;
	}

end:
	mmc_mq_check_expire_time(ds_lld);

	return;
}

void mmc_mq_dispatch_request(const struct request_queue *q)
{
	int ret;
	unsigned long flags;
	struct request *rq = NULL;
	struct blk_mq_hw_ctx *hctx = NULL;

	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;
	struct blk_mq_queue_data bd;

#ifdef CONFIG_MAS_IO_DEBUG_TRACE
	trace_mas_io(__func__, "sync-io", MAS_IO_TRACE_LEN);
#endif
	spin_lock_irqsave(&ds_lld->sync_disp_lock, flags);
	mmc_mq_get_class_iopro(ds_lld);

	rq = mmc_mq_pick_sync_rq(ds_lld);
	if (unlikely(!rq)) {
		spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);
		mmc_mq_run_delay_sync_list(q, 0);
		return;
	}
	list_del_init(&rq->queuelist);
	spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);

	hctx = blk_mq_map_queue(
		(struct request_queue *)q, (int)rq->mq_ctx->cpu);
	bd.rq = rq;
	ret = mmc_mq_queue_rq(rq, hctx, ds_lld, &bd, q);
	if (likely(ret == BLK_STS_OK)) {
		mmc_mq_dispatch_rw_cnt_set(ds_lld, rq);
		return;
	} else if (ret != BLK_STS_RESOURCE) {
		blk_mq_end_request(bd.rq, BLK_STS_IOERR);
		return;
	}
	__blk_mq_requeue_request(bd.rq);
#ifdef CONFIG_MAS_IO_DEBUG_TRACE
	trace_mas_io(__func__, "sync io requeue!", MAS_IO_TRACE_LEN);
#endif
	mmc_mq_requeue_sync_list(bd.rq, rq->q);
}


static void mmc_mq_sync_dispatch(const struct request_queue *q)
{
	unsigned long flags;
	int ret;
	struct request *rq = NULL;
	struct blk_mq_hw_ctx *hctx = NULL;
	struct blk_mq_queue_data bd;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;

	sched_ds->req_dispatch_work.last_enter_tm = ktime_get();

#ifdef CONFIG_MAS_IO_DEBUG_TRACE
	trace_mas_io(__func__, "-", MAS_IO_TRACE_LEN);
#endif
	do {
		spin_lock_irqsave(&ds_lld->sync_disp_lock, flags);
		mmc_mq_get_class_iopro(ds_lld);

		rq = mmc_mq_pick_sync_rq(ds_lld);
		if (unlikely(!rq)) {
			spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);
			break;
		}
		list_del_init(&rq->queuelist);
		spin_unlock_irqrestore(&ds_lld->sync_disp_lock, flags);

		hctx = blk_mq_map_queue(
			(struct request_queue *)q, (int)rq->mq_ctx->cpu);
		bd.rq = rq;
		ret = mmc_mq_queue_rq(rq, hctx, ds_lld, &bd, q);
		if (likely(ret == BLK_STS_OK)) {
			mmc_mq_dispatch_rw_cnt_set(ds_lld, rq);
			continue;
		} else if (ret == BLK_STS_RESOURCE) {
			__blk_mq_requeue_request(rq);
			mmc_mq_requeue_insert_list(rq, (struct request_queue *)q);
			mmc_mq_run_delay_sync_list(q, 0);
			break;
		}
		blk_mq_end_request(rq, BLK_STS_IOERR);
	} while (1);

	mmc_mq_run_delay_sync_list(q, 0);

	sched_ds->req_dispatch_work.last_exit_tm = ktime_get();
}

void mmc_mq_io_dispatch_work_fn(const struct work_struct *work)
{
	struct mas_mmc_mq_work *mq_work = container_of(
		work, struct mas_mmc_mq_work, io_dispatch_work.work);

	mmc_mq_sync_dispatch(mq_work->queue);
}

static enum mas_blk_mq_req_prio mmc_mq_get_bio_class_prio(
	struct mas_mmc_sched_ds_lld *ds_lld,
	struct bio *bio)
{
	if (!(bio->bi_opf & REQ_SYNC) && (bio_op(bio) == REQ_OP_WRITE))
		return MAS_MMC_MQ_REQ_SYNC_WRITE;
	else
		return MAS_MMC_MQ_REQ_MAX_PRIO;
}


static bool mmc_mq_async_sched_fifo_attempt_merge_bio(
	struct bio *bio, struct request_queue *q)
{
	struct request *rq = NULL;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;
	enum mas_blk_mq_req_prio bio_prio = mmc_mq_get_bio_class_prio(ds_lld, bio);

	if (bio_prio != MAS_MMC_MQ_REQ_SYNC_WRITE)
		return false;

	list_for_each_entry_reverse(rq, &ds_lld->mas_queues[bio_prio].fifo, queuelist) {
		int el_ret;

		if (!blk_rq_merge_ok(rq, bio))
			continue;
		el_ret = blk_try_merge(rq, bio);
		if (el_ret == ELEVATOR_BACK_MERGE) {
			if (bio_attempt_back_merge(q, rq, bio))
				return true;
		} else if (el_ret == ELEVATOR_FRONT_MERGE) {
			if (bio_attempt_front_merge(q, rq, bio))
				return true;
		}
	}

	return false;

}


static struct mas_mmc_mq_async_sched mas_mmc_mq_async_io_fifo_sched = {
	.type = MAS_MMC_MQ_SCHED_ASYNC_FIFO,
	.async_sched_attempt_merge_fn =
		mmc_mq_async_sched_fifo_attempt_merge_bio,
};

static inline struct blk_mq_tag_set *get_tag_set_from_ds_lld(
	struct mas_mmc_sched_ds_lld *ds_lld)
{
	return container_of(ds_lld->lld, struct blk_mq_tag_set, lld_func);
}

static void mmc_mq_complete_request(struct request *req)
{
	struct list_head *list = NULL;
	unsigned long flags;

	local_irq_save(flags);
	list = this_cpu_ptr(&blk_cpu_done);
	list_add_tail(&req->ipi_list, list);
	if (likely(list->next == &req->ipi_list))
		raise_softirq_irqoff(BLOCK_SOFTIRQ);
	local_irq_restore(flags);
}

void __mmc_mq_complete_request_remote(const void *data)
{
	struct request *rq = (struct request *)data;

	if (likely(rq->cmd_flags & (REQ_SYNC | REQ_FG))) {
		mas_blk_latency_req_check(rq, REQ_PROC_STAGE_DONE_SFTIRQ);
		rq->q->softirq_done_fn(rq);
	} else {
		mmc_mq_complete_request(rq);
	}
}

static void mmc_mq_rq_requeue(const struct request *req)
{
	blk_mq_requeue_request((struct request *)req, true);
	blk_mq_kick_requeue_list(req->q);
}

static void mmc_mq_io_guard_queue(const struct mas_mmc_mq_sched *sched_ds)
{
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;

	if ((atomic_read(&ds_lld->mq_prio_tag_used_cnt) +
		    atomic_read(&ds_lld->mq_tag_used_cnt)) &&
		!(mmc_mq_get_hp_inflt(ds_lld) +
			atomic_read(&ds_lld->sync_io_inflt_cnt)))
		mmc_mq_sync_dispatch(sched_ds->q);
}


void mmc_mq_io_guard_work_fn(void)
{
	struct mas_mmc_mq_sched *sched_ds = NULL;

	spin_lock(&io_guard_queue_list_lock);
	list_for_each_entry(
		sched_ds, &mas_io_guard_queue_list, io_guard_list_node) {
		spin_unlock(&io_guard_queue_list_lock);
		mmc_mq_io_guard_queue(sched_ds);
		spin_lock(&io_guard_queue_list_lock);
	}
	spin_unlock(&io_guard_queue_list_lock);

	queue_delayed_work(mas_blk_io_guard_wq, &mas_io_guard_work,
		msecs_to_jiffies(MAS_BLK_IO_GUARD_PERIOD_MS));
}

void mmc_mq_req_alloc_prep(
	struct blk_mq_alloc_data *data, unsigned long ioflag, bool fs_submit)
{
	data->io_flag = fs_submit ? ioflag : (ioflag | REQ_SYNC);
}

void mmc_mq_req_init(const struct blk_mq_ctx *ctx, struct request *rq)
{
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (unlikely(atomic_read(&rq->mas_req.req_used)))
		mas_blk_rdr_panic("Reinit unreleased request!");

	atomic_set(&rq->mas_req.req_used, 1);
#endif
	rq->mas_req.mq_ctx_generate = (struct blk_mq_ctx *)ctx;
	rq->mas_req.mas_featrue_flag = 0;
	INIT_LIST_HEAD(&rq->cmdq_list);
}

void mmc_mq_req_complete(
	struct request *rq, const struct request_queue *q, bool succ_done)
{
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;

	if (likely(succ_done)) {
		if (likely(!blk_rq_is_passthrough(rq)))
			mmc_mq_rq_inflt_update(rq, ds_lld, true);
	}

	if (rq->mq_ctx && (rq->mq_ctx->cpu != raw_smp_processor_id()) &&
		(cpu_online(rq->mq_ctx->cpu))) {
		rq->csd.func = __cfi__mmc_mq_complete_request_remote;
		rq->csd.info = rq;
		rq->csd.flags = 0;
		smp_call_function_single_async((int)rq->mq_ctx->cpu, &rq->csd);
	} else {
		mmc_mq_complete_request(rq);
	}
}

void mmc_mq_req_deinit(struct request *rq)
{
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	atomic_set(&rq->mas_req.req_used, 0);
#endif

}

void mmc_mq_req_insert(struct request *req, struct request_queue *q)
{
	mmc_mq_insert_list(req, q);
}

void mmc_mq_req_requeue(
	struct request *req, const struct request_queue *q)
{
	req->mas_req.protocol_nr = 0;
	mmc_mq_req_insert(req, (struct request_queue *)q);
}

void mmc_mq_req_timeout_handler(struct request *req)
{
	const struct blk_mq_ops *ops = req->q->mq_ops;
	enum blk_eh_timer_return proc_ret = BLK_EH_RESET_TIMER;
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(req->q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = sched_ds->sched_ds_lld;

	if (!test_bit(REQ_ATOM_STARTED, &req->atomic_flags))
		return;

	if (!blk_rq_is_passthrough(req))
		mmc_mq_rq_inflt_update(req, ds_lld, true);

	if (ops->timeout)
		proc_ret = ops->timeout(req, false);

	switch (proc_ret) {
	/* Protocol layer has dealt with the timeout event */
	case BLK_EH_HANDLED:
#ifdef CONFIG_WBT
		blk_mq_stat_add(req);
#endif
		mmc_mq_req_complete(req, req->q, false);
		break;
	/* Protocol layer need more time to process the request */
	case BLK_EH_RESET_TIMER:
		mmc_mq_rq_inflt_add(req, ds_lld);
		blk_add_timer(req);
		blk_clear_rq_complete(req);
		break;
	/* Protocol layer would do error-handling itself */
	case BLK_EH_NOT_HANDLED:
		break;
	/* Protocol layer expect block layer can requeue the request */
	case BLK_EH_REQUEUE:
		mmc_mq_rq_requeue(req);
		break;
	default:
		break;
	}
}

void mmc_mq_ctx_put(void)
{
#ifdef CONFIG_PREEMPT_COUNT
	barrier();
	preempt_count_dec();
#endif
}

void mmc_mq_hctx_get_by_req(
	const struct request *rq, struct blk_mq_hw_ctx **hctx)
{
	struct request_queue *q = rq->q;

	*hctx = blk_mq_map_queue(q, rq->mas_req.mq_ctx_generate->cpu);
}

void mmc_mq_exec_queue(const struct request_queue *q)
{
	mmc_mq_sync_dispatch(q);
}

void mmc_mq_run_hw_queue(const struct request_queue *q)
{
	mmc_mq_run_sync_list(q);
}

void mmc_mq_run_requeue(const struct request_queue *q)
{
	mmc_mq_run_sync_list(q);
}

void mmc_mq_poll_enable(bool *enable)
{
	unsigned int i;
	unsigned int count = 0;

	for (i = 0; i < num_possible_cpus(); i++)
		if (cpu_online((int)i))
			count++;

	*enable = count > 1;
}


void mmc_mq_status_dump(const struct request_queue *q, enum blk_dump_scene s)
{
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;
	char *prefix = mas_blk_prefix_str(s);

	if (!sched_ds || !(sched_ds->sched_ds_lld))
		return;

	ds_lld = sched_ds->sched_ds_lld;
	pr_err("%s: vip_wait_cnt: %d\n",
		prefix, atomic_read(&ds_lld->vip_wait_cnt));
	pr_err("%s: h_tag_used_cnt: %d tag_used_cnt: %d r_tag_used_cnt: %d\n",
		prefix, atomic_read(&ds_lld->mq_prio_tag_used_cnt),
		atomic_read(&ds_lld->mq_tag_used_cnt),
		atomic_read(&ds_lld->mq_resrvd_tag_used_cnt));
	pr_err("%s: fg_inflt: %d, vip_inflt: %d, s_inflt: %d a_inflt: %d, cp_inflt: %d\n",
		prefix, atomic_read(&ds_lld->fg_io_inflt_cnt),
		atomic_read(&ds_lld->vip_io_inflt_cnt),
		atomic_read(&ds_lld->sync_io_inflt_cnt),
		atomic_read(&ds_lld->async_io_inflt_cnt),
		atomic_read(&ds_lld->cp_io_inflt_cnt));
}

static void mmc_mq_tag_used_cnt_init(struct mas_mmc_sched_ds_lld *ds_lld)
{
	atomic_set(&ds_lld->mq_tag_used_cnt, 0);
	atomic_set(&ds_lld->mq_resrvd_tag_used_cnt, 0);
	atomic_set(&ds_lld->mq_prio_tag_used_cnt, 0);
	atomic_set(&ds_lld->vip_wait_cnt, 0);
}

static void mmc_mq_inflt_cnt_init(struct mas_mmc_sched_ds_lld *ds_lld)
{
	atomic_set(&ds_lld->cp_io_inflt_cnt, 0);
	atomic_set(&ds_lld->async_io_inflt_cnt, 0);
	atomic_set(&ds_lld->fg_io_inflt_cnt, 0);
	atomic_set(&ds_lld->vip_io_inflt_cnt, 0);
	atomic_set(&ds_lld->sync_io_inflt_cnt, 0);
}

static int mmc_mq_workqueue_init(void)
{
	mas_blk_mq_req_disp_wq =
		alloc_workqueue("sync_dispatch", WQ_HIGHPRI, 0);
	if (!mas_blk_mq_req_disp_wq) {
		pr_err("%s %d Failed to alloc sync_dispatch_workqueue\n",
			__func__, __LINE__);
		return -ENOMEM;
	}

	mas_blk_io_guard_wq =
		alloc_workqueue("io_guard", WQ_UNBOUND | WQ_FREEZABLE, 0);
	if (!mas_blk_io_guard_wq)
		goto destroy_req_disp_wkq;

	INIT_DELAYED_WORK(&mas_io_guard_work, __cfi_mmc_mq_io_guard_work_fn);

	return 0;

destroy_req_disp_wkq:
	destroy_workqueue(mas_blk_mq_req_disp_wq);
	pr_err("%s init Failed!\n", __func__);
	return -ENOMEM;
}

static void mmc_mq_dispatch_list_init(
	struct mas_mmc_sched_ds_lld *ds_lld)
{
	int i;
	for (i = 0; i < MAS_MMC_MQ_REQ_MAX_PRIO; i++) {
		INIT_LIST_HEAD(&ds_lld->mas_queues[i].fifo);
		ds_lld->mas_queues[i].disp_quantum = mas_queues_def[i].quantum;
	}

	ds_lld->reg_prio_starvation_limit =
			MAS_REG_STARVATION_TOLLERANCE;
	ds_lld->low_prio_starvation_limit =
			MAS_LOW_STARVATION_TOLLERANCE;

	ds_lld->reg_prio_starvation_counter = 0;
	ds_lld->low_prio_starvation_counter = 0;

	ds_lld->rd_idle_data.read_idle_freq_ms = MAS_READ_IDLE_TIME_MSEC;
	ds_lld->rd_idle_data.read_begin_idling = false;
	ds_lld->rd_idle_data.last_rd_insert_time = ktime_set(0, 0);
	ds_lld->rd_idle_data.last_rd_send_time = ktime_set(0, 0);

	spin_lock_init(&ds_lld->sync_disp_lock);

	atomic_set(&ds_lld->hp_io_list_cnt, 0);
	atomic_set(&ds_lld->sync_io_list_cnt, 0);
	atomic_set(&ds_lld->async_io_list_cnt, 0);
}

/* Initial sched_ds_lld per lld */
static struct mas_mmc_sched_ds_lld *mmc_mq_sched_ds_lld_init(
	const struct request_queue *q)
{
	struct blk_dev_lld *lld = mas_blk_get_lld((struct request_queue *)q);
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;

	if (lld->sched_ds_lld_inited) {
		ds_lld = (struct mas_mmc_sched_ds_lld *)lld->sched_ds_lld;
		atomic_inc(&ds_lld->ref_cnt);
		return (struct mas_mmc_sched_ds_lld *)lld->sched_ds_lld;
	}

	ds_lld = kzalloc(sizeof(struct mas_mmc_sched_ds_lld), GFP_KERNEL);
	if (unlikely(!ds_lld))
		return NULL;

	atomic_set(&ds_lld->ref_cnt, 1);
	mmc_mq_tag_used_cnt_init(ds_lld);
	mmc_mq_dispatch_list_init(ds_lld);
	mmc_mq_inflt_cnt_init(ds_lld);

	if (mmc_mq_workqueue_init())
		goto free_sched_ds_lld;

	ds_lld->lld = lld;
	lld->sched_ds_lld = ds_lld;
	lld->sched_ds_lld_inited = true;

	if (lld->type == BLK_LLD_TAGSET_BASE)
		mas_blk_flush_list_register(&lld->lld_list);

	return ds_lld;

free_sched_ds_lld:
	kfree(ds_lld);
	return NULL;
}

static void mmc_mq_sched_ds_lld_exit(const struct request_queue *q)
{
	struct blk_dev_lld *lld = mas_blk_get_lld((struct request_queue *)q);
	struct mas_mmc_sched_ds_lld *ds_lld = lld->sched_ds_lld;

	if (lld->type == BLK_LLD_TAGSET_BASE)
		mas_blk_flush_list_unregister(&lld->lld_list);

	if (lld->sched_ds_lld_inited &&
		atomic_dec_and_test(&ds_lld->ref_cnt)) {
		lld->sched_ds_lld_inited = false;
		destroy_workqueue(mas_blk_mq_req_disp_wq);
		destroy_workqueue(mas_blk_io_guard_wq);
		kfree(lld->sched_ds_lld);
		lld->sched_ds_lld = NULL;
	}
}

int mmc_mq_iosched_init(struct request_queue *q)
{
	struct mas_mmc_sched_ds_lld *ds_lld = NULL;
	struct mas_mmc_mq_sched *sched_ds =
		kzalloc(sizeof(struct mas_mmc_mq_sched), GFP_KERNEL);

	if (!sched_ds) {
		pr_err("%s %d Failed to alloc sched_ds!\n", __func__, __LINE__);
		return -ENOMEM;
	}

	ds_lld = mmc_mq_sched_ds_lld_init(q);
	if (!ds_lld)
		goto free_sched_ds;

	if (!q->mas_queue_ops || !q->mas_queue_ops->scheduler_priv)
		goto sched_ds_lld_exit;

	INIT_DELAYED_WORK(&sched_ds->req_dispatch_work.io_dispatch_work,
		__cfi_mmc_mq_io_dispatch_work_fn);
	sched_ds->req_dispatch_work.queue = q;

	sched_ds->q = q;
	sched_ds->sched_ds_lld = ds_lld;
	q->mas_queue.cust_queuedata = (void *)sched_ds;

	spin_lock(&io_guard_queue_list_lock);
	list_add_tail(&sched_ds->io_guard_list_node, &mas_io_guard_queue_list);
	spin_unlock(&io_guard_queue_list_lock);

	queue_delayed_work(mas_blk_io_guard_wq, &mas_io_guard_work,
		msecs_to_jiffies(MAS_BLK_IO_GUARD_PERIOD_MS));

	blk_queue_make_request(q, __cfi_mmc_mq_make_request);

	return 0;

sched_ds_lld_exit:
	mmc_mq_sched_ds_lld_exit(q);
free_sched_ds:
	kfree(sched_ds);
	return -ENOMEM;
}

void mmc_mq_iosched_exit(struct request_queue *q)
{
	struct mas_mmc_mq_sched *sched_ds =
		(struct mas_mmc_mq_sched *)(q->mas_queue.cust_queuedata);

	if (!sched_ds)
		return;

	cancel_delayed_work_sync(&mas_io_guard_work);

	spin_lock(&io_guard_queue_list_lock);
	list_del_init(&sched_ds->io_guard_list_node);
	spin_unlock(&io_guard_queue_list_lock);

	/* mmc_mq_async_sched_exit_fn */
	mmc_mq_sched_ds_lld_exit(q);

	kfree(q->mas_queue.cust_queuedata);
	q->mas_queue.cust_queuedata = NULL;
}

void blk_mq_tagset_mmc_mq_iosched_enable(
	struct blk_mq_tag_set *tag_set, int enable)
{
	if (enable)
		tag_set->lld_func.features |= BLK_LLD_IOSCHED_MMC_MQ;
	else
		tag_set->lld_func.features &= ~BLK_LLD_IOSCHED_MMC_MQ;
}

struct mas_mmc_mq_priv mas_mmc_mq = {
	.async_io_sched_strategy = &mas_mmc_mq_async_io_fifo_sched,
};


