/*
 * led_alwayson.h
 *
 * led_alwayson driver
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

#ifndef __LED_ALWAYSON_H__
#define __LED_ALWAYSON_H__

struct led_alwayson_data {
	struct device *dev;
	struct class *led_alwayson_class;
	struct platform_device *pdev;
};

#define LED_ALWAYSON_NAME    "led_alwayson"
#define LED_ALWAYSON_EXIST   "led_alwayson_support"

#define LED_STATUS_NV_NUM    365
#define LED_STATUS_NV_SIZE   4

#define LED_COMMON_ERR       (-1)
#define LED_COMMON_SUC       0

#endif
