/*
 * sc8545_i2c.c
 *
 * sc8545 i2c interface
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

#include "sc8545_i2c.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sc8545
HWLOG_REGIST();

int sc8545_write_byte(struct sc8545_device_info *di, u8 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int sc8545_read_byte(struct sc8545_device_info *di, u8 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int sc8545_read_word(struct sc8545_device_info *di, u8 reg, s16 *value)
{
	u16 data = 0;

	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	if (power_i2c_u8_read_word(di->client, reg, &data, true))
		return -EIO;

	*value = (s16)data;
	return 0;
}

int sc8545_write_mask(struct sc8545_device_info *di, u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = sc8545_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return sc8545_write_byte(di, reg, val);
}
