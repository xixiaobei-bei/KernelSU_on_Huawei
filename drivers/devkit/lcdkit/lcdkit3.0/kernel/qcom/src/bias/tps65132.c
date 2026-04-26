/*
 * tps65132.c
 *
 * tps65132 bias driver
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

#include "tps65132.h"
#if defined(CONFIG_LCD_KIT_DRIVER)
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#endif

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif

#define DTS_COMP_TPS65132 "ti,tps65132"
static int gpio_vsp_enable;
static int gpio_vsn_enable;
static bool fastboot_display_enable = true;
static int is_nt50358_support = 1;

#define VSP_ENABLE 1
#define VSN_ENABLE 1
#define VSP_DISABLE 0
#define VSN_DISABLE 0

#define VAL_5V5 0
#define VAL_5V8 1
#define VAL_5V6 2

static void tps65132_enable(void)
{
	gpiod_set_value_cansleep(gpio_to_desc(gpio_vsp_enable), VSP_ENABLE);
	mdelay(2);
	gpiod_set_value_cansleep(gpio_to_desc(gpio_vsp_enable), VSN_ENABLE);
}

static int tps65132_reg_inited(struct i2c_client *client, u8 vpos_target_cmd, u8 vneg_target_cmd)
{
	unsigned int vpos;
	unsigned int vneg;
	int ret = 0;

	ret = i2c_smbus_read_byte_data(client, TPS65132_REG_VPOS);
	if (ret < 0) {
		pr_err("%s read vpos voltage failed\n", __func__);
		goto exit;
	}
	vpos = ret;

	ret = i2c_smbus_read_byte_data(client, TPS65132_REG_VNEG);
	if (ret < 0) {
		pr_err("%s read vneg voltage failed\n", __func__);
		goto exit;
	}
	vneg = ret;

	pr_info("vpos : 0x%x, vneg: 0x%x", vpos, vpos);

	if (((vpos & TPS65132_REG_VOL_MASK) == vpos_target_cmd) &&
		((vneg & TPS65132_REG_VOL_MASK) == vneg_target_cmd))
		ret = 1;
	else
		ret = 0;

exit:
	return ret;
}


static int tps65132_reg_init(struct i2c_client *client, u8 vpos_cmd, u8 vneg_cmd)
{
	unsigned int vpos;
	unsigned int vneg;
	int ret = 0;
	unsigned int app_dis;
	unsigned int ctl;

	ret = i2c_smbus_read_byte_data(client, TPS65132_REG_VPOS);
	if (ret < 0) {
		pr_err("%s read vpos voltage failed\n", __func__);
		goto exit;
	}
	vpos = ret;

	ret = i2c_smbus_read_byte_data(client, TPS65132_REG_VNEG);
	if (ret < 0) {
		pr_err("%s read vneg voltage failed\n", __func__);
		goto exit;
	}
	vneg = ret;

	ret = i2c_smbus_read_byte_data(client, TPS65132_REG_APP_DIS);
	if (ret < 0) {
		pr_err("%s read app_dis failed\n", __func__);
		goto exit;
	}
	app_dis = ret;

	ret = i2c_smbus_read_byte_data(client, TPS65132_REG_CTL);
	if (ret < 0) {
		pr_err("%s read ctl failed\n", __func__);
		goto exit;
	}
	ctl = ret;

	vpos = (vpos & (~TPS65132_REG_VOL_MASK)) | vpos_cmd;
	vneg = (vneg & (~TPS65132_REG_VOL_MASK)) | vneg_cmd;
	app_dis = app_dis | TPS65312_APPS_BIT | TPS65132_DISP_BIT | TPS65132_DISN_BIT;
	ctl = ctl | TPS65132_WED_BIT;
	if (is_nt50358_support)
		app_dis &= ~TPS65312_APPS_BIT;

	ret = i2c_smbus_write_byte_data(client, TPS65132_REG_VPOS, (u8)vpos);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, TPS65132_REG_VNEG, (u8)vneg);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, TPS65132_REG_APP_DIS, (u8)app_dis);
	if (ret < 0) {
		pr_err("%s write app_dis failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, TPS65132_REG_CTL, (u8)ctl);
	if (ret < 0) {
		pr_err("%s write ctl failed\n", __func__);
		goto exit;
	}
	msleep(60);

exit:
	return ret;
}

static int tps65132_start_setting(struct i2c_client *client)
{
	int ret = 0;

	ret = devm_gpio_request_one(&client->dev, gpio_vsp_enable, GPIOF_OUT_INIT_HIGH, "rt4801-vsp");
	if (ret) {
		pr_err("devm_gpio_request_one gpio_vsp_enable %d faild\n", gpio_vsp_enable);
		return ret;
	}
	/* delay 2ms */
	mdelay(2);

	/* Request And Enable VSN gpio */
	ret = devm_gpio_request_one(&client->dev, gpio_vsn_enable, GPIOF_OUT_INIT_HIGH, "rt4801-vsn");
	if (ret) {
		pr_err("devm_gpio_request_one gpio_vsn_enable %d faild\n", gpio_vsn_enable);
		return ret;
	}
	mdelay(5);

	return ret;
}

static int tps65132_finish_setting(void)
{
	int retval = 0;

	retval = gpio_direction_output(gpio_vsn_enable, 0);
	if (retval != 0) {
		pr_err("failed to output gpio %d : gpio_lcd_n5v5_enable !\n", gpio_vsn_enable);
		return retval;
	}
	udelay(10);

	retval = gpio_direction_output(gpio_vsp_enable, 0);
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
struct i2c_client *g_client = NULL;

static void tps65132_get_bias_config(int vpos, int vneg, int *outvsp, int *outvsn)
{
	unsigned int i;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct tps65132_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			*outvsp = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct tps65132_voltage)) {
		pr_err("not found vpos voltage, use default voltage:TPS65132_VOL_55\n");
		*outvsp = TPS65132_VOL_55;
	}

	for (i = 0; i < sizeof(vol_table) / sizeof(struct tps65132_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			*outvsn = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct tps65132_voltage)) {
		pr_err("not found vneg voltage, use default voltage:VOL_55\n");
		*outvsn = TPS65132_VOL_55;
	}
	pr_info("tps65132_get_bias_config: %d(vpos)= 0x%x, %d(vneg) = 0x%x\n",
		vpos, *outvsp, vneg, *outvsn);
}

static int tps65132_set_bias_power_down(int vpos, int vneg)
{
	int vsp = 0;
	int vsn = 0;
	int ret;

	tps65132_get_bias_config(vpos, vneg, &vsp, &vsn);
	ret = i2c_smbus_write_byte_data(g_client, TPS65132_REG_VPOS, vsp);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(g_client, TPS65132_REG_VNEG, vsn);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		return ret;
	}
	return ret;
}


static int tps65132_dbg_set_bias(int vpos, int vneg)
{
	int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct tps65132_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			pr_info("tps65132 vsp voltage:0x%x\n", vol_table[i].value);
			vpos = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct tps65132_voltage)) {
		pr_err("not found vsp voltage, use default voltage:TPS65132_VOL_55\n");
		vpos = TPS65132_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct tps65132_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			pr_info("tps65132 vsn voltage:0x%x\n", vol_table[i].value);
			vneg = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct tps65132_voltage)) {
		pr_err("not found vsn voltage, use default voltage:TPS65132_VOL_55\n");
		vneg = TPS65132_VOL_55;
	}
	pr_info("vpos = 0x%x, vneg = 0x%x\n", vpos, vneg);
	if (!g_client) {
		pr_err("g_client is null\n");
		return -1;
	}
	return tps65132_reg_init(g_client, vpos, vneg);
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = tps65132_dbg_set_bias,
	.set_bias_power_down = tps65132_set_bias_power_down,
};
#endif

static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int ret = 0;
	int vpos_target = TPS65132_VOL_57;
	int vneg_target = TPS65132_VOL_57;
	struct device_node *np = NULL;
	struct tps65132_device_info *di = NULL;

	pr_info("tps65132_probe\n");
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_TPS65132);
	if (!np) {
		pr_err("NOT FOUND device node %s!\n", DTS_COMP_TPS65132);
		retval = -ENODEV;
		goto failed_1;
	}
	np = client->dev.of_node;
	gpio_vsp_enable = of_get_named_gpio_flags(np, "gpio_vsp_enable", 0, NULL);
	if (!gpio_is_valid(gpio_vsp_enable)) {
		pr_err("tps65132 get vsp_enable gpio faild\n");
		retval = -ENODEV;
		goto failed_1;
	}
	pr_info("tps65132 get gpio_vsp_enable = %d gpio OK\n", gpio_vsp_enable);
	gpio_vsn_enable = of_get_named_gpio_flags(np, "gpio_vsn_enable", 0, NULL);
	if (!gpio_is_valid(gpio_vsn_enable)) {
		pr_err("get vsn_enable gpio faild\n");
		retval = -ENODEV;
		goto failed_1;
	}
	pr_info("tps65132 get gpio_vsn_enable gpio %d OK\n", gpio_vsn_enable);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[%s,%d]: need I2C_FUNC_I2C\n", __FUNCTION__, __LINE__);
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

	tps65132_start_setting(client);
	tps65132_enable();
	pr_info("tps65132_enable\n");
	ret = tps65132_reg_inited(di->client, (u8)vpos_target, (u8)vneg_target);
	if (ret > 0) {
		pr_info("tps65132 inited needn't reset value\n");
	} else if (ret < 0) {
		pr_err("tps65132 I2C read not success\n");
		retval = -ENODEV;
		goto failed_2;
	} else {
		ret = tps65132_reg_init(di->client, (u8)vpos_target, (u8)vneg_target);
		if (ret) {
			pr_err("tps65132_reg_init failed\n");
			retval = -ENODEV;
			goto failed_2;
		}
		pr_err("tps65132 inited succeed\n");
	}
#ifdef CONFIG_LCD_KIT_DRIVER
	g_client = client;
	lcd_kit_bias_register(&bias_ops);
	return retval;
#endif
	pr_info("tps65132 inited succeed111\n");

failed_2:
	if (!fastboot_display_enable)
		tps65132_finish_setting();

	if (di)
		kfree(di);

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
	int ret = 0;

	ret = i2c_add_driver(&tps65132_driver);
	if (ret)
		pr_err("Unable to register tps65132 driver\n");

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
