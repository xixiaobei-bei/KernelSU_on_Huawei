/* Copyright (c) 2013-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#ifndef HISI_FB_PANEL_STRUCT_EX_EX_H
#define HISI_FB_PANEL_STRUCT_EX_EX_H

#include "hisi_fb_panel_enum.h"
#include "hisi_fb_panel_struct_ex.h"

typedef struct demura_set_info {
	uint8_t *data0;
	uint8_t len0;
	uint8_t *data1;
	uint8_t len1;
} demura_set_info_t;

struct dpu_fb_panel_data {
	struct dpu_panel_info *panel_info;

	/* function entry chain */
	int (*set_fastboot)(struct platform_device *pdev);
	int (*on)(struct platform_device *pdev);
	int (*off)(struct platform_device *pdev);
	int (*lp_ctrl)(struct platform_device *pdev, bool lp_enter);
	int (*remove)(struct platform_device *pdev);
	int (*set_backlight)(struct platform_device *pdev, uint32_t bl_level);
	int (*lcd_set_backlight_by_type_func)(struct platform_device *pdev, int backlight_type);
	int (*lcd_set_hbm_for_screenon)(struct platform_device *pdev, int bl_type);
	int (*lcd_set_hbm_for_mmi_func)(struct platform_device *pdev, int level);
	int (*set_blc_brightness)(struct platform_device *pdev, uint32_t bl_level);
	int (*vsync_ctrl)(struct platform_device *pdev, int enable);
	int (*lcd_fps_scence_handle)(struct platform_device *pdev, uint32_t scence);
	int (*lcd_fps_updt_handle)(struct platform_device *pdev);
	void (*snd_cmd_before_frame)(struct platform_device *pdev);
	int (*esd_handle)(struct platform_device *pdev);
	int (*set_display_region)(struct platform_device *pdev, struct dss_rect *dirty);
	int (*set_pixclk_rate)(struct platform_device *pdev);
	int (*set_display_resolution)(struct platform_device *pdev);
	int (*get_lcd_id)(struct platform_device *pdev);
	int (*panel_bypass_powerdown_ulps_support)(struct platform_device *pdev);
	int (*set_tcon_mode)(struct platform_device *pdev, uint8_t mode);
	void (*get_spr_para_list)(uint32_t *gma, uint32_t *degma);
	int (*panel_switch)(struct platform_device *pdev, uint32_t fold_status);
	struct dpu_panel_info *(*get_panel_info)(struct platform_device *pdev, uint32_t panel_id);

	ssize_t (*snd_mipi_clk_cmd_store)(struct platform_device *pdev, uint32_t clk_val);
	ssize_t (*lcd_model_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_cabc_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_cabc_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_rgbw_set_func)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_hbm_set_func)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_set_ic_dim_on)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_color_param_get_func)(struct dpu_fb_data_type *dpufd);
	ssize_t (*lcd_ce_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_ce_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_check_reg)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_mipi_detect)(struct platform_device *pdev, char *buf);
	ssize_t (*mipi_dsi_bit_clk_upt_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*mipi_dsi_bit_clk_upt_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_hkadc_debug_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_hkadc_debug_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_gram_check_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_gram_check_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_dynamic_sram_checksum_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_dynamic_sram_checksum_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_color_temperature_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_color_temperature_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_ic_color_enhancement_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_ic_color_enhancement_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*led_rg_lcd_color_temperature_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*led_rg_lcd_color_temperature_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_comform_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_comform_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_cinema_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_cinema_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_support_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_support_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_voltage_enable_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_bist_check)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_sleep_ctrl_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_sleep_ctrl_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_test_config_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_test_config_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_reg_read_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_reg_read_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_support_checkmode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_lp2hs_mipi_check_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_lp2hs_mipi_check_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_inversion_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_inversion_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_scan_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_scan_show)(struct platform_device *pdev, char *buf);
	ssize_t (*amoled_pcd_errflag_check)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_hbm_ctrl_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_hbm_ctrl_show)(struct platform_device *pdev, char *buf);
	ssize_t (*sharpness2d_table_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*sharpness2d_table_show)(struct platform_device *pdev, char *buf);
	ssize_t (*panel_info_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_acm_state_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_acm_state_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_acl_ctrl_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_acl_ctrl_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_gmp_state_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_gmp_state_show)(struct platform_device *pdev, char *buf);
	ssize_t (*lcd_amoled_vr_mode_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_amoled_vr_mode_show)(struct platform_device *pdev, char *buf);
	ssize_t (*amoled_alpm_setting_store)(struct platform_device *pdev, const char *buf, size_t count);
	ssize_t (*lcd_xcc_store)(struct platform_device *pdev, const char *buf, size_t count);
	int (*lcd_get_demura)(struct platform_device *pdev, uint8_t dsi,
		 uint8_t *out, int out_len,  uint8_t type, uint8_t len);
	int (*lcd_set_demura)(struct platform_device *pdev, uint8_t type, const demura_set_info_t *info);
	struct platform_device *next;
};

#if defined(CONFIG_HUAWEI_DSM)
struct lcd_reg_read_t {
	u8 reg_addr; /* register address */
	u32 expected_value; /* the expected value should returned when lcd is in good condtion */
	u32 read_mask; /* set read mask if there are bits should ignored */
	char *reg_name; /* register name */
	bool for_debug; /* for debug */
	u8 recovery; /* if need recovery */
};
#endif

struct panel_aging_time_info {
	s64 start_time[EN_AGING_PANEL_NUM];
	s64 duration_time[EN_AGING_PANEL_NUM];
	uint32_t time_state[EN_AGING_PANEL_NUM];
	spinlock_t time_lock;
	bool hiace_enable;

	uint32_t fold_count;
	spinlock_t count_lock;
	uint8_t record_region;
};

struct display_engine_duration {
	uint32_t *dbv_acc;
	uint32_t *original_dbv_acc;
	uint32_t *screen_on_duration;
	uint32_t *hbm_acc;
	uint32_t *hbm_duration;
	uint32_t size_dbv_acc;
	uint32_t size_original_dbv_acc;
	uint32_t size_screen_on_duration;
	uint32_t size_hbm_acc;
	uint32_t size_hbm_duration;
};

#endif /* HISI_FB_PANEL_STRUCT_EX_EX_H */