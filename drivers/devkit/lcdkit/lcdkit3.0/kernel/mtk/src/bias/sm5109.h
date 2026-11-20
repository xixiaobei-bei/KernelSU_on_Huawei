/*
 * sm5109.h
 *
 * sm5109 bias driver head file
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

#ifndef __SM5109_H_
#define __SM5109_H_

#include <linux/module.h>
#include <linux/i2c.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>

struct sm5109_voltage {
	int voltage;
	unsigned char value;
};

enum {
	SM5109_VOL_40 = 0x00,
	SM5109_VOL_41 = 0x01,
	SM5109_VOL_42 = 0x02,
	SM5109_VOL_43 = 0x03,
	SM5109_VOL_44 = 0x04,
	SM5109_VOL_45 = 0x05,
	SM5109_VOL_46 = 0x06,
	SM5109_VOL_47 = 0x07,
	SM5109_VOL_48 = 0x08,
	SM5109_VOL_49 = 0x09,
	SM5109_VOL_50 = 0x0A,
	SM5109_VOL_51 = 0x0B,
	SM5109_VOL_52 = 0x0C,
	SM5109_VOL_53 = 0x0D,
	SM5109_VOL_54 = 0x0E,
	SM5109_VOL_55 = 0x0F,
	SM5109_VOL_56 = 0x10,
	SM5109_VOL_57 = 0x11,
	SM5109_VOL_58 = 0x12,
	SM5109_VOL_59 = 0x13,
	SM5109_VOL_60 = 0x14,
	SM5109_VOL_61 = 0x15,
	SM5109_VOL_62 = 0x16,
	SM5109_VOL_63 = 0x17,
	SM5109_VOL_64 = 0x18,
	SM5109_VOL_65 = 0x19,
	SM5109_VOL_MAX
};

#define SM5109_REG_VPOS 0x00
#define SM5109_REG_VNEG 0x01
#define SM5109_REG_APP_DIS 0x03
#define SM5109_APPS_BIT (1 << 6)
#define SM5109_DISP_BIT (1 << 1)
#define SM5109_DISN_BIT (1 << 0)
#define SM5109_DTS_COMP "sm,sm5109"

struct sm5109_device_info {
	struct device *dev;
	struct i2c_client *client;
};
#endif
