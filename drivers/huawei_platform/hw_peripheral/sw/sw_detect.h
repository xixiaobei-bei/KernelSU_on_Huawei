/*
 * sw_detect.h
 *
 * single-wire driver
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

#ifndef _SW_DETECT_H_
#define _SW_DETECT_H_

#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/completion.h>
#include <linux/of_gpio.h>
#include "sw_core.h"

#define SW_NOTIFY_EVENT_DISCONNECT    1
#define SW_NOTIFY_EVENT_REDETECT      2
#define SW_NOTIFY_EVENT_DESTROY       0x0FF

/* define dev_state value */
#define DEVSTAT_NONEDEV               0
#define DEVSTAT_KBDEV_ONLINE          1
#define DEVSTAT_CHGDEV_ONLINE         2

#define RECHECK_ADC_DELAY_MS          1
/* define start_detect value */
#define DETECT_ON                     1
#define DETECT_OFF                    0

struct sw_dev_detector;
struct sw_gpio_detector;

typedef int (*devdetect_fn_t)(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector);
typedef int (*notifyevent_fn_t)(struct sw_gpio_detector *detector,
	struct sw_dev_detector *devdetector,
	unsigned long event, void *data);
typedef void (*irqcontrol_fn_t)(struct sw_gpio_detector *detector,
	bool enable);

struct sw_dev_detector {
	devdetect_fn_t detect_call;
	notifyevent_fn_t event_call;
	void *param;
};

struct sw_gpio_detector {
	int detect_adc_no;
	int detect_int_gpio;
	int detect_int_irq;
	bool irq_enabled;
	/* use spinlock protect for detect int irq state */
	spinlock_t irq_enabled_lock;
	struct work_struct irq_work;
	irqcontrol_fn_t control_irq;
	/* record detect state DETECT_ON/DETECT_OFF */
	int start_detect;
	/* record conntect dev state (keyboard/charger) */
	int dev_state;
	struct sw_dev_detector *kb_detecor;
	struct sw_dev_detector *chg_detecor;
	struct blocking_notifier_head detect_notifier;
	struct wakeup_source kb_wlock;
};

typedef void (*notify_fn_t)(void *detector, unsigned long event, void *data);
struct sw_detector_ops {
	struct notifier_block detect_nb;
	notify_fn_t notify;
	void *detector;
};

/* function declare */
int sw_get_named_gpio(struct device_node *np, const char *propname,
	enum gpiod_flags flags);

int sw_gpio_detect_register(struct platform_device *pdev,
	struct sw_detector_ops *ops);

struct sw_dev_detector *sw_load_kb_detect(struct device_node *np);
struct sw_dev_detector *sw_load_chg_detect(struct device_node *np,
	u32 val);

#endif /* _SW_DETECT_H_ */
