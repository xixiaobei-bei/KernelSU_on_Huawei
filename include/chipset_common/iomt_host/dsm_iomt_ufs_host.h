/*
 * file_name
 * dsm_iomt_ufs_host.h
 * description
 * stat io latency scatter at driver level,
 * show it in kernel log and host attr node.
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef __DSM_IOMT_UFS_HOST
#define __DSM_IOMT_UFS_HOST

#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_host.h>
#include <linux/iomt_host/dsm_iomt_host.h>

#define dsm_iomt_hba_host_pre_init(hba)		\
	do {					\
		if (!strncmp(dev_name(&(hba->host->shost_dev)),	\
			"host0", strlen("host0")))		\
			(hba)->host->iomt_host_info =		\
				&((hba)->iomt_host_info_entity);\
	} while (0)

unsigned char iomt_host_cmd_dir(struct scsi_cmnd *cmd);

void iomt_host_latency_cmd_init(struct scsi_cmnd *cmd,
	struct Scsi_Host *scsi_host);

void iomt_host_latency_cmd_start_once(struct scsi_cmnd *cmd);

void iomt_host_latency_cmd_start(struct scsi_cmnd *cmd);

void iomt_host_latency_cmd_end(struct Scsi_Host *scsi_host,
	struct scsi_cmnd *cmd);

void dsm_iomt_ufs_host_init(struct Scsi_Host *host);

void dsm_iomt_ufs_host_exit(struct Scsi_Host *host);

#endif
