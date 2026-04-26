/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to manage network slice related function
 * Author: yuxiaoliang@huawei.com
 * Create: 2019-06-20
 */

#include "network_slice_management.h"

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <log/hw_log.h>
#include "network_slice_route.h"

#define assign_byte(p, val) (*(s8 *)(p) = (val))
#define assign_short(p, val) (*(s16 *)(p) = (val))
#define assign_int(p, val) (*(s32 *)(p) = (val))
#define skip_byte(p, num) ((p) = (p) + (num))

#define NOTIFY_BUF_LEN 512
#define IPV6_ADDR_LEN 16

#undef HWLOG_TAG
#define HWLOG_TAG slice_management
HWLOG_REGIST();

static notify_event *notifier = NULL;

static volatile int g_enable_rpt = 0;

static void ip_para_report_control(struct req_msg_head *msg)
{
	struct ip_para_rpt_ctrl_req *req = (struct ip_para_rpt_ctrl_req *)msg;

	if (msg->len < sizeof(struct ip_para_rpt_ctrl_req))
		return;

	g_enable_rpt = req->enable;
	hwlog_info("%s: g_enable_rpt = %d\n", __func__, g_enable_rpt);
}

static void ipv4_para_report(struct sk_buff *skb)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;
	struct sock *sk = NULL;
	const struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;

	sk = skb_to_full_sk(skb);

	iph = ip_hdr(skb);
	if (iph == NULL) {
		hwlog_err("%s:iph == NULL\n", __func__);
		return;
	}

	// type
	assign_short(p, SLICE_IP_PARA_RPT);
	skip_byte(p, sizeof(s16));

	// len = 2B type + 2B len + 4B uid + 4B ipv4 addr + 2B port + 1B protocol
	assign_short(p, 15);
	skip_byte(p, sizeof(s16));

	// uid
	assign_int(p, sk->sk_uid.val);
	skip_byte(p, sizeof(int));

	// ipv4 addr
	assign_int(p, iph->daddr);
	skip_byte(p, sizeof(int));

	// port
	if (iph->protocol == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		if (tcph == NULL) {
			hwlog_err("%s:tcph == NULL\n", __func__);
			return;
		}
		assign_short(p, tcph->dest);
	} else if (iph->protocol == IPPROTO_UDP) {
		udph = udp_hdr(skb);
		if (udph == NULL) {
			hwlog_err("%s:udph == NULL\n", __func__);
			return;
		}
		assign_short(p, udph->dest);
	} else {
		return;
	}
	skip_byte(p, sizeof(s16));

	// protocol id
	assign_byte(p, iph->protocol);
	skip_byte(p, sizeof(u8));

	// mark ip has reported for this sock
	sk->sk_slice_reported = 1;
	notifier((struct res_msg_head *)event);
}

static void ipv6_para_report(struct sk_buff *skb)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;
	struct sock *sk = NULL;
	const struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;

	sk = skb_to_full_sk(skb);

	ip6h = ipv6_hdr(skb);
	if (ip6h == NULL) {
		hwlog_err("%s:ip6h == NULL\n", __func__);
		return;
	}

	// type
	assign_short(p, SLICE_IP_PARA_RPT);
	skip_byte(p, sizeof(s16));

	// len = 2B type + 2B len + 4B uid + 16B ipv6 addr + 2B port + 1B protocol
	assign_short(p, 27);
	skip_byte(p, sizeof(s16));

	// uid
	assign_int(p, sk->sk_uid.val);
	skip_byte(p, sizeof(int));

	// ipv6 addr
	memcpy(p, &ip6h->daddr.s6_addr, IPV6_ADDR_LEN);
	skip_byte(p, IPV6_ADDR_LEN);

	// port
	if (ip6h->nexthdr == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		if (tcph == NULL) {
			hwlog_err("%s:tcph == NULL\n", __func__);
			return;
		}
		assign_short(p, tcph->dest);
	} else if (ip6h->nexthdr == IPPROTO_UDP) {
		udph = udp_hdr(skb);
		if (udph == NULL) {
			hwlog_err("%s:udph == NULL\n", __func__);
			return;
		}
		assign_short(p, udph->dest);
	} else {
		return;
	}
	skip_byte(p, sizeof(s16));

	// protocol id
	assign_byte(p, ip6h->nexthdr);
	skip_byte(p, sizeof(u8));

	// mark ip has reported for this sock
	sk->sk_slice_reported = 1;
	notifier((struct res_msg_head *)event);
}


void slice_ip_para_report(struct sk_buff *skb, unsigned short family)
{
	struct sock *sk = NULL;

	if (g_enable_rpt == 0)
		return;

	if (skb == NULL) {
		hwlog_err("%s:skb == NULL\n", __func__);
		return;
	}

	sk = skb_to_full_sk(skb);
	if (sk == NULL) {
		hwlog_err("%s:sock == NULL\n", __func__);
		return;
	}

	if (sk->sk_protocol == IPPROTO_TCP && sk->sk_state != TCP_SYN_SENT)
		return;

	if (sk->sk_protocol != IPPROTO_TCP && sk->sk_protocol != IPPROTO_UDP)
		return;

	if (sk->sk_slice_reported == 1)
		return;

	if (family == AF_INET)
		ipv4_para_report(skb);
	else if (family == AF_INET6)
		ipv6_para_report(skb);
	else
		hwlog_info("%s:sk_family not AF_INET and AF_INET6\n", __func__);

	return;
}

static void cmd_process(struct req_msg_head *msg)
{
	if (!msg) {
		pr_err("msg is null\n");
		return;
	}
	if (msg->type == SLICE_RPT_CTRL_CMD)
		ip_para_report_control(msg);
	else if (msg->type == BIND_PROCESS_TO_SLICE_CMD)
		bind_process_to_slice(msg);
	else if (msg->type == DEL_SLICE_BIND_CMD)
		unbind_process_to_slice(msg);
	else
		pr_err("unknown msg\n");
	return;
}

msg_process *network_slice_management_init(notify_event *notify)
{
	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		goto init_error;
	}
	notifier = notify;

	return cmd_process;

init_error:
	return NULL;
}
