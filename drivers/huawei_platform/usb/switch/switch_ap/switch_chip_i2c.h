/*
 * switch_chip_i2c.h
 *
 * switch_chip_i2c header file
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

#ifndef _SWITCH_CHIP_I2C_
#define _SWITCH_CHIP_I2C_

#include <linux/i2c.h>

#define I2C_RETRY        5

int switch_write_reg(const struct i2c_client *client, int reg, int val);
int switch_read_reg(const struct i2c_client *client, int reg);
int switch_write_reg_mask(const struct i2c_client *client, int reg,
	int value, int mask);

#endif /* _SWITCH_CHIP_I2C_ */
