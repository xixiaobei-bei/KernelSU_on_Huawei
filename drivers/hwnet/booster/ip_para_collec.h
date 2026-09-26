/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <uapi/linux/netfilter_ipv4.h>

#include "netlink_handle.h"

#define MAX_APP_LIST_LEN 8
#define NF_INET_BUFHOOK NF_INET_NUMHOOKS
#define NS_CONVERT_MS 1048576
#define BUF_MAX 10000
#define DS_NET "rmnet0"
#define DS_NET_SLAVE "rmnet3"
#define WIFI_NET "wlan0"
#define WIFI_NET_SLAVE "wlan1"
#define DS_NET_MTK "ccmni"
#define DS_NET_MTK_PREFIX_LEN 5
#define DS_NET_MTK_LEN 6
#define DS_NET_LEN 6
#define DS_NET_QCOM "rmnet_data"
#define DS_NET_QCOM_PREFIX_LEN 10
#define DS_NET_QCOM_LEN 11
#define WIFI_NET_LEN 5
#define NF_INET_UDP_IN_HOOK (NF_INET_NUMHOOKS + 1) // 6
#define NF_INET_TCP_DUPACK_IN_HOOK (NF_INET_NUMHOOKS + 2) // 7
#define NF_INET_NEW_SK_HOOK (NF_INET_NUMHOOKS + 3) // 8
#define NF_INET_DELETE_SK_HOOK (NF_INET_NUMHOOKS + 4) // 9
#define NF_INET_UPDATE_RTT_HOOK (NF_INET_NUMHOOKS + 5) // 10
#define NF_INET_RETRANS_TIMER_HOOK (NF_INET_NUMHOOKS + 6) // 11
#define NF_INET_OFO_HOOK (NF_INET_NUMHOOKS + 7) // 12
#define CHANNEL_NUM 4
#define IP_TCP_HEAD_LEN 40
#define TCP_PKT_LEN 1460
#define TCP_RTT_ARRAY_LEN 2
#define DNS_UID 1051
#define KEY_FLOW_UID 999
#define BACKGROUND_UID 0
#define DNS_PORT 53
#define CHANNEL_NO_DATA_MAX_TIME (5 * 60 * 1000) // ms
#define JIFFIES_MS (1000 / HZ)
#define MAX_U32_VALUE 2147483647
#define US_MS 1024
#define TIME_TEN_SEC 10000 // ms
#define MAX_RTT 10000 // ms
#define MAX_RTT_US 80000000 // max value of rtt
#define MAX_RTT_US_RETRANS 2400000 // max rtt of retrans scenarios(300ms)
#define MULTIPLE_OF_RTT 8
#define OFO_RTT_UPDATE_THRESHOLD_US 8000000 // 1000ms << 3
#define MAX_DELTA_US (500 * US_MS)
#define MAX_KEY_WORLD_LEN 24
#define MAX_KEY_FLOW_TYPE_NUM 4
#define DS_INTERFACE_NAME_ID 6

/* The structure that sets the app information is defined as: */
struct app_id {
	u32 pid; // The value 0 means does not filter
	u32 uid; // The value 0 means does not filter
};

union key_flow_field {
	u32 pid;
	char key_world[MAX_KEY_WORLD_LEN];
};

struct key_flow_info {
	u32 type;
	union key_flow_field field;
};

struct key_flow {
	u32 cnt;
	struct key_flow_info info[MAX_KEY_FLOW_TYPE_NUM];
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
	struct key_flow key_flow;
	struct app_id id[0]; // The array index depends on the CNT;
};

/* The message content is defined as: */
union notify_req {
	struct tcp_collect_req tcp_req;
};

/* TCPIP protocol stack parameter structure body */
struct tcp_res_info {
	u32 in_rts_pkt; // TCP packet drop retransmission number/s
	u32 in_pkt; // TCP downlink Cumulative number of packets/s
	u32 in_len; // TCP downlink Cumulative Data length/s
	u32 out_rts_pkt; // TCP uplink packet Drop retransmission number
	u32 out_pkt; // TCP uplink cumulative packets number/s
	u32 out_len; // TCP uplink Cumulative Data length/s
	u32 rtt; // TCP uplink Cumulative RTT value/s
	u32 out_all_pkt; // All uplink packets number
	u32 in_udp_pkt; // UDP downlink data packets/s
	u32 out_udp_pkt; // UDP data uplink packets/s
	u32 in_udp_len; // UDP downlink data len/s
	u32 out_udp_len; // UDP data uplink len/s
	int tcp_sk_num; // The value of socket num
	int sk_rtt_sum; // The summation of all socket rtt
	u32 tcp_syn_pkt;
	u32 tcp_fin_pkt;
	u32 in_dupack_pkt;
	u32 dev_id;
	u32 tcp_syn_rts_pkt;
};

/* TCPIP protocol stack parameter structure body */
struct tcp_res {
	u32 pid; // The value 0 means does not filter
	u32 uid; // The value 0 means does not filter
	struct tcp_res_info info[CHANNEL_NUM];
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
	union notify_req req;
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
	u32 report_tstamp;
	u32 fore_app_uid;
	struct key_flow flow;
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

enum net_dev_id {
	DS_NET_ID = 0,
	DS_NET_SLAVE_ID,
	WIFI_NET_ID,
	WIFI_NET_SLAVE_ID,
};

enum flow_type {
	DEFALUT_FLOW = 0,
	VIDEO_FLOW,
	IMAGE_FLOW,
	KEY_PROCESSES_FLOW,
	FLOW_TYPE_BUTT,
};

/* device id and modem id update */
struct link_property_info {
	u32 modem_id;
	u32 interface_id; // numer of ccmni1, ccmni2...., rmnet0, rmnet1
};

/* Notification request issued by the upper layer is defined as: */
struct link_property_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	struct link_property_info property;;
};

#endif // _IP_PARA_COLLEC_H
