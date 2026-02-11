/*
 * cp_hl1506.c
 *
 * charge-pump hl1506 driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/power_devices_info.h>

#include "../stwlc68/stwlc68.h"
#include "cp_hl1506.h"

#define HWLOG_TAG wireless_cp_hl1506
HWLOG_REGIST();

static struct hl1506_dev_info *g_hl1506_di;

static int hl1506_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_length, u8 *read_data, int read_cnt)
{
	int i;
	int ret;
	struct i2c_msg msg[I2C_RD_MSG_LEN];

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	msg[1].addr = client->addr;
	msg[1].buf = read_data;
	msg[1].len = read_cnt;
	msg[1].flags = I2C_M_RD;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(client->adapter, msg, I2C_RD_MSG_LEN);
		if (ret == I2C_RD_MSG_LEN)
			break;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret != I2C_RD_MSG_LEN) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static int hl1506_i2c_write(struct i2c_client *client,
	u8 *cmd, int cmd_length)
{
	int i;
	int ret;
	struct i2c_msg msg[I2C_WR_MSG_LEN];

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(client->adapter, msg, I2C_WR_MSG_LEN);
		if (ret == I2C_WR_MSG_LEN)
			break;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret != I2C_WR_MSG_LEN) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int hl1506_read_block(struct hl1506_dev_info *di,
	u8 reg, u8 *data, u8 len)
{
	return hl1506_i2c_read(di->client, &reg, HL1506_ADDR_LEN, data, len);
}

static int hl1506_write_block(struct hl1506_dev_info *di,
	u8 reg, u8 *data, u8 data_len)
{
	u8 cmd[HL1506_ADDR_LEN + data_len];

	cmd[0] = reg;
	memcpy(&cmd[HL1506_ADDR_LEN], data, data_len);

	return hl1506_i2c_write(di->client, cmd, HL1506_ADDR_LEN + data_len);
}

static int hl1506_read_byte(u8 reg, u8 *data)
{
	struct hl1506_dev_info *di = g_hl1506_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return hl1506_read_block(di, reg, data, BYTE_LEN);
}

static int hl1506_write_byte(u8 reg, u8 data)
{
	struct hl1506_dev_info *di = g_hl1506_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return hl1506_write_block(di, reg, &data, BYTE_LEN);
}

static int hl1506_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hl1506_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return hl1506_write_byte(reg, val);
}

static int hl1506_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = hl1506_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return ret;
}

static int hl1506_i2c_init(void)
{
	int ret;

	/* i2c init */
	ret = hl1506_write_byte(HL1506_I2C_INIT_REG, HL1506_I2C_INIT_CLR);
	usleep_range(500, 1500); /* 1ms */
	ret += hl1506_write_byte(HL1506_I2C_INIT_REG, HL1506_I2C_INIT_RST);

	return ret;
}

static int hl1506_device_match(void)
{
	return hl1506_i2c_init();
}

static int hl1506_chip_init(void)
{
	int ret;

	/* i2c init */
	ret = hl1506_i2c_init();
	/* PMID  Vlimit MAX */
	ret += hl1506_write_byte(HL1506_REG_00, HL1506_00_INIT_VAL);
	/* ilimit 1.5A,  Force ByPass */
	ret += hl1506_write_byte(HL1506_REG_01, HL1506_01_INIT_VAL);
	/* cp-ck 600kHz */
	ret += hl1506_write_byte(HL1506_REG_02, HL1506_02_INIT_VAL);
	/* host enabel, PMID ov 13V */
	ret += hl1506_write_byte(HL1506_REG_03, HL1506_03_INIT_VAL);

	return ret;
}

static int hl1506_set_bp_mode(void)
{
	int ret;

	ret = hl1506_write_mask(HL1506_REG_01, HL1506_01_FORCE_CP_MASK,
		HL1506_01_FORCE_CP_SHIFT, HL1506_01_FORCE_CP_DIS);
	ret += hl1506_write_mask(HL1506_REG_01, HL1506_01_FORCE_BP_MASK,
		HL1506_01_FORCE_BP_SHIFT, HL1506_01_FORCE_BP_EN);

	return ret;
}

static int hl1506_set_cp_mode(void)
{
	return hl1506_write_mask(HL1506_REG_01, HL1506_01_FORCE_CP_MASK,
		HL1506_01_FORCE_CP_SHIFT, HL1506_01_FORCE_CP_EN);
}

static bool hl1506_is_cp_open(void)
{
	int ret;
	u8 status = 0;

	ret = hl1506_read_mask(HL1506_REG_04, HL1506_04_BPCP_MODE_MASK,
		HL1506_04_BPCP_MODE_SHIFT, &status);
	if (!ret && !status)
		return true;

	return false;
}

static bool hl1506_is_bp_open(void)
{
	int ret;
	u8 status = 0;

	ret = hl1506_read_mask(HL1506_REG_04, HL1506_04_BPCP_MODE_MASK,
		HL1506_04_BPCP_MODE_SHIFT, &status);
	if (!ret && status)
		return true;

	return false;
}

static int hl1506_wlchip_ops_register(struct wireless_cp_ops *ops)
{
	return stwlc68_cp_ops_register(ops);
}

static struct wireless_cp_ops hl1506_ops = {
	.chip_init       = hl1506_chip_init,
	.set_bp_mode     = hl1506_set_bp_mode,
	.set_cp_mode     = hl1506_set_cp_mode,
	.is_cp_open      = hl1506_is_cp_open,
	.is_bp_open      = hl1506_is_bp_open,
};

static int hl1506_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct hl1506_dev_info *di = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hl1506_di = di;
	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);

	wlps_control(WLPS_PROBE_PWR, WLPS_CTRL_ON);
	usleep_range(9500, 10500); /* wait 10ms for power supply */
	ret = hl1506_device_match();
	if (ret) {
		wlps_control(WLPS_PROBE_PWR, WLPS_CTRL_OFF);
		hwlog_err("%s: device mismatch\n", __func__);
		goto dev_check_fail;
	}
	wlps_control(WLPS_PROBE_PWR, WLPS_CTRL_OFF);

	ret = hl1506_wlchip_ops_register(&hl1506_ops);
	if (ret) {
		hwlog_err("%s: register ops failed\n", __func__);
		goto ops_regist_fail;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("wireless cp_hl1506 probe ok\n");
	return 0;

ops_regist_fail:
dev_check_fail:
	devm_kfree(&client->dev, di);
	di = NULL;
	g_hl1506_di = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, wireless_cp_hl1506);
static const struct of_device_id hl1506_of_match[] = {
	{
		.compatible = "huawei, wireless_cp_hl1506",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id hl1506_i2c_id[] = {
	{ "wireless_cp_hl1506", 0 }, {}
};

static struct i2c_driver hl1506_driver = {
	.probe = hl1506_probe,
	.id_table = hl1506_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_cp_hl1506",
		.of_match_table = of_match_ptr(hl1506_of_match),
	},
};

static int __init hl1506_init(void)
{
	return i2c_add_driver(&hl1506_driver);
}

static void __exit hl1506_exit(void)
{
	i2c_del_driver(&hl1506_driver);
}

rootfs_initcall(hl1506_init);
module_exit(hl1506_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl1506 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
