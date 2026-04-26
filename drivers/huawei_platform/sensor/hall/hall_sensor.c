/*
 * hall_sensor.c
 *
 * hall_sensor driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include "hall_sensor.h"

#include <linux/version.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG hall
HWLOG_REGIST();

static struct class *hall_class;

static ssize_t hall_info_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	char info[MAX_INFO_LEN] = {0};
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		(void)cdev->hall_get_info(cdev, info, MAX_INFO_LEN);
		hwlog_info("%s: hall info is %s\n", __func__, info);
	}

	return snprintf(buf, PAGE_SIZE, "%s\n", info);
}

static ssize_t hall_count_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	int count = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		count = cdev->hall_count;
		hwlog_info("%s: count = %d\n", __func__, count);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", count);
}

static ssize_t hall_report_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t size)
{
	unsigned long state = 0L;
	struct hall_cdev *cdev = dev_get_drvdata(dev);
	packet_data pdata;

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return (ssize_t)size;
	}

	if (!kstrtol(buf, 10, &state)) {
		pdata = (packet_data)state;
		cdev->hall_report_debug_data(cdev, pdata);
	}

	return (ssize_t)size;
}

static ssize_t hall_now_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	packet_data value = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		value = cdev->hall_get_state(cdev);
		hwlog_info("%s: value = %u\n", __func__, value);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", value);
}

static ssize_t hall_now_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t size)
{
	unsigned long state = 0L;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return size;
	}

	if (kstrtol(buf, 10, &state) != 0) {
		hwlog_err("%s: kstrtol fail\n", __func__);
		return size;
	}

	(void)cdev->hall_report_state(cdev);
	return (ssize_t)size;
}

#ifdef HALL_TEST
static ssize_t hall_value_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	packet_data value = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		value = cdev->value;
		hwlog_info("%s: value = %u\n", __func__, value);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", value);
}

static ssize_t hall_value_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t size)
{
	unsigned long state = 0L;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return size;
	}

	if (!kstrtol(buf, 10, &state)) {
		cdev->value = (packet_data)state;
		cdev->hall_set_report_value(cdev);
	}

	return (ssize_t)size;
}

static ssize_t hall_report_count_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	int report_count = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		report_count = cdev->count;
		hwlog_info("%s: report_count = %d\n", __func__, report_count);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", report_count);
}

static ssize_t hall_report_count_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf,
				       size_t size)
{
	unsigned long state = 0L;
	int count;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return (ssize_t)size;
	}

	if (kstrtol(buf, 10, &state) != 0) {
		hwlog_err("%s: kstrtol fail\n", __func__);
		return (ssize_t)size;
	}

	count = (int)state;
	if (count < HALL_REPORT_MIN_COUNT)
		cdev->count = HALL_REPORT_MIN_COUNT;
	else if (count > HALL_REPORT_MAX_COUNT)
		cdev->count = HALL_REPORT_MAX_COUNT;
	else
		cdev->count = count;

	return (ssize_t)size;
}

static ssize_t hall_flag_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	int flag = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		flag = cdev->flag;
		hwlog_info("%s: flag = %d\n", __func__, flag);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", flag);
}

static ssize_t hall_flag_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t size)
{
	unsigned long state = 0L;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return (ssize_t)size;
	}

	if (kstrtol(buf, 10, &state) != 0) {
		hwlog_err("%s: kstrtol fail\n", __func__);
		return (ssize_t)size;
	}

	cdev->flag = (int)state;
	return (ssize_t)size;
}

static ssize_t hall_period_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int period = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		period = cdev->period;
		hwlog_info("%s: period = %d\n", __func__, period);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", period);
}

static ssize_t hall_period_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t size)
{
	unsigned long state = 0L;
	int period;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return (ssize_t)size;
	}

	if (kstrtol(buf, 10, &state) != 0) {
		hwlog_err("%s: kstrtol fail\n", __func__);
		return (ssize_t)size;
	}

	period = (int)state;
	if (period < HALL_REPORT_MIN_PERIOD)
		cdev->period = HALL_REPORT_MIN_PERIOD;
	else if (period > HALL_REPORT_MAX_PERIOD)
		cdev->period = HALL_REPORT_MAX_PERIOD;
	else
		cdev->period = period;

	return (ssize_t)size;
}

static ssize_t hall_enable_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int enable = 0;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev != NULL) {
		enable = cdev->enable;
		hwlog_info("%s: enable = %d\n", __func__, enable);
	}

	return snprintf(buf, MAX_BUF_SIZE, "%d\n", enable);
}

static ssize_t hall_enable_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t size)
{
	unsigned long state = 0L;
	int enable;
	struct hall_cdev *cdev = dev_get_drvdata(dev);

	if (cdev == NULL || buf == NULL) {
		hwlog_err("%s: null point\n", __func__);
		return (ssize_t)size;
	}

	if (kstrtol(buf, 10, &state) != 0) {
		hwlog_err("%s: kstrtol fail\n", __func__);
		return (ssize_t)size;
	}

	enable = !!state;
	cdev->enable = enable;
	return (ssize_t)size;
}
#endif

#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
DEVICE_ATTR(hall_info, 0440, hall_info_show, NULL);
DEVICE_ATTR(count, 0440, hall_count_show, NULL);
DEVICE_ATTR(report, 0220, NULL, hall_report_store);
DEVICE_ATTR(state, 0660, hall_now_show, hall_now_store);
#ifdef HALL_TEST
DEVICE_ATTR(value, 0660, hall_value_show, hall_value_store);
DEVICE_ATTR(report_count, 0660, hall_report_count_show,
	hall_report_count_store);
DEVICE_ATTR(flag, 0660, hall_flag_show, hall_flag_store);
DEVICE_ATTR(period, 0660, hall_period_show, hall_period_store);
DEVICE_ATTR(hall_enable, 0660, hall_enable_show, hall_enable_store);
#endif

static struct attribute *hall_attributes[] = {
	&dev_attr_hall_info.attr,
	&dev_attr_count.attr,
	&dev_attr_report.attr,
	&dev_attr_state.attr,
	&dev_attr_value.attr,
	&dev_attr_report_count.attr,
	&dev_attr_flag.attr,
	&dev_attr_period.attr,
	&dev_attr_hall_enable.attr,
	NULL,
};
static const struct attribute_group hall_attr_group = {
	.attrs = hall_attributes,
};

static const struct attribute_group *hall_attr_groups[] = {
	&hall_attr_group,
	NULL,
};
#else
static struct device_attribute hall_class_attrs[] = {
	__ATTR(info, 0440, hall_info_show, NULL),
	__ATTR(count, 0440, hall_count_show, NULL),
	__ATTR(report, 0220, NULL, hall_report_store),
	__ATTR(state, 0660, hall_now_show, hall_now_store),
#ifdef HALL_TEST
	__ATTR(value, 0660, hall_value_show, hall_value_store),
	__ATTR(report_count, 0660, hall_report_count_show,
		hall_report_count_store),
	__ATTR(flag, 0660, hall_flag_show, hall_flag_store),
	__ATTR(period, 0660, hall_period_show, hall_period_store),
	__ATTR(enable, 0660, hall_enable_show, hall_enable_store),
#endif
	__ATTR_NULL,
};
#endif

int hall_register(struct hall_cdev *cdev)
{
	cdev->dev = device_create(hall_class, NULL, 0, cdev, "hall");
	if (cdev->dev == NULL) {
		hwlog_err("[%s] Failed to create hall dev", __func__);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(hall_register);

void hall_unregister(struct hall_cdev *cdev)
{
	device_destroy(hall_class, 0);
}
EXPORT_SYMBOL_GPL(hall_unregister);

static int hall_init(void)
{
	hall_class = class_create(THIS_MODULE, "hall");
	if (IS_ERR(hall_class))
		return PTR_ERR(hall_class);
#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
	hall_class->dev_groups = hall_attr_groups;
#else
	hall_class->dev_attrs = hall_class_attrs;
#endif
	hwlog_info("[%s]hall init\n", __func__);
	return 0;
}

static void hall_exit(void)
{
	class_destroy(hall_class);
}

subsys_initcall(hall_init);
module_exit(hall_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hall driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
