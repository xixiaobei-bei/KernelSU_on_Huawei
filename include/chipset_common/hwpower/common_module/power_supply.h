/*
 * power_supply.h
 *
 * power supply for power module
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

#ifndef _POWER_SUPPLY_H_
#define _POWER_SUPPLY_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/power_supply.h>

#define POWER_SUPPLY_DEFAULT_STATUS           1
#define POWER_SUPPLY_DEFAULT_HEALTH           POWER_SUPPLY_HEALTH_UNKNOWN
#define POWER_SUPPLY_DEFAULT_PRESENT          1
#define POWER_SUPPLY_DEFAULT_ONLINE           1
#define POWER_SUPPLY_DEFAULT_TECHNOLOGY       POWER_SUPPLY_TECHNOLOGY_UNKNOWN
#define POWER_SUPPLY_DEFAULT_TEMP             250
#define POWER_SUPPLY_DEFAULT_CYCLE_COUNT      1
#define POWER_SUPPLY_DEFAULT_LIMIT_FCC        4000
#define POWER_SUPPLY_DEFAULT_VOLTAGE_NOW      4000
#define POWER_SUPPLY_DEFAULT_VOLTAGE_MAX      4400
#define POWER_SUPPLY_DEFAULT_CURRENT_NOW      470
#define POWER_SUPPLY_DEFAULT_CAPACITY         50
#define POWER_SUPPLY_DEFAULT_CAPACITY_LEVEL   POWER_SUPPLY_CAPACITY_LEVEL_NORMAL
#define POWER_SUPPLY_DEFAULT_CAPACITY_RM      2000
#define POWER_SUPPLY_DEFAULT_CAPACITY_FCC     4000
#define POWER_SUPPLY_DEFAULT_CHARGE_FULL      4000
#define POWER_SUPPLY_DEFAULT_CHARGE_COUNTER   3500000
#define POWER_SUPPLY_DEFAULT_CHARGE_NOW       2000
#define POWER_SUPPLY_DEFAULT_BATID            200
#define POWER_SUPPLY_DEFAULT_BRAND            "default"

enum power_supply_prop_type {
	POWER_SUPPLY_PROP_BEGIN = 0,
	POWER_SUPPLY_PROP_BAT = POWER_SUPPLY_PROP_BEGIN,
	POWER_SUPPLY_PROP_RAW_BAT,
	POWER_SUPPLY_PROP_ASSITST_BAT,
	POWER_SUPPLY_PROP_MAINS,
	POWER_SUPPLY_PROP_USB,
	POWER_SUPPLY_PROP_WIRELESS,
	POWER_SUPPLY_PROP_END,
};

enum power_supply_bat_mode {
	POWER_SUPPLY_BAT_REGIST_MODE = 0,
	POWER_SUPPLY_BAT_BYPASS_MODE,
};

struct power_supply_ops {
	const char *type_name;
	int (*get_status_prop)(void);
	int (*get_health_prop)(void);
	int (*get_present_prop)(void);
	int (*get_online_prop)(void);
	int (*get_technology_prop)(void);
	int (*get_temp_prop)(void);
	int (*get_cycle_count_prop)(void);
	int (*get_limit_fcc_prop)(void);
	int (*get_voltage_now_prop)(void);
	int (*get_voltage_max_prop)(void);
	int (*get_voltage_max_design_prop)(void);
	int (*get_current_now_prop)(void);
	int (*get_current_avg_prop)(void);
	int (*get_capacity_prop)(void);
	int (*get_capacity_level_prop)(void);
	int (*get_capacity_rm_prop)(void);
	int (*get_capacity_fcc_prop)(void);
	int (*get_charge_full_prop)(void);
	int (*get_charge_counter_prop)(void);
	int (*get_charge_full_design_prop)(void);
	int (*get_charge_now_prop)(void);
	int (*get_batid_prop)(void);
	const char *(*get_brand_prop)(void);
	int (*set_voltage_max_prop)(int);
};

struct power_supply_dev {
	struct device *dev;
	struct device_node *np;
	struct power_supply *bat;
	struct power_supply *raw_bat;
	struct power_supply *assist_bat;
	struct power_supply *mains;
	struct power_supply *usb;
	struct power_supply *wireless;
	int support_bat;
	int support_raw_bat;
	int support_assist_bat;
	int support_mains;
	int support_usb;
	int support_wireless;
	enum power_supply_bat_mode bat_mode;
	unsigned int total_ops;
	struct power_supply_ops *p_ops[POWER_SUPPLY_PROP_END];
};

#ifdef CONFIG_HUAWEI_POWER_SUPPLY
int power_supply_ops_register(struct power_supply_ops *ops);
#else
static inline int power_supply_ops_register(struct power_supply_ops *ops)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_POWER_SUPPLY */

#endif /* _POWER_SUPPLY_H_ */
