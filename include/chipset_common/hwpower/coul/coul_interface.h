/*
 * coul_interface.h
 *
 * interface for coul driver
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

#ifndef _COUL_INTERFACE_H_
#define _COUL_INTERFACE_H_

#include <linux/power_supply.h>

enum coul_interface_type {
	COUL_TYPE_BEGIN = 0,
	COUL_TYPE_MAIN = COUL_TYPE_BEGIN,
	COUL_TYPE_AUX,
	COUL_TYPE_END,
};

struct coul_interface_ops {
	const char *type_name;
	void *dev_data;
	int (*is_coul_ready)(void *);
	int (*is_battery_exist)(void *);
	int (*get_battery_capacity)(void *);
	int (*get_battery_voltage)(void *);
	int (*get_battery_current)(void *);
	int (*get_battery_avg_current)(void *);
	int (*get_battery_temperature)(void *);
	int (*get_battery_cycle)(void *);
	int (*get_battery_fcc)(void *);
	int (*set_vterm_dec)(int, void *);
	int (*set_battery_low_voltage)(int, void *);
	int (*set_battery_last_capacity)(int, void *);
	int (*get_battery_last_capacity)(void *);
	int (*get_battery_rm)(void *);
};

struct coul_interface_dev {
	unsigned int total_ops;
	struct coul_interface_ops *p_ops[COUL_TYPE_END];
};

#ifdef CONFIG_HUAWEI_COUL
int coul_interface_ops_register(struct coul_interface_ops *ops);
int coul_interface_is_coul_ready(int type);
int coul_interface_is_battery_exist(int type);
int coul_interface_get_battery_capacity(int type);
int coul_interface_get_battery_rm(int type);
int coul_interface_get_battery_last_capacity(int type);
int coul_interface_set_battery_last_capacity(int type, int capacity);
int coul_interface_get_battery_voltage(int type);
int coul_interface_get_battery_current(int type);
int coul_interface_get_battery_avg_current(int type);
int coul_interface_get_battery_temperature(int type);
int coul_interface_get_battery_cycle(int type);
int coul_interface_get_battery_fcc(int type);
int coul_interface_set_vterm_dec(int type, int vterm_dec);
int coul_interface_set_battery_low_voltage(int type, int val);
#else
static inline int coul_interface_ops_register(struct coul_interface_ops *ops)
{
	return -1;
}

static inline int coul_interface_is_coul_ready(int type)
{
	return 0;
}

static inline int coul_interface_is_battery_exist(int type)
{
	return 0;
}

static inline int coul_interface_get_battery_capacity(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_rm(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_last_capacity(int type)
{
	return -EPERM;
}

static inline int coul_interface_set_battery_last_capacity(int type, int capacity)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_voltage(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_current(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_avg_current(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_temperature(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_cycle(int type)
{
	return -EPERM;
}

static inline int coul_interface_get_battery_fcc(int type)
{
	return -EPERM;
}

static inline int coul_interface_set_vterm_dec(int type, int vterm_dec)
{
	return 0;
}

static inline int coul_interface_set_battery_low_voltage(int type, int val)
{
	return -EPERM;
}
#endif /* CONFIG_HUAWEI_COUL */

#endif /* _COUL_INTERFACE_H_ */
