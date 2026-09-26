/*
 * bt_debug.h
 *
 * debug api
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

#ifndef BT_DEBUG_H
#define BT_DEBUG_H

#include "bt_dev.h"

enum bt_log_level {
	BT_LOG_ALERT = 0,
	BT_LOG_ERR = 1,
	BT_LOG_WARNING = 2,
	BT_LOG_INFO = 3,
	BT_LOG_DEBUG = 4,
};

/* macro define */
#define BT_PRINT_FUNCTION_NAME \
	do { \
		if (bt_dev_log_level() >= BT_LOG_DEBUG) { \
			printk(KERN_INFO KBUILD_MODNAME ":D]%s]", __func__); \
		} \
	} while (0)

#define BT_PRINT_DBG(s, args...) \
	do { \
		if (bt_dev_log_level() >= BT_LOG_DEBUG) { \
			printk(KERN_INFO KBUILD_MODNAME ":D]%s]" s, __func__, ##args); \
		} \
	} while (0)

#define BT_PRINT_INFO(s, args...) \
	do { \
		if (bt_dev_log_level() >= BT_LOG_INFO) { \
			printk(KERN_INFO KBUILD_MODNAME ":I]%s]" s, __func__, ##args); \
		} \
	} while (0)

#define BT_PRINT_WARNING(s, args...) \
	do { \
		if (bt_dev_log_level() >= BT_LOG_WARNING) { \
			printk(KERN_WARNING KBUILD_MODNAME ":W]%s]" s, __func__, ##args); \
		} \
	} while (0)

#define BT_PRINT_ERR(s, args...) \
	do { \
		if (bt_dev_log_level() >= BT_LOG_ERR) { \
			printk(KERN_ERR KBUILD_MODNAME ":E]%s]" s, __func__, ##args); \
		} \
	} while (0)

#define BT_PRINT_ALERT(s, args...) \
	do { \
		if (bt_dev_log_level() >= BT_LOG_ALERT) { \
			printk(KERN_ALERT KBUILD_MODNAME ":ALERT]%s]" s, __func__, ##args); \
		} \
	} while (0)

#endif
