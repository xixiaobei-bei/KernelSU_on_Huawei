/*
 * power_dts.c
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

#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG power_dts
HWLOG_REGIST();

#ifdef CONFIG_HUAWEI_POWER_DEBUG
#define power_dts_info(fmt, args...) hwlog_info(fmt, ##args)
#else
#define power_dts_info(fmt, args...)
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static struct device_node *power_dts_get_compatible(const char *compatible)
{
	struct device_node *np = NULL;

	if (!compatible) {
		hwlog_err("compatible is null\n");
		return NULL;
	}

	np = of_find_compatible_node(NULL, NULL, compatible);
	if (!np) {
		hwlog_err("compatible %s get fail\n", compatible);
		return NULL;
	}

	return np;
}

int power_dts_read_u8(const char *tag, const struct device_node *np,
	const char *prop, u8 *data, u8 default_value)
{
	if (!tag || !np || !prop || !data) {
		hwlog_err("tag or np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_u8(np, prop, data)) {
		*data = default_value;
		hwlog_err("%s: prop %s read fail, set default %u\n",
			tag, prop, *data);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s=%u\n", tag, prop, *data);
	return 0;
}

int power_dts_read_u32(const char *tag, const struct device_node *np,
	const char *prop, u32 *data, u32 default_value)
{
	if (!tag || !np || !prop || !data) {
		hwlog_err("tag or np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_u32(np, prop, data)) {
		*data = default_value;
		hwlog_err("%s: prop %s read fail, set default %u\n",
			tag, prop, *data);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s=%u\n", tag, prop, *data);
	return 0;
}

int power_dts_read_u8_array(const char *tag, const struct device_node *np,
	const char *prop, u8 *data, u8 len)
{
	int i;

	if (!tag || !np || !prop || !data) {
		hwlog_err("tag or np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_u8_array(np, prop, data, len)) {
		hwlog_err("%s: prop %s read fail, array len %d\n", tag, prop, len);
		return -EINVAL;
	}

	for (i = 0; i < len; i++)
		power_dts_info("%s: prop %s[%d]=0x%x\n", tag, prop, i, data[i]);
	return 0;
}

int power_dts_read_u16_array(const char *tag, const struct device_node *np,
	const char *prop, u16 *data, u16 len)
{
	int i;

	if (!tag || !np || !prop || !data) {
		hwlog_err("tag or np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_u16_array(np, prop, data, len)) {
		hwlog_err("%s: prop %s read fail, array len %d\n", tag, prop, len);
		return -EINVAL;
	}

	for (i = 0; i < len; i++)
		power_dts_info("%s: prop %s[%d]=0x%x\n", tag, prop, i, data[i]);
	return 0;
}

int power_dts_read_u32_array(const char *tag, const struct device_node *np,
	const char *prop, u32 *data, u32 len)
{
	int i;

	if (!tag || !np || !prop || !data) {
		hwlog_err("tag or np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_u32_array(np, prop, data, len)) {
		hwlog_err("%s: prop %s read fail, array len %d\n", tag, prop, len);
		return -EINVAL;
	}

	for (i = 0; i < len; i++)
		power_dts_info("%s: prop %s[%d]=0x%x\n", tag, prop, i, data[i]);
	return 0;
}

int power_dts_read_u32_count(const char *tag, const struct device_node *np,
	const char *prop, u32 row, u32 col)
{
	int len;

	if (!tag || !np || !prop) {
		hwlog_err("tag or np or prop is null\n");
		return -EINVAL;
	}

	len = of_property_count_u32_elems(np, prop);
	if ((len <= 0) || (len % col != 0) || (len > row * col)) {
		hwlog_err("%s: prop %s length read fail\n", tag, prop);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s length=%d\n", tag, prop, len);
	return len;
}

int power_dts_read_str2int(const char *tag, const struct device_node *np,
	const char *prop, int *data, int default_value)
{
	const char *tmp_string = NULL;

	if (!tag || !np || !prop || !data) {
		hwlog_err("tag or np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_string(np, prop, &tmp_string)) {
		*data = default_value;
		hwlog_err("%s: prop %s read fail, set default %d\n",
			tag, prop, *data);
		return -EINVAL;
	}

	if (kstrtoint(tmp_string, 0, data)) {
		*data = default_value;
		hwlog_err("%s: prop %s kstrtoint fail, set default %d\n",
			tag, prop, *data);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s=%d\n", tag, prop, *data);
	return 0;
}

int power_dts_read_string(const char *tag, const struct device_node *np,
	const char *prop, const char **out)
{
	if (!tag || !np || !prop || !out) {
		hwlog_err("tag or np or prop or out is null\n");
		return -EINVAL;
	}

	if (of_property_read_string(np, prop, out)) {
		hwlog_err("%s: prop %s read fail\n", tag, prop);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s=%s\n", tag, prop, *out);
	return 0;
}

int power_dts_read_string_index(const char *tag, const struct device_node *np,
	const char *prop, int index, const char **out)
{
	if (!tag || !np || !prop || !out) {
		hwlog_err("tag or np or prop or out is null\n");
		return -EINVAL;
	}

	if (of_property_read_string_index(np, prop, index, out)) {
		hwlog_err("%s: prop %s[%d] read fail\n", tag, prop, index);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s[%d]=%s\n", tag, prop, index, *out);
	return 0;
}

int power_dts_read_count_strings(const char *tag, const struct device_node *np,
	const char *prop, u32 row, u32 col)
{
	int len;

	if (!tag || !np || !prop) {
		hwlog_err("tag or np or prop is null\n");
		return -EINVAL;
	}

	len = of_property_count_strings(np, prop);
	if ((len <= 0) || (len % col != 0) || (len > row * col)) {
		hwlog_err("%s: prop %s length read fail\n", tag, prop);
		return -EINVAL;
	}

	power_dts_info("%s: prop %s length=%d\n", tag, prop, len);
	return len;
}

int power_dts_read_u8_compatible(const char *tag, const char *comp,
	const char *prop, u8 *data, u8 default_value)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_u8(tag, np, prop, data, default_value);
}

int power_dts_read_u32_compatible(const char *tag, const char *comp,
	const char *prop, u32 *data, u32 default_value)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_u32(tag, np, prop, data, default_value);
}

int power_dts_read_u8_array_compatible(const char *tag, const char *comp,
	const char *prop, u8 *data, u8 len)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_u8_array(tag, np, prop, data, len);
}

int power_dts_read_u16_array_compatible(const char *tag, const char *comp,
	const char *prop, u16 *data, u16 len)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_u16_array(tag, np, prop, data, len);
}

int power_dts_read_u32_array_compatible(const char *tag, const char *comp,
	const char *prop, u32 *data, u32 len)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_u32_array(tag, np, prop, data, len);
}

int power_dts_read_u32_count_compatible(const char *tag, const char *comp,
	const char *prop, u32 row, u32 col)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_u32_count(tag, np, prop, row, col);
}

int power_dts_read_str2int_compatible(const char *tag, const char *comp,
	const char *prop, int *data, int default_value)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_str2int(tag, np, prop, data, default_value);
}

int power_dts_read_string_compatible(const char *tag, const char *comp,
	const char *prop, const char **out)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_string(tag, np, prop, out);
}

int power_dts_read_string_index_compatible(const char *tag, const char *comp,
	const char *prop, int index, const char **out)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_string_index(tag, np, prop, index, out);
}

int power_dts_read_count_strings_compatible(const char *tag, const char *comp,
	const char *prop, u32 row, u32 col)
{
	struct device_node *np = power_dts_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_dts_read_count_strings(tag, np, prop, row, col);
}
