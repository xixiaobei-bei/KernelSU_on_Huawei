/*
 * fpf2283.c
 *
 * vsys ovp switch fpf2283 driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <huawei_platform/power/vsys_switch/vsys_switch.h>
#include "fpf2283.h"

#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif

#ifdef CONFIG_HUAWEI_POWER_DEBUG
#include <chipset_common/hwpower/common_module/power_debug.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG fpf2283_vsys
HWLOG_REGIST();

struct fpf2283_device_info {
	struct device *dev;
	struct i2c_client *client;
	int chip_already_init;
	int get_id_time;
	int device_id;
	int gpio_en;
	int i2c_disabled;
	int state;
};

static struct fpf2283_device_info *g_fpf2283_dev;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static int fpf2283_write_byte(u8 reg, u8 value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di || di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int fpf2283_read_byte(u8 reg, u8 *value)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di || di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
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

static int fpf2283_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = fpf2283_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
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
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int fpf2283_get_device_id(int *id)
{
	u8 id_info = 0;
	int ret;
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di || !id) {
		hwlog_err("di or id is null\n");
		return -1;
	}

	if (di->i2c_disabled) {
		di->device_id = VSYS_OVP_SWITCH_FPF2283;
		*id = di->device_id;
		return 0;
	}

	if (di->get_id_time == FPF2283_USED) {
		*id = di->device_id;
		return 0;
	}

	di->get_id_time = FPF2283_USED;
	ret = fpf2283_read_byte(FPF2283_ID_REG, &id_info);
	if (ret) {
		di->get_id_time = FPF2283_NOT_USED;
		return -1;
	}
	hwlog_info("get_device_id [%x]=0x%x\n", FPF2283_ID_REG, id_info);

	id_info = id_info & FPF2283_ID_VID_MASK;
	switch (id_info) {
	case FPF2283_DEVICE_ID_FPF2283:
		di->device_id = VSYS_OVP_SWITCH_FPF2283;
		break;
	default:
		di->device_id = -1;
		ret = -1;
		hwlog_err("ovp switch get dev_id fail\n");
		break;
	}

	*id = di->device_id;
	return ret;
}

static int fpf2283_set_state(int enable)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_en) {
		hwlog_err("no device\n");
		return -ENODEV;
	}

	enable = !!enable;

	if (di->state == enable) {
		hwlog_info("set_state state is already %d\n", enable);
		return 0;
	}

	gpio_direction_output(di->gpio_en, enable);
	di->chip_already_init = enable ? 1 : 0;
	di->state = enable;
	hwlog_info("set_state: set gpio_en %d\n", enable);

	return 0;
}

static int fpf2283_get_state(void)
{
	struct fpf2283_device_info *di = g_fpf2283_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return di->state;
}

static void fpf2283_para_init(struct fpf2283_device_info *di)
{
	di->chip_already_init = 0;
	di->state = FPF2283_CHIP_DISABLE;
	di->get_id_time = FPF2283_NOT_USED;
}

static struct vsys_ovp_switch_device_ops fpf2283_ops = {
	.chip_name = "fpf2283",
	.set_state = fpf2283_set_state,
	.get_state = fpf2283_get_state,
	.get_id = fpf2283_get_device_id,
};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t fpf2283_dbg_show_reg_value(void *dev_data,
	char *buf, size_t size)
{
	u8 val;
	int ret;
	int i;
	char rd_buf[FPF2283_RD_BUF_SIZE] = {0};
	struct fpf2283_device_info *dev_p = dev_data;

	if (!buf || !dev_p) {
		hwlog_err("buf or dev_p is null\n");
		return scnprintf(buf, size, "buf or dev_p is null\n");
	}

	for (i = 0; i < FPF2283_MAX_REGS; i++) {
		val = 0;
		memset(rd_buf, 0, FPF2283_RD_BUF_SIZE);
		ret = fpf2283_read_byte(i, &val);
		if (ret == 0)
			scnprintf(rd_buf, FPF2283_RD_BUF_SIZE,
				"regaddr: 0x%x regval: 0x%x\n", i, val);
		else
			scnprintf(rd_buf, FPF2283_RD_BUF_SIZE,
				"regaddr: 0x%x regval: invalid\n", i);

		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static ssize_t fpf2283_dbg_store_reg_value(void *dev_data,
	const char *buf, size_t size)
{
	int regaddr = 0;
	int regval = 0;
	int ret;
	struct fpf2283_device_info *dev_p = dev_data;

	if (!buf || !dev_p) {
		hwlog_err("buf or dev_p is null\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%d %d", &regaddr, &regval) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	/* maximum value of 8-bit num is 255 */
	if (regaddr < 0 || regaddr >= FPF2283_MAX_REGS ||
		regval < 0 || regval > 255) {
		hwlog_err("regaddr 0x%x or regval 0x%x invalid\n",
			regaddr, regval);
		return -EINVAL;
	}

	ret = fpf2283_write_byte((u8)regaddr, (u8)regval);
	if (ret)
		return -EINVAL;

	hwlog_info("regaddr 0x%x regval 0x%x succeed\n", regaddr, regval);

	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int fpf2283_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct fpf2283_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_fpf2283_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_disabled", &di->i2c_disabled, 0);

	ret = power_gpio_config_output(np,
		"gpio_en", "gpio_en", &di->gpio_en, FPF2283_CHIP_DISABLE);
	if (ret)
		goto fpf2283_fail_0;

	fpf2283_para_init(di);

	ret = vsys_ovp_switch_ops_register(&fpf2283_ops);
	if (ret)
		goto fpf2283_fail_1;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("fpf2283_regval", i2c_get_clientdata(client),
		(power_dbg_show)fpf2283_dbg_show_reg_value,
		(power_dbg_store)fpf2283_dbg_store_reg_value);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

	return 0;

fpf2283_fail_1:
	gpio_free(di->gpio_en);
fpf2283_fail_0:
	devm_kfree(&client->dev, di);
	g_fpf2283_dev = NULL;

	return ret;
}

static int fpf2283_remove(struct i2c_client *client)
{
	struct fpf2283_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	/* reset fpf2283 */
	gpio_set_value(di->gpio_en, FPF2283_CHIP_DISABLE);

	if (di->gpio_en)
		gpio_free(di->gpio_en);

	g_fpf2283_dev = NULL;

	return 0;
}

MODULE_DEVICE_TABLE(i2c, fpf2283);
static const struct of_device_id fpf2283_of_match[] = {
	{
		.compatible = "huawei,fpf2283_vsys",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id fpf2283_i2c_id[] = {
	{ "fpf2283_vsys", 0 }, {}
};

static struct i2c_driver fpf2283_driver = {
	.probe = fpf2283_probe,
	.remove = fpf2283_remove,
	.id_table = fpf2283_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei,fpf2283_vsys",
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
MODULE_DESCRIPTION("fpf2283_vsys module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
