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
#include "hisi_overlay_utils.h"
#include <linux/device.h>


#define DTS_COMP_HISI_MEDIACOMMON_PANEL	"hisilicon,mediacommon_panel"

/*******************************************************************************
**
*/
/*lint -save -e730 -e732 -e838 -e785 -e747 -e732 -e64 -e528*/

static int hisi_mediacommon_panel_on(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is null.\n");
		return -1;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null.\n");
		return -1;
	}

	HISI_FB_DEBUG("index=%d, enter!\n", hisifd->index);

	HISI_FB_DEBUG("index=%d, exit!\n", hisifd->index);

	return ret;
}

static int hisi_mediacommon_panel_off(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is null.\n");
		return -1;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null.\n");
		return -1;
	}

	HISI_FB_DEBUG("index=%d, enter!\n", hisifd->index);

	HISI_FB_DEBUG("index=%d, exit!\n", hisifd->index);

	return ret;
}

static int hisi_mediacommon_panel_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!pdev) {
		return 0;
	}
	hisifd = platform_get_drvdata(pdev);

	if (!hisifd) {
		return 0;
	}

	HISI_FB_DEBUG("index=%d, enter!\n", hisifd->index);

	HISI_FB_DEBUG("index=%d, exit!\n", hisifd->index);

	return ret;
}

static struct hisi_panel_info hisi_mediacommon_panel_info = {0};
static struct hisi_fb_panel_data hisi_mediacommon_panel_data = {
	.panel_info = &hisi_mediacommon_panel_info,
	.set_fastboot = NULL,
	.on = hisi_mediacommon_panel_on,
	.off = hisi_mediacommon_panel_off,
	.remove = hisi_mediacommon_panel_remove,
	.set_backlight = NULL,
	.sbl_ctrl = NULL,
	.vsync_ctrl = NULL,
	.esd_handle = NULL,
};

static int hisi_mediacommon_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (hisi_fb_device_probe_defer(PANEL_MEDIACOMMON, BL_SET_BY_NONE)) {
		goto err_probe_defer;
	}

	HISI_FB_DEBUG("+.\n");

	if (pdev == NULL) {
		HISI_FB_ERR("pdev is NULL point.\n");
		goto err_probe_defer;
	}

	pdev->id = 1;
	pinfo = hisi_mediacommon_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->xres = g_primary_lcd_xres;
	pinfo->yres = g_primary_lcd_yres;
	pinfo->type = PANEL_MEDIACOMMON;
	pinfo->pxl_clk_rate = g_pxl_clk_rate;

	/* alloc panel device data */
	ret = platform_device_add_data(pdev, &hisi_mediacommon_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "platform_device_add_data failed!\n");
		goto err_device_put;
	}

	hisi_fb_add_device(pdev);

	dev_dbg(&pdev->dev, "-.\n");

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
err_probe_defer:
	return -EPROBE_DEFER;
}

static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_HISI_MEDIACOMMON_PANEL,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);

static struct platform_driver this_driver = {
	.probe = hisi_mediacommon_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "hisi_mediacommon_panel",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init hisi_mediacommon_panel_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(hisi_mediacommon_panel_init);
/* lint -restore */

