/*
 * max77813.c
 *
 * max77813 driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mutex.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/buck_boost.h>
#include "max77813.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG max77813
HWLOG_REGIST();

struct max77813_device_info *g_max77813_dev;

#define MSG_LEN                      2

static int max77813_read_block(struct max77813_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf;
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	buf = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = num_bytes;

	ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);

	/* i2c_transfer returns number of messages transferred */
	if (ret != MSG_LEN) {
		hwlog_err("read_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int max77813_write_block(struct max77813_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	*value = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = num_bytes + 1;

	ret = i2c_transfer(di->client->adapter, msg, 1);

	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		hwlog_err("write_block failed[%x]\n", reg);
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int max77813_write_byte(u8 reg, u8 value)
{
	struct max77813_device_info *di = g_max77813_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = { 0 };

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return max77813_write_block(di, temp_buffer, reg, 1);
}

static int max77813_read_byte(u8 reg, u8 *value)
{
	struct max77813_device_info *di = g_max77813_dev;

	return max77813_read_block(di, value, reg, 1);
}

static int max77813_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = max77813_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return max77813_write_byte(reg, val);
}

int max77813_forced_pwm_enable(unsigned int enable)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;

	ret = max77813_read_byte(MAX77813_CONFIG1, &reg);
	if (ret)
		return -1;

	hwlog_info("before config1 [%x]=0x%x\n", MAX77813_CONFIG1, reg);

	ret = max77813_write_mask(MAX77813_CONFIG1,
		MAX77813_FORCED_PWM_MASK, MAX77813_FORCED_PWM_SHIFT, value);
	if (ret)
		return -1;

	ret = max77813_read_byte(MAX77813_CONFIG1, &reg);
	if (ret)
		return -1;

	hwlog_info("after config1 [%x]=0x%x\n", MAX77813_CONFIG1, reg);
	return 0;
}

static bool max77813_device_check(void)
{
	int ret;
	u8 reg = 0;

	ret = max77813_read_byte(MAX77813_INFO_REG00, &reg);
	if (ret)
		return false;

	hwlog_info("device_check [%x]=0x%x\n", MAX77813_INFO_REG00, reg);
	return true;
}

static void max77813_irq_work(struct work_struct *work)
{
	u8 reg = 0;
	int ret;

	ret = max77813_read_byte(MAX77813_STATUS_REG01, &reg);
	if (ret)
		hwlog_err("irq_work read fail\n");

	hwlog_info("status_reg01 [%x]=0x%x\n", MAX77813_STATUS_REG01, reg);
}

static irqreturn_t max77813_interrupt(int irq, void *_di)
{
	struct max77813_device_info *di = _di;

	hwlog_info("max77813 int happened\n");

	if (!di)
		return IRQ_HANDLED;

	disable_irq_nosync(di->irq_pok);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static void max77813_prase_dts(struct device_node *np,
	struct max77813_device_info *di)
{
}

static struct bbst_device_ops max77813_ops = {
	.device_check = max77813_device_check,
	.set_pwm_enable = max77813_forced_pwm_enable,
};

static int max77813_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct max77813_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_max77813_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	INIT_WORK(&di->irq_work, max77813_irq_work);

	max77813_prase_dts(np, di);

	di->gpio_pok = of_get_named_gpio(np, "gpio_pok", 0);
	hwlog_info("gpio_pok=%d\n", di->gpio_pok);

	if (!gpio_is_valid(di->gpio_pok)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto max77813_fail_0;
	}

	ret = gpio_request(di->gpio_pok, "buckboost_max77813");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto max77813_fail_0;
	}

	ret = gpio_direction_input(di->gpio_pok);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		goto max77813_fail_1;
	}

	di->irq_pok = gpio_to_irq(di->gpio_pok);
	if (di->irq_pok < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto max77813_fail_1;
	}

	ret = request_irq(di->irq_pok, max77813_interrupt,
		IRQF_TRIGGER_FALLING, "max77813_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_pok = -1;
		goto max77813_fail_1;
	}

	ret = buck_boost_ops_register(&max77813_ops);
	if (ret)
		goto max77813_fail_2;

	hwlog_info("probe end\n");
	return 0;

max77813_fail_2:
	free_irq(di->irq_pok, di);
max77813_fail_1:
	gpio_free(di->gpio_pok);
max77813_fail_0:
	g_max77813_dev = NULL;

	return ret;
}

static int max77813_remove(struct i2c_client *client)
{
	struct max77813_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	if (di->gpio_pok)
		gpio_free(di->gpio_pok);

	if (di->irq_pok)
		free_irq(di->irq_pok, di);

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, max77813);
static const struct of_device_id max77813_of_match[] = {
	{
		.compatible = "huawei,max77813",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id max77813_i2c_id[] = {
	{ "max77813", 0 }, {}
};

static struct i2c_driver max77813_driver = {
	.probe = max77813_probe,
	.remove = max77813_remove,
	.id_table = max77813_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "max77813",
		.of_match_table = of_match_ptr(max77813_of_match),
	},
};

static int __init max77813_init(void)
{
	return i2c_add_driver(&max77813_driver);
}

static void __exit max77813_exit(void)
{
	i2c_del_driver(&max77813_driver);
}

module_init(max77813_init);
module_exit(max77813_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("max77813 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
