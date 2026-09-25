/*
 * hl1512.c
 *
 * charge-pump hl1512 driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_i2c.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/power/power_gpio.h>
#include <huawei_platform/power/charge_pump.h>
#include <huawei_platform/power/power_devices_info.h>

#include "hl1512.h"

#define HWLOG_TAG cp_hl1512
HWLOG_REGIST();

static struct hl1512_dev_info *g_hl1512_di;

static int hl1512_read_byte(u8 reg, u8 *data)
{
	struct hl1512_dev_info *di = g_hl1512_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_read_byte(di->client, reg, data))
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -EIO;
}

static int hl1512_write_byte(u8 reg, u8 data)
{
	struct hl1512_dev_info *di = g_hl1512_di;
	int i;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_write_byte(di->client, reg, data))
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -EIO;
}

static int hl1512_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hl1512_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return hl1512_write_byte(reg, val);
}

static int hl1512_chip_init_ex(int type)
{
	int ret;
	u8 mode_val;
	u8 pmid_vout_ov_uv_val;
	u8 vout_ov_val;
	u8 current_limit_val;

	if (type == HL1512_CHARGING_FWD) {
		mode_val = HL1512_MOD_INIT_REG_VAL;
		pmid_vout_ov_uv_val = HL1512_PMID_VOUT_FWD_VAL;
		vout_ov_val = HL1512_VOUT_OV_REG_FWD_VAL;
		current_limit_val = HL1512_CURRENT_LIMIT_FWD_VAL;
	} else {
		mode_val = HL1512_MOD_REV_REG_VAL;
		pmid_vout_ov_uv_val = HL1512_PMID_VOUT_REV_VAL;
		vout_ov_val = HL1512_VOUT_OV_REG_REV_VAL;
		current_limit_val = HL1512_CURRENT_LIMIT_REV_VAL;
	}

	/* init a7 reg */
	ret = hl1512_write_byte(HL1512_INIT_REGA7, HL1512_INIT_REGA7_VAL);
	/* mode config reg */
	ret += hl1512_write_byte(HL1512_MOD_REG, mode_val);
	/* control reg */
	ret += hl1512_write_byte(HL1512_CONTROL_REG, HL1512_CONTROL_REG_VAL);
	/* pmid vout ov uv */
	ret += hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG,
		pmid_vout_ov_uv_val);
	/* vout ov */
	ret += hl1512_write_byte(HL1512_VOUT_OV_REG, vout_ov_val);
	/* wd and clock */
	ret += hl1512_write_byte(HL1512_WD_CLOCK_REG, HL1512_WD_CLOCK_VAL);
	/* current ilimit */
	ret += hl1512_write_byte(HL1512_CURRENT_LIMIT_REG, current_limit_val);
	/* init 58 reg */
	ret += hl1512_write_byte(HL1512_INIT_REG58, HL1512_INIT_REG58_VAL);
	/* temp alarm th */
	ret += hl1512_write_byte(HL1512_TEMP_ALARM_TH_REG,
		HL1512_TEMP_ALARM_TH_REG_VAL);

	return ret;
}

static int hl1512_chip_init(void)
{
	return hl1512_chip_init_ex(HL1512_CHARGING_FWD);
}

static int hl1512_reverse_chip_init(void)
{
	int ret;

	usleep_range(9500, 10500); /* 10ms: delay for sc stability */
	ret = hl1512_chip_init_ex(HL1512_CHARGING_REV);
	if (ret) {
		hwlog_err("reverse_chip_init: fail\n");
		return ret;
	}

	return 0;
}

static bool hl1512_is_cp_open(void)
{
	int ret;
	u8 val = 0;

	ret = hl1512_read_byte(HL1512_STATUS_REG, &val);
	if (!ret && ((val & HL1512_STATUS_REG_CP_VAL) == HL1512_STATUS_REG_CP_VAL))
		return true;

	return false;
}

static bool hl1512_is_bp_open(void)
{
	int ret;
	u8 val = 0;

	ret = hl1512_read_byte(HL1512_STATUS_REG, &val);
	if (!ret && ((val & HL1512_STATUS_REG_BP_VAL) == HL1512_STATUS_REG_BP_VAL))
		return true;

	return false;
}

static int hl1512_set_bp_mode(void)
{
	int ret;

	ret = hl1512_write_mask(HL1512_MOD_REG,
		HL1512_FORCE_BPCP_MASK, HL1512_FORCE_BPCP_SHIFT,
		HL1512_FORCE_BP_EN);
	usleep_range(9500, 10500); /* 10ms for status stability */
	ret += hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG,
		HL1512_PMID_VOUT_FWD_VAL);
	if (ret) {
		hwlog_err("set op to bp failed\n");
		return ret;
	}

	return 0;
}

static int hl1512_set_cp_mode(void)
{
	int ret;

	ret = hl1512_write_byte(HL1512_PMID_VOUT_OV_UV_REG,
		HL1512_PMID_VOUT_CP_VAL);
	ret += hl1512_write_mask(HL1512_MOD_REG,
		HL1512_FORCE_BPCP_MASK, HL1512_FORCE_BPCP_SHIFT,
		HL1512_FORCE_CP_EN);
	if (ret) {
		hwlog_err("set op to cp failed\n");
		return ret;
	}

	return 0;
}

static void hl1512_irq_work(struct work_struct *work)
{
	hwlog_info("[irq_work] ++\n");
}

static irqreturn_t hl1512_irq_handler(int irq, void *p)
{
	struct hl1512_dev_info *di = g_hl1512_di;

	if (!di) {
		hwlog_err("irq_handler: di null\n");
		return IRQ_NONE;
	}
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int hl1512_irq_init(struct hl1512_dev_info *di, struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "hl1512_int", &di->gpio_int, &di->irq_int))
		return -EINVAL;

	ret = request_irq(di->irq_int, hl1512_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "hl1512_irq", di);
	if (ret) {
		hwlog_err("irq_init: could not request hl1512_irq\n");
		di->irq_int = -EINVAL;
		gpio_free(di->gpio_int);
		return ret;
	}
	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, hl1512_irq_work);

	return 0;
}

static int hl1512_device_check(void)
{
	int ret;
	struct hl1512_dev_info *di = g_hl1512_di;

	if (!di) {
		hwlog_err("device_check: di null\n");
		return -1;
	}

	ret = hl1512_read_byte(HL1512_DEVICE_ID_REG, &di->chip_id);
	if (ret) {
		hwlog_err("device_check: get chip_id failed\n");
		return ret;
	}
	hwlog_info("[device_check] chip_id = 0x%x\n", di->chip_id);

	return 0;
}

static int hl1512_post_probe(void)
{
	struct hl1512_dev_info *di = g_hl1512_di;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di) {
		hwlog_err("post_probe: di null\n");
		return -1;
	}

	if (hl1512_irq_init(di, di->client->dev.of_node)) {
		hwlog_err("post_probe: irq init failed\n");
		return -1;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->chip_id;
		power_dev_info->ver_id = 0;
	}

	return 0;
}

static struct charge_pump_ops hl1512_ops = {
	.chip_name         = "hl1512",
	.dev_check         = hl1512_device_check,
	.post_probe        = hl1512_post_probe,
	.chip_init         = hl1512_chip_init,
	.reverse_chip_init = hl1512_reverse_chip_init,
	.set_bp_mode       = hl1512_set_bp_mode,
	.set_cp_mode       = hl1512_set_cp_mode,
	.is_cp_open        = hl1512_is_cp_open,
	.is_bp_open        = hl1512_is_bp_open,
};

static int hl1512_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct hl1512_dev_info *di = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_hl1512_di = di;
	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);

	ret = charge_pump_ops_register(&hl1512_ops);
	if (ret)
		goto ops_register_fail;

	return 0;

ops_register_fail:
	devm_kfree(&client->dev, di);
	di = NULL;
	g_hl1512_di = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_hl1512);
static const struct of_device_id hl1512_of_match[] = {
	{
		.compatible = "charge_pump_hl1512",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id hl1512_i2c_id[] = {
	{ "charge_pump_hl1512", 0 }, {}
};

static struct i2c_driver hl1512_driver = {
	.probe = hl1512_probe,
	.id_table = hl1512_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "charge_pump_hl1512",
		.of_match_table = of_match_ptr(hl1512_of_match),
	},
};

static int __init hl1512_init(void)
{
	return i2c_add_driver(&hl1512_driver);
}

static void __exit hl1512_exit(void)
{
	i2c_del_driver(&hl1512_driver);
}

rootfs_initcall(hl1512_init);
module_exit(hl1512_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl1512 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
