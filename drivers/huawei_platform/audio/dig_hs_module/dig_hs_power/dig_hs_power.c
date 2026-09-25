/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset low power V300 driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "huawei_platform/audio/dig_hs.h"
#include "huawei_platform/audio/dig_hs_power.h"
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

#define HWLOG_TAG dig_hs_power
HWLOG_REGIST();

#ifdef CONFIG_SUPERSWITCH_FSC
/* DBC station: wait for usb disconnect when accept the AT command. */
#define USB_DISCONNECT_WAIT_TIME     4000 /* ms */
#endif

enum dig_hs_power_gpio_type {
	DIG_HS_POWER_GPIO_SOC           = 0,
	DIG_HS_POWER_GPIO_CODEC         = 1,
};

enum dig_hs_power_gpio_one_ctrl {
	DIG_HS_POWER_GPIO_ONE         = 1,
	DIG_HS_POWER_GPIO_DEFAUIT     = 0,
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

struct dig_hs_power_data {
#ifdef CONFIG_SUPERSWITCH_FSC
	int using_superswitch;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	int using_wireless_charger;
#endif
#ifdef CONFIG_BOOST_5V
	int using_boost_5v;
#endif
	int gpio_type;
	int gpio_one_ctrl;
	int gpio_chg_vbst_ctrl;
	int gpio_ear_power_en;
	int gpio_low_power_ctrl;
	bool using_loadswitch;
	/* used to static PM vboost ctrl */
	bool pm_vboost_ref;
	/* used to static audio vboost ctrl */
	bool audio_vboost_ref;
	bool audio_buckboost_enable;
#ifdef CONFIG_WIRELESS_CHARGER
	bool using_ovp_switch;
#endif

	struct mutex vboost_ctrl_lock;
	struct wakeup_source wake_lock;
#ifdef CONFIG_SUPERSWITCH_FSC
	struct workqueue_struct *superswitch_vout_switch_delay_wq;
	struct delayed_work superswitch_vout_switch_delay_work;
#endif
};

static struct dig_hs_power_data *g_pdata;

static void dig_hs_power_gpio_set_value(int gpio, int value)
{
	hwlog_info("%s: gpio %d, value %d\n", __func__, gpio, value);
	if (g_pdata->gpio_type == DIG_HS_POWER_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static void dig_hs_power_set(enum dig_hs_vboost_control_type type)
{
	if (type == DIG_HS_VBOOST_CONTROL_PM) {
		g_pdata->pm_vboost_ref = true;
		if (!g_pdata->audio_vboost_ref)
			dig_hs_power_gpio_set_value(g_pdata->gpio_chg_vbst_ctrl,
				DIG_HS_POWER_GPIO_SET);
	} else if (type == DIG_HS_VBOOST_CONTROL_AUDIO) {
		g_pdata->audio_vboost_ref = true;
		if (!g_pdata->pm_vboost_ref)
			dig_hs_power_gpio_set_value(g_pdata->gpio_chg_vbst_ctrl,
				DIG_HS_POWER_GPIO_SET);
	}
}

static void dig_hs_power_reset(enum dig_hs_vboost_control_type type)
{
	if (type == DIG_HS_VBOOST_CONTROL_PM) {
		g_pdata->pm_vboost_ref = false;
		if (!g_pdata->audio_vboost_ref)
			dig_hs_power_gpio_set_value(g_pdata->gpio_chg_vbst_ctrl,
				DIG_HS_POWER_GPIO_RESET);
	} else if (type == DIG_HS_VBOOST_CONTROL_AUDIO) {
		g_pdata->audio_vboost_ref = false;
		if (!g_pdata->pm_vboost_ref)
			dig_hs_power_gpio_set_value(g_pdata->gpio_chg_vbst_ctrl,
				DIG_HS_POWER_GPIO_RESET);
	}
}

int dig_hs_bst_ctrl_enable(bool enable, enum dig_hs_vboost_control_type type)
{
	if (!g_pdata) {
		hwlog_warn("g_pdata is NULL\n");
		return -ENOMEM;
	}

	mutex_lock(&g_pdata->vboost_ctrl_lock);
	if ((g_pdata->pm_vboost_ref == enable && type == DIG_HS_VBOOST_CONTROL_PM) ||
		(g_pdata->audio_vboost_ref == enable &&
		type == DIG_HS_VBOOST_CONTROL_AUDIO)) {
		mutex_unlock(&g_pdata->vboost_ctrl_lock);
		return 0;
	}
	hwlog_info("%s: enable %d, type %d\n", __func__, enable, type);
	if (enable)
		dig_hs_power_set(type);
	else
		dig_hs_power_reset(type);
	mutex_unlock(&g_pdata->vboost_ctrl_lock);
	return 0;
}

static int buckboost_control(void)
{
#ifdef CONFIG_BOOST_5V
	if (g_pdata->using_boost_5v == USING_BOOST_5V)
		boost_5v_enable(true, BOOST_CTRL_AUDIO);
#endif
	if (g_pdata->gpio_one_ctrl > 0) {
		dig_hs_power_gpio_set_value(g_pdata->gpio_low_power_ctrl,
			DIG_HS_POWER_GPIO_SET);
	} else {
		dig_hs_bst_ctrl_enable(true, DIG_HS_VBOOST_CONTROL_AUDIO);
		dig_hs_power_gpio_set_value(g_pdata->gpio_ear_power_en,
			DIG_HS_POWER_GPIO_SET);
	}
	if (g_pdata->using_loadswitch)
		usleep_range(9500, 10500); /* delay 10ms for voltage stability */
#ifdef CONFIG_WIRELESS_CHARGER
	if (g_pdata->using_ovp_switch)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
#endif
	pd_dpm_vbus_ctrl(CHARGER_TYPE_NONE);
	g_pdata->audio_buckboost_enable = true;
	hwlog_info("%s", __func__);
	return 0;
}

/* use bockboost to get lower power consumption when digital headset in */
int dig_hs_low_power(void)
{
	if (!g_pdata) {
		hwlog_warn("g_pdata is NULL\n");
		return -ENOMEM;
	}

	if (!g_pdata->audio_buckboost_enable && pd_dpm_get_pd_source_vbus())
		buckboost_control();

	return 0;
}

/* for factory test, enable buckboost 3.6V force */
static int dig_hs_buckboost_en_force(void)
{
	if (!g_pdata) {
		hwlog_warn("g_pdata is NULL\n");
		return -ENOMEM;
	}

	if (!g_pdata->audio_buckboost_enable)
		buckboost_control();

	return 0;
}

/* supply 5V to usb port use scharger when digital headset out */
int dig_hs_normal_power(void)
{
	if (!g_pdata) {
		hwlog_warn("g_pdata is NULL\n");
		return -ENOMEM;
	}

	if (g_pdata->audio_buckboost_enable) {
		if (g_pdata->gpio_one_ctrl > 0) {
			dig_hs_power_gpio_set_value(g_pdata->gpio_low_power_ctrl,
				DIG_HS_POWER_GPIO_RESET);
		} else {
			dig_hs_bst_ctrl_enable(false, DIG_HS_VBOOST_CONTROL_AUDIO);
			dig_hs_power_gpio_set_value(g_pdata->gpio_ear_power_en,
				DIG_HS_POWER_GPIO_RESET);
		}
#ifdef CONFIG_WIRELESS_CHARGER
		if (g_pdata->using_ovp_switch) {
			usleep_range(100000, 101000); /* sleep 100ms */
			wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
		}
#endif
#ifdef	CONFIG_BOOST_5V
		if (g_pdata->using_boost_5v == USING_BOOST_5V)
			boost_5v_enable(false, BOOST_CTRL_AUDIO);
#endif
		g_pdata->audio_buckboost_enable = false;
		hwlog_info("%s", __func__);
	}
	return 0;
}

#ifdef CONFIG_SUPERSWITCH_FSC
static void superswitch_vout_switch_work(struct work_struct *work)
{
	int ret;

	ret = dig_hs_low_power();
	if (ret < 0)
		return;

	FUSB3601_vout_enable(SUPERSWITCH_VOUT_SWITCH_CLOSE);
}
#endif

static int dig_hs_power_ioctl_no_headset(void)
{
	int ret;

#ifdef CONFIG_WIRELESS_CHARGER
	if (g_pdata->using_wireless_charger == USING_WIRELESS_CHARGER)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
#endif
#ifdef CONFIG_SUPERSWITCH_FSC
	if (g_pdata->using_superswitch == USING_SUPWRSWITCH) {
		queue_delayed_work(
			g_pdata->superswitch_vout_switch_delay_wq,
			&g_pdata->superswitch_vout_switch_delay_work,
			msecs_to_jiffies(USB_DISCONNECT_WAIT_TIME));
		ret = 0;
	} else {
#endif
		ret = dig_hs_buckboost_en_force();
#ifdef CONFIG_SUPERSWITCH_FSC
	}
#endif
	return ret;
}

static int dig_hs_power_ioctl_scharger(void)
{
#ifdef CONFIG_WIRELESS_CHARGER
	if (g_pdata->using_wireless_charger == USING_WIRELESS_CHARGER)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
#endif
	return dig_hs_normal_power();
}

static long dig_hs_power_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	int ret;

	switch (cmd) {
	case IOCTL_DIG_HS_POWER_BUCKBOOST_NO_HEADSET_CMD:
		pd_dpm_set_audio_power_no_hs_state(true);
		ret = dig_hs_power_ioctl_no_headset();
		break;
	case IOCTL_DIG_HS_POWER_SCHARGER_CMD:
		ret = dig_hs_power_ioctl_scharger();
		pd_dpm_set_audio_power_no_hs_state(false);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct of_device_id dig_hs_power_of_match[] = {
	{ .compatible = "huawei,dig_hs_power", },
	{},
};
MODULE_DEVICE_TABLE(of, dig_hs_power_of_match);

/* load dts config for board difference */
static void load_dig_hs_power_config(struct device_node *node)
{
	int temp;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		g_pdata->gpio_type = temp;
	else
		g_pdata->gpio_type = DIG_HS_POWER_GPIO_SOC;
#ifdef CONFIG_SUPERSWITCH_FSC
	if (!of_property_read_u32(node, "using_superswitch", &temp))
		g_pdata->using_superswitch = temp;
	else
		g_pdata->using_superswitch = NOT_USING_SUPWRSWITCH;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	if (!of_property_read_u32(node, "using_wireless_charger", &temp))
		g_pdata->using_wireless_charger = temp;
	else
		g_pdata->using_wireless_charger =
			NOT_USING_WIRELESS_CHARGER;
#endif
#ifdef CONFIG_BOOST_5V
	if (!of_property_read_u32(node, "using_boost_5v", &temp))
		g_pdata->using_boost_5v = temp;
	else
		g_pdata->using_boost_5v = NOT_USING_BOOST_5V;
#endif
	g_pdata->using_loadswitch = of_property_read_bool(node, "using_loadswitch");
#ifdef CONFIG_WIRELESS_CHARGER
	g_pdata->using_ovp_switch =
		of_property_read_bool(node, "using_ovp_switch");
#endif
}

static void load_dig_hs_power_config_one_gpio(struct device_node *node)
{
	int temp;

	if (!of_property_read_u32(node, "gpio_one_ctrl", &temp))
		g_pdata->gpio_one_ctrl = temp;
	else
		g_pdata->gpio_one_ctrl = DIG_HS_POWER_GPIO_DEFAUIT;
}

struct dig_hs_dev power_ops = {
	.tag = TAG_DIG_HS_POWER,
	.ops.dig_hs_plug_in = dig_hs_low_power,
	.ops.dig_hs_plug_out = dig_hs_normal_power,
};

static const struct file_operations dig_hs_power_fops = {
	.owner            = THIS_MODULE,
	.open            = simple_open,
	.unlocked_ioctl = dig_hs_power_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl    = dig_hs_power_ioctl,
#endif
};

static struct miscdevice dig_hs_power_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "dig_hs_power",
	.fops =     &dig_hs_power_fops,
};

static int gpio_one_ctrl_set(struct device_node *node)
{
	int ret;

	/* get gpio_low_power_ctrl gpio */
	g_pdata->gpio_low_power_ctrl = of_get_named_gpio(
			node, "gpio_low_power_ctrl", 0);
	if (g_pdata->gpio_low_power_ctrl < 0) {
		hwlog_err("gpio_low_power_ctrl is unvalid\n");
		ret = -ENOENT;
		goto err_out;
	}

	if (!gpio_is_valid(g_pdata->gpio_low_power_ctrl)) {
		hwlog_err("gpio is unvalid\n");
		ret = -ENOENT;
		goto err_out;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(g_pdata->gpio_low_power_ctrl,
		"gpio_low_power_ctrl");
	if (ret) {
		hwlog_err("request GPIO for low power ctrl fail %d\n", ret);
		goto err_out;
	}
	gpio_direction_output(g_pdata->gpio_low_power_ctrl, 0);
err_out:
	return ret;
}

static int gpio_chg_vbst_ctrl_set(struct device_node *node)
{
	int ret;

	/* get gpio_chg_vbst_ctrl gpio */
	g_pdata->gpio_chg_vbst_ctrl =  of_get_named_gpio(node,
					"gpio_chg_vbst_ctrl", 0);
	if (g_pdata->gpio_chg_vbst_ctrl < 0) {
		hwlog_err("gpio_chg_vbst_ctrl is unvalid\n");
		return -ENOENT;
	}

	if (!gpio_is_valid(g_pdata->gpio_chg_vbst_ctrl)) {
		hwlog_err("gpio is unvalid\n");
		return -ENOENT;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(g_pdata->gpio_chg_vbst_ctrl,
		"gpio_chg_vbst_ctrl");
	if (ret) {
		hwlog_err("error request GPIO for vbst ctrl fail %d\n", ret);
		return -ENOENT;
	}
	gpio_direction_output(g_pdata->gpio_chg_vbst_ctrl, 0);

	/* get gpio_ear_power_en gpio */
	g_pdata->gpio_ear_power_en = of_get_named_gpio(node,
		"gpio_ear_power_en", 0);
	if (g_pdata->gpio_ear_power_en < 0) {
		hwlog_err("gpio_ear_power_en is unvalid\n");
		ret = -ENOENT;
		goto gpio_chg_vbst_ctrl_err;
	}

	if (!gpio_is_valid(g_pdata->gpio_ear_power_en)) {
		hwlog_err("gpio is unvalid\n");
		ret = -ENOENT;
		goto gpio_chg_vbst_ctrl_err;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(g_pdata->gpio_ear_power_en, "gpio_ear_power_en");
	if (ret) {
		hwlog_err("error request GPIO for mic_gnd fail %d\n", ret);
		goto gpio_chg_vbst_ctrl_err;
	}
	gpio_direction_output(g_pdata->gpio_ear_power_en, 0);
	return 0;

gpio_chg_vbst_ctrl_err:
	gpio_free(g_pdata->gpio_chg_vbst_ctrl);
	return ret;
}

static int dig_hs_power_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node =  dev->of_node;
	int ret;

	g_pdata = devm_kzalloc(dev, sizeof(*g_pdata), GFP_KERNEL);
	if (!g_pdata)
		return -ENOMEM;
	g_pdata->pm_vboost_ref = false;
	g_pdata->audio_vboost_ref = false;
	g_pdata->audio_buckboost_enable = false;
	mutex_init(&g_pdata->vboost_ctrl_lock);

	load_dig_hs_power_config_one_gpio(node);
	if (g_pdata->gpio_one_ctrl > 0) {
		if (gpio_one_ctrl_set(node))
			goto err_out;
	} else {
		if (gpio_chg_vbst_ctrl_set(node))
			goto err_out;
	}
#ifdef CONFIG_SUPERSWITCH_FSC
	g_pdata->superswitch_vout_switch_delay_wq =
		create_singlethread_workqueue(
		"superswitch_vout_switch_delay_wq");
	if (!(g_pdata->superswitch_vout_switch_delay_wq)) {
		hwlog_err("%s : vout switch create failed\n", __func__);
		ret = -ENOMEM;
		goto gpio_ear_power_en_err;
	}
	INIT_DELAYED_WORK(&g_pdata->superswitch_vout_switch_delay_work,
		superswitch_vout_switch_work);
#endif
	load_dig_hs_power_config(node);

	ret = misc_register(&dig_hs_power_miscdev);
	if (ret) {
		hwlog_err("%s: register usb audio power miscdev fail, ret:%d\n",
			__func__, ret);
#ifdef CONFIG_SUPERSWITCH_FSC
		goto superswitch_vout_switch_delay_wq_err;
#else
		goto gpio_ear_power_en_err;
#endif
	}
	dig_hs_dev_register(&power_ops, PRIORITY_H);
	hwlog_info("dig_hs_power probe success\n");

	return 0;
#ifdef CONFIG_SUPERSWITCH_FSC
superswitch_vout_switch_delay_wq_err:
	if (g_pdata->superswitch_vout_switch_delay_wq) {
		cancel_delayed_work(
			&g_pdata->superswitch_vout_switch_delay_work);
		flush_workqueue(g_pdata->superswitch_vout_switch_delay_wq);
		destroy_workqueue(g_pdata->superswitch_vout_switch_delay_wq);
	}
#endif
gpio_ear_power_en_err:
	gpio_free(g_pdata->gpio_ear_power_en);
	gpio_free(g_pdata->gpio_chg_vbst_ctrl);
err_out:
	if (g_pdata->gpio_one_ctrl > 0)
		gpio_free(g_pdata->gpio_low_power_ctrl);

	return ret;
}

static int dig_hs_power_remove(struct platform_device *pdev)
{
	if (!g_pdata)
		goto out_err;
	if (g_pdata->gpio_chg_vbst_ctrl > 0)
		gpio_free(g_pdata->gpio_chg_vbst_ctrl);

	if (g_pdata->gpio_ear_power_en > 0)
		gpio_free(g_pdata->gpio_ear_power_en);

	if (g_pdata->gpio_low_power_ctrl > 0)
		gpio_free(g_pdata->gpio_low_power_ctrl);

#ifdef CONFIG_SUPERSWITCH_FSC
	if (g_pdata->superswitch_vout_switch_delay_wq) {
		cancel_delayed_work(
			&g_pdata->superswitch_vout_switch_delay_work);
		flush_workqueue(g_pdata->superswitch_vout_switch_delay_wq);
		destroy_workqueue(g_pdata->superswitch_vout_switch_delay_wq);
	}
#endif
	dig_hs_dev_deregister(TAG_DIG_HS_POWER);
out_err:
	misc_deregister(&dig_hs_power_miscdev);
	hwlog_info("%s: exit\n", __func__);

	return 0;
}

static struct platform_driver dig_hs_power_driver = {
	.driver = {
		.name   = "dig_hs_power",
		.owner  = THIS_MODULE,
		.of_match_table = dig_hs_power_of_match,
	},
	.probe  = dig_hs_power_probe,
	.remove = dig_hs_power_remove,
};

static int __init dig_hs_power_init(void)
{
	return platform_driver_register(&dig_hs_power_driver);
}

static void __exit dig_hs_power_exit(void)
{
	platform_driver_unregister(&dig_hs_power_driver);
}

fs_initcall(dig_hs_power_init);
module_exit(dig_hs_power_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("digital headset power control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
