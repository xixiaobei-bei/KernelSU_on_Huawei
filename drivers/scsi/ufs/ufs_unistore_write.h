/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: unistore header file
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

#ifndef __UFS_UNISTORE_WRITE_H__
#define __UFS_UNISTORE_WRITE_H__

#include <linux/blkdev.h>
#include <scsi/scsi_cmnd.h>
#include "ufs.h"

int ufshcd_dev_reset_ftl(struct scsi_device *sdev,
	struct stor_dev_reset_ftl *reset_ftl_info);
int ufshcd_dev_fs_sync_done(struct scsi_device *sdev);

int ufshcd_dev_data_move(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info);
int ufshcd_dev_slc_mode_configuration(
	struct scsi_device *sdev, int *status);
int ufshcd_dev_sync_read_verify(struct scsi_device *sdev,
	struct stor_dev_sync_read_verify_info *verify_info);
int ufshcd_dev_data_move_init(void);
void ufshcd_dev_data_move_done(struct scsi_cmnd *cmd,
	struct utp_upiu_rsp *ucd_rsp_ptr);
void ufschd_data_move_prepare_buf(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info, unsigned char *buf);
#ifdef CONFIG_MAS_DEBUG_FS
int ufshcd_dev_rescue_block_inject_data(
	struct scsi_device *sdev, unsigned int lba);
int ufshcd_dev_bad_block_error_inject(
	struct scsi_device *sdev, unsigned char bad_slc_cnt,
	unsigned char bad_tlc_cnt);
#endif
#endif
