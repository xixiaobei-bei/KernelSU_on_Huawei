/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to collect TCP stack parameters
 * Author: liuleimin1@huawei.com
 * Create: 2019-04-18
 */

#include "tcp_para_collec.h"

#include <linux/net.h>
#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/tcp.h>

#include "hw_booster_common.h"

static DEFINE_SPINLOCK(g_tcp_para_collec_lock);

static long tcp_tx_pkt_sum = 0;
static long tcp_rx_pkt_sum = 0;
static long tcp_rx_syn_ack_sum = 0;
static long tcp_rx_data_sum = 0;
static notify_event *notifier = NULL;

void update_tcp_syn_ack_pkt_stat(struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;

	if (skb == NULL)
		return;
	if (skb->dev == NULL || skb->dev->name == NULL)
		return;

	/* identify the network protocol of an IP packet */
	if (ntohs(skb->protocol) == ETH_P_IPV6) {
		ip6h = ipv6_hdr(skb);
		if (ip6h == NULL || ip6h->nexthdr != IPPROTO_TCP)
			return;
	} else {
		iph = ip_hdr(skb);
		if (iph == NULL || iph->protocol != IPPROTO_TCP)
			return;
	}

	tcph = tcp_hdr(skb);
	if (tcph == NULL || skb->data == NULL || tcph->doff == 0)
		return;
	if (tcph->syn == 1 && tcph->ack == 1)
		tcp_rx_syn_ack_sum++;
}

void booster_update_tcp_statistics(u_int8_t af, struct sk_buff *skb,
	struct net_device *in, struct net_device *out)
{
	unsigned int thoff = 0;
	int proto;

	if (skb == NULL)
		return;
	if (!is_ds_rnic(out) && !is_ds_rnic(in))
		return;
	if (af == AF_INET6)
		proto = ipv6_find_hdr(skb, &thoff, -1, NULL, NULL);
	else if (af == AF_INET)
		proto = ip_hdr(skb)->protocol;
	else
		proto = IPPROTO_RAW;

	if (proto != IPPROTO_TCP)
		return;
	spin_lock_bh(&g_tcp_para_collec_lock);
	if (out != NULL)
		tcp_tx_pkt_sum++;
	if (in != NULL) {
		tcp_rx_pkt_sum++;
		update_tcp_syn_ack_pkt_stat(skb);
	}
	spin_unlock_bh(&g_tcp_para_collec_lock);
	return;
}

static unsigned int hook(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	u_int8_t af;
	if (skb == NULL || state == NULL)
		return NF_ACCEPT;

	if (state->pf == NFPROTO_IPV4)
		af = AF_INET;
	else if (state->pf == NFPROTO_IPV6)
		af = AF_INET6;
	else
		af = AF_UNSPEC;
	booster_update_tcp_statistics(af, skb, state->in, NULL);
	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = hook,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FILTER,
	},
	{
		.hook = hook,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FILTER,
	},
};

static void notify_tcp_collec_event(void)
{
	char event[NOTIFY_BUF_LEN];
	char *p = event;
	const int notify_len = 20;
	const int data_sum_len = 8;

	if (!notifier)
		return;
	memset(&event, 0, NOTIFY_BUF_LEN);
	// type
	assign_short(p, TCP_PKT_CONUT_RPT);
	skip_byte(p, sizeof(s16));
	// len(2B type + 2B len + 8B tcp_tx_pkt_sum + 8B tcp_rx_data_sum)
	assign_short(p, notify_len);
	skip_byte(p, sizeof(s16));
	spin_lock_bh(&g_tcp_para_collec_lock);
	// 8B tcp_tx_pkt_sum
	assign_long(p, tcp_tx_pkt_sum);
	skip_byte(p, data_sum_len);
	// 8B tcp_rx_data_sum
	tcp_rx_data_sum = tcp_rx_pkt_sum - tcp_rx_syn_ack_sum;
	assign_long(p, tcp_rx_data_sum);
	spin_unlock_bh(&g_tcp_para_collec_lock);
	notifier((struct res_msg_head *)event);
}

static void cmd_process(struct req_msg_head *msg)
{
	if (!msg) {
		pr_err("msg is null\n");
		return;
	}
	if (msg->type == TCP_PKT_COLLEC_CMD)
		notify_tcp_collec_event();
	return;
}

msg_process *tcp_para_collec_init(notify_event *notify)
{
	int ret;

	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}

	ret = nf_register_net_hooks(&init_net, net_hooks,
		ARRAY_SIZE(net_hooks));
	if (ret) {
		pr_info("nf_init_in error");
		goto init_error;
	}
	notifier = notify;
	return cmd_process;

init_error:
	return NULL;
}
