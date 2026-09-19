/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include "hisi_fb.h"

#define DTS_COMP_DP_FAKE_PANEL	"hisilicon,dp_fake_panel"

static struct hisi_panel_info fake_panel_info = {0};
/*lint -save -e* */
static struct hisi_fb_panel_data fake_panel_data = {
	.panel_info = &fake_panel_info,
	.set_fastboot = NULL,
	.on = NULL,
	.off = NULL,
	.remove = NULL,
	.set_backlight = NULL,
};
/*lint -restore*/

static int dp_fake_panel_probe(struct platform_device *pdev)
{
	int ret = 0;
	uint32_t is_fake_dp;
	struct hisi_panel_info *pinfo;
	struct device_node *np;
	//lint -save -e747
	if (hisi_fb_device_probe_defer(PANEL_DP, BL_SET_BY_NONE)) {
		return -EPROBE_DEFER;
	}
	//lint -restore
	HISI_FB_DEBUG("+.\n");
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_DP_FAKE_PANEL);
	if (!np) {
		HISI_FB_ERR("[DP] NOT FOUND device node %s!\n", DTS_COMP_DP_FAKE_PANEL);
		ret = -1;
		return ret;
	}

	ret = of_property_read_u32(np, "fake_dp", &is_fake_dp);//lint !e64
	if (ret) {
		HISI_FB_DEBUG("dts get fake_dp fail");
		is_fake_dp = 0;
	}
	HISI_FB_INFO("[DP] is_fake_dp=%d.\n", is_fake_dp);

	pdev->id = 1;
	/* init lcd info */
	pinfo = fake_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));

	pinfo->width = 16000;
	pinfo->height = 9000;

	pinfo->type = PANEL_DP;
	pinfo->orientation = LCD_LANDSCAPE;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = BL_SET_BY_NONE;

	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;
	pinfo->ifbc_type = IFBC_TYPE_NONE;
	//pinfo->fake_external = is_fake_dp;

	pinfo->vsync_ctrl_type = 0;
	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;
	pinfo->dirty_region_updt_support = 0;

	pinfo->ldi.hsync_plr = 1;
	pinfo->ldi.vsync_plr = 1;
	pinfo->ldi.pixelclk_plr = 0;
	pinfo->ldi.data_en_plr = 0;

	if (g_fpga_flag == 1) {
		pinfo->xres = 1280;
		pinfo->yres = 720;
		pinfo->ldi.h_back_porch = 220;
		pinfo->ldi.h_front_porch = 110;
		pinfo->ldi.h_pulse_width = 40;
		pinfo->ldi.v_back_porch = 20;
		pinfo->ldi.v_front_porch = 30;
		pinfo->ldi.v_pulse_width = 5;
		pinfo->pxl_clk_rate = 27 * 1000000UL;
		pinfo->pxl_clk_rate_div = 1;
	} else {
		pinfo->xres = 1920;
		pinfo->yres = 1080;
		pinfo->ldi.h_back_porch = 148;
		pinfo->ldi.h_front_porch = 88;
		pinfo->ldi.h_pulse_width = 44;
		pinfo->ldi.v_back_porch = 36;
		pinfo->ldi.v_front_porch = 4;
		pinfo->ldi.v_pulse_width = 5;
		pinfo->pxl_clk_rate = 148500000UL;
		pinfo->pxl_clk_rate_div = 1;
	}
	/* alloc panel device data */
	ret = platform_device_add_data(pdev, &fake_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		HISI_FB_ERR("[DP] platform_device_add_data failed!\n");
		platform_device_put(pdev);
		return ret;
	}
	hisi_fb_add_device(pdev);

	platform_device_put(pdev);
	return ret;
}
/*lint -save -e* */
static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_DP_FAKE_PANEL,
		.data = NULL,
	},
	{},
};
/*lint -restore*/
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);
/*lint -save -e* */
static struct platform_driver this_driver = {
	.probe = dp_fake_panel_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "dp_fake_panel",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};
/*lint -restore*/

static int __init dp_fake_panel_init(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);//lint !e64

	if (ret) {
		HISI_FB_ERR("[DP] platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

/*lint -e528 -esym(528,*)*/
module_init(dp_fake_panel_init);
/*lint -e528 +esym(528,*)*/
