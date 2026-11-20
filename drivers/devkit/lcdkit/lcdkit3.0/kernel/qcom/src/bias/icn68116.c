/*
 * drivers/huawei/drivers/icn68116.c
 *
 * lcdkit backlight function for lcd driver
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#include "icn68116.h"

#if defined(CONFIG_LCD_KIT_DRIVER)
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#endif

static struct icn68116_device_info *icn68116_client = NULL;
static bool is_icn68116_device = false;

static void icn68116_enable()
{
	if (icn68116_client != NULL) {
		gpiod_set_value_cansleep(gpio_to_desc(icn68116_client->config.vsp_enable), VSP_ENABLE);
		mdelay(2);
		gpiod_set_value_cansleep(gpio_to_desc(icn68116_client->config.vsn_enable), VSN_ENABLE);
	}
}

static int icn68116_reg_init(struct i2c_client *client, u8 vpos, u8 vneg)
{
	int ret = 0;
	unsigned int app_dis;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
		goto exit;
	}

	ret = i2c_smbus_read_byte_data(client, ICN68116_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		goto exit;
	}
	app_dis = ret;

	app_dis = app_dis | ICN68116_DISP_BIT | ICN68116_DISN_BIT | ICN68116_DISP_BIT;

	if (icn68116_client->config.app_dis)
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
	int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct icn68116_voltage); i++) {
		if (vol_table[i].voltage == *vpos_target) {
			pr_err("icn68116 vsp voltage:0x%x\n", vol_table[i].value);
			*vpos_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsp voltage, use default voltage:ICN68116_VOL_55\n");
		*vpos_target = ICN68116_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct icn68116_voltage); i++) {
		if (vol_table[i].voltage == *vneg_target) {
			pr_err("icn68116 vsn voltage:0x%x\n", vol_table[i].value);
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


static bool icn68116_device_verify(struct i2c_client *client)
{
	int ret;
	ret = i2c_smbus_read_byte_data(client, ICN68116_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		return false;
	}
	pr_info("ICN68116 verify ok, app_dis = 0x%x\n", ret);
	return true;
}

static int icn68116_start_setting(struct i2c_client *client, int vsp, int vsn)
{
	int ret;

	/* Request and Enable VSP gpio */
	ret = devm_gpio_request_one(&client->dev, vsp, GPIOF_OUT_INIT_HIGH, "rt4801-vsp");
	if (ret) {
		pr_err("devm_gpio_request_one gpio_vsp_enable %d faild\n", vsp);
		return ret;
	}
	/* delay 2ms */
	mdelay(2);

	/* Request And Enable VSN gpio */
	ret = devm_gpio_request_one(&client->dev, vsn, GPIOF_OUT_INIT_HIGH, "rt4801-vsn");
	if (ret) {
		pr_err("devm_gpio_request_one gpio_vsn_enable %d faild\n", vsn);
		return ret;
	}

	return ret;
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
	int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct icn68116_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			pr_err("icn68116 vsp voltage:0x%x\n", vol_table[i].value);
			vpos = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsp voltage, use default voltage:ICN68116_VOL_55\n");
		vpos = ICN68116_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct icn68116_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			pr_err("icn68116 vsn voltage:0x%x\n", vol_table[i].value);
			vneg = vol_table[i].value;
			break;
		}
	}

	if (i >= sizeof(vol_table) / sizeof(struct icn68116_voltage)) {
		pr_err("not found vsn voltage, use default voltage:ICN68116_VOL_55\n");
		vneg = ICN68116_VOL_55;
	}
	icn68116_reg_init(icn68116_client->client, vpos, vneg);
	return 0;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = icn68116_set_bias,
	.set_bias_power_down = icn68116_set_bias_power_down,
};
#endif

static int icn68116_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval;
	int vsp_pin;
	int vsn_pin;
	int app_dis = 0;
	struct device_node *np = NULL;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
		retval = -ENODEV;
		goto failed_1;
	}

	np = client->dev.of_node;
	vsp_pin = of_get_named_gpio_flags(np, "gpio_vsp_enable", 0, NULL);
	if (!gpio_is_valid(vsp_pin)) {
		pr_err("get vsp_enable gpio faild\n");
		retval = -ENODEV;
		goto failed_1;
	}
	vsn_pin = of_get_named_gpio_flags(np, "gpio_vsn_enable", 0, NULL);
	if (!gpio_is_valid(vsn_pin)) {
		pr_err("get vsn_enable gpio faild\n");
		retval = -ENODEV;
		goto failed_1;
	}
	retval = icn68116_start_setting(client, vsp_pin, vsn_pin);
	if (retval) {
		pr_err("icn68116_start_setting faild\n");
		retval = -ENODEV;
		goto failed_1;
	}

	if (icn68116_device_verify(client)) {
		is_icn68116_device = true;
	} else {
		pr_info("icn68116_reg_verify failed\n");
		is_icn68116_device = false;
		retval = -ENODEV;
		goto failed_1;
	}

	retval = of_property_read_u32(np, "icn68116_app_dis", &app_dis);
	if (retval >= 0)
		pr_info("icn68116_app_dis is support\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[%s, %d]: need I2C_FUNC_I2C\n", __FUNCTION__, __LINE__);
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
	icn68116_client->config.vsp_enable = vsp_pin;
	icn68116_client->config.vsn_enable = vsn_pin;
	icn68116_client->config.app_dis = app_dis;
	icn68116_client->config.vpos_cmd = ICN68116_VOL_57;
	icn68116_client->config.vneg_cmd = ICN68116_VOL_57;

	icn68116_enable();
	retval = icn68116_reg_init(icn68116_client->client, (u8)icn68116_client->config.vpos_cmd,
				(u8)icn68116_client->config.vneg_cmd);
	if (retval) {
		retval = -ENODEV;
		pr_err("icn68116_reg_init failed\n");
		goto failed;
	}
	pr_info("icn68116 inited succeed\n");

#ifdef CONFIG_LCD_KIT_DRIVER
	lcd_kit_bias_register(&bias_ops);
#endif

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

module_i2c_driver(icn68116_driver);

MODULE_DESCRIPTION("ICN68116 driver");
MODULE_LICENSE("GPL");
