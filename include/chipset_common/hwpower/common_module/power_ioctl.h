/*
 * power_ioctl.h
 *
 * ioctl for power module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_IOCTL_H_
#define _POWER_IOCTL_H_

#define POWER_IOCTL_ITEM_NAME_MAX    32
#define POWER_IOCTL_DIR_PERMS        0555
#define POWER_IOCTL_FILE_PERMS       0660

typedef long (*power_ioctl_rw)(void *, unsigned int, unsigned long);

struct power_ioctl_attr {
	char name[POWER_IOCTL_ITEM_NAME_MAX];
	void *dev_data;
	power_ioctl_rw rw;
	struct list_head list;
};

void power_ioctl_ops_register(char *name, void *dev_data, power_ioctl_rw rw);

#endif /* _POWER_IOCTL_H_ */
