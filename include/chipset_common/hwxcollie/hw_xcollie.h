/*
 * hw_xcollie.h
 *
 * This file is the header file for hwxcollie
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#ifndef HW_XCOLLIE_H
#define HW_XCOLLIE_H


#define HW_INVALID_ID (-1)
/* do all callback function */
#define HW_XCOLLIE_FLAG_DEFAULT   (~0)
/* do nothing but the caller defined function */
#define HW_XCOLLIE_FLAG_NOOP      0
/* generate log file */
#define HW_XCOLLIE_FLAG_LOG       (1 << 0)
/* generate backtrace */
#define HW_XCOLLIE_FLAG_BACKTRACE (1 << 1)
/* die when timeout */
#define HW_XCOLLIE_FLAG_RECOVERY  (1 << 2)

#define HW_MAX_TIMEOUT_VAL        60
#define HW_MIN_TIMEOUT_VAL        1

#define HW_MAX_XCOLLIE_SHIFT      7
#define HW_MAX_XCOLLIE_NUM        (1 << HW_MAX_XCOLLIE_SHIFT)

#ifdef CONFIG_HW_XCOLLIE
#define hw_xcollie_start(timeout, func, arg) \
__hw_xcollie_start_ex(__func__, timeout, func, arg, HW_XCOLLIE_FLAG_DEFAULT)

#define hw_xcollie_start_ex(timeout, func, arg, flag) \
__hw_xcollie_start_ex(__func__, timeout, func, arg, flag)

int __hw_xcollie_start_ex(const char *name,
			int timeout,
			int (*func)(void *),
			void *arg,
			int flag);
int hw_xcollie_update(int id, int timeout);
void hw_xcollie_end(int id);
#endif

#endif /* HW_XCOLLIE_H */
