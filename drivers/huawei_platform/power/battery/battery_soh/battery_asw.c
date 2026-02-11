/*
 * battery_asw.c
 *
 * Driver adapter for asw.
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

#include "battery_asw.h"
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battery_asw
HWLOG_REGIST();

void asw_dmd_content_prepare(char *buff, unsigned int size)
{
	int voltage = hisi_battery_voltage();
	int temp = hisi_battery_temperature();
	unsigned int charge_cycles = hisi_battery_cycle_count();
	char *bat_brand = hisi_battery_brand();

	if (!bat_brand)
		return;
	snprintf(buff, size - 1,
		"asw batt_name=%s, temp:%d, volt:%d, charge_cycles:%d\n",
		bat_brand, temp, voltage, charge_cycles);
}

#if (defined(CONFIG_HUAWEI_POWER_DEBUG) && defined(CONFIG_SYSFS))
struct asw_data g_asw_data = { 0 };

static ssize_t asw_time_speed_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int speed;

	if (kstrtouint(buf, 0, &speed))
		return -EINVAL;

	g_asw_data.time_speed = speed;
	return count;
}

static ssize_t asw_time_speed_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", g_asw_data.time_speed);
}

static ssize_t asw_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	/* 4: the fields of "enable type voltage temp" */
	if (sscanf(buf, "%d %d %d %d", &g_asw_data.enable_stub,
		&g_asw_data.charger_type,
		&g_asw_data.bat_voltage,
		&g_asw_data.bat_temp) != 4) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	return count;
}

static ssize_t asw_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n",
		g_asw_data.enable_stub,
		g_asw_data.charger_type,
		g_asw_data.bat_voltage,
		g_asw_data.bat_temp);
}

static DEVICE_ATTR_RW(asw_time_speed);
static DEVICE_ATTR_RW(asw_data);
static struct attribute *g_asw_attrs[] = {
	&dev_attr_asw_time_speed.attr,
	&dev_attr_asw_data.attr,
	NULL,
};

static struct attribute_group g_asw_group = {
	.name = "asw",
	.attrs = g_asw_attrs,
};

int asw_sysfs_create_group(struct bsoh_device *di)
{
	return sysfs_create_group(&di->dev->kobj, &g_asw_group);
}

void asw_sysfs_remove_group(struct bsoh_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_asw_group);
}
#else
int asw_sysfs_create_group(struct bsoh_device *di)
{
	return 0;
}

void asw_sysfs_remove_group(struct bsoh_device *di)
{
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG && CONFIG_SYSFS */
