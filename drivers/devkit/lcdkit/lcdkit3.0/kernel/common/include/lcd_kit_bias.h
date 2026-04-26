/*
 * lcd_kit_bias.h
 *
 * lcdkit bias driver for lcdkit head file
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

#ifndef _LCD_KIT_BIAS_H_
#define _LCD_KIT_BIAS_H_

struct lcd_kit_bias_ops {
	int (*set_bias_voltage)(int vpos, int vneg);
	int (*set_bias_power_down)(int vpos, int vneg);
	int (*dbg_set_bias_voltage)(int vpos, int vneg);
	int (*set_ic_disable)(void);
	int (*set_vtc_bias_voltage)(int vpos, int vneg, bool state);
	bool (*set_bias_is_need_disable)(void);
};

/* function declare */
struct lcd_kit_bias_ops *lcd_kit_get_bias_ops(void);
int lcd_kit_bias_register(struct lcd_kit_bias_ops *ops);
int lcd_kit_bias_unregister(struct lcd_kit_bias_ops *ops);
#endif
