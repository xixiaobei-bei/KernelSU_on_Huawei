/*
 * hw_packet_filter_bypass.h
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#ifndef HW_PACKET_FILTER_BYPASS_H
#define HW_PACKET_FILTER_BYPASS_H

#include <linux/netdevice.h>

#include "netlink_handle.h"

enum hw_pfb_type {
	PASS = 0,
	DROP = 1,
	IGNORE = 2,
};

enum hw_pfb_hooks {
	HW_PFB_INET_LOCAL_OUT, // 0
	HW_PFB_INET_POST_ROUTING,
	HW_PFB_INET_BPF_EGRESS,
	HW_PFB_INET_IP_XMIT,
	HW_PFB_INET_DEV_XMIT,
	HW_PFB_INET_PRE_ROUTING, // 5
	HW_PFB_INET_LOCAL_IN,
	HW_PFB_INET_FORWARD,
	HW_PFB_INET_BPF_INGRESS,
	HW_PFB_INET_IP_RCV,
	HW_PFB_INET_IP_DELIVERY, // 10
	HW_PFB_INET6_LOCAL_OUT,
	HW_PFB_INET6_POST_ROUTING,
	HW_PFB_INET6_BPF_EGRESS,
	HW_PFB_INET6_IP_XMIT,
	HW_PFB_INET6_DEV_XMIT, // 15
	HW_PFB_INET6_PRE_ROUTING,
	HW_PFB_INET6_LOCAL_IN,
	HW_PFB_INET6_FORWARD,
	HW_PFB_INET6_BPF_INGRESS,
	HW_PFB_INET6_IP_RCV, // 20
	HW_PFB_INET6_IP_DELIVERY,
	HW_PFB_HOOK_ICMP,
	HW_PFB_HOOK_INVALID,
};

bool hw_bypass_skb(int af, int hook, const struct sock *sk,
	struct sk_buff *skb, struct net_device *in,
	struct net_device *out, int pass);
int hw_translate_hook_num(int af, int hook);
int hw_translate_verdict(u32 verdict);
bool hw_hook_bypass_skb(int af , int hook, struct sk_buff *skb);
bool is_ds_rnic(const struct net_device *dev);

msg_process* hw_packet_filter_bypass_init(notify_event *notify);

#endif
