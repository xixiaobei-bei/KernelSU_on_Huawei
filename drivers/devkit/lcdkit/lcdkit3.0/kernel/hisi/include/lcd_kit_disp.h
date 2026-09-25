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
#define REC_DMD_NO_LIMIT      (-1)
#define DMD_RECORD_BUF_LEN    100
#define RECOVERY_TIMES          1
/* project id length */
#define PROJECTID_LEN 10
#define PANEL_NAME_LEN          50
struct lcd_kit_disp_info *lcd_kit_get_disp_info(void);
#define disp_info lcd_kit_get_disp_info()
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

struct amoled_power_ic_check {
	uint32_t support;
	uint32_t gpio;
	uint32_t expect_value;
};

struct elvdd_detect {
	u32 support;
	u32 detect_type;
	u32 detect_gpio;
	u32 exp_value;
	u32 exp_value_mask;
	struct lcd_kit_dsi_panel_cmds cmds;
};

enum ddic_alpha_mode {
	DDIC_ALPHA_ON = 1,
	DDIC_ALPHA_OFF,
	DDIC_CIRCLE_ON,
	DDIC_CIRCLE_OFF,
	DDIC_ALPHA_CIRCLE_ON, //5
	DDIC_ALPHA_CIRCLE_OFF,
	DDIC_CIRCLE_COORINATE, //7
	DDIC_CIRCLE_RADIUS,
	DDIC_CIRCLE_COLOR,
};

// STRUCT
struct lcd_kit_disp_info {
	struct lcd_kit_cascade_ic cascade_ic;
	/* voltage detect test witch picture */
	struct ddic_lv_detect_desc ddic_lv_detect;
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
	/* lcd kit semaphore */
	struct semaphore lcd_kit_sem;
	/* thp proximity semaphore */
	struct semaphore thp_second_poweroff_sem;
	/* lcd kit mipi mutex lock */
	struct mutex mipi_lock;
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
	/* end */
};

struct lcd_panel_name_map {
	char panel_name[PANEL_NAME_LEN];
	char hide_panel_name[PANEL_NAME_LEN];
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
