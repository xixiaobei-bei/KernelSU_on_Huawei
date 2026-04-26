/*
 * weld_detect.c
 *
 * code for weld_detect
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * Description: code for weld detect
 * Author: hujianglei
 * Create: 2020-05-23
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
#include <linux/version.h>
#include <linux/module.h>
#if defined(CONFIG_HISI_HKADC)
#include <linux/hisi/hisi_adc.h>
#endif
#include <linux/init.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/sensor/weld_detect.h>
#include <securec.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/uaccess.h>
#include <linux/mtd/hisi_nve_interface.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG weld_dect
HWLOG_REGIST();

#define ADC_DETECT_THRESHOLD     1077
#define FPC_60PIN_DET_ID         926007507
static struct weld_device_t weld_device;
static struct delayed_work fold_adc_init_check_work;
static bool is_fold_adc_detect = true;
static int support_weld_detect = 0;
static int weld_detect_is_running = 0;
static const struct of_device_id weld_match_table[] = {
	{ .compatible = "fpc,weld_detect" },
	{},
};
MODULE_DEVICE_TABLE(of, weld_match_table);

#ifdef CONFIG_HUAWEI_DSM
extern struct dsm_client *shb_dclient;
#endif

static void weld_dmd_excep_report(int adc)
{
#ifdef CONFIG_HUAWEI_DSM
	if (dsm_client_ocuppy(shb_dclient))
		return;
	dsm_client_record(shb_dclient, "weld detect fail = %d\n", adc);
	dsm_client_notify(shb_dclient, FPC_60PIN_DET_ID);
	hwlog_err("weld_dmd_excep_report\n");
#endif
}

static int weld_broken_check(void)
{
	int value;

	value = gpio_get_value(weld_device.weld_gpio);
	hwlog_info("%s gpio-%u, val-%d\n", __func__,
		weld_device.weld_gpio, value);
	return value;
}

static int weld_adc_get_gpio(struct weld_device_t *weld,
	struct device_node *node)
{
	int gpio_t;

	if (!weld)
		return -EINVAL;

	gpio_t = of_get_named_gpio(node, "fpc,adc_gpio0", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("weld pole gpio0 is invalid\n");
		return -EINVAL;
	}
	weld->adc_gpio_req[0] = (unsigned int)gpio_t;

	gpio_t = of_get_named_gpio(node, "fpc,adc_gpio1", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("weld pole gpio1 is invalid\n");
		return -EINVAL;
	}
	weld->adc_gpio_req[1] = (unsigned int)gpio_t;

	gpio_t = of_get_named_gpio(node, "fpc,adc_gpio2", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("weld pole gpio2 is invalid\n");
		return -EINVAL;
	}
	weld->adc_gpio_req[2] = (unsigned int)gpio_t;

	gpio_t = of_get_named_gpio(node, "fpc,adc_gpio3", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("weld pole gpio3 is invalid\n");
		return -EINVAL;
	}
	weld->adc_gpio_req[3] = (unsigned int)gpio_t;

	hwlog_info("%s id = %d, %d, %d, %d\n", __func__, weld->adc_gpio_req[0],
		weld->adc_gpio_req[1], weld->adc_gpio_req[2],
		weld->adc_gpio_req[3]);

	gpio_t = of_get_named_gpio(node, "fpc,weld_check", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("weld pole weld_check is invalid\n");
		return -EINVAL;
	}
	weld->weld_gpio = (unsigned int)gpio_t;
	hwlog_info("%s weld_gpio = %u\n", __func__, weld->weld_gpio);

	return 0;
}

static int weld_adc_request(void)
{
	int ret;

	ret = gpio_request(weld_device.adc_gpio_req[0], "sand_weld0");
	if (ret < 0) {
		hwlog_err("request sand0 gpio err\n");
		return ret;
	}
	ret = gpio_request(weld_device.adc_gpio_req[1], "sand_weld1");
	if (ret < 0) {
		hwlog_err("request sand1 gpio err\n");
		goto gpio1_request_err;
	}
	ret = gpio_request(weld_device.adc_gpio_req[2], "sand_weld2");
	if (ret < 0) {
		hwlog_err("request sand2 gpio err\n");
		goto gpio2_request_err;
	}
	ret = gpio_request(weld_device.adc_gpio_req[3], "sand_weld3");
	if (ret < 0) {
		hwlog_err("request sand3 gpio err\n");
		goto gpio3_request_err;
	}

	gpio_direction_output(weld_device.adc_gpio_req[0], GPIO_STAT_HIGH);
	gpio_direction_output(weld_device.adc_gpio_req[1], GPIO_STAT_HIGH);
	gpio_direction_output(weld_device.adc_gpio_req[2], GPIO_STAT_LOW);
	gpio_direction_output(weld_device.adc_gpio_req[3], GPIO_STAT_HIGH);
	return 0;

gpio3_request_err:
	gpio_free(weld_device.adc_gpio_req[2]);

gpio2_request_err:
	gpio_free(weld_device.adc_gpio_req[1]);

gpio1_request_err:
	gpio_free(weld_device.adc_gpio_req[0]);
	return -1;
}

static int weld_adc_release(void)
{
	unsigned int i;

	gpio_direction_output(weld_device.adc_gpio_req[3], GPIO_STAT_LOW);
	for (i = 0; i < WELD_ADC_TYPE_CHANNEL; i++)
		gpio_free(weld_device.adc_gpio_req[i]);

	hwlog_info("%s set R_106 Low\n", __func__);
	return 0;
}

static int weld_adc_excep_check(void)
{
	int weld_val;
	int fold_adc_val = 0;

#if defined(CONFIG_HISI_HKADC)
	hwlog_info("%s", __func__);
	fold_adc_val = hisi_adc_get_value(ADC_DET_CHANNEL);
#endif
	weld_val = weld_broken_check();
	hwlog_info("%s adc val = %d, weld val = %d\n",
		__func__, fold_adc_val, weld_val);

	if ((fold_adc_val > ADC_DETECT_THRESHOLD) || (weld_val == 1))
		weld_dmd_excep_report(fold_adc_val);

	return 0;
}

static void fold_adc_detect(void)
{
	if (is_fold_adc_detect) {
		hwlog_info("%s fold adc is already detecing\n", __func__);
		return;
	}
	is_fold_adc_detect = true;

	if (weld_adc_request() < 0) {
		is_fold_adc_detect = false;
		return;
	}
	weld_adc_excep_check();
	weld_adc_release();
	is_fold_adc_detect = false;
}

static void fold_adc_work(struct work_struct *work)
{
	hwlog_info("%s start", __func__);
	weld_detect_is_running = 1;
	fold_adc_detect();

	weld_detect_is_running = 0;
}

void start_weld_detect_work(int weld_state)
{
	static int pre_weld_state = -1;

	if (support_weld_detect == 0)
		return;
	if (weld_state == pre_weld_state)
		return;
	pre_weld_state = weld_state;
	if (weld_detect_is_running == 1) {
		hwlog_info("start_weld_detect_work is running, just return\n");
		return;
	}
	if (weld_state == 1)
		schedule_delayed_work(&fold_adc_init_check_work, msecs_to_jiffies(5));
	return;
}
EXPORT_SYMBOL_GPL(start_weld_detect_work);

static int weld_detect_probe(struct platform_device *pdev)
{
	struct device_node *temp = NULL;
	int ret;

	hwlog_info("%s in\n", __func__);
	support_weld_detect = 0;
	if (!pdev)
		return -EINVAL;
	temp = pdev->dev.of_node;

	ret = weld_adc_get_gpio(&weld_device, temp);
	if (ret) {
		hwlog_err("weld init configs failed\n");
		goto free_malloc;
	}
	INIT_DELAYED_WORK(&fold_adc_init_check_work, fold_adc_work);
	schedule_delayed_work(&fold_adc_init_check_work, msecs_to_jiffies(100000));
	is_fold_adc_detect = false;
	support_weld_detect = 1; // support weld detect
	hwlog_info("weld probe success\n");
	return 0;
free_malloc:
	hwlog_err("weld probe failed\n");
	return ret;
}

static int weld_pm_suspend(struct device *dev)
{
	return 0;
}

static int weld_pm_resume(struct device *dev)
{
	return 0;
}

static int weld_detect_remove(struct platform_device *pdev)
{
	return 0;
}

const static struct dev_pm_ops weld_pm_ops = {
	.suspend = weld_pm_suspend,
	.resume = weld_pm_resume,
};

struct platform_driver weld_driver = {
	.probe = weld_detect_probe,
	.remove = weld_detect_remove,
	.driver = {
		.name = "weld_detect",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(weld_match_table),
		.pm = &weld_pm_ops,
	},
};

static int __init weld_init(void)
{
	int ret;

	ret = platform_driver_register(&weld_driver);
	hwlog_info("%s = %d", __func__, ret);
	return ret;
}

static void __exit weld_exit(void)
{
	hwlog_info("%s exit\n", __func__);
	platform_driver_unregister(&weld_driver);
}

late_initcall(weld_init);
module_exit(weld_exit);

MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("Weld check driver");
MODULE_LICENSE("GPL");
