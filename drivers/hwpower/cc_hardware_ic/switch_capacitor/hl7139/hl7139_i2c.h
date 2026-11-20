/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hl7139_i2c.h
 *
 * hl7139 i2c header file
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

#ifndef _HL7139_I2C_H_
#define _HL7139_I2C_H_

#include "hl7139.h"

int hl7139_read_block(struct hl7139_device_info *di, u8 *value, u8 reg, unsigned int num_bytes);
int hl7139_write_byte(struct hl7139_device_info *di, u8 reg, u8 value);
int hl7139_read_byte(struct hl7139_device_info *di, u8 reg, u8 *value);
int hl7139_write_mask(struct hl7139_device_info *di, u8 reg, u8 mask, u8 shift, u8 value);
int hl7139_read_word(struct hl7139_device_info *di, u8 reg, u16 *value);

#endif /* _HL7139_I2C_H_ */
