/*
 * power_devices_info.h
 *
 * power devices driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_DEVICES_INFO_H_
#define _POWER_DEVICES_INFO_H_

#include <linux/list.h>
#include <linux/device.h>
#include <linux/workqueue.h>

struct power_devices_info_data {
	struct list_head info_node;
	const char *dev_name;
	unsigned int dev_id;
	unsigned int ver_id;
};

struct power_devices_info_dev {
	struct device *dev;
	struct delayed_work dmd_report;
};

#ifdef CONFIG_HUAWEI_POWER_DEVICES_INFO
struct power_devices_info_data *power_devices_info_register(void);
#else
static inline struct power_devices_info_data *power_devices_info_register(void)
{
	return NULL;
}
#endif /* CONFIG_HUAWEI_POWER_DEVICES_INFO */

#endif /* _POWER_DEVICES_INFO_H_ */
