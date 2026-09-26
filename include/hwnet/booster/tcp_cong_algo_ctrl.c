

#include "tcp_cong_algo_ctrl.h"

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/list.h>
#include <linux/net.h>
#include <net/sock.h>
#include <net/tcp.h>

static struct apk_config g_apks;
static struct curr_uid_info g_uid_info;

static void collect_cfg_info(struct req_msg_head *msg)
{
	int num;
	int len;
	int i;
	char *p = NULL;

	len = msg->len - sizeof(struct req_msg_head);
	p = (char *)msg + sizeof(struct req_msg_head);
	if (len <= sizeof(int))
		return;

	num = *(int *)p;
	len -= sizeof(int);
	p += sizeof(int);
	if (len < (sizeof(int) * num))
		return;

	spin_lock_bh(&g_apks.lock);
	g_apks.nums = num;
	for (i = 0; i < num; ++i) {
		g_apks.config[i].uid = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].network_type = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].uid_state = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].restore_threshold = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].package_loss_rate = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].total_throughput = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].average_rate = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].retransmit_times = *(int *)p;
		p += sizeof(int);
		g_apks.config[i].cong_algo = *(int *)p;
		p += sizeof(int);
	}
	spin_unlock_bh(&g_apks.lock);
}

inline u32 get_socks_uid(struct sock *sk)
{
	return (u32)sk->sk_uid.val;
}

static void tcp_set_all_congestion_control(const char *name)
{
	int bucket = 0;
	int uid;

	for (; bucket <= tcp_hashinfo.ehash_mask; ++bucket) {
		struct sock *sk = NULL;
		struct hlist_nulls_node *node = NULL;
		spinlock_t *lock = inet_ehash_lockp(&tcp_hashinfo, bucket);

		/* Lockless fast path for the common case of empty buckets */
		if (hlist_nulls_empty(&tcp_hashinfo.ehash[bucket].chain))
			continue;

		spin_lock_bh(lock);
		sk_nulls_for_each(sk, node, &tcp_hashinfo.ehash[bucket].chain) {
			if (!sk_fullsock(sk))
				continue;

			uid = get_socks_uid(sk);
			if (uid != g_uid_info.cur_uid)
				continue;

			if (sk->sk_family == AF_INET || sk->sk_family == AF_INET6) {
				bh_lock_sock(sk);
				tcp_set_congestion_control(sk, name, false, true);
				bh_unlock_sock(sk);
			}
		}
		spin_unlock_bh(lock);
	}
}

static void enable_cong_algo_change(void)
{
	int i;
	int algo = INVALID_VAL;
	const char *cubic = "cubic";
	const char *bbr = "bbr";
	const char *mibo = "mibo";

	spin_lock_bh(&g_apks.lock);
	for (i = 0; i < g_apks.nums; i++) {
		if (g_apks.config[i].uid != g_uid_info.cur_uid)
			continue;
		if (g_apks.config[i].uid_state != g_uid_info.cur_uid_state)
			continue;
		if (g_uid_info.cur_ds_state == DATA_SERVICE_ON) {
			algo = g_apks.config[i].cong_algo;
			break;
		}
	}
	spin_unlock_bh(&g_apks.lock);

	if (algo == INVALID_VAL)
		return;

	switch (algo) {
	case CUBIC:
		tcp_set_all_congestion_control(cubic);
		break;
	case BBR:
		tcp_set_all_congestion_control(bbr);
		break;
	case MIBO:
		tcp_set_all_congestion_control(mibo);
		break;
	default:
		pr_info("msg error\n");
		break;
	}
}

static void collect_uid_info(struct req_msg_head *msg)
{
	char *p = NULL;

	p = (char *)msg + sizeof(struct req_msg_head);

	spin_lock_bh(&g_apks.lock);
	g_uid_info.cur_uid = *(int *)p;
	p += sizeof(int);
	g_uid_info.cur_uid_state = *(int *)p;
	spin_unlock_bh(&g_apks.lock);
	enable_cong_algo_change();
}

static void collect_ds_state(struct req_msg_head *msg)
{
	char *p = NULL;

	p = (char *)msg + sizeof(struct req_msg_head);

	spin_lock_bh(&g_apks.lock);
	g_uid_info.cur_ds_state = *(int *)p;
	spin_unlock_bh(&g_apks.lock);
}

static void extract_data(struct req_msg_head *msg)
{
	if (msg == NULL || msg->len <= sizeof(struct req_msg_head))
		return;

	switch (msg->type) {
	case UPDATE_TCP_CA_CONFIG:
		collect_cfg_info(msg);
		break;
	case UPDATE_UID_STATE:
		collect_uid_info(msg);
		break;
	case UPDATE_DS_STATE:
		collect_ds_state(msg);
		break;
	default:
		pr_info("msg error\n");
		break;
	}
}

/* Initialize internal variables and external interfaces */
msg_process *__init hw_tcp_cong_algo_ctrl_init(notify_event *notify)
{
	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}

	g_uid_info.cur_ds_state = DATA_SERVICE_OFF;
	spin_lock_init(&g_apks.lock);

	return extract_data;
}
