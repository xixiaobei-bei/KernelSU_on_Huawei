/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to collect TCP/IP stack parameters
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#include "ip_para_collec.h"

#include <linux/net.h>
#include <net/sock.h>
#include <linux/list.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/tcp.h>

#include "ip_para_collec_ex.h"
#include "netlink_handle.h"
#ifdef CONFIG_HW_NETWORK_SLICE
#include "network_slice_management.h"
#endif

#ifdef CONFIG_STREAM_DETECT
#include "stream_detect.h"
#endif

#define TCP_HDR_DOFF_QUAD 4
#define TCP_HDR_IHL_QUAD 4
#define INVALID_LINK_PROPERTITY_INTERFACE_ID 0xFF

static struct app_ctx *g_app_ctx = NULL;
static struct link_property_info g_link_property[] = {
	{ DS_NET_ID, INVALID_LINK_PROPERTITY_INTERFACE_ID },
	{ DS_NET_SLAVE_ID, INVALID_LINK_PROPERTITY_INTERFACE_ID }
};

inline u32 get_sock_uid(struct sock *sk)
{
	return (u32)sk->sk_uid.val;
}

#if defined(CONFIG_HUAWEI_KSTATE) || defined(CONFIG_MPTCP)
inline u32 get_sock_pid(struct sock *sk)
{
	if (sk->sk_socket == NULL)
		return 0;
	return (u32)sk->sk_socket->pid;
}
#endif

static u32 get_current_srtt(struct tcp_sock *tp)
{
	u32 rtt_ms;
	u32 rcv_rtt_us = 0;
	u32 time;

	if (tp == NULL)
		return 0;

	if (tp->bytes_received < tp->content_length &&
		tp->key_flow &&
		tp->rx_opt.rcv_tsecr)
			rcv_rtt_us = tp->rcv_rtt_est.rtt_us;

	rtt_ms = tp->srtt_us > rcv_rtt_us ?
			tp->srtt_us : rcv_rtt_us;

	time = jiffies_to_msecs(tcp_jiffies32 - tp->rtt_update_tstamp);
	if (time < TIME_TEN_SEC)
		rtt_ms = rtt_ms > tp->prior_srtt_us ?
			rtt_ms : tp->prior_srtt_us;

	rtt_ms = rtt_ms / MULTIPLE_OF_RTT / US_MS;
	rtt_ms = (rtt_ms > MAX_RTT) ? MAX_RTT : rtt_ms;
	return rtt_ms;
}

static bool is_local_or_lan_addr(__be32 ip_addr)
{
	if (ipv4_is_loopback(ip_addr) ||
		ipv4_is_multicast(ip_addr) ||
		ipv4_is_local_multicast(ip_addr) ||
		ipv4_is_lbcast(ip_addr) ||
		ipv4_is_private_10(ip_addr) ||
		ipv4_is_private_172(ip_addr) ||
		ipv4_is_private_192(ip_addr) ||
		ipv4_is_linklocal_169(ip_addr))
		return true;
	else
		return false;
}

static bool is_private_v6_addr(struct sock *sk)
{
	const unsigned int clat_ipv4_index = 3;
	__be32 addr;
	int addr_type;
#if IS_ENABLED(CONFIG_IPV6)
	if (sk == NULL)
		return false;
	addr_type = ipv6_addr_type(&sk->sk_v6_daddr);
	if ((sk->sk_family == AF_INET6) &&
		(addr_type & IPV6_ADDR_MAPPED)) {
		addr = sk->sk_v6_daddr.s6_addr32[clat_ipv4_index];
		return is_local_or_lan_addr(addr);
	}
#endif
	return false;
}

static bool is_sk_daddr_private(struct sock *sk)
{
	if (sk->sk_family == AF_INET6)
		return is_private_v6_addr(sk);
	else if (sk->sk_family == AF_INET)
		return is_local_or_lan_addr(sk->sk_daddr);

	return false;
}

static u32 is_first_pkt_of_period(struct sock *sk)
{
	struct tcp_sock *tp = NULL;

	tp = tcp_sk(sk);
	if (tp == NULL || g_app_ctx == NULL ||
		g_app_ctx->report_tstamp == 0)
		return false;

	if (tp->last_pkt_tstamp == 0 && sk->sk_state == TCP_SYN_SENT)
		return true;

	return before(tp->last_pkt_tstamp, g_app_ctx->report_tstamp);
}

static bool is_sk_fullsock(struct sock *sk)
{
	if (sk != NULL && sk_fullsock(sk))
		return true;

	return false;
}

static bool match_key_processes_flow(unsigned short key_pid, struct sock * sk)
{
#if defined(CONFIG_HUAWEI_KSTATE) || defined(CONFIG_MPTCP)
	u32 pid = get_sock_pid(sk);
	if (key_pid == 0 || pid == 0)
		return false;
	if (pid == key_pid)
		return true;
#endif
	return false;
}

static bool match_flow_type(struct sock *sk, struct sk_buff *skb, u16 hook_type, u32 flow_type)
{
	u32 payloadlen;
	u8 *payload = NULL;

#ifdef CONFIG_STREAM_DETECT
	struct stream_info info = {0};
#endif
	struct tcphdr *th = tcp_hdr(skb);

	if (!(hook_type == NF_INET_LOCAL_IN))
		return false;

	if (th == NULL || skb->data == NULL)
		return false;

	payloadlen = skb->len - skb_transport_offset(skb) - tcp_hdrlen(skb);
	payload = skb_transport_header(skb) + tcp_hdrlen(skb);

#ifdef CONFIG_STREAM_DETECT
	info.dest_port = (u32)htons(th->source);
	return is_target_stream(sk, payload, payloadlen, flow_type, info);
#else
	return false;
#endif
}

static bool match_key_flow(struct key_flow_info *info, struct sock *sk, struct sk_buff *skb, u16 hook_type)
{
	switch (info->type) {
	case VIDEO_FLOW:
	case IMAGE_FLOW:
		return match_flow_type(sk, skb, hook_type, info->type);
	case KEY_PROCESSES_FLOW:
		return match_key_processes_flow(info->field.pid, sk);
	default:
		return false;
	}
}

static bool is_key_flow(struct sock *sk, struct sk_buff *skb, u16 hook_type, u8 protocol)
{
	struct tcp_sock *tp = NULL;
	u32 uid;
	u32 i;
	struct key_flow *flow = &g_app_ctx->flow;

	if (!is_sk_fullsock(sk) || protocol != IPPROTO_TCP)
		return false;

	uid = get_sock_uid(sk);
	tp = tcp_sk(sk);

	if (uid != g_app_ctx->fore_app_uid)
		return false;

	if (tp->key_flow)
		return true;

	for(i = 0; i < flow->cnt; i++)
		if (match_key_flow(&flow->info[i], sk, skb, hook_type)) {
			tp->key_flow = 1;
			pr_info("[IP_PARA]%s:already match key flow:%u\n",
			__func__, flow->info[i].type);
			return true;
		}
	return false;
}

static bool need_update_rtt_ofo(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	u32 time;

	if (!is_sk_fullsock(sk) || is_sk_daddr_private(sk))
		return false;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return false;

	tp = tcp_sk(sk);
	time = jiffies_to_usecs(tcp_jiffies32 - tp->ofo_tstamp) *
		MULTIPLE_OF_RTT;
	if (tp->ofo_tstamp == 0)
		return false;

	if (!RB_EMPTY_ROOT(&tp->out_of_order_queue)) {
		if (before(tp->rcv_nxt_ofo, tp->rcv_nxt) &&
			tp->rcv_nxt_ofo != 0)
			return true;
		else if (tp->prior_srtt_us < MAX_RTT_US &&
			time > tp->prior_srtt_us + OFO_RTT_UPDATE_THRESHOLD_US)
			return true;
	} else {
		if (tp->rcv_nxt_ofo != 0 &&
			tp->ofo_tstamp != 0)
			return true;
	}

	return false;
}

void update_ofo_rtt_for_qoe(struct sock *sk)
{
	if (!need_update_rtt_ofo(sk))
		return;
	update_tcp_para_without_skb(sk, NF_INET_OFO_HOOK);
}

void dec_sk_num_for_qoe(struct sock *sk, int new_state)
{
	int old_state;

	if (!is_sk_fullsock(sk))
		return;

	old_state = sk->sk_state;
	if ((old_state == TCP_ESTABLISHED ||
		old_state == TCP_SYN_SENT) &&
		new_state != TCP_ESTABLISHED &&
		new_state != TCP_SYN_SENT)
		update_tcp_para_without_skb(sk, NF_INET_DELETE_SK_HOOK);
}

static void increace_sk_num(struct sock *sk, struct tcp_res_info *stat)
{
	struct tcp_sock *tp = NULL;
	u32 rtt_ms;

	if (!is_sk_fullsock(sk))
		return;
	if (stat == NULL)
		return;
	tp = tcp_sk(sk);
	if (tp == NULL)
		return;

	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;

	if (is_first_pkt_of_period(sk)) {
		if (tp->last_pkt_tstamp != g_app_ctx->report_tstamp)
			tp->last_pkt_tstamp = g_app_ctx->report_tstamp;

		stat->tcp_sk_num++;
		rtt_ms = get_current_srtt(tp);
		tp->prior_srtt_us = rtt_ms * MULTIPLE_OF_RTT * US_MS;
		stat->sk_rtt_sum += rtt_ms;
	}
}

static void clear_prior_rtt(struct sock *sk)
{
	struct tcp_sock *tp = NULL;

	if (!is_sk_fullsock(sk))
		return;

	tp = tcp_sk(sk);
	if (tp->rtt_update_tstamp == 0)
		return;
	tp->prior_srtt_us = 0;
	tp->rtt_update_tstamp = 0;
}

static void update_sk_rtt_sum_common(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	if (!is_sk_fullsock(sk))
		return;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;

	tp = tcp_sk(sk);
	tp->prior_srtt_us = (tp->srtt_us > MAX_RTT_US) ?
		MAX_RTT_US : tp->srtt_us;
	tp->rtt_update_tstamp = tcp_jiffies32;
}

static void update_sk_rtt_retrans_common(struct sock *sk)
{
	struct inet_connection_sock *icsk = NULL;
	struct tcp_sock *tp = NULL;

	if (!is_sk_fullsock(sk))
		return;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;
	tp = tcp_sk(sk);
	icsk = inet_csk(sk);
	if (icsk == NULL)
		return;

	tp->prior_srtt_us = jiffies_to_usecs(icsk->icsk_rto) * MULTIPLE_OF_RTT;
	tp->prior_srtt_us = (tp->prior_srtt_us > MAX_RTT_US_RETRANS) ?
		MAX_RTT_US_RETRANS : tp->prior_srtt_us;
	tp->rtt_update_tstamp = tcp_jiffies32;
}

static void update_sk_rtt_ofo_common(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	u32 time;

	if (!is_sk_fullsock(sk))
		return;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;
	tp = tcp_sk(sk);

	time = jiffies_to_usecs(tcp_jiffies32 - tp->ofo_tstamp) *
		MULTIPLE_OF_RTT;
	tp->prior_srtt_us = (time > MAX_RTT_US) ?
		MAX_RTT_US : time;
	tp->rtt_update_tstamp = tcp_jiffies32;
}

void update_ofo_tstamp_for_qoe(struct sock *sk)
{
	struct tcp_sock *tp = NULL;

	if (!is_sk_fullsock(sk))
		return;

	tp = tcp_sk(sk);

	if (!RB_EMPTY_ROOT(&tp->out_of_order_queue)) {
		if (tp->rcv_nxt_ofo == 0 &&
			tp->ofo_tstamp == 0) {
			tp->rcv_nxt_ofo = tp->rcv_nxt + tp->rcv_wnd;
			tp->ofo_tstamp = tcp_jiffies32;
		} else if (before(tp->rcv_nxt_ofo, tp->rcv_nxt) &&
			tp->rcv_nxt_ofo != 0) {
			tp->rcv_nxt_ofo = tp->rcv_nxt + tp->rcv_wnd;
			tp->ofo_tstamp = tcp_jiffies32;
		}
	} else {
		if (tp->ofo_tstamp != 0) {
			tp->rcv_nxt_ofo = 0;
			tp->ofo_tstamp = 0;
		}
	}
}

static bool is_dns_sk(struct sock *sk, u8 protocol)
{
	if (sk == NULL)
		return false;
	if (protocol != IPPROTO_UDP)
		return false;
	if (htons(inet_sk(sk)->inet_dport) == DNS_PORT) {
		return true;
	}
	return false;
}

/*
 * the udp pkts from hooks which pid and uid can get.
 *
 * hooks 1  2  3  4
 * sock  0  0  Y  Y
 * UID   N  N  y  y
 * PID   N  N  N  N
 *
 * the tcp pkts from hooks which pid and uid can get.
 *
 * hooks 1  2  3  4
 * sock  1  0  1  1
 * UID   y  N  Y  Y
 * PID   N  N  Y  Y
 */
static u32 match_app(struct sock *sk, struct sk_buff *skb,
	struct tcp_res *stat, u8 protocol, u16 hook_type)
{
	u32 uid;

	if (sk == NULL || skb == NULL || stat == NULL)
		return FORGROUND_UNMACH;

	// forward
	if (stat->uid == 0) // 1-background uid: 0
		return BACKGROUND;
	if (stat->uid == DNS_UID && is_dns_sk(sk, protocol)) // 2-DNS protocol port:53(only UDP)
		return FORGROUND_MACH;
	if ((protocol == IPPROTO_TCP) && (!sk_fullsock(sk)))
		return FORGROUND_UNMACH; // ignore timewait or request socket
	if (stat->uid == KEY_FLOW_UID && is_key_flow(sk, skb, hook_type, protocol))
		return FORGROUND_MACH; // 3-key flow
	uid = get_sock_uid(sk);
	if (uid == stat->uid)
		return FORGROUND_MACH; // 4-others and uid, foreground

	return FORGROUND_UNMACH;
}

static void set_dev_name(struct sock *sk, const struct sk_buff *skb)
{
#ifdef CONFIG_HW_WIFIPRO
	unsigned int dev_max_len;
#endif

	if (sk == NULL || skb == NULL || skb->dev == NULL)
		return;

#ifdef CONFIG_HW_WIFIPRO
	if ((1 << sk->sk_state) & TCPF_SYN_SENT) {
		dev_max_len = strnlen(skb->dev->name,
			IFNAMSIZ - 1);
		strncpy(sk->wifipro_dev_name, skb->dev->name,
			dev_max_len);
		sk->wifipro_dev_name[dev_max_len] = '\0';
	}
#endif
}

void update_stats_srtt(struct tcp_sock *tp, u32 *rtt)
{
	u32 rtt_ms;
	if (rtt == NULL || tp == NULL)
		return;

	rtt_ms = get_current_srtt(tp);
	*rtt += rtt_ms;
}

static void update_post_routing_para(struct tcp_res_info *stat,
	struct sk_buff *skb, struct sock *sk, struct tcphdr *th, s32 tcp_len)
{
	struct tcp_sock *tp = NULL;

	tp = (struct tcp_sock *)sk;
	if (sk->sk_state == TCP_ESTABLISHED) {
		stat->out_pkt++;
		stat->out_len += skb->len;
	} else if (sk->sk_state == TCP_SYN_SENT) {
		if (th->syn == 1) {
			stat->tcp_syn_pkt++;
			set_dev_name(sk, skb);
		}
	} else {
		if (th->fin == 1)
			stat->tcp_fin_pkt++;
	}
	if (tp->snd_nxt != 0 &&
		before(ntohl(th->seq), tp->snd_nxt) &&
		(sk->sk_state == TCP_ESTABLISHED ||
		sk->sk_state == TCP_SYN_SENT)) {
		stat->out_rts_pkt++;
		if (sk->sk_state == TCP_SYN_SENT)
			stat->tcp_syn_rts_pkt++;
	}
	increace_sk_num(sk, stat);
}

/*
 * tcp pkt_in pid=0(default),uid is ok, pkt_out pid,uid is ok.
 */
static void update_tcp_para(struct tcp_res_info *stat, struct sk_buff *skb,
	struct sock *sk, u8 direction, u16 payload_len)
{
	struct tcphdr *th = tcp_hdr(skb);
	struct tcp_sock *tp = NULL;
	s32 tcp_len;

	if (direction == NF_INET_FORWARD) // sk == NULL
		stat->out_all_pkt++;
	if (sk == NULL)
		return;
	if (th == NULL)
		return;
	if (direction == NF_INET_BUFHOOK)
		return;
	tp = (struct tcp_sock *)sk;
	if (tp == NULL)
		return;
	tcp_len = payload_len - th->doff * TCP_HDR_DOFF_QUAD; // 32bits to byte
	if (tcp_len < 0)
		return;
	if (direction == NF_INET_LOCAL_IN) {
		increace_sk_num(sk, stat);
		if (sk->sk_state != TCP_ESTABLISHED)
			return;
		stat->in_pkt++;
		stat->in_len += skb->len;
		if (before(ntohl(th->seq) + tcp_len - 1, tp->rcv_nxt) &&
			tcp_len > 0)
			stat->in_rts_pkt++;

		update_stats_srtt(tp, &stat->rtt);
	} else if (direction == NF_INET_POST_ROUTING) {
		update_post_routing_para(stat, skb, sk, th, tcp_len);
	} else if (direction == NF_INET_LOCAL_OUT) {
		stat->out_all_pkt++;
	}
}

/*
 * udp udp_in uid=0, udp_out pid=0(or any pid,uid is ok),uid is ok
 */
void update_udp_para(struct tcp_res_info *stat, struct sk_buff *skb,
	u8 direction)
{
	if (direction == NF_INET_UDP_IN_HOOK) {
		stat->in_udp_pkt++;
		stat->in_udp_len += skb->len;
	} else if (direction == NF_INET_POST_ROUTING) {
		stat->out_udp_pkt++;
		stat->out_udp_len += skb->len;
	} else if (direction == NF_INET_FORWARD || direction == NF_INET_LOCAL_OUT) {
		stat->out_all_pkt++;
	}
}

static unsigned int get_mtk_net_id(const char *dev_name)
{
	unsigned int dev_id = 0;

	if (dev_name == NULL || strlen(dev_name) != DS_NET_MTK_LEN)
		return DS_NET_ID;

	// devName index is in [0, 9]
	if (dev_name[DS_NET_MTK_LEN - 1] < '0' ||
		dev_name[DS_NET_MTK_LEN - 1] > '9')
		return DS_NET_ID;
	dev_id = (unsigned int)(dev_name[DS_NET_MTK_LEN - 1] - '0');
	if (g_link_property[DS_NET_SLAVE_ID].interface_id == dev_id)
		return DS_NET_SLAVE_ID;
	return DS_NET_ID;
}

static unsigned int get_qcom_net_id(const char *dev_name)
{
	unsigned int dev_id = 0;

	if (dev_name == NULL || strlen(dev_name) != DS_NET_QCOM_LEN)
		return DS_NET_ID;

	// devName index is in [0, 9]
	if (dev_name[DS_NET_QCOM_LEN - 1] < '0' ||
		dev_name[DS_NET_QCOM_LEN - 1] > '9')
		return DS_NET_ID;
	dev_id = (unsigned int)(dev_name[DS_NET_QCOM_LEN - 1] - '0');
	if (g_link_property[DS_NET_SLAVE_ID].interface_id == dev_id)
		return DS_NET_SLAVE_ID;
	return DS_NET_ID;
}

static int match_net_dev(struct sk_buff *skb, struct sock *sk, u32 *dev_id)
{
	char *dev_name = NULL;

	if (skb == NULL || sk == NULL || dev_id == NULL)
		return NET_DEV_ERR;
	if (skb->dev == NULL || skb->dev->name == NULL) {
#ifdef CONFIG_HW_WIFIPRO
		if (sk_fullsock(sk) &&
			(sk->sk_family == AF_INET ||
			sk->sk_family == AF_INET6) &&
			sk->sk_protocol == IPPROTO_TCP)
			dev_name = sk->wifipro_dev_name;
#endif
	} else {
		dev_name = skb->dev->name;
	}

	if (dev_name == NULL)
		return NET_DEV_ERR;

	*dev_id = 0;
	if (strncmp(dev_name, DS_NET, DS_NET_LEN) == 0)
		*dev_id = DS_NET_ID;
	else if (strncmp(dev_name, DS_NET_SLAVE, DS_NET_LEN) == 0)
		*dev_id = DS_NET_SLAVE_ID;
	else if (strncmp(dev_name, DS_NET_MTK, DS_NET_MTK_PREFIX_LEN) == 0)
		*dev_id = get_mtk_net_id(dev_name);
	else if (strncmp(dev_name, DS_NET_QCOM, DS_NET_QCOM_PREFIX_LEN) == 0)
		*dev_id = get_qcom_net_id(dev_name);
	else if (strncmp(dev_name, WIFI_NET, WIFI_NET_LEN) == 0)
		*dev_id = WIFI_NET_ID;
	else if (strncmp(dev_name, WIFI_NET_SLAVE, WIFI_NET_LEN) == 0)
		*dev_id = WIFI_NET_SLAVE_ID;
	else
		return NET_DEV_ERR;
	return RTN_SUCC;
}

static struct app_list *get_match_node_from_list(struct sock *sk,
	struct sk_buff *skb, unsigned int cpu, u8 protocol, u16 hook_type)
{
	struct app_stat *app = NULL;
	struct app_list *pos = NULL;
	struct app_list *select = NULL;
	u32 app_type;

	if (sk == NULL)
		return NULL;

	if (cpu < 0 || cpu >= CONFIG_NR_CPUS)
		return NULL;

	app = &g_app_ctx->cur[cpu];

	if (list_empty_careful(&app->head))
		return NULL;

	list_for_each_entry(pos, &app->head, head) {
		app_type = match_app(sk, skb, &pos->stat, protocol, hook_type);
		if (app_type == BACKGROUND) {
			select = pos;
		} else if (app_type == FORGROUND_MACH) {
			select = pos;
			break;
		}
	}
	return select;
}

static void update_tcp_rtt_sk_num(struct sock *sk,
	struct tcp_res_info *stat, unsigned int direction)
{
	if (stat == NULL)
		return;
	if (direction == NF_INET_DELETE_SK_HOOK) {
		clear_prior_rtt(sk);
	} else if (direction == NF_INET_UPDATE_RTT_HOOK) {
		update_sk_rtt_sum_common(sk);
	} else if (direction == NF_INET_RETRANS_TIMER_HOOK) {
		update_sk_rtt_retrans_common(sk);
	} else if (direction == NF_INET_OFO_HOOK) {
		update_sk_rtt_ofo_common(sk);
	}
}

static void process_stat_info_tcp_layer(struct sock *sk,
	struct app_list *pos_selec, unsigned int direction, u32 dev_id)
{
	if (dev_id < 0 ||
		dev_id >= CHANNEL_NUM ||
		pos_selec == NULL)
		return;

	if (direction >= NF_INET_NEW_SK_HOOK && direction <= NF_INET_OFO_HOOK) {
		update_tcp_rtt_sk_num(sk,
			&pos_selec->stat.info[dev_id],
			direction);
	} else if (direction == NF_INET_TCP_DUPACK_IN_HOOK) {
		pos_selec->stat.info[dev_id].in_dupack_pkt++;
	}
}

static void stat_proces(struct sk_buff *skb, struct sock *sk,
	const struct nf_hook_state *state, u8 protocol, u16 payload_len)
{
	struct app_stat *app = NULL;
	struct app_list *pos_selec = NULL;
	unsigned int cpu = raw_smp_processor_id();
	u32 dev_id;

	if (skb == NULL || sk == NULL)
		return;
	if (cpu < 0 || cpu >= CONFIG_NR_CPUS)
		return;
	if (state == NULL)
		return;

	app = &g_app_ctx->cur[cpu];
	spin_lock_bh(&app->lock);

	if (match_net_dev(skb, sk, &dev_id) != RTN_SUCC)
		goto unlock;

	pos_selec = get_match_node_from_list(sk, skb, cpu,
		protocol, state->hook);
	if (pos_selec == NULL)
		goto unlock;

	if (state->hook >= NF_INET_TCP_DUPACK_IN_HOOK) {
		process_stat_info_tcp_layer(sk,
			pos_selec, state->hook, dev_id);
	} else {
		if (protocol == IPPROTO_TCP)
			update_tcp_para(&pos_selec->stat.info[dev_id], skb, sk,
				state->hook, payload_len);
		else if (protocol == IPPROTO_UDP)
			update_udp_para(&pos_selec->stat.info[dev_id], skb,
				state->hook);
	}

unlock:
	spin_unlock_bh(&app->lock);
}

void update_dupack_num(struct sock *sk,
	bool is_dupack, unsigned int hook)
{
	if (is_dupack)
		update_tcp_para_without_skb(sk, NF_INET_TCP_DUPACK_IN_HOOK);
}

static void update_tcp_para_with_skb(struct sk_buff *skb, struct sock *sk,
	unsigned int hook)
{
	struct nf_hook_state state;
	if (skb == NULL || !is_sk_fullsock(sk) || is_sk_daddr_private(sk))
		return;

	memset(&state, 0, sizeof(struct nf_hook_state));
	state.hook = hook;
	stat_proces(skb, sk, &state, IPPROTO_TCP, (u16)skb->len);
}

static bool is_tcp_udp_sock(struct sock *sk)
{
	if (!is_sk_fullsock(sk))
		return false;

	return (sk->sk_type == SOCK_STREAM || sk->sk_type == SOCK_DGRAM);
}

static unsigned int hook4(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct iphdr *iph = NULL;
	struct sock *sk = NULL;
	u16 payload_len;

	if (state == NULL)
		return NF_ACCEPT;

#ifdef CONFIG_HW_NETWORK_SLICE
	if (state->hook == NF_INET_LOCAL_OUT)
		slice_ip_para_report(skb, AF_INET);
#endif

#ifdef CONFIG_STREAM_DETECT
	if (state->hook == NF_INET_POST_ROUTING)
		stream_process_hook_out(skb, AF_INET);
	if (state->hook == NF_INET_LOCAL_IN)
		stream_process_hook_in(skb, AF_INET);
#endif

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	if (iph->protocol == IPPROTO_UDP && state->hook == NF_INET_LOCAL_IN)
		return NF_ACCEPT;
	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		sk = state->sk;
	if (!is_tcp_udp_sock(sk))
		return NF_ACCEPT;

	if (iph->protocol == IPPROTO_TCP &&
		is_local_or_lan_addr(sk->sk_daddr))
		return NF_ACCEPT;

	payload_len = ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD; // 32bits to byte
	if (iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_UDP)
		stat_proces(skb, sk, state, iph->protocol, payload_len);
	return NF_ACCEPT;
}

void update_tcp_para_without_skb(struct sock *sk, unsigned int hook)
{
	struct sk_buff skb;

	if (sk == NULL)
		return;

	memset(&skb, 0, sizeof(struct sk_buff));
	skb.len = IP_TCP_HEAD_LEN;
	update_tcp_para_with_skb(&skb, sk, hook);
}

void udp_in_hook(struct sk_buff *skb, struct sock *sk)
{
	const struct iphdr *iph = NULL;
	u16 payload_len;
	struct nf_hook_state state;

	memset(&state, 0, sizeof(struct nf_hook_state));
	state.hook = NF_INET_UDP_IN_HOOK;
	if (skb == NULL)
		return;
	iph = ip_hdr(skb);
	if (iph == NULL)
		return;
	payload_len = ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD; // 32bits to byte
	stat_proces(skb, sk, &state, iph->protocol, payload_len);
}

void udp6_in_hook(struct sk_buff *skb, struct sock *sk)
{
	const struct ipv6hdr *iph = NULL;
	u16 payload_len;
	struct nf_hook_state state;

	memset(&state, 0, sizeof(struct nf_hook_state));
	state.hook = NF_INET_UDP_IN_HOOK;

	if (skb == NULL || sk == NULL)
		return;
	iph = ipv6_hdr(skb);
	if (iph == NULL)
		return;
	payload_len = ntohs(iph->payload_len);
	stat_proces(skb, sk, &state, iph->nexthdr, payload_len);
}

static unsigned int hook6(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct ipv6hdr *iph = NULL;
	struct sock *sk = NULL;
	u16 payload_len;

	if (state == NULL)
		return NF_ACCEPT;

#ifdef CONFIG_HW_NETWORK_SLICE
	if (state->hook == NF_INET_LOCAL_OUT)
		slice_ip_para_report(skb, AF_INET6);
#endif

#ifdef CONFIG_STREAM_DETECT
	if (state->hook == NF_INET_POST_ROUTING)
		stream_process_hook_out(skb, AF_INET6);
	if (state->hook == NF_INET_LOCAL_IN)
		stream_process_hook_in(skb, AF_INET6);
#endif

	iph = ipv6_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	if (iph->nexthdr == IPPROTO_UDP && state->hook == NF_INET_LOCAL_IN)
		return NF_ACCEPT;
	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		sk = state->sk;
	if (!is_tcp_udp_sock(sk))
		return NF_ACCEPT;

	if (iph->nexthdr == IPPROTO_TCP &&
		is_private_v6_addr(sk))
		return NF_ACCEPT;

	payload_len = ntohs(iph->payload_len);
	if (iph->nexthdr == IPPROTO_TCP || iph->nexthdr == IPPROTO_UDP)
		stat_proces(skb, sk, state, iph->nexthdr, payload_len);

	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_FORWARD,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_FORWARD,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
};

static void add_to_res(struct tcp_res *add, struct tcp_res *sum)
{
	int i;

	for (i = 0; i < CHANNEL_NUM; i++) {
		sum->info[i].in_rts_pkt += add->info[i].in_rts_pkt;
		sum->info[i].in_pkt += add->info[i].in_pkt;
		sum->info[i].in_len += add->info[i].in_len;
		sum->info[i].out_rts_pkt += add->info[i].out_rts_pkt;
		sum->info[i].out_pkt += add->info[i].out_pkt;
		sum->info[i].out_len += add->info[i].out_len;
		sum->info[i].rtt += add->info[i].rtt;
		sum->info[i].out_all_pkt += add->info[i].out_all_pkt;
		sum->info[i].in_udp_pkt += add->info[i].in_udp_pkt;
		sum->info[i].out_udp_pkt += add->info[i].out_udp_pkt;
		sum->info[i].in_udp_len += add->info[i].in_udp_len;
		sum->info[i].out_udp_len += add->info[i].out_udp_len;
		sum->info[i].tcp_syn_pkt += add->info[i].tcp_syn_pkt;
		sum->info[i].tcp_fin_pkt += add->info[i].tcp_fin_pkt;
		sum->info[i].in_dupack_pkt += add->info[i].in_dupack_pkt;
		sum->info[i].tcp_sk_num += add->info[i].tcp_sk_num;
		sum->info[i].sk_rtt_sum += add->info[i].sk_rtt_sum;
		sum->info[i].tcp_syn_rts_pkt += add->info[i].tcp_syn_rts_pkt;
		if (sum->info[i].in_len > MAX_U32_VALUE)
			sum->info[i].in_len = MAX_U32_VALUE;
		if (sum->info[i].out_len > MAX_U32_VALUE)
			sum->info[i].out_len = MAX_U32_VALUE;
		if (sum->info[i].rtt > MAX_U32_VALUE)
			sum->info[i].rtt = MAX_U32_VALUE;
		if (sum->info[i].in_udp_len > MAX_U32_VALUE)
			sum->info[i].in_udp_len = MAX_U32_VALUE;
		if (sum->info[i].out_udp_len > MAX_U32_VALUE)
			sum->info[i].out_udp_len = MAX_U32_VALUE;
	}
}

static void reset_stat(struct tcp_res *entry)
{
	int i;

	for (i = 0; i < CHANNEL_NUM; i++) {
		entry->info[i].in_rts_pkt = 0;
		entry->info[i].in_pkt = 0;
		entry->info[i].in_len = 0;
		entry->info[i].out_rts_pkt = 0;
		entry->info[i].out_pkt = 0;
		entry->info[i].out_len = 0;
		entry->info[i].rtt = 0;
		entry->info[i].out_all_pkt = 0;
		entry->info[i].in_udp_pkt = 0;
		entry->info[i].out_udp_pkt = 0;
		entry->info[i].in_udp_len = 0;
		entry->info[i].out_udp_len = 0;
		entry->info[i].tcp_syn_pkt = 0;
		entry->info[i].tcp_fin_pkt = 0;
		entry->info[i].in_dupack_pkt = 0;
		entry->info[i].tcp_sk_num = 0;
		entry->info[i].sk_rtt_sum = 0;
		entry->info[i].tcp_syn_rts_pkt = 0;
	}
}

static void rm_list(struct list_head *list)
{
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	struct app_list *pos = NULL;

	list_for_each_safe(p, n, list) {
		pos = list_entry(p, struct app_list, head);
		list_del_init(p);
		if (pos == NULL)
			return;
		kfree(pos);
	}
	list->prev = list;
	list->next = list;
}

static void cum_cpus_stat(struct app_stat *cur,
	struct tcp_collect_res *tcp, u16 cnt)
{
	struct app_list *pos = NULL;
	int i;
	bool need_add = true;

	spin_lock_bh(&cur->lock);
	list_for_each_entry(pos, &cur->head, head) {
		need_add = true;
		for (i = 0; i < tcp->cnt; i++) {
			if (pos->stat.pid == tcp->res[i].pid &&
				pos->stat.uid == tcp->res[i].uid) {
				add_to_res(&pos->stat, &tcp->res[i]);
				reset_stat(&pos->stat);
				need_add = false;
				break;
			}
		}
		if (need_add) {
			if (tcp->cnt >= cnt)
				break;
			tcp->res[tcp->cnt].pid = pos->stat.pid;
			tcp->res[tcp->cnt].uid = pos->stat.uid;
			add_to_res(&pos->stat, &tcp->res[tcp->cnt]);
			reset_stat(&pos->stat);
			tcp->cnt++;
		}
	}
	spin_unlock_bh(&cur->lock);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void stat_report(unsigned long sync)
#else
static void stat_report(struct timer_list* sync)
#endif
{
	struct res_msg *res = NULL;
	u16 cnt;
	u16 len;
	u32 i;
	u32 j;

	spin_lock_bh(&g_app_ctx->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_app_ctx->timer.data = sync + 1; // number of reports
#endif
	g_app_ctx->timer.function = stat_report;
	mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	cnt = g_app_ctx->cnt;
	len = sizeof(struct res_msg) + cnt * sizeof(struct tcp_res);
	res = kmalloc(len, GFP_ATOMIC);

	if (res == NULL)
		goto report_end;

	memset(res, 0, len);
	res->type = APP_QOE_RPT;
	res->len = len;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	res->res.tcp.sync = (u16)sync;
#endif

	for (i = 0; i < cnt; i++) {
		for (j = 0; j < CHANNEL_NUM; j++)
			res->res.tcp.res[i].info[j].dev_id = j;
	}
	for (i = 0; i < CONFIG_NR_CPUS; i++)
		cum_cpus_stat(g_app_ctx->cur + i, &res->res.tcp, cnt);

	g_app_ctx->report_tstamp = tcp_jiffies32;

	if (g_app_ctx->fn)
		g_app_ctx->fn((struct res_msg_head *)res);

	kfree(res);

report_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void record_fore_app_uid(u32 uid)
{
	if (uid == BACKGROUND_UID ||
		uid == DNS_UID ||
		uid == KEY_FLOW_UID)
		return;

	g_app_ctx->fore_app_uid = uid;
}

static void sync_apps_list(struct app_id *id, u16 cnt,
	struct app_stat *stat)
{
	u16 i;
	bool need_new = true;
	struct list_head tmp_head;
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	struct app_list *pos = NULL;

	tmp_head.next = &tmp_head;
	tmp_head.prev = &tmp_head;
	for (i = 0; i < cnt; i++) {
		need_new = true;
		list_for_each_safe(p, n, &stat->head) {
			pos = list_entry(p, struct app_list, head);
			if (pos->stat.uid != id[i].uid ||
				pos->stat.pid != id[i].pid)
				continue;
			list_move_tail(p, &tmp_head);
			need_new = false;
			break;
		}
		if (need_new) {
			pos = kmalloc(sizeof(struct app_list), GFP_ATOMIC);
			if (pos == NULL)
				goto list_end;
			memset(pos, 0, sizeof(struct app_list));
			pos->stat.pid = id[i].pid;
			pos->stat.uid = id[i].uid;
			list_add_tail(&pos->head, &tmp_head);
		}
	}
	record_fore_app_uid(id[cnt - 1].uid);
	rm_list(&stat->head);
	stat->head.prev = tmp_head.prev;
	stat->head.next = tmp_head.next;
	tmp_head.prev->next = &stat->head;
	tmp_head.next->prev = &stat->head;
	return;

list_end:
	rm_list(&tmp_head);
}

static void sync_to_cpus(struct tcp_collect_req *req)
{
	int i;
	struct app_stat *stat = g_app_ctx->cur;

	for (i = 0; i < CONFIG_NR_CPUS; i++) {
		spin_lock_bh(&stat[i].lock);
		sync_apps_list(req->id, req->cnt, &stat[i]);
		spin_unlock_bh(&stat[i].lock);
	}
	g_app_ctx->cnt = req->cnt;
}

static u32 ms_convert_jiffies(u32 cycle)
{
	return cycle / JIFFIES_MS;
}

static void process_sync(struct req_msg *msg)
{
	struct tcp_collect_req *req = &msg->req.tcp_req;
	u16 flag = req->req_flag;
	u32 cycle = req->rpt_cycle;

	pr_info("[IP_PARA]%s data: %d %d", __func__, req->req_flag, req->rpt_cycle);
	spin_lock_bh(&g_app_ctx->lock);
	if (flag & RESTART_SYNC) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
		g_app_ctx->timer.data = 0;
#endif
		g_app_ctx->timer.function = stat_report;
		g_app_ctx->jcycle = ms_convert_jiffies(cycle);
		mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	} else {
		if (!timer_pending(&g_app_ctx->timer))
			goto sync_end;
		del_timer(&g_app_ctx->timer);
	}
sync_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void sync_key_flow(struct key_flow *new_flow) {
	u32 i;
	struct key_flow *flow = &g_app_ctx->flow;
	flow->cnt = new_flow->cnt;
	pr_info("[IP_PARA] %s,cnt=%u", __func__, new_flow->cnt);
	for (i = 0; i < new_flow->cnt; i++)
		flow->info[i] = new_flow->info[i];
}

static void process_app_update(struct req_msg *msg)
{
	struct tcp_collect_req *req = &msg->req.tcp_req;
	u16 flag = req->req_flag;
	u32 cycle = req->rpt_cycle;

	pr_info("[IP_PARA] %s data :flag=%d,cycle=%d,cnt=%d", __func__, req->req_flag,
		req->rpt_cycle, req->cnt);
	spin_lock_bh(&g_app_ctx->lock);

	if (req->cnt > MAX_APP_LIST_LEN)
		goto app_end;

	if ((flag & FORFROUND_STAT) || (flag & BACKGROUND_STAT)) {
		sync_to_cpus(req);
		sync_key_flow(&req->key_flow);
	}

	if (flag & RESTART_SYNC) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
		g_app_ctx->timer.data = 0;
#endif
		g_app_ctx->timer.function = stat_report;
		g_app_ctx->jcycle = ms_convert_jiffies(cycle);
		mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	}

app_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void process_link_property_update(struct link_property_msg *msg)
{
	struct link_property_info *property = &msg->property;
	u32 modem_id = property->modem_id;
	u32 interface_id = property->interface_id;

	pr_info("[LINK PROPERTY] %s modem_id:%d, interface name id:%d\n", __func__,
		modem_id, interface_id);

	if (modem_id != DS_NET_ID && modem_id != DS_NET_SLAVE_ID)
		return;
	if (interface_id > DS_INTERFACE_NAME_ID)
		return;
	g_link_property[modem_id].interface_id = interface_id;
}

static void cmd_process(struct req_msg_head *msg)
{
	if (msg->len > MAX_REQ_DATA_LEN)
		return;

	if (msg->type == APP_QOE_SYNC_CMD)
		process_sync((struct req_msg *)msg);
	else if (msg->type == UPDATE_APP_INFO_CMD)
		process_app_update((struct req_msg *)msg);
	else if (msg->type == UPDAT_INTERFACE_NAME)
		process_link_property_update((struct link_property_msg *)msg);
	else
		pr_info("[IP_PARA]map msg error\n");
}

/*
 * Initialize internal variables and external interfaces
 */
msg_process *ip_para_collec_init(notify_event *fn)
{
	int i;
	int ret;

	if (fn == NULL)
		return NULL;

	g_app_ctx = kmalloc(sizeof(struct app_ctx), GFP_KERNEL);
	if (g_app_ctx == NULL)
		return NULL;
	memset(g_app_ctx, 0, sizeof(struct app_ctx));
	g_app_ctx->fn = fn;
	g_app_ctx->cur = kmalloc(sizeof(struct app_stat) * CONFIG_NR_CPUS,
				 GFP_KERNEL);
	if (g_app_ctx->cur == NULL)
		goto init_error;
	memset(g_app_ctx->cur, 0, sizeof(struct app_stat) * CONFIG_NR_CPUS);
	for (i = 0; i < CONFIG_NR_CPUS; i++) {
		spin_lock_init(&g_app_ctx->cur[i].lock);
		g_app_ctx->cur[i].head.prev = &g_app_ctx->cur[i].head;
		g_app_ctx->cur[i].head.next = &g_app_ctx->cur[i].head;
	}
	spin_lock_init(&g_app_ctx->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	init_timer(&g_app_ctx->timer);
#else
	timer_setup(&g_app_ctx->timer, stat_report, 0);
#endif
	g_app_ctx->report_tstamp = tcp_jiffies32;
	ret = nf_register_net_hooks(&init_net, net_hooks,
				    ARRAY_SIZE(net_hooks));
	if (ret) {
		pr_info("[IP_PARA]nf_init_in ret=%d  ", i);
		goto init_error;
	}
	return cmd_process;

init_error:
	if (g_app_ctx->cur != NULL)
		kfree(g_app_ctx->cur);
	if (g_app_ctx != NULL)
		kfree(g_app_ctx);
	g_app_ctx = NULL;
	return NULL;
}

void ip_para_collec_exit(void)
{
	if (g_app_ctx->cur != NULL)
		kfree(g_app_ctx->cur);
	if (g_app_ctx != NULL)
		kfree(g_app_ctx);
	g_app_ctx = NULL;
}
