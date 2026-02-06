/*
 * battery_fault.h
 *
 * huawei battery fault driver
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

#ifndef _BATTERY_FAULT_H_
#define _BATTERY_FAULT_H_

#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/pm_wakeup.h>
#include <chipset_common/hwpower/coul/coul_interface.h>

#define BAT_FAULT_NORMAL_CUTOFF_VOL  3150
#define BAT_FAULT_SLEEP_CUTOFF_VOL   3350
#define BAT_FAULT_CUTOFF_VOL_OFFSET  10
#define BAT_FAULT_CUTOFF_VOL_FILTERS 3
#define BAT_FAULT_CUTOFF_VOL_PERIOD  1000
#define BAT_FAULT_LOW_TEMP_THLD      (-50)
#define BAT_FAULT_ABNORMAL_DELTA_SOC 20
#define BAT_FAULT_DSM_BUF_SIZE       256

struct bat_fault_config {
	int vol_cutoff_normal;
	int vol_cutoff_sleep;
	int vol_cutoff_low_temp;
	int vol_cutoff_filter_cnt;
};

struct bat_fault_data {
	int vol_cutoff_sign;
	int vol_cutoff_used;
};

struct bat_fault_ops {
	void (*notify)(unsigned int);
};

struct bat_fault_device {
	struct device *dev;
	struct delayed_work fault_work;
	struct notifier_block coul_event_nb;
	struct bat_fault_config config;
	struct bat_fault_data data;
	struct wakeup_source wake_lock;
	const struct bat_fault_ops *ops;
};

#ifdef CONFIG_HUAWEI_BATTERY_FAULT
int bat_fault_register_ops(const struct bat_fault_ops *ops);
int bat_fault_is_cutoff_vol(void);
#else
static inline int bat_fault_register_ops(const struct bat_fault_ops *ops)
{
	return -1;
}

static inline int bat_fault_is_cutoff_vol(void)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_BATTERY_FAULT */

#endif /* _BATTERY_FAULT_H_ */
