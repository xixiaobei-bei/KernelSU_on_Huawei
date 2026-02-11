/*
 * anc_hs_default.c
 *
 * analog headset default driver
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
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <huawei_platform/log/hw_log.h>
#include "huawei_platform/audio/anc_hs_default.h"

#define HWLOG_TAG anc_hs_default
HWLOG_REGIST();

struct anc_hs_default_priv {
	struct device *dev;
	int headset_type;
};

static struct anc_hs_default_priv *g_anc_hs_default_priv;

void anc_hs_default_refresh_headset_type(int headset_type)
{
	if (g_anc_hs_default_priv) {
		g_anc_hs_default_priv->headset_type = headset_type;
		hwlog_info("hs_default: refresh headset_type %d",
			g_anc_hs_default_priv->headset_type);
	}
}

static struct anc_hs_ops anc_hs_default_ops = {
	.anc_hs_dev_register = NULL,
	.anc_hs_check_headset_pluged_in = NULL,
	.anc_hs_start_charge = NULL,
	.anc_hs_charge_detect = NULL,
	.anc_hs_stop_charge = NULL,
	.anc_hs_force_charge = NULL,
	.check_anc_hs_support = NULL,
	.anc_hs_plug_enable = NULL,
	.anc_hs_5v_control = NULL,
	.anc_hs_invert_hs_control = NULL,
	.anc_hs_refresh_headset_type = anc_hs_default_refresh_headset_type,
};

static long anc_hs_default_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret;
	int adc_value;
	unsigned int __user *p_user = (unsigned int __user *)arg;

	if (!g_anc_hs_default_priv)
		return -ENODEV;

	switch (cmd) {
	case IOCTL_ANC_HS_GET_HEADSET_CMD:
		ret = put_user((__u32)(g_anc_hs_default_priv->headset_type),
			p_user);
		break;
	case IOCTL_ANC_HS_GET_HEADSET_RESISTANCE_CMD:
		adc_value = 0;
		ret = put_user((__u32)(adc_value), p_user);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations anc_hs_default_fops = {
	.owner                  = THIS_MODULE,
	.open                   = simple_open,
	.unlocked_ioctl         = anc_hs_default_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl           = anc_hs_default_ioctl,
#endif
};

static struct miscdevice anc_hs_default_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "anc_hs",
	.fops   = &anc_hs_default_fops,
};

static int anc_hs_default_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;

	hwlog_info("%s++\n", __func__);

	g_anc_hs_default_priv = devm_kzalloc(dev, sizeof(*g_anc_hs_default_priv),
		GFP_KERNEL);
	if (!g_anc_hs_default_priv)
		return -ENOMEM;

	g_anc_hs_default_priv->dev = dev;

	ret = anc_hs_ops_register(&anc_hs_default_ops);
	if (ret) {
		hwlog_err("register anc_hs_interface ops failed\n");
		goto err_out;
	}

	ret = misc_register(&anc_hs_default_device);
	if (ret) {
		hwlog_err("%s: anc_hs_default misc device register failed",
			__func__);
		goto err_out;
	}

	hwlog_info("%s--\n", __func__);

	return 0;

err_out:
	devm_kfree(g_anc_hs_default_priv->dev, g_anc_hs_default_priv);
	g_anc_hs_default_priv = NULL;
	return ret;
}

static int anc_hs_default_remove(struct platform_device *pdev)
{
	devm_kfree(g_anc_hs_default_priv->dev, g_anc_hs_default_priv);
	g_anc_hs_default_priv = NULL;

	misc_deregister(&anc_hs_default_device);

	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static const struct of_device_id anc_hs_default_match_table[] = {
	{
		.compatible = "huawei,anc_hs_default",
		.data = NULL,
	},
	{},
};

static struct platform_driver anc_hs_default_driver = {
	.probe = anc_hs_default_probe,
	.remove = anc_hs_default_remove,

	.driver = {
		.name = "huawei,anc_hs_default",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(anc_hs_default_match_table),
	},
};

static int __init anc_hs_default_init(void)
{
	return platform_driver_register(&anc_hs_default_driver);
}

static void __exit anc_hs_default_exit(void)
{
	platform_driver_unregister(&anc_hs_default_driver);
}

device_initcall_sync(anc_hs_default_init);
module_exit(anc_hs_default_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("anc hs default module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
