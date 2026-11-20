/*
 * cmdq code in card level
 * Copyright (c) 2015-2019 The Linux Foundation. All rights reserved.
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
#include <linux/mmc/cmdq_hci.h>
#include <linux/reboot.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/sched/rt.h>

#ifdef CONFIG_MMC_MQ_CQ_HCI
#include <linux/mutex.h>
#endif

#ifdef CONFIG_EMMC_FAULT_INJECT
#include <linux/mmc/emmc_fault_inject.h>
#endif

#ifdef CONFIG_HUAWEI_DSM_IOMT_EMMC_HOST
#include <linux/iomt_host/dsm_iomt_emmc_host.h>
#endif

#include <uapi/linux/sched/types.h>
#include <trace/events/mmc.h>

#include "mmc_zodiac_card.h"
#include "card.h"

#ifdef CONFIG_HUAWEI_EMMC_DSM
void sdhci_dsm_report(struct mmc_host *host, struct mmc_request *mrq);
#endif

#define MMC_SECTOR_TO_BLOCK 9
#define MMC_BLOCK_TO_BYTE 9
#define MMC_BYTE_TO_BLOCK 512

#define INAND_CMD38_ARG_EXT_CSD 113
#define INAND_CMD38_ARG_ERASE 0x00
#define INAND_CMD38_ARG_TRIM 0x01

#define INAND_CMD38_ARG_SECERASE 0x80
#define INAND_CMD38_ARG_SECTRIM1 0x81
#define INAND_CMD38_ARG_SECTRIM2 0x88
#ifdef CONFIG_MMC_MQ_CQ_HCI
#define MMC_CMDQ_MQ_DCMD_TAG 31
#endif

static int mmc_blk_cmdq_switch(struct mmc_card *card, struct mmc_blk_data *md, bool enable)
{
	int ret = 0;
	bool cmdq_mode = !!mmc_card_cmdq(card);
	struct mmc_host *host = card->host;

	if (!card->ext_csd.cmdq_mode_en || (enable && md &&
		!(md->flags & MMC_BLK_CMD_QUEUE)) || (cmdq_mode == enable))
		return 0;

	if (host->cmdq_ops) {
		if (enable) {
			ret = mmc_set_blocklen(card, MMC_CARD_CMDQ_BLK_SIZE);
			if (ret) {
				pr_err("%s: failed to set block-size to 512\n", __func__);
				rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
			}

			ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_CMDQ_MODE, enable, card->ext_csd.generic_cmd6_time);
			if (ret) {
				pr_err("cmdq mode %sable failed %d\n", enable ? "en" : "dis", ret);
				goto out;
			}
			mmc_card_set_cmdq(card);

			/* enable host controller command queue engine */
			ret = host->cmdq_ops->enable(card->host);
			if (ret)
				pr_err("failed to enable host controller cqe %d\n", ret);
		}

		if (ret || !enable) {
			ret = host->cmdq_ops->disable(card->host, true);
			if (ret)
				pr_err("failed to disable host controller cqe %d\n", ret);
			/* disable CQ mode in card */
			ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_CMDQ_MODE, 0, card->ext_csd.generic_cmd6_time);
			if (ret) {
				pr_err("cmdq mode %sable failed %d\n", enable ? "en" : "dis", ret);
				rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
			}
			mmc_card_clr_cmdq(card);
		}
	} else {
		pr_err("%s: No cmdq ops defined!!!\n", __func__);
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
	}

out:
	return ret;
}

int mmc_blk_cmdq_hangup(struct mmc_card *card)
{
	struct mmc_cmdq_context_info *ctx_info = NULL;
	unsigned long timeout = (1 * 60 * 1000); /* wait cmdq complete 60ms */
	int ret = 0;

	if (card && card->ext_csd.cmdq_mode_en) {
		ctx_info = &card->host->cmdq_ctx;
		set_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx_info->curr_state);
		/* wait for cmdq req handle done. */
		while (ctx_info->active_reqs) {
			if (!timeout) {
				pr_err("%s: wait cmdq complete reqs timeout !\n", __func__);
				return -ETIMEDOUT;
			}
			timeout--;
			mdelay(1);
		}
		/* disable CQ mode for ioctl */
		ret = mmc_blk_cmdq_switch(card, NULL, false);
	}
	return ret;
}
EXPORT_SYMBOL(mmc_blk_cmdq_hangup);

void mmc_blk_cmdq_restore(struct mmc_card *card)
{
	struct mmc_cmdq_context_info *ctx_info = NULL;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	struct mmc_queue *mq = NULL;
	struct mmc_blk_data *md = NULL;
#endif

	if (card && card->ext_csd.cmdq_mode_en) {
		ctx_info = &card->host->cmdq_ctx;
		clear_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx_info->curr_state);
#ifdef CONFIG_MMC_MQ_CQ_HCI
		md = dev_get_drvdata(&card->dev);
		mq = &md->queue;
		blk_mq_run_hw_queues(mq->queue, true);
#else
		wake_up(&card->host->cmdq_ctx.wait);
#endif
	}
}
EXPORT_SYMBOL(mmc_blk_cmdq_restore);

/*
 * mmc_blk_cmdq_halt - wait for dbr finished and halt cqe;
 * @card: mmc card;
 * wait for dbr finished and halt cqe,then we can issue a
 * legacy command like flush;
 * emmc internal function, don't need null check
 */
int mmc_blk_cmdq_halt(struct mmc_card *card)
{
	struct mmc_cmdq_context_info *ctx_info = NULL;
	int ret = 0;
	struct mmc_host *host = card->host;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	struct mmc_blk_data *md = NULL;
	struct mmc_queue *mq = NULL;
#endif
	if (card->ext_csd.cmdq_mode_en && (!!mmc_card_cmdq(card))) {
		ctx_info = &card->host->cmdq_ctx;
		/* make sure blk sends no more request */
		set_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx_info->curr_state);

		/*
		 * make sure there are no tasks transfer by cqe and
		 * cqe halt success
		 */
		ret = host->cmdq_ops->clear_and_halt(card->host);
		if (!ret) {
			mmc_host_set_halt(host);
		} else {
			pr_err("%s: halt fail, ret = %d\n", __func__, ret);
			clear_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx_info->curr_state);
#ifdef CONFIG_MMC_MQ_CQ_HCI
			md = dev_get_drvdata(&card->dev);
			mq = &md->queue;
			blk_mq_run_hw_queues(mq->queue, true);
#else
			wake_up(&ctx_info->wait);
#endif
		}
	}
	return ret;
}

/* mmc internal function, don't need null check */
void mmc_blk_cmdq_dishalt(struct mmc_card *card)
{
	struct mmc_cmdq_context_info *ctx_info = NULL;
	struct mmc_host *host = card->host;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	struct mmc_blk_data *md = NULL;
	struct mmc_queue *mq = NULL;
#endif

	if (card->ext_csd.cmdq_mode_en) {
		ctx_info = &card->host->cmdq_ctx;
		host->cmdq_ops->halt(host, (bool)false);
		clear_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx_info->curr_state);
		mmc_host_clr_halt(host);
#ifdef CONFIG_MMC_MQ_CQ_HCI
		md = dev_get_drvdata(&card->dev);
		mq = &md->queue;
		blk_mq_run_hw_queues(mq->queue, true);
#else
		wake_up(&ctx_info->wait);
#endif
	}
}

static int mmc_blk_cmdq_start_req(struct mmc_host *host, struct mmc_cmdq_req *cmdq_req)
{
	struct mmc_request *mrq = &cmdq_req->mrq;

	mrq->done = mmc_blk_cmdq_req_done;
	return mmc_cmdq_start_req(host, cmdq_req);
}

/* prepare for non-data commands */
static struct mmc_cmdq_req *mmc_cmdq_prep_dcmd(struct mmc_queue_req *mqrq, struct mmc_queue *mq)
{
	struct request *req = mmc_queue_req_to_req(mqrq);
	struct mmc_cmdq_req *cmdq_req = &mqrq->mmc_cmdq_req;

	memset(&mqrq->mmc_cmdq_req, 0, sizeof(struct mmc_cmdq_req));

	cmdq_req->mrq.data = NULL;
	cmdq_req->cmd_flags = req->cmd_flags;
	cmdq_req->mrq.req = req;
	req->special = mqrq;
	cmdq_req->cmdq_req_flags |= DCMD;
	cmdq_req->mrq.cmdq_req = cmdq_req;

	return &mqrq->mmc_cmdq_req;
}

static struct mmc_cmdq_req *mmc_blk_cmdq_rw_prep(
	struct mmc_queue_req *mqrq, struct mmc_queue *mq)
{
	struct mmc_card *card = mq->card;
	struct request *req = mmc_queue_req_to_req(mqrq);
	struct mmc_blk_data *md = mq->blkdata;
	bool do_rel_wr = mmc_req_rel_wr(req) && (md->flags & MMC_BLK_REL_WR);
	bool do_data_tag = false;
	bool read_dir = (rq_data_dir(req) == READ);
	bool prio = (IOPRIO_PRIO_CLASS(req_get_ioprio(req)) == IOPRIO_CLASS_RT);
	struct mmc_cmdq_req *cmdq_rq = &mqrq->mmc_cmdq_req;
	u32 map_sg_len;
	unsigned int i = 0;
	int tag;

#ifdef CONFIG_MMC_MQ_CQ_HCI
	tag = cmdq_rq->tag;
#else
	tag = req->tag;
#endif
	memset(&mqrq->mmc_cmdq_req, 0, sizeof(struct mmc_cmdq_req));

	cmdq_rq->tag = tag;
	if (read_dir) {
		cmdq_rq->cmdq_req_flags |= DIR;
		cmdq_rq->data.flags = MMC_DATA_READ;
	} else {
		cmdq_rq->data.flags = MMC_DATA_WRITE;
	}
	if (prio)
		cmdq_rq->cmdq_req_flags |= PRIO;

	if (do_rel_wr)
		cmdq_rq->cmdq_req_flags |= REL_WR;

	cmdq_rq->data.blocks = blk_rq_sectors(req);
	cmdq_rq->blk_addr = blk_rq_pos(req);
	cmdq_rq->data.blksz = MMC_CARD_CMDQ_BLK_SIZE;
	cmdq_rq->data.bytes_xfered = 0;

	mmc_set_data_timeout(&cmdq_rq->data, card);

	do_data_tag = (card->ext_csd.data_tag_unit_size) &&
		(req->cmd_flags & REQ_META) &&
		(rq_data_dir(req) == WRITE) &&
		((cmdq_rq->data.blocks * cmdq_rq->data.blksz) >=
		card->ext_csd.data_tag_unit_size);
	if (do_data_tag)
		cmdq_rq->cmdq_req_flags |= DAT_TAG;
	cmdq_rq->data.sg = mqrq->sg;
	cmdq_rq->data.sg_len = mmc_queue_map_sg(mq, mqrq);
	map_sg_len = cmdq_rq->data.sg_len;

	/* Adjust the sg list so it is the same size as the request. */
	if (cmdq_rq->data.blocks > card->host->max_blk_count)
		cmdq_rq->data.blocks = card->host->max_blk_count;

	if (cmdq_rq->data.blocks != blk_rq_sectors(req)) {
		int data_size = cmdq_rq->data.blocks << MMC_BLOCK_TO_BYTE;
		struct scatterlist *sg = NULL;

		for_each_sg(cmdq_rq->data.sg, sg, cmdq_rq->data.sg_len, i) {
			data_size -= sg->length;
			if (data_size <= 0) {
				sg->length += data_size;
				i++;
				break;
			}
		}
		cmdq_rq->data.sg_len = i;
	}

	mqrq->mmc_cmdq_req.cmd_flags = req->cmd_flags;
	mqrq->mmc_cmdq_req.mrq.req = req;
	mqrq->mmc_cmdq_req.mrq.cmdq_req = &mqrq->mmc_cmdq_req;
	mqrq->mmc_cmdq_req.mrq.data = &mqrq->mmc_cmdq_req.data;
	/* mrq.cmd: no opcode, just for record error */
	mqrq->mmc_cmdq_req.mrq.cmd = &mqrq->mmc_cmdq_req.cmd;
	req->special = mqrq;

	pr_debug("%s: %s: mrq: 0x%pK req: 0x%pK mqrq: 0x%pK "
		"bytes to xf: %u mmc_cmdq_req: 0x%pK card-addr: 0x%08x "
		"data_sg_len: %u map_sg_len: %u dir(r-1/w-0): %d\n",
		mmc_hostname(card->host), __func__, &mqrq->mmc_cmdq_req.mrq,
		req, mqrq, (cmdq_rq->data.blocks * cmdq_rq->data.blksz),
		cmdq_rq, cmdq_rq->blk_addr, cmdq_rq->data.sg_len, map_sg_len,
		(cmdq_rq->cmdq_req_flags & DIR) ? 1 : 0);

#ifdef CONFIG_HUAWEI_DSM_IOMT_EMMC_HOST
	if (card->host->iomt_host_info)
		iomt_host_stat_rw_size((struct iomt_host_info *)card->host->iomt_host_info,
			(unsigned long)cmdq_rq->data.blocks,
			read_dir ? IOMT_DIR_READ : IOMT_DIR_WRITE);
#endif

	return &mqrq->mmc_cmdq_req;
}

static int mmc_blk_cmdq_issue_rw_rq(struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *active_mqrq = req_to_mmc_queue_req(req);
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_req *mc_rq = NULL;
	int ret;
	int tag;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;

	tag = mmc_cmdq_get_tag( card, req);
	if(tag == -1)
		return BLK_STS_RESOURCE;
	active_mqrq->mmc_cmdq_req.tag = tag;
#else
	tag = req->tag;
	if ((req->tag < 0) || ((unsigned int)(req->tag) > card->ext_csd.cmdq_depth))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
	if (test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif
	if (test_and_set_bit(tag, &host->cmdq_ctx.data_active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);

	mc_rq = mmc_blk_cmdq_rw_prep(active_mqrq, mq);

	mmc_cmdq_task_info_init(card, req);

	ret = mmc_blk_cmdq_start_req(card->host, mc_rq);
#ifdef CONFIG_MMC_MQ_CQ_HCI
	if (ret) {
		clear_bit_unlock(tag, &ctx->active_reqs);
		clear_bit(tag, &host->cmdq_ctx.data_active_reqs);
	}
#endif

	return ret;
}

static void mmc_blk_cmdq_dcmd_done(struct mmc_request *mrq)
{
	complete(&mrq->cmdq_completion);
}

static int mmc_blk_cmdq_wait_for_dcmd(struct mmc_host *host, struct mmc_cmdq_req *cmdq_req)
{
	struct mmc_request *mrq = &cmdq_req->mrq;
	int ret;

	init_completion(&mrq->cmdq_completion);
	mrq->done = mmc_blk_cmdq_dcmd_done;
	mrq->host = host;
	ret = mmc_start_cmdq_request(host, mrq);
	if (ret) {
		pr_err("%s: DCMD error\n", __func__);
		return ret;
	}

	cmdq_req->cmdq_req_flags |= WAIT_COMPLETE;
	wait_for_completion_io(&mrq->cmdq_completion);
	if (mrq->cmd->error) {
		pr_err("%s: DCMD %u failed with err %d\n",
			mmc_hostname(host), mrq->cmd->opcode, mrq->cmd->error);
		ret = mrq->cmd->error;
	}
	return ret;
}

static int mmc_cmdq_do_erase(struct mmc_card *card, struct mmc_queue *mq,
	struct request *req, unsigned int from, unsigned int to, unsigned int arg)
{
	unsigned int qty = 0;
	int err;
	struct mmc_queue_req *active_mqrq = NULL;
	struct mmc_cmdq_context_info *ctx_info = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
	int tag;

	/*
	 * qty is used to calculate the erase timeout which depends on how many
	 * erase groups (or allocation units in SD terminology) are affected.
	 * We count erasing part of an erase group as one erase group.
	 * For SD, the allocation units are always a power of 2.  For MMC, the
	 * erase group size is almost certainly also power of 2, but it does not
	 * seem to insist on that in the JEDEC standard, so we fall back to
	 * division in that case.  SD may not specify an allocation unit size,
	 * in which case the timeout is based on the number of write blocks.
	 *
	 * Note that the timeout for secure trim 2 will only be correct if the
	 * number of erase groups specified is the same as the total of all
	 * preceding secure trim 1 commands.  Since the power may have been
	 * lost since the secure trim 1 commands occurred, it is generally
	 * impossible to calculate the secure trim 2 timeout correctly.
	 */
	if (!card->erase_size)
		return -EOPNOTSUPP;

	if (card->erase_shift)
		qty += ((to >> card->erase_shift) - (from >> card->erase_shift)) + 1;
	else if (mmc_card_sd(card))
		qty += to - from + 1;
	else
		qty += ((to / card->erase_size) - (from / card->erase_size)) + 1;

	if (!mmc_card_blockaddr(card)) {
		from <<= MMC_SECTOR_TO_BLOCK;
		to <<= MMC_SECTOR_TO_BLOCK;
	}

	ctx_info = &card->host->cmdq_ctx;
	active_mqrq = req_to_mmc_queue_req(req);
#ifdef CONFIG_MMC_MQ_CQ_HCI
	tag = MMC_CMDQ_MQ_DCMD_TAG;
#else
	tag = req->tag;
#endif
	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = tag;
	cmdq_req->cmd.opcode = MMC_ERASE_GROUP_START;
	cmdq_req->cmd.arg = from;
	cmdq_req->cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;

	err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
	if (err) {
		pr_err("mmc_erase: group start error %d\n", err);
		goto out;
	}

	active_mqrq = req_to_mmc_queue_req(req);
	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = tag;
	cmdq_req->cmd.opcode = MMC_ERASE_GROUP_END;
	cmdq_req->cmd.arg = to;
	cmdq_req->cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;

	err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
	if (err) {
		pr_err("mmc_erase: group end error %d\n", err);
		goto out;
	}

	active_mqrq = req_to_mmc_queue_req(req);
	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = tag;
	cmdq_req->cmd.opcode = MMC_ERASE;
	cmdq_req->cmd.arg = arg;
	cmdq_req->cmd.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	cmdq_req->cmd.busy_timeout = mmc_erase_timeout(card, arg, qty);

	err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
	if (err) {
		pr_err("mmc_erase: erase error %d\n", err);
		goto out;
	}

out:
	return err;
}

static int mmc_cmdq_erase(struct mmc_card *card,
	struct mmc_queue *mq, struct request *req,
	unsigned int from, unsigned int nr, unsigned int arg)
{
	unsigned int rem;
	unsigned int to;

	if (!(card->host->caps & MMC_CAP_ERASE) || !(card->csd.cmdclass & CCC_ERASE))
		return -EOPNOTSUPP;

	if (!card->erase_size)
		return -EOPNOTSUPP;

	if (mmc_card_sd(card) && arg != MMC_ERASE_ARG)
		return -EOPNOTSUPP;

	if ((arg & MMC_SECURE_ARGS) && !(card->ext_csd.sec_feature_support & EXT_CSD_SEC_ER_EN))
		return -EOPNOTSUPP;

	if ((arg & MMC_TRIM_ARGS) && !(card->ext_csd.sec_feature_support & EXT_CSD_SEC_GB_CL_EN))
		return -EOPNOTSUPP;

	if (arg == MMC_SECURE_ERASE_ARG)
		if (from % card->erase_size || nr % card->erase_size)
			return -EINVAL;

	if (arg == MMC_ERASE_ARG) {
		rem = from % card->erase_size;
		if (rem) {
			rem = card->erase_size - rem;
			from += rem;
			if (nr > rem)
				nr -= rem;
			else
				return 0;
		}
		rem = nr % card->erase_size;
		if (rem)
			nr -= rem;
	}

	if (!nr)
		return 0;

	to = from + nr;

	if (to <= from)
		return -EINVAL;

	/* 'from' and 'to' are inclusive */
	to -= 1;

	return mmc_cmdq_do_erase(card, mq, req, from, to, arg);
}

static int mmc_blk_cmdq_issue_discard_rq(
	struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *active_mqrq = NULL;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
	struct mmc_cmdq_context_info *ctx_info = NULL;
	unsigned int from, nr, arg;
	int err;
	int tag;

	if (!card || !card->host) {
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		return -EINVAL;
	}

	host = card->host;
#ifndef CONFIG_MMC_MQ_CQ_HCI
	tag = req->tag;
	if ((req->tag < 0) || ((unsigned int)(req->tag) > card->ext_csd.cmdq_depth))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
	if (test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);

#endif
	ctx_info = &host->cmdq_ctx;

	if (!mmc_can_erase(card)) {
		err = -EOPNOTSUPP;
		goto out;
	}

	from = blk_rq_pos(req);
	nr = blk_rq_sectors(req);

	if (mmc_can_discard(card))
		arg = MMC_DISCARD_ARG;
	else if (mmc_can_trim(card))
		arg = MMC_TRIM_ARG;
	else
		arg = MMC_ERASE_ARG;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	err = mmc_blk_cmdq_wait_for_rw(card->host, req);
	if (err)
		return BLK_STS_RESOURCE;

	tag = MMC_CMDQ_MQ_DCMD_TAG;
	if (test_and_set_bit_lock(tag, &ctx_info->active_reqs))
		return BLK_STS_RESOURCE;

	active_mqrq = req_to_mmc_queue_req(req);
	active_mqrq->mmc_cmdq_req.tag = tag;
#else
	set_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
#endif
	mmc_cmdq_task_info_init(card, req);

	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		active_mqrq = req_to_mmc_queue_req(req);
		cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
		cmdq_req->cmdq_req_flags |= QBR;
		cmdq_req->mrq.cmd = &cmdq_req->cmd;
		cmdq_req->tag = tag;

		err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd,
			EXT_CSD_CMD_SET_NORMAL, INAND_CMD38_ARG_EXT_CSD,
			arg == MMC_TRIM_ARG ? INAND_CMD38_ARG_TRIM :
				INAND_CMD38_ARG_ERASE, 0, true, true);
		if (err)
			goto out;
		err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
		if (err)
			goto out;
	}

	err = mmc_cmdq_erase(card, mq, req, from, nr, arg);

out:
	if (err == -EBADSLT)
		return err;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	blk_mq_complete_request(req);
#else
	blk_complete_request(req);
#endif
	return err;
}

static int mmc_blk_cmdq_issue_secdiscard_rq(struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *active_mqrq = NULL;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
	struct mmc_cmdq_context_info *ctx_info = NULL;
	unsigned int from, nr, arg;
	int err;
	int tag;

	if (!card || !card->host) {
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		return -EINVAL;
	}

	host = card->host;
#ifndef CONFIG_MMC_MQ_CQ_HCI
	tag = req->tag;
	if ((req->tag < 0) || ((unsigned int)(req->tag) > card->ext_csd.cmdq_depth))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
	if (test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif

	ctx_info = &host->cmdq_ctx;

	if (!(mmc_can_secure_erase_trim(card))) {
		err = -EOPNOTSUPP;
		goto out;
	}

	from = blk_rq_pos(req);
	nr = blk_rq_sectors(req);
	if (mmc_can_trim(card) && !mmc_erase_group_aligned(card, from, nr))
		arg = MMC_SECURE_TRIM1_ARG;
	else
		arg = MMC_SECURE_ERASE_ARG;

#ifdef CONFIG_MMC_MQ_CQ_HCI
	err = mmc_blk_cmdq_wait_for_rw(card->host, req);
	if (err)
		return BLK_STS_RESOURCE;

	tag = MMC_CMDQ_MQ_DCMD_TAG;
	if (test_and_set_bit_lock(tag, &ctx_info->active_reqs))
		return BLK_STS_RESOURCE;
	active_mqrq = req_to_mmc_queue_req(req);
	active_mqrq->mmc_cmdq_req.tag = tag;
#else
	set_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
#endif
	mmc_cmdq_task_info_init(card, req);

	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		active_mqrq = req_to_mmc_queue_req(req);
		cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
		cmdq_req->cmdq_req_flags |= QBR;
		cmdq_req->mrq.cmd = &cmdq_req->cmd;
		cmdq_req->tag = tag;
		err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd,
			EXT_CSD_CMD_SET_NORMAL, INAND_CMD38_ARG_EXT_CSD,
			arg == MMC_SECURE_TRIM1_ARG ? INAND_CMD38_ARG_SECTRIM1 :
			INAND_CMD38_ARG_SECERASE, 0, true, true);
		if (err)
			goto out;
		err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
		if (err)
			goto out;
	}
	err = mmc_cmdq_erase(card, mq, req, from, nr, arg);
	if (err)
		goto out;

	if (arg == MMC_SECURE_TRIM1_ARG) {
		if (card->quirks & MMC_QUIRK_INAND_CMD38) {
			active_mqrq = req_to_mmc_queue_req(req);
			cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
			cmdq_req->cmdq_req_flags |= QBR;
			cmdq_req->mrq.cmd = &cmdq_req->cmd;
			cmdq_req->tag = tag;
			err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd,
				EXT_CSD_CMD_SET_NORMAL, INAND_CMD38_ARG_EXT_CSD,
				INAND_CMD38_ARG_SECTRIM2, 0, true, true);
			if (err)
				goto out;
			err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
			if (err)
				goto out;
		}
		err = mmc_cmdq_erase(card, mq, req, from, nr, MMC_SECURE_TRIM2_ARG);
		if (err)
			goto out;
	}

out:
	if (err == -EBADSLT)
		return err;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	blk_mq_complete_request(req);
#else
	blk_complete_request(req);
#endif
	return err ? 1 : 0;
}

/*
 * Issues a dcmd request
 * Try to pull another request from queue and prepare it in the
 * meantime. If its not a dcmd it can be issued as well.
 */
static int mmc_blk_cmdq_issue_flush_rq(struct mmc_queue *mq, struct request *req)
{
	int err;
	struct mmc_queue_req *active_mqrq = NULL;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
	struct mmc_cmdq_context_info *ctx_info = NULL;
	int tag;

	if (!card || !card->host) {
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		return -EINVAL;
	}

	host = card->host;

#ifndef CONFIG_MMC_MQ_CQ_HCI
	tag = req->tag;
	if ((req->tag < 0) || ((unsigned int)(req->tag) > card->ext_csd.cmdq_depth))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
	if (test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif

	ctx_info = &host->cmdq_ctx;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	err = mmc_blk_cmdq_wait_for_rw(card->host, req);
	if (err)
		return BLK_STS_RESOURCE;

	tag = MMC_CMDQ_MQ_DCMD_TAG;
	if (test_and_set_bit_lock(tag, &ctx_info->active_reqs))
		return BLK_STS_RESOURCE;
	active_mqrq = req_to_mmc_queue_req(req);
	active_mqrq->mmc_cmdq_req.tag = tag;
#else
	set_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
#endif
	active_mqrq = req_to_mmc_queue_req(req);

	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = tag;

	mmc_cmdq_task_info_init(card, req);

	err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd, EXT_CSD_CMD_SET_NORMAL,
		EXT_CSD_FLUSH_CACHE, 1, MMC_FLUSH_REQ_TIMEOUT_MS, true, true);
	if (err)
		goto out;

	err = mmc_blk_cmdq_start_req(card->host, cmdq_req);
out:
#ifdef CONFIG_MMC_MQ_CQ_HCI
	if (err)
		clear_bit_unlock(tag, &host->cmdq_ctx.active_reqs);
#endif
	return err;
}

static void mmc_blk_cmdq_reset(struct mmc_host *host, bool clear_all)
{
	int err;

	if (mmc_cmdq_halt(host, true)) {
		pr_err("%s: halt failed\n", mmc_hostname(host));
		goto reset;
	}

	if (clear_all)
		mmc_cmdq_discard_queue(host, 0);

reset:
	host->cmdq_ops->disable_immediatly(host);
	err = mmc_cmdq_hw_reset(host);
	if (err == -EOPNOTSUPP) {
		pr_err("%s: not support reset\n", __func__);
		host->cmdq_ops->enable(host);
		goto out;
	}

	/*
	 * CMDQ HW reset would have already made CQE
	 * in unhalted state, but reflect the same
	 * in software state of cmdq_ctx.
	 */
	mmc_host_clr_halt(host);
	mmc_card_clr_cmdq(host->card);
out:
	return;
}

/*
 * is_cmdq_dcmd_req - Checks if tag belongs to DCMD request.
 * @q: request_queue pointer.
 * @tag: tag number of request to check.
 * This function checks if the request with tag number "tag"
 * is a DCMD request or not based on cmdq_req_flags set.
 *
 * returns true if DCMD req, otherwise false.
 */
static int is_cmdq_dcmd_req(struct request_queue *q, int tag)
{
	struct request *req = NULL;
	struct mmc_queue_req *mq_rq = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	req = mmc_blk_cmdq_mq_find_req(q, tag);
#else
	req = blk_queue_find_tag(q, tag);
#endif
	if (WARN_ON(!req))
		goto out;
	mq_rq = req->special;
	if (WARN_ON(!mq_rq))
		goto out;
	cmdq_req = &(mq_rq->mmc_cmdq_req);
	return (cmdq_req->cmdq_req_flags & DCMD);
out:
	return -ENOENT;
}

/*
 * While doing error handling, if a discard request is waiting, it maybe
 * block in function mmc_cmdq_wait_for_dcmd -> wait_for_completion_io,
 * So, we should do the completion if it is waiting.
 */
static void mmc_cmdq_dcmd_reset(struct request_queue *q, int tag)
{
	struct request *req = NULL;
	struct mmc_queue_req *mq_rq = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
	struct mmc_request *mrq = NULL;

#ifdef CONFIG_MMC_MQ_CQ_HCI
	req = mmc_blk_cmdq_mq_find_req(q, tag);
#else
	req = blk_queue_find_tag(q, tag);
#endif
	if (WARN_ON(!req))
		return;
	mq_rq = req->special;
	if (WARN_ON(!mq_rq))
		return;
	cmdq_req = &(mq_rq->mmc_cmdq_req);
	if (cmdq_req->cmdq_req_flags & WAIT_COMPLETE) {
		mrq = &cmdq_req->mrq;
		if (mrq && !completion_done(&mrq->cmdq_completion)) {
			pr_err("%s: discard req reset done\n", __func__);
			mrq->cmd->error = -EBADSLT;
			mmc_blk_cmdq_dcmd_done(mrq);
		}
	}
}

/*
 * mmc_blk_cmdq_reset_all - Reset everything for CMDQ block request.
 * @host: mmc_host pointer.
 * @err: error for which reset is performed.
 *
 * This function implements reset_all functionality for
 * cmdq. It resets the controller, power cycle the card,
 * and invalidate all busy tags(requeue all request back to
 * elevator).
 */
static void mmc_blk_cmdq_reset_all(struct mmc_host *host, int err)
{
	struct mmc_request *mrq = host->err_mrq;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct request_queue *q = NULL;
	int itag = 0;
	int ret;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	mutex_lock(&ctx_info->cmdq_queue_rq_mutex);
#endif
	WARN_ON(!test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state));

	pr_debug("%s: %s: active_reqs = %lu\n", mmc_hostname(host), __func__, ctx_info->active_reqs);

	mmc_blk_cmdq_reset(host, false);
	if (test_bit(CMDQ_STATE_SWITCH_ERR, &ctx_info->curr_state)) {
		q = host->switch_err_req->q;
		mmc_release_host(host);
		goto invalidate_tags;
	}

	q = mrq->req->q;
	for_each_set_bit(itag, &ctx_info->active_reqs, (int)(host->cmdq_slots)) {
		ret = is_cmdq_dcmd_req(q, itag);
		if (WARN_ON(ret == -ENOENT))
			continue;

		if (!ret) {
			WARN_ON(!test_and_clear_bit(itag,
				&ctx_info->data_active_reqs));
			mmc_cmdq_post_req(host, mrq, err);
		} else {
			mmc_cmdq_dcmd_reset(q, itag);
			clear_bit(CMDQ_STATE_DCMD_ACTIVE,
				&ctx_info->curr_state);
		}
		WARN_ON(!test_and_clear_bit(itag, &ctx_info->active_reqs));
		mmc_release_host(host);
	}

invalidate_tags:
	spin_lock_irq(q->queue_lock);

#ifdef CONFIG_MMC_MQ_CQ_HCI
	mmc_mq_requeue_invalidate_reqs(q);
#else
	blk_queue_invalidate_tags(q);
#endif
	spin_unlock_irq(q->queue_lock);
#ifdef CONFIG_MMC_MQ_CQ_HCI
	mutex_unlock(&ctx_info->cmdq_queue_rq_mutex);
#endif
}

/* mq impossble to be null */
void mmc_blk_cmdq_shutdown(struct mmc_queue *mq)
{
	int err;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;

	mmc_claim_host(host);
	err = mmc_cmdq_halt(host, true);
	if (err)
		pr_err("%s: halt: failed: %d\n", __func__, err);
	host->cmdq_ops->disable(host, false);

	/* disable CQ mode in card */
	if (mmc_card_cmdq(card)) {
		err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			EXT_CSD_CMDQ_MODE, 0, card->ext_csd.generic_cmd6_time);
		if (err) {
			pr_err("%s: failed to switch card to legacy mode: %d\n", __func__, err);
			goto out;
		}
		mmc_card_clr_cmdq(card);
	}

	host->card->cmdq_init = false;
out:
	mmc_release_host(host);
}

/*
 * mmc_blk_cmdq_err: error handling of cmdq error requests.
 * Function should be called in context of error out request
 * which has claim_host and rpm acquired.
 * This may be called with CQ engine halted. Make sure to
 * unhalt it after error recovery.
 *
 * Currently cmdq error handler does reset_all in case
 * of any erorr. Need to optimize error handling.
 * mq impossble to be null
 */
void mmc_blk_cmdq_err(struct mmc_queue *mq)
{
	struct mmc_host *host = mq->card->host;
	struct mmc_request *mrq = host->err_mrq;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct request_queue *q = NULL;
	int err = 0;

	if (WARN_ON(!mrq))
		return;

	q = mrq->req->q;
	if (test_bit(CMDQ_STATE_SWITCH_ERR, &ctx_info->curr_state))
		goto reset;

	/* if there is a waiting dcmd, ignore it */
	host->cmdq_ctx.in_recovery = true;
	wake_up_interruptible(&host->cmdq_ctx.queue_empty_wq);

	err = mmc_cmdq_halt(host, true);
	if (err) {
		pr_err("halt: failed: %d\n", err);
		goto reset;
	}

	/* RED error - Fatal: requires reset */
	if (mrq->cmdq_req->resp_err) {
		err = mrq->cmdq_req->resp_err;
		pr_crit("%s: Response error detected: Device in bad state\n",
			mmc_hostname(host));
		goto reset;
	}

	/*
	 * In case of software request time-out, we schedule err work only for
	 * the first error out request and handles all other request in flight
	 * here.
	 */
	if (test_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state))
		err = -ETIMEDOUT;
	else if (mrq->data && mrq->data->error)
		err = mrq->data->error;
	else if (mrq->cmd && mrq->cmd->error)
		/* DCMD commands */
		err = mrq->cmd->error;

reset:
	mmc_blk_cmdq_reset_all(host, err);
	if (mrq && mrq->cmdq_req->resp_err)
		mrq->cmdq_req->resp_err = false;
	mmc_cmdq_halt(host, false);

	host->err_mrq = NULL;
	host->cmdq_ctx.in_recovery = false;
	clear_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state);
	clear_bit(CMDQ_STATE_SWITCH_ERR, &ctx_info->curr_state);
	WARN_ON(!test_and_clear_bit(CMDQ_STATE_ERR, &ctx_info->curr_state));
 #ifdef CONFIG_MMC_MQ_CQ_HCI
	blk_mq_run_hw_queues(q, true);
 #else
	wake_up(&ctx_info->wait);
 #endif
}

/* invoked by block layer in softirq context */
void mmc_blk_cmdq_complete_rq(struct request *rq)
{
	struct mmc_queue_req *mq_rq = rq->special;
	struct mmc_request *mrq = &mq_rq->mmc_cmdq_req.mrq;
	struct mmc_host *host = mrq->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct mmc_cmdq_req *cmdq_req = &mq_rq->mmc_cmdq_req;
	struct mmc_queue *mq = (struct mmc_queue *)rq->q->queuedata;
	int err = 0;
	bool curr_req_clear = false;

	if (mrq->cmd && mrq->cmd->error)
		err = mrq->cmd->error;
	else if (mrq->data && mrq->data->error)
		err = mrq->data->error;

	if (err || cmdq_req->resp_err) {
		pr_err("%s: request with req: 0x%x, tag: %d, flags: 0x%x,"
			"curr_state:0x%lx, active reqs:0x%lx timed out\n",
			__func__, rq, rq->tag, rq->cmd_flags,
			ctx_info->curr_state, ctx_info->active_reqs);

		pr_err("%s: %s: txfr error:%d/resp_err:%d\n",
			mmc_hostname(mrq->host), __func__, err,
			cmdq_req->resp_err);

		if (test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state)) {
			pr_err("%s: CQ in error state, ending current req: %d\n", __func__, err);
		} else {
			spin_lock_bh(&ctx_info->cmdq_ctx_lock);
			set_bit(CMDQ_STATE_ERR, &ctx_info->curr_state);
			spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
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
	spin_lock_bh(&ctx_info->cmdq_ctx_lock);
	if (test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state)) {
		spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
		pr_err("%s: softirq may come from different cpu cluster, curr_state:0x%lx\n",
			__func__, ctx_info->curr_state);
		WARN_ON(1);
		return;
	}

	/* clear pending request */
	if (!test_and_clear_bit(cmdq_req->tag, &ctx_info->active_reqs))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
	curr_req_clear = true;

	if (cmdq_req->cmdq_req_flags & DCMD) {
		clear_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx_info->curr_state);
		blk_end_request_all(rq, err);
	} else {
		if (!test_and_clear_bit(cmdq_req->tag, &ctx_info->data_active_reqs))
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		mmc_cmdq_post_req(host, mrq, err);
		blk_end_request(rq, err, cmdq_req->data.bytes_xfered);
	}

	mmc_release_host(host);
	spin_unlock_bh(&ctx_info->cmdq_ctx_lock);

out:

	spin_lock_bh(&ctx_info->cmdq_ctx_lock);
	if (!test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state))
		wake_up(&ctx_info->wait);
	else if (curr_req_clear)
		pr_err("%s: CMDQ_STATE_ERR bit is set after req is clear\n", __func__);

	spin_unlock_bh(&ctx_info->cmdq_ctx_lock);

	if (!ctx_info->active_reqs)
		wake_up_interruptible(&host->cmdq_ctx.queue_empty_wq);

	if (blk_queue_stopped(mq->queue) && !ctx_info->active_reqs)
		complete(&mq->cmdq_shutdown_complete);
}

/* emmc internal call, req impossible to be null */
enum blk_eh_timer_return mmc_blk_cmdq_req_timed_out(struct request *req)
{
	struct mmc_queue *mq = req->q->queuedata;
	struct mmc_host *host = mq->card->host;
	struct mmc_queue_req *mq_rq = req->special;
	struct mmc_request *mrq = NULL;
	struct mmc_cmdq_req *cmdq_req = NULL;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct mmc_cmdq_task_info *cmdq_task_info = host->cmdq_task_info;

	if (!host) {
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		return BLK_EH_NOT_HANDLED;
	}

	if (!test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state))
		host->cmdq_ops->dumpstate(host);

	/*
	 * The mmc_queue_req will be present only if the request
	 * is issued to the LLD. The request could be fetched from
	 * block layer queue but could be waiting to be issued
	 * (for e.g. clock scaling is waiting for an empty cmdq queue)
	 * Reset the timer in such cases to give LLD more time
	 */
	if (!mq_rq) {
		pr_err("%s:req 0x%pK restart timer, curr_state:0x%lx, active reqs:0x%lx\n",
			__func__, req, ctx_info->curr_state, ctx_info->active_reqs);
		return BLK_EH_RESET_TIMER;
	}

	mrq = &mq_rq->mmc_cmdq_req.mrq;
	cmdq_req = &mq_rq->mmc_cmdq_req;

	pr_err("%s: request with req: 0x%x, tag: %u, flags: 0x%x, "
		"curr_state:0x%lx, active reqs:0x%lx timed out\n",
		__func__, req, cmdq_req->tag, req->cmd_flags,
		ctx_info->curr_state, ctx_info->active_reqs);
	pr_err("%s: issue time:%lld, start time:%lld, end time:%lld\n",
		__func__, ktime_to_ns(cmdq_task_info[cmdq_req->tag].issue_time),
		ktime_to_ns(cmdq_task_info[cmdq_req->tag].start_dbr_time),
		ktime_to_ns(cmdq_task_info[cmdq_req->tag].end_dbr_time));

#ifdef CONFIG_HUAWEI_EMMC_DSM
	sdhci_dsm_report(host, mrq);
#endif

	if (cmdq_req->cmdq_req_flags & DCMD)
		mrq->cmd->error = -ETIMEDOUT;
	else
		mrq->data->error = -ETIMEDOUT;

	if (mrq->cmd && mrq->cmd->error) {
		if (req_op(mrq->req) != REQ_OP_FLUSH) {
			/*
			 * Notify completion for non flush commands like
			 * discard that wait for DCMD finish.
			 */
			set_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state);
			complete(&mrq->cmdq_completion);
			return BLK_EH_NOT_HANDLED;
		}
	}

	if (test_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state) ||
		test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state))
		return BLK_EH_NOT_HANDLED;

	set_bit(CMDQ_STATE_REQ_TIMED_OUT, &ctx_info->curr_state);

	return BLK_EH_HANDLED;
}

/*
 * Complete reqs from block layer softirq context
 * Invoked in irq context
 * mmc_blk_cmdq_req_done is internal function
 */
void mmc_blk_cmdq_req_done(struct mmc_request *mrq)
{
	struct request *req = mrq->req;

#ifdef CONFIG_HW_MMC_MAINTENANCE_DATA
	record_cmdq_rw_data(mrq);
#endif

#ifdef CONFIG_EMMC_FAULT_INJECT
	if (mmcdbg_cq_timeout_inj(mrq, ERR_INJECT_CMDQ_TIMEOUT))
		return;
#endif
#ifdef CONFIG_MMC_MQ_CQ_HCI
	blk_mq_complete_request(req);
#else
	blk_complete_request(req);
#endif
}

void mmc_cmdq_task_info_init(struct mmc_card *card, struct request *req)
{
	int tag;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	struct mmc_queue_req *active_mqrq = NULL;
	active_mqrq = req_to_mmc_queue_req(req);
	tag = active_mqrq->mmc_cmdq_req.tag;
#else
	tag = req->tag;
#endif
	if (card->host->cmdq_task_info) {
		card->host->cmdq_task_info[tag].req = req;
		card->host->cmdq_task_info[tag].issue_time = ktime_get();
		card->host->cmdq_task_info[tag].start_dbr_time = 0UL;
		card->host->cmdq_task_info[tag].end_dbr_time = 0UL;
	}
}

static void mmc_cmdq_switch_error(struct mmc_card *card,
	struct mmc_queue *mq, struct request *req)
{
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;

	if (test_bit(CMDQ_STATE_ERR, &ctx_info->curr_state)) {
		pr_err("%s: CQ in error state, ending current req\n", __func__);
		mmc_release_host(card->host);
	} else {
		pr_err("%s: start doing error handle\n", __func__);
		host->switch_err_req = req;
		spin_lock_bh(&ctx_info->cmdq_ctx_lock);
		set_bit(CMDQ_STATE_ERR, &ctx_info->curr_state);
		set_bit(CMDQ_STATE_SWITCH_ERR, &ctx_info->curr_state);
		spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
		schedule_work(&mq->cmdq_err_work);
	}
}

static int mmc_blk_cmdq_dcmd_rq(struct mmc_card *card, struct mmc_queue *mq,
	struct request *req)
{
	int ret;
	struct mmc_cmdq_context_info *ctx_info = &card->host->cmdq_ctx;

	if (((req_op(req) == REQ_OP_FLUSH) || (req_op(req) == REQ_OP_DISCARD)) &&
		(((struct cmdq_host *)mmc_cmdq_private(card->host))->quirks &
		CMDQ_QUIRK_EMPTY_BEFORE_DCMD)) {
		ret = wait_event_interruptible(ctx_info->queue_empty_wq,
			(!ctx_info->active_reqs || ctx_info->in_recovery));
		if (ret) {
			pr_err("%s: failed while waiting for the CMDQ to be empty %s err = %d\n",
				mmc_hostname(card->host), __func__, ret);
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		}

		if (ctx_info->in_recovery) {
			pr_err("%s in recovering, give up the dcmd transfer\n", __func__);
			ctx_info->in_recovery = false;
			spin_lock_irq(mq->queue->queue_lock);
#ifdef CONFIG_MMC_MQ_CQ_HCI
			blk_mq_requeue_request(req, true);
#else
			blk_requeue_request(mq->queue, req);
#endif
			spin_unlock_irq(mq->queue->queue_lock);
			return 1;
		}
	}

	return 0;
}

/* mq impossible to be null */
int mmc_blk_cmdq_issue_rq(struct mmc_queue *mq, struct request *req)
{
	int ret;
	struct mmc_blk_data *md = mq->blkdata;
	struct mmc_card *card = md->queue.card;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	__mmc_claim_host(card->host, &mq->ctx ,NULL);
#else
	mmc_claim_host(card->host);
#endif
	ret = mmc_blk_part_switch(card, md->part_type);
	if (ret) {
		pr_err("%s: %s: partition switch failed %d\n",
			md->disk->disk_name, __func__, ret);
		blk_end_request_all(req, BLK_STS_IOERR);
		mmc_release_host(card->host);
		return BLK_STS_IOERR;
	}

	ret = mmc_blk_cmdq_switch(card, md, true);
	if (ret) {
		pr_err("%s curr part config is %u\n",
			__func__, card->ext_csd.part_config);

		mmc_cmdq_switch_error(card, mq, req);
		return BLK_STS_IOERR;
	}

	ret = mmc_blk_cmdq_dcmd_rq(card, mq, req);
	if (ret) {
		mmc_release_host(card->host);
		return BLK_STS_OK;
	}

	switch (req_op(req)) {
	case REQ_OP_DRV_IN:
	case REQ_OP_DRV_OUT:
		pr_err("%s cmdq has no OP_DRV request\n", __func__);
		break;
	case REQ_OP_DISCARD:
		ret = mmc_blk_cmdq_issue_discard_rq(mq, req);
		break;
	case REQ_OP_SECURE_ERASE:
		if (!(card->quirks & MMC_QUIRK_SEC_ERASE_TRIM_BROKEN))
			ret = mmc_blk_cmdq_issue_secdiscard_rq(mq, req);
		break;
	case REQ_OP_FLUSH:
		ret = mmc_blk_cmdq_issue_flush_rq(mq, req);
		break;
	default:
		ret = mmc_blk_cmdq_issue_rw_rq(mq, req);
		break;
	}
#ifdef CONFIG_MMC_MQ_CQ_HCI
	if (ret)
		mmc_release_host(card->host);
#endif
	return ret;
}

static void mmc_cmdq_dispatch_req(struct request_queue *q)
{
	struct mmc_queue *mq = q->queuedata;

	wake_up(&mq->card->host->cmdq_ctx.wait);
}

/*
 * mmc_blk_cmdq_dump_status
 * @q: request queue
 * @dump_type: dump scenario
 * BLK_DUMP_WARNING: scenario of io latency warning
 * BLK_DUMP_PANIC: scenario of system panic
 */
void mmc_blk_cmdq_dump_status(
	struct request_queue *q, enum blk_dump_scene dump_type)
{
	struct mmc_card *card = NULL;
	struct mmc_host *host = NULL;
	struct mmc_queue *mq = q->queuedata;

	if (!mq)
		return;
	card = mq->card;
	if (!card)
		return;
	host = card->host;
	if (!host)
		return;
	pr_err("active_reqs = 0x%lx, data_active_reqs = 0x%lx,"
		" curr_state = 0x%lx, card_state = 0x%x\n",
		host->cmdq_ctx.active_reqs, host->cmdq_ctx.data_active_reqs,
		host->cmdq_ctx.curr_state, card->state);
}

/*
 * mmc_blk_cmdq_setup_queue
 * @mq: mmc queue
 * @card: card to attach to this queue
 *
 * Setup queue for CMDQ supporting MMC card
 */
void mmc_blk_cmdq_setup_queue(struct mmc_queue *mq, struct mmc_card *card)
{
	u64 limit = (u64)BLK_BOUNCE_HIGH;
	struct mmc_host *host = card->host;

	if (mmc_dev(host)->dma_mask && *mmc_dev(host)->dma_mask)
		limit = *mmc_dev(host)->dma_mask;

	blk_queue_prep_rq(mq->queue, mmc_prep_request);
	queue_flag_set_unlocked(QUEUE_FLAG_NONROT, mq->queue);

	if (mmc_can_erase(card))
		mmc_queue_setup_discard(mq->queue, card);

	blk_queue_bounce_limit(mq->queue, limit);
	blk_queue_max_hw_sectors(mq->queue,
		min(host->max_blk_count, host->max_req_size / MMC_BYTE_TO_BLOCK));
	blk_queue_max_segment_size(mq->queue, host->max_seg_size);
	blk_queue_max_segments(mq->queue, host->max_segs);
#ifdef CONFIG_MMC_MQ_CQ_HCI
#ifdef CONFIG_HISI_BUFFERED_READAHEAD
	mq->queue->backing_dev_info->ra_pages_cr =
		(VM_MAX_READAHEAD_CR * 1024) / PAGE_SIZE;
#endif
#endif
}

static void mmc_cmdq_softirq_done(struct request *rq)
{
	struct mmc_queue *mq = rq->q->queuedata;

	if (mq->cmdq_complete_fn)
		mq->cmdq_complete_fn(rq);
}

void mmc_cmdq_error_work(struct work_struct *work)
{
	struct mmc_queue *mq = container_of(work, struct mmc_queue, cmdq_err_work);

	if (mq->cmdq_error_fn)
		mq->cmdq_error_fn(mq);
}

/* req impossible to be null */
enum blk_eh_timer_return mmc_cmdq_rq_timed_out(struct request *req)
{
	struct mmc_queue *mq = req->q->queuedata;
	/* mq->cmdq_req_timed_out impossible to be null */
	return mq->cmdq_req_timed_out(req);
}

/* internal funtion ,needn't null check */
int mmc_cmdq_init(struct mmc_queue *mq, struct mmc_card *card)
{
	int ret;
	/* one slot is reserved for dcmd requests */
	int q_depth = card->ext_csd.cmdq_depth - 1;

	card->cmdq_init = false;
	spin_lock_init(&card->host->cmdq_ctx.cmdq_ctx_lock);
	init_waitqueue_head(&card->host->cmdq_ctx.queue_empty_wq);
	init_waitqueue_head(&card->host->cmdq_ctx.wait);

	ret = blk_queue_init_tags(mq->queue, q_depth, NULL, 0);
	if (ret) {
		pr_warn("%s: unable to allocate cmdq tags %d\n", mmc_card_name(card), q_depth);
		goto out;
	}
#ifdef CONFIG_MAS_BLK
	card->mmc_tags = mq->queue->queue_tags;
	card->mmc_tags_depth = q_depth;
#endif

	blk_queue_softirq_done(mq->queue, mmc_cmdq_softirq_done);
	INIT_WORK(&mq->cmdq_err_work, mmc_cmdq_error_work);
	init_completion(&mq->cmdq_shutdown_complete);

	blk_queue_rq_timed_out(mq->queue, mmc_cmdq_rq_timed_out);
	blk_queue_rq_timeout(mq->queue, 29 * HZ); /* blk request TO 29ms */

	card->cmdq_init = true;
	goto out;

out:
	return ret;
}

/* internal funtion ,needn't null check */
void mmc_cmdq_clean(struct mmc_queue *mq, struct mmc_card *card)
{
#ifndef CONFIG_MMC_MQ_CQ_HCI
	blk_free_tags(mq->queue->queue_tags);
	mq->queue->queue_tags = NULL;
	blk_queue_free_tags(mq->queue);
#endif
}

static struct request *mmc_peek_request(struct mmc_queue *mq)
{
	struct request_queue *q = mq->queue;

	mq->cmdq_req_peeked = NULL;

	spin_lock_irq(q->queue_lock);
	if (!blk_queue_stopped(q))
		mq->cmdq_req_peeked = blk_peek_request(q);
	spin_unlock_irq(q->queue_lock);

	return mq->cmdq_req_peeked;
}

static int mmc_check_blk_queue_start_tag(
	struct request_queue *q, struct request *req)
{
	int ret;

	spin_lock_irq(q->queue_lock);
	ret = blk_queue_start_tag(q, req);
	spin_unlock_irq(q->queue_lock);

	return ret;
}

static inline void mmc_cmdq_ready_wait(
	struct mmc_host *host, struct mmc_queue *mq)
{
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	struct request_queue *q = mq->queue;

	/*
	 * Wait until all of the following conditions are true:
	 * 1. There is a request pending in the block layer queue
	 *    to be processed.
	 * 2. If the peeked request is flush/discard then there shouldn't
	 *    be any other direct command active.
	 * 3. cmdq state should be unhalted.
	 * 4. cmdq state shouldn't be in error state.
	 * 5. free tag available to process the new request.
	 */
	wait_event(ctx->wait, kthread_should_stop() ||
			(!test_bit(CMDQ_STATE_DCMD_ACTIVE, &ctx->curr_state) &&
				!test_bit(CMDQ_STATE_QUEUE_HUNGUP, &ctx->curr_state) &&
				!mmc_host_halt(host) &&
				!mmc_card_suspended(host->card) &&
				!test_bit(CMDQ_STATE_ERR, &ctx->curr_state) &&
				mmc_peek_request(mq) &&
				!mmc_check_blk_queue_start_tag(q, mq->cmdq_req_peeked)));
}

static int mmc_cmdq_thread(void *d)
{
	struct mmc_queue *mq = d;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	int ret;

	current->flags |= PF_MEMALLOC;
#ifdef CONFIG_HISI_MMC_CQ_WAKE_IDLE
	set_wake_up_idle(true);
#endif

	while (1) {
		mmc_cmdq_ready_wait(host, mq);
		if (kthread_should_stop()) {
			pr_err("%s: cmdq thread stop, maybe hungtask!!", __func__);
			break;
		}

		/* mq->cmdq_issue_fn is impossbile to be null */
		ret = mq->cmdq_issue_fn(mq, mq->cmdq_req_peeked);
		/*
		 * Don't requeue if issue_fn fails, just bug on.
		 * We don't expect failure here and there is no recovery other
		 * than fixing the actual issue if there is any.
		 * Also we end the request if there is a partition switch error,
		 * so we should not requeue the request here.
		 */
		if (ret)
			pr_err("%s: cmdq request issue fail ret = %d\n", __func__, ret);
	}

	return 0;
}

/* internal funtion ,needn't null check */
int mmc_cmdq_init_queue(struct mmc_queue *mq, struct mmc_card *card,
	spinlock_t *lock, const char *subname)
{
	int ret;

	mq->queue = blk_alloc_queue(GFP_KERNEL);
	if (!mq->queue)
		return -ENOMEM;
	mq->queue->queue_lock = lock;
	mq->queue->request_fn = mmc_cmdq_dispatch_req;
	mq->queue->init_rq_fn = mmc_init_request;
	mq->queue->exit_rq_fn = mmc_exit_request;
	mq->queue->cmd_size = sizeof(struct mmc_queue_req);
	mq->queue->queuedata = mq;
	mq->qcnt = 0;

	ret = blk_init_allocated_queue(mq->queue);
	if (ret) {
		blk_cleanup_queue(mq->queue);
		return ret;
	}

	mmc_blk_cmdq_setup_queue(mq, card);
	ret = mmc_cmdq_init(mq, card);
	if (ret) {
		pr_err("%s: %d: cmdq: unable to set-up\n", mmc_hostname(card->host), ret);
		blk_cleanup_queue(mq->queue);
	} else {
		sema_init(&mq->thread_sem, 1);
		mq->queue->queuedata = mq;

		mq->thread = kthread_run(mmc_cmdq_thread, mq, "mmc-cmdqd/%d%s",
			card->host->index, subname ? subname : "");
		if (IS_ERR(mq->thread)) {
			ret = PTR_ERR(mq->thread);
			pr_err("%s: %d: cmdq: failed to start mmc-cmdqd thread\n",
				mmc_hostname(card->host), ret);
			blk_cleanup_queue(mq->queue);
		}

		return ret;
	}
	return ret;
}
