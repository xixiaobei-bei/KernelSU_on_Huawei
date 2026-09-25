/*
 * power_gpio.h
 *
 * gpio interface for power module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#define __power_gpio_tag(tag)    #tag
#define power_gpio_tag(tag)      __power_gpio_tag(tag)

#define POWER_GPIO_RD_BUF_SIZE   32

enum power_gpio_op_user {
	POWER_GPIO_OP_USER_BEGIN = 0,
	POWER_GPIO_OP_USER_SHELL = POWER_GPIO_OP_USER_BEGIN, /* for shell */
	POWER_GPIO_OP_USER_KERNEL, /* for kernel */
	POWER_GPIO_OP_USER_END,
};

int power_gpio_request(const char *tag, struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio);
int power_gpio_config_input(const char *tag, struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio);
int power_gpio_config_output(const char *tag, struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio, int value);
int power_gpio_config_interrupt(const char *tag, struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio, int *irq);

#endif /* _POWER_GPIO_H_ */
