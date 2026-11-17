/*
 * cmdq code in card level
 * Copyright (c) 2015-2020 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/ioprio.h>
#include <linux/blkdev.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <trace/events/mmc.h>
#include <linux/mmc/cmdq_hci.h>
#include <linux/reboot.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/sched/rt.h>
#include <uapi/linux/sched/types.h>
#include <linux/blk-mq.h>

#include "mmc_zodiac_card.h"
#include "card.h"
#include "core.h"

#define CMDQ_ASYNC_MAX_TAG 1

struct request * mmc_blk_cmdq_mq_find_req(struct request_queue *q, int tag)
{
	struct list_head *tmp = NULL;
	struct list_head *n = NULL;
	struct request *req = NULL;
	struct mmc_queue_req *mq_rq = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;

	list_for_each_safe(tmp, n, &q->tag_busy_list) {
		req = cmdq_list_entry_rq(tmp);
		if (req && req->special) {
			mq_rq = req->special;
			cmdq_req = &mq_rq->mmc_cmdq_req;
			if (cmdq_req->tag == tag) {
				return req;
			}
		}
	}
	return NULL;
}


/* invoked by block layer in softirq context */
void mmc_blk_cmdq_mq_complete_rq(struct request *rq)
{
	struct mmc_queue_req *mq_rq = rq->special;
	struct mmc_request *mrq = &mq_rq->mmc_cmdq_req.mrq;
	struct mmc_host *host = mrq->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct mmc_cmdq_req *cmdq_req = &mq_rq->mmc_cmdq_req;
	struct mmc_queue *mq = (struct mmc_queue *)rq->q->queuedata;
	int err = 0;
	bool curr_req_clear = false;
	struct request_queue *q = rq->q;
	unsigned long flags;

	if (mrq->cmd && mrq->cmd->error)
		err = mrq->cmd->error;
	else if (mrq->data && mrq->data->error)
		err = mrq->data->error;

	if (err || cmdq_req->resp_err) {
		pr_err("%s: request with req: 0x%x, tag: %d, cmdq_tag: %d, flags: 0x%x,"
			"curr_state:0x%lx, active reqs:0x%lx timed out\n",
			__func__, rq, rq->tag, cmdq_req->tag, rq->cmd_flags,
			ctx_info->curr_state, ctx_info->active_reqs);

		pr_err("%s: %s: txfr error:%d/resp_err:%d\n",
			mmc_hostname(mrq->host), __func__, err,
			cmdq_req->resp_err);

		if (test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state)) {
			pr_err("%s: CQ in error state, ending current req: %d\n", __func__, err);
		} else {
			spin_lock_irqsave(&ctx_info->cmdq_ctx_lock, flags);
			set_bit(CMDQ_STATE_ERR, &ctx_info->curr_state);
			spin_unlock_irqrestore(&ctx_info->cmdq_ctx_lock, flags);
			if (host->err_mrq != NULL)
				rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
			host->err_mrq = mrq;
			schedule_work(&mq->cmdq_err_work);
		}
		goto out;
	}

	/*
	 * In case of error CMDQ is expected to be either in halted
	 * or disable state so cannot receive any completion of
	 * other requests.
	 */
	spin_lock_irqsave(&ctx_info->cmdq_ctx_lock, flags);
	if (test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state)) {
		spin_unlock_irqrestore(&ctx_info->cmdq_ctx_lock, flags);
		pr_err("%s: softirq may come from different cpu cluster, curr_state:0x%lx\n",
			__func__, ctx_info->curr_state);
		WARN_ON(1);
		return;
	}

	/* clear pending request */
	if (!test_bit(cmdq_req->tag, &ctx_info->active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);

	curr_req_clear = true;
	spin_lock_irqsave(q->queue_lock, flags);
	list_del_init(&rq->cmdq_list);
	spin_unlock_irqrestore(q->queue_lock, flags);

	if (cmdq_req->cmdq_req_flags & DCMD) {
		clear_bit_unlock(cmdq_req->tag, &ctx_info->active_reqs);
		clear_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
		blk_mq_end_request(rq, err);
	} else {
		mmc_cmdq_post_req(host, mrq, err);
		if (!test_and_clear_bit(cmdq_req->tag, &ctx_info->data_active_reqs))
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);

		clear_bit_unlock(cmdq_req->tag, &ctx_info->active_reqs);
		if (mrq->data) {
			if (blk_update_request(rq, BLK_STS_OK, cmdq_req->data.bytes_xfered)) {
				blk_mq_requeue_request(rq, true);
			} else {
				__blk_mq_end_request(rq, BLK_STS_OK);
			}
		} else {
			blk_mq_end_request(rq, BLK_STS_OK);
		}

	}
	mmc_release_host(host);
	spin_unlock_irqrestore(&ctx_info->cmdq_ctx_lock, flags);

out:

	spin_lock_irqsave(&ctx_info->cmdq_ctx_lock, flags);
	if (!test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state))
		blk_mq_run_hw_queues(q, true);
	else if (curr_req_clear)
		pr_err("%s: CMDQ_STATE_ERR bit is set after req is clear\n", __func__);
	spin_unlock_irqrestore(&ctx_info->cmdq_ctx_lock, flags);

	if (!ctx_info->active_reqs)
		wake_up_interruptible(&host->cmdq_ctx.queue_empty_wq);

	if (blk_queue_quiesced(mq->queue) && !ctx_info->active_reqs)
		complete(&mq->cmdq_shutdown_complete);
}


/* internal funtion ,needn't null check */
int mmc_mq_cmdq_init(struct mmc_queue *mq, struct mmc_card *card)
{
	struct request_queue *q = mq->queue;
	card->cmdq_init = false;

	spin_lock_init(&card->host->cmdq_ctx.cmdq_ctx_lock);
	mutex_init(&card->host->cmdq_ctx.cmdq_queue_rq_mutex);
	init_waitqueue_head(&card->host->cmdq_ctx.queue_empty_wq);
	INIT_LIST_HEAD(&q->tag_busy_list);
	INIT_WORK(&mq->cmdq_err_work, mmc_cmdq_error_work);
	init_completion(&mq->cmdq_shutdown_complete);
	blk_queue_rq_timeout(mq->queue, 29 * HZ); /* blk request IO 29s */

	card->cmdq_init = true;
	pr_err("%s: CMDQ MQ init end!\n", mmc_hostname(card->host));

	return 0;
}

static enum blk_eh_timer_return mmc_cmdq_mq_timed_out(struct request *req,
						 bool reserved)
{
	struct request_queue *q = req->q;
	unsigned long flags;
	int ret;
	spin_lock_irqsave(q->queue_lock, flags);
	ret = mmc_blk_cmdq_req_timed_out(req);
	spin_unlock_irqrestore(q->queue_lock, flags);

	return ret;
}

enum mmc_issue_type mmc_issue_type(struct request *req)
{
	if (req_op(req) == REQ_OP_DISCARD || req_op(req) == REQ_OP_SECURE_ERASE ||
		req_op(req) == REQ_OP_FLUSH)
		return MMC_ISSUE_DCMD;
	if (req_op(req) == REQ_OP_READ || req_op(req) == REQ_OP_WRITE)
		return MMC_ISSUE_ASYNC;
	return MMC_ISSUE_SYNC;
}

int mmc_blk_cmdq_wait_for_rw(struct mmc_host *host, struct request *req)
{
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	unsigned long timeout = 100;

	/* wait for cmdq req handle done. */
	while (ctx->active_reqs) {
		if (!timeout) {
			pr_err("%s: wait cmdq r/w complete reqs timeout !\n", __func__);
			return -ETIMEDOUT;
		}
		timeout--;
		mdelay(1);
	}
	return BLK_STS_OK;
}


static blk_status_t mmc_cmdq_mq_check_state(struct mmc_host *host, struct request *req)
{
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	enum mmc_issue_type issue_type = mmc_issue_type(req);
	struct request_queue *q = req->q;

	/*
	 * All of the following conditions are false:
	 * 1. If the request is flush/discard then there shouldn't
	 *    be any other direct command active.
	 * 2. There is a request pending in the block layer queue
	 *    to be processed.
	 * 3. cmdq state should be unhalted.
	 * 4. card state shouldn't be in suspend.
	 * 5. cmdq state shouldn't be in error state.
	 */

	if (test_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx->curr_state) ||
		test_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx->curr_state) ||
		mmc_host_halt(host) ||
		mmc_card_suspended(host->card) ||
		test_bit(CMDQ_STATE_ERR, &ctx->curr_state))
		return BLK_STS_RESOURCE;

	if (issue_type == MMC_ISSUE_DCMD &&
		test_and_set_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx->curr_state))
		return BLK_STS_RESOURCE;

	spin_lock_irq(q->queue_lock);
	list_del_init(&req->cmdq_list);
	list_add(&req->cmdq_list, &q->tag_busy_list);
	spin_unlock_irq(q->queue_lock);

	return BLK_STS_OK;
}

int mmc_cmdq_get_tag(struct mmc_card *card, struct request *req)
{
	/* one slot is reserved for dcmd requests */
	int max_depth = card->ext_csd.cmdq_depth - 1;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	int tag;

	if (!rq_is_sync(req) && max_depth > CMDQ_ASYNC_MAX_TAG)
		max_depth -= CMDQ_ASYNC_MAX_TAG;

	tag = find_first_zero_bit(&ctx->active_reqs, max_depth);
	if (tag < 0 || tag >= max_depth)
		return -1;
	if (test_and_set_bit_lock(tag, &ctx->active_reqs))
		return -1;

	return tag;
}


static int  mmc_cmdq_mq_clear_flags(struct mmc_host *host, struct request *req, int err)
{
	int ret = err;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	enum mmc_issue_type issue_type = mmc_issue_type(req);

	spin_lock_irq(req->q->queue_lock);
	list_del_init(&req->cmdq_list);
	spin_unlock_irq(req->q->queue_lock);

	if (issue_type == MMC_ISSUE_DCMD && err == BLK_STS_RESOURCE)
		clear_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx->curr_state);

	if (err == -EHOSTDOWN)
		ret = BLK_STS_RESOURCE;

	return ret;
}

static blk_status_t mmc_cmdq_mq_queue_rq(struct blk_mq_hw_ctx *hctx,
				    const struct blk_mq_queue_data *bd)
{
	struct request *req = bd->rq;
	struct request_queue *q = req->q;
	struct mmc_queue *mq = q->queuedata;
	int ret;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;

	mutex_lock(&ctx_info->cmdq_queue_rq_mutex);
	if (mmc_card_removed(mq->card)) {
		req->rq_flags |= RQF_QUIET;
		mutex_unlock(&ctx_info->cmdq_queue_rq_mutex);
		return BLK_STS_IOERR;
	}

	ret = mmc_cmdq_mq_check_state(host, req);
	if (ret) {
		mutex_unlock(&ctx_info->cmdq_queue_rq_mutex);
		return BLK_STS_RESOURCE;
	}

	blk_mq_start_request(req);

	ret = mmc_blk_cmdq_issue_rq( mq, req);
	if (ret)
		ret = mmc_cmdq_mq_clear_flags(host, req, ret);

	mutex_unlock(&ctx_info->cmdq_queue_rq_mutex);
	return ret;
}

static const struct blk_mq_ops mmc_cmdq_ops = {
	.queue_rq	= mmc_cmdq_mq_queue_rq,
	.init_request	= mmc_mq_init_request,
	.exit_request	= mmc_mq_exit_request,
	.complete	= mmc_blk_cmdq_mq_complete_rq,
	.timeout	= mmc_cmdq_mq_timed_out,
};

/* internal funtion ,needn't null check */
int mmc_cmdq_mq_init_queue(struct mmc_queue *mq, struct mmc_card *card,
	spinlock_t *lock)
{
	int ret;
	int q_depth = 3*(card->ext_csd.cmdq_depth - 1);

	ret = mmc_mq_init_queue(mq, q_depth, &mmc_cmdq_ops, lock);
	if (ret)
		return ret;

	mmc_blk_cmdq_setup_queue(mq, card);

	mmc_mq_cmdq_init(mq, card);
	return ret;
}

