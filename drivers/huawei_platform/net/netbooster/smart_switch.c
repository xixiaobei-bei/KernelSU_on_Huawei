/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: This file is used by the netfilter hook for smart switch.
 * Author: linlixin2@huawei.com
 * Create: 2017-10-23
 */

#include "smart_switch.h"

#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/memory.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <linux/string.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/udp.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <uapi/linux/netlink.h>

#include <huawei_platform/log/hw_log.h>

#include "ai_predict.h"
#include "clf_params.h"
#include "constant.h"
#include "nb_netlink.h"

#ifndef DEBUG
#define DEBUG
#endif

/* smart switch statistic variable */
static struct pkt_stat_swth g_stat;

/* Input and output data processing of the lock */
static spinlock_t g_smart_switch_lock;

/* Result escalation Variable */
static struct report_slow_para g_swth_report;

/* report time Variable */
static long g_report_time;

/* RTT filter Variable */
static unsigned int g_rtt;

/* KSI is enabled identifier */
static uint8_t g_report_flag;

/* Hook is enabled identifier */
static uint8_t g_hook_flag;
#ifdef CONFIG_CHR_NETLINK_MODULE
static struct chr_para g_chr_data;
static struct timer_list g_chr_rpt_timer;
static struct report_chr_stru g_chr_report;
#endif

/* hidata app qoe statistic variable */
static struct pkt_stat_swth g_app_qoe_stat;
static uid_t g_app_qoe_uid = 0;
static int g_app_qoe_cycle = 0; /* seconds */
static int g_app_qoe_level = NETWORK_STATUS_INVALID;
static int g_slow_num = 0;
static int g_no_rx_num = 0;
static int g_normal_num = 0;
static int g_alpha_filter_factor = 1;
static int g_alpha_filter_factor_small = 1;
static int g_alpha_filter_factor_big = 10;
static int g_alpha_filter_change_thresh = 0;
static unsigned int g_no_rx_thresh = 2;
static unsigned long g_no_rx_time_stamp = 0;

static int g_network_detect_window = 5; /* seconds */
static int g_network_status = NETWORK_STATUS_NETWORK_NORMAL;
static int g_slow_count = 0;
static int g_no_rx_count = 0;
static int g_normal_count = 0;
static int g_rsrp = 0;
static int g_dbm = 0;
static unsigned int g_app_rtt;

static void stat_pkt_in(struct tcp_sock *sk, struct tcphdr *th,
	struct pkt_stat_swth *stat_ptr, unsigned int len);
static unsigned int idx_update(struct pkt_stat_swth *stat_ptr);
static void stat_pkt_out(struct tcp_sock *sk, struct tcphdr *th,
	struct pkt_stat_swth *stat_ptr, unsigned int len);

#ifdef CONFIG_APP_QOE_AI_PREDICT
static bool is_need_update_app_qoe(struct sk_buff *skb)
{
	struct sock *sk = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;

	if (g_app_qoe_cycle <= 0 || g_app_qoe_uid <= 0)
		return false;
	if (skb == NULL)
		return false;
	iph = ip_hdr(skb);
	if (iph == NULL)
		return false;
	if (iph->protocol == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		if (tcph == NULL)
			return false;
		sk = skb_to_full_sk(skb);
		/*
		 * When in TCP_TIME_WAIT the sk is not a "struct sock" but
		 * "struct inet_timewait_sock" which is missing fields.
		 * So we ignore it.
		 */
		if (sk && sk->sk_state == TCP_TIME_WAIT)
			return false;
		if (sk && sk->sk_uid.val == g_app_qoe_uid)
			return true;
	}
	return false;
}

static void update_tcp_app_qoe_hook_out(struct sk_buff *skb,
	struct tcphdr *tcph, u32 data_len)
{
	if (skb == NULL || tcph == NULL || data_len > MAX_PKT_LEN)
		return;
	if (is_need_update_app_qoe(skb)) {
		if (skb->sk->sk_state == TCP_ESTABLISHED) {
			idx_update(&g_app_qoe_stat);
			stat_pkt_out((struct tcp_sock *)skb->sk,
				tcph, &g_app_qoe_stat, data_len);
		} else if (skb->sk->sk_state == TCP_SYN_SENT) {
			idx_update(&g_app_qoe_stat);
			if (tcph->syn == 1) /* 【SYN】package */
				g_app_qoe_stat.stat[g_app_qoe_stat.idx].syn++;
		}
	}
}

static void update_tcp_app_qoe_hook_in(struct sk_buff *skb,
	struct tcphdr *tcph, u32 data_len)
{
	if (skb == NULL || tcph == NULL || data_len > MAX_PKT_LEN)
		return;
	if (is_need_update_app_qoe(skb)) {
		idx_update(&g_app_qoe_stat);
		stat_pkt_in((struct tcp_sock *)skb->sk,
			tcph, &g_app_qoe_stat, data_len);
	}
}
#endif

/* check index */
static int idx_check(int index)
{
	if (index < 0)
		index = index + MAX_STAT_SEC;

	if (index < 0)
		return 0;

	index = index % MAX_STAT_SEC;

	return index;
}

/* Smart switching CHR */
#ifdef CONFIG_CHR_NETLINK_MODULE
int get_rtt_list(struct native_event *rtt_event, unsigned int list_len)
{
	int cnt;
	int idx = 0;

	if (list_len > MAX_STAT_SEC || rtt_event == NULL)
		return -1;

	/* Avoiding cache and index timeout for the lack of packets. */
	if (idx_update(&g_stat))
		pr_info("return old second\n");

	for (cnt = 0; cnt < list_len; cnt++) {
		idx = idx_check(g_stat.idx - cnt);
		rtt_event->rtt_list[cnt] = g_stat.stat[idx].rtt;
		rtt_event->max_list[cnt] = g_stat.stat[idx].rtt_max;
		if (g_stat.stat[idx].rtt_cnt != 0)
			rtt_event->avg_list[cnt] = g_stat.stat[idx].rtt_all /
				g_stat.stat[idx].rtt_cnt;
		else
			rtt_event->avg_list[cnt] = 0;
	}
	return 0;
}

unsigned int chr_smart_switch(struct chr_para *report)
{
	if (report != NULL && virt_addr_valid((uintptr_t)report)) {
		memcpy(report, &g_chr_data, sizeof(g_chr_data));
		memset(&g_chr_data, 0, sizeof(g_chr_data));
	}
	return 0;
}

static void mean_stat_old(void)
{
	int cnt;

	g_stat.idx = idx_check(g_stat.idx);
	g_chr_data.nsi_old = g_stat.norm_idx[g_stat.idx].flt_ksi;
	memset(&(g_chr_data.stat_old), 0, sizeof(struct pkt_cnt_swth));
	for (cnt = 0; cnt < MAX_STAT_SEC; cnt++) {
		g_chr_data.stat_old.in_pkt += g_stat.stat[cnt].in_pkt;
		g_chr_data.stat_old.out_pkt += g_stat.stat[cnt].out_pkt;
		g_chr_data.stat_old.in_len += g_stat.stat[cnt].in_len;
		g_chr_data.stat_old.out_len += g_stat.stat[cnt].out_len;
		g_chr_data.stat_old.dupack += g_stat.stat[cnt].dupack;
		g_chr_data.stat_old.rts += g_stat.stat[cnt].rts;
		g_chr_data.stat_old.syn += g_stat.stat[cnt].syn;
		g_chr_data.stat_old.rtt += g_stat.stat[cnt].rtt;
	}
}

static void mean_stat_new(void)
{
	int cnt;

	g_stat.idx = idx_check(g_stat.idx);
	g_chr_data.nsi_new = g_stat.norm_idx[g_stat.idx].flt_ksi;
	memset(&(g_chr_data.stat_new), 0, sizeof(struct pkt_cnt_swth));
	for (cnt = 0; cnt < MAX_STAT_SEC; cnt++) {
		g_chr_data.stat_new.in_pkt += g_stat.stat[cnt].in_pkt;
		g_chr_data.stat_new.out_pkt += g_stat.stat[cnt].out_pkt;
		g_chr_data.stat_new.in_len += g_stat.stat[cnt].in_len;
		g_chr_data.stat_new.out_len += g_stat.stat[cnt].out_len;
		g_chr_data.stat_new.dupack += g_stat.stat[cnt].dupack;
		g_chr_data.stat_new.rts += g_stat.stat[cnt].rts;
		g_chr_data.stat_new.syn += g_stat.stat[cnt].syn;
		g_chr_data.stat_new.rtt += g_stat.stat[cnt].rtt;
	}
}

static void chr_report(unsigned long data)
{
	int rtt;

	g_stat.idx = idx_check(g_stat.idx);
	mean_stat_new();
	g_chr_report.slow_type = NETWORK_STATUS_FOR_CHR;
	g_chr_report.avg_amp = g_stat.norm_idx[g_stat.idx].flt_ksi;
	rtt = g_chr_data.stat_old.rtt / MAX_STAT_SEC / STAT_STEP;
	g_chr_report.old_rtt = (rtt > MAX_RTT) ? MAX_RTT : rtt;
	rtt = g_chr_data.stat_new.rtt / MAX_STAT_SEC / STAT_STEP;
	g_chr_report.new_rtt = (rtt > MAX_RTT) ? MAX_RTT : rtt;
	nb_notify_event(NBMSG_KSI_EVT, &g_chr_report, sizeof(g_chr_report));
	pr_info("KSI chr slow timer report\n");
}

#endif

/* Normalized parameters */
static int norm_para(const unsigned char norm[],
	const unsigned short idx[], int len, int value)
{
	int cnt;
	int val;
	int min_cnt = 0;
	int min_val;

	min_val = absolute(value - idx[0]);
	for (cnt = 1; cnt < len; cnt++) {
		val = absolute(value - idx[cnt]);
		if (val < min_val) {
			min_cnt = cnt;
			min_val = val;
		}
	}
	return norm[min_cnt];
}

/* Calculate the normalized values of each factor */
static void calc_norm(struct pkt_stat_swth *stat_ptr, unsigned int idx)
{
	unsigned int div;

	if (stat_ptr == NULL)
		return;

	idx = idx_check(idx);

	div = stat_ptr->stat[idx].in_len * MUL128;
	if (stat_ptr->stat[idx].out_len == 0)
		stat_ptr->stat[idx].out_len++;

	div = div / stat_ptr->stat[idx].out_len;
	if (div > MUL128)
		div = MUL128;

	stat_ptr->norm_idx[idx].inout =
		norm_para(inout_norm, inout_idx, INOUT_LEN, div);

	stat_ptr->norm_idx[idx].rtt =
		norm_para(rtt_norm, rtt_idx, RTT_LEN, stat_ptr->stat[idx].rtt);

	stat_ptr->norm_idx[idx].in_len = norm_para(in_norm, in_idx, IN_LEN,
		stat_ptr->stat[idx].in_len);

	stat_ptr->norm_idx[idx].out_len = norm_para(out_norm, out_idx, OUT_LEN,
		stat_ptr->stat[idx].out_len);

	stat_ptr->norm_idx[idx].in_pkt = norm_para(inpkt_norm, inpkt_idx,
		INPKT_LEN, stat_ptr->stat[idx].in_pkt);

	stat_ptr->norm_idx[idx].out_pkt = norm_para(outpkt_norm, outpkt_idx,
		OUTPKT_LEN, stat_ptr->stat[idx].out_pkt);

	stat_ptr->norm_idx[idx].syn = norm_para(syn_norm, syn_idx, SYN_LEN,
		stat_ptr->stat[idx].syn);

	stat_ptr->norm_idx[idx].rts = norm_para(rts_norm, rts_idx, RTS_LEN,
		stat_ptr->stat[idx].rts);

	stat_ptr->norm_idx[idx].dupack = norm_para(dupack_norm, dupack_idx,
		DUPACK_LEN, stat_ptr->stat[idx].dupack);
}

/* Calculate KSI values */
static void calc_ksi(struct pkt_stat_swth *stat_ptr, unsigned int idx)
{
	if (stat_ptr == NULL)
		return;

	idx = idx_check(idx);

	stat_ptr->norm_idx[idx].ksi =
		(MUL128 - stat_ptr->norm_idx[idx].inout) * coefficient[COEF_0] +
		stat_ptr->norm_idx[idx].rtt * coefficient[COEF_1] +
		(MUL128 - stat_ptr->norm_idx[idx].in_len) *
		coefficient[COEF_2] +
		(MUL128 - stat_ptr->norm_idx[idx].in_pkt) *
		coefficient[COEF_3] +
		(MUL128 - stat_ptr->norm_idx[idx].out_len) *
		coefficient[COEF_4] +
		(MUL128 - stat_ptr->norm_idx[idx].out_pkt) *
		coefficient[COEF_5] +
		stat_ptr->norm_idx[idx].dupack * coefficient[COEF_6] +
		stat_ptr->norm_idx[idx].rts * coefficient[COEF_7] +
		stat_ptr->norm_idx[idx].syn * coefficient[COEF_8];

	if (stat_ptr->norm_idx[idx].ksi > MAX_KSI)
		stat_ptr->norm_idx[idx].ksi = 0;
}

/* KSI Filter */
static void filter_ksi(struct pkt_stat_swth *stat_ptr, unsigned int idx,
	unsigned int len)
{
	int cnt;

	if (stat_ptr == NULL)
		return;

	idx = idx_check(idx);
	if (len > MAX_STAT_SEC)
		len = MAX_STAT_SEC;

	for (cnt = 0; cnt <= len; cnt++)
		stat_ptr->norm_idx[idx_check(idx + cnt)].flt_ksi =
			stat_ptr->norm_idx[idx_check(idx + cnt - 1)].flt_ksi -
			stat_ptr->norm_idx[idx_check(idx + cnt - 1)].flt_ksi /
			FILT_20 +
			stat_ptr->norm_idx[idx_check(idx + cnt - 1)].ksi /
			FILT_20;
}

static void update_swth_report(int amp, int idx_dura, int idx_start)
{
	if (idx_dura >= MAX_THRED) {
		memset(&g_swth_report, 0, sizeof(g_swth_report));
		g_swth_report.slow_type = 1;
		g_swth_report.avg_amp = amp / idx_dura / MUL128;
		g_swth_report.duration = idx_dura;
		g_swth_report.time_start = idx_check(g_stat.idx - idx_start);
		if (before(g_report_time + REPORT_TIMER * HZ, jiffies) &&
			g_report_flag) {
			nb_notify_event(NBMSG_KSI_EVT, &g_swth_report,
				sizeof(g_swth_report));
			pr_info("KSI network slow %d\n", g_swth_report.avg_amp);
			g_report_time = jiffies;
#ifdef CONFIG_CHR_NETLINK_MODULE
			mean_stat_old();
			mod_timer(&g_chr_rpt_timer, jiffies + CHR_REPORT_TIMER);
#endif
		}
	}
}

/* Calculate KSI values */
static void detect_ksi(void)
{
	int cnt;
	int idx_start = -1;
	int idx_dura = -1;
	int idx_tmp;
	int amp = 0;

	g_stat.idx = idx_check(g_stat.idx);
	if (g_stat.norm_idx[g_stat.idx].flt_ksi > KSI_THRED)
		idx_start = g_stat.idx - MAX_STAT_SEC;

	for (cnt = 1; cnt < MAX_STAT_SEC - 1; cnt++) {
		idx_tmp = idx_check(g_stat.idx - cnt);
		if (g_stat.norm_idx[idx_tmp].flt_ksi <= KSI_THRED &&
			g_stat.norm_idx[idx_check(idx_tmp - 1)].flt_ksi >
				KSI_THRED) {
			amp = g_stat.norm_idx[idx_tmp].flt_ksi;
			idx_start = idx_tmp;
			idx_dura = 1;
		}

		if (g_stat.norm_idx[idx_tmp].flt_ksi >= KSI_THRED &&
			g_stat.norm_idx[idx_check(idx_tmp - 1)].flt_ksi <
				KSI_THRED) {
			if (idx_dura < MAX_THRED) {
				amp = 0;
				idx_start = -1;
				idx_dura = -1;
			} else {
				break;
			}
		}

		if (g_stat.norm_idx[idx_tmp].flt_ksi >= KSI_THRED) {
			amp += g_stat.norm_idx[idx_tmp].flt_ksi;
			idx_dura++;
		}
	}
	update_swth_report(amp, idx_dura, idx_start);
}

/* Update KSI values */
static int ksi_update(struct pkt_stat_swth *stat_ptr)
{
	int cnt;
	int idx_cur;
	int idx_prv;

	if (stat_ptr == NULL)
		return -1;

	stat_ptr->idx = idx_check(stat_ptr->idx);
	calc_norm(stat_ptr, stat_ptr->idx);
	calc_ksi(stat_ptr, stat_ptr->idx);
	if (stat_ptr->stat[stat_ptr->idx].rtt < RTT_THRED) {
		filter_ksi(stat_ptr, stat_ptr->idx, 0);
		return -1;
	}

	for (cnt = 0; cnt < EXTERN_TIME; cnt++) {
		idx_cur = idx_check(stat_ptr->idx - cnt);
		idx_prv = idx_check(stat_ptr->idx - cnt - 1);
		if (stat_ptr->stat[idx_prv].rtt == 0) {
			stat_ptr->stat[idx_prv].rtt =
				stat_ptr->stat[stat_ptr->idx].rtt;
			calc_norm(stat_ptr, idx_prv);
			calc_ksi(stat_ptr, idx_prv);
		} else {
			filter_ksi(stat_ptr, idx_cur, cnt);
			break;
		}
	}

	if (&g_stat == stat_ptr)
		detect_ksi();

	return 0;
}

/* Statistic Input Package Parameters */
static void stat_pkt_in(struct tcp_sock *sk, struct tcphdr *th,
	struct pkt_stat_swth *stat_ptr, unsigned int len)
{
	unsigned int rtt_ms;
	u32 prior_snd_una;
	u32 ack;
	bool is_dupack = false;
	int prior_packets;

	if (th == NULL || sk == NULL || stat_ptr == NULL)
		return;

	stat_ptr->idx = idx_check(stat_ptr->idx);
	stat_ptr->stat[stat_ptr->idx].in_pkt++;
	stat_ptr->stat[stat_ptr->idx].in_len += len;

	prior_snd_una = sk->snd_una;
	ack = ntohl(th->ack_seq);
	prior_packets = sk->packets_out; /* Packets which are "in flight" */
	if ((!prior_packets) || (len > sk->tcp_header_len))
		is_dupack = false;
	else if (ack == prior_snd_una)
		is_dupack = true;
	if (is_dupack == true)
		stat_ptr->stat[stat_ptr->idx].dupack++;

	rtt_ms = (sk->srtt_us) / US_MS / FILT_8;

	/* RTT update */
	if (&g_stat == stat_ptr) {
		g_rtt = g_rtt + rtt_ms / FILT_16 - g_rtt / FILT_16;
		if (stat_ptr->stat[stat_ptr->idx].rtt < g_rtt)
			stat_ptr->stat[stat_ptr->idx].rtt = g_rtt;
	} else {
		if (g_app_rtt == 0)
			g_app_rtt = rtt_ms;
		else
			g_app_rtt = g_app_rtt +
				(rtt_ms * g_alpha_filter_factor) / FILT_16 -
				(g_app_rtt * g_alpha_filter_factor) / FILT_16;
		if (stat_ptr->stat[stat_ptr->idx].rtt < g_app_rtt)
			stat_ptr->stat[stat_ptr->idx].rtt = g_app_rtt;
	}

	stat_ptr->stat[stat_ptr->idx].rtt_all += rtt_ms;
	stat_ptr->stat[stat_ptr->idx].rtt_cnt++;

	if (stat_ptr->stat[stat_ptr->idx].rtt_max < rtt_ms)
		stat_ptr->stat[stat_ptr->idx].rtt_max = rtt_ms;
}

/* Statistic Output Package Parameters */
static void stat_pkt_out(struct tcp_sock *sk, struct tcphdr *th,
	struct pkt_stat_swth *stat_ptr, unsigned int len)
{
	if (th == NULL || sk == NULL || stat_ptr == NULL)
		return;

	stat_ptr->idx = idx_check(stat_ptr->idx);
	stat_ptr->stat[stat_ptr->idx].out_pkt++;
	stat_ptr->stat[stat_ptr->idx].out_len += len;

	if (before(ntohl(th->seq), sk->snd_nxt))
		stat_ptr->stat[stat_ptr->idx].rts++;
}

#ifdef CONFIG_APP_QOE_AI_PREDICT
static void update_alpha_filter_factor(void)
{
	int idx_tmp;

	if (g_alpha_filter_change_thresh <= 0) {
		g_alpha_filter_factor = 1;
		return;
	}
	idx_tmp = idx_check(g_app_qoe_stat.idx);
	if (g_app_qoe_stat.stat[idx_tmp].in_pkt >= g_alpha_filter_change_thresh)
		g_alpha_filter_factor = g_alpha_filter_factor_small;
	else
		g_alpha_filter_factor = g_alpha_filter_factor_big;
}

static bool is_app_qoe_level_changed(void)
{
	unsigned int num_max;
	bool is_no_rx_timeout = false;

	num_max = (g_app_qoe_cycle < APP_QOE_MIN_STAT_SEC) ?
		APP_QOE_MIN_STAT_SEC : g_app_qoe_cycle;
	if ((g_no_rx_time_stamp != 0) &&
		((jiffies - g_no_rx_time_stamp) / HZ >=
		(long)num_max * DOUBLE_THRED) && (g_no_rx_num >= num_max))
		is_no_rx_timeout = true;

	if (g_app_qoe_level != NETWORK_STATUS_APP_QOE_GENERAL_SLOW) {
		if ((g_no_rx_num >= DOUBLE_THRED * num_max) ||
			is_no_rx_timeout || (num_max <= g_slow_num))
			return true;
		if (g_no_rx_num > 0) {
			if (g_slow_count >= num_max) {
				pr_info("g_slow_count=%d", g_slow_count);
				return true;
			}
		}
	}
	if (g_app_qoe_level != NETWORK_STATUS_APP_QOE_NORMAL) {
		if (num_max <= g_normal_num)
			return true;
	}
	return false;
}

static bool check_qoe_sum_num(struct pkt_cnt_swth *stat)
{
	int sum_num;
	unsigned int i, j;
	unsigned int loop_max = APP_QOE_MIN_STAT_SEC;
	unsigned int idx_tmp;
	unsigned int *ptr = NULL;
	unsigned int *ptr1 = NULL;

	if (g_app_qoe_cycle > 0) {
		if (g_app_qoe_cycle < APP_QOE_MIN_STAT_SEC)
			loop_max = APP_QOE_MIN_STAT_SEC;
		else if (g_app_qoe_cycle < MAX_STAT_SEC)
			loop_max = g_app_qoe_cycle;
	}

	sum_num = (int)loop_max;
	ptr = &(stat->in_pkt);
	for (i = 0; i < loop_max; i++) {
		idx_tmp = idx_check(g_app_qoe_stat.idx - i);
		ptr1 = &(g_app_qoe_stat.stat[idx_tmp].in_pkt);
		if ((g_app_qoe_stat.stat[idx_tmp].in_pkt +
			g_app_qoe_stat.stat[idx_tmp].out_pkt +
			g_app_qoe_stat.stat[idx_tmp].syn) == 0) {
			sum_num--;
			continue;
		}
		for (j = 0; j < (sizeof(struct pkt_cnt_swth) /
			sizeof(unsigned int)); j++)
			*(ptr + j) += *(ptr1 + j);
	}
	if (sum_num <= 0) {
		pr_info("[AppQoe]%s,sum_num is 0", __func__);
		g_normal_num = 0;
		g_slow_num = 0;
		return false;
	}
	for (j = 0; j < (sizeof(struct pkt_cnt_swth) /
		sizeof(unsigned int)); j++) {
		unsigned int temp = *(ptr + j);

		if (temp >= sum_num || temp == 0)
			*(ptr + j) = temp / sum_num;
		else
			*(ptr + j) = 1;
	}
	return true;
}

static void update_qoe_num(struct pkt_cnt_swth *stat,
	judge_rlt_info_int_calc_type *rlt_info)
{
	if (((stat->out_pkt + stat->syn) > 0) && (stat->in_pkt == 0)) {
		pr_info("[AppQoe]ai_predict no rx");
		if ((stat->out_pkt + stat->syn) >= g_no_rx_thresh)
			g_no_rx_num++;
		g_slow_num = 0;
		g_normal_num = 0;
		rlt_info->is_ps_slow = TRUE;
		if (g_no_rx_num == 1)
			g_no_rx_time_stamp = jiffies;
	} else if (rlt_info->is_ps_slow == TRUE) {
		g_slow_num++;
		g_normal_num = 0;
		g_no_rx_num = 0;
		g_no_rx_time_stamp = 0;
	} else if (rlt_info->is_ps_slow == FALSE) {
		g_normal_num++;
		g_slow_num = 0;
		g_no_rx_num = 0;
		g_no_rx_time_stamp = 0;
	}
}

static void notify_qoe_level_changed(
	struct report_slow_para *report_para,
	judge_rlt_info_int_calc_type *rlt_info)
{
	if (is_app_qoe_level_changed()) {
		g_app_qoe_level = (rlt_info->is_ps_slow == TRUE) ?
			NETWORK_STATUS_APP_QOE_GENERAL_SLOW :
			NETWORK_STATUS_APP_QOE_NORMAL;
		report_para->slow_type = g_app_qoe_level;
		report_para->avg_amp = 0;
		report_para->duration = g_app_qoe_cycle;
		report_para->time_start = 1;
		nb_notify_event(NBMSG_KSI_EVT, report_para,
			sizeof(struct report_slow_para));
		g_normal_num = 0;
		g_slow_num = 0;
		g_no_rx_num = 0;
		g_no_rx_time_stamp = 0;
		pr_info("[AppQoe]slowType=%d\n", report_para->slow_type);
	}
}

static void hidata_app_qoe_ai_predict(void)
{
	int para[PARA_MAX] = {0};
	struct pkt_cnt_swth stat = {0};
	struct report_slow_para report_para = {0};
	/*
	 * point to gIntFixParam[SUB_CLFS_NUM_MAX][4], i.e g_int_fix_param:
	 * global variable.array pointer from clf_params.h.
	 */
	int (*ppg_int_fix_param)[QUADRUPLES] = g_int_fix_param;
	classifier_info_int_exp_type clf_info =
		(classifier_info_int_exp_type){ppg_int_fix_param,
			ADA_SUB_CLFS_NUM};
	/* meaning: FALSE: not fakeBts; 1.0: all as trueBts. */
	judge_rlt_info_int_calc_type rlt_info = {
		FALSE, 0, 0.0, 1.0, 0, ADA_SUB_CLFS_NUM };

	if (!check_qoe_sum_num(&stat))
		return;

	/*
	 * 10 parameters:
	 * RTT,in,inpkt,out,outpkt,dack,rts,syn,3Gsignal,4Gsignal
	 */
	para[PARA_RTT] = stat.rtt;
	para[PARA_IN_LEN] = stat.in_len;
	para[PARA_IN_PKT] = stat.in_pkt;
	para[PARA_OUT_LEN] = stat.out_len;
	para[PARA_OUT_PKT] = stat.out_pkt;
	para[PARA_PACK] = stat.dupack;
	para[PARA_RTS] = stat.rts;
	para[PARA_SYN] = stat.syn;
	para[PARA_DBM] = g_dbm; /* 3Gsingnal */
	para[PARA_RSRP] = g_rsrp;
	pr_info("[AppQoe]ai_predict params[]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		para[PARA_RTT], para[PARA_IN_LEN], para[PARA_IN_PKT],
		para[PARA_OUT_LEN], para[PARA_OUT_PKT], para[PARA_PACK],
		para[PARA_RTS], para[PARA_SYN], para[PARA_DBM],
		para[PARA_RSRP]);

	rlt_info = judge_single_smp_using_clf_info_int_exp(clf_info, para,
		g_app_qoe_level);
	update_qoe_num(&stat, &rlt_info);
	pr_info("[AppQoe]app qoe AI predict result=%d,ps_slow_proba=%ld,"
		"qoe_level=%d,g_slow_num=%d,g_normal_num=%d,g_no_rx_num=%d\n",
		rlt_info.is_ps_slow, rlt_info.ps_slow_proba, g_app_qoe_level,
		g_slow_num, g_normal_num, g_no_rx_num);

	notify_qoe_level_changed(&report_para, &rlt_info);
}
#endif

static void notify_network_normal_or_slow(
	struct report_slow_para *report_para,
	judge_rlt_info_int_calc_type *rlt_info)
{
	bool no_rx_flag = false;
	bool no_slow_flag = false;
	bool no_normal_flag = false;

	no_rx_flag = g_no_rx_count >=
		DOUBLE_THRED * g_network_detect_window &&
		g_network_status != NETWORK_STATUS_NETWORK_SLOW;
	no_slow_flag = g_network_detect_window <=
		g_slow_count &&
		g_network_status != NETWORK_STATUS_NETWORK_SLOW;
	no_normal_flag = g_network_detect_window <=
		g_normal_count && g_network_status !=
		NETWORK_STATUS_NETWORK_NORMAL;
	if (no_rx_flag || no_slow_flag || no_normal_flag) {
		g_network_status  = (rlt_info->is_ps_slow == TRUE) ?
			NETWORK_STATUS_NETWORK_SLOW :
			NETWORK_STATUS_NETWORK_NORMAL;
		report_para->slow_type = g_network_status;
		nb_notify_event(NBMSG_KSI_EVT, report_para,
			sizeof(struct report_slow_para));
		g_normal_count  = 0;
		g_slow_count = 0;
		g_no_rx_count = 0;
		pr_info("slowType=%d\n", report_para->slow_type);
	}
}

static void update_qoe_count(struct pkt_cnt_swth *stat,
	judge_rlt_info_int_calc_type *rlt_info)
{
	if ((stat->out_pkt + stat->syn) > 0 && stat->in_pkt == 0) {
		pr_info("ai_predict no rx");
		g_no_rx_count++;
		g_slow_count = 0;
		g_normal_count = 0;
		rlt_info->is_ps_slow = TRUE;
	} else if (rlt_info->is_ps_slow == TRUE) {
		g_slow_count++;
		g_normal_count = 0;
		g_no_rx_count = 0;
	} else if (rlt_info->is_ps_slow == FALSE) {
		g_normal_count++;
		g_slow_count = 0;
		g_no_rx_count = 0;
	}
}
static bool check_sum_num(struct pkt_cnt_swth *stat)
{
	int sum_num;
	unsigned int i, j;
	unsigned int loop_max = 5;
	unsigned int idx_tmp;
	unsigned int *ptr = NULL;
	unsigned int *ptr1 = NULL;

	sum_num = (int)loop_max;
	ptr = &(stat->in_pkt);

	for (i = 1; i <= loop_max; i++) {
		idx_tmp = idx_check(g_stat.idx - i);
		ptr1 = &(g_stat.stat[idx_tmp].in_pkt);
		if ((g_stat.stat[idx_tmp].in_pkt +
			g_stat.stat[idx_tmp].out_pkt +
			g_stat.stat[idx_tmp].syn) == 0) {
			sum_num--;
			continue;
		}
		for (j = 0; j < (sizeof(struct pkt_cnt_swth) /
			sizeof(unsigned int)); j++)
			*(ptr + j) += *(ptr1 + j);
	}

	if (sum_num <= 0) {
		g_normal_count  = 0;
		g_slow_count  = 0;
		g_no_rx_count  = 0;
		return false;
	}

	for (j = 0; j < (sizeof(struct pkt_cnt_swth) /
		sizeof(unsigned int)); j++) {
		unsigned int temp = *(ptr + j);

		if (temp >= sum_num || temp == 0)
			*(ptr + j) = temp / sum_num;
		else
			*(ptr + j) = 1;
	}
	return true;
}

static void predict_network_normal_or_slow(void)
{
	int para[PARA_MAX] = {0};
	struct pkt_cnt_swth stat = {0};
	struct report_slow_para report_para = {0};
	/*
	 * point to gIntFixParam[SUB_CLFS_NUM_MAX][4], i.e g_int_fix_param:
	 * global variable. array pointer from clf_params.h.
	 */
	int (*ppg_int_fix_param)[QUADRUPLES] = g_int_fix_param;
	classifier_info_int_exp_type clf_info =
		(classifier_info_int_exp_type){ppg_int_fix_param,
			ADA_SUB_CLFS_NUM};
	/* meaning: FALSE: not fakeBts; 1.0: all as trueBts. */
	judge_rlt_info_int_calc_type rlt_info = {
		FALSE, 0, 0.0, 1.0, 0, ADA_SUB_CLFS_NUM };

	if (!check_sum_num(&stat))
		return;

	/*
	 * 10 parameters:
	 * RTT,in,inpkt,out,outpkt,dack,rts,syn,3Gsignal,4Gsignal
	 */
	para[PARA_RTT] = stat.rtt;
	para[PARA_IN_LEN] = stat.in_len;
	para[PARA_IN_PKT] = stat.in_pkt;
	para[PARA_OUT_LEN] = stat.out_len;
	para[PARA_OUT_PKT] = stat.out_pkt;
	para[PARA_PACK] = stat.dupack;
	para[PARA_RTS] = stat.rts;
	para[PARA_SYN] = stat.syn;
	para[PARA_DBM] = g_dbm; /* 3Gsingnal */
	para[PARA_RSRP] = g_rsrp;

	rlt_info = judge_single_smp_using_clf_info_int_exp(clf_info, para,
		g_network_status);
	pr_info("params[]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,ps_slow_proba=%ld\n",
		para[PARA_RTT], para[PARA_IN_LEN], para[PARA_IN_PKT],
		para[PARA_OUT_LEN], para[PARA_OUT_PKT], para[PARA_PACK],
		para[PARA_RTS], para[PARA_SYN], para[PARA_DBM], para[PARA_RSRP],
		rlt_info.ps_slow_proba);

	update_qoe_count(&stat, &rlt_info);
	notify_network_normal_or_slow(&report_para, &rlt_info);
}

/* Update the index for the statistics array */
static unsigned int idx_update(struct pkt_stat_swth *stat_ptr)
{
	unsigned long time_stamp;
	unsigned long time_stamp_tmp;
	unsigned int index;
	unsigned int index_tmp;
	int cnt;

	if (stat_ptr == NULL)
		return S_OLD_SECOND;

	time_stamp = jiffies / HZ;
	time_stamp_tmp = time_stamp;
	index = time_stamp % MAX_STAT_SEC;
	index_tmp = index;

	if (index != stat_ptr->idx || time_stamp != stat_ptr->time_stamp) {
		for (cnt = 1; cnt <= time_stamp - stat_ptr->time_stamp; cnt++) {
			index_tmp = (stat_ptr->idx + cnt) % MAX_STAT_SEC;
			time_stamp_tmp = stat_ptr->time_stamp + cnt;

			memset(&(stat_ptr->stat[index_tmp]), 0,
				sizeof(struct pkt_cnt_swth));

			if (time_stamp_tmp == time_stamp || cnt >= MAX_STAT_SEC)
				break;
		}
		ksi_update(stat_ptr);

#ifdef CONFIG_APP_QOE_AI_PREDICT
		if (&g_app_qoe_stat == stat_ptr) {
			update_alpha_filter_factor();
			hidata_app_qoe_ai_predict();
		}
#endif
		if (&g_stat == stat_ptr)
			predict_network_normal_or_slow();
		stat_ptr->idx = index;
		stat_ptr->time_stamp = time_stamp;

		return S_NEW_SECOND;
	}

	return S_OLD_SECOND;
}

/* Local out hook function */
static unsigned int hook_out(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	char *tcp_data = NULL;
	u32 total_len;
	u32 ip_hdr_len;
	u32 data_len;

	if (skb == NULL || g_hook_flag == S_HOOK_DISABLE)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;

	if (iph->protocol == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		if (tcph == NULL || skb->data == NULL || tcph->doff == 0)
			return NF_ACCEPT;

		tcp_data = (char *)((u32 *)tcph + tcph->doff);
		if (!virt_addr_valid((uintptr_t)tcp_data) &&
			!virt_addr_valid((uintptr_t)(tcp_data + MAX_PKT_LEN)))
			return NF_ACCEPT;

		total_len = skb->len - skb->data_len;
		ip_hdr_len = (tcp_data - (char *)iph);
		data_len = total_len - ip_hdr_len;

		if (total_len > MAX_PKT_LEN || ip_hdr_len > MAX_PKT_LEN ||
			data_len > MAX_PKT_LEN || skb->dev == NULL ||
			skb->dev->name == NULL)
			return NF_ACCEPT;

		if (strncmp(skb->dev->name, DS_NET, DS_NET_LEN) != 0 &&
			strncmp(skb->dev->name, DS_NET_SLAVE, DS_NET_LEN) != 0)
			return NF_ACCEPT;

		/* When the lock is not locked, the lock is triggered */
		if (skb->sk == NULL || !spin_trylock_bh(&g_smart_switch_lock))
			return NF_ACCEPT;

		if (skb->sk->sk_state == TCP_ESTABLISHED) {
			idx_update(&g_stat);
			stat_pkt_out((struct tcp_sock *)skb->sk, tcph, &g_stat,
				data_len);
		} else if (skb->sk->sk_state == TCP_SYN_SENT) {
			if (tcph->syn == 1) /* 【SYN】package */
				g_stat.stat[g_stat.idx].syn++;
		}
#ifdef CONFIG_APP_QOE_AI_PREDICT
		update_tcp_app_qoe_hook_out(skb, tcph, data_len);
#endif
		spin_unlock_bh(&g_smart_switch_lock);
	}
	return NF_ACCEPT;
}

/* Local in hook function */
static unsigned int hook_in(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	char *tcp_data = NULL;
	u32 total_len;
	u32 ip_hdr_len;
	u32 data_len;

	if (skb == NULL || g_hook_flag == S_HOOK_DISABLE)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;

	if (iph->protocol == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		if (tcph == NULL || skb->data == NULL || tcph->doff == 0)
			return NF_ACCEPT;

		tcp_data = (char *)((u32 *)tcph + tcph->doff);
		if (!virt_addr_valid((uintptr_t)tcp_data) &&
			!virt_addr_valid((uintptr_t)(tcp_data + MAX_PKT_LEN)))
			return NF_ACCEPT;

		if (skb->dev == NULL || skb->dev->name == NULL)
			return NF_ACCEPT;

		if (strncmp(skb->dev->name, DS_NET, DS_NET_LEN) != 0 &&
			strncmp(skb->dev->name, DS_NET_SLAVE, DS_NET_LEN) != 0)
			return NF_ACCEPT;

		g_no_rx_time_stamp = 0;

		if (skb->sk == NULL)
			return NF_ACCEPT;

		total_len = skb->len - skb->data_len;
		ip_hdr_len = (tcp_data - (char *)iph);
		data_len = total_len - ip_hdr_len;

		if (total_len > MAX_PKT_LEN || ip_hdr_len > MAX_PKT_LEN ||
			data_len > MAX_PKT_LEN)
			return NF_ACCEPT;

		if (skb->sk->sk_state != TCP_ESTABLISHED ||
			!spin_trylock_bh(&g_smart_switch_lock))
			return NF_ACCEPT;

		idx_update(&g_stat);
		stat_pkt_in((struct tcp_sock *)skb->sk, tcph, &g_stat,
			data_len);
#ifdef CONFIG_APP_QOE_AI_PREDICT
		update_tcp_app_qoe_hook_in(skb, tcph, data_len);
#endif
		spin_unlock_bh(&g_smart_switch_lock);
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = hook_in,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook_out,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER - 1,
	}
};

/* Set KSI and hook-and-package points to enable */
int set_ksi_enable(uint8_t nf_hook_enable, uint8_t nl_event_enable)
{
	spin_lock_bh(&g_smart_switch_lock);
	g_report_flag = nl_event_enable;
	g_hook_flag = nf_hook_enable;
	spin_unlock_bh(&g_smart_switch_lock);

	return 0;
}

#ifdef CONFIG_APP_QOE_AI_PREDICT
int set_app_qoe_uid(int uid, int period)
{
	pr_info("[AppQoe]%s uid=%d,period=%d\n", __func__, uid, period);
	spin_lock_bh(&g_smart_switch_lock);
	if (g_app_qoe_uid != uid) {
		g_app_qoe_uid = uid;
		memset(&g_app_qoe_stat, 0, sizeof(g_app_qoe_stat));
		g_app_qoe_level = NETWORK_STATUS_INVALID;
		g_app_rtt = 0;
		g_slow_num = 0;
		g_no_rx_num = 0;
		g_normal_num = 0;
		g_no_rx_time_stamp = 0;
		g_app_qoe_cycle = 0;
	}
	if ((period / US_MS) > g_app_qoe_cycle)
		g_app_qoe_cycle = period / US_MS;
	spin_unlock_bh(&g_smart_switch_lock);
	return 0;
}

int set_app_qoe_rsrp(int rsrp, int rsrq)
{
	g_rsrp = rsrp;
	g_dbm = rsrq;
	return 0;
}

void set_alpha_filter_alg_params(int filter_factor_small,
	int filter_factor_big)
{
	if ((filter_factor_small > 0) && (filter_factor_small < FILT_16) &&
		(filter_factor_big > 0) && (filter_factor_big < FILT_16)) {
		pr_info("[AppQoe]%s %d,%d\n", __func__,
			filter_factor_small, filter_factor_big);
		g_alpha_filter_factor_small = filter_factor_small;
		g_alpha_filter_factor_big = filter_factor_big;
	}
}

void set_alpha_filter_alg_change_thresh(int threshold)
{
	pr_info("[AppQoe]%s %d\n", __func__, threshold);
	g_alpha_filter_change_thresh = threshold;
}
#endif

/* initialization function */
int smart_switch_init(void)
{
	int ret;

	g_report_time = jiffies;
	g_report_flag = S_MODULE_DISABLE;
	g_hook_flag = S_HOOK_DISABLE;
	g_rtt = 0;
	memset(&g_stat, 0, sizeof(g_stat));
	memset(&g_swth_report, 0, sizeof(g_swth_report));
#ifdef CONFIG_APP_QOE_AI_PREDICT
	g_app_rtt = 0;
	memset(&g_app_qoe_stat, 0, sizeof(g_app_qoe_stat));
#endif
	spin_lock_init(&g_smart_switch_lock);
	/* Registration hook function */
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	ret = nf_register_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	ret = nf_register_net_hooks(&init_net,
		net_hooks, ARRAY_SIZE(net_hooks));
#endif
	if (ret) {
		pr_err("KSI init fail ret=%d\n", ret);
		return ret;
	}

#ifdef CONFIG_CHR_NETLINK_MODULE
	/* Timer initialization */
	init_timer(&g_chr_rpt_timer);
	g_chr_rpt_timer.data = 0;
	g_chr_rpt_timer.function = chr_report;
	g_chr_rpt_timer.expires = jiffies + CHR_REPORT_TIMER;
	memset(&g_chr_report, 0, sizeof(g_chr_report));
#endif

	pr_info("KSI init success\n");
	return 0;
}

/* Exit function */
void smart_switch_exit(void)
{
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	nf_unregister_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	nf_unregister_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
#endif
	pr_err("KSI exit success\n");
}

#undef DEBUG
