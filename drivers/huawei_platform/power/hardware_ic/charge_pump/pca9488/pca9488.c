/*
 * pca9488.c
 *
 * charge-pump pca9488 driver
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
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_dts.h>
#include <huawei_platform/power/power_gpio.h>
#include <huawei_platform/power/charge_pump.h>
#include <huawei_platform/power/power_devices_info.h>

#include "pca9488.h"

#define HWLOG_TAG cp_pca9488
HWLOG_REGIST();

static struct pca9488_dev_info *g_pca9488_di;

static int pca9488_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_length, u8 *read_data, int read_cnt)
{
	int i;
	int ret;
	struct i2c_msg msg[CP_I2C_RD_MSG_LEN];

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	msg[1].addr = client->addr;
	msg[1].buf = read_data;
	msg[1].len = read_cnt;
	msg[1].flags = I2C_M_RD;

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(client->adapter, msg, CP_I2C_RD_MSG_LEN);
		if (ret == CP_I2C_RD_MSG_LEN)
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -EIO;
}

static int pca9488_i2c_write(struct i2c_client *client,
	u8 *cmd, int cmd_length)
{
	int i;
	int ret;
	struct i2c_msg msg[CP_I2C_WR_MSG_LEN];

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	for (i = 0; i < CP_I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(client->adapter, msg, CP_I2C_WR_MSG_LEN);
		if (ret == CP_I2C_WR_MSG_LEN)
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -EIO;
}

static int pca9488_read_block(struct pca9488_dev_info *di,
	u8 reg, u8 *data, u8 len)
{
	if (len > CP_BYTE_LEN) /* ORed the Auto Increment bit - 0x80 */
		reg |= PCA9488_REG_BIT_AI;
	return pca9488_i2c_read(di->client, &reg, PCA9488_ADDR_LEN, data, len);
}

static int pca9488_write_block(struct pca9488_dev_info *di,
	u8 reg, u8 *data, u8 len)
{
	u8 cmd[PCA9488_ADDR_LEN + len];

	if (len > CP_BYTE_LEN) /* ORed the Auto Increment bit - 0x80 */
		cmd[0] = reg | PCA9488_REG_BIT_AI;
	else
		cmd[0] = reg;
	memcpy(&cmd[PCA9488_ADDR_LEN], data, len);

	return pca9488_i2c_write(di->client, cmd, PCA9488_ADDR_LEN + len);
}

static int pca9488_read_byte(u8 reg, u8 *data)
{
	struct pca9488_dev_info *di = g_pca9488_di;

	if (!di) {
		hwlog_err("read_byte: di null\n");
		return -EIO;
	}

	return pca9488_read_block(di, reg, data, CP_BYTE_LEN);
}

static int pca9488_write_byte(u8 reg, u8 data)
{
	struct pca9488_dev_info *di = g_pca9488_di;

	if (!di) {
		hwlog_err("write_byte: di null\n");
		return -EIO;
	}

	return pca9488_write_block(di, reg, &data, CP_BYTE_LEN);
}

static int pca9488_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = pca9488_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return pca9488_write_byte(reg, val);
}

static int pca9488_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = pca9488_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return ret;
}

static int pca9488_chip_init(void)
{
	int ret;

	ret = pca9488_write_byte(PCA9488_DEV_CTRL_REG0,
		PCA9488_DEV_CTRL_REG0_VAL);
	ret += pca9488_write_byte(PCA9488_HV_SC_CTRL_REG1,
		PCA9488_HV_SC_CTRL_REG1_VAL);
	ret += pca9488_write_byte(PCA9488_TRACK_CTRL_REG,
		PCA9488_TRACK_CTRL_VAL);

	return ret;
}

static int pca9488_reverse_chip_init(void)
{
	int ret;

	usleep_range(9500, 10500); /* 10ms: delay for sc stability */
	ret = pca9488_write_mask(PCA9488_DEV_CTRL_REG0,
		PCA9488_INFET_EN_MASK, PCA9488_INFET_EN_SHIFT,
		PCA9488_INFET_EN);
	if (ret) {
		hwlog_err("reverse_chip_init: fail\n");
		return ret;
	}

	return 0;
}

static bool pca9488_is_cp_open(void)
{
	int ret;
	u8 status = 0;

	ret = pca9488_read_mask(PCA9488_HV_SC_STS_REG0,
		PCA9488_SWITCHING_EN_MASK, PCA9488_SWITCHING_EN_SHIFT,
		&status);
	if (!ret && status)
		return true;

	return false;
}

static bool pca9488_is_bp_open(void)
{
	int ret;
	u8 status = 0;

	ret = pca9488_read_mask(PCA9488_HV_SC_STS_REG0,
		PCA9488_BYPASS_EN_MASK, PCA9488_BYPASS_EN_SHIFT,
		&status);
	if (!ret && status)
		return true;

	return false;
}

static int pca9488_set_bp_mode(void)
{
	int ret;

	ret = pca9488_write_mask(PCA9488_HV_SC_CTRL_REG0,
		PCA9488_SC_OP_MODE_MASK, PCA9488_SC_OP_MODE_SHIFT,
		PCA9488_SC_OP_BP);
	if (ret) {
		hwlog_err("set op to bp failed\n");
		return ret;
	}

	return 0;
}

static int pca9488_set_cp_mode(void)
{
	int ret;

	ret = pca9488_write_mask(PCA9488_HV_SC_CTRL_REG0,
		PCA9488_SC_OP_MODE_MASK, PCA9488_SC_OP_MODE_SHIFT,
		PCA9488_SC_OP_2TO1);
	if (ret) {
		hwlog_err("set op to cp failed\n");
		return ret;
	}

	return 0;
}

static void pca9488_irq_work(struct work_struct *work)
{
	hwlog_info("[irq_work] ++\n");
}

static irqreturn_t pca9488_irq_handler(int irq, void *p)
{
	struct pca9488_dev_info *di = g_pca9488_di;

	if (!di) {
		hwlog_err("irq_handler: di null\n");
		return IRQ_NONE;
	}
	hwlog_info("[irq_handler] ++\n");
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int pca9488_irq_init(struct pca9488_dev_info *di, struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "pca9488_int", &di->gpio_int, &di->irq_int))
		return -EINVAL;

	ret = request_irq(di->irq_int, pca9488_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "pca9488_irq", di);
	if (ret) {
		hwlog_err("irq_init: could not request pca9488_irq\n");
		di->irq_int = -EINVAL;
		gpio_free(di->gpio_int);
		return ret;
	}
	enable_irq_wake(di->irq_int);
	INIT_WORK(&di->irq_work, pca9488_irq_work);

	return 0;
}

static int pca9488_device_check(void)
{
	int ret;
	struct pca9488_dev_info *di = g_pca9488_di;

	if (!di) {
		hwlog_err("device_check: di null\n");
		return -1;
	}

	ret = pca9488_read_byte(PCA9488_DEVICE_ID_REG, &di->chip_id);
	if (ret) {
		hwlog_err("device_check: get chip_id failed\n");
		return ret;
	}
	hwlog_info("[device_check] chip_id = 0x%x\n", di->chip_id);

	return 0;
}

static int pca9488_post_probe(void)
{
	struct pca9488_dev_info *di = g_pca9488_di;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di) {
		hwlog_err("post_probe: di null\n");
		return -1;
	}

	if (pca9488_irq_init(di, di->client->dev.of_node)) {
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

static struct charge_pump_ops pca9488_ops = {
	.chip_name         = "pca9488",
	.dev_check         = pca9488_device_check,
	.post_probe        = pca9488_post_probe,
	.chip_init         = pca9488_chip_init,
	.reverse_chip_init = pca9488_reverse_chip_init,
	.set_bp_mode       = pca9488_set_bp_mode,
	.set_cp_mode       = pca9488_set_cp_mode,
	.is_cp_open        = pca9488_is_cp_open,
	.is_bp_open        = pca9488_is_bp_open,
};

static int pca9488_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct pca9488_dev_info *di = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_pca9488_di = di;
	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);

	ret = charge_pump_ops_register(&pca9488_ops);
	if (ret)
		goto ops_register_fail;

	return 0;

ops_register_fail:
	devm_kfree(&client->dev, di);
	di = NULL;
	g_pca9488_di = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_pca9488);
static const struct of_device_id pca9488_of_match[] = {
	{
		.compatible = "charge_pump_pca9488",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id pca9488_i2c_id[] = {
	{ "charge_pump_pca9488", 0 }, {}
};

static struct i2c_driver pca9488_driver = {
	.probe = pca9488_probe,
	.id_table = pca9488_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "charge_pump_pca9488",
		.of_match_table = of_match_ptr(pca9488_of_match),
	},
};

static int __init pca9488_init(void)
{
	return i2c_add_driver(&pca9488_driver);
}

static void __exit pca9488_exit(void)
{
	i2c_del_driver(&pca9488_driver);
}

rootfs_initcall(pca9488_init);
module_exit(pca9488_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pca9488 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
