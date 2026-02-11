/*
 * usb_analog_hs_interface.c
 *
 * usb analog headset interface driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/mutex.h>
#include <huawei_platform/log/hw_log.h>
#include "huawei_platform/audio/usb_analog_hs_interface.h"

#define HWLOG_TAG usb_ana_hs_interface
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

static struct usb_analog_hs_ops *usb_ana_hs_pdata;

bool check_usb_analog_hs_support(void)
{
	if (usb_ana_hs_pdata && usb_ana_hs_pdata->check_usb_ana_hs_support)
		return usb_ana_hs_pdata->check_usb_ana_hs_support();
	return false;
}

int usb_analog_hs_dev_register(struct usb_analog_hs_dev *dev, void *codec_data)
{
	if (usb_ana_hs_pdata && usb_ana_hs_pdata->usb_ana_hs_dev_register)
		return usb_ana_hs_pdata->usb_ana_hs_dev_register(dev,
			codec_data);
	return -ENODEV;
}

int usb_analog_hs_check_headset_pluged_in(void)
{
	if (usb_ana_hs_pdata &&
		usb_ana_hs_pdata->usb_ana_hs_check_headset_pluged_in)
		return usb_ana_hs_pdata->usb_ana_hs_check_headset_pluged_in();
	return USB_ANA_HS_PLUG_OUT;
}

void usb_analog_hs_plug_in_out_handle(int hs_state)
{
	if (usb_ana_hs_pdata && usb_ana_hs_pdata->usb_ana_hs_plug_in_out_handle)
		usb_ana_hs_pdata->usb_ana_hs_plug_in_out_handle(hs_state);
}

void usb_ana_hs_mic_switch_change_state(void)
{
	if (usb_ana_hs_pdata &&
		usb_ana_hs_pdata->usb_ana_hs_mic_switch_change_state)
		usb_ana_hs_pdata->usb_ana_hs_mic_switch_change_state();
}

int usb_analog_hs_ops_register(struct usb_analog_hs_ops *ops)
{
	if (ops) {
		usb_ana_hs_pdata = ops;
		return 0;
	}
	hwlog_err("%s ana_hs interface ops register fail\n", __func__);
	return -ENODEV;
}

static int usb_analog_hs_interface_probe(struct platform_device *pdev)
{
	return 0;
}

static int usb_analog_hs_interface_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id usb_ana_hs_match_table[] = {
	{
		.compatible = "huawei,usb_analog_hs_interface",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver usb_analog_hs_interface_driver = {
	.probe = usb_analog_hs_interface_probe,
	.remove = usb_analog_hs_interface_remove,

	.driver = {
		.name = "huawei,usb_analog_hs_interface",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(usb_ana_hs_match_table),
	},
};

static int __init usb_analog_hs_interface_init(void)
{
	return platform_driver_register(&usb_analog_hs_interface_driver);
}

static void __exit usb_analog_hs_interface_exit(void)
{
	platform_driver_unregister(&usb_analog_hs_interface_driver);
}

subsys_initcall(usb_analog_hs_interface_init);
module_exit(usb_analog_hs_interface_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb analog interface hs module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
