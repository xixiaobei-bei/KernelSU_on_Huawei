/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the internal header file for
 *              the network slice route module.
 * Author: yuxiaoliang@huawei.com
 * Create: 2019-11-20
 */

#ifndef _NETWORK_SLICE_ROUTE_H
#define _NETWORK_SLICE_ROUTE_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/module.h>
#include <net/ipv6.h>
#include <uapi/linux/in6.h>
#include "netlink_handle.h"

#define MAX_PROTOCOL_NUM 32
#define MAX_REMOTE_PORT_RANGE_NUM 32

struct uid_list {
	struct list_head list;
	u32 uid;
};

struct port_range {
	__be16 start;
	__be16 end;
};

struct remote_ipv4_addr {
	__be32 addr;
	__be32 mask;
};

struct remote_ipv6_addr {
	struct in6_addr addr;
	u32 prefix_len;
};

struct remote_address {
	struct list_head list;
	u32 ai_family;
	union {
		struct remote_ipv6_addr ip6_addr;
		struct remote_ipv4_addr ip4_addr;
	} addr_u;
};

struct slice_route_rule {
	struct list_head list;
	u32 net_id;
	u32 priority;
	u8 uid_num;
	u8 protocol_num;
	u8 port_num;
	u8 addr_num;
	struct list_head uid_list; // can change, add & del
	u8 protocol[MAX_PROTOCOL_NUM];
	struct port_range port[MAX_REMOTE_PORT_RANGE_NUM];
	struct list_head addr_list; // can change, only add
	unsigned long stamp;
};

void bind_process_to_slice(struct req_msg_head *msg);
void unbind_process_to_slice(struct req_msg_head *msg);
int slice_rules_lookup(struct sock *sk, struct sockaddr *uaddr, u8 protocol);

#endif
