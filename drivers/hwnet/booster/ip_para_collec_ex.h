/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the internal header file for the
 *              TCP/IP parameter collection module.
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#ifndef _IP_PARA_COLLEC_EX_H
#define _IP_PARA_COLLEC_EX_H

#include <linux/skbuff.h>
#include <net/tcp.h>

#include "netlink_handle.h"
#include "ip_para_collec.h"

/* initialization function for external modules */
msg_process *ip_para_collec_init(notify_event *fn);
void ip_para_collec_exit(void);
void udp_in_hook(struct sk_buff *skb, struct sock *sk);
void udp6_in_hook(struct sk_buff *skb, struct sock *sk);
void dec_sk_num_for_qoe(struct sock *sk, int new_state);
void update_ofo_rtt_for_qoe(struct sock *sk);
void update_ofo_tstamp_for_qoe(struct sock *sk);
void update_tcp_para_without_skb(struct sock *sk,
	unsigned int hook);
void update_dupack_num(struct sock *sk,
	bool is_dupack, unsigned int hook);
void update_stats_srtt(struct tcp_sock *tp, u32 *rtt);

#endif // _IP_PARA_COLLEC_EX_H
