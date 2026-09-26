/*
 * aw99703.h
 *
 * aw99703 backlight driver
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

#ifndef __AW99703_H
#define __AW99703_H

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

#define AW99703_NAME "aw99703"
#define DTS_COMP_AW99703 "aw,aw99703"
#define AW99703_SUPPORT "aw99703_support"
#define AW99703_I2C_BUS_ID "aw99703_i2c_bus_id"
#define AW99703_HW_ENABLE "aw99703_hw_enable"
#define AW99703_HW_EN_GPIO "aw99703_hw_en_gpio"
#define AW99703_HW_EN_DELAY "bl_on_kernel_mdelay"
#define AW99703_BL_LEVEL "bl_level"
#define AW99703_LED_CURRENT "aw99703_reg_led_current"
#define AW99703_MODE "aw99703_reg_mode"
#define AW99703_HW_EN_PULL_LOW "aw99703_hw_en_pull_low"
#define AW99703_10000_BACKLIGHT "aw99703_10000_backlight"
#define AW99703_FAULT_CHECK_SUPPORT "aw99703_check_fault_support"
#define MAX_STR_LEN 50
#define AW99703_INVALID_VAL 0xFFFF

#define AW99703_REG_STATUS_DEFAULT_VALUE 0x00

/* config reg */
#define AW99703_REG_CHIP_ID 0x00
#define AW99703_REG_RESET 0x01
#define AW99703_REG_MODE 0x02
#define AW99703_REG_LED_CURRENT 0x03
#define AW99703_REG_BOOST_CONTROL1 0x04
#define AW99703_REG_BOOST_CONTROL2 0x05
#define AW99703_REG_LED_BRIGHTNESS_LSB 0x06
#define AW99703_REG_LED_BRIGHTNESS_MSB 0x07
#define AW99703_REG_PWM_CONTROL 0x08
#define AW99703_REG_RAMP_TIME 0x09
#define AW99703_REG_TRANSITION_RAMP_TIME 0x0A
#define AW99703_FLASH_SETTING 0x0B
#define AW99703_AUTO_FREQ_HIGH_THRESHOLD 0x0C
#define AW99703_AUTO_FREQ_LOW_THRESHOLD 0x0D
#define AW99703_FLAGS1_SETTING 0x0E
#define AW99703_FLAGS2_SETTING 0x0F

/* Boost headroom ctl */
#define AW99703_REG_BOOT_HEADROOM_23 0x23
/* Pfm hys */
#define AW99703_REG_PFM_HYS_24 0x24
/* Pfm threshold */
#define AW99703_REG_PFM_THRESHOLD_25 0x25
/* Boost max duty */
#define AW99703_REG_BOOT_DUTY_31 0x31
/* Soft start cfg */
#define AW99703_REG_SOFT_START_68 0x68
/* Pfm mode cfg */
#define AW99703_REG_PFM_67 0x67
/* Boost max duty */
#define AW99703_REG_BOOT_DUTY_31_1 0x31
/* Soft start cfg */
#define AW99703_REG_SOFT_START_68_1 0x68
#define AW99703_FLASH_SETTING_1 0x0B
#define AW99703_REG_BOOST_CONTROL2_1 0x05
#define AW99703_REG_MODE_DELAY 3

#define AW99703_CHIP_ID 0x03

#define GPIO_DIR_OUT 1
#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0

/* status reg */
#define AW99703_OCP_BIT 1
#define AW99703_OVP_BIT 7

/* backlight */
#define AW99703_LSB_LEN 3
#define AW99703_BL_OFFSET 1
#define AW99703_BL_LSB_MASK 0x07
#define AW99703_BL_MSB_MASK 0x0FF
#define AW99703_BL_DEFAULT_LEVEL 255

#define AW99703_FLAG_CHECK_NUM 2
#define AW99703_BL_MAX 2047
#define AW99703_LED_CURRENT_DEFAULT_VALUE 0x9F
#define AW99703_LED_MODE_DEFAULT_VALUE 0x11

struct aw99703_check_flag {
	unsigned char flag;
	int err_no;
};

#define AW99703_RW_REG_MAX 19
struct aw99703_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

struct aw99703_backlight_information {
	int aw99703_hw_en_pull_low;
	int aw99703_hw_en;
	int aw99703_hw_en_gpio;
	int bl_on_kernel_mdelay;
	int aw99703_reg[AW99703_RW_REG_MAX];
};

int aw99703_set_backlight(unsigned int bl_level);
#endif
