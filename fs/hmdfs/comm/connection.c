/* SPDX-License-Identifier: GPL-2.0 */
/*
 * connection.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: send and recv msg  for  hmdfs
 * Author: liuxuesong3@huawei.com chenjinglong1@huawei.com
 * Create: 2020-04-14
 *
 */

#include "connection.h"

#include <linux/file.h>
#include <linux/freezer.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/workqueue.h>

#include "device_node.h"
#include "hmdfs.h"
#include "message_verify.h"
#include "node_cb.h"
#include "protocol.h"
#include "socket_adapter.h"
#include "DFS_1_0/adapter_protocol.h"

#ifdef CONFIG_HMDFS_CRYPTO
#include "crypto.h"
#endif

#define HMDFS_WAIT_REQUST_END_MIN 20
#define HMDFS_WAIT_REQUST_END_MAX 30

#define HMDFS_WAIT_CONN_RELEASE (3 * HZ)

#define HMDFS_RETRY_WB_WQ_MAX_ACTIVE 16

static void hs_fill_crypto_data(struct connection *conn_impl, __u8 ops,
				void *data, __u32 len)
{
	struct crypto_body *body = NULL;

	if (len < sizeof(struct crypto_body)) {
		hmdfs_info("crpto body len %u is err", len);
		return;
	}
	body = (struct crypto_body *)data;

	/* this is only test, later need to fill right algorithm. */
	body->crypto |= HMDFS_HS_CRYPTO_KTLS_AES128;
	body->crypto = cpu_to_le32(body->crypto);

	hmdfs_info("fill crypto. ccrtypto=0x%08x", body->crypto);
}

static int hs_parse_crypto_data(struct connection *conn_impl, __u8 ops,
				 void *data, __u32 len)
{
	struct crypto_body *hs_crypto = NULL;
	uint32_t crypto;

	if (len < sizeof(struct crypto_body)) {
		hmdfs_info("handshake msg len error, len=%u", len);
		return -1;
	}
	hs_crypto = (struct crypto_body *)data;
	crypto = le16_to_cpu(hs_crypto->crypto);
	conn_impl->crypto = crypto;
	hmdfs_info("ops=%u, len=%u, crypto=0x%08x", ops, len, crypto);
	return 0;
}

static void hs_fill_case_sense_data(struct connection *conn_impl, __u8 ops,
				    void *data, __u32 len)
{
	struct case_sense_body *body = (struct case_sense_body *)data;

	if (len < sizeof(struct case_sense_body)) {
		hmdfs_err("case sensitive len %u is err", len);
		return;
	}
	body->case_sensitive = conn_impl->node->sbi->s_case_sensitive ? 1 : 0;
}

static int hs_parse_case_sense_data(struct connection *conn_impl, __u8 ops,
				     void *data, __u32 len)
{
	struct case_sense_body *body = (struct case_sense_body *)data;
	__u8 sensitive = conn_impl->node->sbi->s_case_sensitive ? 1 : 0;

	if (len < sizeof(struct case_sense_body)) {
		hmdfs_info("case sensitive len %u is err", len);
		return -1;
	}
	if (body->case_sensitive != sensitive) {
		hmdfs_err("case sensitive inconsistent, server: %u,client: %u, ops: %u",
			  body->case_sensitive, sensitive, ops);
		return -1;
	}
	return 0;
}

static void hs_fill_feature_data(struct connection *conn_impl, __u8 ops,
				 void *data, __u32 len)
{
	struct feature_body *body = (struct feature_body *)data;

	if (len < sizeof(struct feature_body)) {
		hmdfs_err("feature len %u is err", len);
		return;
	}
	body->features = cpu_to_le64(conn_impl->node->sbi->s_features);
	body->reserved = cpu_to_le64(0);
}

static int hs_parse_feature_data(struct connection *conn_impl, __u8 ops,
				 void *data, __u32 len)
{
	struct feature_body *body = (struct feature_body *)data;

	if (len < sizeof(struct feature_body)) {
		hmdfs_err("feature len %u is err", len);
		return -1;
	}

	conn_impl->node->features = le64_to_cpu(body->features);
	return 0;
}

/* should ensure len is small than 0xffff. */
static const struct conn_hs_extend_reg s_hs_extend_reg[HS_EXTEND_CODE_COUNT] = {
	[HS_EXTEND_CODE_CRYPTO] = {
		.len = sizeof(struct crypto_body),
		.resv = 0,
		.filler = hs_fill_crypto_data,
		.parser = hs_parse_crypto_data
	},
	[HS_EXTEND_CODE_CASE_SENSE] = {
		.len = sizeof(struct case_sense_body),
		.resv = 0,
		.filler = hs_fill_case_sense_data,
		.parser = hs_parse_case_sense_data,
	},
	[HS_EXTEND_CODE_FEATURE_SUPPORT] = {
		.len = sizeof(struct feature_body),
		.resv = 0,
		.filler = hs_fill_feature_data,
		.parser = hs_parse_feature_data,
	},
	[HS_EXTEND_CODE_FEATURE_SUPPORT] = {
		.len = sizeof(struct feature_body),
		.resv = 0,
		.filler = hs_fill_feature_data,
		.parser = hs_parse_feature_data,
	},
};

static __u32 hs_get_extend_data_len(void)
{
	__u32 len;
	int i;

	len = sizeof(struct conn_hs_extend_head);

	for (i = 0; i < HS_EXTEND_CODE_COUNT; i++) {
		len += sizeof(struct extend_field_head);
		len += s_hs_extend_reg[i].len;
	}

	hmdfs_info("extend data total len is %u", len);
	return len;
}

static void hs_fill_extend_data(struct connection *conn_impl, __u8 ops,
				void *extend_data, __u32 len)
{
	struct conn_hs_extend_head *extend_head = NULL;
	struct extend_field_head *field = NULL;
	uint8_t *body = NULL;
	__u32 offset;
	__u16 i;

	if (sizeof(struct conn_hs_extend_head) > len) {
		hmdfs_info("len error. len=%u", len);
		return;
	}
	extend_head = (struct conn_hs_extend_head *)extend_data;
	extend_head->field_cn = 0;
	offset = sizeof(struct conn_hs_extend_head);

	for (i = 0; i < HS_EXTEND_CODE_COUNT; i++) {
		if (sizeof(struct extend_field_head) > (len - offset))
			break;
		field = (struct extend_field_head *)((uint8_t *)extend_data +
						     offset);
		offset += sizeof(struct extend_field_head);

		if (s_hs_extend_reg[i].len > (len - offset))
			break;
		body = (uint8_t *)extend_data + offset;
		offset += s_hs_extend_reg[i].len;

		field->code = cpu_to_le16(i);
		field->len = cpu_to_le16(s_hs_extend_reg[i].len);

		if (s_hs_extend_reg[i].filler)
			s_hs_extend_reg[i].filler(conn_impl, ops,
					body, s_hs_extend_reg[i].len);

		extend_head->field_cn += 1;
	}

	extend_head->field_cn = cpu_to_le32(extend_head->field_cn);
}

static int hs_parse_extend_data(struct connection *conn_impl, __u8 ops,
				void *extend_data, __u32 extend_len)
{
	struct conn_hs_extend_head *extend_head = NULL;
	struct extend_field_head *field = NULL;
	uint8_t *body = NULL;
	__u32 offset;
	__u32 field_cnt;
	__u16 code;
	__u16 len;
	int i;
	int ret;

	if (sizeof(struct conn_hs_extend_head) > extend_len) {
		hmdfs_err("ops=%u,extend_len=%u", ops, extend_len);
		return -1;
	}
	extend_head = (struct conn_hs_extend_head *)extend_data;
	field_cnt = le32_to_cpu(extend_head->field_cn);
	hmdfs_info("extend_len=%u,field_cnt=%u", extend_len, field_cnt);

	offset = sizeof(struct conn_hs_extend_head);

	for (i = 0; i < field_cnt; i++) {
		if (sizeof(struct extend_field_head) > (extend_len - offset)) {
			hmdfs_err("cnt err, op=%u, extend_len=%u, cnt=%u, i=%u",
				  ops, extend_len, field_cnt, i);
			return -1;
		}
		field = (struct extend_field_head *)((uint8_t *)extend_data +
						     offset);
		offset += sizeof(struct extend_field_head);
		code = le16_to_cpu(field->code);
		len = le16_to_cpu(field->len);
		if (len > (extend_len - offset)) {
			hmdfs_err("len err, op=%u, extend_len=%u, cnt=%u, i=%u",
				  ops, extend_len, field_cnt, i);
			hmdfs_err("len err, code=%u, len=%u, offset=%u", code,
				  len, offset);
			return -1;
		}

		body = (uint8_t *)extend_data + offset;
		offset += len;
		if ((code < HS_EXTEND_CODE_COUNT) &&
		    (s_hs_extend_reg[code].parser)) {
			ret = s_hs_extend_reg[code].parser(conn_impl, ops,
							   body, len);
			if (ret)
				return ret;
		}
	}
	return 0;
}

static int hs_proc_msg_data(struct connection *conn_impl, __u8 ops, void *data,
			    __u32 data_len)
{
	struct connection_handshake_req *hs_req = NULL;
	uint8_t *extend_data = NULL;
	__u32 extend_len;
	__u32 req_len;
	int ret;

	if (!data) {
		hmdfs_err("err, msg data is null");
		return -1;
	}

	if (data_len < sizeof(struct connection_handshake_req)) {
		hmdfs_err("ack msg data len error. data_len=%u, device_id=%llu",
			  data_len, conn_impl->node->device_id);
		return -1;
	}

	hs_req = (struct connection_handshake_req *)data;
	req_len = le32_to_cpu(hs_req->len);
	if (req_len > (data_len - sizeof(struct connection_handshake_req))) {
		hmdfs_info(
			"ack msg hs_req len(%u) error. data_len=%u, device_id=%llu",
			req_len, data_len, conn_impl->node->device_id);
		return -1;
	}
	extend_len =
		data_len - sizeof(struct connection_handshake_req) - req_len;
	extend_data = (uint8_t *)data +
		      sizeof(struct connection_handshake_req) + req_len;
	ret = hs_parse_extend_data(conn_impl, ops, extend_data, extend_len);
	if (!ret)
		hmdfs_info(
			"hs msg rcv, ops=%u, data_len=%u, device_id=%llu, req_len=%u",
			ops, data_len, conn_impl->node->device_id, hs_req->len);
	return ret;
}
#ifdef CONFIG_HMDFS_CRYPTO
static int connection_handshake_init_tls(struct connection *conn_impl, __u8 ops)
{
	// init ktls config, use key1/key2 as init write-key of each direction
	__u8 key1[HMDFS_KEY_SIZE];
	__u8 key2[HMDFS_KEY_SIZE];
	int ret;

	if ((ops != CONNECT_MESG_HANDSHAKE_RESPONSE) &&
	    (ops != CONNECT_MESG_HANDSHAKE_ACK)) {
		hmdfs_err("ops %u is err", ops);
		return -EINVAL;
	}

	update_key(conn_impl->master_key, key1, HKDF_TYPE_KEY_INITIATOR);
	update_key(conn_impl->master_key, key2, HKDF_TYPE_KEY_ACCEPTER);

	if (ops == CONNECT_MESG_HANDSHAKE_ACK) {
		memcpy(conn_impl->send_key, key1, HMDFS_KEY_SIZE);
		memcpy(conn_impl->recv_key, key2, HMDFS_KEY_SIZE);
	} else {
		memcpy(conn_impl->send_key, key2, HMDFS_KEY_SIZE);
		memcpy(conn_impl->recv_key, key1, HMDFS_KEY_SIZE);
	}

	memset(key1, 0, HMDFS_KEY_SIZE);
	memset(key2, 0, HMDFS_KEY_SIZE);

	hmdfs_info("hs: ops=%u start set crypto tls", ops);
	ret = tls_crypto_info_init(conn_impl);
	if (ret)
		hmdfs_err("setting tls fail. ops is %u", ops);

	return ret;
}
#endif

static int do_send_handshake(struct connection *conn_impl, __u8 ops,
			     __le16 request_id)
{
	int err;
	struct connection_msg_head *hs_head = NULL;
	struct connection_handshake_req *hs_data = NULL;
	uint8_t *hs_extend_data = NULL;
	struct hmdfs_send_data msg;
	__u64 local_iid = conn_impl->node->sbi->local_info.iid;
	__u32 send_len;
	__u32 len;
	__u32 extend_len;
	char buf[DEVICE_IID_LENGTH] = { 0 };

	len = scnprintf(buf, DEVICE_IID_LENGTH, "%llu", local_iid);
	send_len = sizeof(struct connection_msg_head) +
		   sizeof(struct connection_handshake_req) + len;

	if (((ops == CONNECT_MESG_HANDSHAKE_RESPONSE) ||
	     (ops == CONNECT_MESG_HANDSHAKE_ACK)) &&
	    (conn_impl->node->version >= DFS_2_0)) {
		extend_len = hs_get_extend_data_len();
		send_len += extend_len;
	}

	hs_head = kzalloc(send_len, GFP_KERNEL);
	if (!hs_head)
		return -ENOMEM;

	hs_data = (struct connection_handshake_req
			   *)((uint8_t *)hs_head +
			      sizeof(struct connection_msg_head));

	hs_data->len = cpu_to_le32(len);
	memcpy(hs_data->dev_id, buf, len);

	if (((ops == CONNECT_MESG_HANDSHAKE_RESPONSE) ||
	     ops == CONNECT_MESG_HANDSHAKE_ACK) &&
	    (conn_impl->node->version >= DFS_2_0)) {
		hs_extend_data = (uint8_t *)hs_data +
				  sizeof(struct connection_handshake_req) + len;
		hs_fill_extend_data(conn_impl, ops, hs_extend_data, extend_len);
	}

	hs_head->magic = HMDFS_MSG_MAGIC;
	hs_head->version = DFS_2_0;
	hs_head->flags |= 0x1;
	hmdfs_info("Send handshake message: ops = %d, fd = %d", ops,
		   ((struct tcp_handle *)(conn_impl->connect_handle))->fd);
	hs_head->operations = ops;
	hs_head->request_id = request_id;
	hs_head->datasize = cpu_to_le32(send_len);
	hs_head->source = cpu_to_le64(local_iid);
	hs_head->msg_id = 0;

	msg.head = hs_head;
	msg.head_len = sizeof(struct connection_msg_head);
	msg.data = hs_data;
	msg.len = send_len - msg.head_len;
	msg.sdesc = NULL;
	msg.sdesc_len = 0;
	err = conn_impl->send_message(conn_impl, &msg);
	kfree(hs_head);
	return err;
}

static int hmdfs_node_waiting_evt_sum(const struct hmdfs_peer *node)
{
	int sum = 0;
	int i;

	for (i = 0; i < RAW_NODE_EVT_NR; i++)
		sum += node->waiting_evt[i];

	return sum;
}

static int hmdfs_update_node_waiting_evt(struct hmdfs_peer *node, int evt,
					 unsigned int *seq)
{
	int last;
	int sum;
	unsigned int next;

	sum = hmdfs_node_waiting_evt_sum(node);
	if (sum % RAW_NODE_EVT_NR)
		last = !node->pending_evt;
	else
		last = node->pending_evt;

	/* duplicated event */
	if (evt == last) {
		node->dup_evt[evt]++;
		return 0;
	}

	node->waiting_evt[evt]++;
	hmdfs_debug("add node->waiting_evt[%d]=%d", evt,
		    node->waiting_evt[evt]);

	/* offline wait + online wait + offline wait = offline wait
	 * online wait + offline wait + online wait != online wait
	 * As the first online related resource (e.g. fd) must be invalidated
	 */
	if (node->waiting_evt[RAW_NODE_EVT_OFF] >= 2 &&
	    node->waiting_evt[RAW_NODE_EVT_ON] >= 1) {
		node->waiting_evt[RAW_NODE_EVT_OFF] -= 1;
		node->waiting_evt[RAW_NODE_EVT_ON] -= 1;
		node->seq_wr_idx -= 2;
		node->merged_evt += 2;
	}

	next = hmdfs_node_inc_evt_seq(node);
	node->seq_tbl[(node->seq_wr_idx++) % RAW_NODE_EVT_MAX_NR] = next;
	*seq = next;

	return 1;
}

static void hmdfs_run_evt_cb_verbosely(struct hmdfs_peer *node, int raw_evt,
				       bool sync, unsigned int seq)
{
	int evt = (raw_evt == RAW_NODE_EVT_OFF) ? NODE_EVT_OFFLINE :
						  NODE_EVT_ONLINE;
	int cur_evt_idx = sync ? 1 : 0;

	node->cur_evt[cur_evt_idx] = raw_evt;
	node->cur_evt_seq[cur_evt_idx] = seq;
	hmdfs_node_call_evt_cb(node, evt, sync, seq);
	node->cur_evt[cur_evt_idx] = RAW_NODE_EVT_NR;
}

static void hmdfs_node_evt_work(struct work_struct *work)
{
	struct hmdfs_peer *node =
		container_of(work, struct hmdfs_peer, evt_dwork.work);
	unsigned int seq;

	/*
	 * N-th sync cb completes before N-th async cb,
	 * so use seq_lock as a barrier in read & write path
	 * to ensure we can read the required seq.
	 */
	mutex_lock(&node->seq_lock);
	seq = node->seq_tbl[(node->seq_rd_idx++) % RAW_NODE_EVT_MAX_NR];
	hmdfs_run_evt_cb_verbosely(node, node->pending_evt, false, seq);
	mutex_unlock(&node->seq_lock);

	mutex_lock(&node->evt_lock);
	if (hmdfs_node_waiting_evt_sum(node)) {
		node->pending_evt = !node->pending_evt;
		node->pending_evt_seq =
			node->seq_tbl[node->seq_rd_idx % RAW_NODE_EVT_MAX_NR];
		node->waiting_evt[node->pending_evt]--;
		/* sync cb has been done */
		schedule_delayed_work(&node->evt_dwork,
				      node->sbi->async_cb_delay * HZ);
	} else {
		node->last_evt = node->pending_evt;
		node->pending_evt = RAW_NODE_EVT_NR;
	}
	mutex_unlock(&node->evt_lock);
}

/*
 * The running orders of cb are:
 *
 * (1) sync callbacks are invoked according to the queue order of raw events:
 *     ensured by seq_lock.
 * (2) async callbacks are invoked according to the queue order of raw events:
 *     ensured by evt_lock & evt_dwork
 * (3) async callback is invoked after sync callback of the same raw event:
 *     ensured by seq_lock.
 * (4) async callback of N-th raw event and sync callback of (N+x)-th raw
 *     event can run concurrently.
 */
static void hmdfs_queue_raw_node_evt(struct hmdfs_peer *node, int evt)
{
	unsigned int seq = 0;

	mutex_lock(&node->evt_lock);
	if (node->pending_evt == RAW_NODE_EVT_NR) {
		if (evt == node->last_evt) {
			node->dup_evt[evt]++;
			mutex_unlock(&node->evt_lock);
			return;
		}
		node->pending_evt = evt;
		seq = hmdfs_node_inc_evt_seq(node);
		node->seq_tbl[(node->seq_wr_idx++) % RAW_NODE_EVT_MAX_NR] = seq;
		node->pending_evt_seq = seq;
		mutex_lock(&node->seq_lock);
		mutex_unlock(&node->evt_lock);
		/* call sync cb, then async cb */
		hmdfs_run_evt_cb_verbosely(node, evt, true, seq);
		mutex_unlock(&node->seq_lock);
		schedule_delayed_work(&node->evt_dwork,
				      node->sbi->async_cb_delay * HZ);
	} else if (hmdfs_update_node_waiting_evt(node, evt, &seq) > 0) {
		/*
		 * Take seq_lock firstly to ensure N-th sync cb
		 * is called before N-th async cb.
		 */
		mutex_lock(&node->seq_lock);
		mutex_unlock(&node->evt_lock);
		hmdfs_run_evt_cb_verbosely(node, evt, true, seq);
		mutex_unlock(&node->seq_lock);
	} else {
		mutex_unlock(&node->evt_lock);
	}
}

void connection_send_handshake(struct connection *conn_impl, __u8 ops,
			       __le16 request_id)
{
	struct tcp_handle *tcp = NULL;
	int err = do_send_handshake(conn_impl, ops, request_id);

	if (likely(err >= 0))
		return;

	tcp = conn_impl->connect_handle;
	hmdfs_err("Failed to send handshake: err = %d, fd = %d", err, tcp->fd);
	hmdfs_reget_connection(conn_impl);
}

void connection_handshake_notify(struct hmdfs_peer *node, int notify_type)
{
	struct notify_param param;

	param.notify = notify_type;
	param.remote_iid = node->iid;
	param.fd = INVALID_SOCKET_FD;
	memcpy(param.remote_cid, node->cid, HMDFS_CID_SIZE);
	notify(node, &param);
}


void peer_online(struct hmdfs_peer *peer)
{
	// To evaluate if someone else has made the peer online
	u8 prev_stat = xchg(&peer->status, NODE_STAT_ONLINE);
	unsigned long jif_tmp = jiffies;

	if (prev_stat == NODE_STAT_ONLINE)
		return;
	WRITE_ONCE(peer->conn_time, jif_tmp);
	WRITE_ONCE(peer->sbi->connections.recent_ol, jif_tmp);
	hmdfs_queue_raw_node_evt(peer, RAW_NODE_EVT_ON);
	connection_handshake_notify(peer, NOTIFY_HS_DONE);
}

void connection_to_working(struct hmdfs_peer *node)
{
	struct connection *conn_impl = NULL;
	struct tcp_handle *tcp = NULL;

	if (!node)
		return;
	mutex_lock(&node->conn_impl_list_lock);
	list_for_each_entry(conn_impl, &node->conn_impl_list, list) {
		if (conn_impl->type == CONNECT_TYPE_TCP &&
		    conn_impl->status == CONNECT_STAT_WAIT_RESPONSE) {
			tcp = conn_impl->connect_handle;
			hmdfs_info("fd %d to working", tcp->fd);
			conn_impl->status = CONNECT_STAT_WORKING;
		}
	}
	mutex_unlock(&node->conn_impl_list_lock);
	connection_handshake_notify(node, NOTIFY_HS_DONE);
	peer_online(node);
}

static int connection_check_version(__u8 version)
{
#ifdef CONFIG_HMDFS_1_0
	__u8 min_ver = INVALID_VERSION;
#else
	__u8 min_ver = USERSPACE_MAX_VER;
#endif

	if (version <= min_ver || version >= MAX_VERSION) {
		hmdfs_info("version err. version %u", version);
		return -1;
	}
	return 0;
}

void connection_handshake_recv_handler(struct connection *conn_impl, void *buf,
				       void *data, __u32 data_len)
{
	__u8 version;
	__u8 ops;
	__u8 status;
	int fd = ((struct tcp_handle *)(conn_impl->connect_handle))->fd;
	struct connection_msg_head *head = (struct connection_msg_head *)buf;
	int ret;

	version = head->version;
	conn_impl->node->version = version;
	if (connection_check_version(version) != 0)
		goto out;
	conn_impl->node->iid = le64_to_cpu(head->source);
	conn_impl->node->conn_operations = hmdfs_get_peer_operation(version);
	ops = head->operations;
	status = conn_impl->status;
	switch (ops) {
	case CONNECT_MESG_HANDSHAKE_REQUEST:
		hmdfs_info(
			"Recved handshake request: device_id = %llu, version = %d, head->len = %d, tcp->fd = %d",
			conn_impl->node->device_id, version, head->datasize, fd);
		connection_send_handshake(conn_impl,
					  CONNECT_MESG_HANDSHAKE_RESPONSE,
					  head->msg_id);
		if (conn_impl->node->version >= DFS_2_0) {
			conn_impl->status = CONNECT_STAT_WAIT_ACK;
			conn_impl->node->status = NODE_STAT_SHAKING;
		} else {
			conn_impl->status = CONNECT_STAT_WORKING;
		}
		break;
	case CONNECT_MESG_HANDSHAKE_RESPONSE:
		hmdfs_info(
			"Recved handshake response: device_id = %llu, cmd->status = %hhu, tcp->fd = %d",
			conn_impl->node->device_id, status, fd);
		if (status == CONNECT_STAT_WAIT_REQUEST) {
			// must be 10.1 device, no need to set ktls
			connection_to_working(conn_impl->node);
			goto out;
		}

		if (conn_impl->node->version >= DFS_2_0) {
			ret = hs_proc_msg_data(conn_impl, ops, data, data_len);
			if (ret)
				goto nego_err;
			connection_send_handshake(conn_impl,
						  CONNECT_MESG_HANDSHAKE_ACK,
						  head->msg_id);
			hmdfs_info("respon rcv handle,conn_impl->crypto=0x%0x",
				   conn_impl->crypto);
#ifdef CONFIG_HMDFS_CRYPTO
			ret = connection_handshake_init_tls(conn_impl, ops);
			if (ret) {
				hmdfs_err("init_tls_key fail, ops %u", ops);
				goto out;
			}
#endif
		}

		conn_impl->status = CONNECT_STAT_WORKING;
		peer_online(conn_impl->node);
		break;
	case CONNECT_MESG_HANDSHAKE_ACK:
		if (conn_impl->node->version >= DFS_2_0) {
			ret = hs_proc_msg_data(conn_impl, ops, data, data_len);
			if (ret)
				goto nego_err;
			hmdfs_info("ack rcv handle, conn_impl->crypto=0x%0x",
				   conn_impl->crypto);
#ifdef CONFIG_HMDFS_CRYPTO
			ret = connection_handshake_init_tls(conn_impl, ops);
			if (ret) {
				hmdfs_err("init_tls_key fail, ops %u", ops);
				goto out;
			}
#endif
			conn_impl->status = CONNECT_STAT_WORKING;
			peer_online(conn_impl->node);
			break;
		}
		/* fall-through */
	default:
		if (conn_impl->node->version < DFS_2_0 &&
		    (ops == HANDSHAKE_REQUEST || ops == HANDSHAKE_RESPONSE)) {
			hmdfs_info("recv data, goto working handler, ops %u",
				   ops);
			connection_working_recv_handler(conn_impl, buf,
							data, data_len);
		}
		return;
	}
out:
	kfree(data);
	return;
nego_err:
	conn_impl->status = CONNECT_STAT_NEGO_FAIL;
	connection_handshake_notify(conn_impl->node,
				    NOTIFY_OFFLINE);
	hmdfs_err("protocol negotiation failed, remote device_id = %llu, tcp->fd = %d",
		  conn_impl->node->device_id, fd);
	goto out;
}

#ifdef CONFIG_HMDFS_CRYPTO
static void update_tls_crypto_key(struct connection *conn,
				  struct hmdfs_head_cmd *head, void *data,
				  __u32 data_len)
{
	// rekey message handler
	struct connection_rekey_request *rekey_req = NULL;
	int ret = 0;

	if (hmdfs_message_verify(conn->node, head, data) < 0) {
		hmdfs_err("Rekey msg %d has been abandoned", head->msg_id);
		goto out_err;
	}

	hmdfs_info("recv REKEY request");
	set_crypto_info(conn, SET_CRYPTO_RECV);
	// update send key if requested
	rekey_req = data;
	if (le32_to_cpu(rekey_req->update_request) == UPDATE_REQUESTED) {
		ret = tcp_send_rekey_request(conn);
		if (ret == 0)
			set_crypto_info(conn, SET_CRYPTO_SEND);
	}
out_err:
	kfree(data);
}

static bool cmd_update_tls_crypto_key(struct connection *conn,
				      struct hmdfs_head_cmd *head)
{
	__u8 version = conn->node->version;
	struct tcp_handle *tcp = conn->connect_handle;

	if (version < DFS_2_0 || conn->type != CONNECT_TYPE_TCP || !tcp)
		return false;
	return head->operations.command == F_CONNECT_REKEY;
}
#endif

void connection_working_recv_handler(struct connection *conn_impl, void *buf,
				     void *data, __u32 data_len)
{
#ifdef CONFIG_HMDFS_CRYPTO
	if (cmd_update_tls_crypto_key(conn_impl, buf)) {
		update_tls_crypto_key(conn_impl, buf, data, data_len);
		return;
	}
#endif
	conn_impl->node->conn_operations->recvmsg(conn_impl->node, buf, data);
}

static void connection_release(struct kref *ref)
{
	struct tcp_handle *tcp = NULL;
	struct connection *conn = container_of(ref, struct connection, ref_cnt);

	hmdfs_info("connection release");
	memset(conn->master_key, 0, HMDFS_KEY_SIZE);
	memset(conn->send_key, 0, HMDFS_KEY_SIZE);
	memset(conn->recv_key, 0, HMDFS_KEY_SIZE);
	if (conn->close)
		conn->close(conn);
	tcp = conn->connect_handle;
	crypto_free_aead(conn->tfm);
	// need to check and test: fput(tcp->sock->file);
	if (tcp && tcp->sock) {
		hmdfs_info("connection release: fd = %d, refcount %ld", tcp->fd,
			   file_count(tcp->sock->file));
		sockfd_put(tcp->sock);
	}
	if (tcp && tcp->recv_cache)
		kmem_cache_destroy(tcp->recv_cache);

	if (!list_empty(&conn->list)) {
		mutex_lock(&conn->node->conn_impl_list_lock);
		list_del(&conn->list);
		mutex_unlock(&conn->node->conn_impl_list_lock);
		/*
		 * wakup hmdfs_disconnect_node to check
		 * conn_deleting_list if empty.
		 */
		wake_up_interruptible(&conn->node->deleting_list_wq);
	}

	kfree(tcp);
	kfree(conn);
}

static void hmdfs_peer_release(struct kref *ref)
{
	struct hmdfs_peer *peer = container_of(ref, struct hmdfs_peer, ref_cnt);
	struct mutex *lock = &peer->sbi->connections.node_lock;

	if (!list_empty(&peer->list))
		hmdfs_info("releasing a on-sbi peer: device_id %llu ",
			   peer->device_id);
	else
		hmdfs_info("releasing a redundant peer: device_id %llu ",
			   peer->device_id);

	cancel_delayed_work_sync(&peer->evt_dwork);
	list_del(&peer->list);
	idr_destroy(&peer->msg_idr);
	idr_destroy(&peer->file_id_idr);
	flush_workqueue(peer->req_handle_wq);
	flush_workqueue(peer->async_wq);
	flush_workqueue(peer->retry_wb_wq);
	destroy_workqueue(peer->dentry_wq);
	destroy_workqueue(peer->req_handle_wq);
	destroy_workqueue(peer->async_wq);
	destroy_workqueue(peer->retry_wb_wq);
	destroy_workqueue(peer->reget_conn_wq);
	kfree(peer);
	mutex_unlock(lock);
}

void connection_put(struct connection *conn)
{
	struct mutex *lock = &conn->ref_lock;

	kref_put_mutex(&conn->ref_cnt, connection_release, lock);
}

void peer_put(struct hmdfs_peer *peer)
{
	struct mutex *lock = &peer->sbi->connections.node_lock;

	kref_put_mutex(&peer->ref_cnt, hmdfs_peer_release, lock);
}

static void hmdfs_dump_deleting_list(struct hmdfs_peer *node)
{
	struct connection *con = NULL;
	struct tcp_handle *tcp = NULL;
	int count = 0;

	mutex_lock(&node->conn_impl_list_lock);
	list_for_each_entry(con, &node->conn_deleting_list, list) {
		tcp = con->connect_handle;
		hmdfs_info("deleting list %d:device_id %llu tcp_fd %d refcnt %d",
			   count, node->device_id, tcp ? tcp->fd : -1,
			   kref_read(&con->ref_cnt));
		count++;
	}
	mutex_unlock(&node->conn_impl_list_lock);
}

static bool hmdfs_conn_deleting_list_empty(struct hmdfs_peer *node)
{
	bool empty = false;

	mutex_lock(&node->conn_impl_list_lock);
	empty = list_empty(&node->conn_deleting_list);
	mutex_unlock(&node->conn_impl_list_lock);

	return empty;
}

void hmdfs_disconnect_node(struct hmdfs_peer *node)
{
	LIST_HEAD(local_conns);
	struct connection *conn_impl = NULL;
	struct connection *next = NULL;
	struct tcp_handle *tcp = NULL;

	if (unlikely(!node))
		return;

	hmdfs_node_inc_evt_seq(node);
	/* Refer to comments in hmdfs_is_node_offlined() */
	smp_mb__after_atomic();
	node->status = NODE_STAT_OFFLINE;
	hmdfs_info("Try to disconnect peer: device_id %llu", node->device_id);

	mutex_lock(&node->conn_impl_list_lock);
	if (!list_empty(&node->conn_impl_list))
		list_replace_init(&node->conn_impl_list, &local_conns);
	mutex_unlock(&node->conn_impl_list_lock);

	list_for_each_entry_safe(conn_impl, next, &local_conns, list) {
		tcp = conn_impl->connect_handle;
		if (tcp && tcp->sock) {
			kernel_sock_shutdown(tcp->sock, SHUT_RDWR);
			hmdfs_info("shudown sock: fd = %d, refcount %ld",
				   tcp->fd, file_count(tcp->sock->file));
		}
		if (tcp)
			tcp->fd = INVALID_SOCKET_FD;

		tcp_close_socket(tcp);
		list_del_init(&conn_impl->list);

		connection_put(conn_impl);
	}

	if (wait_event_interruptible_timeout(node->deleting_list_wq,
					hmdfs_conn_deleting_list_empty(node),
					HMDFS_WAIT_CONN_RELEASE) <= 0)
		hmdfs_dump_deleting_list(node);

	/* wait all request process end */
	spin_lock(&node->idr_lock);
	while (node->msg_idr_process) {
		spin_unlock(&node->idr_lock);
		usleep_range(HMDFS_WAIT_REQUST_END_MIN,
			     HMDFS_WAIT_REQUST_END_MAX);
		spin_lock(&node->idr_lock);
	}
	spin_unlock(&node->idr_lock);

	hmdfs_queue_raw_node_evt(node, RAW_NODE_EVT_OFF);
}

static void hmdfs_run_simple_evt_cb(struct hmdfs_peer *node, int evt)
{
	unsigned int seq = hmdfs_node_inc_evt_seq(node);

	mutex_lock(&node->seq_lock);
	hmdfs_node_call_evt_cb(node, evt, true, seq);
	mutex_unlock(&node->seq_lock);
}

static void hmdfs_del_peer(struct hmdfs_peer *node)
{
	/*
	 * No need for offline evt cb, because all files must
	 * have been flushed and closed, else the filesystem
	 * will be un-mountable.
	 */
	cancel_delayed_work_sync(&node->evt_dwork);

	hmdfs_run_simple_evt_cb(node, NODE_EVT_DEL);

	hmdfs_release_peer_sysfs(node);

	flush_workqueue(node->reget_conn_wq);
	peer_put(node);
}

void hmdfs_connections_stop(struct hmdfs_sb_info *sbi)
{
	struct hmdfs_peer *node = NULL;
	struct hmdfs_peer *con_tmp = NULL;

	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry_safe(node, con_tmp, &sbi->connections.node_list,
				  list) {
		mutex_unlock(&sbi->connections.node_lock);
		hmdfs_disconnect_node(node);
		hmdfs_del_peer(node);
		mutex_lock(&sbi->connections.node_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);
}

struct connection *get_conn_impl(struct hmdfs_peer *node, int connect_type)
{
	struct connection *conn_impl = NULL;

	if (!node)
		return NULL;
	mutex_lock(&node->conn_impl_list_lock);
	list_for_each_entry(conn_impl, &node->conn_impl_list, list) {
		if (conn_impl->type == connect_type &&
		    conn_impl->status == CONNECT_STAT_WORKING) {
			connection_get(conn_impl);
			mutex_unlock(&node->conn_impl_list_lock);
			return conn_impl;
		}
	}
	mutex_unlock(&node->conn_impl_list_lock);
	hmdfs_err_ratelimited("device %llu not find connection, type %d",
			      node->device_id, connect_type);
	return NULL;
}

void set_conn_sock_quickack(struct hmdfs_peer *node)
{
	struct connection *conn_impl = NULL;
	struct tcp_handle *tcp = NULL;
	int ret = 0;
	int option = 1;

	if (!node)
		return;
	mutex_lock(&node->conn_impl_list_lock);
	list_for_each_entry(conn_impl, &node->conn_impl_list, list) {
		if (conn_impl->type == CONNECT_TYPE_TCP &&
		    conn_impl->status == CONNECT_STAT_WORKING &&
		    conn_impl->connect_handle) {
			tcp = (struct tcp_handle *)(conn_impl->connect_handle);
			ret = kernel_setsockopt(tcp->sock, SOL_TCP, TCP_QUICKACK,
						(char *)&option, sizeof(option));
			if (ret)
				hmdfs_err("set socket quickack error %d", ret);
			}
	}
	mutex_unlock(&node->conn_impl_list_lock);
}

struct hmdfs_peer *hmdfs_lookup_from_devid(struct hmdfs_sb_info *sbi,
					   uint64_t device_id)
{
	struct hmdfs_peer *con = NULL;
	struct hmdfs_peer *lookup = NULL;

	if (!sbi)
		return NULL;
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(con, &sbi->connections.node_list, list) {
		if (con->status != NODE_STAT_ONLINE ||
		    con->device_id != device_id)
			continue;
		lookup = con;
		peer_get(lookup);
		break;
	}
	mutex_unlock(&sbi->connections.node_lock);
	return lookup;
}

struct hmdfs_peer *hmdfs_lookup_from_cid(struct hmdfs_sb_info *sbi,
					 uint8_t *cid)
{
	struct hmdfs_peer *con = NULL;
	struct hmdfs_peer *lookup = NULL;

	if (!sbi)
		return NULL;
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(con, &sbi->connections.node_list, list) {
		if (strncmp(con->cid, cid, HMDFS_CID_SIZE) != 0)
			continue;
		lookup = con;
		peer_get(lookup);
		break;
	}
	mutex_unlock(&sbi->connections.node_lock);
	return lookup;
}

static struct hmdfs_peer *lookup_peer_by_cid_unsafe(struct hmdfs_sb_info *sbi,
						    uint8_t *cid)
{
	struct hmdfs_peer *node = NULL;

	list_for_each_entry(node, &sbi->connections.node_list, list)
		if (!strncmp(node->cid, cid, HMDFS_CID_SIZE)) {
			peer_get(node);
			return node;
		}
	return NULL;
}

static struct hmdfs_peer *add_peer_unsafe(struct hmdfs_sb_info *sbi,
					  struct hmdfs_peer *peer2add)
{
	struct hmdfs_peer *peer;
	int err;

	peer = lookup_peer_by_cid_unsafe(sbi, peer2add->cid);
	if (peer)
		return peer;

	err = hmdfs_register_peer_sysfs(sbi, peer2add);
	if (err) {
		hmdfs_err("register peer %llu sysfs err %d",
			  peer2add->device_id, err);
		return ERR_PTR(err);
	}
	list_add_tail(&peer2add->list, &sbi->connections.node_list);
	peer_get(peer2add);
	hmdfs_run_simple_evt_cb(peer2add, NODE_EVT_ADD);
	return peer2add;
}

static struct hmdfs_peer *
alloc_peer(struct hmdfs_sb_info *sbi, uint8_t *cid, uint64_t iid,
	   const struct connection_operations *conn_operations)
{
	struct hmdfs_peer *node = kzalloc(sizeof(*node), GFP_KERNEL);

	if (!node)
		return NULL;

	node->device_id = (u32)atomic_inc_return(&sbi->connections.conn_seq);

	node->async_wq = alloc_workqueue("dfs_async%u_%llu", WQ_MEM_RECLAIM, 0,
					 sbi->seq, node->device_id);
	if (!node->async_wq) {
		hmdfs_err("Failed to alloc async wq");
		goto out_err;
	}
	node->req_handle_wq = alloc_workqueue("dfs_req%u_%llu",
					      WQ_UNBOUND | WQ_MEM_RECLAIM,
					      sbi->async_req_max_active,
					      sbi->seq, node->device_id);
	if (!node->req_handle_wq) {
		hmdfs_err("Failed to alloc req wq");
		goto out_err;
	}
	node->dentry_wq = alloc_workqueue("dfs_dentry%u_%llu",
					   WQ_UNBOUND | WQ_MEM_RECLAIM,
					   0, sbi->seq, node->device_id);
	if (!node->dentry_wq) {
		hmdfs_err("Failed to alloc dentry wq");
		goto out_err;
	}
	node->retry_wb_wq = alloc_workqueue("dfs_rwb%u_%llu",
					   WQ_UNBOUND | WQ_MEM_RECLAIM,
					   HMDFS_RETRY_WB_WQ_MAX_ACTIVE,
					   sbi->seq, node->device_id);
	if (!node->retry_wb_wq) {
		hmdfs_err("Failed to alloc retry writeback wq");
		goto out_err;
	}
	node->reget_conn_wq = alloc_workqueue("dfs_regetcon%u_%llu",
					      WQ_UNBOUND, 0,
					      sbi->seq, node->device_id);
	if (!node->reget_conn_wq) {
		hmdfs_err("Failed to alloc reget conn wq");
		goto out_err;
	}
	INIT_LIST_HEAD(&node->conn_impl_list);
	mutex_init(&node->conn_impl_list_lock);
	INIT_LIST_HEAD(&node->conn_deleting_list);
	init_waitqueue_head(&node->deleting_list_wq);
	idr_init(&node->msg_idr);
	spin_lock_init(&node->idr_lock);
	idr_init(&node->file_id_idr);
	spin_lock_init(&node->file_id_lock);
	sbi->local_info.iid = iid;
	INIT_LIST_HEAD(&node->list);
	kref_init(&node->ref_cnt);
	node->owner = sbi->seq;
	node->conn_operations = conn_operations;
	node->sbi = sbi;
	node->status = NODE_STAT_SHAKING;
	node->iid = 0;
	node->conn_time = jiffies;
	memcpy(node->cid, cid, HMDFS_CID_SIZE);
	atomic64_set(&node->sb_dirty_count, 0);
	node->fid_cookie = 0;
	atomic_set(&node->evt_seq, 0);
	mutex_init(&node->seq_lock);
	mutex_init(&node->offline_cb_lock);
	mutex_init(&node->evt_lock);
	node->pending_evt = RAW_NODE_EVT_NR;
	node->last_evt = RAW_NODE_EVT_NR;
	node->cur_evt[0] = RAW_NODE_EVT_NR;
	node->cur_evt[1] = RAW_NODE_EVT_NR;
	node->seq_wr_idx = (unsigned char)UINT_MAX;
	node->seq_rd_idx = node->seq_wr_idx;
	INIT_DELAYED_WORK(&node->evt_dwork, hmdfs_node_evt_work);
	node->msg_idr_process = 0;
	node->offline_start = false;
	spin_lock_init(&node->wr_opened_inode_lock);
	INIT_LIST_HEAD(&node->wr_opened_inode_list);
	spin_lock_init(&node->stashed_inode_lock);
	node->stashed_inode_nr = 0;
	atomic_set(&node->rebuild_inode_status_nr, 0);
	init_waitqueue_head(&node->rebuild_inode_status_wq);
	INIT_LIST_HEAD(&node->stashed_inode_list);
	node->need_rebuild_stash_list = false;

	return node;

out_err:
	if (node->async_wq) {
		destroy_workqueue(node->async_wq);
		node->async_wq = NULL;
	}
	if (node->req_handle_wq) {
		destroy_workqueue(node->req_handle_wq);
		node->req_handle_wq = NULL;
	}
	if (node->dentry_wq) {
		destroy_workqueue(node->dentry_wq);
		node->dentry_wq = NULL;
	}
	if (node->retry_wb_wq) {
		destroy_workqueue(node->retry_wb_wq);
		node->retry_wb_wq = NULL;
	}
	if (node->reget_conn_wq) {
		destroy_workqueue(node->reget_conn_wq);
		node->reget_conn_wq = NULL;
	}
	kfree(node);
	return NULL;
}

struct hmdfs_peer *hmdfs_get_peer(struct hmdfs_sb_info *sbi, uint8_t *cid,
				  uint64_t iid)
{
	struct hmdfs_peer *peer = NULL, *on_sbi_peer = NULL;
	const struct connection_operations *conn_opr_ptr = NULL;

	mutex_lock(&sbi->connections.node_lock);
	peer = lookup_peer_by_cid_unsafe(sbi, cid);
	mutex_unlock(&sbi->connections.node_lock);
	if (peer) {
		hmdfs_info("Got a existing peer: device_id = %llu",
			   peer->device_id);
		goto out;
	}

	conn_opr_ptr = hmdfs_get_peer_operation(DFS_2_0);
	if (unlikely(!conn_opr_ptr)) {
		hmdfs_info("Fatal! Cannot get peer operation");
		goto out;
	}
	peer = alloc_peer(sbi, cid, iid, conn_opr_ptr);
	if (unlikely(!peer)) {
		hmdfs_info("Failed to alloc a peer");
		goto out;
	}

	mutex_lock(&sbi->connections.node_lock);
	on_sbi_peer = add_peer_unsafe(sbi, peer);
	mutex_unlock(&sbi->connections.node_lock);
	if (IS_ERR(on_sbi_peer)) {
		peer_put(peer);
		peer = NULL;
		goto out;
	} else if (unlikely(on_sbi_peer != peer)) {
		hmdfs_info("Got a existing peer: device_id = %llu",
			   on_sbi_peer->device_id);
		peer_put(peer);
		peer = on_sbi_peer;
	} else {
		hmdfs_info("Got a newly allocated peer: device_id = %llu",
			   peer->device_id);
	}

out:
	return peer;
}

void connection_send_echo_sync(struct hmdfs_peer *con)
{
	size_t send_len = sizeof(struct connection_echo_param);
	int ret;
	struct connection_echo_param *response = NULL;
	struct connection_echo_param *release = kzalloc(send_len, GFP_KERNEL);
	struct hmdfs_send_command sm = {
		.data = release,
		.len = send_len,
	};

	hmdfs_init_cmd(&sm.operations, F_CONNECT_ECHO);
	if (unlikely(!release))
		return;

	release->iid = cpu_to_le64(con->sbi->local_info.iid);
	release->length = cpu_to_le64(1);

	hmdfs_info_ratelimited("send echo, peer device_id:%llu",
			       con->device_id);
	ret = hmdfs_sendmessage_request(con, &sm);
	kfree(release);
	if (ret == -ETIME || sm.out_len == 0 || !sm.out_buf)
		return;
	response = sm.out_buf;
	hmdfs_info_ratelimited(
		"recv echo response, remote device_id: %llu, len: %llu",
		con->device_id, response->length);
	kfree(response);
}

void connection_send_echo_async_cb(struct hmdfs_peer *peer,
				   const struct hmdfs_req *req,
				   const struct hmdfs_resp *resp)
{
	struct connection_echo_param *data = resp->out_buf;

	WARN_ON(!req->data);
	if (resp->ret_code || !resp->out_buf || !resp->out_len) {
		hmdfs_info_ratelimited(
			"Failed to recv async echo response, ret:%u",
			resp->ret_code);
		kfree(req->data);
		return;
	}
	hmdfs_info_ratelimited(
		"recv async echo response, p1. peer: %pK, buflen: %lu, ret: %u",
		peer, resp->out_len, resp->ret_code);
	hmdfs_info_ratelimited(
		"recv async echo response, p2. remote device_id: %llu, len: %llu",
		peer->device_id, data->length);
	kfree(req->data);
}

static void connection_send_echo_async(struct hmdfs_peer *con)
{
	int ret = 0;
	struct hmdfs_req req;
	struct connection_echo_param *data = NULL;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (unlikely(!data))
		return;
	data->iid = cpu_to_le64(con->sbi->local_info.iid);
	data->length = cpu_to_le64(1);

	req.data = data;
	req.data_len = sizeof(*data);
	req.timeout = TIMEOUT_CONFIG;
	hmdfs_init_cmd(&req.operations, F_CONNECT_ECHO);
	ret = hmdfs_send_async_request(con, &req);
	hmdfs_info_ratelimited("send async echo, peer device_id:%llu, ret:%d",
			       con->device_id, ret);
	if (unlikely(ret))
		kfree(data);
}

void connection_send_echo(struct hmdfs_peer *con)
{
	if (con->status != NODE_STAT_ONLINE) {
		hmdfs_info("send echo, peer device_id:%llu, OFFLINE",
			   con->device_id);
		return;
	}
	connection_send_echo_sync(con);
	connection_send_echo_async(con);
}

void connection_recv_echo(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			  void *data)
{
	int ret;
	struct connection_echo_param *echo_param = data;
	struct connection_echo_param *response;

	response = kzalloc(sizeof(*response), GFP_KERNEL);
	if (!response)
		return;

	response->iid = cpu_to_le64(con->sbi->local_info.iid);
	response->length = cpu_to_le64(1);
	hmdfs_info_ratelimited(
		"recv echo request, remote device_id: %llu, len: %llu",
		con->device_id, echo_param->length);
	ret = hmdfs_sendmessage_response(
		con, cmd, sizeof(struct connection_echo_param), response, 0);
	if (ret != 0)
		hmdfs_info("Handshake send response error: %d", ret);
	else
		con->iid = le64_to_cpu(echo_param->iid);

	kfree(response);
}

static void head_release(struct kref *kref)
{
	struct hmdfs_msg_idr_head *head;
	struct hmdfs_peer *con;

	head = (struct hmdfs_msg_idr_head *)container_of(kref,
			struct hmdfs_msg_idr_head, ref);
	con = head->peer;
	idr_remove(&con->msg_idr, head->msg_id);
	spin_unlock(&con->idr_lock);

	kfree(head);
}

void head_put(struct hmdfs_msg_idr_head *head)
{
	kref_put_lock(&head->ref, head_release, &head->peer->idr_lock);
}

struct hmdfs_msg_idr_head *hmdfs_find_msg_head(struct hmdfs_peer *peer, int id)
{
	struct hmdfs_msg_idr_head *head = NULL;

	spin_lock(&peer->idr_lock);
	head = idr_find(&peer->msg_idr, id);
	if (head)
		kref_get(&head->ref);
	spin_unlock(&peer->idr_lock);

	return head;
}

int hmdfs_alloc_msg_idr(struct hmdfs_peer *peer, enum MSG_IDR_TYPE type,
			void *ptr)
{
	int ret = -EAGAIN;
	struct hmdfs_msg_idr_head *head = ptr;
	int end = peer->version < DFS_2_0 ? (USHRT_MAX + 1) : 0;

	idr_preload(GFP_KERNEL);
	spin_lock(&peer->idr_lock);
	if (!peer->offline_start)
		ret = idr_alloc_cyclic(&peer->msg_idr, ptr,
				       1, end, GFP_NOWAIT);
	if (ret >= 0) {
		kref_init(&head->ref);
		head->msg_id = ret;
		head->type = type;
		head->peer = peer;
		peer->msg_idr_process++;
		ret = 0;
	}
	spin_unlock(&peer->idr_lock);
	idr_preload_end();

	return ret;
}
