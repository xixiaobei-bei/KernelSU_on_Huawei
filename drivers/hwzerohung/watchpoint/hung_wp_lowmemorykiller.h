/*
 * hung_wp_lowmemorykiller.h
 *
 * This file is the header file for lowmemory killer
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

#ifndef __HUNG_WP_LOWMEMORYKILLER_H_
#define __HUNG_WP_LOWMEMORYKILLER_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_WP_BUFSIZE (8 * 1024)
#define MAX_EVENTS 64

#if defined LMKWP_DEBUG
#define LMKWP_WARN(format, args...) vprintf(format, ##args)
#define LMKWP_INFO(format, args...) vprintf(format, ##args)
#else
#define LMKWP_WARN(format, args...) \
	pr_warn("lmkwp: " format, ##args)
#define LMKWP_INFO(format, args...) \
	pr_info("lmkwp: " format, ##args)
#endif

struct lmkwp_event_t {
	u64                     stamp; /* jiffies */
	struct shrink_control   sc;
	pid_t                   selected_pid;
	pid_t                   selected_tgid;
	char                    selected_comm[TASK_COMM_LEN];
	pid_t                   cur_pid;
	pid_t                   cur_tgid;
	char                    cur_comm[TASK_COMM_LEN];
	long                    cache_size;
	long                    cache_limit;
	long                    free;
	short                   adj;
};

struct lmkwp_config_t {
	u8               is_ready;
	u8               enabled;
	u8               debuggable;
	unsigned int     threshold;
	u64              period;  /* jiffies */
	u64              silence;
};

struct lmkwp_main_t {
	struct lmkwp_config_t  config;
	struct lmkwp_event_t   events[MAX_EVENTS];
	unsigned int    free;
	u64             last_report_stamp;
};

#ifdef __cplusplus
}
#endif
#endif
