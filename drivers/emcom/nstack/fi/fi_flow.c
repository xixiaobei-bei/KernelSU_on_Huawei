/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: This module is flow table for FI
 * Author: songqiubin songqiubin@huawei.com
 * Create: 2019-09-18
 */

#include "fi_flow.h"
#include <linux/list.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <net/inet6_hashtables.h>
#include <net/inet_sock.h>
#include <net/ip.h>
#include <net/tcp.h>
#include "fi.h"
#include "fi_utils.h"

static struct fi_flow g_fi_ipv4_flow;
#if IS_ENABLED(CONFIG_IPV6)
static struct fi_flow g_fi_ipv6_flow;
#endif

struct sk_buff *fi_get_netlink_skb(int type, int len, char **data);
void fi_enqueue_netlink_skb(struct sk_buff *pskb);

uint32_t fi_ipv4_flow_node_num(void)
{
	return atomic_read(&g_fi_ipv4_flow.node_num);
}

void fi_ipv4_flow_lock(uint32_t index)
{
	spin_lock_bh(&(g_fi_ipv4_flow.flow_lock[index & FI_FLOW_TABLE_MASK]));
}

void fi_ipv4_flow_unlock(uint32_t index)
{
	spin_unlock_bh(&(g_fi_ipv4_flow.flow_lock[index & FI_FLOW_TABLE_MASK]));
}

uint32_t fi_ipv4_flow_hash(__be32 addr1, __u16 port1,
			__be32 addr2, __be16 port2, char *flow, int str_len, bool exchange)
{
	int ret;
	static u32 fi_ipv4_flow_secret __read_mostly;

	net_get_random_once(&fi_ipv4_flow_secret, sizeof(fi_ipv4_flow_secret));
	if (exchange) {
		if (flow != NULL) {
			ret = sprintf_s(flow, str_len, "%u%u%u%u", addr2, port2, addr1, port1);
			if (ret == -1) {
				fi_loge("sprintf_s flow err, ret %d", ret);
				flow = NULL;
			}
		}
		return __inet_ehashfn(addr2, port2, addr1, port1, fi_ipv4_flow_secret) & FI_FLOW_TABLE_MASK;
	}
	if (flow != NULL) {
		ret = sprintf_s(flow, str_len, "%u%u%u%u", addr1, port1, addr2, port2);
		if (ret == -1) {
			fi_loge("sprintf_s flow err, ret %d", ret);
			flow = NULL;
		}
	}
	return __inet_ehashfn(addr1, port1, addr2, port2, fi_ipv4_flow_secret) & FI_FLOW_TABLE_MASK;
}

inline struct fi_flow_head *fi_ipv4_flow_header(uint32_t index)
{
	return &(g_fi_ipv4_flow.flow_table[index & FI_FLOW_TABLE_MASK]);
}

bool fi_ipv4_flow_same(struct fi_flow_node *node, __be32 saddr, __u16 sport, __be32 daddr, __be16 dport)
{
	return (((node->ipv4_saddr == saddr)
			&& (node->ipv4_daddr == daddr)
			&& (node->sport == sport)
			&& (node->dport == dport))
			|| ((node->ipv4_saddr == daddr)
			&& (node->ipv4_daddr == saddr)
			&& (node->sport == dport)
			&& (node->dport == sport)));
}

struct fi_flow_node *fi_ipv4_flow_find(const struct fi_flow_head *head,
			__be32 saddr, __u16 sport, __be32 daddr, __be16 dport)
{
	struct fi_flow_node *node = NULL;

	if (head != NULL) {
		list_for_each_entry(node, &(head->list), list) {
			if (fi_ipv4_flow_same(node, saddr, sport, daddr, dport)) {
				node->updatetime = jiffies_to_msecs(jiffies);
				return node;
			}
		}
	}
	return NULL;
}

static int fi_flow_ctx_init(struct fi_flow_node *newnode, fi_flow_cb *flow_cb)
{
	int i;
	int ret = memset_s(&newnode->flow_ctx, sizeof(struct fi_flow_ctx), 0, sizeof(struct fi_flow_ctx));
	if (ret) {
		fi_loge("memset_s failed]ret=%d", ret);
		return ret;
	}
	newnode->flow_ctx.last_up_timestamp = jiffies;
	newnode->flow_ctx.flow_stat.up_down_diff_time = -1;
	if (flow_cb != NULL) {
		for (i = 0; i < FI_DIR_BOTH; i++) {
			if (flow_cb->flow_identify_cfg->periodic[i].report_type == FI_RPT_TIME_CTRL) {
				int len = flow_cb->flow_identify_cfg->periodic[i].roi_stop -
					flow_cb->flow_identify_cfg->periodic[i].roi_start;
				if (len <= 0) {
					fi_logi("time interval config err]len=%d", len);
					flow_cb->flow_identify_cfg->periodic[i].report_type = FI_RPT_OFF;
					return -1;
				}
				newnode->flow_ctx.periodic_ctrl[i].ts_start = jiffies
					+ msecs_to_jiffies(flow_cb->flow_identify_cfg->periodic[i].roi_start);
				newnode->flow_ctx.periodic_ctrl[i].ts_end = newnode->flow_ctx.periodic_ctrl[i].ts_start
					+ msecs_to_jiffies(len);
			}
			if (flow_cb->flow_identify_cfg->pkt[i].report_type == FI_RPT_TIME_CTRL) {
				int len = flow_cb->flow_identify_cfg->pkt[i].roi_stop - flow_cb->flow_identify_cfg->pkt[i].roi_start;
				if (len <= 0) {
					fi_logi("time interval config err]len=%d", len);
					flow_cb->flow_identify_cfg->pkt[i].report_type = FI_RPT_OFF;
					return -1;
				}
				newnode->flow_ctx.pkt_ctrl[i].ts_start = jiffies
					+ msecs_to_jiffies(flow_cb->flow_identify_cfg->pkt[i].roi_start);
				newnode->flow_ctx.pkt_ctrl[i].ts_end = newnode->flow_ctx.pkt_ctrl[i].ts_start
					+ msecs_to_jiffies(len);
			}
		}
	}
	return ret;
}

struct fi_flow_node *fi_ipv4_flow_add(struct fi_flow_head *head, int32_t uid,
			__be32 saddr, __u16 sport, __be32 daddr, __be16 dport, fi_flow_cb flow_cb, struct sock *sk)
{
	struct fi_flow_node *newnode = NULL;
	int ret;
	if (head != NULL) {
		if (atomic_read(&g_fi_ipv4_flow.node_num) > FI_FLOW_NODE_LIMIT) {
			fi_loge("IPV4 flow node out of limited %d", FI_FLOW_NODE_LIMIT);
			return NULL;
		}
		newnode = (struct fi_flow_node *)kzalloc(sizeof(struct fi_flow_node), GFP_ATOMIC);
		if (newnode == NULL) {
			fi_loge("failed to malloc new IPV4 node for uid:%d", uid);
			return NULL;
		}
		ret = memset_s(newnode, sizeof(struct fi_flow_node), 0, sizeof(struct fi_flow_node));
		if (ret) {
			kfree(newnode);
			fi_loge("memset_s failed ret=%d", ret);
			return NULL;
		}
		newnode->ipv4_saddr = saddr;
		newnode->ipv4_daddr = daddr;
		newnode->sport = sport;
		newnode->dport = dport;
		newnode->uid = uid;
		newnode->updatetime = jiffies_to_msecs(jiffies);
		newnode->flow_cb = flow_cb;
		if (fi_flow_ctx_init(newnode, &flow_cb)) {
			fi_loge("fi_flow_ctx_init failed");
			kfree(newnode);
			return NULL;
		}
		if (sk != NULL && (sk->sk_protocol == IPPROTO_TCP || sk->sk_protocol == IPPROTO_UDP)) {
			newnode->sk = sk;
			sock_hold(sk);
		}
		list_add(&(newnode->list), &(head->list));
		atomic_inc(&g_fi_ipv4_flow.node_num);
		fi_logd("add a new IPV4 flow, uid:%d", uid);
	}
	return newnode;
}

struct fi_flow_node *fi_ipv4_flow_get(struct fi_flow_head *head, int32_t uid,
			__be32 saddr, __u16 sport, __be32 daddr, __be16 dport,
			fi_flow_cb flow_cb, bool addflow, bool *newflow, struct sock *sk)
{
	struct fi_flow_node *node = NULL;

	if (head != NULL && newflow != NULL) {
		node = fi_ipv4_flow_find(head, saddr, sport, daddr, dport);
		if (node != NULL) {
			if (addflow && sk && ((node->flow_ctx.flow_msg.dev_if != sk->sk_bound_dev_if) ||
				(node->sk && sk != node->sk))) {
				fi_logi("dev_if change rebuild flow, uid:%d port[%hu -- %hu] dev_if[%d -- %d]",
					uid, sport, dport, node->flow_ctx.flow_msg.dev_if, sk->sk_bound_dev_if);
				fi_ipv4_flow_clear_node(node);
				return NULL;
			} else {
				return node;
			}
		}
		if (addflow) {
			if (sk && sk->sk_state >= TCP_FIN_WAIT1 && sk->sk_protocol == IPPROTO_TCP) {
				fi_logd("Ignore flow add:sk_state=%d", sk->sk_state);
				*newflow = false;
				return NULL;
			}
			*newflow = true;
			return fi_ipv4_flow_add(head, uid, saddr, sport, daddr, dport, flow_cb, sk);
		}
		*newflow = false;
	}
	return NULL;
}

void fi_ipv4_flow_clear_node(struct fi_flow_node *node)
{
	list_del(&(node->list));
	if (node->sk != NULL) {
		sock_put(node->sk);
		node->sk = NULL;
	}
	kfree(node);
	node = NULL;
	atomic_dec(&g_fi_ipv4_flow.node_num);
}

void fi_ipv4_flow_age(void)
{
	uint32_t curtime;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int i;

	curtime = jiffies_to_msecs(jiffies);

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_ipv4_flow.flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		fi_ipv4_flow_lock(i);
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			if (node->sk && ((node->sk->sk_protocol == IPPROTO_TCP && node->sk->sk_state >= TCP_FIN_WAIT1) ||
				node->flow_ctx.flow_msg.dev_if != node->sk->sk_bound_dev_if ||
				node->sk->sk_err == ECONNABORTED || sock_flag(node->sk, SOCK_DEAD))) {
				fi_logi("age free flow, node->uid=%d [%hu--%hu] uid=%d [%hu--%hu] protocol=%d sk_state=%d flag=%d",
					node->uid, node->sport, node->dport, node->flow_ctx.flow_msg.uid,
					node->flow_ctx.flow_msg.sport, node->flow_ctx.flow_msg.dport,
					node->sk->sk_protocol, node->sk->sk_state, sock_flag(node->sk, SOCK_DEAD));
				fi_ipv4_flow_clear_node(node);
			} else if (curtime - node->updatetime > FI_FLOW_AGING_TIME) {
				fi_ipv4_flow_clear_node(node);
			}
		}
		fi_ipv4_flow_unlock(i);
	}
	return;
}

void fi_ipv4_flow_clear(void)
{
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int i;

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_ipv4_flow.flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		fi_ipv4_flow_lock(i);
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			fi_ipv4_flow_clear_node(node);
		}
		fi_ipv4_flow_unlock(i);
	}
	return;
}

void fi_ipv4_flow_clear_by_uid(int32_t uid)
{
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int i;

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_ipv4_flow.flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		fi_ipv4_flow_lock(i);
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			if (node->uid == uid)
				fi_ipv4_flow_clear_node(node);
		}
		fi_ipv4_flow_unlock(i);
	}
	return;
}

void fi_ipv4_flow_init(void)
{
	int i, ret;

	ret = memset_s(&g_fi_ipv4_flow, sizeof(g_fi_ipv4_flow), 0, sizeof(g_fi_ipv4_flow));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}
	atomic_set(&(g_fi_ipv4_flow.node_num), 0);
	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		INIT_LIST_HEAD(&(g_fi_ipv4_flow.flow_table[i].list));
		spin_lock_init(&(g_fi_ipv4_flow.flow_lock[i]));
	}
	return;
}

#if IS_ENABLED(CONFIG_IPV6)
uint32_t fi_ipv6_flow_node_num(void)
{
	return atomic_read(&g_fi_ipv6_flow.node_num);
}

void fi_ipv6_flow_lock(uint32_t index)
{
	spin_lock_bh(&(g_fi_ipv6_flow.flow_lock[index & FI_FLOW_TABLE_MASK]));
}

void fi_ipv6_flow_unlock(uint32_t index)
{
	spin_unlock_bh(&(g_fi_ipv6_flow.flow_lock[index & FI_FLOW_TABLE_MASK]));
}

uint32_t fi_ipv6_flow_hash(const struct in6_addr *addr1, __u16 port1,
			const struct in6_addr *addr2, __be16 port2, char *flow, int str_len, bool exchange)
{
	int ret;
	static u32 fi_ipv6_flow_secret __read_mostly;
	static u32 fi_ipv6_flow_d_secret __read_mostly;
	u32 hash1;
	u32 hash2;

	net_get_random_once(&fi_ipv6_flow_secret, sizeof(fi_ipv6_flow_secret));
	net_get_random_once(&fi_ipv6_flow_d_secret, sizeof(fi_ipv6_flow_d_secret));
	if (exchange) {
		hash2 = (__force u32)addr2->s6_addr32[3];
		hash1 = __ipv6_addr_jhash(addr1, fi_ipv6_flow_d_secret);
		if (flow != NULL) {
			ret = sprintf_s(flow, str_len, "%u%u%u%u", (__force u32)addr2->s6_addr32[3],
				port2, (__force u32)addr1->s6_addr32[3], port1);
			if (ret == -1) {
				fi_loge("sprintf_s flow err, ret %d", ret);
				flow = NULL;
			}
		}
		return __inet6_ehashfn(hash2, port2, hash1, port1, fi_ipv6_flow_secret) & FI_FLOW_TABLE_MASK;
	}
	hash1 = (__force u32)addr1->s6_addr32[3];
	hash2 = __ipv6_addr_jhash(addr2, fi_ipv6_flow_d_secret);
	if (flow != NULL) {
		ret = sprintf_s(flow, str_len, "%u%u%u%u", (__force u32)addr1->s6_addr32[3],
			port1, (__force u32)addr2->s6_addr32[3], port2);
		if (ret == -1) {
			fi_loge("sprintf_s flow err, ret %d", ret);
			flow = NULL;
		}
	}
	return __inet6_ehashfn(hash1, port1, hash2, port2, fi_ipv6_flow_secret) & FI_FLOW_TABLE_MASK;
}

inline struct fi_flow_head *fi_ipv6_flow_header(uint32_t index)
{
	return &(g_fi_ipv6_flow.flow_table[index & FI_FLOW_TABLE_MASK]);
}

bool fi_ipv6_flow_same(struct fi_flow_node *node,
						const struct in6_addr *saddr, __u16 sport,
						const struct in6_addr *daddr, __be16 dport)
{
	return ((ipv6_addr_equal((const struct in6_addr *)(&(node->ipv6_saddr)), saddr) &&
			ipv6_addr_equal((const struct in6_addr *)(&(node->ipv6_daddr)), daddr) &&
			node->sport == sport && node->dport == dport) ||
			(ipv6_addr_equal((const struct in6_addr *)(&(node->ipv6_saddr)), daddr) &&
			ipv6_addr_equal((const struct in6_addr *)(&(node->ipv6_daddr)), saddr) &&
			node->sport == dport && node->dport == sport));
}

struct fi_flow_node *fi_ipv6_flow_find(const struct fi_flow_head *head,
			const struct in6_addr *saddr, __u16 sport,
			const struct in6_addr *daddr, __be16 dport)
{
	struct fi_flow_node *node = NULL;

	if (saddr != NULL && daddr != NULL && head != NULL) {
		list_for_each_entry(node, &(head->list), list) {
			if (fi_ipv6_flow_same(node, saddr, sport, daddr, dport)) {
				node->updatetime = jiffies_to_msecs(jiffies);
				return node;
			}
		}
	}
	return NULL;
}

struct fi_flow_node *fi_ipv6_flow_add(struct fi_flow_head *head, int32_t uid,
			const struct in6_addr *saddr, __u16 sport,
			const struct in6_addr *daddr, __be16 dport, fi_flow_cb flow_cb,  struct sock *sk)
{
	struct fi_flow_node *newnode = NULL;
	int ret;

	if (saddr != NULL && daddr != NULL && head != NULL) {
		if (atomic_read(&g_fi_ipv6_flow.node_num) > FI_FLOW_NODE_LIMIT) {
			fi_loge("IPV6 flow node out of limited %d", FI_FLOW_NODE_LIMIT);
			return NULL;
		}
		newnode = (struct fi_flow_node *)kzalloc(sizeof(struct fi_flow_node), GFP_ATOMIC);
		if (newnode == NULL) {
			fi_loge("failed to malloc new IPV6 node for uid %d", uid);
			return NULL;
		}
		ret = memset_s(newnode, sizeof(struct fi_flow_node), 0, sizeof(struct fi_flow_node));
		if (ret) {
			kfree(newnode);
			fi_loge("memset_s failed ret=%d", ret);
			return NULL;
		}

		newnode->ipv6_saddr = *saddr;
		newnode->ipv6_daddr = *daddr;
		newnode->sport = sport;
		newnode->dport = dport;
		newnode->uid = uid;
		newnode->updatetime = jiffies_to_msecs(jiffies);
		newnode->flow_cb = flow_cb;
		if (fi_flow_ctx_init(newnode, &flow_cb)) {
			fi_loge("fi_flow_ctx_init failed");
			kfree(newnode);
			return NULL;
		}

		if (sk != NULL && (sk->sk_protocol == IPPROTO_TCP || sk->sk_protocol == IPPROTO_UDP)) {
			newnode->sk = sk;
			sock_hold(sk);
		}
		list_add(&(newnode->list), &(head->list));
		atomic_inc(&g_fi_ipv6_flow.node_num);
	}
	return newnode;
}

struct fi_flow_node *fi_ipv6_flow_get(struct fi_flow_head *head, int32_t uid,
			const struct in6_addr *saddr, __u16 sport,
			const struct in6_addr *daddr, __be16 dport,
			fi_flow_cb flow_cb, bool addflow, bool *newflow, struct sock *sk)
{
	struct fi_flow_node *node = NULL;

	if (saddr != NULL && daddr != NULL && head != NULL && newflow != NULL) {
		node = fi_ipv6_flow_find(head, saddr, sport, daddr, dport);
		if (node != NULL) {
			if (addflow && sk && ((node->flow_ctx.flow_msg.dev_if != sk->sk_bound_dev_if) ||
				(node->sk && sk != node->sk))) {
				fi_logi("dev_if change rebuild flow, uid:%d port[%hu -- %hu] dev_if[%d -- %d]",
					uid, sport, dport, node->flow_ctx.flow_msg.dev_if, sk->sk_bound_dev_if);
				fi_ipv6_flow_clear_node(node);
				return NULL;
			} else {
				return node;
			}
		}
		if (addflow) {
			if (sk && sk->sk_state >= TCP_FIN_WAIT1 && sk->sk_protocol == IPPROTO_TCP) {
				fi_logd("Ignore flow add:sk_state=%d", sk->sk_state);
				*newflow = false;
				return NULL;
			}
			*newflow = true;
			return fi_ipv6_flow_add(head, uid, saddr, sport, daddr, dport, flow_cb, sk);
		}
		*newflow = false;
	}
	return NULL;
}

void fi_ipv6_flow_clear_node(struct fi_flow_node *node)
{
	list_del(&(node->list));
	if (node->sk != NULL) {
		sock_put(node->sk);
		node->sk = NULL;
	}
	kfree(node);
	node = NULL;
	atomic_dec(&g_fi_ipv6_flow.node_num);
}

void fi_ipv6_flow_age(void)
{
	uint32_t curtime;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int i;

	curtime = jiffies_to_msecs(jiffies);

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_ipv6_flow.flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		fi_ipv6_flow_lock(i);
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			if (node->sk && ((node->sk->sk_protocol == IPPROTO_TCP && node->sk->sk_state >= TCP_FIN_WAIT1) ||
				node->flow_ctx.flow_msg.dev_if != node->sk->sk_bound_dev_if ||
				node->sk->sk_err == ECONNABORTED || sock_flag(node->sk, SOCK_DEAD))) {
				fi_logi("age free flow, node->uid=%d [%hu--%hu] uid=%d [%hu--%hu] protocol=%d sk_state=%d flag=%d",
					node->uid, node->sport, node->dport, node->flow_ctx.flow_msg.uid,
					node->flow_ctx.flow_msg.sport, node->flow_ctx.flow_msg.dport,
					node->sk->sk_protocol, node->sk->sk_state, sock_flag(node->sk, SOCK_DEAD));
				fi_ipv6_flow_clear_node(node);
			} else if (curtime - node->updatetime > FI_FLOW_AGING_TIME) {
				fi_ipv6_flow_clear_node(node);
			}
		}
		fi_ipv6_flow_unlock(i);
	}
	return;
}

void fi_ipv6_flow_clear(void)
{
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int i;

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_ipv6_flow.flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		fi_ipv6_flow_lock(i);
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			fi_ipv6_flow_clear_node(node);
		}
		fi_ipv6_flow_unlock(i);
	}
	return;
}

void fi_ipv6_flow_clear_by_uid(int32_t uid)
{
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int i;

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		head = g_fi_ipv6_flow.flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		fi_ipv6_flow_lock(i);
		list_for_each_entry_safe(node, tmp, &(head->list), list) {
			if (node->uid == uid)
				fi_ipv6_flow_clear_node(node);
		}
		fi_ipv6_flow_unlock(i);
	}
	return;
}

void fi_ipv6_flow_init(void)
{
	int i, ret;

	ret = memset_s(&g_fi_ipv6_flow, sizeof(g_fi_ipv6_flow), 0, sizeof(g_fi_ipv6_flow));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	atomic_set(&(g_fi_ipv6_flow.node_num), 0);
	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		INIT_LIST_HEAD(&(g_fi_ipv6_flow.flow_table[i].list));
		spin_lock_init(&(g_fi_ipv6_flow.flow_lock[i]));
	}
	return;
}
#endif

static bool fi_payload_len_filter_check(__be16 payloadlen,
	uint32_t filter_pkt_size_start, uint32_t filter_pkt_size_stop)
{
	if (payloadlen < filter_pkt_size_start
		|| payloadlen > filter_pkt_size_stop) {
		fi_logd("payloadlen not match]payloadlen=%hu", payloadlen);
		return false;
	}
	return true;
}

static bool fi_payload_str_filter_check(struct fi_pkt_parse *pktinfo, fi_rpt_cfg *rpt_cfg)
{
	int i;
	__be16 payloadlen = pktinfo->payloadlen;
	char *payload = pktinfo->payload;

	/* If no filter set, default return match */
	if (!rpt_cfg->filter_str_len[0])
		return true;

	if (payload == NULL)
		return false;

	for (i = 0; i < FI_FLOW_FILTER_STR_MAX; i++) {
		if (!rpt_cfg->filter_str_len[i])
			break;
		if (rpt_cfg->filter_str_offset + rpt_cfg->filter_str_len[i] > payloadlen)
			continue;
		if (memcmp(rpt_cfg->filter_str[i],
				payload + rpt_cfg->filter_str_offset,
				rpt_cfg->filter_str_len[i]) == 0)
			return true;
	}
	return false;
}

void fi_flow_get_filesize(struct fi_flow_node *flow)
{
	uint32_t download_bytes;
	struct tcp_sock *tp = tcp_sk(flow->sk);
	if (unlikely(flow->file_size.last_rcv_bytes == 0))
		return;

	if (tp->bytes_received < flow->file_size.last_rcv_bytes)
		return;

	download_bytes = tp->bytes_received - flow->file_size.last_rcv_bytes;
	if (download_bytes > flow->file_size.size)
		flow->file_size.size = download_bytes;
}

static void fi_flow_get_sk_info(struct fi_flow_node *flow)
{
	if (flow->sk != NULL && ((flow->flow_ctx.flow_msg.dev_if != flow->sk->sk_bound_dev_if) ||
		flow->sk->sk_err == ECONNABORTED || sock_flag(flow->sk, SOCK_DEAD))) {
		fi_logi("sock reset, port[%hu--%hu] l4proto[%d] sk_state[%d] dev_if[%d--%d] sk_err[%d] sk[%pK] flag[%d]",
			flow->flow_ctx.flow_msg.sport, flow->flow_ctx.flow_msg.dport,
			flow->flow_ctx.flow_msg.l4proto, flow->sk->sk_state,
			flow->flow_ctx.flow_msg.dev_if, flow->sk->sk_bound_dev_if,
			flow->sk->sk_err, flow->sk, sock_flag(flow->sk, SOCK_DEAD));
		flow->flow_ctx.flow_stat.sock_state = FLOW_RESET;
	} else {
		flow->flow_ctx.flow_stat.sock_state = flow->sk ? flow->sk->sk_state : 0;
	}

	if (flow->sk != NULL && flow->sk->sk_protocol == IPPROTO_TCP) {
		const struct tcp_sock *tp = tcp_sk(flow->sk);
		unsigned int in_flight = tcp_packets_in_flight(tp);
		if (tp->snd_cwnd > in_flight)
			flow->flow_ctx.flow_stat.cwnd_left = tp->snd_cwnd - in_flight;
		else
			flow->flow_ctx.flow_stat.cwnd_left = 0;
		flow->flow_ctx.flow_stat.total_retrans = tp->total_retrans - flow->flow_ctx.last_total_retrans;
		flow->flow_ctx.last_total_retrans = tp->total_retrans;
	}
}

static void fi_flow_raw_stat_update(struct fi_flow_node *flow, const struct fi_pkt_parse *pktinfo,
	enum fi_dir dir)
{
	struct fi_flow_stat *flow_stat = &flow->flow_ctx.flow_stat;
	if (!pktinfo->flow_cb.flow_identify_cfg->basic_rpt_en)
		return;
	if (dir == FI_DIR_UP) {
		flow_stat->raw_up_pkt_num++;
		flow_stat->raw_up_pkt_byte += pktinfo->msg.payloadlen;
	} else {
		flow_stat->raw_down_pkt_num++;
		flow_stat->raw_down_pkt_byte += pktinfo->msg.payloadlen;
	}
}

static void fi_flow_stat_start(struct fi_flow_node *flow, enum fi_dir dir)
{
	flow->flow_ctx.stat_stop |= (dir == FI_DIR_UP) ? FI_FLOW_PERIODIC_REPORT_NEED_UP : FI_FLOW_PERIODIC_REPORT_NEED_DOWN;
}

static void fi_flow_stat_stop(struct fi_flow_node *flow, enum fi_dir dir)
{
	flow->flow_ctx.stat_stop |= (dir == FI_DIR_UP) ? FI_FLOW_PERIODIC_REPORT_STOP_UP :
		FI_FLOW_PERIODIC_REPORT_STOP_DOWN;

	flow->flow_ctx.stat_stop &= (dir == FI_DIR_UP) ?
		~FI_FLOW_PERIODIC_REPORT_NEED_UP : ~FI_FLOW_PERIODIC_REPORT_NEED_DOWN;
}

static void fi_flow_stat_update(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo,
	enum fi_dir dir)
{
	struct fi_flow_stat *flow_stat = &flow->flow_ctx.flow_stat;

	fi_flow_stat_start(flow, dir);
	if (dir == FI_DIR_UP) {
		flow_stat->up_pkt_num++;
		flow_stat->up_pkt_byte += pktinfo->msg.payloadlen;
	} else {
		flow_stat->down_pkt_num++;
		flow_stat->down_pkt_byte += pktinfo->msg.payloadlen;
	}
}

static void fi_flow_stat_echo_delay(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo,
	enum fi_dir dir)
{
	struct fi_flow_stat *flow_stat = &flow->flow_ctx.flow_stat;
	fi_flow_identify_cfg *flow_cfg = pktinfo->flow_cb.flow_identify_cfg;
	if (!flow_cfg->periodic[dir].opt_enable[FI_ECHO_DELAY] && !flow_cfg->pkt[dir].opt_enable[FI_ECHO_DELAY])
		return;

	if (dir == FI_DIR_UP) {
		flow->flow_ctx.last_up_timestamp = jiffies;
	} else {
		if (flow->flow_ctx.last_up_timestamp) {
			flow_stat->up_down_diff_time = jiffies_to_msecs(jiffies - flow->flow_ctx.last_up_timestamp);
			pktinfo->msg.up_down_diff_time = flow_stat->up_down_diff_time;
			flow->flow_ctx.last_up_timestamp = 0;
		} else {
			pktinfo->msg.up_down_diff_time = -1;
		}
	}
}

static void fi_flow_stat_payload(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo, enum fi_dir dir)
{
	int ret;
	uint32_t offset;
	__be16 payloadlen = pktinfo->payloadlen;
	char *payload = pktinfo->payload;
	struct fi_flow_stat *flow_stat = &flow->flow_ctx.flow_stat;
	fi_flow_identify_cfg *flow_cfg = pktinfo->flow_cb.flow_identify_cfg;

	if (!flow_cfg->periodic[dir].opt_enable[FI_PAYLOAD_SEGMENT_PARSE] ||
		payload == NULL || !payloadlen)
		return;

	if (flow_cfg->periodic[dir].seg_begin >= payloadlen)
		return;
	offset = flow_cfg->periodic[dir].seg_end - flow_cfg->periodic[dir].seg_begin + 1;
	if (flow_cfg->periodic[dir].seg_end >= payloadlen)
		offset = payloadlen - flow_cfg->periodic[dir].seg_begin;
	ret = memcpy_s(flow_stat->payload_seg[dir], sizeof(flow_stat->payload_seg[dir]),
			payload + flow_cfg->periodic[dir].seg_begin, offset);
	if (ret)
		fi_loge("memcpy_s failed ret=%d, len=%u", ret, offset);
	fi_logd("get period payload_seg len=%u, payloadlen=%d begin=%u end=%u", offset,
		payloadlen, flow_cfg->periodic[dir].seg_begin, flow_cfg->periodic[dir].seg_end);
}

void fi_flow_statistics_time_ctrl(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo,
	enum fi_dir dir)
{
	if ((jiffies >= flow->flow_ctx.periodic_ctrl[dir].ts_start)
		&& (jiffies <= flow->flow_ctx.periodic_ctrl[dir].ts_end))
		fi_flow_stat_update(flow, pktinfo, dir);
	else if (jiffies > flow->flow_ctx.periodic_ctrl[dir].ts_end)
		fi_flow_stat_stop(flow, dir);
}

void fi_flow_statistics_num_ctrl(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo,
	enum fi_dir dir)
{
	int num;
	fi_rpt_cfg *periodic_cfg = &pktinfo->flow_cb.flow_identify_cfg->periodic[dir];

	flow->flow_ctx.periodic_ctrl[dir].pkt_num++;
	num = flow->flow_ctx.periodic_ctrl[dir].pkt_num;
	if ((num >= periodic_cfg->roi_start) && (num <= periodic_cfg->roi_stop))
		fi_flow_stat_update(flow, pktinfo, dir);
	else if (num > periodic_cfg->roi_stop)
		fi_flow_stat_stop(flow, dir);
}

void fi_flow_statistics(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo,
	enum fi_dir dir)
{
	int ret;
	bool filter_pass = false;
	fi_rpt_cfg *periodic_cfg = NULL;

	fi_flow_stat_echo_delay(flow, pktinfo, dir);
	fi_flow_raw_stat_update(flow, pktinfo, dir);

	if (flow->flow_ctx.stat_stop & ((dir == FI_DIR_UP) ? FI_FLOW_PERIODIC_REPORT_STOP_UP :
		FI_FLOW_PERIODIC_REPORT_STOP_DOWN))
		return;

	if (pktinfo->msg.flow_msg.new_flow) {
		ret = memcpy_s(&flow->flow_ctx.flow_msg, sizeof(struct fi_flow_msg),
			&pktinfo->msg.flow_msg, sizeof(struct fi_flow_msg));
		if (ret) {
			fi_loge("memcpy_s failed ret=%d", ret);
			return;
		}
	}

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	periodic_cfg = &pktinfo->flow_cb.flow_identify_cfg->periodic[dir];

	if (fi_payload_len_filter_check(pktinfo->msg.payloadlen,
			periodic_cfg->filter_pkt_size_start, periodic_cfg->filter_pkt_size_stop)
		&& fi_payload_str_filter_check(pktinfo, periodic_cfg)) {
		filter_pass = true;
		fi_flow_stat_payload(flow, pktinfo, dir);
	}

	switch (periodic_cfg->report_type) {
	case FI_RPT_ALWAYS:
		if (filter_pass)
			fi_flow_stat_update(flow, pktinfo, dir);
		break;
	case FI_RPT_TIME_CTRL:
		if (filter_pass)
			fi_flow_statistics_time_ctrl(flow, pktinfo, dir);
		break;
	case FI_RPT_NUM_CTRL:
		if (filter_pass)
			fi_flow_statistics_num_ctrl(flow, pktinfo, dir);
		break;
	default:
		flow->flow_ctx.stat_stop |= ((dir == FI_DIR_UP) ? FI_FLOW_PERIODIC_REPORT_STOP_UP :
			FI_FLOW_PERIODIC_REPORT_STOP_DOWN);
		break;
	}
	spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	return;
}

static void fi_ip_stat_periodic_log(const struct fi_flow_node *node)
{
	if (node->flow_ctx.flow_msg.l3proto == ETH_P_IP) {
		fi_logi("dev: %s, SrcIP: "IPV4_FMT", SrcPort: %hu, DstIP: "IPV4_FMT", DstPort: %hu, "
			"Proto: %u, SockState: %u, sk %pK, UpPktNum: %d, DownPktNum: %d, rtt: %u, totalRetrans: %u",
			node->flow_ctx.flow_msg.dev,
			ipv4_info(node->flow_ctx.flow_msg.ipv4_sip), node->flow_ctx.flow_msg.sport,
			ipv4_info(node->flow_ctx.flow_msg.ipv4_dip), node->flow_ctx.flow_msg.dport,
			node->flow_ctx.flow_msg.l4proto, node->flow_ctx.flow_stat.sock_state, node->sk,
			node->flow_ctx.flow_stat.up_pkt_num, node->flow_ctx.flow_stat.down_pkt_num,
			node->flow_ctx.flow_stat.rtt, node->flow_ctx.flow_stat.total_retrans);
#if IS_ENABLED(CONFIG_IPV6)
	} else {
		fi_logi("dev: %s, SrcIP: "IPV6_FMT", SrcPort: %hu, DstIP: "IPV6_FMT", DstPort: %hu, "
			"Proto: %u, SockState: %u, UpPktNum: %d, DownPktNum: %d, rtt: %u, totalRetrans: %u",
			node->flow_ctx.flow_msg.dev,
			ipv6_info(node->flow_ctx.flow_msg.ipv6_sip), node->flow_ctx.flow_msg.sport,
			ipv6_info(node->flow_ctx.flow_msg.ipv6_dip), node->flow_ctx.flow_msg.dport,
			node->flow_ctx.flow_msg.l4proto, node->flow_ctx.flow_stat.sock_state,
			node->flow_ctx.flow_stat.up_pkt_num, node->flow_ctx.flow_stat.down_pkt_num,
			node->flow_ctx.flow_stat.rtt, node->flow_ctx.flow_stat.total_retrans);
#endif
	}
}

bool fi_pkt_report_is_enable(struct fi_flow_node *flow, struct fi_pkt_parse *pktinfo, enum fi_dir dir)
{
	bool enable = false;
	bool filter_pass = false;
	fi_rpt_cfg *pkt_cfg = NULL;

	if (flow->flow_ctx.stat_stop & ((dir == FI_DIR_UP) ? FI_FLOW_PKT_REPORT_STOP_UP :
		FI_FLOW_PKT_REPORT_STOP_DOWN))
		return enable;

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	pkt_cfg = &pktinfo->flow_cb.flow_identify_cfg->pkt[dir];
	if (fi_payload_len_filter_check(pktinfo->msg.payloadlen,
			pkt_cfg->filter_pkt_size_start, pkt_cfg->filter_pkt_size_stop)
		&& fi_payload_str_filter_check(pktinfo, pkt_cfg))
		filter_pass = true;

	switch (pkt_cfg->report_type) {
	case FI_RPT_ALWAYS:
		enable = true;
		break;
	case FI_RPT_TIME_CTRL:
		if ((jiffies >= flow->flow_ctx.pkt_ctrl[dir].ts_start)
			&& (jiffies <= flow->flow_ctx.pkt_ctrl[dir].ts_end))
			enable = true;
		else if (jiffies > flow->flow_ctx.pkt_ctrl[dir].ts_end)
			flow->flow_ctx.stat_stop |= ((dir == FI_DIR_UP) ? FI_FLOW_PKT_REPORT_STOP_UP :
				FI_FLOW_PKT_REPORT_STOP_DOWN);
		break;
	case FI_RPT_NUM_CTRL:
		if (filter_pass) {
			flow->flow_ctx.pkt_ctrl[dir].pkt_num++;
			if ((flow->flow_ctx.pkt_ctrl[dir].pkt_num >= pkt_cfg->roi_start)
				&& (flow->flow_ctx.pkt_ctrl[dir].pkt_num <= pkt_cfg->roi_stop))
				enable = true;
			else if (flow->flow_ctx.pkt_ctrl[dir].pkt_num > pkt_cfg->roi_stop)
				flow->flow_ctx.stat_stop |= ((dir == FI_DIR_UP) ? FI_FLOW_PKT_REPORT_STOP_UP :
					FI_FLOW_PKT_REPORT_STOP_DOWN);
		}
		break;
	default:
		flow->flow_ctx.stat_stop |= ((dir == FI_DIR_UP) ? FI_FLOW_PKT_REPORT_STOP_UP :
				FI_FLOW_PKT_REPORT_STOP_DOWN);
		break;
	}
	spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	return (enable && filter_pass);
}

void fi_ip_flow_periodic_report(const struct fi_flow *flow,
	void (*flow_lock)(uint32_t),
	void (*flow_unlock)(uint32_t))
{
	int i;
	struct report_local_var local_var = {0};
	char *data = NULL;
	struct sk_buff *pskb = NULL;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;

	/*lint -e574*/
	local_var.total_left = atomic_read(&flow->node_num) * sizeof(struct fi_periodic_rpt_msg);
	if (!local_var.total_left) {
		fi_logd("no data need to do periodic report");
		return;
	}

	local_var.total_left = (local_var.total_left > FI_NETLINK_REPORT_MAX_NUM * FI_MAX_ALIGN_SIZE) ?
		(FI_NETLINK_REPORT_MAX_NUM * FI_MAX_ALIGN_SIZE) : local_var.total_left;
	local_var.size = (local_var.total_left > FI_MAX_ALIGN_SIZE) ? FI_MAX_ALIGN_SIZE : local_var.total_left;
	pskb = fi_get_netlink_skb(FI_PERIODIC_MSG_RPT, local_var.size, &data);
	if (pskb == NULL) {
		fi_loge("failed to malloc memory for data");
		return;
	}
	/*lint +e574*/

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		if (list_empty(&(flow->flow_table[i].list))) {
			continue;
		}

		(*flow_lock)(i);
		list_for_each_entry_safe(node, tmp, &(flow->flow_table[i].list), list) {
			if ((node->flow_ctx.stat_stop & FI_FLOW_PERIODIC_REPORT_NEED) ||
					node->flow_cb.flow_identify_cfg->basic_rpt_en) {
				if (node->flow_cb.flow_identify_cfg->basic_rpt_en)
					fi_flow_get_sk_info(node);
				if (node->flow_ctx.key_info_check_rst == FI_RPT_BLOCK &&
					node->flow_ctx.flow_stat.sock_state < FLOW_FINISH)
					continue;
				if (local_var.offset >= local_var.size) {
					/* enqueue pre skb and alloc a new skb */
					fi_enqueue_netlink_skb(pskb);

					local_var.total_left -= local_var.size;
					if (local_var.total_left <= 0) {
						fi_logi("exceed total size]size=%u", local_var.size);
						(*flow_unlock)(i);
						return;
					}
					local_var.size = FI_MAX_ALIGN_SIZE;
					pskb = fi_get_netlink_skb(FI_PERIODIC_MSG_RPT, local_var.size, &data);
					if (pskb == NULL) {
						fi_loge("failed get netlink skb");
						(*flow_unlock)(i);
						return;
					}
					local_var.offset = 0;
				}
				node->flow_ctx.flow_stat.rtt = node->qos.rtt;
				local_var.ret = memcpy_s(data + local_var.offset, local_var.size - local_var.offset,
					&node->flow_ctx.flow_stat, sizeof(struct fi_periodic_rpt_msg));
				if (local_var.ret) {
					fi_loge("memcpy_s failed]ret=%d,size=%u,offset=%u",
						local_var.ret, local_var.size, local_var.offset);
					kfree_skb(pskb);
					(*flow_unlock)(i);
					return;
				}
				fi_ip_stat_periodic_log(node);
				local_var.ret = memset_s(&node->flow_ctx.flow_stat,
					sizeof(struct fi_flow_stat), 0, sizeof(struct fi_flow_stat));
				if (local_var.ret)
					fi_loge("memset_s failed ret=%d", local_var.ret);

				node->flow_ctx.flow_stat.up_down_diff_time = -1;
				local_var.offset += sizeof(struct fi_periodic_rpt_msg);
			}
		}
		(*flow_unlock)(i);
	}
	if (pskb != NULL) {
		if (local_var.offset == 0) {
			kfree_skb(pskb);
			return;
		} else if (local_var.offset < local_var.size) {
			struct nlmsghdr *nlh = (struct nlmsghdr *)(data - NLMSG_HDRLEN);
			nlh->nlmsg_len = local_var.offset + NLMSG_HDRLEN;
			fi_logd("need to adjust netlink len]size=%u,offset=%u", local_var.size, local_var.offset);
		}
		fi_enqueue_netlink_skb(pskb);
	}
}

void fi_flow_periodic_report(void)
{
	fi_ip_flow_periodic_report(&g_fi_ipv4_flow,
		fi_ipv4_flow_lock, fi_ipv4_flow_unlock);
#if IS_ENABLED(CONFIG_IPV6)
	fi_ip_flow_periodic_report(&g_fi_ipv6_flow,
		fi_ipv6_flow_lock, fi_ipv6_flow_unlock);
#endif
}

void fi_flow_qos_msg_log(struct fi_flow_node *node)
{
	if (node->flow_ctx.flow_msg.l3proto == ETH_P_IP) {
		fi_logi("Periodic qos report. SrcIP: "IPV4_FMT", SrcPort: %hu, DstIP: "IPV4_FMT", DstPort: %hu, "
			"uid: %u, dev %s, rtt: %u, bw: %u, rcv_bytes: %u, file_size %u up_pkt_num %d",
			ipv4_info(node->flow_ctx.flow_msg.ipv4_sip), node->flow_ctx.flow_msg.sport,
			ipv4_info(node->flow_ctx.flow_msg.ipv4_dip), node->flow_ctx.flow_msg.dport,
			node->uid, node->flow_ctx.flow_msg.dev, node->qos.rtt, node->qos.bw_est,
			node->qos.rcv_bytes, node->file_size.size, node->flow_ctx.up_pkt_num);
	}
#if IS_ENABLED(CONFIG_IPV6)
	else {
		fi_logi("Periodic qos report. SrcIP: "IPV6_FMT", SrcPort: %hu, DstIP: "IPV6_FMT", DstPort: %hu, "
			"uid: %u, dev %s, rtt: %u, bw: %u, rcv_bytes: %u, file_size %u up_pkt_num %d",
			ipv6_info(node->flow_ctx.flow_msg.ipv6_sip), node->flow_ctx.flow_msg.sport,
			ipv6_info(node->flow_ctx.flow_msg.ipv6_dip), node->flow_ctx.flow_msg.dport,
			node->uid, node->flow_ctx.flow_msg.dev, node->qos.rtt, node->qos.bw_est,
			node->qos.rcv_bytes, node->file_size.size, node->flow_ctx.up_pkt_num);
	}
#endif
}

int fi_flow_qos_msg(const struct fi_flow_head *head, struct sk_buff **pskb, char **data, uint32_t *offset, uint32_t *size)
{
	int ret;
	struct fi_flow_node *node = NULL;
	struct fi_flow_node *tmp = NULL;
	int buff_num = 0;
	struct fi_qos_rpt_msg flow_qos_msg;
	list_for_each_entry_safe(node, tmp, &(head->list), list) {
		if (fi_flow_qos_rpt_enable(node)) {
			if (node->sk == NULL || node->sk->sk_protocol != IPPROTO_TCP
			|| (node->qos.rcv_bytes == 0 && node->sk->sk_state != TCP_SYN_SENT)
			|| (node->flow_ctx.up_pkt_num == 0 && node->sk->sk_state == TCP_SYN_SENT))
				continue;
			fi_rtt_update(node);
			if (node->qos.rtt == 0)
				continue;
			fi_bw_calc(node);
			fi_flow_get_filesize(node);
			if (node->flow_ctx.key_info_check_rst == FI_RPT_BLOCK)
				continue;
			fi_flow_qos_msg_log(node);
			if (*offset >= *size) {
				/* enqueue pre skb and alloc a new skb */
				fi_enqueue_netlink_skb(*pskb);
				buff_num += 1;
				if (buff_num >= FI_NETLINK_REPORT_MAX_NUM) {
					fi_logi("exceed max report num %u", FI_NETLINK_REPORT_MAX_NUM);
					return -1;
				}
				*pskb = fi_get_netlink_skb(FI_QOS_MSG_RPT, *size, data);
				if (*pskb == NULL) {
					fi_loge("failed get netlink skb");
					return -1;
				}
				*offset = 0;
			}
			flow_qos_msg.flow_msg = node->flow_ctx.flow_msg;
			flow_qos_msg.qos = node->qos;
			flow_qos_msg.size = node->file_size.size;
			node->qos.rcv_bytes = 0;
			node->qos.bw_est = 0;
			node->flow_ctx.up_pkt_num = 0;
			ret = memcpy_s(*data + *offset, *size - *offset, &flow_qos_msg, sizeof(struct fi_qos_rpt_msg));
			if (ret) {
				fi_loge("memcpy_s failed]ret=%d,size=%u,offset=%u", ret, *size, *offset);
				kfree_skb(*pskb);
				return -1;
			}
			*offset += sizeof(struct fi_qos_rpt_msg);
		}
	}
	return 0;
}

void fi_ip_flow_qos_report(const struct fi_flow *flow,
	void (*flow_lock)(uint32_t),
	void (*flow_unlock)(uint32_t))
{
	int i;
	uint32_t offset = 0;
	uint32_t size = (FI_NETLINK_BUF_MAX_SIZE / sizeof(struct fi_qos_rpt_msg)) * sizeof(struct fi_qos_rpt_msg);
	char *data = NULL;
	struct sk_buff *pskb = NULL;
	if (!atomic_read(&flow->node_num)) {
		fi_logd("no data need to do qos report");
		return;
	}
	pskb = fi_get_netlink_skb(FI_QOS_MSG_RPT, size, &data);
	if (pskb == NULL) {
		fi_loge("failed to malloc memory for data");
		return;
	}

	for (i = 0; i < FI_FLOW_TABLE_SIZE; i++) {
		const struct fi_flow_head *head = flow->flow_table + i;
		if (list_empty(&(head->list))) {
			continue;
		}
		(*flow_lock)(i);
		if (fi_flow_qos_msg(head, &pskb, &data, &offset, &size) != 0) {
			(*flow_unlock)(i);
			return;
		}
		(*flow_unlock)(i);
	}
	if (offset == 0) {
		kfree_skb(pskb);
		return;
	}
	if (pskb != NULL) {
		if (offset < size) {
			struct nlmsghdr *nlh = (struct nlmsghdr *)(data - NLMSG_HDRLEN);
			nlh->nlmsg_len = offset + NLMSG_HDRLEN;
			fi_logd("need to adjust netlink len]size=%u,offset=%u", size, offset);
		}
		fi_enqueue_netlink_skb(pskb);
	}
	return;
}

void fi_flow_qos_report(void)
{
	fi_ip_flow_qos_report(&g_fi_ipv4_flow,
		fi_ipv4_flow_lock, fi_ipv4_flow_unlock);
#if IS_ENABLED(CONFIG_IPV6)
	fi_ip_flow_qos_report(&g_fi_ipv6_flow,
		fi_ipv6_flow_lock, fi_ipv6_flow_unlock);
#endif
}
