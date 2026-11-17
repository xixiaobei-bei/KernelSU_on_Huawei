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

#ifndef __UFS_UNISTORE_INTERNAL_H__
#define __UFS_UNISTORE_INTERNAL_H__

#define DATA_MOVE_TOTAL_LENGTH_OFFSET 0
#define DATA_MOVE_DEST_4K_LBA_OFFSET 4
#define DATA_MOVE_DEST_LUN_INFO_OFFSET 8
#define DATA_MOVE_DEST_STREAM_ID_OFFSET 9
#define DATA_MOVE_DEST_BLK_MODE_OFFSET 12
#define DATA_MOVE_FORCE_FLUSH_OFFSET 16
#define DATA_MOVE_REPEAT_OPTION_OFFSET 17
#define DATA_MOVE_ERROR_INJECTION_OFFSET 19
#define DATA_MOVE_SOURCE_ADDR_OFFSET 96
#define DATA_MOVE_SOURCE_INODE_OFFSET 6240
#define DATA_MOVE_DATA_LENGTH 0x4000
#define DATA_MOVE_LENGTH 0x1000

struct ufshcd_data_move_buf {
	struct scsi_device *sdev;
	struct stor_dev_data_move_info *data_move_info;
};

#endif
