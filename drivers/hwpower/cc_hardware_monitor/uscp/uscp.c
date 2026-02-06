/*
 * uscp.c
 *
 * usb port short circuit protect monitor driver
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_thermalzone.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>

#define HWLOG_TAG uscp
HWLOG_REGIST();

#define DEFAULT_TUSB_THRESHOLD     40
#define USB_TEMP_UPPER_LIMIT       100
#define USB_TEMP_LOWER_LIMIT       (-30)
#define GPIO_SWITCH_OPEN           1
#define GPIO_SWITCH_CLOSE          0
#define MONITOR_INTERVAL_SLOW      10000 /* 10000 ms */
#define MONITOR_INTERVAL_FAST      300 /* 300 ms */
#define CHECK_COUNT_DEFAULT_VAL    (-1)
#define CHECK_COUNT_INIT_VAL       1100
#define CHECK_COUNT_START_VAL      0
#define CHECK_COUNT_END_VAL        1001
#define CHECK_COUNT_STEP_VAL       1
#define USCP_HIZ_ENABLE            1
#define USCP_HIZ_DISABLE           0
#define DT_FOR_START_WORK          2000 /* 2s */

struct uscp_temp_info {
	int bat_temp;
	int usb_temp;
	int diff_usb_bat;
};

struct uscp_device_info {
	struct notifier_block event_nb;
	struct delayed_work start_work;
	struct delayed_work check_work;
	int gpio_uscp;
	int uscp_threshold_tusb;
	int open_mosfet_temp;
	int open_hiz_temp;
	int close_mosfet_temp;
	int interval_switch_temp;
	int dmd_hiz_enable;
	int check_interval;
	int check_count;
	struct wakeup_source *protect_wakelock;
	bool protect_enable;
	bool protect_mode;
	bool rt_protect_mode; /* real time */
	bool hiz_mode;
	bool dmd_notify_enable;
	bool dmd_notify_enable_hiz;
	bool first_in;
	bool dc_adapter;
};

static struct uscp_device_info *g_uscp_di;

static struct uscp_device_info *uscp_get_dev(void)
{
	if (!g_uscp_di) {
		hwlog_err("g_uscp_di is null\n");
		return NULL;
	}

	return g_uscp_di;
}

bool uscp_is_in_protect_mode(void)
{
	struct uscp_device_info *di = uscp_get_dev();

	if (!di)
		return false;

	return di->protect_mode;
}

bool uscp_is_in_rt_protect_mode(void)
{
	struct uscp_device_info *di = uscp_get_dev();

	if (!di)
		return false;

	return di->rt_protect_mode;
}

bool uscp_is_in_hiz_mode(void)
{
	struct uscp_device_info *di = uscp_get_dev();

	if (!di)
		return false;

	return di->hiz_mode;
}

static ssize_t uscp_dbg_show(void *dev_data, char *buf, size_t size)
{
	struct uscp_device_info *dev_p = dev_data;

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
	struct uscp_device_info *dev_p = dev_data;
	int uscp_tusb = 0;
	int open_temp = 0;
	int close_temp = 0;
	int switch_temp = 0;

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

static void uscp_dbg_register(struct uscp_device_info *di)
{
	power_dbg_ops_register("uscp_para", di,
		(power_dbg_show)uscp_dbg_show, (power_dbg_store)uscp_dbg_store);
}

#ifndef CONFIG_HLTHERM_RUNTEST
static void uscp_set_gpio_switch(struct uscp_device_info *di, int value)
{
	/* 1: GPIO_SWITCH_OPEN, open mosfet */
	/* 0: GPIO_SWITCH_CLOSE, close mosfet */
	gpio_set_value(di->gpio_uscp, value);
}

static void uscp_set_dc_adapter(struct uscp_device_info *di, bool value)
{
	di->dc_adapter = value;
}

#ifdef CONFIG_DIRECT_CHARGER
static bool uscp_get_dc_adapter(struct uscp_device_info *di)
{
	return di->dc_adapter;
}
#endif /* CONFIG_DIRECT_CHARGER */

static int uscp_dmd_report(unsigned int dmd_id, struct uscp_temp_info *temp_info)
{
	char dsm_buff[POWER_DSM_BUF_SIZE_0128] = { 0 };

	switch (dmd_id) {
	case ERROR_NO_USB_SHORT_PROTECT_NTC:
		snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"uscp ntc error happened, tusb=%d, tbatt=%d\n",
			temp_info->usb_temp, temp_info->bat_temp);
		break;
	case ERROR_NO_USB_SHORT_PROTECT:
		snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"uscp happened, tusb=%d, tbatt=%d\n",
			temp_info->usb_temp, temp_info->bat_temp);
		break;
	case ERROR_NO_USB_SHORT_PROTECT_HIZ:
		snprintf(dsm_buff, POWER_DSM_BUF_SIZE_0128 - 1,
			"uscp happened, open hiz, tusb=%d, tbatt=%d\n",
			temp_info->usb_temp, temp_info->bat_temp);
		break;
	default:
		return -EINVAL;
	}

	hwlog_info("%s\n", dsm_buff);
	return power_dsm_report_dmd(POWER_DSM_USCP, dmd_id, dsm_buff);
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int uscp_get_usb_temp(void)
{
	return power_temp_get_average_value(POWER_TEMP_USB_PORT) / POWER_TEMP_UNIT;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static int uscp_get_bat_temp(void)
{
	int temp = 0;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &temp);
	return temp;
}

static int uscp_check_usb_temp(void)
{
	int temp = uscp_get_usb_temp();
	struct uscp_temp_info temp_info;

	if ((temp > USB_TEMP_UPPER_LIMIT) || (temp < USB_TEMP_LOWER_LIMIT)) {
		temp_info.bat_temp = uscp_get_bat_temp();
		temp_info.usb_temp = temp;
		(void)uscp_dmd_report(ERROR_NO_USB_SHORT_PROTECT_NTC, &temp_info);
		hwlog_err("disable uscp when usb temp %d abnormal\n", temp);
		return -EINVAL;
	}

	return 0;
}
#else
static int uscp_check_usb_temp(void)
{
	int temp = uscp_get_usb_temp();

	if ((temp > USB_TEMP_UPPER_LIMIT) || (temp < USB_TEMP_LOWER_LIMIT)) {
		hwlog_err("disable uscp when usb temp %d abnormal\n", temp);
		return -EINVAL;
	}

	return 0;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int uscp_check_battery_exist(void)
{
	if (!power_platform_is_battery_exit()) {
		hwlog_err("disable uscp when battery is not exist\n");
		return -EINVAL;
	}

	return 0;
}

static int uscp_check_enable(struct uscp_device_info *di)
{
	int ret;

	ret = uscp_check_usb_temp();
	if (ret)
		return ret;

	di->protect_enable = true;
	hwlog_info("check enable ok\n");
	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static void uscp_check_temperature(struct uscp_device_info *di,
	struct uscp_temp_info *temp_info)
{
	int tusb = temp_info->usb_temp;
	int tdiff = temp_info->diff_usb_bat;

	if ((tusb >= di->uscp_threshold_tusb) && (tdiff >= di->open_hiz_temp)) {
		if (di->dmd_hiz_enable && di->dmd_notify_enable_hiz) {
			if (!uscp_dmd_report(ERROR_NO_USB_SHORT_PROTECT_HIZ, temp_info))
				di->dmd_notify_enable_hiz = false;
		}
	}

	if ((tusb >= di->uscp_threshold_tusb) && (tdiff >= di->open_mosfet_temp)) {
		di->rt_protect_mode = true;
		if (di->dmd_notify_enable) {
			if (!uscp_dmd_report(ERROR_NO_USB_SHORT_PROTECT, temp_info))
				di->dmd_notify_enable = false;
		}
	}
}

static void uscp_set_interval(struct uscp_device_info *di,
	struct uscp_temp_info *temp_info)
{
	int tdiff = temp_info->diff_usb_bat;

	hwlog_info("diff_temp=%d, switch_temp=%d, interval=%d, count=%d\n",
		tdiff, di->interval_switch_temp,
		di->check_interval, di->check_count);

	if (tdiff > di->interval_switch_temp) {
		di->check_interval = MONITOR_INTERVAL_FAST;
		di->check_count = CHECK_COUNT_START_VAL;
		return;
	}

	if (di->check_count > CHECK_COUNT_END_VAL) {
		/*
		 * check the temperature per 0.3 second for 100 times,
		 * when the charger just insert.
		 * set the check interval 300ms
		 */
		di->check_count -= CHECK_COUNT_STEP_VAL;
		di->check_interval = MONITOR_INTERVAL_FAST;
	} else if (di->check_count == CHECK_COUNT_END_VAL) {
		/* reset the flag when the temperature status is stable */
		/* set the check interval 10000ms */
		di->check_count = CHECK_COUNT_DEFAULT_VAL;
		di->check_interval = MONITOR_INTERVAL_SLOW;
	} else if (di->check_count >= CHECK_COUNT_START_VAL) {
		/* set the check interval 300ms */
		di->check_count += CHECK_COUNT_STEP_VAL;
		di->check_interval = MONITOR_INTERVAL_FAST;
	} else {
		/* set the check interval 10000ms */
		di->check_interval = MONITOR_INTERVAL_SLOW;
		di->protect_mode = false;
		power_wakeup_unlock(di->protect_wakelock);
	}
}

static void uscp_process_protection(struct uscp_device_info *di,
	struct uscp_temp_info *temp_info)
{
	int tusb = temp_info->usb_temp;
	int tdiff = temp_info->diff_usb_bat;
#ifdef CONFIG_DIRECT_CHARGER
	int ret;
	int state;
#endif /* CONFIG_DIRECT_CHARGER */

	/* uscp happened: enable charge hiz */
	if ((tusb >= di->uscp_threshold_tusb) && (tdiff >= di->open_hiz_temp)) {
		di->hiz_mode = true;
		hwlog_info("enable charge hiz\n");
		power_platform_set_charge_hiz(USCP_HIZ_ENABLE);
	}

	/* uscp happened: pull up mosfet */
	if ((tusb >= di->uscp_threshold_tusb) && (tdiff >= di->open_mosfet_temp)) {
		power_wakeup_lock(di->protect_wakelock);
		di->protect_mode = true;
		di->rt_protect_mode = true;

#ifdef CONFIG_DIRECT_CHARGER
		direct_charge_set_stop_charging_flag(1);

		/* wait until direct charger stop complete */
		while (true) {
			state = direct_charge_get_stage_status();
			if (direct_charge_get_stop_charging_complete_flag() &&
				((state == DC_STAGE_DEFAULT) ||
				(state == DC_STAGE_CHARGE_DONE)))
				break;
		}

		direct_charge_set_stop_charging_flag(0);

		if (di->first_in) {
			if (state == DC_STAGE_DEFAULT) {
				if (direct_charge_detect_adapter_again())
					uscp_set_dc_adapter(di, false);
				else
					uscp_set_dc_adapter(di, true);
			} else if (state == DC_STAGE_CHARGE_DONE) {
				uscp_set_dc_adapter(di, true);
			}
			di->first_in = false;
		}

		if (uscp_get_dc_adapter(di)) {
			/* close charging path: disable direct charger */
			ret = dc_set_adapter_output_enable(0);
			if (!ret) {
				hwlog_info("disable adapter output success\n");
				uscp_set_dc_adapter(di, false);
				(void)power_msleep(DT_MSLEEP_200MS, 0, NULL);
			} else {
				hwlog_info("disable adapter output fail\n");
			}
		}
#endif /* CONFIG_DIRECT_CHARGER */

		/* close charging path: open mosfet */
		power_usleep(DT_USLEEP_10MS);
		uscp_set_gpio_switch(di, GPIO_SWITCH_OPEN);
		hwlog_info("pull up mosfet\n");
	} else if (tdiff <= di->close_mosfet_temp) {
		if (di->protect_mode) {
			/* open charging path: close mosfet and disable hiz */
			uscp_set_gpio_switch(di, GPIO_SWITCH_CLOSE);
			di->rt_protect_mode = false;
			power_usleep(DT_USLEEP_10MS);
			power_platform_set_charge_hiz(USCP_HIZ_DISABLE);
			di->hiz_mode = false;
			hwlog_info("disable charge hiz and pull down mosfet\n");
		}

		if (di->hiz_mode) {
			/* open charging path: disable hiz */
			power_platform_set_charge_hiz(USCP_HIZ_DISABLE);
			di->hiz_mode = false;
			hwlog_info("disable charge hiz\n");
		}
	}
}

static void uscp_check_work(struct work_struct *work)
{
	struct uscp_device_info *di = uscp_get_dev();
	int t;
	unsigned int type = power_platform_get_charger_type();
	struct uscp_temp_info temp_info;

	if (!di)
		return;

	if ((di->check_count == CHECK_COUNT_DEFAULT_VAL) &&
		(type == CHARGER_REMOVED) &&
		(direct_charge_get_stage_status() == DC_STAGE_DEFAULT)) {
		di->dmd_notify_enable = true;
		uscp_set_gpio_switch(di, GPIO_SWITCH_CLOSE);
		di->check_interval = MONITOR_INTERVAL_SLOW;
		di->protect_mode = false;
		di->check_count = CHECK_COUNT_INIT_VAL;
		di->first_in = true;
		uscp_set_dc_adapter(di, false);

		hwlog_info("charger removed, stop uscp checking\n");
		return;
	}

	temp_info.usb_temp = uscp_get_usb_temp();
	temp_info.bat_temp = uscp_get_bat_temp();
	temp_info.diff_usb_bat = temp_info.usb_temp - temp_info.bat_temp;
	hwlog_info("tusb=%d, tbatt=%d, tdiff=%d\n",
		temp_info.usb_temp, temp_info.bat_temp, temp_info.diff_usb_bat);

	if ((temp_info.usb_temp >= USB_TEMP_LOWER_LIMIT) &&
		(temp_info.usb_temp <= USB_TEMP_UPPER_LIMIT)) {
		uscp_check_temperature(di, &temp_info);
		uscp_set_interval(di, &temp_info);
		uscp_process_protection(di, &temp_info);
	}

	t = di->check_interval;
	schedule_delayed_work(&di->check_work, msecs_to_jiffies(t));
}
#else
static void uscp_check_work(struct work_struct *work)
{
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static void uscp_charge_type_handler(struct uscp_device_info *di)
{
	unsigned int type;

	if (!di->protect_enable)
		return;

	type = power_platform_get_charger_type();
	hwlog_info("handle charger_type=%u\n", type);
	switch (type) {
	case CHARGER_TYPE_USB: /* sdp */
	case CHARGER_TYPE_BC_USB: /* cdp */
	case CHARGER_TYPE_NON_STANDARD: /* unknown */
	case CHARGER_TYPE_STANDARD: /* dcp */
	case CHARGER_TYPE_FCP: /* fcp */
	case CHARGER_TYPE_TYPEC: /* pd charger */
	case CHARGER_TYPE_PD: /* pd charger */
	case CHARGER_TYPE_SCP: /* scp charger */
		break;
	default:
		return;
	}

	if (delayed_work_pending(&di->check_work)) {
		hwlog_info("check work already started, do nothing\n");
		return;
	}

	hwlog_info("start uscp check\n");
	di->first_in = true;
	/*
	 * record 30 seconds after the charger just insert
	 * 30s = (1100 - 1001 + 1) * 300ms
	 * keep check count init number 1100
	 */
	di->check_count = CHECK_COUNT_INIT_VAL;
	schedule_delayed_work(&di->check_work, msecs_to_jiffies(0));
}

static void uscp_start_work(struct work_struct *work)
{
	struct uscp_device_info *di = uscp_get_dev();

	if (!di)
		return;

	uscp_charge_type_handler(di);
}

static int uscp_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct uscp_device_info *di = uscp_get_dev();
	int ret;

	if (!di)
		return NOTIFY_OK;

	ret = uscp_check_battery_exist();
	if (ret)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_START_CHARGING:
	case POWER_NE_STOP_CHARGING:
		cancel_delayed_work(&di->start_work);
		schedule_delayed_work(&di->start_work,
			msecs_to_jiffies(DT_FOR_START_WORK));
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int uscp_parse_dts(struct device_node *np, struct uscp_device_info *di)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"open_mosfet_temp", (u32 *)&di->open_mosfet_temp, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"close_mosfet_temp", (u32 *)&di->close_mosfet_temp, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"interval_switch_temp", (u32 *)&di->interval_switch_temp, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"uscp_threshold_tusb", (u32 *)&di->uscp_threshold_tusb,
		DEFAULT_TUSB_THRESHOLD);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"open_hiz_temp", (u32 *)&di->open_hiz_temp,
		di->open_mosfet_temp);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dmd_hiz_enable", (u32 *)&di->dmd_hiz_enable, 0);

	if (power_gpio_config_output(np,
		"gpio_usb_short_circuit_protect", "gpio_uscp",
		&di->gpio_uscp, GPIO_SWITCH_CLOSE))
		return -EINVAL;

	return 0;
}

static void uscp_init_params(struct uscp_device_info *di)
{
	di->first_in = true;
	di->dc_adapter = false;
	di->dmd_notify_enable = true;
	di->dmd_notify_enable_hiz = true;
	di->protect_enable = false;
	di->protect_mode = false;
	di->rt_protect_mode = false;
	di->hiz_mode = false;
	di->check_count = CHECK_COUNT_INIT_VAL;
}

static int uscp_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct uscp_device_info *di = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_uscp_di = di;
	np = pdev->dev.of_node;

	uscp_init_params(di);

	ret = uscp_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	ret = uscp_check_enable(di);
	if (ret)
		goto fail_free_gpio;

	di->protect_wakelock = power_wakeup_source_register(&pdev->dev,
		"uscp_protect_wakelock");
	if (!di->protect_wakelock) {
		ret = -EINVAL;
		goto fail_free_gpio;
	}

	INIT_DELAYED_WORK(&di->start_work, uscp_start_work);
	INIT_DELAYED_WORK(&di->check_work, uscp_check_work);

	di->event_nb.notifier_call = uscp_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &di->event_nb);
	if (ret)
		goto fail_free_wakelock;

	platform_set_drvdata(pdev, di);
	uscp_dbg_register(di);
	return 0;

fail_free_wakelock:
	power_wakeup_source_unregister(di->protect_wakelock);
fail_free_gpio:
	gpio_free(di->gpio_uscp);
fail_free_mem:
	kfree(di);
	g_uscp_di = NULL;
	return ret;
}

static int uscp_remove(struct platform_device *pdev)
{
	struct uscp_device_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_wakeup_source_unregister(di->protect_wakelock);
	gpio_free(di->gpio_uscp);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &di->event_nb);
	kfree(di);
	g_uscp_di = NULL;
	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
#ifdef CONFIG_PM
static int uscp_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct uscp_device_info *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	if (!di->protect_enable)
		return 0;

	hwlog_info("suspend++\n");
	cancel_delayed_work_sync(&di->start_work);
	cancel_delayed_work_sync(&di->check_work);
	hwlog_info("suspend--\n");

	return 0;
}

static int uscp_resume(struct platform_device *pdev)
{
	struct uscp_device_info *di = platform_get_drvdata(pdev);
	unsigned int type;

	if (!di)
		return 0;

	if (!di->protect_enable)
		return 0;

	type = power_platform_get_charger_type();
	hwlog_info("resume charger_type=%u\n", type);
	switch (type) {
	case CHARGER_TYPE_USB: /* sdp */
	case CHARGER_TYPE_BC_USB: /* cdp */
	case CHARGER_TYPE_NON_STANDARD: /* unknown */
	case CHARGER_TYPE_STANDARD: /* dcp */
	case CHARGER_TYPE_FCP: /* fcp */
	case CHARGER_TYPE_TYPEC: /* pd charger */
	case CHARGER_TYPE_PD: /* pd charger */
	case CHARGER_TYPE_SCP: /* scp charger */
		break;
	default:
		return 0;
	}

	schedule_delayed_work(&di->check_work, msecs_to_jiffies(0));
	hwlog_info("resume succ\n");

	return 0;
}
#endif /* CONFIG_PM */
#endif /* CONFIG_HLTHERM_RUNTEST */

static const struct of_device_id uscp_match_table[] = {
	{
		.compatible = "huawei,usb_short_circuit_protect",
		.data = NULL,
	},
	{},
};

static struct platform_driver uscp_driver = {
	.probe = uscp_probe,
#ifndef CONFIG_HLTHERM_RUNTEST
#ifdef CONFIG_PM
	/*
	 * depend on IPC driver
	 * so we set SR suspend/resume and IPC is suspend_late/early_resume
	 */
	.suspend = uscp_suspend,
	.resume = uscp_resume,
#endif /* CONFIG_PM */
#endif /* CONFIG_HLTHERM_RUNTEST */
	.remove = uscp_remove,
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
