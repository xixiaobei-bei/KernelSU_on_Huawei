/*
 * rt8555.h
 *
 * rt8555 driver for backlight
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#ifndef __LINUX_RT8555_H
#define __LINUX_RT8555_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

#define RT8555_NAME             "rt8555"
#define DTS_COMP_RT8555         "realtek,rt8555"

#define GPIO_DIR_OUT                        1
#define GPIO_OUT_ONE                        1
#define GPIO_OUT_ZERO                       0

#define RT8555_RW_REG_MAX    11

/* rt8555 reg address */
#define RT8555_CONTROL_MODE_ADDR           0x00
#define RT8555_CURRENT_PROTECTION_ADDR     0x01
#define RT8555_CURRENT_SETTING_ADDR        0x02
#define RT8555_VOLTAGE_SETTING_ADDR        0x03
#define RT8555_BRIGHTNESS_SETTING_ADDR     0x08
#define RT8555_TIME_CONTROL_ADDR           0x09
#define RT8555_MODE_DEVISION_ADDR          0x0A
#define RT8555_COMPENSATION_DUTY_ADDR      0x0B
#define RT8555_CLK_PFM_ENABLE_ADDR         0x0D
#define RT8555_LED_PROTECTION_ADDR         0x0E
#define RT8555_REG_CONFIG_50               0x50

#define PARSE_FAILED        0xffff

#define RT8555_DISABLE_DELAY               60
#define RT8555_ENABLE_DELAY                100

#define rt8555_emerg(msg, ...)    \
	do { if (rt8555_msg_level > 0)  \
		printk(KERN_EMERG "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_alert(msg, ...)    \
	do { if (rt8555_msg_level > 1)  \
		printk(KERN_ALERT "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_crit(msg, ...)    \
	do { if (rt8555_msg_level > 2)  \
		printk(KERN_CRIT "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_err(msg, ...)    \
	do { if (rt8555_msg_level > 3)  \
		printk(KERN_ERR "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_warning(msg, ...)    \
	do { if (rt8555_msg_level > 4)  \
		printk(KERN_WARNING "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_notice(msg, ...)    \
	do { if (rt8555_msg_level > 5)  \
		printk(KERN_NOTICE "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_info(msg, ...)    \
	do { if (rt8555_msg_level > 6)  \
		printk(KERN_INFO "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt8555_debug(msg, ...)    \
	do { if (rt8555_msg_level > 7)  \
		printk(KERN_DEBUG "[rt8555]%s: "msg, __func__, ## __VA_ARGS__); } while (0)

struct rt8555_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};
struct rt8555_backlight_info {
	/* whether support rt8555 or not */
	int rt8555_support;
	/* which i2c bus controller rt8555 mount */
	int rt8555_i2c_bus_id;
	int rt8555_2_i2c_bus_id;
	int rt8555_hw_en;
	/* rt8555 hw_en gpio */
	int rt8555_hw_en_gpio;
	int rt8555_2_hw_en_gpio;
	uint32_t reg[RT8555_RW_REG_MAX];
	int bl_on_kernel_mdelay;
	int rt8555_level_lsb;
	int rt8555_level_msb;
	int bl_led_num;
	int bl_set_long_slope;
	int dual_ic;
};

#endif /* __LINUX_RT8555_H */


