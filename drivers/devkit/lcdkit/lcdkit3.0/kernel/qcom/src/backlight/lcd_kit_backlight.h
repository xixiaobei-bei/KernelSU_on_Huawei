/*
 * lcd_kit_backlight.h
 *
 * lcd kit backlight driver of lcd backlight ic
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

#ifndef __LCD_KIT_BACKLIGHT_H
#define __LCD_KIT_BACKLIGHT_H

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>

#define LCD_BACKLIGHT_INIT_CMD_NUM 30
#define LCD_BACKLIGHT_IC_NAME_LEN 20
#define LCD_BACKLIGHT_POWER_ON 0x0
#define LCD_BACKLIGHT_POWER_OFF 0x1
#define LCD_BACKLIGHT_LEVEL_POWER_ON 0x2
#define LCD_BACKLIGHT_DIMMING_SUPPORT 1

#define LCD_BACKLIGHT_VOL_400 4000000
#define LCD_BACKLIGHT_VOL_405 4050000
#define LCD_BACKLIGHT_VOL_410 4100000
#define LCD_BACKLIGHT_VOL_415 4150000
#define LCD_BACKLIGHT_VOL_420 4200000
#define LCD_BACKLIGHT_VOL_425 4250000
#define LCD_BACKLIGHT_VOL_430 4300000
#define LCD_BACKLIGHT_VOL_435 4350000
#define LCD_BACKLIGHT_VOL_440 4400000
#define LCD_BACKLIGHT_VOL_445 4450000
#define LCD_BACKLIGHT_VOL_450 4500000
#define LCD_BACKLIGHT_VOL_455 4550000
#define LCD_BACKLIGHT_VOL_460 4600000
#define LCD_BACKLIGHT_VOL_465 4650000
#define LCD_BACKLIGHT_VOL_470 4700000
#define LCD_BACKLIGHT_VOL_475 4750000
#define LCD_BACKLIGHT_VOL_480 4800000
#define LCD_BACKLIGHT_VOL_485 4850000
#define LCD_BACKLIGHT_VOL_490 4900000
#define LCD_BACKLIGHT_VOL_495 4950000
#define LCD_BACKLIGHT_VOL_500 5000000
#define LCD_BACKLIGHT_VOL_505 5050000
#define LCD_BACKLIGHT_VOL_510 5100000
#define LCD_BACKLIGHT_VOL_515 5150000
#define LCD_BACKLIGHT_VOL_520 5200000
#define LCD_BACKLIGHT_VOL_525 5250000
#define LCD_BACKLIGHT_VOL_530 5300000
#define LCD_BACKLIGHT_VOL_535 5350000
#define LCD_BACKLIGHT_VOL_540 5400000
#define LCD_BACKLIGHT_VOL_545 5450000
#define LCD_BACKLIGHT_VOL_550 5500000
#define LCD_BACKLIGHT_VOL_555 5550000
#define LCD_BACKLIGHT_VOL_560 5600000
#define LCD_BACKLIGHT_VOL_565 5650000
#define LCD_BACKLIGHT_VOL_570 5700000
#define LCD_BACKLIGHT_VOL_575 5750000
#define LCD_BACKLIGHT_VOL_580 5800000
#define LCD_BACKLIGHT_VOL_585 5850000
#define LCD_BACKLIGHT_VOL_590 5900000
#define LCD_BACKLIGHT_VOL_595 5950000
#define LCD_BACKLIGHT_VOL_600 6000000
#define LCD_BACKLIGHT_VOL_605 6050000
#define LCD_BACKLIGHT_VOL_610 6100000
#define LCD_BACKLIGHT_VOL_615 6150000
#define LCD_BACKLIGHT_VOL_620 6200000
#define LCD_BACKLIGHT_VOL_625 6250000
#define LCD_BACKLIGHT_VOL_630 6300000
#define LCD_BACKLIGHT_VOL_635 6350000
#define LCD_BACKLIGHT_VOL_640 6400000
#define LCD_BACKLIGHT_VOL_645 6450000
#define LCD_BACKLIGHT_VOL_650 6500000
#define LCD_BACKLIGHT_VOL_655 6550000
#define LCD_BACKLIGHT_VOL_660 6600000
#define LCD_BACKLIGHT_VOL_665 6650000
#define LCD_BACKLIGHT_VOL_670 6700000
#define LCD_BACKLIGHT_VOL_675 6750000
#define LCD_BACKLIGHT_VOL_680 6800000
#define LCD_BACKLIGHT_VOL_685 6850000
#define LCD_BACKLIGHT_VOL_690 6900000
#define LCD_BACKLIGHT_VOL_695 6950000
#define LCD_BACKLIGHT_VOL_700 7000000
#define LCD_BACKLIGHT_VOL_705 7050000
#define LCD_BACKLIGHT_VOL_710 7100000
#define LCD_BACKLIGHT_VOL_715 7150000

struct backlight_bias_ic_voltage {
	unsigned int voltage;
	unsigned char value;
};

struct backlight_bias_ic_config {
	char name[LCD_BACKLIGHT_IC_NAME_LEN];
	unsigned char len;
	struct backlight_bias_ic_voltage *vol_table;
};

/* backlight bias ic: lm36274 */
enum common_val_type {
	COMMON_VOL_400 = 0x00, /* 4.00V */
	COMMON_VOL_405 = 0x01, /* 4.05V */
	COMMON_VOL_410 = 0x02, /* 4.10V */
	COMMON_VOL_415 = 0x03, /* 4.15V */
	COMMON_VOL_420 = 0x04, /* 4.20V */
	COMMON_VOL_425 = 0x05, /* 4.25V */
	COMMON_VOL_430 = 0x06, /* 4.30V */
	COMMON_VOL_435 = 0x07, /* 4.35V */
	COMMON_VOL_440 = 0x08, /* 4.40V */
	COMMON_VOL_445 = 0x09, /* 4.45V */
	COMMON_VOL_450 = 0x0A, /* 4.50V */
	COMMON_VOL_455 = 0x0B, /* 4.55V */
	COMMON_VOL_460 = 0x0C, /* 4.60V */
	COMMON_VOL_465 = 0x0D, /* 4.65V */
	COMMON_VOL_470 = 0x0E, /* 4.70V */
	COMMON_VOL_475 = 0x0F, /* 4.75V */
	COMMON_VOL_480 = 0x10, /* 4.80V */
	COMMON_VOL_485 = 0x11, /* 4.85V */
	COMMON_VOL_490 = 0x12, /* 4.90V */
	COMMON_VOL_495 = 0x13, /* 4.95V */
	COMMON_VOL_500 = 0x14, /* 5.00V */
	COMMON_VOL_505 = 0x15, /* 5.05V */
	COMMON_VOL_510 = 0x16, /* 5.10V */
	COMMON_VOL_515 = 0x17, /* 5.15V */
	COMMON_VOL_520 = 0x18, /* 5.20V */
	COMMON_VOL_525 = 0x19, /* 5.25V */
	COMMON_VOL_530 = 0x1A, /* 5.30V */
	COMMON_VOL_535 = 0x1B, /* 5.35V */
	COMMON_VOL_540 = 0x1C, /* 5.40V */
	COMMON_VOL_545 = 0x1D, /* 5.45V */
	COMMON_VOL_550 = 0x1E, /* 5.50V */
	COMMON_VOL_555 = 0x1F, /* 5.55V */
	COMMON_VOL_560 = 0x20, /* 5.60V */
	COMMON_VOL_565 = 0x21, /* 5.65V */
	COMMON_VOL_570 = 0x22, /* 5.70V */
	COMMON_VOL_575 = 0x23, /* 5.75V */
	COMMON_VOL_580 = 0x24, /* 5.80V */
	COMMON_VOL_585 = 0x25, /* 5.85V */
	COMMON_VOL_590 = 0x26, /* 5.90V */
	COMMON_VOL_595 = 0x27, /* 5.95V */
	COMMON_VOL_600 = 0x28, /* 6.00V */
	COMMON_VOL_605 = 0x29, /* 6.05V */
	COMMON_VOL_610 = 0x2A, /* 6.10V */
	COMMON_VOL_615 = 0x2B, /* 6.15V */
	COMMON_VOL_620 = 0x2C, /* 6.20V */
	COMMON_VOL_625 = 0x2D, /* 6.25V */
	COMMON_VOL_630 = 0x2E, /* 6.30V */
	COMMON_VOL_635 = 0x2F, /* 6.35V */
	COMMON_VOL_640 = 0x30, /* 6.40V */
	COMMON_VOL_645 = 0x31, /* 6.45V */
	COMMON_VOL_650 = 0x32, /* 6.50V */
	COMMON_VOL_655 = 0x33, /* 6.55V */
	COMMON_VOL_660 = 0x34, /* 6.60V */
	COMMON_VOL_665 = 0x35, /* 6.65V */
	COMMON_VOL_670 = 0x36, /* 6.70V */
	COMMON_VOL_675 = 0x37, /* 6.75V */
	COMMON_VOL_680 = 0x38, /* 6.80V */
	COMMON_VOL_685 = 0x39, /* 6.85V */
	COMMON_VOL_690 = 0x3A, /* 6.90V */
	COMMON_VOL_695 = 0x3B, /* 6.95V */
	COMMON_VOL_700 = 0x3C, /* 7.00V */
	COMMON_VOL_705 = 0x3D, /* 7.05V */
	COMMON_VOL_710 = 0x3E, /* 7.10V */
	COMMON_VOL_715 = 0x3F, /* 7.15V */
	COMMON_VOL_MAX
};

/* backlight bias ic: ktz8864 */
enum ktz8864_val_type {
	KTZ8864_VOL_400 = 0x00, /* 4.00V */
	KTZ8864_VOL_405 = 0x01, /* 4.05V */
	KTZ8864_VOL_410 = 0x02, /* 4.10V */
	KTZ8864_VOL_415 = 0x03, /* 4.15V */
	KTZ8864_VOL_420 = 0x04, /* 4.20V */
	KTZ8864_VOL_425 = 0x05, /* 4.25V */
	KTZ8864_VOL_430 = 0x06, /* 4.30V */
	KTZ8864_VOL_435 = 0x07, /* 4.35V */
	KTZ8864_VOL_440 = 0x08, /* 4.40V */
	KTZ8864_VOL_445 = 0x09, /* 4.45V */
	KTZ8864_VOL_450 = 0x0A, /* 4.50V */
	KTZ8864_VOL_455 = 0x0B, /* 4.55V */
	KTZ8864_VOL_460 = 0x0C, /* 4.60V */
	KTZ8864_VOL_465 = 0x0D, /* 4.65V */
	KTZ8864_VOL_470 = 0x0E, /* 4.70V */
	KTZ8864_VOL_475 = 0x0F, /* 4.75V */
	KTZ8864_VOL_480 = 0x10, /* 4.80V */
	KTZ8864_VOL_485 = 0x11, /* 4.85V */
	KTZ8864_VOL_490 = 0x12, /* 4.90V */
	KTZ8864_VOL_495 = 0x13, /* 4.95V */
	KTZ8864_VOL_500 = 0x14, /* 5.00V */
	KTZ8864_VOL_505 = 0x15, /* 5.05V */
	KTZ8864_VOL_510 = 0x16, /* 5.10V */
	KTZ8864_VOL_515 = 0x17, /* 5.15V */
	KTZ8864_VOL_520 = 0x18, /* 5.20V */
	KTZ8864_VOL_525 = 0x19, /* 5.25V */
	KTZ8864_VOL_530 = 0x1A, /* 5.30V */
	KTZ8864_VOL_535 = 0x1B, /* 5.35V */
	KTZ8864_VOL_540 = 0x1C, /* 5.40V */
	KTZ8864_VOL_545 = 0x1D, /* 5.45V */
	KTZ8864_VOL_550 = 0x1E, /* 5.50V */
	KTZ8864_VOL_555 = 0x1F, /* 5.55V */
	KTZ8864_VOL_560 = 0x20, /* 5.60V */
	KTZ8864_VOL_565 = 0x21, /* 5.65V */
	KTZ8864_VOL_570 = 0x22, /* 5.70V */
	KTZ8864_VOL_575 = 0x23, /* 5.75V */
	KTZ8864_VOL_580 = 0x24, /* 5.80V */
	KTZ8864_VOL_585 = 0x25, /* 5.85V */
	KTZ8864_VOL_590 = 0x26, /* 5.90V */
	KTZ8864_VOL_595 = 0x27, /* 5.95V */
	KTZ8864_VOL_600 = 0x28, /* 6.00V */
	KTZ8864_VOL_605 = 0x29, /* 6.05V */
	KTZ8864_VOL_610 = 0x2A, /* 6.10V */
	KTZ8864_VOL_615 = 0x2B, /* 6.15V */
	KTZ8864_VOL_620 = 0x2C, /* 6.20V */
	KTZ8864_VOL_625 = 0x2D, /* 6.25V */
	KTZ8864_VOL_630 = 0x2E, /* 6.30V */
	KTZ8864_VOL_635 = 0x2F, /* 6.35V */
	KTZ8864_VOL_640 = 0x30, /* 6.40V */
	KTZ8864_VOL_645 = 0x31, /* 6.45V */
	KTZ8864_VOL_650 = 0x32, /* 6.50V */
	KTZ8864_VOL_655 = 0x33, /* 6.55V */
	KTZ8864_VOL_660 = 0x34, /* 6.60V */
	KTZ8864_VOL_MAX
};

enum backlight_reg_ops_mode {
	REG_READ_MODE = 0,
	REG_WRITE_MODE = 1,
	REG_UPDATE_MODE = 2
};

struct backlight_ic_cmd {
	unsigned char ops_type; /* 0:read  1:write  2:update */
	unsigned char cmd_reg;
	unsigned char cmd_val;
	unsigned char cmd_mask;
	unsigned char delay;
};

struct backlight_reg_info {
	unsigned char val_bits;
	unsigned char cmd_reg;
	unsigned char cmd_val;
	unsigned char cmd_mask;
};

enum backlight_ctrl_mode {
	BL_I2C_ONLY_MODE = 0,
	BL_PWM_ONLY_MODE = 1,
	BL_MUL_RAMP_MODE = 2,
	BL_RAMP_MUL_MODE = 3,
	BL_PWM_I2C_MODE = 4
};

struct lcd_kit_backlight_info
{
	unsigned int bl_level;
	unsigned int bl_ctrl_mod;
	unsigned int bl_only;
	unsigned int hw_en_gpio;
	unsigned int hw_en;
	unsigned int gpio_offset;
	unsigned int before_bl_on_mdelay;
	unsigned int bl_on_mdelay;
	unsigned int hw_en_pull_low;
	unsigned int ic_disable_mdelay;
	unsigned int bl_10000_support;
	unsigned int bits_compatible;
	unsigned int msb_before_lsb;
	struct backlight_ic_cmd init_cmds[LCD_BACKLIGHT_INIT_CMD_NUM];
	unsigned int num_of_init_cmds;
	unsigned int init_vsp_index;
	unsigned int init_vsn_index;
	struct backlight_reg_info bl_lsb_reg_cmd;
	struct backlight_reg_info bl_msb_reg_cmd;
	struct backlight_ic_cmd bl_enable_cmd;
	struct backlight_ic_cmd bl_disable_cmd;
	struct backlight_ic_cmd disable_dev_cmd;
	unsigned int pull_down_boost_support;
	unsigned int pull_down_boost_delay;
	struct backlight_ic_cmd pull_down_vsp_cmd;
	struct backlight_ic_cmd pull_down_vsn_cmd;
	struct backlight_ic_cmd pull_down_boost_cmd;
	struct backlight_ic_cmd bias_enable_cmd;
	unsigned int bl_lowpower_delay;
	unsigned int fault_check_enable;
	struct backlight_ic_cmd bl_ovp_flag_cmd;
	struct backlight_ic_cmd bl_ocp_flag_cmd;
	struct backlight_ic_cmd bl_tsd_flag_cmd;
	unsigned int dual_ic;
	unsigned int dual_i2c_bus_id;
	unsigned int dual_hw_en_gpio;
	int bl_dimming_support;
	int bl_dimming_config;
	int bl_dimming_resume;
	int dimming_config_reg;
};

struct lcd_kit_backlight_device {
	struct device *dev;
	struct i2c_client *client;
	struct lcd_kit_backlight_info bl_config;
	struct semaphore sem;
};
#endif
