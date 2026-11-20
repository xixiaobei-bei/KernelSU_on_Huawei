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

#ifndef __PANEL_DRV_H__
#define __PANEL_DRV_H__

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include "mipi_dsi_dev.h"
#include "dkmd_connector.h"

#undef pr_fmt
#define pr_fmt(fmt)  "panel: " fmt

#define DEV_NAME_PANEL "dsi_panel"
#define DTS_COMP_PANEL "hisilicon,dsi_panel"

#define LCD_BL_TYPE_NAME "lcd-bl-type"
#define FPGA_FLAG_NAME "fpga_flag"
#define LCD_DISPLAY_TYPE_NAME "lcd-display-type"
#define LCD_IFBC_TYPE_NAME "lcd-ifbc-type"
#define LCD_DSI_TYPE_NAME "lcd-dsi-type"
#define LCD_PHY_TYPE_NAME "lcd-phy-type"

extern uint32_t gpio_lcd_vsp_enable;
extern uint32_t gpio_lcd_vsn_enable;
extern uint32_t gpio_lcd_reset;
extern uint32_t gpio_lcd_bl_enable;
extern uint32_t gpio_lcd_tp1v8;

struct panel_match_data {
	int version_id;
	int (*of_device_setup)(struct platform_device *pdev);
	void (*of_device_release)(struct platform_device *pdev);
};

#define panel_device_match_data(name, id, setup, release) \
	struct panel_match_data name = { .version_id = id, .of_device_setup = setup, .of_device_release = release }

struct panel_drv_private {
	struct dkmd_connector_info base;
	struct mipi_panel_info mipi;

	int dsi_idx;
	/* for dual mipi panel config */
	int ext_dsi_idx;
	char __iomem *connector_dsi_base;
	char __iomem *ext_connector_dsi_base;

	struct dsi_cmd_desc *disp_on_cmds;
	uint32_t disp_on_cmds_len;
	struct dsi_cmd_desc *disp_off_cmds;
	uint32_t disp_off_cmds_len;

	struct gpio_desc *gpio_request_cmds;
	uint32_t gpio_request_cmds_len;
	struct gpio_desc *gpio_free_cmds;
	uint32_t gpio_free_cmds_len;

	struct vcc_desc *vcc_enable_cmds;
	uint32_t vcc_enable_cmds_len;
	struct vcc_desc *vcc_disable_cmds;
	uint32_t vcc_disable_cmds_len;

	struct pinctrl_cmd_desc *pinctrl_normal_cmds;
	uint32_t pinctrl_normal_cmds_len;
	struct pinctrl_cmd_desc *pinctrl_lowpower_cmds;
	uint32_t pinctrl_lowpower_cmds_len;

	struct gpio_desc *gpio_normal_cmds;
	uint32_t gpio_normal_cmds_len;
	struct gpio_desc *gpio_lowpower_cmds;
	uint32_t gpio_lowpower_cmds_len;

	struct pinctrl_cmd_desc *pinctrl_init_cmds;
	uint32_t pinctrl_init_cmds_len;
	struct pinctrl_cmd_desc *pinctrl_finit_cmds;
	uint32_t pinctrl_finit_cmds_len;

	struct vcc_desc *vcc_init_cmds;
	uint32_t vcc_init_cmds_len;
	struct vcc_desc *vcc_finit_cmds;
	uint32_t vcc_finit_cmds_len;

	int panel_initial_step;
	int panel_uninitial_step;

	int device_initialized;
	struct platform_device *pdev;
};

static inline struct panel_drv_private *to_panel_drv_private(struct platform_device *pdev)
{
	struct dkmd_connector_info *pinfo = platform_get_drvdata(pdev);

	if (pinfo)
		return container_of(pinfo, struct panel_drv_private, base);
	else
		return NULL;
}

int panel_base_of_device_setup(struct platform_device *pdev);
void panel_base_of_device_release(struct platform_device *pdev);
void panel_drv_data_setup(struct platform_device *pdev);

#define DTS_COMP_PANEL_TD4322 "hisilicon,td4322_panel"
extern struct panel_match_data td4322_panel_info;

#endif