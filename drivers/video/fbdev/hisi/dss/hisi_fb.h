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
#ifndef HISI_FB_H
#define HISI_FB_H

#include "hisi_fb_struct.h"
#include "product/hisi_fb_adapt.h"

/*
 * FUNCTIONS PROTOTYPES
 */
extern int g_primary_lcd_xres;
extern int g_primary_lcd_yres;
extern uint64_t g_pxl_clk_rate;

extern uint8_t g_prefix_ce_support;
extern uint8_t g_prefix_sharpness1d_support;
extern uint8_t g_prefix_sharpness2d_support;

extern uint32_t g_online_cmdlist_idxs;
extern uint32_t g_offline_cmdlist_idxs;

extern uint32_t g_fpga_flag;
extern uint32_t g_fastboot_enable_flag;
extern uint32_t g_fake_lcd_flag;
extern uint32_t g_dss_version_tag;
extern uint32_t g_dss_module_resource_initialized;
extern uint32_t g_logo_buffer_base;
extern uint32_t g_logo_buffer_size;
extern uint32_t g_underflow_stop_perf_stat;
extern uint32_t g_mipi_dphy_version; /* C30:0 C50:1 */
extern uint32_t g_mipi_dphy_opt;
extern uint32_t g_chip_id;

extern uint32_t g_fastboot_already_set;

extern int g_debug_online_vactive;
void de_open_update_te(void);
void de_close_update_te(struct work_struct *work);
extern struct fb_info *fbi_list[HISI_FB_MAX_FBI_LIST];
extern struct dpu_fb_data_type *dpufd_list[HISI_FB_MAX_FBI_LIST];
extern struct semaphore g_dpufb_dss_clk_vote_sem;
extern struct mutex g_rgbw_lock;

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_V330) && !defined(CONFIG_HISI_FB_V501)
extern uint32_t g_post_xcc_table_temp[12];
extern u32 g_gmp_lut_table_low32bit_init[4913];
extern u32 g_gmp_lut_table_high4bit_init[4913];
#endif

#if defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V360)
extern char __iomem *dpufd_soc_acpu_ao_ioc_base;
#endif
#ifdef CONFIG_REPORT_VSYNC
extern void mali_kbase_pm_report_vsync(int);
#endif
#ifdef CONFIG_MALI_FENCE_DEBUG
extern void kbase_fence_dump_in_display(void);
#endif
extern int mipi_dsi_ulps_cfg(struct dpu_fb_data_type *dpufd, int enable);
extern bool hisi_dss_check_reg_reload_status(struct dpu_fb_data_type *dpufd);
unsigned int get_boot_into_recovery_flag(void);
uint32_t get_panel_xres(struct dpu_fb_data_type *dpufd);
uint32_t get_panel_yres(struct dpu_fb_data_type *dpufd);

bool is_fastboot_display_enable(void);
bool is_dss_idle_enable(void);

/* mediacomm channel manager */
int hisi_mdc_resource_init(struct dpu_fb_data_type *dpufd, unsigned int platform);
int hisi_mdc_chn_request(struct fb_info *info, void __user *argp);
int hisi_mdc_chn_release(struct fb_info *info, const void __user *argp);

/* dss secure */
void dpufb_atf_config_security(uint32_t master_op_type, uint32_t channel, uint32_t mode);
void dpufb_secure_register(struct platform_device *pdev);
void dpufb_secure_unregister(struct platform_device *pdev);
int hisi_fb_blank_sub(int blank_mode, struct fb_info *info);

/* backlight */
void dpufb_backlight_update(struct dpu_fb_data_type *dpufd);
void dpufb_backlight_cancel(struct dpu_fb_data_type *dpufd);
void dpufb_backlight_register(struct platform_device *pdev);
void dpufb_backlight_unregister(struct platform_device *pdev);
void dpufb_set_backlight(struct dpu_fb_data_type *dpufd, uint32_t bkl_lvl, bool enforce);
int update_cabc_pwm(struct dpu_fb_data_type *dpufd);

/* backlight flicker detector */
void bl_flicker_detector_init(display_engine_flicker_detector_config_t config);
void bl_flicker_detector_collect_upper_bl(int level);
void bl_flicker_detector_collect_algo_delta_bl(int level);
void bl_flicker_detector_collect_device_bl(int level);

/* vsync */
void dpufb_frame_updated(struct dpu_fb_data_type *dpufd);
void dpufb_set_vsync_activate_state(struct dpu_fb_data_type *dpufd, bool infinite);
void dpufb_activate_vsync(struct dpu_fb_data_type *dpufd);
void dpufb_deactivate_vsync(struct dpu_fb_data_type *dpufd);
int dpufb_vsync_ctrl(struct fb_info *info, const void __user *argp);
int dpufb_vsync_resume(struct dpu_fb_data_type *dpufd);
int dpufb_vsync_suspend(struct dpu_fb_data_type *dpufd);
void dpufb_vsync_isr_handler(struct dpu_fb_data_type *dpufd);
void dpufb_vsync_register(struct platform_device *pdev);
void dpufb_vsync_unregister(struct platform_device *pdev);
void dpufb_vsync_disable_enter_idle(struct dpu_fb_data_type *dpufd, bool disable);
void dpufb_esd_register(struct platform_device *pdev);
void dpufb_esd_unregister(struct platform_device *pdev);
void dpufb_masklayer_backlight_flag_config(struct dpu_fb_data_type *dpufd,
	bool masklayer_backlight_flag);

/* buffer sync */
int dpufb_layerbuf_lock(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, struct list_head *lock_list);
void dpufb_layerbuf_flush(struct dpu_fb_data_type *dpufd,
	struct list_head *lock_list);
void dpufb_layerbuf_unlock(struct dpu_fb_data_type *dpufd,
	struct list_head *pfree_list);
void dpufb_layerbuf_lock_exception(struct dpu_fb_data_type *dpufd,
	struct list_head *lock_list);
int dpufb_offline_layerbuf_lock(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, struct list_head *plock_list);

void dpufb_buf_sync_wait_async(struct dpufb_buf_sync *buf_sync);
int dpufb_buf_sync_wait(int fence_fd);
int dpufb_buf_sync_handle(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req);
void dpufb_buf_sync_signal(struct dpu_fb_data_type *dpufd);
void dpufb_buf_sync_suspend(struct dpu_fb_data_type *dpufd);
int dpufb_offline_create_fence(struct dpu_fb_data_type *dpufd);
int dpufb_buf_sync_create_fence(struct dpu_fb_data_type *dpufd, int32_t *release_fence, int32_t *retire_fence);
void dpufb_buf_sync_close_fence(int32_t *release_fence, int32_t *retire_fence);
void dpufb_buf_sync_register(struct platform_device *pdev);
void dpufb_buf_sync_unregister(struct platform_device *pdev);

/* control */
int dpufb_ctrl_fastboot(struct dpu_fb_data_type *dpufd);
int dpufb_ctrl_on(struct dpu_fb_data_type *dpufd);
int dpufb_ctrl_off(struct dpu_fb_data_type *dpufd);
int dpufb_ctrl_lp(struct dpu_fb_data_type *dpufd, bool lp_enter);
int dpufb_ctrl_sbl(struct fb_info *info, uint32_t value);
int dpufb_ctrl_dss_voltage_get(struct fb_info *info, void __user *argp);
int dpufb_ctrl_dss_voltage_set(struct fb_info *info, void __user *argp);
bool check_primary_panel_power_status(struct dpu_fb_data_type *dpufd);
int dpufb_ctrl_dss_vote_cmd_set(struct fb_info *info, const void __user *argp);
int dpufb_fps_upt_isr_handler(struct dpu_fb_data_type *dpufd);
int dpufb_ctrl_esd(struct dpu_fb_data_type *dpufd);
#if defined(CONFIG_DEVICE_ATTRS)
void dpufb_sysfs_attrs_add(struct dpu_fb_data_type *dpufd);
#endif
typedef void (*func_set_reg)(struct dpu_fb_data_type *, char __iomem *, uint32_t, uint8_t, uint8_t);
void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);
void dpufb_set_reg32(struct dpu_fb_data_type *, char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);
uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs);
void dpufb_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);
uint32_t dpufb_line_length(int index, uint32_t xres, int bpp);
void dpufb_get_timestamp(struct timeval *tv);
uint32_t dpufb_timestamp_diff(struct timeval *lasttime, struct timeval *curtime);
void dpufb_save_file(char *filename, const char *buf, uint32_t buf_len);

struct platform_device *hisi_fb_device_alloc(struct dpu_fb_panel_data *pdata,
	uint32_t type, uint32_t id);
struct platform_device *hisi_fb_add_device(struct platform_device *pdev);

#ifdef CONFIG_HUAWEI_OCP
int hisi_lcd_ocp_recover(struct notifier_block *nb,
		unsigned long event, void *data);
#endif

#if defined(CONFIG_HISI_FB_AOD)
/* sensorhub aod */
bool hisi_sensorhub_aod_hw_lock(struct dpu_fb_data_type *dpufd);
bool hisi_sensorhub_aod_hw_unlock(struct dpu_fb_data_type *dpufd);
int hisi_sensorhub_aod_unblank(uint32_t msg_no);
int hisi_sensorhub_aod_blank(uint32_t msg_no);
bool hisi_fb_request_aod_hw_lock(struct dpu_fb_data_type *dpufd);
void hisi_fb_release_aod_hw_lock(struct dpu_fb_data_type *dpufd, bool locked);
int hisi_fb_aod_blank(struct dpu_fb_data_type *dpufd, int blank_mode);
bool hisi_fb_get_aod_lock_status(void);
bool hisi_aod_get_aod_status(void);
int hisi_send_aod_stop(void);
uint32_t hisi_aod_get_panel_id(void);
bool hisi_aod_need_fast_unblank(void);
void hisi_aod_wait_stop_nolock(void);
void wait_for_aod_stop(struct dpu_fb_data_type *dpufd);
void wait_for_fast_unblank_end(struct dpu_fb_data_type *dpufd);
void wait_for_aod_end(struct dpu_fb_data_type *dpufd);
#else
#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
static inline uint32_t hisi_aod_get_panel_id(void) { return 0; };
#endif
#endif
uint32_t hisi_get_panel_product_type(void);

int hisi_fb_open_sub(struct fb_info *info);
int hisi_fb_release_sub(struct fb_info *info);

void hisi_fb_frame_refresh(struct dpu_fb_data_type *dpufd, char *trigger);
int hisi_dss_alloc_cmdlist_buffer(struct dpu_fb_data_type *dpufd);
void hisi_dss_free_cmdlist_buffer(struct dpu_fb_data_type *dpufd);
unsigned long dpufb_alloc_fb_buffer(struct dpu_fb_data_type *dpufd);
void dpufb_free_fb_buffer(struct dpu_fb_data_type *dpufd);

int dpufb_create_buffer_client(struct dpu_fb_data_type *dpufd);
void dpufb_destroy_buffer_client(struct dpu_fb_data_type *dpufd);
void dpufb_free_logo_buffer(struct dpu_fb_data_type *dpufd);

void hisi_fb_unblank_wq_handle(struct work_struct *work);
int hisi_fb_blank_device(struct dpu_fb_data_type *dpufd, int blank_mode, struct fb_info *info);
int hisi_fb_blank_panel_power_on(struct dpu_fb_data_type *dpufd);
int hisi_fb_blank_panel_power_off(struct dpu_fb_data_type *dpufd);
int hisi_fb_blank_update_tui_status(struct dpu_fb_data_type *dpufd);
void hisi_fb_pm_runtime_register(struct dpu_fb_data_type *dpufd);
void hisi_fb_fnc_register_base(struct dpu_fb_data_type *dpufd);
void hisi_fb_sdp_fnc_register(struct dpu_fb_data_type *dpufd);
void hisi_fb_mdc_fnc_register(struct dpu_fb_data_type *dpufd);
void hisi_fb_aux_fnc_register(struct dpu_fb_data_type *dpufd);
void hisi_fb_common_register(struct dpu_fb_data_type *dpufd);
void hisi_fb_init_screeninfo_base(struct fb_fix_screeninfo *fix, struct fb_var_screeninfo *var);
bool hisi_fb_img_type_valid(uint32_t fb_imgType);
void hisi_fb_init_sreeninfo_by_img_type(struct fb_fix_screeninfo *fix,
	struct fb_var_screeninfo *var, uint32_t fb_imgType, int *bpp);
void hisi_fb_init_sreeninfo_by_panel_info(struct fb_var_screeninfo *var,
	struct dpu_panel_info *panel_info, uint32_t fb_num, int bpp);
void hisi_fb_data_init(struct dpu_fb_data_type *dpufd);
void hisi_fb_init_sema(struct dpu_fb_data_type *dpufd);
void hisi_fb_init_spin_lock(struct dpu_fb_data_type *dpufd);
int hisi_fb_registe_callback(struct dpu_fb_data_type *dpufd,
	struct fb_var_screeninfo *var, struct dpu_panel_info *panel_info);
void hisi_fb_offlinecomposer_init(struct fb_var_screeninfo *var, struct dpu_panel_info *panel_info);
void hisi_fb_pdp_fnc_register(struct dpu_fb_data_type *dpufd);
int dpufb_check_ldi_porch(struct dpu_panel_info *pinfo);

int bl_lvl_map(int level);

void dpufb_set_panel_power_status(struct dpu_fb_data_type *dpufd, bool power_on);
#endif /* HISI_FB_H */
