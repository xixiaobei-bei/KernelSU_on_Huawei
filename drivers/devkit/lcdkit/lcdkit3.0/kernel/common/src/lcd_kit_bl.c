/*
 * lcd_kit_bl.c
 *
 * lcdkit backlight driver for lcdkit
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

#include "lcd_kit_bl.h"
#include "lcd_kit_common.h"

static struct lcd_kit_bl_ops *g_bl_ops;
int lcd_kit_bl_register(struct lcd_kit_bl_ops *ops)
{
	if (g_bl_ops) {
		LCD_KIT_ERR("g_bl_ops has already been registered!\n");
		return LCD_KIT_FAIL;
	}
	g_bl_ops = ops;
	LCD_KIT_INFO("g_bl_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_bl_unregister(struct lcd_kit_bl_ops *ops)
{
	if (g_bl_ops == ops) {
		g_bl_ops = NULL;
		LCD_KIT_INFO("g_bl_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_bl_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_bl_ops *lcd_kit_get_bl_ops(void)
{
	return g_bl_ops;
}
