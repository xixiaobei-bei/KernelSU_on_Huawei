/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Count the steady speed and report it.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2020-03-16
 */

#include "hw_steady_speed.h"

#include <asm/atomic.h>
#include <linux/spinlock.h>
#include <linux/net.h>
#include <linux/list.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <log/hw_log.h>
#include <net/tcp.h>
#include <net/sock.h>

#include "hw_booster_common.h"

#undef HWLOG_TAG
#define HWLOG_TAG steady_speed
HWLOG_REGIST();

#define MAX_SOCK_HASH_SIZE 100
#define MAX_LISTENING_UID_NUM 10
#define RPT_STAT_EVT_LEN 16
#define TCP_HDR_DOFF_QUAD 4
#define TCP_HDR_IHL_QUAD 4

/* statistics threshold of uplink and downlink(UL: 128B, DL 1.4MB) */
#define UL_DATA_LENGTH_THRESHOLD 128
#define DL_DATA_LENGTH_THRESHOLD 1468006

/* 1.4MB / 45 = 256Kbps */
#define SOCK_LISTEN_DURATION (45 * HZ)

struct listening_uids {
	int *uids;
	int nums;
	rwlock_t lock;
};

struct stats_node {
	struct sock *sk;
	unsigned long stamp;
	atomic_t stats_out;
	atomic_t stats_in;
	struct stats_node *next;
};

struct hash_info {
	struct stats_node *chain;
	rwlock_t lock;
};

static struct listening_uids uid_list;
static notify_event *notifier = NULL;

/* Hash table for sock statistics of cared uid. */
static struct hash_info table[MAX_SOCK_HASH_SIZE];
static struct timer_list clean_timer;
static bool active_flag = false;

static inline u32 hash(u32 sport, u32 dport)
{
	/* port length is 16 bits */
	u32 temp = (sport << 16) | dport;
	return (temp % MAX_SOCK_HASH_SIZE);
}

/* upgrade the hash lock from read to write */
static inline void upgrade_hash_lock(u32 index)
{
	read_unlock_bh(&table[index].lock);
	write_lock_bh(&table[index].lock);
}

/* downgrade the hash lock from write to read */
static inline void downgrade_hash_lock(u32 index)
{
	write_unlock_bh(&table[index].lock);
	read_lock_bh(&table[index].lock);
}

static inline bool is_client_syn_request(struct tcphdr *th)
{
	return (th->syn && !th->ack);
}

static inline bool is_collision_sock(struct tcphdr *th, struct stats_node *node)
{
	return is_client_syn_request(th) &&
		atomic_read(&node->stats_in) > 0 &&
		atomic_read(&node->stats_out) > 0;
}

static inline void reset_stats_node(struct stats_node *node)
{
	node->stamp = jiffies;
	atomic_set(&node->stats_in, 0);
	atomic_set(&node->stats_out, 0);
}

static struct stats_node *create_stats_node(struct sock *sk)
{
	struct stats_node *node = NULL;

	node = kmalloc(sizeof(struct stats_node), GFP_ATOMIC);
	if (node != NULL) {
		reset_stats_node(node);
		node->sk = sk;
		node->next = NULL;
	}
	return node;
}

static inline void free_stats_node(struct stats_node *node)
{
	if (node)
		kfree(node);
}

/* The caller should already hold the read lock */
static struct stats_node *find_stats_node(u32 index, const struct sock *sk)
{
	struct stats_node *node = NULL;

	for (node = table[index].chain; node; node = node->next)
		if (node->sk == sk)
			break;
	return node;
}

static inline void insert_stats_node(u32 index, struct stats_node *node)
{
	node->next = table[index].chain;
	table[index].chain = node;
}

static void delete_stats_node(u32 index, struct stats_node *node, bool del_after)
{
	struct stats_node **prev = &table[index].chain;
	struct stats_node *p = table[index].chain;
	struct stats_node *q = NULL;

	while (p && p->sk != node->sk) {
		prev = &p->next;
		p = p->next;
	}
	if (p == NULL)
		return;

	if (del_after == true) {
		*prev = NULL;
		while (p) {
			q = p->next;
			free_stats_node(p);
			p = q;
		}
	} else {
		*prev = p->next;
		free_stats_node(p);
	}
}

static void update_uid_list(const char *msg, int len)
{
	int uid;
	int num;
	const char *p = msg;
	int i;

	if (len < sizeof(int)) //lint !e574
		return;
	num = *(int *)p;
	if (num > MAX_LISTENING_UID_NUM)
		return;
	len -= sizeof(int);
	p += sizeof(int);
	if (len < (sizeof(int) * num)) //lint !e574
		return;
	write_lock_bh(&uid_list.lock);
	/* delete previous uids firstly */
	if (uid_list.uids)
		kfree(uid_list.uids);
	uid_list.uids = NULL;
	uid_list.nums = 0;
	if (num > 0)
		uid_list.uids = kmalloc(sizeof(int) * num, GFP_KERNEL);
	if (uid_list.uids == NULL) {
		write_unlock_bh(&uid_list.lock);
		return;
	}
	for (i = 0; i < num; ++i) {
		uid = *(int *)p;
		if (uid < 0)
			break;
		uid_list.uids[i] = uid;
		p += sizeof(int);
	}
	uid_list.nums = i;
	write_unlock_bh(&uid_list.lock);
}

static void do_commands(struct req_msg_head *msg)
{
	if (!msg || msg->len <= sizeof(struct req_msg_head))
		return;

	if (msg->type != UPDATE_UID_LIST)
		return;
	update_uid_list((char *)msg + sizeof(struct req_msg_head),
		msg->len - sizeof(struct req_msg_head));
}

static bool is_listening_uid(struct sock *sk)
{
	uid_t uid;
	int i;
	bool ret = false;

	uid = hw_get_sock_uid(sk);
	read_lock(&uid_list.lock);
	for (i = 0; i < uid_list.nums; ++i)
		if (uid == uid_list.uids[i])
			break;
	if (i < uid_list.nums)
		ret = true;
	read_unlock(&uid_list.lock);
	return ret;
}

static void report_stats(struct stats_node *node)
{
	// 2B type + 2B len + 4B uid + 4B DL data len + 4B duration)
	char event[RPT_STAT_EVT_LEN];
	char *p = event;
	u32 duration;
	uid_t uid = hw_get_sock_uid(node->sk);

	if (!notifier)
		return;

	// type
	assign_short(p, STEADY_SPEED_STATS_RPT);
	skip_byte(p, sizeof(s16));
	// len(2B type + 2B len + 4B uid + 4B DL data len + 4B duration)
	assign_short(p, RPT_STAT_EVT_LEN);
	skip_byte(p, sizeof(s16));
	// uid
	assign_int(p, uid);
	skip_byte(p, sizeof(int));
	// DL data len
	assign_int(p, atomic_read(&node->stats_in));
	skip_byte(p, sizeof(int));
	// duration
	duration = jiffies_to_msecs(jiffies - node->stamp);
	assign_uint(p, duration);
	skip_byte(p, sizeof(u32));

	notifier((struct res_msg_head *)event);
}

static void try_report_stats(u32 index, struct stats_node *node)
{
	struct sock *sk = node->sk;

	if (atomic_read(&node->stats_out) < UL_DATA_LENGTH_THRESHOLD)
		return;
	if (atomic_read(&node->stats_in) < DL_DATA_LENGTH_THRESHOLD)
		return;
	report_stats(node);

	/* delete the stats node from hash table after report */
	upgrade_hash_lock(index);
	node = find_stats_node(index, sk);
	if (node != NULL)
		delete_stats_node(index, node, false);
	downgrade_hash_lock(index);
}

static void tcp_rcv_stats(struct sk_buff *skb, int tcp_len)
{
	struct tcphdr *th = tcp_hdr(skb);
	int data_len;
	struct stats_node *node = NULL;
	u32 index;

	if (th == NULL)
		return;
	/* should exchange the source and dest port for receive */
	index = hash(th->dest, th->source);
	data_len = tcp_len - th->doff * TCP_HDR_DOFF_QUAD;
	if (data_len < 0)
		return;
	read_lock_bh(&table[index].lock);
	node = find_stats_node(index, skb->sk);
	if (node == NULL) {
		read_unlock_bh(&table[index].lock);
		return;
	}

	/* for downlink, we can check condition here */
	if (atomic_read(&node->stats_in) < DL_DATA_LENGTH_THRESHOLD) {
		atomic_add(data_len, &node->stats_in);
		try_report_stats(index, node);
	}
	read_unlock_bh(&table[index].lock);
}

static void tcp_send_stats(struct sk_buff *skb, int tcp_len)
{
	struct tcphdr *th = tcp_hdr(skb);
	int data_len;
	struct stats_node *node = NULL;
	u32 index;
	bool new_flag = false;

	if (th == NULL)
		return;
	data_len = tcp_len - th->doff * TCP_HDR_DOFF_QUAD;
	if (data_len < 0)
		return;

	/* find or create a new stats node */
	index = hash(th->source, th->dest);
	read_lock_bh(&table[index].lock);
	node = find_stats_node(index, skb->sk);
	if (node == NULL) {
		if (is_client_syn_request(th)) {
			node = create_stats_node(skb->sk);
			new_flag = true;
		}
	} else {
		if (is_collision_sock(th, node))
			reset_stats_node(node);
	}
	if (node == NULL) {
		read_unlock_bh(&table[index].lock);
		return;
	}

	/* insert the new node to hash table */
	if (new_flag) {
		upgrade_hash_lock(index);
		/* Maybe another cpu already add this sock to hash table */
		if (likely(find_stats_node(index, skb->sk) == NULL)) {
			insert_stats_node(index, node);
			if (!active_flag) {
				mod_timer(&clean_timer, jiffies + SOCK_LISTEN_DURATION);
				active_flag = true;
			}
		} else {
			free_stats_node(node);
		}
		downgrade_hash_lock(index);
		/* Maybe another cpu already delete this sock from hash table */
		node = find_stats_node(index, skb->sk);
		if (node == NULL) {
			read_unlock_bh(&table[index].lock);
			return;
		}
	}

	/* stats the out */
	if (data_len > 0 &&
		atomic_read(&node->stats_out) < UL_DATA_LENGTH_THRESHOLD) {
		atomic_add(data_len, &node->stats_out);
		try_report_stats(index, node);
	}
	read_unlock_bh(&table[index].lock);
}

static unsigned int tcp_v4_hook(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct iphdr *iph = NULL;

	if (skb == NULL || skb->sk == NULL)
		return NF_ACCEPT;
	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;
	if (!is_listening_uid(skb->sk))
		return NF_ACCEPT;
	if (state->hook == NF_INET_LOCAL_IN) {
		if (!is_ds_rnic(state->in))
			return NF_ACCEPT;
		tcp_rcv_stats(skb, ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD);
	}
	if (state->hook == NF_INET_LOCAL_OUT) {
		if (!is_ds_rnic(state->out))
			return NF_ACCEPT;
		tcp_send_stats(skb, ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD);
	}
	return NF_ACCEPT;
}

static unsigned int tcp_v6_hook(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct ipv6hdr *ip6h = NULL;

	if (skb == NULL || skb->sk == NULL)
		return NF_ACCEPT;
	ip6h = ipv6_hdr(skb);
	if (ip6h == NULL)
		return NF_ACCEPT;
	if (ip6h->nexthdr != IPPROTO_TCP)
		return NF_ACCEPT;
	if (!is_listening_uid(skb->sk))
		return NF_ACCEPT;
	if (state->hook == NF_INET_LOCAL_IN) {
		if (!is_ds_rnic(state->in))
			return NF_ACCEPT;
		tcp_rcv_stats(skb, ntohs(ip6h->payload_len));
	}
	if (state->hook == NF_INET_LOCAL_OUT) {
		if (!is_ds_rnic(state->out))
			return NF_ACCEPT;
		tcp_send_stats(skb, ntohs(ip6h->payload_len));
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = tcp_v4_hook,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = tcp_v4_hook,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = tcp_v6_hook,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = tcp_v6_hook,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
};

static void try_clean_table(unsigned long data)
{
	bool restart = false;
	int i;
	struct stats_node *node = NULL;

	for (i = 0; i < MAX_SOCK_HASH_SIZE; ++i) {
		write_lock_bh(&table[i].lock);
		for (node = table[i].chain; node; node = node->next)
			if (time_after(jiffies, node->stamp + SOCK_LISTEN_DURATION))
				break;
		if (node != NULL)
			delete_stats_node(i, node, true);
		if (table[i].chain != NULL)
			restart = true;
		write_unlock_bh(&table[i].lock);
	}

	if (restart)
		mod_timer(&clean_timer, jiffies + SOCK_LISTEN_DURATION);
	active_flag = restart;
	hwlog_info("%s: restart: %d\n", __func__, active_flag);
}

msg_process* __init hw_steady_speed_init(notify_event *notify)
{
	int ret, i;

	if (notify == NULL) {
		hwlog_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}
	rwlock_init(&uid_list.lock);
	notifier = notify;

	for (i = 0; i < MAX_SOCK_HASH_SIZE; ++i) {
		table[i].chain = NULL;
		rwlock_init(&table[i].lock);
	}
	init_timer(&clean_timer);
	clean_timer.data = 0;
	clean_timer.function = try_clean_table;

	ret = nf_register_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
	if (ret) {
		hwlog_err("%s: nf_register_net_hooks ret=%d\n", __func__, ret);
		return NULL;
	}

	hwlog_info("%s: steady speed init success\n", __func__);
	return do_commands;
}
