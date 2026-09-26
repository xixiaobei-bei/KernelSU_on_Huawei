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
#ifndef HISI_PANEL_ATTRS_H
#define HISI_PANEL_ATTRS_H

#include "hisi_fb_def.h"
#include "hisi_mipi_dsi.h"
#include "hisi_dss.h"

int panel_next_get_lcd_id(struct platform_device *pdev);
int panel_next_panel_switch(struct platform_device *pdev, uint32_t fold_status);
struct dpu_panel_info* panel_next_get_panel_info(struct platform_device *pdev, uint32_t panel_id);
ssize_t panel_next_snd_mipi_clk_cmd_store(struct platform_device *pdev,
	uint32_t clk_val);
ssize_t panel_next_lcd_model_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_cabc_mode_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_cabc_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_check_reg(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_mipi_detect(struct platform_device *pdev, char *buf);
ssize_t panel_next_mipi_dsi_bit_clk_upt_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_mipi_dsi_bit_clk_upt_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_hkadc_debug_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_hkadc_debug_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_gram_check_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_gram_check_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_dynamic_sram_checksum_show(
	struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_dynamic_sram_checksum_store(
	struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_voltage_enable_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_bist_check(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_support_checkmode_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_panel_info_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_sleep_ctrl_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_sleep_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_test_config_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_test_config_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_reg_read_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_reg_read_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_support_mode_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_support_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_lp2hs_mipi_check_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_lp2hs_mipi_check_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_amoled_pcd_errflag_check(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_hbm_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_hbm_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_acl_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_acl_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_sharpness2d_table_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_sharpness2d_table_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_lcd_ic_color_enhancement_mode_store(
	struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_ic_color_enhancement_mode_show(
	struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_amoled_vr_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t panel_next_lcd_amoled_vr_mode_show(struct platform_device *pdev,
	char *buf);
ssize_t panel_next_alpm_setting_store(struct platform_device *pdev,
	const char *buf, size_t count);

#endif /* HISI_PANEL_ATTRS_H */
