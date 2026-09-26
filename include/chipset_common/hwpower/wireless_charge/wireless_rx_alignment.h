/*
 * wireless_rx_alignment.h
 *
 * common interface, variables, definition etc for wireless rx alignment driver
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

#ifndef _WIRELESS_RX_ALIGNMENT_H_
#define _WIRELESS_RX_ALIGNMENT_H_

#define WLAD_MISALIGNED              1
#define WLAD_ALIGNED                 0

struct wlad_device_info {
	struct device *dev;
	struct notifier_block wlc_nb;
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
	bool wlad_support;
	int aligned_status;
	int detect_time;
	int disconnect_time;
	int vrect_threshold_l;
	int vrect_threshold_h;
};

#endif /* _WIRELESS_RX_ALIGNMENT_H_ */
