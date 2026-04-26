/* Copyright (c) 2008-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#include "hisi_fb.h"
#include <huawei_platform/touthscreen/huawei_touchscreen.h>

#define DTS_COMP_LG_TD4322 "hisilicon,mipi_lg_TD4322"
#define USED_FOR_UDP  0
#define USED_FOR_FPGA 1
static int g_lcd_fpga_flag;


/* Display ON/OFF Sequence begin */
/*lint -save -e569, -e574, -e527, -e572*/
static char g_te_enable[] = {
	0x35,
	0x00,
};

static char g_bl_enable[] = {
	0x53,
	0x24,
};

// fix the orlando resume to lignt the lcd
static char g_bl_level[] = {
	0x51,
	0x66,
};

static char g_exit_sleep[] = {
	0x11,
};

static char g_display_on[] = {
	0x29,
};

static char g_display_phy1[] = {
	0xB0,
	0x04,
};
static char g_display_phy2[] = {
	0xB6,
	0x38,
	0x93,
	0x00,
};
static char g_lcd_power_on_cmd1[] = {
	0xD6,
	0x01,
};

static char g_color_enhancement[] = {
	0x30,
	0x00, 0x00, 0x02, 0xA7,
};

static char g_lcd_disp_x[] = {
	0x2A,
	0x00, 0x00, 0x04, 0x37,
};

static char g_lcd_disp_y[] = {
	0x2B,
	0x00, 0x00, 0x07, 0x7F,
};

static struct dsi_cmd_desc g_set_display_address[] = {
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(g_lcd_disp_x), g_lcd_disp_x},
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(g_lcd_disp_y), g_lcd_disp_y},
};

static char g_display_off[] = {
	0x28,
};

static char g_enter_sleep[] = {
	0x10,
};

static struct dsi_cmd_desc g_lcd_display_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(g_bl_enable), g_bl_enable},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(g_te_enable), g_te_enable},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(g_color_enhancement), g_color_enhancement},
	// fix the orlando resume to light the lcd
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(g_bl_level), g_bl_level},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(g_exit_sleep), g_exit_sleep},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(g_display_on), g_display_on},
};

static struct dsi_cmd_desc g_lg_fpga_modify_phyclk_config_cmds[] = {
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(g_lcd_power_on_cmd1), g_lcd_power_on_cmd1},
	{DTYPE_GEN_LWRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(g_display_phy1), g_display_phy1},
	{DTYPE_GEN_LWRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(g_display_phy2), g_display_phy2},
};

static struct dsi_cmd_desc g_lcd_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 52, WAIT_TYPE_MS,
		sizeof(g_display_off), g_display_off},
	{DTYPE_DCS_WRITE, 0, 102, WAIT_TYPE_MS,
		sizeof(g_enter_sleep), g_enter_sleep},
};

/* Display Effect Sequence
 * (smart color, edge enhancement, smart contrast, cabc)
 */
static char g_pwm_out_0x51[] = {
	0x51,
	0xFE,
};

static struct dsi_cmd_desc g_pwm_out_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(g_pwm_out_0x51), g_pwm_out_0x51},
};

/* LCD VCC */
#define VCC_LCDIO_NAME      "lcdio-vcc"
#define VCC_LCDANALOG_NAME  "lcdanalog-vcc"

static struct regulator *g_vcc_lcdio;
static struct regulator *g_vcc_lcdanalog;

static struct vcc_desc g_lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_GET, VCC_LCDANALOG_NAME,
		&g_vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},

	/* vcc set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDANALOG_NAME,
		&g_vcc_lcdanalog, 3100000, 3100000, WAIT_TYPE_MS, 0},
	/* io set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 1800000, 1800000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc g_lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_PUT, VCC_LCDANALOG_NAME,
		&g_vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc g_lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME,
		&g_vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc g_lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_DISABLE, VCC_LCDANALOG_NAME,
		&g_vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};

/* LCD IOMUX */
static struct pinctrl_data g_pctrl;

static struct pinctrl_cmd_desc g_lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &g_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &g_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &g_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &g_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &g_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &g_pctrl, 0},
};

/* LCD GPIO */
#define GPIO_LCD_P5V5_ENABLE_NAME  "g_gpio_lcd_p5v5_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME  "g_gpio_lcd_n5v5_enable"
#define GPIO_LCD_RESET_NAME        "g_gpio_lcd_reset"
#define GPIO_LCD_BL_ENABLE_NAME    "g_gpio_lcd_bl_enable"
#define GPIO_LCD_TP1V8_NAME        "g_gpio_lcd_tp1v8"
#define GPIO_LCD_TP2V8_NAME        "g_gpio_lcd_tp2v8"


static uint32_t g_gpio_lcd_p5v5_enable;
static uint32_t g_gpio_lcd_n5v5_enable;
static uint32_t g_gpio_lcd_reset;
static uint32_t g_gpio_lcd_bl_enable;
static uint32_t g_gpio_lcd_tp1v8;
static uint32_t g_gpio_lcd_tp2v8;

static struct gpio_desc g_fpga_lcd_gpio_request_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	/* TP_1.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &g_gpio_lcd_tp1v8, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
			GPIO_LCD_TP2V8_NAME, &g_gpio_lcd_tp2v8, 0},
};

static struct gpio_desc g_fpga_lcd_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_TP2V8_NAME, &g_gpio_lcd_tp2v8, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_TP1V8_NAME, &g_gpio_lcd_tp1v8, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_TP1V8_NAME, &g_gpio_lcd_tp1v8, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 10,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 50,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 1},
};

static struct gpio_desc g_fpga_lcd_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},

	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
};

static struct gpio_desc g_fpga_lcd_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
};

static struct gpio_desc g_asic_lcd_gpio_request_cmds[] = {
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
};

static struct gpio_desc g_asic_lcd_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 1},
};

static struct gpio_desc g_asic_lcd_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 50,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 50,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 50,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
};

static struct gpio_desc g_asic_lcd_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_BL_ENABLE_NAME, &g_gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_RESET_NAME, &g_gpio_lcd_reset, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &g_gpio_lcd_n5v5_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &g_gpio_lcd_p5v5_enable, 0},
};

static uint32_t g_acm_r0_hh = 0x7f;
static uint32_t g_acm_r0_lh;
static uint32_t g_acm_r1_hh = 0xff;
static uint32_t g_acm_r1_lh = 0x80;
static uint32_t g_acm_r2_hh = 0x17f;
static uint32_t g_acm_r2_lh = 0x100;
static uint32_t g_acm_r3_hh = 0x1ff;
static uint32_t g_acm_r3_lh = 0x180;
static uint32_t g_acm_r4_hh = 0x27f;
static uint32_t g_acm_r4_lh = 0x200;
static uint32_t g_acm_r5_hh = 0x2ff;
static uint32_t g_acm_r5_lh = 0x280;
static uint32_t g_acm_r6_hh = 0x37f;
static uint32_t g_acm_r6_lh = 0x300;

static uint32_t g_acm_hue_param01 = 0x200 << 16 | 0x200;
static uint32_t g_acm_hue_param23 = 0x1FC << 16 | 0x200;
static uint32_t g_acm_hue_param45 = 0x204 << 16 | 0x200;
static uint32_t g_acm_hue_param67 = 0x200 << 16 | 0x200;
static uint32_t g_acm_hue_smooth0 = 0x0040003F;
static uint32_t g_acm_hue_smooth1 = 0x00C000BF;
static uint32_t g_acm_hue_smooth2 = 0x0140013F;
static uint32_t g_acm_hue_smooth3 = 0x01C001BF;
static uint32_t g_acm_hue_smooth4 = 0x02410240;
static uint32_t g_acm_hue_smooth5 = 0x02C102C0;
static uint32_t g_acm_hue_smooth6 = 0x0340033F;
static uint32_t g_acm_hue_smooth7 = 0x03C003BF;
static uint32_t g_acm_color_choose = 1;
static uint32_t g_acm_l_cont_en;
static uint32_t g_acm_lc_param01 = 0x020401FC;
static uint32_t g_acm_lc_param23 = 0x02000200;
static uint32_t g_acm_lc_param45 = 0x020801F8;
static uint32_t g_acm_lc_param67 = 0x020401FC;
static uint32_t g_acm_l_adj_ctrl;
static uint32_t g_acm_capture_ctrl;
static uint32_t g_acm_capture_in;
static uint32_t g_acm_capture_out;
static uint32_t g_acm_ink_ctrl;
static uint32_t g_acm_ink_out;

static u32 g_acm_lut_hue_table[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64,
	68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
	124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172,
	176, 180, 184, 188, 192, 196, 200, 204, 208, 212, 216, 220, 224,
	228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276,
	280, 284, 288, 292, 296, 300, 304, 308, 312, 316, 320, 324, 328,
	332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
	384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432,
	436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484,
	488, 492, 496, 500, 504, 508, 512, 516, 520, 524, 528, 532, 536,
	540, 544, 548, 552, 556, 560, 564, 568, 572, 576, 580, 584, 588,
	592, 596, 600, 604, 608, 612, 616, 620, 624, 628, 632, 636, 640,
	644, 648, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688, 692,
	696, 700, 704, 708, 712, 716, 720, 724, 728, 732, 736, 740, 744,
	748, 752, 756, 760, 764, 768, 772, 776, 780, 784, 788, 792, 796,
	800, 804, 808, 812, 816, 820, 824, 828, 832, 836, 840, 844, 848,
	852, 856, 860, 864, 868, 872, 876, 880, 884, 888, 892, 896, 900,
	904, 908, 912, 916, 920, 924, 928, 932, 936, 940, 944, 948, 952,
	956, 960, 964, 968, 972, 976, 980, 984, 988, 992, 996, 1000,
	1004, 1008, 1012, 1016, 1020
};
static u32 g_acm_lut_sata_table[] = {
	5, 4, 4, 4, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 2,
	2, 2, 3, 4, 4, 6, 7, 8, 10, 12, 13, 14, 16, 18, 21, 23, 26,
	28, 30, 33, 35, 37, 38, 40, 42, 43, 45, 46, 48, 49, 50, 51,
	52, 53, 54, 55, 56, 56, 57, 57, 58, 58, 58, 59, 59, 59, 59,
	59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
	59, 59, 58, 58, 58, 58, 58, 57, 56, 55, 54, 52, 51, 50, 49,
	48, 48, 47, 46, 45, 44, 44, 43, 43, 42, 42, 42, 41, 41, 40,
	40, 40, 40, 41, 41, 41, 42, 42, 42, 42, 42, 41, 41, 41, 40,
	40, 40, 41, 42, 42, 43, 44, 44, 45, 46, 47, 48, 49, 50, 50,
	51, 52, 53, 54, 54, 55, 56, 56, 57, 57, 58, 57, 56, 56, 55,
	54, 54, 53, 52, 52, 51, 50, 50, 50, 49, 48, 48, 47, 46, 45,
	44, 44, 43, 42, 41, 40, 40, 39, 38, 37, 36, 36, 35, 35, 35,
	35, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 32, 32, 32, 32,
	31, 31, 30, 30, 30, 29, 29, 29, 28, 28, 28, 28, 28, 27, 27,
	26, 26, 25, 24, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 18,
	18, 18, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13,
	12, 12, 11, 10, 9, 8, 8, 7, 6
};
static u32 g_acm_lut_satr0_table[] = {
	0, 2, 3, 5, 6, 8, 10, 11, 13, 14, 16, 16, 17, 17, 18, 18, 18,
	19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 23, 24, 24, 25, 25,
	25, 25, 24, 24, 24, 24, 23, 23, 23, 23, 22, 22, 22, 22, 21,
	21, 21, 21, 20, 20, 20, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
};
static u32 g_acm_lut_satr1_table[] = {
	0, 0, 0, 4, 7, 11, 15, 19, 25, 30, 35, 39, 41, 44, 46, 48, 50,
	53, 55, 57, 59, 62, 64, 66, 68, 71, 73, 75, 77, 80, 82, 84,
	83, 81, 80, 78, 77, 75, 74, 72, 71, 69, 68, 67, 65, 64, 62,
	61, 59, 58, 56, 49, 42, 35, 30, 25, 20, 15, 11, 7, 4, 0, 0, 0
};
static u32 g_acm_lut_satr2_table[] = {
	0, 0, 0, 0, 0, 3, 9, 14, 18, 23, 27, 32, 36, 41, 44, 48, 51,
	56, 59, 62, 65, 66, 69, 71, 72, 74, 75, 77, 77, 77, 77, 77,
	75, 75, 74, 72, 71, 68, 66, 63, 60, 57, 54, 50, 47, 42, 38,
	35, 30, 26, 21, 15, 11, 6, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static u32 g_acm_lut_satr3_table[] = {
	0, 0, 0, 0, 0, 0, 0, 3, 7, 9, 13, 16, 19, 22, 25, 28, 30, 34,
	36, 39, 41, 43, 45, 46, 48, 49, 50, 51, 53, 54, 54, 54, 54,
	54, 53, 53, 51, 50, 49, 47, 46, 44, 42, 40, 38, 35, 33, 29,
	26, 24, 21, 18, 15, 11, 9, 7, 5, 3, 0, 0, 0, 0, 0, 0
};
static u32 g_acm_lut_satr4_table[] = {
	0, 0, 0, 0, 0, 0, 0, 3, 7, 9, 13, 16, 19, 22, 25, 28, 30, 34,
	36, 39, 41, 43, 45, 46, 48, 49, 50, 51, 53, 54, 54, 54, 54,
	54, 53, 53, 51, 50, 49, 47, 46, 44, 42, 40, 38, 35, 33, 29,
	26, 24, 21, 18, 15, 11, 9, 7, 5, 3, 0, 0, 0, 0, 0, 0
};
static u32 g_acm_lut_satr5_table[] = {
	0, 0, 0, 0, 0, 0, 0, 2, 6, 9, 13, 16, 19, 22, 25, 28, 30, 34,
	36, 39, 41, 43, 45, 46, 48, 49, 50, 51, 53, 54, 54, 54, 54,
	54, 53, 53, 51, 50, 49, 47, 46, 44, 42, 40, 38, 35, 33, 29,
	26, 24, 21, 18, 15, 11, 7, 4, 1, 0, 0, 0, 0, 0, 0, 0
};
static u32 g_acm_lut_satr6_table[] = {
	0, 0, 0, 5, 10, 16, 22, 28, 33, 39, 46, 50, 51, 52, 53, 54, 54,
	55, 56, 56, 57, 58, 59, 59, 60, 61, 61, 62, 63, 64, 64, 65, 64,
	63, 63, 62, 61, 60, 59, 58, 58, 57, 56, 55, 54, 54, 53, 52, 51,
	50, 49, 48, 47, 43, 38, 33, 28, 21, 15, 10, 5, 0, 0, 0
};
static u32 g_acm_lut_satr7_table[] = {
	0, 4, 7, 11, 15, 18, 22, 26, 30, 33, 37, 38, 38, 39, 40, 40, 41,
	42, 42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 50, 50, 51, 50,
	50, 49, 48, 48, 47, 47, 46, 45, 45, 44, 43, 43, 42, 41, 41, 40,
	40, 39, 38, 38, 37, 33, 30, 26, 22, 18, 15, 11, 7, 4, 0
};

static u32 g_gmp_lut_table_low32bit[9][9][9] = {
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600,
		0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600,
		0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600,
		0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600,
		0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600,
		0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600,
		0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600,
		0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600,
		0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600,
		0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
		0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
		0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
		0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
		0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
		0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
		0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
		0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
		0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
		0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
	0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
	0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
	0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
	0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
	0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
	0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
	0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
	0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
	0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
		0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
		0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
		0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
		0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
		0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
		0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
		0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
		0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
		0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
		0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
		0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
		0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
		0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
		0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
		0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
		0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
		0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
		0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
		0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
		0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
		0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
		0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
		0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
		0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
		0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
		0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
		0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
		0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
		0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
		0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
		0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
		0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
		0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
		0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
		0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
		0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800,
		0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
	{0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800,
		0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
	{0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800,
		0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
	{0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800,
		0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
	{0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800,
		0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
	{0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800,
		0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
	{0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800,
		0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
	{0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800,
		0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
	{0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800,
		0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
	},
	{
	{0xf0000000, 0xf0000200, 0xf0000400, 0xf0000600, 0xf0000800,
		0xf0000a00, 0xf0000c00, 0xf0000e00, 0xf0000ff0, },
	{0xf0200000, 0xf0200200, 0xf0200400, 0xf0200600, 0xf0200800,
		0xf0200a00, 0xf0200c00, 0xf0200e00, 0xf0200ff0, },
	{0xf0400000, 0xf0400200, 0xf0400400, 0xf0400600, 0xf0400800,
		0xf0400a00, 0xf0400c00, 0xf0400e00, 0xf0400ff0, },
	{0xf0600000, 0xf0600200, 0xf0600400, 0xf0600600, 0xf0600800,
		0xf0600a00, 0xf0600c00, 0xf0600e00, 0xf0600ff0, },
	{0xf0800000, 0xf0800200, 0xf0800400, 0xf0800600, 0xf0800800,
		0xf0800a00, 0xf0800c00, 0xf0800e00, 0xf0800ff0, },
	{0xf0a00000, 0xf0a00200, 0xf0a00400, 0xf0a00600, 0xf0a00800,
		0xf0a00a00, 0xf0a00c00, 0xf0a00e00, 0xf0a00ff0, },
	{0xf0c00000, 0xf0c00200, 0xf0c00400, 0xf0c00600, 0xf0c00800,
		0xf0c00a00, 0xf0c00c00, 0xf0c00e00, 0xf0c00ff0, },
	{0xf0e00000, 0xf0e00200, 0xf0e00400, 0xf0e00600, 0xf0e00800,
		0xf0e00a00, 0xf0e00c00, 0xf0e00e00, 0xf0e00ff0, },
	{0xf0ff0000, 0xf0ff0200, 0xf0ff0400, 0xf0ff0600, 0xf0ff0800,
		0xf0ff0a00, 0xf0ff0c00, 0xf0ff0e00, 0xf0ff0ff0, },
	},
};
static u32 g_gmp_lut_table_high4bit[9][9][9] = {
	{
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
	},
	{
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	{0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, },
	},
	{
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	{0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, },
	},
	{
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	{0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, },
	},
	{
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	{0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, },
	},
	{
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	{0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, },
	},
	{
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	{0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, },
	},
	{
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	{0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, },
	},
	{
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	{0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
	},
};

/* GAMMA */
static u32 g_gamma_lut_table_r[] = {
	0, 16, 32, 48, 64, 80, 96, 112, 128, 144,
	160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464,
	480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
	640, 656, 672, 688, 704, 720, 736, 752, 768, 784,
	800, 816, 832, 848, 864, 880, 896, 912, 928, 944,
	960, 976, 992, 1008, 1024, 1040, 1056, 1072, 1088, 1104,
	1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424,
	1440, 1456, 1472, 1488, 1504, 1520, 1536, 1552, 1568, 1584,
	1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744,
	1760, 1776, 1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904,
	1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032, 2048, 2064,
	2080, 2096, 2112, 2128, 2144, 2160, 2176, 2192, 2208, 2224,
	2240, 2256, 2272, 2288, 2304, 2320, 2336, 2352, 2368, 2384,
	2400, 2416, 2432, 2448, 2464, 2480, 2496, 2512, 2528, 2544,
	2560, 2576, 2592, 2608, 2624, 2640, 2656, 2672, 2688, 2704,
	2720, 2736, 2752, 2768, 2784, 2800, 2816, 2832, 2848, 2864,
	2880, 2896, 2912, 2928, 2944, 2960, 2976, 2992, 3008, 3024,
	3040, 3056, 3072, 3088, 3104, 3120, 3136, 3152, 3168, 3184,
	3200, 3216, 3232, 3248, 3264, 3280, 3296, 3312, 3328, 3344,
	3360, 3376, 3392, 3408, 3424, 3440, 3456, 3472, 3488, 3504,
	3520, 3536, 3552, 3568, 3584, 3600, 3616, 3632, 3648, 3664,
	3680, 3696, 3712, 3728, 3744, 3760, 3776, 3792, 3808, 3824,
	3840, 3856, 3872, 3888, 3904, 3920, 3936, 3952, 3968, 3984,
	4000, 4016, 4032, 4048, 4064, 4080, 4095
};
static u32 g_gamma_lut_table_g[] = {
	0, 16, 32, 48, 64, 80, 96, 112, 128, 144,
	160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464,
	480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
	640, 656, 672, 688, 704, 720, 736, 752, 768, 784,
	800, 816, 832, 848, 864, 880, 896, 912, 928, 944,
	960, 976, 992, 1008, 1024, 1040, 1056, 1072, 1088, 1104,
	1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424,
	1440, 1456, 1472, 1488, 1504, 1520, 1536, 1552, 1568, 1584,
	1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744,
	1760, 1776, 1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904,
	1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032, 2048, 2064,
	2080, 2096, 2112, 2128, 2144, 2160, 2176, 2192, 2208, 2224,
	2240, 2256, 2272, 2288, 2304, 2320, 2336, 2352, 2368, 2384,
	2400, 2416, 2432, 2448, 2464, 2480, 2496, 2512, 2528, 2544,
	2560, 2576, 2592, 2608, 2624, 2640, 2656, 2672, 2688, 2704,
	2720, 2736, 2752, 2768, 2784, 2800, 2816, 2832, 2848, 2864,
	2880, 2896, 2912, 2928, 2944, 2960, 2976, 2992, 3008, 3024,
	3040, 3056, 3072, 3088, 3104, 3120, 3136, 3152, 3168, 3184,
	3200, 3216, 3232, 3248, 3264, 3280, 3296, 3312, 3328, 3344,
	3360, 3376, 3392, 3408, 3424, 3440, 3456, 3472, 3488, 3504,
	3520, 3536, 3552, 3568, 3584, 3600, 3616, 3632, 3648, 3664,
	3680, 3696, 3712, 3728, 3744, 3760, 3776, 3792, 3808, 3824,
	3840, 3856, 3872, 3888, 3904, 3920, 3936, 3952, 3968, 3984,
	4000, 4016, 4032, 4048, 4064, 4080, 4095
};
static u32 g_gamma_lut_table_b[] = {
	0, 16, 32, 48, 64, 80, 96, 112, 128, 144,
	160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464,
	480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
	640, 656, 672, 688, 704, 720, 736, 752, 768, 784,
	800, 816, 832, 848, 864, 880, 896, 912, 928, 944,
	960, 976, 992, 1008, 1024, 1040, 1056, 1072, 1088, 1104,
	1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424,
	1440, 1456, 1472, 1488, 1504, 1520, 1536, 1552, 1568, 1584,
	1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744,
	1760, 1776, 1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904,
	1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032, 2048, 2064,
	2080, 2096, 2112, 2128, 2144, 2160, 2176, 2192, 2208, 2224,
	2240, 2256, 2272, 2288, 2304, 2320, 2336, 2352, 2368, 2384,
	2400, 2416, 2432, 2448, 2464, 2480, 2496, 2512, 2528, 2544,
	2560, 2576, 2592, 2608, 2624, 2640, 2656, 2672, 2688, 2704,
	2720, 2736, 2752, 2768, 2784, 2800, 2816, 2832, 2848, 2864,
	2880, 2896, 2912, 2928, 2944, 2960, 2976, 2992, 3008, 3024,
	3040, 3056, 3072, 3088, 3104, 3120, 3136, 3152, 3168, 3184,
	3200, 3216, 3232, 3248, 3264, 3280, 3296, 3312, 3328, 3344,
	3360, 3376, 3392, 3408, 3424, 3440, 3456, 3472, 3488, 3504,
	3520, 3536, 3552, 3568, 3584, 3600, 3616, 3632, 3648, 3664,
	3680, 3696, 3712, 3728, 3744, 3760, 3776, 3792, 3808, 3824,
	3840, 3856, 3872, 3888, 3904, 3920, 3936, 3952, 3968, 3984,
	4000, 4016, 4032, 4048, 4064, 4080, 4095
};

/* IGM */
static u32 g_igm_lut_table_r[] = {
	0, 16, 32, 48, 64, 80, 96, 112, 128, 144,
	160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464,
	480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
	640, 656, 672, 688, 704, 720, 736, 752, 768, 784,
	800, 816, 832, 848, 864, 880, 896, 912, 928, 944,
	960, 976, 992, 1008, 1024, 1040, 1056, 1072, 1088, 1104,
	1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424,
	1440, 1456, 1472, 1488, 1504, 1520, 1536, 1552, 1568, 1584,
	1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744,
	1760, 1776, 1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904,
	1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032, 2048, 2064,
	2080, 2096, 2112, 2128, 2144, 2160, 2176, 2192, 2208, 2224,
	2240, 2256, 2272, 2288, 2304, 2320, 2336, 2352, 2368, 2384,
	2400, 2416, 2432, 2448, 2464, 2480, 2496, 2512, 2528, 2544,
	2560, 2576, 2592, 2608, 2624, 2640, 2656, 2672, 2688, 2704,
	2720, 2736, 2752, 2768, 2784, 2800, 2816, 2832, 2848, 2864,
	2880, 2896, 2912, 2928, 2944, 2960, 2976, 2992, 3008, 3024,
	3040, 3056, 3072, 3088, 3104, 3120, 3136, 3152, 3168, 3184,
	3200, 3216, 3232, 3248, 3264, 3280, 3296, 3312, 3328, 3344,
	3360, 3376, 3392, 3408, 3424, 3440, 3456, 3472, 3488, 3504,
	3520, 3536, 3552, 3568, 3584, 3600, 3616, 3632, 3648, 3664,
	3680, 3696, 3712, 3728, 3744, 3760, 3776, 3792, 3808, 3824,
	3840, 3856, 3872, 3888, 3904, 3920, 3936, 3952, 3968, 3984,
	4000, 4016, 4032, 4048, 4064, 4080, 4095
};
static u32 g_igm_lut_table_g[] = {
	0, 16, 32, 48, 64, 80, 96, 112, 128, 144,
	160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464,
	480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
	640, 656, 672, 688, 704, 720, 736, 752, 768, 784,
	800, 816, 832, 848, 864, 880, 896, 912, 928, 944,
	960, 976, 992, 1008, 1024, 1040, 1056, 1072, 1088, 1104,
	1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424,
	1440, 1456, 1472, 1488, 1504, 1520, 1536, 1552, 1568, 1584,
	1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744,
	1760, 1776, 1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904,
	1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032, 2048, 2064,
	2080, 2096, 2112, 2128, 2144, 2160, 2176, 2192, 2208, 2224,
	2240, 2256, 2272, 2288, 2304, 2320, 2336, 2352, 2368, 2384,
	2400, 2416, 2432, 2448, 2464, 2480, 2496, 2512, 2528, 2544,
	2560, 2576, 2592, 2608, 2624, 2640, 2656, 2672, 2688, 2704,
	2720, 2736, 2752, 2768, 2784, 2800, 2816, 2832, 2848, 2864,
	2880, 2896, 2912, 2928, 2944, 2960, 2976, 2992, 3008, 3024,
	3040, 3056, 3072, 3088, 3104, 3120, 3136, 3152, 3168, 3184,
	3200, 3216, 3232, 3248, 3264, 3280, 3296, 3312, 3328, 3344,
	3360, 3376, 3392, 3408, 3424, 3440, 3456, 3472, 3488, 3504,
	3520, 3536, 3552, 3568, 3584, 3600, 3616, 3632, 3648, 3664,
	3680, 3696, 3712, 3728, 3744, 3760, 3776, 3792, 3808, 3824,
	3840, 3856, 3872, 3888, 3904, 3920, 3936, 3952, 3968, 3984,
	4000, 4016, 4032, 4048, 4064, 4080, 4095
};
static u32 g_igm_lut_table_b[] = {
	0, 16, 32, 48, 64, 80, 96, 112, 128, 144,
	160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464,
	480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
	640, 656, 672, 688, 704, 720, 736, 752, 768, 784,
	800, 816, 832, 848, 864, 880, 896, 912, 928, 944,
	960, 976, 992, 1008, 1024, 1040, 1056, 1072, 1088, 1104,
	1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424,
	1440, 1456, 1472, 1488, 1504, 1520, 1536, 1552, 1568, 1584,
	1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744,
	1760, 1776, 1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904,
	1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032, 2048, 2064,
	2080, 2096, 2112, 2128, 2144, 2160, 2176, 2192, 2208, 2224,
	2240, 2256, 2272, 2288, 2304, 2320, 2336, 2352, 2368, 2384,
	2400, 2416, 2432, 2448, 2464, 2480, 2496, 2512, 2528, 2544,
	2560, 2576, 2592, 2608, 2624, 2640, 2656, 2672, 2688, 2704,
	2720, 2736, 2752, 2768, 2784, 2800, 2816, 2832, 2848, 2864,
	2880, 2896, 2912, 2928, 2944, 2960, 2976, 2992, 3008, 3024,
	3040, 3056, 3072, 3088, 3104, 3120, 3136, 3152, 3168, 3184,
	3200, 3216, 3232, 3248, 3264, 3280, 3296, 3312, 3328, 3344,
	3360, 3376, 3392, 3408, 3424, 3440, 3456, 3472, 3488, 3504,
	3520, 3536, 3552, 3568, 3584, 3600, 3616, 3632, 3648, 3664,
	3680, 3696, 3712, 3728, 3744, 3760, 3776, 3792, 3808, 3824,
	3840, 3856, 3872, 3888, 3904, 3920, 3936, 3952, 3968, 3984,
	4000, 4016, 4032, 4048, 4064, 4080, 4095
};

static u32 g_xcc_table[12] = {0x0, 0x8000, 0x0, 0x0, 0x0, 0x0,
	0x8000, 0x0, 0x0, 0x0, 0x0, 0x8000,};

uint32_t g_lcd_read_value[4] = {0};
uint32_t g_lcd_expected_value[4] = {0x1c, 0x00, 0x07, 0x00};
uint32_t g_lcd_read_mask[4] = {0xFF, 0xFF, 0x07, 0xFF};
char *g_lcd_reg_name[4] = {"power mode", "MADCTR",
	"pixel format", "image mode"};
char g_lcd_reg_0a[] = {0x0a};
char g_lcd_reg_0b[] = {0x0b};
char g_lcd_reg_0c[] = {0x0c};
char g_lcd_reg_0d[] = {0x0d};

struct dsi_cmd_desc g_lcd_check_reg[] = {
	{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
		sizeof(g_lcd_reg_0a), g_lcd_reg_0a},
	{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
		sizeof(g_lcd_reg_0b), g_lcd_reg_0b},
	{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
		sizeof(g_lcd_reg_0c), g_lcd_reg_0c},
	{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
		sizeof(g_lcd_reg_0d), g_lcd_reg_0d},
};

struct mipi_dsi_read_compare_data g_lcd_reg_data = {
	.read_value = g_lcd_read_value,
	.expected_value = g_lcd_expected_value,
	.read_mask = g_lcd_expected_value,
	.reg_name = g_lcd_reg_name,
	.log_on = 1,
	.cmds = g_lcd_check_reg,
	.cnt = ARRAY_SIZE(g_lcd_check_reg),
};

static int mipi_lcd_panel_set_fastboot(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	BUG_ON(pdev == NULL);
	dpufd = platform_get_drvdata(pdev);
	BUG_ON(dpufd == NULL);

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (g_lcd_fpga_flag == USED_FOR_UDP) {
		pinctrl_cmds_tx(pdev, g_lcd_pinctrl_normal_cmds,
			ARRAY_SIZE(g_lcd_pinctrl_normal_cmds));
		gpio_cmds_tx(g_asic_lcd_gpio_request_cmds,
			ARRAY_SIZE(g_asic_lcd_gpio_request_cmds));
	} else {
		gpio_cmds_tx(g_fpga_lcd_gpio_request_cmds,
			ARRAY_SIZE(g_fpga_lcd_gpio_request_cmds));
	}

	hisi_lcd_backlight_on(pdev);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;
}

static uint32_t mipi_read_ic_regs(char __iomem *mipi_dsi0_base,
	uint32_t rd_cmd, struct dpu_fb_data_type *dpufd)
{
	uint32_t status;
	uint32_t try_times = 0;

	outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, rd_cmd);
	status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);

	/* wait for 5ms until reading status timeout */
	while (status & 0x10) {
		udelay(50);
		if (++try_times > 100) {
			DPU_FB_ERR("Read lcd power status timeout!\n");
			break;
		}
		status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
	}
	status = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
	DPU_FB_INFO("fb%d, TD4322 LCD Power State = 0x%x\n",
		dpufd->index, status);

	return 0;
}

static void mipi_lcd_panel_power_on(struct platform_device *pdev)
{
	if (g_lcd_fpga_flag == USED_FOR_UDP) {
		vcc_cmds_tx(pdev, g_lcd_vcc_enable_cmds,
			ARRAY_SIZE(g_lcd_vcc_enable_cmds));
		pinctrl_cmds_tx(pdev, g_lcd_pinctrl_normal_cmds,
			ARRAY_SIZE(g_lcd_pinctrl_normal_cmds));
		gpio_cmds_tx(g_asic_lcd_gpio_request_cmds,
			ARRAY_SIZE(g_asic_lcd_gpio_request_cmds));
		gpio_cmds_tx(g_asic_lcd_gpio_normal_cmds,
			ARRAY_SIZE(g_asic_lcd_gpio_normal_cmds));
	} else {
		gpio_cmds_tx(g_fpga_lcd_gpio_request_cmds,
			ARRAY_SIZE(g_fpga_lcd_gpio_request_cmds));
		gpio_cmds_tx(g_fpga_lcd_gpio_normal_cmds,
			ARRAY_SIZE(g_fpga_lcd_gpio_normal_cmds));
	}
}

static void mipi_lp_send_sequence(char __iomem *mipi_dsi0_base,
	uint32_t bl_set_type)
{
	mipi_dsi_cmds_tx(g_set_display_address,
		ARRAY_SIZE(g_set_display_address), mipi_dsi0_base);
	mipi_dsi_cmds_tx(g_lcd_display_on_cmds,
		ARRAY_SIZE(g_lcd_display_on_cmds), mipi_dsi0_base);
	if (g_fpga_flag == 1)
		mipi_dsi_cmds_tx(g_lg_fpga_modify_phyclk_config_cmds,
			ARRAY_SIZE(g_lg_fpga_modify_phyclk_config_cmds),
			mipi_dsi0_base);

	if ((bl_set_type & BL_SET_BY_BLPWM) ||
		(bl_set_type & BL_SET_BY_SH_BLPWM))
		mipi_dsi_cmds_tx(g_pwm_out_on_cmds,
			ARRAY_SIZE(g_pwm_out_on_cmds), mipi_dsi0_base);
}

static int mipi_lcd_panel_on(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is null\n");
		return 0;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null\n");
		return 0;
	}

	DPU_FB_INFO("fb%d, lcd panel on +\n", dpufd->index);

	pinfo = &(dpufd->panel_info);
	mipi_dsi0_base = dpufd->mipi_dsi0_base;

	if (pinfo->lcd_init_step == LCD_INIT_POWER_ON) {
		mipi_lcd_panel_power_on(pdev);
		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		mipi_lp_send_sequence(mipi_dsi0_base, pinfo->bl_set_type);
		mipi_read_ic_regs(mipi_dsi0_base, 0x0a06, dpufd);
		pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE) {

	} else {
		DPU_FB_ERR("failed to init lcd!\n");
	}
	hisi_lcd_backlight_on(pdev);
	DPU_FB_DEBUG("fb%d, -!\n", dpufd->index);

	return 0;
}

static void mipi_lcd_panel_power_off(struct platform_device *pdev)
{
	if (g_lcd_fpga_flag == USED_FOR_UDP) {
		gpio_cmds_tx(g_asic_lcd_gpio_lowpower_cmds,
			ARRAY_SIZE(g_asic_lcd_gpio_lowpower_cmds));
		gpio_cmds_tx(g_asic_lcd_gpio_free_cmds,
			ARRAY_SIZE(g_asic_lcd_gpio_free_cmds));
		pinctrl_cmds_tx(pdev, g_lcd_pinctrl_lowpower_cmds,
			ARRAY_SIZE(g_lcd_pinctrl_lowpower_cmds));
		mdelay(3); /**/
		vcc_cmds_tx(pdev, g_lcd_vcc_disable_cmds,
			ARRAY_SIZE(g_lcd_vcc_disable_cmds));
	} else {
		gpio_cmds_tx(g_fpga_lcd_gpio_lowpower_cmds,
			ARRAY_SIZE(g_fpga_lcd_gpio_lowpower_cmds));
		gpio_cmds_tx(g_fpga_lcd_gpio_free_cmds,
			ARRAY_SIZE(g_fpga_lcd_gpio_free_cmds));
	}
}

static int mipi_lcd_panel_off(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;

	BUG_ON(pdev == NULL);
	dpufd = platform_get_drvdata(pdev);
	BUG_ON(dpufd == NULL);
	pinfo = &(dpufd->panel_info);
	mipi_dsi0_base = dpufd->mipi_dsi0_base;

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE) {
		hisi_lcd_backlight_off(pdev);

		mipi_dsi_cmds_tx(g_lcd_display_off_cmds,
			ARRAY_SIZE(g_lcd_display_off_cmds), mipi_dsi0_base);
		pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_LP_SEND_SEQUENCE) {
		pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;
	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_POWER_OFF) {
		mipi_lcd_panel_power_off(pdev);
	} else {
		DPU_FB_ERR("failed to uninit lcd!\n");
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);
	return 0;
}

static int mipi_lcd_panel_remove(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;

	BUG_ON(pdev == NULL);
	dpufd = platform_get_drvdata(pdev);
	BUG_ON(dpufd == NULL);

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (g_lcd_fpga_flag == USED_FOR_UDP) {
		vcc_cmds_tx(pdev, g_lcd_vcc_finit_cmds,
			ARRAY_SIZE(g_lcd_vcc_finit_cmds));
		pinctrl_cmds_tx(pdev, g_lcd_pinctrl_finit_cmds,
			ARRAY_SIZE(g_lcd_pinctrl_finit_cmds));
	}
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;
}

static int mipi_lcd_set_backlight(struct platform_device *pdev,
	uint32_t bl_level)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int ret = 0;
	static char last_bl_level;
	char bl_level_adjust[2] = {
		0x51,
		0x00,
	};
	struct dsi_cmd_desc lcd_bl_level_adjust[] = {
		{DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
			sizeof(bl_level_adjust), bl_level_adjust},
	};

	BUG_ON(pdev == NULL);
	dpufd = platform_get_drvdata(pdev);
	BUG_ON(dpufd == NULL);

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	if (dpufd->panel_info.bl_set_type & BL_SET_BY_PWM) {
		ret = hisi_pwm_set_backlight(dpufd, bl_level);
	} else if (dpufd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {
		ret = hisi_blpwm_set_backlight(dpufd, bl_level);
	} else if (dpufd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {
		ret = hisi_sh_blpwm_set_backlight(dpufd, bl_level);
	} else if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
		bl_level_adjust[1] = bl_level * 255 / dpufd->panel_info.bl_max;

		if (last_bl_level != bl_level_adjust[1]) {
			last_bl_level = bl_level_adjust[1];
			mipi_dsi_cmds_tx(lcd_bl_level_adjust,
				ARRAY_SIZE(lcd_bl_level_adjust),
				dpufd->mipi_dsi0_base);
		}
		DPU_FB_INFO("bl_level_adjust[1] = %d\n", bl_level_adjust[1]);
	} else {
		DPU_FB_ERR("fb%d, not support this bl_set_type %d!\n",
			dpufd->index, dpufd->panel_info.bl_set_type);
	}
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);
	return ret;
}

static int mipi_lcd_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;

	BUG_ON(pdev == NULL || dirty == NULL);
	dpufd = platform_get_drvdata(pdev);
	BUG_ON(dpufd == NULL);

	pinfo = &(dpufd->panel_info);

	/*lint -e574 */
	if (((dirty->x % 2) != 0) || ((dirty->y % 2) != 0) ||
		((dirty->w % 2) != 0) || ((dirty->h % 2) != 0) ||
		(dirty->x >= pinfo->xres) || (dirty->w > pinfo->xres) ||
		((dirty->x + dirty->w) > pinfo->xres) ||
		(dirty->y >= pinfo->yres) || (dirty->h > pinfo->yres) ||
		((dirty->y + dirty->h) > pinfo->yres))
		DPU_FB_ERR("dirty_region : %d,%d, %d,%d not support!\n",
			dirty->x, dirty->y, dirty->w, dirty->h);
	/*lint -e574 */

	g_lcd_disp_x[1] = ((uint32_t)(dirty->x) >> 8) & 0xff;
	g_lcd_disp_x[2] = ((uint32_t)(dirty->x)) & 0xff;
	g_lcd_disp_x[3] = ((uint32_t)(dirty->x + dirty->w - 1) >> 8) & 0xff;
	g_lcd_disp_x[4] = ((uint32_t)(dirty->x + dirty->w - 1)) & 0xff;
	g_lcd_disp_y[1] = ((uint32_t)(dirty->y) >> 8) & 0xff;
	g_lcd_disp_y[2] = ((uint32_t)(dirty->y)) & 0xff;
	g_lcd_disp_y[3] = ((uint32_t)(dirty->y + dirty->h - 1) >> 8) & 0xff;
	g_lcd_disp_y[4] = ((uint32_t)(dirty->y + dirty->h - 1)) & 0xff;

	mipi_dsi_cmds_tx(g_set_display_address,
		ARRAY_SIZE(g_set_display_address), dpufd->mipi_dsi0_base);

	return 0;
}

static ssize_t mipi_lcd_model_show(struct platform_device *pdev,
	char *buf)
{
	struct dpu_fb_data_type *dpufd = NULL;
	ssize_t ret;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	ret = snprintf(buf, PAGE_SIZE, "LG_TD4322_6P0 ' CMD TFT Boston\n");

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static ssize_t mipi_lcd_check_reg_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;
	char __iomem *mipi_dsi0_base = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	mipi_dsi0_base = dpufd->mipi_dsi0_base;

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (!mipi_dsi_read_compare(&g_lcd_reg_data, mipi_dsi0_base))
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	else
		ret = snprintf(buf, PAGE_SIZE, "ERROR\n");

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return ret;
}

static struct dpu_panel_info g_panel_info = {0};
static struct dpu_fb_panel_data g_panel_data = {
	.panel_info = &g_panel_info,
	.set_fastboot = mipi_lcd_panel_set_fastboot,
	.on = mipi_lcd_panel_on,
	.off = mipi_lcd_panel_off,
	.remove = mipi_lcd_panel_remove,
	.set_backlight = mipi_lcd_set_backlight,
	.set_display_region = mipi_lcd_set_display_region,
	.lcd_model_show = mipi_lcd_model_show,
	.lcd_check_reg  = mipi_lcd_check_reg_show,
};

/* panel info initialized value from panel spec */
static void mipi_lcd_init_panel_info(struct dpu_panel_info *pinfo,
	uint32_t bl_type, uint32_t lcd_display_type)
{
	pinfo->xres = 1080;
	pinfo->yres = 1920;
	pinfo->width = 75;
	pinfo->height = 133;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = bl_type;
	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;
	pinfo->type = lcd_display_type;
	pinfo->ifbc_type = IFBC_TYPE_NONE;
	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;
	pinfo->esd_skip_mipi_check = 0;
	pinfo->lcd_uninit_step_support = 1;

	pinfo->cinema_mode_support = 0;
}

/* gamma initialized value from panel spec */
static void mipi_lcd_init_gamma(struct dpu_panel_info *pinfo)
{
	pinfo->igm_lut_table_R = g_igm_lut_table_r;
	pinfo->igm_lut_table_G = g_igm_lut_table_g;
	pinfo->igm_lut_table_B = g_igm_lut_table_b;
	pinfo->igm_lut_table_len = ARRAY_SIZE(g_igm_lut_table_r);
	pinfo->gamma_lut_table_R = g_gamma_lut_table_r;
	pinfo->gamma_lut_table_G = g_gamma_lut_table_g;
	pinfo->gamma_lut_table_B = g_gamma_lut_table_b;
	pinfo->gamma_lut_table_len = ARRAY_SIZE(g_gamma_lut_table_r);

	pinfo->xcc_support = 1;
	pinfo->xcc_table = g_xcc_table;
	pinfo->xcc_table_len = ARRAY_SIZE(g_xcc_table);

	pinfo->gmp_support = 0;
	pinfo->gmp_lut_table_low32bit = &g_gmp_lut_table_low32bit[0][0][0];
	pinfo->gmp_lut_table_high4bit = &g_gmp_lut_table_high4bit[0][0][0];
	pinfo->gmp_lut_table_len = ARRAY_SIZE(g_gmp_lut_table_low32bit);
}

/* hiace initialized value from panel spec */
static void mipi_lcd_init_hiace(struct dpu_panel_info *pinfo)
{
	pinfo->noisereduction_support = 1;
	pinfo->hiace_param.iGlobalHistBlackPos = 16;
	pinfo->hiace_param.iGlobalHistWhitePos = 240;
	pinfo->hiace_param.iGlobalHistBlackWeight = 51;
	pinfo->hiace_param.iGlobalHistWhiteWeight = 51;
	pinfo->hiace_param.iGlobalHistZeroCutRatio = 486;
	pinfo->hiace_param.iGlobalHistSlopeCutRatio = 410;
	pinfo->hiace_param.iMaxLcdLuminance = 500;
	pinfo->hiace_param.iMinLcdLuminance = 3;
	strncpy(pinfo->hiace_param.chCfgName,
		"/product/etc/display/effect/algorithm/hdr_engine_MHA.xml",
		sizeof(pinfo->hiace_param.chCfgName) - 1);
}

static void mipi_init_acm_lut_table(struct dpu_panel_info *pinfo)
{
	pinfo->acm_lut_hue_table = g_acm_lut_hue_table;
	pinfo->acm_lut_hue_table_len = ARRAY_SIZE(g_acm_lut_hue_table);
	pinfo->acm_lut_sata_table = g_acm_lut_sata_table;
	pinfo->acm_lut_sata_table_len = ARRAY_SIZE(g_acm_lut_sata_table);
	pinfo->acm_lut_satr0_table = g_acm_lut_satr0_table;
	pinfo->acm_lut_satr0_table_len = ARRAY_SIZE(g_acm_lut_satr0_table);
	pinfo->acm_lut_satr1_table = g_acm_lut_satr1_table;
	pinfo->acm_lut_satr1_table_len = ARRAY_SIZE(g_acm_lut_satr1_table);
	pinfo->acm_lut_satr2_table = g_acm_lut_satr2_table;
	pinfo->acm_lut_satr2_table_len = ARRAY_SIZE(g_acm_lut_satr2_table);
	pinfo->acm_lut_satr3_table = g_acm_lut_satr3_table;
	pinfo->acm_lut_satr3_table_len = ARRAY_SIZE(g_acm_lut_satr3_table);
	pinfo->acm_lut_satr4_table = g_acm_lut_satr4_table;
	pinfo->acm_lut_satr4_table_len = ARRAY_SIZE(g_acm_lut_satr4_table);
	pinfo->acm_lut_satr5_table = g_acm_lut_satr5_table;
	pinfo->acm_lut_satr5_table_len = ARRAY_SIZE(g_acm_lut_satr5_table);
	pinfo->acm_lut_satr6_table = g_acm_lut_satr6_table;
	pinfo->acm_lut_satr6_table_len = ARRAY_SIZE(g_acm_lut_satr6_table);
	pinfo->acm_lut_satr7_table = g_acm_lut_satr7_table;
	pinfo->acm_lut_satr7_table_len = ARRAY_SIZE(g_acm_lut_satr7_table);
}

static void mipi_lcd_init_acm(struct dpu_panel_info *pinfo)
{
	mipi_init_acm_lut_table(pinfo);

	pinfo->r0_hh = g_acm_r0_hh;
	pinfo->r0_lh = g_acm_r0_lh;
	pinfo->r1_hh = g_acm_r1_hh;
	pinfo->r1_lh = g_acm_r1_lh;
	pinfo->r2_hh = g_acm_r2_hh;
	pinfo->r2_lh = g_acm_r2_lh;
	pinfo->r3_hh = g_acm_r3_hh;
	pinfo->r3_lh = g_acm_r3_lh;
	pinfo->r4_hh = g_acm_r4_hh;
	pinfo->r4_lh = g_acm_r4_lh;
	pinfo->r5_hh = g_acm_r5_hh;
	pinfo->r5_lh = g_acm_r5_lh;
	pinfo->r6_hh = g_acm_r6_hh;
	pinfo->r6_lh = g_acm_r6_lh;

	pinfo->hue_param01 = g_acm_hue_param01;
	pinfo->hue_param23 = g_acm_hue_param23;
	pinfo->hue_param45 = g_acm_hue_param45;
	pinfo->hue_param67 = g_acm_hue_param67;
	pinfo->hue_smooth0 = g_acm_hue_smooth0;
	pinfo->hue_smooth1 = g_acm_hue_smooth1;
	pinfo->hue_smooth2 = g_acm_hue_smooth2;
	pinfo->hue_smooth3 = g_acm_hue_smooth3;
	pinfo->hue_smooth4 = g_acm_hue_smooth4;
	pinfo->hue_smooth5 = g_acm_hue_smooth5;
	pinfo->hue_smooth6 = g_acm_hue_smooth6;
	pinfo->hue_smooth7 = g_acm_hue_smooth7;
	pinfo->color_choose = g_acm_color_choose;
	pinfo->l_cont_en = g_acm_l_cont_en;
	pinfo->lc_param01 = g_acm_lc_param01;
	pinfo->lc_param23 = g_acm_lc_param23;
	pinfo->lc_param45 = g_acm_lc_param45;
	pinfo->lc_param67 = g_acm_lc_param67;
	pinfo->l_adj_ctrl = g_acm_l_adj_ctrl;
	pinfo->capture_ctrl = g_acm_capture_ctrl;
	pinfo->capture_in = g_acm_capture_in;
	pinfo->capture_out = g_acm_capture_out;
	pinfo->ink_ctrl = g_acm_ink_ctrl;
	pinfo->ink_out = g_acm_ink_out;

	pinfo->acm_ce_support = 1;
}

static void mipi_lcd_init_display_effect(struct dpu_panel_info *pinfo)
{
	pinfo->gamma_support = 0;
	if (pinfo->gamma_support == 1)
		mipi_lcd_init_gamma(pinfo);

	pinfo->gmp_support = 0;
	pinfo->gmp_lut_table_low32bit = &g_gmp_lut_table_low32bit[0][0][0];
	pinfo->gmp_lut_table_high4bit = &g_gmp_lut_table_high4bit[0][0][0];
	pinfo->gmp_lut_table_len = ARRAY_SIZE(g_gmp_lut_table_low32bit);

	pinfo->hiace_support = 0;
	if (pinfo->hiace_support == 1)
		mipi_lcd_init_hiace(pinfo);

	pinfo->acm_support = 0;
	if (pinfo->acm_support == 1)
		mipi_lcd_init_acm(pinfo);

	pinfo->arsr1p_sharpness_support = 0;
	pinfo->prefix_sharpness2D_support = 0;
}

/* dirty region initialized value from panel spec */
static void mipi_lcd_init_dirty_region(struct dpu_panel_info *pinfo)
{
	pinfo->dirty_region_updt_support = 1;
	pinfo->dirty_region_info.left_align = -1;
	pinfo->dirty_region_info.right_align = -1;
	pinfo->dirty_region_info.top_align = 32;
	pinfo->dirty_region_info.bottom_align = 32;
	pinfo->dirty_region_info.w_align = -1;
	pinfo->dirty_region_info.h_align = -1;
	pinfo->dirty_region_info.w_min = 1080;
	pinfo->dirty_region_info.h_min = -1;
	pinfo->dirty_region_info.top_start = -1;
	pinfo->dirty_region_info.bottom_start = -1;
}

/* vsec rc param initialized value from panel spec */
static void mipi_init_vsec_rc_param(struct dpu_panel_info *pinfo)
{
	// DSC_RC_RANGE_PARAM0: 0x1020100
	pinfo->vesa_dsc.range_min_qp0 = (0x1020100 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp0 = (0x1020100 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset0 = (0x1020100 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp1 = (0x1020100 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp1 = (0x1020100 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset1 = (0x1020100 >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM1: 0x94009be,
	pinfo->vesa_dsc.range_min_qp2 = (0x94009be >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp2 = (0x94009be >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset2 = (0x94009be >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp3 = (0x94009be >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp3 = (0x94009be >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset3 = (0x94009be >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM2, 0x19fc19fa,
	pinfo->vesa_dsc.range_min_qp4 = (0x19fc19fa >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp4 = (0x19fc19fa >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset4 = (0x19fc19fa >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp5 = (0x19fc19fa >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp5 = (0x19fc19fa >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset5 = (0x19fc19fa >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM3, 0x19f81a38,
	pinfo->vesa_dsc.range_min_qp6 = (0x19f81a38 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp6 = (0x19f81a38 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset6 = (0x19f81a38 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp7 = (0x19f81a38 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp7 = (0x19f81a38 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset7 = (0x19f81a38 >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM4, 0x1a781ab6,
	pinfo->vesa_dsc.range_min_qp8 = (0x1a781ab6 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp8 = (0x1a781ab6 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset8 = (0x1a781ab6 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp9 = (0x1a781ab6 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp9 = (0x1a781ab6 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset9 = (0x1a781ab6 >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM5, 0x2af62b34,
	pinfo->vesa_dsc.range_min_qp10 = (0x2af62b34 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp10 = (0x2af62b34 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset10 = (0x2af62b34 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp11 = (0x2af62b34 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp11 = (0x2af62b34 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset11 = (0x2af62b34 >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM6, 0x2b743b74,
	pinfo->vesa_dsc.range_min_qp12 = (0x2b743b74 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp12 = (0x2b743b74 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset12 = (0x2b743b74 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp13 = (0x2b743b74 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp13 = (0x2b743b74 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset13 = (0x2b743b74 >> 0) & 0x3F;

	// DSC_RC_RANGE_PARAM7, 0x6bf40000,
	pinfo->vesa_dsc.range_min_qp14 = (0x6bf40000 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp14 = (0x6bf40000 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset14 = (0x6bf40000 >> 16) & 0x3F;
}

/* vsec initialized value from panel spec */
static void mipi_lcd_init_vsec(struct dpu_panel_info *pinfo)
{
	// dsc parameter info
	pinfo->vesa_dsc.bits_per_component = 8;
	pinfo->vesa_dsc.bits_per_pixel = 8;
	pinfo->vesa_dsc.slice_width = 1079;
	pinfo->vesa_dsc.slice_height = 15;

	pinfo->vesa_dsc.initial_xmit_delay = 512;
	pinfo->vesa_dsc.first_line_bpg_offset = 12;
	pinfo->vesa_dsc.mux_word_size = 48;

	// DSC_CTRL
	pinfo->vesa_dsc.block_pred_enable = 1;
	pinfo->vesa_dsc.linebuf_depth = 9;

	// RC_PARAM3
	pinfo->vesa_dsc.initial_offset = 6144;

	// FLATNESS_QP_TH
	pinfo->vesa_dsc.flatness_min_qp = 3;
	pinfo->vesa_dsc.flatness_max_qp = 12;

	// DSC_PARAM4
	pinfo->vesa_dsc.rc_edge_factor = 0x6;
	pinfo->vesa_dsc.rc_model_size = 8192;

	// DSC_RC_PARAM5: 0x330b0b
	pinfo->vesa_dsc.rc_tgt_offset_lo = (0x330b0b >> 20) & 0xF;
	pinfo->vesa_dsc.rc_tgt_offset_hi = (0x330b0b >> 16) & 0xF;
	pinfo->vesa_dsc.rc_quant_incr_limit1 = (0x330b0b >> 8) & 0x1F;
	pinfo->vesa_dsc.rc_quant_incr_limit0 = (0x330b0b >> 0) & 0x1F;

	// DSC_RC_BUF_THRESH0: 0xe1c2a38
	pinfo->vesa_dsc.rc_buf_thresh0 = (0xe1c2a38 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh1 = (0xe1c2a38 >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh2 = (0xe1c2a38 >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh3 = (0xe1c2a38 >> 0) & 0xFF;

	// DSC_RC_BUF_THRESH1: 0x46546269
	pinfo->vesa_dsc.rc_buf_thresh4 = (0x46546269 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh5 = (0x46546269 >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh6 = (0x46546269 >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh7 = (0x46546269 >> 0) & 0xFF;

	// DSC_RC_BUF_THRESH2: 0x7077797b
	pinfo->vesa_dsc.rc_buf_thresh8 = (0x7077797b >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh9 = (0x7077797b >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh10 = (0x7077797b >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh11 = (0x7077797b >> 0) & 0xFF;

	// DSC_RC_BUF_THRESH3: 0x7d7e0000
	pinfo->vesa_dsc.rc_buf_thresh12 = (0x7d7e0000 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh13 = (0x7d7e0000 >> 16) & 0xFF;

	mipi_init_vsec_rc_param(pinfo);
}

/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param(struct dpu_panel_info *pinfo)
{
	if (g_lcd_fpga_flag == USED_FOR_FPGA) {
		pinfo->mipi.dsi_bit_clk = 120;
		pinfo->dsi_bit_clk_upt_support = 0;
		pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
		pinfo->pxl_clk_rate = 20 * 1000000UL;
		pinfo->dsi_bit_clk_upt_support = 0;
	} else {
		pinfo->pxl_clk_rate = 151 * 1000000UL;
		pinfo->mipi.dsi_bit_clk = 480;
		pinfo->mipi.dsi_bit_clk_val1 = 471;
		pinfo->mipi.dsi_bit_clk_val2 = 480;
		pinfo->mipi.dsi_bit_clk_val3 = 490;
		pinfo->mipi.dsi_bit_clk_val4 = 500;
		pinfo->dsi_bit_clk_upt_support = 0;
		pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
		pinfo->dsi_bit_clk_upt_support = 0;
	}
}

/* porch initialized value from panel spec */
static void mipi_lcd_init_porch(struct dpu_panel_info *pinfo)
{
	if (g_lcd_fpga_flag == USED_FOR_FPGA) {
		pinfo->ldi.h_back_porch = 23;
		pinfo->ldi.h_front_porch = 50;
		pinfo->ldi.h_pulse_width = 20;
		pinfo->ldi.v_back_porch = 12;
		pinfo->ldi.v_front_porch = 14;
		pinfo->ldi.v_pulse_width = 4;
	} else {
		if (is_mipi_cmd_panel_ext(pinfo)) {
			// 63fps
			pinfo->ldi.h_back_porch = 23;
			pinfo->ldi.h_front_porch = 50;
			pinfo->ldi.h_pulse_width = 20;
			pinfo->ldi.v_back_porch = 28;
			pinfo->ldi.v_front_porch = 14;
			pinfo->ldi.v_pulse_width = 8;
		} else {
			// 60fps
			pinfo->ldi.h_back_porch = 23;
			pinfo->ldi.h_front_porch = 107;
			pinfo->ldi.h_pulse_width = 20;
			pinfo->ldi.v_back_porch = 28;
			pinfo->ldi.v_front_porch = 14;
			pinfo->ldi.v_pulse_width = 8;
		}
	}
}

/* timing initialized value from panel spec */
static void mipi_lcd_init_timing(struct dpu_panel_info *pinfo)
{
	mipi_lcd_init_porch(pinfo);
	mipi_lcd_init_dsi_param(pinfo);

	pinfo->mipi.dsi_version = DSI_1_1_VERSION;

	// non_continue adjust : measured in UI
	// JDI requires clk_post >= 60ns + 252ui,
	// so need a clk_post_adjust more than 200ui. Here 215 is used.
	pinfo->mipi.clk_post_adjust = 215;
	pinfo->mipi.clk_pre_adjust = 0;
	pinfo->mipi.clk_t_hs_prepare_adjust = 0;
	pinfo->mipi.clk_t_lpx_adjust = 0;
	pinfo->mipi.clk_t_hs_trial_adjust = 0;
	pinfo->mipi.clk_t_hs_exit_adjust = 0;
	pinfo->mipi.clk_t_hs_zero_adjust = 0;

	pinfo->mipi.lane_nums = DSI_4_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;

	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
	pinfo->mipi.non_continue_en = 1;
	pinfo->pxl_clk_rate_div = 1;
}

static int mipi_lcd_init_relevant_voltage(struct platform_device *pdev)
{
	int ret;

	if (g_lcd_fpga_flag == 0) {
		// lcd vcc init
		ret = vcc_cmds_tx(pdev, g_lcd_vcc_init_cmds,
			ARRAY_SIZE(g_lcd_vcc_init_cmds));
		if (ret != 0) {
			DPU_FB_ERR("LCD vcc init failed!\n");
			return ret;
		}

		// lcd pinctrl init
		ret = pinctrl_cmds_tx(pdev, g_lcd_pinctrl_init_cmds,
			ARRAY_SIZE(g_lcd_pinctrl_init_cmds));
		if (ret != 0) {
			DPU_FB_ERR("Init pinctrl failed, defer\n");
			return ret;
		}

		// lcd vcc enable
		if (is_fastboot_display_enable())
			vcc_cmds_tx(pdev, g_lcd_vcc_enable_cmds,
				ARRAY_SIZE(g_lcd_vcc_enable_cmds));
	}
	return 0;
}

static void mipi_lcd_get_gpio(struct device_node *np)
{
	if (g_lcd_fpga_flag == USED_FOR_FPGA) {
		g_gpio_lcd_p5v5_enable = of_get_named_gpio(np, "gpios", 0);
		g_gpio_lcd_n5v5_enable = of_get_named_gpio(np, "gpios", 1);
		g_gpio_lcd_reset = of_get_named_gpio(np, "gpios", 2);
		g_gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 3);
		g_gpio_lcd_tp1v8 = of_get_named_gpio(np, "gpios", 4);
		g_gpio_lcd_tp2v8 = 137;
	} else {
		g_gpio_lcd_p5v5_enable = of_get_named_gpio(np, "gpios", 0);
		g_gpio_lcd_n5v5_enable = of_get_named_gpio(np, "gpios", 1);
		g_gpio_lcd_reset = of_get_named_gpio(np, "gpios", 2);
		g_gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 3);
	}

	DPU_FB_INFO("used gpio:[+5v: %d, -5v: %d, rst: %d, bl_en: %d]\n",
		g_gpio_lcd_p5v5_enable, g_gpio_lcd_n5v5_enable,
		g_gpio_lcd_reset, g_gpio_lcd_bl_enable);
}

static int mipi_lcd_get_info_from_dts(struct device_node *np, uint32_t *bl_type,
	uint32_t *lcd_display_type, uint32_t *lcd_ifbc_type)
{
	int ret;

	ret = of_property_read_u32(np, LCD_DISPLAY_TYPE_NAME, lcd_display_type);
	if (ret) {
		DPU_FB_ERR("get lcd_display_type failed!\n");
		*lcd_display_type = PANEL_MIPI_CMD;
	}
	ret = of_property_read_u32(np, LCD_IFBC_TYPE_NAME, lcd_ifbc_type);
	if (ret) {
		DPU_FB_ERR("get ifbc_type failed!\n");
		*lcd_ifbc_type = IFBC_TYPE_NONE;
	}
	ret = of_property_read_u32(np, LCD_BL_TYPE_NAME, bl_type);
	if (ret) {
		DPU_FB_ERR("get lcd_bl_type failed!\n");
		*bl_type = BL_SET_BY_MIPI;
	}

	DPU_FB_INFO("lcd_display_type=0x%x lcd_ifbc_type=0x%x bl_type=0x%x\n",
		*lcd_display_type, *lcd_ifbc_type, *bl_type);
	if (hisi_fb_device_probe_defer(*lcd_display_type, *bl_type))
		return -EPROBE_DEFER;

	ret = of_property_read_u32(np, FPGA_FLAG_NAME, &g_lcd_fpga_flag);
	if (ret)
		DPU_FB_WARNING("not needed in asic!\n");

	mipi_lcd_get_gpio(np);

	DPU_FB_DEBUG("-\n");
	return 0;
}

static void mipi_lcd_init_vsea(struct dpu_panel_info *pinfo)
{
	if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) {
		pinfo->pxl_clk_rate_div = 3;
		pinfo->ldi.h_back_porch /= pinfo->pxl_clk_rate_div;
		pinfo->ldi.h_front_porch /= pinfo->pxl_clk_rate_div;
		pinfo->ldi.h_pulse_width /= pinfo->pxl_clk_rate_div;
		mipi_lcd_init_vsec(pinfo);
	};
}

static int mipi_lcd_probe(struct platform_device *pdev)
{
	int ret;
	struct dpu_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	uint32_t bl_type = 0;
	uint32_t lcd_display_type = 0;
	uint32_t lcd_ifbc_type = 0;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is null\n");
		return 0;
	}

	DPU_FB_DEBUG("+\n");
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LG_TD4322);
	if (np == NULL) {
		DPU_FB_ERR("not found device node %s!\n", DTS_COMP_LG_TD4322);
		return 0;
	}
	ret = mipi_lcd_get_info_from_dts(np, &bl_type, &lcd_display_type,
		&lcd_ifbc_type);
	if (ret) {
		DPU_FB_ERR("hisi_fb_device_probe_defer fail!\n");
		return -EPROBE_DEFER;
	}

	pdev->id = 1;
	pinfo = g_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct dpu_panel_info));

	mipi_lcd_init_panel_info(pinfo, bl_type, lcd_display_type);
	mipi_lcd_init_display_effect(pinfo);
	mipi_lcd_init_dirty_region(pinfo);
	mipi_lcd_init_timing(pinfo);
	mipi_lcd_init_vsea(pinfo);

	ret = mipi_lcd_init_relevant_voltage(pdev);
	if (ret != 0)
		return ret;

	// alloc panel device data
	ret = platform_device_add_data(pdev, &g_panel_data,
		sizeof(struct dpu_fb_panel_data));
	if (ret) {
		DPU_FB_ERR("platform_device_add_data failed!\n");
		platform_device_put(pdev);
		return ret;
	}
	hisi_fb_add_device(pdev);

	DPU_FB_DEBUG("-\n");
	return 0;
}

static const struct of_device_id g_hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_LG_TD4322,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_hisi_panel_match_table);

static struct platform_driver g_this_driver = {
	.probe = mipi_lcd_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "mipi_lg_TD4322",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_hisi_panel_match_table),
	},
};

static int __init mipi_lcd_panel_init(void)
{
	int ret;

	ret = platform_driver_register(&g_this_driver);
	if (ret) {
		DPU_FB_ERR("platform_driver_register failed, error=%d!\n",
			ret);
		return ret;
	}

	return ret;
}
/*lint -restore*/
module_init(mipi_lcd_panel_init);
