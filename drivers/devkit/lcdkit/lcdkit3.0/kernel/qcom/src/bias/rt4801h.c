/*
 * drivers/huawei/drivers/rt4801h.c
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

#include "rt4801h.h"

#if defined(CONFIG_LCD_KIT_DRIVER)
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#endif

static struct rt4801h_device_info *rt4801h_client = NULL;
static bool is_rt4801h_device = false;

static void rt4801h_enable()
{
	if (rt4801h_client != NULL) {
		gpiod_set_value_cansleep(gpio_to_desc(rt4801h_client->config.vsp_enable), VSP_ENABLE);
		mdelay(2);
		gpiod_set_value_cansleep(gpio_to_desc(rt4801h_client->config.vsn_enable), VSN_ENABLE);
	}
}

static int rt4801h_reg_init(struct i2c_client *client, u8 vpos, u8 vneg)
{
	int ret = 0;
	unsigned int app_dis;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
		goto exit;
	}

	ret = i2c_smbus_read_byte_data(client, RT4801H_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		goto exit;
	}
	app_dis = ret;

	app_dis = app_dis | RT4801H_DISP_BIT | RT4801H_DISN_BIT | RT4801H_DISP_BIT;

	if (rt4801h_client->config.app_dis)
		app_dis &= ~RT4801H_APPS_BIT;
	ret = i2c_smbus_write_byte_data(client, RT4801H_REG_VPOS, vpos);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, RT4801H_REG_VNEG, vneg);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, RT4801H_REG_APP_DIS, (u8)app_dis);
	if (ret < 0) {
		pr_err("%s write app_dis failed\n", __func__);
		goto exit;
	}

exit:
	return ret;
}

#if defined(CONFIG_LCD_KIT_DRIVER)
int rt4801h_get_bias_voltage(int *vpos_target, int *vneg_target)
{
	int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct rt4801h_voltage); i++) {
		if (vol_table[i].voltage == *vpos_target) {
			pr_err("rt4801h vsp voltage:0x%x\n", vol_table[i].value);
			*vpos_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct rt4801h_voltage)) {
		pr_err("not found vsp voltage, use default voltage:RT4801H_VOL_55\n");
		*vpos_target = RT4801H_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct rt4801h_voltage); i++) {
		if (vol_table[i].voltage == *vneg_target) {
			pr_err("rt4801h vsn voltage:0x%x\n", vol_table[i].value);
			*vneg_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct rt4801h_voltage)) {
		pr_err("not found vsn voltage, use default voltage:RT4801H_VOL_55\n");
		*vpos_target = RT4801H_VOL_55;
	}
	return 0;
}
#endif


static bool rt4801h_device_verify(struct i2c_client *client)
{
	int ret;
	ret = i2c_smbus_read_byte_data(client, RT4801H_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		return false;
	}
	pr_info("RT4801H verify ok, app_dis = 0x%x\n", ret);
	return true;
}

static int rt4801h_start_setting(struct i2c_client *client, int vsp, int vsn)
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
static void rt4801h_get_bias_config(int vpos, int vneg, int *outvsp, int *outvsn)
{
	unsigned int i;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct rt4801h_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			*outvsp = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct rt4801h_voltage)) {
		pr_err("not found vpos voltage, use default voltage:RT4801H_VOL_55\n");
		*outvsp = RT4801H_VOL_55;
	}

	for (i = 0; i < sizeof(vol_table) / sizeof(struct rt4801h_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			*outvsn = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct rt4801h_voltage)) {
		pr_err("not found vneg voltage, use default voltage:RT4801H_VOL_55\n");
		*outvsn = RT4801H_VOL_55;
	}
	pr_info("rt4801h_get_bias_config: %d(vpos)= 0x%x, %d(vneg) = 0x%x\n",
		vpos, *outvsp, vneg, *outvsn);
}

static int rt4801h_set_bias_power_down(int vpos, int vneg)
{
	int vsp = 0;
	int vsn = 0;
	int ret;

	rt4801h_get_bias_config(vpos, vneg, &vsp, &vsn);
	ret = i2c_smbus_write_byte_data(rt4801h_client->client, RT4801H_REG_VPOS, vsp);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(rt4801h_client->client, RT4801H_REG_VNEG, vsn);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		return ret;
	}
	return ret;
}

static int rt4801h_set_bias(int vpos, int vneg)
{
	int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct rt4801h_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			pr_err("rt4801h vsp voltage:0x%x\n", vol_table[i].value);
			vpos = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct rt4801h_voltage)) {
		pr_err("not found vsp voltage, use default voltage:RT4801H_VOL_55\n");
		vpos = RT4801H_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct rt4801h_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			pr_err("rt4801h vsn voltage:0x%x\n", vol_table[i].value);
			vneg = vol_table[i].value;
			break;
		}
	}

	if (i >= sizeof(vol_table) / sizeof(struct rt4801h_voltage)) {
		pr_err("not found vsn voltage, use default voltage:RT4801H_VOL_55\n");
		vneg = RT4801H_VOL_55;
	}
	rt4801h_reg_init(rt4801h_client->client, vpos, vneg);
	return 0;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = rt4801h_set_bias,
	.set_bias_power_down = rt4801h_set_bias_power_down,
};
#endif

static int rt4801h_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int vsp_pin = 0;
	int vsn_pin = 0;
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

	retval = rt4801h_start_setting(client, vsp_pin, vsn_pin);
	if (retval) {
		pr_err("rt4801h_start_setting faild\n");
		retval = -ENODEV;
		goto failed_1;
	}

	if (rt4801h_device_verify(client)) {
		is_rt4801h_device = true;
	} else {
		pr_info("rt4801h_reg_verify failed\n");
		is_rt4801h_device = false;
		retval = -ENODEV;
		goto failed_1;
	}

	retval = of_property_read_u32(np, "rt4801h_app_dis", &app_dis);
	if (retval >= 0)
		pr_info("rt4801h_app_dis is support\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[%s, %d]: need I2C_FUNC_I2C\n", __FUNCTION__, __LINE__);
		retval = -ENODEV;
		goto failed_1;
	}

	rt4801h_client = kzalloc(sizeof(*rt4801h_client), GFP_KERNEL);
	if (!rt4801h_client) {
		dev_err(&client->dev, "failed to allocate device info data\n");
		retval = -ENOMEM;
		goto failed_1;
	}

	i2c_set_clientdata(client, rt4801h_client);
	rt4801h_client->dev = &client->dev;
	rt4801h_client->client = client;
	rt4801h_client->config.vsp_enable = vsp_pin;
	rt4801h_client->config.vsn_enable = vsn_pin;
	rt4801h_client->config.app_dis = app_dis;
	rt4801h_client->config.vpos_cmd = RT4801H_VOL_57;
	rt4801h_client->config.vneg_cmd = RT4801H_VOL_57;

	rt4801h_enable();
	retval = rt4801h_reg_init(rt4801h_client->client, (u8)rt4801h_client->config.vpos_cmd,
				(u8)rt4801h_client->config.vneg_cmd);
	if (retval) {
		retval = -ENODEV;
		pr_err("rt4801h_reg_init failed\n");
		goto failed;
	}
	pr_info("rt4801h inited succeed\n");

#ifdef CONFIG_LCD_KIT_DRIVER
	lcd_kit_bias_register(&bias_ops);
#endif
failed_1:
	return retval;
failed:
	if (rt4801h_client) {
		kfree(rt4801h_client);
		rt4801h_client = NULL;
	}
	return retval;
}

static const struct of_device_id rt4801h_match_table[] = {
	{
		.compatible = DTS_COMP_RT4801H,
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id rt4801h_i2c_id[] = {
	{ "rt4801h", 0 },
	{ }
};

MODULE_DEVICE_TABLE(of, rt4801h_match_table);

static struct i2c_driver rt4801h_driver = {
	.id_table = rt4801h_i2c_id,
	.probe = rt4801h_probe,
	.driver = {
		.name = "rt4801h",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rt4801h_match_table),
	},
};

module_i2c_driver(rt4801h_driver);

MODULE_DESCRIPTION("RT4801H driver");
MODULE_LICENSE("GPL");
