/*
 * Linux kernel modules for VI5300 FlightSense TOF sensor
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "vi5300_platform.h"
#include "vi5300_def.h"

#define I2C_M_WR 0x00
#define BYTES_PER_WORD 2
#define BYTES_PER_DWORD 4

static int vi5300_i2c_write(VI5300_DEV dev, uint8_t reg, uint8_t *data, uint8_t len)
{
	int ret = 0;
	uint8_t *addr_buf = NULL;
	struct i2c_msg msg[1];
	struct i2c_client *client = dev->client;

	if(!client)
		return -EINVAL;
	addr_buf = kmalloc(len + 1, GFP_KERNEL);
	if (!addr_buf)
		return -ENOMEM;
	addr_buf[0] = reg;
	if (memcpy_s(&addr_buf[1], len, data, len) != EOK)
		pr_err("memcpy_s failed");
	msg[0].addr = client->addr;
	msg[0].flags = I2C_M_WR;
	msg[0].buf = addr_buf;
	msg[0].len = len+1;

	ret = i2c_transfer(client->adapter, msg, 1);
	if(ret != 1)
	{
		pr_err("%s: i2c_transfer err:%d, addr:0x%x, reg:0x%x\n",
			__func__, ret, client->addr, reg);
	}
	kfree(addr_buf);
	return ret < 0 ? ret : (ret != 1 ? -EIO : 0);
}

static int vi5300_i2c_read(VI5300_DEV dev, uint8_t reg, uint8_t *data, uint8_t len)
{
	int ret =0;
	struct i2c_msg msg[2];
	struct i2c_client *client = dev->client;

	if(!client)
		return -EINVAL;
	msg[0].addr = client->addr;
	msg[0].flags = I2C_M_WR;
	msg[0].buf = &reg;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = data;
	msg[1].len = len;
	ret = i2c_transfer(client->adapter, msg, 2);
	if(ret != 2)
	{
		pr_err("%s: i2c_transfer err:%d, addr:0x%x, reg:0x%x\n",
			__func__, ret, client->addr, reg);
	}

	return ret < 0 ? ret : (ret != 2 ? -EIO : 0);
}

int32_t vi5300_write_byte(VI5300_DEV dev, uint8_t reg, uint8_t data)
{
	int8_t Status = STATUS_OK;

	Status = vi5300_i2c_write(dev, reg, &data, 1);
	return Status;
}

int32_t vi5300_read_byte(VI5300_DEV dev, uint8_t reg, uint8_t *data)
{
	int8_t Status = STATUS_OK;

	Status = vi5300_i2c_read(dev, reg, data, 1);
	return Status;
}

int32_t vi5300_write_multibytes(VI5300_DEV dev, uint8_t reg, uint8_t *data, int32_t count)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = vi5300_i2c_write(dev, reg, data, count);
	return Status;
}

int32_t vi5300_read_multibytes(VI5300_DEV dev, uint8_t reg, uint8_t *data, int32_t count)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = vi5300_i2c_read(dev, reg, data, count);
	return Status;
}

int32_t vi5300_write_reg_offset(VI5300_DEV dev, uint8_t reg, uint8_t offset, uint8_t data)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = vi5300_write_byte(dev, reg+offset, data);
	return Status;
}

int32_t vi5300_read_reg_offset(VI5300_DEV dev, uint8_t reg, uint8_t offset, uint8_t *data)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = vi5300_read_byte(dev, reg+offset, data);
	return Status;
}
