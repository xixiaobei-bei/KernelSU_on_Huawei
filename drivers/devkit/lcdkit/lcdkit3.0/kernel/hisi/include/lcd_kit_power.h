/*
 * lcd_kit_power.h
 *
 * lcdkit power function for lcd driver head file
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

#ifndef __LCD_KIT_POWER_H_
#define __LCD_KIT_POWER_H_
#include "lcd_kit_utils.h"

/*
 * macro
 */
/* backlight */
#define BACKLIGHT_NAME "backlight"
/* bias */
#define BIAS_NAME "bias"
#define VSN_NAME  "vsn"
#define VSP_NAME  "vsp"
/* vci */
#define VCI_NAME  "vci"
/* iovcc */
#define IOVCC_NAME "iovcc"
/* vdd */
#define VDD_NAME  "vdd"
/* gpio */
#define GPIO_NAME "gpio"

enum gpio_operator {
	GPIO_REQ,
	GPIO_FREE,
	GPIO_HIGH,
	GPIO_LOW,
};

/*
 * struct
 */
struct gpio_power_arra {
	enum gpio_operator oper;
	uint32_t num;
	struct gpio_desc *cm;
};

struct event_callback {
	uint32_t event;
	int (*func)(void *data);
};

/* variable declare */
extern uint32_t g_lcd_kit_gpio;
extern uint32_t poweric_gpio;

/* function declare */
int lcd_kit_pmu_ctrl(uint32_t type, uint32_t enable);
int lcd_kit_pmu_ctrl_plugin(uint32_t type, uint32_t enable);
int lcd_kit_charger_ctrl(uint32_t type, uint32_t enable);
int lcd_kit_gpio_tx(uint32_t type, uint32_t op);
int lcd_kit_gpio_tx_plugin(uint32_t type, uint32_t op);
int lcd_kit_power_finit(struct platform_device *pdev);
int lcd_kit_power_init(struct platform_device *pdev);
int lcd_kit_dbg_set_voltage(void);
int lcd_power_set_vol(uint32_t type);
void lcd_poweric_gpio_operator(uint32_t gpio, uint32_t op);

#ifdef LCD_KIT_DEBUG_ENABLE
int dpd_regu_init(struct platform_device *pdev);
#endif
#endif
