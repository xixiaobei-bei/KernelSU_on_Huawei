/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2010-2019. All rights reserved.
 * Description: partition table
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

#ifndef __PARTITION_AP_KERNEL_H__
#define __PARTITION_AP_KERNEL_H__

#define PTN_ERROR (-1)

#ifdef CONFIG_AB_PARTITION_TABLE
extern int ufs_set_boot_partition_type(int boot_partition_type);
#endif

enum AB_PARTITION_TYPE {
	NO_SUPPORT_AB = 0,
	XLOADER_A = 1,
	XLOADER_B = 2,
	ERROR_VALUE = 3
};

enum AB_SLOT_TYPE {
	SLOT_A = 0,
	SLOT_B = 1,
	SLOT_ERROR = 2
};

extern enum AB_PARTITION_TYPE emmc_boot_partition_type;
extern enum AB_PARTITION_TYPE ufs_boot_partition_type;

extern int flash_find_ptn_s(const char* ptn_name, char* bdev_path, unsigned int pblkname_length);
extern int flash_find_ptn(const char* ptn_name, char* bdev_path);
extern int flash_get_ptn_index(const char* bdev_path);
#endif

