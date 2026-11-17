/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: ufs vcmd implement
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

#include "ufs_vcmd_proc.h"

#include <asm/unaligned.h>

/* vcmd request timeout */
#define VCMD_REQ_TIMEOUT 3000 /* 3 seconds */

#define VCMD_RW_BUFFER_SET 0x1

void ufshcd_init_query_vcmd(struct ufs_hba *hba,
	struct ufs_query_req **request, struct ufs_query_res **response,
	struct ufs_query_vcmd *cmd)
{
	ufshcd_init_query(hba, request, response, cmd->opcode, cmd->idn,
		cmd->index, cmd->selector);

	hba->dev_cmd.query.descriptor = cmd->desc_buf;
	(*request)->query_func = cmd->query_func;
	(*request)->has_data = cmd->has_data;
	(*request)->lun = cmd->lun;
	(*request)->upiu_req.value = cmd->value;
	(*request)->upiu_req.reserved_osf = cmd->reserved_osf;
	(*request)->upiu_req.reserved[0] = cmd->reserved[0];
	(*request)->upiu_req.reserved[1] = cmd->reserved[1];
	(*request)->upiu_req.length = cpu_to_be16(cmd->buf_len);
}

static int __ufshcd_query_vcmd(struct ufs_hba *hba,
	struct ufs_query_vcmd *cmd)
{
	struct ufs_query_req *request = NULL;
	struct ufs_query_res *response = NULL;
	int err;

	mutex_lock(&hba->dev_cmd.lock);

	ufshcd_init_query_vcmd(hba, &request, &response, cmd);

	hba->is_hi186x_query_vcmd = true;
	err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_QUERY, VCMD_REQ_TIMEOUT);
	hba->is_hi186x_query_vcmd = false;
	if (err) {
		dev_err(hba->dev,
			"%s: opcode 0x%.2x for idn 0x%x failed, err = %d\n",
			__func__, cmd->opcode, cmd->idn, err);
		goto out_unlock;
	}

	hba->dev_cmd.query.descriptor = NULL;
	cmd->buf_len = be16_to_cpu(response->upiu_res.length);

	if (cmd->response)
		memcpy(cmd->response, response, sizeof(struct ufs_query_res));

out_unlock:
	mutex_unlock(&hba->dev_cmd.lock);

	return err;
}

int ufshcd_query_vcmd_single(struct ufs_hba *hba,
	struct ufs_query_vcmd *cmd)
{
	return __ufshcd_query_vcmd(hba, cmd);
}

int ufshcd_query_vcmd_retry(struct ufs_hba *hba,
	struct ufs_query_vcmd *cmd)
{
	int err = 0;
	int retries;

	for (retries = VCMD_REQ_RETRIES; retries > 0; retries--) {
		err = __ufshcd_query_vcmd(hba, cmd);
		if (!err || err == -EINVAL)
			break;
	}

	return err;
}

#ifdef CONFIG_SCSI_UFS_UNISTORE
static int ufshcd_rw_buffer_vcmd_retry(struct scsi_device *dev,
	struct ufs_rw_buffer_vcmd *vcmd, int data_direction)
{
	int i;
	int ret = -EINVAL;
	struct scsi_sense_hdr sshdr;
	unsigned char cmd[RW_BUFFER_CDB_LEN] = { 0 };

	cmd[RW_BUFFER_OPCODE_OFFSET] = vcmd->opcode;
	cmd[RW_BUFFER_MODE_OFFSET] = RW_BUFFER_MODE;
	cmd[RW_BUFFER_BUFFER_ID_OFFSET] = vcmd->buffer_id;

	for (i = 0; i < RW_BUFFER_1ST_RESERVED_LEN; ++i)
		cmd[RW_BUFFER_1ST_RESERVED_OFFSET + i] = vcmd->reserved_1st[i];

	put_unaligned_be16(vcmd->buffer_len, cmd + RW_BUFFER_LEN_OFFSET);

	for (i = 0; i < RW_BUFFER_2ND_RESERVED_LEN; ++i)
		cmd[RW_BUFFER_2ND_RESERVED_OFFSET + i] = vcmd->reserved_2nd[i];

	for (i = 0; i < vcmd->retries; ++i) {
		ret = scsi_execute_req(dev, cmd, data_direction, vcmd->buffer,
			vcmd->buffer_len, &sshdr, VCMD_REQ_TIMEOUT,
			vcmd->retries, NULL);
		if (!ret)
			break;
	}

	return ret;
}

int ufshcd_read_buffer_vcmd_retry(struct scsi_device *dev,
	struct ufs_rw_buffer_vcmd *vcmd)
{
	vcmd->opcode = READ_BUFFER;

	return ufshcd_rw_buffer_vcmd_retry(dev, vcmd, DMA_FROM_DEVICE);
}

int ufshcd_write_buffer_vcmd_retry(struct scsi_device *dev,
	struct ufs_rw_buffer_vcmd *vcmd)
{
	vcmd->opcode = WRITE_BUFFER;

	return ufshcd_rw_buffer_vcmd_retry(dev, vcmd, DMA_TO_DEVICE);
}

bool ufshcd_rw_buffer_is_enabled(struct ufs_hba *hba)
{
	if (!hba || !hba->host)
		return false;

	return (hba->host->vcmd_set == VCMD_RW_BUFFER_SET);
}
#endif