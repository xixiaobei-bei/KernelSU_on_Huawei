/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#include "overlay/hisi_overlay_utils.h"


#define DTS_COMP_HISI_OFFLINECOMPOSER_PANEL "hisilicon,offlinecomposer_panel"

static int hisi_offlinecompser_panel_on(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	DPU_FB_DEBUG("index=%d, enter!\n", dpufd->index);

	DPU_FB_DEBUG("index=%d, exit!\n", dpufd->index);

	return ret;
}

static int hisi_offlinecompser_panel_off(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	DPU_FB_DEBUG("index=%d, enter!\n", dpufd->index);

	DPU_FB_DEBUG("index=%d, exit!\n", dpufd->index);

	return ret;
}

static int hisi_offlinecompser_panel_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (!dpufd)
		return 0;

	DPU_FB_DEBUG("index=%d, enter!\n", dpufd->index);

	DPU_FB_DEBUG("index=%d, exit!\n", dpufd->index);

	return ret;
}

static struct dpu_panel_info hisi_offlinecompser_panel_info = {0};
static struct dpu_fb_panel_data hisi_offlinecomposer_panel_data = {
	.panel_info = &hisi_offlinecompser_panel_info,
	.set_fastboot = NULL,
	.on = hisi_offlinecompser_panel_on,
	.off = hisi_offlinecompser_panel_off,
	.remove = hisi_offlinecompser_panel_remove,
	.set_backlight = NULL,
	.vsync_ctrl = NULL,
	.esd_handle = NULL,
};

static int hisi_offlinecompser_probe(struct platform_device *pdev)
{
	int ret;
	struct dpu_panel_info *pinfo = NULL;

	if (hisi_fb_device_probe_defer(PANEL_OFFLINECOMPOSER, BL_SET_BY_NONE))
		goto err_probe_defer;

	DPU_FB_DEBUG("+.\n");

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}

	pdev->id = 1;
	pinfo = hisi_offlinecomposer_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct dpu_panel_info));
	pinfo->xres = g_primary_lcd_xres;
	pinfo->yres = g_primary_lcd_yres;
	pinfo->prefix_ce_support = g_prefix_ce_support;
	pinfo->prefix_sharpness1D_support = g_prefix_sharpness1d_support;
	pinfo->prefix_sharpness2D_support = g_prefix_sharpness2d_support;
	pinfo->type = PANEL_OFFLINECOMPOSER;
	pinfo->pxl_clk_rate = g_pxl_clk_rate;

	/* alloc panel device data */
	ret = platform_device_add_data(pdev, &hisi_offlinecomposer_panel_data,
		sizeof(struct dpu_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "platform_device_add_data failed!\n");
		goto err_device_put;
	}

	hisi_fb_add_device(pdev);

	DPU_FB_DEBUG("-.\n");

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
err_probe_defer:
	return -EPROBE_DEFER;
}

static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_HISI_OFFLINECOMPOSER_PANEL,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);

static struct platform_driver this_driver = {
	.probe = hisi_offlinecompser_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "hisi_offlinecompser_panel",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init hisi_offlinecompser_panel_init(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		DPU_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(hisi_offlinecompser_panel_init);
