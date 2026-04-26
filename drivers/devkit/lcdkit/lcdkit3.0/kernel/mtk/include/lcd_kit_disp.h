/*
 * lcd_kit_disp.h
 *
 * lcdkit display function head file for lcd driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __LCD_KIT_DISP_H_
#define __LCD_KIT_DISP_H_

#include "lcd_kit_common.h"
#include "lcd_kit_utils.h"
#if defined CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
/* macro definition */
#define DTS_COMP_LCD_KIT_PANEL_TYPE     "huawei,lcd_panel_type"
#define LCD_KIT_PANEL_COMP_LENGTH       128
#define ESD_TE_CHECK_ENABLE 0
// elvdd detect type
#define ELVDD_MIPI_CHECK_MODE   1
#define ELVDD_GPIO_CHECK_MODE   2

struct lcd_kit_disp_info *lcd_kit_get_disp_info(void);
#define disp_info	lcd_kit_get_disp_info()
unsigned int lcm_get_panel_state(void);
/* enum */
enum alpm_mode {
	ALPM_DISPLAY_OFF,
	ALPM_ON_HIGH_LIGHT,
	ALPM_EXIT,
	ALPM_ON_LOW_LIGHT,
	ALPM_SINGLE_CLOCK,
	ALPM_DOUBLE_CLOCK,
	ALPM_ON_MIDDLE_LIGHT,
};

enum alpm_state {
	ALPM_OUT,
	ALPM_START,
	ALPM_IN,
};

struct elvdd_detect {
	u32 support;
	u32 detect_type;
	u32 detect_gpio;
	u32 exp_value;
	u32 exp_value_mask;
	u32 delay;
	bool is_start_delay_timer;
	struct lcd_kit_dsi_panel_cmds cmds;
};

/* pcd detect */
enum pcd_check_status {
	PCD_CHECK_WAIT,
	PCD_CHECK_ON,
	PCD_CHECK_OFF,
};

struct poweric_detect_delay {
	struct work_struct wq;
	struct timer_list timer;
};

/* dsc parmas */
struct dsc_params {
	unsigned int ver;
	unsigned int slice_mode;
	unsigned int rgb_swap;
	unsigned int dsc_cfg;
	unsigned int rct_on;
	unsigned int bit_per_channel;
	unsigned int dsc_line_buf_depth;
	unsigned int bp_enable;
	unsigned int bit_per_pixel;
	unsigned int pic_height;
	unsigned int pic_width;
	unsigned int slice_height;
	unsigned int slice_width;
	unsigned int chunk_size;
	unsigned int xmit_delay;
	unsigned int dec_delay;
	unsigned int scale_value;
	unsigned int increment_interval;
	unsigned int decrement_interval;
	unsigned int line_bpg_offset;
	unsigned int nfl_bpg_offset;
	unsigned int slice_bpg_offset;
	unsigned int initial_offset;
	unsigned int final_offset;
	unsigned int flatness_minqp;
	unsigned int flatness_maxqp;
	unsigned int rc_model_size;
	unsigned int rc_edge_factor;
	unsigned int rc_quant_incr_limit0;
	unsigned int rc_quant_incr_limit1;
	unsigned int rc_tgt_offset_hi;
	unsigned int rc_tgt_offset_lo;
};

/* struct */
struct lcd_kit_disp_info {
	/* effect */
	/* gamma calibration */
	struct lcd_kit_gamma gamma_cal;
	/* oem information */
	struct lcd_kit_oem_info oeminfo;
	/* rgbw function */
	struct lcd_kit_rgbw rgbw;
	/* end */
	/* normal */
	/* lcd type */
	u32 lcd_type;
	/* panel information */
	char *compatible;
	/* regulator name */
	char *vci_regulator_name;
	char *iovcc_regulator_name;
	char *vdd_regulator_name;
	/* product id */
	u32 product_id;
	/* vr support */
	u32 vr_support;
	/* lcd kit blank semaphore */
	struct semaphore blank_sem;
	/* lcd kit semaphore */
	struct semaphore lcd_kit_sem;
	/* lcd kit mipi mutex lock */
	struct mutex mipi_lock;
	/* alpm -aod */
	struct lcd_kit_alpm alpm;
	u8 alpm_state;
	ktime_t alpm_start_time;
	struct mutex drm_hw_lock;
	/* pre power off */
	u32 pre_power_off;
	/* quickly sleep out */
	struct lcd_kit_quickly_sleep_out quickly_sleep_out;
	/* fps ctrl */
	struct lcd_kit_fps fps;
	/* project id */
	struct lcd_kit_project_id project_id;
	/* dsc enable */
	unsigned int dsc_enable;
	struct dsc_params dsc_params;
	/* thp_second_poweroff_sem */
	struct semaphore thp_second_poweroff_sem;
	struct display_engine_ddic_rgbw_param ddic_rgbw_param;
	/* elvdd detect */
	struct elvdd_detect elvdd_detect;
	/* end */
	/* normal */
	bool bl_is_shield_backlight;
	u8 bl_is_start_timer;
	/* hbm set */
	u32 hbm_entry_delay;
	ktime_t hbm_blcode_ts;
	u32 te_interval_us;
	u32 hbm_en_times;
	u32 hbm_dis_times;
	/* P3 color set */
	u32 lcm_color;
	/* HBM delay set */
	u32 mtk_high_left_time_th_us;
	u32 mtk_high_right_time_th_us;
	u32 mtk_high_te_inerval_time_us;
	u32 mtk_low_left_time_th_us;
	u32 mtk_low_right_time_th_us;
	u32 mtk_low_te_inerval_time_us;
	/* skip poweron esd check */
	u8 lcd_kit_skip_poweron_esd_check;
	u8 lcd_kit_skip_esd_check_cnt;
	u32 lcd_kit_again_esd_check_delay;
	u32 lcd_kit_esd_check_delay;
	/* pcd errflag */
	struct lcd_kit_pcd_errflag pcd_errflag;
	/* display idle mode support */
	u32 lcd_kit_idle_mode_support;
	struct work_struct recovery_wq;
	u32 bf_stop_vdo_delay;
	u32 panel_need_wait_vblank;
};

extern int lcd_kit_power_init(void);
extern int lcd_kit_sysfs_init(void);
unsigned int lcm_get_panel_backlight_max_level(void);
#if defined CONFIG_HUAWEI_DSM
struct dsm_client *lcd_kit_get_lcd_dsm_client(void);
#endif
#ifdef CONFIG_DRM_MEDIATEK
int lcd_kit_rgbw_set_handle(void);
#endif
#endif
