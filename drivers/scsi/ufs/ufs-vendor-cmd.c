/*
 * ufs-vendor-cmd.c
 *
 * ufs device command implements for different device vendor
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "ufs-vendor-cmd.h"

#include <scsi/scsi_device.h>
#include <scsi/ufs/ufs.h>
#include <scsi/ufs/ioctl.h>
#include <asm/unaligned.h>

#include "ufshcd.h"
#include "ufs_func.h"
#include "ufshcd-kirin-interface.h"
#include "ufs_vcmd_proc.h"

#ifdef CONFIG_SCSI_UFS_UNISTORE
#include "ufs_unistore.h"
#endif

/*
 * entries: 4B, max value 32
 * lba(4B), len(4B),
 * ...
 * lba(4B), len(4B)
 */
#define MAX_WRITE_PGR_SIZE (4 + 32 * 8)
/* HI1861 FSR info default size, 4k byte */
#define HI1861_FSR_INFO_SIZE 4096

static int ufshcd_tz_ctrl(struct scsi_device *sdev, int desc_id, uint8_t index)
{
	int err;
	struct ufs_hba *hba = NULL;
	struct ufs_query_vcmd cmd = { 0 };

	if (!sdev || (index != 0 && index != 1))
		return -EINVAL;

#ifdef CONFIG_HISI_DEBUG_FS
	if (desc_id < TZ_RETURN_FLAG || desc_id >= TZ_DESC_MAX)
#else
	if (desc_id != TZ_RETURN_FLAG && desc_id != TZ_FORCE_CLOSE_FLAG)
#endif
		return -EINVAL;

	hba = shost_priv(sdev->host);
	if (!strstarts(hba->model, UFS_MODEL_THOR920))
		return -EPERM;

	cmd.opcode = UPIU_QUERY_OPCODE_TZ_CTRL;
	cmd.idn = (u8)desc_id;
	cmd.index = index;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;

	err = ufshcd_query_vcmd_retry(hba, &cmd);

	return err;
}

static int ufshcd_get_tz_info(struct ufs_hba *hba, u8 *buf, u32 buf_len)
{
	int ret;
	struct ufs_query_vcmd cmd = { 0 };

	if (hba->tz_version < TZ_VER_1_0)
		return -EPERM;

	cmd.opcode = UPIU_QUERY_OPCODE_READ_TZ_DESC;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
	cmd.desc_buf = buf;
	cmd.buf_len = buf_len;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s: Failed get turbo zone info ret %d",
			__func__, ret);

	return ret;
}

static int ufshcd_tz_get_status(struct ufs_hba *hba, u8 *buf, u32 buf_len)
{
	*buf = hba->tz_version;
	return 0;
}

static int ufshcd_tz_cap_info(struct ufs_hba *hba, u8 *buf, u32 buf_len)
{
	int ret;
	struct tz_cap_info *cap_info = (struct tz_cap_info *)buf;
	struct ufs_query_vcmd cmd = { 0 };

	if (hba->tz_version < TZ_VER_2_0 || !buf)
		return -EINVAL;

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
	cmd.idn = QUERY_TZ_IDN_CAP_INFO;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
	cmd.desc_buf = buf;
	cmd.buf_len = buf_len;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret) {
		dev_err(hba->dev, "%s: Failed get turbo zone cap info ret %d",
			__func__, ret);
		return ret;
	}

	cap_info->marked_slc_blks = be32_to_cpu(cap_info->marked_slc_blks);
	cap_info->marked_tlc_blks = be32_to_cpu(cap_info->marked_tlc_blks);
	cap_info->remain_slc_blks = be32_to_cpu(cap_info->remain_slc_blks);
	cap_info->slc_exist_status = be32_to_cpu(cap_info->slc_exist_status);

	return 0;
}

static int ufshcd_tz_blk_info(struct ufs_hba *hba, u8 lun, u8 *buf, u32 buf_len)
{
	int ret;
	struct tz_blk_info *blk_info = (struct tz_blk_info *)buf;
	struct ufs_query_vcmd cmd = { 0 };
	int query_len = blk_info->len;

	if (hba->tz_version < TZ_VER_2_0 || !blk_info->flags || !query_len)
		return -EINVAL;

	/* buffer size is 5 at least, to fill addr and lun */
	if (blk_info->buf_len < (sizeof(u32) + 1))
		return -EINVAL;

	put_unaligned_le32(blk_info->addr, blk_info->flags);
	blk_info->flags[sizeof(u32)] = lun;

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_READ;
	cmd.idn = QUERY_TZ_IDN_BLK_INFO;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
	cmd.desc_buf = blk_info->flags;
	cmd.buf_len = query_len;
	cmd.value = get_unaligned_be32(blk_info->flags);
	cmd.lun = cmd.desc_buf[sizeof(u32)];

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s: Failed get turbo zone blk info ret %d",
			__func__, ret);

	return ret;
}

/* This interface is offered to f2fs */
static int ufshcd_tz_query(struct scsi_device *sdev, u32 type, u8 *buf,
			   u32 buf_len)
{
	int ret;
	struct ufs_hba *hba = NULL;

	if (!sdev || !buf || !buf_len)
		return -EFAULT;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EINVAL;

	switch (type) {
	case TZ_READ_DESC:
		ret = ufshcd_get_tz_info(hba, buf, buf_len);
		break;
	case TZ_STATUS:
		ret = ufshcd_tz_get_status(hba, buf, buf_len);
		break;
	case TZ_CAP_INFO:
		ret = ufshcd_tz_cap_info(hba, buf, buf_len);
		break;
	case TZ_BLK_INFO:
		ret = ufshcd_tz_blk_info(hba, sdev->lun, buf, buf_len);
		break;
	default:
		ret = -EINVAL;
	}

	if (ret)
		dev_err(hba->dev, "%s: Failed get turbo zone info ret %d",
			__func__, ret);

	return ret;
}

void ufshcd_tz_op_register(struct ufs_hba *hba)
{
	struct scsi_host_template *sht = hba->host->hostt;

	if (!sht)
		return;

	sht->tz_ctrl = ufshcd_tz_ctrl;
	sht->get_tz_info = ufshcd_tz_query;
}

static int ufshcd_tz_set_group_num(struct ufs_hba *hba, u8 *buf,
	u32 buf_len)
{
	int ret;
	struct ufs_query_vcmd cmd = { 0 };

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = SET_TZ_STREAM_ID;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.desc_buf = buf;
	cmd.buf_len = buf_len;
	cmd.has_data = true;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret)
		dev_err(hba->dev, "%s: Failed set tz stream id ret %d",
			__func__, ret);

	return ret;
}

/* Set stream id for turbo zone 2.0 */
static void ufshcd_tz_init(struct ufs_hba *hba, uint32_t feature)
{
	int ret;
	uint32_t group_num = TZ_GROUP_NUM;

	hba->tz_version = TZ_INVALID;
	if (hba->manufacturer_id != UFS_VENDOR_HI1861)
		return;
	if (strstarts(hba->model, UFS_MODEL_THOR920)) {
		hba->tz_version = TZ_VER_1_0;
		return;
	}

	if (feature & VENDOR_FEATURE_TURBO_ZONE) {
		ret = ufshcd_tz_set_group_num(hba, (u8 *)&group_num,
					      sizeof(group_num));
		if (ret)
			return;
		hba->tz_version = TZ_VER_2_0;
		dev_info(hba->dev, "support turbo zone 2.0\n");
	}
}

#ifdef CONFIG_SCSI_UFS_UNISTORE
static bool ufshcd_unistore_is_support(struct ufs_hba *hba,
	uint32_t feature)
{
	if ((hba->manufacturer_id != UFS_VENDOR_HI1861) ||
		strstarts(hba->model, UFS_MODEL_THOR920))
		return false;

	dev_info(hba->dev, "%s %x %s\n", __func__, feature, hba->model);

	return feature & VENDOR_FEATURE_UNISTORE;
}

static void ufshcd_unistore_enable_init(struct ufs_hba *hba,
	uint32_t feature)
{
	hba->host->unistore_enable = ufshcd_unistore_is_support(hba, feature);
}

static void ufshcd_vcmd_set_init(struct ufs_hba *hba,
	uint32_t feature)
{
	hba->host->vcmd_set = 0;

	if ((hba->manufacturer_id != UFS_VENDOR_HI1861) ||
		strstarts(hba->model, UFS_MODEL_THOR920))
		return;

	/* bit14 ~ bit15 stand vcmd set */
	hba->host->vcmd_set = (feature >> 14) & 0x3;
	dev_info(hba->dev, "%s 0x%x \n", __func__, hba->host->vcmd_set);
}
#endif

static void ufshcd_order_preserving_init(struct ufs_hba *hba, uint32_t feature)
{
#ifdef CONFIG_MAS_ORDER_PRESERVE
	hba->host->order_enabled = 0;
	if (hba->manufacturer_id != UFS_VENDOR_HI1861)
		return;

#ifdef CONFIG_SCSI_UFS_UNISTORE
	if (ufshcd_unistore_is_support(hba, feature))
		return;
#endif

	if (feature & VENDOR_FEATURE_ORDER) {
		hba->host->order_enabled = 1;
		dev_info(hba->dev, "support order feature\n");
	}
#endif
}

static void ufshcd_tt_support(struct ufs_hba *hba, uint32_t feature)
{
	hba->ufstt_support = feature & VENDOR_FEATURE_TURBO_TABLE;
}

/* Parse 186x feature info, device descriptor offset 0xF0 */
static void ufshcd_vendor_feature_init(struct ufs_hba *hba,
				       struct ufs_dev_desc *dev_desc)
{
	ufshcd_tt_support(hba, dev_desc->vendor_feature);
	ufshcd_tz_init(hba, dev_desc->vendor_feature);
	ufshcd_order_preserving_init(hba, dev_desc->vendor_feature);
#ifdef CONFIG_SCSI_UFS_UNISTORE
	ufshcd_unistore_enable_init(hba, dev_desc->vendor_feature);
	ufshcd_vcmd_set_init(hba, dev_desc->vendor_feature);
#endif
}

void ufshcd_get_vendor_info(struct ufs_hba *hba, struct ufs_dev_desc *dev_desc)
{
	ufshcd_vendor_feature_init(hba, dev_desc);
}

static int
ufshcd_ioctl_query_vendor_write(struct ufs_hba *hba,
				struct ufs_ioctl_query_data *ioctl_data,
				const void __user *buffer)
{
	int err;
	struct ufs_query_vcmd cmd = { 0 };

	cmd.idn = ioctl_data->idn;
	cmd.buf_len = ioctl_data->buf_size;

	if (cmd.buf_len <= 0)
		return -EINVAL;

	switch (cmd.idn) {
	case QUERY_VENDOR_WRITE_IDN_PGR:
		if (cmd.buf_len > MAX_WRITE_PGR_SIZE)
			cmd.buf_len = MAX_WRITE_PGR_SIZE;
		break;
	default:
		return -EINVAL;
	}

	cmd.desc_buf = kzalloc(cmd.buf_len, GFP_KERNEL);
	if (!cmd.desc_buf) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);
		return -ENOMEM;
	}

	err = copy_from_user(cmd.desc_buf, buffer + UFS_IOCTL_QUERY_DATA_SIZE,
			     cmd.buf_len);
	if (err) {
		dev_err(hba->dev, "%s: copy from user, err %d\n", __func__,
			err);
		goto out;
	}

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
	cmd.has_data = (cmd.idn == SET_TZ_STREAM_ID ||
				cmd.idn == QUERY_VENDOR_WRITE_IDN_PGR);

	err = ufshcd_query_vcmd_retry(hba, &cmd);
	if (err)
		dev_err(hba->dev, "%s:  ret %d", __func__, err);

out:
	kfree(cmd.desc_buf);
	return err;
}

static int ufshcd_ioctl_query_tz_ctrl(struct ufs_hba *hba,
				      struct ufs_ioctl_query_data *ioctl_data,
				      const void __user *buffer)
{
	int err;
	struct ufs_query_vcmd cmd = { 0 };

	if (!strstarts(hba->model, UFS_MODEL_THOR920))
		return -EPERM;

	err = copy_from_user(&cmd.index, buffer + UFS_IOCTL_QUERY_DATA_SIZE,
			     sizeof(u8));
	if (err) {
		dev_err(hba->dev, "%s: copy from user, err %d\n", __func__,
			err);
		return err;
	}

	cmd.opcode = UPIU_QUERY_OPCODE_TZ_CTRL;
	cmd.idn = ioctl_data->idn;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;

	err = ufshcd_query_vcmd_retry(hba, &cmd);
	if (err)
		dev_err(hba->dev, "%s:  ret %d", __func__, err);

	return err;
}

static int
ufshcd_ioctl_query_read_tz_desc(struct ufs_hba *hba,
				struct ufs_ioctl_query_data *ioctl_data,
				void __user *buffer)
{
	int err;
	struct ufs_query_vcmd cmd = { 0 };

	cmd.buf_len = TURBO_ZONE_INFO_SIZE;
	cmd.desc_buf = kzalloc(cmd.buf_len, GFP_KERNEL);
	if (!cmd.desc_buf) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);
		return -ENOMEM;
	}

	cmd.opcode = ioctl_data->opcode;
	cmd.idn = ioctl_data->idn;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;

	err = ufshcd_query_vcmd_retry(hba, &cmd);
	if (err) {
		dev_err(hba->dev, "%s:  ret %d", __func__, err);
		goto out;
	}

	ioctl_data->buf_size = (u16)((cmd.buf_len < (int)TURBO_ZONE_INFO_SIZE) ?
					cmd.buf_len : TURBO_ZONE_INFO_SIZE);
	err = copy_to_user(buffer + sizeof(struct ufs_ioctl_query_data),
			   cmd.desc_buf, ioctl_data->buf_size);
	if (err)
		dev_err(hba->dev, "%s: copy back to user err %d\n", __func__,
			err);
out:
	kfree(cmd.desc_buf);
	return err;
}

static int
ufshcd_ioctl_query_read_fsr(struct ufs_hba *hba,
	struct ufs_ioctl_query_data *ioctl_data, u8 *buffer, u16 *buffer_len)
{
	int err;
	struct ufs_query_vcmd cmd = { 0 };

	cmd.buf_len = *buffer_len;
	cmd.desc_buf = buffer;
	cmd.opcode = ioctl_data->opcode;
	cmd.idn = ioctl_data->idn;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;

	err = ufshcd_query_vcmd_retry(hba, &cmd);
	if (err) {
		dev_err(hba->dev, "%s:  ret %d", __func__, err);

		return err;
	}

	*buffer_len = (u16)((cmd.buf_len < (int)HI1861_FSR_INFO_SIZE) ?
			cmd.buf_len : HI1861_FSR_INFO_SIZE);

	return 0;
}

static int
ufshcd_ioctl_read_fsr(struct ufs_hba *hba,
	struct ufs_ioctl_query_data *ioctl_data, void __user *buffer)
{
	int ret;
	u16 buffer_len = HI1861_FSR_INFO_SIZE;
	u8 *read_buffer = kzalloc(HI1861_FSR_INFO_SIZE, GFP_KERNEL);

	if (!read_buffer) {
		dev_err(hba->dev, "%s: alloc fail\n", __func__);
		return -ENOMEM;
	}

#ifdef CONFIG_SCSI_UFS_UNISTORE
	ret = ufshcd_rw_buffer_is_enabled(hba) ?
		ufshcd_get_fsr_by_read_buffer(hba, read_buffer, buffer_len) :
		ufshcd_ioctl_query_read_fsr(hba, ioctl_data,
			read_buffer, &buffer_len);
#else
	ret = ufshcd_ioctl_query_read_fsr(hba, ioctl_data,
			read_buffer, &buffer_len);
#endif
	if (ret) {
		dev_err(hba->dev, "%s:  ret %d", __func__, ret);

		goto out;
	}

	ioctl_data->buf_size = buffer_len;
	ret = copy_to_user(buffer + sizeof(struct ufs_ioctl_query_data),
			   read_buffer, ioctl_data->buf_size);
	if (ret)
		dev_err(hba->dev, "%s: copy back to user err %d\n", __func__,
			ret);

out:
	kfree(read_buffer);

	return ret;
}

int ufshcd_ioctl_query_vcmd(struct ufs_hba *hba,
			    struct ufs_ioctl_query_data *ioctl_data,
			    void __user *buffer)
{
	int err;

	if (hba->manufacturer_id != UFS_VENDOR_HI1861)
		return -EINVAL;

	switch (ioctl_data->opcode) {
	case UPIU_QUERY_OPCODE_VENDOR_WRITE:
		err = ufshcd_ioctl_query_vendor_write(hba, ioctl_data, buffer);
		break;
	case UPIU_QUERY_OPCODE_TZ_CTRL:
		err = ufshcd_ioctl_query_tz_ctrl(hba, ioctl_data, buffer);
		break;
	case UPIU_QUERY_OPCODE_READ_TZ_DESC:
		err = ufshcd_ioctl_query_read_tz_desc(hba, ioctl_data, buffer);
		break;
	case UPIU_QUERY_OPCODE_READ_HI1861_FSR:
		err = ufshcd_ioctl_read_fsr(hba, ioctl_data, buffer);
		break;
	default:
		err = -ENOIOCTLCMD;
		break;
	}

	return err;
}

bool ufshcd_get_vcmd_tag(struct ufs_hba *hba, int *tag_out)
{
	int tag;
	bool ret = false;
	unsigned long tmp;

	if (!tag_out)
		goto out;

	do {
		tmp = ~hba->lrb_in_use;
		tag = find_last_bit(&tmp, hba->nutrs);
		if (tag >= hba->nutrs)
			goto out;
		if (!test_and_set_bit_lock(tag, &hba->lrb_in_use)) {
			if (tag == HI1861_VCMD_QUERY_TAG)
				break;
			clear_bit_unlock(tag, &hba->lrb_in_use);
			wake_up(&hba->dev_cmd.tag_wq);
		}
	} while (true);

	*tag_out = tag;
	ret = true;
out:
	return ret;
}
