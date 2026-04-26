/*
 * lcd_kit_power.h
 *
 * lcdkit power function head file for lcd driver
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

#ifndef __LCD_KIT_POWER_H_
#define __LCD_KIT_POWER_H_
#include <linux/kernel.h>
#include <linux/ctype.h>

/* macro */
/* gpio */
#define GPIO_NAME "gpio"

#define LDO_ENABLE 1
#define LDO_DISABLE 0

enum {
	WAIT_TYPE_US = 0,
	WAIT_TYPE_MS,
};

/* dtype for gpio */
enum {
	DTYPE_GPIO_REQUEST,
	DTYPE_GPIO_FREE,
	DTYPE_GPIO_INPUT,
	DTYPE_GPIO_OUTPUT,
	DTYPE_GPIO_PMX,
	DTYPE_GPIO_PULL,
};

enum gpio_operator {
	GPIO_REQ,
	GPIO_FREE,
	GPIO_HIGH,
	GPIO_LOW,
};

/* gpio desc */
struct gpio_desc {
	int dtype;
	int waittype;
	int wait;
	char *label;
	uint32_t *gpio;
	int value;
};

/* struct */
struct gpio_power_arra {
	enum gpio_operator oper;
	uint32_t num;
	struct gpio_desc *cm;
};

/* function declare */
void lcd_kit_gpio_tx(uint32_t type, uint32_t op);
#endif
