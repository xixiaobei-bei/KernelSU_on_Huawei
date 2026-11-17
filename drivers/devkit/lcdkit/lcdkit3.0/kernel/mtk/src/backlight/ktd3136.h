/*
 * ktd3136.h
 *
 * ktd3136 backlight driver
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

#ifndef __KTD3136_H
#define __KTD3136_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#ifndef KTD_SUCC
#define KTD_SUCC 0
#endif

#ifndef KTD_FAIL
#define KTD_FAIL (-1)
#endif

#define KTD3136_NAME "ktd3136"
#define DTS_COMP_KTD3136 "ktd,ktd3136"
#define KTD3136_SUPPORT "ktd3136_support"
#define KTD3136_I2C_BUS_ID "ktd3136_i2c_bus_id"
#define KTD3136_HW_ENABLE "ktd3136_hw_enable"
#define KTD3136_HW_EN_GPIO "ktd3136_hw_en_gpio"
#define KTD3136_HW_EN_DELAY "bl_on_kernel_mdelay"
#define KTD3136_BL_LEVEL "bl_level"
#define KTD3136_LED_CURRENT "ktd3136_reg_led_current"
#define KTD3136_HW_EN_PULL_LOW "ktd3136_hw_en_pull_low"
#define KTD3136_10000_BACKLIGHT "ktd3136_10000_backlight"
#define KTD3136_FAULT_CHECK_SUPPORT "ktd3136_check_fault_support"
#define MAX_STR_LEN 50
#define KTD3136_INVALID_VAL 0xFFFF

/* config reg */
#define KTD3136_REG_REVISION 0x00
#define KTD3136_REG_RESET 0x01
#define KTD3136_REG_CTRL_CURRENT_SET 0x02
#define KTD3136_REG_CTRL_BL_CONFIG 0x03
#define KTD3136_REG_CTRL_BRIGHTNESS_LSB 0x04
#define KTD3136_REG_CTRL_BRIGHTNESS_MSB 0x05
#define KTD3136_REG_CTRL_PWM_CONFIG 0x06
#define KTD3136_REG_CTRL_BOOT_CTRL 0x08
#define KTD3136_FLAGS_SETTING 0x0A

#define KTD3136_BL_DISABLE 0x98

#define GPIO_DIR_OUT 1
#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

/* status reg */
#define KTD3136_OCP_BIT 1
#define KTD3136_OVP_BIT 3

/* backlight */
#define KTD3136_LSB_LEN 3
#define KTD3136_BL_LSB_MASK 0x07
#define KTD3136_BL_MSB_MASK 0x0FF
#define KTD3136_BL_DEFAULT_LEVEL 255
#define KTD3136_RW_REG_MAX 6

#define KTD3136_BL_MAX 2047

struct ktd3136_check_flag {
	unsigned char flag;
	int err_no;
};

struct ktd3136_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

struct ktd3136_backlight_information {
	int ktd3136_hw_en_pull_low;
	int ktd3136_hw_en;
	int ktd3136_hw_en_gpio;
	int bl_on_kernel_mdelay;
	int ktd3136_reg[KTD3136_RW_REG_MAX];
};

int ktd3136_set_backlight(unsigned int bl_level);
#endif
