/*
 * npu_log.h
 *
 * about npu log
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#define drv_printk(level, module, fmt, ...) \
	printk(level " [%s] [%s %d] " fmt, module, \
		__func__, __LINE__, ## __VA_ARGS__)

#define drv_err(module, fmt ...)   drv_printk(KERN_ERR, module, fmt)
#define drv_warn(module, fmt ...)  drv_printk(KERN_WARNING, module, fmt)
#define drv_info(module, fmt ...)
#define drv_debug(module, fmt ...)

#define MODULE_DEVDRV   "NPU_DRV"
#define npu_drv_err(fmt, ...)  \
	drv_printk(KERN_ERR, MODULE_DEVDRV"] [E",  fmt, ## __VA_ARGS__)
#define npu_drv_warn(fmt, ...) \
	drv_printk(KERN_WARNING, MODULE_DEVDRV"] [W",  fmt, ## __VA_ARGS__)
#define npu_drv_info(fmt, ...)
#define npu_drv_debug(fmt, ...)

#define cond_return_error(COND, ERRCODE, ...) \
			cond_return_((COND), ERRCODE, ##__VA_ARGS__)

#define cond_return_void(COND, ...) \
			cond_return_((COND), , ##__VA_ARGS__)

// Do NOT use this Marco directly
#define cond_return_(COND, ERRCODE, ...) \
			if ((COND)) { \
				npu_drv_err(__VA_ARGS__); \
				return ERRCODE; \
			}

#define cond_goto_error(COND, LABEL, ERROR, ERRCODE, ...) \
	cond_goto_with_errcode_((COND), LABEL, ERROR, ERRCODE, ##__VA_ARGS__)

#define cond_goto_debug(COND, LABEL, ERROR, ERRCODE, ...) \
	cond_goto_with_errcode_debug_((COND), LABEL, ERROR, ERRCODE, ##__VA_ARGS__)

#define cond_goto_nolog(COND, LABEL, ERROR, ERRCODE) \
			cond_goto_with_errcode_nolog_((COND), LABEL, ERROR, ERRCODE)

// Do NOT use this Marco directly
#define cond_goto_with_errcode_(COND, LABEL, ERROR, ERRCODE, ...) \
			if (COND) { \
				npu_drv_err(__VA_ARGS__); \
				ERROR = (ERRCODE); \
				goto LABEL; \
			}

// Do NOT use this Marco directly
#define cond_goto_with_errcode_debug_(COND, LABEL, ERROR, ERRCODE, ...) \
			if (COND) { \
				npu_drv_debug(__VA_ARGS__); \
				ERROR = (ERRCODE); \
				goto LABEL; \
			}

// Do NOT use this Marco directly
#define cond_goto_with_errcode_nolog_(COND, LABEL, ERROR, ERRCODE) \
			if (COND) { \
				ERROR = (ERRCODE); \
				goto LABEL; \
			}

#define npu_drv_boot_time_tag(fmt, ...)

#endif /* _DRV_LOG_H_ */
