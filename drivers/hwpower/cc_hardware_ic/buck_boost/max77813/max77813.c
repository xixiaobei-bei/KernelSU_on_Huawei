/*
 * max77813.c
 *
 * max77813 driver
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

#include "max77813.h"
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG max77813
HWLOG_REGIST();

static struct max77813_device_info *g_max77813_di;

static int max77813_write_byte(u8 reg, u8 value)
{
	struct max77813_device_info *di = g_max77813_di;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int max77813_read_byte(u8 reg, u8 *value)
{
	struct max77813_device_info *di = g_max77813_di;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
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

static int max77813_forced_pwm_enable(unsigned int enable, void *dev_data)
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

static void max77813_irq_work(struct work_struct *work)
{
	int ret;
	u8 reg = 0;

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

static struct buck_boost_ops max77813_ops = {
	.set_pwm_enable = max77813_forced_pwm_enable,
};

static int max77813_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct max77813_device_info *di = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_max77813_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	INIT_WORK(&di->irq_work, max77813_irq_work);

	max77813_prase_dts(np, di);

	ret = power_gpio_config_interrupt(np,
		"gpio_pok", "buckboost_max77813", &di->gpio_pok, &di->irq_pok);
	if (ret)
		goto max77813_fail_0;

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

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}
	return 0;

max77813_fail_2:
	free_irq(di->irq_pok, di);
max77813_fail_1:
	gpio_free(di->gpio_pok);
max77813_fail_0:
	i2c_set_clientdata(client, NULL);
	g_max77813_di = NULL;
	return ret;
}

static int max77813_remove(struct i2c_client *client)
{
	struct max77813_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (di->gpio_pok)
		gpio_free(di->gpio_pok);
	if (di->irq_pok)
		free_irq(di->irq_pok, di);

	i2c_set_clientdata(client, NULL);
	g_max77813_di = NULL;
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
