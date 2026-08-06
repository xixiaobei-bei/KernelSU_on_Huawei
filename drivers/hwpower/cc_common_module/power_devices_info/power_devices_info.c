/*
 * power_devices_info.c
 *
 * power devices driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_devices_info
HWLOG_REGIST();

#define MAX_PER_DEV_LEN      64
#define MAX_DEVICES_TOTAL    16
#define POWER_DEVICE_TIMEOUT 30000

static LIST_HEAD(devices_info_list);
static unsigned int g_devices_total;
static struct power_devices_info_dev *g_devices_dev;

struct power_devices_info_data *power_devices_info_register(void)
{
	struct power_devices_info_data *temp = NULL;

	if (g_devices_total >= MAX_DEVICES_TOTAL) {
		hwlog_err("too much %u valid devices info\n", g_devices_total);
		return NULL;
	}

	temp = kzalloc(sizeof(*temp), GFP_KERNEL);
	if (temp) {
		list_add_tail(&temp->info_node, &devices_info_list);
		g_devices_total++;
	}
	return temp;
}

static void power_devices_info_release(void)
{
	struct power_devices_info_data *temp = NULL;
	struct power_devices_info_data *next = NULL;

	list_for_each_entry_safe(temp, next, &devices_info_list, info_node) {
		list_del(&temp->info_node);
		kfree(temp);
		temp = NULL;
	}
}

static void power_devices_info_dmd_report(struct work_struct *work)
{
	int ret;
	int len = 0;
	unsigned int count = 0;
	struct power_devices_info_data *temp = NULL;
	char *buf = NULL;

	if (power_cmdline_is_powerdown_charging_mode() ||
		power_cmdline_is_factory_mode()) {
		hwlog_info("no need to statistics device information\n");
		return;
	}

	if (g_devices_total == 0)
		return;

	buf = kcalloc(g_devices_total, MAX_PER_DEV_LEN, GFP_KERNEL);
	if (!buf)
		return;

	list_for_each_entry(temp, &devices_info_list, info_node) {
		if (count++ >= g_devices_total)
			break;

		if (!temp->dev_name)
			continue;

		ret = snprintf(buf + len, MAX_PER_DEV_LEN, "%s, 0x%x, 0x%x\n",
			temp->dev_name, temp->dev_id, temp->ver_id);
		if (ret < 0)
			break;

		len += ret;
	}

	power_dsm_report_dmd(POWER_DSM_BATTERY, DSM_POWER_DEVICES_INFO, buf);
	kfree(buf);
}

#ifdef CONFIG_SYSFS
static ssize_t power_devices_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int len = 0;
	unsigned int count = 0;
	struct power_devices_info_data *temp = NULL;

	if (g_devices_total == 0)
		return snprintf(buf, PAGE_SIZE, "no device information\n");

	list_for_each_entry(temp, &devices_info_list, info_node) {
		if (count++ >= g_devices_total)
			break;

		if (!temp->dev_name)
			continue;

		ret = snprintf(buf + len, MAX_PER_DEV_LEN, "%s, 0x%x, 0x%x\n",
			temp->dev_name, temp->dev_id, temp->ver_id);
		if (ret < 0)
			break;

		len += ret;
	}

	return len;
}

static DEVICE_ATTR(devices, 0440, power_devices_info_show, NULL);
static struct attribute *power_devices_info_attrs[] = {
	&dev_attr_devices.attr,
	NULL,
};

static const struct attribute_group power_devices_info_attr_group = {
	.attrs = power_devices_info_attrs,
};

static struct device *power_devices_info_create_group(void)
{
	return power_sysfs_create_group("hw_power", "power_devices",
		&power_devices_info_attr_group);
}

static void power_devices_info_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_devices_info_attr_group);
}
#else
static inline struct device *power_devices_info_create_group(void)
{
	return NULL;
}

static inline void power_devices_info_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_devices_info_init(void)
{
	struct power_devices_info_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_devices_dev = l_dev;
	l_dev->dev = power_devices_info_create_group();
	INIT_DELAYED_WORK(&l_dev->dmd_report, power_devices_info_dmd_report);
	schedule_delayed_work(&l_dev->dmd_report,
		msecs_to_jiffies(POWER_DEVICE_TIMEOUT));

	return 0;
}

static void __exit power_devices_info_exit(void)
{
	struct power_devices_info_dev *l_dev = g_devices_dev;

	if (!l_dev)
		return;

	power_devices_info_release();
	power_devices_info_remove_group(l_dev->dev);
	kfree(l_dev);
	g_devices_dev = NULL;
}

subsys_initcall_sync(power_devices_info_init);
module_exit(power_devices_info_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power devices informaiton driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
