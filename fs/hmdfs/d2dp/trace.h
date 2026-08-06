/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D tracing system. Used for protocol diagnostic and performance tuning
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM d2dp

#if !defined(_D2D_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _D2D_TRACE_H

#include <linux/tracepoint.h>

#include "headers.h"

/* General event template for D2D header related events */
DECLARE_EVENT_CLASS(d2dp_hdr_template,
	TP_PROTO(struct d2d_header *hdr),
	TP_ARGS(hdr),

	TP_STRUCT__entry(
		__field(__u16, flags)
		__field(__u32, seq_id)
		__field(__u32, data_len)
		__field(__u64, packet_id)
	),

	TP_fast_assign(
		__entry->flags     = hdr->flags;
		__entry->seq_id    = hdr->seq_id;
		__entry->data_len  = hdr->data_len;
		__entry->packet_id = hdr->packet_id;
	),

	TP_printk("flags=%s seq:%u len:%u id:%llu",
		  __print_flags(__entry->flags, "",
				{ D2D_FLAG_ACK,     "A" },
				{ D2D_FLAG_SUSPEND, "S" }),
		  __entry->seq_id, __entry->data_len, __entry->packet_id)
);

DEFINE_EVENT(d2dp_hdr_template,
	     d2dp_ack_recv,
	     TP_PROTO(struct d2d_header *hdr),
	     TP_ARGS(hdr));

DEFINE_EVENT(d2dp_hdr_template,
	     d2dp_ack_send,
	     TP_PROTO(struct d2d_header *hdr),
	     TP_ARGS(hdr));

/* D2DP timer event template */
DECLARE_EVENT_CLASS(d2dp_timer_template,
	TP_PROTO(s64 time, int value),
	TP_ARGS(time, value),

	TP_STRUCT__entry(
		__field(__s64, time)
		__field(int,   value)
	),

	TP_fast_assign(
		__entry->time  = time;
		__entry->value = value;
	),

	TP_printk("time:%lld[ns] value:%d", __entry->time, __entry->value)
);

DEFINE_EVENT(d2dp_timer_template,
	     d2dp_ack_timer,
	     TP_PROTO(s64 time, int value),
	     TP_ARGS(time, value));

DEFINE_EVENT(d2dp_timer_template,
	     d2dp_rto_timer,
	     TP_PROTO(s64 time, int value),
	     TP_ARGS(time, value));

#endif /* _D2D_TRACE_H */

/* Make traces from local file and not from trace/events directory */
#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE trace
#include <trace/define_trace.h>
