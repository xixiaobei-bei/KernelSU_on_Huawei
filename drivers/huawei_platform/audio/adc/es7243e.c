/*
 * es7243e.c
 *
 * adc driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG es7243e_adc
HWLOG_REGIST();

#define NO_DELAY 0
#define REG_TABLE_COL_NUM 60
#define MSG_WRITE_SIZE 2
#define BUFF_SIZE 3
#define REG_VALUE_LEN 8
#define VALUE_HEX 16
#define DIVISION_FOUR 4
#define VALID_EIGHT 0xFF
#define VALID_SIXTEEN 0xFFFF
#define VALUE_SHIFT_RIGHT 24
#define LEN_RW_MSG 1
#define ADC_NUM    2
#define GPIO_VAL_HIGH 1
#define GPIO_VAL_LOW  0

#define ADC_ADDR            0x11
#define REG_RESET           0x00
#define REG_CLOCK_MANAGER1  0x01
#define REG_CLOCK_MANAGER4  0x04
#define REG_ANALOG_PDN      0x16
#define REG_ANALOG_VMIDSEL  0x17
#define REG_ANALOG_PGA1     0x20
#define REG_ANALOG_PGA2     0x21
#define REG_TESTMODE1       0xf7
#define REG_TESTMODE2       0xf9

struct es7243e_snd_private {
	struct i2c_client *client;
};
struct es7243e_snd_private *es7243e_private;

enum {
	ADC_ADDRESS_INDEX = 0,
	REG_ADDRESS_INDEX,
	VALUE_SLEEPTIME_INDEX,
	REG_TABLE_NUM_TOTAL,
};

static const char * const mic_dts_node_name = "everest,es7243e";
static const char * const reg_value_table = "reg_value_table";

static u32 g_register_table[REG_TABLE_COL_NUM][REG_TABLE_NUM_TOTAL];
static int vout_voltage = 3450000;
static int gpio_boost_en;

static int es7243e_get_adc_array(struct device_node *node,
				const char *name, void *out_array, u32 num)
{
	int ret;

	ret = of_property_read_u32_array(node, name, (u32 *)out_array, num);
	if (ret < 0) {
		hwlog_err("module %s  of_property_read_u32_array fail!\n", name);
		return -1;
	}

	return num;
}

static int es7243e_i2c_write(struct i2c_client *client, u8 adc_addr, u8 addr, u8 value)
{
	struct i2c_msg msgs[MSG_WRITE_SIZE];
	u8 buffer[BUFF_SIZE];

	hwlog_info("%s, [0x%02x] = 0x%02x\n", __func__, addr, value);

	buffer[0] = addr;
	buffer[1] = value;

	msgs[0].addr = adc_addr;
	msgs[0].flags = client->flags & I2C_M_TEN;
	msgs[0].len = MSG_WRITE_SIZE;
	msgs[0].buf = buffer;

	if (i2c_transfer(client->adapter, msgs, LEN_RW_MSG) != LEN_RW_MSG) {
		hwlog_err("addr=%x write failed\n", adc_addr);
		return -EIO;
	}

	return 0;
}

static int es7243e_i2c_read(struct i2c_client *client, u8 adc_addr, u8 addr, u8 *data)
{
	struct i2c_msg msgs[MSG_WRITE_SIZE];
	u8 buffer[BUFF_SIZE];

	hwlog_info("%s, [0x%02x\n", __func__, addr);

	buffer[0] = addr;

	msgs[0].addr = adc_addr;
	msgs[0].flags = client->flags & I2C_M_TEN;
	msgs[0].len = LEN_RW_MSG;
	msgs[0].buf = buffer;

	if (i2c_transfer(client->adapter, msgs, LEN_RW_MSG) != LEN_RW_MSG) {
		hwlog_err("addr=%x read failed\n", adc_addr);
		return -EIO;
	}

	if (memset_s(buffer, BUFF_SIZE, 0, BUFF_SIZE) != EOK) {
		hwlog_err("memset_s failed\n");
		return -EIO;
	}

	if (i2c_master_recv(client, buffer, LEN_RW_MSG) != LEN_RW_MSG) {
		hwlog_err("addr=%x read failed\n", adc_addr);
		return -EIO;
	}

	*data = buffer[0];
	hwlog_info("%s, i2c_master_recv data = 0x%02x\n", __func__, buffer[0]);
	return 0;
}

static int es7243e_resume(struct device *dev)
{
	u8 adc_addr = ADC_ADDR;
	struct i2c_client *i2c_client = es7243e_private->client;
	if (i2c_client == NULL)
		return -EINVAL;

	hwlog_info("es7243e going into resume mode\n");
	for (int i = 0; i < ADC_NUM; i++) {
		es7243e_i2c_write(i2c_client, adc_addr, REG_CLOCK_MANAGER1, 0x3a);
		es7243e_i2c_write(i2c_client, adc_addr, REG_RESET, 0x80);
		es7243e_i2c_write(i2c_client, adc_addr, REG_ANALOG_PDN, 0x00);
		es7243e_i2c_write(i2c_client, adc_addr, REG_ANALOG_VMIDSEL, 0x02);
		es7243e_i2c_write(i2c_client, adc_addr, REG_TESTMODE1, 0xf0);
		es7243e_i2c_write(i2c_client, adc_addr, REG_TESTMODE2, 0x00);
		es7243e_i2c_write(i2c_client, adc_addr, REG_CLOCK_MANAGER4, 0x02);
		adc_addr += ADC_NUM;
	}

	if (gpio_boost_en == 0) {
		hwlog_err("gpio_boost_en is invalid\n");
		return -1;
	}
	if (gpio_direction_output(gpio_boost_en, GPIO_VAL_HIGH))
		hwlog_err("enable pvdd failed\n");
	return 0;
}

static int es7243e_suspend(struct device *dev)
{
	u8 adc_addr = ADC_ADDR;
	struct i2c_client *i2c_client = es7243e_private->client;
	if (i2c_client == NULL)
		return -EINVAL;

	hwlog_info("es7243e going into suspend mode\n");

	for (int i = 0; i < ADC_NUM; i++) {
		es7243e_i2c_write(i2c_client, adc_addr, REG_CLOCK_MANAGER4, 0x02);
		es7243e_i2c_write(i2c_client, adc_addr, REG_CLOCK_MANAGER4, 0x01);
		es7243e_i2c_write(i2c_client, adc_addr, REG_TESTMODE1, 0x30);
		es7243e_i2c_write(i2c_client, adc_addr, REG_TESTMODE2, 0x01);
		es7243e_i2c_write(i2c_client, adc_addr, REG_ANALOG_PDN, 0xff);
		es7243e_i2c_write(i2c_client, adc_addr, REG_ANALOG_VMIDSEL, 0x00);
		es7243e_i2c_write(i2c_client, adc_addr, REG_CLOCK_MANAGER1, 0x38);
		es7243e_i2c_write(i2c_client, adc_addr, REG_RESET, 0x8e);
		es7243e_i2c_write(i2c_client, adc_addr, REG_CLOCK_MANAGER1, 0x30);
		adc_addr += ADC_NUM;
	}

	if (gpio_boost_en == 0) {
		hwlog_err("gpio_boost_en is invalid\n");
		return -1;
	}
	if (gpio_direction_output(gpio_boost_en, GPIO_VAL_LOW))
		hwlog_err("disable pvdd failed\n");

	return 0;
}

#ifdef ES7243E_DEBUG
static ssize_t es7243e_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	hwlog_info("echo flag|chip_addr|reg|val > es7243e\n");
	hwlog_info("eg read chip_addr=0x11,star addres=0x06,count 0x10:echo 110610 >es7243e\n");
	hwlog_info("eg write chip_addr=0x11,star addres=0x90,value=0x3c,count=4:echo 411903c >es7243e\n");

	return 0;
}

static ssize_t es7243e_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	unsigned int val;
	unsigned int flag;
	u8 i = 0;
	u8 adc_addr, reg, num, value_w, value_r;

	if (dev == NULL || buf == NULL) {
		hwlog_err("dev or buf is null\n");
		return -EINVAL;
	}

	struct es7243e_snd_private *es7243e = dev_get_drvdata(dev);

	if (kstrtouint(buf, VALUE_HEX, &val))
		return -EINVAL;
	flag = (val >> VALUE_SHIFT_RIGHT) & 0xFF;

	if (flag) {
		adc_addr = (val >> 16) & 0xFF;
		reg = (val >> REG_VALUE_LEN) & 0xFF;
		value_w = val & 0xFF;
		hwlog_info("\nWrite: start REG:0x%02x,val:0x%02x,count:0x%02x\n",
			reg, value_w, flag);
		while (flag--) {
			es7243e_i2c_write(es7243e->client, adc_addr, reg, value_w);
			hwlog_info("chip_addr 0x%02x,Write 0x%02x to REG:0x%02x\n",
				adc_addr, value_w, reg);
			reg++;
		}
	} else {
		adc_addr = (val >> 16) & 0xFF;
		reg = (val >> REG_VALUE_LEN) & 0xFF;
		num = val & 0xff;
		hwlog_info("\nRead: start REG:0x%02x,count:0x%02x\n", reg, num);
		do {
			value_r = 0;
			es7243e_i2c_read(es7243e->client, adc_addr, reg, &value_r);
			hwlog_info("chip_addr[0x%02x],REG[0x%02x]: 0x%02x;  ",
				adc_addr, reg, value_r);
			reg++;
			i++;
			if ((i == num) || (i % DIVISION_FOUR == 0))
				hwlog_info("\n");
		} while (i < num);
	}
	return count;
}

DEVICE_ATTR(es7243e, 0644, es7243e_show, es7243e_store);

static struct attribute *es7243e_debug_attrs[] = {
	&dev_attr_es7243e.attr,
	NULL,
};

static struct attribute_group es7243e_debug_attr_group = {
	.name   = "es7243e_debug",
	.attrs  = es7243e_debug_attrs,
};
#endif

static void es7243e_power_on(struct i2c_client *i2c_client)
{
	int ret;
	struct regulator *ldo = NULL;

	hwlog_info("%s enter\n", __func__);
	ldo = devm_regulator_get(&i2c_client->dev, "adc-vdd");
	if (IS_ERR(ldo)) {
		ldo = NULL;
		hwlog_err("devm_regulator_get fail\n");
		return;
	}

	ret = regulator_set_voltage(ldo, vout_voltage, vout_voltage);
	if (ret != 0)
		hwlog_err("regulator_set_voltage fail\n");

	ret = regulator_enable(ldo);
	if (ret != 0) {
		hwlog_err("regulator_enable fail\n");
		return;
	}
	hwlog_info("%s succ,current voltage = %d", __func__, vout_voltage);
}

static void es7243e_config(struct i2c_client *i2c_client)
{
	u8 i;
	int len;
	u32 register_value;
	u32 sleep_time;
	u32 adc_addr;
	struct device_node *node = NULL;

	hwlog_info("%s enter\n", __func__);

	node = of_find_compatible_node(NULL, NULL, mic_dts_node_name);
	if (node == NULL) {
		hwlog_err("can not find compatible node %s in dts\n",
			mic_dts_node_name);
		return;
	}

	len = es7243e_get_adc_array(node, reg_value_table,
		(u32 *)g_register_table,
		REG_TABLE_COL_NUM * REG_TABLE_NUM_TOTAL);
	if (len < 0) {
		hwlog_err("no valid register_table, len=%d\n", len);
		return;
	}

	hwlog_info("%s, len is %d\n", reg_value_table, len);

	for (i = 0; i < len / REG_TABLE_NUM_TOTAL; i++) {
		adc_addr = g_register_table[i][ADC_ADDRESS_INDEX];
		register_value =
			g_register_table[i][VALUE_SLEEPTIME_INDEX] >> VALUE_HEX;
		sleep_time =
			g_register_table[i][VALUE_SLEEPTIME_INDEX] & 0xFFFF;

		es7243e_i2c_write(i2c_client, adc_addr,
			g_register_table[i][REG_ADDRESS_INDEX], register_value);
		if (sleep_time != NO_DELAY)
			msleep(sleep_time);
	}

	hwlog_info("%s init end\n", __func__);
}

static void gpio_parse_devtree(struct i2c_client *client)
{
	struct device_node *np = client->dev.of_node;
	if (!np) {
		hwlog_err("%s np is null\n", __func__);
		return;
	}

	gpio_boost_en = of_get_named_gpio(np, "gpio_boost_en", 0);
	hwlog_info("gpio boost is %d\n", gpio_boost_en);
	if (!gpio_is_valid(gpio_boost_en)) {
		hwlog_err("gpio boost en invalid\n");
		return;
	}
}

static int es7243e_i2c_probe(struct i2c_client *i2c_client,
				const struct i2c_device_id *id)
{
	hwlog_info("%s %d\n", __func__, __LINE__);

	if (!i2c_client) {
		hwlog_err("i2c_client is null pointer\n");
		return -ENOMEM;
	}

	es7243e_private = devm_kzalloc(&i2c_client->dev,
		sizeof(struct es7243e_snd_private), GFP_KERNEL);
	if (!es7243e_private)
		return -ENOMEM;

	i2c_set_clientdata(i2c_client, es7243e_private);
	es7243e_private->client = i2c_client;
	es7243e_power_on(i2c_client);
	msleep(5);
	es7243e_config(i2c_client);
	gpio_parse_devtree(i2c_client);

#ifdef ES7243_DEBUG
	if (sysfs_create_group(&i2c_client->dev.kobj,
		&es7243e_debug_attr_group) != 0)
		hwlog_err("failed to create attr group\n");
#endif
	return 0;
}

static int es7243e_i2c_remove(struct i2c_client *client)
{
	devm_kfree(&client->dev, es7243e_private);
	es7243e_private = NULL;
#ifdef ES7243E_DEBUG
	sysfs_remove_group(&client->dev.kobj, &es7243e_debug_attr_group);
#endif
	return 0;
}

static const struct i2c_device_id es7243e_id[] = {
	{"es7243e", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, es7243e_id);

static const struct dev_pm_ops es7243e_pm_ops = {
	.suspend = es7243e_suspend,
	.resume = es7243e_resume,
};

static const struct of_device_id es7243e_dt_ids[] = {
	{.compatible = "everest,es7243e"},
	{},
};

static struct i2c_driver es7243e_i2c_driver = {
	.driver = {
		.name = "es7243e",
		.owner = THIS_MODULE,
		.pm = &es7243e_pm_ops,
		.of_match_table = es7243e_dt_ids,
	},
	.id_table = es7243e_id,
	.probe = es7243e_i2c_probe,
	.remove = es7243e_i2c_remove,
};

static int __init es7243e_init(void)
{
	return i2c_add_driver(&es7243e_i2c_driver);
}

static void __exit es7243e_exit(void)
{
	i2c_del_driver(&es7243e_i2c_driver);
}

module_init(es7243e_init);
module_exit(es7243e_exit);

MODULE_DESCRIPTION("ASoC ES7243 ADC driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL");
