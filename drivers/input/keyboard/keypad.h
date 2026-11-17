/*
 * gpio key driver
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <asm/irq.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/proc_fs.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <securec.h>

#define TRUE 1

#define GPIO_KEY_PRESS 1
#define GPIO_KEY_RELEASE 0

#define GPIO_HIGH_VOLTAGE 1
#define GPIO_LOW_VOLTAGE 0
#define TIMER_DEBOUNCE 15

#ifdef CONFIG_KEY_MASK
#define KEY_DISABLE_MASK 1
#define KEY_ENABLE_MASK 0
#endif

#define KEY_IRQ_WAKEUP_INTERVAL 50

struct keypad_key_devices {
	const char *devices_names;
	const char *key_node_name;
	const char *work_init_name;
	const char *gpio_to_irq_name;
	struct input_dev *input_dev;
	struct delayed_work gpio_key_work;
	struct wakeup_source gpio_key_lock;
	int keypad_num;
	int keypad_irq;   /* gpio key irq */

	struct platform_device *pdev;

	void (*key_set_bit)(struct keypad_key_devices *keypad_dev);
	int (*keypad_req_irq)(struct keypad_key_devices *keypad_dev);
	void (*key_work)(struct work_struct *work);
	void (*gpio_key_timer)(unsigned long data);
};

#define MAX_KEY_IRQ_NUM 10

struct keypad_common_data {
	u32 irq_arr[MAX_KEY_IRQ_NUM];
	int irq_cnt;
#ifdef CONFIG_KEY_MASK
	int key_mask_ctl;
	int key_mask_flag;
#endif
};

extern struct keypad_common_data *keypad_get_common_data(void);
extern int keypad_get_of_node(
	const struct of_device_id *keypad_match,
		struct platform_device *pdev);
extern int keypad_register_dev(struct keypad_key_devices *keypad_dev);
extern int keypad_unregister_dev(struct keypad_key_devices *keypad_dev);
void func_key_irq_disable(void);
void func_key_irq_enable(void);

#endif
