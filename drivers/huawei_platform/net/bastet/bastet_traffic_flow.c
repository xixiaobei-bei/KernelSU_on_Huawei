/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014. All rights reserved.
 * Description: Bastet Traffic Flow Control.
 *
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#include <net/sock.h>
#include <linux/file.h>
#include <linux/netfilter/xt_qtaguid.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <huawei_platform/net/bastet/bastet.h>
#include <huawei_platform/net/bastet/bastet_utils.h>

#include "xt_qtaguid_internal.h"

/* traffic flow update interval */
#define BST_FLOW_UPDATE_INTERVAL		(120 * HZ)
/* traffic flow wait timeout */
#define BST_FLOW_WAIT_TIMEOUT			(50)

void bastet_update_if_tag_stat(const char *ifname,
			uid_t uid, const struct sock *sk,
			enum ifs_tx_rx direction, int proto, int bytes)
{
}

int bastet_update_total_bytes(const char *dev_name,
			int proto, unsigned long tx_bytes,
			unsigned long rx_bytes)
{
	return 0;
}

/* declare and define traffic flow parameter */
static wait_queue_head_t bastet_flow_queue;
static int bastet_flow_wait;
static spinlock_t stat_lock;
static unsigned long last_time;
static bool is_waiting;

#pragma pack(1)
struct bst_uid_filter_buf {
	__s32 uid_num;
	__u8 data[0];
};

struct bst_uid_info {
	__u32 uid_min;
	__u32 uid_max;
	__u8 invert;
};
#pragma pack()

struct bst_uid_list_head {
	struct list_head header;
	__s32 uid_num;
};

struct bst_uid_list_node {
	struct list_head list;
	struct bst_uid_info info;
};

struct bst_uid_list_head uid_list_head = {
	.uid_num = 0,
};

/**
 * ifsproto_to_ipproto() - protocol transfer.
 * @protocol: ifs protocol type value
 *
 * transfer ifsproto to ipproto
 *
 * Return the IP protocol value.
 *
 */
static int ifsproto_to_ipproto(int32_t protocol)
{
	switch (protocol) {
	case IFS_TCP:
		return IPPROTO_TCP;
	case IFS_UDP:
		return IPPROTO_UDP;
	case IFS_PROTO_OTHER:
		bastet_loge("other protocol");
		return -EINVAL;
	default:
		bastet_loge("protocol not found");
		return -EINVAL;
	}
}

/**
 * ipproto_to_ifsproto() - protocol transfer.
 * @protocol: ip protocol type value. tcp/udp or others.
 *
 * transfer ipproto to ifsproto
 *
 * Return the IFS protocol value.
 *
 */
static int ipproto_to_ifsproto(int32_t protocol)
{
	switch (protocol) {
	case IPPROTO_TCP:
		return IFS_TCP;
	case IPPROTO_UDP:
		return IFS_UDP;
	default:
		bastet_loge("other protocol");
		return IFS_PROTO_OTHER;
	}
}

/**
 * handle_traffic_flow_update() - handle the update request.
 * @name: device name
 * @sk: pointer to socket
 * @proto: protocol value. tcp/udp or others.
 * @uid: uid of flow.
 * @tx_bytes: the send bytes of flow
 * @rx_bytes: the receive bytes of flow
 *
 * call flow update interface to adjust APP flow
 *
 * Return No
 *
 */
static void handle_traffic_flow_update(const char *name,
		struct sock *sk, int proto, uid_t uid,
		unsigned long tx_bytes, unsigned long rx_bytes)
{
	if (name == NULL) {
		bastet_loge("app flow name error");
		return;
	}

	bastet_update_if_tag_stat(name, uid, sk, IFS_TX, proto, tx_bytes);
	bastet_update_if_tag_stat(name, uid, sk, IFS_RX, proto, rx_bytes);
}

/**
 * adjust_traffic_flow_generic() - adjust the traffic flow.
 * @flow_p: pointer of traffic flow properties.
 *
 * if there is no sock, it can find uid and adjust traffic flow by generic,
 * handle_traffic_flow_update for one flow,
 * and bastet_update_total_bytes for total flow
 *
 * Return 0 is success
 * Otherwise is fail.
 *
 */
static int adjust_traffic_flow_generic(struct bst_traffic_flow_prop *flow_p)
{
	int uid, proto, protocol;
	unsigned long tx_bytes;
	unsigned long rx_bytes;

	if (flow_p == NULL) {
		bastet_loge("flow prop null pointer error!");
		return -EINVAL;
	}

	tx_bytes = flow_p->tx;
	rx_bytes = flow_p->rx;
	protocol = flow_p->protocol;
	uid = flow_p->uid;

	proto = ifsproto_to_ipproto(protocol);
	if (proto < 0) {
		bastet_loge("protocol not found error!");
		return -EINVAL;
	}

	/* adjust APP traffic flow */
	if (is_uid_valid(uid)) {
		handle_traffic_flow_update(cur_netdev_name, NULL,
			proto, uid, tx_bytes, rx_bytes);
	} else {
		bastet_loge("uid error!");
		return -EINVAL;
	}

	/* adjust total traffic flow */
	if (bastet_update_total_bytes(cur_netdev_name,
		protocol, tx_bytes, rx_bytes)) {
		bastet_loge("bastet_update_total_bytes error");
		return -EINVAL;
	}

	return 0;
}

/**
 * adjust_traffic_flow_by_sock() - modify the flow.
 * @sk: pointer of sock.
 * @tx: tx value
 * @rx: rx value.
 *
 * if there is sock, it can adjust traffic flow by sock,
 * handle_traffic_flow_update for one flow,
 * and bastet_update_total_bytes for total flow
 *
 * Return: 0 success
 * Otherwise is fail.
 */
int adjust_traffic_flow_by_sock(struct sock *sk,
		unsigned long tx, unsigned long rx)
{
	uid_t uid = 0;
	int proto = 0;
	const struct file *filp = NULL;
	struct socket *skt = NULL;

	if (sk == NULL) {
		bastet_loge("null sock error");
		return -EINVAL;
	}

	skt = sk->sk_socket;
	proto = ipproto_to_ifsproto((int)(sk->sk_protocol));
	if (skt != NULL) {
		filp = skt->file;
		if (filp != NULL)
			uid = filp->f_cred->uid.val;
	}

	/* adjust APP traffic flow */
	handle_traffic_flow_update(cur_netdev_name,
		sk, sk->sk_protocol, uid, tx, rx);
	/* adjust total traffic flow */
	if (bastet_update_total_bytes(cur_netdev_name, proto, tx, rx)) {
		bastet_loge("bastet_update_total_bytes error");
		return -EINVAL;
	}

	return 0;
}

static void bastet_clean_up_uid_list(void)
{
	struct bst_uid_list_node *pos = NULL;
	struct bst_uid_list_node *n = NULL;

	list_for_each_entry_safe(pos, n, &uid_list_head.header, list) {
		list_del(&pos->list);
		kfree(pos);
	}
	uid_list_head.uid_num = 0;
}

static void bastet_pack_uid_info(struct bst_uid_filter_buf *buf)
{
	struct bst_uid_list_node *pos = NULL;
	struct bst_uid_info *p_bst_uid_info = NULL;

	size_t off = sizeof(*p_bst_uid_info);
	size_t offset = 0;

	if (IS_ERR_OR_NULL(buf))
		return;

	list_for_each_entry(pos, &uid_list_head.header, list) {
		memcpy(buf->data + offset, &pos->info, off);
		offset += off;
	}
}

static void bastet_search_match(struct ipt_entry *ipt_e)
{
	struct xt_entry_match *ematch = NULL;
	struct xt_qtaguid_match_info *info = NULL;
	struct bst_uid_list_node *node = NULL;

	if (IS_ERR_OR_NULL(ipt_e))
		return;

	xt_ematch_foreach(ematch, ipt_e) {
		/*
		 * (xt_qtaguid.c)match name must
		 * be owner and revision must be 1
		 */
		if ((strcmp(ematch->u.kernel.match->name, "owner") != 0)
			|| (ematch->u.kernel.match->revision != 1)) {
			continue;
		}
		info = (struct xt_qtaguid_match_info *)ematch->data;
		if (is_uid_valid(info->uid_min)
			&& is_uid_valid(info->uid_max)) {
			node = kmalloc(sizeof(*node), GFP_ATOMIC);
			if (!IS_ERR_OR_NULL(node)) {
				node->info.uid_min = info->uid_min;
				node->info.uid_max = info->uid_max;
				node->info.invert = info->invert;
				list_add_tail(&node->list,
					&uid_list_head.header);
				uid_list_head.uid_num++;
			}
		}
	}
}

static void bastet_search_entry(struct xt_table_info *newinfo)
{
	void *loc_cpu_entry = NULL;
	struct ipt_entry *ipt_e = NULL;
	struct xt_entry_target *e_t = NULL;

	if (IS_ERR_OR_NULL(newinfo))
		return;

	loc_cpu_entry = newinfo->entries;
	xt_entry_foreach(ipt_e, loc_cpu_entry, newinfo->size) {
		e_t = ipt_get_target(ipt_e);
		/* target must be REJECT */
		if (strcmp(e_t->u.kernel.target->name, "REJECT") == 0)
			bastet_search_match(ipt_e);
	}
}

/**
 * bastet_save_uid_info() - save the uid information.
 * @table_name: point to the table name strings.
 * @newinfo: point to the uid info.
 * @buf_len:the info content length.
 *
 * use the xt_table_info to search the entry,
 * kmalloc the space to store the data.
 *
 * Return: (unsigned char *)saved the infomations.
 */
unsigned char *bastet_save_uid_info(const char *table_name,
		struct xt_table_info *newinfo, int *buf_len)
{
	struct bst_uid_filter_buf *buf = NULL;
	struct bst_uid_info *p_bst_uid_info = NULL;
	int uid_buf_len = 0;

	if (!is_bastet_enabled())
		return NULL;

	if (IS_ERR_OR_NULL(table_name) || strcmp(table_name, "filter") != 0
		|| IS_ERR_OR_NULL(newinfo))
		return NULL;

	bastet_search_entry(newinfo);
	uid_buf_len = sizeof(*buf)
		+ uid_list_head.uid_num * sizeof(*p_bst_uid_info);
	buf = kmalloc(uid_buf_len, GFP_ATOMIC);
	if (IS_ERR_OR_NULL(buf))
		goto err;

	buf->uid_num = uid_list_head.uid_num;
	*buf_len = uid_buf_len;
	bastet_pack_uid_info(buf);
err:
	bastet_clean_up_uid_list();

	return (unsigned char *)buf;
}

/**
 * bastet_indicate_uid_info() - indicate the uid information.
 * @buf: point to the buffer.
 * @buf_len:the length of buffer
 *
 * if bastet is actived, and buffer is not null. then indicate the packet.
 *
 * Return: No
 */
void bastet_indicate_uid_info(unsigned char *buf, int buf_len)
{
	if (!is_bastet_enabled())
		return;

	if (IS_ERR_OR_NULL(buf))
		return;

	post_indicate_packet(BST_IND_NETFILTER_SYNC_UID, buf, buf_len);
	kfree(buf);
}

/**
 * bastet_wait_traffic_flow() - wait the traffic flow.
 *
 * indicate traffic flow request and wait event interrupt,
 * two minute once
 *
 * Return: No
 */
void bastet_wait_traffic_flow(void)
{
	unsigned long delta = 0;
	unsigned long irq_flags;

	if (!is_bastet_enabled())
		return;

	spin_lock_irqsave(&stat_lock, irq_flags);
	delta = jiffies - last_time;
	if (delta < BST_FLOW_UPDATE_INTERVAL) {
		spin_unlock_irqrestore(&stat_lock, irq_flags);
		return;
	}
	last_time = jiffies;
	is_waiting = true;
	spin_unlock_irqrestore(&stat_lock, irq_flags);

	if (post_indicate_packet(BST_IND_TRAFFIC_FLOW_REQ, NULL, 0)) {
		bastet_loge("post_indicate_packet failed!");
		return;
	}

	wait_event_interruptible_timeout(bastet_flow_queue,
		bastet_flow_wait, msecs_to_jiffies(BST_FLOW_WAIT_TIMEOUT));
	bastet_flow_wait = 0;
}

/**
 * bastet_wake_up_traffic_flow() - wake up the traffic flow.
 *
 * wake up traffic flow interrupt
 *
 * Return: No
 */
void bastet_wake_up_traffic_flow(void)
{
	unsigned long irq_flags;

	if (is_waiting) {
		bastet_flow_wait = 1;
		wake_up_interruptible(&bastet_flow_queue);
		spin_lock_irqsave(&stat_lock, irq_flags);
		is_waiting = false;
		spin_unlock_irqrestore(&stat_lock, irq_flags);
	}
}

/**
 * adjust_traffic_flow_by_pkg() - adjust traffic flow.
 * @buf: point to the buffer.
 * @cnt: traffic flow number
 *
 * decompression traffic flow package and wake up interrupt
 *
 * Return: 0 is success
 * Otherwise is fail.
 */
int adjust_traffic_flow_by_pkg(uint8_t *buf, int cnt)
{
	int err, i;
	struct socket *skt = NULL;
	struct sock *sk = NULL;
	struct bst_traffic_flow_prop *ptr = (struct bst_traffic_flow_prop *)buf;

	if (buf == NULL || cnt == 0) {
		bastet_wake_up_traffic_flow();
		return 0;
	}

	/* decompression package, and adjust traffic flow by sock or generic */
	for (i = 0; i < cnt; i++) {
		skt = sockfd_lookup_by_fd_pid(ptr->fd, ptr->pid, &err);
		if (skt != NULL) {
			sk = skt->sk;
			if (sk != NULL) {
				sock_hold(sk);
				adjust_traffic_flow_by_sock(sk,
					ptr->tx, ptr->rx);
				sock_put(sk);
			}
			sockfd_put(skt);
		} else {
			adjust_traffic_flow_generic(ptr);
		}
		ptr++;
	}

	bastet_wake_up_traffic_flow();

	return 0;
}

/**
 * bastet_traffic_flow_init() - initializes the bastet traffic flow.
 *
 * initializes global virables which releated with the
 * bastet traffic flow.
 *
 * Return: No
 */
void bastet_traffic_flow_init(void)
{
	init_waitqueue_head(&bastet_flow_queue);
	spin_lock_init(&stat_lock);
	INIT_LIST_HEAD(&uid_list_head.header);
}
