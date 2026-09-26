/*
 * bus_monitor.h
 *
 * head file for bus_monitor.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#ifndef BUS_MONITOR_H
#define BUS_MONITOR_H

#include <linux/ktime.h>

#define NUM_OF_CNT 11
#define NUM_OF_CFG 10

struct bus_monitor_result {
	u32 latency[NUM_OF_CNT];
	u32 osd[NUM_OF_CNT];
};

struct bus_monitor_cfg_para {
	u32 latency_range[NUM_OF_CFG];
	u32 osd_range[NUM_OF_CFG];
	unsigned int sample_type;
	unsigned int count_time;
};

#ifdef CONFIG_BUS_MONITOR
extern ktime_t bus_monitor_get_result(struct bus_monitor_result *res);
extern void bus_monitor_cfg(struct bus_monitor_cfg_para *res);
extern void bus_monitor_stop(void);
#else
static inline ktime_t bus_monitor_get_result(struct bus_monitor_result *res)
{
	(void)res;
	return 0;
}

static inline void bus_monitor_cfg(struct bus_monitor_cfg_para *res)
{
	(void)res;
}

static inline void bus_monitor_stop(void)
{
}
#endif
#endif
