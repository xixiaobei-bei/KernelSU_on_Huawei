/*
 * opa2333p.h
 *
 * opa2333p driver
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

#ifndef _OPA2333P_H_
#define _OPA2333P_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#define DEVICE_DEFAULT_TEMP      25
#define ADC_DEBOUNCE_TIMES       5
#define ADC_RETRY_TIMES          3
#define ADC_DEFAULT_CHANNEL      13
#define ADC_COEF_MULTIPLE        10000
#define ADC_IN13_VBUS            1
#define ADC_IN13_IBUS            0
#define ADC_DEFAULT_IBUS         14551
#define ADC_DEFAULT_VBUS         56856
#define CURRENT_DET_DISABLE      1
#define CURRENT_DET_ENABLE       0

struct opa2333p_device_info {
	struct device *dev;
	struct notifier_block nb;
	struct mutex ntc_switch_lock;
	spinlock_t int_lock;
	int gpio_cur_det;
	int gpio_ntc_switch;
	int gpio_int;
	int irq_int;
	int under_current_detect;
	bool is_int_en;
	bool channel_switch_en;
	u32 adc_channel;
	u32 coef_ibus;
	u32 coef_vbus;
	u32 ic_role;
};

#endif /* _OPA2333P_H_ */
