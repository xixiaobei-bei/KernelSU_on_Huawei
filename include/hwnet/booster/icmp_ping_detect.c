/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: This module is to detect icmp ping proc
 * Create: 2019-10-09
 */

#include "icmp_ping_detect.h"

#include <linux/net.h>
#include <net/sock.h>
#include <linux/list.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/tcp.h>

#include "netlink_handle.h"
#include "icmp_ping_detect.h"

struct icmp_ping_detect_cfg g_icmp_cfg;

static int is_data_net_dev(struct sk_buff *skb, unsigned int direc)
{
	if (skb == NULL)
		return -1;

	if (skb->dev == NULL || skb->dev->name == NULL)
		return -1;

	if (strncmp(skb->dev->name, ICMP_DS_NET, ICMP_DS_NET_LEN) != 0)
		return -1;

	if (direc != NF_INET_PRE_ROUTING && direc != NF_INET_POST_ROUTING)
		return -1;

	return 0;
}

static void icmp_ping_report(void)
{
	struct icmp_res_msg *res = NULL;
	u16 len = sizeof(struct icmp_res_msg);

	spin_lock_bh(&g_icmp_cfg.lock);
	if (g_icmp_cfg.ap_cmd == STOP_DETECT)
		goto end;
	res = kmalloc(len, GFP_ATOMIC);

	if (res == NULL)
		goto end;

	memset(res, 0, len);
	res->type = ICMP_PING_REPORT;
	res->len = len;
	res->result = ICMP_PING_PROC;
	if (g_icmp_cfg.report_fn)
		g_icmp_cfg.report_fn((struct res_msg_head *)res);

	kfree(res);

end:
	spin_unlock_bh(&g_icmp_cfg.lock);
}

static unsigned int icmp_detect_hook4(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct iphdr *iph = NULL;

	if (is_data_net_dev(skb, state->hook) != 0)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	if (iph->saddr == iph->daddr)
		return NF_ACCEPT;
	if (iph->protocol == IPPROTO_ICMP) {
		if (icmp_hdr(skb)->type == ICMP_ECHO)
			icmp_ping_report();
	}

	return NF_ACCEPT;
}

static unsigned int icmp_detect_hook6(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct ipv6hdr *iphv6 = NULL;
	const struct icmp6hdr *icmp6 = NULL;

	if (is_data_net_dev(skb, state->hook) != 0)
		return NF_ACCEPT;
	iphv6 = ipv6_hdr(skb);
	if (iphv6 == NULL)
		return NF_ACCEPT;
	if (!memcmp(&iphv6->saddr, &iphv6->daddr, sizeof(struct in6_addr)))
		return NF_ACCEPT;
	if (iphv6->nexthdr == IPPROTO_ICMP) {
		icmp6 = icmp6_hdr(skb);
		if (icmp6->icmp6_type == ICMP_ECHO)
			icmp_ping_report();
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops icmp_net_hooks[] = {
	{
		.hook = icmp_detect_hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = icmp_detect_hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = icmp_detect_hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = icmp_detect_hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_LAST,
	},
};

msg_process *icmp_ping_detect_init(notify_event *fn)
{
	int ret;

	memset(&g_icmp_cfg, 0, sizeof(struct icmp_ping_detect_cfg));
	spin_lock_init(&g_icmp_cfg.lock);
	ret = nf_register_net_hooks(&init_net, icmp_net_hooks,
		ARRAY_SIZE(icmp_net_hooks));
	if (ret)
		goto init_error;
	g_icmp_cfg.ap_cmd = STOP_DETECT;
	g_icmp_cfg.report_fn = fn;
	return icmp_ping_process;

init_error:
	return NULL;
}

void icmp_ping_process(struct req_msg_head *msg)
{
	struct icmp_req_msg *icmp_msg = (struct icmp_req_msg *)msg;
	u32 cmd_type = STOP_DETECT;

	if (msg->len > MAX_ICMP_CMD_LEN)
		return;

	if (msg->len < sizeof(struct icmp_req_msg))
		return;

	if (msg->type == ICMP_PING_DETECT_CMD) {
		cmd_type = icmp_msg->cmd_type;
		if (cmd_type == STOP_DETECT || cmd_type == START_DETECT)
			g_icmp_cfg.ap_cmd = cmd_type;
		else
			pr_info("[ICMP_PING] cmd invalid:%d!", cmd_type);
	} else {
		pr_info("[ICMP_PING] type error:%d", msg->type);
	}
}
