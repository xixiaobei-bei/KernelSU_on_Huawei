/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include <linux/device.h>

#define DTS_COMP_HISI_MDC_PANEL	"hisilicon,mediacommon_panel"

#ifdef CONFIG_HISI_FB_970
static int hisi_mediacommon_panel_on(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is null!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is null!\n");

	if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		outp32(dpufd->media_crg_base + MEDIA_CLKDIV9, 0x08000800);
		outp32(dpufd->media_crg_base + MEDIA_PEREN0, 0x20000000);
		outp32(dpufd->media_crg_base + MEDIA_PEREN1, 0x01801000);
		outp32(dpufd->media_crg_base + MEDIA_PEREN1, 0x00006000);

		outp32(dpufd->media_crg_base + MEDIA_PERDIS0, 0x20000000);
		outp32(dpufd->media_crg_base + MEDIA_PERDIS1, 0x01806000);
		outp32(dpufd->media_crg_base + MEDIA_PERRSTDIS0, 0x001c4000);

		outp32(dpufd->media_crg_base + MEDIA_PEREN0, 0x20000000);
		outp32(dpufd->media_crg_base + MEDIA_PEREN1, 0x01806000);
		outp32(dpufd->media_crg_base + MEDIA_CLKDIV8, 0xffc02180);
	}
	return 0;
}

static int hisi_mediacommon_panel_off(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is null!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is null!\n");

	if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		outp32(dpufd->media_crg_base + MEDIA_PERRSTEN0, 0x001c4000);
		outp32(dpufd->media_crg_base + MEDIA_PERDIS0, 0x20000000);
		outp32(dpufd->media_crg_base + MEDIA_PERDIS1, 0x01806000);
		outp32(dpufd->media_crg_base + MEDIA_PERDIS1, 0x00001000);
		outp32(dpufd->media_crg_base + MEDIA_CLKDIV9, 0x08000000);
	}
	return 0;
}
#else
static int hisi_mediacommon_panel_on(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is null!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is null!\n");

	if (mdc_clk_regulator_enable(dpufd)) {
		DPU_FB_ERR("mdc clk regulator enable failed");
		return -EINVAL;
	}

	hisi_mdc_interrupt_config(dpufd);
	if (!dpufd->dss_module_resource_initialized) {
		hisi_mdc_module_default(dpufd);
		dpufd->dss_module_resource_initialized = true;
	}
	hisi_mdc_mif_on(dpufd);
	hisi_mdc_smmu_on(dpufd);
	hisi_mdc_scl_coef_on(dpufd, SCL_COEF_YUV_IDX);

	hisi_mdc_mctl_on(dpufd);

	return 0;
}

static int hisi_mediacommon_panel_off(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is null!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is null!\n");

	mdc_clk_regulator_disable(dpufd);

	return ret;
}
#endif

static int hisi_mediacommon_panel_remove(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is null!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is null!\n");

	return 0;
}

static struct dpu_panel_info hisi_mdc_panel_info = {0};
static struct dpu_fb_panel_data hisi_mdc_panel_data = {
	.panel_info = &hisi_mdc_panel_info,
	.on = hisi_mediacommon_panel_on,
	.off = hisi_mediacommon_panel_off,
	.remove = hisi_mediacommon_panel_remove,
};

static int hisi_mdc_probe(struct platform_device *pdev)
{
	int ret;
	struct dpu_panel_info *pinfo = NULL;
	struct platform_device *this_dev = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (hisi_fb_device_probe_defer(PANEL_MEDIACOMMON, BL_SET_BY_NONE))
		goto err_probe_defer;

	if (!pdev)
		goto err_probe_defer;

	pdev->id = 1; /*lint !e613*/
	pinfo = hisi_mdc_panel_data.panel_info;
	pinfo->xres = g_primary_lcd_xres;
	pinfo->yres = g_primary_lcd_yres;
	pinfo->type = PANEL_MEDIACOMMON;
	pinfo->pxl_clk_rate = g_pxl_clk_rate;

	/* alloc panel device data */
	ret = platform_device_add_data(pdev, &hisi_mdc_panel_data,
		sizeof(struct dpu_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "platform_device_add_data failed!\n"); /*lint !e613*/
		goto err_device_put;
	}

	this_dev = hisi_fb_add_device(pdev);
	if (this_dev != NULL)
		dpufd = platform_get_drvdata(this_dev);

	if (dpufd) {
		dpufd->dss_aclk_media_common_clk = devm_clk_get(&pdev->dev, "aclk_media_common");
		if (IS_ERR(dpufd->dss_aclk_media_common_clk)) {
			ret = PTR_ERR(dpufd->dss_aclk_media_common_clk);
			DPU_FB_ERR("dss_aclk_media_common_clk error, ret=%d", ret);
		}

		dpufd->dss_clk_media_common_clk = devm_clk_get(&pdev->dev, "clk_media_common");
		if (IS_ERR(dpufd->dss_clk_media_common_clk)) {
			ret = PTR_ERR(dpufd->dss_clk_media_common_clk);
			DPU_FB_ERR("dss_clk_media_common_clk error, ret=%d", ret);
		}

		dpufd->mdc_regulator = devm_regulator_get(&pdev->dev, "regulator_mdc");
		if (IS_ERR(dpufd->mdc_regulator)) {
			ret = PTR_ERR(dpufd->mdc_regulator);
			DPU_FB_ERR("mdc_regulator error, ret=%d", ret);
		}
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;

err_probe_defer:
	return -EPROBE_DEFER;
}

static const struct of_device_id hisi_device_match_table[] = {
	{
		.compatible = DTS_COMP_HISI_MDC_PANEL,
		.data = NULL,
	},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_device_match_table);

static struct platform_driver this_driver = {
	.probe = hisi_mdc_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "hisi_mediacommon_panel",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_device_match_table),
	},
};

static int __init hisi_mdc_panel_init(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);
	if (ret)
		pr_err("register mediacommon driver fail\n");

	return ret;
}

module_init(hisi_mdc_panel_init);

