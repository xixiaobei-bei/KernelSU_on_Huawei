/*
 * battery_temp.c
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

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>

#define HWLOG_TAG bat_temp
HWLOG_REGIST();

struct bat_temp_info {
	struct device *dev;
	char name[BAT_TEMP_NAME_MAX + 1];
	struct bat_temp_ops *ops;
	unsigned int test_flag;
	int input_tbat;
};

static struct bat_temp_info *g_bat_temp_info;

enum bat_temp_type {
	SYSFS_BAT_TEMP_0 = 0,
	SYSFS_BAT_TEMP_1,
	SYSFS_BAT_TEMP_MIXED,
	SYSFS_BAT_TEMP_0_NOW,
	SYSFS_BAT_TEMP_1_NOW,
	SYSFS_BAT_TEMP_MIXED_NOW,
	SYSFS_BAT_TEMP_NAME,
	SYSFS_BAT_TEMP_TEST_FLAG,
	SYSFS_INPUT_BAT_TEMP,
};

#ifdef CONFIG_SYSFS
static ssize_t bat_temp_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bat_temp_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info bat_temp_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_0, bat_0),
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_1, bat_1),
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_MIXED, bat_mixed),
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_0_NOW, bat_0_now),
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_1_NOW, bat_1_now),
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_MIXED_NOW, bat_mixed_now),
	power_sysfs_attr_ro(bat_temp, 0440, SYSFS_BAT_TEMP_NAME, bat_temp_name),
	power_sysfs_attr_rw(bat_temp, 0644, SYSFS_BAT_TEMP_TEST_FLAG, test_flag),
	power_sysfs_attr_rw(bat_temp, 0644, SYSFS_INPUT_BAT_TEMP, input_tbat),
};

#define BAT_TEMP_SYSFS_ATTRS_SIZE  ARRAY_SIZE(bat_temp_sysfs_field_tbl)

static struct attribute *bat_temp_sysfs_attrs[BAT_TEMP_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group bat_temp_sysfs_attr_group = {
	.attrs = bat_temp_sysfs_attrs,
};

static ssize_t bat_temp_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct bat_temp_info *di = g_bat_temp_info;
	int bat_temp = 0;
	int len = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_temp_sysfs_field_tbl, BAT_TEMP_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case SYSFS_BAT_TEMP_0:
	case SYSFS_BAT_TEMP_1:
	case SYSFS_BAT_TEMP_MIXED:
		bat_temp_get_temperature(info->name - SYSFS_BAT_TEMP_0, &bat_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", bat_temp);
		break;
	case SYSFS_BAT_TEMP_0_NOW:
	case SYSFS_BAT_TEMP_1_NOW:
	case SYSFS_BAT_TEMP_MIXED_NOW:
		bat_temp_get_rt_temperature(info->name - SYSFS_BAT_TEMP_0_NOW,
			&bat_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", bat_temp);
		break;
	case SYSFS_BAT_TEMP_NAME:
		len = snprintf(buf, PAGE_SIZE, "%s\n", di->name);
		break;
	case SYSFS_BAT_TEMP_TEST_FLAG:
		len = snprintf(buf, PAGE_SIZE, "%u\n", di->test_flag);
		break;
	case SYSFS_INPUT_BAT_TEMP:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->input_tbat);
		break;
	default:
		break;
	}

	return len;
}

static ssize_t bat_temp_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct bat_temp_info *di = g_bat_temp_info;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_temp_sysfs_field_tbl, BAT_TEMP_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case SYSFS_BAT_TEMP_TEST_FLAG:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;
		di->test_flag = (unsigned int)val;
		hwlog_info("set test start flag = %u\n", di->test_flag);
		break;
	case SYSFS_INPUT_BAT_TEMP:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < INVALID_BATT_LOW_TEMP) ||
			(val > INVALID_BATT_HIGH_TEMP))
			return -EINVAL;
		di->input_tbat = (int)val;
		hwlog_info("set input batt temp = %d\n", di->input_tbat);
		break;
	default:
		break;
	}

	return count;
}

static void bat_temp_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(bat_temp_sysfs_attrs,
		bat_temp_sysfs_field_tbl, BAT_TEMP_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "hw_bat_temp",
		dev, &bat_temp_sysfs_attr_group);
}

static void bat_temp_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "hw_bat_temp",
		dev, &bat_temp_sysfs_attr_group);
}
#else
static inline void bat_temp_sysfs_create_group(struct device *dev)
{
}

static inline void bat_temp_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

int bat_temp_get_temperature(enum bat_temp_id id, int *temp)
{
	struct bat_temp_info *di = g_bat_temp_info;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -EINVAL;
	}

	if (!power_platform_is_battery_exit())
		hwlog_err("battery not exist\n");

	if (!di || !di->ops || !di->ops->get_temp) {
		*temp = power_platform_get_battery_temperature();
		hwlog_info("default temp api: temp %d\n", *temp);
		return 0;
	}

	if (di->test_flag) {
		*temp = di->input_tbat;
		return 0;
	}

	return di->ops->get_temp(id, temp);
}

int bat_temp_get_rt_temperature(enum bat_temp_id id, int *temp)
{
	struct bat_temp_info *di = g_bat_temp_info;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -EINVAL;
	}

	if (!power_platform_is_battery_exit())
		hwlog_err("battery not exist\n");

	if (!di || !di->ops || !di->ops->get_rt_temp) {
		*temp = power_platform_get_rt_battery_temperature();
		hwlog_info("default realtime temp api: temp %d\n", *temp);
		return 0;
	}

	if (di->test_flag) {
		*temp = di->input_tbat;
		return 0;
	}

	return di->ops->get_rt_temp(id, temp);
}

int bat_temp_ops_register(const char *name, struct bat_temp_ops *ops)
{
	if (!name || !ops || !ops->get_temp || !ops->get_rt_temp) {
		hwlog_err("input arg err\n");
		return -EINVAL;
	}

	if (!g_bat_temp_info) {
		hwlog_err("temp info ptr is null\n");
		return -ENODEV;
	}

	if (strlen(name) >= BAT_TEMP_NAME_MAX) {
		hwlog_err("%s is err\n", name);
		return -EINVAL;
	}

	strlcpy(g_bat_temp_info->name, name, BAT_TEMP_NAME_MAX);
	g_bat_temp_info->ops = ops;
	return 0;
}

static int bat_temp_probe(struct platform_device *pdev)
{
	struct bat_temp_info *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bat_temp_info = di;
	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	di->test_flag = 0;

	bat_temp_sysfs_create_group(di->dev);

	return 0;
}

static int bat_temp_remove(struct platform_device *pdev)
{
	struct bat_temp_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	bat_temp_sysfs_remove_group(di->dev);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_bat_temp_info = NULL;

	return 0;
}

static const struct of_device_id bat_temp_match_table[] = {
	{
		.compatible = "huawei,battery_temp",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_temp_driver = {
	.probe = bat_temp_probe,
	.remove = bat_temp_remove,
	.driver = {
		.name = "huawei,battery_temp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_temp_match_table),
	},
};

int __init bat_temp_init(void)
{
	return platform_driver_register(&bat_temp_driver);
}

void __exit bat_temp_exit(void)
{
	platform_driver_unregister(&bat_temp_driver);
}

fs_initcall(bat_temp_init);
module_exit(bat_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
