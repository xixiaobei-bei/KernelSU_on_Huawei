/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset power source driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "huawei_platform/audio/dig_hs.h"
#include "huawei_platform/audio/dig_hs_power.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/pm_wakeup.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/notifier.h>
#include <linux/usb.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG dig_hs_core
HWLOG_REGIST();

enum {
	DIG_HS_PLUG_IN                 = 0,
	DIG_HS_PLUG_OUT                = 1,
};

enum {
	USB_SCHARGE_BOOTST_ON               = 0,
	USB_SCHARGE_BOOTST_OFF              = 1,
};

struct dig_hs_core_data {
	/* headset plug state */
	bool hs_state;
	/* ctl scharge 5v flag */
	bool scharge_ctl_flag;
	/* switch for usb headset hw issue */
	int hs_hw_issus;

	struct notifier_block usb_nb;
};

static struct dig_hs_core_data *g_pdata;

int dig_hs_core_plug_in(void)
{
	if (!g_pdata) {
		hwlog_warn("g_pdata is NULL\n");
		return -ENOMEM;
	}

	g_pdata->hs_state = DIG_HS_PLUG_IN;
	hwlog_info("usb headset plug in\n");

	return 0;
}

int dig_hs_core_plug_out(void)
{
	if (!g_pdata) {
		hwlog_warn("g_pdata is NULL\n");
		return -ENOMEM;
	}

	if ((g_pdata->hs_hw_issus == 1) &&
		(g_pdata->scharge_ctl_flag == USB_SCHARGE_BOOTST_ON)) {
		/* turn off scharge 5v boost for hs hw issue */
		charge_otg_mode_enable(OTG_DISABLE, VBUS_CH_USER_AUDIO);
		g_pdata->scharge_ctl_flag = USB_SCHARGE_BOOTST_OFF;
		hwlog_info("headset: turn off scharge 5vboost\n");
	}

	g_pdata->hs_state = DIG_HS_PLUG_OUT;
	hwlog_info("usb headset plug out\n");

	return 0;
}


static int xhci_notifier_fn(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct usb_device *udev = data;

	if (!g_pdata) {
		hwlog_warn("g_pdata or udev is NULL\n");
		return -ENOMEM;
	}
	if (!udev) {
		hwlog_warn("udev is null,just return\n");
		return 0;
	}

	if (g_pdata->hs_hw_issus == 0)
		return 0;

	if ((action == USB_DEVICE_REMOVE) &&
		(g_pdata->hs_state == DIG_HS_PLUG_IN)) {
		if (g_pdata->scharge_ctl_flag == USB_SCHARGE_BOOTST_OFF) {
			/* turn on scharge 5v boost for hs hw issue */
			charge_otg_mode_enable(OTG_ENABLE, VBUS_CH_USER_AUDIO);
			g_pdata->scharge_ctl_flag = USB_SCHARGE_BOOTST_ON;
			hwlog_info("headset: turn on scharge 5vboost\n");
		}
	}

	return 0;
}

struct dig_hs_dev dig_hs_core_ops = {
	.tag = TAG_DIG_HS_CORE,
	.ops.dig_hs_plug_in = dig_hs_core_plug_in,
	.ops.dig_hs_plug_out = dig_hs_core_plug_out,
};

static const struct of_device_id dig_hs_core_of_match[] = {
	{ .compatible = "huawei,dig_hs_core", },
	{},
};
MODULE_DEVICE_TABLE(of, dig_hs_core_of_match);

static int dig_hs_core_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int temp = 0;

	g_pdata = devm_kzalloc(dev, sizeof(*g_pdata), GFP_KERNEL);
	if (!g_pdata) {
		hwlog_err("dig hs core alloc memory fail\n");
		return -ENOMEM;
	}

	g_pdata->hs_state = DIG_HS_PLUG_OUT;
	g_pdata->scharge_ctl_flag = USB_SCHARGE_BOOTST_OFF;


	if (!of_property_read_u32(node, "hs_hw_issus", &temp))
		g_pdata->hs_hw_issus = temp;
	else
		g_pdata->hs_hw_issus = 0;

	dig_hs_dev_register(&dig_hs_core_ops, PRIORITY_H);
	g_pdata->usb_nb.notifier_call = xhci_notifier_fn;
	usb_register_notify(&g_pdata->usb_nb);

	hwlog_info("usb_headset_core probe success\n");

	return 0;
}

static int dig_hs_core_remove(struct platform_device *pdev)
{
	if (!g_pdata)
		return -ENOMEM;

	usb_unregister_notify(&g_pdata->usb_nb);

	dig_hs_dev_deregister(TAG_DIG_HS_CORE);
	return 0;
}

static struct platform_driver dig_hs_core_driver = {
	.driver = {
		.name   = "dig_hs_core",
		.owner  = THIS_MODULE,
		.of_match_table = dig_hs_core_of_match,
	},
	.probe  = dig_hs_core_probe,
	.remove = dig_hs_core_remove,
};

static int __init dig_hs_core_init(void)
{
	return platform_driver_register(&dig_hs_core_driver);
}

static void __exit dig_hs_core_exit(void)
{
	platform_driver_unregister(&dig_hs_core_driver);
}

fs_initcall(dig_hs_core_init);
module_exit(dig_hs_core_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("digital headset core driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
