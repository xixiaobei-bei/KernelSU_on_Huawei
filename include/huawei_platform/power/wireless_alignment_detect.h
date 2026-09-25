/*
 * wireless_alignment_detect.h
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

#ifndef _WIRELESS_ALIGNMENT_DETECT_H_
#define _WIRELESS_ALIGNMENT_DETECT_H_

#define WLAD_MISALIGNED              1
#define WLAD_ALIGNED                 0

struct wlad_device_info {
	struct device *dev;
	struct notifier_block pwrkey_nb;
	struct notifier_block fb_nb;
	struct work_struct irq_work;
	struct delayed_work monitor_work;
	struct delayed_work powerkey_up_work;
	struct delayed_work screen_on_work;
	struct work_struct detect_work;
	struct work_struct disconnect_work;
	struct hrtimer detect_timer;
	struct hrtimer disconnect_timer;
	struct workqueue_struct *detect_wq;
	struct workqueue_struct *check_wq;
	struct wakeup_source irq_wakelock;
	struct mutex mutex_irq;
	int gpio_int;
	int irq_int;
	int gpio_status;
	bool irq_active;
	bool in_wireless_charging;
	bool in_wired_charging;
	int aligned_status;
	int detect_time;
	int disconnect_time;
	int vrect_threshold_l;
	int vrect_threshold_h;
};

enum wlad_ne_type {
	WLAD_EVT_WIRELESS_CONNECT = 0,
	WLAD_EVT_WIRELESS_DISCONNECT,
	WLAD_EVT_WIRED_CONNECT,
	WLAD_EVT_WIRED_DISCONNECT,
};

#ifdef CONFIG_WIRELESS_ALIGNMENT_DETECT
void wlad_event_notify(int event_type);
#else
static inline void wlad_event_notify(int event_type)
{
}
#endif /* CONFIG_WIRELESS_ALIGNMENT_DETECT */

#endif /* _WIRELESS_ALIGNMENT_DETECT_H_ */
