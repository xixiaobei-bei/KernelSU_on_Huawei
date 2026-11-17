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

#include <scsi/scsi_device.h>
#include <scsi/ufs/ufs.h>
#include <asm/unaligned.h>

#include "ufs_func.h"
#include "ufshcd-kirin-interface.h"
#include "ufs_vcmd_proc.h"

#define QUERY_DESC_LEN_OFFSET 0x0
#define QUERY_DESC_IDN_OFFSET 0x1

#define QUERY_DESC_SECTION_SIZE 4
#define QUERY_DESC_SECTION_SIZE_OFFSET 0x12

#define QUERY_DESC_MAX_PARA_DIE_NUM_OFFSET 0xf
#define QUERY_DESC_TLC_PROGRAM_SIZE_OFFSET 0x10
#define QUERY_DESC_SLC_PROGRAM_SIZE_OFFSET 0x16

#define QUERY_DESC_TOTAL_SEG_FTL_BLOCK_OFFSET 0x6
#define QUERY_DESC_TLC_BAD_BLOCK_OFFSET 0x19

#define QUERY_DESC_ENABLE_UNISTORE_OFFSET 0x2
#define QUERY_DESC_NEW_FORMAT_OFFSET 0x3

#define MAX_STREAM_NUM 5
#define MAX_DM_STREAM_NUM 2
#define STREAM_ADDR_OFFSET 0
#define DM_STREAM_ADDR_OFFSET 28
#define STREAM_LUN_OFFSET 48
#define DM_STREAM_LUN_OFFSET 55
#define IO_SLC_MODE_STATUS_OFFSET 60
#define DM_SLC_MODE_STATUS_OFFSET 61
#define RESCUE_SEG_CNT_OFFSET 64
#define RESCUE_SEG_OFFSET 68

#define PWRON_INFO_QUERY_PE_LIMIT_STATUS_OFFSET 8
#define PWRON_INFO_READ_BUFFER_OFFSET 8
#define PWRON_INFO_PE_LIMIT_STATUS_OFFSET 4

#define MAPPING_PARTITION_OFFSET 4
#define MAPPING_PARTITION_SELECTOR 1
#define MAPPING_PARTITION_LUN 3

#define MAX_OOB_ENTRY_CNT 512
#define READ_OOB_NUM_OFFSET 4

#define CAPACITY_128G_4KBASE_93 0x1DCC000
#define CAPACITY_256G_4KBASE_93 0x3B97000
#define CAPACITY_512G_4KBASE_93 0x772E000
#define CAPACITY_LBA_TO_GBASE 18

#define RC16_CDB_LEN 16
#define RC16_LEN 32
#define READ_CAPACITY_RETRIES 5
#define SCSI_CMD_TIMEOUT 30
#define READ_CAPACITY_LBA_LEN 8
#define SECTOR_SIZE_OFFSET 8

#define PWRON_INFO_SYNC_LEN 4096
#define STREAM_OOB_INFO_FETCH_LEN 4096

static int ufshcd_get_pwron_info(u8 *desc_buf,
	struct stor_dev_pwron_info *stor_info,
	unsigned int rescue_seg_size)
{
	int offset;
	unsigned int i;

	offset = STREAM_ADDR_OFFSET;
	for (i = 0; i < MAX_STREAM_NUM; ++i) {
		stor_info->dev_stream_addr[i] =
			get_unaligned_be32(desc_buf + offset);
		offset += 4;
	}

	offset = DM_STREAM_ADDR_OFFSET;
	for (i = 0; i < MAX_DM_STREAM_NUM; ++i) {
		stor_info->dm_stream_addr[i] =
			get_unaligned_be32(desc_buf + offset);
		offset += 4;
	}

	offset = STREAM_LUN_OFFSET;
	for (i = 0; i < MAX_STREAM_NUM; ++i)
		stor_info->stream_lun_info[i] = desc_buf[offset++];

	offset = DM_STREAM_LUN_OFFSET;
	for (i = 0; i < MAX_DM_STREAM_NUM; ++i)
		stor_info->dm_lun_info[i] = desc_buf[offset++];

	stor_info->io_slc_mode_status = desc_buf[IO_SLC_MODE_STATUS_OFFSET];
	stor_info->dm_slc_mode_status = desc_buf[DM_SLC_MODE_STATUS_OFFSET];

	stor_info->rescue_seg_cnt =
		get_unaligned_be16(desc_buf + RESCUE_SEG_CNT_OFFSET);

	if (rescue_seg_size / sizeof(unsigned int) <
		stor_info->rescue_seg_cnt) {
		stor_info->rescue_seg_cnt = 0;

		return -ENOMEM;
	}

	offset = RESCUE_SEG_OFFSET;
	for (i = 0; i < stor_info->rescue_seg_cnt; ++i) {
		stor_info->rescue_seg[i] =
			get_unaligned_be32(desc_buf + offset);
		offset += 4;
	}

	return 0;
}

static int ufshcd_dev_pwron_info_sync_by_query(struct ufs_hba *hba,
	struct stor_dev_pwron_info *stor_info, unsigned int rescue_seg_size,
	u8 *buffer, int buffer_len)
{
	int ret;
	struct ufs_query_res response;
	struct ufs_query_vcmd cmd = { 0 };
	u8 *response_buff = NULL;

	cmd.buf_len = buffer_len;
	cmd.desc_buf = buffer;
	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
	cmd.idn = QUERY_PWRON_INFO_SYNC;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
	cmd.response = &response;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret) {
		dev_err(hba->dev, "%s: ufshcd_query_vcmd_retry fail %d\n",
			__func__, ret);

		return ret;
	}

	response_buff = (u8 *)&response.upiu_res;
	stor_info->pe_limit_status =
		response_buff[PWRON_INFO_QUERY_PE_LIMIT_STATUS_OFFSET];

	ret = ufshcd_get_pwron_info(cmd.desc_buf, stor_info,
		rescue_seg_size);
	if (ret)
		dev_err(hba->dev, "%s: ufshcd_get_pwron_info fail %d\n",
			__func__, ret);

	return ret;
}

static int ufshcd_dev_pwron_info_sync_by_read_buffer(
	struct scsi_device *dev, struct stor_dev_pwron_info *stor_info,
	unsigned int rescue_seg_size, u8 *buffer, u16 buffer_len)
{
	int ret;
	struct ufs_hba *hba = shost_priv(dev->host);
	struct ufs_rw_buffer_vcmd vcmd = { 0 };

	vcmd.buffer_id = VCMD_READ_PWRON_INFO_BUFFER_ID;
	vcmd.buffer_len = buffer_len;
	vcmd.retries = VCMD_REQ_RETRIES;
	vcmd.buffer = buffer;

	ret = ufshcd_read_buffer_vcmd_retry(dev, &vcmd);
	if (ret) {
		dev_err(hba->dev, "%s: ufshcd_read_buffer_vcmd_retry fail %d\n",
			__func__, ret);

		return ret;
	}

	stor_info->pe_limit_status = *(buffer +
		PWRON_INFO_PE_LIMIT_STATUS_OFFSET);

	ret = ufshcd_get_pwron_info(buffer + PWRON_INFO_READ_BUFFER_OFFSET,
		stor_info, rescue_seg_size);
	if (ret)
		dev_err(hba->dev, "%s: ufshcd_get_pwron_info fail %d\n",
			__func__, ret);

	return ret;
}

int ufshcd_dev_pwron_info_sync(struct scsi_device *dev,
	struct stor_dev_pwron_info *stor_info,
	unsigned int rescue_seg_size)
{
	struct ufs_hba *hba = shost_priv(dev->host);
	u8 *buffer = NULL;
	int ret;

	if (!stor_info || !stor_info->rescue_seg)
		return -EINVAL;

	buffer = kzalloc(PWRON_INFO_SYNC_LEN, __GFP_NOFAIL);
	if (!buffer) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);

		return -ENOMEM;
	}

	if (ufshcd_rw_buffer_is_enabled(hba))
		ret = ufshcd_dev_pwron_info_sync_by_read_buffer(dev, stor_info,
			rescue_seg_size, buffer, PWRON_INFO_SYNC_LEN);
	else
		ret = ufshcd_dev_pwron_info_sync_by_query(hba, stor_info,
			rescue_seg_size, buffer, PWRON_INFO_SYNC_LEN);

	kfree(buffer);

	return ret;
}

int ufshcd_dev_read_section_size(struct scsi_device *sdev,
	unsigned int *section_size)
{
	int ret;
	struct ufs_hba *hba = NULL;
	int buf_len = QUERY_DESC_SECTION_SIZE;
	u8 desc_buf[QUERY_DESC_SECTION_SIZE];

	if (!sdev)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_DEVICE_UNISTORE, 0,
		QUERY_DESC_SECTION_SIZE_OFFSET, desc_buf, buf_len);
	pm_runtime_put_sync(hba->dev);
	if (unlikely(ret)) {
		*section_size = 0;
		dev_err(hba->dev, "%s: read section size err! %d\n",
			__func__, ret);

		goto out;
	}
	*section_size = get_unaligned_be32(desc_buf);

out:
	return ret;
}

int ufshcd_dev_read_lrb_in_use(struct scsi_device *sdev,
	unsigned long *lrb_in_use)
{
	struct ufs_hba *hba = NULL;

	if (!sdev)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	*lrb_in_use = hba->lrb_in_use;

	return 0;
}

int ufshcd_dev_read_section_size_hba(struct ufs_hba *hba,
	unsigned int *section_size)
{
	int ret;
	int buf_len = QUERY_DESC_SECTION_SIZE;
	u8 desc_buf[QUERY_DESC_SECTION_SIZE];

	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_DEVICE_UNISTORE, 0,
		QUERY_DESC_SECTION_SIZE_OFFSET, desc_buf, buf_len);
	pm_runtime_put_sync(hba->dev);
	if (unlikely(ret)) {
		*section_size = 0;
		dev_err(hba->dev, "%s: read section size err! <%d>\r\n",
			__func__, ret);

		goto out;
	}
	*section_size = get_unaligned_be32(desc_buf);

out:
	return ret;
}

int ufshcd_dev_read_mapping_partition(
	struct scsi_device *sdev,
	struct stor_dev_mapping_partition *mapping_info)
{
	int ret;
	int i;
	struct ufs_hba *hba = NULL;
	int buf_len = QUERY_DESC_UINT_UNISTORE_MAX_SIZE;
	u8 desc_buf[QUERY_DESC_UINT_UNISTORE_MAX_SIZE];
	int desc_offset;

	if (!sdev || !mapping_info)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
		QUERY_DESC_IDN_UNIT_UNISTORE, MAPPING_PARTITION_LUN,
		MAPPING_PARTITION_SELECTOR, desc_buf, &buf_len);
	pm_runtime_put_sync(hba->dev);
	if (unlikely(ret)) {
		dev_err(hba->dev, "%s: read section num err! %d\n",
			__func__, ret);

		goto out;
	}

	desc_offset = MAPPING_PARTITION_OFFSET;
	for (i = 0; i < PARTITION_TYPE_MAX; ++i) {
		mapping_info->partion_start[i] =
			get_unaligned_be16(desc_buf + desc_offset);
		desc_offset += 2;

		mapping_info->partion_size[i] =
			get_unaligned_be16(desc_buf + desc_offset);
		desc_offset += 2;
	}

out:
	return ret;
}

int ufshcd_dev_config_mapping_partition(
	struct scsi_device *sdev,
	struct stor_dev_mapping_partition *mapping_info)
{
	int ret;
	int i;
	struct ufs_hba *hba = NULL;
	int buf_len = QUERY_DESC_UINT_UNISTORE_MAX_SIZE;
	u8 desc_buf[QUERY_DESC_UINT_UNISTORE_MAX_SIZE];
	int desc_offset;

	if (!sdev || !mapping_info)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	pm_runtime_get_sync(hba->dev);

	desc_buf[QUERY_DESC_LEN_OFFSET] = QUERY_DESC_UINT_UNISTORE_MAX_SIZE;
	desc_buf[QUERY_DESC_IDN_OFFSET] = QUERY_DESC_IDN_UNIT_UNISTORE;
	desc_buf[QUERY_DESC_ENABLE_UNISTORE_OFFSET] = true;
	desc_buf[QUERY_DESC_NEW_FORMAT_OFFSET] = true;
	desc_offset = MAPPING_PARTITION_OFFSET;
	for (i = 0; i < PARTITION_TYPE_MAX; ++i) {
		put_unaligned_be16(mapping_info->partion_start[i],
			desc_buf + desc_offset);
		desc_offset += 2;

		put_unaligned_be16(mapping_info->partion_size[i],
			desc_buf + desc_offset);
		desc_offset += 2;
	}

	ret = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_WRITE_DESC,
		QUERY_DESC_IDN_UNIT_UNISTORE, MAPPING_PARTITION_LUN,
		MAPPING_PARTITION_SELECTOR, desc_buf, &buf_len);
	if (unlikely(ret))
		dev_err(hba->dev, "%s: write desc err! %d\n",
			__func__, ret);

	pm_runtime_put_sync(hba->dev);

	return ret;
}

int ufshcd_stream_oob_info_fetch_by_query(struct ufs_hba *hba,
	struct stor_dev_stream_info stream_info, unsigned int oob_entry_cnt,
	u8 *buff, int buffer_len)
{
	struct ufs_query_vcmd cmd = { 0 };

	cmd.buf_len = buffer_len;
	cmd.desc_buf = buff;
	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
	cmd.idn = QUERY_OOB_INFO_FETCH;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
	cmd.reserved_osf = __cpu_to_be16((stream_info.stream_id |
		((u16)stream_info.dm_stream << 7) << 8) & 0xFF00);
	cmd.value = __cpu_to_be32(stream_info.stream_start_lba);
	cmd.reserved[0] =
		__cpu_to_be32((oob_entry_cnt << 16) & 0xFFFF0000);

	return ufshcd_query_vcmd_retry(hba, &cmd);
}

int ufshcd_stream_oob_info_fetch_by_read_buffer(
	struct scsi_device *dev, struct stor_dev_stream_info stream_info,
	unsigned int oob_entry_cnt, u8 *buffer, u16 buffer_len)
{
	int ret;
	struct ufs_hba *hba = shost_priv(dev->host);
	struct ufs_rw_buffer_vcmd vcmd = { 0 };

	vcmd.buffer_id = VCMD_READ_OOB_BUFFER_ID;
	vcmd.buffer_len = buffer_len;
	vcmd.retries = VCMD_REQ_RETRIES;
	vcmd.buffer = buffer;

	put_unaligned_be32(stream_info.stream_start_lba, vcmd.reserved_2nd);
	put_unaligned_be16(oob_entry_cnt, vcmd.reserved_2nd +
		READ_OOB_NUM_OFFSET);

	ret = ufshcd_read_buffer_vcmd_retry(dev, &vcmd);
	if (ret) {
		dev_err(hba->dev, "%s: ufshcd_read_buffer_vcmd_retry fail %d\n",
			__func__, ret);

		return ret;
	}

	return ret;
}

static void ufshcd_get_stream_oob_info(u8 *buffer,
	unsigned int oob_entry_cnt, struct stor_dev_stream_oob_info *oob_info)
{
	unsigned int i;
	int offset = 0;

	for (i = 0; i < oob_entry_cnt; ++i) {
		oob_info[i].data_ino =
			get_unaligned_be32(buffer + offset);
		offset += 4;

		oob_info[i].data_idx =
			get_unaligned_be32(buffer + offset);
		offset += 4;
	}
}

int ufshcd_stream_oob_info_fetch(struct scsi_device *sdev,
	struct stor_dev_stream_info stream_info, unsigned int oob_entry_cnt,
	struct stor_dev_stream_oob_info *oob_info)
{
	int ret = -EINVAL;
	struct ufs_hba *hba = shost_priv(sdev->host);
	u8 *buffer = NULL;

	if (unlikely(oob_entry_cnt > MAX_OOB_ENTRY_CNT)) {
		dev_err(hba->dev, "%s: oob_entry_cnt beyond limit! %u\n",
			__func__, oob_entry_cnt);

		goto out;
	}

	buffer = kzalloc(STREAM_OOB_INFO_FETCH_LEN, __GFP_NOFAIL);
	if (!buffer) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);

		return -ENOMEM;
	}

	if (ufshcd_rw_buffer_is_enabled(hba))
		ret = ufshcd_stream_oob_info_fetch_by_read_buffer(sdev,
			stream_info, oob_entry_cnt, buffer,
			STREAM_OOB_INFO_FETCH_LEN);
	else
		ret = ufshcd_stream_oob_info_fetch_by_query(hba, stream_info,
			oob_entry_cnt, buffer, STREAM_OOB_INFO_FETCH_LEN);
	if (ret) {
		dev_err(hba->dev, "%s failed, err=%d\n", __func__, ret);

		goto out;
	}

	ufshcd_get_stream_oob_info(buffer, oob_entry_cnt, oob_info);

out:
	kfree(buffer);

	return ret;
}

int ufshcd_query_bad_block_info(struct ufs_hba *hba,
	struct stor_dev_bad_block_info *bad_block_info)
{
	int ret;
	int buf_len = QUERY_DESC_DEVICE_UNISTORE_MAX_SIZE;
	u8 desc_buf[QUERY_DESC_DEVICE_UNISTORE_MAX_SIZE];

	if (!hba)
		return -EINVAL;

	bad_block_info->tlc_total_block_num = 0;
	bad_block_info->tlc_bad_block_num = 0;

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_DEVICE_UNISTORE, 0,
		0, desc_buf, buf_len);
	if (unlikely(ret)) {
		dev_err(hba->dev, "%s: read bad block info err! %d\n",
			__func__, ret);

		goto out;
	}

	bad_block_info->tlc_total_block_num = get_unaligned_be16(desc_buf +
			QUERY_DESC_TOTAL_SEG_FTL_BLOCK_OFFSET);
	bad_block_info->tlc_bad_block_num = desc_buf[QUERY_DESC_TLC_BAD_BLOCK_OFFSET];

out:
	return ret;
}

int ufshcd_dev_get_bad_block_info(struct scsi_device *sdev,
	struct stor_dev_bad_block_info *bad_block_info)
{
	struct ufs_hba *hba = shost_priv(sdev->host);
	int ret;

	bad_block_info->tlc_total_block_num = 0;
	bad_block_info->tlc_bad_block_num = 0;

	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_query_bad_block_info(hba, bad_block_info);
	pm_runtime_put_sync(hba->dev);

	return ret;
}

void ufshcd_bad_block_exception_event_handler(
	struct ufs_hba *hba)
{
	struct Scsi_Host *host = hba->host;
	struct scsi_host_template *hostt = NULL;
	struct stor_dev_bad_block_info bad_block_info = { 0 };
	int ret;

	if (!host)
		return;

	if (!host->unistore_enable)
		return;

	ret = ufshcd_query_bad_block_info(hba, &bad_block_info);
	if (ret)
		return;

	hostt = host->hostt;
	if (!hostt || !hostt->dev_bad_block_notify)
		return;

	hostt->dev_bad_block_notify(host, &bad_block_info);
}

int ufshcd_dev_get_program_size(struct scsi_device *sdev,
	struct stor_dev_program_size *program_size)
{
	int ret;
	struct ufs_hba *hba = NULL;
	int buf_len = QUERY_DESC_DEVICE_UNISTORE_MAX_SIZE;
	u8 desc_buf[QUERY_DESC_DEVICE_UNISTORE_MAX_SIZE];

	if (!sdev || !program_size)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	program_size->tlc_program_size = 0;
	program_size->slc_program_size = 0;

	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_DEVICE_UNISTORE, 0,
		0, desc_buf, buf_len);
	pm_runtime_put_sync(hba->dev);
	if (unlikely(ret)) {
		dev_err(hba->dev, "%s: read section size err! %d\n",
			__func__, ret);

		goto out;
	}

	program_size->tlc_program_size =
		desc_buf[QUERY_DESC_MAX_PARA_DIE_NUM_OFFSET] *
		get_unaligned_be16(desc_buf +
			QUERY_DESC_TLC_PROGRAM_SIZE_OFFSET);

	program_size->slc_program_size =
		desc_buf[QUERY_DESC_MAX_PARA_DIE_NUM_OFFSET] *
		get_unaligned_be16(desc_buf +
			QUERY_DESC_SLC_PROGRAM_SIZE_OFFSET);

out:
	return ret;
}

static void ufshcd_get_op_size(unsigned long long lba, int *op_size)
{
	int capacity_gbase = 0;
	*op_size = 0;

	capacity_gbase = lba >> CAPACITY_LBA_TO_GBASE;

	if (capacity_gbase <= 128)
		*op_size = lba - CAPACITY_128G_4KBASE_93;
	else if ((capacity_gbase > 128) && (capacity_gbase <= 256))
		*op_size = lba - CAPACITY_256G_4KBASE_93;
	else if (capacity_gbase > 256)
		*op_size = lba - CAPACITY_512G_4KBASE_93;
}

int ufshcd_dev_read_op_size(struct scsi_device *dev, int *op_size)
{
	int ret = -EINVAL;
	int i;
	unsigned long long lba = 0;
	struct scsi_sense_hdr sshdr;
	unsigned char cmd[RC16_CDB_LEN] = { 0 };
	u8 buf[RC16_LEN] = { 0 };
	int retries = READ_CAPACITY_RETRIES;

	/*
	 * offset 0: opcode
	 * offset 0: service action
	 * offset 13: rc16 length
	 */
	cmd[0] = SERVICE_ACTION_IN_16;
	cmd[1] = SAI_READ_CAPACITY_16;
	cmd[13] = RC16_LEN;

	while (ret && retries--)
		ret = scsi_execute_req(dev, cmd, DMA_FROM_DEVICE,
					      buf, RC16_LEN, &sshdr,
					      SCSI_CMD_TIMEOUT * HZ,
					      READ_CAPACITY_RETRIES, NULL);

	if (ret)
		return ret;

	for (i = 0; i < READ_CAPACITY_LBA_LEN; ++i)
		lba = (lba << 8) + (unsigned long long)buf[i];

	ufshcd_get_op_size(lba, op_size);

	return ret;
}

int ufshcd_get_fsr_by_read_buffer(struct ufs_hba *hba, u8 *buffer,
	u16 buffer_len)
{
	int ret;
	struct ufs_rw_buffer_vcmd vcmd = { 0 };
	struct scsi_device *dev = hba->sdev_ufs_device;

	if (!dev) {
		dev_err(hba->dev, "%s: scsi_device is null\n", __func__);

		return -EINVAL;
	}

	vcmd.buffer_id = VCMD_READ_FSR_BUFFER_ID;
	vcmd.buffer_len = buffer_len;
	vcmd.retries = VCMD_REQ_RETRIES;
	vcmd.buffer = buffer;

	ret = ufshcd_read_buffer_vcmd_retry(dev, &vcmd);
	if (ret)
		dev_err(hba->dev, "%s: ufshcd_read_buffer_vcmd_retry fail %d\n",
			__func__, ret);

	return ret;
}

void ufshcd_add_buf_to_recovery_list(struct ufs_hba *hba)
{
	int err;
	struct stor_dev_pwron_info stor_info;
	struct request_queue *q = NULL;
	struct scsi_device *sdev = NULL;

	if (!hba->host->unistore_enable)
		return;

	q = hba->sdev_ufs_device->request_queue;
	sdev = q->queuedata;
	stor_info.rescue_seg = vmalloc(sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);
	if (unlikely(!stor_info.rescue_seg)) {
		pr_err("%s: alloc mem failed!\n", __func__);
		return;
	}

	err = ufshcd_dev_pwron_info_sync(sdev, &stor_info,
			sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);
	if (unlikely(err)) {
		pr_err("%s, pwron_info_sync failed, err = %d\n", __func__, err);
		vfree(stor_info.rescue_seg);
		return;
	}

	vfree(stor_info.rescue_seg);

	mas_blk_add_buf_to_recovery_list(q, &stor_info);
}