/*
 * power_bigdata.c
 *
 * bigdata for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_bigdata.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_bigdata
HWLOG_REGIST();

static struct device *g_power_bigdata_dev;

static struct power_bigdata_info g_power_bigdata_info[] = {
	{
		"bigdata_coul",
		POWER_BIGDATA_TYPE_COUL,
		POWER_BIGDATA_EVENT_ID_COUL,
	},
};

static int power_bigdata_check_type(unsigned int type)
{
	if ((type >= POWER_BIGDATA_TYPE_BEGIN) && (type < POWER_BIGDATA_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", type);
	return -1;
}

static int power_bigdata_get_index(unsigned int type)
{
	int i;
	struct power_bigdata_info *info = g_power_bigdata_info;
	int size = ARRAY_SIZE(g_power_bigdata_info);

	for (i = 0; i < size; i++) {
		if (type == info[i].type) {
			hwlog_info("find[%d]: name=%s type=%d event_id=%d\n",
				i,
				info[i].name, info[i].type, info[i].event_id);
			return i;
		}
	}

	hwlog_err("type not find, type=%d\n", type);
	return -1;
}

static unsigned int power_bigdata_get_event_id(unsigned int index)
{
	return g_power_bigdata_info[index].event_id;
}

static void power_bigdata_print_data(unsigned int index)
{
	hwlog_info("bigdata %s,%d,%d send ok\n",
		g_power_bigdata_info[index].name,
		g_power_bigdata_info[index].type,
		g_power_bigdata_info[index].event_id);
}

static struct imonitor_eventobj *power_bigdata_create_object(unsigned int id)
{
	struct imonitor_eventobj *obj = NULL;

	obj = imonitor_create_eventobj(id);
	if (!obj) {
		hwlog_err("imonitor create fail\n");
		return NULL;
	}

	return obj;
}

static int power_bigdata_send_event(struct imonitor_eventobj *obj)
{
	if (!obj) {
		hwlog_err("obj is null\n");
		return -1;
	}

	if (imonitor_send_event(obj) < 0) {
		hwlog_err("imonitor send fail\n");
		return -1;
	}

	return 0;
}

static void power_bigdata_destroy_object(struct imonitor_eventobj *obj)
{
	if (!obj) {
		hwlog_err("obj is null\n");
		return;
	}

	imonitor_destroy_eventobj(obj);
}

int power_bigdata_send_integer(struct imonitor_eventobj *obj,
	const char *para, long value)
{
	if (!obj || !para) {
		hwlog_err("obj or para is null\n");
		return -1;
	}

	return imonitor_set_param_integer_v2(obj, para, value);
}

int power_bigdata_send_string(struct imonitor_eventobj *obj,
	const char *para, const char *value)
{
	if (!obj || !para || !value) {
		hwlog_err("obj or para or value is null\n");
		return -1;
	}

	return imonitor_set_param_string_v2(obj, para, value);
}

int power_bigdata_report(enum power_bigdata_type type,
	power_bigdata_send_cb send_cb, void *data)
{
	int index;
	unsigned int event_id;
	struct imonitor_eventobj *obj = NULL;
	int ret = 0;

	if (!send_cb || !data) {
		hwlog_err("send_cb or data is null\n");
		return -1;
	}

	if (power_bigdata_check_type(type))
		return -1;

	index = power_bigdata_get_index(type);
	if (index == -1)
		return -1;

	event_id = power_bigdata_get_event_id(index);

	/* step-1: create event object */
	obj = power_bigdata_create_object(event_id);
	if (!obj)
		return -1;

	/* step-2: send data */
	if (send_cb(obj, data)) {
		ret = -1;
		goto end_send;
	}

	/* step-3: send event */
	if (power_bigdata_send_event(obj)) {
		ret = -1;
		goto end_send;
	}

	power_bigdata_print_data(index);

end_send:
	/* step-4: destroy event object */
	power_bigdata_destroy_object(obj);
	return ret;
}

#if (defined(CONFIG_HUAWEI_POWER_DEBUG) && defined(CONFIG_SYSFS))
static int power_bigdata_send_test_data(struct imonitor_eventobj *obj,
	void *data)
{
	struct power_bigdata_test_data *test_data = NULL;

	if (!obj || !data) {
		hwlog_err("obj or data is null\n");
		return -1;
	}

	test_data = (struct power_bigdata_test_data *)data;
	(void)power_bigdata_send_integer(obj, "test_int", test_data->integer);
	(void)power_bigdata_send_string(obj, "test_str", test_data->string);
	return 0;
}

static ssize_t power_bigdata_set_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int type;
	unsigned int data;
	struct power_bigdata_test_data test_data;

	if (count >= POWER_BIGDATA_RD_BUF_SIZE) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: numbers for type and data */
	if (sscanf(buf, "%d %d", &type, &data) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	test_data.integer = data;
	test_data.string = (char *)buf;
	power_bigdata_report(type, power_bigdata_send_test_data, &test_data);
	return count;
}

static ssize_t power_bigdata_show_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	struct power_bigdata_info *info = g_power_bigdata_info;
	int size = ARRAY_SIZE(g_power_bigdata_info);
	char rd_buf[POWER_BIGDATA_RD_BUF_SIZE] = { 0 };

	for (i = 0; i < size; i++) {
		memset(rd_buf, 0, POWER_BIGDATA_RD_BUF_SIZE);
		scnprintf(rd_buf, POWER_BIGDATA_RD_BUF_SIZE,
			"[%d] name=%s type=%d event_id=%d\n",
			i, info[i].name, info[i].type, info[i].event_id);
		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static DEVICE_ATTR(info, 0640, power_bigdata_show_info, power_bigdata_set_info);

static struct attribute *power_bigdata_attributes[] = {
	&dev_attr_info.attr,
	NULL,
};

static const struct attribute_group power_bigdata_attr_group = {
	.attrs = power_bigdata_attributes,
};

static struct device *power_bigdata_create_group(void)
{
	return power_sysfs_create_group("hw_power", "power_bigdata",
		&power_bigdata_attr_group);
}

static void power_bigdata_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_bigdata_attr_group);
}
#else
static inline struct device *power_bigdata_create_group(void)
{
	return NULL;
}

static inline void power_bigdata_remove_group(struct device *dev)
{
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG && CONFIG_SYSFS */

static int __init power_bigdata_init(void)
{
	g_power_bigdata_dev = power_bigdata_create_group();
	return 0;
}

static void __exit power_bigdata_exit(void)
{
	power_bigdata_remove_group(g_power_bigdata_dev);
	g_power_bigdata_dev = NULL;
}

subsys_initcall_sync(power_bigdata_init);
module_exit(power_bigdata_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bigdata for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
