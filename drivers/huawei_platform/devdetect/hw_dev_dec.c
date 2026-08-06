/*
 * hw_dev_dec.c
 *
 * Device detect solution which version is 1.0,include MMI test and RT test.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/crc-ccitt.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <huawei_platform/devdetect/hw_dev_array.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG devcheck
HWLOG_REGIST();

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#define NO_CHECK 0
#define BC_CHECK 1
#define RT_CHECK 2
#define PRINT_MAX_LEN 22
#define DTS_COMP_DEVICE_CHECK_NAME "huawei,device_check"

struct dev_flag_device {
	const char *name;
	struct device *dev;
	const int index;
	const struct device_attribute attr;
};

static uint64_t boardid_devcheck_flag;
static uint64_t rt_devcheck_flag;
static uint64_t dev_flag;

static int dev_index;
static int dev_dct_probe_flag;
static DEFINE_MUTEX(devcheck_lock);
static size_t dev_node_create_flag;
static size_t dev_file_create_flag;

int set_hw_dev_flag(int dev_id)
{
	if (dev_id < 0 || dev_id >= (int)DEV_I2C_MAX) {
		hwlog_err("%s: dev_id = %d is invalid\n", __func__, dev_id);
		return false;
	}

	mutex_lock(&devcheck_lock);
	dev_flag = dev_flag | ((uint64_t)1 << (unsigned int)dev_id);
	hwlog_info("set dev_flag succes, dev_id = %d\n", dev_id);
	mutex_unlock(&devcheck_lock);

	return true;
}
EXPORT_SYMBOL(set_hw_dev_flag);

static ssize_t result_check_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("result_check_flag is: %llu!\n", dev_flag);
	return snprintf(buf, PRINT_MAX_LEN, "%llu\n", dev_flag);
}

static ssize_t dbc_check_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("dbc_check_flag is: %llu!\n", boardid_devcheck_flag);
	return snprintf(buf, PRINT_MAX_LEN, "%llu\n", boardid_devcheck_flag);
}

static ssize_t rt_check_flag_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hwlog_info("rt_devcheck_flag is: %llu!\n", rt_devcheck_flag);
	return snprintf(buf, PRINT_MAX_LEN, "%llu\n", rt_devcheck_flag);
}

static struct dev_flag_device dev_list[] = {
	{ .name = "dev_flag", .index = 0,
	.attr = __ATTR(dev_flag, 0444,
			result_check_flag_show, NULL) },
	{ .name = "boardid_devcheck_flag", .index = 0,
	.attr = __ATTR(boardid_devcheck_flag, 0444,
			dbc_check_flag_show, NULL) },
	{ .name = "rt_devcheck_flag", .index = 0,
	.attr = __ATTR(rt_devcheck_flag, 0444,
			rt_check_flag_show, NULL) },
};

static int create_dev_detect_sysfs(struct class *myclass)
{
	int ret;
	int device_array_size = ARRAY_SIZE(dev_list);

	dev_node_create_flag = 0;
	dev_file_create_flag = 0;

	for (dev_index = 0; dev_index < device_array_size; dev_index++) {
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

	hwlog_err("%s: dev_index:%d, node_flag:%zu, file_flag:%zu", __func__,
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
}

static void set_rt_check_flag_func(int device_id)
{
	if (device_id >= DEV_I2C_START && device_id < DEV_I2C_MAX)
		rt_devcheck_flag = rt_devcheck_flag |
		(1ULL << (unsigned int)device_id);
}

/* static function, the input parameter does not need to be checked */
static void get_dev_detect_dts_config(struct device_node *np)
{
	int result;
	unsigned int i;
	unsigned int type;
	int devices_id;

	for (i = 0; i < ARRAY_SIZE(hw_dec_device_array); i++) {
		if (strcmp(hw_dec_device_array[i].devices_name, "NULL") == 0)
			continue;
		result = of_property_read_u32(np,
			hw_dec_device_array[i].devices_name, &type);
		if (!result) {
			devices_id = hw_dec_device_array[i].devices_id;
			if (type == NO_CHECK) {
				continue;
			} else if (type == BC_CHECK) {
				boardid_devcheck_flag = boardid_devcheck_flag |
					(1ULL << (unsigned int)devices_id);
				set_rt_check_flag_func(devices_id);
			} else if (type == RT_CHECK) {
				set_rt_check_flag_func(devices_id);
			} else {
				 hwlog_warn("type: %d is unsupport!", type);
			}
		}
	}
}

static int dev_dct_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;
	struct class *myclass = NULL;

	if (dev_dct_probe_flag) {
		hwlog_info("dev_dct_probe is done. Can't enter again!\n");
		return 0;
	}

	hwlog_info("Enter device check function!\n");

	if (!pdev) {
		hwlog_err("none device\n");
		return -ENODEV;
	}

	np = pdev->dev.of_node;
	if (!np) {
		hwlog_err("unable to find device node\n");
		return -ENODEV;
	}

	myclass = class_create(THIS_MODULE, "dev_dct");
	if (IS_ERR(myclass)) {
		hwlog_err("failed to create dev_detect class\n");
		return -1;
	}

	if (create_dev_detect_sysfs(myclass) < 0) {
		hwlog_err("create dev detect sysfs failed\n");
		ret = -1;
		goto free_dev_and_file;
	}

	get_dev_detect_dts_config(np);

	dev_dct_probe_flag = 1;
	hwlog_info("device check function end!\n");
	return 0;

free_dev_and_file:
	remove_dev_detect_sysfs(myclass);
	return ret;
}

static const struct of_device_id device_check_match_table[] = {
	{
		.compatible = DTS_COMP_DEVICE_CHECK_NAME,
		.data = NULL,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, device_check_match_table);

static struct platform_driver dev_dct_driver = {
	.driver = {
	.name   = DTS_COMP_DEVICE_CHECK_NAME,
	.of_match_table = of_match_ptr(device_check_match_table),
	},
	.probe  = dev_dct_probe,
	.remove = NULL,
};

static int __init hw_dev_dct_init(void)
{
	return platform_driver_register(&dev_dct_driver);
}

static void __exit hw_dev_dct_exit(void)
{
	platform_driver_unregister(&dev_dct_driver);
}

/* priority is 7s */
late_initcall_sync(hw_dev_dct_init);
module_exit(hw_dev_dct_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Device Detect Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_ALIAS("platform:dev_dct");
