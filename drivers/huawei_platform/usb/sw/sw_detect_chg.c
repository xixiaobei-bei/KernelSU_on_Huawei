/*
 * sw_detect_chg.c
 *
 * Pogopin charger id hishow driver
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

#include <linux/kref.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/hidraw.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/tty.h>
#include <linux/crc16.h>
#include <linux/crc-ccitt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <huawei_platform/usb/hw_hishow.h>
#include <huawei_platform/power/huawei_charger_power_interface.h>
#include <huawei_platform/power/vbus_channel/vbus_channel.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include "sw_detect.h"
#include "sw_debug.h"

#define CHG_MAGIC_CODE                      0x1
#define CHG_ONLINE_CONN_MIN_ADC_LIMIT       270
#define CHG_ONLINE_CONN_MAX_ADC_LIMIT       450
#define CHG_ADC_CHECK_COUNT                 5
#define CHG_DETECT_DELAY_TIME_MS            300
#define CHG_IIN_LIMIT_100MA                 100
#define CHG_IIN_LIMIT_DEFAULT               0
#define CHG_TYPEC_BUCK_ON                   1
#define CHG_TYPEC_BUCK_OFF                  0

struct sw_chg_detectparam {
	int chg_connect_adc_min;
	int chg_connect_adc_max;
	int chg_online_adc_min;
	int chg_online_adc_max;
};

static void sw_chg_disconnected(struct sw_gpio_detector *detector,
	struct sw_chg_detectparam *param)
{
	if (!param || !detector) {
		SW_PRINT_ERR("param or detector is null\n");
		return;
	}

	SW_PRINT_FUNCTION_NAME;
	if (detector->dev_state == DEVSTAT_CHGDEV_ONLINE) {
		hishow_notify_android_uevent(HISHOW_DEVICE_OFFLINE,
			HISHOW_POGOPIN_DEVICE);
		detector->dev_state = DEVSTAT_NONEDEV;
		SW_PRINT_INFO("sw_chg_disconnected, enable irq\n");
		detector->start_detect = 1;
		if (detector->control_irq)
			detector->control_irq(detector, true);
	}
}

static void sw_chg_connected(struct sw_gpio_detector *detector,
	struct sw_chg_detectparam *param)
{
	if (!param || !detector) {
		SW_PRINT_ERR("param or detector is null\n");
		return;
	}

	SW_PRINT_FUNCTION_NAME;
	if (detector->dev_state == DEVSTAT_NONEDEV) {
		hishow_notify_android_uevent(HISHOW_DEVICE_ONLINE,
			HISHOW_POGOPIN_DEVICE);
		detector->dev_state = DEVSTAT_CHGDEV_ONLINE;
		SW_PRINT_INFO("sw_chg_connected, notify sensorhub\n");
	}

	detector->start_detect = 0;
}

static bool sw_is_chg_online(int detect_adc, struct sw_chg_detectparam *param)
{
	int adc_val;
	int i;
	int check_ok = 0;
	int ret_check = 3; /* adc check fail retry times */

	if (!param) {
		SW_PRINT_ERR("param is null\n");
		return false;
	}

retry_check:

	for (i = 0; i < CHG_ADC_CHECK_COUNT; i++) {
		adc_val = hkadc_detect_value(detect_adc);

		if ((adc_val > param->chg_connect_adc_min) &&
			(adc_val < param->chg_connect_adc_max))
			check_ok++;
	}

	/* if adc check all success, mean connected, return true */
	if (check_ok == CHG_ADC_CHECK_COUNT)
		return true;

	/*
	 * if adc check had failed ,need retry check ;
	 * if retry_check < 0 , mean disconnect ,
	 * but this checked will have some mistake
	 */
	if (ret_check > 0) {
		ret_check--;
		check_ok = 0;
		msleep(RECHECK_ADC_DELAY_MS);
		goto retry_check;
	}

	return false;
}

static bool sw_is_chg_limit_vbus_iin(void)
{
	int connect_status_now;
	int buckboost_status;
	int ret;

	connect_status_now = pogopin_get_interface_status();
	ret = vbus_ch_get_state(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST,
		&buckboost_status);
	if (ret) {
		hwlog_err("get otg buckboost fail\n");
		buckboost_status = CHG_TYPEC_BUCK_OFF;
	}

	hwlog_info("connect_status=%d,buckboost=%d\n", connect_status_now,
		buckboost_status);

	if (((connect_status_now == POGOPIN_AND_TYPEC) &&
		(buckboost_status == CHG_TYPEC_BUCK_ON)) ||
		(connect_status_now == POGOPIN_INTERFACE)) {
		hwlog_info("only pogopin in or with typc otg,limit iin\n");
		return true;
	}

	return false;
}

static int sw_chg_devdetect(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector)
{
	bool chg_isonline = false;
	struct sw_chg_detectparam *param = NULL;
	int do_iin_limit;

	if (!devdetector || !detector) {
		SW_PRINT_ERR("param is null\n");
		return -EINVAL;
	}

	param = (struct sw_chg_detectparam *)devdetector->param;
	if (!param) {
		SW_PRINT_ERR("param is null\n");
		return -EINVAL;
	}

	msleep(CHG_DETECT_DELAY_TIME_MS);

	do_iin_limit = sw_is_chg_limit_vbus_iin();
	if (do_iin_limit)
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, CHG_IIN_LIMIT_100MA);

	chg_isonline = sw_is_chg_online(detector->detect_adc_no, param);
	if (chg_isonline)
		sw_chg_connected(detector, param);
	else
		sw_chg_disconnected(detector, param);

	if (do_iin_limit)
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_VBUS_IIN_LIMIT, CHG_IIN_LIMIT_DEFAULT);

	return 0;
}

static int sw_chg_notifyevent(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector,
	unsigned long event, void *pData)
{
	struct sw_chg_detectparam *chg_param = NULL;

	if (!detector || !devdetector)
		return -EINVAL;

	chg_param = (struct sw_chg_detectparam *)devdetector->param;
	if (event == SW_NOTIFY_EVENT_DESTORY) {
		kfree(chg_param);
		kfree(devdetector);
		return 0;
	}

	if (event == SW_NOTIFY_EVENT_DISCONNECT)
		sw_chg_disconnected(detector, chg_param);

	return 0;
}

static int sw_parse_chgdetectparam(struct device_node *np,
	struct sw_chg_detectparam *chg_param)
{
	if (!np || !chg_param) {
		SW_PRINT_ERR("para is null\n");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "chg_connect_adc_min",
		&chg_param->chg_connect_adc_min)) {
		SW_PRINT_ERR("dts:can not get chg_connect_adc_min\n");
		chg_param->chg_connect_adc_min = CHG_ONLINE_CONN_MIN_ADC_LIMIT;
	}

	if (of_property_read_u32(np, "chg_connect_adc_max",
		&chg_param->chg_connect_adc_max)) {
		SW_PRINT_ERR("dts:can not get chg_connect_adc_max\n");
		chg_param->chg_connect_adc_max = CHG_ONLINE_CONN_MAX_ADC_LIMIT;
	}

	return 0;
}

struct sw_dev_detector *sw_load_chg_detect(struct device_node *np, u32 val)
{
	struct sw_dev_detector *dev_detector = NULL;
	struct sw_chg_detectparam *chg_param = NULL;
	int ret;

	SW_PRINT_FUNCTION_NAME;
	if (val != CHG_MAGIC_CODE) {
		SW_PRINT_ERR("magic code error\n");
		return NULL;
	}

	if (!np) {
		SW_PRINT_ERR("param failed\n");
		return NULL;
	}

	dev_detector = kzalloc(sizeof(*dev_detector), GFP_KERNEL);
	if (!dev_detector)
		return NULL;

	chg_param = kzalloc(sizeof(*chg_param), GFP_KERNEL);
	if (!chg_param)
		goto fail_chg_param;

	ret = sw_parse_chgdetectparam(np, chg_param);
	if (ret < 0)
		goto fail_core_init;

	dev_detector->detect_call = sw_chg_devdetect;
	dev_detector->event_call = sw_chg_notifyevent;
	dev_detector->param = chg_param;

	return dev_detector;

fail_core_init:
	kfree(chg_param);
fail_chg_param:
	kfree(dev_detector);
	return NULL;
}
