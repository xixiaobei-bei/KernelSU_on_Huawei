/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transport.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: send and recv msg for hmdfs
 * Author: maojingjing1@huawei.com wangminmin4@huawei.com
 *	   liuxuesong3@huawei.com chenjinglong1@huawei.com
 * Create: 2020-03-26
 *
 */

#include "transport.h"

#include <linux/freezer.h>
#include <linux/highmem.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/file.h>
#include <linux/sched/mm.h>

#include "DFS_1_0/adapter_crypto.h"
#include "device_node.h"
#include "hmdfs_trace.h"
#include "socket_adapter.h"
#include "authority/authentication.h"

#ifdef CONFIG_HMDFS_CRYPTO
#include <net/tls.h>
#include "crypto.h"
#endif

typedef void (*connect_recv_handler)(struct connection *, void *, void *,
				     __u32);

static connect_recv_handler connect_recv_callback[CONNECT_STAT_COUNT] = {
	[CONNECT_STAT_WAIT_REQUEST] = connection_handshake_recv_handler,
	[CONNECT_STAT_WAIT_RESPONSE] = connection_handshake_recv_handler,
	[CONNECT_STAT_WORKING] = connection_working_recv_handler,
	[CONNECT_STAT_STOP] = NULL,
	[CONNECT_STAT_WAIT_ACK] = connection_handshake_recv_handler,
	[CONNECT_STAT_NEGO_FAIL] = NULL,
};

static int recvmsg_nofs(struct socket *sock, struct msghdr *msg,
			struct kvec *vec, size_t num, size_t size, int flags)
{
	unsigned int nofs_flags;
	int ret;

	/* enable NOFS for memory allocation */
	nofs_flags = memalloc_nofs_save();
	ret = kernel_recvmsg(sock, msg, vec, num, size, flags);
	memalloc_nofs_restore(nofs_flags);

	return ret;
}

static int sendmsg_nofs(struct socket *sock, struct msghdr *msg,
			struct kvec *vec, size_t num, size_t size)
{
	unsigned int nofs_flags;
	int ret;

	/* enable NOFS for memory allocation */
	nofs_flags = memalloc_nofs_save();
	ret = kernel_sendmsg(sock, msg, vec, num, size);
	memalloc_nofs_restore(nofs_flags);

	return ret;
}

static int tcp_set_recvtimeo(struct socket *sock, int timeout)
{
	long jiffies_left = timeout * msecs_to_jiffies(MSEC_PER_SEC);
	struct timeval tv;
	int rc;
	int option = 1;

	rc = kernel_setsockopt(sock, SOL_TCP, TCP_NODELAY, (char *)&option,
			       sizeof(option));
	if (rc)
		hmdfs_err("Can't set socket NODELAY, error %d", rc);

	jiffies_to_timeval(jiffies_left, &tv);
	rc = kernel_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
			       sizeof(tv));
	if (rc)
		hmdfs_err("Can't set socket recv timeout %ld.%06d: %d",
			  (long)tv.tv_sec, (int)tv.tv_usec, rc);
	return rc;
}

uint32_t hmdfs_tcpi_rtt(struct hmdfs_peer *con)
{
	uint32_t rtt_us = 0;
	struct connection *conn_impl = NULL;
	struct tcp_handle *tcp = NULL;

	conn_impl = get_conn_impl(con, CONNECT_TYPE_TCP);
	if (!conn_impl)
		return rtt_us;
	tcp = (struct tcp_handle *)(conn_impl->connect_handle);
	if (tcp->sock)
		rtt_us = tcp_sk(tcp->sock->sk)->srtt_us >> 3;
	connection_put(conn_impl);
	return rtt_us;
}

static int tcp_read_head_from_socket(struct socket *sock, void *buf,
				     unsigned int to_read)
{
	int rc = 0;
	struct msghdr hmdfs_msg;
	struct kvec iov;

	iov.iov_base = buf;
	iov.iov_len = to_read;
	memset(&hmdfs_msg, 0, sizeof(hmdfs_msg));
	hmdfs_msg.msg_flags = MSG_WAITALL;
	hmdfs_msg.msg_control = NULL;
	hmdfs_msg.msg_controllen = 0;
	rc = recvmsg_nofs(sock, &hmdfs_msg, &iov, 1, to_read,
			  hmdfs_msg.msg_flags);
	if (rc == -EAGAIN || rc == -ETIMEDOUT || rc == -EINTR ||
	    rc == -EBADMSG) {
		usleep_range(1000, 2000);
		return -EAGAIN;
	}
	// error occurred
	if (rc != to_read) {
		hmdfs_err("tcp recv error %d", rc);
		return -ESHUTDOWN;
	}
	return 0;
}

static int tcp_read_buffer_from_socket(struct socket *sock, void *buf,
				       unsigned int to_read)
{
	int read_cnt = 0;
	int retry_time = 0;
	int rc = 0;
	struct msghdr hmdfs_msg;
	struct kvec iov;

	do {
		iov.iov_base = (char *)buf + read_cnt;
		iov.iov_len = to_read - read_cnt;
		memset(&hmdfs_msg, 0, sizeof(hmdfs_msg));
		hmdfs_msg.msg_flags = MSG_WAITALL;
		hmdfs_msg.msg_control = NULL;
		hmdfs_msg.msg_controllen = 0;
		rc = recvmsg_nofs(sock, &hmdfs_msg, &iov, 1,
				  to_read - read_cnt, hmdfs_msg.msg_flags);
		if (rc == -EBADMSG) {
			usleep_range(1000, 2000);
			continue;
		}
		if (rc == -EAGAIN || rc == -ETIMEDOUT || rc == -EINTR) {
			retry_time++;
			hmdfs_info("read again %d", rc);
			usleep_range(1000, 2000);
			continue;
		}
		// error occurred
		if (rc <= 0) {
			hmdfs_err("tcp recv error %d", rc);
			return -ESHUTDOWN;
		}
		read_cnt += rc;
		if (read_cnt != to_read)
			hmdfs_info("read again %d/%d", read_cnt, to_read);
	} while (read_cnt < to_read && retry_time < MAX_RECV_RETRY_TIMES);
	if (read_cnt == to_read)
		return 0;
	return -ESHUTDOWN;
}

static int hmdfs_drop_readpage_buffer(struct socket *sock,
				      struct hmdfs_head_cmd *recv)
{
	unsigned int len;
	void *buf = NULL;
	int err;

	len = le32_to_cpu(recv->data_len) - sizeof(struct hmdfs_head_cmd);
	if (len > HMDFS_PAGE_SIZE || !len) {
		hmdfs_err("recv invalid readpage length %u", len);
		return -EINVAL;
	}

	/* Abort the connection if no memory */
	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ESHUTDOWN;

	err = tcp_read_buffer_from_socket(sock, buf, len);
	kfree(buf);

	return err;
}

static int hmdfs_get_readpage_buffer(struct socket *sock,
				     struct hmdfs_head_cmd *recv,
				     struct page *page)
{
	char *page_buf = NULL;
	unsigned int out_len;
	int err;

	out_len = le32_to_cpu(recv->data_len) - sizeof(struct hmdfs_head_cmd);
	if (out_len > HMDFS_PAGE_SIZE || !out_len) {
		hmdfs_err("recv invalid readpage length %u", out_len);
		return -EINVAL;
	}

	page_buf = kmap(page);
	err = tcp_read_buffer_from_socket(sock, page_buf, out_len);
	if (err)
		goto out_unmap;
	if (out_len != HMDFS_PAGE_SIZE)
		memset(page_buf + out_len, 0, HMDFS_PAGE_SIZE - out_len);

out_unmap:
	kunmap(page);
	return err;
}

static int tcp_recvpage_tls(struct connection *connect,
			    struct hmdfs_head_cmd *recv)
{
	int ret = 0;
	struct tcp_handle *tcp = NULL;
	struct hmdfs_peer *node = NULL;
	struct page *page = NULL;
	struct hmdfs_async_work *async_work = NULL;
	int rd_err;

	if (!connect) {
		hmdfs_err("tcp connect == NULL");
		return -ESHUTDOWN;
	}
	node = connect->node;
	tcp = (struct tcp_handle *)(connect->connect_handle);

	rd_err = le32_to_cpu(recv->ret_code);
	if (rd_err)
		hmdfs_warning("tcp: readpage from peer %llu ret err %d",
			      node->device_id, rd_err);

	async_work = (struct hmdfs_async_work *)hmdfs_find_msg_head(node,
						le32_to_cpu(recv->msg_id));
	if (!async_work || !cancel_delayed_work(&async_work->d_work))
		goto out;

	page = async_work->page;
	if (!page) {
		hmdfs_err("page not found");
		goto out;
	}

	if (!rd_err) {
		ret = hmdfs_get_readpage_buffer(tcp->sock, recv, page);
		if (ret)
			rd_err = ret;
	}
	node->conn_operations->recvpage(node, recv, rd_err, async_work);
	asw_put(async_work);
	return ret;

out:
	/* async_work will be released by recvpage in normal processure */
	if (async_work)
		asw_put(async_work);
	hmdfs_err_ratelimited("timeout and droppage");
	hmdfs_client_resp_statis(node->sbi, F_READPAGE, HMDFS_RESP_DELAY, 0, 0);
	if (!rd_err)
		ret = hmdfs_drop_readpage_buffer(tcp->sock, recv);
	return ret;
}

static int tcp_recvbuffer_cipher(struct connection *connect,
				 struct hmdfs_head_cmd *recv)
{
	int ret = 0;
	struct tcp_handle *tcp = NULL;
	size_t cipherbuffer_len;
	__u8 *cipherbuffer = NULL;
	size_t outlen = 0;
	__u8 *outdata = NULL;
	__u32 recv_len = le32_to_cpu(recv->data_len);

	tcp = (struct tcp_handle *)(connect->connect_handle);
	if (recv_len == sizeof(struct hmdfs_head_cmd))
		goto out_recv_head;
	else if (recv_len > sizeof(struct hmdfs_head_cmd) &&
	    recv_len <= ADAPTER_MESSAGE_LENGTH)
		cipherbuffer_len = recv_len - sizeof(struct hmdfs_head_cmd) +
				   HMDFS_IV_SIZE + HMDFS_TAG_SIZE;
	else
		return -ENOMSG;
	cipherbuffer = kzalloc(cipherbuffer_len, GFP_KERNEL);
	if (!cipherbuffer) {
		hmdfs_err("zalloc cipherbuffer error");
		return -ESHUTDOWN;
	}
	outlen = cipherbuffer_len - HMDFS_IV_SIZE - HMDFS_TAG_SIZE;
	outdata = kzalloc(outlen, GFP_KERNEL);
	if (!outdata) {
		hmdfs_err("encrypt zalloc outdata error");
		kfree(cipherbuffer);
		return -ESHUTDOWN;
	}

	ret = tcp_read_buffer_from_socket(tcp->sock, cipherbuffer,
					  cipherbuffer_len);
	if (ret)
		goto out_recv;
	ret = aeadcipher_decrypt_buffer(connect, cipherbuffer, cipherbuffer_len,
					outdata, outlen);
	if (ret) {
		hmdfs_err("decrypt_buf fail");
		goto out_recv;
	}
out_recv_head:
	if (connect_recv_callback[connect->status]) {
		connect_recv_callback[connect->status](connect, recv, outdata,
						       outlen);
	} else {
		kfree(outdata);
		hmdfs_err("encypt callback NULL status %d", connect->status);
	}
	kfree(cipherbuffer);
	return ret;
out_recv:
	kfree(cipherbuffer);
	kfree(outdata);
	return ret;
}

static int tcp_recvbuffer_tls(struct connection *connect,
			      struct hmdfs_head_cmd *recv)
{
	int ret = 0;
	struct tcp_handle *tcp = NULL;
	size_t outlen;
	__u8 *outdata = NULL;
	__u32 recv_len = le32_to_cpu(recv->data_len);

	tcp = (struct tcp_handle *)(connect->connect_handle);
	outlen = recv_len - sizeof(struct hmdfs_head_cmd);
	if (outlen == 0)
		goto out_recv_head;

	/*
	 * NOTE: Up to half of the allocated memory may be wasted due to
	 * the Internal Fragmentation, however the memory allocation times
	 * can be reduced and we don't have to adjust existing message
	 * transporting mechanism
	 */
	outdata = kmalloc(outlen, GFP_KERNEL);
	if (!outdata)
		return -ESHUTDOWN;

	ret = tcp_read_buffer_from_socket(tcp->sock, outdata, outlen);
	if (ret) {
		kfree(outdata);
		return ret;
	}
	tcp->connect->stat.recv_bytes += outlen;
out_recv_head:
	if (connect_recv_callback[connect->status]) {
		connect_recv_callback[connect->status](connect, recv, outdata,
						       outlen);
	} else {
		kfree(outdata);
		hmdfs_err("callback NULL status %d", connect->status);
	}
	return 0;
}

static int tcp_receive_from_sock(struct tcp_handle *tcp)
{
	struct hmdfs_head_cmd *recv = NULL;
	int ret = 0;

	if (!tcp) {
		hmdfs_info("tcp recv thread !tcp");
		return -ESHUTDOWN;
	}

	if (!tcp->sock) {
		hmdfs_info("tcp recv thread !sock");
		return -ESHUTDOWN;
	}

	recv = kmem_cache_alloc(tcp->recv_cache, GFP_KERNEL);
	if (!recv) {
		hmdfs_info("tcp recv thread !cache");
		return -ESHUTDOWN;
	}

	ret = tcp_read_head_from_socket(tcp->sock, recv,
					sizeof(struct hmdfs_head_cmd));
	if (ret)
		goto out;

	tcp->connect->stat.recv_bytes += sizeof(struct hmdfs_head_cmd);
	tcp->connect->stat.recv_message_count++;

	if (recv->magic != HMDFS_MSG_MAGIC) {
		hmdfs_info_ratelimited("tcp recv fd %d wrong magic. drop message",
				       tcp->fd);
		goto out;
	}

	if ((le32_to_cpu(recv->data_len) >
	    HMDFS_MAX_MESSAGE_LEN + sizeof(struct hmdfs_head_cmd)) ||
	    (le32_to_cpu(recv->data_len) < sizeof(struct hmdfs_head_cmd))) {
		hmdfs_info("tcp recv fd %d length error. drop message",
			   tcp->fd);
		goto out;
	}

	if (recv->version > USERSPACE_MAX_VER &&
	    tcp->connect->status == CONNECT_STAT_WORKING &&
	    recv->operations.command == F_READPAGE &&
	    recv->operations.cmd_flag == C_RESPONSE) {
		ret = tcp_recvpage_tls(tcp->connect, recv);
		goto out;
	}

	if (tcp->connect->status == CONNECT_STAT_WORKING &&
	    recv->version > USERSPACE_MAX_VER)
		ret = tcp_recvbuffer_tls(tcp->connect, recv);
	else
		ret = tcp_recvbuffer_cipher(tcp->connect, recv);

out:
	kmem_cache_free(tcp->recv_cache, recv);
	return ret;
}

static bool tcp_handle_is_available(struct tcp_handle *tcp)
{
#ifdef CONFIG_HMDFS_CRYPTO
	struct tls_context *tls_ctx = NULL;
	struct tls_sw_context_rx *ctx = NULL;

#endif
	if (!tcp || !tcp->sock || !tcp->sock->sk) {
		hmdfs_err("Invalid tcp connection");
		return false;
	}

	if (tcp->sock->sk->sk_state != TCP_ESTABLISHED) {
		hmdfs_err("TCP conn %d is broken, current sk_state is %d",
			  tcp->fd, tcp->sock->sk->sk_state);
		return false;
	}

	if (tcp->sock->state != SS_CONNECTING &&
	    tcp->sock->state != SS_CONNECTED) {
		hmdfs_err("TCP conn %d is broken, current sock state is %d",
			  tcp->fd, tcp->sock->state);
		return false;
	}

#ifdef CONFIG_HMDFS_CRYPTO
	tls_ctx = tls_get_ctx(tcp->sock->sk);
	if (tls_ctx) {
		ctx = tls_sw_ctx_rx(tls_ctx);
		if (ctx && ctx->strp.stopped) {
			hmdfs_err(
				"TCP conn %d is broken, the strparser has stopped",
				tcp->fd);
			return false;
		}
	}
#endif
	return true;
}

static int tcp_recv_thread(void *arg)
{
	int ret = 0;
	struct tcp_handle *tcp = (struct tcp_handle *)arg;
	const struct cred *old_cred;

	WARN_ON(!tcp);
	WARN_ON(!tcp->sock);
	set_freezable();

	old_cred = hmdfs_override_creds(tcp->connect->node->sbi->system_cred);

	while (!kthread_should_stop()) {
		/*
		 * 1. In case the redundant connection has not been mounted on
		 *    a peer
		 * 2. Lock is unnecessary since a transient state is acceptable
		 */
		if (tcp_handle_is_available(tcp) &&
		    list_empty(&tcp->connect->list))
			goto freeze;
		if (!mutex_trylock(&tcp->close_mutex))
			continue;
		if (tcp_handle_is_available(tcp))
			ret = tcp_receive_from_sock(tcp);
		else
			ret = -ESHUTDOWN;
		/*
		 * This kthread will exit if ret is -ESHUTDOWN, thus we need to
		 * set recv_task to NULL to avoid calling kthread_stop() from
		 * tcp_close_socket().
		 */
		if (ret == -ESHUTDOWN)
			tcp->recv_task = NULL;
		mutex_unlock(&tcp->close_mutex);
		if (ret == -ESHUTDOWN) {
			hmdfs_node_inc_evt_seq(tcp->connect->node);
			tcp->connect->status = CONNECT_STAT_STOP;
			if (tcp->connect->node->status != NODE_STAT_OFFLINE)
				hmdfs_reget_connection(tcp->connect);
			break;
		}
freeze:
		schedule();
		try_to_freeze();
	}

	hmdfs_info("Exiting. Now, sock state = %d", tcp->sock->state);
	hmdfs_revert_creds(old_cred);
	connection_put(tcp->connect);
	return 0;
}

static int tcp_send_message_sock_cipher(struct tcp_handle *tcp,
					struct hmdfs_send_data *msg)
{
	int ret = 0;
	__u8 *outdata = NULL;
	size_t outlen = 0;
	int send_len = 0;
	int send_vec_cnt = 0;
	struct msghdr tcp_msg;
	struct kvec iov[TCP_KVEC_ELE_DOUBLE];

	memset(&tcp_msg, 0, sizeof(tcp_msg));
	if (!tcp || !tcp->sock) {
		hmdfs_err("encrypt tcp socket = NULL");
		return -ESHUTDOWN;
	}
	iov[0].iov_base = msg->head;
	iov[0].iov_len = msg->head_len;
	send_vec_cnt = TCP_KVEC_HEAD;
	if (msg->len == 0)
		goto send;

	outlen = msg->len + HMDFS_IV_SIZE + HMDFS_TAG_SIZE;
	outdata = kzalloc(outlen, GFP_KERNEL);
	if (!outdata) {
		hmdfs_err("tcp send message encrypt fail to alloc outdata");
		return -ENOMEM;
	}
	ret = aeadcipher_encrypt_buffer(tcp->connect, msg->data, msg->len,
					outdata, outlen);
	if (ret) {
		hmdfs_err("encrypt_buf fail");
		goto out;
	}
	iov[1].iov_base = outdata;
	iov[1].iov_len = outlen;
	send_vec_cnt = TCP_KVEC_ELE_DOUBLE;
send:
	mutex_lock(&tcp->send_mutex);
	send_len = sendmsg_nofs(tcp->sock, &tcp_msg, iov, send_vec_cnt,
				msg->head_len + outlen);
	mutex_unlock(&tcp->send_mutex);
	if (send_len <= 0) {
		hmdfs_err("error %d", send_len);
		ret = -ESHUTDOWN;
	} else if (send_len != msg->head_len + outlen) {
		hmdfs_err("send part of message. %d/%ld", send_len,
			  msg->head_len + outlen);
		ret = -EAGAIN;
	} else {
		ret = 0;
	}
out:
	kfree(outdata);
	return ret;
}

static int tcp_send_message_sock_tls(struct tcp_handle *tcp,
				     struct hmdfs_send_data *msg)
{
	int send_len = 0;
	int send_vec_cnt = 0;
	struct msghdr tcp_msg;
	struct kvec iov[TCP_KVEC_ELE_TRIPLE];

	memset(&tcp_msg, 0, sizeof(tcp_msg));
	if (!tcp || !tcp->sock) {
		hmdfs_err("tcp socket = NULL");
		return -ESHUTDOWN;
	}
	iov[TCP_KVEC_HEAD].iov_base = msg->head;
	iov[TCP_KVEC_HEAD].iov_len = msg->head_len;
	if (msg->len == 0 && msg->sdesc_len == 0) {
		send_vec_cnt = TCP_KVEC_ELE_SINGLE;
	} else if (msg->sdesc_len == 0) {
		iov[TCP_KVEC_DATA].iov_base = msg->data;
		iov[TCP_KVEC_DATA].iov_len = msg->len;
		send_vec_cnt = TCP_KVEC_ELE_DOUBLE;
	} else {
		iov[TCP_KVEC_FILE_PARA].iov_base = msg->sdesc;
		iov[TCP_KVEC_FILE_PARA].iov_len = msg->sdesc_len;
		iov[TCP_KVEC_FILE_CONTENT].iov_base = msg->data;
		iov[TCP_KVEC_FILE_CONTENT].iov_len = msg->len;
		send_vec_cnt = TCP_KVEC_ELE_TRIPLE;
	}
	mutex_lock(&tcp->send_mutex);
	send_len = sendmsg_nofs(tcp->sock, &tcp_msg, iov, send_vec_cnt,
				msg->head_len + msg->len + msg->sdesc_len);
	mutex_unlock(&tcp->send_mutex);
	if (send_len == -EBADMSG) {
		return -EBADMSG;
	} else if (send_len <= 0) {
		hmdfs_err("error %d", send_len);
		return -ESHUTDOWN;
	} else if (send_len != msg->head_len + msg->len + msg->sdesc_len) {
		hmdfs_err("send part of message. %d/%ld", send_len,
			  msg->head_len + msg->len);
		tcp->connect->stat.send_bytes += send_len;
		return -EAGAIN;
	}
	tcp->connect->stat.send_bytes += send_len;
	tcp->connect->stat.send_message_count++;
	return 0;
}

#ifdef CONFIG_HMDFS_CRYPTO
int tcp_send_rekey_request(struct connection *connect)
{
	int ret = 0;
	struct hmdfs_send_data msg;
	struct tcp_handle *tcp = connect->connect_handle;
	struct hmdfs_head_cmd *head = NULL;
	struct connection_rekey_request *rekey_request_param = NULL;
	struct hmdfs_cmd operations;

	hmdfs_init_cmd(&operations, F_CONNECT_REKEY);
	head = kzalloc(sizeof(struct hmdfs_head_cmd) +
			       sizeof(struct connection_rekey_request),
		       GFP_KERNEL);
	if (!head)
		return -ENOMEM;
	rekey_request_param =
		(struct connection_rekey_request
			 *)((uint8_t *)head + sizeof(struct hmdfs_head_cmd));

	rekey_request_param->update_request = cpu_to_le32(UPDATE_NOT_REQUESTED);

	head->magic = HMDFS_MSG_MAGIC;
	head->version = DFS_2_0;
	head->operations = operations;
	head->data_len =
		cpu_to_le32(sizeof(*head) + sizeof(*rekey_request_param));
	head->reserved = 0;
	head->reserved1 = 0;
	head->ret_code = 0;

	msg.head = head;
	msg.head_len = sizeof(*head);
	msg.data = rekey_request_param;
	msg.len = sizeof(*rekey_request_param);
	msg.sdesc = NULL;
	msg.sdesc_len = 0;
	ret = tcp_send_message_sock_tls(tcp, &msg);
	if (ret != 0)
		hmdfs_err("return error %d", ret);
	kfree(head);
	return ret;
}
#endif

static int tcp_send_message(struct connection *connect,
			    struct hmdfs_send_data *msg)
{
	int ret = 0;
#ifdef CONFIG_HMDFS_CRYPTO
	unsigned long nowtime = jiffies;
#endif
	struct tcp_handle *tcp = NULL;

	if (!connect) {
		hmdfs_err("tcp connection = NULL ");
		return -ESHUTDOWN;
	}
	if (!msg) {
		hmdfs_err("msg = NULL");
		return -EINVAL;
	}
	if (msg->len > HMDFS_MAX_MESSAGE_LEN) {
		hmdfs_err("message->len error: %ld", msg->len);
		return -EINVAL;
	}
	tcp = (struct tcp_handle *)(connect->connect_handle);
	if (connect->status == CONNECT_STAT_STOP)
		return -EAGAIN;

	if (connect->node->version > USERSPACE_MAX_VER)
		trace_hmdfs_tcp_send_message_2_0(msg->head);
	else
		trace_hmdfs_tcp_send_message_1_0(msg->head);

	if (connect->status == CONNECT_STAT_WORKING &&
	    connect->node->version > USERSPACE_MAX_VER)
		ret = tcp_send_message_sock_tls(tcp, msg);
	else
		// Handshake status or version HMDFS1.0
		ret = tcp_send_message_sock_cipher(tcp, msg);

	if (ret != 0) {
		hmdfs_err("return error %d", ret);
		return ret;
	}
#ifdef CONFIG_HMDFS_CRYPTO
	if (nowtime - connect->stat.rekey_time >= REKEY_LIFETIME &&
	    connect->status == CONNECT_STAT_WORKING &&
	    connect->node->version >= DFS_2_0) {
		hmdfs_info("send rekey message to devid %llu",
			   connect->node->device_id);
		ret = tcp_send_rekey_request(connect);
		if (ret == 0)
			set_crypto_info(connect, SET_CRYPTO_SEND);
		connect->stat.rekey_time = nowtime;
	}
#endif
	return ret;
}

void tcp_close_socket(struct tcp_handle *tcp)
{
	if (!tcp)
		return;
	mutex_lock(&tcp->close_mutex);
	if (tcp->recv_task) {
		kthread_stop(tcp->recv_task);
		tcp->recv_task = NULL;
	}
	mutex_unlock(&tcp->close_mutex);
}

static int set_tfm(__u8 *master_key, struct crypto_aead *tfm)
{
	int ret = 0;
	int iv_len;
	__u8 *sec_key = NULL;

	sec_key = master_key;
	crypto_aead_clear_flags(tfm, ~0);
	ret = crypto_aead_setkey(tfm, sec_key, HMDFS_KEY_SIZE);
	if (ret) {
		hmdfs_err("failed to set the key");
		goto out;
	}
	ret = crypto_aead_setauthsize(tfm, HMDFS_TAG_SIZE);
	if (ret) {
		hmdfs_err("authsize length is error");
		goto out;
	}

	iv_len = crypto_aead_ivsize(tfm);
	if (iv_len != HMDFS_IV_SIZE) {
		hmdfs_err("IV recommended value should be set %d", iv_len);
		ret = -ENODATA;
	}
out:
	return ret;
}

static int tcp_update_socket(struct tcp_handle *tcp, int fd,
			     uint8_t *master_key, struct socket *socket)
{
	int err = 0;
	struct hmdfs_peer *node = NULL;

	if (!master_key || fd == 0)
		return -EAGAIN;

	tcp->sock = socket;
	tcp->fd = fd;
	if (!tcp_handle_is_available(tcp)) {
		err = -EPIPE;
		goto put_sock;
	}

	hmdfs_info("socket fd %d, state %d, refcount %ld",
		   fd, socket->state, file_count(socket->file));

	tcp->recv_cache = kmem_cache_create("hmdfs_socket",
					    tcp->recvbuf_maxsize,
					    0, SLAB_HWCACHE_ALIGN, NULL);
	if (!tcp->recv_cache) {
		err = -ENOMEM;
		goto put_sock;
	}

	socket->sk->sk_user_data = tcp;
	err = tcp_set_recvtimeo(socket, TCP_RECV_TIMEOUT);
	if (err) {
		hmdfs_err("tcp set timeout error");
		goto free_mem_cache;
	}

	/* send key and recv key, default MASTER KEY */
	memcpy(tcp->connect->master_key, master_key, HMDFS_KEY_SIZE);
	memcpy(tcp->connect->send_key, master_key, HMDFS_KEY_SIZE);
	memcpy(tcp->connect->recv_key, master_key, HMDFS_KEY_SIZE);
	tcp->connect->tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tcp->connect->tfm)) {
		err = PTR_ERR(tcp->connect->tfm);
		tcp->connect->tfm = NULL;
		hmdfs_err("failed to load transform for gcm(aes):%d", err);
		goto free_mem_cache;
	}

	err = set_tfm(master_key, tcp->connect->tfm);
	if (err) {
		hmdfs_err("tfm seting exit fault");
		goto free_crypto;
	}

	connection_get(tcp->connect);

	node = tcp->connect->node;
	tcp->recv_task = kthread_create(tcp_recv_thread, (void *)tcp,
					"dfs_rcv%u_%llu_%d",
					node->owner, node->device_id, fd);
	if (IS_ERR(tcp->recv_task)) {
		err = PTR_ERR(tcp->recv_task);
		hmdfs_err("tcp->rcev_task %d", err);
		goto put_conn;
	}

	return 0;

put_conn:
	tcp->recv_task = NULL;
	connection_put(tcp->connect);
free_crypto:
	crypto_free_aead(tcp->connect->tfm);
	tcp->connect->tfm = NULL;
free_mem_cache:
	kmem_cache_destroy(tcp->recv_cache);
	tcp->recv_cache = NULL;
put_sock:
	tcp->sock = NULL;
	tcp->fd = 0;

	return err;
}

static struct tcp_handle *tcp_alloc_handle(struct connection *connect,
					   int socket_fd, uint8_t *master_key, struct socket *socket)
{
	int ret = 0;
	struct tcp_handle *tcp = kzalloc(sizeof(*tcp), GFP_KERNEL);

	if (!tcp)
		return NULL;
	tcp->connect = connect;
	tcp->connect->connect_handle = (void *)tcp;
	tcp->recvbuf_maxsize = MAX_RECV_SIZE;
	tcp->recv_task = NULL;
	tcp->recv_cache = NULL;
	tcp->sock = NULL;
	mutex_init(&tcp->close_mutex);
	mutex_init(&tcp->send_mutex);
	ret = tcp_update_socket(tcp, socket_fd, master_key, socket);
	if (ret) {
		kfree(tcp);
		return NULL;
	}
	return tcp;
}

void hmdfs_get_connection(struct hmdfs_peer *peer)
{
	struct notify_param param;

	if (!peer)
		return;
	param.notify = NOTIFY_GET_SESSION;
	param.remote_iid = peer->iid;
	param.fd = INVALID_SOCKET_FD;
	memcpy(param.remote_cid, peer->cid, HMDFS_CID_SIZE);
	notify(peer, &param);
}

static void connection_notify_to_close(struct connection *conn)
{
	struct notify_param param;
	struct hmdfs_peer *peer = NULL;
	struct tcp_handle *tcp = NULL;

	tcp = conn->connect_handle;
	peer = conn->node;

	// libdistbus/src/TcpSession.cpp will close the socket
	param.notify = NOTIFY_GET_SESSION;
	param.remote_iid = peer->iid;
	param.fd = tcp->fd;
	memcpy(param.remote_cid, peer->cid, HMDFS_CID_SIZE);
	notify(peer, &param);
}

void hmdfs_reget_connection(struct connection *conn)
{
	struct tcp_handle *tcp = NULL;
	struct connection *conn_impl = NULL;
	struct connection *next = NULL;
	struct task_struct *recv_task = NULL;
	bool should_put = false;
	bool stop_thread = true;

	if (!conn)
		return;

	// One may put a connection if and only if he took it out of the list
	mutex_lock(&conn->node->conn_impl_list_lock);
	list_for_each_entry_safe(conn_impl, next, &conn->node->conn_impl_list,
				  list) {
		if (conn_impl == conn) {
			should_put = true;
			list_move(&conn->list, &conn->node->conn_deleting_list);
			break;
		}
	}
	if (!should_put) {
		mutex_unlock(&conn->node->conn_impl_list_lock);
		return;
	}

	tcp = conn->connect_handle;
	if (tcp) {
		recv_task = tcp->recv_task;
		/*
		 * To avoid the receive thread to stop itself. Ensure receive
		 * thread stop before process offline event
		 */
		if (!recv_task ||
		    (recv_task && (recv_task->pid == current->pid)))
			stop_thread = false;
	}
	mutex_unlock(&conn->node->conn_impl_list_lock);

	if (tcp) {
		if (tcp->sock) {
			hmdfs_info("shudown sock: fd = %d, sockref = %ld, connref = %u stop_thread = %d",
				   tcp->fd, file_count(tcp->sock->file),
				   kref_read(&conn->ref_cnt), stop_thread);
			kernel_sock_shutdown(tcp->sock, SHUT_RDWR);
		}

		if (stop_thread)
			tcp_close_socket(tcp);

		if (tcp->fd != INVALID_SOCKET_FD)
			connection_notify_to_close(conn);
	}
	connection_put(conn);
}

static struct connection *
lookup_conn_by_socketfd_unsafe(struct hmdfs_peer *node, struct socket *socket)
{
	struct connection *tcp_conn = NULL;
	struct tcp_handle *tcp = NULL;

	list_for_each_entry(tcp_conn, &node->conn_impl_list, list) {
		if (tcp_conn->connect_handle) {
			tcp = (struct tcp_handle *)(tcp_conn->connect_handle);
			if (tcp->sock == socket) {
				connection_get(tcp_conn);
				return tcp_conn;
			}
		}
	}
	return NULL;
}

static void hmdfs_reget_connection_work_fn(struct work_struct *work)
{
	struct connection *conn =
		container_of(work, struct connection, reget_work);

	hmdfs_reget_connection(conn);
	connection_put(conn);
}

struct connection *alloc_conn_tcp(struct hmdfs_peer *node, int socket_fd,
				  uint8_t *master_key, uint8_t status, struct socket *socket)
{
	struct connection *tcp_conn = NULL;
	unsigned long nowtime = jiffies;

	tcp_conn = kzalloc(sizeof(*tcp_conn), GFP_KERNEL);
	if (!tcp_conn)
		goto out_err;

	kref_init(&tcp_conn->ref_cnt);
	mutex_init(&tcp_conn->ref_lock);
	INIT_LIST_HEAD(&tcp_conn->list);
	tcp_conn->node = node;
	tcp_conn->close = tcp_stop_connect;
	tcp_conn->send_message = tcp_send_message;
	tcp_conn->type = CONNECT_TYPE_TCP;
	tcp_conn->status = status;
	tcp_conn->stat.rekey_time = nowtime;
	tcp_conn->connect_handle =
		(void *)tcp_alloc_handle(tcp_conn, socket_fd, master_key, socket);
	INIT_WORK(&tcp_conn->reget_work, hmdfs_reget_connection_work_fn);
	if (!tcp_conn->connect_handle) {
		hmdfs_err("Failed to alloc tcp_handle for strcut conn");
		goto out_err;
	}
	return tcp_conn;

out_err:
	kfree(tcp_conn);
	return NULL;
}

static struct connection *add_conn_tcp_unsafe(struct hmdfs_peer *node,
					      struct socket *socket,
					      struct connection *conn2add)
{
	struct connection *conn;

	conn = lookup_conn_by_socketfd_unsafe(node, socket);
	if (conn) {
		hmdfs_info("socket already in list");
		return conn;
	}

	/* Prefer to use socket opened by local device */
	if (conn2add->status == CONNECT_STAT_WAIT_REQUEST)
		list_add(&conn2add->list, &node->conn_impl_list);
	else
		list_add_tail(&conn2add->list, &node->conn_impl_list);
	connection_get(conn2add);
	return conn2add;
}

struct connection *hmdfs_get_conn_tcp(struct hmdfs_peer *node, int fd,
				      uint8_t *master_key, uint8_t status)
{
	struct connection *tcp_conn = NULL, *on_peer_conn = NULL;
	struct tcp_handle *tcp = NULL;
	struct socket *socket = NULL;
	int err = 0;

	socket = sockfd_lookup(fd, &err);
	if (!socket) {
		hmdfs_err("lookup socket fail, socket_fd %d, err %d", fd, err);
		return NULL;
	}
	mutex_lock(&node->conn_impl_list_lock);
	tcp_conn = lookup_conn_by_socketfd_unsafe(node, socket);
	mutex_unlock(&node->conn_impl_list_lock);
	if (tcp_conn) {
		hmdfs_info("Got a existing tcp conn: fsocket_fd = %d",
			   fd);
		sockfd_put(socket);
		goto out;
	}

	tcp_conn = alloc_conn_tcp(node, fd, master_key, status, socket);
	if (!tcp_conn) {
		hmdfs_info("Failed to alloc a tcp conn, socket_fd %d", fd);
		sockfd_put(socket);
		goto out;
	}

	mutex_lock(&node->conn_impl_list_lock);
	on_peer_conn = add_conn_tcp_unsafe(node, socket, tcp_conn);
	mutex_unlock(&node->conn_impl_list_lock);
	tcp = tcp_conn->connect_handle;
	if (on_peer_conn == tcp_conn) {
		hmdfs_info("Got a newly allocated tcp conn: socket_fd = %d", fd);
		wake_up_process(tcp->recv_task);
		if (status == CONNECT_STAT_WAIT_RESPONSE)
			connection_send_handshake(
				on_peer_conn, CONNECT_MESG_HANDSHAKE_REQUEST,
				0);
	} else {
		hmdfs_info("Got a existing tcp conn: socket_fd = %d", fd);
		tcp->fd = INVALID_SOCKET_FD;
		tcp_close_socket(tcp);
		connection_put(tcp_conn);

		tcp_conn = on_peer_conn;
	}

out:
	return tcp_conn;
}

void tcp_stop_connect(struct connection *connect)
{
	hmdfs_info("now nothing to do");
}
