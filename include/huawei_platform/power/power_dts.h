/*
 * power_dts.h
 *
 * dts parse interface for power module
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

#ifndef _POWER_DTS_H_
#define _POWER_DTS_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>

int power_dts_read_u32(const struct device_node *np,
	const char *prop, u32 *data, u32 default_value);
int power_dts_read_str2int(const struct device_node *np,
	const char *prop, int *data, int default_value);
int power_dts_read_string_index(const struct device_node *np,
	const char *prop, int index, const char **out);
int power_dts_read_count_strings(const struct device_node *np,
	const char *prop, u32 row, u32 col);
int power_dts_read_u32_compatible(const char *compatible,
	const char *prop, u32 *data, u32 default_value);
int power_dts_read_str2int_compatible(const char *compatible,
	const char *prop, int *data, int default_value);

#endif /* _POWER_DTS_H_ */
