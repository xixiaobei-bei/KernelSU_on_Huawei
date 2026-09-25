/*
 * file_name
 * dsm_iomt_emmc_host.h
 * description
 * stat io latency scatter at driver level,
 * show it in kernel log and host attr node.
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DSM_IOMT_EMMC_HOST
#define __DSM_IOMT_EMMC_HOST

#include <linux/mmc/host.h>
#include <linux/iomt_host/dsm_iomt_host.h>

static inline unsigned char iomt_host_mrq_dir(struct mmc_request *mrq)
{
	return mrq->data ? ((mrq->data->flags & MMC_DATA_WRITE) ?
		IOMT_DIR_WRITE : IOMT_DIR_READ) : IOMT_DIR_OTHER;
}

static inline void iomt_host_latency_mrq_init(struct mmc_request *mrq)
{
	mrq->io_start_ticks = 0;
}

static inline void iomt_host_latency_mrq_start_once(struct mmc_request *mrq)
{
	if (mrq->io_start_ticks == 0)
		mrq->io_start_ticks = jiffies;
}

static inline void iomt_host_latency_mrq_start(struct mmc_request *mrq)
{
	mrq->io_start_ticks = jiffies;
}

static inline void iomt_host_latency_mrq_end(struct mmc_host *mmc_host,
						struct mmc_request *mrq)
{
	struct iomt_host_io_timeout_dsm *io_timeout_dsm = NULL;
	struct iomt_host_info *iomt_host_info = NULL;
	unsigned int i;
	unsigned char dir;

	if (mmc_host->iomt_host_info == NULL)
		return;

	iomt_host_info = iomt_info_from_host(mmc_host);

	dir = iomt_host_mrq_dir(mrq);

	i = iomt_host_stat_latency(iomt_host_info, mrq->io_start_ticks, dir);

	if (unlikely(i == IOMT_LATENCY_INVALID_INDEX))
		return;

	if (unlikely(i >= iomt_host_info->io_timeout_dsm.judge_slot)) {
		io_timeout_dsm = &(iomt_host_info->io_timeout_dsm);
		io_timeout_dsm->op_arg = 0;
		io_timeout_dsm->block_ticks = 0;
		io_timeout_dsm->data_dir = dir;

		if (mrq->cmd) {
			io_timeout_dsm->op_arg = (unsigned short)mrq->cmd->arg;
			io_timeout_dsm->op_arg |=
				(((unsigned int)mrq->cmd->opcode) <<
				IOMT_OP_BLOCK_SHIFT);
		}

		io_timeout_dsm->block_ticks =
			(unsigned short)(jiffies - mrq->io_start_ticks);

		if (mrq->data)
			io_timeout_dsm->block_ticks |=
				(((unsigned int)mrq->data->blocks) <<
				IOMT_OP_BLOCK_SHIFT);
	}
}

#define dsm_iomt_mmc_host_pre_init(host)			\
	do {							\
		if (!strncmp(mmc_hostname((host)->mmc),	\
			"mmc0", strlen("mmc0")))			\
			(host)->mmc->iomt_host_info =		\
			&((host)->iomt_host_info_entity);		\
	} while (0)

void dsm_iomt_mmc_host_init(struct mmc_host *host);

void dsm_iomt_mmc_host_exit(struct mmc_host *host);

#endif
