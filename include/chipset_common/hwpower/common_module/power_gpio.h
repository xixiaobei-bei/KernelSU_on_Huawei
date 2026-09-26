/*
 * power_gpio.h
 *
 * gpio interface for power module
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

#ifndef _POWER_GPIO_H_
#define _POWER_GPIO_H_

#include <linux/of.h>
#include <linux/of_gpio.h>

#define POWER_GPIO_RD_BUF_SIZE   32

enum power_gpio_op_user {
	POWER_GPIO_OP_USER_BEGIN = 0,
	POWER_GPIO_OP_USER_SHELL = POWER_GPIO_OP_USER_BEGIN, /* for shell */
	POWER_GPIO_OP_USER_KERNEL, /* for kernel */
	POWER_GPIO_OP_USER_END,
};

int power_gpio_request(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio);
int power_gpio_config_input(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio);
int power_gpio_config_output(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio, int value);
int power_gpio_config_interrupt(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio, int *irq);
int power_gpio_config_output_compatible(const char *comp,
	const char *prop, const char *label, unsigned int *gpio, int value);

#endif /* _POWER_GPIO_H_ */
