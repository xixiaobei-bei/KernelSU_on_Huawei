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
#ifndef HISI_DPE_ATTRS_H
#define HISI_DPE_ATTRS_H

#include "hisi_fb.h"

int dpe_get_lcd_id(struct platform_device *pdev);
ssize_t dpe_lcd_model_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_check_reg_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_mipi_detect_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_hkadc_debug_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_hkadc_debug_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_gram_check_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_gram_check_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_dynamic_sram_checksum_show(struct platform_device *pdev,
	char *buf);
ssize_t dpe_lcd_dynamic_sram_checksum_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_color_temperature_show(struct platform_device *pdev,
char *buf);
ssize_t dpe_lcd_color_temperature_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_ic_color_enhancement_mode_show(struct platform_device *pdev,
	char *buf);
ssize_t dpe_lcd_ic_color_enhancement_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_led_rg_lcd_color_temperature_show(struct platform_device *pdev,
	char *buf);
ssize_t dpe_led_rg_lcd_color_temperature_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_comform_mode_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_comform_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_cinema_mode_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_cinema_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_support_mode_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_support_mode_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_voltage_enable_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_bist_check(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_sleep_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_sleep_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_test_config_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_test_config_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_reg_read_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_reg_read_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_support_checkmode_show(struct platform_device *pdev,
char *buf);
ssize_t dpe_lcd_lp2hs_mipi_check_show(struct platform_device *pdev,
char *buf);
ssize_t dpe_lcd_lp2hs_mipi_check_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_amoled_pcd_errflag_check(struct platform_device *pdev,
char *buf);
ssize_t dpe_lcd_hbm_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_hbm_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t dpe_lcd_acl_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_acl_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t dpe_acm_state_show(struct platform_device *pdev, char *buf);
ssize_t dpe_acm_state_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_gmp_state_show(struct platform_device *pdev, char *buf);
ssize_t dpe_gmp_state_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_alpm_setting_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_sharpness2d_table_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_sharpness2d_table_show(struct platform_device *pdev,
char *buf);
ssize_t dpe_panel_info_show(struct platform_device *pdev, char *buf);
ssize_t dpe_mipi_dsi_bit_clk_upt_show(struct platform_device *pdev,
char *buf);
ssize_t dpe_mipi_dsi_bit_clk_upt_store(struct platform_device *pdev,
	const char *buf, size_t count);
ssize_t dpe_lcd_xcc_store(struct platform_device *pdev,
	const char *buf, size_t count);

#endif /* HISI_DPE_ATTRS_H */
