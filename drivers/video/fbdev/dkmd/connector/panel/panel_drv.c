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
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/module.h>

#include "mipi_dsi_dev.h"
#include "dkmd_utils.h"
#include "panel_dev.h"
#include "panel_drv.h"

uint32_t gpio_lcd_vsp_enable;
uint32_t gpio_lcd_vsn_enable;
uint32_t gpio_lcd_reset;
uint32_t gpio_lcd_bl_enable;
uint32_t gpio_lcd_tp1v8;

static void panel_get_dts_resource(struct dkmd_connector_info *pinfo, struct device_node *np)
{
	int ret = 0;

	ret = of_property_read_u32(np, LCD_DISPLAY_TYPE_NAME, &pinfo->type);
	if (ret) {
		pr_info("get lcd_display_type failed!\n");
		pinfo->type = PANEL_MIPI_CMD;
	}
	pr_info("panel_type=0x%x", pinfo->type);

	ret = of_property_read_u32(np, LCD_DSI_TYPE_NAME, &pinfo->dsi_type);
	if (ret) {
		pr_info("get lcd_display_type failed!\n");
		pinfo->dsi_type = DSI0_INDEX;
	}
	pr_info("dsi_type=0x%x", pinfo->dsi_type);

	ret = of_property_read_u32(np, LCD_IFBC_TYPE_NAME, &pinfo->ifbc_type);
	if (ret) {
		pr_info("get ifbc_type failed!\n");
		pinfo->ifbc_type = IFBC_TYPE_NONE;
	}
	pr_info("ifbc_type=0x%x", pinfo->ifbc_type);

	ret = of_property_read_u32(np, LCD_BL_TYPE_NAME, &pinfo->bl_type);
	if (ret) {
		pr_info("get lcd_bl_type failed!\n");
		pinfo->bl_type = BL_SET_BY_MIPI;
	}
	pr_info("bl_type=0x%x", pinfo->bl_type);

	ret = of_property_read_u32(np, FPGA_FLAG_NAME, &pinfo->fpga_flag);
	if (ret) {
		pr_info("get fpga_flag failed!\n");
		pinfo->fpga_flag = 0;
	}
	pr_info("fpga_flag=0x%x", pinfo->fpga_flag);

	gpio_lcd_vsp_enable = of_get_named_gpio(np, "gpios", 0);
	gpio_lcd_vsn_enable = of_get_named_gpio(np, "gpios", 1);
	gpio_lcd_reset = of_get_named_gpio(np, "gpios", 2);
	gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 3);
	gpio_lcd_tp1v8 = of_get_named_gpio(np, "gpios", 4);

	pr_info("used gpio:[vsp: %d, vsn: %d, rst: %d, bl_en: %d, tp1v8: %d]\n",
		gpio_lcd_vsp_enable, gpio_lcd_vsn_enable, gpio_lcd_reset, gpio_lcd_bl_enable, gpio_lcd_tp1v8);
}

int panel_base_of_device_setup(struct platform_device *pdev)
{
	struct dkmd_connector_info *pinfo = NULL;

	pinfo = (struct dkmd_connector_info *)platform_get_drvdata(pdev);
	if (!pinfo) {
		pr_err("get panel private data failed!\n");
		return -EINVAL;
	}

	// TODO: follow config should read from dtsi
	pinfo->xres = 1080;
	pinfo->yres = 1920;
	pinfo->width = 75;
	pinfo->height = 133;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;

	panel_get_dts_resource(pinfo, pdev->dev.of_node);

	panel_drv_data_setup(pdev);

	pdev->id = 2; // extern device

	pr_info("base setup panel private data!\n");

	return 0;
}

void panel_base_of_device_release(struct platform_device *pdev)
{
	pr_info("base release panel private data!\n");
}

#define DTS_COMP_FAKE_PANEL "hisilicon,dsi_fake_panel"
panel_device_match_data(fake_panel_info, 0, panel_base_of_device_setup, panel_base_of_device_release);

static const struct of_device_id panel_device_match_table[] = {
	{
		.compatible = DTS_COMP_FAKE_PANEL,
		.data = &fake_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_TD4322,
		.data = &td4322_panel_info,
	},
	{},
};
MODULE_DEVICE_TABLE(of, panel_device_match_table);

static int panel_probe(struct platform_device *pdev)
{
	const struct panel_match_data *data = NULL;
	struct panel_drv_private *priv = NULL;

	if (!pdev) {
		pr_err("pdev is null!\n");
		return -EINVAL;
	}

	data = of_device_get_match_data(&pdev->dev);
	if (!data) {
		pr_err("get device data failed!\n");
		return -EINVAL;
	}
	pr_info("panel_id:%d panel_probe enter!\n", data->version_id);

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		pr_err("alloc panel driver private data fialed!\n");
		return -EINVAL;
	}
	priv->pdev = pdev;

	platform_set_drvdata(pdev, &(priv->base));
	if (data->of_device_setup(pdev)) {
		pr_err("Device initialization is failed!\n");
		return -EINVAL;
	}

	if (priv->base.dsi_type & DSI0_INDEX)
		priv->dsi_idx = CONNECTOR_DSI0;

	if (priv->base.dsi_type & (DSI0_INDEX | DSI1_INDEX))
		priv->ext_dsi_idx = CONNECTOR_DSI1;
	else if (priv->base.dsi_type & DSI1_INDEX)
		priv->dsi_idx = CONNECTOR_DSI1;
	else
		pr_err("set err connector idx device!\n");

	/* add panel connector data to device */
	platform_device_add_data(pdev, &(panel_dev_data.base), sizeof(panel_dev_data.base));

	dsi_device_register(pdev);

	priv->device_initialized = 1;

	pr_info("panel_probe exit!\n");

	return 0;
}

/**
 * Clear resource when device removed but not for devicetree device
 */
static int panel_remove(struct platform_device *pdev)
{
	const struct panel_match_data *data = NULL;
	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_err("get dsi private data failed!\n");
		return -EINVAL;
	}

	if (priv->device_initialized == 0) {
		pr_err("has no initialization!\n");
		return -EINVAL;
	}
	dsi_device_unregister(pdev);

	data = of_device_get_match_data(&pdev->dev);
	if (data != NULL)
		data->of_device_release(pdev);

	pr_info("panel remove complete!\n");

	priv->device_initialized = 0;

	return 0;
}

static struct platform_driver panel_platform_driver = {
	.probe  = panel_probe,
	.remove = panel_remove,
	.driver = {
		.name  = DEV_NAME_PANEL,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(panel_device_match_table),
	}
};

static int __init panel_register(void)
{
	return platform_driver_register(&panel_platform_driver);
}

static void __exit panel_unregister(void)
{
	platform_driver_unregister(&panel_platform_driver);
}

late_initcall(panel_register);
module_exit(panel_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Panel Module Driver");
MODULE_LICENSE("GPL");
