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

#include <linux/file.h>
#include <linux/freezer.h>
#include <linux/highmem.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/file.h>
#include <linux/sched/mm.h>
#include <linux/string.h>

#include "DFS_1_0/adapter_crypto.h"
#include "device_node.h"
#include "hmdfs_trace.h"
#include "socket_adapter.h"
#include "authority/authentication.h"

#ifdef CONFIG_HMDFS_CRYPTO
#include <net/tls.h>
#include "crypto.h"
#endif

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
#include <linux/jiffies.h>
#include "d2dp/d2d.h"
#define D2DP_RECV_TIMEOUT 2
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

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static int d2dp_set_recvtimeo(struct d2dp_handle *d2dp, int timeout)
{
	int rc = 0;
	struct d2d_protocol *proto = d2dp->proto;

	if (timeout > S32_MAX / MSEC_PER_SEC) {
		hmdfs_err("Timeout %d (s) in ms does not fit in int", timeout);
		return -EINVAL;
	}

	/* convert from secs to msecs */
	timeout *= MSEC_PER_SEC;
	rc = d2d_set_opt(proto, D2D_OPT_RCVTIMEO, &timeout, sizeof(timeout));
	if (rc)
		hmdfs_err("Can't set D2DP recv timeout: %d ms", timeout);

	return rc;
}
#endif

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

static uint32_t tcp_get_rtt(struct tcp_handle *tcp)
{
	if (tcp && tcp->sock)
		return tcp_sk(tcp->sock->sk)->srtt_us >> 3;

	hmdfs_err("tcp socket is unavailable");
	return 0;
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static uint32_t d2dp_get_rtt(struct d2dp_handle *d2dp)
{
	/*
	 * D2DP does not calculate RTT. Just return U32_MAX here to get maximum
	 * readahead pages count as it may help to increase bandwidth with
	 * increased RTT under heavy load.
	 */
	return -1;
}
#endif

uint32_t hmdfs_get_connection_rtt(struct hmdfs_peer *peer)
{
	uint32_t rtt_us = 0;
	struct connection *conn = NULL;
	void *handle = NULL;

	/* TODO: what to do if peer has no active connections? */
	conn = get_conn_impl(peer, C_REQUEST);
	if (!conn) {
		hmdfs_err_ratelimited("peer doesn't have any connections");
		return 0;
	}

	handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		rtt_us = tcp_get_rtt(handle);
		break;
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		rtt_us = d2dp_get_rtt(handle);
		break;
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		break;
	}

	connection_put(conn);

	return rtt_us;
}

static int tcp_read_head_from_socket(struct tcp_handle *tcp, void *buf,
				     unsigned int to_read)
{
	int rc = 0;
	struct msghdr hmdfs_msg;
	struct kvec iov;
	struct socket *sock = NULL;

	if (!tcp || !tcp->sock) {
		hmdfs_err("socket is unavailable");
		return -ESHUTDOWN;
	}
	sock = tcp->sock;

	iov.iov_base = buf;
	iov.iov_len = to_read;
	memset(&hmdfs_msg, 0, sizeof(hmdfs_msg));
	hmdfs_msg.msg_flags = MSG_WAITALL;
	hmdfs_msg.msg_control = NULL;
	hmdfs_msg.msg_controllen = 0;
	rc = recvmsg_nofs(sock, &hmdfs_msg, &iov, 1, to_read,
			  hmdfs_msg.msg_flags);
	if (rc == -EBADMSG)
		hmdfs_err_ratelimited("recvmsg EBADMSG");
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

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static int d2dp_read_head_to_buffer(struct d2dp_handle *d2dp, void *buf,
				    size_t to_read)
{
	ssize_t recv_bytes = 0;

	if (!d2dp || !d2dp->proto) {
		hmdfs_err("d2d protocol is unavailable");
		return -ESHUTDOWN;
	}

	recv_bytes = d2d_recv(d2dp->proto, buf, to_read, D2D_RECV_FLAG_WAITALL);
	if (recv_bytes == -EAGAIN) {
		usleep_range(1000, 2000);
		return -EAGAIN;
	}

	if (recv_bytes != to_read) {
		hmdfs_err("d2d recv error");
		return -ESHUTDOWN;
	}

	return 0;
}
#endif

static int tcp_read_buffer_from_socket(struct tcp_handle *tcp, void *buf,
				       unsigned to_read)
{
	int read_cnt = 0;
	int retry_time = 0;
	int rc = 0;
	struct msghdr hmdfs_msg;
	struct kvec iov;
	struct socket *sock = NULL;

	if (!tcp || !tcp->sock) {
		hmdfs_err("socket is unavailable");
		return -ESHUTDOWN;
	}
	sock = tcp->sock;

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
			/* should we reget the socket here? */
			hmdfs_err_ratelimited("recvmsg EBADMSG");
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

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static int d2dp_read_to_buffer(struct d2dp_handle *d2dp, void *buf,
			       size_t to_read)
{
	int retry_time = 0;
	ssize_t recv_bytes = 0;

	if (!d2dp || !d2dp->proto) {
		hmdfs_err("D2D Protocol is unavailable");
		return -ESHUTDOWN;
	}

	do {
		recv_bytes = d2d_recv(d2dp->proto, buf, to_read,
				      D2D_RECV_FLAG_WAITALL);
		if (recv_bytes == -EAGAIN) {
			retry_time++;
			hmdfs_info("read again: %d", retry_time);
			usleep_range(1000, 2000);
			continue;
		}
		if (recv_bytes <= 0) {
			hmdfs_err("D2DP recv error: %zd", recv_bytes);
			return -ESHUTDOWN;
		}
	} while (recv_bytes != to_read && retry_time < MAX_RECV_RETRY_TIMES);

	if (recv_bytes == to_read)
		return 0;

	return -ESHUTDOWN;
}
#endif

static int hmdfs_read_buffer_from_socket(struct connection *conn, void *buf,
					 unsigned to_read)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_read_buffer_from_socket(handle, buf, to_read);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return d2dp_read_to_buffer(handle, buf, to_read);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return -ESHUTDOWN;
	}
}

static int hmdfs_drop_readpage_buffer(struct connection *conn,
				      struct hmdfs_head_cmd *recv)
{
	unsigned int len;
	void *buf = NULL;
	int err = 0;

	len = le32_to_cpu(recv->data_len) - sizeof(struct hmdfs_head_cmd);
	if (len > HMDFS_READPAGES_NR_MAX * HMDFS_PAGE_SIZE || !len) {
		hmdfs_err("recv invalid readpage length %u", len);
		return -EINVAL;
	}

	/* Abort the connection if no memory */
	buf = kmalloc(HMDFS_PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ESHUTDOWN;

	while (len) {
		unsigned int rlen = min_t(unsigned int, len, HMDFS_PAGE_SIZE);
		err = hmdfs_read_buffer_from_socket(conn, buf, rlen);
		if (err)
			break;
		len -= rlen;
	}

	kfree(buf);

	return err;
}

static int hmdfs_get_readpage_buffer(struct connection *conn,
				     struct hmdfs_head_cmd *recv,
				     struct page **pages, unsigned int nr)
{
	char *page_buf = NULL;
	unsigned int out_len;
	unsigned int i = 0;

	out_len = le32_to_cpu(recv->data_len) - sizeof(struct hmdfs_head_cmd);
	if (out_len > nr * HMDFS_PAGE_SIZE) {
		hmdfs_err("recv invalid readpage length %u", out_len);
		return -EINVAL;
	}

	while (out_len) {
		int err;
		unsigned int rlen;

		rlen = min_t(unsigned int, HMDFS_PAGE_SIZE, out_len);
		page_buf = kmap(pages[i]);
		err = hmdfs_read_buffer_from_socket(conn, page_buf, rlen);
		if (err) {
			kunmap(pages[i]);
			return err;
		}
		if (rlen != HMDFS_PAGE_SIZE)
			memset(page_buf + rlen, 0, HMDFS_PAGE_SIZE - rlen);
		kunmap(pages[i]);
		SetPageUptodate(pages[i]);

		out_len -= rlen;
		i++;
	}

	for (; i < nr; i++) {
		page_buf = kmap(pages[i]);
		memset(page_buf, 0, HMDFS_PAGE_SIZE);
		kunmap(pages[i]);
		SetPageUptodate(pages[i]);
	}

	return 0;
}

static int hmdfs_recvpage(struct connection *connect,
			  struct hmdfs_head_cmd *recv)
{
	int ret = 0;
	unsigned int msg_id;
	struct hmdfs_peer *node = NULL;
	struct hmdfs_async_work *async_work = NULL;
	struct hmdfs_inode_info *info = NULL;
	int rd_err;

	if (!connect) {
		hmdfs_err("connect == NULL");
		return -ESHUTDOWN;
	}
	node = connect->node;

	msg_id = le32_to_cpu(recv->msg_id);
	rd_err = le32_to_cpu(recv->ret_code);
	if (rd_err)
		hmdfs_warning("tcp: readpage from peer %llu id %u ret err %d",
			      node->device_id, msg_id, rd_err);

	async_work = (struct hmdfs_async_work *)hmdfs_find_msg_head(node,
								    msg_id);
	if (!async_work || !cancel_delayed_work(&async_work->d_work))
		goto out;

	info = hmdfs_i(async_work->pages[0]->mapping->host);
	if (!rd_err)
		ret = hmdfs_get_readpage_buffer(connect, recv,
						async_work->pages,
						async_work->pages_nr);
	else if (rd_err == -EBADF)
		set_bit(HMDFS_FID_NEED_OPEN, &info->fid_flags);

	hmdfs_client_resp_statis(async_work->head.peer->sbi, async_work->cmd,
				 HMDFS_RESP_NORMAL, async_work->start, jiffies);
	trace_hmdfs_client_recv_readpages(async_work->head.peer,
					  info->remote_ino,
					  async_work->pages[0]->index,
					  async_work->pages_nr, rd_err);
	asw_done(async_work);
	asw_put(async_work);
	return ret;

out:
	if (async_work) {
		hmdfs_client_resp_statis(node->sbi, async_work->cmd,
					 HMDFS_RESP_DELAY, 0, 0);
		asw_put(async_work);
	}
	hmdfs_err_ratelimited("timeout and droppage, msg id %u", msg_id);
	if (!rd_err)
		ret = hmdfs_drop_readpage_buffer(connect, recv);
	return ret;
}

static int tcp_recvbuffer_cipher(struct tcp_handle *tcp,
				 struct hmdfs_head_cmd *recv)
{
	int ret = 0;
	size_t cipherbuffer_len;
	__u8 *cipherbuffer = NULL;
	size_t outlen = 0;
	__u8 *outdata = NULL;
	__u32 recv_len = le32_to_cpu(recv->data_len);
	struct connection *connect = tcp->connect;

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

	ret = tcp_read_buffer_from_socket(tcp, cipherbuffer, cipherbuffer_len);
	if (ret)
		goto out_recv;
	ret = aeadcipher_decrypt_buffer(connect->tfm, cipherbuffer,
					cipherbuffer_len, outdata, outlen);
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

static int hmdfs_recvbuffer(struct connection *connect,
			    struct hmdfs_head_cmd *recv)
{
	int ret = 0;
	size_t outlen;
	__u8 *outdata = NULL;
	__u32 recv_len = le32_to_cpu(recv->data_len);

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

	ret = hmdfs_read_buffer_from_socket(connect, outdata, outlen);
	if (ret) {
		kfree(outdata);
		return ret;
	}
	atomic64_add(outlen, &connect->stat.recv_bytes);
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

static int hmdfs_read_head_from_socket(struct connection *conn,
				       void *buf,
				       unsigned int to_read)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_read_head_from_socket(handle, buf, to_read);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return d2dp_read_head_to_buffer(handle, buf, to_read);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return -ESHUTDOWN;
	}
}

static int hmdfs_recvbuffer_cipher(struct connection *connect,
				   struct hmdfs_head_cmd *recv)
{
	void *handle = connect->connect_handle;

	switch (connect->type) {
	case CONNECT_TYPE_TCP:
		return tcp_recvbuffer_cipher(handle, recv);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		/* D2DP is already encrypted from the construction */
		return hmdfs_recvbuffer(connect, recv);
#endif
	default:
		hmdfs_err("wrong connection type: %d", connect->type);
		return -ESHUTDOWN;
	}
}

static int hmdfs_check_message_head(struct connection *conn,
				    struct hmdfs_head_cmd *recv)
{
	if ((le32_to_cpu(recv->data_len) >
	    HMDFS_MAX_MESSAGE_LEN + sizeof(struct hmdfs_head_cmd)) ||
	    (le32_to_cpu(recv->data_len) < sizeof(struct hmdfs_head_cmd))) {
		hmdfs_info("recv fd %d length error. drop message", conn->fd);
		return -EINVAL;
	}

	if (conn->status == CONNECT_STAT_WORKING &&
	    recv->version != conn->node->version) {
		hmdfs_info("message version error, msg ver %u, local ver %u",
			   recv->version, conn->node->version);
		return -EINVAL;
	}

	return 0;
}

static int hmdfs_process_message(struct connection *conn)
{
	int ret = 0;
	struct hmdfs_head_cmd *recv = NULL;

	recv = kmem_cache_alloc(conn->recv_cache, GFP_KERNEL);
	if (!recv) {
		hmdfs_info("cannot allocate hmdfs_head_cmd from cache");
		return -ESHUTDOWN;
	}

	ret = hmdfs_read_head_from_socket(conn, recv, sizeof(*recv));
	if (ret)
		goto out;

	atomic64_add(sizeof(struct hmdfs_head_cmd), &conn->stat.recv_bytes);
	atomic64_inc(&conn->stat.recv_messages);

	if (recv->magic != HMDFS_MSG_MAGIC) {
		hmdfs_info_ratelimited("recv fd %d wrong magic. drop message",
				       conn->fd);
		goto out;
	}
	hmdfs_conn_touch_timer(conn);

	ret = hmdfs_check_message_head(conn, recv);
	if (ret != 0) {
		goto out;
	}

	if (conn->node->version > USERSPACE_MAX_VER &&
	    conn->status == CONNECT_STAT_WORKING &&
	    (recv->operations.command == F_READPAGE ||
	     recv->operations.command == F_READPAGES) &&
	    recv->operations.cmd_flag == C_RESPONSE) {
		ret = hmdfs_recvpage(conn, recv);
		goto out;
	}

	if (conn->status == CONNECT_STAT_WORKING &&
	    conn->node->version > USERSPACE_MAX_VER)
		ret = hmdfs_recvbuffer(conn, recv);
	else
		ret = hmdfs_recvbuffer_cipher(conn, recv);

out:
	kmem_cache_free(conn->recv_cache, recv);
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
			  tcp->connect->fd, tcp->sock->sk->sk_state);
		return false;
	}

	if (tcp->sock->state != SS_CONNECTING &&
	    tcp->sock->state != SS_CONNECTED) {
		hmdfs_err("TCP conn %d is broken, current sock state is %d",
			  tcp->connect->fd, tcp->sock->state);
		return false;
	}

#ifdef CONFIG_HMDFS_CRYPTO
	tls_ctx = tls_get_ctx(tcp->sock->sk);
	if (tls_ctx) {
		ctx = tls_sw_ctx_rx(tls_ctx);
		if (ctx && ctx->strp.stopped) {
			hmdfs_err(
				"TCP conn %d is broken, the strparser has stopped",
				tcp->connect->fd);
			return false;
		}
	}
#endif
	return true;
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static bool d2dp_handle_is_available(const struct d2dp_handle *d2dp)
{
	/* TODO: how can we check the availability of UDP socket? */
	return d2dp->sock->sk->sk_state == TCP_ESTABLISHED;
}
#endif

static bool hmdfs_handle_is_available(struct connection *conn)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_handle_is_available(handle);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return d2dp_handle_is_available(handle);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		break;
	}

	return false;
}

static struct socket *tcp_handle_get_socket(struct tcp_handle *tcp)
{
	return tcp->sock;
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static struct socket *d2dp_handle_get_socket(struct d2dp_handle *d2dp)
{
	return d2dp->sock;
}
#endif

struct socket *hmdfs_handle_get_socket(struct connection *conn)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_handle_get_socket(handle);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return d2dp_handle_get_socket(handle);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		break;
	}

	return NULL;
}

static int hmdfs_recv_thread(void *arg)
{
	int ret = 0;
	struct connection *conn = arg;
	struct socket *sock = NULL;
	const struct cred *old_cred = NULL;

	if (WARN_ON(!conn ||
		    !conn->connect_handle ||
		    !(sock = hmdfs_handle_get_socket(conn)))) {
		hmdfs_err("bad connection in recv thread");
		return 0;
	}

	set_freezable();
	old_cred = hmdfs_override_creds(conn->node->sbi->system_cred);

	while (!kthread_should_stop()) {
		/*
		 * 1. In case the redundant connection has not been mounted on
		 *    a peer
		 * 2. Lock is unnecessary since a transient state is acceptable
		 */
		if (hmdfs_handle_is_available(conn) &&
		    list_empty(&conn->list))
			goto freeze;
		if (!mutex_trylock(&conn->close_mutex))
			continue;

		if (hmdfs_handle_is_available(conn))
			ret = hmdfs_process_message(conn);
		else
			ret = -ESHUTDOWN;

		/*
		 * This kthread will exit if ret is -ESHUTDOWN, thus we need to
		 * set recv_task to NULL to avoid calling kthread_stop() from
		 * tcp_close_socket().
		 */
		if (ret == -ESHUTDOWN)
			conn->recv_task = NULL;

		mutex_unlock(&conn->close_mutex);

		if (ret == -ESHUTDOWN) {
			hmdfs_node_inc_evt_seq(conn->node);
			conn->status = CONNECT_STAT_STOP;
			if (conn->node->status != NODE_STAT_OFFLINE)
				hmdfs_reget_connection(conn);

			break;
		}
freeze:
		schedule();
		try_to_freeze();
	}

	hmdfs_info("Exiting. Now, sock state = %d", sock->state);
	hmdfs_revert_creds(old_cred);
	connection_put(conn);
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
	ret = aeadcipher_encrypt_buffer(tcp->connect->tfm, msg->data, msg->len,
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
		atomic64_add(send_len, &tcp->connect->stat.send_bytes);
		return -EAGAIN;
	}
	atomic64_add(send_len, &tcp->connect->stat.send_bytes);
	atomic64_inc(&tcp->connect->stat.send_messages);
	return 0;
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
#ifdef CONFIG_HMDFS_D2DP_TX_BINDING
static void d2dp_bind_tx_thread(struct d2dp_handle *d2dp,
				uint64_t old_bw,
				uint64_t new_bw)
{
	struct cpumask mask;

	if (new_bw >  D2D_BANDWIDTH_HIGH_THRESHOLD &&
	    old_bw <= D2D_BANDWIDTH_HIGH_THRESHOLD) {
		cpumask_clear(&mask);
		cpumask_set_cpu(HISI_FAST_CORE_6, &mask);
		cpumask_set_cpu(HISI_FAST_CORE_7, &mask);
		d2d_set_opt(d2dp->proto, D2D_OPT_TXCPUMASK,
			    &mask, sizeof(mask));
	} else if (new_bw <  D2D_BANDWIDTH_LOW_THRESHOLD &&
		   old_bw >= D2D_BANDWIDTH_LOW_THRESHOLD) {
		cpumask_setall(&mask);
		d2d_set_opt(d2dp->proto, D2D_OPT_TXCPUMASK,
			    &mask, sizeof(mask));
	}
}
#endif /* CONFIG_HMDFS_D2DP_TX_BINDING */

static void d2dp_try_update_binding(struct d2dp_handle *d2dp)
{
#ifdef CONFIG_HMDFS_D2DP_TX_BINDING
	unsigned long elapsed = 0;
	unsigned long last_time = atomic_long_read(&d2dp->last_time);

	elapsed = jiffies_to_msecs(jiffies - last_time);
	if (elapsed > D2D_BANDWIDTH_INTERVAL_MS) {
		if (mutex_trylock(&d2dp->bind_lock)) {
			struct connection *conn = d2dp->connect;
			uint64_t last_bw        = d2dp->last_bw;
			uint64_t last_bytes     = d2dp->last_bytes;
			uint64_t bytes = atomic64_read(&conn->stat.send_bytes);
			uint64_t sent = bytes - last_bytes;
			uint64_t bw = sent * 8 / elapsed / 1000; /* Mbps */

			d2dp_bind_tx_thread(d2dp, last_bw, bw);

			d2dp->last_bytes = bytes;
			d2dp->last_bw = bw;
			atomic_long_set(&d2dp->last_time, jiffies);

			mutex_unlock(&d2dp->bind_lock);
		}
	}
#endif /* CONFIG_HMDFS_D2DP_TX_BINDING */
}

static int __d2dp_send_message(struct d2dp_handle *d2dp,
			       struct hmdfs_send_data *msg, u32 flag)
{
	int send_len = 0;
	int send_vec_cnt = 0;
	struct kvec iov[TCP_KVEC_ELE_TRIPLE] = { { 0 } };

	if (!d2dp || !d2dp->proto) {
		hmdfs_err("invalid D2DP handle");
		return -ESHUTDOWN;
	}

	iov[TCP_KVEC_HEAD].iov_base = msg->head;
	iov[TCP_KVEC_HEAD].iov_len  = msg->head_len;

	if (msg->len == 0 && msg->sdesc_len == 0) {
		send_vec_cnt = TCP_KVEC_ELE_SINGLE;
	} else if (msg->sdesc_len == 0) {
		iov[TCP_KVEC_DATA].iov_base = msg->data;
		iov[TCP_KVEC_DATA].iov_len  = msg->len;
		send_vec_cnt                = TCP_KVEC_ELE_DOUBLE;
	} else {
		iov[TCP_KVEC_FILE_PARA].iov_base    = msg->sdesc;
		iov[TCP_KVEC_FILE_PARA].iov_len     = msg->sdesc_len;
		iov[TCP_KVEC_FILE_CONTENT].iov_base = msg->data;
		iov[TCP_KVEC_FILE_CONTENT].iov_len  = msg->len;
		send_vec_cnt                        = TCP_KVEC_ELE_TRIPLE;
	}

	send_len = d2d_sendiov(d2dp->proto, iov, send_vec_cnt,
			       msg->head_len + msg->len + msg->sdesc_len, flag);
	if (send_len == -EINVAL)
		return -EINVAL;
	else if (send_len == -EAGAIN)
		return -EAGAIN;
	else if (send_len < 0)
		return -ESHUTDOWN;

	atomic64_add(send_len, &d2dp->connect->stat.send_bytes);
	atomic64_inc(&d2dp->connect->stat.send_messages);
	d2dp_try_update_binding(d2dp);

	return 0;
}

static int d2dp_send_message(struct d2dp_handle *d2dp,
			     struct hmdfs_send_data *msg)
{
	return __d2dp_send_message(d2dp, msg, 0);
}

static int d2dp_send_message_rekey(struct d2dp_handle *d2dp,
				   struct hmdfs_send_data *msg)
{
	return __d2dp_send_message(d2dp, msg, D2D_SEND_FLAG_KEY_UPDATE);
}
#endif

static int hmdfs_send_message_sock_rekey(struct connection *conn,
					 struct hmdfs_send_data *msg)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		/*
		 * As TLS socket is used synchronously, nothing to do when
		 * dealing with rekey messages
		 */
		return tcp_send_message_sock_tls(handle, msg);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		/*
		 * D2DP uses asynchronous transport threads for RX and TX, we
		 * have to process rekey messages with care to avoid race
		 * conditions between updater thread and D2DP transport thread.
		 */
		return d2dp_send_message_rekey(handle, msg);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return -EINVAL;
	}
}

static int hmdfs_send_message_sock(struct connection *conn,
				   struct hmdfs_send_data *msg)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_send_message_sock_tls(handle, msg);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return d2dp_send_message(handle, msg);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return -EINVAL;
	}
}

static int hmdfs_send_message_sock_cipher(struct connection *conn,
					  struct hmdfs_send_data *msg)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_send_message_sock_cipher(handle, msg);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		/* D2DP is already encrypted by the construction */
		return d2dp_send_message(handle, msg);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return -EINVAL;
	}
}

#ifdef CONFIG_HMDFS_CRYPTO
int hmdfs_send_rekey_request(struct connection *connect)
{
	int ret = 0;
	struct hmdfs_send_data msg;
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
	ret = hmdfs_send_message_sock_rekey(connect, &msg);
	if (ret != 0)
		hmdfs_err("return error %d", ret);

	kfree(head);
	return ret;
}
#endif

static int hmdfs_send_message_generic(struct connection *connect,
				      struct hmdfs_send_data *msg)
{
	int ret = 0;

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

	if (connect->status == CONNECT_STAT_STOP)
		return -EAGAIN;

#ifdef CONFIG_HMDFS_CRYPTO
	if (jiffies - connect->stat.rekey_time >= REKEY_LIFETIME &&
	    connect->status == CONNECT_STAT_WORKING &&
	    connect->node->version >= DFS_2_0) {
		hmdfs_info("send rekey message to devid %llu",
			   connect->node->device_id);
		ret = hmdfs_send_rekey_request(connect);
		if (ret == 0)
			set_crypto_info(connect, SET_CRYPTO_SEND);

		connect->stat.rekey_time = jiffies;
	}
#endif

	if (connect->node->version > USERSPACE_MAX_VER)
		trace_hmdfs_tcp_send_message_2_0(msg->head);
	else
		trace_hmdfs_tcp_send_message_1_0(msg->head);

	if (connect->status == CONNECT_STAT_WORKING &&
	    connect->node->version > USERSPACE_MAX_VER)
		ret = hmdfs_send_message_sock(connect, msg);
	else
		// Handshake status or version HMDFS1.0
		ret = hmdfs_send_message_sock_cipher(connect, msg);

	if (ret != 0) {
		hmdfs_err("return error %d", ret);
		return ret;
	}

	hmdfs_conn_touch_timer(connect);

	return ret;
}

static struct tcp_handle *tcp_alloc_handle(struct connection *conn,
					   struct connection_init_info *info)
{
	int err = 0;
	struct tcp_handle *tcp = NULL;
	struct socket *sock = info->sock;

	tcp = kzalloc(sizeof(*tcp), GFP_KERNEL);
	if (!tcp) {
		hmdfs_err("cannot allocate tcp handle");
		return NULL;
	}

	tcp->connect = conn;
	mutex_init(&tcp->send_mutex);
	tcp->sock = sock;

	if (!tcp_handle_is_available(tcp)) {
		err = -EPIPE;
		goto free_tcp;
	}

	hmdfs_info("socket fd %d, state %d, refcount %ld",
		   conn->fd, sock->state, file_count(sock->file));

	err = tcp_set_recvtimeo(sock, TCP_RECV_TIMEOUT);
	if (err) {
		hmdfs_err("tcp set timeout error");
		goto free_tcp;
	}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	/*
	 * Our current D2DP connection is not self-contained, it is linked to
	 * the correspondent TCP connection, so we keep this D2DP-related info
	 * in TCP handle
	 */
	tcp->d2dp_local_udp_port = info->d2dp_udp_port;
	tcp->d2dp_local_version  = info->d2dp_version;
	tcp->local_device_type   = info->device_type;
	tcp->remote_device_type  = 0;
#endif
	return tcp;

free_tcp:
	kfree(tcp);
	return NULL;
}

static struct connection *
lookup_conn_by_socket_unsafe(struct hmdfs_peer *node, struct socket *socket)
{
	struct connection *conn = NULL;

	list_for_each_entry(conn, &node->conn_impl_list, list) {
		if (hmdfs_handle_get_socket(conn) == socket) {
			connection_get(conn);
			return conn;
		}
	}

	return NULL;
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static struct connection *lookup_conn_by_socket_fd(struct hmdfs_peer *peer,
						   int sock_fd)
{
	int err = 0;
	struct connection *conn = NULL;
	struct socket *sock = NULL;

	sock = sockfd_lookup(sock_fd, &err);
	if (!sock) {
		hmdfs_err("lookup socket fail: fd %d, err %d", sock_fd, err);
		return NULL;
	}

	mutex_lock(&peer->conn_impl_list_lock);
	conn = lookup_conn_by_socket_unsafe(peer, sock);
	mutex_unlock(&peer->conn_impl_list_lock);
	if (!conn) {
		hmdfs_err("cannot find connection for fd %d", sock_fd);
		return NULL;
	}
	return conn;
}

static int d2dp_init_session_keys(struct connection *conn)
{
	int ret = 0;
	int hkdf_send = 0;
	int hkdf_recv = 0;
	char key1[HMDFS_KEY_SIZE] = { 0 };
	char key2[HMDFS_KEY_SIZE] = { 0 };

	switch (conn->status) {
	case CONNECT_STAT_WAIT_RESPONSE:
		hkdf_send = HKDF_TYPE_D2DP_INITIATOR;
		hkdf_recv = HKDF_TYPE_D2DP_ACCEPTER;
		break;
	case CONNECT_STAT_WAIT_REQUEST:
		hkdf_send = HKDF_TYPE_D2DP_ACCEPTER;
		hkdf_recv = HKDF_TYPE_D2DP_INITIATOR;
		break;
	default:
		hmdfs_err("bad connection status: %d", conn->status);
		return -EINVAL;
	}

	ret = update_key(conn->master_key, key1, hkdf_send);
	if (ret < 0) {
		hmdfs_err("cannot update send key: %d", ret);
		goto out_memzero;
	}

	ret = update_key(conn->master_key, key2, hkdf_recv);
	if (ret < 0) {
		hmdfs_err("cannot update recv key: %d", ret);
		goto out_memzero;
	}

	memcpy(conn->send_key, key1, HMDFS_KEY_SIZE);
	memcpy(conn->recv_key, key2, HMDFS_KEY_SIZE);
out_memzero:
	memzero_explicit(key1, HMDFS_KEY_SIZE);
	memzero_explicit(key2, HMDFS_KEY_SIZE);
	return ret;
}

static int d2dp_prepare_crypto(struct connection *conn,
			       struct crypto_aead **psend_tfm,
			       struct crypto_aead **precv_tfm)
{
	int err = 0;
	struct crypto_aead *send_tfm = NULL;
	struct crypto_aead *recv_tfm = NULL;

	err = d2dp_init_session_keys(conn);
	if (err) {
		hmdfs_err("cannot init D2DP session keys: %d", err);
		goto out;
	}

	send_tfm = hmdfs_alloc_aead(conn->send_key, HMDFS_KEY_SIZE);
	if (!send_tfm) {
		hmdfs_err("cannot set TX keys for D2DP");
		goto out;
	}

	recv_tfm = hmdfs_alloc_aead(conn->recv_key, HMDFS_KEY_SIZE);
	if (!recv_tfm) {
		hmdfs_err("cannot set RX keys for D2DP");
		goto out_free_send_tfm;
	}

	*psend_tfm = send_tfm;
	*precv_tfm = recv_tfm;

	return 0;

out_free_send_tfm:
	crypto_free_aead(send_tfm);
out:
	return -1;
}

static int d2dp_configure_socket_buffers(struct socket *sock)
{
	int err = 0;
#ifdef CONFIG_HMDFS_D2DP_UDP_INCREASE_SND_BUF
	int bufsize = 4000000;

	err = kernel_setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&bufsize,
				sizeof(bufsize));
	if (err)
		hmdfs_err("Can't increase UDP send buffer, error %d", err);
#endif
	return err;
}

static void d2dp_configure_handle(struct d2dp_handle *d2dp,
				  struct connection *connect,
				  struct socket *sock,
				  struct crypto_aead *send_tfm,
				  struct crypto_aead *recv_tfm)
{
	d2dp->connect              = connect;
	d2dp->sock                 = sock;
	d2dp->send_tfm             = send_tfm;
	d2dp->recv_tfm             = recv_tfm;
	d2dp->security.cred        = connect->node->sbi->system_cred;
	d2dp->security.cr_overhead = HMDFS_IV_SIZE + HMDFS_TAG_SIZE;
	d2dp->security.key_info    = d2dp;
	d2dp->security.encrypt     = d2dp_encrypt;
	d2dp->security.decrypt     = d2dp_decrypt;
#ifdef CONFIG_HMDFS_D2DP_TX_BINDING
	mutex_init(&d2dp->bind_lock);
	atomic_long_set(&d2dp->last_time, jiffies);
#endif
}

static struct d2dp_handle *d2dp_alloc_handle(struct connection *connect,
					     struct connection_init_info *info)
{
	int err = 0;
	struct crypto_aead *send_tfm = NULL;
	struct crypto_aead *recv_tfm = NULL;
	struct d2dp_handle *d2dp = NULL;
	struct socket *sock = info->sock;

	err = d2dp_configure_socket_buffers(sock);
	if (err)
		return NULL;

	d2dp = kzalloc(sizeof(*d2dp), GFP_KERNEL);
	if (!d2dp) {
		hmdfs_err("cannot allocate D2DP handle");
		return NULL;
	}

	/*
	* Init crypto for D2DP immediately. We have to do that because D2DP
	* crypto must be initialized before we can construct the protocol
	*/
	err = d2dp_prepare_crypto(connect, &send_tfm, &recv_tfm);
	if (err)
		goto free_handle;

	d2dp_configure_handle(d2dp, connect, sock, send_tfm, recv_tfm);
	d2dp->proto = d2d_protocol_create(d2dp->sock, 0, &d2dp->security);
	if (!d2dp->proto) {
		hmdfs_err("cannot alloc D2D Protocol instance");
		goto free_crypto;
	}

	err = d2dp_set_recvtimeo(d2dp, D2DP_RECV_TIMEOUT);
	if (err)
		goto close_protocol;

	d2dp->tcp_connect = lookup_conn_by_socket_fd(connect->node,
						     info->binded_fd);
	if (!d2dp->tcp_connect)
		goto close_protocol;

	hmdfs_info("D2D Protocol handle created");
	return d2dp;

close_protocol:
	d2d_protocol_close(d2dp->proto);
free_crypto:
	crypto_free_aead(recv_tfm);
	crypto_free_aead(send_tfm);
free_handle:
	kfree(d2dp);
	return NULL;
}
#endif /* CONFIG_HMDFS_D2DP_TRANSPORT */

void hmdfs_get_connection(struct hmdfs_peer *peer, int flag)
{
	struct notify_param param = { 0 };

	if (!peer)
		return;

	param.notify = NOTIFY_GET_SESSION;
	param.remote_iid = peer->iid;
	param.fd = INVALID_SOCKET_FD;
	param.flag = flag;
	memcpy(param.remote_cid, peer->cid, HMDFS_CID_SIZE);
	notify(peer, &param);
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static void connection_notify_close_d2dp(struct hmdfs_peer *peer,
					 struct connection *conn)
{
	struct notify_param param = {
		.notify = NOTIFY_D2DP_FAILED,
		.fd     = conn->fd,
	};

	memcpy(param.remote_cid, peer->cid, HMDFS_CID_SIZE);
	notify(peer, &param);
}
#endif

// libdistbus/src/TcpSession.cpp will close the socket
static void connection_notify_close_p2p(struct hmdfs_peer *peer,
					struct connection *conn)
{
	struct notify_param param = {
		.notify = NOTIFY_DISCONNECT,
		.fd     = conn->fd,
	};

	memcpy(param.remote_cid, peer->cid, HMDFS_CID_SIZE);
	notify(peer, &param);
}

static void connection_notify_close_default(struct hmdfs_peer *peer,
					    struct connection *conn)
{
	struct notify_param param = {
		.notify     = NOTIFY_GET_SESSION,
		.fd         = conn->fd,
		.remote_iid = peer->iid,
	};

	memcpy(param.remote_cid, peer->cid, HMDFS_CID_SIZE);
	notify(peer, &param);
}

static void connection_notify_to_close(struct connection *conn)
{
	struct hmdfs_peer *peer = conn->node;

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	/* D2DP connection depends on TCP, so don't re-get it */
	if (conn->type == CONNECT_TYPE_D2DP) {
		connection_notify_close_d2dp(peer, conn);
		return;
	}
#endif

	if (conn->link_type == LINK_TYPE_P2P) {
		connection_notify_close_p2p(peer, conn);
		return;
	}

	connection_notify_close_default(peer, conn);
}

static void tcp_handle_shutdown(struct tcp_handle *tcp)
{
	if (!tcp || !tcp->sock)
		return;

	kernel_sock_shutdown(tcp->sock, SHUT_RDWR);
	hmdfs_info("shutdown tcp sock: fd = %d, sockref = %ld, connref = %u",
		   tcp->connect->fd, file_count(tcp->sock->file),
		   kref_read(&tcp->connect->ref_cnt));
}


#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static void d2dp_handle_shutdown(struct d2dp_handle *d2dp)
{
	if (!d2dp)
		return;

	kernel_sock_shutdown(d2dp->sock, SHUT_RDWR);
	hmdfs_info("shutdown UDP sock: fd = %d, sockref = %ld, connref = %u",
		   d2dp->connect->fd, file_count(d2dp->sock->file),
		   kref_read(&d2dp->connect->ref_cnt));
}
#endif

void hmdfs_handle_shutdown(struct connection *conn)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		tcp_handle_shutdown(handle);
		break;
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		d2dp_handle_shutdown(handle);
		break;
#endif
	default:
		hmdfs_err("bad connection type: %d", conn->type);
		break;
	}
}

static void hmdfs_online_to_shaking(struct hmdfs_peer *node)
{
	cmpxchg(&node->status, NODE_STAT_ONLINE, NODE_STAT_SHAKING);
}

void hmdfs_reget_connection(struct connection *conn)
{
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
			if (list_empty(&conn->node->conn_impl_list))
				hmdfs_online_to_shaking(conn->node);
			break;
		}
	}
	if (!should_put) {
		mutex_unlock(&conn->node->conn_impl_list_lock);
		return;
	}

	/*
	 * To avoid the receive thread to stop itself. Ensure receive thread
	 * stop before process offline event
	 */
	recv_task = conn->recv_task;
	if (!recv_task ||
	    (recv_task && (recv_task->pid == current->pid)))
		stop_thread = false;
	mutex_unlock(&conn->node->conn_impl_list_lock);

	hmdfs_handle_shutdown(conn);

	if (stop_thread)
		hmdfs_stop_thread(conn);

	if (conn->fd != INVALID_SOCKET_FD)
		connection_notify_to_close(conn);

	connection_put(conn);
}

static void hmdfs_reget_connection_work_fn(struct work_struct *work)
{
	struct connection *conn =
		container_of(work, struct connection, reget_work);

	hmdfs_reget_connection(conn);
	connection_put(conn);
}

static void *alloc_handle(struct connection *conn,
			  struct connection_init_info *info)
{
	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return tcp_alloc_handle(conn, info);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return d2dp_alloc_handle(conn, info);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return NULL;
	}
}

static void tcp_handle_release(struct tcp_handle *tcp)
{
	// need to check and test: fput(tcp->sock->file);
	if (tcp && tcp->sock) {
		hmdfs_info("tcp handle release: refcount %ld",
			   file_count(tcp->sock->file));
		sockfd_put(tcp->sock);
	}
	kfree(tcp);
}

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static void d2dp_handle_release(struct d2dp_handle *d2dp)
{
	if (!d2dp)
		return;

	hmdfs_info("d2dp handle release: refcount %ld",
		   file_count(d2dp->sock->file));

	/* close protocol _before_ touching crypto or socket */
	d2d_protocol_close(d2dp->proto);

	sockfd_put(d2dp->sock);

	crypto_free_aead(d2dp->recv_tfm);
	crypto_free_aead(d2dp->send_tfm);
	connection_put(d2dp->tcp_connect);
	kfree(d2dp);
}
#endif

void hmdfs_handle_release(struct connection *conn)
{
	void *handle = conn->connect_handle;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		tcp_handle_release(handle);
		break;
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		d2dp_handle_release(handle);
		break;
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		break;
	}
}

static
int configure_connection_crypto(struct connection *conn, uint8_t *master_key)
{
	// send key and recv key, default MASTER KEY
	memcpy(conn->master_key, master_key, HMDFS_KEY_SIZE);
	memcpy(conn->send_key, master_key, HMDFS_KEY_SIZE);
	memcpy(conn->recv_key, master_key, HMDFS_KEY_SIZE);
	conn->tfm = hmdfs_alloc_aead(master_key, HMDFS_KEY_SIZE);
	if (!conn->tfm)
		return -1;

	return 0;
}

static int configure_connection_info(struct connection *conn,
				     struct connection_init_info *info)
{
	conn->type            = info->protocol;
	conn->fd              = info->fd;
	conn->status          = info->status;
	conn->client          = info->status;
	conn->link_type       = info->link_type;
	conn->stat.rekey_time = jiffies;

	return configure_connection_crypto(conn, info->master_key);
}

struct connection *alloc_connection(struct hmdfs_peer *node,
				    struct connection_init_info *info)
{
	struct connection *conn = kzalloc(sizeof(*conn), GFP_KERNEL);

	if (!conn)
		return NULL;

	kref_init(&conn->ref_cnt);
	mutex_init(&conn->ref_lock);
	mutex_init(&conn->close_mutex);
	INIT_LIST_HEAD(&conn->list);
	INIT_WORK(&conn->reget_work, hmdfs_reget_connection_work_fn);
	conn->node = node;
	conn->close = hmdfs_stop_connect;
	conn->send_message = hmdfs_send_message_generic;
	conn->recvbuf_maxsize = MAX_RECV_SIZE;
	conn->recv_cache = kmem_cache_create("hmdfs_socket",
					     conn->recvbuf_maxsize,
					     0, SLAB_HWCACHE_ALIGN, NULL);
	if (!conn->recv_cache)
		goto free_conn;

	if (configure_connection_info(conn, info))
		goto free_cache;

	conn->connect_handle = alloc_handle(conn, info);
	if (!conn->connect_handle) {
		hmdfs_err("Failed to alloc handle for struct connection");
		goto free_crypto;
	}

	connection_get(conn);
	conn->recv_task =
		kthread_create(hmdfs_recv_thread, conn, "dfs_rcv%u_%llu_%d",
			       node->owner, node->device_id, info->fd);
	if (IS_ERR(conn->recv_task)) {
		hmdfs_err("recv_task %ld", PTR_ERR(conn->recv_task));
		conn->recv_task = NULL;
		goto free_handle;
	}

	return conn;

free_handle:
	connection_put(conn);
	hmdfs_handle_release(conn);
free_crypto:
	crypto_free_aead(conn->tfm);
free_cache:
	kmem_cache_destroy(conn->recv_cache);
free_conn:
	kfree(conn);
	return NULL;
}

static unsigned int conn_priority(struct connection *conn)
{
	/* 1. prefer to use wifi link */
	/* 2. prefer to use d2dp connection */
	/* 3. prefer to use the connection marked as client */
	return ((conn->link_type == LINK_TYPE_WLAN ? 0 : 1) << 2) |
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
		((conn->type == CONNECT_TYPE_D2DP ? 0 : 1) << 1) |
#endif
		(conn->client == 1 ? 0 : 1);
}

static struct connection *add_conn_unsafe(struct hmdfs_peer *node,
					  struct socket *socket,
					  struct connection *conn2add)
{
	struct connection *conn;
	struct connection *it = NULL;

	conn = lookup_conn_by_socket_unsafe(node, socket);
	if (conn) {
		hmdfs_info("socket already in list");
		return conn;
	}

	conn2add->priority = conn_priority(conn2add);
	list_for_each_entry(it, &node->conn_impl_list, list) {
		if (conn2add->priority <= it->priority) {
			list_add_tail(&conn2add->list, &it->list);
			goto out;
		}
	}
	list_add_tail(&conn2add->list, &node->conn_impl_list);

out:
	connection_get(conn2add);
	return conn2add;
}

/*
 * must be called from CMD control context
 */
void hmdfs_delete_connection_by_sock(struct hmdfs_peer *peer,
				     struct socket *sock)
{
	struct connection *conn = NULL;

	mutex_lock(&peer->conn_impl_list_lock);
	conn = lookup_conn_by_socket_unsafe(peer, sock);
	if (unlikely(!conn)) {
		mutex_unlock(&peer->conn_impl_list_lock);
		hmdfs_err("failed to delete connection: no such connection");
		return;
	}
	list_del_init(&conn->list);
	if (list_empty(&conn->node->conn_impl_list))
		hmdfs_online_to_shaking(conn->node);
	mutex_unlock(&peer->conn_impl_list_lock);

	hmdfs_info("found the connection, deleting");
	hmdfs_stop_thread(conn);
	hmdfs_handle_shutdown(conn);
	connection_put(conn); /* put after lookup */
	connection_put(conn); /* remove last reference and release conn */
}

struct connection *hmdfs_get_conn(struct hmdfs_peer *node,
				  struct connection_init_info *info)
{
	struct connection *conn = NULL, *on_peer_conn = NULL;

	mutex_lock(&node->conn_impl_list_lock);
	conn = lookup_conn_by_socket_unsafe(node, info->sock);
	mutex_unlock(&node->conn_impl_list_lock);
	if (conn) {
		hmdfs_info("Got an existing conn: fd = %d", info->fd);
		sockfd_put(info->sock);
		goto out;
	}

	conn = alloc_connection(node, info);
	if (!conn) {
		hmdfs_info("Failed to alloc connection, fd = %d", info->fd);
		sockfd_put(info->sock);
		goto out;
	}

	mutex_lock(&node->conn_impl_list_lock);
	on_peer_conn = add_conn_unsafe(node, info->sock, conn);
	mutex_unlock(&node->conn_impl_list_lock);
	if (on_peer_conn == conn) {
		hmdfs_info("Got a newly allocated conn: fd = %d", info->fd);
		wake_up_process(conn->recv_task);
		if (info->status == CONNECT_STAT_WAIT_RESPONSE)
			connection_send_handshake(
				on_peer_conn, CONNECT_MESG_HANDSHAKE_REQUEST,
				0);
	} else {
		hmdfs_info("Got an existing conn: fd = %d", info->fd);
		conn->fd = INVALID_SOCKET_FD;
		hmdfs_stop_thread(conn);
		connection_put(conn);

		conn = on_peer_conn;
	}

out:
	return conn;
}

void hmdfs_stop_connect(struct connection *connect)
{
	hmdfs_info("now nothing to do");
}
