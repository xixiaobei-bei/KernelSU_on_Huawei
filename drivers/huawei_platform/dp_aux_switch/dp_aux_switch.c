/*
 * dp_aux_switch.c
 *
 * dp aux switch driver
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

#include <huawei_platform/dp_aux_switch/dp_aux_switch.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/ion.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/audio/usb_analog_hs_interface.h>

#define HWLOG_TAG dp_aux_switch
HWLOG_REGIST();

#define INVALID_GPIO   (-1)
#define SET_GPIO_HIGH  1
#define SET_GPIO_LOW   0

struct dp_aux_switch_priv {
	bool from_fsa4476;
	bool with_uart;
	uint32_t ch_polarity;
	int aux_gpio;
	int uart_gpio;
};

static struct dp_aux_switch_priv aux_switch_priv;

enum aux_switch_channel_type get_aux_switch_channel(void)
{
	if (aux_switch_priv.from_fsa4476)
		return CHANNEL_FSA4476;

	return CHANNEL_SUPERSWITCH;
}
EXPORT_SYMBOL_GPL(get_aux_switch_channel);

// aux polarity switch
void dp_aux_switch_op(uint32_t value)
{
	if (aux_switch_priv.from_fsa4476) {
		aux_switch_priv.ch_polarity = value;
		return;
	}

	if (gpio_is_valid(aux_switch_priv.aux_gpio))
		gpio_direction_output(aux_switch_priv.aux_gpio, value);
	else
		hwlog_err("%s: invalid gpio %d\n",
			__func__, aux_switch_priv.aux_gpio);
}
EXPORT_SYMBOL_GPL(dp_aux_switch_op);

// NOTE: Don't pay attention to the name of the function!
// 1. aux-uart switch is existed: this func is switch aux or uart.
// 2. aux-uart switch is not existed: this func is dp aux enable or disable.
void dp_aux_uart_switch_enable(void)
{
	if (aux_switch_priv.from_fsa4476) {
		// SBU bypass switch
		if (aux_switch_priv.ch_polarity) {
			// ENN L, EN1/EN2 01
			// SBU1 to SBU2_H, SBU2 to SBU1_H
			usb_analog_hs_plug_in_out_handle(DP_PLUG_IN_CROSS);
			hwlog_info("%s: dp plug in cross\n", __func__);
		} else {
			// ENN L, EN1/EN2 00
			// SBU1 to SBU1_H, SBU2 to SBU2_H
			usb_analog_hs_plug_in_out_handle(DP_PLUG_IN);
			hwlog_info("%s: dp plug in\n", __func__);
		}

		// 1. aux-uart switch is not existed.
		// 2. aux polarity switch of dp by fsa4476.
		// Then, return directly.
		if (!aux_switch_priv.with_uart)
			return;
	}

	if (gpio_is_valid(aux_switch_priv.uart_gpio))
		gpio_direction_output(aux_switch_priv.uart_gpio, SET_GPIO_HIGH);
	else
		hwlog_err("%s: invalid gpio %d\n",
			__func__, aux_switch_priv.uart_gpio);
}
EXPORT_SYMBOL_GPL(dp_aux_uart_switch_enable);

void dp_aux_uart_switch_disable(void)
{
	if (aux_switch_priv.from_fsa4476) {
		// ENN H, EN1/EN2 00
		usb_analog_hs_plug_in_out_handle(DP_PLUG_OUT);
		hwlog_info("%s: dp plug out\n", __func__);

		// aux-uart switch is not existed.
		if (!aux_switch_priv.with_uart)
			return;
	}

	if (gpio_is_valid(aux_switch_priv.uart_gpio))
		gpio_direction_output(aux_switch_priv.uart_gpio, SET_GPIO_LOW);
	else
		hwlog_err("%s: invalid gpio %d\n",
			__func__, aux_switch_priv.uart_gpio);
}
EXPORT_SYMBOL_GPL(dp_aux_uart_switch_disable);

static void dp_aux_switch_gpio_init(struct device_node *np)
{
	int ret;

	aux_switch_priv.aux_gpio  = of_get_named_gpio(np, "cs-gpios", 0);
	aux_switch_priv.uart_gpio = of_get_named_gpio(np, "cs-gpios", 1);
	hwlog_info("%s: get aux switch gpio %d,%d\n", __func__,
		aux_switch_priv.aux_gpio, aux_switch_priv.uart_gpio);

	if (!gpio_is_valid(aux_switch_priv.aux_gpio)) {
		hwlog_err("%s: invalid gpio %d\n",
			__func__, aux_switch_priv.aux_gpio);
		return;
	}

	ret = gpio_request(aux_switch_priv.aux_gpio, "dp_aux_gpio");
	if (ret < 0)
		// NOTE:
		// For blanc, here gpio_request failed, because of gpio
		// requested by fsa4476. But, aux uart_gpio
		// need gpio_request, so don't goto return.
		hwlog_err("%s: request gpio %d failed %d\n",
			__func__, aux_switch_priv.aux_gpio, ret);
	else
		gpio_direction_output(aux_switch_priv.aux_gpio, SET_GPIO_LOW);

	if (!gpio_is_valid(aux_switch_priv.uart_gpio)) {
		hwlog_err("%s: invalid gpio %d\n",
			__func__, aux_switch_priv.uart_gpio);
		return;
	}

	ret = gpio_request(aux_switch_priv.uart_gpio, "dp_aux_uart_gpio");
	if (ret < 0)
		hwlog_err("%s: request gpio %d failed %d\n",
			__func__, aux_switch_priv.uart_gpio, ret);
	else
		gpio_direction_output(aux_switch_priv.uart_gpio, SET_GPIO_LOW);
}

static int dp_aux_switch_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int gpio;
	int ret;

	aux_switch_priv.from_fsa4476 = false;
	aux_switch_priv.with_uart = false;
	aux_switch_priv.ch_polarity = 0;
	aux_switch_priv.aux_gpio = INVALID_GPIO;
	aux_switch_priv.uart_gpio = INVALID_GPIO;

	if (of_property_read_bool(dev->of_node, "aux_switch_from_fsa4476")) {
		aux_switch_priv.from_fsa4476 = true;
		hwlog_info("%s: aux switch from fsa4476\n", __func__);

		if (of_property_read_bool(dev->of_node, "aux_switch_with_uart")) {
			aux_switch_priv.with_uart = true;
			hwlog_info("%s: aux switch with uart\n", __func__);
		}
	}

	// use two switches, not use fsa4476
	if (!aux_switch_priv.from_fsa4476) {
		dp_aux_switch_gpio_init(dev->of_node);
		return 0;
	}

	// aux switch from fsa4476, whether use aux/uart switch or not
	if (!aux_switch_priv.with_uart)
		return 0;

	gpio = of_get_named_gpio(dev->of_node, "aux_uart-gpio", 0);
	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s: get aux_uart-gpio failed %d\n", __func__, gpio);
		return 0;
	}

	// request aux uart gpio
	ret = gpio_request(gpio, "dp_aux_uart_gpio");
	if (ret < 0) {
		hwlog_err("%s: request uart gpio failed %d\n", __func__, ret);
	} else {
		gpio_direction_output(gpio, SET_GPIO_LOW);
		aux_switch_priv.uart_gpio = gpio;
		hwlog_info("%s: init uart gpio success %d\n", __func__, gpio);
	}

	return 0;
}

static int dp_aux_switch_remove(struct platform_device *pdev)
{
	if (!aux_switch_priv.from_fsa4476) {
		if (gpio_is_valid(aux_switch_priv.aux_gpio)) {
			gpio_free((unsigned int)aux_switch_priv.aux_gpio);
			aux_switch_priv.aux_gpio = INVALID_GPIO;
		}

		if (gpio_is_valid(aux_switch_priv.uart_gpio)) {
			gpio_free((unsigned int)aux_switch_priv.uart_gpio);
			aux_switch_priv.uart_gpio = INVALID_GPIO;
		}

		return 0;
	}

	if (aux_switch_priv.with_uart) {
		if (gpio_is_valid(aux_switch_priv.uart_gpio)) {
			gpio_free((unsigned int)aux_switch_priv.uart_gpio);
			aux_switch_priv.uart_gpio = INVALID_GPIO;
		}
	}

	return 0;
}

static const struct of_device_id dp_aux_switch_match[] = {
	{ .compatible = "huawei,dp_aux_switch", },
	{},
};

static struct platform_driver dp_aux_switch_driver = {
	.driver = {
		.name = "dp_aux_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dp_aux_switch_match),
	},
	.probe  = dp_aux_switch_probe,
	.remove = dp_aux_switch_remove,
};

static int __init dp_aux_switch_init(void)
{
	int ret;

	hwlog_info("%s: enter\n", __func__);
	ret = platform_driver_register(&dp_aux_switch_driver);
	if (ret < 0) {
		hwlog_err("%s: register driver failed %d\n", __func__, ret);
		return ret;
	}

	hwlog_info("%s: success\n", __func__);
	return 0;
}

static void __exit dp_aux_switch_exit(void)
{
	hwlog_info("%s: enter\n", __func__);
	platform_driver_unregister(&dp_aux_switch_driver);
}

module_init(dp_aux_switch_init);
module_exit(dp_aux_switch_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei dp aux driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
