/*
 * switch_chip_i2c.c
 *
 * driver file for switch_chip_i2c
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

#include "switch_chip_i2c.h"
#include <linux/delay.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG switch_chip_i2c
HWLOG_REGIST();

int switch_write_reg(const struct i2c_client *client, int reg, int val)
{
	int ret = -EINVAL;
	int i;

	if (!client) {
		hwlog_err("client is null\n");
		return ret;
	}

	for (i = 0; (i < I2C_RETRY) && (ret < 0); i++) {
		ret = i2c_smbus_write_byte_data(client, reg, val);
		if (ret < 0) {
			hwlog_err("write_reg failed[%x]\n", reg);
			usleep_range(1000, 1100); /* sleep 1ms */
		}
	}

	return ret;
}

int switch_read_reg(const struct i2c_client *client, int reg)
{
	int ret = -EINVAL;
	int i;

	if (!client) {
		hwlog_err("client is null\n");
		return ret;
	}

	for (i = 0; (i < I2C_RETRY) && (ret < 0); i++) {
		ret = i2c_smbus_read_byte_data(client, reg);
		if (ret < 0) {
			hwlog_err("read_reg failed[%x]\n", reg);
			usleep_range(1000, 1100); /* sleep 1ms */
		}
	}

	return ret;
}

int switch_write_reg_mask(const struct i2c_client *client, int reg,
	int value, int mask)
{
	int val;

	if (!client) {
		hwlog_err("client is null\n");
		return -EINVAL;
	}

	val = switch_read_reg(client, reg);
	if (val < 0)
		return val;

	val &= ~mask;
	val |= value & mask;

	return switch_write_reg(client, reg, val);
}
