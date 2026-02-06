/*
 * echub_i2c.c
 *
 * I2C driver for echub
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mfd/core.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "echub.h"
#include "echub_i2c.h"

/* add regulator i2c transfer define */
#define REG_SEND_ADDR                    (0x38)
#define REG_SEND_ADAPTER                 (0x07)

static int major;
static struct class *class;
struct echub_i2c_dev *g_echub_i2c_data = NULL;

enum echub_cells {
	POWER    = 0,
	BATTERY  = 1,
	KEYBOARD = 2,
};

static struct mfd_cell echubs[] = {
	[POWER] = {
		.name = "echub-power",
		.of_compatible = "huawei,echub-power",
	},
	[BATTERY] = {
		.name = "echub-battery",
		.of_compatible = "huawei,echub-battery",
	},
	[KEYBOARD] = {
		.name = "echub-keyboard",
		.of_compatible = "huawei,echub-keyboard",
	},
};

struct echub_i2c_dev *get_echub_i2c_dev_data(void)
{
	return g_echub_i2c_data;
}

/**********************************************************
*  Function:       get_transfer_date
*  Discription:    get i2c read/write date
*  Parameters:     char devaddr, int regoffset,
*                  char regdata, char *value, int index
*  return value:   transfer_date
**********************************************************/
static unsigned char get_transfer_date(char devaddr, int regoffset,
					char regdata, char *value, int index)
{
	unsigned char data;

	switch (index) {
	case 0:
		data = (unsigned char)(devaddr<<1);
		break;
	case 1:
		data = (unsigned char)(regoffset >> BIT_OFFSET);
		break;
	case 2:
		data = (unsigned char)(regoffset&0xFF);
		break;
	case 3:
		data = REG_ECHUB_LEN;
		break;
	case 4:
		data = regdata;
		break;
	case 5:
		data = (unsigned char)(devaddr<<1)+1;
		break;
	case 6:
		data = value[0];
		break;
	case 7:
		data = value[1];
		break;
	case 8:
		data = value[2];
		break;
	default:
		echub_err("echub %s error\n", __func__);
		break;
	}
	return data;
}

/**********************************************************
*  Function:       calc_bus_PEC
*  Discription:    calculate_bus_transfer_PEC
*  Parameters:     char devaddr, int regoffset,
*                  char regdata, char *value, int type
*  return value:   i2cCmdPEC
**********************************************************/
static unsigned char calc_bus_PEC(char devaddr, int regoffset,
					char regdata, char *value, int type)
{
	static const char PEC_TABLE[] = {
		0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
		0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D
	};
	unsigned char i2cCmdPEC = 0;
	unsigned char sourcebyte = 0;
	int crc_len = 0;
	int i = 0;

	if (type == ECHUB_I2C_WRITE)
		crc_len = ECHUB_WRITE_LEN;
	else if (type == ECHUB_I2C_READ)
		crc_len = ECHUB_READ_LEN;
	else
		echub_err("%s input value err\n", __func__);

	for (i = 0; i < crc_len; i++) {
		unsigned char data = 0;
		unsigned char crc = 0;

		sourcebyte = get_transfer_date(devaddr, regoffset, regdata, value, i);
		data = i2cCmdPEC ^ sourcebyte;
		crc = (crc  << CRC_SHIFT) ^ PEC_TABLE[((crc ^ data) >> CRC_SHIFT) & CRC_MASK];
		data <<= CRC_SHIFT;
		i2cCmdPEC = (crc  << CRC_SHIFT) ^ PEC_TABLE[((crc ^ data) >> CRC_SHIFT) & CRC_MASK];
	}
	return i2cCmdPEC;
}

/**********************************************************
*  Function:       echub_i2c_read
*  Discription:    i2c read data from EC
*  Parameters:     char devaddr, int regoffset,
*                  char regdata ,char *value, int len
*  return value:   0-sucess or others-fail
**********************************************************/
ssize_t echub_i2c_read(struct echub_i2c_dev *echub_dev, int regoffset,
			char regdata, char *value, int len)
{
	struct i2c_client *client = NULL;
	struct i2c_adapter *adap = NULL;
	int ret;
	struct i2c_msg msg[2];
	unsigned char data[4];
	unsigned char devaddr;

	client = echub_dev->client;
	/* check client is valid */
	if (!client)
		return -ENODEV;

	devaddr = client->addr;
	adap = client->adapter;
	/* check adapter is valid */
	if (!adap)
		return -ENODEV;

	/* write data buffer */
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset&0xFF);
	data[2] = REG_DATE_LEN;
	data[3] = regdata;

	/* read data buffer */
	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, ECHUB_SEND_TRANS_LEN);

	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN)
		return -ENODEV;
	if (value[0]) {
		echub_err("%s failed, value[0] = %d\n", __func__, value[0]);
		return -EIO;
	}
	if (value[1] != REG_DATE_LEN)
		return -EINVAL;
	if (value[3] != calc_bus_PEC(devaddr, regoffset, regdata, value, ECHUB_I2C_READ))
		return -EBADMSG;

	return 0;
}

/**********************************************************
*  Function:       echub_i2c_write_regulator
*  Discription:    i2c read data for regulator moudule
*  Parameters:     u16 regoffset,char regdata
*  return value:   0-sucess or others-fail
**********************************************************/
int echub_i2c_write_regulator(u16 regoffset, char regdata)
{
	int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg[2];
	unsigned char data[4];
	unsigned char value[2];
	unsigned char devaddr = REG_SEND_ADDR;

	adap = i2c_get_adapter(REG_SEND_ADAPTER);
	/* check adapter is valid */
	if (!adap) {
		echub_err("%s adap error\n", __func__);
		return -ENODEV;
	}

	/* write data buffer */
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset&0xFF);
	data[2] = REG_DATE_LEN;
	data[3] = regdata;

	/* read data buffer */
	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = ECHUB_SEND_TRANS_LEN;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, ECHUB_SEND_TRANS_LEN);
	i2c_put_adapter(adap);

	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN) {
		echub_err("%s TRANS_LEN error\n", __func__);
		return -ENODEV;
	}
	if (value[0]) {
		echub_err("%s value[0] error\n", __func__);
		return -EIO;
	}
	if (value[1] != calc_bus_PEC(devaddr, regoffset, regdata, value, ECHUB_I2C_WRITE)) {
		echub_err("%s calc error\n", __func__);
		return -EBADMSG;
	}

	return 0;
}

/**********************************************************
*  Function:       echub_i2c_write
*  Discription:    i2c write data to EC
*  Parameters:     char devaddr, int regoffset, char regdata
*  return value:   0-sucess or others-fail
**********************************************************/
ssize_t echub_i2c_write(struct echub_i2c_dev *echub_dev, int regoffset, char regdata)
{
	struct i2c_client *client = NULL;
	struct i2c_adapter *adap = NULL;
	int ret;
	struct i2c_msg msg[2];
	unsigned char data[4];
	unsigned char value[2];
	unsigned char devaddr;

	client = echub_dev->client;
	/* check client is valid */
	if (!client)
		return -ENODEV;

	devaddr = client->addr;
	adap = client->adapter;
	/* check adap is valid */
	if (!adap)
		return -ENODEV;

	/* write data buffer */
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset&0xFF);
	data[2] = REG_DATE_LEN;
	data[3] = regdata;

	/* read data buffer */
	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = ECHUB_SEND_TRANS_LEN;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, ECHUB_SEND_TRANS_LEN);

	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN)
		return -ENODEV;
	if (value[0])
		return -EIO;
	if (value[1] != calc_bus_PEC(devaddr, regoffset, regdata, value, ECHUB_I2C_WRITE))
		return -EBADMSG;

	return 0;
}

static struct file_operations echub_i2c_fops = {
	.owner = THIS_MODULE,
	/* add fops used in user space */
};

static struct ec_platform_data *echub_i2c_parse_dt_pdata(struct device *dev)
{
	struct ec_platform_data *pd;

	pd = devm_kzalloc(dev, sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return ERR_PTR(-ENOMEM);
	/*
	 * ToDo: the 'wakeup' member in the platform data is more of a linux
	 * specfic information. Hence, there is no binding for that yet and
	 * not parsed here.
	*/
	return pd;
}

/**********************************************************
*  Function:       ec_i2c_probe
*  Discription:    module probe
*  Parameters:     platform_device, i2c_device_id
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	struct ec_platform_data *pdata = dev_get_platdata(&client->dev);
	struct echub_i2c_dev *echub_i2c_data = NULL;

	echub_info("%s enter\n", __func__);

	echub_i2c_data = devm_kzalloc(&client->dev, sizeof(*echub_i2c_data), GFP_KERNEL);
	if (!echub_i2c_data) {
		echub_err("%s kzalloc fail\n", __func__);
		return -ENOMEM;
	}

	if (IS_ENABLED(CONFIG_OF) && client->dev.of_node) {
		pdata = echub_i2c_parse_dt_pdata(&client->dev);
		if (IS_ERR(pdata)) {
			echub_err("echub parse dt pdata fail\n");
			return PTR_ERR(pdata);
		}
	}

	echub_i2c_data->dev = &client->dev;

	/* init struct echub_i2c_dev echub_i2c_data */
	mutex_init(&echub_i2c_data->lock);
	echub_i2c_data->client = client;
	echub_i2c_data->write_func = echub_i2c_write;
	echub_i2c_data->read_func = echub_i2c_read;
	echub_i2c_data->pdata = pdata;
	i2c_set_clientdata(client, echub_i2c_data);
	/* set i2c data  tmp */
	g_echub_i2c_data = echub_i2c_data;

	/*
	 * Perform a one-byte test read to verify that the
	 * chip is functional.
	 */
	err = i2c_smbus_read_byte(client);
	if (err)
		echub_err("nothing at this address\n");

	/*
	 * Create a mfd devices with two cells
	 * [POWER] && [BATTERY]
	 */
	err = mfd_add_devices(echub_i2c_data->dev, -1, echubs,
				  ARRAY_SIZE(echubs), NULL, 0, NULL);
	if (err)
		echub_err("add mfd devices failed with err: %d\n", err);

	/*
	 * Create a char device named echub_i2c with echub_i2c_fops
	 * /dev/echub_i2c
	 */
	major = register_chrdev(0, "echub_i2c", &echub_i2c_fops);
	class = class_create(THIS_MODULE, "echub_i2c");
	device_create(class, NULL, MKDEV(major, 0), NULL, "echub_i2c");
	echub_info("%s ok\n", __func__);

	return 0;
}

/**********************************************************
*  Function:       ec_state_sync_remove
*  Discription:    module remove
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_i2c_remove(struct i2c_client *client)
{
	echub_info("%s\n", __func__);
	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "echub_i2c");

	return 0;
}

static const struct of_device_id echub_i2c_of_match[] = {
	{ .compatible = "huawei,echub_i2c" },
	{},
};

MODULE_DEVICE_TABLE(of, echub_i2c_of_match);

static const struct i2c_device_id echub_i2c_id_table[] = {
	{ "echub_i2c", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, echub_i2c_id_table);

static struct i2c_driver echub_i2c_driver = {
	.driver = {
		.name           = "echub_i2c",
		.of_match_table = of_match_ptr(echub_i2c_of_match),
	},

	.probe       = echub_i2c_probe,
	.remove      = echub_i2c_remove,
	.id_table    = echub_i2c_id_table,
};

/**********************************************************
*  Function:       echub_i2c_init
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int __init echub_i2c_init(void)
{
	echub_info("%s\n", __func__);
	return i2c_add_driver(&echub_i2c_driver);
}

module_init(echub_i2c_init);

/**********************************************************
*  Function:       echub_i2c_exit
*  Discription:    module exit
*  Parameters:     NULL
*  return value:   NULL
**********************************************************/
static void __exit echub_i2c_exit(void)
{
	echub_info("%s\n", __func__);
	i2c_del_driver(&echub_i2c_driver);
}

module_exit(echub_i2c_exit);

MODULE_ALIAS("huawei echub i2c module");
MODULE_DESCRIPTION("huawei echub i2c driver");
MODULE_LICENSE("GPL");
