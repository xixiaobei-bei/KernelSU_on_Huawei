// SPDX-License-Identifier: GPL-2.0
/* D2D protocol base implementation */

#define pr_fmt(fmt) "[D2DP]: " fmt

#include <asm-generic/errno.h>
#include <asm-generic/errno-base.h>
#include <linux/atomic.h>
#include <linux/compiler.h>
#include <linux/completion.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/if_ether.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/signal.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/cpumask.h>

#include "d2d.h"
#include "buffer.h"
#include "kobject.h"
#include "headers.h"
#include "params.h"
#include "protocol.h"
#include "timers.h"
#include "transport.h"
#include "options.h"

/**
 * crypto_check_packet_size - which packet size we can use with encryption
 * @security: security structure instance from the upper layer
 * @packet_size: packet size we can use for plaintext
 *
 * Returns new payload length available for data with encryption overhead in
 * mind or 0 on error.
 */
static size_t crypto_check_packet_size(const struct d2d_security *security,
				       size_t packet_size)
{
	if (security) {
		/* crypto primitives must be implemented */
		if (!security->encrypt || !security->decrypt) {
			pr_err("security is not null, but no implementation\n");
			return 0;
		}

		/* crypto overhead must be less than available packet size */
		if (security->cr_overhead >= packet_size) {
			pr_err("crypto overhead is too big: %zu vs %zu\n",
			       security->cr_overhead, packet_size);
			return 0;
		}

		packet_size -= security->cr_overhead;
	}

	return packet_size;
}

static
int init_tx_handler(struct tx_handler *tx, size_t bufsize, size_t packet_size)
{
	struct tx_buffer *tx_buf = NULL;

	tx_buf = tx_buf_alloc(bufsize, packet_size);
	if (!tx_buf) {
		pr_err("cannot allocate TX buffer for D2DP\n");
		return -ENOMEM;
	}

	memset(tx, 0, sizeof(*tx));

	spin_lock_init(&tx->lock);
	tx->buffer = tx_buf;
	init_waitqueue_head(&tx->wq_free);
	init_waitqueue_head(&tx->wq_data);
	tx->ack_state.ack_to_send = tx->ack_state.acks;
	tx->ack_state.ack_to_fill = tx->ack_state.acks;

	return 0;
}

static void deinit_tx_handler(struct tx_handler *tx)
{
	tx_buf_free(tx->buffer);
}

static
int init_rx_handler(struct rx_handler *rx, size_t bufsize, size_t packet_size)
{
	struct rx_buffer *rx_buf = NULL;

	rx_buf = rx_buf_alloc(bufsize, packet_size);
	if (!rx_buf) {
		pr_err("cannot allocate RX buffer for D2DP\n");
		return -ENOMEM;
	}

	memset(rx, 0, sizeof(*rx));

	spin_lock_init(&rx->lock);
	rx->buffer = rx_buf;
	init_waitqueue_head(&rx->wq_data);

	return 0;
}

static void deinit_rx_handler(struct rx_handler *rx)
{
	rx_buf_free(rx->buffer);
}

/**
 * allocate_frames - helper function to pre-allocate data buffers
 *
 * As D2D protocol does not implement cryptography itself but takes arbitrary
 * crypto functions from caller we must care about limitations caused by several
 * cryptoprimitives. For example, AEAD ciphers require buffers to have addresses
 * which are virt_addr_valid(). To avoid such problems we pre-allocate buffers
 * for TX/RX frames on heap using kzalloc (instead of ordinary vzalloc).
 *
 * Returns 0 on success and negative integer on error.
 */
static int allocate_frames(struct net_frames *frames, size_t frame_len)
{
	void *tx_frame      = NULL;
	void *rx_frame      = NULL;
	void *tx_encr_frame = NULL;
	void *rx_encr_frame = NULL;

	tx_frame      = kzalloc(frame_len, GFP_KERNEL);
	rx_frame      = kzalloc(frame_len, GFP_KERNEL);
	tx_encr_frame = kzalloc(frame_len, GFP_KERNEL);
	rx_encr_frame = kzalloc(frame_len, GFP_KERNEL);
	if (!tx_frame || !rx_frame || !tx_encr_frame || !rx_encr_frame) {
		pr_err("cannot preallocate frames for D2DP\n");
		goto fail;
	}

	frames->tx_frame      = tx_frame;
	frames->rx_frame      = rx_frame;
	frames->tx_encr_frame = tx_encr_frame;
	frames->rx_encr_frame = rx_encr_frame;
	frames->len           = frame_len;
	mutex_init(&frames->crypto_tx_mtx);
	mutex_init(&frames->crypto_rx_mtx);

	return 0;

fail:
	kfree(rx_encr_frame);
	kfree(tx_encr_frame);
	kfree(rx_frame);
	kfree(tx_frame);
	return -ENOMEM;
}

static void deallocate_frames(struct net_frames *frames)
{
	kfree(frames->tx_frame);
	kfree(frames->rx_frame);
	kfree(frames->tx_encr_frame);
	kfree(frames->rx_encr_frame);
}

static int init_proto_timers(struct timers *timers, struct d2d_protocol *proto)
{
	timers->timer_wq = d2d_timer_workqueue_create("d2dp_timer_wq");
	if (!timers->timer_wq) {
		pr_err("cannot allocate timer workqueue for D2DP\n");
		return -ENOMEM;
	}

	d2d_timer_create(timers->timer_wq, &timers->ack_timer,
			 D2D_DEFAULT_ACK_PERIOD_USEC, ack_timer_action, proto);
	d2d_timer_create(timers->timer_wq, &timers->rto_timer,
			 D2D_DEFAULT_RTO_PERIOD_USEC, rto_timer_action, proto);

	return 0;
}

static void deinit_proto_timers(struct timers *timers)
{
	d2d_timer_destroy(&timers->ack_timer);
	d2d_timer_destroy(&timers->rto_timer);
	d2d_timer_workqueue_destroy(timers->timer_wq);
}

static int create_trasport_threads(struct d2dp_transport_threads *transport,
				   struct d2d_protocol *proto)
{
	int err = 0;
	struct task_struct *tx = NULL;
	struct task_struct *rx = NULL;

	/* initialize these fields first, the threads may exit early */
	init_completion(&transport->tx.exit);
	init_completion(&transport->rx.exit);
	mutex_init(&transport->exit_completion_lock);

	tx = kthread_create(tx_transport_thread, proto, "d2dp_tx_thread");
	if (IS_ERR(tx)) {
		err = PTR_ERR(tx);
		pr_err("cannot launch TX thread for D2DP: %d\n", err);
		goto fail;
	}

	rx = kthread_create(rx_transport_thread, proto, "d2dp_rx_thread");
	if (IS_ERR(rx)) {
		err = PTR_ERR(rx);
		pr_err("cannot launch RX thread for D2DP: %d\n", err);
		goto free_tx_thread;
	}

	transport->tx.task = tx;
	transport->rx.task = rx;

	wake_up_process(tx);
	wake_up_process(rx);

	return 0;

free_tx_thread:
	atomic_set_release(&proto->closing, 1);
	wake_up_interruptible_all(&proto->tx.wq_data);
fail:
	return err;
}

static struct d2d_protocol *d2d_protocol_alloc(struct socket *sock,
					       uint32_t flags,
					       struct d2d_security *security,
					       unsigned int payload)
{
	int err = 0;
	struct d2d_protocol *proto = NULL;

	proto = vzalloc(sizeof(*proto));
	if (!proto) {
		pr_err("cannot allocate D2DP instance\n");
		return NULL;
	}

	proto->sock = sock;
	d2d_options_init(&proto->options);
	proto->flags = flags;
	proto->security = security;

	err = allocate_frames(&proto->frames, UDP_DATAGRAM_SIZE);
	if (err)
		goto free_proto;

	err = init_tx_handler(&proto->tx, D2D_DEFAULT_BUFFER_SIZE, payload);
	if (err)
		goto free_frames;

	err = init_rx_handler(&proto->rx, D2D_DEFAULT_BUFFER_SIZE, payload);
	if (err)
		goto free_tx;

	err = init_proto_timers(&proto->timers, proto);
	if (err)
		goto free_rx;

	err = create_trasport_threads(&proto->transport, proto);
	if (err)
		goto free_timers;

	return proto;

free_timers:
	deinit_proto_timers(&proto->timers);
free_rx:
	deinit_rx_handler(&proto->rx);
free_tx:
	deinit_tx_handler(&proto->tx);
free_frames:
	deallocate_frames(&proto->frames);
free_proto:
	vfree(proto);
	return NULL;
}

struct d2d_protocol *d2d_protocol_create(struct socket *sock, uint32_t flags,
					 struct d2d_security *security)
{
	struct d2d_protocol *proto = NULL;
	size_t header_size = d2d_get_header_packed_size();
	size_t packet_size = UDP_DATAGRAM_SIZE - header_size;
	size_t payload = 0;

	payload = crypto_check_packet_size(security, packet_size);
	if (!payload)
		return NULL;

	proto = d2d_protocol_alloc(sock, flags, security, payload);
	if (!proto)
		return NULL;

	if (d2dp_register_kobj(proto)) {
		/* now we have to destroy the protocol as a kobject */
		kobject_put(&proto->kobj);
		return NULL;
	}

	return proto;
}

static void d2dp_stop_transport(struct d2d_protocol *proto)
{
	struct d2dp_transport_threads *transport = &proto->transport;

	/* set `closing` flag and notify all */
	atomic_set_release(&proto->closing, 1);
	d2dp_wake_up_transport(proto);

	/* lock the RX thread because it may be already dead */
	mutex_lock(&transport->exit_completion_lock);

	if (transport->rx.task) {
		/* send signal to RX thread because it may sleep on `recv` */
		struct pid *pid = get_task_pid(transport->rx.task, PIDTYPE_PID);
		kill_pid(pid, SIGTERM, 1);
		put_pid(pid);
	}

	mutex_unlock(&transport->exit_completion_lock);

	wait_for_completion(&transport->tx.exit);
	wait_for_completion(&transport->rx.exit);
}

static bool __d2d_tx_buf_is_empty(struct d2d_protocol *proto)
{
	bool is_empty = false;
	struct tx_handler *tx = &proto->tx;

	spin_lock(&tx->lock);
	is_empty = tx_buf_is_empty(tx->buffer);
	spin_unlock(&tx->lock);

	return is_empty;
}

static bool __d2d_rx_buf_is_fully_acked(struct d2d_protocol *proto)
{
	u32 last_acked = 0;
	u32 rx_buf_ack = 0;
	struct tx_handler *tx = &proto->tx;
	struct rx_handler *rx = &proto->rx;

	spin_lock(&tx->lock);
	spin_lock(&rx->lock);

	rx_buf_ack = rx_buf_get_ack(rx->buffer);
	last_acked = tx->ack_state.last_acked;

	spin_unlock(&rx->lock);
	spin_unlock(&tx->lock);

	return last_acked == rx_buf_ack;
}

/**
 * d2d_protocol_destroy - the finalizer for the D2D Protocol instance
 * @proto: D2DP instance
 *
 * This function is aimed to completely destroy the protocol. It does not care
 * about TX/RX buffers status so if you need it, wait for a completion before
 * calling this function.
 *
 * This function is also registered as a `release` function for D2DP kobject
 */
void d2d_protocol_destroy(struct d2d_protocol *proto)
{
	deinit_proto_timers(&proto->timers);
	d2dp_stop_transport(proto);
	deinit_rx_handler(&proto->rx);
	deinit_tx_handler(&proto->tx);
	deallocate_frames(&proto->frames);
	vfree(proto);
}

/**
 * d2d_protocol_flush - try to flush the D2DP buffers
 * @proto: D2D Protocol instance
 *
 * Returns:
 *   0   when all packets in TX buffer were acked
 *  -EIO when packets have not been acked in D2D_DESTROY_TIMEOUT_MSEC
 */
static int d2d_protocol_flush(struct d2d_protocol *proto)
{
	int exitcode = 0;
	u32 ack_period_ms = D2D_DEFAULT_ACK_PERIOD_USEC / USEC_PER_MSEC;
	unsigned long timeout =
		jiffies + msecs_to_jiffies(D2D_DESTROY_TIMEOUT_MSEC);

	/* Do not wait when the transport failed */
	exitcode = atomic_read_acquire(&proto->exitcode);
	if (exitcode) {
		pr_err("protocol error: %d, no flush\n", exitcode);
		return -EIO;
	}

	/* Wait for the empty tx_buffer and for rx_buffer to be fully acked */
	while (!__d2d_tx_buf_is_empty(proto) ||
	       !__d2d_rx_buf_is_fully_acked(proto)) {
		exitcode = atomic_read_acquire(&proto->exitcode);
		if (exitcode) {
			pr_err("protocol error: %d, stop flush\n", exitcode);
			return -EIO;
		}

		if (time_after(jiffies, timeout)) {
			pr_err("flush timeout, stop flush\n");
			return -EIO;
		}

		msleep(ack_period_ms);
	}

	return 0;
}

int d2d_protocol_close(struct d2d_protocol *proto)
{
	int ret = 0;

	ret = d2d_protocol_flush(proto);
	kobject_put(&proto->kobj);

	return ret;
}

static bool __d2d_send_waitcond_lock(struct d2d_protocol *proto,
				     size_t bytes_wait, int *reason)
{
	int exitcode = 0;
	size_t bytes_free = 0;
	size_t total_bytes = 0;
	struct tx_handler *tx = &proto->tx;

	exitcode = atomic_read_acquire(&proto->exitcode);
	if (unlikely(exitcode)) {
		pr_err("sendwait: proto error: %d\n", exitcode);
		*reason = D2D_SEND_WAKEUP_REASON_PROTO_ERROR;
		return true;
	}

	/*
	 * It is highly unsafe to close a protocol instance while the instance
	 * is used, because use-after-free may happen. Avoid that.
	 */
	if (WARN_ON(atomic_read_acquire(&proto->closing))) {
		pr_err("sendwait: closing while used\n");
		*reason = D2D_SEND_WAKEUP_REASON_CLOSING;
		return true;
	}

	spin_lock(&tx->lock);

	/* check if requested amount of bytes to send exceeds buffer length */
	total_bytes = tx_buf_get_size(tx->buffer);
	if (bytes_wait > total_bytes) {
		spin_unlock(&tx->lock);

		pr_err("sendwait: too big buffer size to send: %zu vs %zu\n",
		       bytes_wait, total_bytes);
		*reason = D2D_SEND_WAKEUP_REASON_INVALID_SIZE;
		return true;
	}

	bytes_free = tx_buf_get_free_bytes(tx->buffer);
	if (bytes_free >= bytes_wait) {
		/*
		 * Do not release the lock if wakeup reason is OK. The lock is
		 * released later by the caller. This is required to provide
		 * atomicity for sending operations and to avoid hard to debug
		 * time-of-check/time-of-use race conditions.
		 */
		*reason = D2D_SEND_WAKEUP_REASON_OK;
		return true;
	}

	/* if no need to wakeup, release the lock */
	spin_unlock(&tx->lock);
	return false;
}

/**
 * __d2d_send_wait_lock - wait for free space in the underlying D2DP buffer
 * @proto: D2DP instance
 * @bytes: how many bytes to wait
 *
 * This function blocks until @bytes of bytes are freed in the TX buffer or
 * error happens. The return value is the reason of wakeup (ok or error).
 *
 * NOTE: this function returns with TX transport lock being held when wakeup
 * with success, otherwise, the TX transport lock is not captured.
 */
static int __d2d_send_wait_lock(struct d2d_protocol *proto, size_t bytes)
{
	int reason = D2D_SEND_WAKEUP_REASON_OK;
	int err = 0;

	err = wait_event_interruptible(proto->tx.wq_free,
				       __d2d_send_waitcond_lock(proto,
								bytes,
								&reason));
	if (err == -ERESTARTSYS && reason == D2D_SEND_WAKEUP_REASON_OK) {
		pr_err("sendwait: interrupted\n");
		reason = D2D_SEND_WAKEUP_REASON_ERESTARTSYS;
	}

	return reason;
}

static ssize_t __d2d_sendiov_locked(struct d2d_protocol *proto,
				    const struct kvec *iov, size_t iovlen,
				    size_t to_send, uint32_t flags)
{
	size_t bytes = 0;
	struct tx_handler *tx = &proto->tx;

	if (flags & D2D_SEND_FLAG_KEY_UPDATE) {
		/* check if someone is already updating the keys */
		if (atomic_read_acquire(&tx->key_update)) {
			spin_unlock(&tx->lock);
			pr_err("TX keys are being updated by someone else\n");
			return -EALREADY;
		}
	}

	bytes = tx_buf_append_iov(tx->buffer, iov, iovlen, to_send);
	if (!bytes) {
		spin_unlock(&tx->lock);
		pr_err("cannot append nodes (%zu KVECs: %zuB) to TX buffer\n",
		       iovlen, to_send);
		return -ENOMEM;
	}
	atomic_add(bytes, &proto->stats.tx_buf_bytes);

	if (flags & D2D_SEND_FLAG_KEY_UPDATE) {
		/* this should always succeed as we have just appended data */
		tx_buf_get_last_seq_id(tx->buffer, &tx->key_update_id);
		atomic_set_release(&tx->key_update, 1);
	}

	spin_unlock(&tx->lock);
	wake_up_interruptible_all(&tx->wq_data);
	return bytes;
}

/**
 * __d2d_sendiov - common entry point for all D2DP sending operations
 * @proto: D2DP instance
 * @iov: kvec array with sparse data
 * @iovlen: number of entries in @iov array
 * @length: total amount of bytes to send
 * @flags: D2DP sending flags
 *
 * This operation is atomic, either all the data is pushed to underlying buffer,
 * or no data is pushed when an error happened.
 *
 * Returns @length when success or negative error:
 * -EINVAL      when @length is longer than underlying buffer length
 * -ERESTARTSYS when the operation was interrupted by a signal
 * -ENOMEM      when the memory allocation for TX buffer failed
 * -EALREADY    when D2D_SEND_FLAG_KEY_UPDATE is set but update is in progress
 * other errors when the underlying networking socket is inoperable
 */
static ssize_t __d2d_sendiov(struct d2d_protocol *proto, const struct kvec *iov,
			     size_t iovlen, size_t length, u32 flags)
{
	int ret = 0;

	ret = __d2d_send_wait_lock(proto, length);
	switch (ret) {
	case D2D_SEND_WAKEUP_REASON_OK:
		break;
	case D2D_SEND_WAKEUP_REASON_INVALID_SIZE:
		return -EINVAL;
	case D2D_SEND_WAKEUP_REASON_PROTO_ERROR:
		return atomic_read_acquire(&proto->exitcode);
	case D2D_SEND_WAKEUP_REASON_ERESTARTSYS:
		return -ERESTARTSYS;
	case D2D_SEND_WAKEUP_REASON_CLOSING:
		return -EPIPE;
	default:
		WARN(true, "bad enum value: %d\n", ret);
		return -EINVAL;
	}

	return __d2d_sendiov_locked(proto, iov, iovlen, length, flags);
}

ssize_t d2d_send(struct d2d_protocol *proto, const void *data, size_t len,
		 u32 flags)
{
	const struct kvec iov = {
		.iov_base = (void *)data,
		.iov_len  = len,
	};

	return __d2d_sendiov(proto, &iov, 1, len, flags);
}

/*
 * TODO: make this function safe against size_t overflow
 */
static size_t d2d_iov_length(const struct kvec *iov, size_t iovlen)
{
	size_t i = 0;
	size_t ret = 0;

	for (i = 0; i < iovlen; i++)
		ret += iov[i].iov_len;

	return ret;
}

ssize_t d2d_sendiov(struct d2d_protocol *proto, const struct kvec *iov,
		    size_t iovlen, size_t to_send, u32 flags)
{
	size_t iov_total_len = 0;

	iov_total_len = d2d_iov_length(iov, iovlen);
	if (unlikely(to_send > iov_total_len)) {
		pr_err("send: invalid arg: to_send=%zuB, but kvec=%zuB\n",
		       to_send, iov_total_len);
		return -EINVAL;
	}

	return __d2d_sendiov(proto, iov, iovlen, to_send, flags);
}

static bool __d2d_recv_waitcond_lock(struct d2d_protocol *proto,
				     size_t bytes_wait, u32 flags, int *reason)
{
	int exitcode = 0;
	bool wakeup  = false;
	struct rx_handler *rx  = &proto->rx;

	exitcode = atomic_read_acquire(&proto->exitcode);
	if (unlikely(exitcode)) {
		pr_err("recvwait: proto error: %d\n", exitcode);
		*reason = D2D_RECV_WAKEUP_REASON_PROTO_ERROR;
		return true;
	}

	/*
	 * It is highly unsafe to close a protocol instance while the instance
	 * is used, because use-after-free may happen. Avoid that.
	 */
	if (WARN_ON(atomic_read_acquire(&proto->closing))) {
		pr_err("recvwait: closing while used\n");
		*reason = D2D_RECV_WAKEUP_REASON_CLOSING;
		return true;
	}

	spin_lock(&rx->lock);

	/* when D2D_RECV_FLAG_WAITALL we need to get all data in once */
	if (flags & D2D_RECV_FLAG_WAITALL) {
		size_t total_bytes = 0;
		size_t available_bytes = 0;

		total_bytes = rx_buf_get_size(rx->buffer);
		if (bytes_wait > total_bytes) {
			spin_unlock(&rx->lock);
			*reason = D2D_RECV_WAKEUP_REASON_INVALID_SIZE;
			pr_err("recvwait: requested too much: %zuB vs %zuB\n",
			       bytes_wait, total_bytes);
			return true;
		}

		available_bytes = rx_buf_get_available_bytes(rx->buffer);
		wakeup = available_bytes >= bytes_wait;
	} else {
		wakeup = rx_buf_has_data(rx->buffer);
	}

	/* do not release the lock if wakeup reason is OK */
	if (wakeup) {
		*reason = D2D_RECV_WAKEUP_REASON_OK;
		return true;
	}

	/* if no need to wakeup, release the lock */
	spin_unlock(&rx->lock);
	return false;
}

static int __d2d_recv_wait_lock_notimeout(struct d2d_protocol *proto,
					  size_t bytes, u32 flags)
{
	int ret = 0;
	int reason = D2D_RECV_WAKEUP_REASON_OK;

	ret = wait_event_interruptible(
		proto->rx.wq_data,
		__d2d_recv_waitcond_lock(proto, bytes, flags, &reason));
	if (ret == -ERESTARTSYS && reason == D2D_RECV_WAKEUP_REASON_OK) {
		pr_err("recvwait: interrupted\n");
		reason = D2D_RECV_WAKEUP_REASON_ERESTARTSYS;
	}

	return reason;
}

static int __d2d_recv_wait_lock_timeout(struct d2d_protocol *proto,
					int timeout_ms, size_t bytes, u32 flags)
{
	int ret = 0;
	int reason = D2D_RECV_WAKEUP_REASON_OK;

	ret = wait_event_interruptible_timeout(
		proto->rx.wq_data,
		__d2d_recv_waitcond_lock(proto, bytes, flags, &reason),
		msecs_to_jiffies(timeout_ms));
	if (ret == -ERESTARTSYS && reason == D2D_RECV_WAKEUP_REASON_OK) {
		pr_err("recvwait: interrupted\n");
		reason = D2D_RECV_WAKEUP_REASON_ERESTARTSYS;
	} else if (!ret && reason == D2D_RECV_WAKEUP_REASON_OK) {
		reason = D2D_RECV_WAKEUP_REASON_TIMEOUT;
	}
	return reason;
}

static int __d2d_recv_wait_lock(struct d2d_protocol *proto, int timeout_ms,
				size_t bytes, u32 flags)
{
	if (!timeout_ms)
		return __d2d_recv_wait_lock_notimeout(proto, bytes, flags);
	else
		return __d2d_recv_wait_lock_timeout(proto, timeout_ms,
						    bytes, flags);
}

static ssize_t __d2d_recviov_locked(struct d2d_protocol *proto,
				    struct kvec *iov, size_t iovlen,
				    size_t to_recv, u32 flags)
{
	size_t i = 0;
	size_t bytes_read = 0;
	struct rx_handler *rx = &proto->rx;

	for (i = 0; i < iovlen; i++) {
		size_t len   = min(iov[i].iov_len, to_recv);
		size_t bytes = rx_buf_get(rx->buffer, iov[i].iov_base, len);

		bytes_read += bytes;
		to_recv    -= bytes;
		if (!to_recv)
			break;
	}
	atomic_sub(bytes_read, &proto->stats.rx_buf_bytes);
	spin_unlock(&rx->lock);

	return bytes_read;
}

static ssize_t __d2d_recviov(struct d2d_protocol *proto, int timeout_ms,
			     struct kvec *iov, size_t iovlen, size_t length,
			     u32 flags)
{
	int ret = 0;

	ret = __d2d_recv_wait_lock(proto, timeout_ms, length, flags);
	switch (ret) {
	case D2D_RECV_WAKEUP_REASON_OK:
		break;
	case D2D_RECV_WAKEUP_REASON_PROTO_ERROR:
		return atomic_read_acquire(&proto->exitcode);
	case D2D_RECV_WAKEUP_REASON_INVALID_SIZE:
		return -EINVAL;
	case D2D_RECV_WAKEUP_REASON_ERESTARTSYS:
		return -ERESTARTSYS;
	case D2D_RECV_WAKEUP_REASON_TIMEOUT:
		return -EAGAIN;
	case D2D_RECV_WAKEUP_REASON_CLOSING:
		return -EPIPE;
	default:
		WARN(true, "bad enum value: %d\n", ret);
		return -EINVAL;
	}

	return __d2d_recviov_locked(proto, iov, iovlen, length, flags);
}

ssize_t d2d_recv(struct d2d_protocol *proto, void *data, size_t len, u32 flags)
{
	int timeout_ms = 0;
	struct kvec iov = {
		.iov_base = data,
		.iov_len  = len,
	};

	WARN_ON(d2d_get_opt(proto, D2D_OPT_RCVTIMEO, &timeout_ms));

	return __d2d_recviov(proto, timeout_ms, &iov, 1, len, flags);
}

ssize_t d2d_recviov(struct d2d_protocol *proto, struct kvec *iov,
		    size_t iovlen, size_t to_recv, u32 flags)
{
	int timeout_ms = 0;
	size_t iov_total_len = 0;

	iov_total_len = d2d_iov_length(iov, iovlen);
	if (unlikely(to_recv > iov_total_len)) {
		pr_err("recv: invalid args: to_recv=%zuB, but kvec=%zuB\n",
		       to_recv, iov_total_len);
		return -EINVAL;
	}

	WARN_ON(d2d_get_opt(proto, D2D_OPT_RCVTIMEO, &timeout_ms));

	return __d2d_recviov(proto, timeout_ms, iov, iovlen, to_recv, flags);
}

void d2dp_wake_up_transport(struct d2d_protocol *proto)
{
	wake_up_interruptible_all(&proto->tx.wq_free);
	wake_up_interruptible_all(&proto->tx.wq_data);
	wake_up_interruptible_all(&proto->rx.wq_data);
}

static int __d2d_crypto_wait_condition(struct d2d_protocol *proto)
{
	if (atomic_read_acquire(&proto->exitcode))
		return true;

	return !atomic_read_acquire(&proto->tx.key_update);
}

int __must_check d2d_crypto_tx_lock(struct d2d_protocol *proto)
{
	int exitcode = 0;
	int ret = wait_event_interruptible(proto->tx.wq_data,
					   __d2d_crypto_wait_condition(proto));

	/* exitcode overrides -ERESTARTSYS */
	exitcode = atomic_read_acquire(&proto->exitcode);
	if (exitcode) {
		pr_err("txlock: proto error: %d\n", exitcode);
		return exitcode;
	}

	/* also return if interrupted */
	if (ret) {
		pr_err("txlock: interrupted\n");
		return ret;
	}

	/* OK, KEY_UPDATE request is acknowledged, we can lock crypto */
	mutex_lock(&proto->frames.crypto_tx_mtx);

	return 0;
}

void d2d_crypto_tx_unlock(struct d2d_protocol *proto)
{
	mutex_unlock(&proto->frames.crypto_tx_mtx);
}

void d2d_crypto_rx_lock(struct d2d_protocol *proto)
{
	mutex_lock(&proto->frames.crypto_rx_mtx);
}

void d2d_crypto_rx_unlock(struct d2d_protocol *proto)
{
	mutex_unlock(&proto->frames.crypto_rx_mtx);
}
