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
#define POWER_DTS_INFO(fmt, args...) hwlog_info(fmt, ##args)
#else
#define POWER_DTS_INFO(fmt, args...)
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

int power_dts_read_u32(const struct device_node *np,
	const char *prop, u32 *data, u32 default_value)
{
	if (!np || !prop || !data) {
		hwlog_err("np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_u32(np, prop, data)) {
		*data = default_value;
		hwlog_err("prop %s read fail, set default %u\n", prop, *data);
		return -EINVAL;
	}

	POWER_DTS_INFO("prop %s=%u\n", prop, *data);
	return 0;
}
EXPORT_SYMBOL_GPL(power_dts_read_u32);

int power_dts_read_str2int(const struct device_node *np,
	const char *prop, int *data, int default_value)
{
	const char *tmp_string = NULL;

	if (!np || !prop || !data) {
		hwlog_err("np or prop or data is null\n");
		return -EINVAL;
	}

	if (of_property_read_string(np, prop, &tmp_string)) {
		*data = default_value;
		hwlog_err("prop %s read fail, set default %d\n", prop, *data);
		return -EINVAL;
	}

	if (kstrtoint(tmp_string, 0, data)) {
		*data = default_value;
		hwlog_err("prop %s kstrtoint fail, set default %d\n",
			prop, *data);
		return -EINVAL;
	}

	POWER_DTS_INFO("prop %s=%d\n", prop, *data);
	return 0;
}
EXPORT_SYMBOL_GPL(power_dts_read_str2int);

int power_dts_read_string_index(const struct device_node *np,
	const char *prop, int index, const char **out)
{
	if (!np || !prop || !out) {
		hwlog_err("np or prop or out is null\n");
		return -EINVAL;
	}

	if (of_property_read_string_index(np, prop, index, out)) {
		hwlog_err("prop %s[%d] read fail\n", prop, index);
		return -EINVAL;
	}

	POWER_DTS_INFO("prop %s[%d]=%s\n", prop, index, *out);
	return 0;
}

int power_dts_read_count_strings(const struct device_node *np,
	const char *prop, u32 row, u32 col)
{
	int len;

	if (!np || !prop) {
		hwlog_err("np or prop is null\n");
		return -EINVAL;
	}

	len = of_property_count_strings(np, prop);
	if ((len <= 0) || (len % col != 0) || (len > row * col)) {
		hwlog_err("prop %s length read fail\n", prop);
		return -EINVAL;
	}

	POWER_DTS_INFO("prop %s length=%d\n", prop, len);
	return len;
}

int power_dts_read_u32_compatible(const char *compatible,
	const char *prop, u32 *data, u32 default_value)
{
	struct device_node *np = power_dts_get_compatible(compatible);

	if (!np)
		return -EINVAL;

	return power_dts_read_u32(np, prop, data, default_value);
}
EXPORT_SYMBOL_GPL(power_dts_read_u32_compatible);

int power_dts_read_str2int_compatible(const char *compatible,
	const char *prop, int *data, int default_value)
{
	struct device_node *np = power_dts_get_compatible(compatible);

	if (!np)
		return -EINVAL;

	return power_dts_read_str2int(np, prop, data, default_value);
}
EXPORT_SYMBOL_GPL(power_dts_read_str2int_compatible);
