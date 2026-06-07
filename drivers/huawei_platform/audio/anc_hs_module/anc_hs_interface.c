/*
 * anc_hs_interface.c
 *
 * analog headset interface
 *
 * Copyright (c) 2014-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/pm_wakeup.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <huawei_platform/log/hw_log.h>
#include "huawei_platform/audio/anc_hs_interface.h"

static struct anc_hs_ops *g_anc_ops;

int anc_hs_interface_dev_register(struct anc_hs_dev *dev, void *codec_data)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_dev_register)
		return 0;

	return g_anc_ops->anc_hs_dev_register(dev, codec_data);
}

int anc_hs_interface_check_headset_pluged_in(void)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_check_headset_pluged_in)
		return NO_MAX14744;

	if (g_anc_ops->anc_hs_check_headset_pluged_in())
		return HANDSET_PLUG_IN;

	return HANDSET_PLUG_OUT;
}

void anc_hs_interface_start_charge(void)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_start_charge)
		return;

	g_anc_ops->anc_hs_start_charge();
}

bool anc_hs_interface_charge_detect(int saradc_value, int headset_type)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_charge_detect)
		return false;

	return g_anc_ops->anc_hs_charge_detect(saradc_value, headset_type);
}

void anc_hs_interface_stop_charge(void)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_stop_charge)
		return;

	g_anc_ops->anc_hs_stop_charge();
}

void anc_hs_interface_force_charge(int disable)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_force_charge)
		return;

	g_anc_ops->anc_hs_force_charge(disable);
}

bool check_anc_hs_interface_support(void)
{
	if (!g_anc_ops || !g_anc_ops->check_anc_hs_support)
		return false;

	return g_anc_ops->check_anc_hs_support();
}

void anc_hs_interface_5v_control(int enable)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_5v_control)
		return;

	g_anc_ops->anc_hs_5v_control(enable);
}

bool anc_hs_interface_plug_enable(void)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_plug_enable)
		return false;

	return g_anc_ops->anc_hs_plug_enable();
}

void anc_hs_interface_invert_hs_control(int connect)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_invert_hs_control)
		return;

	g_anc_ops->anc_hs_invert_hs_control(connect);
}

void anc_hs_interface_refresh_headset_type(int headset_type)
{
	if (!g_anc_ops || !g_anc_ops->anc_hs_refresh_headset_type)
		return;

	g_anc_ops->anc_hs_refresh_headset_type(headset_type);
}

int anc_hs_ops_register(struct anc_hs_ops *ops)
{
	int ret = 0;

	if (!ops) {
		pr_err("anc_hs interface ops register fail\n");
		ret = -EPERM;
	} else {
		g_anc_ops = ops;
	}

	return ret;
}

static int anc_hs_interface_probe(struct platform_device *pdev)
{
	return 0;
}

static int anc_hs_interface_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id anc_hs_interface_match_table[] = {
	{
		.compatible = "huawei,anc_hs_interface",
		.data = NULL,
	},
	{},
};

static struct platform_driver anc_hs_interface_driver = {
	.probe = anc_hs_interface_probe,
	.remove = anc_hs_interface_remove,

	.driver = {
		.name = "huawei,anc_hs_interface",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(anc_hs_interface_match_table),
	},
};

static int __init anc_hs_interface_init(void)
{
	return platform_driver_register(&anc_hs_interface_driver);
}

static void __exit anc_hs_interface_exit(void)
{
	platform_driver_unregister(&anc_hs_interface_driver);
}

device_initcall_sync(anc_hs_interface_init);
module_exit(anc_hs_interface_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("anc hs module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
