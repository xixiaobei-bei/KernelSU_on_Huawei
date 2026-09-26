/*
 * coul_sysfs.c
 *
 * sysfs of coul driver
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

#include "coul_sysfs.h"
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>

#define HWLOG_TAG coul_sysfs
HWLOG_REGIST();

static struct coul_sysfs_dev *g_coul_sysfs_dev;

static struct coul_sysfs_dev *coul_sysfs_get_dev(void)
{
	if (!g_coul_sysfs_dev) {
		hwlog_err("g_coul_sysfs_dev is null\n");
		return NULL;
	}

	return g_coul_sysfs_dev;
}

#ifdef CONFIG_SYSFS
static int coul_sysfs_get_gaugelog_head(struct coul_sysfs_dev *l_dev,
	char *buf, int size)
{
	return snprintf(buf, size, "brand       ocv       ");
}

static int coul_sysfs_get_gaugelog(struct coul_sysfs_dev *l_dev,
	char *buf, int size)
{
	int ret;
	char bat_brand[COUL_BAT_BRAND_SIZE] = { 0 };
	int bat_ocv;

	ret = power_supply_get_str_property_value(l_dev->psy_name,
		POWER_SUPPLY_PROP_BRAND, bat_brand, COUL_BAT_BRAND_SIZE - 1);
	if (ret)
		strncpy(bat_brand, "default", COUL_BAT_BRAND_SIZE - 1);

	ret = power_supply_get_int_property_value(l_dev->psy_name,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &bat_ocv);
	if (ret)
		bat_ocv = 0;

	return snprintf(buf, size, "%-11s %-10d", bat_brand, bat_ocv);
}

static ssize_t coul_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info coul_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(coul, 0444, COUL_SYSFS_GAUGELOG_HEAD, gaugelog_head),
	power_sysfs_attr_ro(coul, 0444, COUL_SYSFS_GAUGELOG, gaugelog),
};

#define COUL_SYSFS_ATTRS_SIZE  ARRAY_SIZE(coul_sysfs_field_tbl)

static struct attribute *coul_sysfs_attrs[COUL_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group coul_sysfs_attr_group = {
	.attrs = coul_sysfs_attrs,
};

static ssize_t coul_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct coul_sysfs_dev *l_dev = coul_sysfs_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		coul_sysfs_field_tbl, COUL_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case COUL_SYSFS_GAUGELOG_HEAD:
		return coul_sysfs_get_gaugelog_head(l_dev, buf, PAGE_SIZE - 1);
	case COUL_SYSFS_GAUGELOG:
		return coul_sysfs_get_gaugelog(l_dev, buf, PAGE_SIZE - 1);
	default:
		return 0;
	}
}

static void coul_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(coul_sysfs_attrs,
		coul_sysfs_field_tbl, COUL_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "coul", "coul_data",
		dev, &coul_sysfs_attr_group);
}

static void coul_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "coul", "coul_data",
		dev, &coul_sysfs_attr_group);
}
#else
static inline void coul_sysfs_create_group(struct device *dev)
{
	return NULL;
}

static inline void coul_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void coul_sysfs_parse_dts(struct device_node *np,
	struct coul_sysfs_dev *l_dev)
{
	int ret;
	const char *psy_name = NULL;

	ret = power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"psy_name", &psy_name);
	if (ret)
		strncpy(l_dev->psy_name, "battery", COUL_PSY_NAME_SIZE - 1);
	else
		strncpy(l_dev->psy_name, psy_name, COUL_PSY_NAME_SIZE - 1);
}

static int coul_sysfs_probe(struct platform_device *pdev)
{
	struct coul_sysfs_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_coul_sysfs_dev = l_dev;
	l_dev->dev = &pdev->dev;
	np = pdev->dev.of_node;
	coul_sysfs_parse_dts(np, l_dev);
	coul_sysfs_create_group(l_dev->dev);
	platform_set_drvdata(pdev, l_dev);

	return 0;
}

static int coul_sysfs_remove(struct platform_device *pdev)
{
	struct coul_sysfs_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	coul_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_coul_sysfs_dev = NULL;

	return 0;
}

static const struct of_device_id coul_sysfs_match_table[] = {
	{
		.compatible = "huawei,fuelguage",
		.data = NULL,
	},
	{},
};

static struct platform_driver coul_sysfs_driver = {
	.probe = coul_sysfs_probe,
	.remove = coul_sysfs_remove,
	.driver = {
		.name = "huawei,fuelguage",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(coul_sysfs_match_table),
	},
};

static int __init coul_sysfs_init(void)
{
	return platform_driver_register(&coul_sysfs_driver);
}

static void __exit coul_sysfs_exit(void)
{
	platform_driver_unregister(&coul_sysfs_driver);
}

late_initcall(coul_sysfs_init);
module_exit(coul_sysfs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("coul sysfs driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
