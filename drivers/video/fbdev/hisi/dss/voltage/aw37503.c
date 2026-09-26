/*
 * drivers/aw37503.c
 *
 * aw37503 driver reffer to lcd
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Author: HUAWEI, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/param.h>
#include <linux/delay.h>
#include <linux/idr.h>
#include <linux/i2c.h>
#include <asm/unaligned.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#include "aw37503.h"
#include "../hisi_fb_def.h"

#if defined(CONFIG_LCD_KIT_DRIVER)
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#endif
#define DTS_COMP_AW37503 "hisilicon,aw37503_phy"

static u8 vpos_cmd = 0;
static u8 vneg_cmd = 0;
static struct aw37503_device_info *aw37503_client = NULL;
static bool is_aw37503_device = false;
static int aw37503_app_dis;

static int aw37503_reg_init(struct i2c_client *client, u8 vpos, u8 vneg)
{
	int ret;
	unsigned int app_dis;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
		goto exit;
	}

	ret = i2c_smbus_read_byte_data(client, AW37503_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		goto exit;
	}
	app_dis = (unsigned int)ret;

	app_dis = app_dis | AW37503_DISP_BIT | AW37503_DISN_BIT | AW37503_DISP_BIT;

	if (aw37503_app_dis)
		app_dis &= ~AW37503_APPS_BIT;
	ret = i2c_smbus_write_byte_data(client, AW37503_REG_VPOS, vpos);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, AW37503_REG_VNEG, vneg);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, AW37503_REG_APP_DIS, (u8)app_dis);
	if (ret < 0) {
		pr_err("%s write app_dis failed\n", __func__);
		goto exit;
	}

exit:
	return ret;
}

#if defined(CONFIG_LCD_KIT_DRIVER)
int aw37503_get_bias_voltage(int *vpos_target, int *vneg_target)
{
	unsigned int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct aw37503_voltage); i++) {
		if (vol_table[i].voltage == *vpos_target) {
			pr_err("aw37503 vsp voltage:0x%x\n", vol_table[i].value);
			*vpos_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct aw37503_voltage)) {
		pr_err("not found vsp voltage, use default voltage:AW37503_VOL_55\n");
		*vpos_target = AW37503_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct aw37503_voltage); i++) {
		if (vol_table[i].voltage == *vneg_target) {
			pr_err("aw37503 vsn voltage:0x%x\n", vol_table[i].value);
			*vneg_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct aw37503_voltage)) {
		pr_err("not found vsn voltage, use default voltage:AW37503_VOL_55\n");
		*vpos_target = AW37503_VOL_55;
	}
	return 0;
}
#endif

static bool aw37503_device_verify(void)
{
	int vendorid = 0;
	vendorid = i2c_smbus_read_byte_data(aw37503_client->client, AW37503_REG_VENDORID);
	if (vendorid != AW37503_ENABLE_FLAG) {
		pr_err("%s no aw37503 device\n", __func__);
		return false;
	}
	DPU_FB_INFO("aw37503 verify ok, vendorid = 0x%x\n", vendorid);
	return true;
}

static void aw37503_get_target_voltage(int *vpos_target, int *vneg_target)
{
#if defined(CONFIG_LCD_KIT_DRIVER)
	struct lcd_kit_ops *lcd_ops = NULL;
#endif

	if ((vpos_target == NULL) || (vneg_target == NULL)) {
		pr_err("%s: NULL point\n", __func__);
		return;
	}

#if defined(CONFIG_LCD_KIT_DRIVER)
	lcd_ops = lcd_kit_get_ops();
	if (lcd_ops && lcd_ops->lcd_kit_support) {
		if (lcd_ops->lcd_kit_support()) {
			if (common_ops->get_bias_voltage) {
				common_ops->get_bias_voltage(vpos_target, vneg_target);
				aw37503_get_bias_voltage(vpos_target, vneg_target);
			}
			return;
		}
	}
#endif
	DPU_FB_INFO("vpos and vneg target is 5.5V\n");
	*vpos_target = AW37503_VOL_55;
	*vneg_target = AW37503_VOL_55;

	return;
}

#ifdef CONFIG_LCD_KIT_DRIVER
static void aw37503_get_bias_config(int vpos, int vneg, int *outvsp, int *outvsn)
{
	unsigned int i;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct aw37503_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			*outvsp = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct aw37503_voltage)) {
		pr_err("not found vpos voltage, use default voltage:AW37503_VOL_55\n");
		*outvsp = AW37503_VOL_55;
	}

	for (i = 0; i < sizeof(vol_table) / sizeof(struct aw37503_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			*outvsn = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct aw37503_voltage)) {
		pr_err("not found vneg voltage, use default voltage:AW37503_VOL_55\n");
		*outvsn = AW37503_VOL_55;
	}
	pr_info("aw37503_get_bias_config: %d(vpos)= 0x%x, %d(vneg) = 0x%x\n",
		vpos, *outvsp, vneg, *outvsn);
}

static int aw37503_set_bias_power_down(int vpos, int vneg)
{
	int vsp = 0;
	int vsn = 0;
	int ret;

	aw37503_get_bias_config(vpos, vneg, &vsp, &vsn);
	ret = i2c_smbus_write_byte_data(aw37503_client->client, AW37503_REG_VPOS, vsp);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(aw37503_client->client, AW37503_REG_VNEG, vsn);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		return ret;
	}
	return ret;
}

static int aw37503_set_bias(int vpos, int vneg)
{
	unsigned int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct aw37503_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			pr_err("aw37503 vsp voltage:0x%x\n", vol_table[i].value);
			vpos = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct aw37503_voltage)) {
		pr_err("not found vsp voltage, use default voltage:AW37503_VOL_55\n");
		vpos = AW37503_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct aw37503_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			pr_err("aw37503 vsn voltage:0x%x\n", vol_table[i].value);
			vneg = vol_table[i].value;
			break;
		}
	}

	if (i >= sizeof(vol_table) / sizeof(struct aw37503_voltage)) {
		pr_err("not found vsn voltage, use default voltage:AW37503_VOL_55\n");
		vneg = AW37503_VOL_55;
	}
	pr_err("vpos = 0x%x, vneg = 0x%x\n", vpos, vneg);
	aw37503_reg_init(aw37503_client->client, vpos_cmd, vneg_cmd);
	return 0;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = aw37503_set_bias,
	.set_bias_power_down = aw37503_set_bias_power_down,
};
#endif

static int aw37503_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int ret = 0;
	int vpos_target = 0;
	int vneg_target = 0;
	struct device_node *np = NULL;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
		retval = -ENODEV;
		goto failed_1;
	}
	client->addr = AW37503_SLAV_ADDR;
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_AW37503);
	if (!np) {
		pr_err("NOT FOUND device node %s!\n", DTS_COMP_AW37503);
		retval = -ENODEV;
		goto failed_1;
	}

	ret = of_property_read_u32(np, "aw37503_app_dis", &aw37503_app_dis);
	if (ret >= 0)
		pr_info("aw37503_app_dis is support\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[%s,%d]: need I2C_FUNC_I2C\n", __FUNCTION__, __LINE__);
		retval = -ENODEV;
		goto failed_1;
	}

	aw37503_client = kzalloc(sizeof(*aw37503_client), GFP_KERNEL);
	if (!aw37503_client) {
		dev_err(&client->dev, "failed to allocate device info data\n");
		retval = -ENOMEM;
		goto failed_1;
	}

	i2c_set_clientdata(client, aw37503_client);
	aw37503_client->dev = &client->dev;
	aw37503_client->client = client;

	aw37503_get_target_voltage(&vpos_target, &vneg_target);
	vpos_cmd = (u8)vpos_target;
	vneg_cmd = (u8)vneg_target;

	if (aw37503_device_verify()) {
		is_aw37503_device = true;
	} else {
		is_aw37503_device = false;
		retval = -ENODEV;
		pr_err("aw37503_reg_verify failed\n");
		goto failed;
	}

	ret = aw37503_reg_init(aw37503_client->client, (u8)vpos_target, (u8)vneg_target);
	if (ret) {
		retval = -ENODEV;
		pr_err("aw37503_reg_init failed\n");
		goto failed;
	}
	pr_info("aw37503 inited succeed\n");

#ifdef CONFIG_LCD_KIT_DRIVER
	lcd_kit_bias_register(&bias_ops);
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_DC_DC);
#endif

failed_1:
	return retval;
failed:
	if (aw37503_client) {
		kfree(aw37503_client);
		aw37503_client = NULL;
	}
	return retval;
}

static const struct of_device_id aw37503_match_table[] = {
	{
		.compatible = DTS_COMP_AW37503,
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id aw37503_i2c_id[] = {
	{ "aw37503", 0 },
	{ }
};

MODULE_DEVICE_TABLE(of, aw37503_match_table);

static struct i2c_driver aw37503_driver = {
	.id_table = aw37503_i2c_id,
	.probe = aw37503_probe,
	.driver = {
		.name = "aw37503",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(aw37503_match_table),
	},
};

static int __init aw37503_module_init(void)
{
	int ret;

	ret = i2c_add_driver(&aw37503_driver);

	if (ret)
		pr_err("Unable to register aw37503 driver\n");

	return ret;
}
static void __exit aw37503_exit(void)
{
	if (aw37503_client) {
		kfree(aw37503_client);
		aw37503_client = NULL;
	}
	i2c_del_driver(&aw37503_driver);
}

late_initcall(aw37503_module_init);
module_exit(aw37503_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AW37503 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
