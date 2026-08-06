/*
 * sm5450_i2c.c
 *
 * sm5450 i2c interface
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

#include "sm5450.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sm5450
HWLOG_REGIST();

int sm5450_read_block(struct sm5450_device_info *di, u8 *value,
	u8 reg, unsigned int num_bytes)
{
	return power_i2c_read_block(di->client, &reg, 1, value, num_bytes);
}

int sm5450_write_byte(struct sm5450_device_info *di, u8 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int sm5450_read_byte(struct sm5450_device_info *di, u8 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int sm5450_write_mask(struct sm5450_device_info *di, u8 reg, u8 mask, u8 shift,
	u8 value)
{
	int ret;
	u8 val = 0;

	ret = sm5450_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return sm5450_write_byte(di, reg, val);
}
