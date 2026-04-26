/*
 * es7210.c
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
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG es7210_adc
HWLOG_REGIST();

#define NO_DELAY 0
#define REG_TABLE_COL_NUM 30
#define MESG_WRITE_SIZE 2
#define BUFF_SIZE 3
#define REG_VALUE_LEN 8
#define VALUE_HEX 16
#define DIVISION_FOUR 4
#define VALID_EIGHT 0xFF
#define VALID_SIXTEEN 0xFFFF
#define LEN_RW_MSG 1

struct es7210_snd_private {
	struct i2c_client *client;
};
struct es7210_snd_private *es7210_private;

enum {
	ADC_ADDRESS_INDEX = 0,
	REG_ADDRESS_INDEX,
	VALUE_SLEEPTIME_INDEX,
	REG_TABLE_NUM_TOTAL,
};

static const char * const mic_dts_node_name = "everest,es7210";
static const char * const reg_value_table = "reg_value_table";

static u32 g_register_table[REG_TABLE_COL_NUM][REG_TABLE_NUM_TOTAL];

static int get_adc_dts_array_byname(struct device_node *node,
				const char *name, void *out_array, u32 num)
{
	int ret;

	ret = of_property_read_u32_array(node, name, (u32 *)out_array, num);
	if (ret < 0) {
		pr_err("module %s  of_property_read_u32_array fail!\n", name);
		return -1;
	}

	return num;
}

static int es7210_i2c_write(struct i2c_client *client, u8 addr, u8 value)
{
	struct i2c_msg msgs[MESG_WRITE_SIZE];
	u8 buffer[BUFF_SIZE];

	hwlog_info("%s, [0x%02x] = 0x%02x\n", __func__, addr, value);

	buffer[0] = addr;
	buffer[1] = value;

	msgs[0].addr = client->addr;
	msgs[0].flags = client->flags & I2C_M_TEN;
	msgs[0].len = MESG_WRITE_SIZE;
	msgs[0].buf = buffer;

	if (i2c_transfer(client->adapter, msgs, LEN_RW_MSG) != LEN_RW_MSG) {
		hwlog_err("addr=%x write failed\n", client->addr);
		return -EIO;
	}

	return 0;
}

static int es7210_i2c_read(struct i2c_client *client, u8 addr, u8 *data)
{
	struct i2c_msg msgs[MESG_WRITE_SIZE];
	u8 buffer[BUFF_SIZE];

	hwlog_info("%s, [0x%02x\n", __func__, addr);

	buffer[0] = addr;

	msgs[0].addr = client->addr;
	msgs[0].flags = client->flags & I2C_M_TEN;
	msgs[0].len = LEN_RW_MSG;
	msgs[0].buf = buffer;

	if (i2c_transfer(client->adapter, msgs, LEN_RW_MSG) != LEN_RW_MSG) {
		hwlog_err("addr=%x read failed\n", client->addr);
		return -EIO;
	}

	if (memset_s(buffer, BUFF_SIZE, 0, BUFF_SIZE) != EOK) {
		hwlog_err("memset_s failed\n");
		return -EIO;
	}

	if (i2c_master_recv(client, buffer, LEN_RW_MSG) != LEN_RW_MSG) {
		hwlog_err("addr=%x read failed\n", client->addr);
		return -EIO;
	}

	*data = buffer[0];
	hwlog_info("%s, i2c_master_recv data = 0x%02x\n", __func__, buffer[0]);
	return 0;
}
/*
 * ES7210 resume from sleep mode
 */
static int es7210_resume(struct device *dev)
{
	pr_debug("es7210 going into es7210_snd_route_resume mode\n");
	struct i2c_client *i2c_client = es7210_private->client;

	if (i2c_client == NULL)
		return -EINVAL;

	return 0;
}
/*
 * ES7210 Enter into sleep mode
 * Note:
 *      Before ES7210 is ready to enter into sleep mode, the I2S MCLK
 *      and LRCK must be active.
 *      After es7210_standby() performed, I2S MCLK and LRCK can be stop
 *      to minimize power consumption
 */
static int es7210_suspend(struct device *dev)
{
	pr_debug("es7210 going into suspend mode\n");
	struct i2c_client *i2c_client = es7210_private->client;

	if (i2c_client == NULL)
		return -EINVAL;

	return 0;
}

#ifdef ES7210_DEBUG
static ssize_t es7210_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	hwlog_info("echo flag|reg|val > es7210\n");
	hwlog_info("eg read star addres=0x06,count 0x10:echo 0610 >es7210\n");
	hwlog_info("eg write star addres=0x90,value=0x3c,count=4:echo 4903c >es7210\n");

	return 0;
}

static ssize_t es7210_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	unsigned int val;
	unsigned int flag;
	u8 i = 0;
	u8 reg, num, value_w, value_r;

	if (dev == NULL || buf == NULL) {
		hwlog_err("dev or buf is null\n");
		return -EINVAL;
	}

	struct es7210_snd_private *es7210 = dev_get_drvdata(dev);

	if (kstrtouint(buf, VALUE_HEX, &val))
		return -EINVAL;
	flag = (val >> VALUE_HEX) & 0xFF;

	if (flag) {
		reg = (val >> REG_VALUE_LEN) & 0xFF;
		value_w = val & 0xFF;
		hwlog_info("\nWrite: start REG:0x%02x,val:0x%02x,count:0x%02x\n",
			reg, value_w, flag);
		while (flag--) {
			es7210_i2c_write(es7210->client, reg, value_w);
			hwlog_info("Write 0x%02x to REG:0x%02x\n",
				value_w, reg);
			reg++;
		}
	} else {
		reg = (val >> REG_VALUE_LEN) & 0xFF;
		num = val & 0xff;
		hwlog_info("\nRead: start REG:0x%02x,count:0x%02x\n", reg, num);
		do {
			value_r = 0;
			es7210_i2c_read(es7210->client, reg, &value_r);
			hwlog_info("REG[0x%02x]: 0x%02x;  ", reg, value_r);
			reg++;
			i++;
			if ((i == num) || (i % DIVISION_FOUR == 0))
				hwlog_info("\n");
		} while (i < num);
	}
	return count;
}

DEVICE_ATTR(es7210, 0644, es7210_show, es7210_store);

static struct attribute *es7210_debug_attrs[] = {
	&dev_attr_es7210.attr,
	NULL,
};

static struct attribute_group es7210_debug_attr_group = {
	.name   = "es7210_debug",
	.attrs  = es7210_debug_attrs,
};
#endif

static void es7210_config(struct i2c_client *i2c_client)
{
	u8 i;
	int len;
	u32 register_value;
	u32 sleep_time;
	struct device_node *node = NULL;

	hwlog_info("%s enter\n", __func__);

	node = of_find_compatible_node(NULL, NULL, mic_dts_node_name);
	if (node == NULL) {
		pr_err("can not find compatible node %s in dts\n",
			mic_dts_node_name);
		return;
	}

	len = get_adc_dts_array_byname(node, reg_value_table,
		(u32 *)g_register_table,
		REG_TABLE_COL_NUM * REG_TABLE_NUM_TOTAL);
	if (len < 0) {
		pr_err("no valid register_table, len=%d\n", len);
		return;
	}

	hwlog_info("%s, len is %d\n", reg_value_table, len);

	for (i = 0; i < len / REG_TABLE_NUM_TOTAL; i++) {
		register_value =
			g_register_table[i][VALUE_SLEEPTIME_INDEX] >> VALUE_HEX;
		sleep_time =
			g_register_table[i][VALUE_SLEEPTIME_INDEX] & 0xFFFF;

		es7210_i2c_write(i2c_client,
			g_register_table[i][REG_ADDRESS_INDEX], register_value);
		if (sleep_time != NO_DELAY)
			msleep(sleep_time);
	}
	hwlog_info("%s init end\n", __func__);
}

static int es7210_i2c_probe(struct i2c_client *i2c_client,
				const struct i2c_device_id *id)
{
	hwlog_info("%s %d\n", __func__, __LINE__);

	if (!i2c_client) {
		pr_err("i2c_client is null pointer\n");
		return -ENOMEM;
	}
	es7210_private = devm_kzalloc(&i2c_client->dev,
		sizeof(struct es7210_snd_private), GFP_KERNEL);
	if (!es7210_private)
		return -ENOMEM;

	i2c_set_clientdata(i2c_client, es7210_private);
	es7210_private->client = i2c_client;
	es7210_config(i2c_client);

#ifdef ES7210_DEBUG
	if (sysfs_create_group(&i2c_client->dev.kobj,
		&es7210_debug_attr_group) != 0)
		pr_err("failed to create attr group\n");
#endif

	return 0;
}

static int es7210_i2c_remove(struct i2c_client *client)
{
	devm_kfree(&client->dev, es7210_private);
	es7210_private = NULL;
#ifdef ES7210_DEBUG
	sysfs_remove_group(&client->dev.kobj, &es7210_debug_attr_group);
#endif
	return 0;
}

static const struct i2c_device_id es7210_id[] = {
	{"es7210", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, es7210_id);

static const struct dev_pm_ops es7210_pm_ops = {
	.suspend = es7210_suspend,
	.resume = es7210_resume,
};

static const struct of_device_id es7210_dt_ids[] = {
	{.compatible = "everest,es7210"},
	{},
};

static struct i2c_driver es7210_i2c_driver = {
	.driver = {
		.name = "es7210",
		.owner = THIS_MODULE,
		.pm = &es7210_pm_ops,
		.of_match_table = es7210_dt_ids,
	},
	.id_table = es7210_id,
	.probe = es7210_i2c_probe,
	.remove = es7210_i2c_remove,
};

static int __init es7210_init(void)
{
	return i2c_add_driver(&es7210_i2c_driver);
}

static void __exit es7210_exit(void)
{
	i2c_del_driver(&es7210_i2c_driver);
}

module_init(es7210_init);
module_exit(es7210_exit);

MODULE_DESCRIPTION("ASoC ES7210 ADC driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL");
