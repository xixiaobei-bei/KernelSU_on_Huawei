#undef TRACE_SYSTEM
#define TRACE_SYSTEM l3p_karma

#if !defined(_TRACE_L3P_KARMA_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_L3P_KARMA_H

#include <linux/tracepoint.h>

TRACE_EVENT(check_from_slp_to_en_temp,/* [false alarm]:原生宏定义 */
	TP_PROTO(unsigned int keep_sleep),
	TP_ARGS(keep_sleep),
	TP_STRUCT__entry(
		__field(unsigned int, keep_sleep)
	),
	TP_fast_assign(
		__entry->keep_sleep = keep_sleep;

	),

	TP_printk("[keep_sleep]=%u",
		  __entry->keep_sleep)
);

TRACE_EVENT(decide_from_en_temp,/* [false alarm]:原生宏定义 */
	TP_PROTO(unsigned int quick_sleep),
	TP_ARGS(quick_sleep),
	TP_STRUCT__entry(
		__field(unsigned int, quick_sleep)
	),
	TP_fast_assign(
		__entry->quick_sleep = quick_sleep;

	),

	TP_printk("[quick_sleep]=%u",
		  __entry->quick_sleep)
);


TRACE_EVENT(check_from_en_to_slp,/* [false alarm]:原生宏定义 */
	TP_PROTO(unsigned int en_to_slp),
	TP_ARGS(en_to_slp),
	TP_STRUCT__entry(
		__field(unsigned int, en_to_slp)
	),
	TP_fast_assign(
		__entry->en_to_slp = en_to_slp;
	),

	TP_printk("[en_to_slp]=%u",
		  __entry->en_to_slp)
);


TRACE_EVENT(l3p_karma_algo_info,/* [false alarm]:原生宏定义 */
	TP_PROTO(unsigned int dsu_hit_rate, unsigned int dsu_bw, unsigned int rep_eff, unsigned int dcp_eff, unsigned int tot_eff,
		 unsigned int hzd_rate, unsigned int acp_hit_rate),
	TP_ARGS(dsu_hit_rate, dsu_bw, rep_eff, dcp_eff, tot_eff, hzd_rate, acp_hit_rate),
	TP_STRUCT__entry(
		__field(unsigned int, dsu_hit_rate)
		__field(unsigned int, dsu_bw)
		__field(unsigned int, rep_eff)
		__field(unsigned int, dcp_eff)
		__field(unsigned int, tot_eff)
		__field(unsigned int, hzd_rate)
		__field(unsigned int, acp_hit_rate)
	),
	TP_fast_assign(
		__entry->dsu_hit_rate = dsu_hit_rate;
		__entry->dsu_bw = dsu_bw;
		__entry->rep_eff = rep_eff;
		__entry->dcp_eff = dcp_eff;
		__entry->tot_eff = tot_eff;
		__entry->hzd_rate = hzd_rate;
		__entry->acp_hit_rate = acp_hit_rate;
	),

	TP_printk("dsu_hit_rate=%u dsu_bw=%u rep_eff=%u dcp_eff=%u tot_eff=%u hzd_rate=%u acp_hit_rate=%u",
		  __entry->dsu_hit_rate, __entry->dsu_bw, __entry->rep_eff, __entry->dcp_eff, __entry->tot_eff,
		  __entry->hzd_rate, __entry->acp_hit_rate)
);

TRACE_EVENT(l3p_karma_freq_trans,/* [false alarm]:原生宏定义 */
	TP_PROTO(unsigned long cur_freq, unsigned long next_freq),
	TP_ARGS(cur_freq, next_freq),
	TP_STRUCT__entry(
		__field(unsigned long, cur_freq)
		__field(unsigned long, next_freq)
	),
	TP_fast_assign(
		__entry->cur_freq = cur_freq;
		__entry->next_freq = next_freq;
	),

	TP_printk("cur_freq=%lu, next_freq=%lu",
		  __entry->cur_freq, __entry->next_freq)
);

TRACE_EVENT(l3p_karma_target,/* [false alarm]:原生宏定义 */
	TP_PROTO(unsigned long target_freq),
	TP_ARGS(target_freq),
	TP_STRUCT__entry(
		__field(unsigned long, target_freq)
	),
	TP_fast_assign(
		__entry->target_freq = target_freq;
	),

	TP_printk("target_freq=%lu",
		  __entry->target_freq)
);

TRACE_EVENT(l3p_karma_acp_callback,/* [false alarm]:原生宏定义 */
	TP_PROTO(const char *action, unsigned long mode),
	TP_ARGS(action, mode),
	TP_STRUCT__entry(
		__string(action, action)
		__field(unsigned long, mode)
	),
	TP_fast_assign(
		__assign_str(action, action);
		__entry->mode = mode;
	),

	TP_printk("action is %s, mode=%lu",
		  __get_str(action), __entry->mode)
);








#endif /* _TRACE_L3P_KARMA_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
