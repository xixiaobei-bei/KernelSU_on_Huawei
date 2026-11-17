/*
 * weld_detect.h
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

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/pm_wakeup.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/slab.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#define GPIO_HIGH_VOLTAGE                       1
#define GPIO_LOW_VOLTAGE                        0
#define WELD_MAX_LENGTH                         20
#define WELD_ADC_TYPE_CHANNEL                   4
#define ADC_DET_CHANNEL                         12
#define MAX_STR_SIZE                            1024
#define GPIO_STAT_HIGH                          1
#define GPIO_STAT_LOW                           0

enum weld_type_t {
	ADC_DET_TYPE = 1,
	MAX_DET_TYPE,
};

struct weld_device_t {
	struct platform_device *pdev;
	unsigned int gpio;
	unsigned int weld_gpio;
	unsigned int adc_gpio_req[WELD_ADC_TYPE_CHANNEL];
	int irq;
	int weld_type;
	unsigned int trigger_type;
	int sand_idx;
	unsigned int sand_int_delay;
	struct list_head list;
	struct delayed_work h_delayed_work;
	char name[WELD_MAX_LENGTH + 1];
};

struct weld_data {
	struct list_head head;
	struct platform_device *pdev;
	int count;
	struct workqueue_struct *sand_wq;
	struct wakeup_source wakelock;
};

#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
void start_weld_detect_work(int weld_state);
#else
static inline void start_weld_detect_work(int weld_state)
{
}
#endif
