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

#include "hisi_fb.h"
#include "hisi_fb_panel.h"
#include "../../panel/mipi_lcd_utils.h"

int panel_next_get_lcd_id(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->get_lcd_id))
			ret = next_pdata->get_lcd_id(next_pdev);
	}
	return ret;
}

int panel_next_panel_switch(struct platform_device *pdev, uint32_t fold_status)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->panel_switch))
			ret = next_pdata->panel_switch(next_pdev, fold_status);
	}
	return ret;
}

struct dpu_panel_info* panel_next_get_panel_info(struct platform_device *pdev, uint32_t panel_id)
{
	struct dpu_panel_info* panel_info = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("[folder]pdev is NULL");
		return NULL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[folder]pdata is NULL");
		return NULL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->get_panel_info))
			panel_info = next_pdata->get_panel_info(next_pdev, panel_id);
	}
	return panel_info;
}

ssize_t panel_next_snd_mipi_clk_cmd_store(struct platform_device *pdev,
	uint32_t clk_val)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->snd_mipi_clk_cmd_store))
			ret = next_pdata->snd_mipi_clk_cmd_store(
					next_pdev, clk_val);
	}

	return ret;
}


ssize_t panel_next_lcd_model_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_model_show))
			ret = next_pdata->lcd_model_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_cabc_mode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_cabc_mode_show))
			ret = next_pdata->lcd_cabc_mode_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_cabc_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_cabc_mode_store))
			ret = next_pdata->lcd_cabc_mode_store(
					next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_check_reg(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = -1;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_check_reg))
			ret = next_pdata->lcd_check_reg(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_mipi_detect(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = -1;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_mipi_detect))
			ret = next_pdata->lcd_mipi_detect(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_mipi_dsi_bit_clk_upt_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->mipi_dsi_bit_clk_upt_store))
			ret = next_pdata->mipi_dsi_bit_clk_upt_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_support_checkmode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_support_checkmode_show))
			ret = next_pdata->lcd_support_checkmode_show(
				next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_mipi_dsi_bit_clk_upt_show(
	struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->mipi_dsi_bit_clk_upt_show))
			ret = next_pdata->mipi_dsi_bit_clk_upt_show(
					next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_hkadc_debug_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hkadc_debug_show))
			ret = next_pdata->lcd_hkadc_debug_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_hkadc_debug_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hkadc_debug_store))
			ret = next_pdata->lcd_hkadc_debug_store(
				next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_gram_check_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_gram_check_show))
			ret = next_pdata->lcd_gram_check_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_gram_check_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_gram_check_store))
			ret = next_pdata->lcd_gram_check_store(
				next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_dynamic_sram_checksum_show(
	struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) &&
			(next_pdata->lcd_dynamic_sram_checksum_show))
			ret = next_pdata->lcd_dynamic_sram_checksum_show(
				next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_dynamic_sram_checksum_store(
	struct platform_device *pdev, const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) &&
			(next_pdata->lcd_dynamic_sram_checksum_store))
			ret = next_pdata->lcd_dynamic_sram_checksum_store(
				next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_ic_color_enhancement_mode_show(
	struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) &&
			(next_pdata->lcd_ic_color_enhancement_mode_show))
			ret = next_pdata->lcd_ic_color_enhancement_mode_show(
				next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_ic_color_enhancement_mode_store(
	struct platform_device *pdev, const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) &&
			(next_pdata->lcd_ic_color_enhancement_mode_store))
			ret = next_pdata->lcd_ic_color_enhancement_mode_store(
				next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_voltage_enable_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_voltage_enable_store))
			ret = next_pdata->lcd_voltage_enable_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_bist_check(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_bist_check))
			ret = next_pdata->lcd_bist_check(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_sleep_ctrl_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_sleep_ctrl_show))
			ret = next_pdata->lcd_sleep_ctrl_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_sleep_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_sleep_ctrl_store))
			ret = next_pdata->lcd_sleep_ctrl_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_test_config_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_test_config_show))
			ret = next_pdata->lcd_test_config_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_test_config_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_test_config_store))
			ret = next_pdata->lcd_test_config_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_reg_read_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_reg_read_show))
			ret = next_pdata->lcd_reg_read_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_reg_read_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_reg_read_store))
			ret = next_pdata->lcd_reg_read_store(
					next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_support_mode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_support_mode_show))
			ret = next_pdata->lcd_support_mode_show(
					next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_support_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_support_mode_store))
			ret = next_pdata->lcd_support_mode_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_lp2hs_mipi_check_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_lp2hs_mipi_check_show))
			ret = next_pdata->lcd_lp2hs_mipi_check_show(
					next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_lp2hs_mipi_check_store(
	struct platform_device *pdev, const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_lp2hs_mipi_check_store))
			ret = next_pdata->lcd_lp2hs_mipi_check_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_amoled_pcd_errflag_check(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->amoled_pcd_errflag_check))
			ret = next_pdata->amoled_pcd_errflag_check(
					next_pdev, buf);
	}

	return ret;
}
ssize_t panel_next_lcd_hbm_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hbm_ctrl_store))
			ret = next_pdata->lcd_hbm_ctrl_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_hbm_ctrl_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_hbm_ctrl_show))
			ret = next_pdata->lcd_hbm_ctrl_show(next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_amoled_vr_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_amoled_vr_mode_store))
			ret = next_pdata->lcd_amoled_vr_mode_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_amoled_vr_mode_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_amoled_vr_mode_show))
			ret = next_pdata->lcd_amoled_vr_mode_show(
					next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_lcd_acl_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_acl_ctrl_store))
			ret = next_pdata->lcd_acl_ctrl_store(
					next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_alpm_setting_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->amoled_alpm_setting_store))
			ret = next_pdata->amoled_alpm_setting_store(next_pdev,
				buf, count);
	}

	return ret;
}

ssize_t panel_next_lcd_acl_ctrl_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_acl_ctrl_show))
			ret = next_pdata->lcd_acl_ctrl_show(next_pdev, buf);
	}

	return ret;
}


ssize_t panel_next_sharpness2d_table_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->sharpness2d_table_store))
			ret = next_pdata->sharpness2d_table_store(
					next_pdev, buf, count);
	}

	return ret;
}

ssize_t panel_next_sharpness2d_table_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->sharpness2d_table_show))
			ret = next_pdata->sharpness2d_table_show(
					next_pdev, buf);
	}

	return ret;
}

ssize_t panel_next_panel_info_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->panel_info_show))
			ret = next_pdata->panel_info_show(next_pdev, buf);
	}

	return ret;
}


