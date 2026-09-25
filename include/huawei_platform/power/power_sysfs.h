/*
 * power_sysfs.h
 *
 * sysfs interface for power module
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

#ifndef _POWER_SYSFS_H_
#define _POWER_SYSFS_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

struct power_sysfs_device_data {
	const char *name;
	struct device *entity;
};

struct power_sysfs_class_data {
	const char *name;
	struct class *entity;
	struct power_sysfs_device_data *dev_data;
	int dev_size;
};

#ifdef CONFIG_SYSFS
struct power_sysfs_attr_info {
	struct device_attribute attr;
	u8 name;
};

#define power_sysfs_attr_ro(_func, _mode, _type, _name) \
{ \
	.attr = __ATTR(_name, _mode, _func##_sysfs_show, NULL), \
	.name = _type, \
}

#define power_sysfs_attr_wo(_func, _mode, _type, _name) \
{ \
	.attr = __ATTR(_name, _mode, NULL, _func##_sysfs_store), \
	.name = _type, \
}

#define power_sysfs_attr_rw(_func, _mode, _type, _name) \
{ \
	.attr = __ATTR(_name, _mode, _func##_sysfs_show, _func##_sysfs_store), \
	.name = _type, \
}

struct device *power_sysfs_create_link_group(
	const char *cls_name, const char *dev_name, const char *link_name,
	struct device *target_dev, const struct attribute_group *link_group);
void power_sysfs_remove_link_group(
	const char *cls_name, const char *dev_name, const char *link_name,
	struct device *target_dev, const struct attribute_group *group);
struct device *power_sysfs_create_group(
	const char *cls_name, const char *dev_name,
	const struct attribute_group *group);
void power_sysfs_remove_group(struct device *dev,
	const struct attribute_group *group);
#else
static inline struct device *power_sysfs_create_link_group(
	const char *cls_name, const char *dev_name, const char *link_name,
	struct device *target_dev, const struct attribute_group *link_group)
{
	return NULL;
}

static inline void power_sysfs_remove_link_group(
	const char *cls_name, const char *dev_name, const char *link_name,
	struct device *target_dev, const struct attribute_group *group)
{
}

static inline struct device *power_sysfs_create_group(
	const char *cls_name, const char *dev_name,
	const struct attribute_group *group)
{
	return NULL;
}

static inline void power_sysfs_remove_group(struct device *dev,
	const struct attribute_group *group)
{
}
#endif /* CONFIG_SYSFS */

#endif /* _POWER_SYSFS_H_ */
