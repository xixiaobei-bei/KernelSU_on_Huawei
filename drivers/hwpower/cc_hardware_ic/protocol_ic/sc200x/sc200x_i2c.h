/*
 * sc200x_i2c.h
 *
 * sc200x i2c header file
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _SC200X_I2C_H_
#define _SC200X_I2C_H_

#include "sc200x.h"

#define SC200X_I2C_ADDR                     0x71

#define SC200X_I2C_RETRY_TIMES              3
#define SC200X_I2C_RETRY_DELAY              1000 /* 1000 us */

#define SC200X_MAX_TX                       16
#define SC200X_MAX_RX                       16

#define SC200X_MAX_SPCE_BASE_REG_ADDR       0x10
#define SC200X_MAX_SPCE_REG_ADDR            0x1F

int sc200x_i2c_write_reg(struct sc200x_device_info *di,
	unsigned char reg, const unsigned char *val, int len);
int sc200x_i2c_read_reg(struct sc200x_device_info *di,
	unsigned char reg, unsigned char *val, int len);
int sc200x_write_reg(struct sc200x_device_info *di, int reg, int value);
int sc200x_write_reg_mask(struct sc200x_device_info *di, int reg, int value, int mask);
int sc200x_read_reg(struct sc200x_device_info *di, int reg);

#endif /* _SC200X_I2C_H_ */
