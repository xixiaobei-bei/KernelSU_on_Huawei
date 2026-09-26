/*
 * led_alwayson.c
 *
 * led_alwayson driver
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
#include "led_alwayson.h"

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/platform_device.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG  led_alwayson
HWLOG_REGIST();

unsigned int runmode_is_factory(void);
static char led_alwayson_nv[LED_STATUS_NV_SIZE + 1] = {0};

static int read_led_status_from_nv(void)
{
	int ret;
	struct hisi_nve_info_user user_info;
	const char *name = "LEDST";
	const unsigned int name_len = sizeof(user_info.nv_name);
	int led_alwayson_flag = 0;

	if (runmode_is_factory() == 0)
		return led_alwayson_flag;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ;
	user_info.nv_number = LED_STATUS_NV_NUM;
	user_info.valid_size = LED_STATUS_NV_SIZE;
	strncpy(user_info.nv_name, name, name_len);
	user_info.nv_name[name_len - 1] = '\0';

	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("%s:nve_direct_access read error %d\n",
			__func__, ret);
		return led_alwayson_flag;
	}

	if (sizeof(user_info.nv_data) >= sizeof(led_alwayson_nv))
		memcpy(led_alwayson_nv, (char *)user_info.nv_data,
			sizeof(led_alwayson_nv));

	hwlog_info("%s:read led color:%d, status:%d\n",
		__func__, led_alwayson_nv[0], led_alwayson_nv[1]);

	if (led_alwayson_nv[1] == 1) {
		led_alwayson_flag = 1; // 1 is led always open
		hwlog_info("%s:led alwayson is open\n", __func__);
	} else {
		led_alwayson_flag = 0; // 0 is led always closed
		hwlog_info("%s:led alwayson is close\n", __func__);
	}

	return led_alwayson_flag;
}

static int write_led_status_to_nv(void)
{
	int ret;
	struct hisi_nve_info_user user_info;
	const char *name = "LEDST";
	const unsigned int name_len = sizeof(user_info.nv_name);

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE;
	user_info.nv_number = LED_STATUS_NV_NUM;
	user_info.valid_size = LED_STATUS_NV_SIZE;
	strncpy(user_info.nv_name, name, name_len);
	user_info.nv_name[name_len - 1] = '\0';
	user_info.nv_data[0] = led_alwayson_nv[0];
	user_info.nv_data[1] = led_alwayson_nv[1];
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("%s:nve_direct_access write error %d\n",
			__func__, ret);
		return LED_COMMON_ERR;
	}
	hwlog_info("%s succ, nv data=%d, %d\n", __func__,
		user_info.nv_data[0], user_info.nv_data[1]);

	return ret;
}


static ssize_t led_status_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	if (buf == NULL) {
		hwlog_err("%s:invalid data\n", __func__);
		return LED_COMMON_ERR;
	}

	return snprintf(buf, PAGE_SIZE, "%d\n", read_led_status_from_nv());
}

static ssize_t led_status_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	int ret;
	uint64_t led_status = 0;

	if (buf == NULL) {
		hwlog_err("%s:invalid data\n", __func__);
		return LED_COMMON_ERR;
	}

	if (kstrtoull(buf, 10, &led_status)) {
		hwlog_err("%s:read led color fail\n", __func__);
		return (ssize_t)size;
	}
	hwlog_info("%s:led status:%llu\n", __func__, led_status);

	if (runmode_is_factory() == 0) {
		hwlog_info("%s:is not factory\n", __func__);
		return (ssize_t)size;
	}

	ret = read_led_status_from_nv();
	if (ret != 0) {
		hwlog_err("%s:read led_status fail\n", __func__);
		return (ssize_t)size;
	}

	/* update led status */
	led_alwayson_nv[1] = (char)led_status;

	ret = write_led_status_to_nv();
	if (ret != 0) {
		hwlog_err("%s:write led status fail\n", __func__);
		return (ssize_t)size;
	}

	return (ssize_t)size;
}

static ssize_t version_mode_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	if (buf == NULL) {
		hwlog_err("%s:invalid data\n", __func__);
		return LED_COMMON_ERR;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", runmode_is_factory());
}

/* files create */
static DEVICE_ATTR(led_status, 0660, led_status_show,
		   led_status_store);
static DEVICE_ATTR(version_mode, 0660, version_mode_show,
		   NULL);

static struct attribute *led_alwayson_attrs[] = {
	&dev_attr_led_status.attr,
	&dev_attr_version_mode.attr,
	NULL
};

static const struct attribute_group led_alwayson_attrs_group = {
	.attrs = led_alwayson_attrs,
};

static const struct of_device_id led_alwayson_match_table[] = {
	{ .compatible = "huawei,led_alwayson", },
	{},
};
MODULE_DEVICE_TABLE(of, led_alwayson_match_table);

static int led_alwayson_probe(struct platform_device *pdev)
{
	int ret;
	const char *state = NULL;
	struct led_alwayson_data *data = NULL;
	struct device_node *led_node = NULL;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;

	data->pdev = pdev;
	led_node = pdev->dev.of_node;
	if (led_node == NULL) {
		hwlog_err("%s failed to find dts node led_alwayson\n",
			__func__);
		ret = LED_COMMON_ERR;
		goto free_devm;
	}

	ret = of_property_read_string(led_node, LED_ALWAYSON_EXIST, &state);
	if (ret != 0) {
		hwlog_err("%s failed to find LED_ALWAYSON_EXIST\n", __func__);
		ret = LED_COMMON_ERR;
		goto free_devm;
	}
	if (strncmp(state, "yes", sizeof("yes")) != 0) {
		hwlog_err("%s:led alwayson is not support\n", __func__);
		ret = LED_COMMON_SUC;
		goto free_devm;
	}

	data->led_alwayson_class = class_create(THIS_MODULE, "led_alwayson");
	if (IS_ERR(data->led_alwayson_class)) {
		ret = PTR_ERR(data->led_alwayson_class);
		goto free_devm;
	}

	data->dev = device_create(data->led_alwayson_class, NULL,
				  0, NULL, "led_alwayson");
	if (data->dev == NULL) {
		hwlog_err("[%s] device_create Failed", __func__);
		ret = LED_COMMON_ERR;
		goto free_class;
	}

	ret = sysfs_create_group(&data->dev->kobj, &led_alwayson_attrs_group);
	if (ret != 0) {
		hwlog_err("%s sysfs_create_group failed ret=%d\n",
			__func__, ret);
		ret = LED_COMMON_ERR;
		goto free_device;
	}
	hwlog_info("%s succ\n", __func__);
	return LED_COMMON_SUC;

free_device:
	device_destroy(data->led_alwayson_class, 0);
free_class:
	class_destroy(data->led_alwayson_class);
free_devm:
	devm_kfree(&pdev->dev, data);
	return ret;
}

static int led_alwayson_remove(struct platform_device *pdev)
{
	struct led_alwayson_data *data = NULL;

	if (pdev == NULL)
		return -EINVAL;
	data = dev_get_drvdata(&pdev->dev);
	if (data == NULL)
		return -EINVAL;

	sysfs_remove_group(&data->dev->kobj, &led_alwayson_attrs_group);
	device_destroy(data->led_alwayson_class, 0);
	class_destroy(data->led_alwayson_class);
	devm_kfree(&pdev->dev, data);
	hwlog_info("%s\n", __func__);
	return 0;
}

struct platform_driver led_alwayson_driver = {
	.probe = led_alwayson_probe,
	.remove = led_alwayson_remove,
	.driver = {
		.name = LED_ALWAYSON_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(led_alwayson_match_table),
	},
};

static int __init led_alwayson_init(void)
{
	hwlog_info("init\n");
	return platform_driver_register(&led_alwayson_driver);
}

static void __exit led_alwayson_exit(void)
{
	hwlog_info("exit\n");
	platform_driver_unregister(&led_alwayson_driver);
}

module_init(led_alwayson_init);
module_exit(led_alwayson_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Led alwayson driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
