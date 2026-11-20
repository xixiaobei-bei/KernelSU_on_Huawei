/*
 * hw_hishow.c
 *
 * hishow driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/usb/hw_hishow.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <huawei_platform/log/hw_log.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_hishow
HWLOG_REGIST();

static struct device *g_hishow_dev;
static struct hishow_info *g_hishow_di;

static ssize_t hishow_dev_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	static const char * const dev_state[] = {
		[HISHOW_UNKNOWN] = "UNKNOWN",
		[HISHOW_DISCONNECTED] = "DISCONNECTED",
		[HISHOW_CONNECTED] = "CONNECTED",
	};
	const char *cur_state = dev_state[HISHOW_UNKNOWN];

	if (!g_hishow_di) {
		hwlog_err("g_hishow_di is null\n");
		return scnprintf(buf, PAGE_SIZE, "%s\n", cur_state);
	}

	hwlog_info("%d:%x\n", g_hishow_di->dev_state, g_hishow_di->dev_no);

	switch (g_hishow_di->dev_state) {
	case HISHOW_DEVICE_OFFLINE:
		cur_state = dev_state[HISHOW_DISCONNECTED];
		break;
	case HISHOW_DEVICE_ONLINE:
		cur_state = dev_state[HISHOW_CONNECTED];
		break;
	default:
		cur_state = dev_state[HISHOW_UNKNOWN];
		break;
	}

	return scnprintf(buf, PAGE_SIZE, "%s:%d\n",
		cur_state, g_hishow_di->dev_no);
}

static DEVICE_ATTR(dev, 0440, hishow_dev_info_show, NULL);

static struct attribute *hishow_ctrl_attributes[] = {
	&dev_attr_dev.attr,
	NULL,
};
static const struct attribute_group hishow_attr_group = {
	.attrs = hishow_ctrl_attributes,
};

static struct device *hishow_create_group(void)
{
	return power_sysfs_create_group("hishow", "monitor",
		&hishow_attr_group);
}

static void hishow_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &hishow_attr_group);
}

void hishow_notify_android_uevent(int dev_state, int dev_no)
{
	char *offline[HISHOW_STATE_MAX] = {
		"HISHOWDEV_STATE=DISCONNECTED", NULL, NULL
	};
	char *online[HISHOW_STATE_MAX] = {
		"HISHOWDEV_STATE=CONNECTED", NULL, NULL
	};
	char *unknown[HISHOW_STATE_MAX] = {
		"HISHOWDEV_STATE=UNKNOWN", NULL, NULL
	};
	char device_data[HISHOW_DEV_DATA_MAX] = {0};

	if (IS_ERR(g_hishow_dev) || !g_hishow_di) {
		hwlog_err("g_hishow_dev or g_hishow_di is null\n");
		return;
	}

	if ((dev_no <= HISHOW_DEVICE_BEGIN) ||
		(dev_no >= HISHOW_DEVICE_END)) {
		hwlog_err("invalid hishow_devno=%d\n", dev_no);
		return;
	}

	g_hishow_di->dev_state = dev_state;
	g_hishow_di->dev_no = dev_no;

	snprintf(device_data, HISHOW_DEV_DATA_MAX, "DEVICENO=%d", dev_no);

	switch (dev_state) {
	case HISHOW_DEVICE_ONLINE:
		online[1] = device_data;
		kobject_uevent_env(&g_hishow_dev->kobj, KOBJ_CHANGE, online);
		hwlog_info("hishow_notify kobject_uevent_env connected\n");
		break;
	case HISHOW_DEVICE_OFFLINE:
		offline[1] = device_data;
		kobject_uevent_env(&g_hishow_dev->kobj, KOBJ_CHANGE, offline);
		hwlog_info("hishow_notify kobject_uevent_env disconnected\n");
		break;
	default:
		unknown[1] = device_data;
		kobject_uevent_env(&g_hishow_dev->kobj, KOBJ_CHANGE, unknown);
		hwlog_info("hishow_notify kobject_uevent_env unknown\n");
		break;
	}
}
EXPORT_SYMBOL_GPL(hishow_notify_android_uevent);

static int hishow_probe(struct platform_device *pdev)
{
	struct hishow_info *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hishow_di = di;
	di->dev_state = HISHOW_DEVICE_OFFLINE;
	di->dev_no = HISHOW_UNKNOWN_DEVICE;
	di->pdev = pdev;
	di->dev = &pdev->dev;

	g_hishow_dev = hishow_create_group();
	platform_set_drvdata(pdev, di);

	return 0;
}

static int hishow_remove(struct platform_device *pdev)
{
	struct hishow_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	hishow_remove_group(g_hishow_dev);
	g_hishow_dev = NULL;
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_hishow_di = NULL;

	return 0;
}

static const struct of_device_id hishow_match_table[] = {
	{
		.compatible = "huawei,hishow",
		.data = NULL,
	},
	{},
};

static struct platform_driver hishow_driver = {
	.probe = hishow_probe,
	.remove = hishow_remove,
	.driver = {
		.name = "huawei,hishow",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hishow_match_table),
	},
};

static int __init hishow_init(void)
{
	return platform_driver_register(&hishow_driver);
}

static void __exit hishow_exit(void)
{
	platform_driver_unregister(&hishow_driver);
}

late_initcall(hishow_init);
module_exit(hishow_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei hishow event module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
