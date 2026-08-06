/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_mipi_dsi.h"

int mipi_dsi_get_lcd_id(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	return panel_next_get_lcd_id(pdev);
}

int mipi_dsi_panel_switch(struct platform_device *pdev, uint32_t fold_status)
{
	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	return panel_next_panel_switch(pdev, fold_status);
}

struct dpu_panel_info* mipi_dsi_get_panel_info(struct platform_device *pdev, uint32_t panel_id)
{
	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return NULL;
	}

	return panel_next_get_panel_info(pdev, panel_id);
}

ssize_t mipi_dsi_bit_clock_upt_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	uint32_t dsi_bit_clk_upt_tmp = 0;
	int n_str = 0;
	int i = 0;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}
	pinfo = &(dpufd->panel_info);

	for (i = 0; buf[i] != '\0' && buf[i] != '\n'; i++) {
		n_str++;
		if (n_str >= 6) {
			DPU_FB_ERR("invalid parameter: n_str = %d, count = %ld\n",
				n_str, count);
			break;
		}
	}

	if (n_str != 5) {
		DPU_FB_ERR("invalid parameter: n_str = %d, count = %ld\n",
			n_str, count);
		return count;
	}

	if (!dpufd->panel_info.dsi_bit_clk_upt_support) {
		DPU_FB_INFO("fb%d, not support!\n", dpufd->index);
		return count;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (!strncmp(buf, MIPI_DSI_BIT_CLK_STR1, n_str))
		dsi_bit_clk_upt_tmp = pinfo->mipi.dsi_bit_clk_val1;
	else if (!strncmp(buf, MIPI_DSI_BIT_CLK_STR2, n_str))
		dsi_bit_clk_upt_tmp = pinfo->mipi.dsi_bit_clk_val2;
	else if (!strncmp(buf, MIPI_DSI_BIT_CLK_STR3, n_str))
		dsi_bit_clk_upt_tmp = pinfo->mipi.dsi_bit_clk_val3;
	else if (!strncmp(buf, MIPI_DSI_BIT_CLK_STR4, n_str))
		dsi_bit_clk_upt_tmp = pinfo->mipi.dsi_bit_clk_val4;
	else if (!strncmp(buf, MIPI_DSI_BIT_CLK_STR5, n_str))
		dsi_bit_clk_upt_tmp = pinfo->mipi.dsi_bit_clk_val5;
	else
		DPU_FB_ERR("fb%d, unknown dsi_bit_clk_index!\n",
			dpufd->index);

	if (dsi_bit_clk_upt_tmp == 0)
		return count;


	pinfo->mipi.dsi_bit_clk_upt = dsi_bit_clk_upt_tmp;

	panel_next_snd_mipi_clk_cmd_store(pdev, pinfo->mipi.dsi_bit_clk_upt);

	DPU_FB_INFO("switch mipi clk to %d\n", pinfo->mipi.dsi_bit_clk_upt);

	return count;
}

ssize_t mipi_dsi_bit_clock_upt_show(struct platform_device *pdev, char *buf)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	ssize_t ret;

	DPU_FB_DEBUG("+\n");

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}
	pinfo = &(dpufd->panel_info);

	if (!dpufd->panel_info.dsi_bit_clk_upt_support) {
		DPU_FB_INFO("fb%d, panel_info.dsi_bit_clk_upt_support 0!\n",
			dpufd->index);
		return -1;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	ret = snprintf(buf, PAGE_SIZE, "%d", pinfo->mipi.dsi_bit_clk_upt);

	DPU_FB_DEBUG("-\n");

	return ret;
}

ssize_t mipi_dsi_lcd_model_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_model_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_check_reg_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_check_reg(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_mipi_detect_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_mipi_detect(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_hkadc_debug_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_hkadc_debug_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_hkadc_debug_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_hkadc_debug_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_gram_check_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_gram_check_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_gram_check_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_gram_check_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_dynamic_sram_checksum_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_dynamic_sram_checksum_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_dynamic_sram_checksum_store(
	struct platform_device *pdev, const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_dynamic_sram_checksum_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}


ssize_t mipi_dsi_lcd_voltage_enable_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_voltage_enable_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_bist_check(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_bist_check(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_amoled_vr_mode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_amoled_vr_mode_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_amoled_vr_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_amoled_vr_mode_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}
ssize_t mipi_dsi_lcd_acl_ctrl_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_acl_ctrl_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_acl_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer");
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_acl_ctrl_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}


ssize_t mipi_dsi_lcd_sleep_ctrl_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_sleep_ctrl_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_sleep_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_sleep_ctrl_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_test_config_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_test_config_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_test_config_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_test_config_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_reg_read_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_reg_read_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_reg_read_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_reg_read_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_support_mode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_support_mode_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_support_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_support_mode_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_support_checkmode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_support_checkmode_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_lp2hs_mipi_check_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_lp2hs_mipi_check_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_lp2hs_mipi_check_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_lp2hs_mipi_check_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_amoled_pcd_errflag_check(struct platform_device
	*pdev, char *buf)
{
	ssize_t ret;

	ret = panel_next_amoled_pcd_errflag_check(pdev, buf);
	return ret;
}

ssize_t mipi_dsi_lcd_ic_color_enhancement_mode_store(
	struct platform_device *pdev,	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("mipi pdev NULL Pointer");
		return 0;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("mipi dpufd NULL Pointer");
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_ic_color_enhancement_mode_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_lcd_ic_color_enhancement_mode_show(
	struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev NULL Pointer");
		return 0;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd NULL Pointer");
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_lcd_ic_color_enhancement_mode_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_sharpness2d_table_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_sharpness2d_table_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_alpm_setting(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_alpm_setting_store(pdev, buf, count);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_sharpness2d_table_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_sharpness2d_table_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

ssize_t mipi_dsi_panel_info_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	ret = panel_next_panel_info_show(pdev, buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}


