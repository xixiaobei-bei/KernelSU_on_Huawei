/*
 * battery_temp.h
 *
 * battery temp driver
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

#ifndef _BATTERY_TEMP_H_
#define _BATTERY_TEMP_H_

#include <huawei_platform/hwpower/common_module/power_platform.h>

#define BAT_TEMP_NAME_MAX 64

enum bat_temp_id {
	BAT_TEMP_0 = 0,
	BAT_TEMP_1,
	BAT_TEMP_MIXED,
	BTB_TEMP_0 = 0,
	BTB_TEMP_1,
	BTB_TEMP_MIXED,
};

/*
 * get_rt_temp return current temperature.
 * get_temp return statistical temperature.
 */
struct bat_temp_ops {
	int (*get_rt_temp)(enum bat_temp_id, int *);
	int (*get_temp)(enum bat_temp_id, int *);
};

#ifdef CONFIG_HUAWEI_BATTERY_TEMP
int bat_temp_get_temperature(enum bat_temp_id id, int *temp);
int bat_temp_get_rt_temperature(enum bat_temp_id id, int *temp);
int bat_temp_ops_register(const char *name, struct bat_temp_ops *ops);
#else
static inline int bat_temp_get_temperature(enum bat_temp_id id, int *temp)
{
	if (!temp)
		return -1;

	*temp = power_platform_get_battery_temperature();
	return 0;
}

static inline int bat_temp_get_rt_temperature(enum bat_temp_id id, int *temp)
{
	if (!temp)
		return -1;

	*temp = power_platform_get_rt_battery_temperature();
	return 0;
}

static inline int bat_temp_ops_register(const char *name,
	struct bat_temp_ops *ops)
{
	return -ENODEV;
}
#endif /* CONFIG_HUAWEI_BATTERY_TEMP */

#endif /* _BATTERY_TEMP_H_ */
