/*
 * lcd_kit_panel.h
 *
 * lcdkit panel head file for lcd driver
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

#ifndef _LCD_KIT_PANEL_H_
#define _LCD_KIT_PANEL_H_
/* product id */
#define PRODUCT_ALPS	1002

/* panel compatible */
#define PANEL_JDI_NT36860C "jdi_2lane_nt36860_5p88_1440P_cmd"

/* struct */
struct lcd_kit_panel_ops {
	int (*lcd_kit_read_project_id)(void);
	int (*lcd_get_2d_barcode)(char *oem_data);
};

struct lcd_kit_panel_map {
	u32 product_id;
	char *compatible;
	int (*callback)(void);
};

/* function declare */
struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void);
int lcd_kit_panel_init(void);
int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops *ops);
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops *ops);
#endif
