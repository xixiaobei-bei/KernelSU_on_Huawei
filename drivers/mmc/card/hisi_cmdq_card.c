/*
 * =============================================================================
 *    Description:  cmdq code in card level
 *
 *        Created:  17/03/2016
 *
 *         Author:
 *   Organization:  HISILICON
 *
 * =============================================================================
 */
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/ioprio.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <trace/events/mmc.h>

/*avoid to invoke mainline code,we can only use this ugly code*/
#include "mmc_hisi_card.h"


#define INAND_CMD38_ARG_EXT_CSD  113
#define INAND_CMD38_ARG_ERASE    0x00
#define INAND_CMD38_ARG_TRIM     0x01

#define INAND_CMD38_ARG_SECERASE 0x80
#define INAND_CMD38_ARG_SECTRIM1 0x81
#define INAND_CMD38_ARG_SECTRIM2 0x88

extern int mmc_blk_part_switch(struct mmc_card *card,
				      struct mmc_blk_data *md);

static int mmc_blk_cmdq_switch(struct mmc_card *card,
			struct mmc_blk_data *md, bool enable)
{
	int ret = 0;
	bool cmdq_mode = !!mmc_card_cmdq(card);
	struct mmc_host *host = card->host;

	if (!card->ext_csd.cmdq_mode_en ||
	    (enable && md && !(md->flags & MMC_BLK_CMD_QUEUE)) ||
	    (cmdq_mode == enable))
		return 0;


	if (host->cmdq_ops) {
		if (enable) {
			ret = mmc_set_blocklen(card, MMC_CARD_CMDQ_BLK_SIZE);
			if (ret) {
				pr_err("%s: failed to set block-size to 512\n",
				       __func__);
				BUG();
			}

			ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					 EXT_CSD_CMDQ_MODE, enable,
					 card->ext_csd.generic_cmd6_time);
			if (ret) {
				pr_err("cmdq mode %sable failed %d\n",
				       enable ? "en" : "dis", ret);
				goto out;
			}
			mmc_card_set_cmdq(card);

			/* enable host controller command queue engine */
			ret = host->cmdq_ops->enable(card->host);
			if (ret) {
				pr_err("failed to enable host controller cqe %d\n",
						ret);
			}

		}

		if (ret || !enable) {
			ret = host->cmdq_ops->disable(card->host, true);
			if (ret) {
				pr_err("failed to disable host controller cqe %d\n", ret);
			}
			/* disable CQ mode in card */
			ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_CMDQ_MODE, 0,
					card->ext_csd.generic_cmd6_time);
			if (ret) {
				pr_err("cmdq mode %sable failed %d\n",
					enable ? "en" : "dis", ret);
				BUG();
			}
			mmc_card_clr_cmdq(card);
		}
	} else {
		pr_err("%s: No cmdq ops defined !!!\n", __func__);
		BUG();
	}

out:

	return ret;
}

int mmc_blk_cmdq_hangup(struct mmc_card *card)
{
	struct mmc_cmdq_context_info *ctx_info;
	unsigned long timeout = (1 * 60 * 1000);
	int ret = 0;

	if (card->ext_csd.cmdq_mode_en) {
		ctx_info = &card->host->cmdq_ctx;
		spin_lock_bh(&ctx_info->cmdq_ctx_lock);
		ctx_info->rpmb_in_wait = true;
		spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
		/* wait for cmdq req handle done.*/
		while (ctx_info->active_reqs) {
			if (timeout == 0) {
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
	struct mmc_cmdq_context_info *ctx_info;
	struct mmc_blk_data *md = dev_get_drvdata(&card->dev);

	if (card->ext_csd.cmdq_mode_en) {
		ctx_info = &card->host->cmdq_ctx;
		spin_lock_bh(&ctx_info->cmdq_ctx_lock);
		ctx_info->rpmb_in_wait = false;
		if (test_and_clear_bit(0, &ctx_info->req_starved)) {
			spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
			/*start handle cmdq req*/
			blk_run_queue(md->queue.queue);
		} else {
			spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
		}
	}
}
EXPORT_SYMBOL(mmc_blk_cmdq_restore);

static int mmc_blk_cmdq_start_req(struct mmc_host *host,
				   struct mmc_cmdq_req *cmdq_req)
{
	struct mmc_request *mrq = &cmdq_req->mrq;

	/*cmdq_req->cmdq_req_flags |= QBR;*/

	mrq->done = mmc_blk_cmdq_req_done;
	return mmc_cmdq_start_req(host, cmdq_req);
}

/* prepare for non-data commands */
struct mmc_cmdq_req *mmc_cmdq_prep_dcmd(
		struct mmc_queue_req *mqrq, struct mmc_queue *mq)
{
	struct request *req = mqrq->req;
	struct mmc_cmdq_req *cmdq_req = &mqrq->mmc_cmdq_req;

	memset(&mqrq->mmc_cmdq_req, 0, sizeof(struct mmc_cmdq_req));

	cmdq_req->mrq.data = NULL;
	cmdq_req->cmd_flags = req->cmd_flags;
	cmdq_req->mrq.req = mqrq->req;
	req->special = mqrq;
	cmdq_req->cmdq_req_flags |= DCMD;
	cmdq_req->mrq.cmdq_req = cmdq_req;

	return &mqrq->mmc_cmdq_req;
}
EXPORT_SYMBOL(mmc_cmdq_prep_dcmd);

#define IS_RT_CLASS_REQ(x)     \
	(IOPRIO_PRIO_CLASS(req_get_ioprio(x)) == IOPRIO_CLASS_RT)

static struct mmc_cmdq_req *mmc_blk_cmdq_rw_prep(
		struct mmc_queue_req *mqrq, struct mmc_queue *mq)
{
	struct mmc_card *card = mq->card;
	struct request *req = mqrq->req;
	struct mmc_blk_data *md = mq->data;
	bool do_rel_wr = mmc_req_rel_wr(req) && (md->flags & MMC_BLK_REL_WR);
	bool do_data_tag;
	bool read_dir = (rq_data_dir(req) == READ);
	bool prio = IS_RT_CLASS_REQ(req);
	struct mmc_cmdq_req *cmdq_rq = &mqrq->mmc_cmdq_req;
	u32 map_sg_len = 0;

	memset(&mqrq->mmc_cmdq_req, 0, sizeof(struct mmc_cmdq_req));

	cmdq_rq->tag = req->tag;
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

	/*
	 * Adjust the sg list so it is the same size as the
	 * request.
	 */
	if (cmdq_rq->data.blocks > card->host->max_blk_count)
		cmdq_rq->data.blocks = card->host->max_blk_count;

	if (cmdq_rq->data.blocks != blk_rq_sectors(req)) {
		int i, data_size = cmdq_rq->data.blocks << 9;
		struct scatterlist *sg;

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
	mqrq->mmc_cmdq_req.mrq.req = mqrq->req;
	mqrq->mmc_cmdq_req.mrq.cmdq_req = &mqrq->mmc_cmdq_req;
	mqrq->mmc_cmdq_req.mrq.data = &mqrq->mmc_cmdq_req.data;
	/* mrq.cmd: no opcode, just for record error */
	mqrq->mmc_cmdq_req.mrq.cmd = &mqrq->mmc_cmdq_req.cmd;
	mqrq->req->special = mqrq;

	pr_debug("%s: %s: mrq: 0x%p req: 0x%p mqrq: 0x%p bytes to xf: %d mmc_cmdq_req: 0x%p card-addr: 0x%08x data_sg_len: %d map_sg_len: %d dir(r-1/w-0): %d\n",
		 mmc_hostname(card->host), __func__, &mqrq->mmc_cmdq_req.mrq,
		 mqrq->req, mqrq, (cmdq_rq->data.blocks * cmdq_rq->data.blksz),
		 cmdq_rq, cmdq_rq->blk_addr, cmdq_rq->data.sg_len, map_sg_len,
		 (cmdq_rq->cmdq_req_flags & DIR) ? 1 : 0);
/*there is no iotrace.c in linux 4.1 and the iotrace
 *use a public config.(CONFIG_TRACING) += iotrace.o
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#else
	trace_mmc_blk_cmdq_rw_start(cmdq_rq->cmdq_req_flags, cmdq_rq->tag, cmdq_rq->blk_addr,
		(cmdq_rq->data.blocks * cmdq_rq->data.blksz));
#endif

	return &mqrq->mmc_cmdq_req;
}

static int mmc_blk_cmdq_issue_rw_rq(struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *active_mqrq;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_req *mc_rq;
	int ret = 0;

	BUG_ON((req->tag < 0) || (req->tag > card->ext_csd.cmdq_depth));
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));

	active_mqrq = &mq->mqrq_cmdq[req->tag];
	active_mqrq->req = req;

	mc_rq = mmc_blk_cmdq_rw_prep(active_mqrq, mq);


	ret = mmc_blk_cmdq_start_req(card->host, mc_rq);
	return ret;
}

void mmc_blk_cmdq_dcmd_done(struct mmc_request *mrq)
{
	complete(&mrq->cmdq_completion);
}

static int mmc_blk_cmdq_wait_for_dcmd(struct mmc_host *host,
				   struct mmc_cmdq_req *cmdq_req)
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
	ret = wait_for_completion_timeout(&mrq->cmdq_completion,
			msecs_to_jiffies(60000));
	ret = ret ? 0 : -ETIMEDOUT;

	return ret;
}

static int mmc_cmdq_do_erase(struct mmc_card *card, struct mmc_queue *mq, struct request *req, unsigned int from,
			unsigned int to, unsigned int arg)
{
	unsigned int qty = 0;
	unsigned int fr, nr;
	int err;
	struct mmc_queue_req *active_mqrq;
	struct mmc_cmdq_context_info *ctx_info;
	struct mmc_cmdq_req *cmdq_req;

	fr = from;
	nr = to - from + 1;
	trace_mmc_blk_erase_start(arg, fr, nr);

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
	if (card->erase_shift)
		qty += ((to >> card->erase_shift) -
			(from >> card->erase_shift)) + 1;
	else if (mmc_card_sd(card))
		qty += to - from + 1;
	else
		qty += ((to / card->erase_size) -
			(from / card->erase_size)) + 1;

	if (!mmc_card_blockaddr(card)) {
		from <<= 9;
		to <<= 9;
	}

	ctx_info = &card->host->cmdq_ctx;
	active_mqrq = &mq->mqrq_cmdq[req->tag];
	active_mqrq->req = req;
	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = req->tag;
	cmdq_req->cmd.opcode = MMC_ERASE_GROUP_START;
	cmdq_req->cmd.arg = from;
	cmdq_req->cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;

	err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
	if (err) {
		pr_err("mmc_erase: group start error %d.", err);
		err = -EIO;
		goto out;
	}

	active_mqrq = &mq->mqrq_cmdq[req->tag];
	active_mqrq->req = req;
	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = req->tag;
	cmdq_req->cmd.opcode = MMC_ERASE_GROUP_END;
	cmdq_req->cmd.arg = to;
	cmdq_req->cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;

	err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
	if (err) {
		pr_err("mmc_erase: group end error %d.\n", err);
		err = -EIO;
		goto out;
	}

	active_mqrq = &mq->mqrq_cmdq[req->tag];
	active_mqrq->req = req;
	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = req->tag;
	cmdq_req->cmd.opcode = MMC_ERASE;
	cmdq_req->cmd.arg = arg;
	cmdq_req->cmd.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	cmdq_req->cmd.busy_timeout = mmc_erase_timeout(card, arg, qty);

	err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
	if (err) {
		pr_err("mmc_erase: erase error %d.\n", err);
		err = -EIO;
		goto out;
	}

out:
	trace_mmc_blk_erase_end(arg, fr, nr);
	return err;
}

static int mmc_cmdq_erase(struct mmc_card *card, struct mmc_queue *mq, struct request *req,
		unsigned int from, unsigned int nr, unsigned int arg)
{
	unsigned int rem, to = from + nr;

	if (!(card->host->caps & MMC_CAP_ERASE) ||
	    !(card->csd.cmdclass & CCC_ERASE))
		return -EOPNOTSUPP;

	if (!card->erase_size)
		return -EOPNOTSUPP;

	if (mmc_card_sd(card) && arg != MMC_ERASE_ARG)
		return -EOPNOTSUPP;

	if ((arg & MMC_SECURE_ARGS) &&
	    !(card->ext_csd.sec_feature_support & EXT_CSD_SEC_ER_EN))
		return -EOPNOTSUPP;

	if ((arg & MMC_TRIM_ARGS) &&
	    !(card->ext_csd.sec_feature_support & EXT_CSD_SEC_GB_CL_EN))
		return -EOPNOTSUPP;

	if (arg == MMC_SECURE_ERASE_ARG) {
		if (from % card->erase_size || nr % card->erase_size)
			return -EINVAL;
	}

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

	if (nr == 0)
		return 0;

	to = from + nr;

	if (to <= from)
		return -EINVAL;

	/* 'from' and 'to' are inclusive */
	to -= 1;

	return mmc_cmdq_do_erase(card, mq, req, from, to, arg);
}

static int mmc_blk_cmdq_issue_discard_rq(struct mmc_queue *mq, struct request *req)
{
	struct mmc_queue_req *active_mqrq;
	struct mmc_card *card = mq->card;
	struct mmc_host *host;
	struct mmc_cmdq_req *cmdq_req;
	struct mmc_cmdq_context_info *ctx_info;
	unsigned int from, nr, arg;
	int err = 0;

	BUG_ON(!card);
	host = card->host;
	BUG_ON(!host);
	BUG_ON((req->tag < 0) || (req->tag > card->ext_csd.cmdq_depth));
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));

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

	spin_lock_bh(&ctx_info->cmdq_ctx_lock);
	ctx_info->active_dcmd = true;
	spin_unlock_bh(&ctx_info->cmdq_ctx_lock);

	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		active_mqrq = &mq->mqrq_cmdq[req->tag];
		active_mqrq->req = req;
		cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
		cmdq_req->cmdq_req_flags |= QBR;
		cmdq_req->mrq.cmd = &cmdq_req->cmd;
		cmdq_req->tag = req->tag;

		err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd, EXT_CSD_CMD_SET_NORMAL,
				INAND_CMD38_ARG_EXT_CSD,
				arg == MMC_TRIM_ARG ? INAND_CMD38_ARG_TRIM : INAND_CMD38_ARG_ERASE,
				0, true, true);
		if (err)
			goto out;
		err = mmc_blk_cmdq_wait_for_dcmd(card->host, cmdq_req);
		if (err)
			goto out;
	}

	err = mmc_cmdq_erase(card, mq, req, from, nr, arg);

out:
	blk_complete_request(req);
	return err;
}

static int mmc_blk_cmdq_issue_secdiscard_rq(struct mmc_queue *mq,
				       struct request *req)
{
	struct mmc_queue_req *active_mqrq;
	struct mmc_card *card = mq->card;
	struct mmc_host *host;
	struct mmc_cmdq_req *cmdq_req;
	struct mmc_cmdq_context_info *ctx_info;
	unsigned int from, nr, arg;
	int err = 0;

	BUG_ON(!card);
	host = card->host;
	BUG_ON(!host);
	BUG_ON((req->tag < 0) || (req->tag > card->ext_csd.cmdq_depth));
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));

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

	spin_lock_bh(&ctx_info->cmdq_ctx_lock);
	ctx_info->active_dcmd = true;
	spin_unlock_bh(&ctx_info->cmdq_ctx_lock);

	if (card->quirks & MMC_QUIRK_INAND_CMD38) {
		active_mqrq = &mq->mqrq_cmdq[req->tag];
		active_mqrq->req = req;
		cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
		cmdq_req->cmdq_req_flags |= QBR;
		cmdq_req->mrq.cmd = &cmdq_req->cmd;
		cmdq_req->tag = req->tag;
		err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd, EXT_CSD_CMD_SET_NORMAL,
				INAND_CMD38_ARG_EXT_CSD,
				arg == MMC_SECURE_TRIM1_ARG ? INAND_CMD38_ARG_SECTRIM1 : INAND_CMD38_ARG_SECERASE,
				0, true, true);
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
			active_mqrq = &mq->mqrq_cmdq[req->tag];
			active_mqrq->req = req;
			cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
			cmdq_req->cmdq_req_flags |= QBR;
			cmdq_req->mrq.cmd = &cmdq_req->cmd;
			cmdq_req->tag = req->tag;
			err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd, EXT_CSD_CMD_SET_NORMAL,
					INAND_CMD38_ARG_EXT_CSD,
					INAND_CMD38_ARG_SECTRIM2,
					0, true, true);
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
	blk_complete_request(req);

	return err ? 0 : 1;
}

/*
 * Issues a dcmd request
 * FIXME:
 *	Try to pull another request from queue and prepare it in the
 *	meantime. If its not a dcmd it can be issued as well.
 */
int mmc_blk_cmdq_issue_flush_rq(struct mmc_queue *mq, struct request *req)
{
	int err;
	struct mmc_queue_req *active_mqrq;
	struct mmc_card *card = mq->card;
	struct mmc_host *host;
	struct mmc_cmdq_req *cmdq_req;
	struct mmc_cmdq_context_info *ctx_info;

	BUG_ON(!card);
	host = card->host;
	BUG_ON(!host);
	BUG_ON((req->tag < 0) || (req->tag > card->ext_csd.cmdq_depth));
	BUG_ON(test_and_set_bit(req->tag, &host->cmdq_ctx.active_reqs));

	ctx_info = &host->cmdq_ctx;
#if 0
	if (in_irq() || irqs_disabled() || in_serving_softirq()) {
		lock_irq_flag = 0;
	} else {
		lock_irq_flag = 1;
	}

	if (0 == lock_irq_flag)
		spin_lock(&ctx_info->cmdq_ctx_lock);
	else
		spin_lock_bh(&ctx_info->cmdq_ctx_lock);
#else
	spin_lock_bh(&ctx_info->cmdq_ctx_lock);
#endif
	ctx_info->active_dcmd = true;
#if 0
	if (0 == lock_irq_flag)
		spin_unlock(&ctx_info->cmdq_ctx_lock);
	else
		spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
#else
	spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
#endif
	active_mqrq = &mq->mqrq_cmdq[req->tag];
	active_mqrq->req = req;

	cmdq_req = mmc_cmdq_prep_dcmd(active_mqrq, mq);
	cmdq_req->cmdq_req_flags |= QBR;
	cmdq_req->mrq.cmd = &cmdq_req->cmd;
	cmdq_req->tag = req->tag;

	// need this??? can do this???
	err = __mmc_switch_cmdq_mode(cmdq_req->mrq.cmd, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_FLUSH_CACHE, 1,
				     MMC_FLUSH_REQ_TIMEOUT_MS, true, true);
	if (err)
		return err;

	err = mmc_blk_cmdq_start_req(card->host, cmdq_req);
	return err;
}
EXPORT_SYMBOL(mmc_blk_cmdq_issue_flush_rq);

/* invoked by block layer in softirq context */
void mmc_blk_cmdq_complete_rq(struct request *rq)
{
	struct mmc_queue_req *mq_rq = rq->special;
	struct mmc_request *mrq = &mq_rq->mmc_cmdq_req.mrq;
	struct mmc_host *host = mrq->host;
	struct mmc_cmdq_context_info *ctx_info = &host->cmdq_ctx;
	struct mmc_cmdq_req *cmdq_req = &mq_rq->mmc_cmdq_req;
	struct request_queue *q = rq->q;
	int err = 0;

	if (mrq->cmd && mrq->cmd->error)
		err = mrq->cmd->error;
	else if (mrq->data && mrq->data->error)
		err = mrq->data->error;

	mmc_cmdq_post_req(host, mrq, err);
	spin_lock(&ctx_info->cmdq_ctx_lock);
	if (err) {
		pr_err("%s: %s: txfr error: %d\n", mmc_hostname(mrq->host),
		       __func__, err);

		if (mmc_cmdq_halt(host, true))
			BUG();
		ctx_info->curr_state |= CMDQ_STATE_ERR;
		/* TODO: wake-up kernel thread to handle error */
	}

	BUG_ON(!test_and_clear_bit(cmdq_req->tag,
				   &ctx_info->active_reqs));
	if (cmdq_req->cmdq_req_flags & DCMD) {
		ctx_info->active_dcmd = false;
		spin_unlock(&ctx_info->cmdq_ctx_lock);
		blk_end_request_all(rq, 0);

	} else {
		spin_unlock(&ctx_info->cmdq_ctx_lock);
/*there is no iotrace.c in linux 4.1 and the iotrace
 *use a public config.(CONFIG_TRACING) += iotrace.o
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
#else
	trace_mmc_blk_cmdq_rw_end(cmdq_req->cmdq_req_flags, cmdq_req->tag, cmdq_req->blk_addr,
			cmdq_req->data.bytes_xfered);
#endif
		blk_end_request(rq, 0, cmdq_req->data.bytes_xfered);
	}

	spin_lock(&ctx_info->cmdq_ctx_lock);
	if (test_and_clear_bit(0, &ctx_info->req_starved)) {
		spin_unlock(&ctx_info->cmdq_ctx_lock);
		blk_run_queue(q);
	} else {
		spin_unlock(&ctx_info->cmdq_ctx_lock);
	}
}

/*
 * Complete reqs from block layer softirq context
 * Invoked in irq context
 */
void mmc_blk_cmdq_req_done(struct mmc_request *mrq)
{
	struct request *req = mrq->req;

	blk_complete_request(req);
}
EXPORT_SYMBOL(mmc_blk_cmdq_req_done);

extern int cmdq_is_reset(struct mmc_host *host);
int mmc_blk_cmdq_issue_rq(struct mmc_queue *mq, struct request *req)
{
	int ret = 0;
	struct mmc_blk_data *md = mq->data;
	struct mmc_card *card = md->queue.card;
	unsigned int cmd_flags = req->cmd_flags;

	struct mmc_cmdq_context_info *ctx_info = &card->host->cmdq_ctx;
	struct mmc_queue_req *mq_rq = &mq->mqrq_cmdq[req->tag];
	struct mmc_cmdq_req *cmdq_req = &mq_rq->mmc_cmdq_req;

	mmc_claim_host(card->host);

	if (cmdq_is_reset(card->host)) {
                   ret = -EHOSTUNREACH;
                   goto requeue_for_reset;
	}

	if (mmc_card_suspended(card)) {
		pr_err("cmdq issue rq in suspend!!!!\n");
		if ((!(cmd_flags & REQ_DISCARD)) && (!(cmd_flags & REQ_FLUSH)))
			pr_err("%s<cmdq>: req addr: 0x%lx blocks: %d dir(r-1/w-0): %d\n",
				mmc_hostname(card->host), blk_rq_pos(req),
				blk_rq_sectors(req), (rq_data_dir(req) == READ) ? 1 : 0);
		spin_lock_irq(mq->queue->queue_lock);
		blk_requeue_request(mq->queue, req);
		spin_unlock_irq(mq->queue->queue_lock);
		goto switch_failure;
	}

	ret = mmc_blk_part_switch(card, md);
	if (ret) {
		pr_err("%s: %s: partition switch failed %d\n",
				md->disk->disk_name, __func__, ret);
		blk_end_request_all(req, ret);
		goto switch_failure;
	}

	ret = mmc_blk_cmdq_switch(card, md, true);
	if (ret) {
		pr_err("%s curr part config is %u\n", __func__, card->ext_csd.part_config);
		/* TODO: put a limit on the number of requeues if switch fails
		 * and if possible disable cmd queing for buggy cards.
		 */
		spin_lock_irq(mq->queue->queue_lock);
		blk_requeue_request(mq->queue, req);
		spin_unlock_irq(mq->queue->queue_lock);
		goto switch_failure;
	}

	if (cmd_flags & REQ_DISCARD) {
		 if (req->cmd_flags & REQ_SECURE &&
			!(card->quirks & MMC_QUIRK_SEC_ERASE_TRIM_BROKEN))
			ret = mmc_blk_cmdq_issue_secdiscard_rq(mq, req);
		 else
			ret = mmc_blk_cmdq_issue_discard_rq(mq, req);
	} else if (cmd_flags & REQ_FLUSH) {
		ret = mmc_blk_cmdq_issue_flush_rq(mq, req);
	} else {
		ret = mmc_blk_cmdq_issue_rw_rq(mq, req);
	}
	/*EHOSTDOWN means that cq_host->enabled is false when cmdq_request;
	 *we need re-enable the cmdq feature and requeue the
	 *failed request
	 */
	if (-EHOSTDOWN == ret) {
		ret = mmc_blk_cmdq_switch(card, NULL, false);
		if (ret)
			pr_err("%s %d disable cmdq feature false.\n", __func__, __LINE__);

		ret = mmc_blk_cmdq_switch(card, md, true);
		if (ret)
			pr_err("%s %d. enable cmdq feature false\n", __func__, __LINE__);

		clear_bit(req->tag, &card->host->cmdq_ctx.active_reqs);
		pr_err("%s reqest failed;requeue req.\n", __func__);
		spin_lock_irq(mq->queue->queue_lock);
		blk_requeue_request(mq->queue, req);
		spin_unlock_irq(mq->queue->queue_lock);
	}

requeue_for_reset:
               if (-EHOSTUNREACH == ret) {
                              clear_bit(req->tag, &card->host->cmdq_ctx.active_reqs);
                              spin_lock_irq(mq->queue->queue_lock);
                              blk_requeue_request(mq->queue, req);
                              spin_unlock_irq(mq->queue->queue_lock);

                              if (cmdq_req->cmdq_req_flags & DCMD) {
                                             spin_lock_bh(&ctx_info->cmdq_ctx_lock);
                                             ctx_info->active_dcmd =false;
                                             spin_unlock_bh(&ctx_info->cmdq_ctx_lock);
                              }
	}

switch_failure:
	mmc_release_host(card->host);
	return ret;
}

extern int mmc_prep_request(struct request_queue *q, struct request *req);
extern void mmc_queue_setup_discard(struct request_queue *q,
				    struct mmc_card *card);
extern struct scatterlist *mmc_alloc_sg(int sg_len, int *err);

static inline bool mmc_cmdq_should_pull_reqs(struct mmc_host *host,
					struct mmc_cmdq_context_info *ctx)
{

	if (ctx->active_dcmd || ctx->rpmb_in_wait) {
		/* TODO: check ctx->curr_state */
		pr_debug("%s: %s: skip pulling reqs: dcmd: %d rpmb: %d state: %d\n",
			 mmc_hostname(host), __func__, ctx->active_dcmd,
			 ctx->rpmb_in_wait, ctx->curr_state);

		return false;
	} else {
		return true;
	}
}
#if 0
static void mmc_cmdq_dispatch_req(struct request_queue *q)
{
	struct request *req;
	struct mmc_queue *mq = q->queuedata;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;
	u32 lock_irq_flag = 0;

	if (in_serving_softirq()) {
		lock_irq_flag = 0;
	} else {
		lock_irq_flag = 1;
	}

	while (1) {
		if (0 == lock_irq_flag)
			spin_unlock(q->queue_lock);
		else
			spin_unlock_irq(q->queue_lock);

		if (!mmc_cmdq_should_pull_reqs(host, ctx)) {

			if (0 == lock_irq_flag)
				spin_lock(q->queue_lock);
			else
				spin_lock_irq(q->queue_lock);

			test_and_set_bit(0, &ctx->req_starved);
			return;
		}

		if (0 == lock_irq_flag)
			spin_lock(q->queue_lock);
		else
			spin_lock_irq(q->queue_lock);
		req = blk_peek_request(q);

		if (!req)
			return;

		if (blk_queue_start_tag(q, req)) {
			test_and_set_bit(0, &ctx->req_starved);
			return;
		}

		if (0 == lock_irq_flag)
			spin_unlock(q->queue_lock);
		else
			spin_unlock_irq(q->queue_lock);

		mq->cmdq_issue_fn(mq, req);

		if (0 == lock_irq_flag)
			spin_lock(q->queue_lock);
		else
			spin_lock_irq(q->queue_lock);
	}
}

static void mmc_cmdq_req_work(struct work_struct *work)
{
	return;
}

#else
static void mmc_cmdq_dispatch_req(struct request_queue *q)
{
	struct mmc_queue *mq = q->queuedata;

	queue_work(mq->workqueue_cmdq, &mq->work_cmdq);
}

static void mmc_cmdq_req_work(struct work_struct *work)
{
	struct request *req;
	struct mmc_queue *mq =
		container_of(work, struct mmc_queue, work_cmdq);
	struct request_queue *q = mq->queue;
	struct mmc_card *card = mq->card;
	struct mmc_host *host = card->host;
	struct mmc_cmdq_context_info *ctx = &host->cmdq_ctx;

	down(&mq->thread_sem);
	while (1) {
		spin_lock_bh(&ctx->cmdq_ctx_lock);
		if (!mmc_cmdq_should_pull_reqs(host, ctx)) {
			test_and_set_bit(0, &ctx->req_starved);
			spin_unlock_bh(&ctx->cmdq_ctx_lock);
			break;
		}
		spin_unlock_bh(&ctx->cmdq_ctx_lock);

		spin_lock_irq(q->queue_lock);
		req = blk_peek_request(q);
		if (!req) {
#ifdef CONFIG_BLK_DEV_HI_PRIO_FOR_FG
			if (!list_empty(&q->queue_head))
				test_and_set_bit(0, &ctx->req_starved);
#endif
			spin_unlock_irq(q->queue_lock);
			break;
		}

		if (blk_queue_start_tag(q, req)) {
			spin_unlock_irq(q->queue_lock);
			continue;
		}

		spin_unlock_irq(q->queue_lock);
		mq->cmdq_issue_fn(mq, req);
	}
	up(&mq->thread_sem);

	return;
}
#endif

/**
 * mmc_blk_cmdq_setup_queue
 * @mq: mmc queue
 * @card: card to attach to this queue
 *
 * Setup queue for CMDQ supporting MMC card
 */
void mmc_blk_cmdq_setup_queue(struct mmc_queue *mq, struct mmc_card *card)
{
	u64 limit = BLK_BOUNCE_HIGH;
	struct mmc_host *host = card->host;

	if (mmc_dev(host)->dma_mask && *mmc_dev(host)->dma_mask)
		limit = *mmc_dev(host)->dma_mask;

	blk_queue_prep_rq(mq->queue, mmc_prep_request);
	queue_flag_set_unlocked(QUEUE_FLAG_NONROT, mq->queue);

	if (mmc_can_erase(card))
		mmc_queue_setup_discard(mq->queue, card);

	blk_queue_bounce_limit(mq->queue, limit);
	blk_queue_max_hw_sectors(mq->queue, min(host->max_blk_count,
						host->max_req_size / 512));
	blk_queue_max_segment_size(mq->queue, host->max_seg_size);
	blk_queue_max_segments(mq->queue, host->max_segs);
}


static void mmc_cmdq_softirq_done(struct request *rq)
{
	struct mmc_queue *mq = rq->q->queuedata;

	mq->cmdq_complete_fn(rq);
}

int mmc_cmdq_init(struct mmc_queue *mq, struct mmc_card *card)
{
	int i, ret = 0;
	/* one slot is reserved for dcmd requests */
	int q_depth = card->ext_csd.cmdq_depth - 1;

	card->cmdq_init = false;
	spin_lock_init(&card->host->cmdq_ctx.cmdq_ctx_lock);

	mq->mqrq_cmdq = kzalloc(
			sizeof(struct mmc_queue_req) * q_depth, GFP_KERNEL);
	if (!mq->mqrq_cmdq) {
		pr_warn("%s: unable to allocate mqrq's for q_depth %d\n",
			mmc_card_name(card), q_depth);
		ret = -ENOMEM;
		goto out;
	}

	for (i = 0; i < q_depth; i++) {
		/* TODO: reduce the sg allocation by delaying them */
		mq->mqrq_cmdq[i].sg = mmc_alloc_sg(card->host->max_segs, &ret);
		if (ret) {
			pr_warn("%s: unable to allocate cmdq sg of size %d\n",
				mmc_card_name(card),
				card->host->max_segs);
			goto free_mqrq_sg;
		}
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	ret = blk_queue_init_tags(mq->queue, q_depth, NULL, 0);
#else
	ret = blk_queue_init_tags(mq->queue, q_depth, NULL);
#endif
	if (ret) {
		pr_warn("%s: unable to allocate cmdq tags %d\n",
				mmc_card_name(card), q_depth);
		goto free_mqrq_sg;
	}

	blk_queue_softirq_done(mq->queue, mmc_cmdq_softirq_done);

	mq->workqueue_cmdq = create_singlethread_workqueue("cmdq_workqueue");
	INIT_WORK(&mq->work_cmdq, mmc_cmdq_req_work);

	card->cmdq_init = true;
	goto out;

free_mqrq_sg:
	/* only can free to the Nth sg which failed to allocate */
	for (i = 0; i < q_depth; i++)
		kfree(mq->mqrq_cmdq[i].sg);
	kfree(mq->mqrq_cmdq);
	mq->mqrq_cmdq = NULL;
out:
	return ret;
}

void mmc_cmdq_clean(struct mmc_queue *mq, struct mmc_card *card)
{
	int i;
	int q_depth = card->ext_csd.cmdq_depth - 1;

	blk_free_tags(mq->queue->queue_tags);
	mq->queue->queue_tags = NULL;
	blk_queue_free_tags(mq->queue);

	for (i = 0; i < q_depth; i++)
		kfree(mq->mqrq_cmdq[i].sg);
	kfree(mq->mqrq_cmdq);
	mq->mqrq_cmdq = NULL;
}


int mmc_cmdq_init_queue(struct mmc_queue *mq, struct mmc_card * card,
			spinlock_t *lock)
{
	int ret;

	mq->queue = blk_init_queue(mmc_cmdq_dispatch_req, lock);
	if (!mq->queue)
		return -ENOMEM;
	mmc_blk_cmdq_setup_queue(mq, card);
	ret = mmc_cmdq_init(mq, card);
	if (ret) {
		pr_err("%s: %d: cmdq: unable to set-up\n",
			mmc_hostname(card->host), ret);
		blk_cleanup_queue(mq->queue);
	} else {
		sema_init(&mq->thread_sem, 1);
		mq->queue->queuedata = mq;
		return ret;
	}
	return ret;
}

/*this reset func may cause dead lock of claim_host*/

void mmc_blk_cmdq_reset(struct mmc_host *host)
{
	struct mmc_card *card = host->card;
	struct mmc_blk_data *md = dev_get_drvdata(&card->dev);

	mmc_claim_host(host);
	mmc_card_clr_cmdq(card);
	if (host->cmdq_ops && host->cmdq_ops->disable_immediatly)
		host->cmdq_ops->disable_immediatly(card->host);

	mmc_hw_reset(host);

	if(!md)
		pr_err("%s get md failed!\n",__func__);
	(void)mmc_blk_cmdq_switch(card, md, (bool)true);
	mmc_release_host(host);
}
