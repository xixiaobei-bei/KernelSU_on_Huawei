/*
 * led_config.h
 *
 * led_config driver
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LED_CONFIG_H__
#define __LED_CONFIG_H__

#include <linux/module.h>
#include <linux/types.h>

#define LED_CONFIG          "led_config"

#define MAX_DT_NAME_LEN     128
#define LED_CONFIG_BUFF_LEN 32
#define MAX_TP_INFO_LEN     16

#define turn_to_num(a, b) \
	(((a) - '\0') * 10 + ((b) - '\0'))

struct led_current_config {
	uint8_t red_curr;
	uint8_t green_curr;
	uint8_t blue_curr;
	bool got_flag;
};

struct tp_info {
	char color[LED_CONFIG_BUFF_LEN];
	char module[LED_CONFIG_BUFF_LEN];
	char ic[LED_CONFIG_BUFF_LEN];
};

struct led_config_data {
	uint8_t retry_times;
	bool tp_color_flag;
	bool tp_module_flag;
	bool tp_ic_flag;
	bool work_flag;
	struct delayed_work work;
	struct led_current_config curr;
	struct device_node *node;
};

#endif
