/*
 * wireless_alignment_detect.c
 *
 * wireless alignment detect driver
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
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/hisi/hisi_powerkey_event.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/wireless_alignment_detect.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_transmitter.h>

#define HWLOG_TAG wireless_alignment_detect
HWLOG_REGIST();

static struct wlad_device_info *g_wlad_di;

static void wlad_enable_irq(struct wlad_device_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

static void wlad_disable_irq_nosync(struct wlad_device_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

static void wlad_wake_lock(struct wlad_device_info *di)
{
	if (di->irq_wakelock.active) {
		hwlog_info("[%s] already lock\n", __func__);
		return;
	}
	__pm_stay_awake(&di->irq_wakelock);
	hwlog_info("[%s] ++\n", __func__);
}

static void wlad_wake_unlock(struct wlad_device_info *di)
{
	if (!di->irq_wakelock.active) {
		hwlog_info("[%s] already unlock\n", __func__);
		return;
	}
	__pm_relax(&di->irq_wakelock);
	hwlog_info("[%s] --\n", __func__);
}

static void wlad_set_aligned_status(struct wlad_device_info *di,
	int aligned_status)
{
	di->aligned_status = aligned_status;
	power_ui_event_notify(POWER_UI_NE_WL_OFF_POS, &di->aligned_status);
}

static void wlad_misaligned_status_handler(struct wlad_device_info *di)
{
	hwlog_info("[%s] misaligned\n", __func__);
	wlad_disable_irq_nosync(di);
	wlad_set_aligned_status(di, WLAD_MISALIGNED);
	charge_send_uevent(NO_EVENT); /* notify framework aligned status */
}

static void wlad_aligned_status_handler(struct wlad_device_info *di)
{
	hwlog_info("[%s] aligned\n", __func__);
	wlad_set_aligned_status(di, WLAD_ALIGNED);
	if (!di->in_wireless_charging && !di->in_wired_charging)
		wlad_enable_irq(di);
}

static void wlad_detect_timeout_work(struct work_struct *work)
{
	int vrect;
	bool tx_open_flag = false;
	struct wlad_device_info *di = g_wlad_di;

	if (!di)
		return;

	hwlog_info("[%s] gpio_status = %d\n", __func__, di->gpio_status);
	vrect = wireless_charge_get_rx_vrect();
	tx_open_flag = wireless_tx_get_tx_open_flag();

	if ((vrect < di->vrect_threshold_l) &&
		(di->gpio_status == 0) && (!tx_open_flag))
		wlad_misaligned_status_handler(di);
	else if ((vrect > di->vrect_threshold_h) || (tx_open_flag))
		wlad_aligned_status_handler(di);
}

static enum hrtimer_restart wlad_detect_timer_func(struct hrtimer *timer)
{
	struct wlad_device_info *di = NULL;

	if (!timer)
		return HRTIMER_NORESTART;

	di = container_of(timer, struct wlad_device_info, detect_timer);
	if (!di)
		return HRTIMER_NORESTART;

	queue_work(di->detect_wq, &di->detect_work);
	return HRTIMER_NORESTART;
}

static void wlad_rx_disconnect_timeout_work(struct work_struct *work)
{
	struct wlad_device_info *di = g_wlad_di;

	if (!di)
		return;

	if (!di->in_wireless_charging && !di->in_wired_charging)
		wlad_enable_irq(di);
}

static enum hrtimer_restart wlad_rx_disconnect_timer_func(struct hrtimer *timer)
{
	struct wlad_device_info *di = NULL;

	if (!timer)
		return HRTIMER_NORESTART;

	di = container_of(timer, struct wlad_device_info, disconnect_timer);
	if (!di)
		return HRTIMER_NORESTART;

	queue_work(di->detect_wq, &di->disconnect_work);
	return HRTIMER_NORESTART;
}

static void wlad_monitor_work(struct work_struct *work)
{
	struct wlad_device_info *di = g_wlad_di;

	if (!di)
		return;

	msleep(50); /* delay 50ms for anti-shake, typically 1~100ms */
	di->gpio_status = gpio_get_value(di->gpio_int);
	hwlog_info("[%s] gpio_status = %d\n", __func__, di->gpio_status);
}

static void wlad_update_aligned_status(struct wlad_device_info *di)
{
	if (!di)
		return;

	if (!di->in_wireless_charging && !di->in_wired_charging) {
		wlad_set_aligned_status(di, WLAD_ALIGNED);
		wlad_enable_irq(di);
	}
}

static void wlad_powerkey_up_work(struct work_struct *work)
{
	wlad_update_aligned_status(g_wlad_di);
}

static void wlad_screen_on_work(struct work_struct *work)
{
	wlad_update_aligned_status(g_wlad_di);
}

static void wlad_rx_disconnect_handler(struct wlad_device_info *di)
{
	int t;

	di->in_wireless_charging = false;
	wlad_disable_irq_nosync(di);
	hrtimer_cancel(&di->detect_timer);
	cancel_work_sync(&di->detect_work);
	t = di->disconnect_time;
	hrtimer_start(&di->disconnect_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

static void wlad_rx_connect_handler(struct wlad_device_info *di)
{
	di->in_wireless_charging = true;
	wlad_disable_irq_nosync(di);
	hrtimer_cancel(&di->detect_timer);
	cancel_work_sync(&di->detect_work);
	hrtimer_cancel(&di->disconnect_timer);
	cancel_work_sync(&di->disconnect_work);
}

void wlad_event_notify(int event_type)
{
	struct wlad_device_info *di = g_wlad_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return;
	}

	hwlog_info("[%s] event type = %d\n", __func__, event_type);
	switch (event_type) {
	case WLAD_EVT_WIRELESS_CONNECT:
		wlad_rx_connect_handler(di);
		break;
	case WLAD_EVT_WIRED_CONNECT:
		di->in_wired_charging = true;
		wlad_disable_irq_nosync(di);
		hrtimer_cancel(&di->detect_timer);
		cancel_work_sync(&di->detect_work);
		break;
	case WLAD_EVT_WIRED_DISCONNECT:
		di->in_wired_charging = false;
		if (!di->in_wireless_charging && !di->in_wired_charging)
			wlad_enable_irq(di);
		break;
	case WLAD_EVT_WIRELESS_DISCONNECT:
		wlad_rx_disconnect_handler(di);
		break;
	default:
		break;
	}
	wlad_set_aligned_status(di, WLAD_ALIGNED);
}

static int wlad_pwrkey_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct wlad_device_info *di = g_wlad_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return NOTIFY_OK;
	}

	switch (event) {
	case HISI_PRESS_KEY_UP:
		hwlog_info("[%s] power key up\n", __func__);
		/* 100ms anti-shake work, typically 70ms */
		schedule_delayed_work(&di->powerkey_up_work,
			msecs_to_jiffies(100));
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int wlad_fb_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	int blank;
	struct wlad_device_info *di = g_wlad_di;
	struct fb_event *blank_event = (struct fb_event *)data;

	if (!di || !blank_event) {
		hwlog_err("%s: di or blank_event is null\n", __func__);
		return NOTIFY_OK;
	}

	blank = *((int *)blank_event->data);
	switch (event) {
	case FB_EVENT_BLANK:
		switch (blank) {
		case FB_BLANK_UNBLANK:
			hwlog_info("[%s] screen on\n", __func__);
			/* 100ms anti-shake work, typically 70ms */
			schedule_delayed_work(&di->screen_on_work,
				msecs_to_jiffies(100));
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static void wlad_irq_work(struct work_struct *work)
{
	int t;
	struct wlad_device_info *di = g_wlad_di;

	if (!di) {
		wlad_wake_unlock(di);
		return;
	}

	hwlog_info("[%s] begin\n", __func__);
	wlad_disable_irq_nosync(di);
	t = di->detect_time;
	hrtimer_start(&di->detect_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
	queue_delayed_work(di->check_wq, &di->monitor_work,
		msecs_to_jiffies(0));
	wlad_wake_unlock(di);
}

static irqreturn_t wlad_interrupt(int irq, void *_di)
{
	struct wlad_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	wlad_wake_lock(di);
	hwlog_info("[%s] ++\n", __func__);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("[%s] irq is not enable\n", __func__);
		wlad_wake_unlock(di);
	}
	hwlog_info("[%s] --\n", __func__);

	return IRQ_HANDLED;
}

static int wlad_init_irq(struct wlad_device_info *di, struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "wlc_int", &di->gpio_int, &di->irq_int))
		return -EINVAL;

	ret = request_irq(di->irq_int, wlad_interrupt, IRQF_TRIGGER_FALLING |
		IRQF_NO_SUSPEND, "wlad_irq", di);
	if (ret) {
		hwlog_err("%s: request irq fail\n", __func__);
		gpio_free(di->gpio_int);
		return ret;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, wlad_irq_work);

	return 0;
}

static int wlad_parse_dts(struct wlad_device_info *di, struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"disconnect_time", &di->disconnect_time,
		2500); /* default rx disconnect timeout 2500ms */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"detect_time", &di->detect_time,
		1500); /* default align detect timeout 1500ms */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vrect_threshold_l", &di->vrect_threshold_l,
		3500); /* default misaligned vrect threshold 3500mv */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vrect_threshold_h", &di->vrect_threshold_h,
		4500); /* default aligned vrect threshold 4500mv */

	return 0;
}

static void wlad_init_resource(struct wlad_device_info *di)
{
	di->aligned_status = WLAD_ALIGNED;
	di->in_wireless_charging = false;
	di->in_wired_charging = false;

	wakeup_source_init(&di->irq_wakelock, "irq_wakelock");
	mutex_init(&di->mutex_irq);

	di->detect_wq = create_singlethread_workqueue("detect_wq");
	INIT_WORK(&di->detect_work, wlad_detect_timeout_work);
	hrtimer_init(&di->detect_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->detect_timer.function = wlad_detect_timer_func;
	INIT_WORK(&di->disconnect_work, wlad_rx_disconnect_timeout_work);
	hrtimer_init(&di->disconnect_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->disconnect_timer.function = wlad_rx_disconnect_timer_func;
	di->check_wq = create_workqueue("wlad_gpio_check_wq");
	INIT_DELAYED_WORK(&di->monitor_work, wlad_monitor_work);
	INIT_DELAYED_WORK(&di->powerkey_up_work, wlad_powerkey_up_work);
	INIT_DELAYED_WORK(&di->screen_on_work, wlad_screen_on_work);
}

static int wlad_probe(struct platform_device *pdev)
{
	int ret;
	struct wlad_device_info *di = NULL;
	struct device_node *np = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wlad_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;

	wlad_init_resource(di);

	ret = wlad_parse_dts(di, np);
	if (ret)
		goto dts_parse_fail;

	ret = wlad_init_irq(di, np);
	if (ret)
		goto irq_init_fail;

	di->pwrkey_nb.notifier_call = wlad_pwrkey_event_notifier_call;
	ret = hisi_powerkey_register_notifier(&di->pwrkey_nb);
	if (ret < 0)
		goto pwrkey_reg_fail;

	di->fb_nb.notifier_call = wlad_fb_event_notifier_call;
	ret = fb_register_client(&di->fb_nb);
	if (ret < 0)
		goto fb_reg_fail;

	platform_set_drvdata(pdev, di);
	return 0;

fb_reg_fail:
pwrkey_reg_fail:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
irq_init_fail:
dts_parse_fail:
	wakeup_source_trash(&di->irq_wakelock);
	kfree(di);
	g_wlad_di = NULL;
	return ret;
}

static int wlad_remove(struct platform_device *pdev)
{
	struct wlad_device_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	wakeup_source_trash(&di->irq_wakelock);
	cancel_delayed_work(&di->monitor_work);
	cancel_delayed_work(&di->powerkey_up_work);
	cancel_delayed_work(&di->screen_on_work);
	hrtimer_cancel(&di->detect_timer);
	hrtimer_cancel(&di->disconnect_timer);
	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);
	kfree(di);
	g_wlad_di = NULL;

	return 0;
}

static const struct of_device_id wlad_match_table[] = {
	{
		.compatible = "huawei,wireless_alignment_detect",
		.data = NULL,
	},
	{},
};

static struct platform_driver wlad_driver = {
	.probe = wlad_probe,
	.remove = wlad_remove,
	.driver = {
		.name = "huawei,wireless_alignment_detect",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wlad_match_table),
	},
};

static int __init wlad_init(void)
{
	return platform_driver_register(&wlad_driver);
}

static void __exit wlad_exit(void)
{
	platform_driver_unregister(&wlad_driver);
}

device_initcall_sync(wlad_init);
module_exit(wlad_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless alignment detect module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
