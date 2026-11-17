/*
 * network_qos.c
 *
 * Network Qos schedule implementation
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

#include "netqos_sched/netqos_sched.h"

#include <linux/sched.h>
#include <linux/tcp.h>
#include <net/inet_sock.h>
#include <net/sock.h>

#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#define CREATE_TRACE_POINTS
#include <trace/events/netqos.h>

int sysctl_netqos_switch __read_mostly;
int sysctl_netqos_debug __read_mostly;
int sysctl_netqos_limit __read_mostly;
int sysctl_netqos_period __read_mostly = 200;
static unsigned long g_high_prio_flux_time;

enum {
	NETQOS_PRIO_LOW,
	NETQOS_PRIO_MID,
	NETQOS_PRIO_HIGH,
};

#define TIME_MS_PER_JIFFY (1000 / HZ)

#define MAX_STAT_TIME_JIFFIES (sysctl_netqos_period / TIME_MS_PER_JIFFY)
#define RCVWND_LEVEL_TIME_JIFFIES \
	(sysctl_netqos_period / 4 / TIME_MS_PER_JIFFY)

#define MAX_UPDATE_PRIO_JIFFIES (500 / TIME_MS_PER_JIFFY)
#define NETQOS_TRACE_INTERVAL (32 / TIME_MS_PER_JIFFY)

#define HIGH_PRIO_WIFI_MARK 0x5a
#define MIN_RCVWND_SIZE (4 * 1460)

int get_net_qos_level(struct task_struct *cur_task)
{
	int level = NETQOS_PRIO_MID;
#ifdef CONFIG_HW_QOS_THREAD
	int task_qos_level = get_task_qos(cur_task);

	switch (task_qos_level) {
	case VALUE_QOS_LOW:
		level = NETQOS_PRIO_LOW;
		break;
	case VALUE_QOS_NORMAL:
		level = NETQOS_PRIO_MID;
		break;
	case VALUE_QOS_HIGH:
	case VALUE_QOS_CRITICAL:
		level = NETQOS_PRIO_HIGH;
		break;
	default:
		net_qos_debug("netqos unknown task qos value %d uid %d\n",
			     task_qos_level, current_uid().val);
		break;
	}
#endif

	return level;
}

static int get_priority(struct sock *sk, struct task_struct *cur_task,
	unsigned long cur_time)
{
	if (unlikely(!sk))
		return NETQOS_PRIO_MID;

	if ((sk->sk_netqos_level != -1) &&
		(cur_time - sk->sk_netqos_time < MAX_UPDATE_PRIO_JIFFIES))
		return sk->sk_netqos_level;

	if (unlikely(!cur_task))
		return NETQOS_PRIO_MID;

	sk->sk_netqos_level = get_net_qos_level(cur_task);
	sk->sk_netqos_time = cur_time;
	net_qos_debug("netqos %s sk_netqos_level %d uid %d\n",
		__func__, sk->sk_netqos_level, current_uid().val);
	return sk->sk_netqos_level;
}

static inline void mark_high_prio_fluxing(unsigned long cur_time)
{
	g_high_prio_flux_time = cur_time;
}

static inline bool is_high_prio_fluxing(unsigned long cur_time)
{
	return g_high_prio_flux_time + MAX_STAT_TIME_JIFFIES > cur_time;
}

static void limit_speed(unsigned long cur_time)
{
	int sleeptime;

	if (!sysctl_netqos_switch || !sysctl_netqos_limit)
		return;

	sleeptime = (MAX_STAT_TIME_JIFFIES -
		(cur_time - g_high_prio_flux_time)) * TIME_MS_PER_JIFFY;
	if ((sleeptime > 0) && (sleeptime <= sysctl_netqos_period)) {
		net_qos_debug("netqos %s begin sleep %d for uid %d\n",
			__func__, sleeptime, current_uid().val);
		msleep_interruptible(sleeptime);
		net_qos_debug("netqos %s end sleep %d for uid %d\n",
			__func__, sleeptime, current_uid().val);
	}
}

void netqos_sendrcv(struct sock *sk, int len, bool is_recv)
{
	int level;
	unsigned long cur_time;

	if (!sysctl_netqos_switch)
		return;

	cur_time = jiffies;
	trace_netqos_trx(sk, len, is_recv, cur_time);
	level = get_priority(sk, current, cur_time);
	if (level == NETQOS_PRIO_HIGH)
		mark_high_prio_fluxing(cur_time);
	else if ((level == NETQOS_PRIO_LOW) && is_high_prio_fluxing(cur_time))
		limit_speed(cur_time);
	else
		return;
}

int tcp_is_low_priority(struct sock *sk)
{
	return (get_priority(sk, NULL, jiffies) == NETQOS_PRIO_LOW);
}

static int check_netqos_rcvwnd_params(const struct tcp_sock *tp,
	const uint32_t *size)
{
	return (!sysctl_netqos_switch || !sysctl_netqos_limit || !tp || !size);
}

void netqos_rcvwnd(struct sock *sk, uint32_t *size)
{
	struct tcp_sock *tp = tcp_sk(sk);
	unsigned long cur_time;
	unsigned long time_aligned;
	u32 rcv_wnd_modify;

	if (check_netqos_rcvwnd_params(tp, size))
		return;

	if ((tp->rcv_rate.rcv_wnd == 0) || (tp->rcv_rate.rcv_wnd == ~0U) ||
		(*size <= MIN_RCVWND_SIZE))
		return;

	cur_time = jiffies;
	if ((get_priority(sk, NULL, cur_time) == NETQOS_PRIO_LOW) &&
		is_high_prio_fluxing(cur_time) &&
		(MAX_STAT_TIME_JIFFIES != 0)) {
		time_aligned = ((cur_time - g_high_prio_flux_time) /
			RCVWND_LEVEL_TIME_JIFFIES + 1) *
			RCVWND_LEVEL_TIME_JIFFIES;

		if (time_aligned > MAX_STAT_TIME_JIFFIES)
			time_aligned = MAX_STAT_TIME_JIFFIES;

		rcv_wnd_modify = (u32)(time_aligned * tp->rcv_rate.rcv_wnd /
			MAX_STAT_TIME_JIFFIES);
		if (rcv_wnd_modify < MIN_RCVWND_SIZE)
			rcv_wnd_modify = MIN_RCVWND_SIZE;

		net_qos_debug("netqos origin %d, rcv_wnd %u modify %u\n",
			*size, tp->rcv_rate.rcv_wnd, rcv_wnd_modify);
		*size = min_t(uint32_t, rcv_wnd_modify, *size);
	}
}

int netqos_qdisc_band(const struct sk_buff *skb, int orig_band)
{
	int level;

	if (!sysctl_netqos_switch || !skb)
		return orig_band;

	level = get_priority(skb->sk, NULL, jiffies);
	if (level == NETQOS_PRIO_HIGH) {
		net_qos_debug("netqos qdisc band update\n");
		return 0;
	}

	return orig_band;
}

bool netqos_trace_add_check(struct sock *sk, int len, bool is_recv,
	unsigned long cur_time)
{
	unsigned int *trx = NULL;

	if (unlikely(!sk || (len <= 0)))
		return false;

	trx = is_recv ? &sk->sk_netqos_rx : &sk->sk_netqos_tx;
	*trx += len;
	return cur_time - sk->sk_netqos_ttime >= NETQOS_TRACE_INTERVAL;
}

void netqos_trace_assign_set(struct sock *sk, int *tx, int *rx,
	unsigned long cur_time)
{
	if (unlikely(!sk || !tx || !rx))
		return;

	*tx = sk->sk_netqos_tx;
	*rx = sk->sk_netqos_rx;
	sk->sk_netqos_tx = 0;
	sk->sk_netqos_rx = 0;
	sk->sk_netqos_ttime = cur_time;
}
