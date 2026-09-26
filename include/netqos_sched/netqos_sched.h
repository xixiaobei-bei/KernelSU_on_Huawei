/*
 * netqos_sched.h
 *
 * Net qos schedule declaration
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

#ifndef NETQOS_SCHED_H
#define NETQOS_SCHED_H

#include <asm/param.h>
#include <net/sock.h>

void netqos_rcvwnd(struct sock *sk, uint32_t *size);
void netqos_sendrcv(struct sock *sk, int len, bool is_recv);
int netqos_qdisc_band(const struct sk_buff *skb, int band);
int tcp_is_low_priority(struct sock *sk);
int get_net_qos_level(struct task_struct *cur_task);

extern int sysctl_netqos_debug;

#define net_qos_debug(fmt, arg...) \
do { \
	if (sysctl_netqos_debug) \
		pr_info(fmt, ##arg); \
} while (0)

#endif // NETQOS_SCHED_H
