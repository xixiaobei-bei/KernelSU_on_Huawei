/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset low power V600 driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "huawei_platform/audio/dig_hs.h"
#include "huawei_platform/audio/dig_hs_power_v600.h"
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

#define HWLOG_TAG dig_hs_power_v600
HWLOG_REGIST();

/* DBC station: wait for usb disconnect when accept the AT command */
#define USB_DISCONNECT_TIME        4000 /* ms */

struct dig_hs_power_v600_data {
	int gpio_ear_power_en;
	struct workqueue_struct *v600_delay_wq;
	struct delayed_work v600_delay_work;
};

static struct dig_hs_power_v600_data *g_pdata;

__attribute__((weak)) int set_otg_switch_mode_enable(void)
{
	hwlog_info("%s use weak\n", __func__);
	return 0;
}

int dig_hs_v600_low_power(void)
{
	if (!g_pdata) {
		hwlog_warn("%s g_pdata is NULL\n", __func__);
		return -1;
	}
	if (g_pdata->gpio_ear_power_en > 0) {
		gpio_direction_output(g_pdata->gpio_ear_power_en, 1);
		hwlog_info("%s increase the charge votage\n", __func__);
	}
	set_otg_switch_mode_enable();

	return 0;
}

int dig_hs_v600_normal_power(void)
{
	if (!g_pdata) {
		hwlog_warn("%s g_pdata is NULL\n", __func__);
		return -1;
	}
	if (g_pdata->gpio_ear_power_en > 0) {
		gpio_direction_output(g_pdata->gpio_ear_power_en, 0);
		hwlog_info("%s restore the charge votage\n", __func__);
	}

	return 0;
}

static void v600_switch_work(struct work_struct *work)
{
	pd_dpm_vbus_ctrl(PLEASE_PROVIDE_POWER);
	set_otg_switch_mode_enable();
}

static long dig_hs_power_v600_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case IOCTL_DIG_HS_POWER_BUCKBOOST_NO_HEADSET_CMD:
		pd_dpm_set_audio_power_no_hs_state(true);
		queue_delayed_work(g_pdata->v600_delay_wq,
			&g_pdata->v600_delay_work,
			msecs_to_jiffies(USB_DISCONNECT_TIME));
		ret = 0;
		break;
	case IOCTL_DIG_HS_POWER_SCHARGER_CMD:
		pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
		pd_dpm_set_audio_power_no_hs_state(false);
		ret = 0;
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

struct dig_hs_dev power_v600_ops = {
	.tag = TAG_DIG_HS_POWER_V600,
	.ops.dig_hs_plug_in = dig_hs_v600_low_power,
	.ops.dig_hs_plug_out = dig_hs_v600_normal_power,
};

static const struct of_device_id dig_hs_power_v600_of_match[] = {
	{ .compatible = "huawei,dig_hs_power_v600", },
	{},
};
MODULE_DEVICE_TABLE(of, dig_hs_power_v600_of_match);

static const struct file_operations dig_hs_power_v600_fops = {
	.owner            = THIS_MODULE,
	.open             = simple_open,
	.unlocked_ioctl   = dig_hs_power_v600_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl     = dig_hs_power_v600_ioctl,
#endif
};

static struct miscdevice dig_hs_power_v600_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "dig_hs_power",
	.fops =     &dig_hs_power_v600_fops,
};

static int dig_hs_power_v600_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;

	g_pdata = devm_kzalloc(dev, sizeof(*g_pdata), GFP_KERNEL);
	if (!g_pdata) {
		hwlog_err("dig hs power v600 alloc memory fail\n");
		return -ENOMEM;
	}

	g_pdata->gpio_ear_power_en = of_get_named_gpio(node,
		"gpio_ear_power_en", 0);
	if (g_pdata->gpio_ear_power_en > 0) {
		if (!gpio_is_valid(g_pdata->gpio_ear_power_en)) {
			hwlog_err("gpio is unvalid\n");
			ret = -ENOENT;
			goto err_out;
		}

		ret = gpio_request(g_pdata->gpio_ear_power_en,
			"gpio_ear_power_en");
		if (ret) {
			hwlog_err("error request ear_power_en fail%d\n", ret);
			goto err_out;
		}
		gpio_direction_output(g_pdata->gpio_ear_power_en, 0);
	}

	g_pdata->v600_delay_wq =
		create_singlethread_workqueue("v600_delay_wq");
	if (!g_pdata->v600_delay_wq) {
		hwlog_err("%s: v600 delay work queue create fail\n", __func__);
		ret = -ENOMEM;
		goto gpio_ear_power_en_err;
	}
	INIT_DELAYED_WORK(&g_pdata->v600_delay_work, v600_switch_work);

	ret = misc_register(&dig_hs_power_v600_miscdev);
	if (ret) {
		hwlog_err("%s: register digital headset power miscdev fail, ret:%d\n",
			__func__, ret);
		goto v600_delay_wq_err;
	}

	dig_hs_dev_register(&power_v600_ops, PRIORITY_H);

	hwlog_info("dig_hs_power_v600 probe success\n");
	return 0;

v600_delay_wq_err:
	if (g_pdata->v600_delay_wq) {
		cancel_delayed_work(&g_pdata->v600_delay_work);
		flush_workqueue(g_pdata->v600_delay_wq);
		destroy_workqueue(g_pdata->v600_delay_wq);
	}
gpio_ear_power_en_err:
	if (g_pdata->gpio_ear_power_en > 0)
		gpio_free(g_pdata->gpio_ear_power_en);
err_out:
	return ret;
}

static int dig_hs_power_v600_remove(struct platform_device *pdev)
{
	if (g_pdata) {
		if (g_pdata->v600_delay_wq) {
			cancel_delayed_work(&g_pdata->v600_delay_work);
			flush_workqueue(g_pdata->v600_delay_wq);
			destroy_workqueue(g_pdata->v600_delay_wq);
		}

		if (g_pdata->gpio_ear_power_en > 0)
			gpio_free(g_pdata->gpio_ear_power_en);
	}

	misc_deregister(&dig_hs_power_v600_miscdev);
	dig_hs_dev_deregister(TAG_DIG_HS_POWER_V600);
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static struct platform_driver dig_hs_power_v600_driver = {
	.driver = {
		.name   = "dig_hs_power_v600",
		.owner  = THIS_MODULE,
		.of_match_table = dig_hs_power_v600_of_match,
	},
	.probe  = dig_hs_power_v600_probe,
	.remove = dig_hs_power_v600_remove,
};

static int __init dig_hs_power_v600_init(void)
{
	return platform_driver_register(&dig_hs_power_v600_driver);
}

static void __exit dig_hs_power_v600_exit(void)
{
	platform_driver_unregister(&dig_hs_power_v600_driver);
}

fs_initcall(dig_hs_power_v600_init);
module_exit(dig_hs_power_v600_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("digital headset power consumption control v600 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
