/*
 * huawei_battery_capacity.c
 *
 * huawei battery capacity
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

#include <linux/types.h>
#include <linux/err.h>
#include <linux/power_supply.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_battery_capacity.h>

#define HWLOG_TAG huawei_battery_capacity
HWLOG_REGIST();

static int huawei_battery_get_prop(enum power_supply_property psp,
			union power_supply_propval *val)
{
	int ret;
	struct power_supply *psy = NULL;

	psy = power_supply_get_by_name(HUAWEI_BATTERY);
	if (!psy)
		return -ENODEV;

	ret = power_supply_get_property(psy, psp, val);
	power_supply_put(psy);
	return ret;
}

static int huawei_battery_get_cap(int *cap)
{
	int ret;
	union power_supply_propval val;

	ret = huawei_battery_get_prop(POWER_SUPPLY_PROP_CAPACITY, &val);
	if (!ret)
		*cap = val.intval;

	return ret;
}

static int huawei_battery_get_health(int *health)
{
	int ret;
	union power_supply_propval val;

	ret = huawei_battery_get_prop(POWER_SUPPLY_PROP_HEALTH, &val);
	if (!ret)
		*health = val.intval;

	return ret;
}

int huawei_battery_capacity(void)
{
	int cap = 0;

	if (huawei_battery_get_cap(&cap))
		return hisi_battery_capacity();
	else
		return cap;
}

int huawei_battery_health(void)
{
	int health = 0;

	if (huawei_battery_get_health(&health))
		return hisi_battery_health();
	else
		return health;
}
