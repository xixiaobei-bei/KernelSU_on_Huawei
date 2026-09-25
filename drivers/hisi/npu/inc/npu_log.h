/*
 * npu_log.h
 *
 * about npu log
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
#ifndef _DRV_LOG_H_
#define _DRV_LOG_H_

#define DRV_PRINTK(level, module, fmt, ...) \
	printk(level " [%s] [%s %d] " fmt, module, \
		__func__, __LINE__, ## __VA_ARGS__)

#define DRV_ERR(module, fmt ...)   DRV_PRINTK(KERN_ERR, module, fmt)
#define DRV_WARN(module, fmt ...)  DRV_PRINTK(KERN_WARNING, module, fmt)
#define DRV_INFO(module, fmt ...)
#define DRV_DEBUG(module, fmt ...)

#define MODULE_DEVDRV   "NPU_DRV"
#define NPU_DRV_ERR(fmt, ...)  DRV_PRINTK(KERN_ERR, MODULE_DEVDRV"] [E",  fmt, ## __VA_ARGS__)
#define NPU_DRV_WARN(fmt, ...)  DRV_PRINTK(KERN_WARNING, MODULE_DEVDRV"] [W",  fmt, ## __VA_ARGS__)
#define NPU_DRV_INFO(fmt, ...)   /* DRV_PRINTK(KERN_INFO, MODULE_DEVDRV"] [I", fmt, ## __VA_ARGS__) */
#define NPU_DRV_DEBUG(fmt, ...)   /* DRV_PRINTK(KERN_DEBUG, MODULE_DEVDRV"] [D", fmt, ## __VA_ARGS__) */

#define COND_RETURN_ERROR(COND, ERRCODE, ...) \
			COND_RETURN_((COND), ERRCODE, ##__VA_ARGS__)

#define COND_RETURN_VOID(COND, ...) \
			COND_RETURN_((COND), , ##__VA_ARGS__)

// Do NOT use this Marco directly
#define COND_RETURN_(COND, ERRCODE, ...) \
			if ((COND)) { \
				NPU_DRV_ERR(__VA_ARGS__); \
				return ERRCODE; \
			}

#define COND_GOTO_ERROR(COND, LABEL, ERROR, ERRCODE, ...) \
			COND_GOTO_WITH_ERRCODE_((COND), LABEL, ERROR, ERRCODE, ##__VA_ARGS__)

#define COND_GOTO_DEBUG(COND, LABEL, ERROR, ERRCODE, ...) \
			COND_GOTO_WITH_ERRCODE_DEBUG_((COND), LABEL, ERROR, ERRCODE, ##__VA_ARGS__)

#define COND_GOTO_NOLOG(COND, LABEL, ERROR, ERRCODE) \
			COND_GOTO_WITH_ERRCODE_NOLOG_((COND), LABEL, ERROR, ERRCODE)

// Do NOT use this Marco directly
#define COND_GOTO_WITH_ERRCODE_(COND, LABEL, ERROR, ERRCODE, ...) \
			if (COND) { \
				NPU_DRV_ERR(__VA_ARGS__); \
				ERROR = (ERRCODE); \
				goto LABEL; \
			}

// Do NOT use this Marco directly
#define COND_GOTO_WITH_ERRCODE_DEBUG_(COND, LABEL, ERROR, ERRCODE, ...) \
			if (COND) { \
				NPU_DRV_DEBUG(__VA_ARGS__); \
				ERROR = (ERRCODE); \
				goto LABEL; \
			}

// Do NOT use this Marco directly
#define COND_GOTO_WITH_ERRCODE_NOLOG_(COND, LABEL, ERROR, ERRCODE) \
			if (COND) { \
				ERROR = (ERRCODE); \
				goto LABEL; \
			}

#define NPU_DRV_BOOT_TIME_TAG(fmt, ...) /* NPU_DRV_WARN(fmt, ## __VA_ARGS__) */

#endif /* _DRV_LOG_H_ */
