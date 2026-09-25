/*
 * ufs-kirin-lib.h
 *
 * The header file for ufs-kirin-lib.c
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

#ifndef _UFS_KIRIN_LIB_H
#define _UFS_KIRIN_LIB_H

void ufs_get_boot_partition_type(struct ufs_hba *hba);
int ufs_set_boot_partition_type(int boot_partition_type);

#endif /* End of Header */
