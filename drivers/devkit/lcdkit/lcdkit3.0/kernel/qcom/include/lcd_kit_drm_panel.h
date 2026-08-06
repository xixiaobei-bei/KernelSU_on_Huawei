/*
 * lcd_kit_drm_panel.h
 *
 * lcdkit display function head file for lcd driver
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

#ifndef __LCD_KIT_DRM_PANEL_H_
#define __LCD_KIT_DRM_PANEL_H_
#include <linux/backlight.h>
#include <drm/drmP.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>
#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include "lcd_kit_common.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_core.h"
#include "lcd_kit_utils.h"
#if defined CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

/* macro definition */
#define DTS_COMP_LCD_KIT_PANEL_TYPE "huawei,lcd_panel_type"
#define LCD_KIT_PANEL_COMP_LENGTH 128
#define BACKLIGHT_HIGH_LEVEL 1
#define BACKLIGHT_LOW_LEVEL  2

struct lcd_kit_disp_info *lcd_kit_get_disp_info(void);
#define disp_info	lcd_kit_get_disp_info()
unsigned int lcm_get_panel_state(void);

/* enum */
enum alpm_mode {
	ALPM_DISPLAY_OFF,
	ALPM_ON_MIDDLE_LIGHT,
	ALPM_EXIT,
	ALPM_ON_LOW_LIGHT,
};

struct lcd_kit_disp_info {
	/* effect */
	/* gamma calibration */
	struct lcd_kit_gamma gamma_cal;
	/* oem information */
	struct lcd_kit_oem_info oeminfo;
	/* rgbw function */
	struct lcd_kit_rgbw rgbw;
	/* end */
	/* normal */
	/* lcd type */
	u32 lcd_type;
	/* panel information */
	char *compatible;
	/* product id */
	u32 product_id;
	/* vr support */
	u32 vr_support;
	/* lcd kit semaphore */
	struct semaphore lcd_kit_sem;
	/* lcd kit mipi mutex lock */
	struct mutex mipi_lock;
	/* alpm -aod */
	struct lcd_kit_alpm alpm;
	/* quickly sleep out */
	struct lcd_kit_quickly_sleep_out quickly_sleep_out;
	/* fps ctrl */
	struct lcd_kit_fps fps;
	/* project id */
	struct lcd_kit_project_id project_id;
	/* thp_second_poweroff_sem */
	struct semaphore thp_second_poweroff_sem;
	struct display_engine_ddic_rgbw_param ddic_rgbw_param;
	/* end */
	/* normal */
	u8 bl_is_shield_backlight;
	u8 bl_is_start_timer;
};

struct qcom_panel_info *lcm_get_panel_info(void);
int lcd_kit_init(struct dsi_panel *panel);
void lcd_kit_power_init(struct dsi_panel *panel);
int lcd_kit_sysfs_init(void);
unsigned int lcm_get_panel_backlight_max_level(void);
#if defined CONFIG_HUAWEI_DSM
struct dsm_client *lcd_kit_get_lcd_dsm_client(void);
#endif
int is_mipi_cmd_panel(void);
int lcd_kit_rgbw_set_handle(void);
void lcd_kit_disp_on_check_delay(void);
int lcm_rgbw_mode_set_param(struct drm_device *dev, void *data,
	struct drm_file *file_priv);
int lcm_rgbw_mode_get_param(struct drm_device *dev, void *data,
	struct drm_file *file_priv);
int lcm_display_engine_get_panel_info(struct drm_device *dev, void *data,
	struct drm_file *file_priv);
int lcm_display_engine_init(struct drm_device *dev, void *data,
	struct drm_file *file_priv);
int panel_drm_hbm_set(struct drm_device *dev, void *data,
	struct drm_file *file_priv);
int lcd_kit_panel_pre_prepare(struct dsi_panel *panel);
int lcd_kit_panel_prepare(struct dsi_panel *panel);
int lcd_kit_panel_enable(struct dsi_panel *panel);
int lcd_kit_panel_pre_disable(struct dsi_panel *panel);
int lcd_kit_panel_disable(struct dsi_panel *panel);
int lcd_kit_panel_unprepare(struct dsi_panel *panel);
int lcd_kit_panel_post_unprepare(struct dsi_panel *panel);
int lcd_kit_bl_ic_set_backlight(unsigned int bl_level);
int lcd_kit_dsi_panel_update_backlight(struct dsi_panel *panel,
	unsigned int bl_lvl);
#endif
