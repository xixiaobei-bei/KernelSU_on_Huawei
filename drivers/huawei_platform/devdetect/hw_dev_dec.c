/*
 * hw_dev_dec.c
 *
 * device detect driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/devdetect/hw_dev_dec.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG                  dev_detect
HWLOG_REGIST();

#define FLAG_NOT_SET               '0'
#define FLAG_SET                   '1'
#define DEV_DETECT_PROBE_SUCCESS    1
/* 64 devices need to detect */
#define DEV_DETECT_NUMS             64
#define PRINT_MAX_LEN               128
/* the compatible name in the dts */
#define DEV_DETECT_NAME             "huawei,dev_detect"
#define DEV_DETECT_DEFAULT          \
"0000000000000000000000000000000000000000000000000000000000000000"

struct detect_flag_device {
	const char *name;
	struct device *dev;
	const unsigned int index;
	const struct device_attribute attr;
};

enum dec_detect_station {
	NO_DETECT   = 0, /* no need to detect */
	DBC_DETECT  = 1, /* dbc station detect */
	RT_DETECT   = 2, /* rt station detect */
	MMI1_DETECT = 4, /* mmi1 factory detect */
	MMI2_DETECT = 8, /* mmi2 normal detect */
};

static size_t dev_index;
static int dev_detect_probe_flag;
static DEFINE_MUTEX(devdetect_lock);
static unsigned long dev_node_create_flag;
static unsigned long dev_file_create_flag;

/* set the detect result to '1', when devices have been detected successful */
static char dev_detect_result[DEV_DETECT_NUMS + 1] = DEV_DETECT_DEFAULT;
/* save the message of dts configs, which need to detect at different station */
static char dbc_detect_flag[DEV_DETECT_NUMS + 1] = DEV_DETECT_DEFAULT;
static char rt_detect_flag[DEV_DETECT_NUMS + 1] = DEV_DETECT_DEFAULT;
static char mmi1_detect_flag[DEV_DETECT_NUMS + 1] = DEV_DETECT_DEFAULT;
static char mmi2_detect_flag[DEV_DETECT_NUMS + 1] = DEV_DETECT_DEFAULT;

/*
 * function: set the device detect flag in dev_detect_result[].
 * input: dev_id: device flag set place in dev_detect_result[].
 */
int set_hw_dev_flag(int dev_id)
{
	if (dev_id < 0 || dev_id >= (int)DEV_I2C_MAX) {
		hwlog_err("%s: dev_id = %d is invalid\n", __func__, dev_id);
		return false;
	}

	mutex_lock(&devdetect_lock);
	dev_detect_result[dev_id] = FLAG_SET;
	mutex_unlock(&devdetect_lock);
	hwlog_info("%s: set dev_id = %d successful\n", __func__, dev_id);
	return true;
}
EXPORT_SYMBOL(set_hw_dev_flag);

static ssize_t dev_detect_result_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: detect result is:%s\n", __func__, dev_detect_result);
	return snprintf((char *)buf, PRINT_MAX_LEN, "%s\n", dev_detect_result);
}

static ssize_t dbc_detect_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: dbc_detect_flag is:%s\n", __func__, dbc_detect_flag);
	return snprintf((char *)buf, PRINT_MAX_LEN, "%s\n", dbc_detect_flag);
}

static ssize_t rt_detect_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: rt_detect_flag is:%s\n", __func__, rt_detect_flag);
	return snprintf((char *)buf, PRINT_MAX_LEN, "%s\n", rt_detect_flag);
}

static ssize_t mmi1_detect_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: mmi1_detect_flag is:%s\n", __func__, mmi1_detect_flag);
	return snprintf((char *)buf, PRINT_MAX_LEN, "%s\n", mmi1_detect_flag);
}

static ssize_t mmi2_detect_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: mmi2_detect_flag is:%s\n", __func__, mmi2_detect_flag);
	return snprintf((char *)buf, PRINT_MAX_LEN, "%s\n", mmi2_detect_flag);
}

static struct detect_flag_device dev_list[] = {
	{ .name = "dev_detect_result_flag", .index = 0,
	.attr = __ATTR(dev_detect_result_flag, 0444,
			dev_detect_result_show, NULL) },
	{ .name = "dev_detect_dbc_flag", .index = 0,
	.attr = __ATTR(dev_detect_dbc_flag, 0444,
			dbc_detect_flag_show, NULL) },
	{ .name = "dev_detect_rt_flag", .index = 0,
	.attr = __ATTR(dev_detect_rt_flag, 0444,
			rt_detect_flag_show, NULL) },
	{ .name = "dev_detect_mmi1_flag", .index = 0,
	.attr = __ATTR(dev_detect_mmi1_flag, 0444,
			mmi1_detect_flag_show, NULL) },
	{ .name = "dev_detect_mmi2_flag", .index = 0,
	.attr = __ATTR(dev_detect_mmi2_flag, 0444,
			mmi2_detect_flag_show, NULL) },
};

static int create_dev_detect_sysfs(struct class *myclass)
{
	int ret;

	dev_node_create_flag = 0;
	dev_file_create_flag = 0;
	hwlog_info("%s: function begin\n", __func__);

	for (dev_index = 0; dev_index < ARRAY_SIZE(dev_list); dev_index++) {
		dev_list[dev_index].dev = device_create(myclass, NULL,
			MKDEV(0, dev_list[dev_index].index),
			NULL, dev_list[dev_index].name);
		if (dev_list[dev_index].dev == NULL) {
			hwlog_err("%s: failed to create %s device\n",
				__func__, dev_list[dev_index].name);
			return -1;
		}
		(void)test_and_set_bit(dev_index, &dev_node_create_flag);

		ret = device_create_file(dev_list[dev_index].dev,
					&dev_list[dev_index].attr);
		if (ret < 0) {
			hwlog_err("%s: failed to create %s file\n",
				__func__, dev_list[dev_index].name);
			return -1;
		}
		(void)test_and_set_bit(dev_index, &dev_file_create_flag);
	}

	hwlog_info("%s: dev_index:%zu, node_flag:%zu, file_flag:%zu", __func__,
		dev_index, dev_node_create_flag, dev_file_create_flag);
	return 0;
}

static void remove_dev_detect_sysfs(struct class *myclass)
{
	size_t j;

	for (j = 0; j < dev_index; j++) {
		if (test_and_clear_bit(j, &dev_file_create_flag))
			device_remove_file(dev_list[j].dev, &dev_list[j].attr);

		if (test_and_clear_bit(j, &dev_node_create_flag))
			device_destroy(myclass, dev_list[j].dev->devt);
	}

	dev_index = 0;
	dev_node_create_flag = 0;
	dev_file_create_flag = 0;
	class_destroy(myclass);
	hwlog_info("%s: remove dev detect sysfs end\n", __func__);
}

static void get_dev_detect_dts_config(struct device_node *np)
{
	size_t i;
	int result;
	unsigned int type;

	hwlog_info("%s: function begin\n", __func__);
	for (i = 0; i < ARRAY_SIZE(dev_detect_table); i++) {
		result = of_property_read_u32(np,
			dev_detect_table[i].device_name, &type);
		if (!result) {
			dbc_detect_flag[i] =
				((type != NO_DETECT) && (type & DBC_DETECT))
				? FLAG_SET : FLAG_NOT_SET;
			rt_detect_flag[i] =
				((type != NO_DETECT) && (type & RT_DETECT))
				? FLAG_SET : FLAG_NOT_SET;
			mmi1_detect_flag[i] =
				((type != NO_DETECT) && (type & MMI1_DETECT))
				? FLAG_SET : FLAG_NOT_SET;
			mmi2_detect_flag[i] =
				((type != NO_DETECT) && (type & MMI2_DETECT))
				? FLAG_SET : FLAG_NOT_SET;
		} else {
			dbc_detect_flag[i] = FLAG_NOT_SET;
			rt_detect_flag[i] = FLAG_NOT_SET;
			mmi1_detect_flag[i] = FLAG_NOT_SET;
			mmi2_detect_flag[i] = FLAG_NOT_SET;
			hwlog_err("get devid:%zu device name:%s value fail\n",
				i, dev_detect_table[i].device_name);
		}
	}

	hwlog_info("%s: function end\n", __func__);
}

static int dev_detect_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = NULL;
	struct class *myclass = NULL;

	if (dev_detect_probe_flag) {
		hwlog_err("%s: dev_detect_probe is done\n", __func__);
		return 0;
	}

	hwlog_info("%s: function begin\n", __func__);
	if (!pdev) {
		hwlog_err("%s: none device\n", __func__);
		return -ENODEV;
	}

	np = pdev->dev.of_node;
	if (!np) {
		hwlog_err("%s: unable to find device node\n", __func__);
		return -ENODEV;
	}

	myclass = class_create(THIS_MODULE, "dev_detect");
	if (IS_ERR(myclass)) {
		hwlog_err("%s: failed to create dev_detect class\n", __func__);
		return -1;
	}

	if (create_dev_detect_sysfs(myclass) < 0) {
		hwlog_err("%s: create dev detect sysfs failed\n", __func__);
		ret = -1;
		goto free_dev_and_file;
	}

	get_dev_detect_dts_config(np);

	dev_detect_probe_flag = DEV_DETECT_PROBE_SUCCESS;
	hwlog_info("%s: function end!\n", __func__);
	return 0;

free_dev_and_file:
	remove_dev_detect_sysfs(myclass);
	return ret;
}

static const struct of_device_id device_detect_match_table[] = {
	{
		.compatible = DEV_DETECT_NAME,
		.data = NULL,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, device_detect_match_table);

static struct platform_driver dev_detect_driver = {
	.driver = {
		.name = DEV_DETECT_NAME,
		.of_match_table = of_match_ptr(device_detect_match_table),
	},
	.probe  = dev_detect_probe,
	.remove = NULL,
};

static int __init hw_dev_detect_init(void)
{
	return platform_driver_register(&dev_detect_driver);
}

static void __exit hw_dev_detect_exit(void)
{
	platform_driver_unregister(&dev_detect_driver);
}

/* priority is 7s */
late_initcall_sync(hw_dev_detect_init);
module_exit(hw_dev_detect_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("device detect driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_ALIAS("platform:dev_detect");
