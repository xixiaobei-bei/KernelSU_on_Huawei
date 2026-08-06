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

#ifndef __SGM37603A_H
#define __SGM37603A_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#ifndef SGM_SUCC
#define SGM_SUCC 0
#endif

#ifndef SGM_FAIL
#define SGM_FAIL (-1)
#endif

#define SGM37603A_NAME "sgm37603a"
#define DTS_COMP_SGM37603A "sgm,sgm37603a"
#define SGM37603A_SUPPORT "sgm37603a_support"
#define SGM37603A_I2C_BUS_ID "sgm37603a_i2c_bus_id"
#define SGM37603A_HW_ENABLE "sgm37603a_hw_enable"
#define SGM37603A_HW_EN_GPIO "sgm37603a_hw_en_gpio"
#define SGM37603A_HW_EN_DELAY "bl_on_lk_mdelay"
#define SGM37603A_BL_LEVEL "bl_level"
#define MAX_STR_LEN 50
#define SGM37603A_INVALID_VAL 0xFFFF
#define SGM37603A_MSB_LEN 4

/* base reg */
#define SGM37603A_REG_DEV_ID 0x01
#define SGM37603A_REG_SW_RESET 0x01
#define SGM37603A_REG_STATUS 0x0A

/* config reg */
#define SGM37603A_REG_RATIO_LSB 0x1A
#define SGM37603A_REG_RATIO_MSB 0x19
#define SGM37603A_REG_BRIGHTNESS_CONTROL 0x11
#define SGM37603A_REG_LEDS_CONFIG 0x10
#define SGM37603A_REG_FAULT_FLAG 0x1F

#define SGM37603A_DEVICE_FAULT_OCCUR 0

#define SGM37603A_VENDOR_ID 0x18
#define SGM37603A_REG_LEDS_CONFIG_DISABLE 0x00


#define GPIO_DIR_OUT 1
#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

/* backlight */
#define SGM37603A_BL_LEN 4
#define SGM37603A_BL_OFFSET 1
#define SGM37603A_BL_DEFAULT_LEVEL 255

#define SGM37603A_FLAG_CHECK_NUM 2

enum {
	SGM37603A_OVP_FAULT_FLAG = 0x1,
	SGM37603A_OCP_FAULT_FLAG = 0x2,
};

struct sgm37603a_check_flag {
	u8 flag;
	int err_no;
};

#define SGM37603A_RW_REG_MAX 4
struct sgm37603a_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

struct sgm37603a_backlight_information {
	/* whether support sgm37603a or not */
	unsigned int sgm37603a_support;
	/* which i2c bus controller sgm37603a mount */
	unsigned int sgm37603a_i2c_bus_id;
	unsigned int sgm37603a_hw_en;
	/* sgm37603a hw_en gpio */
	unsigned int sgm37603a_hw_en_gpio;
	unsigned int sgm37603a_reg[SGM37603A_RW_REG_MAX];
	unsigned int bl_on_lk_mdelay;
	unsigned int sgm37603a_level_lsb;
	unsigned int sgm37603a_level_msb;
	unsigned int bl_level;
};

int sgm37603a_set_backlight(unsigned int bl_level);

#endif
