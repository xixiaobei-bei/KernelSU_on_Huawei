/*
 * LEDs driver for hisi
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
#include "hisi_leds_spmi.h"

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hisi/hisi_leds.h>
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
#include <linux/hisi/hisi_log.h>
#include <linux/of_device.h>
#include <securec.h>

#ifdef CONFIG_HW_LED_CONFIG
void led_config_get_current_setting(struct hisi_led_platform_data *hisi_leds);
#else
void led_config_get_current_setting(struct hisi_led_platform_data *hisi_leds)
{
	pr_info("%s,no need set irset tp color, using default!\n", __func__);
}
#endif

static struct hisi_led_drv_data *hisi_led_pdata;

static struct hisi_led_platform_data hisi_leds = {
	.leds_size = HISI_LEDS_MAX,
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

static unsigned char hisi_led_reg_read(u32 led_address)
{
	return hisi_pmic_reg_read(led_address);
}

static void hisi_led_reg_write(u8 led_set, u32 led_address)
{
	hisi_pmic_reg_write(led_address, led_set);
}

static void hisi_led_set_disable(u8 id)
{
	u32 hisi_led_dr_ctl;

	hisi_led_dr_ctl = hisi_led_reg_read(hisi_leds.dr_led_ctrl);
	/* id must be less than 3 */
	hisi_led_dr_ctl &= ~(0x1 << id);
	hisi_led_reg_write(hisi_led_dr_ctl, hisi_leds.dr_led_ctrl);
}

static int hisi_led_panic_handler(
	struct notifier_block *nb, unsigned long action, void *data)
{
	u8 index;

	pr_info("A panic: %s\n", __func__);
	for (index = 0; index < HISI_LEDS_MAX; index++)
		hisi_led_set_disable(index);

	return 0;
}

static struct notifier_block panic_led = {
	.notifier_call = hisi_led_panic_handler,
};

static void hisi_led_set_reg_write(struct led_set_config *brightness_config)
{
	/* config current */
	hisi_led_reg_write(brightness_config->brightness_set,
		brightness_config->hisi_led_iset_address);
	/* start_delay */
	hisi_led_reg_write(
		DR_START_DEL_512, brightness_config->hisi_led_start_address);
	/* set_on */
	hisi_led_reg_write(
		DR_DELAY_ON, brightness_config->hisi_led_tim_address);
	/* enable */
	hisi_led_reg_write(
		brightness_config->hisi_led_dr_ctl, hisi_leds.dr_led_ctrl);
	/* output enable */
	hisi_led_reg_write(
		brightness_config->hisi_led_dr_out_ctl, hisi_leds.dr_out_ctrl);
}

/* set led half brightness or full brightness */
static void hisi_led_set_enable(u8 brightness_set, u8 id)
{
	unsigned char hisi_led_dr_ctl;
	unsigned char hisi_led_dr_out_ctl;
	struct led_set_config brightness_config;

	hisi_led_dr_ctl = hisi_led_reg_read(hisi_leds.dr_led_ctrl);
	hisi_led_dr_out_ctl = hisi_led_reg_read(hisi_leds.dr_out_ctrl);

	brightness_config.brightness_set = brightness_set;
	brightness_config.hisi_led_iset_address = hisi_leds.leds[id].dr_iset;
	brightness_config.hisi_led_start_address =
		hisi_leds.leds[id].dr_start_del;
	brightness_config.hisi_led_tim_address =
		hisi_leds.leds[id].dr_time_config0;
	brightness_config.hisi_led_tim1_address =
		hisi_leds.leds[id].dr_time_config1;
	brightness_config.hisi_led_dr_ctl = hisi_led_dr_ctl | (0x1 << id);
	brightness_config.hisi_led_dr_out_ctl =
		hisi_led_dr_out_ctl & LED_OUT_CTRL_VAL(id);

	pr_info("Led_id=%d, brightness_set=%d,\n", id, brightness_set);
	hisi_led_set_reg_write(&brightness_config);
}

/* set enable or disable led of dr3,4,5 */
static int hisi_led_set(struct hisi_led_data *led,
	enum led_brightness brightness)
{
	int ret = 0;
	u8 id = led->id;
	u8 iset;
	struct hisi_led_drv_data *data = hisi_led_pdata;

	mutex_lock(&data->lock);

	led_config_get_current_setting(&hisi_leds);

	switch (id) {
	case HISI_LED0: /* fall through */
	case HISI_LED1: /* fall through */
	case HISI_LED2:
		if (brightness == LED_OFF) {
			/* set led off */
			hisi_led_set_disable(id);
			pr_info("[%s] off id is %d\n", __func__, id);
		} else if (brightness == LED_FULL) {
			/* set led brightness */
			iset = hisi_leds.leds[id].each_maxdr_iset;
			hisi_led_set_enable(iset, id);
			pr_info("[%s] full id is %d, iset:%d\n",
				__func__, id, iset);
		} else {
			/* set led half brightness */
			hisi_led_set_enable(DR_BRIGHTNESS_HALF, id);
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

static void hisi_led_set_blink_reg_write(u8 id, u8 set_time)
{
	hisi_led_reg_write(set_time, hisi_leds.leds[id].dr_time_config0);
	hisi_led_reg_write(DR_RISA_TIME,
		hisi_leds.leds[id].dr_time_config1);
}

/* get the set time in area */
static u8 hisi_led_get_time(unsigned long delay, u8 flag)
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
static int hisi_led_set_blink(struct led_classdev *led_ldev,
	unsigned long *delay_on, unsigned long *delay_off)
{
	struct hisi_led_data *led_dat =
		container_of(led_ldev, struct hisi_led_data, ldev);
	struct hisi_led_drv_data *data = hisi_led_pdata;
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

	if ((id == HISI_LED0) || (id == HISI_LED1) || (id == HISI_LED2)) {
		led_ldev->blink_delay_on = *delay_on;
		led_ldev->blink_delay_off = *delay_off;

		set_time_on = hisi_led_get_time(*delay_on, DELAY_ON);
		set_time_off = hisi_led_get_time(*delay_off, DELAY_OFF);
		hisi_led_set_blink_reg_write(id, set_time_on | set_time_off);
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
static void hisi_led_set_brightness(
	struct led_classdev *led_ldev, enum led_brightness brightness)
{
	struct hisi_led_data *led =
		container_of(led_ldev, struct hisi_led_data, ldev);

	if (!led) {
		pr_err("led set btrightnss error!\n");
		return;
	}
	led->status.brightness = brightness;
	if (hisi_led_set(led, led->status.brightness))
		pr_err("hisi_led_set fail\n");
}

/* config led lights */
static int hisi_led_configure(struct spmi_device *pdev,
	struct hisi_led_drv_data *data, struct hisi_led_platform_data *pdata)
{
	int i;
	int ret = 0;
	struct hisi_led *pled = NULL;
	struct hisi_led_data *led = NULL;

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
		led->ldev.blink_set = hisi_led_set_blink;
		led->ldev.brightness_set = hisi_led_set_brightness;

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
static const struct of_device_id hisi_led_match[] = {
	{
		.compatible = "hisilicon-hisi-led-pmic-spmi",
		.data = &hisi_leds,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_led_match);
#endif

static struct device_node *hisi_led_get_led_node_root(
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

static int hisi_led_read_led_cfg(struct hisi_led *led,
	struct spmi_device *pdev, int index)
{
	struct device_node *root = NULL;
	int ret;

	root = hisi_led_get_led_node_root(pdev, index);
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
		led->each_maxdr_iset = hisi_leds.max_iset;
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

static int hisi_led_basic_dt_cfg(struct spmi_device *pdev)
{
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, "hisilicon,dr_led_ctrl",
		&hisi_leds.dr_led_ctrl);
	if (ret < 0) {
		dev_err(&pdev->dev, "config dr_led_ctrl failure!\n");
		return ret;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "hisilicon,dr_out_ctrl",
		&hisi_leds.dr_out_ctrl);
	if (ret < 0) {
		dev_err(&pdev->dev, "config dr_out_ctrl failure!\n");
		return ret;
	}

	ret = of_property_read_u32(
		pdev->dev.of_node, "hisilicon,max_iset", &hisi_leds.max_iset);
	if (ret < 0) {
		dev_info(&pdev->dev, "config max_iset fail! use default\n");
		hisi_leds.max_iset = DR_BRIGHTNESS_FULL;
	}

	return 0;
}

static struct hisi_led_drv_data *hisi_led_new_drv_data(struct spmi_device *pdev)
{
	struct hisi_led_drv_data *data = NULL;

	data = devm_kzalloc(
		&pdev->dev, sizeof(struct hisi_led_drv_data), GFP_KERNEL);
	if (!data)
		return NULL;

	mutex_init(&data->lock);

	dev_set_drvdata(&pdev->dev, data);
	hisi_led_pdata = data;

	return data;
}

static int hisi_led_probe(struct spmi_device *pdev)
{
	struct hisi_led_platform_data *pdata = NULL;
	struct hisi_led_drv_data *data = NULL;
	const struct of_device_id *match = NULL;
	int index;
	int ret;

	match = of_match_node(hisi_led_match, pdev->dev.of_node);
	if (!match) {
		dev_err(&pdev->dev, "dev_node is not match, exiting\n");
		return -ENODEV;
	}

	pdata = (struct hisi_led_platform_data *)match->data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data\n");
		return -EINVAL;
	}

	data = hisi_led_new_drv_data(pdev);
	if (!data) {
		dev_err(&pdev->dev, "new drv data fail\n");
		return -ENOMEM;
	}

	ret = hisi_led_basic_dt_cfg(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "read basic dts cfg fail\n");
		goto err_clr;
	}

	for (index = 0; index < HISI_LEDS_MAX; index++) {
		ret = hisi_led_read_led_cfg(&hisi_leds.leds[index],
			pdev, index);
		if (ret < 0) {
			dev_err(&pdev->dev, "read led dts cfg fail\n");
			goto err_clr;
		}
	}

	ret = hisi_led_configure(pdev, data, pdata);
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

static int hisi_led_remove(struct spmi_device *pdev)
{
	int i;
	struct hisi_led_platform_data *pdata = NULL;
	const struct of_device_id *match = NULL;
	struct hisi_led_drv_data *data = dev_get_drvdata(&pdev->dev);

	if (!data) {
		dev_err(&pdev->dev, "%s:data is NULL\n", __func__);
		return -ENODEV;
	}

	match = of_match_node(hisi_led_match, pdev->dev.of_node);
	if (!match) {
		dev_err(&pdev->dev, "%s:Device id is NULL\n", __func__);
		return -ENODEV;
	}

	pdata = (struct hisi_led_platform_data *)match->data;
	for (i = 0; i < pdata->leds_size; i++)
		led_classdev_unregister(&data->leds[i].ldev);

	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static void hisi_led_shutdown(struct spmi_device *pdev)
{
	u8 index;
	struct hisi_led_drv_data *data = dev_get_drvdata(&pdev->dev);

	if (!data) {
		dev_err(&pdev->dev, "%s:data is NULL\n", __func__);
		return;
	}

	for (index = 0; index < HISI_LEDS_MAX; index++)
		hisi_led_set_disable(index);
}

#ifdef CONFIG_PM
static int hisi_led_suspend(struct spmi_device *pdev, pm_message_t state)
{
	struct hisi_led_drv_data *data = dev_get_drvdata(&pdev->dev);

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

static int hisi_led_resume(struct spmi_device *pdev)
{
	struct hisi_led_drv_data *data = dev_get_drvdata(&pdev->dev);

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

static const struct spmi_device_id hisi_led_spmi_id[] = {
	{ "hisilicon-hisi-led-pmic-spmi", 0 }, {},
};

static struct spmi_driver hisi_led_driver = {
	.probe = hisi_led_probe,
	.remove = hisi_led_remove,
	.shutdown = hisi_led_shutdown,
	.id_table = hisi_led_spmi_id,
	.driver = {
		.name = HISI_LEDS,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_led_match),
	},
#ifdef CONFIG_PM
	.suspend = hisi_led_suspend,
	.resume = hisi_led_resume,
#endif
};

static int __init hisi_leds_init(void)
{
	return spmi_driver_register(&hisi_led_driver);
}

static void __exit hisi_leds_exit(void)
{
	spmi_driver_unregister(&hisi_led_driver);
}

module_init(hisi_leds_init);
module_exit(hisi_leds_exit);

MODULE_ALIAS("hisi-leds");
MODULE_DESCRIPTION("hisi LED driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
