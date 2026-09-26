/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: analog headset module driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "ana_hs.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG ana_hs
HWLOG_REGIST();

struct ana_hs_data {
	int headset_type;
	int ana_hs_init_flag;
	struct mutex mutex;
	struct list_head dev_list;
};
static struct ana_hs_data pdata;

void ana_hs_refresh_headset_type(int headset_type)
{
	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return;
	}

	pdata.headset_type = headset_type;
	hwlog_info("hs_default: refresh headset_type %d\n", pdata.headset_type);
}

void ana_hs_dev_register(struct ana_hs_dev *dev, int priority)
{
	if (priority == ANA_PRIORITY_H) {
		mutex_lock(&(pdata.mutex));
		list_add(&(dev->list), &(pdata.dev_list));
		mutex_unlock(&(pdata.mutex));
	} else {
		mutex_lock(&(pdata.mutex));
		list_add_tail(&(dev->list), &(pdata.dev_list));
		mutex_unlock(&(pdata.mutex));
	}
}

void ana_hs_dev_deregister(enum ana_hs_tag tag)
{
	struct ana_hs_dev *p = NULL;
	struct ana_hs_dev *n = NULL;

	list_for_each_entry_safe(p, n, &(pdata.dev_list), list) {
		if (p->tag == tag) {
			mutex_lock(&(pdata.mutex));
			list_del(&(p->list));
			mutex_unlock(&(pdata.mutex));
			return;
		}
	}
}

int ana_hs_pluged_state(void)
{
	struct ana_hs_dev *p = NULL;
	int ret;

	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return ANA_HS_PLUG_OUT;
	}

	list_for_each_entry(p, &(pdata.dev_list), list) {
		if (p->tag == TAG_ANA_HS_CORE &&
			p->ops.ana_hs_pluged_state != NULL) {
			ret = p->ops.ana_hs_pluged_state();
			return ret;
		}
	}

	return ANA_HS_PLUG_OUT;
}

bool ana_hs_support_usb_sw(void)
{
	struct ana_hs_dev *p = NULL;
	bool ret = false;

	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return false;
	}

	list_for_each_entry(p, &(pdata.dev_list), list) {
		if (p->tag == TAG_ANA_HS_CORE &&
			p->ops.ana_hs_support_usb_sw != NULL) {
			ret = p->ops.ana_hs_support_usb_sw();
			return ret;
		}
	}

	return false;
}

int ana_hs_codec_dev_register(struct ana_hs_codec_dev *dev, void *codec_data)
{
	struct ana_hs_dev *p = NULL;
	int ret = 0;

	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return -ENODEV;
	}

	list_for_each_entry(p, &(pdata.dev_list), list) {
		if (p->tag == TAG_ANA_HS_CORE &&
			p->ops.ana_hs_dev_register != NULL) {
			ret = p->ops.ana_hs_dev_register(dev, codec_data);
			return ret;
		}
	}

	return -ENODEV;
}

void ana_hs_plug_handle(int hs_state)
{
	struct ana_hs_dev *p = NULL;

	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return;
	}

	list_for_each_entry(p, &(pdata.dev_list), list) {
		if (p->ops.ana_hs_plug_handle != NULL)
			p->ops.ana_hs_plug_handle(hs_state);
	}
}

void ana_hs_mic_gnd_swap(void)
{
	struct ana_hs_dev *p = NULL;

	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return;
	}

	list_for_each_entry(p, &(pdata.dev_list), list) {
		if (p->ops.ana_hs_mic_gnd_swap != NULL)
			p->ops.ana_hs_mic_gnd_swap();
	}
}

void ana_hs_invert_hs_control(int connect)
{
	struct ana_hs_dev *p = NULL;

	if (pdata.ana_hs_init_flag == 0) {
		hwlog_info("%s: ana hs is not init\n", __func__);
		return;
	}

	list_for_each_entry(p, &(pdata.dev_list), list) {
		if (p->ops.invert_hs_control != NULL)
			p->ops.invert_hs_control(connect);
	}
}

static long ana_hs_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	int ret = 0;
	int adc_value;
	uintptr_t arg_tmp = (uintptr_t)arg;
	unsigned int __user *p_user = (unsigned int __user *)arg_tmp;

	switch (cmd) {
	case IOCTL_ANA_HS_GET_HEADSET_CMD:
		ret = put_user((__u32)(pdata.headset_type),
			p_user);
		break;
	case IOCTL_ANA_HS_GET_HEADSET_RESISTANCE_CMD:
		adc_value = 0;
		ret = put_user((__u32)(adc_value), p_user);
		break;
	default:
		hwlog_err("%s unsupport cmd\n", __func__);
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations ana_hs_fops = {
	.owner               = THIS_MODULE,
	.open                = simple_open,
	.unlocked_ioctl      = ana_hs_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl        = ana_hs_ioctl,
#endif
};

static struct miscdevice ana_hs_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "ana_hs",
	.fops   = &ana_hs_fops,
};

static const struct of_device_id ana_hs_core_match_table[] = {
	{
		.compatible = "huawei,ana_hs_core",
		.data = NULL,
	},
	{
		.compatible = "huawei,invert_hs_core",
		.data = NULL,
	},
	{
	},
};

static int ana_hs_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	hwlog_info("ana_hs_probe++\n");

	INIT_LIST_HEAD(&(pdata.dev_list));
	mutex_init(&(pdata.mutex));

	/* register misc device for userspace */
	ret = misc_register(&ana_hs_device);
	if (ret) {
		hwlog_err("%s misc device register failed\n", __func__);
		goto err_misc_register;
	}

	of_platform_populate(np, ana_hs_core_match_table, NULL, dev);

	pdata.ana_hs_init_flag = 1;
err_misc_register:
	return ret;
}

static int ana_hs_remove(struct platform_device *pdev)
{
	hwlog_info("%s: enter\n", __func__);

	misc_deregister(&ana_hs_device);

	pdata.ana_hs_init_flag = 0;
	return 0;
}

static const struct of_device_id ana_hs_match_table[] = {
	{
		.compatible = "huawei,ana_hs",
		.data = NULL,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, ana_hs_match_table);

static struct platform_driver ana_hs_driver = {
	.probe = ana_hs_probe,
	.remove = ana_hs_remove,

	.driver = {
		.name = "huawei,ana_hs",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ana_hs_match_table),
	},
};

static int __init ana_hs_init(void)
{
	return platform_driver_register(&ana_hs_driver);
}

static void __exit ana_hs_exit(void)
{
	platform_driver_unregister(&ana_hs_driver);
}

subsys_initcall(ana_hs_init);
module_exit(ana_hs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("analog headset module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
