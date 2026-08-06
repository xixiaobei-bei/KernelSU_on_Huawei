/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */
/*lint -e86 -e446 -e529 -e559 -e574 -e666*/

#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/clk.h>
#include <drm/drmP.h>
#include <linux/of_address.h>

#include <drm/drm_crtc.h>
#include "hisi_panel.h"

#include "hisi_dpe.h"
#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_panel.h"
#include "lcdkit_backlight_ic_common.h"
#endif

#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_utils.h"
#endif


#ifdef CONFIG_HUAWEI_DUBAI
#include <chipset_common/dubai/dubai.h>
#endif

#include "backlight_linear_to_exponential.h"
#include "backlight/lm36923.h"

enum bl_control_mode {
		REG_ONLY_MODE = 1,
		PWM_ONLY_MODE,
		MUTI_THEN_RAMP_MODE,
		RAMP_THEN_MUTI_MODE,
		I2C_ONLY_MODE = 6,
		BLPWM_AND_CABC_MODE,
		COMMON_IC_MODE = 8,
		AMOLED_NO_BL_IC_MODE = 9,
		BLPWM_MODE = 10,
};

/* default pwm clk */
#define DEFAULT_PWM_CLK_RATE (114 * 1000000L)

#define BACKLIGHT_LOG_PRINTF   (16)
#define DISPLAYENGINE_BL_DEBUG_FRAMES 5
#define BL_PWM_GPIO   161

struct mutex g_rgbw_lock;

static char __iomem *hisifd_blpwm_base;
static struct clk *g_dss_blpwm_clk;
static struct platform_device *g_blpwm_pdev;
static int g_blpwm_on;
static int g_blpwm_fpga_flag;
static bool g_blpwm_thread_stop;

static struct pinctrl_data blpwmpctrl;

struct display_engine_info {
	bool is_ready;
	bool blc_used;
	bool blc_enable;
	int blc_delta;
	int ddic_cabc_mode;
	int ddic_color_mode;
	int ddic_panel_id;
	int ddic_rgbw_mode;
	int ddic_rgbw_backlight;
	int rgbw_saturation_control;
	int frame_gain_limit;
	int frame_gain_speed;
	int color_distortion_allowance;
	int pixel_gain_limit;
	int pixel_gain_speed;
	int pwm_duty_gain;
	int rgbw_total_glim;
	bool hbm_dimming;
	int hbm_level;
	int last_hbm_level;
};

struct display_engine_info de_info = {
	.ddic_panel_id =  3,
};

static struct pinctrl_cmd_desc blpwm_pinctrl_init_cmds[] = {
	{ DTYPE_PINCTRL_GET, &blpwmpctrl, 0 },
	{ DTYPE_PINCTRL_STATE_GET, &blpwmpctrl, DTYPE_PINCTRL_STATE_DEFAULT },
	{ DTYPE_PINCTRL_STATE_GET, &blpwmpctrl, DTYPE_PINCTRL_STATE_IDLE },
};

static struct pinctrl_cmd_desc blpwm_pinctrl_normal_cmds[] = {
	{ DTYPE_PINCTRL_SET, &blpwmpctrl, DTYPE_PINCTRL_STATE_DEFAULT },
};

static struct pinctrl_cmd_desc blpwm_pinctrl_lowpower_cmds[] = {
	{ DTYPE_PINCTRL_SET, &blpwmpctrl, DTYPE_PINCTRL_STATE_IDLE },
};

static struct pinctrl_cmd_desc blpwm_pinctrl_finit_cmds[] = {
	{ DTYPE_PINCTRL_PUT, &blpwmpctrl, 0 },
};


#define BLPWM_OUT_CTRL (0x100)
#define BLPWM_OUT_DIV (0x104)
#define BLPWM_OUT_CFG (0x108)

#define BLPWM_OUT_PRECISION_DEFAULT (800)
#define BLPWM_OUT_PRECISION         (10000)
#define BLPWM_OUT_PRECISION_2048    (2047)
#define BLPWM_OUT_PRECISION_4096    (4095)
#define BLPWM_BL_LEVEL_MIN (4)

/* BLPWM input address */
#define PWM_IN_CTRL_OFFSET (0x000)
#define PWM_IN_DIV_OFFSET (0x004)
#define PWM_IN_NUM_OFFSET (0x008)
#define PWM_IN_MAX_COUNTER (0x010)
#define PWM_IN_PRECISION_DEFAULT (512)
#define PWM_IN_PRECISION_RGBW (8192)
#define PWM_IN_SENSITY (2)

#define CABC_DIMMING_STEP_TOTAL_NUM    32
#define RGBW_DIMMING_STEP_TOTAL_NUM    3
static struct task_struct *cabc_pwm_task;

static uint8_t rgbw_lcd_support;
#define PWM_PERIOD_TOTAL_COUNTOR_JDI_SHARP   4000
#define PWM_PERIOD_TOTAL_COUNTOR_LGD   1000
#define PWM_MAX_COUNTER_FOR_RGBW_JDI_SHARP   4500
#define PWMDUTY_FILTER_NUMBER   20
#define HIGHLEVEL_PWMDUTY_FILTER_NUMBER   3
#define BRIGHTNESS_FILTER_FOR_PWMDUTY   100

static int pwmduty[PWMDUTY_FILTER_NUMBER] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

#define BACKLIGHT_FILTER_NUMBER 20
static int backlight_buf[BACKLIGHT_FILTER_NUMBER] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};
static unsigned int g_count;

#define MIN_PWMDUTY_FOR_RGBW 13
#define RGBW_BL_STOP_THRESHOLD 200
#define RGBW_LOW_LIGHT_THRESHOLD  35
#define RGBW_LOW_LIGHT_STOP 30
#define RGBW_LG_RGB 480
#define RGBW_LG_RGBW 800
#define RGBW_LG_CDA 50
#define RGBW_LG_CDA_OUTDOOR 766
#define RGBW_LG_BIT 4096
#define RGBW_LG_RGB_HMA 500
#define RGBW_LG_RGBW_HMA 843
#define RGBW_BOE_RGB_HMA 500
#define RGBW_BOE_RGBW_HMA 820
#define RGBW_BOE_FGL_HMA 224
#define RGBW_BOE_PDG_HMA 142
#define RGBW_BOE_CDA_HMA 255
#define RGBW_BOE_PGL_HMA 100
#define RGBW_BOE_TGL_HMA 128

static uint32_t rgbw_bl_stop_threshold_num;
static int32_t low_light_count;
static int32_t last_backlight;
static int32_t last_pwm_duty = MIN_PWMDUTY_FOR_RGBW;
static int32_t last_delta_pwm_duty_abs;

struct bl_info {
	int32_t bl_max;
	int32_t bl_min;
	int32_t ap_brightness;
	int32_t last_ap_brightness;
	int32_t index_cabc_dimming;
	int32_t cabc_pwm;
	int32_t prev_cabc_pwm;
	int32_t current_cabc_pwm;
	int32_t cabc_pwm_in;
	int32_t last_bl_level;
	int32_t bl_ic_ctrl_mode;
	int32_t blpwm_input_precision;
	int32_t blpwm_out_precision;
	int32_t blpwm_preci_no_convert;
	struct semaphore bl_semaphore;
	int (*set_common_backlight)(int bl_level);
};

#ifdef CONFIG_LCDKIT_DRIVER
extern struct lcdkit_bl_ic_info g_bl_config;
static void update_bl(struct hisi_fb_data_type *hisifd, uint32_t backlight);
#endif

static struct bl_info g_bl_info;
#define BL_LVL_MAP_SIZE (2047)
#define BL_MAX_11BIT (2047)
#define BL_MAX_12BIT (4095)

static int bl_lvl_map(int level)
{
	int ret = 0;
	int idx;

	if (level < 0 || level > 10000) {
		HISI_DRM_ERR("Need Valid Data, level=%d!", level);
		return ret;
	}

	for (idx = 0; idx <= BL_LVL_MAP_SIZE; idx++) {
		if (level_map[idx] >= level) {
			if (level_map[idx] > level)
				idx = idx - 1;
			break;
		}
	}

	return idx;
}

#if defined(CONFIG_LCDKIT_DRIVER)
static int hisi_blpwm_bl_callback(int bl_level)
{
	if (g_bl_info.set_common_backlight)
		return g_bl_info.set_common_backlight(bl_level);

	return -EINVAL;
}
#endif

static void init_bl_info(struct hisi_panel_info *pinfo)
{
	g_bl_info.bl_max = pinfo->bl_max;
	g_bl_info.bl_min = pinfo->bl_min;
	g_bl_info.ap_brightness = 0;
	g_bl_info.last_ap_brightness = 0;

	if (pinfo->blpwm_input_precision == 0)
		pinfo->blpwm_input_precision = PWM_IN_PRECISION_DEFAULT;

	rgbw_lcd_support = pinfo->rgbw_support;
	if (rgbw_lcd_support)
		pinfo->blpwm_input_precision = PWM_IN_PRECISION_RGBW;

	pinfo->blpwm_in_num = 0xFFFF0000;
	g_bl_info.blpwm_input_precision = pinfo->blpwm_input_precision;
	g_bl_info.index_cabc_dimming = 0;

	g_bl_info.last_bl_level = 0;
	if (pinfo->blpwm_precision_type == BLPWM_PRECISION_10000_TYPE) {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION;
		outp32(hisifd_blpwm_base + BLPWM_OUT_DIV, 0);
	} else if (pinfo->blpwm_precision_type == BLPWM_PRECISION_2048_TYPE) {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION_2048;
		outp32(hisifd_blpwm_base + BLPWM_OUT_DIV, 0x2);
	} else if (pinfo->blpwm_precision_type == BLPWM_PRECISION_4096_TYPE) {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION_4096;
		outp32(hisifd_blpwm_base + BLPWM_OUT_DIV, 0x1);
	} else {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION_DEFAULT;
		outp32(hisifd_blpwm_base + BLPWM_OUT_DIV, 0x2);
	}

	if (pinfo->blpwm_out_div_value)
		outp32(hisifd_blpwm_base + BLPWM_OUT_DIV,
			pinfo->blpwm_out_div_value);

	g_bl_info.cabc_pwm = g_bl_info.blpwm_input_precision;
	g_bl_info.prev_cabc_pwm = g_bl_info.blpwm_input_precision;
	g_bl_info.current_cabc_pwm = g_bl_info.blpwm_input_precision;
	g_bl_info.cabc_pwm_in = g_bl_info.blpwm_input_precision;
	sema_init(&g_bl_info.bl_semaphore, 1);
	g_bl_info.bl_ic_ctrl_mode = pinfo->bl_ic_ctrl_mode;
	g_bl_info.blpwm_preci_no_convert = pinfo->blpwm_preci_no_convert;
}

static uint32_t get_backlight_level(uint32_t bl_level_src)
{
	uint32_t bl_level_dst = bl_level_src;

	if (!g_bl_info.blpwm_preci_no_convert)
		bl_level_dst = (bl_level_src * g_bl_info.blpwm_out_precision) /
			g_bl_info.bl_max;
	return bl_level_dst;
}

static void update_backlight(uint32_t backlight)
{
	char __iomem *blpwm_base = NULL;
	uint32_t brightness = 0;
	uint32_t bl_level = get_backlight_level(backlight);

	blpwm_base = hisifd_blpwm_base;
	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is nullptr!");
		return;
	}

#ifdef CONFIG_HUAWEI_DUBAI
	/* notify dubai module to update brightness */
	dubai_update_brightness(backlight);
#endif

	if ((g_bl_info.bl_ic_ctrl_mode >= REG_ONLY_MODE) &&
		(g_bl_info.bl_ic_ctrl_mode <= I2C_ONLY_MODE)) {
		bl_level = backlight;
		bl_level = bl_lvl_map(bl_level);

		if (g_bl_info.bl_ic_ctrl_mode == REG_ONLY_MODE)
			lm36923_set_backlight_reg(bl_level);

		return;
	}
#ifdef CONFIG_LCDKIT_DRIVER
	else if (g_bl_info.bl_ic_ctrl_mode == COMMON_IC_MODE) {
		int return_value;

		bl_level = backlight;
		switch (g_bl_config.bl_level) {
		case BL_MAX_12BIT:
			bl_level = bl_level * g_bl_config.bl_level /
				g_bl_info.bl_max;
			break;
		case BL_MAX_11BIT:
		default:
			bl_level = bl_lvl_map(bl_level);
			break;
		};

		return_value = hisi_blpwm_bl_callback(bl_level);
		if (return_value == 0)
			return;
	}
#endif
	else if (g_bl_info.bl_ic_ctrl_mode == AMOLED_NO_BL_IC_MODE) {
		HISI_DRM_INFO("bl_ic_ctrl_mode = %d",
			g_bl_info.bl_ic_ctrl_mode);
		return;
	}
	bl_level = level_map[bl_level];

	brightness = (bl_level << 16) |
		(g_bl_info.blpwm_out_precision - bl_level);
	outp32(blpwm_base + BLPWM_OUT_CFG, brightness);
}

static uint32_t get_pwm_duty(void)
{
	char __iomem *blpwm_base = NULL;

	uint32_t continue_pwm_in_num = 0;
	uint32_t continue_pwm_in_high_num = 0;
	uint32_t continue_pwm_in_low_num  = 0;
	uint32_t continue_pwm_in_duty = 0;
	uint32_t default_duty = MIN_PWMDUTY_FOR_RGBW;

	blpwm_base = hisifd_blpwm_base;

	if (pwmduty[PWMDUTY_FILTER_NUMBER-1] != 0xffff)
		default_duty = (pwmduty[PWMDUTY_FILTER_NUMBER-1] >=
			MIN_PWMDUTY_FOR_RGBW) ? pwmduty[PWMDUTY_FILTER_NUMBER-1] :
				MIN_PWMDUTY_FOR_RGBW;

	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is nullptr!");
		return default_duty;
	}

	continue_pwm_in_num = inp32(blpwm_base + PWM_IN_NUM_OFFSET);
	if (continue_pwm_in_num <= 0) {
		HISI_DRM_DEBUG("pwm_in_num is nullptr!");
		return default_duty;
	}

	continue_pwm_in_high_num = continue_pwm_in_num >> 16;
	continue_pwm_in_low_num  = continue_pwm_in_num & 0xFFFF;

	if (((continue_pwm_in_high_num + continue_pwm_in_low_num) <
		PWM_PERIOD_TOTAL_COUNTOR_JDI_SHARP) &&
		(de_info.ddic_panel_id == 1 || de_info.ddic_panel_id == 3)) {
		HISI_DRM_DEBUG("pwm_in_num is abnormal!\n");
		return default_duty;
	}

	if (((continue_pwm_in_high_num + continue_pwm_in_low_num) <
		PWM_PERIOD_TOTAL_COUNTOR_LGD) && (de_info.ddic_panel_id == 2)) {
		HISI_DRM_DEBUG("pwm_in_num is abnormal!\n");
		return default_duty;
	}

	continue_pwm_in_duty = g_bl_info.blpwm_input_precision *
	continue_pwm_in_high_num /
		(continue_pwm_in_high_num + continue_pwm_in_low_num);

	if (continue_pwm_in_duty < MIN_PWMDUTY_FOR_RGBW)
		continue_pwm_in_duty = MIN_PWMDUTY_FOR_RGBW;

	return continue_pwm_in_duty;
}

static void get_ap_dimming_to_update_backlight(
	struct hisi_panel_info *pinfo)
{
	int32_t delta_cabc_pwm;
	int32_t pwm_duty;
	int32_t backlight;

	if (g_bl_info.index_cabc_dimming > CABC_DIMMING_STEP_TOTAL_NUM) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		g_bl_info.index_cabc_dimming = 1;
	} else {
		down(&g_bl_info.bl_semaphore);
		if (g_bl_info.cabc_pwm_in != 0) {
			g_bl_info.cabc_pwm = g_bl_info.cabc_pwm_in;
			g_bl_info.cabc_pwm_in = 0;
			g_bl_info.index_cabc_dimming = 1;
			g_bl_info.prev_cabc_pwm = g_bl_info.current_cabc_pwm;
		}
		delta_cabc_pwm = g_bl_info.cabc_pwm - g_bl_info.prev_cabc_pwm;
		pwm_duty = delta_cabc_pwm * g_bl_info.index_cabc_dimming/32 +
		delta_cabc_pwm * g_bl_info.index_cabc_dimming % 32 / 16;

		g_bl_info.current_cabc_pwm = g_bl_info.prev_cabc_pwm + pwm_duty;
		backlight = g_bl_info.current_cabc_pwm * g_bl_info.ap_brightness /
		g_bl_info.blpwm_input_precision;
		if (backlight > 0 && backlight < g_bl_info.bl_min)
			backlight = g_bl_info.bl_min;

		if (g_bl_info.ap_brightness != 0 &&
			backlight != g_bl_info.last_bl_level) {
#ifndef CONFIG_LCD_KIT_DRIVER
			update_backlight(backlight);
#endif
			g_bl_info.last_bl_level = backlight;
		}

		g_bl_info.index_cabc_dimming++;
		up(&g_bl_info.bl_semaphore);

		msleep(16);
	}
}

static int is_pwm_dimming_stop(int pwm_duty)
{
	int i;
	int delta_pwmduty;
	int dimming_stop_counter = 0;
	static int dimming_stop_flag;

	if (rgbw_bl_stop_threshold_num < RGBW_BL_STOP_THRESHOLD)
		return 0;

	for (i = 0; i < PWMDUTY_FILTER_NUMBER; i++) {
		if (i < (PWMDUTY_FILTER_NUMBER - 1))
			pwmduty[i] = pwmduty[i+1];

		if (i == (PWMDUTY_FILTER_NUMBER - 1))
			pwmduty[i] = pwm_duty;
	}

	for (i = 0; i < (PWMDUTY_FILTER_NUMBER - 1); i++) {
		delta_pwmduty = pwmduty[i+1] - pwmduty[i];
		if (delta_pwmduty <= PWM_IN_SENSITY && delta_pwmduty >= (-PWM_IN_SENSITY))
			dimming_stop_counter++;
	}

	dimming_stop_flag = (dimming_stop_counter == (PWMDUTY_FILTER_NUMBER-1)) ? 1 : 0;
	if (dimming_stop_flag == 1)
		rgbw_bl_stop_threshold_num = 0;

	return dimming_stop_flag;
}

static void reset_pwm_buf(uint32_t value)
{
	int i;

	for (i = 0; i < PWMDUTY_FILTER_NUMBER; i++)
		pwmduty[i] = value;

	for (i = 0; i < BACKLIGHT_FILTER_NUMBER; i++)
		backlight_buf[i] = value;
}

#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
static void set_rgbw_lg(struct hisi_fb_data_type *hisifd)
{
	int backlight_indoor_lgd;
	int rgbw_lg_gfl = RGBW_LG_RGBW * 4096 / RGBW_LG_RGB;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_DRM_ERR("hisifd is null!\n");
		return;
	}

	pinfo = &(g_panel_info);

	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is null!\n");
		return;
	}

	backlight_indoor_lgd = (int)pinfo->bl_max * RGBW_LG_RGB / RGBW_LG_RGBW;
	if (backlight_indoor_lgd == 0) {
		HISI_DRM_ERR("backlight_indoor_lgd is err!\n");
		return;
	}
	if (de_info.ddic_rgbw_backlight < backlight_indoor_lgd) {
		de_info.frame_gain_limit = rgbw_lg_gfl;
		de_info.color_distortion_allowance = RGBW_LG_CDA;
		de_info.pixel_gain_limit = 0;
		de_info.pwm_duty_gain = RGBW_LG_BIT;
	} else {
		de_info.frame_gain_limit = (int)pinfo->bl_max * RGBW_LG_BIT /
			de_info.ddic_rgbw_backlight;
		de_info.color_distortion_allowance = (de_info.ddic_rgbw_backlight - backlight_indoor_lgd) *
			RGBW_LG_CDA_OUTDOOR / backlight_indoor_lgd + RGBW_LG_CDA;
		de_info.pixel_gain_limit = rgbw_lg_gfl - de_info.frame_gain_limit;
		de_info.pwm_duty_gain = RGBW_LG_BIT;
	}
}

static void set_rgbw_lg_hma(struct hisi_fb_data_type *hisifd)
{
	int backlight_indoor_lgd = 0;
	int rgbw_lg_gfl = RGBW_LG_RGBW_HMA * 4096 / RGBW_LG_RGB_HMA;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_DRM_ERR("hisifd is null!\n");
		return;
	}

	pinfo = &(g_panel_info);

	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is null!\n");
		return;
	}

	backlight_indoor_lgd = (int)pinfo->bl_max * RGBW_LG_RGB_HMA / RGBW_LG_RGBW_HMA;
	if (backlight_indoor_lgd == 0) {
		HISI_DRM_ERR("backlight_indoor_lgd is err!\n");
		return;
	}
	if (de_info.ddic_rgbw_backlight < backlight_indoor_lgd) {
		de_info.frame_gain_limit = rgbw_lg_gfl;
		de_info.color_distortion_allowance = RGBW_LG_CDA;
		de_info.pixel_gain_limit = 0;
		de_info.pwm_duty_gain = RGBW_LG_BIT;
	} else {
		de_info.frame_gain_limit = (int)pinfo->bl_max * RGBW_LG_BIT /
			de_info.ddic_rgbw_backlight;
		de_info.color_distortion_allowance = (de_info.ddic_rgbw_backlight - backlight_indoor_lgd) *
			RGBW_LG_CDA_OUTDOOR / backlight_indoor_lgd + RGBW_LG_CDA;
		de_info.pixel_gain_limit = rgbw_lg_gfl - de_info.frame_gain_limit;
		de_info.pwm_duty_gain = RGBW_LG_BIT;
	}
}

static void set_rgbw_boe_hma(struct hisi_fb_data_type *hisifd)
{
	int backlight_indoor_boe;
	struct hisi_panel_info *pinfo = NULL;
	int rgbw_lg_gfl = RGBW_BOE_RGBW_HMA * 4096 / RGBW_BOE_RGB_HMA;

	if (hisifd == NULL) {
		HISI_DRM_ERR("hisifd is null!\n");
		return;
	}

	pinfo = &(g_panel_info);

	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is null!\n");
		return;
	}

	backlight_indoor_boe = (int)pinfo->bl_max * RGBW_BOE_RGB_HMA / RGBW_BOE_RGBW_HMA;
	if (backlight_indoor_boe == 0) {
		HISI_DRM_ERR("backlight_indoor_lgd is err!\n");
		return;
	}
	de_info.frame_gain_limit = RGBW_BOE_FGL_HMA;
	de_info.pwm_duty_gain = RGBW_BOE_PDG_HMA;
	if (de_info.ddic_rgbw_backlight < backlight_indoor_boe) {
		de_info.color_distortion_allowance = 0;
		de_info.pixel_gain_limit = 0;
		de_info.rgbw_total_glim = RGBW_BOE_TGL_HMA;
	} else {
		de_info.color_distortion_allowance = RGBW_BOE_CDA_HMA *
			(de_info.ddic_rgbw_backlight - backlight_indoor_boe) /
			((int)pinfo->bl_max - backlight_indoor_boe);
		de_info.rgbw_total_glim = de_info.ddic_rgbw_backlight * RGBW_BOE_TGL_HMA /
			backlight_indoor_boe;
		de_info.pixel_gain_limit = de_info.rgbw_total_glim - RGBW_BOE_TGL_HMA;
	}
}
#endif

static void set_backlight_by_de_info(int32_t *backlight)
{
	if (de_info.ddic_panel_id) {
		switch (de_info.ddic_panel_id) {
#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
		case JDI_NT36860C_PANEL_ID:
		case SHARP_NT36870_PANEL_ID:
		case JDI_HX83112C_PANLE_ID:
		case SHARP_HX83112C_PANEL_ID:
		case JDI_TD4336_PANEL_ID:
		case SHARP_TD4336_PANEL_ID:
		case JDI_TD4336_HMA_PANEL_ID:
		case SHARP_TD4336_HMA_PANEL_ID:
			if (g_bl_info.blpwm_input_precision > 0)
				*backlight = ((int32_t)g_panel_info.bl_max) *
					g_bl_info.cabc_pwm_in / g_bl_info.blpwm_input_precision;
			else
				*backlight = g_bl_info.ap_brightness;
			break;
		case LG_NT36870_PANEL_ID:
		case LG_NT36772A_PANEL_ID:
			if (g_bl_info.blpwm_input_precision > 0)
				*backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW /
					RGBW_LG_RGB, (int32_t)g_panel_info.bl_max) *
					g_bl_info.cabc_pwm_in / g_bl_info.blpwm_input_precision; /* for LG */
			else
				*backlight = g_bl_info.ap_brightness;
			set_rgbw_lg(hisifd);
			break;
		case LG_NT36772A_HMA_PANEL_ID:
			if (g_bl_info.blpwm_input_precision > 0)
				*backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW_HMA /
					RGBW_LG_RGB_HMA, (int32_t)g_panel_info.bl_max) *
					g_bl_info.cabc_pwm_in / g_bl_info.blpwm_input_precision; /* for LG HMA */
			else
				*backlight = g_bl_info.ap_brightness;
			set_rgbw_lg_hma(hisifd);
			break;
		case BOE_HX83112E_HMA_PANEL_ID:
			if (g_bl_info.blpwm_input_precision > 0)
				*backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW_HMA /
					RGBW_LG_RGB_HMA * g_bl_info.cabc_pwm_in /
					g_bl_info.blpwm_input_precision,
						(int32_t)g_panel_info.bl_max); /* for BOE HMA */
			else
				*backlight = g_bl_info.ap_brightness;
			set_rgbw_boe_hma(hisifd);
			break;
#endif
		default:
			*backlight = g_bl_info.ap_brightness;
			break;
		}
	} else {
		*backlight = g_bl_info.ap_brightness;
	}
	last_backlight = *backlight;
}

static int calc_backlight(int32_t pwm_duty)
{
	int32_t backlight;
	int32_t delta_pwm_duty;
	int32_t delta_pwm_duty_abs;

	delta_pwm_duty = pwm_duty - last_pwm_duty;
	delta_pwm_duty_abs = abs(delta_pwm_duty);

	if ((delta_pwm_duty_abs <= 1) || ((delta_pwm_duty_abs == 3) &&
		(last_delta_pwm_duty_abs == 1))) {
		g_bl_info.cabc_pwm_in = last_pwm_duty;
	} else {
		if (pwm_duty > last_pwm_duty)
			last_pwm_duty = pwm_duty - 1;
		else
			last_pwm_duty = pwm_duty + 1;
		g_bl_info.cabc_pwm_in = last_pwm_duty;
	}
	last_delta_pwm_duty_abs = delta_pwm_duty_abs;

	if (g_bl_info.cabc_pwm_in > g_bl_info.blpwm_input_precision)
		g_bl_info.cabc_pwm_in = g_bl_info.blpwm_input_precision;
	else if (g_bl_info.cabc_pwm_in < 0)
		g_bl_info.cabc_pwm_in = 0;

	if ((g_bl_info.last_ap_brightness == g_bl_info.ap_brightness) &&
		(g_bl_info.ap_brightness < RGBW_LOW_LIGHT_THRESHOLD))
		low_light_count++;
	else
		low_light_count = 0; /* clear low light count */

	if (low_light_count > RGBW_LOW_LIGHT_STOP)
		backlight = last_backlight;
	else
		set_backlight_by_de_info(&backlight);

	return backlight;
}

static int get_smooth_backlight(int32_t backlight)
{
	int i;
	int j = 0;
	int sum_backlight = 0;

	backlight_buf[g_count % BACKLIGHT_FILTER_NUMBER] = backlight;
	g_count++;

	for (i = 0; i < BACKLIGHT_FILTER_NUMBER; i++) {
		if (backlight_buf[i] == 0xffff)
			continue;

		sum_backlight += backlight_buf[i];
		j++;
	}

	if (j != 0)
		backlight = (sum_backlight + j / 2) / j;

	return backlight;
}

static void get_rgbw_pwmduty_to_update_backlight(
	struct hisi_panel_info *pinfo)
{
	int32_t backlight = 0;
	int dimming_stop;
	int temp_current_pwm_duty = 0;

	if ((g_bl_info.last_ap_brightness == 0 &&
		g_bl_info.ap_brightness != 0)) {
		reset_pwm_buf(0xffff);
		last_pwm_duty = MIN_PWMDUTY_FOR_RGBW;
		last_delta_pwm_duty_abs = 0;
	}

	if (g_bl_info.ap_brightness != 0)
		temp_current_pwm_duty = get_pwm_duty();

	rgbw_bl_stop_threshold_num++;
	dimming_stop = is_pwm_dimming_stop(temp_current_pwm_duty);

	if ((g_bl_info.last_ap_brightness == g_bl_info.ap_brightness) &&
		(dimming_stop == 1)) {
		down(&g_bl_info.bl_semaphore);
		if (g_blpwm_thread_stop) {
			msleep(5);
			up(&g_bl_info.bl_semaphore);
			return;
		}

		set_current_state(TASK_INTERRUPTIBLE);
		up(&g_bl_info.bl_semaphore);
		schedule();
	} else {
		down(&g_bl_info.bl_semaphore);

		backlight = calc_backlight(temp_current_pwm_duty);

		/* smooth filter for backlight */
		backlight = get_smooth_backlight(backlight);

		if (backlight > 0 && backlight < g_bl_info.bl_min)
			backlight = g_bl_info.bl_min;

		g_bl_info.last_ap_brightness = g_bl_info.ap_brightness;

		if (g_bl_info.ap_brightness != 0 && backlight != g_bl_info.last_bl_level) {
#ifndef CONFIG_LCD_KIT_DRIVER
			update_backlight(backlight);
#endif
			g_bl_info.last_bl_level = backlight;
		}
		msleep(12);
		up(&g_bl_info.bl_semaphore);
	}

	if (g_bl_info.ap_brightness == 0)
		reset_pwm_buf(0xffff);

	mutex_lock(&g_rgbw_lock);
	de_info.ddic_rgbw_backlight = MIN(MAX((de_info.blc_enable ? de_info.blc_delta : 0) +
		g_bl_info.last_bl_level, (int)pinfo->bl_min), (int)pinfo->bl_max);

	mutex_unlock(&g_rgbw_lock);
}

static int cabc_pwm_thread(void *p)
{
	struct hisi_panel_info *pinfo = NULL;

	pinfo = (struct hisi_panel_info *)p;
	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is nullptr!");
		return -EINVAL;
	}

	while (!kthread_should_stop()) {
		if (rgbw_lcd_support)
			get_rgbw_pwmduty_to_update_backlight(pinfo);
		else
			get_ap_dimming_to_update_backlight(pinfo);
	}

	return 0;
}

static void hisi_cabc_set_backlight(uint32_t cabc_pwm_in)
{
	if (cabc_pwm_task == NULL)
		return;

	if (g_bl_info.ap_brightness == 0) {
		g_bl_info.current_cabc_pwm = cabc_pwm_in;
		return;
	}

	if (!rgbw_lcd_support)
		g_bl_info.cabc_pwm_in = cabc_pwm_in;

	wake_up_process(cabc_pwm_task);
}

static int hisi_blpwm_input_enable(struct hisi_panel_info *pinfo)
{
	char __iomem *blpwm_base = NULL;

	blpwm_base = hisifd_blpwm_base;
	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is nullptr!");
		return -EINVAL;
	}

	outp32(blpwm_base + PWM_IN_CTRL_OFFSET, 1);
	if (rgbw_lcd_support) {
		outp32(blpwm_base + PWM_IN_DIV_OFFSET, 0x01);
		outp32(blpwm_base + PWM_IN_MAX_COUNTER, PWM_MAX_COUNTER_FOR_RGBW_JDI_SHARP);
	} else {
		outp32(blpwm_base + PWM_IN_DIV_OFFSET, 0x02);
	}

	cabc_pwm_task = kthread_create(cabc_pwm_thread, pinfo, "cabc_pwm_task");
	if (IS_ERR(cabc_pwm_task)) {
		HISI_DRM_ERR("Unable to start kernel cabc_pwm_task!");
		cabc_pwm_task = NULL;
		return -EINVAL;
	}

	if (rgbw_lcd_support)
		g_blpwm_thread_stop = false;

	return 0;
}

static int hisi_blpwm_input_disable(void)
{
	char __iomem *blpwm_base = NULL;

	if (cabc_pwm_task) {
		if (rgbw_lcd_support) {
			down(&g_bl_info.bl_semaphore);
			g_blpwm_thread_stop = true;
			up(&g_bl_info.bl_semaphore);
		}

		kthread_stop(cabc_pwm_task);
		cabc_pwm_task = NULL;
	}

	blpwm_base = hisifd_blpwm_base;
	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is null!");
		return -EINVAL;
	}

	outp32(blpwm_base + PWM_IN_CTRL_OFFSET, 0);

	return 0;
}

static uint32_t set_bl_lvl(struct hisi_panel_info *pinfo, uint32_t bl_level)
{
	if (pinfo->blpwm_input_ena && pinfo->blpwm_in_num) {
		if (bl_level > 0) {
			bl_level = bl_level * g_bl_info.current_cabc_pwm /
				pinfo->blpwm_input_precision;
			bl_level = bl_level < g_bl_info.bl_min ? g_bl_info.bl_min : bl_level;
		}

		g_bl_info.last_bl_level = bl_level;
	}
	HISI_DRM_INFO("+1");
#ifdef CONFIG_HUAWEI_DUBAI
	/* notify dubai module to update brightness */
	dubai_update_brightness(bl_level);
#endif

	if ((g_bl_info.bl_ic_ctrl_mode >= REG_ONLY_MODE) &&
		(g_bl_info.bl_ic_ctrl_mode <= I2C_ONLY_MODE)) {
		bl_level = bl_lvl_map(bl_level);

		if (pinfo->bl_ic_ctrl_mode == REG_ONLY_MODE)
			lm36923_set_backlight_reg(bl_level);

		return 0;
	}

	return bl_level;

}

void hisi_blpwm_set_brightness(uint32_t brightness)
{
	char __iomem *blpwm_base = NULL;

	HISI_DRM_INFO("+");

	blpwm_base = hisifd_blpwm_base;
	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is nullptr!");
		return;
	}

	outp32(blpwm_base + BLPWM_OUT_CFG, brightness);

	HISI_DRM_INFO("-");
}

int hisi_blpwm_set_backlight(
	struct hisi_panel_info *pinfo,
	uint32_t bl_level)
{
	uint32_t brightness;

	HISI_DRM_INFO("+");
	if (!pinfo) {
		HISI_DRM_ERR("pinfo is nullptr!");
		return -EINVAL;
	}

	if (g_blpwm_on == 0) {
		HISI_DRM_ERR("blpwm is not on, return!");
		return 0;
	}

	HISI_DRM_INFO("bl_level=%d", bl_level);

	if (pinfo->bl_max < 1) {
		HISI_DRM_ERR("bl_max %d is out of range!!", pinfo->bl_max);
		return -EINVAL;
	}

	if (bl_level > pinfo->bl_max)
		bl_level = pinfo->bl_max;

	if (rgbw_lcd_support)
		g_bl_info.last_ap_brightness = g_bl_info.ap_brightness;

	/* allow backlight zero */
	if (bl_level < pinfo->bl_min && bl_level)
		bl_level = pinfo->bl_min;

	down(&g_bl_info.bl_semaphore);
	g_bl_info.ap_brightness = bl_level;

	if (bl_level && rgbw_lcd_support && pinfo->blpwm_input_ena) {
		if (g_bl_info.last_ap_brightness  != g_bl_info.ap_brightness)
			hisi_cabc_set_backlight(g_bl_info.current_cabc_pwm);

		up(&g_bl_info.bl_semaphore);
		return 0;
	}

	bl_level = set_bl_lvl(pinfo, bl_level);

	bl_level = get_backlight_level(bl_level);
	bl_level = level_map[bl_level];

	brightness = (bl_level << 16) | (g_bl_info.blpwm_out_precision - bl_level);
	hisi_blpwm_set_brightness(brightness);

	up(&g_bl_info.bl_semaphore);

	HISI_DRM_INFO("-");

	return 0;
}

int hisi_blpwm_on(struct hisi_panel_info *pinfo)
{
	int ret;
	struct clk *clk_tmp = NULL;
	char __iomem *blpwm_base = NULL;

#ifdef CONFIG_HISI_DSS_V600
	return 0;
#endif
	HISI_DRM_INFO("+");

	if (!pinfo) {
		HISI_DRM_ERR("pinfo is nullptr!");
		return -EINVAL;
	}

	blpwm_base = hisifd_blpwm_base;
	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is nullptr!");
		return -EINVAL;
	}

	HISI_DRM_INFO("blpwm_base:%lx\n", blpwm_base);
	if (g_blpwm_on == 1)
		return 0;

	clk_tmp = g_dss_blpwm_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_DRM_ERR("dss_blpwm_clk clk_prepare failed, error=%d!", ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_DRM_ERR("dss_blpwm_clk clk_enable failed, error=%d!", ret);
			return -EINVAL;
		}
	}

	ret = pinctrl_cmds_tx(&g_blpwm_pdev->dev, blpwm_pinctrl_normal_cmds,
		ARRAY_SIZE(blpwm_pinctrl_normal_cmds));
	if (ret)
		HISI_DRM_ERR("failed to pinctrl_cmds_tx, ret=%d!", ret);

	HISI_DRM_INFO("++");

	/* if enable BLPWM, please set IOMG_003, IOMG_004 in IOC_AO module */
	/* set IOMG_003: select BLPWM_CABC */
	/* set IOMG_004: select BLPWM_BL */
	outp32(blpwm_base + BLPWM_OUT_CTRL, 0x1);
	init_bl_info(pinfo);

	HISI_DRM_INFO("+++");
	if (pinfo->blpwm_input_ena)
		hisi_blpwm_input_enable(pinfo);

	g_blpwm_on = 1;

	HISI_DRM_INFO("-");

	return ret;
}

int hisi_blpwm_off(struct hisi_panel_info *pinfo)
{
	int ret;
	struct clk *clk_tmp = NULL;
	char __iomem *blpwm_base = NULL;

	HISI_DRM_INFO("+");

	if (!pinfo) {
		HISI_DRM_ERR("pinfo is nullptr!");
		return -EINVAL;
	}

	blpwm_base = hisifd_blpwm_base;
	if (!blpwm_base) {
		HISI_DRM_ERR("blpwm_base is nullptr!");
		return -EINVAL;
	}

	if (g_blpwm_on == 0)
		return 0;

	outp32(blpwm_base + BLPWM_OUT_CTRL, 0x0);

	ret = pinctrl_cmds_tx(&g_blpwm_pdev->dev, blpwm_pinctrl_lowpower_cmds,
		ARRAY_SIZE(blpwm_pinctrl_lowpower_cmds));
	gpio_direction_output(BL_PWM_GPIO, 0);
	if (ret)
		HISI_DRM_ERR("failed to pinctrl_cmds_tx, ret=%d!", ret);

	clk_tmp = g_dss_blpwm_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	if (pinfo->blpwm_input_ena)
		hisi_blpwm_input_disable();

	g_blpwm_on = 0;

	HISI_DRM_INFO("-");

	return ret;
}

static int hisi_blpwm_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (!pdev) {
		HISI_DRM_ERR("pdev is nullptr!");
		return -EINVAL;
	}

	mutex_init(&g_rgbw_lock);
	g_blpwm_pdev = pdev;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_BLPWM_NAME);
	if (!np) {
		HISI_DRM_ERR("NOT FOUND device node %s!", DTS_COMP_BLPWM_NAME);
		return -ENXIO;
	}

	/* get blpwm reg base */
	hisifd_blpwm_base = of_iomap(np, 0);
	if (!hisifd_blpwm_base) {
		HISI_DRM_ERR("failed to get blpwm_base resource!");
		return -ENXIO;
	}
	HISI_DRM_INFO("0-hisifd_blpwm_base:%lx\n", hisifd_blpwm_base);

	ret = of_property_read_u32(np, "fpga_flag", &g_blpwm_fpga_flag);
	if (ret) {
		HISI_DRM_ERR("failed to get fpga_flag resource!");
		return -ENXIO;
	}

	if (g_blpwm_fpga_flag == 0) {
		/* blpwm pinctrl init */
		ret = pinctrl_cmds_tx(&pdev->dev, blpwm_pinctrl_init_cmds,
			ARRAY_SIZE(blpwm_pinctrl_init_cmds));
		if (ret != 0) {
			HISI_DRM_ERR("Init blpwm pinctrl failed, ret=%d!", ret);
			return ret;
		}

		/* get blpwm clk resource */
		g_dss_blpwm_clk = of_clk_get(np, 0);
		if (IS_ERR(g_dss_blpwm_clk)) {
			HISI_DRM_ERR("%s clock not found!", np->name);
			return -ENXIO;
		}

		HISI_DRM_INFO("dss_blpwm_clk:[%lu]->[%lu].\n",
			DEFAULT_PWM_CLK_RATE, clk_get_rate(g_dss_blpwm_clk));
	}

	HISI_DRM_INFO("-");

	return 0;
}

static int hisi_blpwm_remove(struct platform_device *pdev)
{
	struct clk *clk_tmp = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (!pdev) {
		HISI_DRM_ERR("pdev is nullptr!");
		return -EINVAL;
	}

	ret = pinctrl_cmds_tx(&pdev->dev, blpwm_pinctrl_finit_cmds,
		ARRAY_SIZE(blpwm_pinctrl_finit_cmds));
	if (ret)
		HISI_DRM_ERR("failed to pinctrl_cmds_tx, ret=%d!", ret);

	clk_tmp = g_dss_blpwm_clk;
	if (clk_tmp) {
		clk_put(clk_tmp);
		clk_tmp = NULL;
	}

	mutex_destroy(&g_rgbw_lock);

	HISI_DRM_INFO("-");

	return ret;
}

static const struct of_device_id hisi_blpwm_match_table[] = {
	{
		.compatible = DTS_COMP_BLPWM_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_blpwm_match_table);

static struct platform_driver this_driver = {
	.probe = hisi_blpwm_probe,
	.remove = hisi_blpwm_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_BLPWM,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_blpwm_match_table),
	},
};

static int __init hisi_blpwm_init(void)
{
	int ret = 0;

	HISI_DRM_INFO("+");

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_DRM_ERR("platform_driver_register failed, ret=%d!", ret);
		return ret;
	}

	HISI_DRM_INFO("-");

	return ret;
}


module_init(hisi_blpwm_init);

/*lint +e86 +e446 +e529 +e559 +e574 +e666*/
