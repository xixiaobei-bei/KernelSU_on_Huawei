/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: unistore implement
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

#include "ufs_unistore_internal.h"

#include <scsi/scsi_device.h>
#include <scsi/ufs/ufs.h>
#include <asm/unaligned.h>
#include <linux/mutex.h>

#include "ufshcd.h"
#include "ufs_func.h"
#include "ufshcd-kirin-interface.h"
#include "ufs_vcmd_proc.h"

enum {
	OP_RESET_SEG_FTL,
	OP_CLOSE_SECTION,
	OP_TYPE_MAX
};

enum {
	STREAM_NORMAL,
	STREAM_DATAMOVE,
	STREAM_TYPE_MAX
};

#define DATA_MOVE_RESPONSE_LEN 16
#define STREAM_DATAMOVE_OFFSET 32

#define VERIFY_INFO_NEXT_TO_BE_VERIFY_OFFSET 0
#define VERIFY_INFO_NEXT_AVAILABLE_OFFSET 4

#define QUERY_INFO_LUN_INFO_OFFSET 8
#define QUERY_VERIFY_DONE_STATUS_OFFSET 12
#define QUERY_VERIFY_FAIL_REASON_OFFSET 13
#define QUERY_PU_SIZE_OFFSET 14

#define WRITE_BUFFER_VERIFY_DONE_STATUS_OFFSET 8
#define WRITE_BUFFER_VERIFY_FAIL_REASON_OFFSET 9
#define WRITE_BUFFER_PU_SIZE_OFFSET 10

#define QUERY_SLC_STATUS_OFFSET 4

struct ufshcd_data_move_ctrl {
	struct mutex lock;
	u8 *send_buffer;
	bool response_flag;
	u8 response[DATA_MOVE_RESPONSE_LEN];
};

struct ufshcd_data_move_ctrl g_data_move_ctl;

static void ufshcd_get_reset_ftl_stream_id(unsigned char *stream_id,
	struct stor_dev_reset_ftl *reset_ftl_info)
{
	*stream_id = 0;

	if (reset_ftl_info->op_type == OP_CLOSE_SECTION) {
		*stream_id = reset_ftl_info->stream_id;

		if (reset_ftl_info->stream_type == STREAM_DATAMOVE)
			*stream_id += STREAM_DATAMOVE_OFFSET;
	}
}

static int ufshcd_reset_ftl(struct ufs_hba *hba,
	struct stor_dev_reset_ftl *reset_ftl_info)
{
	int ret;
	unsigned char stream_id;
	struct ufs_query_vcmd cmd = { 0 };

	ufshcd_get_reset_ftl_stream_id(&stream_id, reset_ftl_info);

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_VENDOR_RESET_FTL;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.reserved_osf = __cpu_to_be16((
			(unsigned short)reset_ftl_info->op_type << 8) & 0xFF00);
	cmd.reserved_osf |= __cpu_to_be16(stream_id & 0xFF);

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s failed, err=%d\n", __func__, ret);

	return ret;
}

int ufshcd_dev_reset_ftl(struct scsi_device *sdev,
	struct stor_dev_reset_ftl *reset_ftl_info)
{
	int err;
	struct ufs_hba *hba = NULL;

	if (!sdev || !reset_ftl_info)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	err = ufshcd_reset_ftl(hba, reset_ftl_info);
	if (unlikely(err))
		dev_err(hba->dev, "unistore reset ftl failed , %s\n", __func__);

	return err;
}

static int ufshcd_fs_sync_done(struct ufs_hba *hba)
{
	int ret;
	struct ufs_query_vcmd cmd = { 0 };

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_VENDOR_FS_SYNC_DONE;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s failed, err=%d\n", __func__, ret);

	return ret;
}

int ufshcd_dev_fs_sync_done(struct scsi_device *sdev)
{
	int ret;
	struct ufs_hba *hba = shost_priv(sdev->host);

	ret = ufshcd_fs_sync_done(hba);
	if (unlikely(ret))
		dev_err(hba->dev, "%s failed, err=%d\n", __func__, ret);

	return ret;
}

int ufshcd_dev_data_move_init(void)
{
	mutex_init(&g_data_move_ctl.lock);
	g_data_move_ctl.response_flag = false;
	g_data_move_ctl.send_buffer = kzalloc(DATA_MOVE_DATA_LENGTH, __GFP_NOFAIL);
	if (!g_data_move_ctl.send_buffer)
		return -ENOMEM;

	return 0;
}

static u8 *ufshcd_get_data_move_send_buffer(void)
{
	return g_data_move_ctl.send_buffer;
}

static u8 *ufshcd_get_data_move_response(void)
{
	if (g_data_move_ctl.response_flag)
		return g_data_move_ctl.response;
	else
		return NULL;
}

static void ufshcd_clear_data_move_response(void)
{
	g_data_move_ctl.response_flag = false;
	memset(g_data_move_ctl.response, 0, DATA_MOVE_RESPONSE_LEN);
}

void ufshcd_dev_data_move_done(struct scsi_cmnd *cmd,
	struct utp_upiu_rsp *ucd_rsp_ptr)
{
	if ((cmd->req.cmd[RW_BUFFER_OPCODE_OFFSET] == WRITE_BUFFER) &&
		(cmd->req.cmd[RW_BUFFER_BUFFER_ID_OFFSET] ==
			VCMD_WRITE_DATA_MOVE_BUFFER_ID)) {
		memcpy(g_data_move_ctl.response, ucd_rsp_ptr->sr.reserved,
			DATA_MOVE_RESPONSE_LEN);
		g_data_move_ctl.response_flag = true;
	}
}

static void ufshcd_data_move_lock(void)
{
	mutex_lock(&g_data_move_ctl.lock);
}

static void ufshcd_data_move_unlock(void)
{
	mutex_unlock(&g_data_move_ctl.lock);
}

static void ufshcd_get_data_move_info(struct ufs_hba *hba,
	struct stor_dev_data_move_info *data_move_info, u8 *buf)
{
	data_move_info->verify_info.next_to_be_verify_4k_lba =
		get_unaligned_be32(buf + VERIFY_INFO_NEXT_TO_BE_VERIFY_OFFSET);
	data_move_info->verify_info.next_available_write_4k_lba =
		get_unaligned_be32(buf + VERIFY_INFO_NEXT_AVAILABLE_OFFSET);

	if (ufshcd_rw_buffer_is_enabled(hba)) {
		data_move_info->verify_info.verify_done_status =
			buf[WRITE_BUFFER_VERIFY_DONE_STATUS_OFFSET];
		data_move_info->verify_info.verify_fail_reason =
			buf[WRITE_BUFFER_VERIFY_FAIL_REASON_OFFSET];
		data_move_info->verify_info.pu_size =
			get_unaligned_be16(buf + WRITE_BUFFER_PU_SIZE_OFFSET);
	} else {
		data_move_info->verify_info.verify_done_status =
			buf[QUERY_VERIFY_DONE_STATUS_OFFSET];
		data_move_info->verify_info.verify_fail_reason =
			buf[QUERY_VERIFY_FAIL_REASON_OFFSET];
		data_move_info->verify_info.pu_size =
			get_unaligned_be16(buf + QUERY_PU_SIZE_OFFSET);
	}
}

static int ufshcd_dev_data_move_by_query(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info, u8 *buffer)
{
	int ret;
	struct ufs_hba *hba = shost_priv(sdev->host);
	struct ufs_query_vcmd cmd = { 0 };
	struct ufshcd_data_move_buf *data_move_buf = NULL;

	data_move_buf = (struct ufshcd_data_move_buf *)buffer;
	data_move_buf->sdev = sdev;
	data_move_buf->data_move_info = data_move_info;

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_VENDOR_DATA_MOVE;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.buf_len = DATA_MOVE_DATA_LENGTH;
	cmd.desc_buf = buffer;

	ret = ufshcd_query_vcmd_single(hba, &cmd);
	if (ret) {
		dev_err(hba->dev, "%s: query fail %d\n", __func__, ret);

		return ret;
	}

	ufshcd_get_data_move_info(hba, data_move_info, buffer);

	return 0;
}

static void ufschd_data_move_prepare_addr(u8 lun_id,
	struct stor_dev_data_move_info *data_move_info, unsigned char *buf)
{
	unsigned int i;
	struct stor_dev_data_move_source_addr *source_addr = NULL;
	unsigned int source_addr_size = 8;
	unsigned int source_addr_num =
		(data_move_info->source_addr_num <= DATA_MOVE_MAX_NUM) ?
		data_move_info->source_addr_num : DATA_MOVE_MAX_NUM;

	for (i = 0; i < source_addr_num; ++i) {
		source_addr = data_move_info->source_addr + i;
		put_unaligned_be32(source_addr->data_move_source_addr,
			buf + DATA_MOVE_SOURCE_ADDR_OFFSET +
			i * source_addr_size);

		buf[DATA_MOVE_SOURCE_ADDR_OFFSET + 4 + i * source_addr_size] =
			lun_id;
		buf[DATA_MOVE_SOURCE_ADDR_OFFSET + 5 + i * source_addr_size] =
			0;
		buf[DATA_MOVE_SOURCE_ADDR_OFFSET + 6 + i * source_addr_size] =
			(unsigned char)((source_addr->source_length >> 8) &
				0xFF);
		buf[DATA_MOVE_SOURCE_ADDR_OFFSET + 7 + i * source_addr_size] =
			(unsigned char)(source_addr->source_length & 0xFF);
	}

	for (; i < DATA_MOVE_MAX_NUM; ++i)
		memset(&buf[DATA_MOVE_SOURCE_ADDR_OFFSET +
			i * source_addr_size], 0, source_addr_size);
}

static void ufschd_data_move_prepare_inode(
	struct stor_dev_data_move_info *data_move_info, unsigned char *buf)
{
	unsigned int i;
	unsigned int source_inode_size = 8;
	struct stor_dev_data_move_source_inode *source_inode = NULL;
	unsigned int source_inode_num =
		(data_move_info->source_inode_num <= DATA_MOVE_MAX_NUM) ?
		data_move_info->source_inode_num : DATA_MOVE_MAX_NUM;

	for (i = 0; i < source_inode_num; ++i) {
		source_inode = (data_move_info->source_inode) + i;
		put_unaligned_be32(source_inode->data_move_source_inode,
			buf + DATA_MOVE_SOURCE_INODE_OFFSET +
			i * source_inode_size);
		put_unaligned_be32(source_inode->data_move_source_offset,
			buf + DATA_MOVE_SOURCE_INODE_OFFSET +
			4 + i * source_inode_size);
	}

	for (; i < DATA_MOVE_MAX_NUM; ++i)
		memset(&buf[DATA_MOVE_SOURCE_INODE_OFFSET +
			i * source_inode_size], 0, source_inode_size);
}

void ufschd_data_move_prepare_buf(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info, unsigned char *buf)
{
	u8 lun_id = ufshcd_scsi_to_upiu_lun(sdev->lun);

	put_unaligned_be32(data_move_info->data_move_total_length,
		buf + DATA_MOVE_TOTAL_LENGTH_OFFSET);
	put_unaligned_be32(data_move_info->dest_4k_lba,
		buf + DATA_MOVE_DEST_4K_LBA_OFFSET);

	buf[DATA_MOVE_DEST_LUN_INFO_OFFSET] = data_move_info->dest_lun_info;
	buf[DATA_MOVE_DEST_STREAM_ID_OFFSET] = data_move_info->dest_stream_id;
	buf[DATA_MOVE_DEST_BLK_MODE_OFFSET] = data_move_info->dest_blk_mode;
	buf[DATA_MOVE_FORCE_FLUSH_OFFSET] = data_move_info->force_flush_option;
	buf[DATA_MOVE_REPEAT_OPTION_OFFSET] = data_move_info->repeat_option;
	buf[DATA_MOVE_ERROR_INJECTION_OFFSET] = data_move_info->error_injection;

	ufschd_data_move_prepare_addr(lun_id, data_move_info, buf);
	ufschd_data_move_prepare_inode(data_move_info, buf);
}

static int ufshcd_dev_data_move_by_write_buff(
	struct scsi_device *dev,
	struct stor_dev_data_move_info *data_move_info, u8 *buffer)
{
	int ret;
	struct ufs_hba *hba = shost_priv(dev->host);
	struct ufs_rw_buffer_vcmd vcmd = { 0 };
	u8 *response = NULL;

	ufshcd_clear_data_move_response();

	vcmd.buffer_id = VCMD_WRITE_DATA_MOVE_BUFFER_ID;
	vcmd.buffer_len = DATA_MOVE_DATA_LENGTH;
	vcmd.retries = 1; /* data move cmd send only once */
	vcmd.buffer = buffer;

	ufschd_data_move_prepare_buf(dev, data_move_info, buffer);

	ret = ufshcd_write_buffer_vcmd_retry(dev, &vcmd);
	if (ret) {
		dev_err(hba->dev, "%s: write buffer fail %d\n", __func__, ret);

		return ret;
	}

	response = ufshcd_get_data_move_response();
	if (!response) {
		dev_err(hba->dev, "%s: get response fail\n", __func__);

		return -EFAULT;
	}

	ufshcd_get_data_move_info(hba, data_move_info, response);

	return 0;
}

int ufshcd_dev_data_move(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info)
{
	int ret;
	struct ufs_hba *hba = shost_priv(sdev->host);
	unsigned char *buffer = ufshcd_get_data_move_send_buffer();

	if (!buffer) {
		dev_err(hba->dev, "%s: send buff null\n", __func__);

		return -ENOMEM;
	}

	ufshcd_data_move_lock();

	if (ufshcd_rw_buffer_is_enabled(hba))
		ret = ufshcd_dev_data_move_by_write_buff(sdev, data_move_info,
			buffer);
	else
		ret = ufshcd_dev_data_move_by_query(sdev, data_move_info,
			buffer);

	ufshcd_data_move_unlock();

	return ret;
}

static int ufshcd_slc_mode_configuration(struct ufs_hba *hba,
	int *status)
{
	int ret;
	struct ufs_query_res response;
	struct ufs_query_vcmd cmd = { 0 };
	u8 *response_buff = NULL;

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_VENDOR_SLC_MODE_CONFIGURATION;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.response = &response;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret) {
		dev_err(hba->dev, "%s failed, err=%d\n", __func__, ret);

		return ret;
	}

	response_buff = (u8 *)&response.upiu_res;
	*status = response_buff[QUERY_SLC_STATUS_OFFSET];

	return ret;
}

int ufshcd_dev_slc_mode_configuration(
	struct scsi_device *sdev, int *status)
{
	int err;
	struct ufs_hba *hba = shost_priv(sdev->host);

	err = ufshcd_slc_mode_configuration(hba, status);
	if (unlikely(err))
		dev_err(hba->dev,
			"unistore slc mode configfaile err=%d status=%d\n",
			err, *status);

	return err;
}

int ufshcd_dev_sync_read_verify(struct scsi_device *sdev,
	struct stor_dev_sync_read_verify_info *verify_info)
{
	int err;
	struct ufs_hba *hba = shost_priv(sdev->host);
	int buf_size = PAGE_SIZE;
	struct ufs_query_vcmd cmd = { 0 };
	unsigned char *buf = kzalloc(PAGE_SIZE, __GFP_NOFAIL);

	if (!buf)
		return -ENOMEM;

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_SYNC_READ_VERIFY;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.buf_len = buf_size;
	cmd.desc_buf = buf;
	cmd.reserved_osf =
		__cpu_to_be16((verify_info->stream_id << 8) & 0xFF00);
	cmd.reserved_osf |=
		__cpu_to_be16(verify_info->error_injection & 0x00FF);
	cmd.value = __cpu_to_be32(verify_info->cp_verify_l4k);
	cmd.reserved[0] = __cpu_to_be32(verify_info->cp_open_l4k);
	cmd.reserved[1] = __cpu_to_be32(verify_info->cp_cache_l4k);

	err = ufshcd_query_vcmd_retry(hba, &cmd);

	verify_info->verify_info.next_to_be_verify_4k_lba =
		get_unaligned_be32(buf + VERIFY_INFO_NEXT_TO_BE_VERIFY_OFFSET);
	verify_info->verify_info.next_available_write_4k_lba =
		get_unaligned_be32(buf + VERIFY_INFO_NEXT_AVAILABLE_OFFSET);

	verify_info->verify_info.lun_info = buf[QUERY_INFO_LUN_INFO_OFFSET];
	verify_info->verify_info.verify_done_status =
			buf[QUERY_VERIFY_DONE_STATUS_OFFSET];
	verify_info->verify_info.verify_fail_reason =
			buf[QUERY_VERIFY_FAIL_REASON_OFFSET];
	verify_info->verify_info.pu_size =
			get_unaligned_be16(buf + QUERY_PU_SIZE_OFFSET);

	if (unlikely(err))
		dev_err(hba->dev,
			"%s: execute sync read verify cmd failed! %d\n",
			__func__, err);

	kfree(buf);

	return err;
}

#ifdef CONFIG_MAS_DEBUG_FS
int ufshcd_dev_rescue_block_inject_data(
	struct scsi_device *sdev, unsigned int lba)
{
	int ret;
	struct ufs_hba *hba = shost_priv(sdev->host);
	struct ufs_query_vcmd cmd = { 0 };
	u8 rsv_osf = ufshcd_scsi_to_upiu_lun(sdev->lun);

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_RESCUE_BLOCK_INJECT_DATA;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.reserved_osf = __cpu_to_be16((u16)rsv_osf << 8);
	cmd.reserved[1] = __cpu_to_be32(lba);

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s failed, err=%d\n", __func__, ret);

	return ret;
}

int ufshcd_dev_bad_block_error_inject(
	struct scsi_device *sdev, unsigned char bad_slc_cnt,
	unsigned char bad_tlc_cnt)
{
	int ret;
	struct ufs_hba *hba = shost_priv(sdev->host);
	struct ufs_query_vcmd cmd = { 0 };

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = QUERY_BAD_BLOCK_ERROR_INJECT;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.reserved_osf =
		__cpu_to_be16(((bad_slc_cnt << 8) & 0xFF00) | bad_tlc_cnt);

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s failed, err = %d\n", __func__, ret);

	return ret;
}
#endif
