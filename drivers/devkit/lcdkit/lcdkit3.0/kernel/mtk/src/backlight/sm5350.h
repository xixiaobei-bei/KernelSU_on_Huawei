/*
 * sm5350.h
 *
 * sm5350 backlight driver
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

#ifndef __SM5350_H
#define __SM5350_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#ifndef AW_SUCC
#define AW_SUCC 0
#endif

#ifndef AW_FAIL
#define AW_FAIL (-1)
#endif

#define SM5350_NAME "sm5350"
#define DTS_COMP_SM5350 "sm,sm5350"
#define SM5350_SUPPORT "sm5350_support"
#define SM5350_I2C_BUS_ID "sm5350_i2c_bus_id"
#define SM5350_HW_ENABLE "sm5350_hw_enable"
#define SM5350_HW_EN_GPIO "sm5350_hw_en_gpio"
#define SM5350_HW_EN_DELAY "bl_on_kernel_mdelay"
#define SM5350_BL_LEVEL "bl_level"
#define SM5350_LED_CURRENT "sm5350_reg_led_current"
#define SM5350_HW_EN_PULL_LOW "sm5350_hw_en_pull_low"
#define SM5350_10000_BACKLIGHT "sm5350_10000_backlight"
#define MAX_STR_LEN 50
#define SM5350_INVALID_VAL 0xFFFF


/* config reg */
#define SM5350_REG_REVISION 0x00
#define SM5350_REG_RESET 0x01
#define SM5350_REG_HVLED_CURRENT_SINK_OUT_CONFIG 0x10
#define SM5350_REG_CTRL_A_UP_DOWN_RAMP_TIME 0x11
#define SM5350_REG_CTRL_B_UP_DOWN_RAMP_TIME 0x12
#define SM5350_REG_CTRL_RUN_RAMP_TIME 0x13
#define SM5350_REG_CTRL_RUN_RAMP_TIME_CONFIG 0x14
#define SM5350_REG_BL_CONFIG 0x16
#define SM5350_REG_BL_CTRL_A_FULL_SCALE_CURRENT_SETTING 0x17
#define SM5350_REG_BL_CTRL_B_FULL_SCALE_CURRENT_SETTING 0x18
#define SM5350_REG_HVLED_CURRENT_SINK_FEEDBACK_ENABLE 0x19
#define SM5350_REG_BOOST_CTRL 0x1A
#define SM5350_REG_AUTO_FREQUENC_THRESHOLD 0x1B
#define SM5350_REG_PWM_CONFIG 0x1C
#define SM5350_REG_CTRL_A_BRIGHTNESS_LSB 0x20
#define SM5350_REG_CTRL_A_BRIGHTNESS_MSB 0x21
#define SM5350_REG_CTRL_B_BRIGHTNESS_LSB 0x22
#define SM5350_REG_CTRL_B_BRIGHTNESS_MSB 0x23
#define SM5350_REG_CTRL_BANK_ENABLE 0x24
#define SM5350_REG_HVLED_OPEN_FAULT 0xB0
#define SM5350_REG_HVLED_SHORT_FAULT 0xB2
#define SM5350_REG_LED_FAULT_ENABLE 0xB4

#define SM5350_CHIP_ID 0x00

#define GPIO_DIR_OUT 1
#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

/* status reg */
#define SM5350_OCP_BIT 1
#define SM5350_OVP_BIT 7

/* backlight */
#define SM5350_LSB_LEN 3
#define SM5350_BL_OFFSET 1
#define SM5350_BL_LSB_MASK 0x07
#define SM5350_BL_MSB_MASK 0x0FF
#define SM5350_BL_DEFAULT_LEVEL 255

#define SM5350_BL_MAX 2047

struct sm5350_check_flag {
	unsigned char flag;
	int err_no;
};

#define SM5350_RW_REG_MAX 9
struct sm5350_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

struct sm5350_backlight_information {
	int sm5350_hw_en_pull_low;
	int sm5350_hw_en;
	int sm5350_hw_en_gpio;
	int bl_on_kernel_mdelay;
	int sm5350_reg[SM5350_RW_REG_MAX];
};

int sm5350_set_backlight(unsigned int bl_level);
#endif
