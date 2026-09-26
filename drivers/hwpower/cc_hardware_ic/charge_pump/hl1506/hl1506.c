/*
 * hl1506.c
 *
 * charge-pump hl1506 driver
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

#include "hl1506.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cp_hl1506
HWLOG_REGIST();

static int hl1506_read_byte(struct i2c_client *client, u8 reg, u8 *data)
{
	int i;

	if (!client) {
		hwlog_err("client null\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_read_byte(client, reg, data))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int hl1506_write_byte(struct i2c_client *client, u8 reg, u8 data)
{
	int i;

	if (!client) {
		hwlog_err("client null\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_write_byte(client, reg, data))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int hl1506_write_mask(struct i2c_client *client, u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hl1506_read_byte(client, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return hl1506_write_byte(client, reg, val);
}

static int hl1506_read_mask(struct i2c_client *client, u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = hl1506_read_byte(client, reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return ret;
}

static int hl1506_i2c_init(struct i2c_client *client)
{
	int ret;

	/* i2c init */
	ret = hl1506_write_byte(client, HL1506_I2C_INIT_REG, HL1506_I2C_INIT_CLR);
	usleep_range(500, 1500); /* 1ms */
	ret += hl1506_write_byte(client, HL1506_I2C_INIT_REG, HL1506_I2C_INIT_RST);

	return ret;
}

static int hl1506_chip_init(void *dev_data)
{
	int ret = 0;
	u8 val = HL1506_01_INIT_VAL;
	struct hl1506_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (di->cp_type & CP_TYPE_AUX)
		val = HL1506_01_INIT_AUX_VAL;

	/* i2c init */
	if (di->cp_type & CP_TYPE_MAIN)
		ret = hl1506_i2c_init(di->client);
	/* ilimit 1.5a, force bypass or cp */
	ret += hl1506_write_byte(di->client, HL1506_REG_01, val);
	/* cp-ck 500khz */
	ret += hl1506_write_byte(di->client, HL1506_REG_02, HL1506_02_INIT_VAL);
	/* host enable, pmid ov 11v */
	ret += hl1506_write_byte(di->client, HL1506_REG_03, HL1506_03_INIT_VAL);

	return ret;
}

static int hl1506_set_bp_mode(void *dev_data)
{
	int ret;
	struct hl1506_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = hl1506_write_mask(di->client, HL1506_REG_01,
		HL1506_01_FORCE_CP_MASK, HL1506_01_FORCE_CP_SHIFT, HL1506_01_FORCE_CP_DIS);
	ret += hl1506_write_mask(di->client, HL1506_REG_01,
		HL1506_01_FORCE_BP_MASK, HL1506_01_FORCE_BP_SHIFT, HL1506_01_FORCE_BP_EN);

	return ret;
}

static int hl1506_set_cp_mode(void *dev_data)
{
	struct hl1506_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return hl1506_write_mask(di->client, HL1506_REG_01,
		HL1506_01_FORCE_CP_MASK, HL1506_01_FORCE_CP_SHIFT,
		HL1506_01_FORCE_CP_EN);
}

static bool hl1506_is_cp_open(void *dev_data)
{
	int ret;
	u8 status = 0;
	struct hl1506_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	ret = hl1506_read_mask(di->client, HL1506_REG_04,
		HL1506_04_BPCP_MODE_MASK, HL1506_04_BPCP_MODE_SHIFT, &status);
	if (!ret && !status)
		return true;

	return false;
}

static bool hl1506_is_bp_open(void *dev_data)
{
	int ret;
	u8 status = 0;
	struct hl1506_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	ret = hl1506_read_mask(di->client, HL1506_REG_04,
		HL1506_04_BPCP_MODE_MASK, HL1506_04_BPCP_MODE_SHIFT, &status);
	if (!ret && status)
		return true;

	return false;
}

static int hl1506_reverse_cp_chip_init(void *dev_data)
{
	int ret;

	ret = hl1506_chip_init(dev_data);
	msleep(50); /* delay 50ms for chip init */
	ret += hl1506_set_cp_mode(dev_data);

	return ret;
}

static struct charge_pump_ops g_hl1506_ops_main = {
	.chip_name = "hl1506_main",
	.chip_init = hl1506_chip_init,
	.rvs_cp_chip_init = hl1506_reverse_cp_chip_init,
	.set_bp_mode = hl1506_set_bp_mode,
	.set_cp_mode = hl1506_set_cp_mode,
	.is_cp_open = hl1506_is_cp_open,
	.is_bp_open = hl1506_is_bp_open,
};

static struct charge_pump_ops g_hl1506_ops_aux = {
	.chip_name = "hl1506_aux",
	.chip_init = hl1506_chip_init,
	.set_bp_mode = hl1506_set_bp_mode,
	.set_cp_mode = hl1506_set_cp_mode,
	.is_cp_open = hl1506_is_cp_open,
	.is_bp_open = hl1506_is_bp_open,
};

static int hl1506_device_match(struct hl1506_dev_info *di, struct device_node *np)
{
	int ret;
	struct charge_pump_ops *ops = NULL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "cp_type",
		(u32 *)&di->cp_type, CP_TYPE_MAIN);

	if (!(di->cp_type & CP_OPS_BYPASS)) {
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, true);
		power_usleep(DT_USLEEP_10MS);
		ret = hl1506_i2c_init(di->client);
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
		if (ret)
			return ret;
		di->cp_type |= CP_OPS_BYPASS;
		g_hl1506_ops_main.dev_data = (void *)di;
		ops = &g_hl1506_ops_main;
	} else {
		if (di->cp_type & CP_TYPE_MAIN) {
			g_hl1506_ops_main.dev_data = (void *)di;
			ops = &g_hl1506_ops_main;
		} else if (di->cp_type & CP_TYPE_AUX) {
			g_hl1506_ops_aux.dev_data = (void *)di;
			ops = &g_hl1506_ops_aux;
		}
	}

	if (!ops) {
		hwlog_err("device_match: failed\n");
		return -1;
	}

	ops->cp_type = di->cp_type;
	return charge_pump_ops_register(ops);
}

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

	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);

	ret = hl1506_device_match(di, di->client->dev.of_node);
	if (ret) {
		hwlog_err("device mismatch\n");
		goto dev_check_fail;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	return 0;

dev_check_fail:
	i2c_set_clientdata(client, NULL);
	devm_kfree(&client->dev, di);
	return ret;
}

static int hl1506_remove(struct i2c_client *client)
{
	struct hl1506_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	i2c_set_clientdata(client, NULL);
	return 0;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_hl1506);
static const struct of_device_id hl1506_of_match[] = {
	{
		.compatible = "charge_pump_hl1506",
		.data = NULL,
	},
	{
		.compatible = "charge_pump_hl1506_aux",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id hl1506_i2c_id[] = {
	{ "cp_hl1506", 0 },
	{ "cp_hl1506_aux", 1 },
	{},
};

static struct i2c_driver hl1506_driver = {
	.probe = hl1506_probe,
	.remove = hl1506_remove,
	.id_table = hl1506_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "charge_pump_hl1506",
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
