/*
 * water_check.h
 *
 * water check monitor driver
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

#ifndef _WATER_CHECK_H_
#define _WATER_CHECK_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define WATER_IN                      1
#define WATER_NULL                    0
#define WATER_CHECK_PARA_LEVEL        16
#define DSM_LOG_SIZE                  60
#define DSM_IGNORE_NUM                99
#define DSM_NEED_REPORT               1
#define DSM_REPORTED                  0
#define DEBOUNCE_TIME                 3000
#define DELAY_WORK_TIME               8000
#define SINGLE_POSITION               1
#define GPIO_NAME_SIZE                16
#define BATFET_DISABLED_ACTION        1

enum water_check_para_info {
	WATER_TYPE = 0,
	WATER_GPIO_NAME,
	WATER_IRQ_NO,
	WATER_MULTIPLE_HANDLE,
	WATER_DMD_NO_OFFSET,
	WATER_IS_PROMPT,
	WATER_ACTION,
	WATER_PARA_TOTAL,
};

struct water_check_para {
	int type;
	char gpio_name[GPIO_NAME_SIZE];
	int irq_no;
	int dmd_no_offset;
	int gpio_no;
	u8 multiple_handle;
	u8 prompt;
	u8 action;
};

struct water_check_data {
	int total_type;
	struct water_check_para para[WATER_CHECK_PARA_LEVEL];
};

struct water_check_info {
	struct device *dev;
	struct delayed_work irq_work;
	struct water_check_data data;
	struct wakeup_source *wakelock;
	char dsm_buff[DSM_LOG_SIZE];
	u8 last_check_status[WATER_CHECK_PARA_LEVEL];
	u8 dsm_report_status[WATER_CHECK_PARA_LEVEL];
	u32 gpio_type;
	u32 irq_trigger_type;
	u32 pinctrl_enable;
};

#endif /* _WATER_CHECK_H_ */
