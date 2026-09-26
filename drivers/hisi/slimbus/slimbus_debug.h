/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#ifndef __SLIMBUS_DEBUG_H__
#define __SLIMBUS_DEBUG_H__

extern uint32_t slimbus_log_count;
extern uint32_t slimbus_rdwrerr_times;
extern volatile uint32_t slimbus_drv_log_count;
extern volatile uint32_t lostms_times;

#ifdef CONFIG_AUDIO_DEBUG
/* used to print read/write error */
#define slimbus_limit_err(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_err("[slimbus %s:%d] times:%d "msg, __func__, __LINE__, slimbus_rdwrerr_times, ## __VA_ARGS__); \
	} while (0)

#define slimbus_limit_info(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_info("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_recover_info(msg, ...) \
	do { \
		if (slimbus_rdwrerr_times > 0) \
			pr_info("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
		slimbus_rdwrerr_times = 0; \
	} while (0)

/* used to print slimbus_drv.c such as LOST_MS */
#define slimbus_drv_limit_err(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_err("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_dev_limit_info(msg, ...) \
	do { \
		lostms_times++; \
		if (lostms_times < slimbus_drv_log_count) \
			pr_info("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_dev_limit_err(msg, ...) \
	do { \
		lostms_times++; \
		if (lostms_times < slimbus_drv_log_count) \
			pr_err("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_dev_lostms_recover(msg, ...) \
	do { \
		if (lostms_times > 0) \
			pr_info("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
		lostms_times = 0; \
		slimbus_rdwrerr_times = 0; \
		dsm_notify_limit = 0; \
	} while (0)

/* core driver */
#define slimbus_core_limit_err(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_err("[slimbus %s:%d] "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)
#else
/* used to print read/write error */
#define slimbus_limit_err(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_err("[slimbus %d] times:%d "msg, __LINE__, slimbus_rdwrerr_times, ## __VA_ARGS__); \
	} while (0)

#define slimbus_limit_info(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_info("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_recover_info(msg, ...) \
	do { \
		if (slimbus_rdwrerr_times > 0) \
			pr_info("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
		slimbus_rdwrerr_times = 0; \
	} while (0)

/* used to print slimbus_drv.c such as LOST_MS */
#define slimbus_drv_limit_err(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_err("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_dev_limit_info(msg, ...) \
	do { \
		lostms_times++; \
		if (lostms_times < slimbus_drv_log_count) \
			pr_info("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_dev_limit_err(msg, ...) \
	do { \
		lostms_times++; \
		if (lostms_times < slimbus_drv_log_count) \
			pr_err("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define slimbus_dev_lostms_recover(msg, ...) \
	do { \
		if (lostms_times > 0) \
			pr_info("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
		lostms_times = 0; \
		slimbus_rdwrerr_times = 0; \
		dsm_notify_limit = 0; \
	} while (0)

/* core driver */
#define slimbus_core_limit_err(msg, ...) \
	do { \
		slimbus_rdwrerr_times++; \
		if (slimbus_rdwrerr_times < slimbus_log_count) \
			pr_err("[slimbus %d] "msg, __LINE__, ## __VA_ARGS__); \
	} while (0)
#endif /* CONFIG_AUDIO_DEBUG */
#endif

