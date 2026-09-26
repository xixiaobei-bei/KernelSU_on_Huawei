/*
 * tps63810.c
 *
 * tps63810 driver
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

#include "tps63810.h"
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG tps63810
HWLOG_REGIST();

static struct tps63810_device_info *g_tps63810_di;

static int tps63810_write_byte(u8 reg, u8 val)
{
	struct tps63810_device_info *di = g_tps63810_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < TPS_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_write_byte(di->client, reg, val))
			return 0;
		power_usleep(DT_USLEEP_5MS);
	}

	return -EIO;
}

static int tps63810_read_byte(u8 reg, u8 *val)
{
	struct tps63810_device_info *di = g_tps63810_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < TPS_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_read_byte(di->client, reg, val))
			return 0;
		power_usleep(DT_USLEEP_5MS);
	}

	return -EIO;
}

static int tps63810_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val;

	ret = tps63810_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}

static int tps63810_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = tps63810_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return tps63810_write_byte(reg, val);
}

static int tps63810_set_vout1(u8 vset)
{
	return tps63810_write_byte(TPS63810_VOUT1_ADDR, vset);
}

static int tps63810_set_vout2(u8 vset)
{
	return tps63810_write_byte(TPS63810_VOUT2_ADDR, vset);
}

static int tps63810_set_vout(unsigned int vout, void *dev_data)
{
	u8 vset;
	unsigned int vout_min;
	unsigned int vout_max;
	unsigned int vout_step;
	struct tps63810_device_info *di = g_tps63810_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	switch (di->dev_id) {
	case TPS63810_DEVID_TI:
		if (!di->vsel_state) {
			if (di->vsel_range == TPS63810_LOW_RANGE) {
				vout_min = TPS63810_VOUT1_LR_MIN;
				vout_max = TPS63810_VOUT1_LR_MAX;
			} else {
				vout_min = TPS63810_VOUT1_HR_MIN;
				vout_max = TPS63810_VOUT1_HR_MAX;
			}
			vout_step = TPS63810_VOUT1_STEP;
		} else {
			if (di->vsel_range == TPS63810_LOW_RANGE) {
				vout_min = TPS63810_VOUT2_LR_MIN;
				vout_max = TPS63810_VOUT2_LR_MAX;
			} else {
				vout_min = TPS63810_VOUT2_HR_MIN;
				vout_max = TPS63810_VOUT2_HR_MAX;
			}
			vout_step = TPS63810_VOUT2_STEP;
		}
		break;
	case TPS63810_DEVID_RT:
		if (!di->vsel_state) {
			vout_min = RT6160_VOUT1_MIN;
			vout_max = RT6160_VOUT1_MAX;
			vout_step = RT6160_VOUT1_STEP;
		} else {
			vout_min = RT6160_VOUT2_MIN;
			vout_max = RT6160_VOUT2_MAX;
			vout_step = RT6160_VOUT2_STEP;
		}
		break;
	default:
		return -EINVAL;
	}

	if ((vout < vout_min) || (vout > vout_max)) {
		hwlog_err("vout out of range %u,%u\n", vout_min, vout_max);
		return -EINVAL;
	}

	vset = (u8)((vout - vout_min) / vout_step);
	hwlog_info("dev_id=%u vsel_state=%u vsel_range=%u vset=%u\n",
		di->dev_id, di->vsel_state, di->vsel_range, vset);

	if (!di->vsel_state)
		return tps63810_set_vout1(vset);

	return tps63810_set_vout2(vset);
}

static bool tps63810_power_good(void *dev_data)
{
	int ret;
	u8 pg_val = 0;

	/* Bit[0] 0: power good; 1: power not good */
	ret = tps63810_read_mask(TPS63810_STATUS_ADDR, TPS63810_STATUS_PG_MASK,
		TPS63810_STATUS_PG_SHIFT, &pg_val);
	if (ret)
		return false;

	if (!pg_val)
		return true;

	return false;
}

static bool tps63810_set_enable(unsigned int value, void *dev_data)
{
	u32 ctrl_val;
	struct tps63810_device_info *di = g_tps63810_di;

	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	if (!gpio_is_valid(di->gpio_no))
		return false;

	gpio_set_value(di->gpio_no, value);
	power_usleep(DT_USLEEP_2MS);

	if (di->dev_id == TPS63810_DEVID_TI) {
		ctrl_val = (di->vsel_range << 1) | TPS63810_CONTROL_ENABLE;
		(void)tps63810_write_mask(TPS63810_CONTROL_ADDR, TPS63810_CONTROL_MASK,
			TPS63810_CONTROL_SHIFT, ctrl_val);
	}

	return true;
}

static u8 tps63810_get_device_id(void)
{
	int ret;
	u8 id = 0;

	tps63810_set_enable(1, g_tps63810_di);

	ret = tps63810_read_byte(TPS63810_DEVID_ADDR, &id);
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	if (ret)
		set_hw_dev_flag(DEV_I2C_BUCKBOOST);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	tps63810_set_enable(0, g_tps63810_di);

	hwlog_info("device_id is %d,%u\n", ret, id);
	return id;
}

static void tps63810_parse_dts(struct device_node *np,
	struct tps63810_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vsel_state", &di->vsel_state, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vsel_range", &di->vsel_range, 1);
	(void)power_gpio_config_output(np,
		"gpio_enable", "gpio_enable", &di->gpio_no, 0);
}

static struct buck_boost_ops tps63810_ops = {
	.pwr_good = tps63810_power_good,
	.set_vout = tps63810_set_vout,
	.set_enable = tps63810_set_enable,
};

static int tps63810_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct tps63810_device_info *di = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_tps63810_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	tps63810_parse_dts(np, di);
	ret = buck_boost_ops_register(&tps63810_ops);
	if (ret)
		return ret;

	di->dev_id = tps63810_get_device_id();
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->dev_id;
		power_dev_info->ver_id = 0;
	}
	return 0;
}

static int tps63810_remove(struct i2c_client *client)
{
	struct tps63810_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (gpio_is_valid(di->gpio_no))
		gpio_free(di->gpio_no);

	i2c_set_clientdata(client, NULL);
	g_tps63810_di = NULL;
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bbst_tps63810);
static const struct of_device_id tps63810_of_match[] = {
	{
		.compatible = "huawei, buckboost_tps63810",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id tps63810_i2c_id[] = {
	{ "bbst_tps63810", 0 }, {}
};

static struct i2c_driver tps63810_driver = {
	.probe = tps63810_probe,
	.remove = tps63810_remove,
	.id_table = tps63810_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bbst_tps63810",
		.of_match_table = of_match_ptr(tps63810_of_match),
	},
};

static int __init tps63810_init(void)
{
	return i2c_add_driver(&tps63810_driver);
}

static void __exit tps63810_exit(void)
{
	i2c_del_driver(&tps63810_driver);
}

fs_initcall_sync(tps63810_init);
module_exit(tps63810_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("tps63810 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
