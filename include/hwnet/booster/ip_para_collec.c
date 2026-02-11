/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
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

static struct app_ctx *g_app_ctx;

static void __send_buf_to_chr(struct tcp_collect_res *tcp)
{
	int i;
	struct tcp_res *res = tcp->res;
	u32 tcp_buf = 0;
	u32 tcp_out_pkt = 0;
	u32 udp_buf = 0;
	u32 out_udp_pkt = 0;

	for (i = 0; i < tcp->cnt; i++) {
		if (res[i].uid == 0)
			continue;
		tcp_buf += res[i].tcp_buf;
		udp_buf += res[i].udp_buf;
		out_udp_pkt += res[i].out_udp_pkt;
		tcp_out_pkt += res[i].out_pkt;
	}
	if (tcp_out_pkt == 0)
		tcp_out_pkt = 1;
	if (out_udp_pkt == 0)
		out_udp_pkt = 1;
	pr_info("[IP_PARA]tcp buf:%d,udp buf:%d\n", tcp_buf / tcp_out_pkt,
		udp_buf / out_udp_pkt);
}

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
static u32 match_app(struct sock *sk, struct tcp_res *stat,
	u8 protocal, u32 direc)
{
	u32 uid;
	u32 pid;

	// udp_in、forward
	if (stat->uid == 0)
		return BACKGROUND;
	if (sk == NULL)
		return FORGROUND_UNMACH;
	if ((protocal == IPPROTO_TCP) && (!sk_fullsock(sk)))
		return FORGROUND_UNMACH; // ignore timewait or request socket
	uid = get_sock_uid(sk);
#if defined(CONFIG_HUAWEI_KSTATE) || defined(CONFIG_MPTCP)
	if (stat->pid == 0) {
		if (uid == stat->uid)
			return FORGROUND_MACH; // udp_out
	} else {
		// add in any pid if pid=0 is not existence udp_out
		if (protocal == IPPROTO_UDP && uid == stat->uid)
			return BACKGROUND;
		// tcp_in
		if (direc == NF_INET_LOCAL_IN)
			pid = 0;
		else
			pid = get_sock_pid(sk);
		// tcp_out
		if (uid == stat->uid && pid == stat->pid)
			return FORGROUND_MACH;
	}
#else
	if (uid == stat->uid)
		return FORGROUND_MACH;
#endif

	return FORGROUND_UNMACH;
}
/*
 * tcp pkt_in pid=0(default),uid is ok, pkt_out pid,uid is ok.
 */
static void update_tcp_para(struct tcp_res *stat, struct sk_buff *skb,
	struct sock *sk, const struct nf_hook_state *state, u16 payload_len)
{
	u8 direc = (u8)state->hook;
	struct tcphdr *th = tcp_hdr(skb);
	struct tcp_sock *skt;
	u32 rtt_ms;
	s32 tcp_len;

	if (direc == NF_INET_FORWARD) // sk == NULL
		stat->drop_pkt++;
	if (sk == NULL)
		return;
	if (th == NULL)
		return;
	if (direc == NF_INET_BUFHOOK) {
		stat->tcp_buf += ((struct nf_hook_state_ex *)state)->buf;
		return;
	}
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;
	skt = (struct tcp_sock *)sk;
	tcp_len = payload_len - th->doff * 4; // 4 is 32bits to byte
	if (tcp_len < 0)
		return;
	if (direc == NF_INET_LOCAL_IN) {
		if (sk->sk_state != TCP_ESTABLISHED)
			return;
		stat->in_pkt++;
		stat->in_len += skb->len;
		if (before(ntohl(th->seq), skt->rcv_nxt) && tcp_len > 0)
			stat->in_rts_pkt++;
		rtt_ms = (skt->srtt_us) / US_MS / 8; // srtt translate to rtt
		stat->rtt += (rtt_ms > MAX_RTT) ? MAX_RTT : rtt_ms;
	} else if (direc == NF_INET_POST_ROUTING) {
		if (sk->sk_state == TCP_ESTABLISHED) {
			stat->out_pkt++;
			stat->out_len += skb->len;
		} else if (sk->sk_state == TCP_SYN_SENT) {
			if (th->syn == 1)
				stat->tcp_syn_pkt++;
		}
		if (before(ntohl(th->seq), skt->snd_nxt))
			stat->out_rts_pkt++;
	} else if (direc == NF_INET_LOCAL_OUT) {
		stat->drop_pkt++;
	}
}
/*
 * udp udp_in uid=0, udp_out pid=0(or any pid,uid is ok),uid is ok
 */
void update_udp_para(struct tcp_res *stat, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	u8 direc = (u8)state->hook;

	if (direc == NF_INET_LOCAL_IN) {
		stat->in_udp_pkt++;
		stat->in_udp_len += skb->len;
	} else if (direc == NF_INET_POST_ROUTING) {
		stat->out_udp_pkt++;
		stat->out_udp_len += skb->len;
	} else if (direc == NF_INET_FORWARD || direc == NF_INET_LOCAL_OUT) {
		stat->drop_pkt++;
	} else if (direc == NF_INET_BUFHOOK) {
		stat->udp_buf += ((struct nf_hook_state_ex *)state)->buf;
	}
}

static int stat_proces(struct sk_buff *skb, const struct nf_hook_state *state,
	u8 protocal, u16 payload_len)
{
	struct app_stat *app = NULL;
	struct app_list *pos = NULL;
	struct app_list *pos_selec = NULL;
	u32 app_type = APP_MACH_BUTT;
	struct sock *sk = NULL;
	unsigned int cpu = smp_processor_id();

	if (skb == NULL)
		return SK_NULL_ERR;
	if (cpu < 0 || cpu > CONFIG_NR_CPUS)
		return CPU_IDX_ERR;
	if (state == NULL)
		return HOOK_STAT_ERR;
	sk = skb_to_full_sk(skb);
	app = &g_app_ctx->cur[cpu];
	spin_lock_bh(&app->lock);
	if (list_empty_careful(&app->head))
		goto unlock;
	list_for_each_entry(pos, &app->head, head) {
		app_type = match_app(sk, &pos->stat, protocal, state->hook);
		if (app_type == BACKGROUND) {
			pos_selec = pos;
		} else if (app_type == FORGROUND_MACH) {
			pos_selec = pos;
			break;
		}
	}
	if (pos_selec == NULL)
		goto unlock;
	if (protocal == IPPROTO_TCP)
		update_tcp_para(&pos_selec->stat, skb, sk, state, payload_len);
	else if (protocal == IPPROTO_UDP)
		update_udp_para(&pos_selec->stat, skb, state);
unlock:
	spin_unlock_bh(&app->lock);
	return RTN_SUCC;
}

void collec_update_buf_time(struct sk_buff *skb, u8 protocal)
{
	s64 prv;
	s64 cur;
	s64 buf;
	struct nf_hook_state_ex state_ex;

	prv = ktime_to_ns(skb->tstamp);
	cur = ktime_to_ns(ktime_get_real());
	if (prv == 0 || cur == 0)
		return;
	buf = cur - prv;
	if (buf < 0)
		return;
	buf = buf / NS_CONVERT_MS;
	if (buf > BUF_MAX)
		buf = BUF_MAX;

	state_ex.buf = (int)buf;
	state_ex.state.hook = NF_INET_BUFHOOK;
	stat_proces(skb, (const struct nf_hook_state *)&state_ex, protocal, 0);
}

static int is_data_net_dev(struct sk_buff *skb, unsigned int direc)
{
	if (skb == NULL)
		return NET_DEV_ERR;

	if (direc == NF_INET_LOCAL_OUT || direc == NF_INET_FORWARD)
		return RTN_SUCC;

	if (skb->dev == NULL || skb->dev->name == NULL)
		return NET_DEV_ERR;

	if (strncmp(skb->dev->name, DS_NET, DS_NET_LEN) != 0 &&
		strncmp(skb->dev->name, DS_NET_SLAVE, DS_NET_LEN) != 0)
		return NET_DEV_ERR;
	return RTN_SUCC;
}

static unsigned int hook4(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct iphdr *iph = NULL;
	u16 payload_len = 0;
	int ret = 0;

	if (is_data_net_dev(skb, state->hook) != RTN_SUCC)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	payload_len = ntohs(iph->tot_len) - iph->ihl * 4; // 4 is 32bits to byte
	if (iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_UDP)
		ret = stat_proces(skb, state, iph->protocol, payload_len);
	if (ret != 0)
		pr_info("[IP_PARA]hook4 packet process error:%d.\n", ret);

	return NF_ACCEPT;
}

static unsigned int hook6(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct ipv6hdr *iph = NULL;
	int ret = 0;
	u16 payload_len = 0;
	if (is_data_net_dev(skb, state->hook) != RTN_SUCC)
		return NF_ACCEPT;
	iph = ipv6_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	payload_len = ntohs(iph->payload_len);
	if (iph->nexthdr == IPPROTO_TCP || iph->nexthdr == IPPROTO_UDP)
		ret = stat_proces(skb, state, iph->nexthdr, payload_len);
	if (ret != 0)
		pr_info("[IP_PARA]hook6 packet process error:%d.\n", ret);

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
	sum->in_rts_pkt += add->in_rts_pkt;
	sum->in_pkt += add->in_pkt;
	sum->in_len += add->in_len;
	sum->out_rts_pkt += add->out_rts_pkt;
	sum->out_pkt += add->out_pkt;
	sum->out_len += add->out_len;
	sum->rtt += add->rtt;
	sum->drop_pkt += add->drop_pkt;
	sum->in_udp_pkt += add->in_udp_pkt;
	sum->out_udp_pkt += add->out_udp_pkt;
	sum->in_udp_len += add->in_udp_len;
	sum->out_udp_len += add->out_udp_len;
	sum->tcp_buf += add->tcp_buf;
	sum->udp_buf += add->udp_buf;
	sum->tcp_syn_pkt += add->tcp_syn_pkt;

	if (sum->in_len > MAX_U32_VALUE)
		sum->in_len = MAX_U32_VALUE;
	if (sum->out_len > MAX_U32_VALUE)
		sum->in_len = MAX_U32_VALUE;
	if (sum->rtt > MAX_U32_VALUE)
		sum->rtt = MAX_U32_VALUE;
	if (sum->in_udp_len > MAX_U32_VALUE)
		sum->in_udp_len = MAX_U32_VALUE;
	if (sum->out_udp_len > MAX_U32_VALUE)
		sum->out_udp_len = MAX_U32_VALUE;
	if (sum->tcp_buf > MAX_U32_VALUE)
		sum->tcp_buf = MAX_U32_VALUE;
	if (sum->udp_buf > MAX_U32_VALUE)
		sum->udp_buf = MAX_U32_VALUE;
}

static void reset_stat(struct tcp_res *entry)
{
	entry->in_rts_pkt = 0;
	entry->in_pkt = 0;
	entry->in_len = 0;
	entry->out_rts_pkt = 0;
	entry->out_pkt = 0;
	entry->out_len = 0;
	entry->rtt = 0;
	entry->drop_pkt = 0;
	entry->in_udp_pkt = 0;
	entry->out_udp_pkt = 0;
	entry->in_udp_len = 0;
	entry->out_udp_len = 0;
	entry->tcp_buf = 0;
	entry->udp_buf = 0;
	entry->tcp_syn_pkt = 0;
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

static void stat_report(unsigned long sync)
{
	struct res_msg *res = NULL;
	u16 cnt;
	u16 len;
	u32 i;

	spin_lock_bh(&g_app_ctx->lock);
	g_app_ctx->timer.data = sync + 1; // number of reports
	g_app_ctx->timer.function = stat_report;
	mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	cnt = g_app_ctx->cnt;
	len = sizeof(struct res_msg) + cnt * sizeof(struct tcp_res);
	res = kmalloc(len, GFP_KERNEL);

	if (res == NULL)
		goto report_end;

	memset(res, 0, len);
	res->type = APP_QOE_SYNC_RPT;
	res->len = len;
	res->res.tcp.sync = (u16)sync;

	for (i = 0; i < CONFIG_NR_CPUS; i++)
		cum_cpus_stat(g_app_ctx->cur + i, &res->res.tcp, cnt);

	if (g_app_ctx->fn)
		g_app_ctx->fn((struct res_msg_head *)res);

	__send_buf_to_chr(&res->res.tcp);

	kfree(res);

report_end:
	spin_unlock_bh(&g_app_ctx->lock);
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
			list_move(p, &tmp_head);
			need_new = false;
			break;
		}
		if (need_new) {
			pos = kmalloc(sizeof(struct app_list), GFP_KERNEL);
			if (pos == NULL)
				goto list_end;
			memset(pos, 0, sizeof(struct app_list));
			pos->stat.pid = id[i].pid;
			pos->stat.uid = id[i].uid;
			list_add(&pos->head, &tmp_head);
		}
	}
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
		g_app_ctx->timer.data = 0;
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

static void process_app_update(struct req_msg *msg)
{
	struct tcp_collect_req *req = &msg->req.tcp_req;
	u16 flag = req->req_flag;
	u32 cycle = req->rpt_cycle;

	pr_info("[IP_PARA] %s data :%d %d %d", __func__, req->req_flag, req->rpt_cycle,
		req->cnt);
	spin_lock_bh(&g_app_ctx->lock);

	if (req->cnt > MAX_APP_LIST_LEN)
		goto app_end;

	if (flag & FORFROUND_STAT || flag & BACKGROUND_STAT)
		sync_to_cpus(req);

	if (flag & RESTART_SYNC) {
		g_app_ctx->timer.data = 0;
		g_app_ctx->timer.function = stat_report;
		g_app_ctx->jcycle = ms_convert_jiffies(cycle);
		mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	}

app_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void cmd_process(struct req_msg_head *msg)
{
	if (msg->len > MAX_REQ_DATA_LEN)
		return;

	if (msg->len < sizeof(struct req_msg))
		return;

	if (msg->type == APP_QOE_SYNC_CMD)
		process_sync((struct req_msg *)msg);
	else if (msg->type == UPDATE_APP_INFO_CMD)
		process_app_update((struct req_msg *)msg);
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
	init_timer(&g_app_ctx->timer);

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
