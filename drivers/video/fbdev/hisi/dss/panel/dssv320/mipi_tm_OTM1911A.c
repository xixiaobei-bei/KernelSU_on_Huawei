/* Copyright (c) 2008-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e551*/
#include "hisi_fb.h"

#define DTS_COMP_TM_OTM1911A "hisilicon,mipi_tm_OTM1911A"
#define DTS_TM_OTM1911A_LCD_TYPE "tm-otm1911a-lcd-type"

static uint32_t g_lcd_fpga_flag;
extern volatile int g_tskit_ic_type;  //this type means oncell incell tddi ... in order to decide the power policy between lcd & tp
extern int mipi_dsi_ulps_cfg(struct hisi_fb_data_type* hisifd, int enable);

#define LCD_POWER_STATUS_CHECK	(0)

#define OTM1911A_30FPS 0

#if OTM1911A_30FPS
/*lint -e569*/
static char novatek_cmd0[] = { 0xff,0x25 };
static char novatek_cmd1[] = { 0xcb,0x07 };
static char novatek_cmd2[] = { 0xcc,0x8a };
static char novatek_cmd3[] = { 0xfb,0x01 };
static char novatek_cmd4[] = { 0xff,0x24 };
static char novatek_cmd5[] = { 0x80,0x06 };
static char novatek_cmd6[] = { 0xfb,0x01 };
static char novatek_cmd7[] = { 0xff,0x10 };
static char novatek_cmd8[] = { 0x26,0x08 };
static char novatek_cmd9[] = { 0xb1,0x00 };
static char novatek_cmd10[] = { 0xfb,0x01 };

static char novatek_cmd11[] = { 0xff,0x25 };
static char novatek_cmd12[] = { 0xcb,0x00 };
static char novatek_cmd13[] = { 0xcc,0x04 };
static char novatek_cmd14[] = { 0xfb,0x01 };
static char novatek_cmd15[] = { 0xff,0x24 };
static char novatek_cmd16[] = { 0x80,0x02 };
static char novatek_cmd17[] = { 0xfb,0x01 };
static char novatek_cmd18[] = { 0xff,0x10 };
static char novatek_cmd19[] = { 0x26,0x01 };
static char novatek_cmd20[] = { 0xb1,0x03 };//open dfr
static char novatek_cmd21[] = { 0xfb,0x01 };
/*lint +e569*/
#endif

#if OTM1911A_30FPS
static struct dsi_cmd_desc otm1911a_fps_to_30[] = {
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd0), novatek_cmd0},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd1), novatek_cmd1},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd2), novatek_cmd2},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd3), novatek_cmd3},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd4), novatek_cmd4},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd5), novatek_cmd5},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd6), novatek_cmd6},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd7), novatek_cmd7},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd8), novatek_cmd8},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd9), novatek_cmd9},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd10), novatek_cmd10},
};

static struct dsi_cmd_desc otm1911a_fps_to_60[] = {
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd11), novatek_cmd11},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd12), novatek_cmd12},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd13), novatek_cmd13},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd14), novatek_cmd14},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd15), novatek_cmd15},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd16), novatek_cmd16},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd17), novatek_cmd17},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd18), novatek_cmd18},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd19), novatek_cmd19},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd20), novatek_cmd20},
	{DTYPE_DCS_WRITE1, 0, 20, WAIT_TYPE_US,
		sizeof(novatek_cmd21), novatek_cmd21},
};
#endif

/*******************************************************************************
** Power ON Sequence(sleep mode to Normal mode)
*/
/*lint -e569*/
static char tm_poweron_param1[] = {
	0x00,
	0x00,
};

static char tm_poweron_param2[] = {
	0xff,
	0x19, 0x11, 0x01,
};

static char tm_poweron_param3[] = {
	0x00,
	0x80,
};

static char tm_poweron_param4[] = {
	0xff,
	0x19, 0x11,
};

static char tm_poweron_param5[] = {
	0x00,
	0x93,
};

static char tm_poweron_param6[] = {
	0xb3,
	0x06,
};

static char tm_poweron_param7[] = {
	0x00,
	0xb0,
};

static char tm_poweron_param8[] = {
	0xca,
	0x03,
};

static char tm_poweron_param9[] = {
	0x00,
	0xb0,
};

static char tm_poweron_param10[] = {
	0xb3,
	0x04, 0x38, 0x08, 0x70,
};

static char tm_poweron_param11[] = {
	0x00,
	0x80,
};

static char tm_poweron_param12[] = {
	0xc5,
	0x55, 0x05, 0x11, 0xa2, 0xb7, 0x96, 0xaa,
};

static char tm_poweron_param13[] = {
	0x00,
	0x90,
};

static char tm_poweron_param14[] = {
	0xc5,
	0x88, 0xa0, 0x75, 0x65, 0x80,
};

static char tm_poweron_param15[] = {
	0x00,
	0xa0,
};

static char tm_poweron_param16[] = {
	0xc5,
	0x9e, 0xb2, 0x00, 0x88,
};

static char tm_poweron_param17[] = {
	0x00,
	0x00,
};

static char tm_poweron_param18[] = {
	0xff,
	0x00, 0x00, 0x01,
};

static char tm_poweron_param19[] = {
	0x00,
	0x80,
};

static char tm_poweron_param20[] = {
	0xff,
	0x00, 0x00,
};

static char tm_poweron_param21[] = {
	0x51,
	0xff,
};

static char tm_poweron_param22[] = {
	0x53,
	0x2c,
};

static char tm_poweron_param23[] = {
	0x55,
	0x00,
};

/*******************************************************************************
** Power OFF Sequence(Normal to power off)
*/
static char exit_sleep[] = {
	0x11,
};

static char display_on[] = {
	0x29,
};

static char display_off[] = {
	0x28,
};

static char enter_sleep[] = {
	0x10,
};

static char tm_poweroff_param1[] = {
	0x00,
	0x00,
};

static char tm_poweroff_param2[] = {
	0xff,
	0x19, 0x11, 0x01,
};

static char tm_poweroff_param3[] = {
	0x00,
	0x80,
};

static char tm_poweroff_param4[] = {
	0xff,
	0x19, 0x11,
};

static char tm_poweroff_param5[] = {
	0x00,
	0x90,
};

static char tm_poweroff_param6[] = {
	0xB3,
	0x34,
};
/*lint +e569*/

static struct dsi_cmd_desc lcd_display_on_cmds[] = {
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param1), tm_poweron_param1},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param2), tm_poweron_param2},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param3), tm_poweron_param3},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param4), tm_poweron_param4},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param5), tm_poweron_param5},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param6), tm_poweron_param6},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param7), tm_poweron_param7},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param8), tm_poweron_param8},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param9), tm_poweron_param9},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param10), tm_poweron_param10},

	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param11), tm_poweron_param11},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param12), tm_poweron_param12},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param13), tm_poweron_param13},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param14), tm_poweron_param14},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param15), tm_poweron_param15},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param16), tm_poweron_param16},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param17), tm_poweron_param17},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param18), tm_poweron_param18},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param19), tm_poweron_param19},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param20), tm_poweron_param20},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param21), tm_poweron_param21},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param22), tm_poweron_param22},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(tm_poweron_param23), tm_poweron_param23},

	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 0, 20, WAIT_TYPE_MS,
		sizeof(display_on), display_on},
#if OTM1911A_30FPS
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd18), novatek_cmd18},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd20), novatek_cmd20},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd21), novatek_cmd21},
#endif
};

static struct dsi_cmd_desc lcd_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 60, WAIT_TYPE_MS,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(enter_sleep), enter_sleep},

	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(tm_poweroff_param1), tm_poweroff_param1},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(tm_poweroff_param2), tm_poweroff_param2},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(tm_poweroff_param3), tm_poweroff_param3},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(tm_poweroff_param4), tm_poweroff_param4},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(tm_poweroff_param5), tm_poweroff_param5},
	{DTYPE_GEN_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(tm_poweroff_param6), tm_poweroff_param6},
};

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME		"lcdio-vcc"
#define VCC_LCDANALOG_NAME	"lcdanalog-vcc"

static struct regulator *vcc_lcdio;
static struct regulator *vcc_lcdanalog;

static struct vcc_desc lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_GET, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},

	/* vcc set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 3100000, 3100000, WAIT_TYPE_MS, 0},
	/* io set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME, &vcc_lcdio, 1800000, 1800000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_PUT, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_DISABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};


/*******************************************************************************
** LCD IOMUX
*/
static struct pinctrl_data pctrl;

static struct pinctrl_cmd_desc lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &pctrl, 0},
};


/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_VCC2V8_NAME	"gpio_lcd_vcc2v8"
#define GPIO_LCD_VCC1V8_NAME	"gpio_lcd_vcc1v8"
#define GPIO_LCD_BL_ENABLE_NAME		"gpio_lcd_bl_enable"
#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_P5V5_ENABLE_NAME	"gpio_lcd_p5v5_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME	"gpio_lcd_n5v5_enable"

static uint32_t gpio_lcd_vcc2v8;
static uint32_t gpio_lcd_vcc1v8;
static uint32_t gpio_lcd_bl_enable;
static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_p5v5_enable;
static uint32_t gpio_lcd_n5v5_enable;

static struct gpio_desc fpga_lcd_gpio_request_cmds[] = {
	/* vcc3v1 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_VCC2V8_NAME, &gpio_lcd_vcc2v8, 0},
	/* vcc1v8 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
};

static struct gpio_desc fpga_lcd_gpio_free_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* vcc3v1 */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_VCC2V8_NAME, &gpio_lcd_vcc2v8, 0},
	/* vcc1v8 */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
};

static struct gpio_desc fpga_lcd_gpio_normal_cmds[] = {
	/* vcc3v1 enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_VCC2V8_NAME, &gpio_lcd_vcc2v8, 1},
	/* vcc1v8 enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* AVDD_5.5V*/
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 1},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc fpga_lcd_gpio_lowpower_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V*/
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* vcc1v8 disable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* vcc3v1 disable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_VCC2V8_NAME, &gpio_lcd_vcc2v8, 0},
	/* backlight enable input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* reset input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* vcc1v8 disable */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* vcc3v1 disable */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_VCC2V8_NAME, &gpio_lcd_vcc2v8, 0},
};

static struct gpio_desc asic_lcd_gpio_request_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc asic_lcd_gpio_free_cmds[] = {
	/* AVEE_-5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 100,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 100,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 100,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc asic_lcd_gpio_normal_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 1},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
};

static struct gpio_desc asic_lcd_gpio_lowpower_cmds[] = {
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V*/
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

/*******************************************************************************
** ACM
*/
static uint32_t acm_r0_hh = 0x7f;
static uint32_t acm_r0_lh = 0x0;
static uint32_t acm_r1_hh = 0xff;
static uint32_t acm_r1_lh = 0x80;
static uint32_t acm_r2_hh = 0x17f;
static uint32_t acm_r2_lh = 0x100;
static uint32_t acm_r3_hh = 0x1ff;
static uint32_t acm_r3_lh = 0x180;
static uint32_t acm_r4_hh = 0x27f;
static uint32_t acm_r4_lh = 0x200;
static uint32_t acm_r5_hh = 0x2ff;
static uint32_t acm_r5_lh = 0x280;
static uint32_t acm_r6_hh = 0x37f;
static uint32_t acm_r6_lh = 0x300;

static uint32_t acm_hue_param01 = 0x200 << 16 | 0x200;
static uint32_t acm_hue_param23 = 0x1FC << 16 | 0x200;
static uint32_t acm_hue_param45 = 0x204 << 16 | 0x200;
static uint32_t acm_hue_param67 = 0x200 << 16 | 0x200;
static uint32_t acm_hue_smooth0 = 0x0040003F;
static uint32_t acm_hue_smooth1 = 0x00C000BF;
static uint32_t acm_hue_smooth2 = 0x0140013F;
static uint32_t acm_hue_smooth3 = 0x01C001BF;
static uint32_t acm_hue_smooth4 = 0x02410240;
static uint32_t acm_hue_smooth5 = 0x02C102C0;
static uint32_t acm_hue_smooth6 = 0x0340033F;
static uint32_t acm_hue_smooth7 = 0x03C003BF;
static uint32_t acm_color_choose = 1;
static uint32_t acm_l_cont_en = 0;
static uint32_t acm_lc_param01 = 0x020401FC;
static uint32_t acm_lc_param23 = 0x02000200;
static uint32_t acm_lc_param45 = 0x020801F8;
static uint32_t acm_lc_param67 = 0x020401FC;
static uint32_t acm_l_adj_ctrl = 0;
static uint32_t acm_capture_ctrl = 0;
static uint32_t acm_capture_in = 0;
static uint32_t acm_capture_out = 0;
static uint32_t acm_ink_ctrl = 0;
static uint32_t acm_ink_out = 0;


static u32 acm_lut_hue_table[] = {
	  0,    4,    8,   12,   16,   20,   24,   28,   32,   36,   40,
	 44,   48,   52,   56,   60,   64,   68,   72,   76,   80,   84,
	 88,   92,   96,  100,  104,  108,  112,  116,  120,  124,  128,
	132,  136,  140,  144,  148,  152,  156,  160,  164,  168,  172,
	176,  180,  184,  188,  192,  196,  200,  204,  208,  212,  216,
	220,  224,  228,  232,  236,  240,  244,  248,  252,  256,  260,
	264,  268,  272,  276,  280,  284,  288,  292,  296,  300,  304,
	308,  312,  316,  320,  324,  328,  332,  336,  340,  344,  348,
	352,  356,  360,  364,  368,  372,  376,  380,  384,  388,  392,
	396,  400,  404,  408,  412,  416,  420,  424,  428,  432,  436,
	440,  444,  448,  452,  456,  460,  464,  468,  472,  476,  480,
	484,  488,  492,  496,  500,  504,  508,  512,  516,  520,  524,
	528,  532,  536,  540,  544,  548,  552,  556,  560,  564,  568,
	572,  576,  580,  584,  588,  592,  596,  600,  604,  608,  612,
	616,  620,  624,  628,  632,  636,  640,  644,  648,  652,  656,
	660,  664,  668,  672,  676,  680,  684,  688,  692,  696,  700,
	704,  708,  712,  716,  720,  724,  728,  732,  736,  740,  744,
	748,  752,  756,  760,  764,  768,  772,  776,  780,  784,  788,
	792,  796,  800,  804,  808,  812,  816,  820,  824,  828,  832,
	836,  840,  844,  848,  852,  856,  860,  864,  868,  872,  876,
	880,  884,  888,  892,  896,  900,  904,  908,  912,  916,  920,
	924,  928,  932,  936,  940,  944,  948,  952,  956,  960,  964,
	968,  972,  976,  980,  984,  988,  992,  996, 1000, 1004, 1008,
	1012, 1016, 1020
};
static u32 acm_lut_sata_table[] = {
	 5,  4,  4,  4,  3,  2,  2,  2,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  1,  2,
	 2,  2,  3,  4,  4,  6,  7,  8, 10, 12, 13, 14, 16, 18, 21, 23, 26, 28, 30, 33,
	35, 37, 38, 40, 42, 43, 45, 46, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 57,
	58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
	59, 59, 59, 59, 58, 58, 58, 58, 58, 57, 56, 55, 54, 52, 51, 50, 49, 48, 48, 47,
	46, 45, 44, 44, 43, 43, 42, 42, 42, 41, 41, 40, 40, 40, 40, 41, 41, 41, 42, 42,
	42, 42, 42, 41, 41, 41, 40, 40, 40, 41, 42, 42, 43, 44, 44, 45, 46, 47, 48, 49,
	50, 50, 51, 52, 53, 54, 54, 55, 56, 56, 57, 57, 58, 57, 56, 56, 55, 54, 54, 53,
	52, 52, 51, 50, 50, 50, 49, 48, 48, 47, 46, 45, 44, 44, 43, 42, 41, 40, 40, 39,
	38, 37, 36, 36, 35, 35, 35, 35, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 32, 32,
	32, 32, 31, 31, 30, 30, 30, 29, 29, 29, 28, 28, 28, 28, 28, 27, 27, 26, 26, 25,
	24, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 18, 18, 18, 17, 17, 16, 16, 16, 15,
	15, 15, 14, 14, 14, 13, 13, 12, 12, 11, 10,  9,  8,  8,  7,  6
};
static u32 acm_lut_satr0_table[] = {
	 0,  2,  3,  5,  6,  8, 10, 11, 13, 14,
	16, 16, 17, 17, 18, 18, 18, 19, 19, 20,
	20, 21, 21, 22, 22, 23, 23, 23, 24, 24,
	25, 25, 25, 25, 24, 24, 24, 24, 23, 23,
	23, 23, 22, 22, 22, 22, 21, 21, 21, 21,
	20, 20, 20, 20, 18, 16, 14, 12, 10,  8,
	 6,  4,  2,  0
};
static u32 acm_lut_satr1_table[] = {
	 0,  0,  0,  4,  7, 11, 15, 19, 25, 30,
	35, 39, 41, 44, 46, 48, 50, 53, 55, 57,
	59, 62, 64, 66, 68, 71, 73, 75, 77, 80,
	82, 84, 83, 81, 80, 78, 77, 75, 74, 72,
	71, 69, 68, 67, 65, 64, 62, 61, 59, 58,
	56, 49, 42, 35, 30, 25, 20, 15, 11,  7,
	 4,  0,  0,  0
};
static u32 acm_lut_satr2_table[] = {
	 0,  0,  0,  0,  0,  3,  9, 14, 18, 23,
	27, 32, 36, 41, 44, 48, 51, 56, 59, 62,
	65, 66, 69, 71, 72, 74, 75, 77, 77, 77,
	77, 77, 75, 75, 74, 72, 71, 68, 66, 63,
	60, 57, 54, 50, 47, 42, 38, 35, 30, 26,
	21, 15, 11,  6,  2,  0,  0,  0,  0,  0,
	 0,  0,  0,  0
};
static u32 acm_lut_satr3_table[] = {
	 0,  0,  0,  0,  0,  0,  0,  3,  7,  9,
	13, 16, 19, 22, 25, 28, 30, 34, 36, 39,
	41, 43, 45, 46, 48, 49, 50, 51, 53, 54,
	54, 54, 54, 54, 53, 53, 51, 50, 49, 47,
	46, 44, 42, 40, 38, 35, 33, 29, 26, 24,
	21, 18, 15, 11, 9, 7, 5, 3, 0, 0,
	 0, 0, 0, 0
};
static u32 acm_lut_satr4_table[] = {
	 0,  0,  0,  0,  0,  0,  0,  3,  7,  9,
	13, 16, 19, 22, 25, 28, 30, 34, 36, 39,
	41, 43, 45, 46, 48, 49, 50, 51, 53, 54,
	54, 54, 54, 54, 53, 53, 51, 50, 49, 47,
	46, 44, 42, 40, 38, 35, 33, 29, 26, 24,
	21, 18, 15, 11,  9,  7,  5,  3,  0,  0,
	 0,  0,  0,  0
};
static u32 acm_lut_satr5_table[] = {
	 0,  0,  0,  0,  0,  0,  0,  2,  6,  9,
	13, 16, 19, 22, 25, 28, 30, 34, 36, 39,
	41, 43, 45, 46, 48, 49, 50, 51, 53, 54,
	54, 54, 54, 54, 53, 53, 51, 50, 49, 47,
	46, 44, 42, 40, 38, 35, 33, 29, 26, 24,
	21, 18, 15, 11,  7,  4,  1,  0,  0,  0,
	 0,  0,  0,  0
};
static u32 acm_lut_satr6_table[] = {
	 0,  0,  0,  5, 10, 16, 22, 28, 33, 39,
	46, 50, 51, 52, 53, 54, 54, 55, 56, 56,
	57, 58, 59, 59, 60, 61, 61, 62, 63, 64,
	64, 65, 64, 63, 63, 62, 61, 60, 59, 58,
	58, 57, 56, 55, 54, 54, 53, 52, 51, 50,
	49, 48, 47, 43, 38, 33, 28, 21, 15, 10,
	 5,  0,  0,  0
};
static u32 acm_lut_satr7_table[] = {
	 0,  4,  7, 11, 15, 18, 22, 26, 30, 33,
	37, 38, 38, 39, 40, 40, 41, 42, 42, 43,
	44, 44, 45, 46, 46, 47, 48, 48, 49, 50,
	50, 51, 50, 50, 49, 48, 48, 47, 47, 46,
	45, 45, 44, 43, 43, 42, 41, 41, 40, 40,
	39, 38, 38, 37, 33, 30, 26, 22, 18, 15,
	11,  7,  4,  0
};


/*******************************************************************************
** GAMMA
*/
static u32 gamma_lut_table_R[] = {
	  0,	 16,	 32,	 48,	 64,	 80,	 96,	112,	128,	144,
	160,	176,	192,	208,	224,	240,	256,	272,	288,	304,
	320,	336,	352,	368,	384,	400,	416,	432,	448,	464,
	480,	496,	512,	528,	544,	560,	576,	592,	608,	624,
	640,	656,	672,	688,	704,	720,	736,	752,	768,	784,
	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,
	960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,
	1120,	1136,	1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,
	1280,	1296,	1312,	1328,	1344,	1360,	1376,	1392,	1408,	1424,
	1440,	1456,	1472,	1488,	1504,	1520,	1536,	1552,	1568,	1584,
	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	1728,	1744,
	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,
	1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,
	2080,	2096,	2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,
	2240,	2256,	2272,	2288,	2304,	2320,	2336,	2352,	2368,	2384,
	2400,	2416,	2432,	2448,	2464,	2480,	2496,	2512,	2528,	2544,
	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	2688,	2704,
	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,
	2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,
	3040,	3056,	3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,
	3200,	3216,	3232,	3248,	3264,	3280,	3296,	3312,	3328,	3344,
	3360,	3376,	3392,	3408,	3424,	3440,	3456,	3472,	3488,	3504,
	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	3648,	3664,
	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,
	3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,
	4000,	4016,	4032,	4048,	4064,	4080,	4095
};
static u32 gamma_lut_table_G[] = {
	  0,	 16,	 32,	 48,	 64,	 80,	 96,	112,	128,	144,
	160,	176,	192,	208,	224,	240,	256,	272,	288,	304,
	320,	336,	352,	368,	384,	400,	416,	432,	448,	464,
	480,	496,	512,	528,	544,	560,	576,	592,	608,	624,
	640,	656,	672,	688,	704,	720,	736,	752,	768,	784,
	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,
	960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,
	1120,	1136,	1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,
	1280,	1296,	1312,	1328,	1344,	1360,	1376,	1392,	1408,	1424,
	1440,	1456,	1472,	1488,	1504,	1520,	1536,	1552,	1568,	1584,
	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	1728,	1744,
	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,
	1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,
	2080,	2096,	2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,
	2240,	2256,	2272,	2288,	2304,	2320,	2336,	2352,	2368,	2384,
	2400,	2416,	2432,	2448,	2464,	2480,	2496,	2512,	2528,	2544,
	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	2688,	2704,
	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,
	2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,
	3040,	3056,	3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,
	3200,	3216,	3232,	3248,	3264,	3280,	3296,	3312,	3328,	3344,
	3360,	3376,	3392,	3408,	3424,	3440,	3456,	3472,	3488,	3504,
	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	3648,	3664,
	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,
	3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,
	4000,	4016,	4032,	4048,	4064,	4080,	4095
};
static u32 gamma_lut_table_B[] = {
	  0,	 16,	 32,	 48,	 64,	 80,	 96,	112,	128,	144,
	160,	176,	192,	208,	224,	240,	256,	272,	288,	304,
	320,	336,	352,	368,	384,	400,	416,	432,	448,	464,
	480,	496,	512,	528,	544,	560,	576,	592,	608,	624,
	640,	656,	672,	688,	704,	720,	736,	752,	768,	784,
	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,
	960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,
	1120,	1136,	1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,
	1280,	1296,	1312,	1328,	1344,	1360,	1376,	1392,	1408,	1424,
	1440,	1456,	1472,	1488,	1504,	1520,	1536,	1552,	1568,	1584,
	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	1728,	1744,
	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,
	1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,
	2080,	2096,	2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,
	2240,	2256,	2272,	2288,	2304,	2320,	2336,	2352,	2368,	2384,
	2400,	2416,	2432,	2448,	2464,	2480,	2496,	2512,	2528,	2544,
	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	2688,	2704,
	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,
	2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,
	3040,	3056,	3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,
	3200,	3216,	3232,	3248,	3264,	3280,	3296,	3312,	3328,	3344,
	3360,	3376,	3392,	3408,	3424,	3440,	3456,	3472,	3488,	3504,
	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	3648,	3664,
	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,
	3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,
	4000,	4016,	4032,	4048,	4064,	4080,	4095
};

/*******************************************************************************
** IGM
*/
static u32 igm_lut_table_R[] = {
	  0,	 16,	 32,	 48,	 64,	 80,	 96,	112,	128,	144,
	160,	176,	192,	208,	224,	240,	256,	272,	288,	304,
	320,	336,	352,	368,	384,	400,	416,	432,	448,	464,
	480,	496,	512,	528,	544,	560,	576,	592,	608,	624,
	640,	656,	672,	688,	704,	720,	736,	752,	768,	784,
	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,
	960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,
	1120,	1136,	1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,
	1280,	1296,	1312,	1328,	1344,	1360,	1376,	1392,	1408,	1424,
	1440,	1456,	1472,	1488,	1504,	1520,	1536,	1552,	1568,	1584,
	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	1728,	1744,
	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,
	1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,
	2080,	2096,	2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,
	2240,	2256,	2272,	2288,	2304,	2320,	2336,	2352,	2368,	2384,
	2400,	2416,	2432,	2448,	2464,	2480,	2496,	2512,	2528,	2544,
	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	2688,	2704,
	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,
	2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,
	3040,	3056,	3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,
	3200,	3216,	3232,	3248,	3264,	3280,	3296,	3312,	3328,	3344,
	3360,	3376,	3392,	3408,	3424,	3440,	3456,	3472,	3488,	3504,
	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	3648,	3664,
	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,
	3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,
	4000,	4016,	4032,	4048,	4064,	4080,	4095
};
static u32 igm_lut_table_G[] = {
	  0,	 16,	 32,	 48,	 64,	 80,	 96,	112,	128,	144,
	160,	176,	192,	208,	224,	240,	256,	272,	288,	304,
	320,	336,	352,	368,	384,	400,	416,	432,	448,	464,
	480,	496,	512,	528,	544,	560,	576,	592,	608,	624,
	640,	656,	672,	688,	704,	720,	736,	752,	768,	784,
	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,
	960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,
	1120,	1136,	1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,
	1280,	1296,	1312,	1328,	1344,	1360,	1376,	1392,	1408,	1424,
	1440,	1456,	1472,	1488,	1504,	1520,	1536,	1552,	1568,	1584,
	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	1728,	1744,
	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,
	1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,
	2080,	2096,	2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,
	2240,	2256,	2272,	2288,	2304,	2320,	2336,	2352,	2368,	2384,
	2400,	2416,	2432,	2448,	2464,	2480,	2496,	2512,	2528,	2544,
	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	2688,	2704,
	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,
	2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,
	3040,	3056,	3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,
	3200,	3216,	3232,	3248,	3264,	3280,	3296,	3312,	3328,	3344,
	3360,	3376,	3392,	3408,	3424,	3440,	3456,	3472,	3488,	3504,
	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	3648,	3664,
	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,
	3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,
	4000,	4016,	4032,	4048,	4064,	4080,	4095
};
static u32 igm_lut_table_B[] = {
	  0,	 16,	 32,	 48,	 64,	 80,	 96,	112,	128,	144,
	160,	176,	192,	208,	224,	240,	256,	272,	288,	304,
	320,	336,	352,	368,	384,	400,	416,	432,	448,	464,
	480,	496,	512,	528,	544,	560,	576,	592,	608,	624,
	640,	656,	672,	688,	704,	720,	736,	752,	768,	784,
	800,	816,	832,	848,	864,	880,	896,	912,	928,	944,
	960,	976,	992,	1008,	1024,	1040,	1056,	1072,	1088,	1104,
	1120,	1136,	1152,	1168,	1184,	1200,	1216,	1232,	1248,	1264,
	1280,	1296,	1312,	1328,	1344,	1360,	1376,	1392,	1408,	1424,
	1440,	1456,	1472,	1488,	1504,	1520,	1536,	1552,	1568,	1584,
	1600,	1616,	1632,	1648,	1664,	1680,	1696,	1712,	1728,	1744,
	1760,	1776,	1792,	1808,	1824,	1840,	1856,	1872,	1888,	1904,
	1920,	1936,	1952,	1968,	1984,	2000,	2016,	2032,	2048,	2064,
	2080,	2096,	2112,	2128,	2144,	2160,	2176,	2192,	2208,	2224,
	2240,	2256,	2272,	2288,	2304,	2320,	2336,	2352,	2368,	2384,
	2400,	2416,	2432,	2448,	2464,	2480,	2496,	2512,	2528,	2544,
	2560,	2576,	2592,	2608,	2624,	2640,	2656,	2672,	2688,	2704,
	2720,	2736,	2752,	2768,	2784,	2800,	2816,	2832,	2848,	2864,
	2880,	2896,	2912,	2928,	2944,	2960,	2976,	2992,	3008,	3024,
	3040,	3056,	3072,	3088,	3104,	3120,	3136,	3152,	3168,	3184,
	3200,	3216,	3232,	3248,	3264,	3280,	3296,	3312,	3328,	3344,
	3360,	3376,	3392,	3408,	3424,	3440,	3456,	3472,	3488,	3504,
	3520,	3536,	3552,	3568,	3584,	3600,	3616,	3632,	3648,	3664,
	3680,	3696,	3712,	3728,	3744,	3760,	3776,	3792,	3808,	3824,
	3840,	3856,	3872,	3888,	3904,	3920,	3936,	3952,	3968,	3984,
	4000,	4016,	4032,	4048,	4064,	4080,	4095
};

/*******************************************************************************
** GMP
*/
static u32 gmp_lut_table_low32bit[9][9][9] = {
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0x00000000, 0x00000200, 0x00000400, 0x00000600, 0x00000800, 0x00000a00, 0x00000c00, 0x00000e00, 0x00000ff0, },
        {0x00200000, 0x00200200, 0x00200400, 0x00200600, 0x00200800, 0x00200a00, 0x00200c00, 0x00200e00, 0x00200ff0, },
        {0x00400000, 0x00400200, 0x00400400, 0x00400600, 0x00400800, 0x00400a00, 0x00400c00, 0x00400e00, 0x00400ff0, },
        {0x00600000, 0x00600200, 0x00600400, 0x00600600, 0x00600800, 0x00600a00, 0x00600c00, 0x00600e00, 0x00600ff0, },
        {0x00800000, 0x00800200, 0x00800400, 0x00800600, 0x00800800, 0x00800a00, 0x00800c00, 0x00800e00, 0x00800ff0, },
        {0x00a00000, 0x00a00200, 0x00a00400, 0x00a00600, 0x00a00800, 0x00a00a00, 0x00a00c00, 0x00a00e00, 0x00a00ff0, },
        {0x00c00000, 0x00c00200, 0x00c00400, 0x00c00600, 0x00c00800, 0x00c00a00, 0x00c00c00, 0x00c00e00, 0x00c00ff0, },
        {0x00e00000, 0x00e00200, 0x00e00400, 0x00e00600, 0x00e00800, 0x00e00a00, 0x00e00c00, 0x00e00e00, 0x00e00ff0, },
        {0x00ff0000, 0x00ff0200, 0x00ff0400, 0x00ff0600, 0x00ff0800, 0x00ff0a00, 0x00ff0c00, 0x00ff0e00, 0x00ff0ff0, },
    },
    {
        {0xf0000000, 0xf0000200, 0xf0000400, 0xf0000600, 0xf0000800, 0xf0000a00, 0xf0000c00, 0xf0000e00, 0xf0000ff0, },
        {0xf0200000, 0xf0200200, 0xf0200400, 0xf0200600, 0xf0200800, 0xf0200a00, 0xf0200c00, 0xf0200e00, 0xf0200ff0, },
        {0xf0400000, 0xf0400200, 0xf0400400, 0xf0400600, 0xf0400800, 0xf0400a00, 0xf0400c00, 0xf0400e00, 0xf0400ff0, },
        {0xf0600000, 0xf0600200, 0xf0600400, 0xf0600600, 0xf0600800, 0xf0600a00, 0xf0600c00, 0xf0600e00, 0xf0600ff0, },
        {0xf0800000, 0xf0800200, 0xf0800400, 0xf0800600, 0xf0800800, 0xf0800a00, 0xf0800c00, 0xf0800e00, 0xf0800ff0, },
        {0xf0a00000, 0xf0a00200, 0xf0a00400, 0xf0a00600, 0xf0a00800, 0xf0a00a00, 0xf0a00c00, 0xf0a00e00, 0xf0a00ff0, },
        {0xf0c00000, 0xf0c00200, 0xf0c00400, 0xf0c00600, 0xf0c00800, 0xf0c00a00, 0xf0c00c00, 0xf0c00e00, 0xf0c00ff0, },
        {0xf0e00000, 0xf0e00200, 0xf0e00400, 0xf0e00600, 0xf0e00800, 0xf0e00a00, 0xf0e00c00, 0xf0e00e00, 0xf0e00ff0, },
        {0xf0ff0000, 0xf0ff0200, 0xf0ff0400, 0xf0ff0600, 0xf0ff0800, 0xf0ff0a00, 0xf0ff0c00, 0xf0ff0e00, 0xf0ff0ff0, },
    },
};
static u32 gmp_lut_table_high4bit[9][9][9] = {
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

/*******************************************************************************
** XCC
*/
static u32 xcc_table[12] = {0x0, 0x8000, 0x0,0x0,0x0,0x0,0x8000,0x0,0x0,0x0,0x0,0x8000,};


/*******************************************************************************
**
*/
static int mipi_tm_panel_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	char payload[2] = {0, 0};
	struct dsi_cmd_desc bl_cmd[] = {
		{DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
			sizeof(payload), payload},
	};

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {
		ret = hisi_pwm_set_backlight(hisifd, bl_level);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {
		ret = hisi_blpwm_set_backlight(hisifd, bl_level);
	}else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {
		ret = hisi_sh_blpwm_set_backlight(hisifd, bl_level);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
		bl_cmd[0].payload[0] = 0x51;
		bl_cmd[0].payload[1] = bl_level * 255 / hisifd->panel_info.bl_max;

		mipi_dsi_cmds_tx(bl_cmd, ARRAY_SIZE(bl_cmd), hisifd->mipi_dsi0_base);
	} else {
		HISI_FB_ERR("fb%d, not support this bl_set_type(%d)!\n",
			hisifd->index, hisifd->panel_info.bl_set_type);
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int mipi_tm_panel_set_fastboot(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_lcd_fpga_flag == 0) {
		// lcd pinctrl normal
		pinctrl_cmds_tx(pdev, lcd_pinctrl_normal_cmds,
			ARRAY_SIZE(lcd_pinctrl_normal_cmds));

		// lcd gpio request
		gpio_cmds_tx(asic_lcd_gpio_request_cmds,
			ARRAY_SIZE(asic_lcd_gpio_request_cmds));
	} else {
		// lcd gpio request
		gpio_cmds_tx(fpga_lcd_gpio_request_cmds,
			ARRAY_SIZE(fpga_lcd_gpio_request_cmds));
	}

	// backlight on
	hisi_lcd_backlight_on(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_tm_panel_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	uint32_t status = 0;
	uint32_t try_times = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +!\n", hisifd->index);

	pinfo = &(hisifd->panel_info);
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	if (pinfo->lcd_init_step == LCD_INIT_POWER_ON) {
		if (g_lcd_fpga_flag == 0) {
			// lcd vcc enable
			vcc_cmds_tx(pdev, lcd_vcc_enable_cmds,
				ARRAY_SIZE(lcd_vcc_enable_cmds));
		}

		udelay(100);//wait pll clk
		//bit13 lock sel enable (dual_mipi_panel bit29 set 0) ,open clock gate
		set_reg(hisifd->pctrl_base + PERI_CTRL33, 0x0, 1, 13);
		if (is_dual_mipi_panel(hisifd)) {
			set_reg(hisifd->pctrl_base + PERI_CTRL30, 0x0, 1, 29);
		}

		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		if (g_lcd_fpga_flag == 0) {
			// lcd pinctrl normal
			pinctrl_cmds_tx(pdev, lcd_pinctrl_normal_cmds,
				ARRAY_SIZE(lcd_pinctrl_normal_cmds));

			// lcd gpio request
			gpio_cmds_tx(asic_lcd_gpio_request_cmds, \
				ARRAY_SIZE(asic_lcd_gpio_request_cmds));

			// lcd gpio normal
			gpio_cmds_tx(asic_lcd_gpio_normal_cmds, \
				ARRAY_SIZE(asic_lcd_gpio_normal_cmds));
		} else {
			// lcd gpio request
			gpio_cmds_tx(fpga_lcd_gpio_request_cmds, \
				ARRAY_SIZE(fpga_lcd_gpio_request_cmds));

			// lcd gpio normal
			gpio_cmds_tx(fpga_lcd_gpio_normal_cmds, \
				ARRAY_SIZE(fpga_lcd_gpio_normal_cmds));
		}

		// lcd display on sequence
		mipi_dsi_cmds_tx(lcd_display_on_cmds, \
			ARRAY_SIZE(lcd_display_on_cmds), mipi_dsi0_base);

		// check lcd power state
		outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, 0x0A06);
		status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		while (status & 0x10) {
			udelay(50);
			if (++try_times > 100) {
				try_times = 0;
				HISI_FB_ERR("Read lcd power status timeout!\n");
				break;
			}

			status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		}
		status = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		HISI_FB_INFO("LCD Power State = 0x%x.\n", status);

		pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE) {
		;
	} else {
		HISI_FB_ERR("failed to init lcd!\n");
	}

	/* backlight on */
	hisi_lcd_backlight_on(pdev);

	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return 0;
}

static int mipi_tm_panel_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_INFO("fb%d, +!\n", hisifd->index);

	// backlight off
	hisi_lcd_backlight_off(pdev);

	// lcd display off sequence
	mipi_dsi_cmds_tx(lcd_display_off_cmds, \
		ARRAY_SIZE(lcd_display_off_cmds), hisifd->mipi_dsi0_base);

	/* switch to cmd mode */
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7f, 7, 8);
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0xf, 4, 16);
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);

	/* disable generate High Speed clock */
	set_reg(hisifd->mipi_dsi0_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
	mipi_dsi_ulps_cfg(hisifd, 0);
	udelay(10);

	if (g_lcd_fpga_flag == 0) {
		// lcd gpio lowpower
		gpio_cmds_tx(asic_lcd_gpio_lowpower_cmds, \
			ARRAY_SIZE(asic_lcd_gpio_lowpower_cmds));
		// lcd gpio free
		gpio_cmds_tx(asic_lcd_gpio_free_cmds, \
			ARRAY_SIZE(asic_lcd_gpio_free_cmds));

		// lcd pinctrl lowpower
		pinctrl_cmds_tx(pdev, lcd_pinctrl_lowpower_cmds,
			ARRAY_SIZE(lcd_pinctrl_lowpower_cmds));

		mdelay(3);
		// lcd vcc disable
		vcc_cmds_tx(pdev, lcd_vcc_disable_cmds,
			ARRAY_SIZE(lcd_vcc_disable_cmds));
	} else {
		// lcd gpio lowpower
		gpio_cmds_tx(fpga_lcd_gpio_lowpower_cmds, \
			ARRAY_SIZE(fpga_lcd_gpio_lowpower_cmds));
		// lcd gpio free
		gpio_cmds_tx(fpga_lcd_gpio_free_cmds, \
			ARRAY_SIZE(fpga_lcd_gpio_free_cmds));
	}

	HISI_FB_INFO("fb%d, -!\n", hisifd->index);

	return 0;
}

static int mipi_tm_panel_remove(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);

	if (!hisifd) {
		return 0;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_lcd_fpga_flag == 0) {
		// lcd vcc finit
		vcc_cmds_tx(pdev, lcd_vcc_finit_cmds,
			ARRAY_SIZE(lcd_vcc_finit_cmds));

		// lcd pinctrl finit
		pinctrl_cmds_tx(pdev, lcd_pinctrl_finit_cmds,
			ARRAY_SIZE(lcd_pinctrl_finit_cmds));
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}



/******************************************************************************/
static ssize_t mipi_tm_panel_lcd_model_show(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = snprintf(buf, PAGE_SIZE, "tm_OTM1911A 5P93 VIDEO TFT 1080 x 2160\n");

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tm_panel_lcd_cabc_mode_show(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tm_panel_lcd_cabc_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tm_panel_lcd_check_reg_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	uint32_t read_value[4] = {0};
	uint32_t expected_value[4] = {0x9c, 0x00, 0x07, 0x00};
	uint32_t read_mask[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	char* reg_name[4] = {"power mode", "MADCTR", "pixel format", "image mode"};
	char lcd_reg_0a[] = {0x0a};
	char lcd_reg_0b[] = {0x0b};
	char lcd_reg_0c[] = {0x0c};
	char lcd_reg_0d[] = {0x0d};

	struct dsi_cmd_desc lcd_check_reg[] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0a), lcd_reg_0a},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0b), lcd_reg_0b},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0c), lcd_reg_0c},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0d), lcd_reg_0d},
	};

	struct mipi_dsi_read_compare_data data = {
		.read_value = read_value,
		.expected_value = expected_value,
		.read_mask = read_mask,
		.reg_name = reg_name,
		.log_on = 1,
		.cmds = lcd_check_reg,
		.cnt = ARRAY_SIZE(lcd_check_reg),
	};

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);
	if (!mipi_dsi_read_compare(&data, mipi_dsi0_base)) {
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	} else {
		ret = snprintf(buf, PAGE_SIZE, "ERROR\n");
	}
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static char lcd_disp_x[] = {
	0x2A,
	0x00, 0x00,0x04,0x37
};

static char lcd_disp_y[] = {
	0x2B,
	0x00, 0x00,0x08,0x6F
};

static struct dsi_cmd_desc set_display_address[] = {
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(lcd_disp_x), lcd_disp_x},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(lcd_disp_y), lcd_disp_y},
};

/*lint -e574*/
static int mipi_tm_panel_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	BUG_ON(pdev == NULL || dirty == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	pinfo = &(hisifd->panel_info);

	if (((dirty->x % 2) != 0) || ((dirty->y % 2) != 0) ||
		((dirty->w % 2) != 0) || ((dirty->h % 2) != 0) ||
		(dirty->x >= pinfo->xres) || (dirty->w > pinfo->xres) || ((dirty->x + dirty->w) > pinfo->xres) ||
		(dirty->y >= pinfo->yres) || (dirty->h > pinfo->yres) || ((dirty->y + dirty->h) > pinfo->yres)) {
		HISI_FB_ERR("dirty_region(%d,%d, %d,%d) not support!\n",
			dirty->x, dirty->y, dirty->w, dirty->h);

		BUG_ON(1);
	}

	if (hisifd->panel_info.ifbc_type != IFBC_TYPE_NONE)
		dirty->y /= 2;

	lcd_disp_x[1] = (dirty->x >> 8) & 0xff;
	lcd_disp_x[2] = dirty->x & 0xff;
	lcd_disp_x[3] = ((dirty->x + dirty->w - 1) >> 8) & 0xff;
	lcd_disp_x[4] = (dirty->x + dirty->w - 1) & 0xff;
	lcd_disp_y[1] = (dirty->y >> 8) & 0xff;
	lcd_disp_y[2] = dirty->y & 0xff;
	lcd_disp_y[3] = ((dirty->y + dirty->h - 1) >> 8) & 0xff;
	lcd_disp_y[4] = (dirty->y + dirty->h - 1) & 0xff;

	mipi_dsi_cmds_tx(set_display_address, \
		ARRAY_SIZE(set_display_address), hisifd->mipi_dsi0_base);

	return 0;
}
/*lint +e574*/

#if OTM1911A_30FPS
/*lint -e569*/
static char novatek_cmd2_open[] = { 0xff,0x10 };
static char novatek_cmd_dfr_enable[] = { 0xb1,0x03 };//open dfr
static char novatek_cmd_dfr_disable[] = { 0xb1,0x00 };//close dfr
static char novatek_cmd2_close[] = { 0xfb,0x01 };
/*lint +e569*/

static struct dsi_cmd_desc otm1911a_disable_dfr[] = {
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd2_open), novatek_cmd2_open},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd_dfr_enable), novatek_cmd_dfr_disable},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd2_close), novatek_cmd2_close},
};

static struct dsi_cmd_desc otm1911a_enable_dfr[] = {
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd2_open), novatek_cmd2_open},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd_dfr_enable), novatek_cmd_dfr_enable},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(novatek_cmd2_close), novatek_cmd2_close},
};

static int mipi_jdi_panel_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;

	HISI_FB_DEBUG("+.");
	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd){
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	pinfo = &(hisifd->panel_info);
	if (NULL == pinfo){
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	switch(scence){
		case LCD_FPS_SCENCE_IDLE:
			pinfo->fps_updt = LCD_FPS_30;
			HISI_FB_DEBUG("scence is LCD_FPS_SCENCE_IDLE, framerate is 30fps\n");
			break;
		case LCD_FPS_SCENCE_NORMAL:
			pinfo->fps_updt = LCD_FPS_60;
			HISI_FB_DEBUG("scence is LCD_FPS_SCENCE_NORMAL, framerate is 60fps\n");
			break;
		case LCD_FPS_SCENCE_VIDEO:
			HISI_FB_INFO("disable drf\n");
			hisifb_set_vsync_activate_state(hisifd, true);
			hisifb_activate_vsync(hisifd);
			mipi_dsi_cmds_tx(otm1911a_disable_dfr,
					ARRAY_SIZE(otm1911a_disable_dfr), mipi_dsi0_base);
			hisifb_set_vsync_activate_state(hisifd, false);
			hisifb_deactivate_vsync(hisifd);
			break;
		case LCD_FPS_SCENCE_GAME:
			HISI_FB_INFO("enable drf\n");
			hisifb_set_vsync_activate_state(hisifd, true);
			hisifb_activate_vsync(hisifd);
			mipi_dsi_cmds_tx(otm1911a_enable_dfr,
					ARRAY_SIZE(otm1911a_enable_dfr), mipi_dsi0_base);
			hisifb_set_vsync_activate_state(hisifd, false);
			hisifb_deactivate_vsync(hisifd);
			break;
		case LCD_FPS_SCENCE_FORCE_30FPS:
			pinfo->fps_updt = LCD_FPS_30;
			pinfo->fps_updt_panel_only = 1;
			HISI_FB_INFO("scence is LCD_FPS_SCENCE_IDLE_FORCE_ENABLE, framerate is 30fps\n");
			break;
		default:
			pinfo->fps_updt = LCD_FPS_60;
			HISI_FB_DEBUG("scence is LCD_FPS_SCENCE_NORMAL, framerate is 60fps\n");
	}
	HISI_FB_DEBUG("-.\n");
	return 0;
}

static int mipi_jdi_panel_lcd_fps_updt_handle(struct platform_device *pdev){
	struct hisi_fb_data_type *hisifd = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	struct hisi_panel_info *pinfo = NULL;

	HISI_FB_DEBUG("+.\n");
	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd){
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	pinfo = &(hisifd->panel_info);
	if (NULL == pinfo){
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	switch (pinfo->fps_updt){
		case LCD_FPS_30:
			HISI_FB_INFO("lcd switch %d fps\n", LCD_FPS_30);
			hisifb_set_vsync_activate_state(hisifd, true);
			hisifb_activate_vsync(hisifd);
			mipi_dsi_cmds_tx(otm1911a_fps_to_30,
					ARRAY_SIZE(otm1911a_fps_to_30), mipi_dsi0_base);
			hisifb_set_vsync_activate_state(hisifd, false);
			hisifb_deactivate_vsync(hisifd);
			break;
		case LCD_FPS_60:
			HISI_FB_INFO("lcd switch %d fps\n", LCD_FPS_60);
			hisifb_set_vsync_activate_state(hisifd, true);
			hisifb_activate_vsync(hisifd);
			mipi_dsi_cmds_tx(otm1911a_fps_to_60,
					ARRAY_SIZE(otm1911a_fps_to_60), mipi_dsi0_base);
			hisifb_set_vsync_activate_state(hisifd, false);
			hisifb_deactivate_vsync(hisifd);
			break;
		default:
			HISI_FB_ERR("lcd unknown attr\n");
			break;
	}

	HISI_FB_DEBUG("-.\n");
	return 0;
}
#endif


static struct hisi_panel_info g_panel_info = {0};
static struct hisi_fb_panel_data g_panel_data = {
	.panel_info = &g_panel_info,
	.set_fastboot = mipi_tm_panel_set_fastboot,
	.on = mipi_tm_panel_on,
	.off = mipi_tm_panel_off,
	.remove = mipi_tm_panel_remove,
	.set_backlight = mipi_tm_panel_set_backlight,
	.lcd_model_show = mipi_tm_panel_lcd_model_show,
	.lcd_cabc_mode_show = mipi_tm_panel_lcd_cabc_mode_show,
	.lcd_cabc_mode_store = mipi_tm_panel_lcd_cabc_mode_store,
	.lcd_check_reg = mipi_tm_panel_lcd_check_reg_show,
	.set_display_region = mipi_tm_panel_set_display_region,
#if OTM1911A_30FPS
	.lcd_fps_scence_handle = mipi_jdi_panel_lcd_fps_scence_handle,
	.lcd_fps_updt_handle = mipi_jdi_panel_lcd_fps_updt_handle,
#endif
};


/*******************************************************************************
**
*/
static int mipi_tm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	uint32_t bl_type = 0;
	uint32_t lcd_display_type = 0;
	uint32_t lcd_ifbc_type = 0;
	const char *lcd_bl_ic_name;
	char lcd_bl_ic_name_buf[LCD_BL_IC_NAME_MAX];
	int value = 0;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_TM_OTM1911A);
	if (!np) {
		HISI_FB_ERR("NOT FOUND device node %s!\n", DTS_COMP_TM_OTM1911A);
		goto err_return;
	}

	ret = of_property_read_u32(np, LCD_DISPLAY_TYPE_NAME, &lcd_display_type);
	if (ret) {
		HISI_FB_ERR("get lcd_display_type failed!\n");
		lcd_display_type = PANEL_MIPI_CMD;
	}

	ret = of_property_read_u32(np, LCD_IFBC_TYPE_NAME, &lcd_ifbc_type);
	if (ret) {
		HISI_FB_ERR("get ifbc_type failed!\n");
		lcd_ifbc_type = IFBC_TYPE_NONE;
	}

	ret = of_property_read_u32(np, LCD_BL_TYPE_NAME, &bl_type);
	if (ret) {
		HISI_FB_ERR("get lcd_bl_type failed!\n");
		bl_type = BL_SET_BY_MIPI;
	}
	HISI_FB_INFO("bl_type=0x%x.", bl_type);

	ret = of_property_read_u32(np, DTS_TM_OTM1911A_LCD_TYPE, &value);
	if (ret) {
		HISI_FB_ERR("get g_tskit_ic_type failed!\n");
	} else {
		g_tskit_ic_type = value;
	}
	HISI_FB_INFO("g_tskit_ic_type = 0x%x.", g_tskit_ic_type);

	if (hisi_fb_device_probe_defer(lcd_display_type, bl_type)) {
		goto err_probe_defer;
	}

	HISI_FB_DEBUG("+.\n");

	HISI_FB_INFO("%s\n", DTS_COMP_TM_OTM1911A);

	ret = of_property_read_u32(np, FPGA_FLAG_NAME, &g_lcd_fpga_flag);
	if (ret) {
		HISI_FB_WARNING("need to get g_lcd_fpga_flag resource in fpga, not needed in asic!\n");
	}
	ret = of_property_read_string_index(np, "lcd-bl-ic-name", 0, &lcd_bl_ic_name);
	if (ret != 0) {
		memcpy(lcd_bl_ic_name_buf, "INVALID", strlen("INVALID") + 1) ;
	} else {
		memcpy(lcd_bl_ic_name_buf, lcd_bl_ic_name, strlen(lcd_bl_ic_name) + 1);
	}
	HISI_FB_INFO("lcd_bl_ic_name=%s!\n", lcd_bl_ic_name_buf);

	if (g_lcd_fpga_flag == 1) {
		//gpio_16_0
		gpio_lcd_p5v5_enable = of_get_named_gpio(np, "gpios", 1);
		//gpio_20_7
		gpio_lcd_n5v5_enable = of_get_named_gpio(np, "gpios", 2);
		//gpio_17_3
		gpio_lcd_reset = of_get_named_gpio(np, "gpios", 3);
		//gpio_20_6
		gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 4);
		//gpio_17_1
		gpio_lcd_vcc2v8 = of_get_named_gpio(np, "gpios", 5);
		//gpio_16_1
		gpio_lcd_vcc1v8 = of_get_named_gpio(np, "gpios", 6);
		HISI_FB_INFO("vcc1v8(%d), p5v5(%d), n5v5(%d), reset(%d), bl(%d), vcc2v8(%d)\n", gpio_lcd_vcc1v8, gpio_lcd_p5v5_enable, gpio_lcd_n5v5_enable, gpio_lcd_reset, gpio_lcd_bl_enable, gpio_lcd_vcc2v8);
	} else {
		// gpio_23_0
		gpio_lcd_p5v5_enable = of_get_named_gpio(np, "gpios", 0);
		//gpio_22_7
		gpio_lcd_n5v5_enable = of_get_named_gpio(np, "gpios", 1);
		//gpio_1_3
		gpio_lcd_reset = of_get_named_gpio(np, "gpios", 2);
		HISI_FB_INFO("p5v5(%d), n5v5(%d), reset(%d)\n", gpio_lcd_p5v5_enable, gpio_lcd_n5v5_enable, gpio_lcd_reset);
	}

	pdev->id = 1;
	// init lcd panel info
	pinfo = g_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->xres = 1080;
	pinfo->yres = 2160;
	pinfo->width = 67;
	pinfo->height = 135;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = bl_type;

#if OTM1911A_30FPS
	/* open fps switch func */
	pinfo->fps = 60;
	pinfo->fps_updt = 60;
	pinfo->fps_updt_support = 1;
	pinfo->fps_updt_panel_only = 0;
#endif

	if (pinfo->bl_set_type == BL_SET_BY_BLPWM)
		pinfo->blpwm_input_ena = 0;

	if (!strncmp(lcd_bl_ic_name_buf, "LM36923YFFR", strlen("LM36923YFFR"))) {
		pinfo->bl_min = 13;
		pinfo->bl_max = 1636;
		pinfo->bl_default = 819;
		pinfo->blpwm_precision_type = BLPWM_PRECISION_2048_TYPE;
		pinfo->bl_ic_ctrl_mode = REG_ONLY_MODE;
	} else {
	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;
	}

	pinfo->type = lcd_display_type;
	pinfo->ifbc_type = lcd_ifbc_type;

	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;

	pinfo->arsr1p_sharpness_support = 0;
	pinfo->prefix_ce_support = 0;
	pinfo->prefix_sharpness1D_support = 0;
	pinfo->prefix_sharpness2D_support = 0;

	//sbl
	pinfo->sbl_support = 0;
	pinfo->smart_bl.strength_limit = 0;
	pinfo->smart_bl.calibration_a = 60;
	pinfo->smart_bl.calibration_b = 95;
	pinfo->smart_bl.calibration_c = 5;
	pinfo->smart_bl.calibration_d = 1;
	pinfo->smart_bl.t_filter_control = 5;
	pinfo->smart_bl.backlight_min = 480;
	pinfo->smart_bl.backlight_max = 4096;
	pinfo->smart_bl.backlight_scale = 0xff;
	pinfo->smart_bl.ambient_light_min = 14;
	pinfo->smart_bl.filter_a = 1738;
	pinfo->smart_bl.filter_b = 6;
	pinfo->smart_bl.logo_left = 0;
	pinfo->smart_bl.logo_top = 0;

	//ACM
	pinfo->acm_support = 0;
	if (pinfo->acm_support == 1) {
		pinfo->acm_lut_hue_table = acm_lut_hue_table;
		pinfo->acm_lut_hue_table_len = sizeof(acm_lut_hue_table) / sizeof(acm_lut_hue_table[0]);
		pinfo->acm_lut_sata_table = acm_lut_sata_table;
		pinfo->acm_lut_sata_table_len = sizeof(acm_lut_sata_table) / sizeof(acm_lut_sata_table[0]);
		pinfo->acm_lut_satr0_table = acm_lut_satr0_table;
		pinfo->acm_lut_satr0_table_len = sizeof(acm_lut_satr0_table) / sizeof(acm_lut_satr0_table[0]);
		pinfo->acm_lut_satr1_table = acm_lut_satr1_table;
		pinfo->acm_lut_satr1_table_len = sizeof(acm_lut_satr1_table) / sizeof(acm_lut_satr1_table[0]);
		pinfo->acm_lut_satr2_table = acm_lut_satr2_table;
		pinfo->acm_lut_satr2_table_len = sizeof(acm_lut_satr2_table) / sizeof(acm_lut_satr2_table[0]);
		pinfo->acm_lut_satr3_table = acm_lut_satr3_table;
		pinfo->acm_lut_satr3_table_len = sizeof(acm_lut_satr3_table) / sizeof(acm_lut_satr3_table[0]);
		pinfo->acm_lut_satr4_table = acm_lut_satr4_table;
		pinfo->acm_lut_satr4_table_len = sizeof(acm_lut_satr4_table) / sizeof(acm_lut_satr4_table[0]);
		pinfo->acm_lut_satr5_table = acm_lut_satr5_table;
		pinfo->acm_lut_satr5_table_len = sizeof(acm_lut_satr5_table) / sizeof(acm_lut_satr5_table[0]);
		pinfo->acm_lut_satr6_table = acm_lut_satr6_table;
		pinfo->acm_lut_satr6_table_len = sizeof(acm_lut_satr6_table) / sizeof(acm_lut_satr6_table[0]);
		pinfo->acm_lut_satr7_table = acm_lut_satr7_table;
		pinfo->acm_lut_satr7_table_len = sizeof(acm_lut_satr7_table) / sizeof(acm_lut_satr7_table[0]);
		pinfo->r0_hh = acm_r0_hh;
		pinfo->r0_lh = acm_r0_lh;
		pinfo->r1_hh = acm_r1_hh;
		pinfo->r1_lh = acm_r1_lh;
		pinfo->r2_hh = acm_r2_hh;
		pinfo->r2_lh = acm_r2_lh;
		pinfo->r3_hh = acm_r3_hh;
		pinfo->r3_lh = acm_r3_lh;
		pinfo->r4_hh = acm_r4_hh;
		pinfo->r4_lh = acm_r4_lh;
		pinfo->r5_hh = acm_r5_hh;
		pinfo->r5_lh = acm_r5_lh;
		pinfo->r6_hh = acm_r6_hh;
		pinfo->r6_lh = acm_r6_lh;

		pinfo->hue_param01 = acm_hue_param01;
		pinfo->hue_param23 = acm_hue_param23;
		pinfo->hue_param45 = acm_hue_param45;
		pinfo->hue_param67 = acm_hue_param67;
		pinfo->hue_smooth0 = acm_hue_smooth0;
		pinfo->hue_smooth1 = acm_hue_smooth1;
		pinfo->hue_smooth2 = acm_hue_smooth2;
		pinfo->hue_smooth3 = acm_hue_smooth3;
		pinfo->hue_smooth4 = acm_hue_smooth4;
		pinfo->hue_smooth5 = acm_hue_smooth5;
		pinfo->hue_smooth6 = acm_hue_smooth6;
		pinfo->hue_smooth7 = acm_hue_smooth7;
		pinfo->color_choose = acm_color_choose;
		pinfo->l_cont_en = acm_l_cont_en;
		pinfo->lc_param01 = acm_lc_param01;
		pinfo->lc_param23 = acm_lc_param23;
		pinfo->lc_param45 = acm_lc_param45;
		pinfo->lc_param67 = acm_lc_param67;
		pinfo->l_adj_ctrl = acm_l_adj_ctrl;
		pinfo->capture_ctrl = acm_capture_ctrl;
		pinfo->capture_in = acm_capture_in;
		pinfo->capture_out = acm_capture_out;
		pinfo->ink_ctrl = acm_ink_ctrl;
		pinfo->ink_out = acm_ink_out;

		//ACM_CE
		pinfo->acm_ce_support = 1;
	}


	//Gama LCP
	pinfo->gamma_support = 0;
	if (pinfo->gamma_support == 1) {

		pinfo->igm_lut_table_R = igm_lut_table_R;
		pinfo->igm_lut_table_G = igm_lut_table_G;
		pinfo->igm_lut_table_B = igm_lut_table_B;
		pinfo->igm_lut_table_len = ARRAY_SIZE(igm_lut_table_R);

		pinfo->gamma_lut_table_R = gamma_lut_table_R;
		pinfo->gamma_lut_table_G = gamma_lut_table_G;
		pinfo->gamma_lut_table_B = gamma_lut_table_B;
		pinfo->gamma_lut_table_len = ARRAY_SIZE(gamma_lut_table_R);

		pinfo->xcc_support = 0;
		pinfo->xcc_table = xcc_table;
		pinfo->xcc_table_len = ARRAY_SIZE(xcc_table);

		pinfo->gmp_support = 0;
		pinfo->gmp_lut_table_low32bit = &gmp_lut_table_low32bit[0][0][0];
		pinfo->gmp_lut_table_high4bit = &gmp_lut_table_high4bit[0][0][0];
		pinfo->gmp_lut_table_len = ARRAY_SIZE(gmp_lut_table_low32bit);
	}


		// hiace
	pinfo->hiace_support = 0;
	if (pinfo->hiace_support == 1) {
		pinfo->hiace_param.iGlobalHistBlackPos = 16;
		pinfo->hiace_param.iGlobalHistWhitePos = 240;
		pinfo->hiace_param.iGlobalHistBlackWeight = 51;
		pinfo->hiace_param.iGlobalHistWhiteWeight = 51;
		pinfo->hiace_param.iGlobalHistZeroCutRatio = 486;
		pinfo->hiace_param.iGlobalHistSlopeCutRatio = 410;
		pinfo->hiace_param.iMaxLcdLuminance = 500;
		pinfo->hiace_param.iMinLcdLuminance = 3;
		strncpy(pinfo->hiace_param.chCfgName, "/product/etc/display/effect/algorithm/hdr_engine_MHA.xml", sizeof(pinfo->hiace_param.chCfgName) - 1);
	}

	if (g_lcd_fpga_flag == 1) {
		//ldi
		pinfo->ldi.h_back_porch = 22;
		pinfo->ldi.h_front_porch = 5;
		pinfo->ldi.h_pulse_width = 3;
		pinfo->ldi.v_back_porch = 25;
		pinfo->ldi.v_front_porch = 4;
		pinfo->ldi.v_pulse_width = 10;

		//mipi
		pinfo->mipi.dsi_bit_clk = 120;
		pinfo->mipi.dsi_bit_clk_val1 = 110;
		pinfo->mipi.dsi_bit_clk_val2 = 120;
		pinfo->mipi.dsi_bit_clk_val3 = 130;
		pinfo->mipi.dsi_bit_clk_val4 = 140;
		pinfo->mipi.dsi_bit_clk_val5 = 150;
		pinfo->dsi_bit_clk_upt_support = 0;
		pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;

		pinfo->mipi.non_continue_en = 0;
		pinfo->mipi.lp11_flag = 1;

		pinfo->pxl_clk_rate = 20 * 1000000;
	} else {
		//ldi
		//60fps
		pinfo->ldi.h_back_porch = 45;
		pinfo->ldi.h_front_porch = 99;
		pinfo->ldi.h_pulse_width = 8;
		pinfo->ldi.v_back_porch = 26;
		pinfo->ldi.v_front_porch = 16;
		pinfo->ldi.v_pulse_width = 10;

		//mipi
		pinfo->mipi.dsi_bit_clk = 480;
		pinfo->mipi.dsi_bit_clk_val1 = 471;
		pinfo->mipi.dsi_bit_clk_val2 = 480;
		pinfo->mipi.dsi_bit_clk_val3 = 490;
		pinfo->mipi.dsi_bit_clk_val4 = 500;
		//pinfo->mipi.dsi_bit_clk_val5 = ;
		pinfo->dsi_bit_clk_upt_support = 0;
		pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;

		pinfo->non_check_ldi_porch = 1;
		pinfo->mipi.non_continue_en = 0; //TBD
		pinfo->mipi.lp11_flag = 1;

		pinfo->pxl_clk_rate = 160 * 1000000UL;
	}

	// mipi
	pinfo->mipi.clk_post_adjust = 100;
	pinfo->mipi.lane_nums = DSI_4_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;
	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;

	pinfo->pxl_clk_rate_div = 1;
	pinfo->vsync_ctrl_type = 0;
	pinfo->dirty_region_updt_support = 0;

	if (g_lcd_fpga_flag == 0) {
		// lcd vcc init
		ret = vcc_cmds_tx(pdev, lcd_vcc_init_cmds,
			ARRAY_SIZE(lcd_vcc_init_cmds));
		if (ret != 0) {
			HISI_FB_ERR("LCD vcc init failed!\n");
			goto err_return;
		}

		// lcd pinctrl init
		ret = pinctrl_cmds_tx(pdev, lcd_pinctrl_init_cmds,
			ARRAY_SIZE(lcd_pinctrl_init_cmds));
		if (ret != 0) {
			HISI_FB_ERR("Init pinctrl failed, defer\n");
			goto err_return;
		}

		// lcd vcc enable
		if (is_fastboot_display_enable()) {
			vcc_cmds_tx(pdev, lcd_vcc_enable_cmds,
				ARRAY_SIZE(lcd_vcc_enable_cmds));
		}
	}

	// alloc panel device data
	ret = platform_device_add_data(pdev, &g_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		HISI_FB_ERR("platform_device_add_data failed!\n");
		goto err_device_put;
	}

	hisi_fb_add_device(pdev);

	HISI_FB_DEBUG("-.\n");

	return 0;

err_device_put:
	platform_device_put(pdev);
err_return:
	return ret;
err_probe_defer:
	return -EPROBE_DEFER;
}

static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_TM_OTM1911A,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);

static struct platform_driver this_driver = {
	.probe = mipi_tm_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "mipi_tm_OTM1911A",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init mipi_tm_panel_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(mipi_tm_panel_init);
/*lint +e551*/
