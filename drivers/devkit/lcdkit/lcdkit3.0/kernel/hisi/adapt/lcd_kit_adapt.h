/*
 * lcd_kit_adapt.h
 *
 * lcdkit adapt function for lcd driver head file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

int lcd_kit_dsi_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi_cmds_rx(void *hld, uint8_t *out,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi1_cmds_rx(void *hld, uint8_t *out,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi_cmds_tx_no_lock(void *hld,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_adapt_init(void);
#endif
