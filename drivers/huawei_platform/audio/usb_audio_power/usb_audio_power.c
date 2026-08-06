/*
 * usb_audio_power.c
 *
 * usb audio power driver
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
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/wakelock.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#ifdef CONFIG_SUPERSWITCH_FSC
#include <huawei_platform/usb/superswitch/fsc/core/hw_scp.h>
#endif
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wired_channel_switch.h>
#endif
#ifdef CONFIG_BOOST_5V
#include <huawei_platform/power/boost_5v.h>
#endif
#include "huawei_platform/audio/usb_audio_power.h"

#define HWLOG_TAG usb_audio_power
HWLOG_REGIST();

#ifdef CONFIG_SUPERSWITCH_FSC
/* DBC station: wait for usb disconnect when accept the AT command. */
#define USB_DISCONNECT_WAIT_TIME     4000 /* ms */
#endif

enum usb_audio_power_gpio_type {
	USB_AUDIO_POWER_GPIO_SOC           = 0,
	USB_AUDIO_POWER_GPIO_CODEC         = 1,
};

enum usb_audio_power_gpio_one_ctrl {
	USB_AUDIO_POWER_GPIO_ONE         = 1,
	USB_AUDIO_POWER_GPIO_DEFAUIT     = 0,
};

#ifdef CONFIG_SUPERSWITCH_FSC
enum {
	NOT_USING_SUPWRSWITCH              = 0,
	USING_SUPWRSWITCH                  = 1,
};

enum {
	SUPERSWITCH_VOUT_SWITCH_OPEN       = 0,
	SUPERSWITCH_VOUT_SWITCH_CLOSE      = 1,
};
#endif

#ifdef CONFIG_WIRELESS_CHARGER
enum {
	NOT_USING_WIRELESS_CHARGER         = 0,
	USING_WIRELESS_CHARGER             = 1,
};
#endif

#ifdef CONFIG_BOOST_5V
enum {
	NOT_USING_BOOST_5V                 = 0,
	USING_BOOST_5V                     = 1,
};
#endif

struct usb_audio_power_data {
#ifdef CONFIG_SUPERSWITCH_FSC
	int using_superswitch;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	int using_wireless_charger;
#endif
#ifdef CONFIG_BOOST_5V
	int using_boost_5v;
#endif
	int gpio_chg_vbst_ctrl;
	int gpio_ear_power_en;
	int gpio_type;
	int gpio_low_power_ctrl;
	int gpio_one_ctrl;
	/* used to static PM vboost ctrl */
	bool pm_ref;
	/* used to static audio vboost ctrl */
	bool audio_ref;
	bool audio_buckboost_enable;

	struct mutex vboost_ctrl_lock;
	struct wake_lock wake_lock;
#ifdef CONFIG_SUPERSWITCH_FSC
	struct workqueue_struct *superswitch_vout_switch_delay_wq;
	struct delayed_work superswitch_vout_switch_delay_work;
#endif
};

static struct usb_audio_power_data *power_pdata;

static inline int usb_audio_power_gpio_get_value(int gpio)
{
	if (power_pdata->gpio_type == USB_AUDIO_POWER_GPIO_CODEC)
		return gpio_get_value_cansleep(gpio);
	return gpio_get_value(gpio);
}

static inline void power_gpio_set_value(int gpio, int value)
{
	hwlog_info("%s: gpio %d, value %d\n", __func__, gpio, value);
	if (power_pdata->gpio_type == USB_AUDIO_POWER_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static void audio_power_set(enum vboost_control_source_type type)
{
	if (type == VBOOST_CONTROL_PM) {
		power_pdata->pm_ref = true;
		if (!power_pdata->audio_ref)
			power_gpio_set_value(power_pdata->gpio_chg_vbst_ctrl,
				AUDIO_POWER_GPIO_SET);
	} else if (type == VBOOST_CONTROL_AUDIO) {
		power_pdata->audio_ref = true;
		if (!power_pdata->pm_ref)
			power_gpio_set_value(power_pdata->gpio_chg_vbst_ctrl,
				AUDIO_POWER_GPIO_SET);
	}
}

static void audio_power_reset(enum vboost_control_source_type type)
{
	if (type == VBOOST_CONTROL_PM) {
		power_pdata->pm_ref = false;
		if (!power_pdata->audio_ref)
			power_gpio_set_value(power_pdata->gpio_chg_vbst_ctrl,
				AUDIO_POWER_GPIO_RESET);
	} else if (type == VBOOST_CONTROL_AUDIO) {
		power_pdata->audio_ref = false;
		if (!power_pdata->pm_ref)
			power_gpio_set_value(power_pdata->gpio_chg_vbst_ctrl,
				AUDIO_POWER_GPIO_RESET);
	}
}

int bst_ctrl_enable(bool enable, enum vboost_control_source_type type)
{
	if (!power_pdata) {
		hwlog_warn("power_pdata is NULL\n");
		return -ENOMEM;
	}

	mutex_lock(&power_pdata->vboost_ctrl_lock);
	if ((power_pdata->pm_ref == enable && type == VBOOST_CONTROL_PM) ||
		(power_pdata->audio_ref == enable &&
		type == VBOOST_CONTROL_AUDIO)) {
		mutex_unlock(&power_pdata->vboost_ctrl_lock);
		return 0;
	}
	hwlog_info("%s: enable %d, type %d\n", __func__, enable, type);
	if (enable)
		audio_power_set(type);
	else
		audio_power_reset(type);
	mutex_unlock(&power_pdata->vboost_ctrl_lock);
	return 0;
}

static int buckboost_voltage_control(void)
{
#ifdef CONFIG_BOOST_5V
	if (power_pdata->using_boost_5v == USING_BOOST_5V)
		boost_5v_enable(true, BOOST_CTRL_AUDIO);
#endif
	if (power_pdata->gpio_one_ctrl > 0) {
		power_gpio_set_value(power_pdata->gpio_low_power_ctrl,
			AUDIO_POWER_GPIO_SET);
	} else {
		bst_ctrl_enable(true, VBOOST_CONTROL_AUDIO);
		power_gpio_set_value(power_pdata->gpio_ear_power_en,
			AUDIO_POWER_GPIO_SET);
	}
	pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
	power_pdata->audio_buckboost_enable = true;
	hwlog_info("%s", __func__);
	return 0;
}

int usb_audio_power_buckboost(void)
{
	if (!power_pdata) {
		hwlog_warn("power_pdata is NULL\n");
		return -ENOMEM;
	}

	if (!power_pdata->audio_buckboost_enable && pd_dpm_get_pd_source_vbus())
		buckboost_voltage_control();

	return 0;
}

static int usb_audio_power_buckboost_no_headset(void)
{
	if (!power_pdata) {
		hwlog_warn("power_pdata is NULL\n");
		return -ENOMEM;
	}

	if (!power_pdata->audio_buckboost_enable)
		buckboost_voltage_control();

	return 0;
}

int usb_audio_power_scharger(void)
{
	if (!power_pdata) {
		hwlog_warn("power_pdata is NULL\n");
		return -ENOMEM;
	}

	if (power_pdata->audio_buckboost_enable) {
		if (power_pdata->gpio_one_ctrl > 0) {
			power_gpio_set_value(power_pdata->gpio_low_power_ctrl,
				AUDIO_POWER_GPIO_RESET);
		} else {
			bst_ctrl_enable(false, VBOOST_CONTROL_AUDIO);
			power_gpio_set_value(power_pdata->gpio_ear_power_en,
				AUDIO_POWER_GPIO_RESET);
		}
#ifdef	CONFIG_BOOST_5V
		if (power_pdata->using_boost_5v == USING_BOOST_5V)
			boost_5v_enable(false, BOOST_CTRL_AUDIO);
#endif
		power_pdata->audio_buckboost_enable = false;
		hwlog_info("%s", __func__);
	}
	return 0;
}

#ifdef CONFIG_SUPERSWITCH_FSC
static void superswitch_vout_switch_work(struct work_struct *work)
{
	usb_audio_power_buckboost();
	FUSB3601_vout_enable(SUPERSWITCH_VOUT_SWITCH_CLOSE);
}
#endif

static int usb_audio_power_ioctl_no_headset(void)
{
	int ret;

#ifdef CONFIG_WIRELESS_CHARGER
	if (power_pdata->using_wireless_charger == USING_WIRELESS_CHARGER)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
#endif
#ifdef CONFIG_SUPERSWITCH_FSC
	if (power_pdata->using_superswitch == USING_SUPWRSWITCH) {
		queue_delayed_work(
			power_pdata->superswitch_vout_switch_delay_wq,
			&power_pdata->superswitch_vout_switch_delay_work,
			msecs_to_jiffies(USB_DISCONNECT_WAIT_TIME));
		ret = 0;
	} else {
#endif
		ret = usb_audio_power_buckboost_no_headset();
#ifdef CONFIG_SUPERSWITCH_FSC
	}
#endif
	return ret;
}

static int usb_audio_power_ioctl_scharger(void)
{
#ifdef CONFIG_WIRELESS_CHARGER
	if (power_pdata->using_wireless_charger == USING_WIRELESS_CHARGER)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
#endif
	return usb_audio_power_scharger();

}

static long usb_audio_power_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	int ret;

	switch (cmd) {
	case IOCTL_USB_AUDIO_POWER_BUCKBOOST_NO_HEADSET_CMD:
		ret = usb_audio_power_ioctl_no_headset();
		break;
	case IOCTL_USB_AUDIO_POWER_SCHARGER_CMD:
		ret = usb_audio_power_ioctl_scharger();
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct of_device_id usb_audio_power_of_match[] = {
	{ .compatible = "huawei,usb_audio_power", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_audio_power_of_match);

/* load dts config for board difference */
static void load_usb_audio_power_config(struct device_node *node)
{
	int temp;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		power_pdata->gpio_type = temp;
	else
		power_pdata->gpio_type = USB_AUDIO_POWER_GPIO_SOC;
#ifdef CONFIG_SUPERSWITCH_FSC
	if (!of_property_read_u32(node, "using_superswitch", &temp))
		power_pdata->using_superswitch = temp;
	else
		power_pdata->using_superswitch = NOT_USING_SUPWRSWITCH;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	if (!of_property_read_u32(node, "using_wireless_charger", &temp))
		power_pdata->using_wireless_charger = temp;
	else
		power_pdata->using_wireless_charger = NOT_USING_WIRELESS_CHARGER;
#endif
#ifdef CONFIG_BOOST_5V
	if (!of_property_read_u32(node, "using_boost_5v", &temp))
		power_pdata->using_boost_5v = temp;
	else
		power_pdata->using_boost_5v = NOT_USING_BOOST_5V;
#endif
}

static void load_usb_audio_power_config_one_gpio(struct device_node *node)
{
	int temp;

	if (!of_property_read_u32(node, "gpio_one_ctrl", &temp))
		power_pdata->gpio_one_ctrl = temp;
	else
		power_pdata->gpio_one_ctrl = USB_AUDIO_POWER_GPIO_DEFAUIT;
}

static const struct file_operations usb_audio_power_fops = {
	.owner            = THIS_MODULE,
	.open            = simple_open,
	.unlocked_ioctl = usb_audio_power_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl    = usb_audio_power_ioctl,
#endif
};

static struct miscdevice usb_audio_power_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "usb_audio_power",
	.fops =     &usb_audio_power_fops,
};

static int gpio_one_ctrl_set(struct device_node *node)
{
	int ret;

	/* get gpio_low_power_ctrl gpio */
	power_pdata->gpio_low_power_ctrl = of_get_named_gpio(
			node, "gpio_low_power_ctrl", 0);
	if (power_pdata->gpio_low_power_ctrl < 0) {
		hwlog_err("gpio_low_power_ctrl is unvalid\n");
		ret = -ENOENT;
		goto err_out;
	}

	if (!gpio_is_valid(power_pdata->gpio_low_power_ctrl)) {
		hwlog_err("gpio is unvalid\n");
		ret = -ENOENT;
		goto err_out;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(power_pdata->gpio_low_power_ctrl,
		"gpio_low_power_ctrl");
	if (ret) {
		hwlog_err("request GPIO for low power ctrl fail %d\n", ret);
		goto err_out;
	}
	gpio_direction_output(power_pdata->gpio_low_power_ctrl, 0);
err_out:
	return ret;
}

static int gpio_chg_vbst_ctrl_set(struct device_node *node)
{
	int ret;

	/* get gpio_chg_vbst_ctrl gpio */
	power_pdata->gpio_chg_vbst_ctrl =  of_get_named_gpio(node,
					"gpio_chg_vbst_ctrl", 0);
	if (power_pdata->gpio_chg_vbst_ctrl < 0) {
		hwlog_err("gpio_chg_vbst_ctrl is unvalid\n");
		return -ENOENT;
	}

	if (!gpio_is_valid(power_pdata->gpio_chg_vbst_ctrl)) {
		hwlog_err("gpio is unvalid\n");
		return -ENOENT;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(power_pdata->gpio_chg_vbst_ctrl,
		"gpio_chg_vbst_ctrl");
	if (ret) {
		hwlog_err("error request GPIO for vbst ctrl fail %d\n", ret);
		return -ENOENT;
	}
	gpio_direction_output(power_pdata->gpio_chg_vbst_ctrl, 0);

	/* get gpio_ear_power_en gpio */
	power_pdata->gpio_ear_power_en = of_get_named_gpio(node,
		"gpio_ear_power_en", 0);
	if (power_pdata->gpio_ear_power_en < 0) {
		hwlog_err("gpio_ear_power_en is unvalid\n");
		ret = -ENOENT;
		goto gpio_chg_vbst_ctrl_err;
	}

	if (!gpio_is_valid(power_pdata->gpio_ear_power_en)) {
		hwlog_err("gpio is unvalid\n");
		ret = -ENOENT;
		goto gpio_chg_vbst_ctrl_err;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(power_pdata->gpio_ear_power_en, "gpio_ear_power_en");
	if (ret) {
		hwlog_err("error request GPIO for mic_gnd fail %d\n", ret);
		goto gpio_chg_vbst_ctrl_err;
	}
	gpio_direction_output(power_pdata->gpio_ear_power_en, 0);
	return 0;

gpio_chg_vbst_ctrl_err:
	gpio_free(power_pdata->gpio_chg_vbst_ctrl);
	return ret;
}

static int usb_audio_power_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node =  dev->of_node;
	int ret = -1;

	power_pdata = devm_kzalloc(dev, sizeof(*power_pdata), GFP_KERNEL);
	if (!power_pdata)
		return -ENOMEM;
	power_pdata->pm_ref = false;
	power_pdata->audio_ref = false;
	power_pdata->audio_buckboost_enable = false;
	mutex_init(&power_pdata->vboost_ctrl_lock);

	load_usb_audio_power_config_one_gpio(node);
	if (power_pdata->gpio_one_ctrl > 0) {
		if (gpio_one_ctrl_set(node))
			goto err_out;
	} else {
		if (gpio_chg_vbst_ctrl_set(node))
			goto err_out;
	}
#ifdef CONFIG_SUPERSWITCH_FSC
	power_pdata->superswitch_vout_switch_delay_wq =
		create_singlethread_workqueue(
		"superswitch_vout_switch_delay_wq");
	if (!(power_pdata->superswitch_vout_switch_delay_wq)) {
		hwlog_err("%s : vout switch create failed\n", __func__);
		ret = -ENOMEM;
		goto gpio_ear_power_en_err;
	}
	INIT_DELAYED_WORK(&power_pdata->superswitch_vout_switch_delay_work,
		superswitch_vout_switch_work);
#endif
	load_usb_audio_power_config(node);

	ret = misc_register(&usb_audio_power_miscdev);
	if (ret) {
		hwlog_err("%s: register usb audio power miscdev fail, ret:%d\n",
			__func__, ret);
#ifdef CONFIG_SUPERSWITCH_FSC
		goto superswitch_vout_switch_delay_wq_err;
#else
		goto gpio_ear_power_en_err;
#endif
	}

	hwlog_info("usb_audio_power probe success\n");

	return 0;
#ifdef CONFIG_SUPERSWITCH_FSC
superswitch_vout_switch_delay_wq_err:
	if (power_pdata->superswitch_vout_switch_delay_wq) {
		cancel_delayed_work(
			&power_pdata->superswitch_vout_switch_delay_work);
		flush_workqueue(power_pdata->superswitch_vout_switch_delay_wq);
		destroy_workqueue(power_pdata->superswitch_vout_switch_delay_wq);
	}
#endif
gpio_ear_power_en_err:
	gpio_free(power_pdata->gpio_ear_power_en);
	gpio_free(power_pdata->gpio_chg_vbst_ctrl);
err_out:
	if (power_pdata->gpio_one_ctrl > 0)
		gpio_free(power_pdata->gpio_low_power_ctrl);
	devm_kfree(dev, power_pdata);
	power_pdata = NULL;

	return ret;
}

static int usb_audio_power_remove(struct platform_device *pdev)
{
	if (!power_pdata)
		goto out_err;
	if (power_pdata->gpio_chg_vbst_ctrl > 0)
		gpio_free(power_pdata->gpio_chg_vbst_ctrl);

	if (power_pdata->gpio_ear_power_en > 0)
		gpio_free(power_pdata->gpio_ear_power_en);

	if (power_pdata->gpio_low_power_ctrl > 0)
		gpio_free(power_pdata->gpio_low_power_ctrl);

#ifdef CONFIG_SUPERSWITCH_FSC
	if (power_pdata->superswitch_vout_switch_delay_wq) {
		cancel_delayed_work(
			&power_pdata->superswitch_vout_switch_delay_work);
		flush_workqueue(power_pdata->superswitch_vout_switch_delay_wq);
		destroy_workqueue(power_pdata->superswitch_vout_switch_delay_wq);
	}
#endif
	devm_kfree(&pdev->dev, power_pdata);
	power_pdata = NULL;
out_err:
	misc_deregister(&usb_audio_power_miscdev);
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static struct platform_driver usb_audio_power_driver = {
	.driver = {
		.name   = "usb_audio_power",
		.owner  = THIS_MODULE,
		.of_match_table = usb_audio_power_of_match,
	},
	.probe  = usb_audio_power_probe,
	.remove = usb_audio_power_remove,
};

static int __init usb_audio_power_init(void)
{
	return platform_driver_register(&usb_audio_power_driver);
}

static void __exit usb_audio_power_exit(void)
{
	platform_driver_unregister(&usb_audio_power_driver);
}

fs_initcall(usb_audio_power_init);
module_exit(usb_audio_power_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb audio power control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
