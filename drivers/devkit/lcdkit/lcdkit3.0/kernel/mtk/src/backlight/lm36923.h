/* Copyright (c) 2019-2019, Huawei terminal Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef __LM36923_H
#define __LM36923_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#ifndef LM_SUCC
#define LM_SUCC 0
#endif

#ifndef LM_FAIL
#define LM_FAIL (-1)
#endif

#define LM36923_NAME "lm36923"
#define DTS_COMP_LM36923 "lm,lm36923"
#define LM36923_SUPPORT "lm36923_support"
#define LM36923_I2C_BUS_ID "lm36923_i2c_bus_id"
#define LM36923_HW_ENABLE "lm36923_hw_enable"
#define LM36923_HW_EN_GPIO "lm36923_hw_en_gpio"
#define LM36923_HW_EN_DELAY "bl_on_lk_mdelay"
#define LM36923_BL_LEVEL "bl_level"
#define MAX_STR_LEN 50
#define LM36923_INVALID_VAL 0xFFFF
#define LM36923_MSB_LEN 3
#define LM36923_MSB_MASK 0xFF
#define LM36923_LSB_MASK 0x07
#define LM36923_BL_DEFAULT_LEVEL 255

/* base reg */
#define LM36923_REG_DEV_ID 0x01
#define LM36923_REG_SW_RESET 0x01
#define LM36923_REG_STATUS 0x0A

/* config reg */
#define LM36923_REG_RATIO_LSB 0x18
#define LM36923_REG_RATIO_MSB 0x19
#define LM36923_REG_BRIGHTNESS_CONTROL 0x11
#define LM36923_REG_LEDS_CONFIG 0x10
#define LM36923_REG_FAULT_FLAG 0x1F

#define LM36923_DEVICE_FAULT_OCCUR 0

#define LM36923_VENDOR_ID 0x18
#define LM36923_REG_LEDS_CONFIG_DISABLE 0x10


#define GPIO_DIR_OUT 1
#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

/* backlight */
#define LM36923_BL_LEN 3

#define LM36923_FLAG_CHECK_NUM 2

enum {
	LM36923_OVP_FAULT_FLAG = 0x1,
	LM36923_OCP_FAULT_FLAG = 0x2,
};

struct lm36923_check_flag {
	u8 flag;
	int err_no;
};

#define LM36923_RW_REG_MAX 4
struct lm36923_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

struct lm36923_backlight_information {
	/* whether support lm36923 or not */
	unsigned int lm36923_support;
	/* which i2c bus controller lm36923 mount */
	unsigned int lm36923_i2c_bus_id;
	unsigned int lm36923_hw_en;
	/* lm36923 hw_en gpio */
	unsigned int lm36923_hw_en_gpio;
	unsigned int lm36923_reg[LM36923_RW_REG_MAX];
	unsigned int bl_on_lk_mdelay;
	unsigned int lm36923_level_lsb;
	unsigned int lm36923_level_msb;
	unsigned int bl_level;
};

int lm36923_set_backlight(unsigned int bl_level);

#endif
