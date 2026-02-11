/*
  * netqos_sched.h
  *
  * NetQos schedule declaration
  *
  * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#ifndef _NETQOS_SCHED_H
#define _NETQOS_SCHED_H

#include <asm/param.h>
#include <net/sock.h>

void netqos_rcvwnd(struct sock *sk, uint32_t *pSize);
void netqos_sendrcv(struct sock *sk, int len);
int netqos_qdisc_band(struct sk_buff *skb, int band);
int tcp_is_low_priority(struct sock *sk);
int get_net_qos_level(struct task_struct *curTask);

extern int sysctl_netqos_debug;

#define NETQOS_DEBUG(fmt, arg...) \
	do { if (sysctl_netqos_debug) pr_info(fmt, ##arg); } while (0)

#endif
