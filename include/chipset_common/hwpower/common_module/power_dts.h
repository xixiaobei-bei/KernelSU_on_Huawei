/*
 * power_dts.h
 *
 * dts parse interface for power module
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

#ifndef _POWER_DTS_H_
#define _POWER_DTS_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define __power_dts_tag(tag)    #tag
#define power_dts_tag(tag)      __power_dts_tag(tag)

int power_dts_read_u8(const char *tag, const struct device_node *np,
	const char *prop, u8 *data, u8 default_value);
int power_dts_read_u32(const char *tag, const struct device_node *np,
	const char *prop, u32 *data, u32 default_value);
int power_dts_read_u8_array(const char *tag, const struct device_node *np,
	const char *prop, u8 *data, u16 len);
int power_dts_read_u16_array(const char *tag, const struct device_node *np,
	const char *prop, u16 *data, u16 len);
int power_dts_read_u32_array(const char *tag, const struct device_node *np,
	const char *prop, u32 *data, u32 len);
int power_dts_read_u32_count(const char *tag, const struct device_node *np,
	const char *prop, u32 row, u32 col);
int power_dts_read_u32_index(const char *tag, const struct device_node *np,
	const char *prop, int index, u32 *data);
int power_dts_read_str2int(const char *tag, const struct device_node *np,
	const char *prop, int *data, int default_value);
int power_dts_read_string(const char *tag, const struct device_node *np,
	const char *prop, const char **out);
int power_dts_read_string_index(const char *tag, const struct device_node *np,
	const char *prop, int index, const char **out);
int power_dts_read_count_strings(const char *tag, const struct device_node *np,
	const char *prop, u32 row, u32 col);
int power_dts_read_string_array(const char *tag, const struct device_node *np,
	const char *prop, int *data, u32 row, u32 col);
int power_dts_read_u8_compatible(const char *tag, const char *comp,
	const char *prop, u8 *data, u8 default_value);
int power_dts_read_u32_compatible(const char *tag, const char *comp,
	const char *prop, u32 *data, u32 default_value);
int power_dts_read_u8_array_compatible(const char *tag, const char *comp,
	const char *prop, u8 *data, u16 len);
int power_dts_read_u16_array_compatible(const char *tag, const char *comp,
	const char *prop, u16 *data, u16 len);
int power_dts_read_u32_array_compatible(const char *tag, const char *comp,
	const char *prop, u32 *data, u32 len);
int power_dts_read_u32_count_compatible(const char *tag, const char *comp,
	const char *prop, u32 row, u32 col);
int power_dts_read_u32_index_compatible(const char *tag, const char *comp,
	const char *prop, int index, u32 *data);
int power_dts_read_str2int_compatible(const char *tag, const char *comp,
	const char *prop, int *data, int default_value);
int power_dts_read_string_compatible(const char *tag, const char *comp,
	const char *prop, const char **out);
int power_dts_read_string_index_compatible(const char *tag, const char *comp,
	const char *prop, int index, const char **out);
int power_dts_read_count_strings_compatible(const char *tag, const char *comp,
	const char *prop, u32 row, u32 col);
int power_dts_read_string_array_compatible(const char *tag, const char *comp,
	const char *prop, int *data, u32 row, u32 col);

#endif /* _POWER_DTS_H_ */
