/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: ufs vcmd header file
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

#ifndef __UFS_VCMD_PROC_H__
#define __UFS_VCMD_PROC_H__

#include "ufs.h"
#include "ufshcd.h"

#define VCMD_REQ_RETRIES 5

#ifdef CONFIG_SCSI_UFS_UNISTORE
#define RW_BUFFER_CDB_LEN 16
#define RW_BUFFER_1ST_RESERVED_LEN 4
#define RW_BUFFER_2ND_RESERVED_LEN 6

#define RW_BUFFER_MODE 0x1
#define RW_BUFFER_CONTROL 0x0

#define RW_BUFFER_OPCODE_OFFSET 0
#define RW_BUFFER_MODE_OFFSET 1
#define RW_BUFFER_BUFFER_ID_OFFSET 2
#define RW_BUFFER_1ST_RESERVED_OFFSET 3
#define RW_BUFFER_LEN_OFFSET 7
#define RW_BUFFER_CONTROL_OFFSET 9
#define RW_BUFFER_2ND_RESERVED_OFFSET 10

#define VCMD_READ_PWRON_INFO_BUFFER_ID 0x20
#define VCMD_READ_OOB_BUFFER_ID 0x21
#define VCMD_READ_FSR_BUFFER_ID 0x22

#define VCMD_WRITE_DATA_MOVE_BUFFER_ID 0x21
#endif

struct ufs_query_vcmd {
	enum query_opcode opcode;
	u8 idn;
	u8 index;
	u8 selector;
	u8 query_func;
	u8 lun;
	bool has_data;
	__be32 value;
	__be16 reserved_osf;
	__be32 reserved[2];

	u8 *desc_buf;
	int buf_len;
	struct ufs_query_res *response;
};

#ifdef CONFIG_SCSI_UFS_UNISTORE
struct ufs_rw_buffer_vcmd {
	u8 opcode;
	u8 buffer_id;
	u8 retries;
	u8 reserved_1st[RW_BUFFER_1ST_RESERVED_LEN];
	u8 reserved_2nd[RW_BUFFER_2ND_RESERVED_LEN];
	u16 buffer_len;
	u8 *buffer;
};
#endif

int ufshcd_query_vcmd_single(struct ufs_hba *hba,
	struct ufs_query_vcmd *cmd);
int ufshcd_query_vcmd_retry(struct ufs_hba *hba,
	struct ufs_query_vcmd *cmd);
#ifdef CONFIG_SCSI_UFS_UNISTORE
int ufshcd_read_buffer_vcmd_retry(struct scsi_device *dev,
	struct ufs_rw_buffer_vcmd *vcmd);
int ufshcd_write_buffer_vcmd_retry(struct scsi_device *dev,
	struct ufs_rw_buffer_vcmd *vcmd);
bool ufshcd_rw_buffer_is_enabled(struct ufs_hba *hba);
#endif
#endif
