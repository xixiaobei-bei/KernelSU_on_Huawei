/*
 * lcd_kit_adapt.h
 *
 * lcdkit adapt function for lcd driver head file
 *
 * Copyright (c) 2018-2021 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_ADAPT_H_
#define __LCD_KIT_ADAPT_H_
#include <linux/types.h>
#include "lcd_kit_common.h"

char __iomem *lcd_kit_get_dsi_base_addr(struct hisi_fb_data_type *hisifd);
int lcd_kit_dsi_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi_cmds_rx(void *hld, uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi1_cmds_rx(void *hld, uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi_cmds_tx_no_lock(void *hld,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_adapt_init(void);
int lcd_kit_dsi_diff_cmds_tx(void *hld,
	struct lcd_kit_dsi_panel_cmds *dsi0_cmds,
	struct lcd_kit_dsi_panel_cmds *dsi1_cmds);
bool lcd_kit_is_current_frame_ok_to_set_backlight(
	struct hisi_fb_data_type *hisifd);
bool lcd_kit_is_current_frame_ok_to_set_fp_backlight(
	struct hisi_fb_data_type *hisifd, int backlight_type);
void lcd_kit_display_effect_screen_on(struct hisi_fb_data_type *hisifd);
int lcd_kit_set_panel_irc(struct hisi_fb_data_type *hisifd, bool enable);
void lcd_kit_swap_fps_gamma_if_needed(struct hisi_fb_data_type *hisifd);
void lcd_kit_optimize_grayscale(struct hisi_fb_data_type *hisifd);
void lcd_kit_display_effect_screen_on_default(struct hisi_fb_data_type *hisifd);
int lcd_kit_dual_dsi_cmds_rx(void *hld, uint8_t *dsi0_out,
	uint8_t *dsi1_out, int out_len, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi0_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi1_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
#endif
