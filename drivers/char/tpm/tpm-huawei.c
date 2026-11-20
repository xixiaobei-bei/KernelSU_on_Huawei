/*
 * tpm driver for GP
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

#include "tpm.h"
#include "tpm-huawei.h"
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

int tpm_reset(struct spi_device *dev)
{
	int rst_gpio;
	struct device_node *np = dev->dev.of_node;
	if (!np) {
		dev_err(&dev->dev, "no dt node defined\n");
		return -EINVAL;
	}

	rst_gpio = of_get_named_gpio(np, "rst_gpio", 0);
	if (rst_gpio < 0) {
		dev_err(&dev->dev, "get rst_gpio failed\n");
		return -EINVAL;
	}

	if (!gpio_is_valid(rst_gpio)) {
		dev_err(&dev->dev, "rst_gpio is not valid\n");
		return -EINVAL;
	}

	if (devm_gpio_request(&dev->dev, rst_gpio, "rst_gpio")) {
		dev_err(&dev->dev, "Could not request %d gpio\n", rst_gpio);
		return -EINVAL;
	} else if (gpio_direction_output(rst_gpio, GPIO_LOW)) {
		dev_err(&dev->dev, "Could not set gpio %d as output\n", rst_gpio);
		return -EINVAL;
	}

	gpio_set_value(rst_gpio, GPIO_LOW);
	tpm_msleep(5);
	gpio_set_value(rst_gpio, GPIO_HIGH);
	tpm_msleep(2);

	return 0;
}

int tpm_set_voltage(struct spi_device *dev)
{
	int ret;
	dev->supply = devm_regulator_get(&dev->dev, "tpm");
	if (IS_ERR(dev->supply)) {
		ret = PTR_ERR(dev->supply);
		if (ret != -EPROBE_DEFER)
			dev_err(&dev->dev, "Failed to get regulator: %d\n",
				ret);
		return ret;
	}

	regulator_set_voltage(dev->supply, 1800000, 1800000);

	ret = regulator_enable(dev->supply);
	if (ret < 0) {
		dev_err(&dev->dev, "Failed to enable regulator: %d\n",
			ret);
		return ret;
	}

	return 0;
}

/* set voltage and reset tpm */
int tpm_set_voltage_and_reset(struct spi_device *dev)
{
	int ret;
	ret = tpm_set_voltage(dev);
	if (ret < 0) {
		dev_err(&dev->dev, "Failed to set voltage tpm\n");
		return ret;
	}

	tpm_msleep(2);
	ret = tpm_reset(dev);
	if (ret < 0) {
		dev_err(&dev->dev, "Failed to reset tpm\n");
		return ret;
	}

	return 0;
}


