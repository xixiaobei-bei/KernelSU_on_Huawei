/*
 * lcd_kit_panel.c
 *
 * lcdkit panel function for lcd driver
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

#include "lcd_kit_disp.h"
#include "lcd_kit_panel.h"
#include "panel/090_703_6p53.c"

static struct lcd_kit_panel_map panel_map[] = {
	{ PRODUCT_JM, PANEL_090_703, panel_090_703_6p53_probe },
	{ PRODUCT_JSCH, PANEL_090_703, panel_090_703_6p53_probe },
};

struct lcd_kit_panel_ops *g_lcd_kit_panel_ops = NULL;
int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops *ops)
{
	if (!ops) {
		LCD_KIT_ERR("ops is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (!g_lcd_kit_panel_ops) {
		g_lcd_kit_panel_ops = ops;
		LCD_KIT_INFO("ops register success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops have been registered!\n");
	return LCD_KIT_FAIL;
}
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops *ops)
{
	if (!ops) {
		LCD_KIT_ERR("ops is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (g_lcd_kit_panel_ops == ops) {
		g_lcd_kit_panel_ops = NULL;
		LCD_KIT_INFO("ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void)
{
	return g_lcd_kit_panel_ops;
}

int lcd_kit_panel_init(void)
{
	int ret;
	int i;

	if (!disp_info->compatible) {
		LCD_KIT_ERR("compatible is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < ARRAY_SIZE(panel_map); i++) {
		if (disp_info->product_id == panel_map[i].product_id) {
			if (strncmp(disp_info->compatible,
				panel_map[i].compatible,
				strlen(disp_info->compatible)))
				continue;
			ret = panel_map[i].callback();
			if (ret) {
				LCD_KIT_ERR("ops init fail\n");
				return LCD_KIT_FAIL;
			}
			break;
		}
	}
	/* init ok */
	if (i >= ARRAY_SIZE(panel_map))
		LCD_KIT_INFO("not find adapter ops\n");
	return LCD_KIT_OK;
}
