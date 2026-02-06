/*
 * series batt speaker smart charger driver
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


#ifndef _SERIES_BATT_SPEAKER_SMART_CHARGER_H
#define _SERIES_BATT_SPEAKER_SMART_CHARGER_H

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <huawei_platform/power/huawei_charger.h>

#define SMART_CHARGING_WORK_TIMEOUT    1000

struct smart_charge_device_info {
	struct device *dev;
	struct charge_device_info *charge_di;
	struct delayed_work charge_work;
};

#ifdef CONFIG_HUAWEI_SPEAKER_CHARGER
int series_batt_speaker_smart_charge_start_charging(struct charge_device_info *di);
int series_batt_speaker_smart_charge_stop_charging(struct charge_device_info *di);
#else
static inline int series_batt_speaker_smart_charge_start_charging(struct charge_device_info *di)
{
	return -1;
}

static inline int series_batt_speaker_smart_charge_stop_charging(struct charge_device_info *di)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_SPEAKER_CHARGER */

#endif /* _SERIES_BATT_SPEAKER_SMART_CHARGER_H */
