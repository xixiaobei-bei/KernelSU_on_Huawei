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

#include "hisi_mipi_dsi.h"
#include "product/attrs/hisi_mipi_attrs.h"

static int mipi_dsi_remove(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	ret = panel_next_remove(pdev);

	if (need_config_dsi0(dpufd)) {
		if (dpufd->dss_dphy0_ref_clk != NULL) {
			clk_put(dpufd->dss_dphy0_ref_clk);
			dpufd->dss_dphy0_ref_clk = NULL;
		}

		if (dpufd->dss_dphy0_cfg_clk != NULL) {
			clk_put(dpufd->dss_dphy0_cfg_clk);
			dpufd->dss_dphy0_cfg_clk = NULL;
		}
	}

	if (need_config_dsi1(dpufd)) {
		if (dpufd->dss_dphy1_ref_clk != NULL) {
			clk_put(dpufd->dss_dphy1_ref_clk);
			dpufd->dss_dphy1_ref_clk = NULL;
		}

		if (dpufd->dss_dphy1_cfg_clk != NULL) {
			clk_put(dpufd->dss_dphy1_cfg_clk);
			dpufd->dss_dphy1_cfg_clk = NULL;
		}
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static int mipi_dsi_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	ret = panel_next_set_backlight(pdev, bl_level);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static int mipi_dsi_vsync_ctrl(struct platform_device *pdev, int enable)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	ret = panel_next_vsync_ctrl(pdev, enable);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static int mipi_dsi_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	ret = panel_next_lcd_fps_scence_handle(pdev, scence);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static int mipi_dsi_lcd_fps_updt_handle(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +!\n", dpufd->index);

	ret = panel_next_lcd_fps_updt_handle(pdev);

	DPU_FB_DEBUG("fb%d, -!\n", dpufd->index);

	return ret;
}

static int mipi_dsi_esd_handle(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	bool is_timeout = true;
	uint32_t cmp_stopstate_val = 0;
	uint32_t try_times;
	uint32_t temp;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	mipi_dsi0_base = dpufd->mipi_dsi0_base;
	pinfo = &(dpufd->panel_info);

	DPU_FB_DEBUG("fb%d, +!\n", dpufd->index);

	if (pinfo->esd_skip_mipi_check == 1)
		goto panel_check;

	if (dpufd->panel_info.mipi.lane_nums >= DSI_4_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9) | BIT(11));
	else if (dpufd->panel_info.mipi.lane_nums >= DSI_3_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7) | BIT(9));
	else if (dpufd->panel_info.mipi.lane_nums >= DSI_2_LANES)
		cmp_stopstate_val = (BIT(4) | BIT(7));
	else
		cmp_stopstate_val = (BIT(4));

	/* check DPHY data and clock lane stopstate */
	try_times = 0;
	temp = inp32(mipi_dsi0_base + MIPIDSI_PHY_STATUS_OFFSET);
	while ((temp & cmp_stopstate_val) != cmp_stopstate_val) {
		udelay(10);  /* 10us */
		if (++try_times > 100) {  /* 1ms */
			is_timeout = false;
			break;
		}

		temp = inp32(mipi_dsi0_base + MIPIDSI_PHY_STATUS_OFFSET);
	}

	if (is_timeout) {
		DPU_FB_ERR("fb%d, check DPHY data lane status failed! MIPIDSI_PHY_STATUS=0x%x.\n",
			dpufd->index, temp);
		ret = -EINVAL;
		goto error;
	}

panel_check:
	/* disable generate High Speed clock
	 * check panel power status
	 */
	ret = panel_next_esd_handle(pdev);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;

error:
	return ret;
}

static int mipi_dsi_set_display_region(struct platform_device *pdev, struct dss_rect *dirty)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return((!pdev || !dirty), -EINVAL, ERR, "pdev or dirty is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("index=%d, enter!\n", dpufd->index);

	ret = panel_next_set_display_region(pdev, dirty);

	DPU_FB_DEBUG("index=%d, exit!\n", dpufd->index);

	return ret;
}


int dpufb_wait_for_mipi_resource_available(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	(void)dpufd;
	(void)pov_req;

	return 0;
}

static int mipi_dsi_primary_clk_irq_setup(struct dpu_fb_data_type *dpufd, struct platform_device *pdev)
{
	int ret;

	dpufd->dss_dphy0_ref_clk = devm_clk_get(&pdev->dev, dpufd->dss_dphy0_ref_clk_name);
	if (IS_ERR(dpufd->dss_dphy0_ref_clk)) {
		ret = PTR_ERR(dpufd->dss_dphy0_ref_clk);
		return ret;
	}

	ret = clk_set_rate(dpufd->dss_dphy0_ref_clk, DEFAULT_MIPI_CLK_RATE);
	dpu_check_and_return((ret < 0), -EINVAL, ERR,
		"fb%d dss_dphy0_ref_clk clk_set_rate[%lu] failed, error=%d!\n",
		dpufd->index, DEFAULT_MIPI_CLK_RATE, ret);

	DPU_FB_INFO("dss_dphy0_ref_clk:[%lu]->[%lu]\n",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(dpufd->dss_dphy0_ref_clk));

	dpufd->dss_dphy0_cfg_clk = devm_clk_get(&pdev->dev, dpufd->dss_dphy0_cfg_clk_name);
	if (IS_ERR(dpufd->dss_dphy0_cfg_clk)) {
		ret = PTR_ERR(dpufd->dss_dphy0_cfg_clk);
		return ret;
	}

	ret = clk_set_rate(dpufd->dss_dphy0_cfg_clk, DEFAULT_MIPI_CLK_RATE);
	dpu_check_and_return((ret < 0), -EINVAL, ERR,
		"fb%d dss_dphy0_cfg_clk clk_set_rate[%lu] failed, error=%d!\n",
		dpufd->index, DEFAULT_MIPI_CLK_RATE, ret);

	DPU_FB_INFO("dss_dphy0_cfg_clk:[%lu]->[%lu]\n",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(dpufd->dss_dphy0_cfg_clk));

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V320) || \
	defined(CONFIG_HISI_FB_330) || defined(CONFIG_HISI_FB_V501)
	dpufd->dss_pclk_dsi0_clk = devm_clk_get(&pdev->dev, dpufd->dss_pclk_dsi0_name);
	if (IS_ERR(dpufd->dss_pclk_dsi0_clk)) {
		ret = PTR_ERR(dpufd->dss_pclk_dsi0_clk);
		return ret;
	}

	DPU_FB_INFO("dss_pclk_dsi0_clk:[%lu]->[%lu]\n",
		DEFAULT_PCLK_DSI_RATE, clk_get_rate(dpufd->dss_pclk_dsi0_clk));
#endif

	return 0;
}


static int mipi_dsi_external_clk_irq_setup(struct dpu_fb_data_type *dpufd, struct platform_device *pdev)
{
	int ret;

	dpufd->dss_dphy1_ref_clk = devm_clk_get(&pdev->dev, dpufd->dss_dphy1_ref_clk_name);
	if (IS_ERR(dpufd->dss_dphy1_ref_clk)) {
		ret = PTR_ERR(dpufd->dss_dphy1_ref_clk);
		return ret;
	}
	ret = clk_set_rate(dpufd->dss_dphy1_ref_clk, DEFAULT_MIPI_CLK_RATE);
	dpu_check_and_return((ret < 0), -EINVAL, ERR,
		"fb%d dss_dphy1_ref_clk clk_set_rate[%lu] failed, error=%d!\n",
		dpufd->index, DEFAULT_MIPI_CLK_RATE, ret);

	DPU_FB_INFO("dss_dphy1_ref_clk:[%lu]->[%lu]\n",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(dpufd->dss_dphy1_ref_clk));


	dpufd->dss_dphy1_cfg_clk = devm_clk_get(&pdev->dev, dpufd->dss_dphy1_cfg_clk_name);
	if (IS_ERR(dpufd->dss_dphy1_cfg_clk)) {
		ret = PTR_ERR(dpufd->dss_dphy1_cfg_clk);
		return ret;
	}
	ret = clk_set_rate(dpufd->dss_dphy1_cfg_clk, DEFAULT_MIPI_CLK_RATE);
	dpu_check_and_return((ret < 0), -EINVAL, ERR,
		"fb%d dss_dphy1_cfg_clk clk_set_rate[%lu] failed, error=%d!\n",
		dpufd->index, DEFAULT_MIPI_CLK_RATE, ret);

	DPU_FB_INFO("dss_dphy1_cfg_clk:[%lu]->[%lu]\n",
		DEFAULT_MIPI_CLK_RATE, clk_get_rate(dpufd->dss_dphy1_cfg_clk));

#if !defined(CONFIG_HISI_FB_V510) && !defined(CONFIG_HISI_FB_V600) && \
	!defined(CONFIG_HISI_FB_V360)
	dpufd->dss_pclk_dsi1_clk = devm_clk_get(&pdev->dev, dpufd->dss_pclk_dsi1_name);
	if (IS_ERR(dpufd->dss_pclk_dsi1_clk)) {
		ret = PTR_ERR(dpufd->dss_pclk_dsi1_clk);
		return ret;
	}

	DPU_FB_INFO("dss_pclk_dsi1_clk:[%lu]->[%lu]\n",
		DEFAULT_PCLK_DSI_RATE, clk_get_rate(dpufd->dss_pclk_dsi1_clk));
#endif

	return 0;
}

static int mipi_dsi_clk_irq_setup(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int ret = 0;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	if (need_config_dsi0(dpufd) ||
		(dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE)) {
		ret = mipi_dsi_primary_clk_irq_setup(dpufd, pdev);
		if (ret)
			return ret;
	}


	if (need_config_dsi1(dpufd) ||
		(dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE)) {
		ret = mipi_dsi_external_clk_irq_setup(dpufd, pdev);
		if (ret)
			return ret;
	}

	return ret;
}

static void mipi_dsi_pdata_registe_cb(struct dpu_fb_panel_data *pdata)
{
	pdata->set_fastboot = mipi_dsi_set_fastboot;
	pdata->on = mipi_dsi_on;
	pdata->off = mipi_dsi_off;
	pdata->remove = mipi_dsi_remove;
	pdata->set_backlight = mipi_dsi_set_backlight;
	pdata->vsync_ctrl = mipi_dsi_vsync_ctrl;
	pdata->lcd_fps_scence_handle = mipi_dsi_lcd_fps_scence_handle;
	pdata->lcd_fps_updt_handle = mipi_dsi_lcd_fps_updt_handle;
	pdata->esd_handle = mipi_dsi_esd_handle;
	pdata->set_display_region = mipi_dsi_set_display_region;
#if defined(CONFIG_DEVICE_ATTRS)
	pdata->mipi_dsi_bit_clk_upt_store = mipi_dsi_bit_clock_upt_store;
	pdata->mipi_dsi_bit_clk_upt_show = mipi_dsi_bit_clock_upt_show;
	pdata->lcd_model_show = mipi_dsi_lcd_model_show;
	pdata->lcd_check_reg = mipi_dsi_lcd_check_reg_show;
	pdata->lcd_mipi_detect = mipi_dsi_lcd_mipi_detect_show;
	pdata->lcd_hkadc_debug_show = mipi_dsi_lcd_hkadc_debug_show;
	pdata->lcd_hkadc_debug_store = mipi_dsi_lcd_hkadc_debug_store;
	pdata->lcd_gram_check_show = mipi_dsi_lcd_gram_check_show;
	pdata->lcd_gram_check_store = mipi_dsi_lcd_gram_check_store;
	pdata->lcd_dynamic_sram_checksum_show = mipi_dsi_lcd_dynamic_sram_checksum_show;
	pdata->lcd_dynamic_sram_checksum_store = mipi_dsi_lcd_dynamic_sram_checksum_store;
	pdata->lcd_voltage_enable_store = mipi_dsi_lcd_voltage_enable_store;
	pdata->lcd_bist_check = mipi_dsi_lcd_bist_check;
	pdata->lcd_sleep_ctrl_show = mipi_dsi_lcd_sleep_ctrl_show;
	pdata->lcd_sleep_ctrl_store = mipi_dsi_lcd_sleep_ctrl_store;
	pdata->lcd_acl_ctrl_show = mipi_dsi_lcd_acl_ctrl_show;
	pdata->lcd_acl_ctrl_store = mipi_dsi_lcd_acl_ctrl_store;
	pdata->lcd_amoled_vr_mode_show = mipi_dsi_lcd_amoled_vr_mode_show;
	pdata->lcd_amoled_vr_mode_store = mipi_dsi_lcd_amoled_vr_mode_store;
	pdata->lcd_test_config_show = mipi_dsi_lcd_test_config_show;
	pdata->lcd_test_config_store = mipi_dsi_lcd_test_config_store;
	pdata->lcd_reg_read_show = mipi_dsi_lcd_reg_read_show;
	pdata->lcd_reg_read_store = mipi_dsi_lcd_reg_read_store;
	pdata->lcd_support_mode_show = mipi_dsi_lcd_support_mode_show;
	pdata->lcd_support_mode_store = mipi_dsi_lcd_support_mode_store;
	pdata->lcd_support_checkmode_show = mipi_dsi_lcd_support_checkmode_show;
	pdata->lcd_lp2hs_mipi_check_show = mipi_dsi_lcd_lp2hs_mipi_check_show;
	pdata->lcd_lp2hs_mipi_check_store = mipi_dsi_lcd_lp2hs_mipi_check_store;
	pdata->amoled_pcd_errflag_check = mipi_dsi_amoled_pcd_errflag_check;
	pdata->lcd_ic_color_enhancement_mode_show = mipi_dsi_lcd_ic_color_enhancement_mode_show;
	pdata->lcd_ic_color_enhancement_mode_store = mipi_dsi_lcd_ic_color_enhancement_mode_store;
	pdata->sharpness2d_table_store = mipi_dsi_sharpness2d_table_store;
	pdata->sharpness2d_table_show = mipi_dsi_sharpness2d_table_show;
	pdata->panel_info_show = mipi_dsi_panel_info_show;
	pdata->get_lcd_id = mipi_dsi_get_lcd_id;
	pdata->amoled_alpm_setting_store = mipi_dsi_alpm_setting;
	pdata->panel_switch = mipi_dsi_panel_switch;
	pdata->get_panel_info = mipi_dsi_get_panel_info;
#endif

}

static int mipi_dsi_probe(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct platform_device *dpp_dev = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	int ret;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	dpufd = platform_get_drvdata(pdev);
	dev_check_and_return(&pdev->dev, !dpufd, -EINVAL, err, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	mipi_transfer_lock_init();

	ret = mipi_dsi_clk_irq_setup(pdev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d mipi_dsi_irq_clk_setup failed, error=%d!\n", dpufd->index, ret);
		goto err;
	}

	/* alloc device */
	dpp_dev = platform_device_alloc(DEV_NAME_DSS_DPE, pdev->id);
	if (!dpp_dev) {
		dev_err(&pdev->dev, "fb%d platform_device_alloc failed, error=%d!\n", dpufd->index, ret);
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	/* link to the latest pdev */
	dpufd->pdev = dpp_dev;

	/* alloc panel device data */
	ret = platform_device_add_data(dpp_dev, dev_get_platdata(&pdev->dev),
		sizeof(struct dpu_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add_data failed error=%d!\n", dpufd->index, ret);
		goto err_device_put;
	}

	/* data chain */
	pdata = dev_get_platdata(&dpp_dev->dev);
	pdata->lp_ctrl = NULL;
	mipi_dsi_pdata_registe_cb(pdata);
	pdata->next = pdev;

	/* get/set panel info */
	memcpy(&dpufd->panel_info, pdata->panel_info, sizeof(dpufd->panel_info));

	/* set driver data */
	platform_set_drvdata(dpp_dev, dpufd);
	/* device add */
	ret = platform_device_add(dpp_dev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add failed, error=%d!\n", dpufd->index, ret);
		goto err_device_put;
	}


	if (dpufd->panel_info.delayed_cmd_queue_support)
		mipi_dsi_init_delayed_cmd_queue();

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;

err_device_put:
	platform_device_put(dpp_dev);
	dpufd->pdev = NULL;
err_device_alloc:
err:
	return ret;
}

static struct platform_driver this_driver = {
	.probe = mipi_dsi_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_MIPIDSI,
	},
};

static int __init mipi_dsi_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		DPU_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(mipi_dsi_driver_init);

