/*
 * tps65132.c
 *
 * tps65132 bias driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/i2c.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>
#include "tps65132.h"
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif

#define DTS_COMP_TPS65132 "ti,tps65132"

struct i2c_client *g_client;

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = NULL,
	.dbg_set_bias_voltage = NULL,
};

static int tps65132_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int retval = 0;
	struct device_node *np = NULL;
	struct tps65132_device_info *di = NULL;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_TPS65132);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_TPS65132);
		retval = -ENODEV;
		goto failed_1;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		LCD_KIT_ERR("[%s,%d]: need I2C_FUNC_I2C\n", __func__, __LINE__);
		retval = -ENODEV;
		goto failed_1;
	}
	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		dev_err(&client->dev, "failed to allocate device info data\n");
		retval = -ENOMEM;
		goto failed_1;
	}
	i2c_set_clientdata(client, di);
	di->dev = &client->dev;
	di->client = client;
#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
	/* detect current device successful, set the flag as present */
	set_hw_dev_detect_result(DEV_DETECT_DC_DC);
#endif
	g_client = client;
	lcd_kit_bias_register(&bias_ops);
	return retval;

failed_1:
	return retval;
}

static const struct of_device_id tps65132_match_table[] = {
	{
		.compatible = DTS_COMP_TPS65132,
		.data = NULL,
	},
	{},
};


static const struct i2c_device_id tps65132_i2c_id[] = {
	{ "tps65132", 0 },
	{}
};

MODULE_DEVICE_TABLE(of, tps65132_match_table);


static struct i2c_driver tps65132_driver = {
	.id_table = tps65132_i2c_id,
	.probe = tps65132_probe,
	.driver = {
		.name = "tps65132",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(tps65132_match_table),
	},
};

static int __init tps65132_module_init(void)
{
	int ret;

	ret = i2c_add_driver(&tps65132_driver);
	if (ret)
		LCD_KIT_ERR("Unable to register tps65132 driver\n");
	return ret;
}
static void __exit tps65132_exit(void)
{
	i2c_del_driver(&tps65132_driver);
}

module_init(tps65132_module_init);
module_exit(tps65132_exit);

MODULE_DESCRIPTION("TPS65132 driver");
MODULE_LICENSE("GPL");
