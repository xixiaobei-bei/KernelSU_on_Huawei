/*
 * keyboard and clickpad driver for GP
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#include "i2c-hid-huawei.h"

#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>

int i2c_hid_parse_dts(struct i2c_client *client)
{
	int irq_gpio;
	struct device *dev = &client->dev;

	irq_gpio = of_get_named_gpio(dev->of_node, "irq_gpio", 0);
	if (irq_gpio < 0) {
		dev_err(&client->dev, "get irq_gpio failed\n");
		return -EINVAL;
	}

	if (!gpio_is_valid(irq_gpio)) {
		dev_err(&client->dev, "irq_gpio is not valid\n");
		return -EINVAL;
	}

	if (devm_gpio_request(&client->dev, irq_gpio, "i2c-hid-irq")) {
		dev_err(&client->dev, "Could not request %d gpio\n", irq_gpio);
		return -EINVAL;
	} else if (gpio_direction_input(irq_gpio)) {
		dev_err(&client->dev, "Could not set gpio %d as input\n",
			irq_gpio);
		return -EINVAL;
	}
	client->irq = gpio_to_irq(irq_gpio);
	if (client->irq < 0) {
		dev_err(&client->dev, "gpio_to_irq failed: %d\n", irq_gpio);
		return -EINVAL;
	}

	return 0;
}
