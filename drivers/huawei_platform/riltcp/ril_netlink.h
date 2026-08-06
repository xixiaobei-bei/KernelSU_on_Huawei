/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Netlink head file.
 * Author: zhuweichen@huawei.com
 * Create: 2017-04-10
 */

#ifndef _RIL_NETLINK_H
#define _RIL_NETLINK_H

#define RIL_NETLINK_INIT 1
#define RIL_NETLINK_EXIT 0
#define MAX_IF_NAME 16

enum {
	NETLINK_TCP_REG = 0,
	NETLINK_TCP_RST_MSG,
	NETLINK_KNL_RLT_MSG,
	NETLINK_TCP_UNREG
};

typedef struct msg_ril_to_knl {
	char if_name[MAX_IF_NAME];
} ril_to_knl_msg;

void notify_chr_thread_to_send_msg(unsigned int dst_addr,
	unsigned int src_addr, struct sock *sk);
int chr_notify_event(int event, int pid, unsigned int src_addr,
	struct http_return *prtn);

#endif /* _RIL_NETLINK_H */
