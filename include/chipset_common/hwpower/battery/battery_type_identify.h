/*
 * battery_type_identify.h
 *
 * interfaces for switch mode between adc and
 * onewire communication to identify battery type.
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

#ifndef _BATTERY_TYPE_IDENTIFY_H_
#define _BATTERY_TYPE_IDENTIFY_H_

#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/device.h>

#define BAT_TYPE_GPIO_OUT     0
#define BAT_TYPE_GPIO_IN      1

#define BAT_TYPE_ID_SN_SIZE   6
#define BAT_TYPE_TEST_TIMES   10

struct bat_type_gpio_state {
	int direction;
	int value;
};

struct bat_security_ic_ops {
	int (*open_ic)(void);
	int (*close_ic)(void);
};

enum bat_type_identify_mode {
	BAT_ID_VOLTAGE = 0,
	BAT_ID_SN,
	BAT_INVALID_MODE,
};

struct bat_type_dev {
	struct mutex lock;
	int gpio;
	struct bat_type_gpio_state id_voltage;
	struct bat_type_gpio_state id_sn;
	const struct bat_security_ic_ops *ops;
	int cur_mode;
};

#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
void bat_type_apply_mode(int mode);
void bat_type_release_mode(bool flag);
void bat_security_ic_ops_register(const struct bat_security_ic_ops *ops);
void bat_security_ic_ops_unregister(const struct bat_security_ic_ops *ops);
#else
static inline void bat_type_apply_mode(int mode)
{
}

static inline void bat_type_release_mode(bool flag)
{
}

static inline void bat_security_ic_ops_register(const struct bat_security_ic_ops *ops)
{
}

static inline void bat_security_ic_ops_unregister(const struct bat_security_ic_ops *ops)
{
}
#endif /* CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY */

#endif /* _BATTERY_TYPE_IDENTIFY_H_ */
