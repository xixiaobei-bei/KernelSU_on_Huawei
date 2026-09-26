/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <dpu/soc_dpu_define.h>

#include "dkmd_utils.h"
#include "panel_dev.h"
#include "panel_drv.h"

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME		"lcdio-vcc"
#define VCC_LCDANALOG_NAME	"lcdanalog-vcc"

struct regulator *vcc_lcdio;
struct regulator *vcc_lcdanalog;

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
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_DISABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};

/*******************************************************************************
** LCD IOMUX
*/
static struct pinctrl_data panel_pctrl;

static struct pinctrl_cmd_desc lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &panel_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &panel_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &panel_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &panel_pctrl, 0},
};

static struct pinctrl_cmd_desc lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &panel_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &panel_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_P5V5_ENABLE_NAME	"gpio_lcd_vsp_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME "gpio_lcd_vsn_enable"
#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_BL_ENABLE_NAME	"gpio_lcd_bl_enable"
#define GPIO_LCD_TP1V8_NAME	"gpio_lcd_tp1v8"

static struct gpio_desc lcd_gpio_request_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* TP_1.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_tp1v8, 0},
};

static struct gpio_desc lcd_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_tp1v8, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_tp1v8, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 10,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 50,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc lcd_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_tp1v8, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},

	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_tp1v8, 1},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc lcd_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_TP1V8_NAME, &gpio_lcd_tp1v8, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static int panel_dev_power_on(struct platform_device *pdev)
{
	int ret = 0;
	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_info("get panel drv private err!\n");
		return -EINVAL;
	}

	if (priv->vcc_enable_cmds && (priv->vcc_enable_cmds_len > 0)) {
		ret = vcc_cmds_tx(pdev, priv->vcc_enable_cmds, priv->vcc_enable_cmds_len);
		if (ret) {
			pr_info("vcc cmds handle err!\n");
			return ret;
		}
	}

	if (priv->pinctrl_normal_cmds && (priv->pinctrl_normal_cmds_len > 0)) {
		ret = pinctrl_cmds_tx(pdev, priv->pinctrl_normal_cmds, priv->pinctrl_normal_cmds_len);
		if (ret) {
			pr_info("pinctrl cmds handle err!\n");
			return ret;
		}
	}

	if (priv->gpio_normal_cmds && (priv->gpio_normal_cmds_len > 0)) {
		ret = gpio_cmds_tx(priv->gpio_normal_cmds, priv->gpio_normal_cmds_len);
		if (ret) {
			pr_info("gpio cmds handle err!\n");
			return ret;
		}
	}

	return 0;
}

static int panel_dev_initial_code(struct panel_drv_private *priv)
{
	int ret = 0;

	if (priv->disp_on_cmds && (priv->disp_on_cmds_len > 0)) {
		ret = mipi_dsi_cmds_tx(priv->disp_on_cmds, priv->disp_on_cmds_len, priv->connector_dsi_base);
		pr_info("disp on cmds handle %d!\n", ret);
	}

	return 0;
}

static int read_lcd_power_status(char __iomem *mipi_dsi_base)
{
	uint32_t status = 0;
	uint32_t try_times = 0;

	outp32(DPU_DSI_APB_WR_LP_HDR_ADDR(mipi_dsi_base), 0x0A06);
	status = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(mipi_dsi_base));
	/* wait for 5ms until reading status timeout */
	while (status & 0x10) {
		udelay(5);
		if (++try_times > 100) {
			try_times = 0;
			pr_info("Read lcd power status timeout!\n");
			break;
		}
		outp32(DPU_DSI_APB_WR_LP_HDR_ADDR(mipi_dsi_base), 0x0A06);
		status = inp32(DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(mipi_dsi_base));
	}
	status = inp32(DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(mipi_dsi_base));
	pr_info("TD4322 LCD Power State = %#x!\n", status);
	return status;
}

static int mipi_lcd_panel_on(struct platform_device *pdev)
{
	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_info("get panel drv private err!\n");
		return -EINVAL;
	}
	pr_info("panel_initial_step:%d enter", priv->panel_initial_step);

	if (priv->panel_initial_step == LCD_INIT_POWER_ON) {
		priv->panel_initial_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
		return panel_dev_power_on(pdev);
	}

	if (priv->panel_initial_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		priv->panel_initial_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
		return panel_dev_initial_code(priv);
	}

	if (priv->panel_initial_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE)
		read_lcd_power_status(priv->connector_dsi_base);

	pr_info("panel_initial_step:%d exit", priv->panel_initial_step);

	return 0;
}

static int panel_dev_power_off(struct platform_device *pdev)
{
	int ret = 0;
	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_info("get panel drv private err!\n");
		return -EINVAL;
	}

	if (priv->vcc_disable_cmds && (priv->vcc_disable_cmds_len > 0)) {
		ret = vcc_cmds_tx(pdev, priv->vcc_disable_cmds, priv->vcc_disable_cmds_len);
		if (ret) {
			pr_info("vcc cmds handle err!\n");
			return ret;
		}
	}

	if (priv->pinctrl_lowpower_cmds && (priv->pinctrl_lowpower_cmds_len > 0)) {
		ret = pinctrl_cmds_tx(pdev, priv->pinctrl_lowpower_cmds, priv->pinctrl_lowpower_cmds_len);
		if (ret) {
			pr_info("pinctrl handle err!\n");
			return ret;
		}
	}

	if (priv->gpio_lowpower_cmds && (priv->gpio_lowpower_cmds_len > 0)) {
		ret = gpio_cmds_tx(priv->gpio_lowpower_cmds, priv->gpio_lowpower_cmds_len);
		if (ret) {
			pr_info("gpio lowpower handle err!\n");
			return ret;
		}
	}

	return 0;
}

static int panel_dev_uninitial_code(struct panel_drv_private *priv)
{
	int ret = 0;

	if (priv->disp_off_cmds && (priv->disp_off_cmds_len > 0)) {
		ret = mipi_dsi_cmds_tx(priv->disp_off_cmds, priv->disp_off_cmds_len, priv->connector_dsi_base);
		pr_info("display off cmds handle ret=%d!\n", ret);
	}

	return 0;
}

static int mipi_lcd_panel_off(struct platform_device *pdev)
{
	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_info("get panel drv private err!\n");
		return -EINVAL;
	}
	pr_info("panel_uninitial_step:%d enter", priv->panel_uninitial_step);

	if (priv->panel_uninitial_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE) {
		priv->panel_uninitial_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
		return panel_dev_uninitial_code(priv);
	}

	if (priv->panel_uninitial_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		priv->panel_uninitial_step = LCD_UNINIT_POWER_OFF;
		;
	}

	if (priv->panel_uninitial_step == LCD_UNINIT_POWER_OFF)
		return panel_dev_power_off(pdev);

	pr_info("panel_uninitial_step:%d exit", priv->panel_uninitial_step);

	return 0;
}

static int mipi_lcd_set_backlight(struct platform_device *pdev, void *value)
{
	static char last_bl_level = 255;

	char bl_level_adjust[2] = {
		0x51,
		0x00,
	};
	struct dsi_cmd_desc lcd_bl_level_adjust[] = {
		{DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
			sizeof(bl_level_adjust), bl_level_adjust},
	};

	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_info("get panel drv private err!\n");
		return -EINVAL;
	}

	bl_level_adjust[1] = last_bl_level * 255 / 255;
	mipi_dsi_cmds_tx(lcd_bl_level_adjust, ARRAY_SIZE(lcd_bl_level_adjust), priv->connector_dsi_base);

	pr_info("bl_level_adjust[1] = %d\n", bl_level_adjust[1]);

	return 0;
}

static int mipi_lcd_set_display_region(struct platform_device *pdev, void *value)
{
	struct panel_drv_private *priv = to_panel_drv_private(pdev);
	struct disp_rect *dirty = (struct disp_rect *)value;

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

	if (!priv) {
		pr_info("get panel drv private err!\n");
		return -EINVAL;
	}

	pr_info("dirty_region : %d,%d, %d,%d not support!\n", dirty->x, dirty->y, dirty->w, dirty->h);

	lcd_disp_x[1] = ((uint32_t)(dirty->x) >> 8) & 0xff;
	lcd_disp_x[2] = ((uint32_t)(dirty->x)) & 0xff;
	lcd_disp_x[3] = ((uint32_t)(dirty->x + dirty->w - 1) >> 8) & 0xff;
	lcd_disp_x[4] = ((uint32_t)(dirty->x + dirty->w - 1)) & 0xff;
	lcd_disp_y[1] = ((uint32_t)(dirty->y) >> 8) & 0xff;
	lcd_disp_y[2] = ((uint32_t)(dirty->y)) & 0xff;
	lcd_disp_y[3] = ((uint32_t)(dirty->y + dirty->h - 1) >> 8) & 0xff;
	lcd_disp_y[4] = ((uint32_t)(dirty->y + dirty->h - 1)) & 0xff;

	mipi_dsi_cmds_tx(set_display_address, ARRAY_SIZE(set_display_address), priv->connector_dsi_base);

	return 0;
}

struct panel_ops_handle_data panel_ops_table[] = {
	{"set_backlight", mipi_lcd_set_backlight},
	{"set_display_region", mipi_lcd_set_display_region},
};

static int mipi_lcd_panel_ops_handle(struct platform_device *pdev, char *ops_cmd, void *value)
{
	int i = 0;
	struct panel_ops_handle_data *ops_handle = NULL;
	struct panel_dev_private_data *pdata = NULL;

	pdata = to_panel_dev_private_data(pdev);
	for (i = 0; i < ARRAY_SIZE(panel_ops_table); i++) {
		ops_handle = &(panel_ops_table[i]);
		if (!strcmp(ops_cmd, ops_handle->ops_cmd) && ops_handle->handle_func) {
			return ops_handle->handle_func(pdev, value);
		}
	}

	return 0;
}

struct panel_dev_private_data panel_dev_data = {
	.base = {
		.on_func = mipi_lcd_panel_on,
		.off_func = mipi_lcd_panel_off,
		.ops_handle_func = mipi_lcd_panel_ops_handle,
		.next = NULL,
	},
};

void panel_drv_data_setup(struct platform_device *pdev)
{
	struct panel_drv_private *priv = to_panel_drv_private(pdev);

	if (!priv) {
		pr_err("get panel private data failed!\n");
		return;
	}

	priv->gpio_request_cmds = lcd_gpio_request_cmds;
	priv->gpio_request_cmds_len = ARRAY_SIZE(lcd_gpio_request_cmds);
	priv->gpio_free_cmds = lcd_gpio_free_cmds;
	priv->gpio_free_cmds_len = ARRAY_SIZE(lcd_gpio_free_cmds);

	priv->vcc_enable_cmds = lcd_vcc_enable_cmds;
	priv->vcc_enable_cmds_len = ARRAY_SIZE(lcd_vcc_enable_cmds);
	priv->vcc_disable_cmds = lcd_vcc_disable_cmds;
	priv->vcc_disable_cmds_len = ARRAY_SIZE(lcd_vcc_disable_cmds);

	priv->pinctrl_normal_cmds = lcd_pinctrl_normal_cmds;
	priv->pinctrl_normal_cmds_len = ARRAY_SIZE(lcd_pinctrl_normal_cmds);
	priv->pinctrl_lowpower_cmds = lcd_pinctrl_lowpower_cmds;
	priv->pinctrl_lowpower_cmds_len = ARRAY_SIZE(lcd_pinctrl_lowpower_cmds);

	priv->gpio_normal_cmds = lcd_gpio_normal_cmds;
	priv->gpio_normal_cmds_len = ARRAY_SIZE(lcd_gpio_normal_cmds);
	priv->gpio_lowpower_cmds = lcd_gpio_lowpower_cmds;
	priv->gpio_lowpower_cmds_len = ARRAY_SIZE(lcd_gpio_lowpower_cmds);

	priv->pinctrl_init_cmds = lcd_pinctrl_init_cmds;
	priv->pinctrl_init_cmds_len = ARRAY_SIZE(lcd_pinctrl_init_cmds);
	priv->pinctrl_finit_cmds = lcd_pinctrl_finit_cmds;
	priv->pinctrl_finit_cmds_len = ARRAY_SIZE(lcd_pinctrl_finit_cmds);

	priv->vcc_init_cmds = lcd_vcc_init_cmds;
	priv->vcc_init_cmds_len = ARRAY_SIZE(lcd_vcc_init_cmds);
	priv->vcc_finit_cmds = lcd_vcc_finit_cmds;
	priv->vcc_finit_cmds_len = ARRAY_SIZE(lcd_vcc_finit_cmds);

}

MODULE_LICENSE("GPL");