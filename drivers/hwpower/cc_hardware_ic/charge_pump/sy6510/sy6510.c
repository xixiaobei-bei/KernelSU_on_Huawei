/*
 * sy6510.c
 *
 * charge-pump sy6510 driver
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

#include "sy6510.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cp_sy6510
HWLOG_REGIST();

static struct sy6510_dev_info *g_sy6510_di;

static int sy6510_read_byte(u8 reg, u8 *data)
{
	struct sy6510_dev_info *di = g_sy6510_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_read_byte(di->client, reg, data))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int sy6510_write_byte(u8 reg, u8 data)
{
	struct sy6510_dev_info *di = g_sy6510_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_write_byte(di->client, reg, data))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -EIO;
}

static int sy6510_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = sy6510_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return sy6510_write_byte(reg, val);
}

static int sy6510_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = sy6510_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return ret;
}

static int sy6510_i2c_init(void)
{
	int ret;
	u8 status = 0;

	ret = sy6510_read_mask(SY6510_REG_02, SY6510_02_POWER_READY_MASK,
		SY6510_02_POWER_READY_SHIFT, &status);
	hwlog_info("status=%d, ret=%d\n", status, ret);
	return ret;
}

static int sy6510_device_check(void *dev_data)
{
	int ret;
	struct sy6510_dev_info *di = g_sy6510_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = sy6510_i2c_init();
	if (ret) {
		hwlog_err("i2c init failed\n");
		return ret;
	}
	return 0;
}

static int sy6510_chip_init(void *dev_data)
{
	int ret = 0;

	/* enable */
	ret += sy6510_write_byte(SY6510_REG_00, SY6510_00_INIT_VAL);
	/* config ocp\ovp\bp\SCP threshold */
	ret += sy6510_write_byte(SY6510_REG_01, SY6510_01_INIT_VAL);

	return ret;
}

static int sy6510_set_bp_mode(void *dev_data)
{
	return sy6510_write_mask(SY6510_REG_00, SY6510_01_FORCE_BPCP_MASK,
		SY6510_01_FORCE_BPCP_SHIFT, SY6510_01_FORCE_BP_EN);
}

static int sy6510_set_cp_mode(void *dev_data)
{
	return sy6510_write_mask(SY6510_REG_00, SY6510_01_FORCE_BPCP_MASK,
		SY6510_01_FORCE_BPCP_SHIFT, SY6510_01_FORCE_CP_EN);
}

static bool sy6510_is_cp_open(void *dev_data)
{
	int ret;
	u8 status = 0;

	ret = sy6510_read_mask(SY6510_REG_00, SY6510_01_FORCE_BPCP_MASK,
		SY6510_01_FORCE_BPCP_SHIFT, &status);
	if (!ret && !status)
		return true;

	return false;
}

static bool sy6510_is_bp_open(void *dev_data)
{
	int ret;
	u8 status = 0;

	ret = sy6510_read_mask(SY6510_REG_00, SY6510_01_FORCE_BPCP_MASK,
		SY6510_01_FORCE_BPCP_SHIFT, &status);
	if (!ret && status)
		return true;

	return false;
}

static int sy6510_post_probe(void *dev_data)
{
	struct sy6510_dev_info *di = g_sy6510_di;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	return 0;
}

static int sy6510_reverse_chip_init(void *dev_data)
{
	return sy6510_write_mask(SY6510_REG_03, SY6510_03_REVERSE_BP_MASK,
		SY6510_03_REVERSE_BP_SHIFT, SY6510_03_REVERSE_BP_EN);
}

static struct charge_pump_ops sy6510_ops = {
	.cp_type = CP_TYPE_MAIN,
	.chip_name = "sy6510",
	.dev_check = sy6510_device_check,
	.post_probe = sy6510_post_probe,
	.chip_init = sy6510_chip_init,
	.set_bp_mode = sy6510_set_bp_mode,
	.set_cp_mode = sy6510_set_cp_mode,
	.is_cp_open = sy6510_is_cp_open,
	.is_bp_open = sy6510_is_bp_open,
	.rvs_chip_init = sy6510_reverse_chip_init,
};

static int sy6510_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct sy6510_dev_info *di = NULL;

	if (!client)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_sy6510_di = di;
	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);

	sy6510_ops.dev_data = (void *)di;
	ret = charge_pump_ops_register(&sy6510_ops);
	if (ret)
		goto ops_register_fail;

	return 0;

ops_register_fail:
	i2c_set_clientdata(client, NULL);
	devm_kfree(&client->dev, di);
	g_sy6510_di = NULL;
	return ret;
}

static int sy6510_remove(struct i2c_client *client)
{
	struct sy6510_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	i2c_set_clientdata(client, NULL);
	g_sy6510_di = NULL;
	return 0;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_sy6510);
static const struct of_device_id sy6510_of_match[] = {
	{
		.compatible = "charge_pump_sy6510",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sy6510_i2c_id[] = {
	{ "charge_pump_sy6510", 0 }, {}
};

static struct i2c_driver sy6510_driver = {
	.probe = sy6510_probe,
	.remove = sy6510_remove,
	.id_table = sy6510_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "charge_pump_sy6510",
		.of_match_table = of_match_ptr(sy6510_of_match),
	},
};

static int __init sy6510_init(void)
{
	return i2c_add_driver(&sy6510_driver);
}

static void __exit sy6510_exit(void)
{
	i2c_del_driver(&sy6510_driver);
}

rootfs_initcall(sy6510_init);
module_exit(sy6510_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sy6510 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
