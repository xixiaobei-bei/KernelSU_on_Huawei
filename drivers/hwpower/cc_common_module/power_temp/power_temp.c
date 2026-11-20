/*
 * power_temp.c
 *
 * temperature interface for power module
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

#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_temp
HWLOG_REGIST();

static struct power_temp_dev *g_power_temp_dev;

static int power_temp_get_tz_temp(const char *sensor_name, int *temp)
{
	struct thermal_zone_device *tz = NULL;
	int ret;

	if (!sensor_name || !temp) {
		hwlog_err("sensor_name or temp is null\n");
		return -1;
	}

	tz = thermal_zone_get_zone_by_name(sensor_name);
	if (IS_ERR(tz)) {
		hwlog_err("get %s tz fail\n", sensor_name);
		return -1;
	}

	ret = thermal_zone_get_temp(tz, temp);
	if (ret) {
		hwlog_err("get %s tz temp fail\n", sensor_name);
		return -1;
	}

	return 0;
}

int power_temp_get_raw_value(const char *sensor_name)
{
	int temp = POWER_TEMP_INVALID_TEMP;

	if (power_temp_get_tz_temp(sensor_name, &temp))
		return POWER_TEMP_INVALID_TEMP / POWER_TEMP_UNIT;

	return temp / POWER_TEMP_UNIT;
}

int power_temp_get_average_value(const char *sensor_name)
{
	int samples[POWER_TEMP_SAMPLES] = { 0 };
	int retrys = POWER_TEMP_RETRYS;
	int i, max_temp, min_temp;
	int invalid_flag;

	while (retrys--) {
		invalid_flag = POWER_TEMP_VALID;

		for (i = 0; i < POWER_TEMP_SAMPLES; ++i) {
			if (power_temp_get_tz_temp(sensor_name, &samples[i])) {
				invalid_flag = POWER_TEMP_INVALID;
				break;
			}
			hwlog_info("sensor:%s, samples[%d]:%d\n",
				sensor_name, i, samples[i]);
		}

		if (invalid_flag == POWER_TEMP_INVALID)
			continue;

		/* check sample value is valid */
		max_temp = power_get_max_value(samples, POWER_TEMP_SAMPLES);
		min_temp = power_get_min_value(samples, POWER_TEMP_SAMPLES);
		if (max_temp - min_temp > POWER_TEMP_DIFF_THLD) {
			hwlog_err("invalid temp: max=%d min=%d\n",
				max_temp, min_temp);
			invalid_flag = POWER_TEMP_INVALID;
		}

		if (invalid_flag == POWER_TEMP_VALID)
			break;
	}

	if (invalid_flag == POWER_TEMP_INVALID) {
		hwlog_err("use default temp %d\n", POWER_TEMP_INVALID_TEMP);
		return POWER_TEMP_INVALID_TEMP;
	}

	return power_get_average_value(samples, POWER_TEMP_SAMPLES);
}

static int __init power_temp_init(void)
{
	struct power_temp_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_temp_dev = l_dev;
	return 0;
}

static void __exit power_temp_exit(void)
{
	if (!g_power_temp_dev)
		return;

	kfree(g_power_temp_dev);
	g_power_temp_dev = NULL;
}

subsys_initcall_sync(power_temp_init);
module_exit(power_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("temp for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
