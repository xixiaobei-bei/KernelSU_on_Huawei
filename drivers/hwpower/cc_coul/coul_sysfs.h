/*
 * coul_sysfs.h
 *
 * sysfs of coul driver
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

#ifndef _COUL_SYSFS_H_
#define _COUL_SYSFS_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define COUL_RD_BUF_SIZE                32
#define COUL_WR_BUF_SIZE                32
#define COUL_PSY_NAME_SIZE              16
#define COUL_BAT_BRAND_SIZE             16

enum coul_sysfs_type {
	COUL_SYSFS_BEGIN = 0,
	COUL_SYSFS_GAUGELOG_HEAD = COUL_SYSFS_BEGIN,
	COUL_SYSFS_GAUGELOG,
	COUL_SYSFS_END,
};

struct coul_sysfs_dev {
	struct device *dev;
	char psy_name[COUL_PSY_NAME_SIZE];
};

#endif /* _COUL_SYSFS_H_ */
