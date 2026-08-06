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

#ifndef _HUFS_LIB_H
#define _HUFS_LIB_H

#ifdef CONFIG_HISI_DIEID
#ifdef CONFIG_SCSI_UFSHCD
int hufs_get_dieid(char *dieid, unsigned int len);
#else
int hufs_get_dieid(char *dieid, unsigned int len)
{
	return -1;
}
#endif
#endif

void ufs_get_boot_partition_type(struct ufs_hba *hba);
int ufs_set_boot_partition_type(int boot_partition_type);

#endif /* End of Header */
