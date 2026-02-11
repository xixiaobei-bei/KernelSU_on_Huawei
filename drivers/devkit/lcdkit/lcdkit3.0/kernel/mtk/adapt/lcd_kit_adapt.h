/*
 * lcd_kit_adapt.h
 *
 * lcdkit adapt function head file for lcd driver
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

/* dsc dsi1.2 vesa3x long write */
#define DTYPE_DSC_LWRITE  0x0A

#ifndef __LCD_KIT_ADAPT_H_
#define __LCD_KIT_ADAPT_H_
int lcd_kit_dsi_cmds_tx(void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dsi_cmds_rx(void *hld, uint8_t *out,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_adapt_init(void);
int lcd_kit_dsi_cmds_extern_rx(uint8_t *out,
	struct lcd_kit_dsi_panel_cmds *cmds, unsigned int len);
#endif
