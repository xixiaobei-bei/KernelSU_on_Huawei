/*
 * drivers/huawei/drivers/icn68116.c
 *
 * icn68116 driver reffer to lcd
 *
 * Copyright (C) 2012-2015 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include "icn68116.h"
#include "../hisi_fb_def.h"

#if defined(CONFIG_LCD_KIT_DRIVER)
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#endif
#define DTS_COMP_ICN68116 "hisilicon,icn68116_phy"
static int gpio_vsp_enable = 0;
static int gpio_vsn_enable = 0;
static bool fastboot_display_enable = true;

static u8 vpos_cmd = 0;
static u8 vneg_cmd = 0;
static struct icn68116_device_info *icn68116_client = NULL;
static bool is_icn68116_device = false;
static int icn68116_app_dis;
#define DTS_COMP_SHARP_DUKE_NT35597     "hisilicon,mipi_sharp_duke_NT35597"
#define DTS_COMP_JDI_DUKE_R63450_5P7    "hisilicon,mipi_jdi_duke_R63450_5P7"
#define DTS_COMP_TIANMA_DUKE_TD4302_5P7 "hisilicon,mipi_tianma_duke_TD4302_5P7"

#define VAL_5V5 (0)
#define VAL_5V8 (1)
#define VAL_5V6 (2)

#define VSP_ENABLE (1)
#define VSN_ENABLE (1)
#define VSP_DISABLE (0)
#define VSN_DISABLE (0)
#define GPIOS_NUM0 (0)
#define GPIOS_NUM1 (1)

static int get_lcd_type(void)
{
	struct device_node *np = NULL;
	int ret = 0;
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_SHARP_DUKE_NT35597);
	ret = of_device_is_available(np);
	if (np && ret) {
		DPU_FB_INFO("device %s! set voltage 5.8V\n", DTS_COMP_SHARP_DUKE_NT35597);
		return VAL_5V8;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_JDI_DUKE_R63450_5P7);
	ret = of_device_is_available(np);
	if (np && ret) {
		DPU_FB_INFO("device %s! set voltage 5.8V\n", DTS_COMP_JDI_DUKE_R63450_5P7);
		return VAL_5V5;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_TIANMA_DUKE_TD4302_5P7);
	ret = of_device_is_available(np);
	if (np && ret) {
		DPU_FB_INFO("device %s! set voltage 5.8V\n", DTS_COMP_TIANMA_DUKE_TD4302_5P7);
		return VAL_5V5;
	}

	DPU_FB_INFO("not found device set vsp/vsn voltage 5.5V\n");
	return VAL_5V5;
}

static int icn68116_reg_init(struct i2c_client *client, u8 vpos, u8 vneg)
{
	int ret;
	unsigned int app_dis;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n",__FUNCTION__,__LINE__);
		goto exit;
	}

	ret = i2c_smbus_read_byte_data(client, ICN68116_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		goto exit;
	}
	app_dis = ret;

	app_dis = app_dis | ICN68116_DISP_BIT | ICN68116_DISN_BIT | ICN68116_DISP_BIT;

	if (icn68116_app_dis)
		app_dis &= ~ICN68116_APPS_BIT;
	ret = i2c_smbus_write_byte_data(client, ICN68116_REG_VPOS, vpos);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, ICN68116_REG_VNEG, vneg);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, ICN68116_REG_APP_DIS, (u8)app_dis);
	if (ret < 0) {
		pr_err("%s write app_dis failed\n", __func__);
		goto exit;
	}

exit:
	return ret;
}

#if defined(CONFIG_LCD_KIT_DRIVER)
int icn68116_get_bias_voltage(int *vpos_target, int *vneg_target)
{
	unsigned int i = 0;

	for (i = 0;i < sizeof(vol_table) / sizeof(struct icn68116_voltage);i++) {
		if (vol_table[i].voltage == *vpos_target) {
			pr_err("icn68116 vsp voltage:0x%x\n",vol_table[i].value);
			*vpos_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsp voltage, use default voltage:ICN68116_VOL_55\n");
		*vpos_target = ICN68116_VOL_55;
	}
	for (i = 0;i < sizeof(vol_table) / sizeof(struct icn68116_voltage);i++) {
		if (vol_table[i].voltage == *vneg_target) {
			pr_err("icn68116 vsn voltage:0x%x\n",vol_table[i].value);
			*vneg_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsn voltage, use default voltage:ICN68116_VOL_55\n");
		*vpos_target = ICN68116_VOL_55;
	}
	return 0;
}
#endif


static bool icn68116_device_verify(void)
{
	int ret = 0;
	ret = i2c_smbus_read_byte_data(icn68116_client->client, ICN68116_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		return false;
	}
	DPU_FB_INFO("ICN68116 verify ok, app_dis = 0x%x\n", ret);
	return true;
}

bool check_icn68116_device(void)
{
	return is_icn68116_device;
}

int icn68116_set_voltage(void)
{
	DPU_FB_INFO("ICN68116 set vol reg, vpos = 0x%x, vneg = 0x%x\n", vpos_cmd, vneg_cmd);
	return icn68116_reg_init(icn68116_client->client, vpos_cmd, vneg_cmd);
}

static void icn68116_get_target_voltage(int *vpos_target, int *vneg_target)
{
	int ret = 0;
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
				icn68116_get_bias_voltage(vpos_target, vneg_target);
			}
			return;
		}
	}
#endif


	ret = get_lcd_type();
	if (ret == VAL_5V8) {
		DPU_FB_INFO("vpos and vneg target is 5.8V\n");
		*vpos_target = ICN68116_VOL_58;
		*vneg_target = ICN68116_VOL_58;
	} else if (ret == VAL_5V6) {
		DPU_FB_INFO("vpos and vneg target is 5.6V\n");
		*vpos_target = ICN68116_VOL_56;
		*vneg_target = ICN68116_VOL_56;
	} else {
		DPU_FB_INFO("vpos and vneg target is 5.5V\n");
		*vpos_target = ICN68116_VOL_55;
		*vneg_target = ICN68116_VOL_55;
	}
	return;
}

static int icn68116_start_setting(void)
{
	int retval;

	retval = gpio_request(gpio_vsp_enable, "gpio_lcd_p5v5_enable");
	if (retval != 0) {
		pr_err("failed to request gpio %d : gpio_lcd_p5v5_enable !\n", gpio_vsp_enable);
		return retval;
	}

	retval = gpio_request(gpio_vsn_enable, "gpio_lcd_n5v5_enable");
	if (retval != 0) {
		pr_err("failed to request gpio %d : gpio_lcd_n5v5_enable !\n", gpio_vsn_enable);
		return retval;
	}

	retval = gpio_direction_output(gpio_vsp_enable, VSP_ENABLE);
	if (retval != 0) {
		pr_err("failed to output gpio %d : gpio_lcd_p5v5_enable !\n", gpio_vsp_enable);
		return retval;
	}
	mdelay(5);

	retval = gpio_direction_output(gpio_vsn_enable, VSN_ENABLE);
	if (retval != 0) {
		pr_err("failed to output gpio %d : gpio_lcd_p5v5_enable !\n", gpio_vsn_enable);
		return retval;
	}
	mdelay(5);

	return retval;
}

static int icn68116_finish_setting(void)
{
	int retval = 0;

	retval = gpio_direction_output(gpio_vsn_enable, VSP_DISABLE);
	if (retval != 0) {
		pr_err("failed to output gpio %d : gpio_lcd_n5v5_enable !\n", gpio_vsn_enable);
		return retval;
	}
	udelay(10);

	retval = gpio_direction_output(gpio_vsp_enable, VSP_DISABLE);
	if (retval != 0) {
		pr_err("failed to output gpio %d : gpio_lcd_p5v5_enable !\n", gpio_vsp_enable);
		return retval;
	}
	udelay(10);

	retval = gpio_direction_input(gpio_vsn_enable);
	if (retval != 0) {
		pr_err("failed to set gpio %d input: gpio_lcd_n5v5_enable !\n", gpio_vsn_enable);
		return retval;
	}
	udelay(10);

	retval = gpio_direction_input(gpio_vsp_enable);
	if (retval != 0) {
		pr_err("failed to set gpio %d input: gpio_lcd_p5v5_enable !\n", gpio_vsp_enable);
		return retval;
	}
	udelay(10);

	gpio_free(gpio_vsn_enable);
	gpio_free(gpio_vsp_enable);

	return retval;
}

#ifdef CONFIG_LCD_KIT_DRIVER
static void icn68116_get_bias_config(int vpos, int vneg, int *outvsp, int *outvsn)
{
	unsigned int i;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct icn68116_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			*outvsp = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vpos voltage, use default voltage:ICN68116_VOL_55\n");
		*outvsp = ICN68116_VOL_55;
	}

	for (i = 0; i < sizeof(vol_table) / sizeof(struct icn68116_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			*outvsn = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vneg voltage, use default voltage:ICN68116_VOL_55\n");
		*outvsn = ICN68116_VOL_55;
	}
	pr_info("icn68116_get_bias_config: %d(vpos)= 0x%x, %d(vneg) = 0x%x\n",
		vpos, *outvsp, vneg, *outvsn);
}

static int icn68116_set_bias_power_down(int vpos, int vneg)
{
	int vsp = 0;
	int vsn = 0;
	int ret;

	icn68116_get_bias_config(vpos, vneg, &vsp, &vsn);
	ret = i2c_smbus_write_byte_data(icn68116_client->client, ICN68116_REG_VPOS, vsp);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(icn68116_client->client, ICN68116_REG_VNEG, vsn);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		return ret;
	}
	return ret;
}

static int icn68116_set_bias(int vpos, int vneg)
{
	unsigned int i = 0;

	for (i = 0;i < sizeof(vol_table) / sizeof(struct icn68116_voltage);i++) {
		if(vol_table[i].voltage == vpos) {
			pr_err("icn68116 vsp voltage:0x%x\n",vol_table[i].value);
			vpos = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsp voltage, use default voltage:ICN68116_VOL_55\n");
		vpos = ICN68116_VOL_55;
	}
	for (i = 0;i < sizeof(vol_table) / sizeof(struct icn68116_voltage);i++) {
		if (vol_table[i].voltage == vneg) {
			pr_err("icn68116 vsn voltage:0x%x\n",vol_table[i].value);
			vneg = vol_table[i].value;
			break;
		}
	}

	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsn voltage, use default voltage:ICN68116_VOL_55\n");
		vneg = ICN68116_VOL_55;
	}
	pr_err("vpos = 0x%x, vneg = 0x%x\n", vpos, vneg);
	icn68116_reg_init(icn68116_client->client, vpos_cmd, vneg_cmd);
	return 0;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = icn68116_set_bias,
	.set_bias_power_down = icn68116_set_bias_power_down,
};
#endif

static int icn68116_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int ret = 0;
	int vpos_target = 0;
	int vneg_target = 0;
	struct device_node *np = NULL;

	if(client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n",__FUNCTION__,__LINE__);
		retval = -ENODEV;
		goto failed_1;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_ICN68116);
	if (!np) {
		pr_err("NOT FOUND device node %s!\n", DTS_COMP_ICN68116);
		retval = -ENODEV;
		goto failed_1;
	}

	gpio_vsp_enable = of_get_named_gpio(np, "gpios", GPIOS_NUM0);
	gpio_vsn_enable = of_get_named_gpio(np, "gpios", GPIOS_NUM1);

	ret = of_property_read_u32(np, "icn68116_app_dis", &icn68116_app_dis);
	if (ret >= 0)
		pr_info("icn68116_app_dis is support\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[%s,%d]: need I2C_FUNC_I2C\n",__FUNCTION__,__LINE__);
		retval = -ENODEV;
		goto failed_1;
	}

	icn68116_client = kzalloc(sizeof(*icn68116_client), GFP_KERNEL);
	if (!icn68116_client) {
		dev_err(&client->dev, "failed to allocate device info data\n");
		retval = -ENOMEM;
		goto failed_1;
	}

	i2c_set_clientdata(client, icn68116_client);
	icn68116_client->dev = &client->dev;
	icn68116_client->client = client;

	if (!fastboot_display_enable)
		icn68116_start_setting();

	icn68116_get_target_voltage(&vpos_target, &vneg_target);
	vpos_cmd = (u8)vpos_target;
	vneg_cmd = (u8)vneg_target;

	if (icn68116_device_verify()) {
		is_icn68116_device = true;
	} else {
		is_icn68116_device = false;
		retval = -ENODEV;
		pr_err("icn68116_reg_verify failed\n");
		goto failed;
	}

	ret = icn68116_reg_init(icn68116_client->client, (u8)vpos_target, (u8)vneg_target);
	if (ret) {
		retval = -ENODEV;
		pr_err("icn68116_reg_init failed\n");
		goto failed;
	}
	pr_info("icn68116 inited succeed\n");

#ifdef CONFIG_LCD_KIT_DRIVER
	lcd_kit_bias_register(&bias_ops);
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_DC_DC);
#endif

	if (!fastboot_display_enable)
		icn68116_finish_setting();
failed_1:
	return retval;
failed:
	if (icn68116_client) {
		kfree(icn68116_client);
		icn68116_client = NULL;
	}
	return retval;
}

static const struct of_device_id icn68116_match_table[] = {
	{
		.compatible = DTS_COMP_ICN68116,
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id icn68116_i2c_id[] = {
	{ "icn68116", 0 },
	{ }
};

MODULE_DEVICE_TABLE(of, icn68116_match_table);

static struct i2c_driver icn68116_driver = {
	.id_table = icn68116_i2c_id,
	.probe = icn68116_probe,
	.driver = {
		.name = "icn68116",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(icn68116_match_table),
	},
};

static int __init icn68116_module_init(void)
{
	int ret;

	ret = i2c_add_driver(&icn68116_driver);
	if (ret)
		pr_err("Unable to register icn68116 driver\n");

	return ret;
}
static void __exit icn68116_exit(void)
{
	if (icn68116_client) {
		kfree(icn68116_client);
		icn68116_client = NULL;
	}
	i2c_del_driver(&icn68116_driver);
}

late_initcall(icn68116_module_init);
module_exit(icn68116_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ICN68116 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
