/*
 * sm5450_i2c.h
 *
 * sm5450 i2c header file
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

#ifndef _SM5450_I2C_H_
#define _SM5450_I2C_H_

#include "sm5450.h"

int sm5450_read_block(struct sm5450_device_info *di, u8 *value, u8 reg, unsigned int num_bytes);
int sm5450_write_byte(struct sm5450_device_info *di, u8 reg, u8 value);
int sm5450_read_byte(struct sm5450_device_info *di, u8 reg, u8 *value);
int sm5450_write_mask(struct sm5450_device_info *di, u8 reg, u8 mask, u8 shift, u8 value);

#endif /* _SM5450_I2C_H_ */
