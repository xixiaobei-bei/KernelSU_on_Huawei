/*
 * lcd_kit_utils.h
 *
 * lcdkit utils function head file for lcd driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_UTILS_H_
#define __LCD_KIT_UTILS_H_
#include <linux/kernel.h>
#include <linux/ctype.h>
#include "dsi_display.h"
#include "dsi_panel.h"
#include "lcd_kit_common.h"
#include "lcd_kit_panel.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_adapt.h"

/* macro */
/* default panel */
#define LCD_KIT_DEFAULT_PANEL  "auo_otm1901a_5p2_1080p_video_default"

/* dcs read/write */
#define DTYPE_DCS_WRITE		0x05 /* short write, 0 parameter */
#define DTYPE_DCS_WRITE1	0x15 /* short write, 1 parameter */
#define DTYPE_DCS_READ		0x06 /* read */
#define DTYPE_DCS_LWRITE	0x39 /* long write */
#define DTYPE_DSC_LWRITE	0x0A /* dsc dsi1.2 vesa3x long write */

/* generic read/write */
#define DTYPE_GEN_WRITE		0x03 /* short write, 0 parameter */
#define DTYPE_GEN_WRITE1	0x13 /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2	0x23 /* short write, 2 parameter */
#define DTYPE_GEN_LWRITE	0x29 /* long write */
#define DTYPE_GEN_READ		0x04 /* long read, 0 parameter */
#define DTYPE_GEN_READ1		0x14 /* long read, 1 parameter */
#define DTYPE_GEN_READ2		0x24 /* long read, 2 parameter */

/* lcd fps scence */
#define LCD_KIT_FPS_SCENCE_IDLE        BIT(0)
#define LCD_KIT_FPS_SCENCE_VIDEO       BIT(1)
#define LCD_KIT_FPS_SCENCE_GAME        BIT(2)
#define LCD_KIT_FPS_SCENCE_WEB         BIT(3)
#define LCD_KIT_FPS_SCENCE_EBOOK       BIT(4)
#define LCD_KIT_FPS_SCENCE_FORCE_30FPS          BIT(5)
#define LCD_KIT_FPS_SCENCE_FUNC_DEFAULT_ENABLE  BIT(6)
#define LCD_KIT_FPS_SCENCE_FUNC_DEFAULT_DISABLE BIT(7)
/* lcd fps value */
#define LCD_KIT_FPS_30 30
#define LCD_KIT_FPS_55 55
#define LCD_KIT_FPS_60 60
#define MAX_BUF 60
#define LCD_REG_LENGTH_MAX 200
#define LCD_DDIC_INFO_LEN 64
/* 2d barcode */
#define BARCODE_LENGTH 46

/* project id */
#define PROJECTID_LEN 10
#define SN_CODE_LENGTH_MAX 54

struct display_engine_ddic_rgbw_param {
	int ddic_panel_id;
	int ddic_rgbw_mode;
	int ddic_rgbw_backlight;
	int pixel_gain_limit;
};

struct display_engine_panel_info_param {
	int width;
	int height;
	int maxluminance;
	int minluminance;
	int maxbacklight;
	int minbacklight;
};

struct display_engine {
	u8 ddic_cabc_support;
	u8 ddic_rgbw_support;
};

/* lcd fps scence */
enum {
	LCD_FPS_SCENCE_60 = 0,
	LCD_FPS_SCENCE_H60 = 1,
	LCD_FPS_SCENCE_90 = 2,
	LCD_FPS_SCENCE_120 = 3,
	LCD_FPS_SCENCE_MAX = 4,
};

/* fps dsi mipi parameter index */
enum {
	FPS_HFP_INDEX = 0,
	FPS_HBP_INDEX = 1,
	FPS_HS_INDEX = 2,
	FPS_VFP_INDEX = 3,
	FPS_VBP_INDEX = 4,
	FPS_VS_INDEX = 5,
	FPS_VRE_INDEX = 6,
	FPS_RATE_INDEX = 7,
	FPS_LOWER_INDEX = 8,
	FPS_DSI_TIMMING_PARA_NUM = 9,
};

struct qcom_panel_info {
	struct dsi_display *display;
	u32 panel_state;
	u32 panel_lcm_type;
	u32 panel_dsi_mode;
	u32 type;
	u32 xres;
	u32 yres;
	u32 bl_set_type;
	u32 bl_min;
	u32 bl_max;
	u32 bl_default;
	u32 bl_current;
	u32 vrefresh;
	u32 gpio_offset;
	int maxluminance;
	int minluminance;
	/* sn code */
	uint32_t sn_code_length;
	unsigned char sn_code[SN_CODE_LENGTH_MAX];
};
/* enum */
enum {
	RGBW_SET1_MODE = 1,
	RGBW_SET2_MODE = 2,
	RGBW_SET3_MODE = 3,
	RGBW_SET4_MODE = 4,
};

enum {
	LCD_OFFLINE = 0,
	LCD_ONLINE = 1,
};

struct lcd_kit_gamma {
	u32 support;
	u32 addr;
	u32 length;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_color_coordinate {
	u32 support;
	/* color consistency support */
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_panel_id {
	u32 modulesn;
	u32 equipid;
	u32 modulemanufactdate;
	u32 vendorid;
};

struct lcd_kit_2d_barcode {
	u32 support;
	int number_offset;
	struct lcd_kit_dsi_panel_cmds cmds;
	bool flags;
};

struct lcd_kit_oem_info {
	u32 support;
	/* 2d barcode */
	struct lcd_kit_2d_barcode barcode_2d;
	/* color coordinate */
	struct lcd_kit_color_coordinate col_coordinate;
};

struct lcd_kit_brightness_color_oeminfo {
	u32 support;
	struct lcd_kit_oem_info oem_info;
};

struct lcd_kit_project_id {
	u32 support;
	char id[LCD_DDIC_INFO_LEN];
	char *default_project_id;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_fps {
	u32 support;
	u32 fps_switch_support;
	unsigned int default_fps;
	unsigned int current_fps;
	unsigned int hop_support;
	struct lcd_kit_dsi_panel_cmds dfr_enable_cmds;
	struct lcd_kit_dsi_panel_cmds dfr_disable_cmds;
	struct lcd_kit_dsi_panel_cmds fps_to_30_cmds;
	struct lcd_kit_dsi_panel_cmds fps_to_60_cmds;
	struct lcd_kit_array_data low_frame_porch;
	struct lcd_kit_array_data normal_frame_porch;
	struct lcd_kit_array_data panel_support_fps_list;
	struct lcd_kit_dsi_panel_cmds fps_to_cmds[LCD_FPS_SCENCE_MAX];
	struct lcd_kit_array_data fps_dsi_timming[LCD_FPS_SCENCE_MAX];
	struct lcd_kit_array_data hop_info[LCD_FPS_SCENCE_MAX];
};

struct lcd_kit_rgbw {
	u32 support;
	u32 rgbw_bl_max;
	struct lcd_kit_dsi_panel_cmds mode1_cmds;
	struct lcd_kit_dsi_panel_cmds mode2_cmds;
	struct lcd_kit_dsi_panel_cmds mode3_cmds;
	struct lcd_kit_dsi_panel_cmds mode4_cmds;
	struct lcd_kit_dsi_panel_cmds backlight_cmds;
	struct lcd_kit_dsi_panel_cmds saturation_ctrl_cmds;
	struct lcd_kit_dsi_panel_cmds frame_gain_limit_cmds;
	struct lcd_kit_dsi_panel_cmds frame_gain_speed_cmds;
	struct lcd_kit_dsi_panel_cmds color_distor_allowance_cmds;
	struct lcd_kit_dsi_panel_cmds pixel_gain_limit_cmds;
	struct lcd_kit_dsi_panel_cmds pixel_gain_speed_cmds;
	struct lcd_kit_dsi_panel_cmds pwm_gain_cmds;
};

struct lcd_kit_alpm {
	u32 support;
	u32 state;
	struct lcd_kit_dsi_panel_cmds exit_cmds;
	struct lcd_kit_dsi_panel_cmds off_cmds;
	struct lcd_kit_dsi_panel_cmds low_light_cmds;
	struct lcd_kit_dsi_panel_cmds high_light_cmds;
};

struct lcd_kit_snd_disp {
	u32 support;
	struct lcd_kit_dsi_panel_cmds on_cmds;
	struct lcd_kit_dsi_panel_cmds off_cmds;
};

struct lcd_kit_quickly_sleep_out {
	u32 support;
	u32 interval;
	u32 panel_on_tag;
	struct timeval panel_on_record_tv;
};

enum bl_control_mode {
	MTK_AP_MODE = 0,
	I2C_ONLY_MODE = 1,
	PWM_ONLY_MODE,
	MTK_PWM_HIGH_I2C_MODE,
	MUTI_THEN_RAMP_MODE,
	RAMP_THEN_MUTI_MODE,
	MTK_AAL_I2C_MODE,
	MTK_MIPI_MODE,
	MTK_MIPI_BL_IC_PWM_MODE,
};

enum bias_control_mode {
	MT_AP_MODE = 0,
	PMIC_ONLY_MODE = 1,
	GPIO_ONLY_MODE,
	GPIO_THEN_I2C_MODE,
};

struct display_engine_ddic_hbm_param {
	int type;      // 0:fp   1:MMI   2:light
	int level;
	bool dimming;  // 0:no dimming  1:dimming
};

enum HBM_CFG_TYPE {
	HBM_FOR_FP = 0,
	HBM_FOR_MMI = 1,
	HBM_FOR_LIGHT = 2
};

struct hbm_type_cfg {
	int source;
	void *dsi;
	void *cb;
	void *handle;
};

/* function declare */
int lcd_kit_read_project_id(void);
int lcd_kit_utils_init(struct device_node *np, struct qcom_panel_info *pinfo);
bool lcd_kit_support(void);
void lcd_kit_disp_on_record_time(void);
int lcd_kit_get_bl_max_nit_from_dts(void);
void lcd_kit_disp_on_check_delay(void);
void lcd_kit_set_bl_cmd(uint32_t level);
int lcd_kit_mipi_set_backlight(struct hbm_type_cfg hbm_source, uint32_t level);
int lcd_panel_sncode_store(void);
#endif
