// SPDX-License-Identifier: GPL-2.0
/*
 * hl7139_i2c.c
 *
 * hl7139 i2c interface
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

#include "hl7139.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG hl7139_i2c
HWLOG_REGIST();

int hl7139_read_block(struct hl7139_device_info *di, u8 *value,
	u8 reg, unsigned int num_bytes)
{
	return power_i2c_read_block(di->client, &reg, 1, value, num_bytes);
}

int hl7139_write_byte(struct hl7139_device_info *di, u8 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int hl7139_read_byte(struct hl7139_device_info *di, u8 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int hl7139_write_mask(struct hl7139_device_info *di, u8 reg, u8 mask, u8 shift,
	u8 value)
{
	int ret;
	u8 val = 0;

	ret = hl7139_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return hl7139_write_byte(di, reg, val);
}

int hl7139_read_word(struct hl7139_device_info *di, u8 reg, u16 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_word(di->client, reg, value, true);
}
