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

#include "hisi_fb.h"
#include <linux/hisi/hw_cmdline_parse.h>
#include <huawei_platform/touthscreen/huawei_touchscreen.h>
#include <huawei_platform/log/log_jank.h>
#include "mipi_samsung_S6E3HF4.h"

#define DTS_COMP_SAMSUNG_S6E3HF4 "hisilicon,mipi_samsung_S6E3HF4"

//#define AMOLED_CHECK_INT

static struct hisi_fb_panel_data samsung_s6e3hf4_panel_data;
static bool g_debug_enable = false;
extern int fastboot_set_needed;
static int g_hbm_mode = 0;
static bool lcd_rs_poweroff = true;

static bool g_display_on = false;
static struct hisi_fb_panel_data g_panel_data;
static int g_acl_ctrl = 0;
static int g_support_mode = 0;

enum {
	ACL_DEBUG = 0,
	ACL_SETTING = 1,
};
enum {
	ACL_OFF = 0,
	ACL_OFFSET1_ON = 1,
	ACL_OFFSET2_ON = 2,
	ACL_OFFSET3_ON = 3,
};

//errflag setting
static char errflag_setting[] = {
	0xED,
	0x44,
};

//common setting
static char tear_on[] = {
	0x35,
	0x00,
};

static char dimming_ctrl[] = {
	0x53,
	0x20,
};

static char bl_disable[] = {
	0x51,
	0x00,
};

static char exit_sleep[] = {
	0x11,
};

static char display_on[] = {
	0x29,
};

static char acl_mode[] = {
	0x55, 0x00,
};

/*DSC setting*/
/*pps settings*/
static char test_key_enable_0[] = {
	0xF0,
	0x5A, 0x5A,
};

static char test_key_enable_1[] = {
	0xFC,
	0x5A, 0x5A,
};

static char pps_setting[] = {
	0x9E,
	0x11,0x00,0x00,0x89,0x30,0x80,0x0A, 0x00,0x05,
	0xA0,0x00,0x20,0x02,0xD0,0x02,0xD0, 0x02,0x00,
	0x02,0x68,0x00,0x20,0x03,0x87,0x00, 0x0A,0x00,
	0x0C,0x03,0x19,0x02,0x63,0x18,0x00, 0x10,0xF0,
	0x03,0x0C,0x20,0x00,0x06,0x0B,0x0B, 0x33,0x0E,
	0x1C,0x2A,0x38,0x46,0x54,0x62,0x69, 0x70,0x77,
	0x79,0x7B,0x7D,0x7E,0x01,0x02,0x01, 0x00,0x09,
	0x40,0x09,0xBE,0x19,0xFC,0x19,0xFA, 0x19,0xF8,
	0x1A,0x38,0x1A,0x78,0x1A,0xB6,0x2A, 0xF6,0x2B,
	0x34,0x2B,0x74,0x3B,0x74,0x6B,0xF4, 0x00,
};

static char test_key_disable_1[] = {
	0xFC,
	0xA5, 0xA5,
};

static char test_key_disable_0[] = {
	0xF0,
	0xA5, 0xA5,
};

/*dsc decoder on*/
static char compression_mode_enable[] = {
	0x9D,
	0x01,
};

/*******************************************************************************
** Power OFF Sequence(Normal to power off)
*/
static char display_off[] = {
	0x28,
};

static char enter_sleep[] = {
	0x10,
};

static struct dsi_cmd_desc display_on_cmds[] = {
/*dsc setting*/
	{DTYPE_GEN_LWRITE, 0, 200, WAIT_TYPE_US,
		sizeof(test_key_enable_0), test_key_enable_0},
	{DTYPE_GEN_LWRITE, 0, 200, WAIT_TYPE_US,
		sizeof(test_key_enable_1), test_key_enable_1},
	{DTYPE_GEN_LWRITE, 0, 200, WAIT_TYPE_US,
		sizeof(pps_setting), pps_setting},
	{DTYPE_GEN_LWRITE, 0, 200, WAIT_TYPE_US,
		sizeof(compression_mode_enable), compression_mode_enable},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(errflag_setting), errflag_setting},
	{DTYPE_GEN_LWRITE, 0, 200, WAIT_TYPE_US,
		sizeof(test_key_disable_1), test_key_disable_1},
	{DTYPE_GEN_LWRITE, 0, 200, WAIT_TYPE_US,
		sizeof(test_key_disable_0), test_key_disable_0},
	{DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(tear_on), tear_on},
	{DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(dimming_ctrl), dimming_ctrl},
	{DTYPE_DCS_WRITE1, 0, 115, WAIT_TYPE_MS,
		sizeof(bl_disable), bl_disable},
	{DTYPE_GEN_LWRITE, 0, 120, WAIT_TYPE_US,
		sizeof(acl_mode), acl_mode},	
};

static struct dsi_cmd_desc display_on_cmd[] = {
	{DTYPE_DCS_WRITE, 0, 500, WAIT_TYPE_US,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 20, WAIT_TYPE_MS,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 0, 130, WAIT_TYPE_MS,
		sizeof(enter_sleep), enter_sleep}
};

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME		"lcdio-vcc"
#define VCC_LCDANALOG_NAME	"lcdanalog-vcc"

static struct regulator *vcc_lcdio;
static struct regulator *vcc_lcdanalog;

static struct vcc_desc lcd_vcc_init_cmds[] = {
	/* vci get */
	{DTYPE_VCC_GET, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
	/* vddio get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},

	/* vci set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 3300000, 3300000, WAIT_TYPE_MS, 0},
	/* vci set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME, &vcc_lcdio, 1850000, 1850000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_finit_cmds[] = {
	/* vci put */
	{DTYPE_VCC_PUT, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
	/* vddio put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_enable_cmds[] = {
	/* vci enable */
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
	/* vddio enable */
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 12},
};

static struct vcc_desc lcd_vcc_disable_cmds[] = {
	/* vddio disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	/* vci disable */
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

#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_ID0_NAME	"gpio_lcd_id0"
#define GPIO_LCD_ERR_FLAG_NAME "gpio_lcd_errflag"

static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_id0;
static uint32_t gpio_lcd_err_flag;

/***panel on**/

static struct gpio_desc lcd_gpio_request_cmds[] = {
	/*err flag*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_ERR_FLAG_NAME, &gpio_lcd_err_flag, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* id0 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 1,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
};

static struct gpio_desc lcd_gpio_free_cmds[] = {
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* id */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
	/*err flag*/
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_ERR_FLAG_NAME, &gpio_lcd_err_flag, 0},
};

static struct gpio_desc lcd_gpio_normal_cmds[] = {
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	/* id */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
	/*err flag*/
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_ERR_FLAG_NAME, &gpio_lcd_err_flag, 0},
};

static struct gpio_desc lcd_gpio_lowpower_cmds[] = {
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc lcd_gpio_sleep_request_cmds[] = {
	/*err flag*/
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_ERR_FLAG_NAME, &gpio_lcd_err_flag, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* id */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_ID0_NAME, &gpio_lcd_id0, 0},
};

static struct work_struct errflag_detect_work;



/*******************************************************************************
**
*/
static int mipi_samsung_s6e3hf4_set_fastboot(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);
 
	// lcd pinctrl normal
	pinctrl_cmds_tx(pdev, lcd_pinctrl_normal_cmds,
		ARRAY_SIZE(lcd_pinctrl_normal_cmds));

	// lcd gpio request
	gpio_cmds_tx(lcd_gpio_request_cmds,
		ARRAY_SIZE(lcd_gpio_request_cmds));

	// backlight on
	hisi_lcd_backlight_on(pdev);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_samsung_s6e3hf4_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	struct lcd_reg_read_t lcd_status_reg[] = {
		{0x0A, 0x98, 0xFF, "lcd power state"},
	};
	if(pdev == NULL) {
		HISI_FB_ERR("pdev is NULL!\n");
		return -1;
	}

	hisifd = platform_get_drvdata(pdev);
	if(hisifd == NULL){
		HISI_FB_ERR("hisifd is NULL!\n");
		return -1;
	}
	HISI_FB_INFO("fb%d, +!\n", hisifd->index);

	pinfo = &(hisifd->panel_info);
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	if (pinfo->lcd_init_step == LCD_INIT_POWER_ON) {
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "JL_KERNEL_LCD_POWER_ON");
		if (lcd_rs_poweroff) {
			HISI_FB_INFO("Init power on(regulator enabling).\n");
			// lcd vcc enable
			vcc_cmds_tx(pdev, lcd_vcc_enable_cmds,
				ARRAY_SIZE(lcd_vcc_enable_cmds));
		} else {
			HISI_FB_INFO("Init power on(regulator has enabled).\n");
		}

		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		// lcd pinctrl normal
		pinctrl_cmds_tx(pdev, lcd_pinctrl_normal_cmds,
			ARRAY_SIZE(lcd_pinctrl_normal_cmds));

		// lcd gpio normal
		gpio_cmds_tx(lcd_gpio_normal_cmds, \
			ARRAY_SIZE(lcd_gpio_normal_cmds));

		mipi_dsi_cmds_tx(display_on_cmds, \
			ARRAY_SIZE(display_on_cmds), mipi_dsi0_base);

		g_debug_enable = true;
		
		panel_check_status_and_report_by_dsm(lcd_status_reg, \
			ARRAY_SIZE(lcd_status_reg), mipi_dsi0_base);
		pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE) {
	} else {
		HISI_FB_ERR("failed to init lcd!\n");
	}

	/* backlight on */
	hisi_lcd_backlight_on(pdev);

	HISI_FB_INFO("fb%d, -!\n", hisifd->index);

	return 0;
}

static int mipi_samsung_s6e3hf4_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (pdev == NULL) {
		HISI_FB_ERR("pdev is NULL!\n");
		return -1;
	}
	hisifd = platform_get_drvdata(pdev);
	if(hisifd == NULL){
		HISI_FB_ERR("hisifd is NULL!\n");
		return -1;
	}
	struct lcd_reg_read_t lcd_mipi_detect[] = {
		{0x05, 0x00, 0xFF, "lcd mipi detect"},
	};
	HISI_FB_INFO("fb%d, +!\n", hisifd->index);

	pinfo = &(hisifd->panel_info);

	if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE) {
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "JL_KERNEL_LCD_POWER_OFF");
		HISI_FB_INFO("display off(download display off sequence).\n");
		panel_status_report_by_dsm(lcd_mipi_detect, \
			ARRAY_SIZE(lcd_mipi_detect), hisifd->mipi_dsi0_base, 10);
		/* backlight off */
		hisi_lcd_backlight_off(pdev);

		// lcd display off sequence
		mipi_dsi_cmds_tx(display_off_cmds, \
			ARRAY_SIZE(display_off_cmds), hisifd->mipi_dsi0_base);
		g_display_on = false;
		pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_LP_SEND_SEQUENCE) {
		pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;
	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_POWER_OFF) {
		if (!hisifd->fb_shutdown) {
			// lcd gpio lowpower
			gpio_cmds_tx(lcd_gpio_lowpower_cmds, \
				ARRAY_SIZE(lcd_gpio_lowpower_cmds));

			if (lcd_rs_poweroff) {
				// lcd pinctrl lowpower
				pinctrl_cmds_tx(pdev, lcd_pinctrl_lowpower_cmds,
					ARRAY_SIZE(lcd_pinctrl_lowpower_cmds));

				HISI_FB_INFO("display off(regulator disabling).\n");
				vcc_cmds_tx(pdev, lcd_vcc_disable_cmds, \
					ARRAY_SIZE(lcd_vcc_disable_cmds));
			} else {
				HISI_FB_INFO("display off(regulator not need disabling).\n");
			}
		} else {
			HISI_FB_INFO("display shutting down(regulator disabling).\n");
			// lcd gpio lowpower
			gpio_cmds_tx(lcd_gpio_lowpower_cmds, \
				ARRAY_SIZE(lcd_gpio_lowpower_cmds));

			// lcd gpio free
			gpio_cmds_tx(lcd_gpio_free_cmds, \
				ARRAY_SIZE(lcd_gpio_free_cmds));

			// lcd pinctrl lowpower
			pinctrl_cmds_tx(pdev, lcd_pinctrl_lowpower_cmds,
				ARRAY_SIZE(lcd_pinctrl_lowpower_cmds));

			vcc_cmds_tx(pdev, lcd_vcc_disable_cmds, \
				ARRAY_SIZE(lcd_vcc_disable_cmds));
			ts_thread_stop_notify();
		}
	} else {
		HISI_FB_ERR("failed to uninit lcd!\n");
	}
	HISI_FB_INFO("fb%d, -!\n", hisifd->index);

	return 0;
}

static int mipi_samsung_s6e3hf4_remove(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (!pdev){
		HISI_FB_INFO("pdev is NULL.\n");
		return 0;
	}
	hisifd = platform_get_drvdata(pdev);

	if (!hisifd) {
		return 0;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	// lcd vcc finit
	vcc_cmds_tx(pdev, lcd_vcc_finit_cmds,
		ARRAY_SIZE(lcd_vcc_finit_cmds));

	// lcd pinctrl finit
	pinctrl_cmds_tx(pdev, lcd_pinctrl_finit_cmds,
		ARRAY_SIZE(lcd_pinctrl_finit_cmds));

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_samsung_s6e3hf4_set_brightness(struct platform_device *pdev, uint32_t bl_level)
{
	char __iomem *mipi_dsi0_base = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;
	static char last_bl_level=0;
	char bl_level_adjust[2] = {
		0x51,
		0x00,
	};

	struct dsi_cmd_desc lcd_bl_level_adjust[] = {
		{DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
			sizeof(bl_level_adjust), bl_level_adjust},
	};

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (unlikely(g_debug_enable)) {
		HISI_FB_INFO("set cmd display_cmd(0x29)!!!\n");
		mipi_dsi_cmds_tx(display_on_cmd, \
			ARRAY_SIZE(display_on_cmd), hisifd->mipi_dsi0_base);
		g_display_on = true;
	}

	if (bl_level == 0) {
		HISI_FB_INFO("Set brightness to 0!\n");
	}

	if (unlikely(g_debug_enable)) {
		HISI_FB_INFO("Set brightness to %d\n", bl_level);
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_ON, "JL_KERNEL_LCD_BACKLIGHT_ON,%u", bl_level);
		g_debug_enable = false;
	}

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {
		ret = hisi_pwm_set_backlight(hisifd, bl_level);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {
		ret = hisi_blpwm_set_backlight(hisifd, bl_level);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) {
		mipi_dsi0_base = hisifd->mipi_dsi0_base;

		bl_level_adjust[1] = bl_level * 255 / hisifd->panel_info.bl_max;
		if (last_bl_level != bl_level_adjust[1]){
			last_bl_level = bl_level_adjust[1];
			mipi_dsi_cmds_tx(lcd_bl_level_adjust, \
				ARRAY_SIZE(lcd_bl_level_adjust), mipi_dsi0_base);
		}
	} else {
		HISI_FB_ERR("No such bl_set_type!\n");
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

/******************************************************************************/
static ssize_t mipi_samsung_s6e3hf4_model_show(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "SAMSUNG_S6E3HF4 5.1' CMD AMOLED 2K\n");
	return ret;
}

static ssize_t mipi_samsung_s6e3hf4_sleep_ctrl_show(struct platform_device *pdev, char *buf)
{
	ssize_t ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);
	ret = snprintf(buf, PAGE_SIZE, "enable_lcd_sleep_in=%d\n", g_debug_enable_lcd_sleep_in);
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_samsung_s6e3hf4_sleep_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	ssize_t ret = 0;
	unsigned long val = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	ret = strict_strtoul(buf, 0, &val);
	if (ret) {
		HISI_FB_ERR("strict_strtoul error, buf=%s", buf);
		return ret;
	}

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	if (hisifd->panel_info.lcd_adjust_support) {
		g_debug_enable_lcd_sleep_in = val;
	}

	if (g_debug_enable_lcd_sleep_in == 2) {
		HISI_FB_INFO("LCD power off and Touch goto sleep\n");
		lcd_rs_poweroff = true;
		g_tp_power_ctrl = 0;	//used for pt  current test, tp sleep
	} else {
		HISI_FB_INFO("g_debug_enable_lcd_sleep_in is %d\n", g_debug_enable_lcd_sleep_in);
		lcd_rs_poweroff = false;
		g_tp_power_ctrl = 0;	//used for pt  current test, tp power off
	}

	return ret;
}


static ssize_t mipi_samsung_S6E3HF4_panel_acl_ctrl_show(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = snprintf(buf, PAGE_SIZE, "%d\n", g_acl_ctrl);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_samsung_S6E3HF4_panel_acl_ctrl_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	unsigned int value[10] = {0};
	char *token, *cur;
	int i = 0;
	char payload_acl_switch[2] = {0};
	struct dsi_cmd_desc acl_switch_cmd[] = {
		{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
			sizeof(payload_acl_switch), payload_acl_switch},
	};

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}
	mipi_dsi0_base = hisifd->mipi_dsi0_base;
	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	cur = buf;
	while (token = strsep(&cur, ",")) {
		value[i++] = simple_strtol(token, NULL, 0);
		if(i >= 10) {
			HISI_FB_ERR("out of range of value[10] buf!\n");
			return count;
		}
	}

	if (ACL_DEBUG == value[0]) { /* debug */

	} else if (ACL_SETTING == value[0]) {
		if (ACL_OFF == value[1]) {
			acl_switch_cmd[0].payload[0] = 0x55;
			acl_switch_cmd[0].payload[1] = ACL_OFF;
			mipi_dsi_cmds_tx(acl_switch_cmd, ARRAY_SIZE(acl_switch_cmd), mipi_dsi0_base);
			g_acl_ctrl = ACL_OFF;
			HISI_FB_INFO("ACL OFF\n");
		} else if (ACL_OFFSET1_ON == value[1]) {
			acl_switch_cmd[0].payload[0] = 0x55;
			acl_switch_cmd[0].payload[1] = ACL_OFFSET1_ON;
			mipi_dsi_cmds_tx(acl_switch_cmd, ARRAY_SIZE(acl_switch_cmd), mipi_dsi0_base);
			g_acl_ctrl = ACL_OFFSET1_ON;
			HISI_FB_INFO("ACL OFFSET 1 ON\n");
		} else if (ACL_OFFSET2_ON == value[1]) {
			acl_switch_cmd[0].payload[0] = 0x55;
			acl_switch_cmd[0].payload[1] = ACL_OFFSET2_ON;
			mipi_dsi_cmds_tx(acl_switch_cmd, ARRAY_SIZE(acl_switch_cmd), mipi_dsi0_base);
			g_acl_ctrl = ACL_OFFSET2_ON;
			HISI_FB_INFO("ACL OFFSET 2 ON\n");
		} else if (ACL_OFFSET3_ON == value[1]) {
			acl_switch_cmd[0].payload[0] = 0x55;
			acl_switch_cmd[0].payload[1] = ACL_OFFSET3_ON;
			mipi_dsi_cmds_tx(acl_switch_cmd, ARRAY_SIZE(acl_switch_cmd), mipi_dsi0_base);
			g_acl_ctrl = ACL_OFFSET3_ON;
			HISI_FB_INFO("ACL OFFSET 3 ON\n");
		} else {
			HISI_FB_INFO("invalid parm!\n");
		}
	} else {
		HISI_FB_INFO("invalid parm!\n");
	}
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
	return count;
}

static ssize_t mipi_samsung_s6e3hf4_errflag_check(struct platform_device *pdev,
	char *buf)
{
	ssize_t ret = 0;
	u8 result_value = 0;
	int errflag_gpio = 0;

	struct hisi_fb_data_type *hisifd = NULL;

	if(pdev == NULL){
		HISI_FB_ERR("pdev is null\n");
		return -1;
	}

	hisifd = platform_get_drvdata(pdev);

	if (!hisifd) {
		HISI_FB_ERR("hisifd is null\n");
		ret = snprintf(buf, PAGE_SIZE, "%d\n", result_value);
		return ret;
	}

	if (!hisifd->panel_power_on){
		HISI_FB_INFO("panel is poweroff\n");
		ret = snprintf(buf, PAGE_SIZE, "%d\n", result_value);
		return ret;
	}

	errflag_gpio = gpio_get_value(gpio_lcd_err_flag);
	HISI_FB_INFO("errflag:%u\n", errflag_gpio);

	if (!errflag_gpio) {
		result_value = 0; // PCD_ERR_FLAG_SUCCESS
	} else {
		result_value = 2; //only ERRFLAG FAIL
	}

	errflag_gpio = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", result_value);
	return ret;
}

/*for esd check*/
static int mipi_samsung_s6e3hf4_check_esd(struct platform_device* pdev)
{
	int ret = 0, errflag_detect=0, power_status_reg_detect=0, te_status_detect=0, i = 0;
	struct hisi_fb_data_type* hisifd = NULL;

	uint32_t read_value_power_mode[1] = {0};
	uint32_t expected_value_power_mode[1] = {0x9c};
	uint32_t read_mask_power_mode[1] = {0xff};
	char* reg_name_power_mode[1] = {"power mode"};
	char lcd_reg_power_mode[1] = {0x0a};

	struct dsi_cmd_desc lcd_check_reg_power_mode[] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_power_mode), lcd_reg_power_mode},
	};

	struct mipi_dsi_read_compare_data data_power_mode = {
		.read_value = read_value_power_mode,
		.expected_value = expected_value_power_mode,
		.read_mask = read_mask_power_mode,
		.reg_name = reg_name_power_mode,
		.log_on = 1,
		.cmds = lcd_check_reg_power_mode,
		.cnt = ARRAY_SIZE(lcd_check_reg_power_mode),
	};

	uint32_t read_value_te_mode[1] = {0};
	uint32_t expected_value_te_mode[1] = {0x80};
	uint32_t read_mask_te_mode[1] = {0xff};
	char* reg_name_te_mode[1] = {"TE mode"};
	char lcd_reg_te_mode[1] = {0x0e};

	struct dsi_cmd_desc lcd_check_reg_te_mode[] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_te_mode), lcd_reg_te_mode},
	};

	struct mipi_dsi_read_compare_data data_te_mode = {
		.read_value = read_value_te_mode,
		.expected_value = expected_value_te_mode,
		.read_mask = read_mask_te_mode,
		.reg_name = reg_name_te_mode,
		.log_on = 1,
		.cmds = lcd_check_reg_te_mode,
		.cnt = ARRAY_SIZE(lcd_check_reg_te_mode),
	};

	struct dsi_cmd_desc pkt_size_cmd;

	memset(&pkt_size_cmd, 0, sizeof(struct dsi_cmd_desc));
	pkt_size_cmd.dtype = DTYPE_MAX_PKTSIZE;
	pkt_size_cmd.dlen = 1;


	if (NULL == pdev) {
		HISI_FB_ERR("NULL pdev Pointer\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type*)platform_get_drvdata(pdev);
	if(hisifd == NULL) {
		HISI_FB_ERR("NULL hisifd Pointer\n");
		return -EINVAL;
	};

	if (NULL == hisifd->mipi_dsi0_base) {
		HISI_FB_ERR("NULL mipi_dsi0_base Pointer\n");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);
	if(gpio_get_value(gpio_lcd_err_flag) == 1){
		HISI_FB_INFO("esd err_flag detect!\n");
		errflag_detect = 1;
		return errflag_detect;
	}else{
		HISI_FB_DEBUG("no err_flag detect!\n");
		errflag_detect = 0;
	}

	if (false == g_display_on) {
		HISI_FB_DEBUG("display on false right now, stop ESD detect\n");
		return 0;
	}else{
		//we need 20 ms time to ensure that read 0A normal
		mdelay(20);
	}

	mipi_dsi_max_return_packet_size(&pkt_size_cmd, hisifd->mipi_dsi0_base);
	ret = mipi_dsi_read_compare(&data_power_mode, hisifd->mipi_dsi0_base);
	if (!ret){
		power_status_reg_detect = 0;
	} else 	{
		if (read_value_power_mode[0]) {
			power_status_reg_detect = 1;
			HISI_FB_INFO("esd power status detect abnormal:0x%02x, expected:0x%02x\n", read_value_power_mode[0], expected_value_power_mode[0]);
			return power_status_reg_detect;
		}else{
			power_status_reg_detect = 0;
			HISI_FB_DEBUG("esd power status detect 0\n");
		}
	}
	mipi_dsi_max_return_packet_size(&pkt_size_cmd, hisifd->mipi_dsi0_base);
	ret = mipi_dsi_read_compare(&data_te_mode, hisifd->mipi_dsi0_base);
	if (!ret){
		te_status_detect = 0;
	} else {
		if (read_value_te_mode[0]) {
			te_status_detect = 1;
			HISI_FB_INFO("esd te mode(0xe0) detect abnormal:0x%02x, expected:0x%02x\n", read_value_te_mode[0], expected_value_te_mode[0]);
			return te_status_detect;
		}else{
			te_status_detect = 0;
			HISI_FB_DEBUG("esd te mode detect 0\n");
		}
	}
	ret = errflag_detect + power_status_reg_detect + te_status_detect;
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static char lcd_disp_x[] = {
	0x2A,
	0x00, 0x00,0x05,0x9F
};

static char lcd_disp_y[] = {
	0x2B,
	0x00, 0x00,0x09,0xFF
};

static struct dsi_cmd_desc set_display_address[] = {
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(lcd_disp_x), lcd_disp_x},
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(lcd_disp_y), lcd_disp_y},
};

static int mipi_samsung_panel_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL || dirty == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	lcd_disp_x[1] = (dirty->x >> 8) & 0xff;
	lcd_disp_x[2] = dirty->x & 0xff;
	lcd_disp_x[3] = ((dirty->x + dirty->w - 1) >> 8) & 0xff;
	lcd_disp_x[4] = (dirty->x + dirty->w - 1) & 0xff;
	lcd_disp_y[1] = (dirty->y >> 8) & 0xff;
	lcd_disp_y[2] = dirty->y & 0xff;
	lcd_disp_y[3] = ((dirty->y + dirty->h - 1) >> 8) & 0xff;
	lcd_disp_y[4] = (dirty->y + dirty->h - 1) & 0xff;

	HISI_FB_DEBUG("x[1] = 0x%2x, x[2] = 0x%2x, x[3] = 0x%2x, x[4] = 0x%2x.\n",
		lcd_disp_x[1], lcd_disp_x[2], lcd_disp_x[3], lcd_disp_x[4]);
	HISI_FB_DEBUG("y[1] = 0x%2x, y[2] = 0x%2x, y[3] = 0x%2x, y[4] = 0x%2x.\n",
		lcd_disp_y[1], lcd_disp_y[2], lcd_disp_y[3], lcd_disp_y[4]);

	mipi_dsi_cmds_tx(set_display_address, \
		ARRAY_SIZE(set_display_address), hisifd->mipi_dsi0_base);

	return 0;
}

static ssize_t mipi_samsung_s6e3hf4_support_mode_show(struct platform_device *pdev,
		char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = snprintf(buf, PAGE_SIZE, "%d\n", g_support_mode);
	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
	return ret;
}

static ssize_t mipi_samsung_s6e3hf4_support_mode_store(struct platform_device *pdev,
		const char *buf, size_t count)
{
	int ret = 0;
	unsigned long val = 0;
	int flag = -1;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	ret = strict_strtoul(buf, 0, &val);
	if (ret)
		return ret;

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	flag = (int)val;

	g_support_mode = flag;

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
	return snprintf((char *)buf, PAGE_SIZE, "%d\n", g_support_mode);
}

static struct hisi_panel_info g_panel_info = {0};
static struct hisi_fb_panel_data g_panel_data = {
	.panel_info = &g_panel_info,
	.set_fastboot = mipi_samsung_s6e3hf4_set_fastboot,
	.on = mipi_samsung_s6e3hf4_on,
	.off = mipi_samsung_s6e3hf4_off,
	.lcd_acl_ctrl_show = mipi_samsung_S6E3HF4_panel_acl_ctrl_show,
	.lcd_acl_ctrl_store = mipi_samsung_S6E3HF4_panel_acl_ctrl_store,
	.remove = mipi_samsung_s6e3hf4_remove,
	.set_backlight = mipi_samsung_s6e3hf4_set_brightness,
	.lcd_model_show = mipi_samsung_s6e3hf4_model_show,
	.lcd_sleep_ctrl_show = mipi_samsung_s6e3hf4_sleep_ctrl_show,
	.lcd_sleep_ctrl_store = mipi_samsung_s6e3hf4_sleep_ctrl_store,
	.set_display_resolution = NULL,
	.amoled_pcd_errflag_check = mipi_samsung_s6e3hf4_errflag_check,
	.esd_handle = mipi_samsung_s6e3hf4_check_esd,
	.set_display_region = mipi_samsung_panel_set_display_region,
	.lcd_support_mode_show = mipi_samsung_s6e3hf4_support_mode_show,
	.lcd_support_mode_store = mipi_samsung_s6e3hf4_support_mode_store,
};

/*******************************************************************************
**
*/
static int mipi_samsung_s6e3hf4_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	uint32_t bl_type = 0;

	if (hisi_fb_device_probe_defer(PANEL_MIPI_CMD, bl_type))
		goto err_probe_defer;

	HISI_FB_INFO("+.\n");

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SAMSUNG_S6E3HF4);
	if (!np) {
		HISI_FB_ERR("NOT FOUND device node %s!\n", DTS_COMP_SAMSUNG_S6E3HF4);
		goto err_return;
	}

	gpio_lcd_reset = of_get_named_gpio(np, "gpios", 0);  /*gpio_2_4, gpio_020*/
	gpio_lcd_id0 = of_get_named_gpio(np, "gpios", 1);  /*gpio_1_0, gpio_008*/
	gpio_lcd_err_flag = of_get_named_gpio(np, "gpios", 2); /*gpio_10_1, gpio_81*/

	pdev->id = 1;
	// init lcd panel info
	pinfo = g_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->xres = 1440;
	pinfo->yres = 2560;
	pinfo->width = 63;
	pinfo->height = 113;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = BL_SET_BY_MIPI;

	pinfo->bl_min = 2;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;

	pinfo->frc_enable = 0;
	pinfo->esd_enable = 1;
	pinfo->esd_skip_mipi_check = 1;
	pinfo->lcd_uninit_step_support = 1;
	pinfo->lcd_adjust_support = 1;

	pinfo->color_temperature_support = 1;
	pinfo->comform_mode_support = 1;
	g_support_mode = COMFORM_MODE;

	//prefix ce & sharpness
	pinfo->prefix_ce_support = 0;
	pinfo->prefix_sharpness1D_support = 1;
	pinfo->prefix_sharpness2D_support = 1;

	//sbl
	pinfo->sbl_support = 1;
	pinfo->smart_bl.strength_limit = 128;
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
	pinfo->smart_bl.variance_intensity_space = 145;
	pinfo->smart_bl.slope_max = 54;
	pinfo->smart_bl.slope_min = 160;

	//ACM
	pinfo->acm_support = 0;
	if (pinfo->acm_support == 1) {
		pinfo->acm_lut_hue_table = acm_lut_hue_table;
		pinfo->acm_lut_hue_table_len = ARRAY_SIZE(acm_lut_hue_table);
		pinfo->acm_lut_sata_table = acm_lut_sata_table;
		pinfo->acm_lut_sata_table_len = ARRAY_SIZE(acm_lut_sata_table);
		pinfo->acm_lut_satr_table = acm_lut_satr_table;
		pinfo->acm_lut_satr_table_len = ARRAY_SIZE(acm_lut_satr_table);
		pinfo->acm_valid_num = acm_valid_num;
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

		//ACM_CE
		pinfo->acm_ce_support = 1;
	}
 
	// Contrast Algorithm
	if (pinfo->prefix_ce_support == 1 || pinfo->acm_ce_support == 1) {
		pinfo->ce_alg_param.iDiffMaxTH = 900;
		pinfo->ce_alg_param.iDiffMinTH = 100;
		pinfo->ce_alg_param.iFlatDiffTH = 500;
		pinfo->ce_alg_param.iAlphaMinTH = 16;
		pinfo->ce_alg_param.iBinDiffMaxTH = 40000;

		pinfo->ce_alg_param.iDarkPixelMinTH = 16;
		pinfo->ce_alg_param.iDarkPixelMaxTH = 24;
		pinfo->ce_alg_param.iDarkAvePixelMinTH = 40;
		pinfo->ce_alg_param.iDarkAvePixelMaxTH = 80;
		pinfo->ce_alg_param.iWhitePixelTH = 236;
		pinfo->ce_alg_param.fweight = 42;
		pinfo->ce_alg_param.fDarkRatio = 51;
		pinfo->ce_alg_param.fWhiteRatio = 51;

		pinfo->ce_alg_param.iDarkPixelTH = 64;
		pinfo->ce_alg_param.fDarkSlopeMinTH = 149;
		pinfo->ce_alg_param.fDarkSlopeMaxTH = 161;
		pinfo->ce_alg_param.fDarkRatioMinTH = 18;
		pinfo->ce_alg_param.fDarkRatioMaxTH = 38;

		pinfo->ce_alg_param.iBrightPixelTH = 192;
		pinfo->ce_alg_param.fBrightSlopeMinTH = 149;
		pinfo->ce_alg_param.fBrightSlopeMaxTH = 174;
		pinfo->ce_alg_param.fBrightRatioMinTH = 20;
		pinfo->ce_alg_param.fBrightRatioMaxTH = 36;

		pinfo->ce_alg_param.iZeroPos0MaxTH = 120;
		pinfo->ce_alg_param.iZeroPos1MaxTH = 128;

		pinfo->ce_alg_param.iDarkFMaxTH = 16;
		pinfo->ce_alg_param.iDarkFMinTH = 12;
		pinfo->ce_alg_param.iPos0MaxTH = 120;
		pinfo->ce_alg_param.iPos0MinTH = 96;

		pinfo->ce_alg_param.fKeepRatio = 61;
	}

	//Gama LCP
	pinfo->gamma_support = 1;
	if (pinfo->gamma_support == 1) {
		pinfo->gamma_lut_table_R = gamma_lut_table_R;
		pinfo->gamma_lut_table_G = gamma_lut_table_G;
		pinfo->gamma_lut_table_B = gamma_lut_table_B;
		pinfo->gamma_lut_table_len = ARRAY_SIZE(gamma_lut_table_R);
		pinfo->igm_lut_table_R = igm_lut_table_R;
		pinfo->igm_lut_table_G = igm_lut_table_G;
		pinfo->igm_lut_table_B = igm_lut_table_B;
		pinfo->igm_lut_table_len = ARRAY_SIZE(igm_lut_table_R);

		pinfo->gmp_support = 1;
		pinfo->gmp_lut_table_low32bit = &gmp_lut_table_low32bit[0][0][0];
		pinfo->gmp_lut_table_high4bit = &gmp_lut_table_high4bit[0][0][0];
		pinfo->gmp_lut_table_len = ARRAY_SIZE(gmp_lut_table_low32bit);

		pinfo->xcc_support = 1;
		pinfo->xcc_table = xcc_table;
		pinfo->xcc_table_len = ARRAY_SIZE(xcc_table);
	}

	//ldi
	pinfo->ldi.h_back_porch = 96;
	pinfo->ldi.h_front_porch = 108;
	pinfo->ldi.h_pulse_width = 48;
	pinfo->ldi.v_back_porch = 12;
	pinfo->ldi.v_front_porch = 14;
	pinfo->ldi.v_pulse_width = 4;

	//mipi
	pinfo->mipi.dsi_bit_clk = 480;
	pinfo->mipi.dsi_bit_clk_val1 = 471;
	pinfo->mipi.dsi_bit_clk_val2 = 480;
	pinfo->mipi.dsi_bit_clk_val3 = 490;
	pinfo->mipi.dsi_bit_clk_val4 = 500;
	//pinfo->mipi.dsi_bit_clk_val5 = ;
	pinfo->dsi_bit_clk_upt_support = 1;
	pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
	pinfo->mipi.clk_post_adjust = 215;
	pinfo->pxl_clk_rate = 288 * 1000000UL;

	//mipi
	pinfo->mipi.lane_nums = DSI_4_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;
	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.burst_mode = 0;
	pinfo->mipi.non_continue_en = 1;
	//non_continue adjust : measured in UI
	//requires clk_post >= 60ns + 252ui, so need a clk_post_adjust more than 200ui. Here 215 is used.
	pinfo->mipi.clk_post_adjust = 215;

	pinfo->pxl_clk_rate_div = 1;

	pinfo->type = PANEL_MIPI_CMD;
	pinfo->ifbc_type = IFBC_TYPE_VESA3X_DUAL;
	pinfo->vsync_ctrl_type = VSYNC_CTRL_ISR_OFF | VSYNC_CTRL_MIPI_ULPS | VSYNC_CTRL_CLK_OFF;

	pinfo->dirty_region_updt_support = 1;
	pinfo->dirty_region_info.left_align = -1;
	pinfo->dirty_region_info.right_align = -1;
	pinfo->dirty_region_info.top_align = 32;
	pinfo->dirty_region_info.bottom_align = 32;
	pinfo->dirty_region_info.w_align = -1;
	pinfo->dirty_region_info.h_align = -1;
	pinfo->dirty_region_info.w_min = 1440;
	pinfo->dirty_region_info.h_min = 32;
	pinfo->dirty_region_info.top_start = -1;
	pinfo->dirty_region_info.bottom_start = -1;

	/* The host processor must wait for more than 15us from the end of write data transfer to a command 2Ah/2Bh */
	if (pinfo->dirty_region_updt_support == 1)
		pinfo->mipi.hs_wr_to_time = 17000;        /* measured in nS */

	if(runmode_is_factory()) {
		HISI_FB_INFO("Factory mode, disable features: dirty update etc.\n");
		pinfo->dirty_region_updt_support = 0;
		pinfo->prefix_ce_support = 0;
		pinfo->prefix_sharpness1D_support = 0;
		pinfo->prefix_sharpness2D_support = 0;
		pinfo->sbl_support = 0;
		pinfo->acm_support = 0;
		pinfo->acm_ce_support = 0;
		pinfo->esd_enable = 0;
		pinfo->blpwm_input_ena = 0;
		pinfo->blpwm_precision_type = BLPWM_PRECISION_DEFAULT_TYPE;
		pinfo->bl_min = 2;
		pinfo->bl_max = 255;
		pinfo->comform_mode_support = 0;
		g_support_mode = 0;
	}

	if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL) {
		pinfo->pxl_clk_rate_div = 3;

		/* dsc parameter info */
		pinfo->vesa_dsc.bits_per_component = 8;
		pinfo->vesa_dsc.bits_per_pixel = 8;
		pinfo->vesa_dsc.slice_width = 719;
		pinfo->vesa_dsc.slice_height = 31;

		pinfo->vesa_dsc.initial_xmit_delay = 512;
		pinfo->vesa_dsc.first_line_bpg_offset = 12;
		pinfo->vesa_dsc.mux_word_size = 48;

		/*  DSC_CTRL */
		pinfo->vesa_dsc.block_pred_enable = 1;
		pinfo->vesa_dsc.linebuf_depth = 9;

		/* RC_PARAM3 */
		pinfo->vesa_dsc.initial_offset = 6144;

		/* FLATNESS_QP_TH */
		pinfo->vesa_dsc.flatness_min_qp = 3;
		pinfo->vesa_dsc.flatness_max_qp = 12;

		/* DSC_PARAM4 */
		pinfo->vesa_dsc.rc_edge_factor= 0x6;
		pinfo->vesa_dsc.rc_model_size = 8192;

		/* DSC_RC_PARAM5: 0x330b0b */
		pinfo->vesa_dsc.rc_tgt_offset_lo = (0x330b0b >> 20) & 0xF;
		pinfo->vesa_dsc.rc_tgt_offset_hi = (0x330b0b >> 16) & 0xF;
		pinfo->vesa_dsc.rc_quant_incr_limit1 = (0x330b0b >> 8) & 0x1F;
		pinfo->vesa_dsc.rc_quant_incr_limit0 = (0x330b0b >> 0) & 0x1F;

		/* DSC_RC_BUF_THRESH0: 0xe1c2a38 */
		pinfo->vesa_dsc.rc_buf_thresh0 = (0xe1c2a38 >> 24) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh1 = (0xe1c2a38 >> 16) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh2 = (0xe1c2a38 >> 8) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh3 = (0xe1c2a38 >> 0) & 0xFF;

		/* DSC_RC_BUF_THRESH1: 0x46546269 */
		pinfo->vesa_dsc.rc_buf_thresh4 = (0x46546269 >> 24) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh5 = (0x46546269 >> 16) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh6 = (0x46546269 >> 8) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh7 = (0x46546269 >> 0) & 0xFF;

		/* DSC_RC_BUF_THRESH2: 0x7077797b */
		pinfo->vesa_dsc.rc_buf_thresh8 = (0x7077797b >> 24) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh9 = (0x7077797b >> 16) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh10 = (0x7077797b >> 8) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh11 = (0x7077797b >> 0) & 0xFF;

		/* DSC_RC_BUF_THRESH3: 0x7d7e0000 */
		pinfo->vesa_dsc.rc_buf_thresh12 = (0x7d7e0000 >> 24) & 0xFF;
		pinfo->vesa_dsc.rc_buf_thresh13 = (0x7d7e0000 >> 16) & 0xFF;

		/* DSC_RC_RANGE_PARAM0: 0x1020100 */
		pinfo->vesa_dsc.range_min_qp0 = (0x1020100 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp0 = (0x1020100 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset0 = (0x1020100 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp1 = (0x1020100 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp1 = (0x1020100 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset1 = (0x1020100 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM1: 0x94009be */
		pinfo->vesa_dsc.range_min_qp2 = (0x94009be >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp2 = (0x94009be >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset2 = (0x94009be >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp3 = (0x94009be >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp3 = (0x94009be >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset3 = (0x94009be >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM2, 0x19fc19fa */
		pinfo->vesa_dsc.range_min_qp4 = (0x19fc19fa >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp4 = (0x19fc19fa >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset4 = (0x19fc19fa >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp5 = (0x19fc19fa >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp5 = (0x19fc19fa >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset5 = (0x19fc19fa >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM3, 0x19f81a38 */
		pinfo->vesa_dsc.range_min_qp6 = (0x19f81a38 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp6 = (0x19f81a38 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset6 = (0x19f81a38 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp7 = (0x19f81a38 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp7 = (0x19f81a38 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset7 = (0x19f81a38 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM4, 0x1a781ab6 */
		pinfo->vesa_dsc.range_min_qp8 = (0x1a781ab6 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp8 = (0x1a781ab6 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset8 = (0x1a781ab6 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp9 = (0x1a781ab6 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp9 = (0x1a781ab6 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset9 = (0x1a781ab6 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM5, 0x2af62b34 */
		pinfo->vesa_dsc.range_min_qp10 = (0x2af62b34 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp10 = (0x2af62b34 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset10 = (0x2af62b34 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp11 = (0x2af62b34 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp11 = (0x2af62b34 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset11 = (0x2af62b34 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM6, 0x2b743b74 */
		pinfo->vesa_dsc.range_min_qp12 = (0x2b743b74 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp12 = (0x2b743b74 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset12 = (0x2b743b74 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp13 = (0x2b743b74 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp13 = (0x2b743b74 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset13 = (0x2b743b74 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM7, 0x6bf40000 */
		pinfo->vesa_dsc.range_min_qp14 = (0x6bf40000 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp14 = (0x6bf40000 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset14 = (0x6bf40000 >> 16) & 0x3F;


		if (pinfo->pxl_clk_rate_div > 1) {
			pinfo->ldi.h_back_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_front_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_pulse_width /= pinfo->pxl_clk_rate_div;
		}
		/* IFBC Setting end */
	}

	// lcd vcc init
	ret = vcc_cmds_tx(pdev, lcd_vcc_init_cmds,
		ARRAY_SIZE(lcd_vcc_init_cmds));
	if (ret != 0) {
		HISI_FB_ERR("LCD vcc init failed!\n");
		goto err_return;
	}

	// lcd vcc enable
	if (is_fastboot_display_enable()) {
		vcc_cmds_tx(pdev, lcd_vcc_enable_cmds,
			ARRAY_SIZE(lcd_vcc_enable_cmds));
	}

	// lcd pinctrl init
	ret = pinctrl_cmds_tx(pdev, lcd_pinctrl_init_cmds,
		ARRAY_SIZE(lcd_pinctrl_init_cmds));
	if (ret != 0) {
		HISI_FB_ERR("Init pinctrl failed, defer\n");
		goto err_return;
	}

	/* alloc panel device data */
	ret = platform_device_add_data(pdev, &g_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		HISI_FB_ERR("platform_device_add_data failed!\n");
		goto err_device_put;
	}


	hisi_fb_add_device(pdev);

	HISI_FB_INFO("-.\n");

	return 0;

err_device_put:
	platform_device_put(pdev);
err_return:
	return ret;
err_probe_defer:
	return -EPROBE_DEFER;

	return ret;
}

static const struct of_device_id hisi_panel_match_table[] = {
    {
        .compatible = DTS_COMP_SAMSUNG_S6E3HF4,
        .data = NULL,
    },
    {},
};
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);

static struct platform_driver this_driver = {
	.probe = mipi_samsung_s6e3hf4_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "mipi_samsung_S6E3HF4",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init mipi_samsung_s6e3hf4_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(mipi_samsung_s6e3hf4_init);
