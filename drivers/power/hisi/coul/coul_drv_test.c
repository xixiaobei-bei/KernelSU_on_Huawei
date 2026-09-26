/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: this file provide sys  interface to set handle  battery state such as
 *         capacity, voltage, current, temperature
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <securec.h>
#include "coul_core.h"
#include "coul_drv_test.h"

#define coul_test_debug(fmt, args...) pr_debug("[coul_test]" fmt, ## args)
#define coul_test_info(fmt, args...) pr_info("[coul_test]" fmt, ## args)
#define coul_test_warn(fmt, args...) pr_warn("[coul_test]" fmt, ## args)
#define coul_test_err(fmt, args...) pr_err("[coul_test]" fmt, ## args)

struct coul_drv_test_info *g_coul_drv_test_info;
struct coul_drv_test_info *get_coul_drv_test_info(void)
{
	return g_coul_drv_test_info;
}

static ssize_t coul_set_test_start_flag(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, HEX, &val) < 0) || (val < 0)) /* size */
		return -EINVAL;
#ifdef CONFIG_HISI_DEBUG_FS
	di->test_start_flag = (unsigned int)val;
	coul_test_info("set %s is %d\n", __func__, di->test_start_flag);
#else
	coul_test_info("set %s is %d for user\n", __func__, di->test_start_flag);
#endif
	return status;
}

static ssize_t coul_show_test_start_flag(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->test_start_flag;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val);
}

static ssize_t coul_set_input_batt_exist(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) || (val < 0) || (val > 1)) /* size */
		return -EINVAL;
	di->input_batt_exist = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_exist);
	return status;
}

static ssize_t coul_show_input_batt_exist(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_exist;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val); /* size */
}

static ssize_t coul_set_input_batt_capacity(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) || (val < 0) || (val > CAPACITY_MAX)) /* capacity */
		return -EINVAL;
	di->input_batt_capacity = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_capacity);
	return status;
}

static ssize_t coul_show_input_batt_capacity(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_capacity;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val); /* size */
}

static ssize_t coul_set_input_batt_temp(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) ||
		(val < TEMP_MIN) || (val > TEMP_MAX)) /* temp */
		return -EINVAL;
	di->input_batt_temp = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_temp);
	return status;
}

static ssize_t coul_show_input_batt_temp(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_temp;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val); /* size */
}

static ssize_t coul_set_input_batt_full(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) || (val < 0) || (val > 1)) /* size */
		return -EINVAL;
	di->input_batt_full = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_full);
	return status;
}

static ssize_t coul_show_input_batt_full(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_full;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val); /* size */
}

static ssize_t coul_set_input_batt_volt(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) ||
		(val < VOLTAGE_MIN) || (val > VOLTAGE_MAX)) /* mv */
		return -EINVAL;
	di->input_batt_volt = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_volt);
	return status;
}

static ssize_t coul_show_input_batt_volt(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_volt;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%ld\n", val); /* size */
}

static ssize_t coul_set_input_batt_cur(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) ||
		(val < CURRENT_MIN) || (val > CURRENT_MAX)) /* ma */
		return -EINVAL;
	di->input_batt_cur = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_cur);
	return status;
}

static ssize_t coul_show_input_batt_cur(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_cur;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%ld\n", val); /* size */
}

static ssize_t coul_set_input_batt_fcc(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, DEC, &val) < 0) ||
		(val < 0) || (val > BATT_FCC_MAX)) /* mah */
		return -EINVAL;
	di->input_batt_fcc = val;
	coul_test_info("set %s is %d\n", __func__, di->input_batt_fcc);
	return status;
}

static ssize_t coul_show_input_batt_fcc(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_batt_fcc;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val); /* size */
}

static ssize_t coul_set_input_event(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val;
	int status = count;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	if ((kstrtol(buf, HEX, &val) < 0) ||
		(val < 0) || (val > INVALID_INPUT_EVENT)) /* size */
		return -EINVAL;
	di->input_event = val;
	coul_test_info("set %s is %d\n", __func__, di->input_event);
	return status;
}

static ssize_t coul_show_input_event(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long val;
	struct coul_drv_test_info *di = dev_get_drvdata(dev);

	val = di->input_event;
	return snprintf_s(buf, PAGE_SIZE, SRC_LEN, "%lu\n", val); /* buf size */
}

static DEVICE_ATTR(test_start_flag, S_IWUSR | S_IRUGO,
	coul_show_test_start_flag, coul_set_test_start_flag);
static DEVICE_ATTR(input_batt_exist, S_IWUSR | S_IRUGO,
	coul_show_input_batt_exist, coul_set_input_batt_exist);
static DEVICE_ATTR(input_batt_capacity, S_IWUSR | S_IRUGO,
	coul_show_input_batt_capacity, coul_set_input_batt_capacity);
static DEVICE_ATTR(input_batt_temp, S_IWUSR | S_IRUGO,
	coul_show_input_batt_temp, coul_set_input_batt_temp);
static DEVICE_ATTR(input_batt_full, S_IWUSR | S_IRUGO,
	coul_show_input_batt_full, coul_set_input_batt_full);
static DEVICE_ATTR(input_batt_volt, S_IWUSR | S_IRUGO,
	coul_show_input_batt_volt, coul_set_input_batt_volt);
static DEVICE_ATTR(input_batt_cur, S_IWUSR | S_IRUGO,
	coul_show_input_batt_cur, coul_set_input_batt_cur);
static DEVICE_ATTR(input_batt_fcc, S_IWUSR | S_IRUGO,
	coul_show_input_batt_fcc, coul_set_input_batt_fcc);
static DEVICE_ATTR(input_event, S_IWUSR | S_IRUGO,
	coul_show_input_event, coul_set_input_event);

static struct attribute *coul_attributes[] = {
	&dev_attr_test_start_flag.attr,
	&dev_attr_input_batt_exist.attr,
	&dev_attr_input_batt_capacity.attr,
	&dev_attr_input_batt_temp.attr,
	&dev_attr_input_batt_full.attr,
	&dev_attr_input_batt_volt.attr,
	&dev_attr_input_batt_cur.attr,
	&dev_attr_input_batt_fcc.attr,
	&dev_attr_input_event.attr,
	NULL,
};

static const struct attribute_group coul_attr_group = {
	.attrs = coul_attributes,
};

static int coul_drv_test_probe(struct platform_device *pdev)
{
	struct coul_drv_test_info *di = NULL;
	struct class *coul_class = NULL;
	struct device *new_dev = NULL;
	int ret;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	g_coul_drv_test_info = di;
	dev_set_drvdata(&pdev->dev, di);
	di->test_start_flag = DEFAULT_TEST_START_FLAG;
	di->input_batt_capacity = DEFAULT_INPUT_BATT_CAPACITY; /* cap */
	di->input_batt_cur = DEFAULT_INPUT_BATT_CURRENT; /* mv */
	di->input_batt_exist = DEFAULT_INPUT_BATT_EXIST;
	di->input_batt_fcc = DEFAULT_INPUT_BATT_FCC; /* mAh */
	di->input_batt_full = DEFAULT_INPUT_BATT_FULL;
	di->input_batt_temp = DEFAULT_INPUT_BATT_TEMP; /* temp */
	di->input_batt_volt = DEFAULT_INPUT_BATT_VOLTAGE; /* mv */

	ret = sysfs_create_group(&pdev->dev.kobj, &coul_attr_group);
	if (ret) {
		coul_test_info("could not create sysfs files\n");
		goto err_sysfs;
	}

	coul_class = class_create(THIS_MODULE, "coul_drv_test");
	if (IS_ERR(coul_class)) {
		coul_test_info("could not create sysfs class\n");
		goto err_sysfs1;
	}

	new_dev = device_create(coul_class, NULL, 0, NULL, "coul");
	if (IS_ERR(new_dev)) {
		coul_test_info("could not create sysfs device\n");
		goto err_sysfs2;
	}

	ret = sysfs_create_link(&new_dev->kobj,
		&pdev->dev.kobj, "coul_test_info");
	if (ret < 0)
		coul_test_info("create link to charge data fail\n");
	coul_test_info("%s ok!", __func__);
	return 0;
err_sysfs2:
	class_destroy(coul_class);
err_sysfs1:
	sysfs_remove_group(&pdev->dev.kobj, &coul_attr_group);
err_sysfs:
	kfree(di);
	return -1;
}

static int coul_drv_test_remove(struct platform_device *pdev)
{
	struct coul_drv_test_info *di = dev_get_drvdata(&pdev->dev);

	sysfs_remove_group(&pdev->dev.kobj, &coul_attr_group);
	if (!di)
		return -ENOMEM;

	kfree(di);
	return 0;
}

static void coul_drv_test_shutdown(struct platform_device *pdev)
{
	struct coul_drv_test_info *di = dev_get_drvdata(&pdev->dev);

	if (!di) {
		coul_test_err("di is NULL!\n");
		return;
	}
	kfree(di);
	g_coul_drv_test_info = NULL;
}

static const struct of_device_id coul_drv_test_match_table[] = {
	{
		.compatible = "hisilicon,coul_drv_test",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver coul_drv_test_driver = {
	.probe = coul_drv_test_probe,
	.remove = coul_drv_test_remove,
	.shutdown = coul_drv_test_shutdown,
	.driver = {
		.name = "coul_drv_test",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(coul_drv_test_match_table),
	},
};

int __init coul_test_init(void)
{
	coul_test_info("%s\n", __func__);
	return platform_driver_register(&coul_drv_test_driver);
}

module_init(coul_test_init);

void __exit coul_test_exit(void)
{
	platform_driver_unregister(&coul_drv_test_driver);
}

module_exit(coul_test_exit);

MODULE_DESCRIPTION("coul test module driver");
MODULE_LICENSE("GPL");

