/*
 * power_calibration.h
 *
 * calibration for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_CALIBRATION_H_
#define _POWER_CALIBRATION_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/list.h>

#define POWER_CALI_RD_BUF_SIZE           32
#define POWER_CALI_WR_BUF_SIZE           64
#define POWER_CALI_MAX_OPS               32

enum power_cali_sysfs_type {
	POWER_CALI_SYSFS_BEGIN = 0,
	POWER_CALI_SYSFS_DC_CUR_A = POWER_CALI_SYSFS_BEGIN,
	POWER_CALI_SYSFS_DC_CUR_B,
	POWER_CALI_SYSFS_AUX_SC_CUR_A,
	POWER_CALI_SYSFS_AUX_SC_CUR_B,
	POWER_CALI_SYSFS_DC_SAVE,
	POWER_CALI_SYSFS_DC_SHOW,
	POWER_CALI_SYSFS_COUL_CUR_A,
	POWER_CALI_SYSFS_COUL_CUR_B,
	POWER_CALI_SYSFS_COUL_VOL_A,
	POWER_CALI_SYSFS_COUL_VOL_B,
	POWER_CALI_SYSFS_COUL_CUR,
	POWER_CALI_SYSFS_COUL_VOL,
	POWER_CALI_SYSFS_COUL_MODE,
	POWER_CALI_SYSFS_COUL_SAVE,
	POWER_CALI_SYSFS_END,
};

enum power_cali_user {
	POWER_CALI_USER_BEGIN = 0,
	POWER_CALI_USER_ATCMD = POWER_CALI_USER_BEGIN,
	POWER_CALI_USER_HIDL,
	POWER_CALI_USER_SHELL,
	POWER_CALI_USER_END,
};

struct power_cali_ops {
	struct list_head node;
	const char *name;
	void *dev_data;
	int (*set_data)(unsigned int, int, void *);
	int (*get_data)(unsigned int, int *, void *);
	int (*set_mode)(int, void *);
	int (*get_mode)(int *, void *);
	int (*save_data)(void *);
	int (*show_data)(char *, unsigned int, void *);
};

struct power_cali_dev {
	struct device *dev;
};

int power_cali_ops_register(struct power_cali_ops *ops);
int power_cali_common_get_data(const char *name,
	unsigned int offset, int *data);

#endif /* _POWER_CALIBRATION_H_ */
