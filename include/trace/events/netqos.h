/*
 * netqos.h
 *
 * Net qos trace declaration
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
#undef TRACE_SYSTEM
#define TRACE_SYSTEM netqos

#if !defined(TRACE_NETQOS_H) || defined(TRACE_HEADER_MULTI_READ)
#define TRACE_NETQOS_H

#include <linux/tracepoint.h>
#include <net/inet_sock.h>
#include <net/sock.h>

bool netqos_trace_add_check(struct sock *sk, int len,
	bool is_recv, unsigned long cur_time);
void netqos_trace_assign_set(struct sock *sk, int *tx,
	int *rx, unsigned long cur_time);

TRACE_EVENT_CONDITION(netqos_trx,

	TP_PROTO(struct sock *sk, int len, bool is_recv,
		unsigned long cur_time),

	TP_ARGS(sk, len, is_recv, cur_time),

	TP_CONDITION(netqos_trace_add_check(sk, len, is_recv, cur_time)),

	TP_STRUCT__entry(
		__field(u16, sport)
		__field(int, tx)
		__field(int, rx)
		),

	TP_fast_assign(
		__entry->sport = inet_sk(sk)->inet_num;
		netqos_trace_assign_set(sk, &__entry->tx,
			&__entry->rx, cur_time);
		),

	TP_printk("port=%u tx=%d rx=%d",
		__entry->sport,
		__entry->tx,
		__entry->rx)
);

#endif // TRACE_NETQOS_H

/* This part must be outside protection */
#include <trace/define_trace.h>
