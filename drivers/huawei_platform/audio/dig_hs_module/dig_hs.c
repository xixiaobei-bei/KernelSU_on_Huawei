/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset interface driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "huawei_platform/audio/dig_hs.h"
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
#include <linux/of_platform.h>
#include <linux/mutex.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG dig_hs
HWLOG_REGIST();

static struct list_head dev_list;
static struct mutex g_mutex;
static int g_init_flag;

void dig_hs_dev_register(struct dig_hs_dev *dev, int priority)
{
	if (priority == PRIORITY_H) {
		mutex_lock(&g_mutex);
		list_add(&(dev->list), &dev_list);
		mutex_unlock(&g_mutex);
	} else {
		mutex_lock(&g_mutex);
		list_add_tail(&(dev->list), &dev_list);
		mutex_unlock(&g_mutex);
	}
}

void dig_hs_dev_deregister(enum dig_hs_tag tag)
{
	struct dig_hs_dev *p = NULL;

	list_for_each_entry(p, &dev_list, list) {
		if (p->tag == tag) {
			mutex_lock(&g_mutex);
			list_del(&(p->list));
			mutex_unlock(&g_mutex);
			return;
		}
	}
}

unsigned int dig_hs_plug_in(void)
{
	struct dig_hs_dev *p = NULL;
	unsigned int ret = 0;
	int temp;

	if (g_init_flag == 0)
		return 0;

	list_for_each_entry(p, &dev_list, list) {
		if (p->ops.dig_hs_plug_in != NULL) {
			/* record return value of each handler */
			temp = p->ops.dig_hs_plug_in();
			if (temp != 0)
				ret |= (1 << p->tag);
		}
	}

	return ret;
}

unsigned int dig_hs_plug_out(void)
{
	struct dig_hs_dev *p = NULL;
	unsigned int ret = 0;
	int temp;

	if (g_init_flag == 0)
		return 0;

	list_for_each_entry(p, &dev_list, list) {
		if (p->ops.dig_hs_plug_out != NULL) {
			/* record return value of each handler */
			temp = p->ops.dig_hs_plug_out();
			if (temp != 0)
				ret |= (1 << p->tag);
		}
	}

	return ret;
}

int dig_hs_plug_in_single(enum dig_hs_tag tag)
{
	struct dig_hs_dev *p = NULL;
	int ret = 0;

	if (g_init_flag == 0)
		return 0;

	list_for_each_entry(p, &dev_list, list) {
		if (p->tag == tag && p->ops.dig_hs_plug_in != NULL) {
			ret = p->ops.dig_hs_plug_in();
			return ret;
		}
	}

	return 0;
}

int dig_hs_plug_out_single(enum dig_hs_tag tag)
{
	struct dig_hs_dev *p = NULL;
	int ret = 0;

	if (g_init_flag == 0)
		return 0;

	list_for_each_entry(p, &dev_list, list) {
		if (p->tag == tag && p->ops.dig_hs_plug_out != NULL) {
			ret = p->ops.dig_hs_plug_out();
			return ret;
		}
	}

	return 0;
}

static const struct of_device_id dig_hs_core_match_table[] = {
	{
		.compatible = "huawei,dig_hs_core",
		.data = NULL,
	},
	{
		.compatible = "huawei,dig_hs_power",
		.data = NULL,
	},
	{
		.compatible = "huawei,dig_hs_power_v600",
		.data = NULL,
	},
	{
	},
};

static int dig_hs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	INIT_LIST_HEAD(&dev_list);
	mutex_init(&g_mutex);
	g_init_flag = 1;

	of_platform_populate(np, dig_hs_core_match_table, NULL, dev);

	return 0;
}

static int dig_hs_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id dig_hs_match_table[] = {
	{
		.compatible = "huawei,dig_hs",
		.data = NULL,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, dig_hs_match_table);

static struct platform_driver dig_hs_driver = {
	.probe = dig_hs_probe,
	.remove = dig_hs_remove,
	.driver = {
		.name = "huawei,dig_hs",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dig_hs_match_table),
	},
};

static int __init dig_hs_init(void)
{
	return platform_driver_register(&dig_hs_driver);
}

static void __exit dig_hs_exit(void)
{
	platform_driver_unregister(&dig_hs_driver);
}

subsys_initcall(dig_hs_init);
module_exit(dig_hs_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("digital headset module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
