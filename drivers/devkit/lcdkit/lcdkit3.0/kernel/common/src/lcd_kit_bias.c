/*
 * lcd_kit_bias.c
 *
 * lcdkit bias driver for lcdkit
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
#include "lcd_kit_common.h"
#include "lcd_kit_bias.h"

static struct lcd_kit_bias_ops *g_bias_ops;
int lcd_kit_bias_register(struct lcd_kit_bias_ops *ops)
{
	if (g_bias_ops) {
		LCD_KIT_ERR("g_bias_ops has already been registered!\n");
		return LCD_KIT_FAIL;
	}
	g_bias_ops = ops;
	LCD_KIT_INFO("g_bias_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_bias_unregister(struct lcd_kit_bias_ops *ops)
{
	if (g_bias_ops == ops) {
		g_bias_ops = NULL;
		LCD_KIT_INFO("g_bias_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_bias_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_bias_ops *lcd_kit_get_bias_ops(void)
{
	return g_bias_ops;
}

