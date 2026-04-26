/*
 * ufs-vendor-mode.c
 *
 * vendor specific ioctl
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "ufshcd :" fmt

#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
/* enter vendor mode */
#include "ufs-vendor-mode.h"
#include "ufshcd.h"

DEFINE_MUTEX(s_health_info_lock);

static int ufs_cmd_judge(unsigned char vendor_cdb)
{
	if ((vendor_cdb == FORMAT_UNIT) || (vendor_cdb == READ_6) ||
		(vendor_cdb == READ_10) || (vendor_cdb == READ_16) ||
		(vendor_cdb == UNMAP) || (vendor_cdb == WRITE_6) ||
		(vendor_cdb == WRITE_10) || (vendor_cdb == WRITE_16))
		return 1;
	else
		return 0;
}

static int ufs_get_user_cmd(struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p,
	struct ufs_hba *hba, const void __user *buffer)
{
	char data_flag = 0;
	int err;

	err = copy_from_user(&(ufs_vendor_ioctl_state_p->vendor_cmd
		[ufs_vendor_ioctl_state_p->cmd_count].vendor_cdb),
		buffer, (unsigned long)UFS_IOCTL_VENDOR_CDB_LEN);
	if (err) {
		dev_err(hba->dev, "%s: copy cdb error\n", __func__);
		return -EFAULT;
	}

	err = copy_from_user(&data_flag, buffer + UFS_IOCTL_VENDOR_CDB_LEN, 1ul);
	if (err) {
		dev_err(hba->dev, "%s: copy data flag\n", __func__);
		return -EFAULT;
	}
	if (data_flag) {
		ufs_vendor_ioctl_state_p
			->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count]
			.buf = kzalloc((size_t)(PAGE_SIZE), GFP_KERNEL);
		if (ufs_vendor_ioctl_state_p
			->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count]
			.buf == NULL) {
			return -ENOMEM;
		}
		ufs_vendor_ioctl_state_p
			->vendor_cmd[ufs_vendor_ioctl_state_p->cmd_count]
			.data_flag = 1;
	}
	ufs_vendor_ioctl_state_p->cmd_count++;

	return err;
}

int ufs_ioctl_vendor_package(struct ufs_hba *hba,
	struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p,
	const void __user *buffer)
{
	int err = 0;

	if (!ufs_vendor_ioctl_state_p) {
		dev_err(hba->dev, "ufs_vendor_ioctl_state_p addr is NULL\n");
		return -EINVAL;
	}
	if (!buffer) {
		dev_err(hba->dev, "%s: User buffer is NULL!\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&s_health_info_lock);
	dev_info(hba->dev, "%s: ioctl vendor cmd buffering\n", __func__);
	if (ufs_vendor_ioctl_state_p->cmd_count >
		UFS_IOCTL_VENDOR_PACKAGE_COUNT_MAX - 1) {
		dev_err(hba->dev, "%s: ioctl vendor cmd over count, %d\n",
			__func__, ufs_vendor_ioctl_state_p->cmd_count);
		err = -EINVAL;
		goto out;
	}
	if (!ufs_vendor_ioctl_state_p->vendor_cmd) {
		ufs_vendor_ioctl_state_p->vendor_cmd =
			kzalloc((size_t)(sizeof(struct ufs_vendor_cmd) *
					UFS_IOCTL_VENDOR_PACKAGE_COUNT_MAX),
				GFP_KERNEL);
		if (!ufs_vendor_ioctl_state_p->vendor_cmd) {
			dev_err(hba->dev, "%s: vendor cmd mem alloc error\n",
				__func__);
			err = -ENOMEM;
			goto out;
		}
		ufs_vendor_ioctl_state_p->cmd_count = 0;
	}
	if (ufs_get_user_cmd(ufs_vendor_ioctl_state_p, hba, buffer)) {
		err = -ENOMEM;
		goto out;
	}
out:
	mutex_unlock(&s_health_info_lock);

	return err;
}

static void ufs_ioctl_error_handle(struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p)
{
	int index;

	if (ufs_vendor_ioctl_state_p->vendor_cmd) {
		for (index = 0; index < ufs_vendor_ioctl_state_p->cmd_count;
			index++) {
			if (ufs_vendor_ioctl_state_p->vendor_cmd[index].data_flag)
				ufs_vendor_ioctl_state_p->vendor_cmd[index].data_flag = 0;
			if (ufs_vendor_ioctl_state_p->vendor_cmd[index].buf) {
				kfree(ufs_vendor_ioctl_state_p->vendor_cmd[index].buf);
				ufs_vendor_ioctl_state_p->vendor_cmd[index].buf = NULL;
			}
		}
		kfree(ufs_vendor_ioctl_state_p->vendor_cmd);
		ufs_vendor_ioctl_state_p->vendor_cmd = NULL;
	}
	ufs_vendor_ioctl_state_p->cmd_count = 0;
}

static int vendor_data_copy(struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p,
	struct ufs_hba *hba, struct scsi_device *dev, void __user *buffer)
{
	struct ufs_vendor_cmd *vendor_cmd = NULL;
	char *buf = NULL;
	int index;
	int err = 0;

	for (index = 0; index < ufs_vendor_ioctl_state_p->cmd_count; index++) {
		vendor_cmd = &(ufs_vendor_ioctl_state_p->vendor_cmd[index]);
		if (ufs_cmd_judge(vendor_cmd->vendor_cdb[0])) {
			dev_err(hba->dev, "%s: not allow ioctl this cmd:0x%x\n",
				__func__, vendor_cmd->vendor_cdb[0]);
			err = -EINVAL;
			return err;
		}
		if (vendor_cmd->data_flag) {
			if (!vendor_cmd->buf) {
				dev_err(hba->dev, "data bufffer is null\n");
				err = -EINVAL;
				return err;
			}
			buf = vendor_cmd->buf;
		}
		err = ufshcd_send_vendor_scsi_cmd(
			hba, dev, vendor_cmd->vendor_cdb, (void *)buf);
		if (err) {
			dev_err(hba->dev, "%s: send cmd[0x%x] error %d\n", __func__,
			vendor_cmd->vendor_cdb[0], err);
			return err;
		}
		if (buf) {
			err = copy_to_user(
				buffer + (unsigned int)(index *
						UFS_VENDOR_DATA_SIZE_MAX),
				buf, (unsigned long)UFS_VENDOR_DATA_SIZE_MAX);
			if (err) {
				dev_err(hba->dev, "%s: copy to user error %d\n",
				__func__, err);
				return err;
			}
		}
	}

	return err;
}

int ufs_ioctl_vendor_package_tick(struct ufs_hba *hba, struct scsi_device *dev,
	struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state_p, void __user *buffer)
{
	int err = 0;
#ifndef HEALTH_INFO_FACTORY_MODE
	static unsigned long s_timeout;
#endif
	if (!hba || !dev) {
		pr_err("hba addr is NULL\n");
		return -EINVAL;
	}
	if (!ufs_vendor_ioctl_state_p) {
		dev_err(hba->dev, "ufs_vendor_ioctl_state_p addr is NULL\n");
		return -EINVAL;
	}
	if (!buffer) {
		dev_err(hba->dev, "%s: User buffer is NULL!\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&s_health_info_lock);
#ifndef HEALTH_INFO_FACTORY_MODE
	if (time_after(s_timeout, jiffies)) {
		dev_err(hba->dev, "%s: wait for half an hour and try again\n", __func__);
		err = -EBUSY;
		goto out;
	} else {
		s_timeout = jiffies + msecs_to_jiffies(1800000);
	}
#endif
	dev_info(hba->dev, "%s: send all vendor cmd\n", __func__);
	if (ufs_vendor_ioctl_state_p->vendor_cmd == NULL) {
		dev_err(hba->dev, "%s: vendor package cmd not init\n", __func__);
		err = -EINVAL;
		goto out;
	}
	scsi_block_requests(hba->host);
	err = wait_for_ufs_all_complete(hba, 5000);
	if (err) {
		dev_err(hba->dev, "%s: wait for req cleard time out, ret == %d\n", __func__, err);
		goto error_out;
	}
	err = vendor_data_copy(ufs_vendor_ioctl_state_p, hba, dev, buffer);
	if (err)
		goto error_out;
error_out:
	scsi_unblock_requests(hba->host);
out:
	ufs_ioctl_error_handle(ufs_vendor_ioctl_state_p);
	mutex_unlock(&s_health_info_lock);
	return err;
}
#endif
