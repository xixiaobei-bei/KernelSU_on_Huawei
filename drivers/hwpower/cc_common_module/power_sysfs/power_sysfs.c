/*
 * power_sysfs.c
 *
 * sysfs interface for power module
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

#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_sysfs
HWLOG_REGIST();

static struct power_sysfs_device_data g_power_sysfs_hw_power[] = {
	{ "charger", NULL },
	{ "vsys_switch", NULL },
	{ "coul", NULL },
};

static struct power_sysfs_class_data g_power_sysfs_class_data[] = {
	{ "hw_power", NULL,
		g_power_sysfs_hw_power, ARRAY_SIZE(g_power_sysfs_hw_power) },
	{ "hw_typec", NULL, NULL, 0 },
	{ "hw_usb", NULL, NULL, 0 },
	{ "hishow", NULL, NULL, 0 },
	{ "hw_accessory", NULL, NULL, 0 },
};

static struct class *power_sysfs_create_class(const char *cls_name)
{
	struct class *l_class = NULL;

	if (!cls_name)
		return NULL;

	l_class = class_create(THIS_MODULE, cls_name);
	if (IS_ERR(l_class))
		return NULL;

	return l_class;
}

struct class *power_sysfs_get_class(const char *cls_name)
{
	int i;
	struct power_sysfs_class_data *cls_data = g_power_sysfs_class_data;
	int cls_size = ARRAY_SIZE(g_power_sysfs_class_data);

	for (i = 0; i < cls_size; i++) {
		if (!strcmp(cls_data[i].name, cls_name))
			return cls_data[i].entity;
	}

	hwlog_err("invalid cls_name=%s\n", cls_name);
	return NULL;
}

static struct device *power_sysfs_create_device(struct class *l_class,
	const char *dev_name)
{
	struct device *l_device = NULL;

	if (!l_class || !dev_name)
		return NULL;

	l_device = device_create(l_class, NULL, 0, NULL, "%s", dev_name);
	if (IS_ERR(l_device))
		return NULL;

	return l_device;
}

static void power_sysfs_destroy_device(struct device *l_device)
{
	if (!l_device)
		return;

	put_device(l_device);
	device_unregister(l_device);
}

struct device *power_sysfs_get_device(const char *cls_name,
	const char *dev_name)
{
	int i;
	int j;
	struct power_sysfs_class_data *cls_data = g_power_sysfs_class_data;
	int cls_size = ARRAY_SIZE(g_power_sysfs_class_data);
	struct power_sysfs_device_data *dev_data = NULL;
	int dev_size;

	for (i = 0; i < cls_size; i++) {
		if (strcmp(cls_data[i].name, cls_name))
			continue;

		if (!cls_data[i].dev_data)
			continue;

		dev_data = cls_data[i].dev_data;
		dev_size = cls_data[i].dev_size;
		for (j = 0; j < dev_size; j++) {
			if (!strcmp(dev_data[j].name, dev_name))
				return dev_data[j].entity;
		}
	}

	hwlog_err("invalid dev_name=%s\n", dev_name);
	return NULL;
}

struct device *power_sysfs_create_link_group(
	const char *cls_name, const char *dev_name, const char *link_name,
	struct device *target_dev, const struct attribute_group *group)
{
	struct class *l_class = NULL;
	struct device *l_device = NULL;

	if (!cls_name || !dev_name || !link_name || !target_dev || !group)
		return NULL;

	l_class = power_sysfs_get_class(cls_name);
	if (!l_class) {
		hwlog_err("class %s get fail\n", cls_name);
		return NULL;
	}

	l_device = power_sysfs_get_device(cls_name, dev_name);
	if (!l_device) {
		hwlog_err("device %s get fail\n", dev_name);
		return NULL;
	}

	if (sysfs_create_group(&target_dev->kobj, group))
		return NULL;

	if (sysfs_create_link(&l_device->kobj, &target_dev->kobj, link_name))
		return NULL;

	hwlog_info("link group %s/%s/%s create succ\n",
		cls_name, dev_name, link_name);
	return l_device;
}

void power_sysfs_remove_link_group(
	const char *cls_name, const char *dev_name, const char *link_name,
	struct device *target_dev, const struct attribute_group *group)
{
	struct device *l_device = NULL;

	if (!cls_name || !dev_name || !link_name || !target_dev || !group)
		return;

	l_device = power_sysfs_get_device(cls_name, dev_name);
	if (!l_device) {
		hwlog_err("device %s get fail\n", dev_name);
		return;
	}

	sysfs_remove_link(&l_device->kobj, link_name);
	sysfs_remove_group(&target_dev->kobj, group);
}

struct device *power_sysfs_create_group(
	const char *cls_name, const char *dev_name,
	const struct attribute_group *group)
{
	struct class *l_class = NULL;
	struct device *l_device = NULL;

	if (!cls_name || !dev_name || !group)
		return NULL;

	l_class = power_sysfs_get_class(cls_name);
	if (!l_class) {
		hwlog_err("class %s get fail\n", cls_name);
		return NULL;
	}

	l_device = power_sysfs_create_device(l_class, dev_name);
	if (!l_device) {
		hwlog_err("device %s get fail\n", dev_name);
		return NULL;
	}

	if (sysfs_create_group(&l_device->kobj, group)) {
		power_sysfs_destroy_device(l_device);
		return NULL;
	}

	hwlog_info("group %s/%s create succ\n", cls_name, dev_name);
	return l_device;
}

void power_sysfs_remove_group(struct device *dev,
	const struct attribute_group *group)
{
	if (!dev || !group)
		return;

	sysfs_remove_group(&dev->kobj, group);
	put_device(dev);
	device_unregister(dev);
}

void power_sysfs_init_attrs(struct attribute **attrs,
	struct power_sysfs_attr_info *attr_info, int size)
{
	int i;

	if (!attrs || !attr_info)
		return;

	for (i = 0; i < size; i++)
		attrs[i] = &attr_info[i].attr.attr;

	attrs[size] = NULL;
}

struct power_sysfs_attr_info *power_sysfs_lookup_attr(const char *name,
	struct power_sysfs_attr_info *attr_info, int size)
{
	int i;

	if (!name || !attr_info)
		return NULL;

	for (i = 0; i < size; i++) {
		if (!strcmp(name, attr_info[i].attr.attr.name))
			return &attr_info[i];
	}

	return NULL;
}

static int __init power_sysfs_init(void)
{
	int i;
	int j;
	struct power_sysfs_class_data *cls_data = g_power_sysfs_class_data;
	int cls_size = ARRAY_SIZE(g_power_sysfs_class_data);
	struct class *l_class = NULL;
	struct power_sysfs_device_data *dev_data = NULL;
	int dev_size;

	for (i = 0; i < cls_size; i++) {
		cls_data[i].entity = power_sysfs_create_class(cls_data[i].name);
		if (!cls_data[i].entity)
			continue;

		if (!cls_data[i].dev_data)
			continue;

		l_class = cls_data[i].entity;
		dev_data = cls_data[i].dev_data;
		dev_size = cls_data[i].dev_size;
		for (j = 0; j < dev_size; j++)
			dev_data[j].entity = power_sysfs_create_device(l_class,
				dev_data[j].name);
	}

	return 0;
}

static void __exit power_sysfs_exit(void)
{
}

subsys_initcall(power_sysfs_init);
module_exit(power_sysfs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sysfs for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
