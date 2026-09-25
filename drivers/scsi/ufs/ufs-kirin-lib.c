/*
 * ufs-kirin-lib.c
 *
 * lib for ufs
 *
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "ufshcd :" fmt

#include "ufshcd.h"
#include <linux/hisi/kirin_partition.h>

static struct ufs_hba *ufs_hba_tmp;

void ufs_get_boot_partition_type(struct ufs_hba *hba)
{
	u32 boot_partition_type;
	int ret;

	ufs_hba_tmp = hba;
	ret = ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_READ_ATTR,
	    QUERY_ATTR_IDN_BOOT_LU_EN, 0, 0, &boot_partition_type);
	if (ret) {
		dev_err(hba->dev, "%s: Failed getting boot partition type\n",
		    __func__);
		ufs_boot_partition_type = XLOADER_A;
		return;
	}

	ufs_boot_partition_type = boot_partition_type;

	dev_info(hba->dev, "%s: ufs boot partition type is %s\n", __func__,
	    (ufs_boot_partition_type == 0x1) ? "XLOADER_A" : "xloader_B");
}

int ufs_set_boot_partition_type(int boot_partition_type)
{
	int ret;

	ret = ufshcd_query_attr(ufs_hba_tmp, UPIU_QUERY_OPCODE_WRITE_ATTR,
	    QUERY_ATTR_IDN_BOOT_LU_EN, 0, 0, &boot_partition_type);
	if (ret)
		dev_err(ufs_hba_tmp->dev, "%s: Failed setting boot partition type\n",
		    __func__);

	return ret;
}
