/*
 * battery_core.h
 *
 * huawei battery core features
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

#ifndef _BATTERY_CORE_H_
#define _BATTERY_CORE_H_

#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/notifier.h>

#define BAT_CORE_UNIT_UTOM              1000
#define BAT_CORE_WORK_PARA_COL          3
#define BAT_CORE_WORK_PARA_ROW          6
#define BAT_CORE_WORK_INTERVAL_NORMAL   10000
#define BAT_CORE_WORK_INTERVAL_MAX      30000
#define BAT_CORE_CAPACITY_FULL          100
#define BAT_CORE_COLD_TEMP              (-200)
#define BAT_CORE_OVERHEAT_TEMP          600

struct bat_core_capacity_level {
	int min_cap;
	int max_cap;
	int level;
};

struct bat_core_monitor_para {
	int min_cap;
	int max_cap;
	int interval;
};

struct bat_core_config {
	int voltage_now_scale;
	int voltage_max_scale;
	int charge_fcc_scale;
	int charge_rm_scale;
	int coul_type;
	int work_para_cols;
	struct bat_core_monitor_para work_para[BAT_CORE_WORK_PARA_ROW];
};

struct bat_core_data {
	int exist;
	int charge_status;
	int health;
	int ui_capacity;
	int capacity_level;
	int temp_now;
	int cycle_count;
	int fcc;
	int voltage_max_now;
	int capacity_rm;
};

struct bat_core_device {
	struct device *dev;
	int work_interval;
	struct delayed_work monitor_work;
	struct mutex data_lock;
	struct notifier_block event_nb;
	struct bat_core_config config;
	struct bat_core_data data;
};

#endif /* _BATTERY_CORE_H_ */
