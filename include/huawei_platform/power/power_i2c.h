/*
 * power_i2c.h
 *
 * i2c interface for power module
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

#ifndef _POWER_I2C_H_
#define _POWER_I2C_H_

#include <linux/i2c.h>

int power_i2c_u8_write_byte(struct i2c_client *client, u8 reg, u8 data);
int power_i2c_u8_write_word(struct i2c_client *client, u8 reg, u16 data);
int power_i2c_u16_write_byte(struct i2c_client *client, u16 reg, u8 data);
int power_i2c_u16_write_word(struct i2c_client *client, u16 reg, u16 data);
int power_i2c_u8_read_byte(struct i2c_client *client, u8 reg, u8 *data);
int power_i2c_u8_read_word(struct i2c_client *client, u8 reg, u16 *data);
int power_i2c_u16_read_byte(struct i2c_client *client, u16 reg, u8 *data);
int power_i2c_u16_read_word(struct i2c_client *client, u16 reg, u16 *data);

#endif /* _POWER_I2C_H_ */
