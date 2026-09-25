/*
 * hw_accessory.c
 *
 * huawei accessory driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/hw_accessory.h>
#include <huawei_platform/power/power_sysfs.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_accessory
HWLOG_REGIST();

static struct device *g_accessory_device;
static struct acc_info *g_accessory_di;

void accessory_notify_android_uevent(struct key_info *p_dev_info,
	u8 dev_info_no, u8 dev_no)
{
	int i;
	char *p_uevent_data[ACC_DEV_INFO_NUM_MAX] = { 0 };
	char *p_data = NULL;
	char *p_data_tmp = NULL;

	hwlog_info("accessory notify uevent begin\n");
	if (!g_accessory_di || !g_accessory_device ||
		(dev_no >= ACC_DEV_NO_MAX) ||
		(dev_info_no > ACC_DEV_INFO_NUM_MAX)) {
		hwlog_err("input is invalid, accessory not notify uevent\n");
		return;
	}
	g_accessory_di->dev_info_tab[dev_no].info_no = dev_info_no;
	g_accessory_di->dev_info_tab[dev_no].p_info = p_dev_info;
	p_data = kzalloc(((dev_info_no * ACC_DEV_INFO_LEN) + 1), GFP_KERNEL);
	if (!p_data)
		return;

	p_data_tmp = p_data;
	for (i = 0; i < dev_info_no; i++) {
		snprintf(p_data_tmp, ACC_DEV_INFO_LEN, "%s=%s",
			p_dev_info[i].name, p_dev_info[i].value);
		p_uevent_data[i] = p_data_tmp;
		p_data_tmp += ACC_DEV_INFO_LEN;
		hwlog_info("acc uevent_data[%d]:%s\n", i, p_uevent_data[i]);
	}
	kobject_uevent_env(&g_accessory_device->kobj,
		KOBJ_CHANGE, p_uevent_data);
	kfree(p_data);
	hwlog_info("accessory notify uevent end\n");
}
EXPORT_SYMBOL_GPL(accessory_notify_android_uevent);

static ssize_t accessory_dev_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int dev_no;
	int info_no;
	char acc_info[ACC_DEV_INFO_MAX] = { 0 };
	char *p_acc_info_tmp = acc_info;
	struct dev_info *p_di = NULL;
	struct key_info *p_info = NULL;

	if (!g_accessory_di) {
		hwlog_err("%s: g_accessory_di is null\n", __func__);
		goto out;
	}

	for (dev_no = ACC_DEV_NO_BEGIN; dev_no < ACC_DEV_NO_MAX; dev_no++) {
		p_di = &g_accessory_di->dev_info_tab[dev_no];
		if (!p_di->p_info) {
			hwlog_info("acc dev[%d] info is null\n", dev_no);
			continue;
		}
		for (info_no = 0; info_no < p_di->info_no; info_no++) {
			p_info = p_di->p_info + info_no;
			if (strlen(acc_info) >= (ACC_DEV_INFO_MAX -
				ACC_DEV_INFO_LEN)) {
				hwlog_err("%s: info show large\n", __func__);
				goto out;
			}
			snprintf(p_acc_info_tmp, ACC_DEV_INFO_LEN,
				"%s=%s,", p_info->name, p_info->value);
			/* info contain '=' and ',' two bytes */
			p_acc_info_tmp += strlen(p_info->name) +
				strlen(p_info->value) + 2;
		}
		/* two info separated by ','; two devices separated by ';' */
		if (dev_no == (ACC_DEV_NO_MAX - 1))
			p_acc_info_tmp[strlen(p_acc_info_tmp) - 1] = '\0';
		else
			p_acc_info_tmp[strlen(p_acc_info_tmp) - 1] = ';';
	}
out:
	return scnprintf(buf, PAGE_SIZE, "%s\n", acc_info);
}

static DEVICE_ATTR(dev, 0440, accessory_dev_info_show, NULL);

static struct attribute *accessory_ctrl_attributes[] = {
	&dev_attr_dev.attr,
	NULL,
};
static const struct attribute_group accessory_attr_group = {
	.attrs = accessory_ctrl_attributes,
};

static struct device *accessory_create_group(void)
{
	return power_sysfs_create_group("hw_accessory", "monitor",
		&accessory_attr_group);
}

static void accessory_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &accessory_attr_group);
}

static int accessory_probe(struct platform_device *pdev)
{
	struct acc_info *di = NULL;

	hwlog_info("acc probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_accessory_di = di;
	di->pdev = pdev;
	di->dev = &pdev->dev;

	g_accessory_device = accessory_create_group();
	platform_set_drvdata(pdev, di);

	hwlog_info("acc probe end\n");
	return 0;
}

static int accessory_remove(struct platform_device *pdev)
{
	struct acc_info *di = platform_get_drvdata(pdev);

	hwlog_info("acc remove begin\n");

	if (!di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	accessory_remove_group(g_accessory_device);
	g_accessory_device = NULL;
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_accessory_di = NULL;

	hwlog_info("acc remove end\n");
	return 0;
}

static const struct of_device_id accessory_match_table[] = {
	{
		.compatible = "huawei,accessory",
		.data = NULL,
	},
	{},
};

static struct platform_driver accessory_driver = {
	.probe = accessory_probe,
	.remove = accessory_remove,
	.driver = {
		.name = "huawei,accessory",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(accessory_match_table),
	},
};

static int __init accessory_init(void)
{
	return platform_driver_register(&accessory_driver);
}

static void __exit accessory_exit(void)
{
	platform_driver_unregister(&accessory_driver);
}

late_initcall(accessory_init);
module_exit(accessory_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei accessory module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

