/*
 * power_ui.h
 *
 * ui display for power module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_UI_H_
#define _POWER_UI_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define POWER_UI_RD_BUF_SIZE  32
#define POWER_UI_WR_BUF_SIZE  32

enum power_ui_op_user {
	POWER_UI_OP_USER_BEGIN = 0,
	POWER_UI_OP_USER_SHELL = POWER_UI_OP_USER_BEGIN, /* for shell */
	POWER_UI_OP_USER_SYSTEM_UI, /* for system ui */
	POWER_UI_OP_USER_END,
};

enum power_ui_sysfs_type {
	POWER_UI_SYSFS_BEGIN = 0,
	POWER_UI_SYSFS_CABLE_TYPE = POWER_UI_SYSFS_BEGIN,
	POWER_UI_SYSFS_MAX_POWER,
	POWER_UI_SYSFS_WL_OFF_POS,
	POWER_UI_SYSFS_WL_FAN_STATUS,
	POWER_UI_SYSFS_WL_COVER_STATUS,
	POWER_UI_SYSFS_WATER_STATUS,
	POWER_UI_SYSFS_HEATING_STATUS,
	POWER_UI_SYSFS_END,
};

enum power_ui_ne_list {
	POWER_UI_NE_BEGIN = 0,
	POWER_UI_NE_CABLE_TYPE = POWER_UI_NE_BEGIN,
	POWER_UI_NE_MAX_POWER, /* maximum power of adapter */
	POWER_UI_NE_WL_OFF_POS, /* off position of wireless rx */
	POWER_UI_NE_WL_FAN_STATUS,
	POWER_UI_NE_WL_COVER_STATUS,
	POWER_UI_NE_WATER_STATUS,
	POWER_UI_NE_HEATING_STATUS,
	POWER_UI_NE_DEFAULT,
	POWER_UI_NE_END,
};

struct power_ui_info {
	int cable_type;
	int max_power;
	int wl_off_pos;
	int wl_fan_status;
	int wl_cover_status;
	int water_status;
	int heating_status;
};

struct power_ui_dev {
	struct device *dev;
	struct notifier_block nb;
	struct power_ui_info info;
};

void power_ui_event_notify(unsigned long event, void *data);

#endif /* _POWER_UI_H_ */
