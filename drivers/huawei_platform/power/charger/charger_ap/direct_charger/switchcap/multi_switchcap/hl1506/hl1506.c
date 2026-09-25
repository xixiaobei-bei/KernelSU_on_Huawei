/*
 * hl1506.c
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
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/multi_switchcap.h>
#include "hl1506.h"

#define HWLOG_TAG hl1506
HWLOG_REGIST();

static int hl1506_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_length, u8 *read_data, int read_cnt)
{
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

	ret = i2c_transfer(client->adapter, msg, I2C_RD_MSG_LEN);

	if (ret != I2C_RD_MSG_LEN) {
		hwlog_err("%s: fail\n", __func__);
		return -EIO;
	}

	return 0;
}

static int hl1506_i2c_write(struct i2c_client *client,
	u8 *cmd, int cmd_length)
{
	int ret;
	struct i2c_msg msg[I2C_WR_MSG_LEN];

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	ret = i2c_transfer(client->adapter, msg, I2C_WR_MSG_LEN);

	if (ret != I2C_WR_MSG_LEN) {
		hwlog_err("%s: fail\n", __func__);
		return -EIO;
	}

	return 0;
}

static int hl1506_read_block(struct hl1506_device_info *di,
	u8 reg, u8 *data, u8 len)
{
	return hl1506_i2c_read(di->client, &reg, HL1506_ADDR_LEN, data, len);
}

static int hl1506_write_block(struct hl1506_device_info *di,
	u8 reg, u8 *data, u8 data_len)
{
	u8 cmd[HL1506_ADDR_LEN + data_len];

	cmd[0] = reg;
	memcpy(&cmd[HL1506_ADDR_LEN], data, data_len);

	return hl1506_i2c_write(di->client, cmd, HL1506_ADDR_LEN + data_len);
}

static int hl1506_read_byte(struct hl1506_device_info *di, u8 reg, u8 *data)
{
	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return hl1506_read_block(di, reg, data, BYTE_LEN);
}

static int hl1506_write_byte(struct hl1506_device_info *di, u8 reg, u8 data)
{
	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return hl1506_write_block(di, reg, &data, BYTE_LEN);
}

static int hl1506_read_mask(struct hl1506_device_info *di,
	u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = hl1506_read_byte(di, reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return ret;
}

static int hl1506_vbst_5v_ctl(struct hl1506_device_info *di, int enable)
{
	int ret;
	int value = enable ? 1 : 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_wchg_5v_en, value);

	ret = boost_5v_enable(value, BOOST_CTRL_SC_CHIP);
	if (ret)
		return -1;
	hwlog_info("dev_id[%d] ctl vbst_5v[%d]\n", di->dev_id, value);

	return 0;
}

static void hl1506_dump_register(struct hl1506_device_info *di)
{
	int ret;
	u8 i;
	u8 val = 0;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	for (i = 0; i <= HL1506_MAX_REGS; i++) {
		ret = hl1506_read_byte(di, i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");

		hwlog_info("dev_id[%d]reg[%x]=0x%x\n",
			di->dev_id, i, val);
	}
}

static bool hl1506_is_cp_open(struct hl1506_device_info *di)
{
	int ret;
	u8 status = 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = hl1506_read_mask(di, HL1506_REG_04, HL1506_04_BPCP_MODE_MASK,
		HL1506_04_BPCP_MODE_SHIFT, &status);
	if (!ret && !status)
		return true;

	return false;
}

static int hl1506_is_device_close(void *data)
{
	int gpio_value;
	struct hl1506_device_info *di = (struct hl1506_device_info *)data;

	if (!di) {
		hwlog_err("di is null\n");
		return 1;
	}

	gpio_value = gpio_get_value(di->gpio_en);

	return (gpio_value == HL1506_CHIP_ENABLE) ? 0 : 1;
}

static int hl1506_get_device_id(void *data)
{
	return SWITCHCAP_MULTI_SC;
}

static int hl1506_charge_init(void *data)
{
	int ret = 0;
	struct hl1506_device_info *di = (struct hl1506_device_info *)data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* PMID Vlimit MAX */
	ret |= hl1506_write_byte(di, HL1506_REG_00, HL1506_00_INIT_VAL);

	/* ilimit 1.5A, Force CP */
	ret |= hl1506_write_byte(di, HL1506_REG_01, HL1506_01_INIT_VAL);

	/* cp-ck 600kHz */
	ret |= hl1506_write_byte(di, HL1506_REG_02, HL1506_02_INIT_VAL);

	/* host enabel, PMID ov 13V */
	ret |= hl1506_write_byte(di, HL1506_REG_03, HL1506_03_INIT_VAL);

	if (ret) {
		hwlog_info("%s failed\n", __func__);
		return -1;
	}

	hl1506_dump_register(di);

	return 0;
}

static int hl1506_charge_enable(void *data, int enable)
{
	int gpio_value;
	int value = enable ? 1 : 0;
	bool status = false;
	struct hl1506_device_info *di = (struct hl1506_device_info *)data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	status = hl1506_is_cp_open(di);
	if (!status) {
		hwlog_err("is not cp mode\n");
		return -1;
	}

	gpio_value = gpio_get_value(di->gpio_en) ? 1 : 0;
	if (value ^ gpio_value) {
		hwlog_info("dev_id[%d] set gpio_en=%d\n", di->dev_id, value);
		gpio_set_value(di->gpio_en, value);
	}

	return 0;
}

static int hl1506_charge_exit(void *data)
{
	int ret;
	struct hl1506_device_info *di = (struct hl1506_device_info *)data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = hl1506_charge_enable(di, HL1506_CHIP_DISABLE);
	if (ret) {
		hwlog_err("charge_exit failed\n");
		return -1;
	}

	ret = hl1506_write_byte(di, HL1506_REG_03, HL1506_03_DISABLE_CHIP);
	if (ret) {
		hwlog_err("disable chip failed\n");
		return -1;
	}

	return 0;
}

static int hl1506_chip_power(void *data, bool enable)
{
	struct hl1506_device_info *di = (struct hl1506_device_info *)data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return hl1506_vbst_5v_ctl(di, enable);
}

static struct multi_sc_ops hl1506_ops = {
	.sc_init = hl1506_charge_init,
	.sc_enable = hl1506_charge_enable,
	.is_sc_close = hl1506_is_device_close,
	.get_sc_id = hl1506_get_device_id,
	.sc_exit = hl1506_charge_exit,
	.sc_chip_power = hl1506_chip_power,
};

static int hl1506_device_check(struct hl1506_device_info *di)
{
	int ret;
	u8 val = 0;

	/* supply power for chip */
	ret = hl1506_vbst_5v_ctl(di, HL1506_5V_ENABLE);

	/* sleep 10ms for chip ready */
	usleep_range(10000, 11000);

	/* check chip is on board */
	ret |= hl1506_read_byte(di, HL1506_REG_04, &val);

	/* close power for chip */
	ret |= hl1506_vbst_5v_ctl(di, HL1506_5V_DISABLE);
	if (ret)
		return -EINVAL;

	hl1506_dump_register(di);

	return 0;
}

static int hl1506_prase_dts(struct hl1506_device_info *di,
	struct device_node *np)
{
	int ret;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "dev_id",
		&di->dev_id, HL1506_INVALID_ID))
		return -EINVAL;

	di->gpio_wchg_5v_en = of_get_named_gpio(np, "gpio_wchg_5v_en", 0);
	hwlog_info("gpio_wchg_5v_en=%d\n", di->gpio_wchg_5v_en);

	if (!gpio_is_valid(di->gpio_wchg_5v_en)) {
		hwlog_err("gpio_wchg_5v_en is not valid\n");
		return -EINVAL;
	}

	ret = gpio_request(di->gpio_wchg_5v_en, "gpio_wchg_5v_en");
	if (ret)
		hwlog_err("gpio request fail\n");

	ret = gpio_direction_output(di->gpio_wchg_5v_en, HL1506_5V_DISABLE);
	if (ret) {
		hwlog_err("gpio_wchg_5v_en set output fail\n");
		goto gpio_fail_0;
	}

	di->gpio_en = of_get_named_gpio(np, "gpio_en", 0);
	hwlog_info("gpio_en=%d\n", di->gpio_en);

	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto gpio_fail_0;
	}

	ret = gpio_request(di->gpio_en, "hl1506_gpio_en");
	if (ret)
		hwlog_err("gpio request fail\n");

	ret = gpio_direction_output(di->gpio_en, HL1506_CHIP_DISABLE);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto gpio_fail_1;
	}
	return 0;

gpio_fail_1:
	gpio_free(di->gpio_en);
gpio_fail_0:
	gpio_free(di->gpio_wchg_5v_en);
	return ret;
}

static int hl1506_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct hl1506_device_info *di = NULL;
	struct multi_sc_chip_info *chip_info = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	chip_info = devm_kzalloc(&client->dev, sizeof(*chip_info), GFP_KERNEL);
	if (!chip_info)
		return -ENOMEM;

	di->dev = &client->dev;
	di->client = client;
	np = di->dev->of_node;
	i2c_set_clientdata(client, di);
	chip_info->data = di;
	chip_info->ops =
		(struct multi_sc_ops *)of_device_get_match_data(di->dev);

	if (hl1506_prase_dts(di, np))
		return -EINVAL;

	chip_info->dev_id = di->dev_id;

	ret = hl1506_device_check(di);
	if (ret) {
		hwlog_err("hl1506 device check failed\n");
		goto prase_dts_fail;
	}

	ret = multi_switchcap_chip_register(chip_info);
	if (ret) {
		hwlog_err("multi_switchcap_ops_register fail\n");
		goto prase_dts_fail;
	}
	hwlog_info("probe end\n");
	return 0;

prase_dts_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_wchg_5v_en);
	return ret;
}

static int hl1506_remove(struct i2c_client *client)
{
	struct hl1506_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	if (di->gpio_en)
		gpio_free(di->gpio_en);

	if (di->gpio_wchg_5v_en)
		gpio_free(di->gpio_wchg_5v_en);

	devm_kfree(&client->dev, di);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, hl1506);
static const struct of_device_id hl1506_of_match[] = {
	{ .compatible = "hl1506_id_0", .data = &hl1506_ops },
	{ .compatible = "hl1506_id_1", .data = &hl1506_ops },
	{ .compatible = "hl1506_id_2", .data = &hl1506_ops },
	{},
};

static const struct i2c_device_id hl1506_i2c_id[] = {
	{ "hl1506_id_0", 0 },
	{ "hl1506_id_1", 1 },
	{ "hl1506_id_2", 2 },
};

static struct i2c_driver hl1506_driver = {
	.probe = hl1506_probe,
	.remove = hl1506_remove,
	.id_table = hl1506_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hl1506",
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

module_init(hl1506_init);
module_exit(hl1506_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl1506 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
