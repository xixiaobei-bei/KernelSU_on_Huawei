/*
 * power_i2c.c
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

#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_i2c
HWLOG_REGIST();

#define LEN_WR_MSG     1
#define LEN_RD_MSG     2
#define LEN_U8_REG     1
#define LEN_U16_REG    2
#define LEN_U8_DAT     1
#define LEN_U16_DAT    2

int power_i2c_write_block(struct i2c_client *client, u8 *buf, u8 buf_len)
{
	struct i2c_msg msg[LEN_WR_MSG];

	if (!client || !buf) {
		hwlog_err("client or buf is null\n");
		return -EINVAL;
	}

	/* flags: 0 is write data, from master to slave */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = buf;
	msg[0].len = buf_len;

	/* i2c_transfer return number of messages transferred */
	if (i2c_transfer(client->adapter, msg, LEN_WR_MSG) != LEN_WR_MSG) {
		hwlog_err("addr=%x write fail\n", client->addr);
		return -EIO;
	}

	return 0;
}

int power_i2c_read_block(struct i2c_client *client, u8 *cmd, u8 cmd_len,
	u8 *buf, u8 buf_len)
{
	struct i2c_msg msg[LEN_RD_MSG];

	if (!client || !cmd || !buf) {
		hwlog_err("client or cmd or buf is null\n");
		return -EINVAL;
	}

	/* the cmd to be written before reading data */
	/* flags: 0 is write data, from master to slave */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = cmd;
	msg[0].len = cmd_len;

	/* flags: I2C_M_RD is read data, from slave to master */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = buf_len;

	/* i2c_transfer return number of messages transferred */
	if (i2c_transfer(client->adapter, msg, LEN_RD_MSG) != LEN_RD_MSG) {
		hwlog_err("addr=%x read fail\n", client->addr);
		return -EIO;
	}

	return 0;
}

int power_i2c_u8_write_byte(struct i2c_client *client, u8 reg, u8 data)
{
	u8 buf[LEN_U8_REG + LEN_U8_DAT] = { 0 };

	/*
	 * buf[0]: 8bit slave register address
	 * buf[1]: 8bit data to be written
	 */
	buf[0] = reg;
	buf[1] = data;

	return power_i2c_write_block(client, buf, sizeof(buf));
}

int power_i2c_u8_write_word(struct i2c_client *client, u8 reg, u16 data)
{
	u8 buf[LEN_U8_REG + LEN_U16_DAT] = { 0 };

	/*
	 * buf[0]: 8bit slave register address
	 * buf[1]: low-8bit data to be written
	 * buf[2]: high-8bit data to be written
	 */
	buf[0] = reg;
	buf[1] = data & 0xff;
	buf[2] = data >> 8;

	return power_i2c_write_block(client, buf, sizeof(buf));
}

int power_i2c_u16_write_byte(struct i2c_client *client, u16 reg, u8 data)
{
	u8 buf[LEN_U16_REG + LEN_U8_DAT] = { 0 };

	/*
	 * buf[0]: high-8bit slave register address
	 * buf[1]: low-8bit slave register address
	 * buf[2]: data to be written
	 */
	buf[0] = reg >> 8;
	buf[1] = reg & 0xff;
	buf[2] = data;

	return power_i2c_write_block(client, buf, sizeof(buf));
}

int power_i2c_u16_write_word(struct i2c_client *client, u16 reg, u16 data)
{
	u8 buf[LEN_U16_REG + LEN_U16_DAT] = { 0 };

	/*
	 * buf[0]: high-8bit slave register address
	 * buf[1]: low-8bit slave register address
	 * buf[2]: low-8bit data to be written
	 * buf[3]: high-8bit data to be written
	 */
	buf[0] = reg >> 8;
	buf[1] = reg & 0xff;
	buf[2] = data & 0xff;
	buf[3] = data >> 8;

	return power_i2c_write_block(client, buf, sizeof(buf));
}

int power_i2c_u8_read_byte(struct i2c_client *client, u8 reg, u8 *data)
{
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[LEN_U8_DAT] = { 0 };

	/* cmd[0]: 8bit register */
	cmd[0] = reg;

	if (power_i2c_read_block(client, cmd, sizeof(cmd), buf, sizeof(buf)))
		return -EIO;

	/* buf[0]: 8bit data */
	*data = buf[0];
	return 0;
}

int power_i2c_u8_read_word(struct i2c_client *client, u8 reg, u16 *data, bool endian)
{
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[LEN_U16_DAT] = { 0 };

	/* cmd[0]: 8bit register */
	cmd[0] = reg;

	if (power_i2c_read_block(client, cmd, sizeof(cmd), buf, sizeof(buf)))
		return -EIO;

	/*
	 * endian is ture, buf[0]: high-8bit data, buf[1]: low-8bit data
	 * endian is false, buf[0]: low-8bit data, buf[1]: high-8bit data
	 */
	if (endian)
		*data = (buf[1] | (buf[0] << 8));
	else
		*data = (buf[0] | (buf[1] << 8));
	return 0;
}

int power_i2c_u16_read_byte(struct i2c_client *client, u16 reg, u8 *data)
{
	u8 cmd[LEN_U16_REG] = { 0 };
	u8 buf[LEN_U8_DAT] = { 0 };

	/* cmd[0]: high-8bit register, cmd[1]: low-8bit register */
	cmd[0] = reg >> 8;
	cmd[1] = reg & 0xff;

	if (power_i2c_read_block(client, cmd, sizeof(cmd), buf, sizeof(buf)))
		return -EIO;

	/* buf[0]: 8bit data */
	*data = buf[0];
	return 0;
}

int power_i2c_u16_read_word(struct i2c_client *client, u16 reg, u16 *data, bool endian)
{
	u8 cmd[LEN_U16_REG] = { 0 };
	u8 buf[LEN_U16_DAT] = { 0 };

	/* cmd[0]: high-8bit register, cmd[1]: low-8bit register */
	cmd[0] = reg >> 8;
	cmd[1] = reg & 0xff;

	if (power_i2c_read_block(client, cmd, sizeof(cmd), buf, sizeof(buf)))
		return -EIO;

	/*
	 * endian is ture, buf[0]: high-8bit data, buf[1]: low-8bit data
	 * endian is false, buf[0]: low-8bit data, buf[1]: high-8bit data
	 */
	if (endian)
		*data = (buf[1] | (buf[0] << 8));
	else
		*data = (buf[0] | (buf[1] << 8));
	return 0;
}

int power_i2c_u8_write_byte_mask(struct i2c_client *client, u8 reg, u8 data,
	u8 mask, u8 shift)
{
	u8 tmp_data = 0;

	if (power_i2c_u8_read_byte(client, reg, &tmp_data))
		return -EIO;

	tmp_data &= ~mask;
	tmp_data |= ((data << shift) & mask);

	return power_i2c_u8_write_byte(client, reg, tmp_data);
}

int power_i2c_u8_read_byte_mask(struct i2c_client *client, u8 reg, u8 *data,
	u8 mask, u8 shift)
{
	u8 tmp_data = 0;

	if (power_i2c_u8_read_byte(client, reg, &tmp_data))
		return -EIO;

	tmp_data &= mask;
	tmp_data >>= shift;
	*data = tmp_data;

	return 0;
}
