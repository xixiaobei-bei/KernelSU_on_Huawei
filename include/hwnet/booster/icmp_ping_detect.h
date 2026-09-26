/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: internal header file for icmp ping detect
 * Create: 2019-10-09
 */

#ifndef _ICMP_PING_DETECT_H
#define _ICMP_PING_DETECT_H

#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter.h>
#include <linux/timer.h>
#include <uapi/linux/netfilter.h>

#include "netlink_handle.h"

#define STOP_DETECT 0
#define START_DETECT 1
#define MAX_ICMP_CMD_LEN 12
#define ICMP_PING_END 0
#define ICMP_PING_START 1
#define ICMP_DS_NET "rmnet"
#define ICMP_DS_NET_LEN 5
#define STOP_TIMER_INTERVAL (2 * HZ)

struct icmp_ping_detect_info {
	spinlock_t lock; // this context lock
	u16 ap_cmd; // 0:stop ping detect, 1:start ping detect
	notify_event *report_fn; // callback function
	struct timer_list timer; // report status if no ping pkt after 2s
};

/* Notification request issued by the upper layer is defined as: */
struct icmp_req_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 8
	u32 cmd_type; // 1:start ping detect, 0:stop
};

/* Each module sends the message request is defined as: */
struct icmp_res_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 8
	u32 result; // 1: ping proc, 0:no ping
};

msg_process *icmp_ping_detect_init(notify_event *fn);
void icmp_ping_process(struct req_msg_head *msg);
void icmp_ping_timer_expire(unsigned long data);
#endif // _ICMP_PING_DETECT_H
