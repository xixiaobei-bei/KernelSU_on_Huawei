/*
 * PMIC LEDs drive
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2016. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "leds_spmi.h"

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hisi/leds_pmic.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <pr_log.h>
#include <linux/of_device.h>
#include <securec.h>

#ifdef CONFIG_HW_LED_CONFIG
void led_config_get_current_setting(struct led_spmi_platform_data *leds_spmi);
#else
void led_config_get_current_setting(struct led_spmi_platform_data *leds_spmi)
{
	pr_info("%s,no need set irset tp color, using default!\n", __func__);
}
#endif

static struct led_spmi_drv_data *led_spmi_pdata;

static struct led_spmi_platform_data leds_spmi = {
	.leds_size = LEDS_MAX,
	.leds = {
		[0] = {
			.name = "red",
			.brightness = LED_OFF,
			.delay_on = 0,
			.delay_off = 0,
			.default_trigger = "timer",
			.each_maxdr_iset = 0,
		},
		[1] = {
			.name = "green",
			.brightness = LED_OFF,
			.delay_on = 0,
			.delay_off = 0,
			.default_trigger = "timer",
			.each_maxdr_iset = 0,
		},
		[2] = {
			.name = "blue",
			.brightness = LED_OFF,
			.delay_on = 0,
			.delay_off = 0,
			.default_trigger = "timer",
			.each_maxdr_iset = 0,
		},
	},
};

static unsigned char led_reg_read(u32 led_address)
{
	return pmic_read_reg(led_address);
}

static void led_reg_write(u8 led_set, u32 led_address)
{
	pmic_write_reg(led_address, led_set);
}

static void led_set_disable(u8 id)
{
	u32 led_dr_ctl;

	led_dr_ctl = led_reg_read(leds_spmi.dr_led_ctrl);
	/* id must be less than 3 */
	led_dr_ctl &= ~(0x1 << id);
	led_reg_write(led_dr_ctl, leds_spmi.dr_led_ctrl);
}

static int led_panic_handler(
	struct notifier_block *nb, unsigned long action, void *data)
{
	u8 index;

	pr_info("A panic: %s\n", __func__);
	for (index = 0; index < LEDS_MAX; index++)
		led_set_disable(index);

	return 0;
}

static struct notifier_block panic_led = {
	.notifier_call = led_panic_handler,
};

static void led_set_reg_write(struct led_set_config *brightness_config)
{
	/* config current */
	led_reg_write(brightness_config->brightness_set,
		brightness_config->led_iset_address);
	/* start_delay */
	led_reg_write(
		DR_START_DEL_512, brightness_config->led_start_address);
	/* set_on */
	led_reg_write(
		DR_DELAY_ON, brightness_config->led_tim_address);
	/* enable */
	led_reg_write(
		brightness_config->led_dr_ctl, leds_spmi.dr_led_ctrl);
	/* output enable */
	led_reg_write(
		brightness_config->led_dr_out_ctl, leds_spmi.dr_out_ctrl);
}

/* set led half brightness or full brightness */
static void led_set_enable(u8 brightness_set, u8 id)
{
	unsigned char led_dr_ctl;
	unsigned char led_dr_out_ctl;
	struct led_set_config brightness_config;

	led_dr_ctl = led_reg_read(leds_spmi.dr_led_ctrl);
	led_dr_out_ctl = led_reg_read(leds_spmi.dr_out_ctrl);

	brightness_config.brightness_set = brightness_set;
	brightness_config.led_iset_address = leds_spmi.leds[id].dr_iset;
	brightness_config.led_start_address =
		leds_spmi.leds[id].dr_start_del;
	brightness_config.led_tim_address =
		leds_spmi.leds[id].dr_time_config0;
	brightness_config.led_tim1_address =
		leds_spmi.leds[id].dr_time_config1;
	brightness_config.led_dr_ctl = led_dr_ctl | (0x1 << id);
	brightness_config.led_dr_out_ctl =
		led_dr_out_ctl & LED_OUT_CTRL_VAL(id);

	pr_info("Led_id=%d, brightness_set=%d,\n", id, brightness_set);
	led_set_reg_write(&brightness_config);
}

/* set enable or disable led of dr3,4,5 */
static int led_set(struct led_spmi_data *led,
	enum led_brightness brightness)
{
	int ret = 0;
	u8 id = led->id;
	u8 iset;
	struct led_spmi_drv_data *data = led_spmi_pdata;

	mutex_lock(&data->lock);

	led_config_get_current_setting(&leds_spmi);

	switch (id) {
	case LED0: /* fall through */
	case LED1: /* fall through */
	case LED2:
		if (brightness == LED_OFF) {
			/* set led off */
			led_set_disable(id);
			pr_info("[%s] off id is %d\n", __func__, id);
		} else if (brightness == LED_FULL) {
			/* set led brightness */
			iset = leds_spmi.leds[id].each_maxdr_iset;
			led_set_enable(iset, id);
			pr_info("[%s] full id is %d, iset:%d\n",
				__func__, id, iset);
		} else {
			/* set led half brightness */
			led_set_enable(DR_BRIGHTNESS_HALF, id);
			pr_info("[%s] half id is %d\n", __func__, id);
		}
		break;
	default:
		pr_err("%s id:%d is error\n", __func__, id);
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&data->lock);

	return ret;
}

static void led_set_blink_reg_write(u8 id, u8 set_time)
{
	led_reg_write(set_time, leds_spmi.leds[id].dr_time_config0);
	led_reg_write(DR_RISA_TIME,
		leds_spmi.leds[id].dr_time_config1);
}

/* get the set time in area */
static u8 led_get_time(unsigned long delay, u8 flag)
{
	u8 set_time = 0;

	if (delay == DEL_0)
		set_time = DR_DEL00;
	else if (delay <= DEL_500)
		set_time = DR_DEL01;
	else if (delay <= DEL_1000)
		set_time = DR_DEL02;
	else if (delay <= DEL_2000)
		set_time = DR_DEL03;
	else if (delay <= DEL_4000)
		set_time = DR_DEL04;
	else if (delay <= DEL_6000)
		set_time = DR_DEL05;
	else if (delay <= DEL_8000)
		set_time = DR_DEL06;
	else if (delay <= DEL_12000)
		set_time = DR_DEL07;
	else if (delay <= DEL_14000)
		set_time = DR_DEL08;
	else
		set_time = DR_DEL09;

	if (flag)
		return set_time << SET_TIME_LEFT_SHIFT;
	else
		return set_time;
}

/* config of dr delay_on and delay_off registers */
static int led_set_blink(struct led_classdev *led_ldev,
	unsigned long *delay_on, unsigned long *delay_off)
{
	struct led_spmi_data *led_dat =
		container_of(led_ldev, struct led_spmi_data, ldev);
	struct led_spmi_drv_data *data = led_spmi_pdata;
	u8 id;
	int ret = 0;
	u8 set_time_on = 0;
	u8 set_time_off = 0;

	if (!led_dat) {
		pr_err("led set blink error\n");
		return -EINVAL;
	}
	id = led_dat->id;
	if (!(*delay_on) && !(*delay_off))
		return ret;

	mutex_lock(&data->lock);

	if ((id == LED0) || (id == LED1) || (id == LED2)) {
		led_ldev->blink_delay_on = *delay_on;
		led_ldev->blink_delay_off = *delay_off;

		set_time_on = led_get_time(*delay_on, DELAY_ON);
		set_time_off = led_get_time(*delay_off, DELAY_OFF);
		led_set_blink_reg_write(id, set_time_on | set_time_off);
		pr_info("[%s] id is %d, delay-on:%lu, delay-off:%lu\n",
			__func__, id, *delay_on, *delay_off);
	} else {
		pr_err("%s id:%d is error\n", __func__, id);
		ret = -EINVAL;
	}

	mutex_unlock(&data->lock);

	return ret;
}

/* set brightness of dr3,4,5 lights */
static void pmic_led_set_brightness(
	struct led_classdev *led_ldev, enum led_brightness brightness)
{
	struct led_spmi_data *led =
		container_of(led_ldev, struct led_spmi_data, ldev);

	if (!led) {
		pr_err("led set btrightnss error!\n");
		return;
	}
	led->status.brightness = brightness;
	if (led_set(led, led->status.brightness))
		pr_err("led_set fail\n");
}

/* config led lights */
static int led_configure(struct spmi_device *pdev,
	struct led_spmi_drv_data *data, struct led_spmi_platform_data *pdata)
{
	int i;
	int ret = 0;
	struct led_spmi *pled = NULL;
	struct led_spmi_data *led = NULL;

	for (i = 0; i < pdata->leds_size; i++) {
		pled = &pdata->leds[i];
		led = &data->leds[i];
		led->id = (u8)i;
		led->status.brightness = pled->brightness;
		led->status.delay_on = pled->delay_on;
		led->status.delay_off = pled->delay_off;
		led->ldev.name = pled->name;
		led->ldev.default_trigger = pled->default_trigger;
		led->ldev.max_brightness = LED_FULL;
		led->ldev.blink_set = led_set_blink;
		led->ldev.brightness_set = pmic_led_set_brightness;

		ret = led_classdev_register(&pdev->dev, &led->ldev);
		if (ret < 0) {
			dev_err(&pdev->dev, "couldn't register LED %s:%d\n",
				led->ldev.name, i);
			goto err_clsdev_register;
		}
	}

	return 0;

err_clsdev_register:
	if (i > 0) {
		for (i = i - 1; i >= 0; i--)
			led_classdev_unregister(&data->leds[i].ldev);
	}

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id pmic_led_match[] = {
	{
		.compatible = "hisilicon-hisi-led-pmic-spmi",
		.data = &leds_spmi,
	},
	{},
};
MODULE_DEVICE_TABLE(of, pmic_led_match);
#endif

static struct device_node *led_get_led_node_root(
	struct spmi_device *pdev, int index)
{
	struct device_node *root = NULL;
	char compatible_string[LED_DTS_ATTR_LEN] = {0};
	int ret;

	ret = snprintf_s(compatible_string,
		LED_DTS_ATTR_LEN, LED_DTS_ATTR_LEN - 1,
		"hisilicon,hisi-led%d", index);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s snprintf_s fail, index:%d\n",
			__func__, index);
		return NULL;
	}
	root = of_find_compatible_node(
		pdev->dev.of_node, NULL, (const char *)compatible_string);
	if (!root) {
		dev_err(&pdev->dev, "%s root error!\n", compatible_string);
		return NULL;
	}

	return root;
}

static int led_read_led_cfg(struct led_spmi *led,
	struct spmi_device *pdev, int index)
{
	struct device_node *root = NULL;
	int ret;

	root = led_get_led_node_root(pdev, index);
	if (!root) {
		dev_err(&pdev->dev, "led%d get dts node fail\n", index);
		return 0;
	}

	ret = of_property_read_string(root, "hisilicon,dr_ctrl", &led->name);
	if (ret < 0) {
		dev_err(&pdev->dev, "led%d's color fail!\n", index);
		return ret;
	}

	ret = of_property_read_u32(root, "hisilicon,each_max_iset",
		&led->each_maxdr_iset);
	if (ret < 0) {
		dev_info(&pdev->dev, "led%d's each_maxdr_iset fail!\n", index);
		led->each_maxdr_iset = leds_spmi.max_iset;
	}

	ret = of_property_read_u32(root, "hisilicon,dr_start_del",
		&led->dr_start_del);
	if (ret < 0) {
		dev_err(&pdev->dev, "led%d's dr_start_del fail!\n", index);
		return ret;
	}

	ret = of_property_read_u32(root, "hisilicon,dr_iset", &led->dr_iset);
	if (ret < 0) {
		dev_err(&pdev->dev, "led%d's dr_iset fail!\n", index);
		return ret;
	}

	ret = of_property_read_u32(root, "hisilicon,dr_time_config0",
		&led->dr_time_config0);
	if (ret < 0) {
		dev_err(&pdev->dev, "led%d's dr_time_config0 fail!\n", index);
		return ret;
	}

	ret = of_property_read_u32(root, "hisilicon,dr_time_config1",
		&led->dr_time_config1);
	if (ret < 0) {
		dev_err(&pdev->dev, "led%d's dr_time_config1 fail!\n", index);
		return ret;
	}

	return 0;
}

static int led_basic_dt_cfg(struct spmi_device *pdev)
{
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, "hisilicon,dr_led_ctrl",
		&leds_spmi.dr_led_ctrl);
	if (ret < 0) {
		dev_err(&pdev->dev, "config dr_led_ctrl failure!\n");
		return ret;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "hisilicon,dr_out_ctrl",
		&leds_spmi.dr_out_ctrl);
	if (ret < 0) {
		dev_err(&pdev->dev, "config dr_out_ctrl failure!\n");
		return ret;
	}

	ret = of_property_read_u32(
		pdev->dev.of_node, "hisilicon,max_iset", &leds_spmi.max_iset);
	if (ret < 0) {
		dev_info(&pdev->dev, "config max_iset fail! use default\n");
		leds_spmi.max_iset = DR_BRIGHTNESS_FULL;
	}

	return 0;
}

static struct led_spmi_drv_data *led_spmi_new_drv_data(struct spmi_device *pdev)
{
	struct led_spmi_drv_data *data = NULL;

	data = devm_kzalloc(
		&pdev->dev, sizeof(struct led_spmi_drv_data), GFP_KERNEL);
	if (!data)
		return NULL;

	mutex_init(&data->lock);

	dev_set_drvdata(&pdev->dev, data);
	led_spmi_pdata = data;

	return data;
}

static int led_spmi_probe(struct spmi_device *pdev)
{
	struct led_spmi_platform_data *pdata = NULL;
	struct led_spmi_drv_data *data = NULL;
	const struct of_device_id *match = NULL;
	int index;
	int ret;

	match = of_match_node(pmic_led_match, pdev->dev.of_node);
	if (!match) {
		dev_err(&pdev->dev, "dev_node is not match, exiting\n");
		return -ENODEV;
	}

	pdata = (struct led_spmi_platform_data *)match->data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data\n");
		return -EINVAL;
	}

	data = led_spmi_new_drv_data(pdev);
	if (!data) {
		dev_err(&pdev->dev, "new drv data fail\n");
		return -ENOMEM;
	}

	ret = led_basic_dt_cfg(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "read basic dts cfg fail\n");
		goto err_clr;
	}

	for (index = 0; index < LEDS_MAX; index++) {
		ret = led_read_led_cfg(&leds_spmi.leds[index],
			pdev, index);
		if (ret < 0) {
			dev_err(&pdev->dev, "read led dts cfg fail\n");
			goto err_clr;
		}
	}

	ret = led_configure(pdev, data, pdata);
	if (ret < 0) {
		dev_err(&pdev->dev, "registe led fail\n");
		goto err_clr;
	}

	atomic_notifier_chain_register(&panic_notifier_list, &panic_led);
	dev_err(&pdev->dev, "hisi leds init success\n");

	return 0;

err_clr:
	dev_set_drvdata(&pdev->dev, NULL);
	return ret;
}

static int led_spmi_remove(struct spmi_device *pdev)
{
	int i;
	struct led_spmi_platform_data *pdata = NULL;
	const struct of_device_id *match = NULL;
	struct led_spmi_drv_data *data = dev_get_drvdata(&pdev->dev);

	if (!data) {
		dev_err(&pdev->dev, "%s:data is NULL\n", __func__);
		return -ENODEV;
	}

	match = of_match_node(pmic_led_match, pdev->dev.of_node);
	if (!match) {
		dev_err(&pdev->dev, "%s:Device id is NULL\n", __func__);
		return -ENODEV;
	}

	pdata = (struct led_spmi_platform_data *)match->data;
	for (i = 0; i < pdata->leds_size; i++)
		led_classdev_unregister(&data->leds[i].ldev);

	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static void led_spmi_shutdown(struct spmi_device *pdev)
{
	u8 index;
	struct led_spmi_drv_data *data = dev_get_drvdata(&pdev->dev);

	if (!data) {
		dev_err(&pdev->dev, "%s:data is NULL\n", __func__);
		return;
	}

	for (index = 0; index < LEDS_MAX; index++)
		led_set_disable(index);
}

#ifdef CONFIG_PM
static int led_spmi_suspend(struct spmi_device *pdev, pm_message_t state)
{
	struct led_spmi_drv_data *data = dev_get_drvdata(&pdev->dev);

	dev_info(&pdev->dev, "%s: suspend +\n", __func__);
	if (!data) {
		dev_err(&pdev->dev, "%s:data is NULL\n", __func__);
		return -ENODEV;
	}

	if (!mutex_trylock(&data->lock)) {
		dev_err(&pdev->dev, "%s: mutex_trylock\n", __func__);
		return -EAGAIN;
	}

	dev_info(&pdev->dev, "%s: suspend -\n", __func__);

	return 0;
}

static int led_spmi_resume(struct spmi_device *pdev)
{
	struct led_spmi_drv_data *data = dev_get_drvdata(&pdev->dev);

	dev_info(&pdev->dev, "%s: resume +\n", __func__);
	if (!data) {
		dev_err(&pdev->dev, "%s:data is NULL\n", __func__);
		return -ENODEV;
	}

	mutex_unlock(&data->lock);
	dev_info(&pdev->dev, "%s: resume -\n", __func__);

	return 0;
}
#endif

static const struct spmi_device_id led_spmi_id[] = {
	{ "hisilicon-hisi-led-pmic-spmi", 0 }, {},
};

static struct spmi_driver led_spmi_driver = {
	.probe = led_spmi_probe,
	.remove = led_spmi_remove,
	.shutdown = led_spmi_shutdown,
	.id_table = led_spmi_id,
	.driver = {
		.name = LEDS_SPMI,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pmic_led_match),
	},
#ifdef CONFIG_PM
	.suspend = led_spmi_suspend,
	.resume = led_spmi_resume,
#endif
};

static int __init leds_spmi_init(void)
{
	return spmi_driver_register(&led_spmi_driver);
}

static void __exit leds_spmi_exit(void)
{
	spmi_driver_unregister(&led_spmi_driver);
}

module_init(leds_spmi_init);
module_exit(leds_spmi_exit);

MODULE_ALIAS("pmic-leds");
MODULE_DESCRIPTION("pmic LED driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
