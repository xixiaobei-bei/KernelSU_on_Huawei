/*
 * bl_gpio.c
 *
 * bl_gpio driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "blpwm.h"
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/suspend.h>
#include <linux/fs.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/uaccess.h>
#include <linux/timex.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <asm/arch_timer.h>
#include <asm/arch_timer.h>
#include <huawei_platform/log/hw_log.h>
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif /* CONFIG_HUAWEI_DSM */

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bl_gpio
HWLOG_REGIST();

/* ADC */
#define ADC_LOOP_TIMES                 3
#define ADC_LOOP_PERIOD                5
#define ADC_CHANNEL                    9
#define BLPWM_CLOSE_RESET_TIME         300
#define ADC_PREPARE_TIME               10
#define ADC_SENSORHUB_PREPARE_TIME     5
#define ADC_DETECT_TIME                15000000
#define ADC_MIN_SAFE_THRESH            100
#define ADC_MAX_SAFE_THRESH            300
/* ADC v2 */
#define ADC_DETECT_NUMBER              50
#define BOM_VERSION_V1                 0
#define BOM_VERSION_V2                 1
#define ADC_V2_MIN_THRESH              200
#define ADC_V2_MAX_THRESH              1800

#define BLPWM_DSM_CLIENT_NAME          "dsm_audio_info"
#define BLPWM_DSM_ERROR_NO_ADC         921001058

#define ns2cycles(X) (((((X) / 1000 * 0x10C7UL + \
	(X) % 1000 * 0x5UL) * loops_per_jiffy * HZ) + \
	(1UL << 31)) >> 32)

struct bl_gpio_dev {
	int gpio_enable;
	int gpio_hiz;
	uint32_t volt;
	uint32_t volt_diff;
	uint32_t bom_version;
	raw_spinlock_t spin_lock;
	bool (*bl_gpio_adc_detect)(void);
};

static struct bl_gpio_dev *g_bl_gpio_dev;

static void bl_gpio_adc_dsm_report(int volt)
{
	struct dsm_client *bl_gpio_dsm_client;

	bl_gpio_dsm_client = dsm_find_client(BLPWM_DSM_CLIENT_NAME);
	if (!bl_gpio_dsm_client)
		hwlog_err("%s: dsm_find_client fail\n", __func__);
	else
		hwlog_info("%s: dsm_find_client succ\n", __func__);

	if (!dsm_client_ocuppy(bl_gpio_dsm_client)) {
		hwlog_info("%s: bl_gpio_dsm_client occupy succ\n", __func__);
		dsm_client_record(bl_gpio_dsm_client, "ADC abnormal, volt = %dmV\n", volt);
		dsm_client_notify(bl_gpio_dsm_client, BLPWM_DSM_ERROR_NO_ADC);
	} else {
		hwlog_err("%s: adc dsm report fail\n", __func__);
	}
}

static void bl_gpio_gpio_enable(int enable)
{
	if (enable)
		gpio_set_value(g_bl_gpio_dev->gpio_enable, 1);
	else
		gpio_set_value(g_bl_gpio_dev->gpio_enable, 0);

	hwlog_info("%s: %d\n", __func__, enable);
}

static int bl_gpio_set_hiz(int enable)
{
	int ret;

	if (enable) {
		ret = gpio_direction_input(g_bl_gpio_dev->gpio_hiz);
		if (ret < 0)
			hwlog_err("%s: can not set gpio-%d's direction to input\n",
				__func__, g_bl_gpio_dev->gpio_hiz);
	} else {
		ret = gpio_direction_output(g_bl_gpio_dev->gpio_hiz, 0);
		if (ret < 0)
			hwlog_err("%s can not set gpio-%d's direction to output\n",
				__func__, g_bl_gpio_dev->gpio_hiz);
	}
	hwlog_info("%s: %d", __func__, enable);

	return ret;
}

static void bl_gpio_close(void)
{
	bl_gpio_gpio_enable(DISABLE);
	blpwm_close_sensorhub();

	if (g_bl_gpio_dev->bom_version == BOM_VERSION_V1) {
		bl_gpio_set_hiz(DISABLE);
	} else {
		bl_gpio_set_hiz(ENABLE);
	}
}

static void bl_gpio_delay(unsigned long ns)
{
	cycles_t start;
	unsigned long cycles = ns2cycles(ns);

	/* make sure IO write finished */
	wmb();
	cycles = (cycles > 1) ? (cycles - 1) : cycles;
	start = get_cycles();
	while ((get_cycles() - start) < cycles)
		;
}

static bool bl_gpio_adc_detect_v1(void)
{
	unsigned long flags;
	int loop = ADC_LOOP_TIMES;

	while (loop--) {
		mdelay(ADC_LOOP_PERIOD); /* 5ms */
		blpwm_config_sensorhub(SUB_CMD_BLPWM_ADC);
		mdelay(ADC_SENSORHUB_PREPARE_TIME);
		bl_gpio_gpio_enable(ENABLE);
		bl_gpio_set_hiz(ENABLE);
		mdelay(ADC_PREPARE_TIME);

		raw_spin_lock_irqsave(&g_bl_gpio_dev->spin_lock, flags);
		bl_gpio_set_hiz(DISABLE);
		bl_gpio_delay(ADC_DETECT_TIME); /* 15ms */
		raw_spin_unlock_irqrestore(&g_bl_gpio_dev->spin_lock, flags);

		g_bl_gpio_dev->volt = hisi_adc_get_value(ADC_CHANNEL);
		if (g_bl_gpio_dev->volt < 0)
			hwlog_err("%s: get adc fail, err: %d\n", __func__, g_bl_gpio_dev->volt);
		hwlog_info("volt is %d\n", g_bl_gpio_dev->volt);
		bl_gpio_close();
		if ((g_bl_gpio_dev->volt >= ADC_MIN_SAFE_THRESH) &&
			(g_bl_gpio_dev->volt <= ADC_MAX_SAFE_THRESH)) {
			hwlog_info("volt is normal, loop is %d\n", loop);
			blpwm_open_sensorhub();
			return false;
		} else if (g_bl_gpio_dev->volt < ADC_MIN_SAFE_THRESH) {
			hwlog_info("capacity is large, loop is %d\n", loop);
		} else {
			hwlog_info("shortcurcuit, loop is %d\n", loop);
		}
	}

	return true;
}

static bool bl_gpio_adc_detect_v2(void)
{
	int max_volt = INT_MIN;
	int min_volt = INT_MAX;
	int tmp;
	int cnt = 0;
	int loop = ADC_LOOP_TIMES;

	while (loop > 0) {
		loop--;
		blpwm_open_sensorhub();
		blpwm_config_sensorhub(SUB_CMD_BLPWM_ADC_V2);
		bl_gpio_set_hiz(ENABLE);
		bl_gpio_gpio_enable(ENABLE);
		mdelay(5); /* sleep 5ms to finsh config_sensorhub */
		bl_gpio_set_hiz(DISABLE);
		while (cnt <= ADC_DETECT_NUMBER) {
			tmp = hisi_adc_get_value(ADC_CHANNEL);
			if (tmp > max_volt)
				max_volt = tmp;
			else if (tmp < min_volt)
				min_volt = tmp;
			cnt++;
			bl_gpio_delay(2000000); /* interval 2ms to read adc volt */
		}

		bl_gpio_gpio_enable(DISABLE);
		bl_gpio_set_hiz(ENABLE);
		hwlog_info("loop: %d, max volt: %d, min volt: %d\n", loop, max_volt, min_volt);
		g_bl_gpio_dev->volt_diff = max_volt - min_volt;

		if (g_bl_gpio_dev->volt_diff >= ADC_V2_MIN_THRESH) {
			hwlog_info("blpwm detect ok, diff is %d\n", g_bl_gpio_dev->volt_diff);
			return false;
		} else {
			hwlog_err("blpwm not support, diff is %d\n", g_bl_gpio_dev->volt_diff);
		}
	}

	return true;
}

static void bl_gpio_adc_init(void)
{
	if (g_bl_gpio_dev->bom_version == BOM_VERSION_V1)
		g_bl_gpio_dev->bl_gpio_adc_detect = bl_gpio_adc_detect_v1;
	else if (g_bl_gpio_dev->bom_version == BOM_VERSION_V2)
		g_bl_gpio_dev->bl_gpio_adc_detect = bl_gpio_adc_detect_v2;
}

static void bl_gpio_sensorhub_notify(pkt_blpwm_sensor_req_t *msg)
{
	if (!g_bl_gpio_dev) {
		hwlog_info("%s: g_bl_gpio_dev null\n", __func__);
		return;
	}

	if (!blpwm_is_support())
		return;

	if (msg->status == AP_CLOSE) {
		bl_gpio_gpio_enable(DISABLE);
		if (g_bl_gpio_dev->bom_version == BOM_VERSION_V1)
			bl_gpio_set_hiz(DISABLE);
		else
			bl_gpio_set_hiz(ENABLE);
		hwlog_info("%s: close blpwm done\n", __func__);
	} else if (msg->status == AP_OPEN) {
		bl_gpio_gpio_enable(ENABLE);
		bl_gpio_set_hiz(ENABLE);
		hwlog_info("%s: open blpwm done\n", __func__);
	} else {
		hwlog_err("%s: invalid status\n", __func__);
	}
}

static int bl_gpio_cover_detect(void)
{
	int color;
	int support = -1;
	int cover_status;

	cover_status = blpwm_get_cover_status();
	if (cover_status == COVER_STATUS_DEBUG) {
		hwlog_info("%s: cover status has been debuged\n", __func__);
		return -1;
	}

	color = blpwm_get_color();
	if (blpwm_is_valid_color(color)) {
		if (g_bl_gpio_dev->bl_gpio_adc_detect()) {
			hwlog_info("%s: B2 abnormal with color\n", __func__);
			support = ABNORMAL_WITH_COLOR;
		} else {
			hwlog_info("%s: B2 normal with color\n", __func__);
			support = SUPPORT_WITH_COLOR;
		}
	} else {
		if (g_bl_gpio_dev->bl_gpio_adc_detect()) {
			hwlog_info("%s: B2 unsupprt with color null\n", __func__);
			support = UNSUPPORT_WITH_NOCOLOR;
		} else {
			hwlog_info("%s: B2 normal with color null\n", __func__);
			support = SUPPORT_WITH_NOCOLOR;
		}
	}

	if (support == ABNORMAL_WITH_COLOR) {
		if (g_bl_gpio_dev->bom_version == BOM_VERSION_V1)
			bl_gpio_adc_dsm_report(g_bl_gpio_dev->volt);
		else if (g_bl_gpio_dev->bom_version == BOM_VERSION_V2)
			bl_gpio_adc_dsm_report(g_bl_gpio_dev->volt_diff);
	}
	hwlog_info("%s: detect done\n", __func__);
	return support;
}

static void bl_gpio_set_usage(int usage)
{
	if (!blpwm_is_support())
		return;

	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_USAGE);
	hwlog_info("%s : done\n", __func__);
}

static void bl_gpio_set_duty(int duty)
{
	if (!blpwm_is_support())
		return;

	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_PWM);
	bl_gpio_set_hiz(ENABLE);
	bl_gpio_gpio_enable(ENABLE);
	hwlog_info("%s : done\n", __func__);
}

static void bl_gpio_set_usage_stop(int usage_stop)
{
	if (!blpwm_is_support())
		return;

	bl_gpio_gpio_enable(DISABLE);
	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_USAGE_STOP);
	if (g_bl_gpio_dev->bom_version == BOM_VERSION_V1) {
		hwlog_info("bom version is 0\n");
		bl_gpio_set_hiz(DISABLE);
	} else {
		hwlog_info("bom version is 1\n");
		bl_gpio_set_hiz(ENABLE);
	}
	hwlog_info("%s : done\n", __func__);
}

static struct blpwm_ops g_ops = {
	.blpwm_cover_detect = bl_gpio_cover_detect,
	.blpwm_set_duty = bl_gpio_set_duty,
	.blpwm_set_usage = bl_gpio_set_usage,
	.blpwm_set_usage_stop = bl_gpio_set_usage_stop,
	.blpwm_notiby_by_sns = bl_gpio_sensorhub_notify,
};

static ssize_t bl_gpio_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int value = 0;

	if (!buf)
		return -EINVAL;

	if (!g_bl_gpio_dev)
		return -ENODEV;

	ret = kstrtoint(buf, DEBUG_BUFFER_SIZE, &value);
	if (ret) {
		hwlog_err("%s: convert to int type failed\n", __func__);
		return ret;
	}
	if (value) {
		bl_gpio_set_hiz(ENABLE);
		bl_gpio_gpio_enable(ENABLE);
		blpwm_open_sensorhub();
	} else {
		bl_gpio_close();
	}

	return count;
}

static DEVICE_ATTR(bl_gpio, 0220, NULL, bl_gpio_store);

static struct attribute *bl_gpio_attributes[] = {
	&dev_attr_bl_gpio.attr,
	NULL
};

static const struct attribute_group bl_gpio_attr_group = {
	.attrs = bl_gpio_attributes,
};

static int bl_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	g_bl_gpio_dev = devm_kzalloc(dev, sizeof(*g_bl_gpio_dev), GFP_KERNEL);
	if (!g_bl_gpio_dev)
		return -ENOMEM;

	g_bl_gpio_dev->gpio_enable = of_get_named_gpio(dev->of_node, "gpio_enable", 0);
	if (g_bl_gpio_dev->gpio_enable < 0) {
		hwlog_err("%s can not get gpio_enable number!", __func__);
		goto get_enable_gpio_fail;
	}

	g_bl_gpio_dev->gpio_hiz = of_get_named_gpio(dev->of_node, "gpio_hiz", 0);
	if (g_bl_gpio_dev->gpio_hiz < 0) {
		hwlog_err("%s can not get gpio_hiz number!", __func__);
		goto get_hiz_gpio_fail;
	}

	if (of_property_read_u32(dev->of_node, "bom_version", &g_bl_gpio_dev->bom_version)) {
		hwlog_err("read bom_version error\n");
		goto get_hiz_gpio_fail;
	}

	bl_gpio_adc_init();
	blpwm_register_ops(&g_ops);

	raw_spin_lock_init(&g_bl_gpio_dev->spin_lock);

	ret = sysfs_create_group(&dev->kobj, &bl_gpio_attr_group);
	if (ret < 0)
		hwlog_err("%s: failed to register sysfs bl_gpio\n", __func__);

	return ret;

get_hiz_gpio_fail:
	gpio_free(g_bl_gpio_dev->gpio_enable);
get_enable_gpio_fail:
	devm_kfree(dev, g_bl_gpio_dev);
	g_bl_gpio_dev = NULL;

	return -1;
}

static int bl_gpio_remove(struct platform_device *pdev)
{
	if (!g_bl_gpio_dev)
		return -1;

	sysfs_remove_group(&pdev->dev.kobj, &bl_gpio_attr_group);
	gpio_free(g_bl_gpio_dev->gpio_enable);
	gpio_free(g_bl_gpio_dev->gpio_hiz);
	g_bl_gpio_dev = NULL;

	return 0;
}

static const struct of_device_id bl_gpio_of_match[] = {
	{ .compatible = "huawei,bl_gpio", },
	{},
};
MODULE_DEVICE_TABLE(of, bl_gpio_of_match);

static struct platform_driver bl_gpio_driver = {
	.driver = {
		.name = "bl_gpio",
		.owner = THIS_MODULE,
		.of_match_table = bl_gpio_of_match,
	},
	.probe = bl_gpio_probe,
	.remove = bl_gpio_remove,
};

static int __init bl_gpio_init(void)
{
	return platform_driver_register(&bl_gpio_driver);
}

static void __exit bl_gpio_exit(void)
{
	platform_driver_unregister(&bl_gpio_driver);
}

device_initcall(bl_gpio_init);
module_exit(bl_gpio_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bl_gpio driver");
MODULE_AUTHOR("Huawei Technologies Co, Ltd");
