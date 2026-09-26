/*
 * sc200x_i2c.c
 *
 * sc200x i2c interface
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

#include "sc200x_i2c.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sc200x
HWLOG_REGIST();

int sc200x_i2c_write_reg(struct sc200x_device_info *di,
	unsigned char reg, const unsigned char *val, int len)
{
	int i;
	int ret;
	unsigned char buf[SC200X_MAX_TX + 1] = { 0 };

	if (!di)
		return -ENODEV;

	if (len > SC200X_MAX_TX) {
		hwlog_err("write_reg is too big, len=%d\n", len);
		return SC200X_FAILED;
	}

	buf[0] = reg;
	for (i = 0; i < len; i++)
		buf[i + 1] = val[i];

	for (i = 0; i < SC200X_I2C_RETRY_TIMES; i++) {
		ret = power_i2c_write_block(di->client, buf, len + 1);
		if (ret >= 0) {
			ret = SC200X_SUCCESS;
			break;
		}
		hwlog_err("write_reg fail, reg=0x%x len=%d\n", reg, len);
		power_usleep(SC200X_I2C_RETRY_DELAY);
	}

	return ret;
}

int sc200x_i2c_read_reg(struct sc200x_device_info *di,
	unsigned char reg, unsigned char *val, int len)
{
	int i;
	int ret;

	if (!di)
		return -ENODEV;

	if (len > SC200X_MAX_RX) {
		hwlog_err("read_reg is too big, len=%d\n", len);
		return SC200X_FAILED;
	}

	for (i = 0; i < SC200X_I2C_RETRY_TIMES; i++) {
		ret = power_i2c_read_block(di->client, &reg, 0x01, val, len);
		if (ret >= 0) {
			ret = SC200X_SUCCESS;
			break;
		}
		hwlog_err("read_reg fail, reg=0x%x len=%d\n", reg, len);
		power_usleep(SC200X_I2C_RETRY_DELAY);
	}

	return ret;
}

/*
 * sc200x chip i2c write:
 * 1. For registers 0x00~0x0f, write continuously from addr 0x00.
 *    e.g: if the target reg is 0x03, write 0x00 -> 0x01 -> 0x03.
 * 2. For registers 0x10~0x1f, write continuously from addr 0x10.
 *    e.g: if the target reg is 0x14, write 0x10 -> 0x11 -> 0x13 -> 0x14.
 * 3. For registers 0x20 or 0x21, write 3 bytes;
 *    e.g: if the target reg is 0x20, write 0x20 -> 0x21 -> an extra value.
 */
static int sc200x_base_write_reg(struct sc200x_device_info *di,
	int reg, int value, int mask, bool flag)
{
	int ret;
	int val;
	int rd_len;
	int wr_len;
	int reg_pos;
	unsigned char reg_base;
	unsigned char buf[SC200X_MAX_TX] = { 0 };

	reg_pos = reg & 0x0F;
	reg_base = (unsigned char)(reg & 0xF0);
	/* reg 0x00~0x0F, 0x10~0x1F */
	if (reg_base <= SC200X_MAX_SPCE_BASE_REG_ADDR) {
		rd_len = reg_pos + 1;
		wr_len = reg_pos + 1;
	} else { /* reg 0x20 or 0x21 */
		rd_len = 0x02;
		wr_len = 0x03;
	}

	ret = sc200x_i2c_read_reg(di, reg_base, buf, rd_len);
	if (ret < 0) {
		hwlog_err("base read fail, reg=0x%x\n", reg);
		return ret;
	}

	if (flag) {
		val = buf[reg_pos];
		val &= ~mask;
		val |= (value & mask);
	} else {
		val = value;
	}
	buf[reg_pos] = (unsigned char)val;

	ret = sc200x_i2c_write_reg(di, reg_base, buf, wr_len);
	if (ret < 0) {
		hwlog_err("base write fail, reg=0x%x val=0x%x\n", reg, val);
		return ret;
	}
	hwlog_info("base write success, reg=0x%x val=0x%x\n", reg, val);

	return SC200X_SUCCESS;
}

/*
 * sc200x chip i2c read:
 * 1. For registers 0x00~0x0f, read continuously from addr 0x00.
 *    e.g: if the target reg is 0x03, read 0x00 -> 0x01 -> 0x03.
 * 2. For registers 0x10~0x1f, read continuously from addr 0x10.
 *    e.g: if the target reg is 0x14, read 0x10 -> 0x11 -> 0x13 -> 0x14.
 * 3. For registers 0x20 or 0x21, read directly.
 */
static int sc200x_base_read_reg(struct sc200x_device_info *di, int reg)
{
	int len;
	int ret;
	unsigned char reg_base;
	unsigned char buf[SC200X_MAX_RX] = { 0 };

	if (reg > SC200X_MAX_SPCE_REG_ADDR) { /* reg 0x20 or 0x21 */
		len = 1;
		reg_base = reg;
	} else { /* reg 0x00~0x0F, 0x10~0x1F */
		len = (reg & 0x0F) + 1;
		reg_base = (unsigned char)(reg & 0xF0);
	}

	ret = sc200x_i2c_read_reg(di, reg_base, buf, len);
	if (ret < 0) {
		hwlog_err("base read fail, reg=0x%x ret=%d\n", reg, ret);
		return ret;
	}

	ret = buf[len - 1];
	hwlog_info("base read success, reg=0x%x val=0x%x\n", reg, ret);

	return ret;
}

int sc200x_write_reg(struct sc200x_device_info *di, int reg, int value)
{
	if (!di)
		return -ENODEV;

	return sc200x_base_write_reg(di, reg, value, 0x00, false);
}

int sc200x_write_reg_mask(struct sc200x_device_info *di, int reg, int value, int mask)
{
	if (!di)
		return -ENODEV;

	return sc200x_base_write_reg(di, reg, value, mask, true);
}

int sc200x_read_reg(struct sc200x_device_info *di, int reg)
{
	if (!di)
		return -ENODEV;

	return sc200x_base_read_reg(di, reg);
}
