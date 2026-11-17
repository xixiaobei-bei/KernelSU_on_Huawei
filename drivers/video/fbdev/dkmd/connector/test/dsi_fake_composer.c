
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include "dkmd_utils.h"
#include "mipi_dsi_drv.h"

#define DEFAULT_DSS_CORE_CLK_RATE_L4    (72000000UL)
#define DEFAULT_DSS_CORE_CLK_RATE_L3    (480000000UL)
#define DEFAULT_DSS_CORE_CLK_RATE_L2    (417000000UL)
#define DEFAULT_DSS_CORE_CLK_RATE_L1    (278000000UL)
#define DEFAULT_DSS_CORE_CLK_RATE_POWER_OFF    (104000000UL)

static int composer_on(struct platform_device *pdev)
{
	int ret = 0;
	struct mipi_dsi_drv_private *priv = NULL;
	struct dkmd_connector_data *pdata = NULL;

	priv = to_mipi_dsi_private(pdev);

	ret = regulator_enable(priv->media1_subsys_regulator);
	if (ret)
		pr_err("id: %d, media1_subsys_regulatore failed, error=%d!\n", priv->id, ret);

	ret = regulator_enable(priv->vivobus_regulator);
	if (ret)
		pr_err("id: %d, vivobus_regulator failed, error=%d!\n", priv->id, ret);

	ret = clk_prepare_enable(priv->aclk_gate_dss);
	if (ret)
		pr_err("id: %d, aclk_gate_dss failed, error=%d!\n", priv->id, ret);

	ret = clk_prepare_enable(priv->pclk_gate_dss);
	if (ret)
		pr_err("id: %d, pclk_gate_dss failed, error=%d!\n", priv->id, ret);

	ret = clk_prepare_enable(priv->clk_gate_edc);
	if (ret)
		pr_err("id: %d, clk_gate_edc failed, error=%d!\n", priv->id, ret);

	ret = clk_set_rate(priv->clk_gate_edc, DEFAULT_DSS_CORE_CLK_RATE_L1);
	if (ret)
		pr_err("id: %d, clk_gate_edc failed, error=%d!\n", priv->id, ret);

	ret = regulator_enable(priv->dsssubsys_regulator);
	if (ret)
		pr_err("id: %d, dsssubsys_regulator failed, error=%d!\n", priv->id, ret);

	pdata = dev_get_platdata(&pdev->dev);

	return pdata->on_func(pdev);
}

static int composer_off(struct platform_device *pdev)
{
	struct mipi_dsi_drv_private *priv = NULL;
	struct dkmd_connector_data *pdata = NULL;

	priv = to_mipi_dsi_private(pdev);
	pdata = dev_get_platdata(&pdev->dev);

	pdata->off_func(pdev);

	regulator_disable(priv->dsssubsys_regulator);

	clk_set_rate(priv->clk_gate_edc, DEFAULT_DSS_CORE_CLK_RATE_L1);
	clk_disable_unprepare(priv->clk_gate_edc);
	clk_disable_unprepare(priv->pclk_gate_dss);
	clk_disable_unprepare(priv->aclk_gate_dss);

	regulator_disable(priv->vivobus_regulator);
	regulator_disable(priv->media1_subsys_regulator);

	return 0;
}

struct composer_dev_private_data composer_dev_data = {
	.base = {
		.on_func = composer_on,
		.off_func = composer_off,
		.ops_handle_func = NULL,
		.next = NULL,
	},
};
EXPORT_SYMBOL(composer_dev_data);

void comp_mgr_register_device(struct platform_device *pdev)
{
	struct dkmd_connector_info *pinfo = NULL;
	struct composer_dev_private_data *pdata = &composer_dev_data;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return;
	}
	/* Link next device */
	pdata->base.next = pdev;
	pdata->pdev = pdev;

	pinfo = platform_get_drvdata(pdev);
	if (!pinfo) {
		pr_err("pinfo is NULL!\n");
		return;
	}
}
EXPORT_SYMBOL(comp_mgr_register_device);
