/*
 * fpf2283.c
 *
 * fpf2283 driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/multi_switchcap.h>
#include "fpf2283.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG fpf2283_sc
HWLOG_REGIST();

static struct fpf2283_device_info *g_fpf2283_dev;

static int fpf2283_write_byte(u8 reg, u8 value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int fpf2283_read_byte(u8 reg, u8 *value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

static int fpf2283_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = fpf2283_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return fpf2283_write_byte(reg, val);
}

static void fpf2283_dump_register(void)
{
	int ret;
	u8 i;
	u8 val = 0;

	for (i = 0; i < FPF2283_MAX_REGS; ++i) {
		ret = fpf2283_read_byte(i, &val);
		if (ret)
			hwlog_err("dump_register read fail\n");

		hwlog_info("reg [%x]=0x%x\n", i, val);
	}
}

static int fpf2283_check_i2c(struct fpf2283_device_info *di)
{
	u8 val = 0;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, FPF2283_ID_REG, &val);
}

static int fpf2283_reg_init(void)
{
	int ret;
	unsigned int value = 0;

	ret = fpf2283_write_byte(FPF2283_ENABLE_REG, FPF2283_ENABLE_INIT);
	ret |= fpf2283_write_byte(FPF2283_INTERRUPT_MASK_REG,
		FPF2283_INTERRUPT_MASK_INIT);

	/* fpf2283 level: offset 0mv, center value 23v */
	value |= ((FPF2283_OVP_OFFSET_0MV <<
		FPF2283_OVP_OFFSET_SHIFT) & FPF2283_OVP_OFFSET_MASK);
	value |= (FPF2283_OVP_CENTER_VALUE_23000MV <<
		FPF2283_OVP_CENTER_VALUE_SHIFT) & FPF2283_OVP_CENTER_VALUE_MASK;
	ret |= fpf2283_write_byte(FPF2283_OVP_REG, value);
	if (ret)
		return -1;

	fpf2283_dump_register();

	return 0;
}

static int fpf2283_charge_enable(int enable)
{
	int ret;
	int value = enable ? 1 : 0;

	if (!g_fpf2283_dev)
		return -1;

	if (value) {
		ret = fpf2283_reg_init();
		/* set OV mode to internal for ENABLE */
		ret |= fpf2283_write_mask(FPF2283_OVP_REG,
			FPF2283_OVP_OV_MODE_MASK, FPF2283_OVP_OV_MODE_SHIFT,
			value);
		if (ret)
			return -1;

		fpf2283_dump_register();

		return 0;
	}

	fpf2283_write_mask(FPF2283_OVP_REG,
		FPF2283_OVP_OV_MODE_MASK, FPF2283_OVP_OV_MODE_SHIFT,
		value);
	fpf2283_write_mask(FPF2283_ENABLE_REG,
		FPF2283_ENABLE_SW_ENB_MASK, FPF2283_ENABLE_SW_ENB_SHIFT,
		!value ? 1 : 0);

	return 0;
}

static struct ovp_ops fpf2283_ops = {
	.ovp_enable = fpf2283_charge_enable,
};

static int fpf2283_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct fpf2283_device_info *di = NULL;
	struct device_node *np = NULL;

	if (g_fpf2283_dev) {
		hwlog_err("driver already matched\n");
		return -ENODEV;
	}

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	di->dev_name = (char *)of_device_get_match_data(di->dev);
	hwlog_info("device is %s\n", di->dev_name);

	if (fpf2283_check_i2c(di)) {
		hwlog_info("i2c error, ignore this probe\n");
		return -ENODEV;
	}
	g_fpf2283_dev = di;

	ret = ovp_for_sc_ops_register(&fpf2283_ops);
	if (ret) {
		hwlog_err("%s register failed\n", di->dev_name);
		return -EINVAL;
	}

	return 0;
}

static int fpf2283_remove(struct i2c_client *client)
{
	struct fpf2283_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	return 0;
}

MODULE_DEVICE_TABLE(i2c, fpf2283);
static const struct of_device_id fpf2283_of_match[] = {
	{
		.compatible = "fpf2283",
		.data = "fpf2283",
	},
	{
		.compatible = "nxp6093a",
		.data = "nxp6093a",
	},
	{},
};

static const struct i2c_device_id fpf2283_i2c_id[] = {
	{ "fpf2283", 0 },
	{ "nxp6093a", 1 },
};

static struct i2c_driver fpf2283_driver = {
	.probe = fpf2283_probe,
	.remove = fpf2283_remove,
	.id_table = fpf2283_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "fpf2283",
		.of_match_table = of_match_ptr(fpf2283_of_match),
	},
};

static int __init fpf2283_init(void)
{
	return i2c_add_driver(&fpf2283_driver);
}

static void __exit fpf2283_exit(void)
{
	i2c_del_driver(&fpf2283_driver);
}

module_init(fpf2283_init);
module_exit(fpf2283_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fpf2283 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
