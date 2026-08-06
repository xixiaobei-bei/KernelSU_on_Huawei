/*
 * power_event.h
 *
 * event for power module
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

#ifndef _POWER_EVENT_H_
#define _POWER_EVENT_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define POWER_EVENT_RD_BUF_SIZE  128
#define POWER_EVENT_WR_BUF_SIZE  128

enum power_event_sysfs_type {
	POWER_EVENT_SYSFS_BEGIN = 0,
	POWER_EVENT_SYSFS_TRIGGER = POWER_EVENT_SYSFS_BEGIN,
	POWER_EVENT_SYSFS_END,
};

struct power_event_dev {
	struct device *dev;
	struct kobject *sysfs_ne;
};

#endif /* _POWER_EVENT_H_ */
