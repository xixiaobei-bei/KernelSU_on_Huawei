/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
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

#ifndef __UFS_UNISTORE_H__
#define __UFS_UNISTORE_H__

#include <scsi/ufs/ioctl.h>
#include "ufshcd.h"

void ufshcd_prepare_utp_query_req_upiu_unistore(
	struct ufs_query *query, struct utp_upiu_req *ucd_req_ptr,
	u8 *descp, u16 len);
void ufshcd_unistore_op_register(struct ufs_hba *hba);
void ufshcd_bad_block_exception_event_handler(
	struct ufs_hba *hba);
void ufshcd_add_buf_to_recovery_list(struct ufs_hba *hba);
int ufshcd_custom_upiu_unistore(
	struct utp_upiu_req *ucd_req_ptr, struct request *req,
	struct scsi_cmnd *scmd, struct ufs_hba *hba);
int ufshcd_unistore_init(struct ufs_hba *hba);
int ufshcd_get_fsr_by_read_buffer(struct ufs_hba *hba, u8 *buffer,
	u16 buffer_len);
void ufshcd_unistore_done(struct ufs_hba *hba, struct scsi_cmnd *cmd,
	struct utp_upiu_rsp *ucd_rsp_ptr);
#endif
