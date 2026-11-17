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
#ifndef HISI_FB_PANEL_H
#define HISI_FB_PANEL_H

#include "hisi_fb_panel_struct.h"

/* record lcd display fps support value
 */
enum {
	DISPLAY_FPS_DEFAULT = 0,
	DISPLAY_FPS_90HZ = 1,
	DISPLAY_FPS_120HZ = 2,
	DISPLAY_FPS_MAX,
};

/*
 * FUNCTIONS PROTOTYPES
 */

extern uint32_t g_dts_resouce_ready;
extern mipi_ifbc_division_t g_mipi_ifbc_division[MIPI_DPHY_NUM][IFBC_TYPE_MAX];

int resource_cmds_tx(struct platform_device *pdev,
	struct resource_desc *cmds, int cnt);
int vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt);
int pinctrl_cmds_tx(struct platform_device *pdev, struct pinctrl_cmd_desc *cmds, int cnt);
int gpio_cmds_tx(struct gpio_desc *cmds, int cnt);

int panel_next_set_fastboot(struct platform_device *pdev);
int panel_next_on(struct platform_device *pdev);
int panel_next_off(struct platform_device *pdev);
int panel_next_lp_ctrl(struct platform_device *pdev, bool lp_enter);
int panel_next_remove(struct platform_device *pdev);
int panel_next_set_backlight(struct platform_device *pdev, uint32_t bl_level);
int panel_next_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence);
int panel_next_lcd_fps_updt_handle(struct platform_device *pdev);
int panel_next_vsync_ctrl(struct platform_device *pdev, int enable);
int panel_next_esd_handle(struct platform_device *pdev);
int panel_next_set_display_region(struct platform_device *pdev, struct dss_rect *dirty);
int panel_next_get_lcd_id(struct platform_device *pdev);
int panel_next_bypass_powerdown_ulps_support(struct platform_device *pdev);
int panel_next_panel_switch(struct platform_device *pdev, uint32_t fold_status);
struct dpu_panel_info *panel_next_get_panel_info(struct platform_device *pdev, uint32_t panel_id);

ssize_t panel_next_snd_mipi_clk_cmd_store(struct platform_device *pdev, uint32_t clk_val);
ssize_t panel_next_lcd_model_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_cabc_mode_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_cabc_mode_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_check_reg(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_mipi_detect(struct platform_device *pdev, char *buf);
ssize_t panel_next_mipi_dsi_bit_clk_upt_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_mipi_dsi_bit_clk_upt_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_hkadc_debug_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_hkadc_debug_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_gram_check_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_gram_check_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_dynamic_sram_checksum_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_dynamic_sram_checksum_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_voltage_enable_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_bist_check(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_support_checkmode_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_panel_info_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_sleep_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_sleep_ctrl_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_test_config_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_test_config_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_reg_read_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_reg_read_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_support_mode_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_support_mode_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_lp2hs_mipi_check_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_lp2hs_mipi_check_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_amoled_pcd_errflag_check(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_hbm_ctrl_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_hbm_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_acl_ctrl_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_acl_ctrl_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_sharpness2d_table_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_sharpness2d_table_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_ic_color_enhancement_mode_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_ic_color_enhancement_mode_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_lcd_amoled_vr_mode_store(struct platform_device *pdev, const char *buf, size_t count);
ssize_t panel_next_lcd_amoled_vr_mode_show(struct platform_device *pdev, char *buf);
ssize_t panel_next_alpm_setting_store(struct platform_device *pdev,	const char *buf, size_t count);

void hisi_blpwm_fill_light(uint32_t backlight);
int hisi_pwm_set_backlight(struct dpu_fb_data_type *dpufd, uint32_t bl_level);
int hisi_pwm_off(struct platform_device *pdev);
int hisi_pwm_on(struct platform_device *pdev);

int hisi_blpwm_set_backlight(struct dpu_fb_data_type *dpufd, uint32_t bl_level);

void hisi_blpwm_bl_regisiter(int (*set_bl)(int bl_level));

int hisi_blpwm_off(struct platform_device *pdev);
int hisi_blpwm_on(struct platform_device *pdev);

int hisi_sh_blpwm_set_backlight(struct dpu_fb_data_type *dpufd, uint32_t bl_level);
int hisi_sh_blpwm_off(struct platform_device *pdev);
int hisi_sh_blpwm_on(struct platform_device *pdev);

int hisi_lcd_backlight_on(struct platform_device *pdev);
int hisi_lcd_backlight_off(struct platform_device *pdev);

bool is_lcd_dfr_support(struct dpu_fb_data_type *dpufd);
bool is_ldi_panel(struct dpu_fb_data_type *dpufd);
bool is_mipi_cmd_panel(struct dpu_fb_data_type *dpufd);
bool is_mipi_cmd_panel_ext(struct dpu_panel_info *pinfo);
bool is_mipi_video_panel(struct dpu_fb_data_type *dpufd);
bool is_dp_panel(struct dpu_fb_data_type *dpufd);
bool is_hisync_mode(struct dpu_fb_data_type *dpufd);
bool is_video_idle_ctrl_mode(struct dpu_fb_data_type *dpufd);
bool is_mipi_panel(struct dpu_fb_data_type *dpufd);
bool is_dual_mipi_panel(struct dpu_fb_data_type *dpufd);
bool need_config_dsi0(struct dpu_fb_data_type *dpufd);
bool need_config_dsi1(struct dpu_fb_data_type *dpufd);
bool is_dual_mipi_panel_ext(struct dpu_panel_info *pinfo);
bool is_ifbc_panel(struct dpu_fb_data_type *dpufd);
bool is_ifbc_vesa_panel(struct dpu_fb_data_type *dpufd);
bool is_single_slice_dsc_panel(struct dpu_fb_data_type *dpufd);
bool is_dsi0_pipe_switch_connector(struct dpu_fb_data_type *dpufd);
bool is_dsi1_pipe_switch_connector(struct dpu_fb_data_type *dpufd);
ssize_t panel_mode_switch_store(struct dpu_fb_data_type *dpufd, const char *buf, size_t count);
void panel_mode_switch_isr_handler(struct dpu_fb_data_type *dpufd, uint8_t mode_switch_to);
bool mipi_panel_check_reg(struct dpu_fb_data_type *dpufd, uint32_t *read_value, int buf_len);
int mipi_ifbc_get_rect(struct dpu_fb_data_type *dpufd, struct dss_rect *rect);
bool is_hisi_writeback_panel(struct dpu_fb_data_type *dpufd);
void dpufb_snd_cmd_before_frame(struct dpu_fb_data_type *dpufd);

void hisi_fb_device_set_status0(uint32_t status);
int hisi_fb_device_set_status1(struct dpu_fb_data_type *dpufd);
bool hisi_fb_device_probe_defer(uint32_t panel_type, uint32_t bl_type);

int32_t bl_config_max_value(void);
#if defined(CONFIG_HUAWEI_DSM)
void panel_check_status_and_report_by_dsm(struct lcd_reg_read_t *lcd_status_reg, int cnt, char __iomem *mipi_dsi0_base);
void panel_status_report_by_dsm(struct lcd_reg_read_t *lcd_status_reg, int cnt,
	char __iomem *mipi_dsi0_base, int report_cnt);
#endif
#ifdef CONFIG_LCD_KIT_DRIVER
int hisi_blpwm_set_bl(struct dpu_fb_data_type *dpufd, uint32_t bl_level);
void lcd_switch_region(struct dpu_fb_data_type *dpufd,
	uint8_t curr_mode, uint8_t pre_mode);
int lcd_get_dbv_stat(struct dpu_fb_data_type *dpufd,
	struct display_engine_duration param);
#endif

int panel_next_tcon_mode(struct platform_device *pdev, struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo);
int panel_set_display_region(struct dpu_fb_data_type *dpufd, const void __user *argp);
int set_display_panel_status(struct dpu_fb_data_type *dpufd, const void __user *argp);
int dpufb_config_panel_esd_status(struct dpu_fb_data_type *dpufd, const void __user *argp);
int dpufb_get_lcd_panel_info(struct dpu_fb_data_type *dpufd, struct platform_product_info *product_info);
void dpufd_get_panel_info(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	uint32_t fold_disp_panel_id);
void get_lcd_panel_info(struct dpu_fb_data_type *dpufd, struct dpu_panel_info **pinfo, uint32_t panel_id);

/* for display time */
void dpufb_panel_display_time_init(struct dpu_fb_data_type *dpufd);
void dpufb_panel_set_hiace_timestamp(struct dpu_fb_data_type *dpufd, bool enable, int mode);
void dpufb_panel_set_display_region_timestamp(struct dpu_fb_data_type *dpufd);
void dpufb_panel_get_hiace_display_time(struct dpu_fb_data_type *dpufd, uint32_t *time);

/* for fold count */
void dpufb_panel_add_fold_count(struct dpu_fb_data_type *dpufd, uint8_t region);
uint32_t dpufb_panel_get_fold_count(struct dpu_fb_data_type *dpufd);
void get_lcd_panel_info(struct dpu_fb_data_type *dpufd, struct dpu_panel_info **pinfo, uint32_t panel_id);

int lm36274_set_backlight_reg(uint32_t bl_level);
int ktz8864_set_backlight_reg(uint32_t bl_level);
bool is_ktz8864_used(void);

#endif /* HISI_FB_PANEL_H */
