/*
 * sm5109.c
 *
 * sm5109 bias driver
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

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#include "sm5109.h"

#define BIAS_TRUE 1
#define BIAS_FALSE 0

static struct sm5109_device_info *dev_info = NULL;
static int g_vol_mapped = BIAS_FALSE;
static unsigned char vpos_cmd;
static unsigned char vneg_cmd;

static struct sm5109_voltage voltage_table[] = {
	{ 4000000, SM5109_VOL_40 },
	{ 4100000, SM5109_VOL_41 },
	{ 4200000, SM5109_VOL_42 },
	{ 4300000, SM5109_VOL_43 },
	{ 4400000, SM5109_VOL_44 },
	{ 4500000, SM5109_VOL_45 },
	{ 4600000, SM5109_VOL_46 },
	{ 4700000, SM5109_VOL_47 },
	{ 4800000, SM5109_VOL_48 },
	{ 4900000, SM5109_VOL_49 },
	{ 5000000, SM5109_VOL_50 },
	{ 5100000, SM5109_VOL_51 },
	{ 5200000, SM5109_VOL_52 },
	{ 5300000, SM5109_VOL_53 },
	{ 5400000, SM5109_VOL_54 },
	{ 5500000, SM5109_VOL_55 },
	{ 5600000, SM5109_VOL_56 },
	{ 5700000, SM5109_VOL_57 },
	{ 5800000, SM5109_VOL_58 },
	{ 5900000, SM5109_VOL_59 },
	{ 6000000, SM5109_VOL_60 },
	{ 6100000, SM5109_VOL_61 },
	{ 6200000, SM5109_VOL_62 },
	{ 6300000, SM5109_VOL_63 },
	{ 6400000, SM5109_VOL_64 },
	{ 6500000, SM5109_VOL_65 }
};

static void sm5109_get_target_voltage(int vpos_target, int vneg_target)
{
	int i;
	int vol_size = ARRAY_SIZE(voltage_table);

	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vpos_target) {
			vpos_cmd = voltage_table[i].value;
			break;
		}
	}

	if (i >= vol_size) {
		LCD_KIT_ERR("not found vsp voltage, use default voltage\n");
		vpos_cmd = SM5109_VOL_55;
	}

	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vneg_target) {
			vneg_cmd = voltage_table[i].value;
			break;
		}
	}

	if (i >= vol_size) {
		LCD_KIT_ERR("not found vsn voltage, use default voltage\n");
		vneg_cmd = SM5109_VOL_55;
	}

	LCD_KIT_INFO("get vpos = 0x%x, vneg = 0x%x\n", vpos_cmd, vneg_cmd);
}

static int sm5109_reg_init(struct i2c_client *client, unsigned char vpos_cmd,
	unsigned char vneg_cmd)
{
	unsigned char app_dis;
	int ret = LCD_KIT_FAIL;

	if (client == NULL) {
		LCD_KIT_ERR("%s: NULL point for client\n", __func__);
		return ret;
	}

	ret = i2c_smbus_read_byte_data(client, SM5109_REG_APP_DIS);
	if (ret < 0) {
		lcd_bias_i2c_dmd();
		LCD_KIT_ERR("%s: read app_dis failed\n", __func__);
		return ret;
	}

	app_dis = (unsigned char)ret;
	app_dis = app_dis | SM5109_DISP_BIT | SM5109_DISN_BIT;

	ret = i2c_smbus_write_byte_data(client, SM5109_REG_VPOS, vpos_cmd);
	if (ret < 0) {
		lcd_bias_i2c_dmd();
		LCD_KIT_ERR("%s: write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(client, SM5109_REG_VNEG, vneg_cmd);
	if (ret < 0) {
		lcd_bias_i2c_dmd();
		LCD_KIT_ERR("%s: write vneg failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(client, SM5109_REG_APP_DIS,
		app_dis);
	if (ret < 0) {
		lcd_bias_i2c_dmd();
		LCD_KIT_ERR("%s: write app_dis failed\n", __func__);
	}

	LCD_KIT_INFO("vpos = 0x%x, vneg = 0x%x, app_dis = 0x%x\n",
		vpos_cmd, vneg_cmd, app_dis);
	return ret;
}

void sm5109_set_voltage(unsigned char vpos, unsigned char vneg)
{
	int ret;

	if (vpos >= SM5109_VOL_MAX) {
		LCD_KIT_ERR("set vpos error, vpos = %d is out of range\n", vpos);
		return;
	}

	if (vneg >= SM5109_VOL_MAX) {
		LCD_KIT_ERR("set vneg error, vneg = %d is out of range\n", vneg);
		return;
	}

	if (dev_info == NULL)
		return;

	ret = sm5109_reg_init(dev_info->client, vpos, vneg);
	if (ret)
		LCD_KIT_ERR("sm5109 reg init not success\n");
	else
		LCD_KIT_INFO("sm5109 set voltage succeed\n");
}

static int sm5109_set_bias(int vpos, int vneg)
{
	if (g_vol_mapped == BIAS_FALSE) {
		sm5109_get_target_voltage(vpos, vneg);
		g_vol_mapped = BIAS_TRUE;
	}

	sm5109_set_voltage(vpos_cmd, vneg_cmd);
	return LCD_KIT_OK;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = sm5109_set_bias,
	.dbg_set_bias_voltage = NULL,
};

static int sm5109_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int retval = 0;

	if (client == NULL) {
		pr_err("%s: NULL point for client\n", __func__);
		retval = -ENODEV;
		goto failed_1;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		LCD_KIT_ERR("%s: need I2C_FUNC_I2C\n", __func__);
		retval = -ENODEV;
		goto failed_1;
	}
	dev_info = kzalloc(sizeof(*dev_info), GFP_KERNEL);
	if (!dev_info) {
		dev_err(&client->dev, "failed to allocate device info\n");
		retval = -ENOMEM;
		goto failed_1;
	}
	i2c_set_clientdata(client, dev_info);
	dev_info->dev = &client->dev;
	dev_info->client = client;

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
	set_hw_dev_detect_result(DEV_DETECT_DC_DC);
#endif
	lcd_kit_bias_register(&bias_ops);
	return retval;

failed_1:
	if (dev_info) {
		kfree(dev_info);
		dev_info = NULL;
	}
	return retval;
}

static const struct of_device_id sm5109_match_table[] = {
	{
		.compatible = SM5109_DTS_COMP,
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sm5109_i2c_id[] = {
	{ "sm5109", 0 },
	{}
};

MODULE_DEVICE_TABLE(of, sm5109_match_table);

static struct i2c_driver sm5109_driver = {
	.id_table = sm5109_i2c_id,
	.probe = sm5109_probe,
	.driver = {
		.name = "sm5109",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sm5109_match_table),
	},
};

static int __init sm5109_module_init(void)
{
	int ret;

	ret = i2c_add_driver(&sm5109_driver);
	if (ret)
		LCD_KIT_ERR("unable to register sm5109 driver\n");
	return ret;
}

static void __exit sm5109_exit(void)
{
	i2c_del_driver(&sm5109_driver);
}

module_init(sm5109_module_init);
module_exit(sm5109_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("driver for sm5109");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
