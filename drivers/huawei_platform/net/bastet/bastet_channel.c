/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: Bastet Tcp priority channel.
 * Author: zhuweichen@huawei.com
 * Create: 2015-09-01
 */

#include <linux/fdtable.h>
#include <linux/file.h>
#include <net/tcp.h>
#include <net/inet_sock.h>
#include <net/sock.h>
#include <huawei_platform/net/bastet/bastet.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet_interface.h>

#define BST_PRIO_NORM 0
#define BST_PRIO_HIGH 1
#define BST_PRIO_LPWR 2

#define MAX_PRIORITY_DATA_MEM 1500
#define MAX_PRIORITY_DATA_PKT 1330
#define MAX_COUNT_SIZE 1024
#define WAIT_SOCK_PROP_TIMEOUT (3 * HZ)

#define MOBILE_NAME "rmnet"

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#ifndef tcp_jiffies32
#define tcp_jiffies32 tcp_time_stamp
#endif
#endif

const uint8_t flags_prop_none = 0;
const uint8_t flags_prop_seq_only = 1;
const uint8_t flags_prop_seq_ts = 2;
const uint8_t flags_prop_full = 3;

const uint8_t flags_pkt_type_none;
const uint8_t flags_pkt_type_head;
const uint8_t flags_pkt_type_tail;
const uint8_t flags_pkt_type_single;

static void bastet_get_link_id(struct sock *sk, struct bst_sock_id *guide)
{
	struct fdtable *fdt = NULL;
	struct socket *sock = NULL;
	unsigned int i;
	int err = 0;
	bool match = false;

	fdt = files_fdtable(current->files);
	for (i = 0; i <= fdt->max_fds; i++) {
		sock = sockfd_lookup(i, &err);
		if (!sock)
			continue;
		match = (sock->sk == sk);
		sockfd_put(sock);
		if (match)
			break;
	}
	guide->pid = current->pid;
	guide->fd = match ? i : 0;
}

static uint8_t bastet_get_prio_level(uint32_t flags)
{
	uint8_t level;

	if (flags & MSG_HRT)
		level = BST_PRIO_HIGH;
	else if (flags & MSG_LPW)
		level = BST_PRIO_LPWR;
	else
		level = BST_PRIO_NORM;

	return level;
}

static uint8_t set_packet_type(uint32_t cur, uint32_t count, size_t size)
{
	uint8_t type;

	if (count == 0 || size == MAX_PRIORITY_DATA_PKT)
		type = flags_pkt_type_single;
	else if (cur == 0)
		type = flags_pkt_type_head;
	else if (cur == count)
		type = flags_pkt_type_tail;
	else
		type = flags_pkt_type_none;

	return type;
}

static int32_t bastet_send_priority_data(struct sock *sk,
	struct msghdr *msg, struct bst_ind_priority_prop *prop,
	bool wq_empty)
{
	int32_t ret = 0;
	const struct iovec *iov = msg->msg_iter.iov;
	size_t size = iov->iov_len;
	unsigned char __user *from = iov->iov_base;
	uint32_t count;
	uint32_t len;
	uint32_t i;

	count = size / MAX_PRIORITY_DATA_PKT;
	if (count > MAX_COUNT_SIZE) {
		bastet_loge("invalid count size %d", count);
		return -EINVAL;
	}
	for (i = 0; i <= count; i++) {
		len = count > i ?
			MAX_PRIORITY_DATA_PKT : size % MAX_PRIORITY_DATA_PKT;
		if (len == 0)
			break;
		prop->pkt_type = set_packet_type(i, count, size);
		prop->sync_prop.seq = tcp_sk(sk)->write_seq +
			i * MAX_PRIORITY_DATA_PKT;
		tcp_mstamp_refresh(tcp_sk(sk));
		prop->sync_prop.ts_current = tcp_time_stamp(tcp_sk(sk)) +
			tcp_sk(sk)->tsoffset;
		prop->len = len;
		if (i == 0) {
			if (wq_empty)
				prop->seq_type = flags_prop_full;
			else
				prop->seq_type = flags_prop_seq_ts;
		} else {
			prop->seq_type = flags_prop_none;
		}
		ret = copy_from_user(prop->data,
			from + i * MAX_PRIORITY_DATA_PKT, len);
		if (ret)
			break;

		post_indicate_packet(BST_IND_PRIORITY_DATA, prop,
			sizeof(*prop) + len);
		memset(prop->data, 0, len);
	}
	return ret;
}

static void bastet_send_wq_data(struct sock *sk,
	struct sk_buff *skb, struct bst_ind_priority_prop *prop,
	bool first_skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	uint32_t size = skb->len;
	uint32_t count;
	uint32_t len;
	uint32_t i;

	count = size / MAX_PRIORITY_DATA_PKT;
	for (i = 0; i <= count; i++) {
		len = count > i
			? MAX_PRIORITY_DATA_PKT : size % MAX_PRIORITY_DATA_PKT;
		if (len == 0)
			break;
		prop->pkt_type = set_packet_type(i, count, size);
		prop->sync_prop.seq = TCP_SKB_CB(skb)->seq +
			i * MAX_PRIORITY_DATA_PKT;
		prop->sync_prop.ts_current = tp->tsoffset;
		prop->len = len;
		prop->seq_type = (first_skb && i == 0) ?
			flags_prop_full : flags_prop_seq_ts;
		memcpy(prop->data, skb->data, len);
		post_indicate_packet(BST_IND_PRIORITY_DATA, prop,
			sizeof(*prop) + len);
		memset(prop->data, 0, len);
	}
}

static void bastet_pick_wq(struct sock *sk, struct bst_ind_priority_prop *prop)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct sk_buff *skb = NULL;
	bool first = true;

	skb = tcp_write_queue_head(sk);
	while (skb != NULL) {
		bastet_send_wq_data(sk, skb, prop, first);
		if (first)
			first = false;
		tcp_unlink_write_queue(skb, sk);
		sk_wmem_free_skb(sk, skb);
		if (skb == tp->retransmit_skb_hint)
			tp->retransmit_skb_hint = NULL;
		if (skb == tp->lost_skb_hint)
			tp->lost_skb_hint = NULL;
		skb = tcp_write_queue_head(sk);
	}
	tp->packets_out = 0;
	tp->retrans_out = 0;
	tp->fackets_out = 0;
	tp->sacked_out = 0;
	tp->lost_out = 0;
	tp->rcv_tstamp = tcp_jiffies32;
	sk->sk_send_head = NULL;

	tcp_rearm_rto(sk);
}

static void bastet_update_seq(struct sock *sk, size_t size, bool block)
{
	struct tcp_sock *tp = tcp_sk(sk);
	int ret;

	if (block && sk->bastet) {
		sk->bastet->flag = false;
		ret = wait_event_interruptible_timeout(sk->bastet->wq,
			sk->bastet->flag, WAIT_SOCK_PROP_TIMEOUT);
		bastet_logi("wait_event_interruptible_timeout ret=%d", ret);
	}
	tp->write_seq += size;
	tp->snd_nxt = tp->write_seq;
}

static void handle_bastet_sock_state(uint8_t sync_state,
	struct sock *sk, size_t size)
{
	switch (sync_state) {
	case BST_SOCK_VALID:
		/* sock sync property is in AP, so cancel sync timer */
		bastet_sync_prop_cancel(sk);
		/* fall-through */
	case BST_SOCK_NOT_USED:
	case BST_SOCK_NOT_CREATED:
		bastet_update_seq(sk, size, false);
		break;
	case BST_SOCK_INVALID:
	case BST_SOCK_UPDATING:
		bastet_update_seq(sk, size, true);
		break;
	default:
		bastet_loge("Unknown sync state: %d", sync_state);
		break;
	}
}

static int bastet_send_priority_msg_internal(struct sock *sk,
	struct msghdr *msg, size_t size)
{
	struct bst_ind_priority_prop *prop = NULL;
	struct bst_sock_id guide;
	struct bst_sock_sync_prop sync_prop;
	struct bst_sock_comm_prop comm_prop;
	uint32_t flags = msg->msg_flags;
	uint8_t sync_state;
	bool empty = false;

	sk->prio_channel = true;
	bastet_get_link_id(sk, &guide);
	bastet_get_comm_prop(sk, &comm_prop);
	bastet_get_sock_prop(sk, &sync_prop);
	sync_state = get_bastet_sock_state(sk);

	prop = kzalloc(MAX_PRIORITY_DATA_MEM, GFP_ATOMIC);
	if (prop == NULL) {
		bastet_loge("failed to kmalloc");
		return 0;
	}
	memset(prop, 0, MAX_PRIORITY_DATA_MEM);
	prop->guide = guide;
	prop->comm_prop = comm_prop;
	prop->sync_prop = sync_prop;
	prop->priority = bastet_get_prio_level(flags);
	prop->sync_state = sync_state;

	set_channel_occupied();
	empty = tcp_write_queue_empty(sk);
	if (!empty)
		bastet_pick_wq(sk, prop);
	if (bastet_send_priority_data(sk, msg, prop, empty)) {
		kfree(prop);
		clear_channel_occupied();
		return -EFAULT;
	}

	kfree(prop);
	handle_bastet_sock_state(sync_state, sk, size);

	return 1;
}

/*
 * bastet_check_mobile - bastet mobile state check.
 * @sk, struct sock pointer
 *
 * check whether socket network type is mobile
 *
 * Return: ture, mobile
 * false, non-mobile
 */
static bool bastet_check_mobile(struct sock *sk)
{
	if (!is_wifi_proxy(sk)) {
		struct dst_entry *dst = NULL;
		/* struct net_device is in struct dst_entry */
		dst = __sk_dst_get(sk);
		if (dst && dst->dev) {
			/* check whether socket network device name is mobile */
			if (memcmp(MOBILE_NAME, dst->dev->name,
				strlen(MOBILE_NAME)) == 0)
				return true;
		}
	}

	return false;
}

static bool bastet_check_prio_flag(struct sock *sk, struct msghdr *msg)
{
	if (IS_ERR_OR_NULL(msg) && !(msg->msg_flags & (MSG_HRT | MSG_LPW)))
		return false;

	/* only mobile type support hrt&lpw channel */
	if (!bastet_check_mobile(sk))
		return false;
	if (msg->msg_flags & MSG_HRT) {
		/*
		 * if uid is foreground, it means this
		 * uid had been identified as hrt uid.
		 */
		if (!is_sock_foreground(sk)) {
			/*
			 * if sk is not identified as prio_channel,
			 * it means it is the first time,
			 * indicate bastetd to identify it to hrt uid.
			 */
			if (!sk->prio_channel) {
				uid_t uid = sock_i_uid(sk).val;

				bastet_logi("BST_IND_PRIORITY_UID, uid=%d",
					uid);
				post_indicate_packet(BST_IND_PRIORITY_UID,
					&uid, sizeof(int32_t));
				/*
				 * continue to send hrt data,
				 * no matter foreground or background
				 */
			} else {
				/*
				 * it is prio_channel,
				 * but not in the foreground
				 */
				return false;
			}
		}
	}
	return true;
}

/*
 * bastet_send_priority_msg - send the priority msg.
 * @sk: struct sock pointer
 * @msg: message header
 * @size: message size
 *
 * jude the message state, if the channel,proxy,buffer is ok,
 * then send the message into priority.
 *
 * Return: 1 success
 * Otherwise is fail.
 */
int bastet_send_priority_msg(struct sock *sk, struct msghdr *msg, size_t size)
{
	if (!is_bastet_enabled())
		return 0;
	/* cdma mode and modem proxy mask is 0 will disable channel */
	if (!is_channel_enable())
		return 0;
	if (!is_proxy_available())
		return 0;
	if (!is_buffer_available())
		return 0;
	if (!is_channel_available())
		return 0;
	if (bastet_check_prio_flag(sk, msg))
		return bastet_send_priority_msg_internal(sk, msg, size);

	return 0;
}
