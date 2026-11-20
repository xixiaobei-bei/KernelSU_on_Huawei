/*
 * huawei_battery_capacity.c
 *
 * huawei battery capacity
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

#include <linux/types.h>
#include <linux/err.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_battery_capacity.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>

#define HWLOG_TAG huawei_battery_capacity
HWLOG_REGIST();

int huawei_battery_capacity(void)
{
	int cap = 0;

	if (power_supply_get_int_property_value(HUAWEI_BATTERY,
		POWER_SUPPLY_PROP_CAPACITY, &cap))
		return coul_drv_battery_capacity();

	return cap;
}

int huawei_battery_health(void)
{
	int health = 0;

	if (power_supply_get_int_property_value(HUAWEI_BATTERY,
		POWER_SUPPLY_PROP_HEALTH, &health))
		return coul_drv_battery_health();

	return health;
}
