/* Copyright (c) 2016-2022, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *	 * Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	 * Redistributions in binary form must reproduce the above
 *	   copyright notice, this list of conditions and the following
 *	   disclaimer in the documentation and/or other materials provided
 *	   with the distribution.
 *	 * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *	   contributors may be used to endorse or promote products derived
 *	   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "hisi_fb.h"

#define DTS_COMP_NT36860 "hisilicon,mipi_sharp_NT36860"
static int g_lcd_fpga_flag;
/*lint -e569*/
/*******************************************************************************
** Power ON Sequence(sleep mode to Normal mode) begin
*/
//switch to CMD1
static char page_select[] = {
	0xFF,
	0x10,
};

static char reload_cmd[] = {
	0xFB,
	0x01,
};

// 3 Lanes BAh=0x03,3 Lanes BAh = 0x02,2 Lanes BAh = 0x01,1 Lanes BAh = 0x00
// 0x13 lane0 continuous,  0x03 lane1 continuous
static char lane_number[] = {
	0xBA,
	0x03,
};
//hsync mode 0x01, vsync mode 0x00
static char tear_on[] = {
	0x35,
	0x00,
};

static char fte_modeC_enable[] = {
	0x44,
	0x05,0x00,
};

//SDC(0x00),DSC(0x03),HD720(0x04),FHD(0x05)
static char vesa3x_enable[] = {
	0xC0,
	0x03,
};

static char vesa3_75x_slice8L_C1[] = {
	0xC1,
	0xAB,0x28,0x00,0x08,0x02,0x00,0x02,0x68,0x00,0xD5,0x00,0x0A,0x0D,0xB7,0x09,0x89,
};

static char vesa3_75x_slice8L_C2[] = {
	0xC2,
	0x10,0xF0,
};

static char sleep_out[] = {
	0x11,
};

static char led_pwm_full_duty[] = {
	0x51,
	0xFF,
};

//bl_ctl_on/off is used to set led PWM control
static char bl_ctl_on[] = {
	0x53,
	0x24,
};

static char display_on[] = {
	0x29,
};

static char cmd_mode[] = {
	0xBB,
	0x10,
};

//video mode,0x03 GRAM access disable,0x13 GRAM access enable
static char video_mode[] = {
	0xBB,
	0x03,
};

//Param[0:3] = VBP[8:11] VBP[0:7] VFP[8:11] VFP[0:7]
//0x00,0x0C,0x00,0x0E,
static char video_control[] = {
	0x3B,
	0x00,0x10,0x00,0x0E,
};

static char CMD3_page_sel[] = {
	0xFF,
	0xD0,
};

static char video_timeout_disable[] = {
	0x90,
	0x1F,
};

static char CMD2_page4_sel[] = {
	0xFF,
	0x24,
};

static char video_drop_disable[] = {
	0xF5,
	0x00,
};

/*Random 00h, Black 01h*/
/*static char bk_en[] = {
	0xE5,
	0x01,
};*/

/*
** Power ON Sequence end
*******************************************************************************/

/*******************************************************************************
** Power OFF Sequence(Normal to power off) begin
*/
static char display_off[] = {
	0x28,
};

static char sleep_in[] = {
	0x10,
};

/*lint +e569*/

/*
** Power OFF Sequence end
*******************************************************************************/

static struct dsi_cmd_desc lcd_display_init_cmds[] = {
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(page_select), page_select},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(reload_cmd), reload_cmd},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(lane_number), lane_number},
	{DTYPE_DCS_WRITE1, 0,10, WAIT_TYPE_US,
		sizeof(tear_on), tear_on},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US,
		sizeof(fte_modeC_enable), fte_modeC_enable},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(led_pwm_full_duty), led_pwm_full_duty},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(bl_ctl_on), bl_ctl_on},

};

//only for 5HZ, >30HZ delete this
static struct dsi_cmd_desc lcd_display_init_cmds_for5HZ[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(CMD3_page_sel), CMD3_page_sel},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(reload_cmd), reload_cmd},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(video_timeout_disable), video_timeout_disable},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(CMD2_page4_sel), CMD2_page4_sel},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(reload_cmd), reload_cmd},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(video_drop_disable), video_drop_disable},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(page_select), page_select},
};

static struct dsi_cmd_desc lcd_display_cmd_mode_cmds[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_MS,
		sizeof(cmd_mode), cmd_mode},
};

static struct dsi_cmd_desc lcd_display_video_mode_cmds[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_MS,
		sizeof(video_mode), video_mode},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_MS,
		sizeof(video_control), video_control},
};

static struct dsi_cmd_desc lcd_ifbc_vesa3_75x_slice8L_init_cmd[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3_75x_slice8L_C1), vesa3_75x_slice8L_C1},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US,
		sizeof(vesa3_75x_slice8L_C2), vesa3_75x_slice8L_C2},
};

static struct dsi_cmd_desc lcd_ifbc_vesa_en_cmd[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US,
		sizeof(vesa3x_enable), vesa3x_enable},
};

static struct dsi_cmd_desc lcd_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 200, WAIT_TYPE_MS,
		sizeof(sleep_out), sleep_out},
	{DTYPE_DCS_WRITE, 0, 100, WAIT_TYPE_MS,
		sizeof(display_on), display_on},
};

static struct dsi_cmd_desc lcd_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 60, WAIT_TYPE_MS,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 0, 240, WAIT_TYPE_MS,
		sizeof(sleep_in), sleep_in}
};

/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_P5V8_ENABLE_NAME	"gpio_lcd_p5v8_enable"
#define GPIO_LCD_N5V8_ENABLE_NAME "gpio_lcd_n5v8_enable"
#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_BL_ENABLE_NAME	"gpio_lcd_bl_enable"
#define GPIO_LCD_TP1V8_NAME	"gpio_lcd_1v8"

static uint32_t gpio_lcd_p5v8_enable;
static uint32_t gpio_lcd_n5v8_enable;
static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_bl_enable;
static uint32_t gpio_lcd_1v8;

static struct gpio_desc fpga_lcd_gpio_request_cmds[] = {
	/* AVDD_5.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},
};

static struct gpio_desc fpga_lcd_gpio_on_cmds[] = {
	/* TP _1.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 1},
	/* AVDD_5.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 1},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 50,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc lcd_gpio_off_cmds[] = {
	/* backlight disable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* AVDD_5.8V*/
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* TP_1.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},

	/* backlight enable input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.8V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* AVDD_5.8V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* reset input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* TP_1.8V */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},

};

static struct gpio_desc lcd_gpio_free_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_N5V8_ENABLE_NAME, &gpio_lcd_n5v8_enable, 0},
	/* AVDD_5.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_P5V8_ENABLE_NAME, &gpio_lcd_p5v8_enable, 0},
	/* TP_1.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_1v8, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

/*******************************************************************************
**
*/
/*lint -e747 -e846 -e516 -e514 -e866 -e30 -e84 -e778 -e845 -e572*/
static int mipi_sharp_NT36860_panel_set_fastboot(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_lcd_fpga_flag == 0) {
		// lcd pinctrl normal
		// lcd gpio request
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

static int mipi_sharp_NT36860_panel_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;
	uint32_t status = 0;
	uint32_t try_times = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
	pinfo = &(hisifd->panel_info);
	BUG_ON(pinfo == NULL);

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	pinfo = &(hisifd->panel_info);
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	if (pinfo->lcd_init_step == LCD_INIT_POWER_ON) {
		// check lcd power state
		if (g_lcd_fpga_flag == 0) {
			;
		} else {
			gpio_cmds_tx(fpga_lcd_gpio_request_cmds, \
				ARRAY_SIZE(fpga_lcd_gpio_request_cmds));
			gpio_cmds_tx(fpga_lcd_gpio_on_cmds, \
				ARRAY_SIZE(fpga_lcd_gpio_on_cmds));
		}
		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;

	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		mipi_dsi_cmds_tx(lcd_display_init_cmds, \
			ARRAY_SIZE(lcd_display_init_cmds), mipi_dsi0_base);

		if (g_lcd_fpga_flag) {
			mipi_dsi_cmds_tx(lcd_display_init_cmds_for5HZ, \
				ARRAY_SIZE(lcd_display_init_cmds_for5HZ), mipi_dsi0_base);
		}

		if (is_mipi_cmd_panel(hisifd)) {
			mipi_dsi_cmds_tx(lcd_display_cmd_mode_cmds, \
				ARRAY_SIZE(lcd_display_cmd_mode_cmds), mipi_dsi0_base);
		} else {
			HISI_FB_INFO(" is PANEL_MIPI_VIDEO mode\n");
			mipi_dsi_cmds_tx(lcd_display_video_mode_cmds, \
				ARRAY_SIZE(lcd_display_video_mode_cmds), mipi_dsi0_base);
		}

		if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_DUAL) {
			mipi_dsi_cmds_tx(lcd_ifbc_vesa_en_cmd, \
				ARRAY_SIZE(lcd_ifbc_vesa_en_cmd), mipi_dsi0_base);
		} else if (pinfo->ifbc_type == IFBC_TYPE_VESA3_75X_DUAL) {
			HISI_FB_ERR("IFBC_TYPE_VESA3_75X_DUAL +++\n");
			mipi_dsi_cmds_tx(lcd_ifbc_vesa3_75x_slice8L_init_cmd, \
				ARRAY_SIZE(lcd_ifbc_vesa3_75x_slice8L_init_cmd), mipi_dsi0_base);
			mipi_dsi_cmds_tx(lcd_ifbc_vesa_en_cmd, \
				ARRAY_SIZE(lcd_ifbc_vesa_en_cmd), mipi_dsi0_base);
		}

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

	// backlight on
	hisi_lcd_backlight_on(pdev);
	HISI_FB_DEBUG("fb%d, -!\n", hisifd->index);

	return 0;
}


static int mipi_sharp_NT36860_panel_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *mipi_dsi0_base = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
	pinfo = &(hisifd->panel_info);
	mipi_dsi0_base = hisifd->mipi_dsi0_base;

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE) {

		hisi_lcd_backlight_off(pdev);

		mipi_dsi_cmds_tx(lcd_display_off_cmds, \
			ARRAY_SIZE(lcd_display_off_cmds), mipi_dsi0_base);
		pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;

	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_LP_SEND_SEQUENCE) {
		pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;

	} else if (pinfo->lcd_uninit_step == LCD_UNINIT_POWER_OFF) {
		if (g_lcd_fpga_flag==0) {

		} else {
			// lcd gpio lowpower
			gpio_cmds_tx(lcd_gpio_off_cmds, \
				ARRAY_SIZE(lcd_gpio_off_cmds));
			// lcd gpio free
			gpio_cmds_tx(lcd_gpio_free_cmds, \
				ARRAY_SIZE(lcd_gpio_free_cmds));
		}
	} else {
		HISI_FB_ERR("failed to uninit lcd!\n");
	}

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_sharp_NT36860_panel_remove(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		return 0;
	}
	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (g_lcd_fpga_flag == 0) {

	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
	return 0;
}

static int mipi_sharp_NT36860_panel_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
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

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {

		ret = hisi_pwm_set_backlight(hisifd, bl_level);

	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {

		ret = hisi_blpwm_set_backlight(hisifd, bl_level);

	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {

		ret = hisi_sh_blpwm_set_backlight(hisifd, bl_level);

	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) {

		bl_level_adjust[1] = bl_level * 255 / hisifd->panel_info.bl_max;

		if (last_bl_level != bl_level_adjust[1]){
			last_bl_level = bl_level_adjust[1];
			mipi_dsi_cmds_tx(lcd_bl_level_adjust, \
				ARRAY_SIZE(lcd_bl_level_adjust), hisifd->mipi_dsi0_base);
			}
		HISI_FB_INFO("lw bl_level_adjust[1] = %d.\n",bl_level_adjust[1]);

	} else {
		HISI_FB_ERR("fb%d, not support this bl_set_type(%d)!\n",
			hisifd->index, hisifd->panel_info.bl_set_type);
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
	0x00, 0x00,0x07,0x7F
};

static struct dsi_cmd_desc set_display_address[] = {
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(lcd_disp_x), lcd_disp_x},
	{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
		sizeof(lcd_disp_y), lcd_disp_y},
};

static int mipi_sharp_NT36860_panel_set_display_region(struct platform_device *pdev,
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
	}

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

static ssize_t mipi_sharp_NT36860_lcd_model_show(struct platform_device *pdev,
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

	ret = snprintf(buf, PAGE_SIZE, "sharp_NT36860 CMD TFT Boston\n");

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

/*******************************************************************************
**
*/
static struct hisi_panel_info g_panel_info = {0};
static struct hisi_fb_panel_data g_panel_data = {
	.panel_info = &g_panel_info,
	.set_fastboot = mipi_sharp_NT36860_panel_set_fastboot,
	.on = mipi_sharp_NT36860_panel_on,
	.off = mipi_sharp_NT36860_panel_off,
	.remove = mipi_sharp_NT36860_panel_remove,
	.set_backlight = mipi_sharp_NT36860_panel_set_backlight,
	.set_display_region = mipi_sharp_NT36860_panel_set_display_region,

	.lcd_model_show = mipi_sharp_NT36860_lcd_model_show,
	//.lcd_check_reg  = mipi_jdi_panel_lcd_check_reg_show,
};

/*******************************************************************************
**
*/
static int mipi_sharp_NT36860_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	uint32_t bl_type = 0;
	uint32_t lcd_display_type = 0;
	uint32_t lcd_ifbc_type = 0;

	HISI_FB_INFO("mipi_sharp_NT36860_probe+!\n");
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_NT36860);
	if (!np) {
		HISI_FB_ERR("not found device node %s!\n", DTS_COMP_NT36860);
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

	if (hisi_fb_device_probe_defer(lcd_display_type, bl_type))
		goto err_probe_defer;

	HISI_FB_DEBUG("+.\n");

	ret = of_property_read_u32(np, FPGA_FLAG_NAME, &g_lcd_fpga_flag);
	if (ret) {
		HISI_FB_WARNING("need to get g_lcd_fpga_flag resource in fpga, not needed in asic!\n");
	}

	if (g_lcd_fpga_flag == 1) {
		gpio_lcd_p5v8_enable = of_get_named_gpio(np, "gpios", 1);
		gpio_lcd_n5v8_enable = of_get_named_gpio(np, "gpios", 2);
		gpio_lcd_reset = of_get_named_gpio(np, "gpios", 3);
		gpio_lcd_bl_enable = of_get_named_gpio(np, "gpios", 4);
		gpio_lcd_1v8 = of_get_named_gpio(np, "gpios", 0);
	} else {

	}

	pdev->id = 1;
	//init lcd panel info
	pinfo = g_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->xres = 1440;
	pinfo->yres = 2560;
	pinfo->width = 75;
	pinfo->height = 133;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = bl_type;

	pinfo->bl_min = 1;
	pinfo->bl_max = 255;
	pinfo->bl_default = 102;

	pinfo->type = lcd_display_type;//PANEL_MIPI_VIDEO;//
	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;
	pinfo->esd_skip_mipi_check = 0;
	pinfo->lcd_uninit_step_support = 1;

	if (g_lcd_fpga_flag == 1) {
		if (pinfo->type == PANEL_MIPI_CMD) {
			//ldi
			pinfo->ldi.h_back_porch = 96;//0x6ff;//96;
			pinfo->ldi.h_front_porch = 1068; //108
			pinfo->ldi.h_pulse_width = 48;
			pinfo->ldi.v_back_porch = 12;
			pinfo->ldi.v_front_porch = 14;
			pinfo->ldi.v_pulse_width = 4;

			//mipi
			pinfo->mipi.dsi_bit_clk = 240;//240;
			pinfo->dsi_bit_clk_upt_support = 0;
			pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
			pinfo->pxl_clk_rate = 20* 1000000UL;
		} else {
			//ldi
			pinfo->ldi.h_back_porch = 96;//0x6ff;//96;
			pinfo->ldi.h_front_porch = 1068; //108
			pinfo->ldi.h_pulse_width = 48;
			pinfo->ldi.v_back_porch = 12;
			pinfo->ldi.v_front_porch = 14;
			pinfo->ldi.v_pulse_width = 4;

			//mipi
			pinfo->mipi.dsi_bit_clk = 240;
			pinfo->dsi_bit_clk_upt_support = 0;
			pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
			pinfo->pxl_clk_rate = 20* 1000000UL;
		}

	} else {
		;
	}

	pinfo->mipi.dsi_version = DSI_1_1_VERSION;

	pinfo->mipi.clk_post_adjust = 215;
	pinfo->mipi.clk_pre_adjust= 0;
	pinfo->mipi.clk_t_hs_prepare_adjust= 0;
	pinfo->mipi.clk_t_lpx_adjust= 0;
	pinfo->mipi.clk_t_hs_trial_adjust= 0;
	pinfo->mipi.clk_t_hs_exit_adjust= 0;
	pinfo->mipi.clk_t_hs_zero_adjust= 0;
	pinfo->mipi.non_continue_en = 1;

	//mipi
	pinfo->mipi.lane_nums = DSI_3_LANES;//DSI_3_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;//DSI_DSC24_COMPRESSED_DATA

	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
	pinfo->mipi.phy_mode = CPHY_MODE;

	pinfo->vsync_ctrl_type = 0;//VSYNC_CTRL_ISR_OFF | VSYNC_CTRL_MIPI_ULPS | VSYNC_CTRL_CLK_OFF;

	//IFBC_TYPE_VESA3X_DUAL & IFBC_TYPE_VESA3_75X_DUAL are both supported
	pinfo->ifbc_type = IFBC_TYPE_VESA3_75X_DUAL;//IFBC_TYPE_VESA3X_DUAL;//
	pinfo->pxl_clk_rate_div = 3;


	/* IFBC Setting begin */
	/* dsc parameter info */
	if (pinfo->ifbc_type == IFBC_TYPE_VESA3_75X_DUAL) {
		//pinfo->bpp = LCD_RGB101010;
		//pinfo->mipi.color_mode = DSI_30BITS_1;

		pinfo->vesa_dsc.bits_per_component = 10;
		pinfo->vesa_dsc.linebuf_depth = 11;
		pinfo->vesa_dsc.bits_per_pixel = 8;
		pinfo->vesa_dsc.initial_xmit_delay = 512;

		pinfo->vesa_dsc.slice_width = 719;//1439
		pinfo->vesa_dsc.slice_height = 7;//31;

		pinfo->vesa_dsc.first_line_bpg_offset = 12;
		pinfo->vesa_dsc.mux_word_size = 48;

		/* DSC_CTRL */
		pinfo->vesa_dsc.block_pred_enable = 1;//0;

		/* RC_PARAM3 */
		pinfo->vesa_dsc.initial_offset = 6144;

		/* FLATNESS_QP_TH */
		pinfo->vesa_dsc.flatness_min_qp = 7;
		pinfo->vesa_dsc.flatness_max_qp = 16;

		/* DSC_PARAM4 */
		pinfo->vesa_dsc.rc_edge_factor= 0x6;
		pinfo->vesa_dsc.rc_model_size = 8192;

		/* DSC_RC_PARAM5: 0x330f0f */
		pinfo->vesa_dsc.rc_tgt_offset_lo = (0x330f0f >> 20) & 0xF;
		pinfo->vesa_dsc.rc_tgt_offset_hi = (0x330f0f >> 16) & 0xF;
		pinfo->vesa_dsc.rc_quant_incr_limit1 = (0x330f0f >> 8) & 0x1F;
		pinfo->vesa_dsc.rc_quant_incr_limit0 = (0x330f0f >> 0) & 0x1F;

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

		/* DSC_RC_RANGE_PARAM0: 0x2022200 */
		pinfo->vesa_dsc.range_min_qp0 = (0x2022200 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp0 = (0x2022200 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset0 = (0x2022200 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp1 = (0x2022200 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp1 = (0x2022200 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset1 = (0x2022200 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM1: 0x94009be */
		pinfo->vesa_dsc.range_min_qp2 = 5;//(0x94009be >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp2 = 9;//(0x94009be >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset2 = (0x94009be >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp3 = 5;//(0x94009be >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp3 = 10;//(0x94009be >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset3 = (0x94009be >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM2, 0x19fc19fa */
		pinfo->vesa_dsc.range_min_qp4 = 7;//(0x19fc19fa >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp4 = 11;//(0x19fc19fa >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset4 = (0x19fc19fa >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp5 = 7;//(0x19fc19fa >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp5 = 11;//(0x19fc19fa >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset5 = (0x19fc19fa >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM3, 0x19f81a38 */
		pinfo->vesa_dsc.range_min_qp6 = 7;//(0x19f81a38 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp6 = 11;//(0x19f81a38 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset6 = (0x19f81a38 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp7 = 7;//(0x19f81a38 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp7 = 12;//(0x19f81a38 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset7 = (0x19f81a38 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM4, 0x1a781ab6 */
		pinfo->vesa_dsc.range_min_qp8 = 7;//(0x1a781ab6 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp8 = 13;//(0x1a781ab6 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset8 = (0x1a781ab6 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp9 = 7;//(0x1a781ab6 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp9 = 14;//(0x1a781ab6 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset9 = (0x1a781ab6 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM5, 0x2af62b34 */
		pinfo->vesa_dsc.range_min_qp10 = 9;//(0x2af62b34 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp10 = 15;//(0x2af62b34 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset10 = (0x2af62b34 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp11 = 9;//(0x2af62b34 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp11 = 16;//(0x2af62b34 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset11 = (0x2af62b34 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM6, 0x2b743b74 */
		pinfo->vesa_dsc.range_min_qp12 = 9;//(0x2b743b74 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp12 = 17;//(0x2b743b74 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset12 = (0x2b743b74 >> 16) & 0x3F;
		pinfo->vesa_dsc.range_min_qp13 = 11;//(0x2b743b74 >> 11) & 0x1F;
		pinfo->vesa_dsc.range_max_qp13 = 17;//(0x2b743b74 >> 6) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset13 = (0x2b743b74 >> 0) & 0x3F;

		/* DSC_RC_RANGE_PARAM7, 0x6bf40000 */
		pinfo->vesa_dsc.range_min_qp14 = 17;//(0x6bf40000 >> 27) & 0x1F;
		pinfo->vesa_dsc.range_max_qp14 = 19;//(0x6bf40000 >> 22) & 0x1F;
		pinfo->vesa_dsc.range_bpg_offset14 = (0x6bf40000 >> 16) & 0x3F;

	} else {
		pinfo->vesa_dsc.bits_per_component = 8;
		pinfo->vesa_dsc.linebuf_depth = 9;
		pinfo->vesa_dsc.bits_per_pixel = 8;
		pinfo->vesa_dsc.initial_xmit_delay = 512;

		pinfo->vesa_dsc.slice_width = 719;//1439
		pinfo->vesa_dsc.slice_height = 7;//31;

		pinfo->vesa_dsc.first_line_bpg_offset = 12;
		pinfo->vesa_dsc.mux_word_size = 48;

		/* DSC_CTRL */
		pinfo->vesa_dsc.block_pred_enable = 1;//0;

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
	}



	if (g_lcd_fpga_flag == 0) {
		if (pinfo->pxl_clk_rate_div > 1) {
			pinfo->ldi.h_back_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_front_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_pulse_width /= pinfo->pxl_clk_rate_div;
		}
	}
	/* IFBC Setting end */

	if (g_lcd_fpga_flag == 0) {
		// lcd vcc init
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

	return ret;

}
/*lint +e747 +e846 +e516 +e514 +e866 +e30 +e84 +e778 +e845 +e572*/

static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_NT36860,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_panel_match_table);

static struct platform_driver this_driver = {
	.probe = mipi_sharp_NT36860_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "mipi_sharp_NT36860",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init mipi_sharp_NT36860_panel_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(mipi_sharp_NT36860_panel_init);

