/*
 * lcd_kit_ext_disp.h
 *
 * lcdkit display function for lcd driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_EXT_DISP_H_
#define __LCD_KIT_EXT_DISP_H_
#include <linux/ctype.h>
#include "lcd_kit_common.h"
#include "hisi_fb.h"

#define LCD_FOLDER_STR 10
#define LCD_MAX_PANEL_NUM 2

enum lcd_actvie_panel_id {
	LCD_MAIN_PANEL,
	LCD_EXT_PANEL,
	LCD_ACTIVE_PANEL_BUTT,
};

enum lcd_product_type {
	LCD_NORMAL_TYPE,
	LCD_FOLDER_TYPE,
	LCD_MULTI_PANEL_BUTT,
};

struct lcd_public_config {
	unsigned int product_type;
	unsigned int active_panel_id;
	unsigned int cmdline_panel_id;
	unsigned int dev_id;
	bool lcd_probe_defer;
};

struct lcd_pwr_off_optimize_config {
	void *hisifd;
	bool working_flag;
	unsigned int delay_time;
	unsigned int panel_power_off_seq_cnt;
	struct lcd_kit_array_data *panel_power_off_event;
	struct lcd_kit_array_data *lcd_vci;
	struct lcd_kit_array_data *lcd_iovcc;
	struct lcd_kit_array_data *lcd_rst;
	struct lcd_kit_array_data *lcd_vdd;
	struct delayed_work pwr_off_optimize_work;
};

extern struct lcd_public_config g_lcd_public_cfg;
extern struct lcd_pwr_off_optimize_config g_pwr_off_optimize_cfg;
#define LCD_ACTIVE_PANEL \
	((g_lcd_public_cfg.active_panel_id >= LCD_ACTIVE_PANEL_BUTT) ? \
		0 : g_lcd_public_cfg.active_panel_id)

struct lcd_public_config *lcd_kit_get_public_config(void);
#define PUBLIC_CFG lcd_kit_get_public_config()
void factory_init(struct hisi_panel_info *pinfo);
bool lcd_kit_probe_defer(uint32_t panel_id);
bool lcd_kit_need_add_device(uint32_t panel_id);
void lcd_kit_panel_switch(uint32_t panel_id);
int lcd_kit_panel_switch_func(struct platform_device *pdev, uint32_t panel_id);
void lcd_kit_set_probe_defer(uint32_t panel_id);
void lcd_kit_ext_panel_init(void);
int lcd_kit_get_product_type(void);
struct hisi_panel_info *lcd_kit_get_pinfo(uint32_t panel_id);
struct hisi_fb_panel_data *lcd_kit_pdata(void);
struct hisi_panel_info *lcd_kit_get_fold_panel_info(
	struct platform_device *pdev, uint32_t panel_id);
void lcd_kit_power_off_optimize_handle(struct hisi_fb_data_type *hisifd);
int lcd_kit_event_handler_plugin(void *hld, uint32_t event, uint32_t data,
	uint32_t delay);
void lcd_kit_fold_aod_power_on_handle(struct hisi_fb_data_type *hisifd);
int lcd_kit_get_panel_power_on_status(struct hisi_fb_data_type *hisifd);
void lcd_kit_clear_sctrl_reg(struct hisi_fb_data_type *hisifd);
uint32_t hisi_aod_get_panel_id(void);
void lcd_kit_register_power_key_notify(void);
#endif
