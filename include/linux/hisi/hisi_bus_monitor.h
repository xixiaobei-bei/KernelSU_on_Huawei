/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: bus_monitor.h
 * Author: tangshuo
 * Create: 2019-09-09
 */
#ifndef HISI_BUS_MONITOR_H
#define HISI_BUS_MONITOR_H

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

#ifdef CONFIG_HISI_BUS_MONITOR
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
