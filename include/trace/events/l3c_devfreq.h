/*
 * l3c_devfreq.h
 *
 * L3cache devfreq
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#undef TRACE_SYSTEM
#define TRACE_SYSTEM l3c_devfreq

#if !defined(_TRACE_DSU_PCTRL_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_L3C_DEVFREQ_H

#include <linux/tracepoint.h>

TRACE_EVENT(l3c_devfreq_counter_info,
	TP_PROTO(unsigned long l3_count, unsigned long ba_count),
	TP_ARGS(l3_count, ba_count),
	TP_STRUCT__entry(
		__field(unsigned long, l3_count)
		__field(unsigned long, ba_count)
	),
	TP_fast_assign(
		__entry->l3_count = l3_count;
		__entry->ba_count = ba_count;
	),

	TP_printk("l3_cnt=%lu ba_cnt=%lu",
		  __entry->l3_count, __entry->ba_count)
);

TRACE_EVENT(l3c_devfreq_bw_info,
	TP_PROTO(unsigned long usec_delta, unsigned long cur_freq,
		 unsigned long l3c_bw, unsigned long hit_bw),
	TP_ARGS(usec_delta, cur_freq, l3c_bw, hit_bw),
	TP_STRUCT__entry(
		__field(unsigned long, usec_delta)
		__field(unsigned long, cur_freq)
		__field(unsigned long, l3c_bw)
		__field(unsigned long, hit_bw)
	),
	TP_fast_assign(
		__entry->usec_delta = usec_delta;
		__entry->cur_freq = cur_freq;
		__entry->l3c_bw = l3c_bw;
		__entry->hit_bw = hit_bw;
	),

	TP_printk("usec_delta=%lu cur_freq=%lu l3c_bw=%lu hit_bw=%lu",
		  __entry->usec_delta, __entry->cur_freq,
		  __entry->l3c_bw, __entry->hit_bw)
);

TRACE_EVENT(l3c_devfreq_target,
	TP_PROTO(unsigned long taget_freq),
	TP_ARGS(taget_freq),
	TP_STRUCT__entry(
		__field(unsigned long, taget_freq)
	),
	TP_fast_assign(
		__entry->taget_freq = taget_freq;
	),

	TP_printk("taget_freq=%lu",
		  __entry->taget_freq)
);

#endif /* _TRACE_DSU_PCTRL_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
