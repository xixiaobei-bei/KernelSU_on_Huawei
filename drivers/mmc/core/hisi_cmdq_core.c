/*
 * cmdq code in core level
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
#include <linux/mmc/ioctl.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/ioprio.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <trace/events/mmc.h>

#include "card.h"
#include "core.h"
#include "mmc_zodiac_card.h"

#define CMDQ_RETRY 32

static unsigned long cmdq_read_blocks;
static unsigned long cmdq_write_blocks;

static void mmc_start_cmdq_request_rwlog_enable(struct mmc_host *host, struct mmc_request *mrq)
{
	if (rwlog_enable_flag == 1) { /* 0 : Disable , 1: Enable */
		if (mrq->data) {
			if (mrq->data->flags & MMC_DATA_READ) {
				cmdq_read_blocks += mrq->data->blocks;
				pr_info("%s-lifetime:blk_addr %u blksz %u blocks %u"
					" flags %08x tsac %lu ms nsac %u (r blocks %lu)\n",
					mmc_hostname(host),
					mrq->cmdq_req->blk_addr,
					mrq->data->blksz, mrq->data->blocks,
					mrq->data->flags,
					mrq->data->timeout_ns / NSEC_PER_MSEC,
					mrq->data->timeout_clks,
					cmdq_read_blocks);
			}
			if (mrq->data->flags & MMC_DATA_WRITE) {
				cmdq_write_blocks += mrq->data->blocks;
				pr_info("%s-lifetime:blk_addr %u blksz %u blocks %u"
					" flags %08x tsac %lu ms nsac %u (w blocks %lu)\n",
					mmc_hostname(host),
					mrq->cmdq_req->blk_addr,
					mrq->data->blksz, mrq->data->blocks,
					mrq->data->flags,
					mrq->data->timeout_ns / NSEC_PER_MSEC,
					mrq->data->timeout_clks,
					cmdq_write_blocks);
			}
		}
		if (mrq->cmdq_req && (mrq->cmdq_req->cmdq_req_flags & DCMD))
			if (mrq->cmd->opcode == MMC_SWITCH ||
				mrq->cmd->opcode == MMC_ERASE_GROUP_START ||
				mrq->cmd->opcode == MMC_ERASE_GROUP_END ||
				mrq->cmd->opcode == MMC_ERASE)
				pr_info("%s-lifetime:cmd=%u,index=%d,arg=0x%x\n",
					mmc_hostname(host), (int)mrq->cmd->opcode, host->index, mrq->cmd->arg);
	}
}

/* emmc internal function needn't check null aviod affect read/write performance */
int mmc_start_cmdq_request(struct mmc_host *host, struct mmc_request *mrq)
{
	int ret = 0;

	if (mrq->data)
		pr_debug("%s:blksz %u blocks %u flags %08x tsac %lu ms nsac %u\n",
			mmc_hostname(host), mrq->data->blksz,
			mrq->data->blocks, mrq->data->flags,
			mrq->data->timeout_ns / NSEC_PER_MSEC,
			mrq->data->timeout_clks);

	mmc_start_cmdq_request_rwlog_enable(host, mrq);

	mrq->cmd->error = 0;
	mrq->cmd->retries = CMDQ_RETRY;
	mrq->cmd->mrq = mrq;
	if (mrq->data) {
		if (mrq->data->blksz > host->max_blk_size)
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		if (mrq->data->blocks > host->max_blk_count)
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
		if (mrq->data->blocks * mrq->data->blksz > host->max_req_size)
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);

		mrq->cmd->data = mrq->data;
		mrq->data->error = 0;
		mrq->data->mrq = mrq;
	}

#ifdef CONFIG_HW_MMC_MAINTENANCE_CMD
	record_mmc_cmdq_cmd(mrq);
#endif
	if (host->cmdq_ops->request)
		ret = host->cmdq_ops->request(host, mrq);
	return ret;
}

/*
 * mmc_cmdq_post_req - post process of a completed request
 * @host: host instance
 * @mrq: the request to be processed
 * @err: non-zero is error, success otherwise
 */
void mmc_cmdq_post_req(struct mmc_host *host, struct mmc_request *mrq, int err)
{
	if (host->cmdq_ops->post_req)
		host->cmdq_ops->post_req(host, mrq, err);
}

/*
 * mmc_cmdq_halt - halt/un-halt the command queue engine
 * @host: host instance
 * @halt: true - halt, un-halt otherwise
 *
 * Host halts the command queue engine. It should complete
 * the ongoing transfer and release the SD bus.
 * All legacy SD commands can be sent upon successful
 * completion of this function.
 * Returns 0 on success, negative otherwise
 */
int mmc_cmdq_halt(struct mmc_host *host, bool halt)
{
	int err = 0;
#ifdef CONFIG_MMC_MQ_CQ_HCI
	struct mmc_blk_data *md = NULL;
	struct mmc_queue *mq = NULL;
#endif
	if ((halt && mmc_host_halt(host)) || (!halt && !mmc_host_halt(host))) {
		pr_debug("%s: %s: CQE is already %s\n", mmc_hostname(host),
			__func__, halt ? "halted" : "un-halted");
		return 0;
	}

	if (host->cmdq_ops->halt) {
		err = host->cmdq_ops->halt(host, halt);
		if (!err && halt) {
			mmc_host_set_halt(host);
		} else if (!err && !halt) {
			mmc_host_clr_halt(host);
#ifdef CONFIG_MMC_MQ_CQ_HCI
			md = dev_get_drvdata(&host->card->dev);
			mq = &md->queue;
			blk_mq_run_hw_queues(mq->queue, true);
#else
			wake_up(&host->cmdq_ctx.wait);
#endif
		}
	}
	return err;
}

/* emmc internal function needn't check null */
int mmc_cmdq_start_req(struct mmc_host *host, struct mmc_cmdq_req *cmdq_req)
{
	struct mmc_request *mrq = &cmdq_req->mrq;
	int ret;

	mrq->host = host;
	if (mmc_card_removed(host->card)) {
		mrq->cmd->error = -ENOMEDIUM;
		return -ENOMEDIUM;
	}
	ret = mmc_start_cmdq_request(host, mrq);
	return ret;
}

static inline void mmc_prepare_switch(struct mmc_command *cmd, u8 index,
	u8 value, u8 set, unsigned int tout_ms, bool use_busy_signal)
{
	cmd->opcode = MMC_SWITCH;
	/* write cmd6 arg [25:24] Access, [23:16] Index, [15:8] Value */
	cmd->arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) | (value << 8) | set;
	cmd->flags = MMC_CMD_AC;
	cmd->busy_timeout = tout_ms;
	if (use_busy_signal)
		cmd->flags |= MMC_RSP_SPI_R1B | MMC_RSP_R1B;
	else
		cmd->flags |= MMC_RSP_SPI_R1 | MMC_RSP_R1;
}

/* emmc internal function needn't check null */
int __mmc_switch_cmdq_mode(struct mmc_command *cmd, u8 set, u8 index, u8 value,
	unsigned int timeout_ms, bool use_busy_signal, bool ignore_timeout)
{
	mmc_prepare_switch(cmd, index, value, set, timeout_ms, use_busy_signal);
	return 0;
}

/* emmc internal function needn't check null */
int mmc_cmdq_hw_reset(struct mmc_host *host)
{
	if (host->cmdq_ops->reset)
		host->cmdq_ops->reset(host);
	return mmc_hw_reset(host);
}

/* emmc internal function needn't check null */
int mmc_discard_queue(struct mmc_host *host, u32 tasks)
{
	struct mmc_command cmd = {0};

	cmd.opcode = MMC_CMDQ_TASK_MGMT;
	if (tasks) {
		cmd.arg = DISCARD_TASK;
		/* write cmd48 arg [20:16]: TaskID */
		cmd.arg |= (tasks << 16);
	} else {
		cmd.arg = DISCARD_QUEUE;
	}

	cmd.flags = MMC_RSP_R1B | MMC_CMD_AC;

	return mmc_wait_for_cmd(host, &cmd, 0);
}

/*
 * mmc_cmdq_discard_card_queue - discard the task[s] in the device
 * @host: host instance
 * @tasks: mask of tasks to be knocked off
 *      0: remove all queued tasks
 */
int mmc_cmdq_discard_queue(struct mmc_host *host, u32 tasks)
{
	int ret = 0;

	if (host->cmdq_ops->discard_task) {
		ret = host->cmdq_ops->discard_task(host, tasks, true);
		if (ret)
			pr_err("%s: discard queue fail\n", __func__);
	}
	return ret;
}
