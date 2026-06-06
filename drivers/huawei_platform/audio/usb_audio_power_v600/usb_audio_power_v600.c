/*
 * usb_audio_power_v600.c
 *
 * usb audio power v600 driver
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

#include "huawei_platform/audio/usb_audio_power_v600.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/ioctl.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG usb_audio_power_v600
HWLOG_REGIST();

/* DBC station: wait for usb disconnect when accept the AT command */
#define USB_DISCONNECT_TIME        4000 /* ms */

struct usb_audio_power_v600_data {
	int gpio_ear_power_en;
	struct workqueue_struct *v600_delay_wq;
	struct delayed_work v600_delay_work;
};

static struct usb_audio_power_v600_data *v600_pdata;

__attribute__((weak)) int set_otg_switch_mode_enable(void)
{
	hwlog_info("%s use weak\n", __func__);
	return 0;
}

void set_otg_switch_enable_v600(void)
{
	if (!v600_pdata) {
		hwlog_warn("%s pdata is NULL\n", __func__);
		return;
	}
	if (v600_pdata->gpio_ear_power_en > 0) {
		gpio_direction_output(v600_pdata->gpio_ear_power_en, 1);
		hwlog_info("%s increase the charge votage\n", __func__);
	}
	set_otg_switch_mode_enable();
}

void restore_headset_state(void)
{
	if (!v600_pdata) {
		hwlog_warn("%s pdata is NULL\n", __func__);
		return;
	}
	if (v600_pdata->gpio_ear_power_en > 0) {
		gpio_direction_output(v600_pdata->gpio_ear_power_en, 0);
		hwlog_info("%s restore the charge votage\n", __func__);
	}
}

static void v600_switch_work(struct work_struct *work)
{
	pd_dpm_vbus_ctrl(PLEASE_PROVIDE_POWER);
	set_otg_switch_mode_enable();
}

static long usb_audio_power_v600_ioctl(struct file *file, unsigned int cmd,
					unsigned long arg)
{
	int ret;

	switch (cmd) {
	case IOCTL_USB_AUDIO_POWER_BUCKBOOST_NO_HEADSET_CMD:
		queue_delayed_work(v600_pdata->v600_delay_wq,
			&v600_pdata->v600_delay_work,
			msecs_to_jiffies(USB_DISCONNECT_TIME));
		ret = 0;
		break;
	case IOCTL_USB_AUDIO_POWER_SCHARGER_CMD:
		pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
		ret = 0;
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct of_device_id usb_audio_power_v600_of_match[] = {
	{ .compatible = "huawei,usb_audio_power_v600", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_audio_power_v600_of_match);

static const struct file_operations usb_audio_power_v600_fops = {
	.owner            = THIS_MODULE,
	.open             = simple_open,
	.unlocked_ioctl   = usb_audio_power_v600_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl     = usb_audio_power_v600_ioctl,
#endif
};

static struct miscdevice usb_audio_power_v600_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "usb_audio_power",
	.fops =     &usb_audio_power_v600_fops,
};

static int usb_audio_power_v600_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;

	v600_pdata = devm_kzalloc(dev, sizeof(*v600_pdata), GFP_KERNEL);
	if (!v600_pdata)
		return -ENOMEM;

	v600_pdata->gpio_ear_power_en = of_get_named_gpio(node,
		"gpio_ear_power_en", 0);
	if (v600_pdata->gpio_ear_power_en > 0) {
		if (!gpio_is_valid(v600_pdata->gpio_ear_power_en)) {
			hwlog_err("gpio is unvalid\n");
			ret = -ENOENT;
			goto err_out;
		}

		ret = gpio_request(v600_pdata->gpio_ear_power_en,
			"gpio_ear_power_en");
		if (ret) {
			hwlog_err("error request ear_power_en fail%d\n", ret);
			goto err_out;
		}
		gpio_direction_output(v600_pdata->gpio_ear_power_en, 0);
	}

	v600_pdata->v600_delay_wq =
		create_singlethread_workqueue("v600_delay_wq");
	if (!v600_pdata->v600_delay_wq) {
		hwlog_err("%s: v600 delay work queue create fail\n", __func__);
		ret = -ENOMEM;
		goto gpio_ear_power_en_err;
	}
	INIT_DELAYED_WORK(&v600_pdata->v600_delay_work, v600_switch_work);

	ret = misc_register(&usb_audio_power_v600_miscdev);
	if (ret) {
		hwlog_err("%s: register usb audio power miscdev fail, ret:%d\n",
			__func__, ret);
		goto v600_delay_wq_err;
	}

	hwlog_info("usb_audio_power_v600 probe success\n");
	return 0;

v600_delay_wq_err:
	if (v600_pdata->v600_delay_wq) {
		cancel_delayed_work(&v600_pdata->v600_delay_work);
		flush_workqueue(v600_pdata->v600_delay_wq);
		destroy_workqueue(v600_pdata->v600_delay_wq);
	}
gpio_ear_power_en_err:
	if (v600_pdata->gpio_ear_power_en > 0)
		gpio_free(v600_pdata->gpio_ear_power_en);
err_out:
	devm_kfree(dev, v600_pdata);
	v600_pdata = NULL;
	return ret;
}

static int usb_audio_power_v600_remove(struct platform_device *pdev)
{
	if (v600_pdata) {
		if (v600_pdata->v600_delay_wq) {
			cancel_delayed_work(&v600_pdata->v600_delay_work);
			flush_workqueue(v600_pdata->v600_delay_wq);
			destroy_workqueue(v600_pdata->v600_delay_wq);
		}

		if (v600_pdata->gpio_ear_power_en > 0)
			gpio_free(v600_pdata->gpio_ear_power_en);

		devm_kfree(&pdev->dev, v600_pdata);
		v600_pdata = NULL;
	}

	misc_deregister(&usb_audio_power_v600_miscdev);
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static struct platform_driver usb_audio_power_v600_driver = {
	.driver = {
		.name   = "usb_audio_power_v600",
		.owner  = THIS_MODULE,
		.of_match_table = usb_audio_power_v600_of_match,
	},
	.probe  = usb_audio_power_v600_probe,
	.remove = usb_audio_power_v600_remove,
};

static int __init usb_audio_power_v600_init(void)
{
	return platform_driver_register(&usb_audio_power_v600_driver);
}

static void __exit usb_audio_power_v600_exit(void)
{
	platform_driver_unregister(&usb_audio_power_v600_driver);
}

fs_initcall(usb_audio_power_v600_init);
module_exit(usb_audio_power_v600_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb audio power v600 control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
