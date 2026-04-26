/*
 * lp8557.h
 *
 * lp8557 driver for backlight
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef __LINUX_LP8557_H
#define __LINUX_LP8557_H

#include "hisi_fb.h"
#include <linux/hisi/hw_cmdline_parse.h> // for runmode_is_factory

#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_panel.h"
#endif

#define LP8557_NAME "lp8557"
#define DTS_COMP_LP8557 "ti,lp8557"

/* base reg */
#define LP8557_COMMAND     (0x00)
#define LP8557_BRT_LOW     (0x03)
#define LP8557_BRT_HIGH    (0x04)
#define LP8557_CONFIG      (0x10)
#define LP8557_CURRENT     (0x11)
#define LP8557_PGEN        (0x12)
#define LP8557_BOOST       (0x13)
#define LP8557_LED_ENABLE  (0x14)
#define LP8557_STEP        (0x15)

#define LP8557_FUALT_FLAG  (0X01)
#define LP8557_DISABLE_VAL (0x00)

#define LP8557_ENABLE_ALL_LEDS      (0x3f)
#define LP8557_DISABLE_ALL_LEDS     (0X00)
#define LP8557_FAULT_SHORT_BIT      (6)
#define LP8557_FAULT_OPEN_BIT       (7)
#define LP8557_LED1_SHORT_ERR_BIT   (10)
#define LP8557_LED1_OPEN_ERR_BIT    (4)
#define LP8557_LED_NUM              (6)

#define TEST_OK                  (0)
#define TEST_ERROR_DEV_NULL      BIT(0)
#define TEST_ERROR_DATA_NULL     BIT(1)
#define TEST_ERROR_CLIENT_NULL   BIT(2)
#define TEST_ERROR_I2C           BIT(3)
#define TEST_ERROR_LED1_OPEN     BIT(4)
#define TEST_ERROR_LED2_OPEN     BIT(5)
#define TEST_ERROR_LED3_OPEN     BIT(6)
#define TEST_ERROR_LED4_OPEN     BIT(7)
#define TEST_ERROR_LED5_OPEN     BIT(8)
#define TEST_ERROR_LED6_OPEN     BIT(9)

#define LP8557_BL_MIN            (0)
#define LP8557_BL_MAX            (4095)

/* I2C Bus number */
#define I2C3_BUSNUM     (3)
#define I2C4_BUSNUM     (4)

#ifndef BIT
#define BIT(x)  (1 << (x))
#endif

#define LP8557_EMERG(msg, ...)    \
	do { if (lp8557_msg_level > 0)  \
		printk(KERN_EMERG "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_ALERT(msg, ...)    \
	do { if (lp8557_msg_level > 1)  \
		printk(KERN_ALERT "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_CRIT(msg, ...)    \
	do { if (lp8557_msg_level > 2)  \
		printk(KERN_CRIT "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_ERR(msg, ...)    \
	do { if (lp8557_msg_level > 3)  \
		printk(KERN_ERR "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_WARNING(msg, ...)    \
	do { if (lp8557_msg_level > 4)  \
		printk(KERN_WARNING "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_NOTICE(msg, ...)    \
	do { if (lp8557_msg_level > 5)  \
		printk(KERN_NOTICE "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_INFO(msg, ...)    \
	do { if (lp8557_msg_level > 6)  \
		printk(KERN_INFO "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define LP8557_DEBUG(msg, ...)    \
	do { if (lp8557_msg_level > 7)  \
		printk(KERN_DEBUG "[lp8557]%s: "msg, __func__, ## __VA_ARGS__); } while (0)

struct lp8557_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

#define LP8557_RW_REG_MAX  (9)

struct lp8557_backlight_information {
	/* whether support lp8557 or not */
	int lp8557_support;
	/* which i2c bus controller lp8557 mount */
	int lp8557_i2c_bus_id;
	int lp8557_reg[LP8557_RW_REG_MAX];
	int dual_ic;
	int bl_on_kernel_mdelay;
	int lp8557_level_lsb;
	int lp8557_level_msb;
	int bl_led_num;
};

ssize_t lp8557_set_backlight_init(uint32_t bl_level);

#endif /* __LINUX_LP8557_H */
