/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is performance statistics for data transmission.
 * Author: lianchaofeng@huawei.com
 * Create: 2019-12-10
 */

#include <linux/cpufreq.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/smp.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <net/inet_sock.h>

#include "hw_pt.h"
#include "../chr/wbc_hw_hook.h"

#define DS_NET_PREFIX "rmnet"
#define RNIC_UL_DELAY_LIMIT (HZ / 10)
#define RNIC_DL_DELAY_LIMIT (HZ / 20)
#define L4_DL_DELAY_LIMIT (HZ / 10)
#define MAX_LISTENING_UID_NUM 1
#define PACKET_DELAY_DATA_LEN 64
#define MIN_APP_UID 10000

#define assign_short(p, val) (*(s16 *)(p) = (val))
#define assign_int(p, val) (*(s32 *)(p) = (val))
#define skip_byte(p, num) ((p) = (p) + (num))

// units: jiffie(==4ms)
#define MAX_UL_TRANS_TIME 28
#define MAX_DL_TRANS_TIME 28

#define MAX_DL_RECV_QUEUE_WAIT_TIME 2000
#define TIMER_EXCEED_MAX (600 * HZ)
#define TIMER_STATISTICS_PEROID (10 * HZ)
#define TIMER_REPORT_BACKOFF_PEROID (3600 * HZ)
#define CHR_TIMER_NOT_START 0
#define CHR_TIMER_STATISTICS 1
#define CHR_TIMER_REPORT_WAIT 2

#define KERNEL_ABNORMAL_PACK_CONTS 10
#define RECV_QUEUE_ABNORMAL_PACK_CONTS 4000
#define KENRL_KEY_LENGTH_MAX 32
#define TCP_HDR_DOFF_QUAD 4
#define TCP_HDR_IHL_QUAD 4

extern int g_app_top_uid;
static notify_event *notifier = NULL;

enum DIR {
	DOWNLINK_IN = 0,
	UPLINK = 1,
	DOWNLINK_OUT = 2,
};

static int g_hw_pt_ul_data_time_cost = 0;
static int g_hw_pt_ul_data_count = 0;
static int g_hw_pt_ul_abnormal_data_count = 0;

static int g_hw_pt_dl_data_time_cost = 0;
static int g_hw_pt_dl_in_data_count = 0;
static int g_hw_pt_dl_in_abnormal_data_count = 0;
static int g_hw_pt_dl_out_abnormal_data_count = 0;

static int g_hw_pt_ul_abnormal_data_not_rpt_count = 0;
static int g_hw_pt_dl_abnormal_data_not_rpt_count = 0;

static int g_chr_data_process_timer_state = CHR_TIMER_NOT_START;
static struct timer_list g_kernel_data_process_timer;

static int g_last_app_top_uid = 0;

static uid_t get_uid_from_sock_new(struct sock *sk)
{
	kuid_t kuid;

	if (!sk || !sk_fullsock(sk))
		return overflowuid;
	kuid = sock_net_uid(sock_net(sk), sk);
	return from_kuid_munged(sock_net(sk)->user_ns, kuid);
}

static void chr_kernel_clear_data_record(void)
{
	g_hw_pt_ul_data_time_cost = 0;
	g_hw_pt_ul_data_count = 0;
	g_hw_pt_ul_abnormal_data_count = 0;

	g_hw_pt_dl_data_time_cost = 0;
	g_hw_pt_dl_in_data_count = 0;
	g_hw_pt_dl_in_abnormal_data_count = 0;
	g_hw_pt_dl_out_abnormal_data_count = 0;

	g_hw_pt_ul_abnormal_data_not_rpt_count = 0;
	g_hw_pt_dl_abnormal_data_not_rpt_count = 0;
}

static bool is_chr_report_needed(void)
{
	return (((g_hw_pt_ul_abnormal_data_count +
		g_hw_pt_dl_in_abnormal_data_count) >
		KERNEL_ABNORMAL_PACK_CONTS) ||
		(g_hw_pt_ul_abnormal_data_count >
		KERNEL_ABNORMAL_PACK_CONTS) ||
		(g_hw_pt_dl_in_abnormal_data_count >
		KERNEL_ABNORMAL_PACK_CONTS) ||
		(g_hw_pt_dl_out_abnormal_data_count >
		RECV_QUEUE_ABNORMAL_PACK_CONTS));
}

static void prepare_report_data(struct hw_kernel_delay_info *info_rpt)
{
	info_rpt->uplink_delay_count = g_hw_pt_ul_abnormal_data_count;
	info_rpt->uplink_data_count = g_hw_pt_ul_data_count;
	if (g_hw_pt_ul_abnormal_data_count == 0)
		info_rpt->uplink_time_cost_ave = 0;
	else
		info_rpt->uplink_time_cost_ave = g_hw_pt_ul_data_time_cost /
			g_hw_pt_ul_abnormal_data_count;

	info_rpt->downlink_delay_cnt = g_hw_pt_dl_in_abnormal_data_count;
	info_rpt->downlink_data_count = g_hw_pt_dl_in_data_count;
	if (g_hw_pt_dl_in_abnormal_data_count == 0)
		info_rpt->downlink_time_cost_ave = 0;
	else
		info_rpt->downlink_time_cost_ave = g_hw_pt_dl_data_time_cost /
			g_hw_pt_dl_in_abnormal_data_count;

	info_rpt->downlink_out_wait_cnt = g_hw_pt_dl_out_abnormal_data_count;
	info_rpt->uplink_abnormal_not_report_cnt =
		g_hw_pt_ul_abnormal_data_not_rpt_count;
	info_rpt->downlink_abnormal_not_report_cnt =
		g_hw_pt_dl_abnormal_data_not_rpt_count;
}

static void set_chr_report_timer(int status, int len)
{
	pr_err("%s: ENTER, old status = %d, new status = %d, timerLen = %d \n",
		__func__, g_chr_data_process_timer_state, status, len);
	if (status == CHR_TIMER_NOT_START || len <= 0)
		return;
	if (g_chr_data_process_timer_state == CHR_TIMER_NOT_START && !timer_pending(&g_kernel_data_process_timer)) {
		g_kernel_data_process_timer.expires = jiffies + len;
		add_timer(&g_kernel_data_process_timer);
		g_chr_data_process_timer_state = status;
	}
}

static bool is_top_app_changed(void)
{
	return (g_app_top_uid > MIN_APP_UID) && (g_last_app_top_uid != g_app_top_uid);
}

static void notify_packet_delay_event(int uid, struct hw_kernel_delay_info *info_rpt)
{
	char event[PACKET_DELAY_DATA_LEN] = {0};
	char *p = event;

	if (!notifier)
		return;

	assign_short(p, PACKET_DELAY_RPT);
	skip_byte(p, sizeof(s16));

	// data len(2B type + 2B len + 40B DATA), type is short
	assign_short(p, 44);
	skip_byte(p, sizeof(s16));

	assign_int(p, uid);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->uplink_delay_count);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->uplink_data_count);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->uplink_time_cost_ave);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->downlink_delay_cnt);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->downlink_data_count);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->downlink_time_cost_ave);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->downlink_out_wait_cnt);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->uplink_abnormal_not_report_cnt);
	skip_byte(p, sizeof(int));

	assign_int(p, info_rpt->downlink_abnormal_not_report_cnt);

	notifier((struct res_msg_head *)event);
}

static bool is_socket_belong_fg_app(struct sock *sk)
{
	uid_t uid = get_uid_from_sock_new(sk);
	struct hw_kernel_delay_info info_rpt = {0};

	if (is_top_app_changed()) {
		if (g_chr_data_process_timer_state == CHR_TIMER_STATISTICS) {
			if (!del_timer(&g_kernel_data_process_timer))
				return false;

			g_chr_data_process_timer_state = CHR_TIMER_NOT_START;
			if (is_chr_report_needed()) {
				prepare_report_data(&info_rpt);
				notify_packet_delay_event(g_last_app_top_uid, &info_rpt);
				set_chr_report_timer(CHR_TIMER_REPORT_WAIT, TIMER_REPORT_BACKOFF_PEROID);
			}
		}
		chr_kernel_clear_data_record();
		g_last_app_top_uid = g_app_top_uid;
	}
	return (g_app_top_uid > MIN_APP_UID) && (uid == g_app_top_uid);
}

static void do_commands(struct req_msg_head *msg)
{
	if (!msg) {
		pr_err("msg is null\n");
		return;
	}
	return;
}

static void hw_pt_update_counters(int dir, int time_cost)
{
	if (dir == UPLINK) {
		g_hw_pt_ul_abnormal_data_count++;
		g_hw_pt_ul_data_time_cost += time_cost;
	} else if (dir == DOWNLINK_IN) {
		g_hw_pt_dl_in_abnormal_data_count++;
		g_hw_pt_dl_data_time_cost += time_cost;
	} else {
		g_hw_pt_dl_out_abnormal_data_count++;
	}
}

static void hw_pt_exception_happen(int dir, int time_cost)
{
	if (dir == UPLINK)
		g_hw_pt_ul_data_count++;
	else if (dir == DOWNLINK_IN)
		g_hw_pt_dl_in_data_count++;

	if (time_cost >= MAX_UL_TRANS_TIME) {
		switch (g_chr_data_process_timer_state) {
		case CHR_TIMER_NOT_START:
			set_chr_report_timer(CHR_TIMER_STATISTICS, TIMER_STATISTICS_PEROID);
			hw_pt_update_counters(dir, time_cost);
			break;
		case CHR_TIMER_STATISTICS:
			hw_pt_update_counters(dir, time_cost);
			break;
		case CHR_TIMER_REPORT_WAIT:
			if (dir == UPLINK) {
				g_hw_pt_ul_abnormal_data_not_rpt_count++;
			} else {
				g_hw_pt_dl_abnormal_data_not_rpt_count++;
			}
			break;
		default:
			break;
		}
	}
}

void hw_pt_set_skb_stamp(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo = NULL;
	if (skb == NULL)
		return;
	shinfo = skb_shinfo(skb);
	if (shinfo != NULL) {
		shinfo->born_stamp = jiffies;
		shinfo->proc_bitmask = PROC_BITMASK_SETTED;
	}
}
EXPORT_SYMBOL(hw_pt_set_skb_stamp);

static bool hw_pt_log_exception_need_continue(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo = NULL;
	struct iphdr *ih = NULL;

	shinfo = skb_shinfo(skb);
	if (shinfo == NULL) {
		pr_err("%s ERR: shinfo is NULL \n", __func__);
		return false;
	}

	if ((shinfo->proc_bitmask & PROC_BITMASK_SET_MASK) != PROC_BITMASK_SETTED
		|| shinfo->born_stamp == 0) {
		pr_err("%s ERR: input param error\n", __func__);
		return false;
	}

	if (skb->protocol == ntohs(ETH_P_IP)) {
		ih = ip_hdr(skb);
		if (ih->saddr == ih->daddr)
			return false;
	}
	return true;
}

static void hw_pt_log_exception(struct sock *sk, struct sk_buff *skb, int prot,
	int delay, const char *func, int dir)
{
	struct tcphdr *th = NULL;
	struct skb_shared_info *shinfo = NULL;
	int time_cost;

	if (!hw_pt_log_exception_need_continue(skb))
		return;

	shinfo = skb_shinfo(skb);
	time_cost = jiffies_to_msecs(jiffies - shinfo->born_stamp);
	if (time_cost > TIMER_EXCEED_MAX)
		return;

	switch (prot) {
	case IPPROTO_TCP:
		th = tcp_hdr(skb);
		if (th->fin == 1 || th->rst == 1) {
			break;
		}
		hw_pt_exception_happen(dir, time_cost);
		break;
	case IPPROTO_UDP:
		hw_pt_exception_happen(dir, time_cost);
		break;
	default:
		break;
	}
}

static bool hw_pt_is_uplink_need_continue(struct sk_buff *skb, struct net_device *dev)
{
	if (!skb || !dev) {
		pr_err("%s ERR: input params is null\n", __func__);
		return false;
	}

	if (!is_socket_belong_fg_app(skb->sk))
		return false;

	if (strncmp(dev->name, DS_NET_PREFIX, strlen(DS_NET_PREFIX)))
		return false;

	if ((skb->protocol != ntohs(ETH_P_IP)) && (skb->protocol != ntohs(ETH_P_IPV6))) {
		pr_err("%s ERR: skb->protocol = %d \n", __func__, skb->protocol);
		return false;
	}
	return true;
}

static int get_layer4_protocl(struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	__u8 l4_prot = 0;

	if (skb->protocol == ntohs(ETH_P_IP)) {
		iph = (struct iphdr *)(skb_network_header(skb));
		if (!iph || (iph->saddr == iph->daddr))
			return 0;
		l4_prot = iph->protocol;
	} else if (skb->protocol == ntohs(ETH_P_IPV6)) {
		ip6h = (struct ipv6hdr *)skb_network_header(skb);
		if (!ip6h)
			return 0;
		l4_prot = ip6h->nexthdr;
	}
	return l4_prot;
}

void hw_pt_dev_uplink(struct sk_buff *skb, struct net_device *dev)
{
	struct skb_shared_info *shinfo = NULL;
	struct tcphdr *th = NULL;
	struct iphdr *iph = NULL;
	int len; // tcp header length + data length
	int time_cost;
	int package_len; // tcp header length
	struct ipv6hdr *ip6h = NULL;

	if (!hw_pt_is_uplink_need_continue(skb, dev))
		return;

	shinfo = skb_shinfo(skb);
	if (!shinfo || (shinfo->proc_bitmask & PROC_BITMASK_RNIC_UL_OUT))
		return;

	time_cost = jiffies - shinfo->born_stamp;
	if (time_cost < 0)
		return;

	switch (get_layer4_protocl(skb)) {
	case IPPROTO_TCP:
		th = tcp_hdr(skb);
		if (th != NULL) {
			package_len = th->doff * TCP_HDR_DOFF_QUAD;
			if (skb->protocol == ntohs(ETH_P_IPV6)) {
				ip6h = (struct ipv6hdr *)skb_network_header(skb);
				len = ntohs(ip6h->payload_len);
			} else if (skb->protocol == ntohs(ETH_P_IP)) {
				iph = (struct iphdr *)(skb_network_header(skb));
				len = ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD;
			} else {
				return;
			}

			if (len > package_len) {
				shinfo->proc_bitmask |= PROC_BITMASK_RNIC_UL_OUT;
				return;
			}
		}
		hw_pt_log_exception(NULL, skb, IPPROTO_TCP, RNIC_UL_DELAY_LIMIT, __func__, UPLINK);
		break;
	case IPPROTO_UDP:
		hw_pt_log_exception(NULL, skb, IPPROTO_UDP, RNIC_UL_DELAY_LIMIT, __func__, UPLINK);
		break;
	default:
		return;
	}

	shinfo->proc_bitmask |= PROC_BITMASK_RNIC_UL_OUT; // record this skb is sent
}
EXPORT_SYMBOL(hw_pt_dev_uplink);

void hw_pt_l4_downlink_in(struct sock *skt, struct sk_buff *skb, int eaten)
{
	struct skb_shared_info *shinfo = NULL;
	struct sock *sk = NULL;

	if (!skb)
		return;
	if (skt)
		sk = sk_to_full_sk((struct sock *)skt);
	else
		sk = sk_to_full_sk(skb->sk);

	if (!is_socket_belong_fg_app(sk))
		return;

	switch (sk->sk_protocol) {
	case IPPROTO_TCP:
		hw_pt_log_exception(sk, skb, IPPROTO_TCP, RNIC_DL_DELAY_LIMIT, __func__, DOWNLINK_IN);
		if (!eaten) {
			shinfo = skb_shinfo(skb);
			if (shinfo == NULL)
				break;
			shinfo->born_stamp = jiffies;
		}
		break;
	case IPPROTO_UDP:
		hw_pt_log_exception(sk, skb, IPPROTO_UDP, RNIC_DL_DELAY_LIMIT, __func__, DOWNLINK_IN);
		shinfo = skb_shinfo(skb);
		if (shinfo == NULL)
			break;
		shinfo->born_stamp = jiffies;
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(hw_pt_l4_downlink_in);

void hw_pt_l4_downlink_out(struct sock *skt, struct sk_buff *skb)
{
	struct sock *sk = NULL;

	if (!skb)
		return;

	if (skt)
		sk = sk_to_full_sk((struct sock *)skt);
	else
		sk = sk_to_full_sk(skb->sk);

	if (!is_socket_belong_fg_app(sk))
		return;

	switch (sk->sk_protocol) {
	case IPPROTO_TCP:
		hw_pt_log_exception(sk, skb, IPPROTO_TCP, L4_DL_DELAY_LIMIT, __func__, DOWNLINK_OUT);
		break;
	case IPPROTO_UDP:
		hw_pt_log_exception(sk, skb, IPPROTO_UDP, L4_DL_DELAY_LIMIT, __func__, DOWNLINK_OUT);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(hw_pt_l4_downlink_out);

static void chr_kernel_data_process_timer(unsigned long data)
{
	struct hw_kernel_delay_info info_rpt = {0};

	g_chr_data_process_timer_state = CHR_TIMER_NOT_START;

	if (is_chr_report_needed()) {
		prepare_report_data(&info_rpt);
		notify_packet_delay_event(g_last_app_top_uid, &info_rpt);
		set_chr_report_timer(CHR_TIMER_REPORT_WAIT, TIMER_REPORT_BACKOFF_PEROID);
		chr_kernel_clear_data_record();
	}
}

msg_process* __init hw_pt_init(notify_event *notify)
{
	pr_debug("%s: ENTER \n", __func__);

	init_timer(&g_kernel_data_process_timer);
	g_kernel_data_process_timer.data = 0;
	g_kernel_data_process_timer.function = chr_kernel_data_process_timer;

	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}
	notifier = notify;
	return do_commands;
}

MODULE_LICENSE("GPL");
