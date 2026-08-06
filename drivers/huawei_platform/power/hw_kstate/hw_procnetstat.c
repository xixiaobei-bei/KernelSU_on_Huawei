/*
 * hw_procnetstat.c
 *
 * This file use to statistics net info, including mobile and wifi bytes and packets
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <net/inet_sock.h>
#include <net/sock.h>
#include <securec.h>
#include <linux/crc32.h>
#include <linux/err.h>
#include <linux/fdtable.h>
#include <linux/hashtable.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/proc_fs.h>
#include <linux/profile.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/pid.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/ip.h>
#include <linux/timekeeping.h>
#ifdef CONFIG_HUAWEI_DUBAI
#include <chipset_common/dubai/dubai.h>
#endif
#include <huawei_platform/power/hw_kcollect.h>

#define MAX_PATH_LENGTH 255
#define KEY_LENGTH 80
#define STAT_HASHTABLE_SIZE 10
#define MAX_CTRL_CMD_LENGTH 512
#define MAX_SHARED_UID_NUM 80
#define MAX_THREAD_DECOMPOSE_UID_NUM 20
#define MAX_STATS_NUM 100
#define DEFAULT_SHARED_UID_NUMBER 4
#define AID_SYSTEM 1000 /* system server */
#define AID_RADIO_INDEX 2
#define AID_RADIO 1001 /* telephony subsystem, RIL */
#define AID_WIFI_INDEX 3
#define AID_WIFI 1010 /* wifi subsystem */

#define PROC_NET_INFO_NODE "proc_netstat"
#define PROCESS_STATS "stats"
#define THREAD_STATS "thread_stats"
#define CTRL "ctrl"
#define LO_IFNAME "lo"

#define DEFAULT_PREFIX "default_"
#define DEFAULT_SK DEFAULT_PREFIX"sk"
#define DEFAULT_SOCKET DEFAULT_PREFIX"socket"
#define DEFAULT_TIME_WAIT DEFAULT_PREFIX"timewait"
#define DEFAULT_ICMP DEFAULT_PREFIX"icmp"

#define LOG_DEFAULT_SK_SWITCH_ON 1
#define LOG_TAG "hw_procnetstat"
#define stat_logd(fmt, ...) printk(KERN_DEBUG "["LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__)
#define stat_logi(fmt, ...) printk(KERN_INFO "["LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__)
#define stat_loge(fmt, ...) printk(KERN_ERR "["LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__)

enum {
	CTRL_SET_SHARED_UID = 1,
	CTRL_SET_STATS_UID,
	CTRL_SET_STATS_PID,
	CTRL_SET_DEFAULT_SHARED_UID,
	CTRL_SET_THREAD_DECOMPOSE_UID,
	CTRL_SET_LOG_SWITCH,
};

static bool log_default_sk_switch;

/* max store 80 shared uids */
static int shared_uids[MAX_SHARED_UID_NUM] = {0};
static int shared_uid_num = 0;
static int default_shared_uid_num = DEFAULT_SHARED_UID_NUMBER;

/* max store 20 thread decompose uids */
static int thread_decompose_uids[MAX_THREAD_DECOMPOSE_UID_NUM] = {0};
static int thread_decompose_uid_num = 0;

/* max store 100 stats uids */
static int stats_uids[MAX_STATS_NUM] = {0};
static int stats_uid_num = 0;

/* max store 100 stats pids */
static int stats_pids[MAX_STATS_NUM] = {0};
static int stats_pid_num = 0;

/* stat_splock used to stat data */
static DEFINE_RWLOCK(stat_splock);

/* iface list, such as wlan0, rmnet0 */
static struct list_head iface_stat_list;

/* to store bytes and packets */
struct data_entry {
	uint64_t rx_bytes;
	uint64_t tx_bytes;
	uint64_t rx_packets;
	uint64_t tx_packets;
};

/* pid info */
struct pid_stat {
	/* hash list node */
	struct hlist_node node;
	/* key for hash list */
	pid_t pid;
	uid_t uid;
	struct data_entry data;
};

/* proc info */
struct proc_stat {
	/* hash list node */
	struct hlist_node node;
	/* key transfer from procName + uid */
	uint32_t hash_code;
	pid_t pid;
	uid_t uid;
	char comm[TASK_COMM_LEN];
	struct data_entry data;
};

/* iface info */
struct iface_stat {
	struct list_head list;
	DECLARE_HASHTABLE(proc_pid_stat_table, STAT_HASHTABLE_SIZE);
	DECLARE_HASHTABLE(proc_died_pid_stat_table, STAT_HASHTABLE_SIZE);
	DECLARE_HASHTABLE(thread_pid_stat_table, STAT_HASHTABLE_SIZE);
	DECLARE_HASHTABLE(thread_died_pid_stat_table, STAT_HASHTABLE_SIZE);
	char if_name[IFNAMSIZ + 1];
};

/* sk_buff info */
struct sk_buff_info {
	struct sock *sk;
	struct net_device *dev;
	uid_t uid;
	pid_t tgid;
	pid_t pid;
	bool is_default;
	bool get_sk;
	const char *comm;
	int protocol;
	int sport;
	int dport;
};

static struct iface_stat *find_or_create_iface_entry(const char *if_name)
{
	struct iface_stat *iface_entry = NULL;

	list_for_each_entry(iface_entry, &iface_stat_list, list) {
		if (!strncmp(if_name, iface_entry->if_name, IFNAMSIZ))
			return iface_entry;
	}

	iface_entry = kzalloc(sizeof(struct iface_stat), GFP_ATOMIC);
	if (!iface_entry) {
		stat_loge("no mem");
		return NULL;
	}

	strncpy(iface_entry->if_name, if_name, IFNAMSIZ);
	hash_init(iface_entry->proc_pid_stat_table);
	hash_init(iface_entry->proc_died_pid_stat_table);
	hash_init(iface_entry->thread_pid_stat_table);
	hash_init(iface_entry->thread_died_pid_stat_table);
	INIT_LIST_HEAD(&iface_entry->list);

	write_lock_bh(&stat_splock);
	list_add(&iface_entry->list, &iface_stat_list);
	write_unlock_bh(&stat_splock);

	return iface_entry;
}

static struct proc_stat *alloc_proc_entry(uint32_t hash_code, const char *comm, uid_t uid)
{
	struct proc_stat *proc_entry = NULL;

	proc_entry = kzalloc(sizeof(struct proc_stat), GFP_ATOMIC);
	if (!proc_entry) {
		stat_loge("no mem");
		return NULL;
	}

	if (memset_s(&proc_entry->data, sizeof(proc_entry->data), 0, sizeof(proc_entry->data)) != EOK) {
		stat_loge("failed to memset_s");
		kfree(proc_entry);
		return NULL;
	}

	strncpy(proc_entry->comm, comm, TASK_COMM_LEN - 1);
	proc_entry->comm[TASK_COMM_LEN - 1] = '\0';
	proc_entry->hash_code = hash_code;
	proc_entry->pid = -1;
	proc_entry->uid = uid;
	INIT_HLIST_NODE(&proc_entry->node);

	return proc_entry;
}

static int make_hash_code(const char *comm, uid_t uid, uint32_t *hash_code)
{
	char key[KEY_LENGTH] = {0};

	/* comm_uid is the key */
	if (snprintf_s(key, KEY_LENGTH, (KEY_LENGTH - 1), "%s_%d", comm, uid) < 0) {
		stat_loge("failed to make hash code");
		return 0;
	}

	/* use crc32 of comm for hash */
	*hash_code = crc32(0, key, strlen(key));

	return 1;
}

static struct proc_stat *find_or_create_proc_died_pid_entry(struct iface_stat *iface_entry,
	const char *comm, uid_t uid)
{
	struct proc_stat *proc_entry = NULL;
	uint32_t hash_code;

	if (!make_hash_code(comm, uid, &hash_code)) {
		stat_loge("failed to make hash code");
		return NULL;
	}

	hash_for_each_possible(iface_entry->proc_died_pid_stat_table, proc_entry, node, hash_code) {
		if (proc_entry->hash_code == hash_code)
			return proc_entry;
	}

	proc_entry = alloc_proc_entry(hash_code, comm, uid);
	if (!proc_entry) {
		stat_loge("failed to alloc proc entry");
		return NULL;
	}

	hash_add(iface_entry->proc_died_pid_stat_table, &proc_entry->node, hash_code);

	return proc_entry;
}

static struct proc_stat *find_or_create_thread_died_pid_entry(struct iface_stat *iface_entry,
	const char *comm, uid_t uid)
{
	struct proc_stat *proc_entry = NULL;
	uint32_t hash_code;

	if (!make_hash_code(comm, uid, &hash_code)) {
		stat_loge("failed to make hash code");
		return NULL;
	}

	hash_for_each_possible(iface_entry->thread_died_pid_stat_table, proc_entry, node, hash_code) {
		if (proc_entry->hash_code == hash_code)
			return proc_entry;
	}

	proc_entry = alloc_proc_entry(hash_code, comm, uid);
	if (!proc_entry) {
		stat_loge("failed to alloc proc entry");
		return NULL;
	}

	hash_add(iface_entry->thread_died_pid_stat_table, &proc_entry->node, hash_code);

	return proc_entry;
}

static struct pid_stat *alloc_pid_entry(pid_t pid, uid_t uid)
{
	struct pid_stat *pid_entry = NULL;

	pid_entry = kzalloc(sizeof(struct pid_stat), GFP_ATOMIC);
	if (!pid_entry) {
		stat_loge("no mem");
		return NULL;
	}

	if (memset_s(&pid_entry->data, sizeof(pid_entry->data), 0, sizeof(pid_entry->data)) != EOK) {
		stat_loge("failed to memset_s");
		kfree(pid_entry);
		return NULL;
	}

	pid_entry->pid = pid;
	pid_entry->uid = uid;
	INIT_HLIST_NODE(&pid_entry->node);

	return pid_entry;
}

static struct data_entry *get_proc_data_entry(struct iface_stat *iface_entry, pid_t pid, uid_t uid)
{
	struct pid_stat *pid_entry = NULL;
	struct proc_stat *proc_entry = NULL;
	ulong bkt;

	/* first check pid table */
	hash_for_each_possible(iface_entry->proc_pid_stat_table, pid_entry, node, pid) {
		if ((pid_entry->pid == pid) && (pid_entry->uid == uid))
			return &(pid_entry->data);
	}

	/* then check died pid table, for some just died pid. */
	hash_for_each(iface_entry->proc_died_pid_stat_table, bkt, proc_entry, node) {
		if ((proc_entry->pid == pid) && (proc_entry->uid == uid))
			return &(proc_entry->data);
	}

	/* create new pid entry */
	pid_entry = alloc_pid_entry(pid, uid);
	if (!pid_entry) {
		stat_loge("failed to alloc pid entry");
		return NULL;
	}

	hash_add(iface_entry->proc_pid_stat_table, &pid_entry->node, pid);

	return &(pid_entry->data);
}

static struct data_entry *get_thread_data_entry(struct iface_stat *iface_entry, pid_t pid, uid_t uid)
{
	struct pid_stat *pid_entry = NULL;
	struct proc_stat *proc_entry = NULL;
	ulong bkt;

	/* first check pid table */
	hash_for_each_possible(iface_entry->thread_pid_stat_table, pid_entry, node, pid) {
		if ((pid_entry->pid == pid) && (pid_entry->uid == uid))
			return &(pid_entry->data);
	}

	/* then check died pid table, for some just died pid. */
	hash_for_each(iface_entry->thread_died_pid_stat_table, bkt, proc_entry, node) {
		if ((proc_entry->pid == pid) && (proc_entry->uid == uid))
			return &(proc_entry->data);
	}

	/* create new pid entry */
	pid_entry = alloc_pid_entry(pid, uid);
	if (!pid_entry) {
		stat_loge("failed to alloc pid entry");
		return NULL;
	}

	hash_add(iface_entry->thread_pid_stat_table, &pid_entry->node, pid);

	return &(pid_entry->data);
}

static void update_data_entry(struct data_entry *data_entry, uint hooknum, uint len)
{
	switch (hooknum) {
	case NF_INET_LOCAL_IN:
		data_entry->rx_bytes += len;
		data_entry->rx_packets++;
		break;
	case NF_INET_LOCAL_OUT:
		data_entry->tx_bytes += len;
		data_entry->tx_packets++;
		break;
	default:
		break;
	}
}

static bool is_shared_uid(uid_t uid)
{
	int i;
	bool ret = false;

	read_lock_bh(&stat_splock);
	for (i = 0; (i < shared_uid_num) && (i < MAX_SHARED_UID_NUM); i++) {
		if (shared_uids[i] == uid) {
			ret = true;
			break;
		}
	}
	read_unlock_bh(&stat_splock);

	return ret;
}

static bool is_thread_decompose_uid(uid_t uid)
{
	int i;
	bool ret = false;

	read_lock_bh(&stat_splock);
	for (i = 0; (i < thread_decompose_uid_num) && (i < MAX_THREAD_DECOMPOSE_UID_NUM); i++) {
		if (thread_decompose_uids[i] == uid) {
			ret = true;
			break;
		}
	}
	read_unlock_bh(&stat_splock);

	return ret;
}

static int set_packet_stats(const char *curr_pointer, int *stats_arr, int *stats_num)
{
	int val;
	int index = 0;
	int ret;
	char *blank = NULL;

	while ((blank = strchr(curr_pointer, ' ')) != NULL) {
		ret = sscanf(curr_pointer, "%d", &val);
		if (ret == -1) { /* -1 means no num found */
			stat_loge("invald packet stats key");
			break;
		}

		if (index < MAX_STATS_NUM) {
			stats_arr[index] = val;
			index++;
		} else {
			stat_loge("packet stats key size more than limit, not store");
			break;
		}
		curr_pointer = ++blank;
	}
	*stats_num = index;

	return 0;
}

static int update_packet_stats(int *stats_arr, int *stats_num, int key)
{
	int size = *stats_num;
	int ret = -1;
	int i;

	for(i = 0 ; i < size; i++) {
		if (stats_arr[i] == key) {
			stats_arr[i] = stats_arr[size - 1];
			size--;
			break;
		}
	}
	if (*stats_num > size) {
		*stats_num = size;
		ret = 0;
	}

	return ret;
}

static void report_stats_packet(uid_t uid, pid_t pid)
{
	int ret;

	write_lock_bh(&stat_splock);
	ret = update_packet_stats(stats_uids, &stats_uid_num, uid);
	if (ret == 0) {
		hw_packet_cb(uid, -1);
	} else {
		ret = update_packet_stats(stats_pids, &stats_pid_num, pid);
		if (ret == 0)
			hw_packet_cb(uid, pid);
	}
	write_unlock_bh(&stat_splock);
}

static void add_default_data_to_proc_stat(const char *if_name, uint len, uint hooknum,
	uid_t uid, const char *comm)
{
	struct iface_stat *iface_entry = NULL;
	struct proc_stat *proc_entry = NULL;

	if (!is_shared_uid(uid))
		return;

	if (!if_name || !comm) {
		stat_loge("null dev_name or comm");
		return;
	}

	iface_entry = find_or_create_iface_entry(if_name);
	if (!iface_entry) {
		stat_loge("no mem");
		return;
	}

	write_lock_bh(&stat_splock);
	proc_entry = find_or_create_proc_died_pid_entry(iface_entry, comm, uid);
	if (!proc_entry) {
		stat_loge("failed to find proc died pid entry");
		write_unlock_bh(&stat_splock);
		return;
	}

	update_data_entry(&proc_entry->data, hooknum, len);
	write_unlock_bh(&stat_splock);
}

/* stat process data to hash table */
void stat_process_data(pid_t pid, uint len, const char *if_name, uint hooknum, int uid)
{
	struct iface_stat *iface_entry = NULL;
	struct data_entry *data_entry = NULL;

	if ((pid < 0) || !is_shared_uid(uid))
		return;

	if (!if_name) {
		stat_loge("null dev_name");
		return;
	}

	iface_entry = find_or_create_iface_entry(if_name);
	if (!iface_entry) {
		stat_loge("failed to find or create iface entry");
		return;
	}

	write_lock_bh(&stat_splock);
	data_entry = get_proc_data_entry(iface_entry, pid, uid);
	if (!data_entry) {
		write_unlock_bh(&stat_splock);
		stat_loge("failed to get proc entry");
		return;
	}

	update_data_entry(data_entry, hooknum, len);
	write_unlock_bh(&stat_splock);
}

/* stat thread data to hash table */
void stat_thread_data(pid_t pid, uint len, const char *if_name, uint hooknum, int uid)
{
	struct iface_stat *iface_entry = NULL;
	struct data_entry *data_entry = NULL;

	if ((pid < 0) || !is_thread_decompose_uid(uid))
		return;

	if (!if_name) {
		stat_loge("null dev_name");
		return;
	}

	iface_entry = find_or_create_iface_entry(if_name);
	if (!iface_entry) {
		stat_loge("failed to find or create iface entry");
		return;
	}

	write_lock_bh(&stat_splock);
	data_entry = get_thread_data_entry(iface_entry, pid, uid);
	if (!data_entry) {
		write_unlock_bh(&stat_splock);
		stat_loge("failed to get thread entry");
		return;
	}

	update_data_entry(data_entry, hooknum, len);
	write_unlock_bh(&stat_splock);
}

static void find_udp_sk(struct sk_buff *skb, struct sk_buff_info *skb_info, const int version)
{
	struct udphdr *uh = NULL;
	u64 now = 0;

	if (!skb || !skb_info || skb_info->protocol != IPPROTO_UDP)
		return;

	if (log_default_sk_switch)
		now = ktime_get_ns();

	uh = udp_hdr(skb);
	switch (version) {
	case NFPROTO_IPV4:
		skb_info->sk = udp4_lib_lookup_skb(skb, uh->source, uh->dest);
		break;
	case NFPROTO_IPV6:
		skb_info->sk = udp6_lib_lookup_skb(skb, uh->source, uh->dest);
		break;
	default:
		break;
	}

	skb_info->get_sk = (skb_info->sk != NULL);
	if (log_default_sk_switch)
		stat_logd("rcv %s udp sk %s cost:%ld", skb_info->sk ? "find" : "not find", version == NFPROTO_IPV4 ? "ipv4" : "ipv6",
			ktime_get_ns() - now);
	return;
}

/* log default sk info before stat */
void log_skb_info_before_stat(struct sk_buff *skb, struct sk_buff_info *skb_info,
	const struct nf_hook_state *state)
{
	if  (!skb || !skb_info || !state || !log_default_sk_switch)
		return;

	if (skb_info->protocol == IPPROTO_UDP) {
		skb_info->sport = htons(udp_hdr(skb)->source);
		skb_info->dport = htons(udp_hdr(skb)->dest);
	} else if (skb_info->protocol == IPPROTO_TCP) {
		skb_info->sport = htons(tcp_hdr(skb)->source);
		skb_info->dport = htons(tcp_hdr(skb)->dest);
	}

	if (!skb_info->sk) {
		stat_logd("iface:%s, no sk before stat %s %s, protocol:%d, sport:%d, dport:%d", skb_info->dev->name,
			state->pf == NFPROTO_IPV4 ? "ipv4" : "ipv6",
			state->hook == NF_INET_LOCAL_OUT ? "out" : "in",
			skb_info->protocol, skb_info->sport, skb_info->dport);
	}
}

/* log default sk info when stat */
void log_skb_info_when_stat(struct sk_buff_info *skb_info)
{
	if  (!skb_info || !log_default_sk_switch)
		return;

	if (skb_info->get_sk)
		stat_logd("iface:%s, get sk when stat uid:%d pid:%d protocol:%d", skb_info->dev->name,
			skb_info->uid, skb_info->tgid, skb_info->protocol);
	else if (skb_info->comm)
		stat_logd("iface:%s, no sk when stat protocol:%d comm:%s", skb_info->dev->name,
			skb_info->protocol, skb_info->comm);
}

/* hook to get data to account */
static unsigned int hook_datastat(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct sk_buff_info skb_info = {
		.sk = NULL,
		.dev = NULL,
		.uid = 0,
		.tgid = -1,
		.pid = -1,
		.is_default = false,
		.get_sk = false,
		.comm = NULL,
		.protocol = -1,
		.sport = -1,
		.dport = -1
	};

	if (!shared_uid_num && !stats_uid_num && !stats_pid_num && !thread_decompose_uid_num)
		goto exit;

	/* partial atomic, !in_atomic() */
	if (!skb || !skb->len || !state)
		goto exit;

	/* check run-time state */
	if (NF_INET_LOCAL_OUT == state->hook) {
		skb_info.dev = state->out;
	} else if (NF_INET_LOCAL_IN == state->hook) {
		skb_info.dev = state->in;
	}

	if (!skb_info.dev || strcmp(skb_info.dev->name, LO_IFNAME) == 0)
		goto exit;

	skb_info.sk = skb_to_full_sk(skb);
	skb_info.protocol = ip_hdr(skb)->protocol;
	log_skb_info_before_stat(skb, &skb_info, state);
	if (!skb_info.sk && NF_INET_LOCAL_IN == state->hook)
		find_udp_sk(skb, &skb_info, state->pf);

	/*
	* When in TCP_TIME_WAIT the sk is not a "struct sock" but
	* "struct inet_timewait_sock" which is missing fields.
	* So we ignore it.
	* Otherwise if you visit sk->sk_socket->file, it may cause reboot.
	*/

	if (!skb_info.sk) {
		skb_info.comm = (skb_info.protocol == IPPROTO_ICMP) ? DEFAULT_ICMP : DEFAULT_SK;
		goto account_default;
	} else if (TCP_TIME_WAIT == skb_info.sk->sk_state) {
		skb_info.comm = DEFAULT_TIME_WAIT;
		goto account_default;
	}

	read_lock_bh(&skb_info.sk->sk_callback_lock);
	skb_info.uid = from_kuid_munged(sock_net(skb_info.sk)->user_ns,
		sock_net_uid(sock_net(skb_info.sk), skb_info.sk));
	if (skb_info.sk->sk_socket) {
		skb_info.tgid = skb_info.sk->sk_socket->pid;
		skb_info.pid = skb_info.sk->sk_socket->tpid;
#ifdef CONFIG_HUAWEI_DUBAI
		dubai_send_app_wakeup(skb, state->hook, state->pf, skb_info.uid, skb_info.tgid);
#endif
#ifndef CONFIG_CHIP_HONGHU
	} else if (skb_info.sk->sk_pid > 0) {
		skb_info.tgid = skb_info.sk->sk_pid;
#endif
	} else {
		/* sk->sk_socket is null, should account to default */
		skb_info.is_default = true;
	}
	read_unlock_bh(&skb_info.sk->sk_callback_lock);

	if (skb_info.is_default) {
		skb_info.comm = DEFAULT_SOCKET;
		goto account_default;
	}

	if (state->hook == NF_INET_LOCAL_IN)
		report_stats_packet(skb_info.uid, skb_info.tgid);

	stat_process_data(skb_info.tgid, skb->len, skb_info.dev->name, state->hook, skb_info.uid);
	stat_thread_data(skb_info.pid, skb->len, skb_info.dev->name, state->hook, skb_info.uid);
	log_skb_info_when_stat(&skb_info);
	return NF_ACCEPT;

account_default:
	if (skb_info.comm) {
		log_skb_info_when_stat(&skb_info);
		add_default_data_to_proc_stat(skb_info.dev->name, skb->len, state->hook, skb_info.uid, skb_info.comm);
	}
exit:
	/* must return NF_ACCEPT, so skb can be processed by other flow */
	return NF_ACCEPT;
}

/* read process stats node will call this func. */
static int process_stats_show(struct seq_file *m, void *v)
{
	struct iface_stat *iface_entry = NULL;
	struct proc_stat *proc_entry = NULL;
	struct pid_stat *pid_entry = NULL;
	struct task_struct *task = NULL;
	char comm[TASK_COMM_LEN] = {0};
	ulong bkt;
	uid_t uid;

	/* not atomic, !in_atomic() */
	seq_printf(m, "if_name\tuid\tpid\tproc_name\trx_bytes\ttx_bytes\trx_packets\ttx_packets\n");
	read_lock_bh(&stat_splock);
	list_for_each_entry(iface_entry, &iface_stat_list, list) {
		hash_for_each(iface_entry->proc_pid_stat_table, bkt, pid_entry, node) {
			task = NULL;
			comm[0] = '\0';
			rcu_read_lock();
			task = find_task_by_vpid(pid_entry->pid);
			if (task) {
				uid = from_kuid_munged(current_user_ns(), task_uid(task));
				if (uid == pid_entry->uid)
					strncpy(comm, task->comm, TASK_COMM_LEN - 1);
				else
					stat_logi("Mismatch uid: %d, pid: %d, comm: %s", uid, pid_entry->pid, task->comm);
			}
			rcu_read_unlock();
			seq_printf(m, "%s\t%d\t%d\t%s\t%llu\t%llu\t%llu\t%llu\n",
				iface_entry->if_name, pid_entry->uid, pid_entry->pid, comm,
				pid_entry->data.rx_bytes, pid_entry->data.tx_bytes,
				pid_entry->data.rx_packets, pid_entry->data.tx_packets);
		}
		hash_for_each(iface_entry->proc_died_pid_stat_table, bkt, proc_entry, node) {
			seq_printf(m, "%s\t%d\t%d\t%s\t%llu\t%llu\t%llu\t%llu\n",
				iface_entry->if_name, proc_entry->uid, -1, proc_entry->comm,
				proc_entry->data.rx_bytes, proc_entry->data.tx_bytes,
				proc_entry->data.rx_packets, proc_entry->data.tx_packets);
		}
	}
	read_unlock_bh(&stat_splock);

	return 0;
}

/* read thread stats node will call this func. */
static int thread_stats_show(struct seq_file *m, void *v)
{
	struct iface_stat *iface_entry = NULL;
	struct proc_stat *proc_entry = NULL;
	struct pid_stat *pid_entry = NULL;
	struct task_struct *task = NULL;
	char comm[TASK_COMM_LEN] = {0};
	ulong bkt;
	uid_t uid;

	/* not atomic, !in_atomic() */
	seq_printf(m, "if_name\tuid\tpid\tproc_name\trx_bytes\ttx_bytes\trx_packets\ttx_packets\n");
	read_lock_bh(&stat_splock);
	list_for_each_entry(iface_entry, &iface_stat_list, list) {
		hash_for_each(iface_entry->thread_pid_stat_table, bkt, pid_entry, node) {
			task = NULL;
			comm[0] = '\0';
			rcu_read_lock();
			task = find_task_by_vpid(pid_entry->pid);
			if (task) {
				uid = from_kuid_munged(current_user_ns(), task_uid(task));
				if (uid == pid_entry->uid)
					strncpy(comm, task->comm, TASK_COMM_LEN - 1);
				else
					stat_logi("Mismatch uid: %d, pid: %d, comm: %s", uid, pid_entry->pid, task->comm);
			} else {
				stat_loge("Task not found, uid: %d, pid: %d", uid, pid_entry->pid);
			}
			rcu_read_unlock();
			seq_printf(m, "%s\t%d\t%d\t%s\t%llu\t%llu\t%llu\t%llu\n",
				iface_entry->if_name, pid_entry->uid, pid_entry->pid, comm,
				pid_entry->data.rx_bytes, pid_entry->data.tx_bytes,
				pid_entry->data.rx_packets, pid_entry->data.tx_packets);
		}
		hash_for_each(iface_entry->thread_died_pid_stat_table, bkt, proc_entry, node) {
			seq_printf(m, "%s\t%d\t%d\t%s\t%llu\t%llu\t%llu\t%llu\n",
				iface_entry->if_name, proc_entry->uid, -1, proc_entry->comm,
				proc_entry->data.rx_bytes, proc_entry->data.tx_bytes,
				proc_entry->data.rx_packets, proc_entry->data.tx_packets);
		}
	}
	read_unlock_bh(&stat_splock);

	return 0;
}

static void calc_pid_to_proc(struct pid_stat *pid_entry, struct proc_stat *proc_entry)
{
	proc_entry->data.rx_bytes += pid_entry->data.rx_bytes;
	proc_entry->data.tx_bytes += pid_entry->data.tx_bytes;
	proc_entry->data.rx_packets += pid_entry->data.rx_packets;
	proc_entry->data.tx_packets += pid_entry->data.tx_packets;
	/* mark this pid, becuase some data may on the road, we will add them to proc entry. */
	proc_entry->pid = pid_entry->pid;
}

static void move_dead_pid_to_proc_stat(struct iface_stat *iface_entry, struct pid_stat *pid_entry, const char *comm)
{
	struct proc_stat *proc_entry = NULL;

	proc_entry = find_or_create_proc_died_pid_entry(iface_entry, comm, pid_entry->uid);
	if (!proc_entry) {
		stat_loge("failed to find or create proc died entry");
		return;
	}

	calc_pid_to_proc(pid_entry, proc_entry);
}

static void move_dead_pid_to_thread_stat(struct iface_stat *iface_entry, struct pid_stat *pid_entry, const char *comm)
{
	struct proc_stat *proc_entry = NULL;

	proc_entry = find_or_create_thread_died_pid_entry(iface_entry, comm, pid_entry->uid);
	if (!proc_entry) {
		stat_loge("failed to find or create thread died entry");
		return;
	}

	calc_pid_to_proc(pid_entry, proc_entry);
}

static void handler_proc_pid_died(struct task_struct *task, struct iface_stat *iface_entry)
{
	struct pid_stat *pid_entry = NULL;
	struct hlist_node *tmp = NULL;
	unsigned long bkt;

	if (task == task->group_leader) {
		hash_for_each_safe(iface_entry->proc_pid_stat_table, bkt, tmp, pid_entry, node) {
			if (pid_entry->pid == task->pid) {
				move_dead_pid_to_proc_stat(iface_entry, pid_entry, task->comm);
				hash_del(&pid_entry->node);
				kfree(pid_entry);
				break;
			}
		}
	}
}

static void handler_thread_pid_died(struct task_struct *task, struct iface_stat *iface_entry)
{
	struct pid_stat *pid_entry = NULL;
	struct hlist_node *tmp = NULL;
	unsigned long bkt;

	hash_for_each_safe(iface_entry->thread_pid_stat_table, bkt, tmp, pid_entry, node) {
		if (pid_entry->pid == task->pid) {
			move_dead_pid_to_thread_stat(iface_entry, pid_entry, task->comm);
			hash_del(&pid_entry->node);
			kfree(pid_entry);
			break;
		}
	}
}

/* pid exit callback func */
static int process_exit_callback(struct notifier_block *self, ulong cmd, void *v)
{
	struct task_struct *task = v;
	struct iface_stat *iface_entry = NULL;
	uid_t uid;
	bool shared_uid = false;
	bool thread_decompose_uid = false;

	/* ignore child threads, only care about main process. */
	if (!task)
		return NOTIFY_OK;

	/* get the uid of this packet. */
	uid = from_kuid_munged(current_user_ns(), task_uid(task));
	shared_uid = is_shared_uid(uid);
	thread_decompose_uid = is_thread_decompose_uid(uid);
	if (!shared_uid && !thread_decompose_uid)
		return NOTIFY_OK;

	/*
	* dead process's task_struct will be invalid.
	* need to update pid_stat to proc_stat if its task points to this task.
	*/
	write_lock_bh(&stat_splock);
	list_for_each_entry(iface_entry, &iface_stat_list, list) {
		if (shared_uid)
			handler_proc_pid_died(task, iface_entry);
		if (thread_decompose_uid)
			handler_thread_pid_died(task, iface_entry);
	}
	write_unlock_bh(&stat_splock);

	return NOTIFY_OK;
}

static void init_default_shared_uids(void) {
	memset_s(shared_uids, sizeof(int) * MAX_SHARED_UID_NUM, 0, sizeof(int) * MAX_SHARED_UID_NUM);
	shared_uids[0] = 0;
	shared_uids[1] = AID_SYSTEM;
	shared_uids[AID_RADIO_INDEX] = AID_RADIO;
	shared_uids[AID_WIFI_INDEX] = AID_WIFI;
	shared_uid_num = default_shared_uid_num;
}

static void init_thread_decompose_uids(void) {
	memset_s(thread_decompose_uids, sizeof(int) * MAX_THREAD_DECOMPOSE_UID_NUM,
		0, sizeof(int) * MAX_THREAD_DECOMPOSE_UID_NUM);
	thread_decompose_uid_num = 0;
}

static bool check_default_shared_uid(int uid)
{
	int i;

	for (i = 0; (i < default_shared_uid_num) && (i < MAX_SHARED_UID_NUM); i++) {
		if (shared_uids[i] == uid)
			return true;
	}

	return false;
}

static bool check_thread_decompose_uid(int uid)
{
	int i;

	for (i = 0; (i < thread_decompose_uid_num) && (i < MAX_THREAD_DECOMPOSE_UID_NUM); i++) {
		if (thread_decompose_uids[i] == uid)
			return true;
	}

	return false;
}

static void set_shared_uids(const char *curr_pointer)
{
	char *blank = NULL;
	int ret, uid;

	shared_uid_num = default_shared_uid_num;

	while ((blank = strchr(curr_pointer, ' ')) != NULL) {
		if (shared_uid_num >= MAX_SHARED_UID_NUM)
			break;

		ret = sscanf(curr_pointer, "%d", &uid);
		if (ret == -1) {
			stat_loge("invald uid, buffer: %s", curr_pointer);
			init_default_shared_uids();
			return;
		}

		if (!check_default_shared_uid(uid)) {
			shared_uids[shared_uid_num] = uid;
			shared_uid_num++;
		}

		curr_pointer = ++blank;
	}
}

static void set_default_shared_uids(const char *curr_pointer)
{
	char *blank = NULL;
	int ret, uid, i;
	int old_shared_uids[MAX_SHARED_UID_NUM] = {0};
	int old_default_uid_num = default_shared_uid_num;
	int old_shared_uid_num = shared_uid_num;

	memcpy(old_shared_uids, shared_uids, sizeof(int) * MAX_SHARED_UID_NUM);

	while ((blank = strchr(curr_pointer, ' ')) != NULL) {
		if (default_shared_uid_num >= MAX_SHARED_UID_NUM)
			break;

		ret = sscanf(curr_pointer, "%d", &uid);
		if (ret == -1) {
			stat_loge("invald uid, buffer: %s", curr_pointer);
			memcpy(shared_uids, old_shared_uids, sizeof(int) * MAX_SHARED_UID_NUM);
			default_shared_uid_num = old_default_uid_num;
			return;
		}

		if (!check_default_shared_uid(uid)) {
			shared_uids[default_shared_uid_num] = uid;
			default_shared_uid_num++;
		}

		curr_pointer = ++blank;
	}
	shared_uid_num = default_shared_uid_num;
	for (i = old_default_uid_num; i < old_shared_uid_num; i++) {
		if (!check_default_shared_uid(old_shared_uids[i]) && (shared_uid_num < MAX_SHARED_UID_NUM)) {
			shared_uids[shared_uid_num] = old_shared_uids[i];
			shared_uid_num++;
		}
	}
}

static void set_thread_decompose_uids(const char *curr_pointer)
{
	char *blank = NULL;
	int ret, uid;

	thread_decompose_uid_num = 0;

	while ((blank = strchr(curr_pointer, ' ')) != NULL) {
		if (thread_decompose_uid_num >= MAX_THREAD_DECOMPOSE_UID_NUM)
			break;

		ret = sscanf(curr_pointer, "%d", &uid);
		if (ret == -1) {
			stat_loge("invald uid, buffer: %s", curr_pointer);
			init_thread_decompose_uids();
			break;
		}

		if (!check_thread_decompose_uid(uid)) {
			thread_decompose_uids[thread_decompose_uid_num] = uid;
			thread_decompose_uid_num++;
		}

		curr_pointer = ++blank;
	}
}

static void set_log_switch(const char *curr_pointer)
{
	int ret;
	int debug_switch = 0;

	ret = sscanf(curr_pointer, "%d", &debug_switch);
	if (ret == -1) {
		stat_loge("invalid log_switch, buffer:%s", curr_pointer);
		return;
	}

	log_default_sk_switch = (debug_switch == LOG_DEFAULT_SK_SWITCH_ON);
	stat_logi("change log_default_sk_switch:%d", log_default_sk_switch);
}

/* write ctrl node will call this func. */
static ssize_t ctrl_stat_write(struct file *file, const char __user *buffer, size_t count, loff_t *offp)
{
	char input_buf[MAX_CTRL_CMD_LENGTH] = {0};
	int cmd, ret;
	char *curr_pointer = NULL;
	char *blank = NULL;

	if ((count <= 0) || (count >= MAX_CTRL_CMD_LENGTH)) {
		stat_loge("invalid count %ld", count);
		return -EINVAL;
	}

	if (copy_from_user(input_buf, buffer, count)) {
		stat_loge("copy_from_user error");
		return -EFAULT;
	}

	curr_pointer = input_buf;
	blank = strchr(curr_pointer, ' ');
	if (!blank) {
		stat_loge("invald buffer: %s", curr_pointer);
		return -EINVAL;
	}

	ret = sscanf(curr_pointer, "%d", &cmd);
	if (ret == -1) { /* -1 means no num found */
		stat_loge("invald cmd, buffer: %s", curr_pointer);
		return -EINVAL;
	}

	curr_pointer = ++blank;

	write_lock_bh(&stat_splock);
	switch (cmd) {
	case CTRL_SET_SHARED_UID:
		set_shared_uids(curr_pointer);
		break;
	case CTRL_SET_STATS_UID:
		set_packet_stats(curr_pointer, stats_uids, &stats_uid_num);
		break;
	case CTRL_SET_STATS_PID:
		set_packet_stats(curr_pointer, stats_pids, &stats_pid_num);
		break;
	case CTRL_SET_DEFAULT_SHARED_UID:
		set_default_shared_uids(curr_pointer);
		break;
	case CTRL_SET_THREAD_DECOMPOSE_UID:
		set_thread_decompose_uids(curr_pointer);
		break;
	case CTRL_SET_LOG_SWITCH:
		set_log_switch(curr_pointer);
		break;
	default:
		stat_loge("unexpected cmd: %d", cmd);
		break;
	}
	write_unlock_bh(&stat_splock);

	return count;
}

static const struct file_operations g_ctrl_stat_fops = {
	.write = ctrl_stat_write,
};

static int process_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, process_stats_show, PDE_DATA(inode));
}

static int thread_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, thread_stats_show, PDE_DATA(inode));
}

static struct notifier_block netstat_process_nb = {
	.notifier_call = process_exit_callback,
};

static const struct file_operations g_process_stats_fops = {
	.open = process_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations g_thread_stats_fops = {
	.open = thread_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

// priority should be bigger than NF_IP_PRI_FILTER, some iface name may change
static struct nf_hook_ops netstat_nf_hooks[] = {
	{
		.hook = hook_datastat,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook_datastat,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP6_PRI_FILTER + 1,
	},
	{
		.hook = hook_datastat,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook_datastat,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP6_PRI_FILTER + 1,
	}
};

/* module init func. */
static int __init procnetstat_init(void)
{
	struct proc_dir_entry *parent = NULL;
	struct proc_dir_entry *file = NULL;
	int err;

	init_default_shared_uids();
	init_thread_decompose_uids();

	INIT_LIST_HEAD(&iface_stat_list);

	/* create root dir */
	parent = proc_mkdir(PROC_NET_INFO_NODE, init_net.proc_net);
	if (!parent) {
		stat_loge("mk_proc_dir_err");
		goto mk_proc_dir_err;
	}

	/* create process stats node */
	file = proc_create_data(PROCESS_STATS, S_IRUGO, parent, &g_process_stats_fops, NULL);
	if (!file) {
		stat_loge("mk_proc_stats_err");
		goto mk_proc_stats_err;
	}

	/* create thread stats node */
	file = proc_create_data(THREAD_STATS, S_IRUGO, parent, &g_thread_stats_fops, NULL);
	if (!file) {
		stat_loge("mk_thread_stats_err");
		goto mk_thread_stats_err;
	}

	/* create ctrl node */
	file = proc_create_data(CTRL, S_IRUGO | S_IWUGO, parent, &g_ctrl_stat_fops, NULL);
	if (!file) {
		stat_loge("mk_proc_ctrl_err");
		goto mk_proc_ctrl_err;
	}

	/* register process exit notifier function */
	profile_event_register(PROFILE_TASK_EXIT, &netstat_process_nb);

	/* register data hook function */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	err = nf_register_hooks(netstat_nf_hooks, ARRAY_SIZE(netstat_nf_hooks));
#else
	err = nf_register_net_hooks(&init_net, netstat_nf_hooks, ARRAY_SIZE(netstat_nf_hooks));
#endif
	if (err < 0) {
		stat_loge("nf_register_hooks_err");
		goto nf_register_hooks_err;
	}
	return 0;

nf_register_hooks_err:
	profile_event_unregister(PROFILE_TASK_EXIT, &netstat_process_nb);
	remove_proc_entry(CTRL, parent);

mk_proc_ctrl_err:
	remove_proc_entry(THREAD_STATS, parent);

mk_thread_stats_err:
	remove_proc_entry(PROCESS_STATS, parent);

mk_proc_stats_err:
	remove_proc_entry(PROC_NET_INFO_NODE, init_net.proc_net);

mk_proc_dir_err:
	return -1;
}

module_init(procnetstat_init);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ProcNetStat: network flow statistics via process name and pid.");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
