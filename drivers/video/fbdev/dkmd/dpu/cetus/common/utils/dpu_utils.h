/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the tedpus of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef DPU_UTILS_H
#define DPU_UTILS_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/io.h>

#include "dkmd_utils.h"

#define dpu_trace_ts_begin(start_ts) dpu_get_timestamp(start_ts)
#define dpu_trace_ts_end(start_ts, msg)  do { \
		struct timeval ts; \
		dpu_get_timestamp(&ts); \
		dpu_pr_info("%s timediff=%u", msg, dpu_timestamp_diff(&ts, start_ts)); \
	} while (0)

static inline void dpu_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	ktime_get_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}

static inline uint32_t dpu_timestamp_diff(struct timeval *lasttime, struct timeval *curtime)
{
	return (curtime->tv_usec >= lasttime->tv_usec) ? (curtime->tv_usec - lasttime->tv_usec) :
		   (1000000 - (lasttime->tv_usec - curtime->tv_usec));  /* 1s */
}

#endif /* DPU_UTILS_H */
