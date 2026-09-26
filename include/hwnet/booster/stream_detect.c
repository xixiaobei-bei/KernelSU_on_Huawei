/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This module is to detect stream abnormal.
 * Author: tongxilin@huawei.com
 * Create: 2020-02-20
 */

#include "stream_detect.h"

#include <linux/net.h>
#include <linux/ip.h>
#include <net/sock.h>
#include <net/tcp.h>

static struct stream_ctx *g_stream_ctx;

/* add_list return pos. */
static struct stream_list* add_list(struct list_head *head,
	struct stream_info info)
{
	struct stream_list *pos = NULL;

	pr_info("[STREAM_DETECT]%s", __func__);
	pos = kmalloc(sizeof(struct stream_list), GFP_ATOMIC);
	if (pos == NULL)
		return NULL;
	memset(pos, 0, sizeof(struct stream_list));
	pos->stat.info.uid = info.uid;
	pos->stat.info.dest_port = info.dest_port;
	pos->stat.info.src_port = info.src_port;
	pos->stat.info.family = info.family;
	pos->stat.info.protocol = info.protocol;
	if (info.family == AF_INET) {
		pos->stat.info.dest_addr.ip4_addr = info.dest_addr.ip4_addr;
		pos->stat.info.src_addr.ip4_addr = info.src_addr.ip4_addr;
	} else {
		pos->stat.info.dest_addr.ip6_addr = info.dest_addr.ip6_addr;
		pos->stat.info.src_addr.ip6_addr = info.src_addr.ip6_addr;
	}
	list_add(&pos->head, head); // add to list.
	return pos;
}

static bool is_first_video_download(struct tcp_sock *tp)
{
	u32 delta_us;

	if (tp->tcp_mstamp == 0 || tp->rcv_rtt_est.time == 0)
		return true;
	delta_us = tcp_stamp_us_delta(tp->tcp_mstamp, tp->rcv_rtt_est.time);
	if (delta_us > MAX_DELTA_US)
		return false;
	return true;
}

static bool is_param_valid(struct sock *sk, u8 *str, u32 len, struct stream_info info)
{
	if (sk == NULL || str == NULL)
		return false;

	if (ntohl(info.dest_addr.ip4_addr) == LOOP_IP)
		return false;

	if (info.dest_port != VIDEO_HTTP_PORT)
		return false;

	if (len < MIN_HTTP_LEN || len > MAX_HTTP_LEN)
		return false;

	if (len > MAX_DATA_LEN)
		len = MAX_DATA_LEN;

	if (strncmp(str, STR_HTTP, STR_HTTP_LEN) != 0)
		return false;

	if (str[VIDEO_HTTP_ACK_FROM_START] != '2')
		return false;

	return true;
}

/* stream type identification maintenance function */
static u32 get_stream_type (struct sock *sk, u8 *str, u32 len, struct stream_info info)
{
	u32 cnt;
	u32 stream_type = DEFALUT_FLOW;
	u8 *content_length_start = NULL;
	unsigned long long content_length = 0;
	struct tcp_sock *tp = tcp_sk(sk);

	if (!is_param_valid(sk, str, len, info))
		return DEFALUT_FLOW;

	for (cnt = 0; cnt < len - CONTENT_TYPE_LEN; cnt++) {
		if (str[cnt] == ASCII_CR && str[cnt + 1] == ASCII_LF) {
			cnt += 2; // jump CR\LF.
			if (cnt >= len - CONTENT_TYPE_LEN) {
				break;
			} else if (str[cnt] == ASCII_CR &&
				str[cnt + 1] == ASCII_LF) {
				break;
			} else if (strncmp(&str[cnt], CONTENT_TYPE_VIDEO1,
				CONTENT_TYPE_LEN) == 0 ||
				strncmp(&str[cnt], CONTENT_TYPE_VIDEO2,
				CONTENT_TYPE_LEN) == 0) {
				stream_type = VIDEO_FLOW;
			} else if (strncmp(&str[cnt], CONTENT_TYPE_IMAGE1,
				CONTENT_TYPE_LEN) == 0 ||
				strncmp(&str[cnt], CONTENT_TYPE_IMAGE2,
				CONTENT_TYPE_LEN) == 0) {
				stream_type = IMAGE_FLOW;
				break;
			} else if (strncmp(&str[cnt], CONTENT_LENGTH,
				CONTENT_LENGTH_LEN) == 0) {
				if (stream_type != VIDEO_FLOW)
					break;

				if (!is_first_video_download(tp))
					break;

				content_length_start = &str[cnt + CONTENT_LENGTH_LEN];
				content_length = simple_strtoull(content_length_start, NULL, DECIMAL_BASE);
				tp->content_length = content_length + tp->bytes_received;
				break;
			} else {
				continue;
			}
		} else if (str[cnt + 1] > MAX_ASCII) {
			break;
		} else {
			continue;
		}
	}
	return stream_type;
}

bool is_target_stream(struct sock *sk, u8 *str, u32 len, u32 stream_type, struct stream_info info)
{
	return get_stream_type(sk, str, len, info) == stream_type ? 1 : 0;
}

static bool is_match_stream(struct stream_info skb_info, struct stream_info info)
{
	if (skb_info.uid != info.uid)
		return false;
	if (skb_info.src_port != info.src_port || skb_info.dest_port != info.dest_port)
		return false;
	if (skb_info.family == AF_INET)
		return (skb_info.dest_addr.ip4_addr == info.dest_addr.ip4_addr);
	else
		return ipv6_addr_equal(&skb_info.dest_addr.ip6_addr, &info.dest_addr.ip6_addr);
}

static char* get_ip_addr(struct sk_buff *skb, struct stream_info *info,
	u16 family, u16 hook_type)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *iph6 = NULL;
	char *piph = NULL;

	if (skb == NULL)
		return NULL;
	if (family == AF_INET) {
		iph = ip_hdr(skb);
		if (iph == NULL)
			return NULL;
		if (hook_type == HOOK_IN) {
			info->dest_addr.ip4_addr = iph->saddr;
			info->src_addr.ip4_addr = iph->daddr;
		} else {
			info->dest_addr.ip4_addr = iph->daddr;
			info->src_addr.ip4_addr = iph->saddr;
		}
		piph = (char *)iph;
	} else {
		iph6 = ipv6_hdr(skb);
		if (iph6 == NULL)
			return NULL;
		piph = (char *)iph6;
		if (hook_type == HOOK_IN) {
			info->dest_addr.ip6_addr = iph6->saddr;
			info->src_addr.ip6_addr = iph6->daddr;
		} else {
			info->dest_addr.ip6_addr = iph6->daddr;
			info->src_addr.ip6_addr = iph6->saddr;
		}
	}
	return piph;
}

static struct sock *get_suit_sk(struct sk_buff *skb)
{
	struct sock *sk = NULL;

	if (skb == NULL)
		return NULL;
	if (skb->dev == NULL || skb->dev->name == NULL)
		return NULL;
	sk = skb_to_full_sk(skb);
	if (sk == NULL || (!sk_fullsock(sk)))
		return NULL;
	if (sk->sk_protocol != IPPROTO_TCP)
		return NULL;
	if (sk->sk_state != TCP_SYN_SENT && sk->sk_state != TCP_ESTABLISHED) // sk_state
		return NULL;
	return sk;
}

static u32 get_tcp_len(struct sk_buff *skb, char const *tcp_data, char const *piph)
{
	u32 total_len;
	u32 ip_hdr_len;
	u32 tcp_len;

	total_len = skb->len - skb->data_len;
	ip_hdr_len = (tcp_data - piph);
	tcp_len = total_len - ip_hdr_len;
	return tcp_len;
}

static char* get_tcp_data(struct sk_buff *skb, struct stream_info *info,
	u16 hook_type)
{
	struct tcphdr *tcph = NULL;
	char *tcp_data = NULL;

	tcph = tcp_hdr(skb);
	if (tcph == NULL || skb->data == NULL)
		return NULL;
	tcp_data = (char *)((u32 *)tcph + tcph->doff);
	if (!virt_addr_valid(tcp_data))
		return NULL;
	if (hook_type == HOOK_OUT) {
		info->dest_port = (u32)htons(tcph->dest);
		info->src_port = (u32)htons(tcph->source);
	} else {
		info->dest_port = (u32)htons(tcph->source);
		info->src_port = (u32)htons(tcph->dest);
	}
	return tcp_data;
}

static struct stream_list *process_new_stream( struct sock *sk,
	struct stream_stat *stream, struct stream_info info, char *tcp_data,
	u32 tcp_len)
{
	struct stream_list *pos_selec = NULL;

	if (!is_target_stream(sk, tcp_data, tcp_len, g_stream_ctx->stream_type, info))
		return NULL;
	if (stream->cnt >= g_stream_ctx->stream_cnt)
		return NULL;
	pos_selec = add_list(&stream->head, info);
	if (pos_selec == NULL)
		return NULL;
	return pos_selec;
}

static struct stream_list *find_match_entry(struct stream_stat *stream,
	struct stream_info info)
{
	struct stream_list *pos = NULL;
	struct stream_list *pos_selec = NULL;

	list_for_each_entry(pos, &stream->head, head) {
		if (is_match_stream(info, pos->stat.info)) {
			pos_selec = pos;
			break;
		} else {
			continue;
		}
	}
	return pos_selec;
}

static void update_tcp_in_para(struct stream_res *stat,
	struct sk_buff *skb, struct sock *sk, u32 tcp_len)
{
	struct tcp_sock *tp = NULL;
	struct tcphdr *th = tcp_hdr(skb);

	if (stat == NULL)
		return;
	if (sk == NULL)
		return;
	tp = (struct tcp_sock *)sk;
	stat->in_pkt++;
	stat->in_len += skb->len;
	if (stat->in_len > MAX_U32_VALUE)
		stat->in_len = MAX_U32_VALUE;
	update_stats_srtt(tp, &stat->rtt);
	if (before(ntohl(th->seq) + tcp_len - 1, tp->rcv_nxt) &&
		tcp_len > 0)
		stat->in_rts_pkt++;
}

void stream_process_hook_in(struct sk_buff *skb, u16 family)
{
	char *piph = NULL;
	char *tcp_data = NULL;
	struct sock *sk = NULL;
	struct stream_stat *stream = NULL;
	struct stream_list *pos_selec = NULL;
	u32 tcp_len;
	struct stream_info info;
	if (family != AF_INET && family != AF_INET6)
		return;
	sk = get_suit_sk(skb);
	if (sk == NULL)
		return;
	info.uid = (u32)sk->sk_uid.val;
	if (info.uid != g_stream_ctx->uid)
		return;
	piph = get_ip_addr(skb, &info, family, HOOK_IN);
	if (piph == NULL)
		return;
	tcp_data = get_tcp_data(skb, &info, HOOK_IN);
	if (tcp_data == NULL)
		return;
	tcp_len = get_tcp_len(skb, tcp_data, piph);

	info.protocol = sk->sk_protocol;
	info.family = family;
	stream = g_stream_ctx->cur;
	spin_lock_bh(&stream->lock);
	pos_selec = find_match_entry(stream, info);
	if (pos_selec == NULL) {
		pos_selec = process_new_stream(sk, stream, info, tcp_data, tcp_len);
		if (pos_selec == NULL)
			goto unlock;
		stream->cnt = stream->cnt + 1;
	}
	update_tcp_in_para(&pos_selec->stat, skb, sk, tcp_len);
unlock:
	spin_unlock_bh(&stream->lock);
}

static void update_tcp_out_para(struct stream_res *stat,
	struct sk_buff *skb, struct sock *sk)
{
	struct tcphdr *th = NULL;
	struct tcp_sock *tp = NULL;

	if (sk == NULL || skb == NULL || stat == NULL)
		return;
	th = tcp_hdr(skb);
	if (th == NULL)
		return;

	tp = tcp_sk(sk);
	if (sk->sk_state == TCP_ESTABLISHED) {
		stat->out_pkt++;
		stat->out_len += skb->len;
	} else if (sk->sk_state == TCP_SYN_SENT) {
		if (th->syn == 1)
			stat->tcp_syn_pkt++;
	}
	if (tp->snd_nxt != 0 &&
		before(ntohl(th->seq), tp->snd_nxt) &&
		(sk->sk_state == TCP_ESTABLISHED ||
		sk->sk_state == TCP_SYN_SENT))
		stat->out_rts_pkt++;
}

void stream_process_hook_out(struct sk_buff *skb, u16 family)
{
	char *piph = NULL;
	char *tcp_data = NULL;
	struct sock *sk = NULL;
	struct stream_stat *stream = NULL; // stream list info
	struct stream_list *pos_selec = NULL;
	struct stream_info info; // skb info.

	if (skb == NULL)
		return;
	if (family != AF_INET && family != AF_INET6)
		return;
	sk = get_suit_sk(skb);
	if (sk == NULL)
		return;
	info.uid = (u32)sk->sk_uid.val;
	if (info.uid != g_stream_ctx->uid)
		return;
	piph = get_ip_addr(skb, &info, family, HOOK_OUT);
	if (piph == NULL)
		return;
	tcp_data = get_tcp_data(skb, &info, HOOK_OUT);
	if (tcp_data == NULL)
		return;
	info.protocol = sk->sk_protocol;
	info.family = family;
	stream = g_stream_ctx->cur;
	spin_lock_bh(&stream->lock);
	pos_selec = find_match_entry(stream, info);
	if (pos_selec != NULL)
		update_tcp_out_para(&pos_selec->stat, skb, sk);
	spin_unlock_bh(&stream->lock);
}

static void reset_stat(struct stream_res *entry)
{
	entry->in_pkt = 0;
	entry->in_len = 0;
	entry->out_pkt = 0;
	entry->out_len = 0;
	entry->out_dupack_pkt = 0;
	entry->out_rts_pkt = 0;
	entry->rtt = 0;
	entry->tcp_syn_pkt = 0;
	entry->in_rts_pkt = 0;
}

static void update_report_stat(struct stream_stat *cur,
	struct stream_collect_res *res)
{
	struct stream_list *pos = NULL;
	int i = 0;

	res->cnt = cur->cnt;
	list_for_each_entry(pos, &cur->head, head) {
		memcpy(&res->stream[i], &pos->stat, sizeof(struct stream_res));
		reset_stat(&pos->stat);
		i++;
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void report_stat(unsigned long sync)
#else
static void report_stat(struct timer_list* sync)
#endif
{
	char *msg = NULL;
	struct stream_stat *stream = NULL;
	struct stream_collect_res *stream_res = NULL;
	struct stream_res_msg res_msg;

	memset(&res_msg, 0, sizeof(struct stream_res_msg));
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_stream_ctx->report_timer.data = sync + 1; // number of reports
#endif
	g_stream_ctx->report_timer.function = report_stat;
	mod_timer(&g_stream_ctx->report_timer,
		jiffies + g_stream_ctx->report_expires);
	stream = g_stream_ctx->cur;
	stream_res = &res_msg.res;
	spin_lock_bh(&stream->lock);
	update_report_stat(stream, stream_res);
	spin_unlock_bh(&stream->lock);

	res_msg.type = STREAM_DETECTION_RPT;
	res_msg.len = 8 + stream_res->cnt * sizeof(struct stream_res); // 8 is size of type,len,cnt.
	msg = (char *)&res_msg;
	if (g_stream_ctx->fn)
		g_stream_ctx->fn((struct res_msg_head *)msg);
}

static void clear_list(struct list_head *head)
{
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	struct stream_list *pos = NULL;

	list_for_each_safe(p, n, head) {
		pos = list_entry(p, struct stream_list, head);
		list_del_init(p);
		if (pos == NULL)
			return;
		kfree(pos);
	}
	head->prev = head;
	head->next = head;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void reset_list(unsigned long sync)
#else
static void reset_list(struct timer_list* sync)
#endif
{
	struct stream_stat *stream = NULL;

	pr_info("[STREAM_DETECT]%s", __func__);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_stream_ctx->list_timer.data = sync + 1;
#endif
	g_stream_ctx->list_timer.function = reset_list;
	mod_timer(&g_stream_ctx->list_timer,
		jiffies + g_stream_ctx->list_expires);
	stream = g_stream_ctx->cur;
	spin_lock_bh(&stream->lock);
	clear_list(&stream->head);
	stream->cnt = 0;
	spin_unlock_bh(&stream->lock);
}

static void stream_para_report_start(struct req_msg_head *msg)
{
	struct stream_req_msg *req = NULL;
	u32 report_expires, list_expires;
	if (msg->len < sizeof(struct stream_req_msg))
		return;
	req = (struct stream_req_msg *)msg;
	if (req->stream_cnt > MAX_STREAM_LIST_LEN)
		return;
	report_expires = req->report_expires;
	list_expires = req->list_expires;

	pr_info("[STREAM_DETECT]%s,uid=%u,stream_type=%u,report_expires=%u,"
		"list_expires=%u,res=%u", __func__, req->uid, req->stream_type,
		req->report_expires, req->list_expires, req->res);

	spin_lock_bh(&g_stream_ctx->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_stream_ctx->list_timer.data = 0;
#endif
	g_stream_ctx->list_timer.function = reset_list;
	g_stream_ctx->list_expires = list_expires / JIFFIES_MS;
	mod_timer(&g_stream_ctx->list_timer,
		jiffies + g_stream_ctx->list_expires);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_stream_ctx->report_timer.data = 0;
#endif
	g_stream_ctx->report_timer.function = report_stat;
	g_stream_ctx->report_expires = report_expires / JIFFIES_MS;
	mod_timer(&g_stream_ctx->report_timer,
		jiffies + g_stream_ctx->report_expires);

	g_stream_ctx->uid = req->uid;
	g_stream_ctx->stream_cnt = req->stream_cnt;
	g_stream_ctx->stream_type = req->stream_type;
	spin_unlock_bh(&g_stream_ctx->lock);
}

static void stream_para_report_stop()
{
	struct stream_stat *stream = NULL;

	pr_info("[STREAM_DETECT]%s", __func__);
	if (!timer_pending(&g_stream_ctx->list_timer))
		return;
	del_timer(&g_stream_ctx->list_timer);
	if (!timer_pending(&g_stream_ctx->report_timer))
		return;
	del_timer(&g_stream_ctx->report_timer);

	stream = g_stream_ctx->cur;
	spin_lock_bh(&stream->lock);
	clear_list(&stream->head);
	stream->cnt = 0;
	spin_unlock_bh(&stream->lock);
}

static void cmd_process(struct req_msg_head *msg)
{
	if (!msg) {
		pr_err("%s,msg is NULL \n", __func__);
		return;
	}
	if (msg->type == STREAM_DETECTION_START)
		stream_para_report_start(msg);
	else if (msg->type == STREAM_DETECTION_STOP)
		stream_para_report_stop();
}

/*
 * Initialize internal variables and external interfaces
 */
msg_process *stream_detect_init(notify_event *fn)
{
	if (fn == NULL)
		return NULL;

	pr_info("[STREAM_DETECT]%s", __func__);
	g_stream_ctx = kmalloc(sizeof(struct stream_ctx), GFP_KERNEL);
	if (g_stream_ctx == NULL)
		return NULL;
	memset(g_stream_ctx, 0, sizeof(struct stream_ctx));
	g_stream_ctx->fn = fn;
	g_stream_ctx->cur = kmalloc(sizeof(struct stream_stat), GFP_KERNEL);
	if (g_stream_ctx->cur == NULL)
		goto init_error;
	memset(g_stream_ctx->cur, 0, sizeof(struct stream_stat));

	spin_lock_init(&g_stream_ctx->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	init_timer(&g_stream_ctx->list_timer);
	init_timer(&g_stream_ctx->report_timer);
#else
	timer_setup(&g_stream_ctx->list_timer, reset_list, 0);
	timer_setup(&g_stream_ctx->report_timer, report_stat, 0);
#endif
	spin_lock_init(&g_stream_ctx->cur->lock);
	g_stream_ctx->cur->head.prev = &g_stream_ctx->cur->head;
	g_stream_ctx->cur->head.next = &g_stream_ctx->cur->head;
	return cmd_process;

init_error:
	if (g_stream_ctx != NULL)
		kfree(g_stream_ctx);
	g_stream_ctx = NULL;
	return NULL;
}

void stream_detect_exit(void)
{
	if (g_stream_ctx->cur != NULL)
		kfree(g_stream_ctx->cur);
	if (g_stream_ctx != NULL)
		kfree(g_stream_ctx);
	g_stream_ctx = NULL;
}
