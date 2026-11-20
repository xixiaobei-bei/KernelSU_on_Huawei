/*
 * hisi_blpwm.c
 *
 * backlight for display functions
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#include "hisi_fb.h"
#include "backlight_linear_to_exponential.h"
#include "backlight/lm36923.h"
#include "backlight/lp8556.h"
#include "backlight/rt8555.h"
#include "hisi_fb_panel.h"
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/device.h>
#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_panel.h"
#include "lcdkit_backlight_ic_common.h"
#endif



#ifdef CONFIG_HUAWEI_DUBAI
#include <chipset_common/dubai/dubai.h>
#endif

/* default pwm clk */
#define DEFAULT_PWM_CLK_RATE (114 * 1000000L)
static char __iomem *g_dpufd_blpwm_base;
static struct clk *g_dss_blpwm_clk;
static struct platform_device *g_blpwm_pdev;
static int g_blpwm_on;
static int g_blpwm_fpga_flag;
static bool g_blpwm_thread_stop;

static struct pinctrl_data blpwmpctrl;

static struct pinctrl_cmd_desc blpwm_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &blpwmpctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &blpwmpctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &blpwmpctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc blpwm_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &blpwmpctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc blpwm_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &blpwmpctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc blpwm_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &blpwmpctrl, 0},
};

struct panel_calc_backlight {
	int panel_id;
	int (*get_calc_bkl)(struct dpu_fb_data_type *dpufd);
};

struct pwm {
	uint32_t pwm_in_num;
	uint32_t pwm_in_high_num;
	uint32_t pwm_in_low_num;
	uint32_t pwm_in_duty;
	uint32_t pre_pwm_in_num;
	uint32_t pre_pwm_in_high_num;
	uint32_t pre_pwm_in_low_num;
	uint32_t pre_pwm_in_duty;
};

#define PANEL_ID_DEFAULT 255

#define BLPWM_OUT_CTRL 0x100
#define BLPWM_OUT_DIV 0x104
#define BLPWM_OUT_CFG 0x108

#define BLPWM_OUT_PRECISION_DEFAULT 800
#define BLPWM_OUT_PRECISION 10000
#define BLPWM_OUT_PRECISION_2048 2047
#define BLPWM_OUT_PRECISION_4096 4095
#define BLPWM_BL_LEVEL_MIN 4

/* BLPWM input address */
#define PWM_IN_CTRL_OFFSET 0x000
#define PWM_IN_DIV_OFFSET 0x004
#define PWM_IN_NUM_OFFSET 0x008
#define PWM_IN_MAX_COUNTER 0x010
#define PWM_IN_PRECISION_DEFAULT 512
#define PWM_IN_PRECISION_RGBW 8192
#define PWM_IN_SENSITY 2

#define CABC_DIMMING_STEP_TOTAL_NUM 32
#define RGBW_DIMMING_STEP_TOTAL_NUM 3
static struct task_struct *cabc_pwm_task;

static uint8_t g_rgbw_lcd_support;
#define PWM_PERIOD_TOTAL_COUNTOR_JDI_SHARP 4000
#define PWM_PERIOD_TOTAL_COUNTOR_LGD 1000
#define PWM_MAX_COUNTER_FOR_RGBW_JDI_SHARP 4500
#define PWMDUTY_FILTER_NUMBER 20
#define HIGHLEVEL_PWMDUTY_FILTER_NUMBER 3
#define BRIGHTNESS_FILTER_FOR_PWMDUTY 100

static int pwmduty[PWMDUTY_FILTER_NUMBER] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff};

#define BACKLIGHT_FILTER_NUMBER 20
static int backlight_buf[BACKLIGHT_FILTER_NUMBER] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff};

#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
static unsigned int g_backlight_buf_count;
#endif

#define MIN_PWMDUTY_FOR_RGBW 13
#define RGBW_BL_STOP_THRESHOLD 200
#define RGBW_LOW_LIGHT_THRESHOLD 35
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
#define RGBW_JDI_RGB_HMA 500
#define RGBW_JDI_RGBW_HMA 858
#define RGBW_JDI_PIXEL_MAX_LIMIT 128

static uint32_t g_rgbw_bl_stop_threshold_num;
static int32_t g_low_light_count;
static int32_t g_last_backlight;
static int32_t g_last_pwm_duty = MIN_PWMDUTY_FOR_RGBW;
static int32_t g_last_delta_pwm_duty_abs;

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
#endif

static struct bl_info g_bl_info;
#define BL_LVL_MAP_SIZE 2047
#define BL_MAX_11BIT 2047
#define BL_MAX_12BIT 4095
#define BL_MAX_10BIT 1023
#define BL_MAX_16BIT 65535

int bl_lvl_map(int level)
{
	uint32_t low = 0;
	uint32_t high = BL_LVL_MAP_SIZE;
	int mid;

	if (level < 0 || level > 10000) {  /* level at 0~10000 is valid data */
		DPU_FB_ERR("Need Valid Data! level = %d", level);
		return 0;
	}

	while (low < high) {
		mid = (low + high) >> 1;  /* '>>1'  same as  '/2' */
		if (level > level_map[mid]) {
			low = mid + 1;
		} else if (level < level_map[mid]) {
			high = mid - 1;
		} else {
			while ((mid > 0) &&
				(level_map[mid - 1] == level_map[mid]))
				mid--;
			return mid;
		}
	}
	while (low > 0 && level_map[low] > level)
		low--;

	return low;
}

void hisi_blpwm_bl_regisiter(int (*set_bl)(int bl_level))
{
	g_bl_info.set_common_backlight = set_bl;
}

int hisi_blpwm_bl_callback(int bl_level)
{
	if (g_bl_info.set_common_backlight != NULL)
		return g_bl_info.set_common_backlight(bl_level);

	return -1;
}

int32_t bl_config_max_value(void)
{
	return g_bl_info.bl_max;
}

static void init_bl_info(struct dpu_panel_info *pinfo)
{
	if (!pinfo) {
		DPU_FB_ERR("pinfo is null pointer\n");
		return;
	}

	g_bl_info.bl_max = pinfo->bl_max;
	g_bl_info.bl_min = pinfo->bl_min;
	g_bl_info.ap_brightness = 0;
	g_bl_info.last_ap_brightness = 0;

	if (pinfo->blpwm_input_precision == 0)
		pinfo->blpwm_input_precision = PWM_IN_PRECISION_DEFAULT;

	g_rgbw_lcd_support = pinfo->rgbw_support;
	if (g_rgbw_lcd_support)
		pinfo->blpwm_input_precision = PWM_IN_PRECISION_RGBW;

	pinfo->blpwm_in_num = 0xFFFF0000;
	g_bl_info.blpwm_input_precision = pinfo->blpwm_input_precision;
	g_bl_info.index_cabc_dimming = 0;

	g_bl_info.last_bl_level = 0;
	if (pinfo->blpwm_precision_type == BLPWM_PRECISION_10000_TYPE) {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION;
		outp32(g_dpufd_blpwm_base + BLPWM_OUT_DIV, 0);
	} else if (pinfo->blpwm_precision_type == BLPWM_PRECISION_2048_TYPE) {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION_2048;
		outp32(g_dpufd_blpwm_base + BLPWM_OUT_DIV, 0x2);
	} else if (pinfo->blpwm_precision_type == BLPWM_PRECISION_4096_TYPE) {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION_4096;
		outp32(g_dpufd_blpwm_base + BLPWM_OUT_DIV, 0x1);
	} else {
		g_bl_info.blpwm_out_precision = BLPWM_OUT_PRECISION_DEFAULT;
		outp32(g_dpufd_blpwm_base + BLPWM_OUT_DIV, 0x2);
	}
	if (pinfo->blpwm_out_div_value)
		outp32(g_dpufd_blpwm_base + BLPWM_OUT_DIV, pinfo->blpwm_out_div_value);

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

	if (!g_bl_info.blpwm_preci_no_convert && g_bl_info.bl_max)
		bl_level_dst = (bl_level_src * g_bl_info.blpwm_out_precision) / g_bl_info.bl_max;

	DPU_FB_DEBUG("bl_level_dst=%d, bl_level_src=%d, blpwm_out_precision=%d, "
		"bl_max=%d, blpwm_preci_no_convert = %d\n",
		bl_level_dst, bl_level_src, g_bl_info.blpwm_out_precision,
		g_bl_info.bl_max, g_bl_info.blpwm_preci_no_convert);

	return bl_level_dst;
}

#ifdef CONFIG_LCDKIT_DRIVER
static int update_backlight_accord_common_ic_mode(uint32_t *bl_level, uint32_t backlight)
{
	int return_value;

	*bl_level = backlight;
	switch (g_bl_config.bl_level) {
	case BL_MAX_12BIT:
		*bl_level = *bl_level * g_bl_config.bl_level / g_bl_info.bl_max;
		break;
	case BL_MAX_11BIT:
	default:
		*bl_level = bl_lvl_map(*bl_level);
		break;
	}

	return_value = hisi_blpwm_bl_callback(*bl_level);

	return return_value;
}
#endif

static void update_backlight(struct dpu_fb_data_type *dpufd, uint32_t backlight)
{
	char __iomem *blpwm_base = NULL;
	uint32_t brightness;
	uint32_t bl_level = get_backlight_level(backlight);

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");

	if (dpufd->online_play_count < BACKLIGHT_LOG_PRINTF)
		DPU_FB_INFO("cabc8:bl_level=%d, backlight=%d, blpwm_out_precision=%d, bl_max=%d\n",
			bl_level, backlight, g_bl_info.blpwm_out_precision, g_bl_info.bl_max);
	else
		DPU_FB_DEBUG("cabc8:bl_level=%d, backlight=%d, blpwm_out_precision=%d, bl_max=%d\n",
			bl_level, backlight, g_bl_info.blpwm_out_precision, g_bl_info.bl_max);

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_no_retval(!blpwm_base, ERR, "blpwm_base is NULL!\n");

#ifdef CONFIG_HUAWEI_DUBAI
	/* notify dubai module to update brightness */
	dubai_update_brightness(backlight);
#endif

	if ((g_bl_info.bl_ic_ctrl_mode >= REG_ONLY_MODE) && (g_bl_info.bl_ic_ctrl_mode <= I2C_ONLY_MODE)) {
		bl_level = backlight;
		bl_level = bl_lvl_map(bl_level);
		DPU_FB_DEBUG("cabc9:bl_level=%d\n", bl_level);
		if (g_bl_info.bl_ic_ctrl_mode == REG_ONLY_MODE)
			lm36923_set_backlight_reg(bl_level);
		else if (g_bl_info.bl_ic_ctrl_mode == I2C_ONLY_MODE)
		return;
	}
#ifdef CONFIG_LCDKIT_DRIVER
	else if (g_bl_info.bl_ic_ctrl_mode == COMMON_IC_MODE) {
		if (update_backlight_accord_common_ic_mode(&bl_level, backlight) == 0)
			return;
	}
#endif
	else if (g_bl_info.bl_ic_ctrl_mode == AMOLED_NO_BL_IC_MODE) {
		DPU_FB_INFO("bl_ic_ctrl_mode = %d\n", g_bl_info.bl_ic_ctrl_mode);
		return;
	}

	brightness = (bl_level << 16) | (g_bl_info.blpwm_out_precision - bl_level);  /* top 16 bit bkl */
	outp32(blpwm_base + BLPWM_OUT_CFG, brightness);
}

static uint32_t get_pwm_duty(struct dpu_fb_data_type *dpufd)
{
	char __iomem *blpwm_base = NULL;

	uint32_t continue_pwm_in_num;
	uint32_t continue_pwm_in_high_num;
	uint32_t continue_pwm_in_low_num;
	uint32_t continue_pwm_in_duty;
	uint32_t default_duty = MIN_PWMDUTY_FOR_RGBW;

	blpwm_base = g_dpufd_blpwm_base;

	if (pwmduty[PWMDUTY_FILTER_NUMBER - 1] != 0xffff)  /* pwm_duty is 100% */
		default_duty = (pwmduty[PWMDUTY_FILTER_NUMBER - 1] >= MIN_PWMDUTY_FOR_RGBW) ?
			pwmduty[PWMDUTY_FILTER_NUMBER - 1] : MIN_PWMDUTY_FOR_RGBW;

	if (!blpwm_base) {
		DPU_FB_ERR("blpwm_base is null!\n");
		return default_duty;
	}

	continue_pwm_in_num = inp32(blpwm_base + PWM_IN_NUM_OFFSET);
	if (continue_pwm_in_num <= 0) {
		DPU_FB_DEBUG("pwm_in_num is null!\n");
		return default_duty;
	}

	continue_pwm_in_high_num = continue_pwm_in_num >> 16; /* two high bytes */
	continue_pwm_in_low_num = continue_pwm_in_num & 0xFFFF; /* two low bytes */
	/* ddic_panel_id == 1 --> JDI_NT36860C_PANEL_ID
	 * ddic_panel_id == 2 --> LG_NT36870_PANEL_ID
	 * ddic_panel_id == 3 --> SHARP_NT36870_PANEL_ID
	 */
	if (((continue_pwm_in_high_num + continue_pwm_in_low_num) < PWM_PERIOD_TOTAL_COUNTOR_JDI_SHARP) &&
		(dpufd->de_info.ddic_panel_id == 1 || dpufd->de_info.ddic_panel_id == 3)) {
		DPU_FB_DEBUG("pwm_in_num is abnormal!\n");
		return default_duty;
	}
	if (((continue_pwm_in_high_num + continue_pwm_in_low_num) < PWM_PERIOD_TOTAL_COUNTOR_LGD) &&
		(dpufd->de_info.ddic_panel_id == 2)) {
		DPU_FB_DEBUG("pwm_in_num is abnormal!\n");
		return default_duty;
	}

	continue_pwm_in_duty = g_bl_info.blpwm_input_precision * continue_pwm_in_high_num /
		(continue_pwm_in_high_num + continue_pwm_in_low_num);

	if (continue_pwm_in_duty < MIN_PWMDUTY_FOR_RGBW)
		continue_pwm_in_duty = MIN_PWMDUTY_FOR_RGBW;

	return continue_pwm_in_duty;
}

static void get_ap_dimming_to_update_backlight(struct dpu_fb_data_type *dpufd)
{
	int32_t delta_cabc_pwm;
	int32_t pwm_duty;
	int32_t backlight;

	DPU_FB_DEBUG("cabc3:jump while\n");
	if (g_bl_info.index_cabc_dimming > CABC_DIMMING_STEP_TOTAL_NUM) {
		DPU_FB_DEBUG("cabc4:dimming 32 time\n");
		set_current_state(TASK_INTERRUPTIBLE);  /*lint !e666*/
		schedule();
		g_bl_info.index_cabc_dimming = 1;
	} else {
		down(&g_bl_info.bl_semaphore);
		DPU_FB_DEBUG("cabc5:dimming time=%d, g_bl_info.cabc_pwm_in=%d\n",
			g_bl_info.index_cabc_dimming, g_bl_info.cabc_pwm_in);

		if (g_bl_info.cabc_pwm_in != 0) {
			g_bl_info.cabc_pwm = g_bl_info.cabc_pwm_in;
			g_bl_info.cabc_pwm_in = 0;
			g_bl_info.index_cabc_dimming = 1;
			g_bl_info.prev_cabc_pwm = g_bl_info.current_cabc_pwm;
			DPU_FB_DEBUG("cabc6:cabc_pwm=%d, cabc_pwm_in=%d, prev_cabc_pwm=%d\n",
				g_bl_info.cabc_pwm, g_bl_info.cabc_pwm_in, g_bl_info.prev_cabc_pwm);
		}
		/* this follow code calculate cabc pwm duty accord algorithm
		 * It contains some fixed numbers
		 */
		delta_cabc_pwm = g_bl_info.cabc_pwm - g_bl_info.prev_cabc_pwm;
		pwm_duty = delta_cabc_pwm * g_bl_info.index_cabc_dimming / 32 +
			delta_cabc_pwm * g_bl_info.index_cabc_dimming % 32 / 16;

		g_bl_info.current_cabc_pwm = g_bl_info.prev_cabc_pwm + pwm_duty;
		backlight = g_bl_info.current_cabc_pwm * g_bl_info.ap_brightness / g_bl_info.blpwm_input_precision;
		if ((backlight > 0) && (backlight < g_bl_info.bl_min))
			backlight = g_bl_info.bl_min;

		DPU_FB_DEBUG("cabc7:g_bl_info.ap_brightness=%d, g_bl_info.last_bl_level=%d, backlight=%d, "
			"g_bl_info.current_cabc_pwm=%d, pwm_duty=%d, g_bl_info.cabc_pwm=%d, "
			"g_bl_info.prev_cabc_pwm=%d, delta_cabc_pwm=%d\n",
			g_bl_info.ap_brightness, g_bl_info.last_bl_level, backlight, g_bl_info.current_cabc_pwm,
			pwm_duty, g_bl_info.cabc_pwm, g_bl_info.prev_cabc_pwm, delta_cabc_pwm);

		if (g_bl_info.ap_brightness != 0 && backlight != g_bl_info.last_bl_level) {
			update_backlight(dpufd, backlight);
			g_bl_info.last_bl_level = backlight;
		}

		g_bl_info.index_cabc_dimming++;
		up(&g_bl_info.bl_semaphore);
		msleep(16);  /* 16ms */
	}
}

static int is_pwm_dimming_stop(int pwm_duty)
{
	int dimming_stop_counter = 0;
	int delta_pwmduty;
	static int dimming_stop_flag;
	int i;

	if (g_rgbw_bl_stop_threshold_num < RGBW_BL_STOP_THRESHOLD)
		return 0;

	for (i = 0; i < PWMDUTY_FILTER_NUMBER; i++) {
		if (i < (PWMDUTY_FILTER_NUMBER - 1))
			pwmduty[i] = pwmduty[i + 1];

		if (i == (PWMDUTY_FILTER_NUMBER - 1))
			pwmduty[i] = pwm_duty;
	}

	for (i = 0; i < (PWMDUTY_FILTER_NUMBER - 1); i++) {
		delta_pwmduty = pwmduty[i + 1] - pwmduty[i];
		if (delta_pwmduty <= PWM_IN_SENSITY && delta_pwmduty >= (-PWM_IN_SENSITY))
			dimming_stop_counter++;
	}

	dimming_stop_flag = (dimming_stop_counter == (PWMDUTY_FILTER_NUMBER - 1)) ? 1 : 0;
	if (dimming_stop_flag == 1)
		g_rgbw_bl_stop_threshold_num = 0;

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
static void set_rgbw_lg(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	int rgbw_lg_fgl = RGBW_LG_RGBW * RGBW_LG_BIT / RGBW_LG_RGB;
	int backlight_indoor_lgd;

	pinfo = &(dpufd->panel_info);

	backlight_indoor_lgd = (int)pinfo->bl_max * RGBW_LG_RGB / RGBW_LG_RGBW;
	if (backlight_indoor_lgd == 0) {
		DPU_FB_ERR("backlight_indoor_lgd is err!\n");
		return;
	}

	if (dpufd->de_info.ddic_rgbw_backlight < backlight_indoor_lgd) {
		dpufd->de_info.frame_gain_limit = rgbw_lg_fgl;
		dpufd->de_info.color_distortion_allowance = RGBW_LG_CDA;
		dpufd->de_info.pixel_gain_limit = 0;
		dpufd->de_info.pwm_duty_gain = RGBW_LG_BIT;
	} else {
		if (dpufd->de_info.ddic_rgbw_backlight == 0) {
			DPU_FB_ERR("ddic_rgbw_backlight is err!\n");
			return;
		}

		dpufd->de_info.frame_gain_limit = (int)pinfo->bl_max * RGBW_LG_BIT /
			dpufd->de_info.ddic_rgbw_backlight;
		dpufd->de_info.color_distortion_allowance =
			(dpufd->de_info.ddic_rgbw_backlight - backlight_indoor_lgd) * RGBW_LG_CDA_OUTDOOR /
			backlight_indoor_lgd + RGBW_LG_CDA;
		dpufd->de_info.pixel_gain_limit = rgbw_lg_fgl - dpufd->de_info.frame_gain_limit;
		dpufd->de_info.pwm_duty_gain = RGBW_LG_BIT;
	}
}

static void set_rgbw_lg_hma(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	int rgbw_lg_fgl = RGBW_LG_RGBW_HMA * RGBW_LG_BIT / RGBW_LG_RGB_HMA;
	int backlight_indoor_lgd;

	pinfo = &(dpufd->panel_info);

	backlight_indoor_lgd = (int)pinfo->bl_max * RGBW_LG_RGB_HMA / RGBW_LG_RGBW_HMA;
	if (backlight_indoor_lgd == 0) {
		DPU_FB_ERR("backlight_indoor_lgd is err!\n");
		return;
	}

	if (dpufd->de_info.ddic_rgbw_backlight < backlight_indoor_lgd) {
		dpufd->de_info.frame_gain_limit = rgbw_lg_fgl;
		dpufd->de_info.color_distortion_allowance = RGBW_LG_CDA;
		dpufd->de_info.pixel_gain_limit = 0;
		dpufd->de_info.pwm_duty_gain = RGBW_LG_BIT;
	} else {
		dpufd->de_info.frame_gain_limit = (int)pinfo->bl_max * RGBW_LG_BIT /
			dpufd->de_info.ddic_rgbw_backlight;
		dpufd->de_info.color_distortion_allowance =
			(dpufd->de_info.ddic_rgbw_backlight - backlight_indoor_lgd) * RGBW_LG_CDA_OUTDOOR /
			backlight_indoor_lgd + RGBW_LG_CDA;
		dpufd->de_info.pixel_gain_limit = rgbw_lg_fgl - dpufd->de_info.frame_gain_limit;
		dpufd->de_info.pwm_duty_gain = RGBW_LG_BIT;
	}
}

static void set_rgbw_boe_hma(struct dpu_fb_data_type *dpufd)
{
	int backlight_indoor_boe;
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	backlight_indoor_boe = (int)pinfo->bl_max * RGBW_BOE_RGB_HMA / RGBW_BOE_RGBW_HMA;
	if (backlight_indoor_boe == 0) {
		DPU_FB_ERR("backlight_indoor_lgd is err!\n");
		return;
	}
	dpufd->de_info.frame_gain_limit = RGBW_BOE_FGL_HMA;
	dpufd->de_info.pwm_duty_gain = RGBW_BOE_PDG_HMA;

	if (dpufd->de_info.ddic_rgbw_backlight < backlight_indoor_boe) {
		dpufd->de_info.color_distortion_allowance = 0;
		dpufd->de_info.pixel_gain_limit = 0;
		dpufd->de_info.rgbw_total_glim = RGBW_BOE_TGL_HMA;
	} else {
		dpufd->de_info.color_distortion_allowance = RGBW_BOE_CDA_HMA *
			(dpufd->de_info.ddic_rgbw_backlight - backlight_indoor_boe) /
			((int)pinfo->bl_max - backlight_indoor_boe);
		dpufd->de_info.rgbw_total_glim = dpufd->de_info.ddic_rgbw_backlight *
			RGBW_BOE_TGL_HMA / backlight_indoor_boe;
		dpufd->de_info.pixel_gain_limit = dpufd->de_info.rgbw_total_glim - RGBW_BOE_TGL_HMA;
	}
}

static void set_rgbw_jdi_hma_replace(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	int backlight_indoor;

	pinfo = &(dpufd->panel_info);

	backlight_indoor = (int)pinfo->bl_max * RGBW_JDI_RGB_HMA / RGBW_JDI_RGBW_HMA;
	if (backlight_indoor == 0) {
		DPU_FB_ERR("backlight_indoor_lgd is err!\n");
		return;
	}

	if (dpufd->de_info.ddic_rgbw_backlight < backlight_indoor)
		dpufd->de_info.pixel_gain_limit = 0;
	else
		dpufd->de_info.pixel_gain_limit = RGBW_JDI_PIXEL_MAX_LIMIT *
			(dpufd->de_info.ddic_rgbw_backlight - backlight_indoor) / backlight_indoor;
}

static void set_rgbw_lg_hma_replace(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	int rgbw_lg_fgl = RGBW_LG_RGBW_HMA * RGBW_LG_BIT / RGBW_LG_RGB_HMA;
	int backlight_indoor_lgd;

	pinfo = &(dpufd->panel_info);

	backlight_indoor_lgd = (int)pinfo->bl_max * RGBW_LG_RGB_HMA / RGBW_LG_RGBW_HMA;
	if (backlight_indoor_lgd == 0) {
		DPU_FB_ERR("backlight_indoor_lgd is err!\n");
		return;
	}

	if (dpufd->de_info.ddic_rgbw_backlight < backlight_indoor_lgd) {
		dpufd->de_info.frame_gain_limit = RGBW_LG_BIT;
		dpufd->de_info.color_distortion_allowance = RGBW_LG_CDA;
		dpufd->de_info.pixel_gain_limit = 0;
		dpufd->de_info.pwm_duty_gain = RGBW_LG_BIT;
	} else {
		if (((int)pinfo->bl_max - backlight_indoor_lgd) == 0) {
			DPU_FB_ERR("backlight_indoor_lgd is err!\n");
			return;
		}
		dpufd->de_info.frame_gain_limit = RGBW_LG_BIT;
		dpufd->de_info.color_distortion_allowance = (dpufd->de_info.ddic_rgbw_backlight -
		    backlight_indoor_lgd) * RGBW_LG_CDA_OUTDOOR / backlight_indoor_lgd + RGBW_LG_CDA;
		dpufd->de_info.pixel_gain_limit = (rgbw_lg_fgl - RGBW_LG_BIT) *
		    (dpufd->de_info.ddic_rgbw_backlight - backlight_indoor_lgd) /
		    ((int)pinfo->bl_max - backlight_indoor_lgd);
		dpufd->de_info.pwm_duty_gain = RGBW_LG_BIT;
	}
}
#endif

#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
static int get_calc_bkl_jdi_sharp(struct dpu_fb_data_type *dpufd)
{
	int32_t backlight;

	if (g_bl_info.blpwm_input_precision > 0)
		backlight = ((int32_t)dpufd->panel_info.bl_max) * g_bl_info.cabc_pwm_in /
			g_bl_info.blpwm_input_precision;
	else
		backlight = g_bl_info.ap_brightness;

	return backlight;
}

static int get_calc_bkl_lg(struct dpu_fb_data_type *dpufd)
{
	int32_t backlight;

	if (g_bl_info.blpwm_input_precision > 0)
		backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW / RGBW_LG_RGB,
			(int32_t)dpufd->panel_info.bl_max) * g_bl_info.cabc_pwm_in / g_bl_info.blpwm_input_precision;
	else
		backlight = g_bl_info.ap_brightness;

	set_rgbw_lg(dpufd);

	return backlight;
}

static int get_calc_bkl_lg_hma(struct dpu_fb_data_type *dpufd)
{
	int32_t backlight;

	if (g_bl_info.blpwm_input_precision > 0)
		backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW_HMA / RGBW_LG_RGB_HMA,
			(int32_t)dpufd->panel_info.bl_max) * g_bl_info.cabc_pwm_in / g_bl_info.blpwm_input_precision;
	else
		backlight = g_bl_info.ap_brightness;

	set_rgbw_lg_hma(dpufd);

	return backlight;
}

static int get_calc_bkl_boe_hma(struct dpu_fb_data_type *dpufd)
{
	int32_t backlight;

	if (g_bl_info.blpwm_input_precision > 0)
		backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW_HMA / RGBW_LG_RGB_HMA *
			g_bl_info.cabc_pwm_in / g_bl_info.blpwm_input_precision, (int32_t)dpufd->panel_info.bl_max);
	else
		backlight = g_bl_info.ap_brightness;

	set_rgbw_boe_hma(dpufd);

	return backlight;
}

static int get_calc_bkl_jdi_sharp_rt8555_hma(struct dpu_fb_data_type *dpufd)
{
	int32_t backlight;

	if (g_bl_info.blpwm_input_precision > 0)
		backlight = ((int32_t)dpufd->panel_info.bl_max) * g_bl_info.cabc_pwm_in /
			g_bl_info.blpwm_input_precision;
	else
		backlight = g_bl_info.ap_brightness;

	set_rgbw_jdi_hma_replace(dpufd);

	return backlight;
}

static int get_calc_bkl_lg_rt8555_hma(struct dpu_fb_data_type *dpufd)
{
	int32_t backlight;

	if (g_bl_info.blpwm_input_precision > 0)
		backlight = MIN(g_bl_info.ap_brightness * RGBW_LG_RGBW_HMA / RGBW_LG_RGB_HMA,
			(int32_t)dpufd->panel_info.bl_max);
	else
		backlight = g_bl_info.ap_brightness;

	set_rgbw_lg_hma_replace(dpufd);

	return backlight;
}
#endif

static int get_calc_bkl_def(struct dpu_fb_data_type *dpufd)
{
	(void)dpufd;

	return g_bl_info.ap_brightness;
}

static struct panel_calc_backlight panel_calc[] = {
#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
	{JDI_NT36860C_PANEL_ID, get_calc_bkl_jdi_sharp},
	{SHARP_NT36870_PANEL_ID, get_calc_bkl_jdi_sharp},
	{JDI_HX83112C_PANLE_ID, get_calc_bkl_jdi_sharp},
	{SHARP_HX83112C_PANEL_ID, get_calc_bkl_jdi_sharp},
	{JDI_TD4336_PANEL_ID, get_calc_bkl_jdi_sharp},
	{SHARP_TD4336_PANEL_ID, get_calc_bkl_jdi_sharp},
	{JDI_TD4336_HMA_PANEL_ID, get_calc_bkl_jdi_sharp},
	{SHARP_TD4336_HMA_PANEL_ID, get_calc_bkl_jdi_sharp},
	{LG_NT36870_PANEL_ID, get_calc_bkl_lg},
	{LG_NT36772A_PANEL_ID, get_calc_bkl_lg},
	{LG_NT36772A_HMA_PANEL_ID, get_calc_bkl_lg_hma},
	{BOE_HX83112E_HMA_PANEL_ID, get_calc_bkl_boe_hma},
	{JDI_TD4336_RT8555_HMA_PANEL_ID, get_calc_bkl_jdi_sharp_rt8555_hma},
	{SHARP_TD4336_RT8555_HMA_PANEL_ID, get_calc_bkl_jdi_sharp_rt8555_hma},
	{LG_NT36772A_RT8555_HMA_PANEL_ID, get_calc_bkl_lg_rt8555_hma},
#endif
	{PANEL_ID_DEFAULT, get_calc_bkl_def},
};

static void get_calc_backlight_arrord_panel_id(struct dpu_fb_data_type *dpufd, int32_t *backlight)
{
	uint32_t i = 0;

	if (g_low_light_count > RGBW_LOW_LIGHT_STOP) {
		*backlight = g_last_backlight;
		DPU_FB_DEBUG("cabc_rgbw lock_backlight: backlight= %d\n", *backlight);
	} else {
		if (dpufd->de_info.ddic_panel_id) {
			DPU_FB_DEBUG("[RGBW] panel_id = %d\n", dpufd->de_info.ddic_panel_id);
#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
			for (i = 0; i < ARRAY_SIZE(panel_calc) - 1; i++) {
				if (panel_calc[i].panel_id == dpufd->de_info.ddic_panel_id)
					break;
			}
#endif
			*backlight = panel_calc[i].get_calc_bkl(dpufd);

		} else {
			*backlight = g_bl_info.ap_brightness;
		}

		g_last_backlight = *backlight;
	}
}

static int calc_backlight(struct dpu_fb_data_type *dpufd, int32_t pwm_duty)
{
	int32_t backlight = 0;
	int32_t delta_pwm_duty = pwm_duty - g_last_pwm_duty;
	int32_t delta_pwm_duty_abs = abs(delta_pwm_duty);

	dpu_check_and_return(!dpufd, -1, ERR, "dpufd is null pointer\n");

	/* threshold about delta pwm duty */
	if ((delta_pwm_duty_abs <= 1) || ((delta_pwm_duty_abs == 3) && (g_last_delta_pwm_duty_abs == 1))) {
		g_bl_info.cabc_pwm_in = g_last_pwm_duty;
	} else {
		if (pwm_duty > g_last_pwm_duty)
			g_last_pwm_duty =  pwm_duty - 1;
		else
			g_last_pwm_duty =  pwm_duty + 1;

		g_bl_info.cabc_pwm_in = g_last_pwm_duty;
	}
	g_last_delta_pwm_duty_abs = delta_pwm_duty_abs;

	if (g_bl_info.cabc_pwm_in > g_bl_info.blpwm_input_precision)
		g_bl_info.cabc_pwm_in = g_bl_info.blpwm_input_precision;
	else if (g_bl_info.cabc_pwm_in < 0)
		g_bl_info.cabc_pwm_in = 0;

	if ((g_bl_info.last_ap_brightness == g_bl_info.ap_brightness) &&
		(g_bl_info.ap_brightness < RGBW_LOW_LIGHT_THRESHOLD))
		g_low_light_count++;
	else
		g_low_light_count = 0;  /* clear low light count */

	get_calc_backlight_arrord_panel_id(dpufd, &backlight);

	return backlight;
}

#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
static int get_smooth_backlight(int32_t backlight, int32_t ddic_panel_id)
{
	int i;
	int j = 0;
	int sum_backlight = 0;

	if (ddic_panel_id == JDI_TD4336_RT8555_HMA_PANEL_ID ||
		ddic_panel_id == SHARP_TD4336_RT8555_HMA_PANEL_ID ||
		ddic_panel_id == LG_NT36772A_RT8555_HMA_PANEL_ID)
		return backlight;

	backlight_buf[g_backlight_buf_count % BACKLIGHT_FILTER_NUMBER] = backlight;
	g_backlight_buf_count++;

	for (i = 0; i < BACKLIGHT_FILTER_NUMBER; i++) {
		if (backlight_buf[i] == 0xffff)
			continue;
		sum_backlight += backlight_buf[i];
		j++;
	}

	if (j != 0)
		backlight = (sum_backlight + j / 2)  / j;

	return backlight;
}
#endif

static void rgbw_set(struct dpu_fb_data_type *dpufd, struct dpu_fb_panel_data *pdata)
{
	if (dpufd->panel_power_on && pdata->lcd_rgbw_set_func && dpufd->de_info.ddic_panel_id) {
		down(&dpufd->power_sem);
		dpufb_vsync_disable_enter_idle(dpufd, true);
		dpufb_activate_vsync(dpufd);
		pdata->lcd_rgbw_set_func(dpufd);
		dpufb_vsync_disable_enter_idle(dpufd, false);
		dpufb_deactivate_vsync(dpufd);
		up(&dpufd->power_sem);
	}
}

static void update_bkl_arrord_rgbw_pwmduty(struct dpu_fb_data_type *dpufd, int temp_current_pwm_duty)
{
	int32_t backlight;

	down(&g_bl_info.bl_semaphore);

	backlight = calc_backlight(dpufd, temp_current_pwm_duty);
	DPU_FB_DEBUG("cabc_rgbw backlight = %d", backlight);
#if defined(CONFIG_LCDKIT_DRIVER) || defined(CONFIG_LCD_KIT_DRIVER)
	/* smooth filter for backlight */
	backlight = get_smooth_backlight(backlight, dpufd->de_info.ddic_panel_id);
#endif
	DPU_FB_DEBUG("cabc_rgbw panel_id =%d last_ap_brightness =%d ap_brightness =%d, "
		"current_duty =%d temp_duty =%d backlight =%d\n",
		dpufd->de_info.ddic_panel_id, g_bl_info.last_ap_brightness, g_bl_info.ap_brightness,
		g_bl_info.cabc_pwm_in, temp_current_pwm_duty, backlight);

	if (backlight > 0 && backlight < g_bl_info.bl_min)
		backlight = g_bl_info.bl_min;

	g_bl_info.last_ap_brightness = g_bl_info.ap_brightness;

	if (g_bl_info.ap_brightness != 0 && backlight != g_bl_info.last_bl_level) {
		update_backlight(dpufd, backlight);
		g_bl_info.last_bl_level = backlight;
	}
	msleep(12);  /* sleep 12ms */
	up(&g_bl_info.bl_semaphore);
}

static void dpufd_print_de_info(struct dpu_fb_data_type *dpufd)
{
	DPU_FB_DEBUG("[rgbw] %d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d\n",
		dpufd->de_info.ddic_panel_id,
		dpufd->de_info.ddic_rgbw_mode,
		dpufd->de_info.rgbw_saturation_control,
		dpufd->de_info.frame_gain_limit,
		dpufd->de_info.color_distortion_allowance,
		dpufd->de_info.pixel_gain_limit,
		dpufd->de_info.pwm_duty_gain,
		dpufd->de_info.rgbw_total_glim,
		dpufd->de_info.ddic_rgbw_backlight,
		dpufd->de_info.blc_delta,
		dpufd->bl_level);
}

static void get_rgbw_pwmduty_to_update_backlight(struct dpu_fb_data_type *dpufd)
{
	int temp_current_pwm_duty = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_panel_info *pinfo = NULL;
	int dimming_stop;

	pinfo = &(dpufd->panel_info);

	if ((g_bl_info.last_ap_brightness == 0 && g_bl_info.ap_brightness != 0)) {
		reset_pwm_buf(0xffff);  /* reset pwm buf to default value */
		g_last_pwm_duty = MIN_PWMDUTY_FOR_RGBW;
		g_last_delta_pwm_duty_abs = 0;
	}

	if (g_bl_info.ap_brightness != 0)
		temp_current_pwm_duty = get_pwm_duty(dpufd);

	g_rgbw_bl_stop_threshold_num++;
	dimming_stop = is_pwm_dimming_stop(temp_current_pwm_duty);

	DPU_FB_DEBUG("cabc_rgbw temp_current_pwm_duty = %d\n", temp_current_pwm_duty);

	if ((g_bl_info.last_ap_brightness == g_bl_info.ap_brightness) && (dimming_stop == 1)) {
		down(&g_bl_info.bl_semaphore);
		DPU_FB_DEBUG("cabc_rgbw :(stop!!!) last_ap_brightness =%d, ap_brightness = %d, "
			"current_duty =%d  prev_duty = %d g_blpwm_thread_stop = %d\n",
			g_bl_info.last_ap_brightness, g_bl_info.ap_brightness,
			temp_current_pwm_duty, g_bl_info.cabc_pwm_in, g_blpwm_thread_stop);
		if (g_blpwm_thread_stop) {
			msleep(5);  /* sleep 5ms */
			up(&g_bl_info.bl_semaphore);
			return;
		}

		set_current_state(TASK_INTERRUPTIBLE);  /*lint !e666*/
		up(&g_bl_info.bl_semaphore);
		schedule();
	} else {
		update_bkl_arrord_rgbw_pwmduty(dpufd, temp_current_pwm_duty);
	}

	if (g_bl_info.ap_brightness == 0)
		reset_pwm_buf(0xffff);  /* reset pwm buf to default value */

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval(!pdata, ERR, "[effect] pdata is NULL Pointer\n");

	mutex_lock(&g_rgbw_lock);
	dpufd->de_info.ddic_rgbw_backlight = MIN(MAX((dpufd->de_info.blc_enable ? dpufd->de_info.blc_delta : 0) +
		(int)dpufd->bl_level, (int)pinfo->bl_min), (int)pinfo->bl_max);

	dpufd_print_de_info(dpufd);

	rgbw_set(dpufd, pdata);

	mutex_unlock(&g_rgbw_lock);
}

static int cabc_pwm_thread(void *p)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpufd = (struct dpu_fb_data_type *)p;
	if (!dpufd) {
		DPU_FB_ERR("dpufd is null pointer\n");
		return -1;
	}

	while (!kthread_should_stop()) {
		if (g_rgbw_lcd_support)
			get_rgbw_pwmduty_to_update_backlight(dpufd);
		else
			get_ap_dimming_to_update_backlight(dpufd);
	}

	return 0;
}

int hisi_cabc_set_backlight(uint32_t cabc_pwm_in)
{
	DPU_FB_DEBUG("cabc2:cabc_pwm_in=%d,g_bl_info.ap_brightness=%d,if null=%d\n",
		cabc_pwm_in, g_bl_info.ap_brightness, cabc_pwm_task == NULL);
	if (!cabc_pwm_task)
		return 0;

	if (g_bl_info.ap_brightness == 0) {
		g_bl_info.current_cabc_pwm = cabc_pwm_in;
		return 0;
	}

	if (!g_rgbw_lcd_support)
		g_bl_info.cabc_pwm_in = cabc_pwm_in;

	wake_up_process(cabc_pwm_task);

	return 0;
}

static int hisi_blpwm_input_enable(struct dpu_fb_data_type *dpufd)
{
	char __iomem *blpwm_base = NULL;
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_return(!blpwm_base, -EINVAL, ERR, "blpwm_base is null!\n");

	outp32(blpwm_base + PWM_IN_CTRL_OFFSET, 1);
	if (g_rgbw_lcd_support) {
		outp32(blpwm_base + PWM_IN_DIV_OFFSET, 0x01);
		outp32(blpwm_base + PWM_IN_MAX_COUNTER, PWM_MAX_COUNTER_FOR_RGBW_JDI_SHARP);
	} else {
		outp32(blpwm_base + PWM_IN_DIV_OFFSET, 0x02);
	}
	cabc_pwm_task = kthread_create(cabc_pwm_thread, dpufd, "cabc_pwm_task");
	if (IS_ERR(cabc_pwm_task)) {
		DPU_FB_ERR("Unable to start kernel cabc_pwm_task.\n");
		cabc_pwm_task = NULL;
		return -EINVAL;
	}

	if (g_rgbw_lcd_support)
		g_blpwm_thread_stop = false;

	return 0;
}

static int hisi_blpwm_input_disable(struct dpu_fb_data_type *dpufd)
{
	char __iomem *blpwm_base = NULL;

	if (cabc_pwm_task) {
		if (g_rgbw_lcd_support) {
			down(&g_bl_info.bl_semaphore);
			g_blpwm_thread_stop = true;
			up(&g_bl_info.bl_semaphore);
		}
		kthread_stop(cabc_pwm_task);
		cabc_pwm_task = NULL;
	}

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_return(!blpwm_base, -EINVAL, ERR, "blpwm_base is null!\n");

	outp32(blpwm_base + PWM_IN_CTRL_OFFSET, 0);

	return 0;
}

static bool update_cabc_pwm_stop(struct dpu_fb_data_type *dpufd)
{
	bool pwm_stop = true;

	pwm_stop = (!g_blpwm_on || !dpufd->bl_level || !dpufd->backlight.bl_updated || g_bl_info.ap_brightness == 0);
	DPU_FB_DEBUG("update_cabc_pwm_stop:%d\n", pwm_stop);

	return pwm_stop;
}

int update_cabc_pwm(struct dpu_fb_data_type *dpufd)
{
	bool hiace_refresh = false;
	char __iomem *blpwm_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct pwm cabc_pwm = {0};
	int delta_duty;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	pinfo = &(dpufd->panel_info);

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_return(!blpwm_base, -EINVAL, ERR, "blpwm_base is NULL\n");

	if (update_cabc_pwm_stop(dpufd))
		return 0;

	hiace_refresh = dpufb_display_effect_fine_tune_backlight(dpufd,
		(int)dpufd->bl_level, &g_bl_info.ap_brightness);

	if (pinfo->blpwm_input_ena) {
		cabc_pwm.pwm_in_num = inp32(blpwm_base + PWM_IN_NUM_OFFSET);
		if (cabc_pwm.pwm_in_num <= 0) {
			DPU_FB_DEBUG("pwm_in_num is null!\n");
			return -EINVAL;
		}
		cabc_pwm.pwm_in_high_num = cabc_pwm.pwm_in_num >> 16;
		cabc_pwm.pwm_in_low_num  = cabc_pwm.pwm_in_num & 0xFFFF;
		/* high duty must larger than low duty */
		if ((!g_rgbw_lcd_support) && (cabc_pwm.pwm_in_high_num < cabc_pwm.pwm_in_low_num)) {
			DPU_FB_DEBUG("high duty should not be smaller than low duty!\n");
			return -EINVAL;
		}

		cabc_pwm.pwm_in_duty = pinfo->blpwm_input_precision * cabc_pwm.pwm_in_high_num /
			(cabc_pwm.pwm_in_high_num + cabc_pwm.pwm_in_low_num);

		DPU_FB_DEBUG("cabc0:pwm_in_numall=%d, pwm_in_high_num=%d, pwm_in_low_num=%d\n",
			cabc_pwm.pwm_in_high_num + cabc_pwm.pwm_in_low_num,
			cabc_pwm.pwm_in_high_num, cabc_pwm.pwm_in_low_num);
		cabc_pwm.pre_pwm_in_num = pinfo->blpwm_in_num;
		cabc_pwm.pre_pwm_in_high_num = cabc_pwm.pre_pwm_in_num >> 16;
		cabc_pwm.pre_pwm_in_low_num  = cabc_pwm.pre_pwm_in_num & 0xFFFF;
		cabc_pwm.pre_pwm_in_duty = pinfo->blpwm_input_precision * cabc_pwm.pre_pwm_in_high_num /
			(cabc_pwm.pre_pwm_in_high_num + cabc_pwm.pre_pwm_in_low_num);

		delta_duty = cabc_pwm.pwm_in_duty - cabc_pwm.pre_pwm_in_duty;
		DPU_FB_DEBUG("cabc1:Previous pwm in duty:%d, Current pwm in duty:%d, delta_duty:%d,pwm_in_num=%d\n",
			cabc_pwm.pre_pwm_in_duty, cabc_pwm.pwm_in_duty, delta_duty, cabc_pwm.pwm_in_num);

		if (g_rgbw_lcd_support || hiace_refresh || (delta_duty > PWM_IN_SENSITY ||
			delta_duty < (-PWM_IN_SENSITY))) {
			pinfo->blpwm_in_num = cabc_pwm.pwm_in_num;
			hisi_cabc_set_backlight(cabc_pwm.pwm_in_duty);
		}
	}

	return 0;
}

void hisi_blpwm_fill_light(uint32_t backlight)
{
	char __iomem *blpwm_base = NULL;
	uint32_t bl_level = backlight;
	uint32_t brightness;

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_no_retval(!blpwm_base, ERR, "blpwm_base is null!\n");

	down(&g_bl_info.bl_semaphore);

#ifdef CONFIG_HUAWEI_DUBAI
	/* notify dubai module to update brightness */
	dubai_update_brightness(backlight);
#endif

	DPU_FB_DEBUG("bl_level=%d, backlight=%d, blpwm_out_precision=%d, bl_max=%d\n",
			bl_level, backlight, g_bl_info.blpwm_out_precision, g_bl_info.bl_max);

	brightness = (bl_level << 16) | ((uint32_t)g_bl_info.blpwm_out_precision - bl_level);
	outp32(blpwm_base + BLPWM_OUT_CFG, brightness);

	up(&g_bl_info.bl_semaphore);
}

static int hisi_set_backlight_check(struct dpu_fb_data_type *dpufd, uint8_t rgbw_support,
	uint32_t *bl_level)
{
	struct dpu_panel_info *pinfo = &(dpufd->panel_info);

	dpu_check_and_return((g_blpwm_on == 0), 0, ERR, "blpwm is not on, return!\n");
	dpu_check_and_return((pinfo->bl_max < 1), -EINVAL, ERR, "bl_max[%d] is out of range!!\n", pinfo->bl_max);
	DPU_FB_DEBUG("fb%d, bl_level=%d.\n", dpufd->index, *bl_level);

	if (*bl_level > pinfo->bl_max)
		*bl_level = pinfo->bl_max;

	if (rgbw_support)
		g_bl_info.last_ap_brightness = g_bl_info.ap_brightness;

	/* allow backlight zero */
	if (*bl_level < pinfo->bl_min && *bl_level)
		*bl_level = pinfo->bl_min;

	return 1; /* continue to excute */
}

static int hisi_set_backlight_prepare(struct dpu_fb_data_type *dpufd, uint8_t rgbw_support,
	uint32_t *bl_level)
{
	struct dpu_panel_info *pinfo = &(dpufd->panel_info);

	dpufb_display_effect_fine_tune_backlight(dpufd, (int)(*bl_level), (int *)bl_level);

	g_bl_info.ap_brightness = *bl_level;

	if (*bl_level && rgbw_support && pinfo->blpwm_input_ena) {
		if (g_bl_info.last_ap_brightness  != g_bl_info.ap_brightness)
			hisi_cabc_set_backlight(g_bl_info.current_cabc_pwm);

		return 0;
	}

	if (pinfo->blpwm_input_ena && pinfo->blpwm_in_num) {
		if (*bl_level > 0) {
			*bl_level = *bl_level * g_bl_info.current_cabc_pwm / pinfo->blpwm_input_precision;
			*bl_level = (*bl_level < (uint32_t)g_bl_info.bl_min) ? g_bl_info.bl_min : *bl_level;
		}
		g_bl_info.last_bl_level = *bl_level;
		DPU_FB_DEBUG("cabc:ap_brightness=%d, current_cabc_pwm=%d, blpwm_input_precision=%d, bl_level=%d\n",
			g_bl_info.ap_brightness, g_bl_info.current_cabc_pwm,
			pinfo->blpwm_input_precision, *bl_level);
	}

#ifdef CONFIG_HUAWEI_DUBAI
	/* notify dubai module to update brightness */
	dubai_update_brightness(*bl_level);
#endif

	return 1; /* continue to excute */
}

static int hisi_reg_set_brightness(struct dpu_panel_info *pinfo, uint32_t bl_level)
{
	uint32_t brightness;

	dpu_check_and_return(!g_dpufd_blpwm_base, -EINVAL, ERR, "blpwm_base is NULL\n");

	brightness = (bl_level << 16) | ((uint32_t)g_bl_info.blpwm_out_precision - bl_level);
	outp32(g_dpufd_blpwm_base + BLPWM_OUT_CFG, brightness);

	DPU_FB_DEBUG("cabc:ap_brightness=%d, current_cabc_pwm=%d, blpwm_input_precision=%d, \
		blpwm_out_precision=%d, bl_level=%d,\
		brightness=%d\n", g_bl_info.ap_brightness, g_bl_info.current_cabc_pwm,
		pinfo->blpwm_input_precision, g_bl_info.blpwm_out_precision, bl_level, brightness);

	return 0;
}


static void hisi_set_backlight_lm36xx(struct dpu_panel_info *pinfo, uint32_t bl_level)
{
	if (pinfo->bl_ic_ctrl_mode == REG_ONLY_MODE) {
#if defined(CONFIG_LCDKIT_DRIVER)
		if (lcdkit_info.panel_infos.init_lm36923_after_panel_power_on_support)
			lm36923_set_backlight_init(bl_level);
#endif
		lm36923_set_backlight_reg(bl_level);
	} else if (pinfo->bl_ic_ctrl_mode == I2C_ONLY_MODE) {
	}
}

static int hisi_ctrl_mode_set_backlight(struct dpu_panel_info *pinfo, uint32_t bl_level)
{
	if ((g_bl_info.bl_ic_ctrl_mode >= REG_ONLY_MODE) && (g_bl_info.bl_ic_ctrl_mode <= I2C_ONLY_MODE)) {
		bl_level = bl_lvl_map(bl_level);
		DPU_FB_DEBUG("cabc:bl_level=%d\n", bl_level);
		hisi_set_backlight_lm36xx(pinfo, bl_level);
		return 0;
	} else if (g_bl_info.bl_ic_ctrl_mode == BLPWM_AND_CABC_MODE) {
	}
#ifdef CONFIG_LCDKIT_DRIVER
	else if (g_bl_info.bl_ic_ctrl_mode == COMMON_IC_MODE) {
		if (g_bl_config.bl_level == BL_MAX_12BIT)
			bl_level = bl_level * g_bl_config.bl_level / g_bl_info.bl_max;
		else if (g_bl_config.bl_level == BL_MAX_10BIT)
			bl_level = bl_level * g_bl_config.bl_level / g_bl_info.bl_max;
		else if (g_bl_config.bl_level == BL_MAX_16BIT)
			bl_level = bl_level * g_bl_config.bl_level / g_bl_info.bl_max;
		else
			bl_level = bl_lvl_map(bl_level);

		if (hisi_blpwm_bl_callback(bl_level) == 0)
			return 0;
	}
#endif
	bl_level = get_backlight_level(bl_level);

	return hisi_reg_set_brightness(pinfo, bl_level);
}

int hisi_blpwm_set_backlight(struct dpu_fb_data_type *dpufd, uint32_t bl_level)
{
	struct dpu_panel_info *pinfo = NULL;
	int ret;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	pinfo = &(dpufd->panel_info);

	ret = hisi_set_backlight_check(dpufd, g_rgbw_lcd_support, &bl_level);
	if (ret != 1)  /* direct return */
		return ret;

	DPU_FB_DEBUG("cabc:fb%d, bl_level=%d, blpwm_input_ena=%d, blpwm_in_num=%d\n",
		dpufd->index, bl_level, pinfo->blpwm_input_ena, pinfo->blpwm_in_num);
	down(&g_bl_info.bl_semaphore);

	ret = hisi_set_backlight_prepare(dpufd, g_rgbw_lcd_support, &bl_level);
	if (ret != 1) { /* direct return */
		up(&g_bl_info.bl_semaphore);
		return ret;
	}

	ret = hisi_ctrl_mode_set_backlight(pinfo, bl_level);
	up(&g_bl_info.bl_semaphore);

	return ret;
}

int hisi_blpwm_on(struct platform_device *pdev)
{
	struct clk *clk_tmp = NULL;
	char __iomem *blpwm_base = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	int ret;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL!\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");

	pinfo = &(dpufd->panel_info);

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_return(!blpwm_base, -EINVAL, ERR, "blpwm_base is NULL!\n");

	if (g_blpwm_on == 1)
		return 0;

	clk_tmp = g_dss_blpwm_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			DPU_FB_ERR("dss_blpwm_clk clk_prepare failed, error=%d!\n", ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			DPU_FB_ERR("dss_blpwm_clk clk_enable failed, error=%d!\n", ret);
			return -EINVAL;
		}
	}

	ret = pinctrl_cmds_tx(g_blpwm_pdev, blpwm_pinctrl_normal_cmds,
		ARRAY_SIZE(blpwm_pinctrl_normal_cmds));
	/* if enable BLPWM, please set IOMG_003, IOMG_004 in IOC_AO module
	 * set IOMG_003: select BLPWM_CABC
	 * set IOMG_004: select BLPWM_BL
	 */
	outp32(blpwm_base + BLPWM_OUT_CTRL, 0x1);
	init_bl_info(pinfo);
	if (pinfo->blpwm_input_ena)
		hisi_blpwm_input_enable(dpufd);


	g_blpwm_on = 1;

	return ret;
}

int hisi_blpwm_off(struct platform_device *pdev)
{
	struct clk *clk_tmp = NULL;
	char __iomem *blpwm_base = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	int ret;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL!\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");

	pinfo = &(dpufd->panel_info);

	blpwm_base = g_dpufd_blpwm_base;
	dpu_check_and_return(!blpwm_base, -EINVAL, ERR, "blpwm_base is NULL!\n");

	if (g_blpwm_on == 0)
		return 0;

	outp32(blpwm_base + BLPWM_OUT_CTRL, 0x0);

	ret = pinctrl_cmds_tx(g_blpwm_pdev, blpwm_pinctrl_lowpower_cmds,
		ARRAY_SIZE(blpwm_pinctrl_lowpower_cmds));

	clk_tmp = g_dss_blpwm_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	if (pinfo->blpwm_input_ena)
		hisi_blpwm_input_disable(dpufd);

	g_blpwm_on = 0;

	return ret;
}

static int hisi_blpwm_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct device *dev = NULL;
	int ret;

	DPU_FB_DEBUG("+.\n");

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is null!\n");

	g_blpwm_pdev = pdev;
	dev = &pdev->dev;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_BLPWM_NAME);
	if (!np) {
		dev_err(dev, "NOT FOUND device node %s!\n", DTS_COMP_BLPWM_NAME);
		ret = -ENXIO;
		goto err_return;
	}

	/* get blpwm reg base */
	g_dpufd_blpwm_base = of_iomap(np, 0);
	if (!g_dpufd_blpwm_base) {
		dev_err(dev, "failed to get blpwm_base resource.\n");
		ret = -ENXIO;
		goto err_return;
	}

	ret = of_property_read_u32(np, "fpga_flag", &g_blpwm_fpga_flag);
	if (ret) {
		dev_err(dev, "failed to get fpga_flag resource.\n");
		ret = -ENXIO;
		goto err_return;
	}

	if (g_blpwm_fpga_flag == 0) {
		/* blpwm pinctrl init */
		ret = pinctrl_cmds_tx(pdev, blpwm_pinctrl_init_cmds, ARRAY_SIZE(blpwm_pinctrl_init_cmds));
		if (ret != 0) {
			dev_err(dev, "Init blpwm pinctrl failed! ret=%d.\n", ret);
			goto err_return;
		}

		/* get blpwm clk resource */
		g_dss_blpwm_clk = of_clk_get(np, 0);
		if (IS_ERR(g_dss_blpwm_clk)) {
			dev_err(dev, "%s clock not found: %d!\n", np->name, (int)PTR_ERR(g_dss_blpwm_clk));
			ret = -ENXIO;
			goto err_return;
		}

		dev_info(dev, "dss_blpwm_clk:[%lu]->[%lu].\n", DEFAULT_PWM_CLK_RATE, clk_get_rate(g_dss_blpwm_clk));
	}

	hisi_fb_device_set_status0(DTS_PWM_READY);

	DPU_FB_DEBUG("-.\n");

	return 0;

err_return:
	return ret;
}

static int hisi_blpwm_remove(struct platform_device *pdev)
{
	struct clk *clk_tmp = NULL;
	int ret;

	ret = pinctrl_cmds_tx(pdev, blpwm_pinctrl_finit_cmds, ARRAY_SIZE(blpwm_pinctrl_finit_cmds));

	clk_tmp = g_dss_blpwm_clk;
	if (clk_tmp) {
		clk_put(clk_tmp);
		g_dss_blpwm_clk = NULL;
		clk_tmp = NULL;
	}

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
	int ret;

	ret = platform_driver_register(&this_driver);
	if (ret)
		DPU_FB_ERR("platform_driver_register failed, error=%d!\n", ret);

	return ret;
}

module_init(hisi_blpwm_init);

