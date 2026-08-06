/**
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
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/genalloc.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include <dpu/soc_dpu_define.h>

#include "mipi_dsi_drv.h"

struct platform_device *g_dsi_dev[CONNECTOR_MAX_NUM];
static int mipi_dsi_get_dts_resource(struct mipi_dsi_drv_private *priv, struct device_node *np)
{
	priv->aclk_gate_dss = of_clk_get(np, 0);
	if (IS_ERR(priv->aclk_gate_dss)) {
		pr_err("dsi-%d failed to get aclk_gate_dss!\n", priv->id);
		return -ENXIO;
	}

	priv->pclk_gate_dss = of_clk_get(np, 1);
	if (IS_ERR(priv->pclk_gate_dss)) {
		pr_err("dsi-%d failed to get pclk_gate_dss!\n", priv->id);
		return -ENXIO;
	}

	priv->clk_gate_edc = of_clk_get(np, 2);
	if (IS_ERR(priv->clk_gate_edc)) {
		pr_err("dsi-%d failed to get clk_gate_edc!\n", priv->id);
		return -ENXIO;
	}

	priv->clk_gate_txdphy_ref = of_clk_get(np, 3);
	if (IS_ERR(priv->clk_gate_txdphy_ref)) {
		pr_err("dsi-%d failed to get clk_gate_txdphy_ref!\n", priv->id);
		return -ENXIO;
	}

	priv->clk_gate_txdphy_cfg = of_clk_get(np, 4);
	if (IS_ERR(priv->clk_gate_txdphy_cfg)) {
		pr_err("dsi-%d failed to get clk_gate_txdphy_cfg!\n", priv->id);
		return -ENXIO;
	}

	priv->pclk_gate_dsi = of_clk_get(np, 5);
	if (IS_ERR(priv->pclk_gate_dsi)) {
		pr_err("dsi-%d failed to get pclk_gate_dsi!\n", priv->id);
		return -ENXIO;
	}

	priv->irq = of_irq_get(np, 0);
	pr_err("dsi-%d get irq = %d", priv->id, priv->irq);

	priv->dpu_base = of_iomap(np, 0);
	if (!priv->dpu_base) {
		pr_err("dsi-%d failed to get dpu_base!\n", priv->id);
		return -ENXIO;
	}
	pr_info("dsi-%d get dpu_base = %#x", priv->id, priv->dpu_base);

	priv->peri_crg_base = of_iomap(np, 1);
	if (!priv->peri_crg_base) {
		pr_err("dsi-%d failed to get peri_crg_base!\n", priv->id);
		return -ENXIO;
	}
	pr_info("dsi-%d get peri_crg_base = %#x", priv->id, priv->peri_crg_base);

	priv->dsi_base = priv->dpu_base + DPU_MIPI_DSI0_OFFSET + priv->id * 0x400;

	return 0;
}

static int mipi_dsi_of_device_setup(struct platform_device *pdev)
{
	int ret = 0;
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);

	if (!priv) {
		pr_err("get dsi private data failed!\n");
		return -1;
	}

	ret = mipi_dsi_get_dts_resource(priv, pdev->dev.of_node);
	if (ret) {
		pr_err("get dsi dts data failed!\n");
		return -1;
	}

	priv->dsssubsys_regulator = devm_regulator_get(&pdev->dev, "regulator_dsssubsys");
	if (IS_ERR(priv->dsssubsys_regulator)) {
		ret = PTR_ERR(priv->dsssubsys_regulator);
		pr_err("dsssubsys_regulator error, ret=%d", ret);
		return ret;
	}

	priv->vivobus_regulator = devm_regulator_get(&pdev->dev, "regulator_vivobus");
	if (IS_ERR(priv->vivobus_regulator)) {
		ret = PTR_ERR(priv->vivobus_regulator);
		pr_err("vivobus_regulator error, ret=%d", ret);
		return ret;
	}

	priv->media1_subsys_regulator = devm_regulator_get(&pdev->dev, "regulator_media_subsys");
	if (IS_ERR(priv->media1_subsys_regulator)) {
		ret = PTR_ERR(priv->media1_subsys_regulator);
		pr_err("media1_subsys_regulator error, ret=%d", ret);
		return ret;
	}

	return 0;
}

static void mipi_dsi_of_device_release(struct platform_device *pdev)
{
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);

	if (!priv) {
		pr_err("get dsi private data failed!\n");
		return;
	}
}

#define dsi_device_match_data(name, ver, setup, release) \
	static struct mipi_dsi_match_data name = { .version_id = ver, .of_device_setup = setup, .of_device_release = release }

dsi_device_match_data(mipi_dsi0_info, CONNECTOR_DSI0, mipi_dsi_of_device_setup, mipi_dsi_of_device_release);
dsi_device_match_data(mipi_dsi1_info, CONNECTOR_DSI1, mipi_dsi_of_device_setup, mipi_dsi_of_device_release);

static const struct of_device_id dsi_device_match_table[] = {
	{
		.compatible = DTS_COMP_DSI0,
		.data = &mipi_dsi0_info,
	},
	{
		.compatible = DTS_COMP_DSI1,
		.data = &mipi_dsi1_info,
	},
	{},
};
MODULE_DEVICE_TABLE(of, dsi_device_match_table);

static int mipi_dsi_probe(struct platform_device *pdev)
{
	const struct mipi_dsi_match_data *data = NULL;
	struct mipi_dsi_drv_private *priv = NULL;

	pr_info("mipi_dsi_probe enter!\n");

	if (!pdev) {
		pr_err("pdev is null!\n");
		return -EINVAL;
	}

	data = of_device_get_match_data(&pdev->dev);
	if (!data) {
		pr_err("get device data failed!\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		pr_err("alloc dsi private data fialed!\n");
		return -EINVAL;
	}
	priv->pdev = pdev;
	priv->id = data->version_id;
	pr_info("dsi-%d driver pdev %p!\n", priv->id, pdev);
	g_dsi_dev[priv->id] = pdev;

	pr_info("dsi-%d driver g_dsi_dev %p!\n", priv->id, g_dsi_dev[priv->id]);

	platform_set_drvdata(pdev, &(priv->base));
	if (data->of_device_setup(pdev)) {
		pr_err("device setup failed!\n");
		return -EINVAL;
	}

	priv->device_initialized = 1;

	pr_info("dsi-%d driver init success!\n", priv->id);

	return 0;
}

/**
 * Clear resource when device removed but not for devicetree device
 */
static int mipi_dsi_remove(struct platform_device *pdev)
{
	const struct mipi_dsi_match_data *data = NULL;
	struct mipi_dsi_drv_private *priv = to_mipi_dsi_private(pdev);

	if (!priv) {
		pr_err("get dsi private data failed!\n");
		return -EINVAL;
	}

	if (priv->device_initialized == 0) {
		pr_err("has no init!\n");
		return -EINVAL;
	}

	data = of_device_get_match_data(&pdev->dev);
	if (!data)
		data->of_device_release(pdev);

	pr_info("dsi-%d remove complete!\n", priv->id);

	priv->device_initialized = 0;

	return 0;
}

static struct platform_driver mipi_dsi_platform_driver = {
	.probe  = mipi_dsi_probe,
	.remove = mipi_dsi_remove,
	.driver = {
		.name  = DEV_NAME_DSI,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dsi_device_match_table),
	},
};

static int __init mipi_dsi_register(void)
{
	return platform_driver_register(&mipi_dsi_platform_driver);
}

static void __exit mipi_dsi_unregister(void)
{
	platform_driver_unregister(&mipi_dsi_platform_driver);
}

module_init(mipi_dsi_register);
module_exit(mipi_dsi_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Dsi Module Driver");
MODULE_LICENSE("GPL");
