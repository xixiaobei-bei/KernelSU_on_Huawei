/*
 * usb_short_circuit_protect.c
 *
 * usb port short circuit protect (control by ap) monitor driver
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/notifier.h>
#include <linux/wakelock.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/delay.h>
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger.h>
#endif
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/usb_short_circuit_protect.h>
#include <linux/thermal.h>
#include <huawei_platform/power/huawei_battery_temp.h>

#define HWLOG_TAG uscp_ap
HWLOG_REGIST();

static bool uscp_enable = true;
/* uscp enable parameter 0644 */
module_param(uscp_enable, bool, USCP_ENABLE_PAR);

static int usb_temp = USB_TEMP_NUM; /* usb default temp is 25 centigrade */
/* usb temperature parameter 0444 */
module_param(usb_temp, int, USB_TEMP_PAR);

static bool uscp_enforce;
/* uscp enforce parameter 0644 */
module_param(uscp_enforce, bool, USCP_ENFORCE_PAR);

struct uscp_device_info {
	struct device *dev;
	struct workqueue_struct *uscp_wq;
	struct work_struct uscp_check_wk;
	struct notifier_block usb_nb;
	struct notifier_block tcpc_nb;
	struct hrtimer timer;
	int gpio_uscp;
	int uscp_threshold_tusb;
	int open_mosfet_temp;
	int open_hiz_temp;
	int close_mosfet_temp;
	int interval_switch_temp;
	int check_interval;
	int keep_check_cnt;
	int dmd_hiz_enable;
};

static int protect_enable;
static int protect_dmd_notify_enable = 1; /* enable */
static int protect_dmd_notify_enable_hiz = 1; /* enable */
static int is_uscp_mode;
static int is_hiz_mode = NOT_HIZ_MODE;
static int is_rt_uscp_mode;
static unsigned int first_in = 1;
#ifdef CONFIG_DIRECT_CHARGER
static int is_scp_charger;
#endif /* CONFIG_DIRECT_CHARGER */

static struct uscp_device_info *g_uscp_di;
static struct wake_lock uscp_wakelock;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t uscp_dbg_show(void *dev_data, char *buf, size_t size)
{
	struct uscp_device_info *dev_p = (struct uscp_device_info *)dev_data;

	if (!buf || !dev_p) {
		hwlog_err("buf or dev_p is null\n");
		return scnprintf(buf, size, "buf or dev_p is null\n");
	}

	return scnprintf(buf, size,
		"uscp_threshold_tusb=%d\n"
		"open_mosfet_temp=%d\n"
		"close_mosfet_temp=%d\n"
		"interval_switch_temp=%d\n",
		dev_p->uscp_threshold_tusb,
		dev_p->open_mosfet_temp,
		dev_p->close_mosfet_temp,
		dev_p->interval_switch_temp);
}

static ssize_t uscp_dbg_store(void *dev_data, const char *buf, size_t size)
{
	struct uscp_device_info *dev_p = (struct uscp_device_info *)dev_data;
	unsigned int uscp_tusb = 0;
	unsigned int open_temp = 0;
	unsigned int close_temp = 0;
	unsigned int switch_temp = 0;

	if (!buf || !dev_p) {
		hwlog_err("buf or dev_p is null\n");
		return -EINVAL;
	}

	/* 4: four parameters */
	if (sscanf(buf, "%d %d %d %d",
		&uscp_tusb, &open_temp, &close_temp, &switch_temp) != 4) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	dev_p->uscp_threshold_tusb = uscp_tusb;
	dev_p->open_mosfet_temp = open_temp;
	dev_p->close_mosfet_temp = close_temp;
	dev_p->interval_switch_temp = switch_temp;

	hwlog_info("uscp_threshold_tusb=%d\n", dev_p->interval_switch_temp);
	hwlog_info("open_mosfet_temp=%d\n", dev_p->open_mosfet_temp);
	hwlog_info("close_mosfet_temp=%d\n", dev_p->close_mosfet_temp);
	hwlog_info("interval_switch_temp=%d\n", dev_p->interval_switch_temp);

	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static void uscp_wake_lock(void)
{
	if (!wake_lock_active(&uscp_wakelock)) {
		hwlog_info("wake lock\n");
		wake_lock(&uscp_wakelock);
	}
}

static void uscp_wake_unlock(void)
{
	if (wake_lock_active(&uscp_wakelock)) {
		hwlog_info("wake unlock\n");
		wake_unlock(&uscp_wakelock);
	}
}

static void uscp_charge_type_handler(struct uscp_device_info *di,
	enum hisi_charger_type type)
{
	int interval;

	if (!protect_enable)
		return;

	if ((type == CHARGER_TYPE_DCP) || (type == CHARGER_TYPE_UNKNOWN) ||
		(type == CHARGER_TYPE_SDP) || (type == CHARGER_TYPE_CDP)) {
		if (hrtimer_active(&(di->timer))) {
			hwlog_info("timer already armed, do nothing\n");
		} else {
			hwlog_info("start uscp check\n");
			interval = 0;
			first_in = 1;
			/*
			 * record 30 seconds after the charger just insert
			 * 30s = (1100 - 1001 + 1) * 300ms
			 * keep check count init number 1100
			 */
			di->keep_check_cnt = CHECK_CNT_INIT;
			hrtimer_start(&di->timer,
				ktime_set(interval / MSEC_PER_SEC,
				(interval % MSEC_PER_SEC) * USEC_PER_SEC),
				HRTIMER_MODE_REL);
		}
	} else {
		hwlog_info("charge_type=%d, do nothing\n", type);
	}
}

static int uscp_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct uscp_device_info *di = NULL;
	enum hisi_charger_type type = (enum hisi_charger_type)event;

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct uscp_device_info, usb_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	uscp_charge_type_handler(di, type);

	return NOTIFY_OK;
}

#ifdef CONFIG_TCPC_CLASS
static int uscp_pd_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	return uscp_notifier_call(nb, event, data);
}
#endif /* CONFIG_TCPC_CLASS */

static int uscp_usb_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	return uscp_notifier_call(nb, event, data);
}

static int uscp_get_temperature_value(void)
{
	int i;
	int j;
	int ret;
	struct thermal_zone_device *tz = NULL;
	int temp_array[NTC_SAMPLES] = { 0 };
	int temp_samples = 3; /* retry 3 times */
	int temp_invalid_flag;
	int sum = 0;

	tz = thermal_zone_get_zone_by_name("usb_port");
	if (IS_ERR(tz)) {
		hwlog_err("get usb_port tz fail\n");
		return 0;
	}

	while (temp_samples--) {
		temp_invalid_flag = 0;

		for (i = 0; i < NTC_SAMPLES; ++i) {
			ret = thermal_zone_get_temp(tz, &temp_array[i]);
			temp_array[i] = temp_array[i] / 1000; /* kilo temp */
			if (ret) {
				hwlog_err("get usb_port temp fail\n");
				temp_invalid_flag = 1;
			}

			hwlog_info("usb_port temp[%d]=%d\n", i, temp_array[i]);
		}

		if (temp_invalid_flag == 1)
			continue;

		/* check temperature value is valid */
		for (i = 0; i < (NTC_SAMPLES - 1); ++i) {
			for (j = (i + 1); j < NTC_SAMPLES; ++j) {
				if (abs(temp_array[i] - temp_array[j]) >
					NTC_INVALID_TEMP_THRE) {
					hwlog_err("invalid temp[%d,%d][%d,%d]\n",
						i, temp_array[i],
						j, temp_array[j]);
					temp_invalid_flag = 1;
					break;
				}
			}
		}

		if (temp_invalid_flag == 0)
			break;
	}

	if (temp_invalid_flag == 0) {
		/* get average temperature */
		for (i = 0; i < NTC_SAMPLES; ++i)
			sum += temp_array[i];

		return (sum / NTC_SAMPLES);
	}

	hwlog_err("use 0 as default usb_port temp\n");
	return 0;
}

static void uscp_set_interval(struct uscp_device_info *di, int tdiff)
{
	hwlog_info("tdiff=%d, switch_temp=%d, interval=%d, cnt=%d\n",
		tdiff, di->interval_switch_temp,
		di->check_interval, di->keep_check_cnt);

	if (tdiff > di->interval_switch_temp) {
		di->check_interval = CHECK_INTERVAL_FAST;
		di->keep_check_cnt = 0;
		return;
	}

	if (di->keep_check_cnt > CHECK_CNT_LIMIT) {
		/*
		 * check the temperature per 0.3 second for 100 times,
		 * when the charger just insert.
		 */
		di->keep_check_cnt -= 1;
		di->check_interval = CHECK_INTERVAL_FAST;
		is_uscp_mode = 0;
	} else if (di->keep_check_cnt == CHECK_CNT_LIMIT) {
		/* reset the flag when the temperature status is stable */
		di->keep_check_cnt = -1;
		/* set the check interval 10000ms */
		di->check_interval = CHECK_INTERVAL_SLOW;
		is_uscp_mode = 0;
		uscp_wake_unlock();
	} else if (di->keep_check_cnt >= 0) {
		di->keep_check_cnt = di->keep_check_cnt + 1;
		/* set the check interval 300ms */
		di->check_interval = CHECK_INTERVAL_FAST;
	} else {
		/* set the check interval 10000ms */
		di->check_interval = CHECK_INTERVAL_SLOW;
		is_uscp_mode = 0;
	}
}

static void uscp_protection_process(struct uscp_device_info *di,
	int tbatt, int tusb)
{
#ifdef CONFIG_DIRECT_CHARGER
	int ret;
	int state;
#endif /* CONFIG_DIRECT_CHARGER */
	int tdiff;

	if (!uscp_enable) {
		hwlog_info("current uscp is disabled\n");
		return;
	}

	tdiff = tusb - tbatt;

	/* uscp happened: enable charge hiz */
	if ((tusb >= di->uscp_threshold_tusb) && (tdiff >= di->open_hiz_temp)) {
		is_hiz_mode = HIZ_MODE;
		hwlog_err("enable charge hiz\n");
		charge_set_hiz_enable(HIZ_MODE_ENABLE);
	}

	/* uscp happened: pull up mosfet */
	if (((tusb >= di->uscp_threshold_tusb) &&
		(tdiff >= di->open_mosfet_temp)) || uscp_enforce) {
		uscp_wake_lock();
		is_uscp_mode = 1;
		is_rt_uscp_mode = 1;
		if (uscp_enforce)
			hwlog_err("force uscp work\n");

#ifdef CONFIG_DIRECT_CHARGER
		direct_charge_set_stop_charging_flag(1);
		state = direct_charge_get_stage_status();

		/* wait until direct charger stop complete */
		while (1) {
			state = direct_charge_get_stage_status();
			if (is_scp_stop_charging_complete() &&
				((state == SCP_STAGE_DEFAULT) ||
				(state == SCP_STAGE_CHARGE_DONE)))
				break;
		}

		direct_charge_set_stop_charging_flag(0);
		if (state == SCP_STAGE_DEFAULT) {
			if (first_in) {
				if (scp_adaptor_detect() ==
					SCP_ADAPTOR_DETECT_SUCC)
					is_scp_charger = 1;
				else
					is_scp_charger = 0;

				first_in = 0;
			}
		} else if (state == SCP_STAGE_CHARGE_DONE) {
			is_scp_charger = 1;
		}

		if (is_scp_charger) {
			/* close charging path: disable direct charger */
			ret = scp_adaptor_set_output_enable(0);
			if (!ret) {
				hwlog_err("disable scp adaptor output success\n");
				msleep(SLEEP_200MS);
			} else {
				hwlog_err("disable scp adaptor output fail\n");
			}
		}
#endif /* CONFIG_DIRECT_CHARGER */

		/* close charging path: open mosfet  */
		msleep(SLEEP_10MS);
		gpio_set_value(di->gpio_uscp, 1);
		hwlog_err("pull up mosfet\n");
	} else if (tdiff <= di->close_mosfet_temp) {
#ifdef CONFIG_DIRECT_CHARGER
		if (is_scp_charger) {
			/* open charging path: enable direct charger */
			ret = scp_adaptor_set_output_enable(1);
			if (!ret)
				hwlog_err("enable scp adaptor output success\n");
			else
				hwlog_err("enable scp adaptor output fail\n");
		}
#endif /* CONFIG_DIRECT_CHARGER */

		if (is_uscp_mode) {
			/* open charging path: close mosfet and disable hiz */
			gpio_set_value(di->gpio_uscp, 0);
			is_rt_uscp_mode = 0;
			msleep(SLEEP_10MS);
			charge_set_hiz_enable(HIZ_MODE_DISABLE);
			is_hiz_mode = NOT_HIZ_MODE;
			hwlog_info("disable charge hiz and pull down mosfet\n");
		}

		if (is_hiz_mode) {
			/* open charging path: disable hiz */
			charge_set_hiz_enable(HIZ_MODE_DISABLE);
			is_hiz_mode = NOT_HIZ_MODE;
			hwlog_info("disable charge hiz\n");
		}
	}
}

static void uscp_check_temperature(struct uscp_device_info *di)
{
	int tusb;
	int tbatt = 0;
	int tdiff;
	int batt_id;
	char buf1[POWER_DSM_BUF_SIZE_0128] = { 0 };
	char buf2[POWER_DSM_BUF_SIZE_0128] = { 0 };

	tusb = uscp_get_temperature_value();
	usb_temp = tusb;
	huawei_battery_temp(BAT_TEMP_MIXED, &tbatt);
	tdiff = tusb - tbatt;
	batt_id = hisi_battery_id_voltage();

	hwlog_info("tusb=%d, tbatt=%d, tdiff=%d\n", tusb, tbatt, tdiff);

	if ((tusb >= di->uscp_threshold_tusb) &&
		(tdiff >= di->open_hiz_temp)) {
		if (di->dmd_hiz_enable && protect_dmd_notify_enable_hiz) {
			snprintf(buf1, POWER_DSM_BUF_SIZE_0128 - 1,
				"uscp happened(ap), open hiz, tusb=%d,tbatt=%d\n",
				tusb, tbatt);
			hwlog_info("%s\n", buf1);

			if (power_dsm_dmd_report(POWER_DSM_USCP,
				ERROR_NO_USB_SHORT_PROTECT_HIZ, buf1) == 0)
				protect_dmd_notify_enable_hiz = 0; /* disable */
		}
	}

	if ((tusb >= di->uscp_threshold_tusb) &&
		(tdiff >= di->open_mosfet_temp)) {
		is_rt_uscp_mode = 1;
		if (protect_dmd_notify_enable) {
			snprintf(buf2, POWER_DSM_BUF_SIZE_0128 - 1,
				"uscp happened(ap), tusb=%d,tbatt=%d,batt_id=%d\n",
				tusb, tbatt, batt_id);
			hwlog_info("%s\n", buf2);

			if (power_dsm_dmd_report(POWER_DSM_USCP,
				ERROR_NO_USB_SHORT_PROTECT, buf2) == 0)
				protect_dmd_notify_enable = 0; /* disable */
		}
	}

	uscp_set_interval(di, tdiff);

	uscp_protection_process(di, tbatt, tusb);
}

int is_in_uscp_mode(void)
{
	return is_uscp_mode;
}

int is_uscp_hiz_mode(void)
{
	return is_hiz_mode;
}

int is_in_rt_uscp_mode(void)
{
	return is_rt_uscp_mode;
}

int get_usb_ntc_temp(void)
{
	return uscp_get_temperature_value();
}

static void uscp_check_work(struct work_struct *work)
{
	struct uscp_device_info *di = NULL;
	int interval;
	enum hisi_charger_type type = hisi_get_charger_type();

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct uscp_device_info, uscp_check_wk);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

#ifdef CONFIG_DIRECT_CHARGER
	if ((di->keep_check_cnt == -1) && (type == CHARGER_TYPE_NONE) &&
		(direct_charge_get_stage_status() == SCP_STAGE_DEFAULT))
#else
	if ((di->keep_check_cnt == -1) && (type == CHARGER_TYPE_NONE))
#endif /* CONFIG_DIRECT_CHARGER */
	{
		protect_dmd_notify_enable = 1; /* enable dmd notify */
		gpio_set_value(di->gpio_uscp, 0); /* close mosfet */
		di->keep_check_cnt = -1;
		di->check_interval = CHECK_INTERVAL_SLOW;
		is_uscp_mode = 0;
		di->keep_check_cnt = CHECK_CNT_INIT; /* check count init 1100 */
		first_in = 1;
#ifdef CONFIG_DIRECT_CHARGER
		is_scp_charger = 0;
#endif /* CONFIG_DIRECT_CHARGER */

		hwlog_info("charger removed, stop uscp checking\n");
		return;
	}

	uscp_check_temperature(di);

	interval = di->check_interval;
	hrtimer_start(&di->timer, ktime_set(interval / MSEC_PER_SEC,
		(interval % MSEC_PER_SEC) * USEC_PER_SEC), HRTIMER_MODE_REL);
}

static enum hrtimer_restart uscp_timer_func(struct hrtimer *timer)
{
	struct uscp_device_info *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct uscp_device_info, timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->uscp_wq, &di->uscp_check_wk);

	return HRTIMER_NORESTART;
}

static void uscp_check_ntc(void)
{
	int temp;
#ifndef CONFIG_HLTHERM_RUNTEST
	int tbatt = INVALID_TEMP_VAL;
	int batt_id;
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };
#endif /* CONFIG_HLTHERM_RUNTEST */

	temp = uscp_get_temperature_value();

	if (temp > CHECK_NTC_TEMP_MAX || temp < CHECK_NTC_TEMP_MIN) {
#ifndef CONFIG_HLTHERM_RUNTEST
		huawei_battery_temp(BAT_TEMP_MIXED, &tbatt);
		batt_id = hisi_battery_id_voltage();

		snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
			"usb ntc error happened, tusb=%d,tbatt=%d,batt_id=%d\n",
			temp, tbatt, batt_id);

		power_dsm_dmd_report(POWER_DSM_USCP,
			ERROR_NO_USB_SHORT_PROTECT_NTC, buf);
#endif /* CONFIG_HLTHERM_RUNTEST */

		hwlog_err("disable uscp when temp is invalid\n");
		protect_enable = 0;
	} else {
		hwlog_info("enable uscp\n");
		protect_enable = 1;
	}
}

static void uscp_check_battery(void)
{
	if (!is_hisi_battery_exist()) {
		hwlog_err("disable uscp when battery is not exist\n");
		protect_enable = 0;
	}
}

static int uscp_parse_dts(struct device_node *np,
	struct uscp_device_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "uscp_threshold_tusb",
		&(di->uscp_threshold_tusb));
	if (ret) {
		hwlog_err("uscp_threshold_tusb dts read failed\n");
		di->uscp_threshold_tusb = DEFAULT_TUSB_THRESHOLD;
	}
	hwlog_info("uscp_threshold_tusb=%d\n", di->uscp_threshold_tusb);

	ret = of_property_read_u32(np, "open_mosfet_temp",
		&(di->open_mosfet_temp));
	if (ret) {
		hwlog_err("open_mosfet_temp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("open_mosfet_temp=%d\n", di->open_mosfet_temp);

	ret = of_property_read_u32(np, "open_hiz_temp",
		&(di->open_hiz_temp));
	if (ret) {
		hwlog_err("open_hiz_temp dts read failed\n");
		di->open_hiz_temp = di->open_mosfet_temp;
	}
	hwlog_info("open_hiz_temp=%d\n", di->open_hiz_temp);

	ret = of_property_read_u32(np, "dmd_hiz_enable",
		&(di->dmd_hiz_enable));
	if (ret) {
		hwlog_err("dmd_hiz_enable dts read failed\n");
		di->dmd_hiz_enable = DMD_HIZ_DISABLE;
	}
	hwlog_info("dmd_hiz_enable=%d\n", di->dmd_hiz_enable);

	ret = of_property_read_u32(np, "close_mosfet_temp",
		&(di->close_mosfet_temp));
	if (ret) {
		hwlog_err("close_mosfet_temp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("close_mosfet_temp=%d\n", di->close_mosfet_temp);

	ret = of_property_read_u32(np, "interval_switch_temp",
		&(di->interval_switch_temp));
	if (ret) {
		hwlog_err("interval_switch_temp dts read failed\n");
		return -EINVAL;
	}
	hwlog_info("interval_switch_temp=%d\n", di->interval_switch_temp);

	di->gpio_uscp = of_get_named_gpio(np,
		"gpio_usb_short_circuit_protect", 0);
	hwlog_info("gpio_uscp=%d\n", di->gpio_uscp);

	if (!gpio_is_valid(di->gpio_uscp)) {
		hwlog_err("gpio is not valid\n");
		return -EINVAL;
	}

	ret = gpio_request(di->gpio_uscp, "uscp_ap");
	if (ret) {
		hwlog_err("gpio request fail\n");
		return -EINVAL;
	}

	ret = gpio_direction_output(di->gpio_uscp, 0);
	if (ret) {
		gpio_free(di->gpio_uscp);
		hwlog_err("gpio set output fail\n");
		return -EINVAL;
	}

	return 0;
}

static int uscp_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct uscp_device_info *di = NULL;
	enum hisi_charger_type type = hisi_get_charger_type();
	int ret;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_uscp_di = di;
	np = pdev->dev.of_node;

	ret = uscp_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	is_uscp_mode = 0;
	is_rt_uscp_mode = 0;
	di->keep_check_cnt = CHECK_CNT_INIT;

	/* first: check ntc */
	uscp_check_ntc();

	/* second: check battery */
	uscp_check_battery();

	/* third: disable uscp when ntc and battery are invalid */
	if (!protect_enable)
		goto fail_free_mem;

	wake_lock_init(&uscp_wakelock, WAKE_LOCK_SUSPEND, "uscp_wakelock");
	di->uscp_wq = create_singlethread_workqueue("uscp_wq");
	INIT_WORK(&di->uscp_check_wk, uscp_check_work);
	hrtimer_init(&di->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->timer.function = uscp_timer_func;

#ifdef CONFIG_TCPC_CLASS
	if (is_pd_supported()) {
		di->tcpc_nb.notifier_call = uscp_pd_notifier_call;
		ret = register_pd_dpm_notifier(&di->tcpc_nb);
	} else {
		di->usb_nb.notifier_call = uscp_usb_notifier_call;
		ret = hisi_charger_type_notifier_register(&di->usb_nb);
	}
#else
	di->usb_nb.notifier_call = uscp_usb_notifier_call;
	ret = hisi_charger_type_notifier_register(&di->usb_nb);
#endif /* CONFIG_TCPC_CLASS */

	if (ret < 0) {
		hwlog_err("charger_type_notifier register failed\n");
		goto fail_free_wakelock;
	}

	uscp_charge_type_handler(di, type);

	platform_set_drvdata(pdev, di);

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("uscp_para", platform_get_drvdata(pdev),
		(power_dbg_show)uscp_dbg_show, (power_dbg_store)uscp_dbg_store);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

	hwlog_info("probe end\n");
	return 0;

fail_free_wakelock:
	wake_lock_destroy(&uscp_wakelock);
fail_free_mem:
	kfree(di);
	g_uscp_di = NULL;

	return ret;
}

#ifdef CONFIG_PM
static int uscp_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct uscp_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("suspend begin\n");

	if (!di)
		return 0;

	cancel_work_sync(&di->uscp_check_wk);
	hrtimer_cancel(&di->timer);

	hwlog_info("suspend end\n");
	return 0;
}

static int uscp_resume(struct platform_device *pdev)
{
	enum hisi_charger_type type = hisi_get_charger_type();
	struct uscp_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("resume begin\n");

	if (!di)
		return 0;

	hwlog_info("charger_type=%d\n", type);
	if (type == CHARGER_TYPE_NONE)
		return 0;

	queue_work(di->uscp_wq, &di->uscp_check_wk);

	hwlog_info("resume end\n");
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id uscp_match_table[] = {
	{
		.compatible = "huawei,usb_short_circuit_protect",
		.data = NULL,
	},
	{},
};

static struct platform_driver uscp_driver = {
	.probe = uscp_probe,
#ifdef CONFIG_PM
	/*
	 * depend on IPC driver
	 * so we set SR suspend/resume and IPC is suspend_late/early_resume
	 */
	.suspend = uscp_suspend,
	.resume = uscp_resume,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,usb_short_circuit_protect",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(uscp_match_table),
	},
};

static int __init uscp_init(void)
{
	return platform_driver_register(&uscp_driver);
}

static void __exit uscp_exit(void)
{
	platform_driver_unregister(&uscp_driver);
}

device_initcall_sync(uscp_init);
module_exit(uscp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("usb port short circuit protect module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
