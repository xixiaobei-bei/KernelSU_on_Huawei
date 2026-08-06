/**
 * @file dkmd_connector.h
 * @brief The ioctl the interface file for cmdlist node.
 *
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

#ifndef __DKMD_CONNECTOR_H__
#define __DKMD_CONNECTOR_H__

#include <linux/types.h>
#include <linux/platform_device.h>

enum panel_fps_value {
	FPS_60HZ = 60,
	FPS_HIGH_60HZ = FPS_60HZ + 1, /* the real fps of hight60 mode is also 60 */
	FPS_90HZ = 90,
	FPS_120HZ = 120,
};

enum mipi_dsi_phy_mode {
	DPHY_MODE = 0,
	CPHY_MODE,
};

enum DSI_INDEX {
	DSI0_INDEX = BIT(0),
	DSI1_INDEX = BIT(1),
};

/* panel type list */
enum panel_type {
	PANEL_NO   =  BIT(0),      /* No Panel */
	PANEL_LCDC =  BIT(1),      /* internal LCDC type */
	PANEL_HDMI =  BIT(2),      /* HDMI TV */
	PANEL_MIPI_VIDEO = BIT(3), /* MIPI */
	PANEL_MIPI_CMD   = BIT(4), /* MIPI */
	PANEL_DUAL_MIPI_VIDEO  = BIT(5),  /* DUAL MIPI */
	PANEL_DUAL_MIPI_CMD    = BIT(6),  /* DUAL MIPI */
	PANEL_DP               = BIT(7),  /* DisplayPort */
	PANEL_MIPI2RGB         = BIT(8),  /* MIPI to RGB */
	PANEL_RGB2MIPI         = BIT(9),  /* RGB to MIPI */
	PANEL_OFFLINECOMPOSER  = BIT(10), /* offline composer */
	PANEL_WRITEBACK        = BIT(11), /* Wifi display */
};

/**
 * @brief if the left_align is 8,right_align is 8,and w_min is larger than 802,then w_min should be set to 808,
 * make sure that it is 8 align,if w_min is set to 802,there will be an error.left_align,right_align,top_align
 * bottom_align,w_align,h_align,w_min and h_min's valid value should be larger than 0,top_start and bottom_start
 * maybe equal to 0. if it's not surpport partial update, these value should set to invalid value(-1).
 *
 */
typedef struct lcd_dirty_region_info {
	int left_align;
	int right_align;
	int top_align;
	int bottom_align;

	int w_align;
	int h_align;
	int w_min;
	int h_min;

	int top_start;
	int bottom_start;
	int spr_overlap;
	int reserved;
} lcd_dirty_region_info_t;

/* this structure do not contain pointers */
struct dkmd_connector_info {
	uint32_t scene_id;
	uint32_t type;
	uint32_t product_type;

	uint32_t xres;
	uint32_t yres;
	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	uint32_t orientation;
	uint32_t bgr_fmt;

	uint32_t bl_set_type;
	uint32_t bl_min;
	uint32_t bl_max;
	uint32_t bl_default;
	uint32_t vsync_ctrl_type;
	uint32_t ifbc_type;
	uint32_t bl_type;
	uint32_t dsi_type;

	uint32_t fps;
	uint32_t fpga_flag;

	uint8_t dirty_region_updt_support;
	struct lcd_dirty_region_info dirty_region_info;
};

struct dkmd_connector_data {

	int (*on_func)(struct platform_device *pdev);
	int (*off_func)(struct platform_device *pdev);
	int (*ops_handle_func)(struct platform_device *pdev, char *ops_cmd, void *value);

	struct platform_device *next;
};

static __attribute__((unused)) bool is_mipi_dsi0_panel(const struct dkmd_connector_info *pinfo)
{
	return pinfo->dsi_type & DSI0_INDEX;
}

static __attribute__((unused)) bool is_mipi_dsi1_panel(const struct dkmd_connector_info *pinfo)
{
	return pinfo->dsi_type & DSI1_INDEX;
}

static __attribute__((unused)) bool is_mipi_video_panel(const struct dkmd_connector_info *pinfo)
{
	return pinfo->type & (PANEL_MIPI_VIDEO | PANEL_DUAL_MIPI_VIDEO);
}

static __attribute__((unused)) bool is_mipi_cmd_panel(const struct dkmd_connector_info *pinfo)
{
	return pinfo->type & (PANEL_MIPI_CMD | PANEL_DUAL_MIPI_CMD);
}

static __attribute__((unused)) bool is_dual_mipi_panel(const struct dkmd_connector_info *pinfo)
{
	return pinfo->type & (PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD);
}

// TODO: add dp config
static __attribute__((unused)) bool is_dp_panel(const struct dkmd_connector_info *pinfo)
{
	return false;
}

static __attribute__((unused)) int pipeline_next_on(struct platform_device *pdev)
{
	int ret = 0;
	struct dkmd_connector_data *pdata = NULL;
	struct dkmd_connector_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		pr_err("pdata is NULL!\n");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->on_func))
			ret = next_pdata->on_func(next_pdev);
	}

	return ret;
}

static __attribute__((unused)) int pipeline_next_off(struct platform_device *pdev)
{
	int ret = 0;
	struct dkmd_connector_data *pdata = NULL;
	struct dkmd_connector_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		pr_err("pdata is NULL!\n");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->off_func))
			ret = next_pdata->off_func(next_pdev);
	}

	return ret;
}

static __attribute__((unused)) int pipeline_next_ops_handle(struct platform_device *pdev, char *ops_cmd, void *value)
{
	int ret = 0;
	struct dkmd_connector_data *pdata = NULL;
	struct dkmd_connector_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (!pdev) {
		pr_err("pdev is NULL!\n");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		pr_err("pdata is NULL!\n");
		return -EINVAL;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->ops_handle_func))
			ret = next_pdata->ops_handle_func(next_pdev, ops_cmd, value);
	}

	return ret;
}

void comp_mgr_register_device(struct platform_device *pdev);
void comp_mgr_unregister_device(struct platform_device *pdev);

#endif
