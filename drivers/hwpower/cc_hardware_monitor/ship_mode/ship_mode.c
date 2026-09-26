// SPDX-License-Identifier: GPL-2.0
/*
 * ship_mode.c
 *
 * ship mode control driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/hardware_monitor/ship_mode.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>

#define HWLOG_TAG ship_mode
HWLOG_REGIST();

static struct ship_mode_dev *g_ship_mode_dev;

static const char * const g_ship_mode_op_user_table[] = {
	[SHIP_MODE_OP_USER_SHELL] = "shell",
	[SHIP_MODE_OP_USER_ATCMD] = "atcmd",
	[SHIP_MODE_OP_USER_HIDL] = "hidl",
};

static ssize_t ship_mode_dbg_para_show(void *dev_data,
	char *buf, size_t size)
{
	struct ship_mode_dev *l_dev = dev_data;

	if (!buf || !l_dev) {
		hwlog_err("buf or l_dev is null\n");
		return scnprintf(buf, size, "buf or l_dev is null\n");
	}

	return scnprintf(buf, size, "delay_time=%u\nentry_time=%u\n",
		l_dev->para.delay_time, l_dev->para.entry_time);
}

static ssize_t ship_mode_dbg_para_store(void *dev_data,
	const char *buf, size_t size)
{
	struct ship_mode_dev *l_dev = dev_data;
	int delay_time = 0;
	int entry_time = 0;

	if (!buf || !l_dev) {
		hwlog_err("buf or l_dev is null\n");
		return -EINVAL;
	}

	/* 2: two parameters */
	if (sscanf(buf, "%d %d", &delay_time, &entry_time) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	l_dev->para.delay_time = delay_time;
	l_dev->para.entry_time = entry_time;

	hwlog_info("delay_time=%u\n", l_dev->para.delay_time);
	hwlog_info("entry_time=%u\n", l_dev->para.entry_time);

	return size;
}

static int ship_mode_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_ship_mode_op_user_table); i++) {
		if (!strcmp(str, g_ship_mode_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -EPERM;
}

static struct ship_mode_dev *ship_mode_get_dev(void)
{
	if (!g_ship_mode_dev) {
		hwlog_err("g_ship_mode_dev is null\n");
		return NULL;
	}

	return g_ship_mode_dev;
}

int ship_mode_ops_register(struct ship_mode_ops *ops)
{
	if (!g_ship_mode_dev || !ops || !ops->ops_name) {
		hwlog_err("g_ship_mode_dev or ops or ops_name is null\n");
		return -EPERM;
	}

	g_ship_mode_dev->ops = ops;

	hwlog_info("%s ops register ok\n", ops->ops_name);
	return 0;
}

int ship_mode_entry(const struct ship_mode_para *para)
{
	struct ship_mode_dev *l_dev = ship_mode_get_dev();
	struct ship_mode_ops *l_ops = NULL;

	if (!l_dev || !para)
		return -EINVAL;

	l_ops = l_dev->ops;
	if (!l_ops || !l_ops->set_work_mode) {
		hwlog_err("l_ops or set_work_mode is null\n");
		return -EINVAL;
	}

	hwlog_info("entry: entry_time=%u work_mode=%u delay_time=%u\n",
		para->entry_time, para->work_mode, para->delay_time);

	if (l_ops->set_entry_time)
		l_ops->set_entry_time(para->entry_time, l_ops->dev_data);

	l_ops->set_work_mode(para->work_mode, l_ops->dev_data);
	(void)power_msleep(para->delay_time, 0, NULL);

	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t ship_mode_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t ship_mode_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info ship_mode_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(ship_mode, 0440, SHIP_MODE_SYSFS_DELAY_TIME, delay_time),
	power_sysfs_attr_ro(ship_mode, 0440, SHIP_MODE_SYSFS_ENTRY_TIME, entry_time),
	power_sysfs_attr_rw(ship_mode, 0660, SHIP_MODE_SYSFS_WORK_MODE, work_mode),
};

#define SHIP_MODE_SYSFS_ATTRS_SIZE  ARRAY_SIZE(ship_mode_sysfs_field_tbl)

static struct attribute *ship_mode_sysfs_attrs[SHIP_MODE_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group ship_mode_sysfs_attr_group = {
	.attrs = ship_mode_sysfs_attrs,
};

static ssize_t ship_mode_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct ship_mode_dev *l_dev = ship_mode_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		ship_mode_sysfs_field_tbl, SHIP_MODE_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case SHIP_MODE_SYSFS_DELAY_TIME:
		return scnprintf(buf, PAGE_SIZE, "%u\n", l_dev->para.delay_time);
	case SHIP_MODE_SYSFS_ENTRY_TIME:
		return scnprintf(buf, PAGE_SIZE, "%u\n", l_dev->para.entry_time);
	case SHIP_MODE_SYSFS_WORK_MODE:
		return scnprintf(buf, PAGE_SIZE, "%u\n", l_dev->para.work_mode);
	default:
		return 0;
	}
}

static ssize_t ship_mode_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct ship_mode_dev *l_dev = ship_mode_get_dev();
	char user[SHIP_MODE_RW_BUF_SIZE] = { 0 };
	int value;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		ship_mode_sysfs_field_tbl, SHIP_MODE_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (SHIP_MODE_RW_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: the fields of "user value" */
	if (sscanf(buf, "%s %d", user, &value) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (ship_mode_get_op_user(user) < 0)
		return -EINVAL;

	hwlog_info("set: name=%d, user=%s, value=%d\n", info->name, user, value);

	switch (info->name) {
	case SHIP_MODE_SYSFS_WORK_MODE:
		if ((value != SHIP_MODE_NOT_IN_SHIP) && (value != SHIP_MODE_IN_SHIP) &&
			(value != SHIP_MODE_IN_SHUTDOWN_SHIP)) {
			hwlog_err("invalid value=%d\n", value);
			return -EINVAL;
		}
		l_dev->para.work_mode = value;
		if ((value != SHIP_MODE_IN_SHUTDOWN_SHIP) &&
			ship_mode_entry(&l_dev->para))
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static struct device *ship_mode_sysfs_create_group(void)
{
	power_sysfs_init_attrs(ship_mode_sysfs_attrs,
		ship_mode_sysfs_field_tbl, SHIP_MODE_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "ship_mode",
		&ship_mode_sysfs_attr_group);
}

static void ship_mode_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &ship_mode_sysfs_attr_group);
}
#else
static inline struct device *ship_mode_sysfs_create_group(void)
{
	return NULL;
}

static inline void ship_mode_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void ship_mode_parse_dts(struct device_node *np,
	struct ship_mode_dev *l_dev)
{
	if (power_cmdline_is_factory_mode())
		(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
			"entry_time_fac", &l_dev->para.entry_time,
			SHIP_MODE_DEFAULT_ENTRY_TIME_FAC);
	else
		(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
			"entry_time_user", &l_dev->para.entry_time,
			SHIP_MODE_DEFAULT_ENTRY_TIME_USER);

	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"delay_time", &l_dev->para.delay_time, SHIP_MODE_DEFAULT_DELAY_TIME);
}

static void ship_mode_dbg_register(struct ship_mode_dev *l_dev)
{
	power_dbg_ops_register("ship_mode_para", (void *)l_dev,
		ship_mode_dbg_para_show, ship_mode_dbg_para_store);
}

static int ship_mode_probe(struct platform_device *pdev)
{
	struct ship_mode_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_ship_mode_dev = l_dev;
	np = pdev->dev.of_node;
	ship_mode_parse_dts(np, l_dev);

	l_dev->dev = ship_mode_sysfs_create_group();
	ship_mode_dbg_register(l_dev);
	platform_set_drvdata(pdev, l_dev);
	return 0;
}

static int ship_mode_remove(struct platform_device *pdev)
{
	struct ship_mode_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	ship_mode_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_ship_mode_dev = NULL;

	return 0;
}

static void ship_mode_shutdown(struct platform_device *pdev)
{
	struct ship_mode_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return;

	if (l_dev->para.work_mode == SHIP_MODE_IN_SHUTDOWN_SHIP)
		(void)ship_mode_entry(&l_dev->para);
}

static const struct of_device_id ship_mode_match_table[] = {
	{
		.compatible = "huawei,ship_mode",
		.data = NULL,
	},
	{},
};

static struct platform_driver ship_mode_driver = {
	.probe = ship_mode_probe,
	.remove = ship_mode_remove,
	.shutdown = ship_mode_shutdown,
	.driver = {
		.name = "huawei,ship_mode",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ship_mode_match_table),
	},
};

static int __init ship_mode_init(void)
{
	return platform_driver_register(&ship_mode_driver);
}

static void __exit ship_mode_exit(void)
{
	platform_driver_unregister(&ship_mode_driver);
}

fs_initcall_sync(ship_mode_init);
module_exit(ship_mode_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ship mode control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
