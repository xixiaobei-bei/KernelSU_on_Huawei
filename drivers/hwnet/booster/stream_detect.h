/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file is the internal header file for the
 *              stream detect module.
 * Author: tongxilin@huawei.com
 * Create: 2020-03-09
 */

#ifndef _STREAM_DETECT_H
#define _STREAM_DETECT_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <uapi/linux/in6.h>
#include <uapi/linux/netfilter.h>
#include <net/ipv6.h>

#include "netlink_handle.h"
#include "ip_para_collec_ex.h"

#define MAX_STREAM_LIST_LEN 10

// loop ip addr:127.0.0.1
#define LOOP_IP ((127 << 24) + (0 << 16) + (0 << 8) + 1)

// HTTP response associated with video character string
#define CONTENT_TYPE_VIDEO1 "Content-Type: video"
#define CONTENT_TYPE_VIDEO2 "CONTENT-TYPE: video"
#define CONTENT_TYPE_LEN 18
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_LEN 16
#define DECIMAL_BASE 10
#define CONTENT_TYPE_IMAGE1 "Content-Type: image"
#define CONTENT_TYPE_IMAGE2 "CONTENT-TYPE: image"

#define MIN_HTTP_LEN 30
#define MAX_HTTP_LEN 1500

// TCP port for HTTP
#define VIDEO_HTTP_PORT 80

// Data service string and length
#define VIDEO_DS_NET "rmnet0"

// Maximum search http string length, can cause single packet
// processing Siyanyo 3us, maximum bandwidth about 4Gbps
#define MAX_DATA_LEN 600

// Maximum ASCII value
#define MAX_ASCII 0x7f
#define ASCII_CR 0x0d
#define ASCII_LF 0x0a
#define STR_HTTP "HTTP"
#define STR_HTTP_LEN 4

// The starting point of the return code for http
// HTTP\1.0 200OK ***
#define VIDEO_HTTP_ACK_FROM_START 9

enum HOOK_TYPE {
	HOOK_IN = 0,
	HOOK_OUT,
};

enum stream_type {
	DEFALUT = 0,
	VEDIO,
	IMAGE,
	TYPE_BUTT,
};

struct stream_req_msg {
	u16 type;
	u16 len;
	u32 uid;
	u32 stream_cnt;
	u32 stream_type;
	u32 report_expires;
	u32 list_expires;
	u32 res;
};

// The structure that sets the app information is defined as:
struct stream_info {
	u32 uid;
	u32 protocol;
	u32 family;
	u32 dest_port;
	u32 src_port;
	union {
		struct in6_addr ip6_addr;
		__be32 ip4_addr;
	} dest_addr;
	union {
		struct in6_addr ip6_addr;
		__be32 ip4_addr;
	} src_addr;
};

struct stream_res {
	struct stream_info info;
	u32 in_pkt;
	u32 in_len;
	u32 in_rts_pkt;
	u32 out_pkt;
	u32 out_len;
	u32 out_rts_pkt;
	u32 out_dupack_pkt;
	u32 rtt;
	u32 tcp_syn_pkt;
};

// The structure of information acquisition is defined as:
struct stream_collect_res {
	u32 cnt;
	struct stream_res stream[MAX_STREAM_LIST_LEN];
};

struct stream_res_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	struct stream_collect_res res;
};

struct stream_stat {
	spinlock_t lock;
	u32 cnt;
	struct list_head head;
};

struct stream_list {
	struct stream_res stat;
	struct list_head head;
};

// context struct
struct stream_ctx {
	spinlock_t lock; // this context lock
	u32 uid;
	u32 stream_cnt; // stream count for app_stat
	u32 stream_type; // stream count for app_stat
	u32 list_expires; // timer cycle by jiffies, set by upper.
	u32 report_expires; // timer cycle by jiffies, set by upper.
	struct stream_stat *cur; // cpus private statistics context parameter
	notify_event *fn; // recv notification interface callback function.
	struct timer_list list_timer; // report timer for statistics
	struct timer_list report_timer; // report timer for statistics
};

msg_process *stream_detect_init(notify_event *fn);
void stream_process_hook_out(struct sk_buff *skb, u16 family);
void stream_process_hook_in(struct sk_buff *skb, u16 family);
bool is_target_stream(struct sock *sk, u8 *str, u32 len, u32 stream_type, struct stream_info info);

#endif
