
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Hisilicon K3 soc camera ISP log debug switch header file
 * Author: yangkai
 * Create: 2016-03-15
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __CAM_LOG_H__
#define __CAM_LOG_H__

#include <linux/types.h>

enum {
	CAM_DEBUG_EMERG     = 1 << 7,
	CAM_DEBUG_ALERT     = 1 << 6,
	CAM_DEBUG_CRIT      = 1 << 5,
	CAM_DEBUG_ERR       = 1 << 4,
	CAM_DEBUG_WARING    = 1 << 3,
	CAM_DEBUG_NOTICE    = 1 << 2,
	CAM_DEBUG_INFO      = 1 << 1,
	CAM_DEBUG_DEBUG     = 1 << 0,
};

extern uint32_t cam_debug_mask;

#define CAMERA_LOG_DEBUG
#ifdef CAMERA_LOG_DEBUG
#define cam_debug(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_DEBUG) \
			printk("[CAMERA]" "DEBUG: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_info(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_INFO) \
			printk("[CAMERA]" "INFO: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_notice(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_NOTICE) \
			printk("[CAMERA]" "NOTICE: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_warn(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_WARING) \
			printk("[CAMERA]" "WARN: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_err(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_ERR) \
			printk("[CAMERA]" "ERROR: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_crit(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_CRIT) \
			printk("[CAMERA]" "CRIT: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_alert(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_ALERT) \
			printk("[CAMERA]" "ALERT:" fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_emerg(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_EMERG) \
			printk("[CAMERA]" "EMERG: " fmt "\n", ##__VA_ARGS__); \
	} while (0)
#else
#define cam_debug(fmt, ...)

#define cam_info(fmt, ...)

#define cam_notice(fmt, ...)

#define cam_warn(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_WARING) \
			printk("[CAMERA]" "WARN: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_err(fmt, ...) \
	do { \
		if(cam_debug_mask & CAM_DEBUG_ERR) \
			printk("[CAMERA]" "ERROR: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_crit(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_CRIT) \
			printk("[CAMERA]" "CRIT: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_alert(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_ALERT) \
			printk("[CAMERA]" "ALERT: " fmt "\n", ##__VA_ARGS__); \
	} while (0)

#define cam_emerg(fmt, ...) \
	do { \
		if (cam_debug_mask & CAM_DEBUG_EMERG) \
			printk("[CAMERA]" "EMERG: " fmt "\n", ##__VA_ARGS__); \
	} while (0)
#endif

#endif /* __CAM_LOG_H__ */