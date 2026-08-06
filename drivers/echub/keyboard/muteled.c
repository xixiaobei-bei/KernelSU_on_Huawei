/*
 * muteled.c
 *
 * LEDs driver for F7 hotkey
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

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/mfd/core.h>
#include <linux/ioctl.h>
#include "../echub.h"
#include "../echub_i2c.h"

#define DEVICE_NAME "muteled"
#define CLOSE_LED                 (1)
#define OPEN_LED                  (0)
#define MUTELED_REG               (0x036E)
#define MUTELED_OPEN_DATA         (0x03)
#define MUTELED_CLOSE_DATA        (0x03)
#define MUTELED_READ_LEN          (0x02)
#define MUTELED_WRITE_LEN         (0x02)

#define MUTELED_REG_OPEN          (0x55)
#define MUTELED_REG_CLOSE         (0x5A)

struct echub_muteled {
	struct device *dev;
	/* MFD driver */
	struct echub_i2c_dev *mfd;
	int led_status;
	int events;
};
struct echub_muteled g_muteled_data;

/**********************************************************
*  Function:       echub_muteled_open
*  Discription:    mute led control - open
*  Parameters:     struct echub_muteled *muteled_data
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_muteled_open(struct echub_muteled *muteled_data)
{
	return muteled_data->mfd->write_func(muteled_data->mfd, MUTELED_REG, MUTELED_REG_OPEN);
}

/**********************************************************
*  Function:       echub_muteled_close
*  Discription:    mute led control - close
*  Parameters:     struct echub_muteled *muteled_data
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_muteled_close(struct echub_muteled *muteled_data)
{
	return muteled_data->mfd->write_func(muteled_data->mfd, MUTELED_REG, MUTELED_REG_CLOSE);
}

/**********************************************************
*  Function:       echub_muteled_read
*  Discription:    Read mute_led status
*  Parameters:     struct echub_muteled *muteled_data, char *data
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_muteled_read(struct echub_muteled *muteled_data, int *data)
{
	/* Add function */
	return 0;
}

/**********************************************************
*  Function:       mute_led_ioctl
*  Discription:    OS hotkey-service I/O control
*  Parameters:     struct file *flip,unsigned int cmd, unsigned long arg
*  return value:   0-sucess or others-fail
**********************************************************/
static long mute_led_ioctl(struct file *flip, unsigned int cmd,
			unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case CLOSE_LED:
		/* close LED */
		ret = echub_muteled_close(&g_muteled_data);
		if (ret) {
			echub_err("%s echub_muteled_close err\n", __func__);
			ret = echub_muteled_close(&g_muteled_data);
		}
		echub_info("%s close LED ret = %d\n", __func__, ret);
		break;
	case OPEN_LED:
		/* open LED */
		ret = echub_muteled_open(&g_muteled_data);
		if (ret) {
			echub_err("%s echub_muteled_open err\n", __func__);
			ret = echub_muteled_open(&g_muteled_data);
		}
		echub_info("%s open LED ret = %d\n", __func__, ret);
		break;
	default:
		echub_err("%s input value err\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**********************************************************
* Function:		mute_led_write
* Discription:	OS hotkey-service write
* Parameters:	struct file *file, const char __user *buf,
				size_t count, loff_t *ppos
* return value:	0-sucess or others-fail
**********************************************************/
static ssize_t mute_led_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	/* add read mute led control */
	return 0;
}

/**********************************************************
* Function:		mute_led_read
* Discription:	OS hotkey-service read
* Parameters:	struct file *filp, char __user *buf,
				size_t size, loff_t *ppos
* return value:	0-sucess or others-fail
**********************************************************/
static ssize_t mute_led_read(struct file *filp, char __user *buf,
				size_t size, loff_t *ppos)
{
	/* add read mute led status */
	return 0;
}

/* init file_operations struct for OS */
static struct file_operations mute_led_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = mute_led_ioctl,
	.write          = mute_led_write,
	.read           = mute_led_read,
};

/**********************************************************
*  Function:       echub_muteled_probe
*  Discription:    module probe
*  Parameters:     struct platform_device *pdev
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_muteled_probe(struct platform_device *pdev)
{
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);
	int ret = 0;
	if (echub_dev == NULL) {
		printk("echub muteled dev null get echub_dev form tmp");
		echub_dev = get_echub_i2c_dev_data();
		if (echub_dev == NULL) {
			printk("echub_dev and g_echub_i2c_data is NULL return");
			return 0;
		}
	}

	echub_info("%s enter\n", __func__);
	g_muteled_data.dev = &pdev->dev;
	g_muteled_data.mfd = echub_dev;
	g_muteled_data.dev = echub_dev->dev;
	g_muteled_data.led_status = 0;
	g_muteled_data.events = 0;

	/* Check for muteled presence */
	ret = echub_muteled_read(&g_muteled_data, &g_muteled_data.led_status);
	if (ret)
		echub_err("%s(): Error in reading reg\n", __func__);

	echub_info("%s ok\n", __func__);

	return 0;
}

/* init miscdevice mute_led_dev */
static struct miscdevice mute_led_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	.fops  = &mute_led_dev_fops,
};

/* init platform_driver echub_muteled_driver */
static struct platform_driver echub_muteled_driver = {
	.driver = {
		.name = "echub-keyboard",
	},
	.probe = echub_muteled_probe,
};

/**********************************************************
*  Function:       mute_led_dev_init
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int __init mute_led_dev_init(void)
{
	int ret;

	echub_info("echub_muteled_init\n");
	ret = misc_register(&mute_led_dev);
	if (ret)
		echub_err("mute_led misc_register err\n");

	return platform_driver_register(&echub_muteled_driver);
}

/**********************************************************
*  Function:       mute_led_dev_exit
*  Discription:    module exit
*  Parameters:     NULL
*  return value:   NULL
**********************************************************/
static void __exit mute_led_dev_exit(void)
{
	echub_info("echub_muteled_exit\n");
	misc_deregister(&mute_led_dev);
	platform_driver_unregister(&echub_muteled_driver);
}

module_init(mute_led_dev_init);
module_exit(mute_led_dev_exit);

MODULE_ALIAS("huawei F7_hotkey_muteled");
MODULE_DESCRIPTION("huawei LED driver");
MODULE_LICENSE("GPL");
