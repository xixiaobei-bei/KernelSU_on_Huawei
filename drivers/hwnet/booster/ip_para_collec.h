/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: This file is the internal header file for the
 *              TCP/IP parameter collection module.
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#ifndef _IP_PARA_COLLEC_H
#define _IP_PARA_COLLEC_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter.h>
#include <linux/timer.h>
#include <uapi/linux/netfilter.h>

#include "netlink_handle.h"

#define MAX_REQ_DATA_LEN 2048
#define MAX_APP_LIST_LEN 8
#define JIFFIES_MS (1000/HZ)
#define MAX_RTT 10000
#define NF_INET_BUFHOOK NF_INET_NUMHOOKS
#define NS_CONVERT_MS 1048576
#define BUF_MAX 10000
#define US_MS 1024
#define DS_NET "rmnet0"
#define DS_NET_SLAVE "rmnet3"
#define DS_NET_LEN 6
#define MAX_U32_VALUE 2147483647

/* The structure that sets the app information is defined as: */
struct app_id {
	u32 pid; // The value 0 means does not filter
	u32 uid; // The value 0 means does not filter
};

/* The foreground app infomation is defined as: */
struct tcp_collect_req {
	/*
	 * reqFlag maens whether to turn on information collection,
	 * 1 means to turn on background filtering,
	 * 2 to open app filtering,
	 * 3 to open app and background filtering at the same time,
	 * and other delegates to turn off and not parse appinfo.
	 */
	u16 req_flag;
	u16 cnt; // Number of parameter groups issued, lower than 8
	u32 rpt_cycle; // Escalation cycle, Unit ms
	struct app_id id[0]; // The array index depends on the CNT;
};

/* The message content is defined as: */
union notify_req {
	struct tcp_collect_req tcp_req;
};

/* TCPIP protocol stack parameter structure body */
struct tcp_res {
	u32 pid; // The value 0 means does not filter
	u32 uid; // The value 0 means does not filter
	u32 in_rts_pkt; // TCP packet drop retransmission number/s
	u32 in_pkt; // TCP downlink Cumulative number of packets/s
	u32 in_len; // TCP downlink Cumulative Data length/s
	u32 out_rts_pkt; // TCP uplink packet Drop retransmission number
	u32 out_pkt; // TCP uplink cumulative packets number/s
	u32 out_len; // TCP uplink Cumulative Data length/s
	u32 rtt; // TCP uplink Cumulative RTT value/s
	u32 drop_pkt; // Uplink cumulative Protocol Stack Drop pack number
	u32 in_udp_pkt; // UDP downlink data packets/s
	u32 out_udp_pkt; // UDP data uplink packets/s
	u32 in_udp_len; // UDP downlink data len/s
	u32 out_udp_len; // UDP data uplink len/s
	u32 tcp_buf; // TCP protocol stack buf time accumulation/s
	u32 udp_buf; // UDP protocol stack buf time accumulation/s
	u32 tcp_syn_pkt;
};

/* The structure of information acquisition is defined as: */
struct tcp_collect_res {
	u16 cnt; // Number of groups of information collected
	u16 sync; // sync num
	struct tcp_res res[0]; // The array index depends on the cnt
};

/* the structure of collection from the TCPIP protocol stack is defined as: */
union sent_res {
	struct tcp_collect_res tcp;
};

/* Notification request issued by the upper layer is defined as: */
struct req_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	union notify_req  req;
};

/* Each module sends the message request is defined as: */
struct res_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	union sent_res res;
};

/* Statistic table items for single CPU and apk */
struct app_list {
	struct tcp_res stat;
	struct list_head head;
};

/* Statistical list head of single CPU */
struct app_stat {
	spinlock_t lock;
	struct list_head head;
};

/* ip stack paramter collect context struct */
struct app_ctx {
	spinlock_t lock; // this context lock
	u16 cnt; // app count for app_stat
	u32 jcycle; // timer cycle by jiffies, set by upper.
	struct app_stat *cur; // cpus private statistics context parameter
	notify_event *fn; // recv notification interface callback function.
	struct timer_list timer; // report timer for statistics
};

struct nf_hook_state_ex {
	struct nf_hook_state state;
	u32 buf;
};

enum app_mach_type {
	BACKGROUND = 1,
	FORGROUND_MACH,
	FORGROUND_UNMACH,
	APP_MACH_BUTT,
};

enum rtn_type {
	RTN_SUCC = 0,
	CPU_IDX_ERR,
	HOOK_STAT_ERR,
	SK_NULL_ERR,
	NET_DEV_ERR,
};

enum req_flag {
	BACKGROUND_STAT = 0x01,
	FORFROUND_STAT = 0x02,
	RESTART_SYNC = 0x04,
};

enum res_msg_type {
	APP_QOE_SYNC_RPT = 0,
	UPDATE_APP_INFO_RPT,
};
#endif // _IP_PARA_COLLEC_H
