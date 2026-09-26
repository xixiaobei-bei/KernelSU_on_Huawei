/*
 * ufs-vendor-mode.h
 *
 * data struct and macro for vendor specififor vendor specific
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


#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
#ifndef LINUX_UFS_VENOR_MODE_H
#define LINUX_UFS_VENOR_MODE_H
#include "ufshcd.h"

#define UFS_IOCTL_VENDOR_PACKAGE_COUNT_MAX 10
#define UFS_IOCTL_VENDOR_CDB_LEN 16
#define UFS_VENDOR_DATA_SIZE_MAX 0x100
#define UFS_IOCTL_VENDOR_CMD 6

struct ufs_vendor_cmd {
	/* vendor_cdb has 16 slots */
	unsigned char vendor_cdb[16];
	int data_flag;
	char *buf;
};

struct ufs_ioctl_vendor_state_t {
	struct ufs_vendor_cmd *vendor_cmd;
	int cmd_count;
};

int ufs_ioctl_vendor_package(struct ufs_hba *hba,
	struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state,
	const void __user *buffer);
int ufs_ioctl_vendor_package_tick(struct ufs_hba *hba, struct scsi_device *dev,
	struct ufs_ioctl_vendor_state_t *ufs_vendor_ioctl_state,
	void __user *buffer);
#endif
#endif
