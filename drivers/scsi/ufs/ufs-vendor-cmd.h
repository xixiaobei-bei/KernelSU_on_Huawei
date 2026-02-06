/*
 * ufs-vendor-cmd.h
 *
 * basic macro and data struct for hufs
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

#ifndef __UFS_VENDOR_CMD_H__
#define __UFS_VENDOR_CMD_H__

#include <scsi/ufs/ioctl.h>
#include "ufshcd.h"

#define VENDOR_FEATURE_TURBO_TABLE BIT(0)
#define VENDOR_FEATURE_ORDER       BIT(2)
#define VENDOR_FEATURE_TURBO_ZONE  BIT(3)
#define VENDOR_FEATURE_UNISTORE BIT(16)

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
void ufshcd_get_vendor_info(struct ufs_hba *hba, struct ufs_dev_desc *dev_desc);
void ufshcd_tz_op_register(struct ufs_hba *hba);
int ufshcd_ioctl_query_vcmd(struct ufs_hba *hba,
			    struct ufs_ioctl_query_data *ioctl_data,
			    void __user *buffer);
bool ufshcd_get_vcmd_tag(struct ufs_hba *hba, int *tag_out);
#else
static inline void ufshcd_get_vendor_info(struct ufs_hba *hba,
					  struct ufs_dev_desc *dev_desc){};
static inline void ufshcd_tz_op_register(struct ufs_hba *hba){};
static inline int
ufshcd_ioctl_query_vcmd(struct ufs_hba *hba,
			struct ufs_ioctl_query_data *ioctl_data,
			void __user *buffer)
{
	return -EINVAL;
}
#endif

#endif
