/**
 * @file dp_drv.c
 * @brief display port driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "dp_drv.h"

#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/genalloc.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include  "dp_dev.h"

static int dp_of_device_setup(struct platform_device *pdev)
{
	return 0;
}

static void dp_of_device_release(struct platform_device *pdev)
{

}

static struct dp_match_data dp_info = {
	.of_device_setup = dp_of_device_setup,
	.of_device_release = dp_of_device_release,
};

static const struct of_device_id dsi_device_match_table[] = {
	{
		.compatible = DTS_COMP_DP0,
		.data = &dp_info,
	},
	{},
};
MODULE_DEVICE_TABLE(of, dsi_device_match_table);

static int dp_probe(struct platform_device *pdev)
{
	const struct dp_match_data *data = NULL;

	if (!pdev)
		return -EINVAL;

	data = of_device_get_match_data(&pdev->dev);
	if (!data) {
		pr_err("get device data failed!\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * Clear resource when device removed but not for devicetree device
 */
static int dp_remove(struct platform_device *pdev)
{
	const struct dp_match_data *data = NULL;
	struct dp_private *priv = (struct dp_private *)platform_get_drvdata(pdev);

	if (!priv) {
		pr_err("get dsi private data failed!\n");
		return -EINVAL;
	}

	if (priv->device_initialized == 0) {
		pr_err("has no initialization!\n");
		return -EINVAL;
	}

	data = of_device_get_match_data(&pdev->dev);
	if (data != NULL)
		data->of_device_release(pdev);

	pr_info("dp remove complete!\n");

	priv->device_initialized = 0;

	return 0;
}

static struct platform_driver dp_platform_driver = {
	.probe  = dp_probe,
	.remove = dp_remove,
	.driver = {
		.name  = DEV_NAME_DP,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dsi_device_match_table),
	}
};

static int __init dp_register(void)
{
	return platform_driver_register(&dp_platform_driver);
}

static void __exit dp_unregister(void)
{
	platform_driver_unregister(&dp_platform_driver);
}

module_init(dp_register);
module_exit(dp_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Dsi Module Driver");
MODULE_LICENSE("GPL");
