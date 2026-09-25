/*
 * classd_vibrator.c
 *
 * classd_vibrator driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/of_device.h>
#include <linux/hisi/hisi_vibrator.h>
#include <linux/leds.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/regulator/consumer.h>

#include <securec.h>
#include "huawei_platform/log/hw_log.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG hw_classd_vibrator
HWLOG_REGIST();

#define CLASSD_VIBRATOR "classd_vibrator"

#define ON                                 1
#define OFF                                0
#define SEC_MS_RATIO                       1000
#define MS_NSEC_RATIO                      1000000
#define DEFAULT_STOP_DELAY_MS              20
#define DEFAULT_MIN_VIB_TIMEOUT            10

struct classd_vibrator_data {
	struct device *dev;
	struct led_classdev cdev;

	struct work_struct enable_work;
	struct mutex lock;
	struct mutex ldo_lock;
	struct hrtimer stop_timer;

	uint32_t classd_reg_address;
	uint32_t start_vib_value;
	uint32_t reverse_vib_value;
	uint32_t stop_vib_value;
	uint32_t stop_delay_ms;

	uint8_t support_quick_stop;
	uint8_t ldo_status;

	int state;

	struct regulator *ldo;
	int vout_voltage;
	uint8_t vibrator_working;
};

#ifdef CONFIG_HISI_VIBRATOR
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif
static uint32_t min_vib_time_ms;
static uint32_t stop_delay_time_ms;

static void set_ldo_power(struct classd_vibrator_data *priv, uint8_t state)
{
	int ret;

	if (priv->ldo == NULL) {
		hwlog_debug("%s: ldo null ,not suppot", __func__);
		return;
	}

	mutex_lock(&priv->ldo_lock);
	if (state == priv->ldo_status) {
		mutex_unlock(&priv->ldo_lock);
		return;
	}

	if (state == ON)
		ret = regulator_enable(priv->ldo);
	else
		ret = regulator_disable(priv->ldo);
	if (ret < 0)
		hwlog_err("%s, fail!! state = %u", state);
	else
		priv->ldo_status = state;

	mutex_unlock(&priv->ldo_lock);
}

static void vibrator_on(struct classd_vibrator_data *priv)
{
	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	hwlog_info("%s: enter!!", __func__);
	set_ldo_power(priv, ON);
	hisi_pmic_reg_write(priv->classd_reg_address, priv->start_vib_value);
	vibrator_shake = ON;
	priv->vibrator_working = ON;
}

static void vibrator_power_off(struct classd_vibrator_data *priv)
{
	hisi_pmic_reg_write(priv->classd_reg_address, priv->stop_vib_value);
	set_ldo_power(priv, OFF);
	vibrator_shake = OFF;
}

static void vibrator_off(struct classd_vibrator_data *priv)
{
	if (priv->vibrator_working == OFF) {
		hwlog_info("%s: already off!!", __func__);
		return 0;
	}

	if (priv->support_quick_stop == 0) {
		priv->vibrator_working = OFF;
		vibrator_power_off(priv);
		hwlog_info("%s: stop right now!!", __func__);
		return;
	}

	hwlog_info("%s: enter!!", __func__);
	hisi_pmic_reg_write(priv->classd_reg_address, priv->reverse_vib_value);
	hrtimer_start(&priv->stop_timer,
		ktime_set(stop_delay_time_ms / SEC_MS_RATIO,
			(stop_delay_time_ms % SEC_MS_RATIO) * MS_NSEC_RATIO),
		HRTIMER_MODE_REL);
}


static enum hrtimer_restart classd_vibrator_stop_timer_func
	(struct hrtimer *timer)
{
	struct classd_vibrator_data *priv = NULL;

	priv = container_of(timer, struct classd_vibrator_data, stop_timer);
	if (priv == NULL) {
		hwlog_err("%s: invalid data", __func__);
		return HRTIMER_NORESTART;
	}

	priv->vibrator_working = OFF;
	vibrator_power_off(priv);
	hwlog_info("%s: succ!!\n", __func__);
	return HRTIMER_NORESTART;
}


static void vibrator_enable_work(struct work_struct *work)
{
	struct classd_vibrator_data *priv = NULL;
	int state;

	priv = container_of(work, struct classd_vibrator_data, enable_work);
	if (priv == NULL) {
		hwlog_err("%s: invalid data", __func__);
		return;
	}

	state = priv->state;
	hwlog_info("%s: enter!! state = %d", __func__, state);

	mutex_lock(&priv->lock);
	if (state)
		vibrator_on(priv);
	else
		vibrator_off(priv);

	mutex_unlock(&priv->lock);
}
static void classd_vibrator_enable(struct led_classdev *cdev,
	enum led_brightness state)
{
	struct classd_vibrator_data *priv = NULL;

	priv = container_of(cdev, struct classd_vibrator_data, cdev);
	if (priv == NULL) {
		hwlog_err("%s: invalid data", __func__);
		return;
	}

	hwlog_info("%s: enter!! state = %d\n", __func__, (int)state);
	priv->state = (int) state;
	schedule_work(&priv->enable_work);
}

static int classd_vibrator_register_led_classdev(
	struct classd_vibrator_data *priv)
{
	struct led_classdev *cdev = &priv->cdev;

	cdev->name = "vibrator";
	cdev->flags = LED_CORE_SUSPENDRESUME;
	cdev->brightness_set = classd_vibrator_enable;
	cdev->default_trigger = "transient";

	return led_classdev_register(NULL, cdev);
}

static int classd_vibrator_parse_dts_param(struct classd_vibrator_data *priv)
{
	struct device_node *dn = NULL;
	int ret = 0;

	dn = priv->dev->of_node;

	ret += of_property_read_u32(dn, "reg_address",
		&priv->classd_reg_address);
	ret += of_property_read_u32(dn, "start_vib_value",
		&priv->start_vib_value);
	ret += of_property_read_u32(dn, "stop_vib_value",
		&priv->stop_vib_value);
	if (ret < 0) {
		hwlog_err("%s read reg fail, ret = %d", __func__, ret);
		return -1;
	}
	hwlog_info("%s reg is 0x%x, val is 0x%x, 0x%x", __func__,
		priv->classd_reg_address,
		priv->start_vib_value,
		priv->stop_vib_value);

	ret = of_property_read_u32(dn, "reverse_vib_value",
		&priv->reverse_vib_value);
	if (ret < 0) {
		hwlog_info("%s, read reverse_vib_value fail, not support",
			__func__);
		priv->support_quick_stop = 0; // not support quick stop
	} else {
		priv->support_quick_stop = 1; // support quick stop
	}
	hwlog_info("%s reverse_vib_value is 0x%x, support_quick_stop is %u",
		__func__, priv->reverse_vib_value, priv->support_quick_stop);

	ret = of_property_read_u32(dn, "stop_delay_ms", &stop_delay_time_ms);
	if (ret < 0) {
		hwlog_info("%s, read stop_delay_ms fail, use default",
			__func__);
		stop_delay_time_ms = DEFAULT_STOP_DELAY_MS;
	}
	ret = of_property_read_u32(dn, "min_vib_timeout", &min_vib_time_ms);
	if (ret < 0) {
		hwlog_info("%s, read min_vib_timeout fail, use default",
			__func__);
		min_vib_time_ms = DEFAULT_MIN_VIB_TIMEOUT;
	}
	hwlog_info("%s stop_delay_ms = %u, min_vib_timeout is %u", __func__,
		stop_delay_time_ms, min_vib_time_ms);

	return 0;
}

static int classd_vibrator_get_ldo_regulator(struct platform_device *pdev,
	struct classd_vibrator_data *priv)
{
	struct device_node *dn = NULL;
	int ret;

	dn = priv->dev->of_node;
	if (dn == NULL) {
		hwlog_err("%s: null node", __func__);
		return -1;
	}

	priv->ldo = devm_regulator_get(&pdev->dev, "vibrator-vdd");
	if (IS_ERR_OR_NULL(priv->ldo)) {
		hwlog_info("%s: not find ldo node", __func__);
		return -1;
	}

	ret = of_property_read_u32(dn, "vout_voltage", (u32 *)&(priv->vout_voltage));
	if (ret != 0) {
		hwlog_err("%s: read voltage fail ,use default", __func__);
		priv->vout_voltage = 3000000; // 3V
	}

	ret = regulator_set_voltage(priv->ldo,
		priv->vout_voltage, priv->vout_voltage);
	if (ret != 0) {
		hwlog_err("%s: set voltage fail", __func__);
		return -1;
	}

	hwlog_info("%s: succ! vout_voltage = %d", __func__, priv->vout_voltage);
	return 0;
}

#ifdef CONFIG_CLASSD_DEBUG
static ssize_t classd_vibrator_min_timeout_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		hwlog_info("%s: NULL point", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 10, &value)) {
		hwlog_info("%s read value error", __func__);
		return (ssize_t)count;
	}

	min_vib_time_ms = (uint32_t)value;
	hwlog_info("%s change min_vib_time_ms is %u", min_vib_time_ms);
	return (ssize_t)count;
}

static ssize_t classd_vibrator_stop_delay_time_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", (int)stop_delay_time_ms);
}

static ssize_t classd_vibrator_stop_delay_time_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;

	if (buf == NULL) {
		hwlog_info("%s: NULL point\n", __func__);
		return (ssize_t)count;
	}

	if (kstrtoull(buf, 10, &value)) {
		hwlog_info("%s read value error\n", __func__);
		return (ssize_t)count;
	}

	stop_delay_time_ms = (uint32_t)value;
	hwlog_info("%s change stop delay time is %u", stop_delay_time_ms);
	return (ssize_t)count;
}
#endif

static ssize_t classd_vibrator_min_timeout_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%d\n", (int)min_vib_time_ms);
}


#ifdef CONFIG_CLASSD_DEBUG
static DEVICE_ATTR(vibrator_stop_delay_time,
	0600,
	classd_vibrator_stop_delay_time_show,
	classd_vibrator_stop_delay_time_store);

static DEVICE_ATTR(
	vibrator_min_timeout,
	0664,
	classd_vibrator_min_timeout_show,
	classd_vibrator_min_timeout_store);
#else
static DEVICE_ATTR(
	vibrator_min_timeout,
	0664,
	classd_vibrator_min_timeout_show,
	NULL);
#endif

static struct attribute *vb_attributes[] = {
	&dev_attr_vibrator_min_timeout.attr,
#ifdef CONFIG_CLASSD_DEBUG
	&dev_attr_vibrator_stop_delay_time.attr,
#endif
	NULL
};

static const struct attribute_group vb_attr_group = {
	.attrs = vb_attributes,
};


static const struct of_device_id classd_vibrator_match[] = {
	{ .compatible = "huawei,classd-vibrator", },
	{},
};
MODULE_DEVICE_TABLE(of, classd_vibrator_match);

static int classd_vibrator_probe(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = NULL;
	int ret;

	if (!of_match_node(classd_vibrator_match, pdev->dev.of_node)) {
		hwlog_err("%s, dev node no match. exiting", __func__);
		return -ENODEV;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	priv->dev = &pdev->dev;

	ret = classd_vibrator_parse_dts_param(priv);
	if (ret < 0) {
		hwlog_err("%s, pares dts fail", __func__);
		return -EINVAL;
	}

	ret = classd_vibrator_get_ldo_regulator(pdev, priv);
	if (ret < 0) {
		hwlog_info("%s, get ldo fail, not need power", __func__);
		priv->ldo = NULL;
	}

	ret = classd_vibrator_register_led_classdev(priv);
	if (ret < 0) {
		hwlog_err("%s failed to register led_classdevpriv\n", __func__);
		goto led_register_err;
	}

	ret = sysfs_create_group(&priv->cdev.dev->kobj, &vb_attr_group);
	if (ret < 0) {
		hwlog_err("%s, failed to creat sysfs", __func__);
		goto sys_creat_err;
	}

	mutex_init(&priv->lock);
	mutex_init(&priv->ldo_lock);
	INIT_WORK(&priv->enable_work, vibrator_enable_work);
	hrtimer_init(&priv->stop_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->stop_timer.function = classd_vibrator_stop_timer_func;
	priv->ldo_status = OFF;
	priv->vibrator_working = OFF;
	platform_set_drvdata(pdev, priv);
	hwlog_info("%s: successful!\n", __func__);
	return 0;

sys_creat_err:
	led_classdev_unregister(&priv->cdev);
led_register_err:
	devm_regulator_put(priv->ldo);
	priv->ldo = NULL;
	dev_err(&pdev->dev, "%s: failed!\n", __func__);
	return ret;
}

static int classd_vibrator_remove(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);

	if (priv == NULL) {
		dev_err(&pdev->dev, "%s:pdata is NULL\n", __func__);
		return -ENODEV;
	}

	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	sysfs_remove_group(&priv->cdev.dev->kobj, &vb_attr_group);
	led_classdev_unregister(&priv->cdev);
	mutex_destroy(&priv->lock);
	mutex_destroy(&priv->ldo_lock);
	cancel_work_sync(&priv->enable_work);
	devm_regulator_put(priv->ldo);
	priv->ldo = NULL;

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static void classd_vibrator_shutdown(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);
	if (priv == NULL) {
		hwlog_err("%s: priv null!\n", __func__);
		return;
	}

	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	priv->vibrator_working = OFF;
	vibrator_power_off(priv);
}

#ifdef CONFIG_PM
static int classd_vibrator_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);

	if (priv == NULL) {
		hwlog_err("%s: priv null!", __func__);
		return -ENODEV;
	}

	if (hrtimer_active(&priv->stop_timer))
		hrtimer_cancel(&priv->stop_timer);

	priv->vibrator_working = OFF;
	vibrator_power_off(priv);
	hwlog_info("%s: enter!\n", __func__);
	return 0;
}

static int classd_vibrator_resume(struct platform_device *pdev)
{
	struct classd_vibrator_data *priv = platform_get_drvdata(pdev);

	if (priv == NULL) {
		hwlog_err("%s: priv null!", __func__);
		return -ENODEV;
	}

	hwlog_info("%s: enter!\n", __func__);
	return 0;
}
#endif


static struct platform_driver classd_vibrator_driver = {
	.probe    = classd_vibrator_probe,
	.remove   = classd_vibrator_remove,
	.shutdown = classd_vibrator_shutdown,
#ifdef CONFIG_PM
	.suspend  = classd_vibrator_suspend,
	.resume   = classd_vibrator_resume,
#endif
	.driver   = {
		.name = CLASSD_VIBRATOR,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(classd_vibrator_match),
	},
};

static int __init classd_vibrator_init(void)
{
	return platform_driver_register(&classd_vibrator_driver);
}

static void __exit classd_vibrator_exit(void)
{
	platform_driver_unregister(&classd_vibrator_driver);
}

module_init(classd_vibrator_init);
module_exit(classd_vibrator_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei classd-vibrator driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
