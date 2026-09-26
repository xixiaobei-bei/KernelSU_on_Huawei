/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _HISI_DSS_MNTN_LOG_H_
#define _HISI_DSS_MNTN_LOG_H_

#define LOG_TAG "DSS_MNTN"

#define PRINT_ERR   1
#define PRINT_WARN  1
#define PRINT_INFO  1
#define PRINT_DEBUG  1

#if PRINT_ERR
#define DSS_MNTN_ERR(msg, ...) pr_err("[dss_mntn E]:%s: "msg, \
__func__, ## __VA_ARGS__)
#else
#define DSS_MNTN_ERR(msg, ...) ((void)0)
#endif

#if PRINT_WARN
#define DSS_MNTN_WARN(msg, ...) pr_info("[dss_mntn W]:%s: "msg, \
__func__, ## __VA_ARGS__)
#else
#define DSS_MNTN_WARN(msg, ...) ((void)0)
#endif

#if PRINT_INFO
#define DSS_MNTN_INFO(msg, ...) pr_info("[dss_mntn I]:%s: "msg, \
__func__, ## __VA_ARGS__)
#else
#define DSS_MNTN_INFO(msg, ...) ((void)0)
#endif

#if PRINT_DEBUG
#define DSS_MNTN_DBG(msg, ...) pr_info("[dss_mntn D]:%s: "msg, \
__func__, ## __VA_ARGS__)
#else
#define DSS_MNTN_DBG(msg, ...) ((void)0)
#endif
#endif
