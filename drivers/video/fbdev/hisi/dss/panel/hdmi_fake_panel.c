/* Copyright (c) 2013-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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


#define DTS_COMP_HDMI_FAKE_PANEL "hisilicon,hdmi_fake_panel"


static struct hisi_panel_info g_fake_panel_info = {0};
static struct hisi_fb_panel_data g_fake_panel_data = {
	.panel_info = &g_fake_panel_info,
	.set_fastboot = NULL,
	.on = NULL,
	.off = NULL,
	.remove = NULL,
	.set_backlight = NULL,
};

/* lcd initialized value from panel spec */
static int init_lcd_info(uint32_t is_fake_hdmi)
{
	struct hisi_panel_info *pinfo = g_fake_panel_data.panel_info;

	memset(pinfo, 0, sizeof(struct hisi_panel_info));

	pinfo->xres = 1080;
	pinfo->yres = 1920;
	pinfo->width  = 61;
	pinfo->height = 109;
	pinfo->type = PANEL_HDMI;
	pinfo->orientation = LCD_LANDSCAPE;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = BL_SET_BY_NONE;

	if (pinfo->bl_set_type == BL_SET_BY_BLPWM)
		pinfo->blpwm_input_ena = 0;

	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;

	pinfo->ifbc_type = IFBC_TYPE_NONE;
	pinfo->fake_external = is_fake_hdmi;

	pinfo->vsync_ctrl_type = 0;
	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;
	pinfo->dirty_region_updt_support = 0;

	pinfo->ldi.h_back_porch = 23;
	pinfo->ldi.h_front_porch = 50;
	pinfo->ldi.h_pulse_width = 20;
	pinfo->ldi.v_back_porch = 12;
	pinfo->ldi.v_front_porch = 14;
	pinfo->ldi.v_pulse_width = 4;

	pinfo->mipi.lane_nums = DSI_4_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;
	pinfo->mipi.vc = 0;
	pinfo->mipi.dsi_bit_clk = 480;

	pinfo->pxl_clk_rate = 27 * 1000000UL;

	return 0;
}

static int hdmi_fake_panel_probe(struct platform_device *pdev)
{
	// return ret = 0 before assign value to it
	int ret = 0;
	uint32_t is_fake_hdmi = 0;
	struct device_node *np = NULL;

	if (hisi_fb_device_probe_defer(PANEL_HDMI, BL_SET_BY_NONE))
		return -EPROBE_DEFER;

	HISI_FB_DEBUG("+.\n");

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_HDMI_FAKE_PANEL);
	if (np == NULL) {
		HISI_FB_ERR("NOT FOUND device node %s!\n",
			DTS_COMP_HDMI_FAKE_PANEL);
		return ret;
	}

	ret = of_property_read_u32(np, "fake_hdmi", &is_fake_hdmi);
	if (ret != 0) {
		HISI_FB_DEBUG("dts get fake_hdmi fail");
		is_fake_hdmi = 0;
	}
	HISI_FB_INFO("is_fake_hdmi=%d.\n", is_fake_hdmi);

	if (pdev == NULL) {
		HISI_FB_ERR("NOT FOUND platform_device");
		return ret;
	}

	pdev->id = 1;

	/* init lcd info */
	ret = init_lcd_info(is_fake_hdmi);
	if (ret != 0) {
		HISI_FB_DEBUG("INITIATE lcd fail\n");
		return ret;
	}

	/* alloc panel device data */
	ret = platform_device_add_data(pdev, &g_fake_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret != 0) {
		HISI_FB_ERR("platform_device_add_data failed!\n");
		platform_device_put(pdev);
		return ret;
	}

	hisi_fb_add_device(pdev);

	HISI_FB_DEBUG("-.\n");

	return 0;
}

static const struct of_device_id g_hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_HDMI_FAKE_PANEL,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_hisi_panel_match_table);

static struct platform_driver g_this_driver = {
	.probe = hdmi_fake_panel_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "hdmi_fake_panel",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_hisi_panel_match_table),
	},
};

static int __init hdmi_fake_panel_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&g_this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n",
			ret);
		return ret;
	}

	return ret;
}

module_init(hdmi_fake_panel_init);
