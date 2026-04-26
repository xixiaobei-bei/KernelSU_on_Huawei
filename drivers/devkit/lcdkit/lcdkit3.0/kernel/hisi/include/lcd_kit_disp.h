/*
 * lcd_kit_disp.h
 *
 * lcdkit display function for lcd driver head file
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
#include "hisi_fb.h"
#include "hisi_mipi_dsi.h"
#include "lcd_kit_utils.h"
/*
 * macro definition
 */
#define DTS_COMP_LCD_KIT_PANEL_TYPE     "huawei,lcd_panel_type"
#define POWERKEY_RESTART_TYPE_PROP     "androidboot.powerkey_restart_type"
#define LCD_KIT_PANEL_COMP_LENGTH       128
// app setting default backlight
#define MAX_BACKLIGHT_FROM_APP  993
#define MIN_BACKLIGHT_FROM_APP  500
#define BACKLIGHT_HIGH_LEVEL 1
#define BACKLIGHT_LOW_LEVEL  2
#define SCBAKDATA11 (0x438)

/* parse power length */
#define LCD_POWER_LEN 3

// UD PrintFinger HBM
#define LCD_KIT_FP_HBM_ENTER 1
#define LCD_KIT_FP_HBM_EXIT  2
#define LCD_KIT_DATA_LEN_TWO 2
#define LCD_KIT_ENABLE_ELVSSDIM  0
#define LCD_KIT_DISABLE_ELVSSDIM 1
#define LCD_KIT_ELVSSDIM_NO_WAIT 0

// UD PrintFinger unlock when screen on
#define MASK_LAYER_SCREENON 0x02
#define CIRCLE_LAYER 0x04
// elvdd detect type
#define ELVDD_MIPI_CHECK_MODE   1
#define ELVDD_GPIO_CHECK_MODE   2
#define ELVDD_GPIO_CHECK_MODE_AP 3
#define ELVDD_SH_MIPI_AP_GPIO_CHECK_MODE 4
#define REC_DMD_NO_LIMIT      (-1)
#define DMD_RECORD_BUF_LEN    100
#define RECOVERY_TIMES          1
/* project id length */
#define PROJECTID_LEN 10

#define LCD_NEED_ULPS 1
#define LCD_NO_NEED_ULPS 0

#define LCD_NV_READ 1
#define LCD_NV_WRITE 0
#define LCD_NV_NUMBER 451
#define LCD_NV_OPS_TIMEOUT 50
#define LCD_NV_SN_CODE_LEN 21

struct lcd_kit_disp_info *lcd_kit_get_disp_info(void);
#define disp_info lcd_kit_get_disp_info()
struct lcd_kit_disp_lock *lcd_kit_get_disp_lock(void);
#define DISP_LOCK lcd_kit_get_disp_lock()

// ENUM
enum alpm_mode {
	ALPM_DISPLAY_OFF,
	ALPM_ON_MIDDLE_LIGHT,
	ALPM_EXIT,
	ALPM_ON_LOW_LIGHT,
};
enum pcd_check_status {
	PCD_CHECK_WAIT,
	PCD_CHECK_ON,
	PCD_CHECK_OFF,
};
enum powerkey_restart_type {
	PRESS_POWER_ONLY,
	PRESS_POWER_VOL_DOWN,
	PRESS_POWER_VOL_UP,
};

enum lcd_gpio_type {
	NORMAL_GPIO_TYPE,
	EXTEND_GPIO_TYPE,
};

struct amoled_power_ic_check {
	uint32_t support;
	uint32_t gpio;
	uint32_t expect_value;
};

struct elvdd_detect {
	u32 support;
	u32 detect_type;
	u32 detect_gpio;
	u32 detect_gpio_type;
	u32 exp_value;
	u32 exp_value_mask;
	u32 delay;
	bool is_start_delay_timer;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct poweric_detect_delay {
	struct work_struct wq;
	struct timer_list timer;
	struct hisi_fb_data_type *hisifd;
};
enum ddic_alpha_mode {
	DDIC_ALPHA_ON = 1,
	DDIC_ALPHA_OFF,
	DDIC_CIRCLE_ON,
	DDIC_CIRCLE_OFF,
	DDIC_ALPHA_CIRCLE_ON, // 5
	DDIC_ALPHA_CIRCLE_OFF,
	DDIC_CIRCLE_COORINATE, // 7
	DDIC_CIRCLE_RADIUS,
	DDIC_CIRCLE_COLOR,
	DDIC_LOCAL_HBM, // 10
};
struct lcd_kit_disp_lock {
	/* lcd kit semaphore */
	struct semaphore lcd_kit_sem;
	/* thp proximity semaphore */
	struct semaphore thp_second_poweroff_sem;
};
struct lcd_kit_power_off_optimize_info {
	u32 support;
	u32 delay_time;
};

enum sot_repair_mode {
	SOT_REPAIR_NONE,
	SOT_REPAIR_11V = 1,
	SOT_REPAIR_12V,
	SOT_REPAIR_BUTT,
};

struct lcd_kit_sot_err_check {
	uint32_t support;
	uint32_t dsi0_sot_err_cnt;
	uint32_t dsi1_sot_err_cnt;
	enum sot_repair_mode dsi0_repair_flag;
	enum sot_repair_mode dsi1_repair_flag;
	struct lcd_kit_array_data check_value;
	struct lcd_kit_dsi_panel_cmds check_cmds;
	struct lcd_kit_dsi_panel_cmds repair11_cmds;
	struct lcd_kit_dsi_panel_cmds repair12_cmds;
};

/* nv info 104 byte */
struct lcd_kit_nv_sot_cfg {
	u8 panel0_sncode[LCD_NV_SN_CODE_LEN];
	u8 panel1_sncode[LCD_NV_SN_CODE_LEN];
	u8 panel0_dsi0_flag;
	u8 panel0_dsi1_flag;
	u8 panel1_dsi0_flag;
};

// STRUCT
struct lcd_kit_disp_info {
	struct lcd_kit_cascade_ic cascade_ic;
	/* pcd errflag */
	struct lcd_kit_pcd_errflag pcd_errflag;
	/* effect */
	/* gamma calibration */
	struct lcd_kit_gamma gamma_cal;
	/* oem information */
	struct lcd_kit_oem_info oeminfo;
	/* rgbw function */
	struct lcd_kit_rgbw rgbw;
	/* demura */
	struct lcd_kit_demura demura;
	/* end */
	/* normal */
	u8 bl_is_shield_backlight;
	u8 bl_is_start_second_timer;
	/* lcd type */
	u32 lcd_type;
	/* panel information */
	char *compatible;
	/* board version */
	u32 board_version;
	/* product id */
	u32 product_id;
	/* vr support */
	u32 vr_support;
	/* ulps support */
	u8 ramless_aod;
	/* PPS parameters calc algorithm support */
	u32 calc_mode;
	/* 0: Parse effect parameter from .h
	   1: Parse effect parameter from dts */
	u8 effect_para_is_from_dts;
	/* dual dsi send need sync */
	u32 dual_dsi_send_sync;
	/* mipi cmds always run in lpmode */
	u32 mipi_cmds_always_run_in_lpmode;
	/* alpm -aod */
	struct lcd_kit_alpm alpm;
	/* quickly sleep out */
	struct lcd_kit_quickly_sleep_out quickly_sleep_out;
	/* fps ctrl */
	struct lcd_kit_fps fps;
	/* project id */
	struct lcd_kit_project_id project_id;
	/* panel version */
	struct lcd_kit_panel_version panel_version;
	/* otp gamma */
	struct lcd_kit_otp_gamma otp_gamma;
	/* dbv statistics */
	struct dbv_stat_desc dbv_stat;
	/* elvdd detect */
	struct elvdd_detect elvdd_detect;
	/* fps gamma code */
	struct lcd_kit_fps_gamma fps_gamma;
	/* frame odd even code */
	struct lcd_kit_frame_odd_even frame_odd_even;
	/* amoled power ic check */
	struct amoled_power_ic_check amoled_power_ic_check;
	/* power key press 6s */
	struct lcd_kit_power_key_info pwrkey_press;
	/* panel irc code */
	struct lcd_kit_panel_irc panel_irc;
	/* power off time optimize */
	struct lcd_kit_power_off_optimize_info pwr_off_optimize_info;
	/* separate transmission */
	struct lcd_kit_transmission transmission;
	/* sot error irc check */
	struct lcd_kit_sot_err_check sot_err_check;
	/* end */
};

/*
 * variable declaration
 */
/* extern variable */
extern int lcd_kit_power_init(struct platform_device *pdev);
extern int lcd_kit_sysfs_init(void);
extern int lcd_kit_dbg_init(void);
/*
 * function declaration
 */
#ifdef LCD_FACTORY_MODE
int lcd_factory_init(struct device_node *np);
#endif
#if defined CONFIG_HUAWEI_DSM
struct dsm_client *lcd_kit_get_lcd_dsm_client(void);
#endif
#endif
