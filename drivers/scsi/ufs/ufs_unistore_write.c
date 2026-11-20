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
#include <linux/atomic.h>

#include "ufshcd.h"
#include "ufs_func.h"
#include "ufshcd-kirin-interface.h"
#include "ufs_vcmd_proc.h"
#include "ufs_unistore_read.h"

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

#define MAX_DM_RECOVERY_NUM 10

struct ufshcd_data_move_ctrl {
	atomic_t in_process_cnt;
	u8 *send_buffer;
	bool response_flag;
	bool scsi_err_flag;
	u8 recovery_cnt;
	u8 repeat_cnt;
	u8 stream_id;
	u8 response[DATA_MOVE_RESPONSE_LEN];
	struct stor_dev_pwron_info stor_info;
	struct stor_dev_data_move_info data_move_info;
	struct scsi_device *sdev;
	struct work_struct recovery_work;
	struct work_struct repeat_work;
};

struct ufshcd_data_move_ctrl g_data_move_ctl[DATA_MOVE_STREAM_NUM];
static struct workqueue_struct *g_data_move_recovery_workqueue = NULL;

static int ufshcd_dev_data_move_by_write_buff(
	struct scsi_device *dev,
	struct stor_dev_data_move_info *data_move_info, u8 stream_id);

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

static void ufshcd_dev_data_move_recovery_fail(u8 stream_id, u8 reason)
{
	struct stor_dev_verify_info verify_info = { 0 };
	stor_dev_data_move_done_fn *done = NULL;
	void *private_data = NULL;

	/* Fail:1, Succes:0 */
	verify_info.verify_done_status = 1;
	/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
	verify_info.verify_fail_reason = reason;

	pr_err("%s verify_fail_reason %u\n", __func__, verify_info.verify_fail_reason);

	done = g_data_move_ctl[stream_id].data_move_info.done_info.done;
	private_data = g_data_move_ctl[stream_id].data_move_info.done_info.private_data;
	g_data_move_ctl[stream_id].data_move_info.done_info.done = NULL;
	atomic_set(&g_data_move_ctl[stream_id].in_process_cnt, 0);

	if (done)
		done(verify_info, private_data);
}

static void ufshcd_dev_data_move_recovery_work(struct work_struct *work)
{
	int ret;
	struct ufshcd_data_move_ctrl *data_move_ctl =
		container_of(work, struct ufshcd_data_move_ctrl, recovery_work);

	if (data_move_ctl->stream_id >= DATA_MOVE_STREAM_NUM) {
		pr_err("%s: stream_id %u is err\n", __func__, data_move_ctl->stream_id);
		return;
	}

retry:
	if (data_move_ctl->recovery_cnt > MAX_DM_RECOVERY_NUM) {
		/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
		ufshcd_dev_data_move_recovery_fail(data_move_ctl->stream_id, 4);
		pr_err("%s recovery_cnt is max\n", __func__);
		return;
	}

	data_move_ctl->recovery_cnt++;
	pr_err("%s recovery_cnt %u\n", __func__, data_move_ctl->recovery_cnt);
	ret = ufshcd_dev_pwron_info_sync(data_move_ctl->sdev,
		&data_move_ctl->stor_info, 0);
	if (ret)
		goto retry;
}

static void ufshcd_dev_data_move_recovery(u8 stream_id)
{
	if (g_data_move_recovery_workqueue)
		queue_work(g_data_move_recovery_workqueue,
			&g_data_move_ctl[stream_id].recovery_work);
	else
		/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
		ufshcd_dev_data_move_recovery_fail(stream_id, 4);
}

static void ufshcd_dev_data_move_repeat_work(struct work_struct *work)
{
	int ret;
	struct ufshcd_data_move_ctrl *data_move_ctl =
		container_of(work, struct ufshcd_data_move_ctrl, repeat_work);

	if (data_move_ctl->stream_id >= DATA_MOVE_STREAM_NUM) {
		pr_err("%s: stream_id %u is err\n", __func__, data_move_ctl->stream_id);
		return;
	}

retry:
	if (data_move_ctl->repeat_cnt > MAX_DM_RECOVERY_NUM) {
		/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
		ufshcd_dev_data_move_recovery_fail(data_move_ctl->stream_id, 4);
		pr_err("%s recovery_cnt is max\n", __func__);
		return;
	}

	data_move_ctl->repeat_cnt++;
	data_move_ctl->data_move_info.error_injection = 0;
	pr_err("%s repeat_cnt %u\n", __func__, data_move_ctl->repeat_cnt);
	ret = ufshcd_dev_data_move_by_write_buff(data_move_ctl->sdev,
		&data_move_ctl->data_move_info, data_move_ctl->stream_id);
	if (ret)
		goto retry;
}

static void ufshcd_dev_data_move_repeat(u8 stream_id)
{
	if (g_data_move_recovery_workqueue)
		queue_work(g_data_move_recovery_workqueue,
			&g_data_move_ctl[stream_id].repeat_work);
	else
		/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
		ufshcd_dev_data_move_recovery_fail(stream_id, 4);
}

static void ufshcd_dev_data_move_pwron_done(int err, u8 pwron_type)
{
	u8 stream_id;

	if ((pwron_type != DM_STRAM0_TYPE) && (pwron_type != DM_STRAM1_TYPE)) {
		pr_err("%s pwron_type is err %u\n", __func__, pwron_type);
		return;
	}

	stream_id = pwron_type - DM_STRAM0_TYPE;
	if (err) {
		pr_err("%s datamove recovery\n", __func__);
		ufshcd_dev_data_move_recovery(stream_id);
		return;
	}

	if (g_data_move_ctl[stream_id].data_move_info.dest_4k_lba !=
		g_data_move_ctl[stream_id].stor_info.dm_stream_addr[stream_id])
		/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
		ufshcd_dev_data_move_recovery_fail(stream_id, 5);
	else
		ufshcd_dev_data_move_repeat(stream_id);
}

int ufshcd_dev_data_move_ctl_init(void)
{
	u8 stream_id;
	u8 free_stream_id;

	g_data_move_recovery_workqueue = alloc_workqueue("proc_data_move_recovery",
		WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (unlikely(!g_data_move_recovery_workqueue))
		return -EFAULT;

	for (stream_id = 0; stream_id < DATA_MOVE_STREAM_NUM; stream_id++) {
		atomic_set(&g_data_move_ctl[stream_id].in_process_cnt, 0);
		g_data_move_ctl[stream_id].stream_id = stream_id;
		g_data_move_ctl[stream_id].response_flag = false;
		g_data_move_ctl[stream_id].scsi_err_flag = false;
		g_data_move_ctl[stream_id].stor_info.done_info.done =
			ufshcd_dev_data_move_pwron_done;
		g_data_move_ctl[stream_id].stor_info.done_info.pwron_type =
			stream_id + DM_STRAM0_TYPE;
		g_data_move_ctl[stream_id].data_move_info.done_info.done = NULL;
		g_data_move_ctl[stream_id].data_move_info.done_info.private_data = NULL;
		INIT_WORK(&g_data_move_ctl[stream_id].recovery_work,
			ufshcd_dev_data_move_recovery_work);
		INIT_WORK(&g_data_move_ctl[stream_id].repeat_work,
			ufshcd_dev_data_move_repeat_work);
		g_data_move_ctl[stream_id].send_buffer =
			kzalloc(DATA_MOVE_DATA_LENGTH, __GFP_NOFAIL);
		if (!g_data_move_ctl[stream_id].send_buffer)
			goto out;
	}

	return 0;

out:
	for (free_stream_id = 0; free_stream_id < stream_id; free_stream_id++)
		kfree(g_data_move_ctl[free_stream_id].send_buffer);

	return -ENOMEM;
}

static u8 *ufshcd_get_data_move_send_buffer(u8 stream_id)
{
	return g_data_move_ctl[stream_id].send_buffer;
}

static u8 *ufshcd_get_data_move_response(u8 stream_id)
{
	if (g_data_move_ctl[stream_id].response_flag)
		return g_data_move_ctl[stream_id].response;
	else
		return NULL;
}

static int ufshcd_data_move_init(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info, u8 stream_id)
{
	if (atomic_cmpxchg(&g_data_move_ctl[stream_id].in_process_cnt, 0, 1))
		return -EACCES;

	g_data_move_ctl[stream_id].response_flag = false;
	g_data_move_ctl[stream_id].scsi_err_flag = false;
	g_data_move_ctl[stream_id].sdev = sdev;
	g_data_move_ctl[stream_id].recovery_cnt = 0;
	g_data_move_ctl[stream_id].repeat_cnt = 0;
	g_data_move_ctl[stream_id].stream_id = stream_id;
	memset(g_data_move_ctl[stream_id].response, 0, DATA_MOVE_RESPONSE_LEN);
	memcpy(&g_data_move_ctl[stream_id].data_move_info, data_move_info,
		sizeof(struct stor_dev_data_move_info));

	return 0;
}

void ufshcd_dev_data_move_done(struct scsi_cmnd *cmd,
	struct ufshcd_lrb *lrbp)
{
	int ocs;
	u8 stream_id;
	struct utp_upiu_rsp *ucd_rsp_ptr = lrbp->ucd_rsp_ptr;

	if ((cmd->req.cmd[RW_BUFFER_OPCODE_OFFSET] == WRITE_BUFFER) &&
		(cmd->req.cmd[RW_BUFFER_BUFFER_ID_OFFSET] ==
			VCMD_WRITE_DATA_MOVE_BUFFER_ID) && ucd_rsp_ptr) {
		stream_id = cmd->req.cmd[RW_BUFFER_2ND_RESERVED_OFFSET];
		if (stream_id >= DATA_MOVE_STREAM_NUM) {
			pr_err("%s stream_id %u is err\n", __func__, stream_id);
			return;
		}
		memcpy(g_data_move_ctl[stream_id].response, ucd_rsp_ptr->sr.reserved,
			DATA_MOVE_RESPONSE_LEN);
		g_data_move_ctl[stream_id].response_flag = true;

		if (lrbp->is_hufs_utrd)
			ocs = le32_to_cpu(lrbp->hufs_utr_descriptor_ptr->header.dword_2) & MASK_OCS;
		else
			ocs = le32_to_cpu(lrbp->utr_descriptor_ptr->header.dword_2) & MASK_OCS;

		if (ocs == OCS_INVALID_COMMAND_STATUS)
			g_data_move_ctl[stream_id].scsi_err_flag = false;
		else
			g_data_move_ctl[stream_id].scsi_err_flag = true;
	}
}

static void ufshcd_get_data_move_info(
	struct stor_dev_verify_info *verify_info, const unsigned char *buf)
{
	verify_info->next_to_be_verify_4k_lba =
		get_unaligned_be32(buf + VERIFY_INFO_NEXT_TO_BE_VERIFY_OFFSET);
	verify_info->next_available_write_4k_lba =
		get_unaligned_be32(buf + VERIFY_INFO_NEXT_AVAILABLE_OFFSET);

	verify_info->verify_done_status =
		buf[WRITE_BUFFER_VERIFY_DONE_STATUS_OFFSET];
	verify_info->verify_fail_reason =
		buf[WRITE_BUFFER_VERIFY_FAIL_REASON_OFFSET];
	verify_info->pu_size =
		get_unaligned_be16(buf + WRITE_BUFFER_PU_SIZE_OFFSET);
}

static void ufshcd_dev_data_move_io_done(struct request *rq,
	blk_status_t err)
{
	struct stor_dev_verify_info verify_info = { 0 };
	struct scsi_request *req = scsi_req(rq);
	u8 stream_id = req->cmd[RW_BUFFER_2ND_RESERVED_OFFSET];
	stor_dev_data_move_done_fn *done = NULL;
	void *private_data = NULL;

	scsi_unistore_execute_done(rq);

	if (stream_id >= DATA_MOVE_STREAM_NUM) {
		pr_err("%s stream_id %u is err\n", __func__, stream_id);
		return;
	}

	if (err) {
		if (unlikely(!g_data_move_ctl[stream_id].scsi_err_flag)) {
			pr_err("%s datamove recovery\n", __func__);
			ufshcd_dev_data_move_recovery(stream_id);
			return;
		}

		/* Fail:1, Succes:0 */
		verify_info.verify_done_status = 1;
		/* 1:verify Fail, 2: UNC fail, 3:Program Fail 4:IO Fail 5:reset err */
		verify_info.verify_fail_reason = 4;

		pr_err("%s verify_fail_reason %u\n", __func__, verify_info.verify_fail_reason);
	} else {
		ufshcd_get_data_move_info(&verify_info,
			g_data_move_ctl[stream_id].response);

		if ((verify_info.next_to_be_verify_4k_lba >=
			g_data_move_ctl[stream_id].data_move_info.done_info.start_addr) &&
			(verify_info.next_available_write_4k_lba >=
			g_data_move_ctl[stream_id].data_move_info.done_info.start_addr)) {
			verify_info.next_to_be_verify_4k_lba -=
				g_data_move_ctl[stream_id].data_move_info.done_info.start_addr;
			verify_info.next_available_write_4k_lba -=
				g_data_move_ctl[stream_id].data_move_info.done_info.start_addr;
		} else {
			pr_err("%s response_flag %u, response addr is err\n",
				__func__, g_data_move_ctl[stream_id].response_flag);
		}
	}

	done = g_data_move_ctl[stream_id].data_move_info.done_info.done;
	private_data = g_data_move_ctl[stream_id].data_move_info.done_info.private_data;
	g_data_move_ctl[stream_id].data_move_info.done_info.done = NULL;
	atomic_set(&g_data_move_ctl[stream_id].in_process_cnt, 0);

	if (done)
		done(verify_info, private_data);
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
	struct stor_dev_data_move_info *data_move_info, u8 stream_id)
{
	int ret;
	struct ufs_hba *hba = shost_priv(dev->host);
	struct ufs_rw_buffer_vcmd vcmd = { 0 };
	unsigned char *buffer = ufshcd_get_data_move_send_buffer(stream_id);

	if (!buffer) {
		dev_err(hba->dev, "%s: send buff null\n", __func__);
		return -ENOMEM;
	}

	vcmd.buffer_id = VCMD_WRITE_DATA_MOVE_BUFFER_ID;
	vcmd.buffer_len = DATA_MOVE_DATA_LENGTH;
	vcmd.retries = 1; /* data move cmd send only once */
	vcmd.buffer = buffer;
	vcmd.reserved_2nd[0] = stream_id;
	if (data_move_info->done_info.done)
		vcmd.done = ufshcd_dev_data_move_io_done;
	else
		vcmd.done = NULL;

	ufschd_data_move_prepare_buf(dev, data_move_info, buffer);
	ret = ufshcd_write_buffer_vcmd_retry(dev, &vcmd);
	if (ret) {
		dev_err(hba->dev, "%s: write buffer fail %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int ufshcd_dev_data_move(struct scsi_device *sdev,
	struct stor_dev_data_move_info *data_move_info)
{
	int ret;
	struct ufs_hba *hba = shost_priv(sdev->host);
	u8 stream_id = data_move_info->dest_stream_id;
	u8 *response = NULL;

	if (stream_id >= DATA_MOVE_STREAM_NUM) {
		dev_err(hba->dev, "%s: dest_stream_id %u is err\n", __func__, stream_id);
		return -EINVAL;
	}

	ret = ufshcd_data_move_init(sdev, data_move_info, stream_id);
	if (ret) {
		dev_err(hba->dev, "%s: ufshcd_data_move_init %d\n", __func__, ret);

		return ret;
	}

	ret = ufshcd_dev_data_move_by_write_buff(sdev, data_move_info, stream_id);
	if (ret) {
		atomic_set(&g_data_move_ctl[stream_id].in_process_cnt, 0);
		return ret;
	}

	if (!data_move_info->done_info.done) {
		response = ufshcd_get_data_move_response(stream_id);
		if (!response) {
			dev_err(hba->dev, "%s: get response fail\n", __func__);
			return -EFAULT;
		}

		ufshcd_get_data_move_info(&data_move_info->verify_info, response);
		atomic_set(&g_data_move_ctl[stream_id].in_process_cnt, 0);
	}

	return 0;
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
