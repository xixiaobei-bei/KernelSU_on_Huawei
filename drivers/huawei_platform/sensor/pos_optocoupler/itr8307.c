/*
 * itr8307.c
 *
 * step optocoupler driver
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/hisi/hisi_adc.h>

#include "huawei_platform/log/hw_log.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG step_opto
HWLOG_REGIST();

#ifdef FAILURE
#undef FAILURE
#endif
#define FAILURE (-1)

#ifdef SUCCESS
#undef SUCCESS
#endif
#define SUCCESS 0

#ifdef INVALID_POS
#undef INVALID_POS
#endif
#define INVALID_POS (-1)

#ifdef GPIO_VAL_HIGH
#undef GPIO_VAL_HIGH
#endif
#define GPIO_VAL_HIGH 1

#ifdef GPIO_VAL_LOW
#undef GPIO_VAL_LOW
#endif
#define GPIO_VAL_LOW 0

#define ITR8307_RETCODE_SUCCESS 0
#define ITR8307_RETCODE_COMMON_ERR (-1)
#define ITR8307_RETCODE_NULL_POINT_ERR (-11)

#define ITR8307_DATA_COUNT 2

struct itr8307_drv_data {
	struct device *pdev;
	struct work_struct sensor_int_work;
	struct mutex sensor_mutex;
	struct mutex val_mutex;
	wait_queue_head_t wq;
	struct hrtimer poll_timer;
	struct workqueue_struct *work_queue;
	struct delayed_work dwork;
	bool data_refresh_flag;
	int32_t sensor_data[ITR8307_DATA_COUNT];
	int gpio_top;
	int gpio_bottom;
	int irq_top;
	int irq_bottom;
};

enum {
	CAMERA_POS_UNKNOWN,
	CAMERA_POS_OPEN,
	CAMERA_POS_CLOSED,
	CAMERA_POS_MIDDLE,
};
static const struct of_device_id g_itr8307_match[] = {
	{.compatible = "huawei,itr8307"},
	{},
};

static void itr8307_sensor_int_work_func(struct work_struct *work)
{
	struct itr8307_drv_data *di = container_of(work, struct itr8307_drv_data,
		sensor_int_work);

	mutex_lock(&di->sensor_mutex);
	di->sensor_data[0] = gpio_get_value(di->gpio_top);
	di->sensor_data[1] = gpio_get_value(di->gpio_bottom);

	hwlog_info("opto_status:top = %d, bottom = %d\n", di->sensor_data[0], di->sensor_data[1]);

	enable_irq(di->irq_top);
	enable_irq(di->irq_bottom);
	mutex_unlock(&di->sensor_mutex);
}

static irqreturn_t itr8307_opto_irq_handle(int irq, void *data)
{
	struct itr8307_drv_data *di = data;
	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return IRQ_NONE;
	}

	disable_irq_nosync(di->irq_top);
	disable_irq_nosync(di->irq_bottom);
	schedule_work(&di->sensor_int_work);

	return IRQ_HANDLED;
}

static int itr8307_gpio_top_init(struct itr8307_drv_data *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return FAILURE;
	}
	np = di->pdev->of_node;

	di->gpio_top = of_get_named_gpio(np, "gpio_top", 0);
	if (!gpio_is_valid(di->gpio_top)) {
		hwlog_err("gpio top invalid\n");
		return FAILURE;
	}
	ret = devm_gpio_request_one(di->pdev, di->gpio_top, GPIOF_IN,
		"itr8307_top_int");
	if (ret != 0) {
		hwlog_err("irq top request fail, ret:%d\n", ret);
		return FAILURE;
	}
	di->irq_top = gpio_to_irq(di->gpio_top);
	ret = devm_request_irq(di->pdev, di->irq_top,
	itr8307_opto_irq_handle, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
		IRQF_NO_SUSPEND, "itr8307_top", di);
	if (ret != 0) {
		hwlog_err("request top irq fail\n");
		return FAILURE;
	}
	disable_irq_nosync(di->irq_top);
	enable_irq(di->irq_top);

	return SUCCESS;
}

static int itr8307_gpio_bottom_init(struct itr8307_drv_data *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return FAILURE;
	}
	np = di->pdev->of_node;

	di->gpio_bottom = of_get_named_gpio(np, "gpio_bottom", 0);
	if (!gpio_is_valid(di->gpio_bottom)) {
		hwlog_err("gpio top invalid\n");
		return FAILURE;
	}

	ret = devm_gpio_request_one(di->pdev, di->gpio_bottom, GPIOF_IN,
		"itr8307_bottom_int");
	if (ret != 0) {
		hwlog_err("irq bottom request fail, ret:%d\n", ret);
		return FAILURE;
	}

	di->irq_bottom = gpio_to_irq(di->gpio_bottom);
	ret = devm_request_irq(di->pdev, di->irq_bottom,
	itr8307_opto_irq_handle, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
		IRQF_NO_SUSPEND, "itr8307_bottom", di);
	if (ret != 0) {
		hwlog_err("request bottom irq fail\n");
		return FAILURE;
	}
	disable_irq_nosync(di->irq_bottom);
	enable_irq(di->irq_bottom);

	return SUCCESS;
}

static u8 itr8307_update_campos(struct itr8307_drv_data *di)
{
	u8 cam_pos = CAMERA_POS_UNKNOWN;
	mutex_lock(&di->sensor_mutex);

	if (di->sensor_data[0] && !di->sensor_data[1]) {
		cam_pos = CAMERA_POS_CLOSED;
	} else if (!di->sensor_data[0] && di->sensor_data[1]) {
		cam_pos = CAMERA_POS_OPEN;
	} else if (di->sensor_data[0] && di->sensor_data[1]) {
		cam_pos = CAMERA_POS_MIDDLE;
	}

	mutex_unlock(&di->sensor_mutex);
	return cam_pos;
}

static ssize_t sensor_data_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct itr8307_drv_data *di = NULL;
	u8 cam_pos;
	int ret;
	int error_flag = 0;

	if (!dev || !attr || !buf) {
		hwlog_err("%s: input NULL!! \n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s: di NULL!! \n", __func__);
		return -EINVAL;
	}

	cam_pos = itr8307_update_campos(di);
	return scnprintf(buf, PAGE_SIZE, "%d, %d\n",
		cam_pos, error_flag);
}

static ssize_t sensor_adc_value_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct itr8307_drv_data *di = NULL;
	int adc_top_value;
	int adc_bottom_value;
	int error_flag = 0;

	if (!dev || !attr || !buf) {
		hwlog_err("%s: input NULL!! \n", __func__);
		return -EINVAL;
	}

	di = dev_get_drvdata(dev);
	if (!di) {
		hwlog_err("%s: di NULL!! \n", __func__);
		return -EINVAL;
	}

	adc_top_value = hisi_adc_get_value(13);
	if (adc_top_value < 0)
		hwlog_err("adc read channel 13 fail\n");

	adc_bottom_value = hisi_adc_get_value(9);
	if (adc_top_value < 0)
		hwlog_err("adc read channel 9 fail\n");

	return scnprintf(buf, PAGE_SIZE, "%d, %d\n",
		adc_top_value, adc_bottom_value);
}

DEVICE_ATTR(sensor_data, 0440, sensor_data_show, NULL);
DEVICE_ATTR(sensor_adc_value, 0440, sensor_adc_value_show, NULL);

static struct attribute *g_itr8307_attributes[] = {
	&dev_attr_sensor_data.attr,
	&dev_attr_sensor_adc_value.attr,
	NULL,
};

static const struct attribute_group g_itr8307_attr_group = {
	.attrs = g_itr8307_attributes,
};

static int itr8307_struct_init(struct itr8307_drv_data *di)
{
	hwlog_info("%s: called\n", __func__);
	mutex_init(&di->sensor_mutex);
	mutex_init(&di->val_mutex);

	di->sensor_data[0] = gpio_get_value(di->gpio_top);
	di->sensor_data[1] = gpio_get_value(di->gpio_bottom);
	di->data_refresh_flag = false;

	init_waitqueue_head(&di->wq);
	INIT_WORK(&di->sensor_int_work, itr8307_sensor_int_work_func);

	return ITR8307_RETCODE_SUCCESS;
}

static int itr8307_probe(struct platform_device *pdev)
{
	struct itr8307_drv_data *di = NULL;

	if (!pdev) {
		hwlog_err("%s pdev is null\n", __func__);
		return -EFAULT;
	}
	if (!of_match_node(g_itr8307_match, pdev->dev.of_node)) {
		hwlog_err("dev node is not match exiting\n");
		return -ENODEV;
	}

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->pdev = &pdev->dev;
	if (itr8307_gpio_top_init(di) != SUCCESS)
		goto error_end;
	if (itr8307_gpio_bottom_init(di) != SUCCESS)
		goto error_end;

	itr8307_struct_init(di);
	platform_set_drvdata(pdev, di);

	if (sysfs_create_group(&di->pdev->kobj, &g_itr8307_attr_group) != 0) {
		hwlog_err("sysfs create group fail\n");
		goto error_end;
	}

	hwlog_info("%s init ok\n", __func__);
	return 0;

error_end:
	mutex_destroy(&di->val_mutex);
	mutex_destroy(&di->sensor_mutex);
	devm_kfree(di->pdev, di);
	hwlog_err("%s gpio init fail\n", __func__);
	return -EFAULT;
}

static int itr8307_remove(struct platform_device *pdev)
{
	struct itr8307_drv_data *di =
		(struct itr8307_drv_data *)platform_get_drvdata(pdev);
	if (!di) {
		hwlog_err("%s di is NULL\n", __func__);
		return 0;
	}

	mutex_destroy(&di->val_mutex);
	mutex_destroy(&di->sensor_mutex);
	devm_kfree(di->pdev, di);
	sysfs_remove_group(&di->pdev->kobj, &g_itr8307_attr_group);
	return 0;
}

static struct platform_driver g_itr8307_driver = {
	.probe = itr8307_probe,
	.remove = itr8307_remove,
	.driver = {
		.name   = "itr8307",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(g_itr8307_match),
	},
};

static int __init itr8307_init(void)
{
	hwlog_info("%s: called\n", __func__);
	return platform_driver_register(&g_itr8307_driver);
}

static void __exit itr8307_exit(void)
{
	hwlog_info("%s: called\n", __func__);
	platform_driver_unregister(&g_itr8307_driver);
}

module_init(itr8307_init);
module_exit(itr8307_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ITR8307 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
