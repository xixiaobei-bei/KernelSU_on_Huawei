/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: This module is to collect IP package parameters
 * Author: songqiubin songqiubin@huawei.com
 * Create: 2019-09-18
 */

#include "fi.h"
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/netdevice.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/udp.h>
#include <linux/version.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "emcom/emcom_xengine.h"

struct fi_ctx g_fi_ctx;
static struct fi_app_info_table g_fi_app_info;
static uint64_t g_fi_iface_bytes[FI_DEV_NUM];

void fi_qos_update(const struct sk_buff *skb, struct fi_flow_node *flow);
void fi_filesize_update(const struct sk_buff *skb, struct fi_flow_node *flow);
void fi_iface_report(void);
static int fi_get_http_rsp_code(const unsigned char *buf, unsigned int buf_len,
								unsigned char *out_buf, unsigned int out_len);

static unsigned int fi_ipv4_hook_out(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state);

static unsigned int fi_ipv4_hook_in(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state);

#if IS_ENABLED(CONFIG_IPV6)
static unsigned int fi_ipv6_hook_out(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state);

static unsigned int fi_ipv6_hook_in(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 1)
	static const struct nf_hook_ops fi_nfhooks[] = {
#else
	static struct nf_hook_ops fi_nfhooks[] = {
#endif
	{
		.hook = fi_ipv4_hook_out,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = fi_ipv4_hook_in,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER - 1,
	},
#if IS_ENABLED(CONFIG_IPV6)
	{
		.hook = fi_ipv6_hook_out,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = fi_ipv6_hook_in,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER - 1,
	},
#endif
};

void fi_register_nfhook(void)
{
	int ret;
	int i;
	struct net_device *dev = NULL;
	char *iface = NULL;
	struct rtnl_link_stats64 stats;

	mutex_lock(&(g_fi_ctx.nf_mutex));
	if (g_fi_ctx.nf_exist) {
		mutex_unlock(&(g_fi_ctx.nf_mutex));
		return;
	}
	local_irq_disable();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 1)
	ret = nf_register_net_hooks(&init_net, fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#else
	ret = nf_register_hooks(fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#endif
	local_irq_enable();
	if (ret == 0) {
		g_fi_ctx.nf_exist = true;
		g_fi_ctx.tm.expires = jiffies + HZ / FI_TIMER_INTERVAL;
		if (!timer_pending(&(g_fi_ctx.tm)))
			add_timer(&(g_fi_ctx.tm));
		else
			fi_loge("timer_pending, not need to add again");
	}
	mutex_unlock(&(g_fi_ctx.nf_mutex));
	for (i = 0; i < FI_DEV_NUM; i++) {
		iface = emcom_xengine_get_network_iface_name(i);
		dev = dev_get_by_name(&init_net, iface);
		if (dev == NULL) {
			/* clear g_fi_iface_bytes if dev is off */
			g_fi_iface_bytes[i] = 0;
			fi_logi(" can't find dev %s", iface);
			continue;
		}
		read_lock(&dev_base_lock);
		dev_get_stats(dev, &stats);
		read_unlock(&dev_base_lock);
		dev_put(dev);
		g_fi_iface_bytes[i] = stats.rx_bytes;
		fi_logi("init ifname: %s, total_byes: %lu", iface, stats.rx_bytes);
	}
	if (ret == 0)
		fi_logi("FI register nf hooks successfully");
	else
		fi_loge("FI register nf hooks failed, ret:%d", ret);
}

void fi_unregister_nfhook(void)
{
	mutex_lock(&(g_fi_ctx.nf_mutex));
	if (!g_fi_ctx.nf_exist) {
		mutex_unlock(&(g_fi_ctx.nf_mutex));
		return;
	}

	local_irq_disable();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 1)
	nf_unregister_net_hooks(&init_net, fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#else
	nf_unregister_hooks(fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#endif
	local_irq_enable();

	g_fi_ctx.nf_exist = false;
	del_timer_sync(&(g_fi_ctx.tm));
	fi_ipv4_flow_clear();
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_clear();
#endif
	mutex_unlock(&(g_fi_ctx.nf_mutex));
	fi_logi("FI unregister nf hooks successfully");
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
void fi_timer(unsigned long arg)
#else
void fi_timer(struct timer_list *t)
#endif
{
	fi_flow_periodic_report();
	fi_flow_qos_report();
	fi_iface_report();
	fi_ipv4_flow_age();
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_age();
#endif
	mod_timer(&(g_fi_ctx.tm), jiffies + HZ / FI_TIMER_INTERVAL);
}

static inline bool fi_payloadlen_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpseq_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpack_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpflags_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpwindowsize_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpoptions_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_payload_segment_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);

static struct fi_pkt_parse_f g_fi_pkt_parse_f[] = {
	{FI_PAYLOADLEN_PARSE,		fi_payloadlen_parse},
	{FI_PAYLOAD_SEGMENT_PARSE,	fi_payload_segment_parse},
	{FI_ECHO_DELAY,				NULL},
	{FI_TCPSEQ_PARSE,			fi_tcpseq_parse},
	{FI_TCPACK_PARSE,			fi_tcpack_parse},
	{FI_TCPFLAGS_PARSE,			fi_tcpflags_parse},
	{FI_TCPWINDOWSIZE_PARSE,	fi_tcpwindowsize_parse},
	{FI_TCPOPTIONS_PARSE,		fi_tcpoptions_parse},
	{FI_END_PARSE,				NULL},
};

static bool fi_payloadlen_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_UDP) {
		if (skb->len > skb_transport_offset(skb) + sizeof(struct udphdr))
			pktinfo->msg.payloadlen = skb->len -
				skb_transport_offset(skb) - sizeof(struct udphdr);
		else
			pktinfo->msg.payloadlen = 0;
	} else {
		if (skb->len > skb_transport_offset(skb) + tcp_hdrlen(skb))
			pktinfo->msg.payloadlen = skb->len - skb_transport_offset(skb) - tcp_hdrlen(skb);
		else
			pktinfo->msg.payloadlen = 0;
	}
	return true;
}

static bool fi_tcpseq_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_seq = ntohl(tcph->seq);
		return true;
	}
	return true;
}

static bool fi_tcpack_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_ack = ntohl(tcph->ack);
		return true;
	}
	return true;
}

static bool fi_tcpflags_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_flags |= tcph->fin;
		pktinfo->msg.tcp_flags |= tcph->syn << TCPHDR_OFFSET_SYN;
		pktinfo->msg.tcp_flags |= tcph->rst << TCPHDR_OFFSET_RST;
		pktinfo->msg.tcp_flags |= tcph->psh << TCPHDR_OFFSET_PSH;
		pktinfo->msg.tcp_flags |= tcph->ack << TCPHDR_OFFSET_ACK;
		pktinfo->msg.tcp_flags |= tcph->urg << TCPHDR_OFFSET_URG;
		pktinfo->msg.tcp_flags |= tcph->ece << TCPHDR_OFFSET_ECE;
		pktinfo->msg.tcp_flags |= tcph->cwr << TCPHDR_OFFSET_CWR;
		return true;
	}
	return true;
}

static bool fi_tcpwindowsize_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_window = ntohs(tcph->window);
		return true;
	}
	return true;
}

static bool fi_tcpoptions_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	unsigned int optionslen;
	int ret;

	if (pktinfo->msg.flow_msg.l4proto != IPPROTO_TCP)
		return true;
	if (tcp_hdrlen(skb) <= sizeof(struct tcphdr))
		return true;
	optionslen = tcp_hdrlen(skb) - sizeof(struct tcphdr);
	if (optionslen > FI_TCP_OPTIONS_LEN)
		return false;
	ret = memcpy_s(pktinfo->msg.tcp_options, FI_TCP_OPTIONS_LEN,
		(char *)(skb_transport_header(skb) + sizeof(struct tcphdr)), optionslen);
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return false;
	}

	return true;
}

static bool fi_payload_segment_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	int ret;
	__be16 payloadlen = 0;
	uint32_t offset;
	char *payload = NULL;
	fi_flow_identify_cfg *flow_cfg = pktinfo->flow_cb.flow_identify_cfg;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_UDP) {
		if (skb->len > skb_transport_offset(skb) + sizeof(struct udphdr)) {
			payloadlen = skb->len -
				skb_transport_offset(skb) - sizeof(struct udphdr);
			payload = skb_transport_header(skb) + sizeof(struct udphdr);
		}
	} else {
		if (skb->len > skb_transport_offset(skb) + tcp_hdrlen(skb)) {
			payloadlen = skb->len - skb_transport_offset(skb) - tcp_hdrlen(skb);
			payload = skb_transport_header(skb) + tcp_hdrlen(skb);
		}
	}

	pktinfo->payload = payload;
	pktinfo->payloadlen = payloadlen;

	if (payloadlen == 0)
		return true;
	if (flow_cfg->pkt[pktinfo->dir].seg_begin >= payloadlen)
		return true;
	offset = flow_cfg->pkt[pktinfo->dir].seg_end - flow_cfg->pkt[pktinfo->dir].seg_begin + 1;
	if (flow_cfg->pkt[pktinfo->dir].seg_end >= payloadlen)
		offset = payloadlen - flow_cfg->pkt[pktinfo->dir].seg_begin;
	ret = memcpy_s(pktinfo->msg.payload_seg, sizeof(pktinfo->msg.payload_seg),
		payload + flow_cfg->pkt[pktinfo->dir].seg_begin, offset);
	if (ret) {
		fi_loge("memcpy_s failed ret=%d, len=%u", ret, offset);
		return false;
	}
	fi_logd("get payload_seg len=%u, payloadlen=%d begin=%u end=%u", offset,
		payloadlen, flow_cfg->pkt[pktinfo->dir].seg_begin, flow_cfg->pkt[pktinfo->dir].seg_end);
	return true;
}

/**
 * parse http dl packet and return the http response code
 * return 0: may try parse next time; -1: no need parse next time; other: the http real rsp code
 */
int fi_parse_http_rsp_code(const struct sk_buff *skb)
{
	uint16_t port;
	struct sock *sk = NULL;
	unsigned int payloadlen;
	unsigned char *payload = NULL;
	unsigned char rsp_code[FI_HTTP_RSP_CODE_MAX_LEN + 1] = {0};
	int code = -1;

	if (skb == NULL)
		return 0;
	sk = skb->sk;
	if (sk == NULL)
		return -1;
	if (sk->sk_protocol != IPPROTO_TCP)
		return -1;

	port = ntohs(tcp_hdr(skb)->source);
	if (port != EMCOM_MPFLOW_FI_PORT_80)
		return -1;

	if (sk->sk_state != TCP_ESTABLISHED)
		return 0; // deal next time

	payloadlen = skb->len - skb_transport_offset(skb) - tcp_hdrlen(skb);
	if (payloadlen == 0) // may be an ack
		return 0; // deal next time
	if (payloadlen < FI_MIN_HTTP_LEN) // first dl pkt lenth samll
		return -1;

	payload = skb_transport_header(skb) + tcp_hdrlen(skb);
	if (payload == NULL)
		return 0;
	if (fi_get_http_rsp_code(payload, FI_MIN_HTTP_LEN, rsp_code, FI_HTTP_RSP_CODE_MAX_LEN) != 0)
		return -1;
	if (kstrtoint(rsp_code, FI_DECIMAL_BASE, &code) != 0)
		return -1;
	fi_logd("src port:%u, dst port:%u, rsp_code:%d", port, sk->sk_num, code);
	return (code <= 0 ? -1 : code);
}

static int fi_get_http_rsp_code(const unsigned char *buf, unsigned int buf_len,
								unsigned char *out_buf, unsigned int out_len)
{
	unsigned int i;
	unsigned int code_begin;
	unsigned int code_end;
	unsigned int code_len;
	bool has_end = false;
	const unsigned char *http_rsp_hdr = FI_HTTP_VERSION_STR;
	const unsigned int http_rsp_hdr_len = strlen(FI_HTTP_VERSION_STR);

	for (i = 0; i < http_rsp_hdr_len; i++) {
		if (buf[i] != http_rsp_hdr[i])
			return -1;
	}
	code_begin = i;
	for (i = 0; i < FI_HTTP_RSP_CODE_MAX_LEN &&
		*(buf + code_begin + i) != 0 && *(buf + code_begin + i) != '\t' &&
		*(buf + code_begin + i) != '\r' && *(buf + code_begin + i) != '\n'; i++) {
		if (*(buf + code_begin + i) == ' ') {
			code_end = code_begin + i - 1;
			has_end = true;
			break;
		}
	}
	if (!has_end)
		return -1;

	code_len = code_end - code_begin + 1;
	if (code_len == 0 || code_len > out_len)
		return -1;

	if (memcpy_s(out_buf, FI_HTTP_RSP_CODE_MAX_LEN, buf + code_begin, code_len) != EOK)
		return -1;
	return 0;
}

static void fi_reset(void)
{
	int i;

	fi_logi("fi need reset when netlink pid change]free_cnt=%d", atomic_read(&(g_fi_app_info.free_cnt)));
	if (atomic_read(&(g_fi_app_info.free_cnt)) >= FI_LAUNCH_APP_MAX)
		return;
	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.status != FI_INVALID_APP_STATUS) {
			struct fi_app_info_node *app = &(g_fi_app_info.app[i]);

			app->rand_num = 0;
			app->msg.status = FI_INVALID_APP_STATUS;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	atomic_set(&(g_fi_app_info.free_cnt), FI_LAUNCH_APP_MAX);
	fi_unregister_nfhook();
}

static void fi_set_nl_pid(const struct fi_cfg_head *cfg)
{
	if (cfg->len < sizeof(struct fi_cfg_head) + sizeof(uint32_t)) {
		fi_loge(" received a message without enough FI app info, len %d, min_len %zu\n",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(uint32_t));
		return;
	}
	/* fi need reset when netlink pid change, in case of daemon process restart */
	fi_reset();
	 g_fi_ctx.nl_pid = *(uint32_t *)((char *)cfg +
		sizeof(struct fi_cfg_head));
	fi_logi("set netlink pid from frameworkJnI_pid %u", g_fi_ctx.nl_pid);
	return;
}

static void fi_iface_qos_rpt_update(void)
{
	int i;

	g_fi_ctx.iface_qos_rpt_enable = 0;
	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.uid != 0 &&
			g_fi_app_info.app[i].msg.status == FI_COLLECT_RUN &&
			g_fi_app_info.app[i].msg.qos_rpt_enable) {
			g_fi_ctx.iface_qos_rpt_enable = 1;
			spin_unlock_bh(&(g_fi_app_info.app[i].lock));
			break;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
}

static bool fi_app_info_update_status_check_invalid(const enum fi_app_status status)
{
	if ((status <= FI_INVALID_APP_STATUS) || (status >= FI_MAX_APP_STATUS)) {
		fi_loge("received an app with illegal status %d", status);
		return true;
	} else {
		return false;
	}
}

static bool fi_app_info_update_same_app_check(struct fi_app_update_msg *update_msg,
	struct fi_app_info_node *app)
{
	if (app->msg.uid == update_msg->uid)
		return true;

	return false;
}

static int protocol_trans(int proto)
{
	switch (proto) {
	case FI_TCP:
		return IPPROTO_TCP;
	case FI_UDP:
		return IPPROTO_UDP;
	default:
		break;
	}
	return proto;
}

static void fi_app_info_update(const struct fi_cfg_head *cfg)
{
	struct fi_app_update_msg *update_msg = NULL;
	struct fi_app_info_node *app = NULL;
	struct fi_app_info_node *new_app = NULL;
	fi_rpt_cfg *periodic = NULL;
	fi_rpt_cfg *pkt = NULL;
	int i, ret;

	if (cfg->len != sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg)) {
		fi_loge("received message without enough FI app info, len %d, min_len %zu",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg));
		return;
	}
	update_msg = (struct fi_app_update_msg *)((char *)cfg +
		sizeof(struct fi_cfg_head));
	if (fi_app_info_update_status_check_invalid(update_msg->status) ||
		(update_msg->status == FI_COLLECT_STOP)) {
		fi_loge("received an app with illegal status %d",
			update_msg->status);
		return;
	}

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (fi_app_info_update_same_app_check(update_msg, &g_fi_app_info.app[i]) &&
			g_fi_app_info.app[i].msg.status != FI_INVALID_APP_STATUS) {
			app = &(g_fi_app_info.app[i]);
			break;
		}
		if (new_app == NULL && g_fi_app_info.app[i].msg.status ==
			FI_INVALID_APP_STATUS) {
			new_app = &(g_fi_app_info.app[i]);
			continue;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	if (app == NULL && new_app == NULL) {
		fi_loge("reach max app launch size %d", FI_LAUNCH_APP_MAX);
		return;
	} else if (app == NULL) {
		app = new_app;
		atomic_dec(&(g_fi_app_info.free_cnt));
	} else if (new_app != NULL) {
		spin_unlock_bh(&(new_app->lock));
	}

	ret = memcpy_s(&(app->msg), sizeof(struct fi_app_update_msg),
		update_msg, sizeof(struct fi_app_update_msg));
	if (ret) {
		fi_loge("memcpy_s failed ret=%d", ret);
		spin_unlock_bh(&(app->lock));
		return;
	}

	for (i = 0; i < FI_FLOW_IDENTIFY_MAX; i++)
		app->msg.flow_identify_cfg[i].proto = protocol_trans(app->msg.flow_identify_cfg[i].proto);

	app->rand_num++;
	spin_unlock_bh(&(app->lock));
	for (i = 0; i < FI_DIR_BOTH; i++) {
		periodic = &(app->msg.flow_identify_cfg[0].periodic[i]);
		pkt = &(app->msg.flow_identify_cfg[0].pkt[i]);
		fi_logi("Config received.app %s[%d], status=%d,proto=%d,dport=%d,"
			"periodic[dir=%d roiType=%d roiStart=%d roiStart=%d],"
			"immediate[dir=%d roiType=%d roiStart=%d roiStart=%d] version:%s",
			app->msg.app_name, app->msg.uid, app->msg.status,
			app->msg.flow_identify_cfg[0].proto, app->msg.flow_identify_cfg[0].dport[0],
			i, periodic->report_type, periodic->roi_start, periodic->roi_stop,
			i, pkt->report_type, pkt->roi_start, pkt->roi_stop, FI_VER);
	}
	fi_register_nfhook();
	fi_iface_qos_rpt_update();
	return;
}

static void fi_app_status_update(const struct fi_cfg_head *cfg)
{
	struct fi_app_update_msg *update_msg = NULL;
	struct fi_app_info_node *app = NULL;
	int i;

	if (cfg->len < sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg)) {
		fi_loge("receive a message without enough FI app info, len %d, min_len %zu",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg));
		return;
	}
	update_msg = (struct fi_app_update_msg *)((char *)cfg +
		sizeof(struct fi_cfg_head));
	if (fi_app_info_update_status_check_invalid(update_msg->status)) {
		return;
	}
	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.uid == update_msg->uid &&
			g_fi_app_info.app[i].msg.status != FI_INVALID_APP_STATUS) {
			app = &(g_fi_app_info.app[i]);
			break;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	if (app == NULL) {
		fi_loge("app %s, uid %d not exist", update_msg->app_name, update_msg->uid);
		return;
	}
	if (update_msg->status == FI_COLLECT_STOP) {
		app->rand_num = 0;
		app->msg.status = FI_INVALID_APP_STATUS;
		spin_unlock_bh(&(app->lock));
		fi_ipv4_flow_clear_by_uid(update_msg->uid);
#if IS_ENABLED(CONFIG_IPV6)
		fi_ipv6_flow_clear_by_uid(update_msg->uid);
#endif
		atomic_inc(&(g_fi_app_info.free_cnt));
		fi_logd("app %s, uid %d stop",
			update_msg->app_name, update_msg->uid);
	} else {
		app->msg.status = update_msg->status;
		app->rand_num++;
		spin_unlock_bh(&(app->lock));
	}
	if (atomic_read(&(g_fi_app_info.free_cnt)) >= FI_LAUNCH_APP_MAX)
		fi_unregister_nfhook();

	fi_iface_qos_rpt_update();

	fi_logi("app %s status update, uid:%d, status:%d, rand_num:%lu, free_cnt:%d",
		app->msg.app_name, app->msg.uid, app->msg.status, app->rand_num, atomic_read(&(g_fi_app_info.free_cnt)));
	return;
}

void fi_cfg_process(const struct fi_cfg_head *cfg)
{
	if (!g_fi_ctx.running) {
		fi_loge("FI not initialized yet, cfg type:%d", cfg->type);
		return;
	}
	if (cfg == NULL || cfg->len == 0)
		return;

	switch (cfg->type) {
	case FI_SET_NL_PID_CMD:
		fi_set_nl_pid(cfg);
		break;
	case FI_COLLECT_START_CMD:
	case FI_COLLECT_INFO_UPDATE_CMD:
		fi_app_info_update(cfg);
		break;
	case FI_COLLECT_STATUS_UPDATE_CMD:
		fi_app_status_update(cfg);
		break;
	default:
		fi_loge("receive a not FI message, type:%d", cfg->type);
		break;
	}
	return;
}
EXPORT_SYMBOL(fi_cfg_process); /*lint !e580*/

static void fi_pkt_msg_report(const struct fi_pkt_parse *pktinfo, int type)
{
	uint32_t size;
	char *data = NULL;
	int ret;
	struct sk_buff *pskb = NULL;

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	if (pktinfo->rand_num == pktinfo->flow_cb.app_info->rand_num) {
		spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
		size = sizeof(struct fi_pkt_msg);
		pskb = fi_get_netlink_skb(type, size, &data);
		if (pskb == NULL) {
			fi_loge("failed to malloc memory for data");
			return;
		}
		ret = memcpy_s(data, size, &(pktinfo->msg), sizeof(struct fi_pkt_msg));
		if (ret) {
			fi_loge("memcpy_s failed ret=%d", ret);
			kfree_skb(pskb);
			return;
		}

		fi_enqueue_netlink_skb(pskb);
		fi_logd("finish to send a message to native, flow:%s", pktinfo->msg.flow_msg.flow);
	} else {
		spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	}
	return;
}

static bool fi_check_ip_addrss(struct fi_pkt_parse *pktinfo, int32_t uid)
{
	struct sockaddr_in6 addr = {0};
	if (pktinfo->msg.flow_msg.l3proto == ETH_P_IP) {
		struct sockaddr_in *usin = (struct sockaddr_in *)&addr;
		usin->sin_family = AF_INET;
		usin->sin_addr.s_addr = pktinfo->msg.flow_msg.ipv4_dip;
	} else {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)&addr;
		usin6->sin6_family = AF_INET6;
		usin6->sin6_addr = pktinfo->msg.flow_msg.ipv6_dip;
	}
	if (!emcom_xengine_check_ip_addrss((struct sockaddr *)&addr) ||
		emcom_xengine_check_ip_is_private((struct sockaddr *)&addr)) {
		fi_logd("invalid addr. uid: %d", uid);
		return false;
	}
	return true;
}

static bool fi_app_info_get(struct fi_pkt_parse *pktinfo,
				int32_t uid, enum fi_app_status status)
{
	struct fi_app_info_node *app = NULL;
	int i, j;
	bool match = false;

	if (pktinfo == NULL || !fi_check_ip_addrss(pktinfo, uid))
		return false;

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.uid == uid &&
			g_fi_app_info.app[i].msg.status == status) {
			app = &(g_fi_app_info.app[i]);
			break;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	if (app == NULL)
		return false;
	pktinfo->flow_cb.app_info = app;
	pktinfo->rand_num = pktinfo->flow_cb.app_info->rand_num;
	pktinfo->msg.flow_msg.uid = uid;
	for (i = 0; i < FI_FLOW_IDENTIFY_MAX; i++) {
		int proto = app->msg.flow_identify_cfg[i].proto;
		if (proto == FI_PROTO_ANY || proto == pktinfo->msg.flow_msg.l4proto) {
			for (j = 0; j < FI_DPORT_MAX; j++) {
				if (app->msg.flow_identify_cfg[i].dport[j] == FI_PORT_ANY ||
					pktinfo->msg.flow_msg.dport == app->msg.flow_identify_cfg[i].dport[j]) {
					match = true;
					break;
				}
			}
		}
		if (match)
			break;
	}
	spin_unlock_bh(&(app->lock));
	if (!match)
		return false;
	pktinfo->flow_cb.flow_identify_cfg = &app->msg.flow_identify_cfg[i];
	return true;
}

static bool fi_ipv4_pkt_check(struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);
	if (iph == NULL)
		return false;
	if (ipv4_is_loopback(iph->saddr) || ipv4_is_loopback(iph->daddr))
		return false;
	if (iph->protocol == IPPROTO_UDP) {
		if (skb->len >= skb_transport_offset(skb) + sizeof(struct udphdr))
			return true;
	} else if (iph->protocol == IPPROTO_TCP) {
		if (skb->len >= skb_transport_offset(skb) + tcp_hdrlen(skb))
			return true;
	}
	return false;
}

static void fi_ip_save_5tuple_info(struct fi_pkt_parse *pktinfo, uint16_t protocol, const struct sk_buff *skb)
{
	struct udphdr *udph = NULL;
	struct tcphdr *tcph = NULL;
	if (protocol == IPPROTO_UDP) {
		udph = udp_hdr(skb);
		pktinfo->msg.flow_msg.l4proto = IPPROTO_UDP;
		pktinfo->msg.flow_msg.sport = ntohs(udph->source);
		pktinfo->msg.flow_msg.dport = ntohs(udph->dest);
	} else {
		tcph = tcp_hdr(skb);
		pktinfo->msg.flow_msg.l4proto = IPPROTO_TCP;
		pktinfo->msg.flow_msg.sport = ntohs(tcph->source);
		pktinfo->msg.flow_msg.dport = ntohs(tcph->dest);
	}
}

static void fi_ipv4_5tuple_parse(struct fi_pkt_parse *pktinfo, const struct sk_buff *skb,
				enum fi_dir dir)
{
	struct iphdr *iph = NULL;

	pktinfo->msg.flow_msg.l3proto = ntohs(skb->protocol);
	do_gettimeofday(&(pktinfo->msg.tstamp));
	pktinfo->msg.dir = dir;
	iph = ip_hdr(skb);
	pktinfo->msg.flow_msg.ipv4_sip = iph->saddr;
	pktinfo->msg.flow_msg.ipv4_dip = iph->daddr;
	fi_ip_save_5tuple_info(pktinfo, iph->protocol, skb);
	return;
}

static void fi_ipv4_pkt_parse(struct fi_pkt_parse *pktinfo, struct sk_buff *skb)
{
	int i;

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	for (i = 0; i < FI_END_PARSE; i++) {
		if ((pktinfo->flow_cb.flow_identify_cfg->pkt[pktinfo->dir].opt_enable[i] ||
			pktinfo->flow_cb.flow_identify_cfg->periodic[pktinfo->dir].opt_enable[i]) &&
			(g_fi_pkt_parse_f[i].opt_val_parse != NULL)) {
			fi_logd("parse %d function", i);
			(void)(g_fi_pkt_parse_f[i].opt_val_parse(skb, pktinfo));
		}
	}
	spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	return;
}

static void fi_flow_get_dev(struct fi_flow_msg *flow_msg, const struct net_device *dev)
{
	int ret = memcpy_s(flow_msg->dev, IFNAMSIZ, dev->name, strlen(dev->name) + 1);
	if (ret)
		fi_loge("memcpy_s failed ret=%d, dev %s", ret, dev->name);
	return;
}

bool fi_is_blocked_rsp_code(int rsp_code)
{
	if (rsp_code == FI_HTTP_RESPONSE_MOVED || rsp_code == FI_HTTP_RESPONSE_REDIRECT ||
		rsp_code == FI_HTTP_RESPONSE_OTHER_LOCATION || rsp_code == FI_HTTP_RESPONSE_USE_GATEWAY ||
		rsp_code == FI_HTTP_RESPONSE_BAD_REQUEST || rsp_code == FI_HTTP_RESPONSE_SERVER_INTERNAL_ERROR ||
		rsp_code == F_HTTP_RESPONSE_GATEWAY_ERROR)
		return true;
	return false;
}

static void fi_ipv4_hook(struct sk_buff *skb, const struct nf_hook_state *state, enum fi_dir dir)
{
	struct sock *sk = NULL;
	kuid_t kuid = {0};
	struct fi_pkt_parse pktinfo;
	uint32_t hash;
	bool enable = false;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *flow = NULL;

	if (unlikely(skb == NULL))
		return;
	if (!fi_ipv4_pkt_check(skb))
		return;

	if (memset_s(&pktinfo, sizeof(pktinfo), 0, sizeof(struct fi_pkt_parse))) {
		fi_loge("ipv4 hook memset_s failed");
		return;
	}

	fi_ipv4_5tuple_parse(&pktinfo, skb, dir);
	if (dir == FI_DIR_UP) {
		sk = skb->sk;
		if (sk == NULL)
			return;
		/* If sk state is SYN_RECV, sk is actually a request_sock, can not call sock_i_uid */
		if (pktinfo.msg.flow_msg.l4proto == IPPROTO_TCP &&
			(sk->sk_state == TCP_NEW_SYN_RECV || sk->sk_state == TCP_SYN_RECV)) {
			fi_logd("Ignore SYN_RECV sk, sk_state=%d l4proto=%d port[%hu--%hu]",
				sk->sk_state, pktinfo.msg.flow_msg.l4proto, pktinfo.msg.flow_msg.sport, pktinfo.msg.flow_msg.dport);
			return;
		}
		kuid = sock_i_uid(sk);
		if (!fi_app_info_get(&pktinfo, kuid.val, FI_COLLECT_RUN))
			return;
		pktinfo.msg.flow_msg.dev_if = sk->sk_bound_dev_if;
	}
	hash = fi_ipv4_flow_hash(pktinfo.msg.flow_msg.ipv4_sip,
		pktinfo.msg.flow_msg.sport,
		pktinfo.msg.flow_msg.ipv4_dip,
		pktinfo.msg.flow_msg.dport,
		pktinfo.msg.flow_msg.flow,
		FI_FLOW_LEN + 1,
		((dir == FI_DIR_DOWN) ? true : false));
	head = fi_ipv4_flow_header(hash);
	fi_ipv4_flow_lock(hash);
	flow = fi_ipv4_flow_get(head, kuid.val,
		pktinfo.msg.flow_msg.ipv4_sip,
		pktinfo.msg.flow_msg.sport,
		pktinfo.msg.flow_msg.ipv4_dip,
		pktinfo.msg.flow_msg.dport,
		pktinfo.flow_cb, ((dir == FI_DIR_UP) ? true : false),
		&(pktinfo.msg.flow_msg.new_flow), sk);
	if (flow == NULL) {
		fi_ipv4_flow_unlock(hash);
		return;
	}

	if (pktinfo.msg.flow_msg.l4proto == IPPROTO_TCP &&
		flow->sk && flow->sk->sk_state >= TCP_FIN_WAIT1) {
		fi_logd("Ignore flow:l4proto=%d,sk=%pK, sk_state=%d port[%hu--%hu]",
			pktinfo.msg.flow_msg.l4proto, flow->sk, flow->sk->sk_state,
			pktinfo.msg.flow_msg.sport, pktinfo.msg.flow_msg.dport);
		fi_ipv4_flow_unlock(hash);
		return;
	}

	if (pktinfo.msg.flow_msg.new_flow == true) {
		if (state->out) {
			fi_flow_get_dev(&pktinfo.msg.flow_msg, state->out);
			fi_flow_get_dev(&flow->flow_ctx.flow_msg, state->out);
		} else {
			fi_logi("new flow dev is null");
		}
	}
	if (fi_flow_qos_rpt_enable(flow) && pktinfo.msg.flow_msg.l4proto == IPPROTO_TCP) {
		if (dir == FI_DIR_UP) {
			fi_filesize_update(skb, flow);
		} else {
			fi_qos_update(skb, flow);
		}
	}
	/* if all stat report have stopped, bypass this packet */
	if ((flow->flow_ctx.stat_stop & FI_FLOW_ALL_REPORT_STOP) == FI_FLOW_ALL_REPORT_STOP
			&& !flow->flow_cb.flow_identify_cfg->basic_rpt_en) {
		fi_ipv4_flow_unlock(hash);
		return;
	}
	if (dir == FI_DIR_DOWN) {
		pktinfo.flow_cb = flow->flow_cb;
		pktinfo.rand_num = pktinfo.flow_cb.app_info->rand_num;
		pktinfo.msg.flow_msg.uid = flow->uid;
		pktinfo.msg.flow_msg.dev_if = flow->flow_ctx.flow_msg.dev_if;
		if (flow->flow_ctx.key_info_check_rst == FI_RPT_NULL) {
			int http_rsp_code = fi_parse_http_rsp_code(skb);
			if (http_rsp_code > 0 && fi_is_blocked_rsp_code(http_rsp_code))
				flow->flow_ctx.key_info_check_rst = FI_RPT_BLOCK;
			else if (http_rsp_code > 0 || http_rsp_code == -1)
				flow->flow_ctx.key_info_check_rst = FI_RPT_GO;
		}
	}
	pktinfo.dir = dir;
	fi_ipv4_pkt_parse(&pktinfo, skb);
	fi_flow_statistics(flow, &pktinfo, dir);
	enable = fi_pkt_report_is_enable(flow, &pktinfo, dir) && flow->flow_ctx.key_info_check_rst != FI_RPT_BLOCK;
	if (enable) {
		int ret = strcpy_s(pktinfo.msg.flow_msg.dev, IFNAMSIZ, flow->flow_ctx.flow_msg.dev);
		if (ret)
			fi_loge("strcpy_s failed ret=%d, dev=%s", ret, flow->flow_ctx.flow_msg.dev);
	}
	fi_ipv4_flow_unlock(hash);

	if (enable)
		fi_pkt_msg_report(&pktinfo, FI_PKT_MSG_RPT);
	return;
}

static unsigned int fi_ipv4_hook_out(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state)
{
	fi_ipv4_hook(skb, state, FI_DIR_UP);
	return NF_ACCEPT;
}

static unsigned int fi_ipv4_hook_in(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state)
{
	fi_ipv4_hook(skb, state, FI_DIR_DOWN);
	return NF_ACCEPT;
}

#if IS_ENABLED(CONFIG_IPV6)
static bool fi_ipv6_pkt_check(struct sk_buff *skb)
{
	int thoff = 0;
	int tproto;
	struct ipv6hdr *iph = ipv6_hdr(skb);
	if (iph == NULL)
		return false;
	if (ipv6_addr_loopback(&(iph->saddr)) ||
		ipv6_addr_loopback(&(iph->daddr)))
		return false;
	tproto = ipv6_find_hdr(skb, &thoff, -1, NULL, NULL);
	if (tproto == IPPROTO_UDP) {
		if (skb->len >= skb_transport_offset(skb) + sizeof(struct udphdr))
			return true;
	} else if (tproto == IPPROTO_TCP) {
		if (skb->len >= skb_transport_offset(skb) + tcp_hdrlen(skb))
			return true;
	}
	return false;
}

static void fi_ipv6_5tuple_parse(struct fi_pkt_parse *pktinfo, struct sk_buff *skb,
				enum fi_dir dir)
{
	struct ipv6hdr *iph = NULL;
	int thoff = 0;
	int tproto;
	int ret;

	pktinfo->msg.flow_msg.l3proto = ntohs(skb->protocol);
	do_gettimeofday(&(pktinfo->msg.tstamp));
	pktinfo->msg.dir = dir;
	iph = ipv6_hdr(skb);
	ret = memcpy_s(&(pktinfo->msg.flow_msg.ipv6_sip), sizeof(pktinfo->msg.flow_msg.ipv6_sip),
		&(iph->saddr),
		sizeof(pktinfo->msg.flow_msg.ipv6_sip));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	ret = memcpy_s(&(pktinfo->msg.flow_msg.ipv6_dip), sizeof(pktinfo->msg.flow_msg.ipv6_dip),
		&(iph->daddr),
		sizeof(pktinfo->msg.flow_msg.ipv6_dip));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}
	tproto = ipv6_find_hdr(skb, &thoff, -1, NULL, NULL);
	fi_ip_save_5tuple_info(pktinfo, tproto, skb);
	return;
}

static void fi_ipv6_pkt_parse(struct fi_pkt_parse *pktinfo, struct sk_buff *skb)
{
	int i;

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	for (i = 0; i < FI_END_PARSE; i++) {
		if (pktinfo->flow_cb.flow_identify_cfg->pkt[pktinfo->dir].opt_enable[i] &&
			g_fi_pkt_parse_f[i].opt_val_parse != NULL) {
			fi_logd("parse %d", i);
			(void)(g_fi_pkt_parse_f[i].opt_val_parse(skb, pktinfo));
		}
	}
	spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	return;
}

static void fi_ipv6_hook(struct sk_buff *skb, const struct nf_hook_state *state, enum fi_dir dir)
{
	struct sock *sk = NULL;
	kuid_t kuid = {0};
	struct fi_pkt_parse pktinfo;
	uint32_t hash;
	bool enable = false;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *flow = NULL;

	if (unlikely(skb == NULL))
		return;

	if (!fi_ipv6_pkt_check(skb))
		return;

	if (memset_s(&pktinfo, sizeof(pktinfo), 0, sizeof(struct fi_pkt_parse))) {
		fi_loge("ipv6 hook memset_s failed");
		return;
	}

	fi_ipv6_5tuple_parse(&pktinfo, skb, dir);
	if (dir == FI_DIR_UP) {
		sk = skb->sk;
		if (sk == NULL)
			return;
		/* If sk state is SYN_RECV, sk is actually a request_sock, can not call sock_i_uid */
		if (pktinfo.msg.flow_msg.l4proto == IPPROTO_TCP &&
			(sk->sk_state == TCP_NEW_SYN_RECV || sk->sk_state == TCP_SYN_RECV)) {
			fi_logd("Ignore SYN_RECV sk, sk_state=%d l4proto=%d port[%hu--%hu]",
				sk->sk_state, pktinfo.msg.flow_msg.l4proto, pktinfo.msg.flow_msg.sport, pktinfo.msg.flow_msg.dport);
			return;
		}
		kuid = sock_i_uid(sk);
		if (!fi_app_info_get(&pktinfo, kuid.val, FI_COLLECT_RUN))
			return;
		pktinfo.msg.flow_msg.dev_if = sk->sk_bound_dev_if;
	}
	hash = fi_ipv6_flow_hash(
		(const struct in6_addr *)&(pktinfo.msg.flow_msg.ipv6_sip),
		pktinfo.msg.flow_msg.sport,
		(const struct in6_addr *)&(pktinfo.msg.flow_msg.ipv6_dip),
		pktinfo.msg.flow_msg.dport,
		pktinfo.msg.flow_msg.flow,
		FI_FLOW_LEN + 1,
		((dir == FI_DIR_DOWN) ? true : false));
	head = fi_ipv6_flow_header(hash);
	fi_ipv6_flow_lock(hash);
	flow = fi_ipv6_flow_get(head, kuid.val,
		(const struct in6_addr *)&(pktinfo.msg.flow_msg.ipv6_sip),
		pktinfo.msg.flow_msg.sport,
		(const struct in6_addr *)&(pktinfo.msg.flow_msg.ipv6_dip),
		pktinfo.msg.flow_msg.dport,
		pktinfo.flow_cb, ((dir == FI_DIR_UP) ? true : false),
		&(pktinfo.msg.flow_msg.new_flow), sk);
	if (flow == NULL) {
		fi_ipv6_flow_unlock(hash);
		return;
	}

	if (pktinfo.msg.flow_msg.l4proto == IPPROTO_TCP &&
		flow->sk && flow->sk->sk_state >= TCP_FIN_WAIT1) {
		fi_logd("Ignore flow:l4proto=%d,sk=%pK, sk_state=%d port[%hu--%hu]",
			pktinfo.msg.flow_msg.l4proto, flow->sk, flow->sk->sk_state,
			pktinfo.msg.flow_msg.sport, pktinfo.msg.flow_msg.dport);
		fi_ipv6_flow_unlock(hash);
		return;
	}

	if (pktinfo.msg.flow_msg.new_flow == true) {
		if (state->out) {
			fi_flow_get_dev(&pktinfo.msg.flow_msg, state->out);
			fi_flow_get_dev(&flow->flow_ctx.flow_msg, state->out);
		} else {
			fi_logi("new flow dev is null");
		}
	}
	if (fi_flow_qos_rpt_enable(flow) && pktinfo.msg.flow_msg.l4proto == IPPROTO_TCP) {
		if (dir == FI_DIR_UP) {
			fi_filesize_update(skb, flow);
		} else {
			fi_qos_update(skb, flow);
		}
	}
	/* if all stat report have stopped, bypass this packet */
	if ((flow->flow_ctx.stat_stop & FI_FLOW_ALL_REPORT_STOP) == FI_FLOW_ALL_REPORT_STOP
			&& !flow->flow_cb.flow_identify_cfg->basic_rpt_en) {
		fi_ipv6_flow_unlock(hash);
		return;
	}
	if (dir == FI_DIR_DOWN) {
		pktinfo.flow_cb = flow->flow_cb;
		pktinfo.rand_num = pktinfo.flow_cb.app_info->rand_num;
		pktinfo.msg.flow_msg.uid = flow->uid;
		pktinfo.msg.flow_msg.dev_if = flow->flow_ctx.flow_msg.dev_if;
		if (flow->flow_ctx.key_info_check_rst == FI_RPT_NULL) {
			int http_rsp_code = fi_parse_http_rsp_code(skb);
			if (http_rsp_code > 0 && fi_is_blocked_rsp_code(http_rsp_code))
				flow->flow_ctx.key_info_check_rst = FI_RPT_BLOCK;
			else if (http_rsp_code > 0 || http_rsp_code == -1)
				flow->flow_ctx.key_info_check_rst = FI_RPT_GO;
		}
	}
	pktinfo.dir = dir;
	fi_ipv6_pkt_parse(&pktinfo, skb);

	fi_flow_statistics(flow, &pktinfo, dir);
	enable = fi_pkt_report_is_enable(flow, &pktinfo, dir) && flow->flow_ctx.key_info_check_rst != FI_RPT_BLOCK;
	if (enable) {
		int ret = strcpy_s(pktinfo.msg.flow_msg.dev, IFNAMSIZ, flow->flow_ctx.flow_msg.dev);
		if (ret)
			fi_loge("strcpy_s failed ret=%d, dev=%s", ret, flow->flow_ctx.flow_msg.dev);
	}
	fi_ipv6_flow_unlock(hash);

	if (enable)
		fi_pkt_msg_report(&pktinfo, FI_PKT_MSG_RPT);
	return;
}

static unsigned int fi_ipv6_hook_out(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state)
{
	fi_ipv6_hook(skb, state, FI_DIR_UP);
	return NF_ACCEPT;
}

static unsigned int fi_ipv6_hook_in(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state)
{
	fi_ipv6_hook(skb, state, FI_DIR_DOWN);
	return NF_ACCEPT;
}
#endif

void fi_init(struct sock *nlfd)
{
	int ret;
	int i;

	ret = memset_s(&g_fi_app_info, sizeof(g_fi_app_info), 0, sizeof(g_fi_app_info));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++)
		spin_lock_init(&(g_fi_app_info.app[i].lock));
	atomic_set(&(g_fi_app_info.free_cnt), FI_LAUNCH_APP_MAX);
	fi_logd("app_info init success");

	fi_ipv4_flow_init();
	fi_logd("IPV4 flow table init success");
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_init();
	fi_logd("IPV6 flow table init success");
#endif

	ret  = memset_s(&g_fi_ctx, sizeof(g_fi_ctx), 0, sizeof(g_fi_ctx));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	mutex_init(&g_fi_ctx.nf_mutex);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	init_timer(&(g_fi_ctx.tm));
	g_fi_ctx.tm.function = fi_timer;
#else
	timer_setup(&(g_fi_ctx.tm), fi_timer, 0);
#endif
	g_fi_ctx.nlfd = nlfd;
	g_fi_ctx.running = true;
	skb_queue_head_init(&g_fi_ctx.skb_queue);
	sema_init(&g_fi_ctx.netlink_sync_sema, 0);
	g_fi_ctx.netlink_task = kthread_run(fi_netlink_thread, NULL, "fi_netlink_thread");
	fi_logi("init success");
	return;
}
EXPORT_SYMBOL(fi_init); /*lint !e580*/

void fi_deinit(void)
{
	int ret;

	fi_unregister_nfhook();

	ret = memset_s(&g_fi_app_info, sizeof(g_fi_app_info), 0, sizeof(g_fi_app_info));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}
	fi_logd("app_info deinit success");

	fi_ipv4_flow_clear();
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_clear();
#endif
	fi_logd("flow table deinit success");

	g_fi_ctx.running = false;
	if (g_fi_ctx.netlink_task != NULL) {
		kthread_stop(g_fi_ctx.netlink_task);
		g_fi_ctx.netlink_task = NULL;
	}
	fi_empty_netlink_skb_queue();

	fi_logi("deinit success");
	return;
}
EXPORT_SYMBOL(fi_deinit); /*lint !e580*/
/**
 * fi_mstamp_get - get current timestamp
 * @cl: place to store timestamps
 */
static void fi_mstamp_get(struct fi_mstamp *cl)
{
	u64 val = local_clock();
	do_div(val, NSEC_PER_USEC);
	cl->stamp_us = (u32)val;
	cl->stamp_jiffies = (u32)jiffies;
}

/**
 * fi_mstamp_delta - compute the difference in usec between two fi_mstamp
 * @t1: pointer to newest sample
 * @t0: pointer to oldest sample
 */
static u32 fi_mstamp_us_delta(struct fi_mstamp *t1, struct fi_mstamp *t0)
{
	s32 delta_us = t1->stamp_us - t0->stamp_us;
	u32 delta_jiffies = t1->stamp_jiffies - t0->stamp_jiffies;

	/* If delta_us is negative, this might be because interval is too big,
	 * or local_clock() drift is too big : fallback using jiffies.
	 */
	if ((delta_us <= 0) || (delta_jiffies >= (INT_MAX / (USEC_PER_SEC / HZ))))
		delta_us = jiffies_to_usecs(delta_jiffies);

	return delta_us;
}

void fi_bw_calc(struct fi_flow_node *flow)
{
	uint32_t delta_us;

	if ((flow->rcv_bw.first_rcv_time.stamp_jiffies == 0) && (flow->rcv_bw.first_rcv_time.stamp_us == 0))
		return;
	delta_us = fi_mstamp_us_delta(&(flow->rcv_bw.last_rcv_time), &(flow->rcv_bw.first_rcv_time));
	if (flow->rcv_bw.total_bytes == 0) {
		if (flow->rcv_bw.burst_bytes < FI_MIN_BURST_SIZE || delta_us * FI_TIMER_INTERVAL < USEC_PER_SEC)
			return;
	}
	if (flow->rcv_bw.burst_bytes >= FI_MIN_BURST_SIZE) {
		flow->rcv_bw.total_bytes += flow->rcv_bw.burst_bytes;
		flow->rcv_bw.total_time += delta_us;
	}
	if (flow->rcv_bw.total_time < FI_MIN_BURST_DURATION)
		flow->qos.bw_est = (uint64_t)flow->rcv_bw.total_bytes * USEC_PER_SEC / FI_MIN_BURST_DURATION;
	else
		flow->qos.bw_est = (uint64_t)flow->rcv_bw.total_bytes * USEC_PER_SEC / flow->rcv_bw.total_time;
	fi_logi("bw: uid %u, sport: %u, total_bytes: %u, total_time: %u, bw_est: %u",
			flow->uid, flow->sport, flow->rcv_bw.total_bytes, flow->rcv_bw.total_time, flow->qos.bw_est);
	flow->rcv_bw.total_bytes = 0;
	flow->rcv_bw.total_time = 0;
}

void fi_bw_update(struct fi_flow_node *flow)
{
	if (flow->rcv_bw.burst_bytes > FI_MIN_BURST_SIZE) {
		uint32_t delta_us = fi_mstamp_us_delta(&(flow->rcv_bw.last_rcv_time), &(flow->rcv_bw.first_rcv_time));
		fi_logd("burst: uid %u, sport: %u, burst_bytes: %u, delta_us: %u",
			flow->uid, flow->sport, flow->rcv_bw.burst_bytes, delta_us);
		delta_us = (delta_us > FI_MIN_BURST_DURATION) ? delta_us : FI_MIN_BURST_DURATION;
		flow->rcv_bw.total_bytes += flow->rcv_bw.burst_bytes;
		flow->rcv_bw.total_time += delta_us;
	}
}

void fi_rtt_update(struct fi_flow_node *flow)
{
	unsigned long now_jiffies = jiffies;
	u64 now_mstamp = tcp_clock_us();
	struct tcp_sock *tp = NULL;
	uint32_t srtt;
	uint32_t rcv_rtt;
	unsigned long srtt_time;
	u64 rcv_rtt_time;
	u32 delta_us;

	if ((flow->sk == NULL) || (flow->sk->sk_protocol != IPPROTO_TCP)
		|| (flow->sk->sk_state != TCP_ESTABLISHED && flow->sk->sk_state != TCP_SYN_SENT))
		return;

	tp = tcp_sk(flow->sk);
	srtt = tp->srtt_us >> FI_RTT_SHIFT;
	srtt_time = tp->rcv_tstamp;
	rcv_rtt = tp->rcv_rtt_est.rtt_us >> FI_RTT_SHIFT;
	rcv_rtt_time = tp->rcv_rtt_est.time;
	fi_logd("sport %u now %lu srtt_time %lu srtt %u now_mstamp %ju rcv_rtt_time %ju rcv_rtt %u",
			flow->sk->sk_num, now_jiffies, srtt_time, srtt, now_mstamp, rcv_rtt_time, rcv_rtt);
	fi_logd("updatetime %u tcp_mstamp %ju mdev_max_us %u rttvar_us %u total_retrans %u retrans_stamp %u",
			flow->updatetime, tp->tcp_mstamp, tp->mdev_us, tp->rttvar_us, tp->total_retrans, tp->retrans_stamp);
	/* SYN TIMEOUT */
	if (flow->sk->sk_state == TCP_SYN_SENT && tp->total_retrans > 0) {
		if (!strncmp(FI_CELL0_IFNAME, flow->flow_ctx.flow_msg.dev, strlen(flow->flow_ctx.flow_msg.dev)) ||
			!strncmp(FI_CELL1_IFNAME, flow->flow_ctx.flow_msg.dev, strlen(flow->flow_ctx.flow_msg.dev)))
			flow->qos.rtt = tp->total_retrans * USEC_PER_SEC;
		else
			flow->qos.rtt = 0;
		fi_logi("syn time out, set rtt = %u", flow->qos.rtt);
		return;
	}
	if (rcv_rtt == 0) {
		flow->qos.rtt = srtt;
		return;
	}
	delta_us = tcp_stamp_us_delta(now_mstamp, rcv_rtt_time);
	/*lint -e666*/
	if (time_before(now_jiffies, srtt_time + msecs_to_jiffies(MSEC_PER_SEC)) ||
			time_before(now_jiffies, srtt_time + usecs_to_jiffies(delta_us)) || srtt < tp->rcv_rtt_est.rtt_us)
		flow->qos.rtt = srtt;
	else
		flow->qos.rtt = rcv_rtt;
	/*lint +e666*/
}

void fi_qos_update(const struct sk_buff *skb, struct fi_flow_node *flow)
{
	uint32_t max_delta;
	uint32_t delta_time_last;
	uint32_t delta_time_first;
	struct fi_mstamp now;
	fi_mstamp_get(&now);
	if (unlikely(flow->qos.rtt == 0))
		fi_rtt_update(flow);
	if (flow->sk == NULL || flow->sk->sk_protocol != IPPROTO_TCP)
		return;
	max_delta = flow->qos.rtt >> 1;
	delta_time_last = fi_mstamp_us_delta(&now, &(flow->rcv_bw.last_rcv_time));
	delta_time_first = fi_mstamp_us_delta(&now, &(flow->rcv_bw.first_rcv_time));
	fi_logd(" dev %s, sport %u, jiffies %u us %u seq %u len %u", flow->flow_ctx.flow_msg.dev, flow->sk->sk_num,
		now.stamp_jiffies, now.stamp_us, TCP_SKB_CB(skb)->seq, skb->len);
	if (delta_time_last <= max_delta || delta_time_last <= FI_MIN_DELTA_US || delta_time_first <= flow->qos.rtt) {
		flow->rcv_bw.burst_bytes += skb->len;
	} else {
		fi_bw_update(flow);
		flow->rcv_bw.first_rcv_time = now;
		flow->rcv_bw.burst_bytes = 0;
	}
	flow->rcv_bw.last_rcv_time = now;
	flow->qos.rcv_bytes += skb->len;
}

void fi_filesize_update(const struct sk_buff *skb, struct fi_flow_node *flow)
{
	uint32_t download_bytes;
	flow->flow_ctx.up_pkt_num++;
	if (skb->len <= skb_transport_offset(skb) + tcp_hdrlen(skb) ||
		(flow->sk == NULL || flow->sk->sk_protocol != IPPROTO_TCP)) {
		return;
	} else {
		struct tcp_sock *tp = tcp_sk(flow->sk);
		if (unlikely(flow->file_size.last_rcv_bytes == 0)) {
			flow->file_size.last_rcv_bytes = tp->bytes_received;
			return;
		}

		if (tp->bytes_received < flow->file_size.last_rcv_bytes)
			return;

		download_bytes = tp->bytes_received - flow->file_size.last_rcv_bytes;
		flow->file_size.last_rcv_bytes = tp->bytes_received;
		if (download_bytes > flow->file_size.size) {
			flow->file_size.size = download_bytes;
			fi_logd("sport %u filesize %u", flow->sk->sk_num, flow->file_size.size);
		}
	}
}

int fi_iface_msg_by_index(char *data, int index, uint32_t size)
{
	struct fi_iface_msg iface_msg;
	struct net_device *dev = NULL;
	struct rtnl_link_stats64 stats;
	char *iface = NULL;
	int ret;
	iface = emcom_xengine_get_network_iface_name(index);
	ret = strncpy_s(iface_msg.dev, IFNAMSIZ, iface, strlen(iface));
	if (ret != EOK) {
		fi_loge("strncpy_s failed ret: %d, iface: %s", ret, iface);
		return -1;
	}

	dev = dev_get_by_name(&init_net, iface);
	if (dev) {
		read_lock(&dev_base_lock);
		dev_get_stats(dev, &stats);
		read_unlock(&dev_base_lock);
		dev_put(dev);
		if (stats.rx_bytes > g_fi_iface_bytes[index])
			iface_msg.rcv_bytes = stats.rx_bytes - g_fi_iface_bytes[index];
		else
			iface_msg.rcv_bytes = 0;
		g_fi_iface_bytes[index] = stats.rx_bytes;
	} else {
		iface_msg.rcv_bytes = 0;
		/* clear g_fi_iface_bytes if dev is off */
		g_fi_iface_bytes[index] = 0;
		fi_logi("can't find dev %s", iface);
		iface_msg.dev[0] = '\0';
	}
	fi_logi("Periodic report. iface: %s, total_byes: %lu", iface, g_fi_iface_bytes[index]);
	ret = memcpy_s(data + index * sizeof(struct fi_iface_msg), size - index * sizeof(struct fi_iface_msg),
		&iface_msg, sizeof(struct fi_iface_msg));
	if (ret != EOK) {
		fi_loge("memcpy_s failed ret: %d, size: %u", ret, size);
		return -1;
	}
	fi_logi("iface %s message rcv_bytes: %u", iface_msg.dev, iface_msg.rcv_bytes);
	return 0;
}

void fi_iface_report(void)
{
	int i;
	uint32_t size = sizeof(struct fi_iface_msg) * FI_DEV_NUM;
	char *data = NULL;
	struct sk_buff *pskb = NULL;
	if (!g_fi_ctx.iface_qos_rpt_enable) {
		return;
	}
	pskb = fi_get_netlink_skb(FI_IFACE_MSG_RPT, size, &data);
	if (pskb == NULL) {
		fi_loge("failed to malloc memory for data");
		return;
	}
	for (i = 0; i < FI_DEV_NUM; i++) {
		if (fi_iface_msg_by_index(data, i, size) != 0) {
			kfree_skb(pskb);
			return;
		}
	}
	fi_enqueue_netlink_skb(pskb);
	return;
}
