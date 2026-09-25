/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: Battery thermal driver
 * Authors: Hisilicon
 * Create: 2019-10-25
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/version.h>

struct battery_thermal_priv {
	struct thermal_zone_device *bat_thermal;
	struct thermal_zone_device *supply_avg;
	struct thermal_zone_device *supply_peak;
};

static int battery_thermal_get_temp(
	struct thermal_zone_device *thermal, int *temp)
{
	int raw_temp;

	if (thermal == NULL || temp == NULL)
		return -EINVAL;
	hisi_battery_temperature_permille(&raw_temp);
	*temp = raw_temp; /* to fit iaware interface */
	return 0;
}

static struct thermal_zone_device_ops battery_thermal_ops = {
	.get_temp = battery_thermal_get_temp,
};

static int supply_avg_get_temp(struct thermal_zone_device *thermal, int *temp)
{
	int raw_temp;

	if (thermal == NULL || temp == NULL)
		return -EINVAL;
	raw_temp = hisi_get_polar_avg();
	*temp = raw_temp;
	return 0;
}

static struct thermal_zone_device_ops supply_avg_ops = {
	.get_temp = supply_avg_get_temp,
};

static int supply_peak_get_temp(struct thermal_zone_device *thermal, int *temp)
{
	int raw_temp;

	if (thermal == NULL || temp == NULL)
		return -EINVAL;
	raw_temp = hisi_get_polar_peak();
	*temp = raw_temp;
	return 0;
}

static struct thermal_zone_device_ops supply_peak_ops = {
	.get_temp = supply_peak_get_temp,
};

static int battery_thermal_probe(struct platform_device *pdev)
{
	struct battery_thermal_priv *priv;
	int result;

	priv = kzalloc(sizeof(struct battery_thermal_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	priv->bat_thermal = thermal_zone_device_register(
		"bat_raw_temp", 0, 0, priv, &battery_thermal_ops, NULL, 0, 0);
	if (IS_ERR(priv->bat_thermal)) {
		result = PTR_ERR(priv->bat_thermal);
		goto free_priv;
	}

	priv->supply_avg = thermal_zone_device_register(
		"supply_avg", 0, 0, priv, &supply_avg_ops, NULL, 0, 0);
	if (IS_ERR(priv->supply_avg)) {
		result = PTR_ERR(priv->supply_avg);
		goto free_priv;
	}

	priv->supply_peak = thermal_zone_device_register(
		"supply_peak", 0, 0, priv, &supply_peak_ops, NULL, 0, 0);
	if (IS_ERR(priv->supply_peak)) {
		result = PTR_ERR(priv->supply_peak);
		goto free_priv;
	}

	return 0;

free_priv:
	kfree(priv);
	return result;
}

static int battery_thermal_remove(struct platform_device *pdev)
{
	struct battery_thermal_priv *priv = platform_get_drvdata(pdev);

	thermal_zone_device_unregister(priv->bat_thermal);
	thermal_zone_device_unregister(priv->supply_avg);
	thermal_zone_device_unregister(priv->supply_peak);
	kfree(priv);
	return 0;
}

static const struct of_device_id battery_thermal_match[] = {
	{.compatible = "hisilicon,bat_thermal", .data = NULL}, {}};

MODULE_DEVICE_TABLE(of, battery_thermal_match);

static struct platform_driver battery_thermal_driver = {
	.probe = battery_thermal_probe,
	.remove = battery_thermal_remove,
	.driver =
		{
			.name = "battery thermal",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(battery_thermal_match),
		},
};

static int __init hisi_bat_thermal_init(void)
{
	platform_driver_register(&battery_thermal_driver);
	return 0;
}

device_initcall(hisi_bat_thermal_init);

static void __exit hisi_bat_thermal_exit(void)
{
	platform_driver_unregister(&battery_thermal_driver);
}

module_exit(hisi_bat_thermal_exit);

MODULE_DESCRIPTION("Battery Thermal driver");
MODULE_AUTHOR("Hisilicon");
MODULE_LICENSE("GPL");
