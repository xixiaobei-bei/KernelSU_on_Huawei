/*
 * max77813.h
 *
 * max77813 header
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

#ifndef _MAX77813_H_
#define _MAX77813_H_

#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

/* register info */
#define MAX77813_INFO_REG00         0x00

/* register status */
#define MAX77813_STATUS_REG01       0x01
#define MAX77813_FORCED_PWM_SHIFT   0
#define MAX77813_FORCED_PWM_MASK    BIT(0)

/* register config */
#define MAX77813_CONFIG1            0x02

struct max77813_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	int gpio_pok;
	int irq_pok;
};

#endif /* _MAX77813_H_ */
