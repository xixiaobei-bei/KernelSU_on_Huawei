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

#ifndef __UFS_UNISTORE_READ_H__
#define __UFS_UNISTORE_READ_H__

#include <linux/blkdev.h>
#include "ufshcd.h"

int ufshcd_dev_pwron_info_sync(struct scsi_device *dev,
	struct stor_dev_pwron_info *stor_info,
	unsigned int rescue_seg_size);
int ufshcd_stream_oob_info_fetch(struct scsi_device *sdev,
	struct stor_dev_stream_info stream_info, unsigned int oob_entry_cnt,
	struct stor_dev_stream_oob_info *oob_info);
int ufshcd_dev_read_section_size(struct scsi_device *sdev,
	unsigned int *section_size);
int ufshcd_dev_read_lrb_in_use(struct scsi_device *sdev,
	unsigned long *lrb_in_use);
int ufshcd_dev_read_section_size_hba(struct ufs_hba *hba,
	unsigned int *section_size);
int ufshcd_dev_read_mapping_partition(
	struct scsi_device *sdev,
	struct stor_dev_mapping_partition *mapping_info);
int ufshcd_dev_config_mapping_partition(
	struct scsi_device *sdev,
	struct stor_dev_mapping_partition *mapping_info);
int ufshcd_dev_get_bad_block_info(struct scsi_device *sdev,
	struct stor_dev_bad_block_info *bad_block_info);
int ufshcd_dev_get_program_size(struct scsi_device *sdev,
	struct stor_dev_program_size *program_size);
int ufshcd_dev_read_op_size(struct scsi_device *dev, int *op_size);

#endif
