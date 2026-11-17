/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Bastet TCP sequence sync.
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#include <net/tcp.h>
#include <net/inet_sock.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet.h>
#include <huawei_platform/net/bastet/bastet_interface.h>

#define BST_MAX_SEQ_VALUE 0xFFFFFFFF

#define BST_REQ_SOCK_SYNC_TIME (HZ / 2)
#define BST_REQ_SOCK_WAIT_SYNC_TIME (HZ*2 / 5)
#define BST_SKIP_SOCK_OWNER_TIME (HZ / 2)
#define BST_WAKELOCK_TIMEOUT (HZ / 10)
#define BST_NULL_RETURN_VALUE 1

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#ifndef tcp_jiffies32
#define tcp_jiffies32 tcp_time_stamp
#endif
#endif

enum {
	BST_TMR_EVT_INVALID = 0,
	BST_TMR_REQ_SOCK_SYNC,
	BST_TMR_SET_SOCK_SYNC,
	BST_TMR_DELAY_SOCK_SYNC,
	BST_TMR_CLOSE_SOCK,
};

enum {
	BST_USER_STOP = 0,
	BST_USER_START,
};

static DEFINE_SPINLOCK(create_bastet_lock);

void bastet_sock_seq_num_invalid(struct sock *sk)
{
	struct bst_sock_comm_prop prop;

	memset(&prop, 0, sizeof(prop));
	if (!is_ipv6_addr(sk)) {
		bastet_get_comm_prop(sk, &prop);
		post_indicate_packet(BST_IND_SN_INVALID, &prop, sizeof(prop));
	}
}

/*
 * cancel_sock_bastet_timer () - cancel the timer.
 * @sk: point to the sock.
 *
 * Cancel bastet sock timer anyway.
 *
 * Returns:No
 */
static void cancel_sock_bastet_timer(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;

	bsk->bastet_timer_event = BST_TMR_EVT_INVALID;
	sk_stop_timer(sk, &bsk->bastet_timer);
}

/*
 * setup_sock_sync_delay_timer () - setup the delay timer.
 * @sk: point to the sock.
 *
 * Setup bastet sock delay sync timer.
 *
 * Returns:No
 */
static void setup_sock_sync_delay_timer(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;

	bastet_logi("delay_time=%ld", bsk->delay_sync_time_section);
	bsk->bastet_timer_event = BST_TMR_DELAY_SOCK_SYNC;
	bsk->bastet_timeout = bsk->last_sock_active_time_point +
		bsk->delay_sync_time_section;

	sk_reset_timer(sk, &bsk->bastet_timer, bsk->bastet_timeout);
}

/*
 * process_sock_send_and_recv () - process the sock data.
 * @sk: point to the sock.
 *
 * Send out user data and Recv User data if necessary.
 *
 * Returns:No
 */
static void process_sock_send_and_recv(struct sock *sk)
{
	int mss_now;
	struct bastet_sock *bsk = sk->bastet;

	if (sk->sk_send_head != NULL) {
		mss_now = tcp_current_mss(sk);
		__tcp_push_pending_frames(sk, mss_now, tcp_sk(sk)->nonagle);
	}

	if (!skb_queue_empty(&bsk->recv_queue)) {
		struct sk_buff *skb = NULL;
		skb = __skb_dequeue(&bsk->recv_queue);
		bastet_loge("bastet backlog: begin to process bastet_receive_queue");
		while (skb) {
			sk_backlog_rcv(sk, skb);
			skb = __skb_dequeue(&bsk->recv_queue);
		}
		bsk->recv_len = 0;
	}
}

/*
 * setup_sock_sync_request_timer () - setup the timer.
 * @sk: point to the sock.
 * @retry: retry numbers.
 *
 * Setup bastet sock request sync timer,
 * Insure user sock can turn to valid after a while.
 *
 * Returns:No
 */
static void setup_sock_sync_request_timer(struct sock *sk, bool retry)
{
	struct bastet_sock *bsk = sk->bastet;

	bsk->sync_retry = retry;
	bsk->bastet_timer_event = BST_TMR_REQ_SOCK_SYNC;
	bsk->bastet_timeout = jiffies + BST_REQ_SOCK_SYNC_TIME;

	sk_reset_timer(sk, &bsk->bastet_timer, bsk->bastet_timeout);
}

static int request_sock_sync_ipv4(struct sock *sk)
{
	int err;
	struct bst_sock_comm_prop prop;

	memset(&prop, 0, sizeof(prop));

	sk->bastet->bastet_sock_state = BST_SOCK_UPDATING;
	setup_sock_sync_request_timer(sk, false);
	bastet_get_comm_prop(sk, &prop);
	err = post_indicate_packet(BST_IND_SOCK_SYNC_REQ, &prop, sizeof(prop));

	return err;
}

static int request_sock_sync_ipv6(struct sock *sk)
{
	int err;
	struct bst_sock_comm_prop_ipv6 prop;

	memset(&prop, 0, sizeof(prop));

	sk->bastet->bastet_sock_state = BST_SOCK_UPDATING;
	setup_sock_sync_request_timer(sk, false);
	bastet_get_comm_prop_ipv6(sk, &prop);
	err = post_indicate_packet(BST_IND_SOCK_SYNC_REQ_IPV6,
		&prop, sizeof(prop));

	return err;
}

/*
 * request_sock_sync () - sync the sock.
 * @sk: point to the sock.
 *
 * Sock requests sock sync when sock is invalid.
 *
 * Returns:0 means success
 * Otherwise is fail.
 */
static int request_sock_sync(struct sock *sk)
{
	int err;

	if (IS_ERR_OR_NULL(sk) || IS_ERR_OR_NULL(sk->bastet)) {
		bastet_loge("invalid parameter");
		return -EFAULT;
	}

	if (is_ipv6_addr(sk))
		err = request_sock_sync_ipv6(sk);
	else
		err = request_sock_sync_ipv4(sk);

	return err;
}

static bool do_sock_send_prepare(struct sock *sk)
{
	u8 sync_state;

	if (IS_ERR_OR_NULL(sk) || IS_ERR_OR_NULL(sk->bastet)) {
		bastet_loge("invalid parameter");
		return false;
	}

	sync_state = sk->bastet->bastet_sock_state;
	bastet_logi("sync_state:%d", sync_state);
	switch (sync_state) {
	case BST_SOCK_INVALID:
		/*
		 * We should never add lock outside switch grammar,
		 * or it will cause interlock when
		 * we call process_sock_send_and_recv.
		 */
		spin_lock_bh(&sk->sk_lock.slock);
		request_sock_sync(sk);
		spin_unlock_bh(&sk->sk_lock.slock);
		return true;
	case BST_SOCK_UPDATING:
		return true;
	default:
		return false;
	}
}

/*
 * bastet_sock_send_prepare () - prepare the send sock.
 * @sk: point to the sock.
 *
 * Judge if send data need to wait.
 * If current sock is invalid, requests socket sync and saves send data;
 * Otherwise, do nothing.
 *
 * Returns:true means success.
 *         false means false.
 */
bool bastet_sock_send_prepare(struct sock *sk)
{
	if (sk->bastet != NULL)
		return do_sock_send_prepare(sk);

	return false;
}

static inline bool bastet_rcvqueues_full(struct sock *sk,
	const struct sk_buff *skb)
{
	return ((IS_ERR_OR_NULL(skb) || IS_ERR_OR_NULL(sk) ||
		IS_ERR_OR_NULL(sk->bastet)) ? false :
		sk->bastet->recv_len + skb->truesize > sk->sk_rcvbuf);
}

static int add_rcvqueues_queue(struct sock *sk, struct sk_buff *skb)
{
	struct bst_sock_comm_prop prop;
	struct bst_sock_comm_prop_ipv6 prop_ipv6;

	memset(&prop, 0, sizeof(prop));
	memset(&prop_ipv6, 0, sizeof(prop_ipv6));

	if (bastet_rcvqueues_full(sk, skb)) {
		if (!is_ipv6_addr(sk)) {
			bastet_get_comm_prop(sk, &prop);
			post_indicate_packet(BST_IND_RCVQUEUE_FULL,
				&prop, sizeof(prop));
		} else {
			bastet_get_comm_prop_ipv6(sk, &prop_ipv6);
			post_indicate_packet(BST_IND_RCVQUEUE_FULL_IPV6,
				&prop_ipv6, sizeof(prop_ipv6));
		}
		return -ENOBUFS;
	}

	bastet_loge("bastet backlog: push the skb data into bastet_receive_queue");
	skb_dst_force(skb);
	__skb_queue_tail(&sk->bastet->recv_queue, skb);
	sk->bastet->recv_len += skb->truesize;
	return 0;
}

/*
 * request_sock_bastet_timeout () - request timeout event process
 * @sk: point to the sock.
 *
 * BST_TMR_REQ_SOCK_SYNC timeout.
 * Request sync time is up, but sock sync properties still invalid.
 * Usually, daemon should set sock sync properties before timeout.
 *
 * Returns:No
 */
static void request_sock_bastet_timeout(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;
	struct bst_sock_comm_prop prop;
	struct bst_sock_comm_prop_ipv6 prop_ipv6;

	memset(&prop, 0, sizeof(prop));
	memset(&prop_ipv6, 0, sizeof(prop_ipv6));

	/* Accurating time */
	if (time_after(bsk->bastet_timeout, jiffies)) {
		sk_reset_timer(sk, &bsk->bastet_timer, bsk->bastet_timeout);
		return;
	}

	/*
	 * We must reset timer event, bastet_delay_sock_sync_notify
	 * depends on it this code must be put after accurating time
	 */
	bsk->bastet_timer_event = BST_TMR_EVT_INVALID;

	if (bsk->bastet_sock_state != BST_SOCK_UPDATING) {
		if (!is_ipv6_addr(sk)) {
			bastet_get_comm_prop(sk, &prop);
			post_indicate_packet(BST_IND_SKSTATE_NOT_UPDATING,
				&prop, sizeof(prop));
		} else {
			bastet_get_comm_prop_ipv6(sk, &prop_ipv6);
			post_indicate_packet(BST_IND_SKSTATE_NOT_UPDATING_IPV6,
				&prop_ipv6, sizeof(prop_ipv6));
		}
		bastet_loge("proxy_id:%d state: %d not expected",
			bsk->proxy_id, bsk->bastet_sock_state);
		return;
	}

	/* Try reuqest timer again */
	if (bsk->sync_retry) {
		request_sock_sync(sk);
		return;
	}

	/*
	 * If goes here, bastet sock sync failed,
	 * Send or recv data anyway.
	 */
	if (!is_ipv6_addr(sk)) {
		bastet_get_comm_prop(sk, &prop);
		post_indicate_packet(BST_IND_SOCK_SYNC_FAILED,
			&prop, sizeof(prop));
	} else {
		bastet_get_comm_prop_ipv6(sk, &prop_ipv6);
		post_indicate_packet(BST_IND_SOCK_SYNC_FAILED_IPV6,
			&prop_ipv6, sizeof(prop_ipv6));
	}
	bastet_loge("proxy_id:%d request timeout", bsk->proxy_id);

	if (bsk->user_ctrl == BST_USER_START)
		/* Before send or recv data, set state to BST_SOCK_VALID */
		bsk->bastet_sock_state = BST_SOCK_VALID;
	else
		bsk->bastet_sock_state = BST_SOCK_NOT_USED;
}

static int post_sock_prop_ipv4(struct sock *sk)
{
	struct bst_set_sock_sync_prop sock_p;

	memset(&sock_p, 0, sizeof(sock_p));
	bastet_get_comm_prop(sk, &sock_p.guide);
	bastet_get_sock_prop(sk, &sock_p.sync_prop);

	return post_indicate_packet(BST_IND_SOCK_SYNC_PROP,
		&sock_p, sizeof(sock_p));
}

static int post_sock_prop_ipv6(struct sock *sk)
{
	struct bst_set_sock_sync_prop_ipv6 sock_p;

	memset(&sock_p, 0, sizeof(sock_p));
	bastet_get_comm_prop_ipv6(sk, &sock_p.guide);
	bastet_get_sock_prop(sk, &sock_p.sync_prop);

	return post_indicate_packet(BST_IND_SOCK_SYNC_PROP_IPV6,
		&sock_p, sizeof(sock_p));
}

static void delay_sock_bastet_timeout_ex(struct sock *sk)
{
	int err;
	struct bastet_sock *bsk = sk->bastet;
	struct bst_set_sock_sync_prop sock_p;
	struct bst_sock_comm_prop_ipv6 prop_ipv6;

	memset(&sock_p, 0, sizeof(sock_p));
	memset(&prop_ipv6, 0, sizeof(prop_ipv6));

	if (!skb_queue_empty(&sk->sk_receive_queue)) {
		if (!is_ipv6_addr(sk)) {
			bastet_get_comm_prop(sk, &sock_p.guide);
			post_indicate_packet(BST_IND_RECV_DATA_INQUEUE,
				&sock_p.guide, sizeof(sock_p.guide));
		} else {
			bastet_get_comm_prop_ipv6(sk, &prop_ipv6);
			post_indicate_packet(BST_IND_RECV_DATA_INQUEUE_IPV6,
				&prop_ipv6, sizeof(prop_ipv6));
		}
		bastet_logi("proxy_id:%d has received data in queue",
			bsk->proxy_id);
		bsk->last_sock_active_time_point = jiffies;
		setup_sock_sync_delay_timer(sk);
		post_indicate_packet(BST_IND_TRIGGER_THAW,
			&bsk->pid, sizeof(pid_t));
		return;
	}

	if (is_ipv6_addr(sk))
		err = post_sock_prop_ipv6(sk);
	else
		err = post_sock_prop_ipv4(sk);

	if (!err)
		bsk->bastet_sock_state = BST_SOCK_INVALID;
}

/*
 * delay_sock_bastet_timeout() - delay sock timeout.
 * @sk: point to the sock.
 *
 * BST_TMR_DELAY_SOCK_SYNC timeout.
 * If sock is ready, get sock sync properties and post them to daemon
 *
 * Returns:No
 */
static void delay_sock_bastet_timeout(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;
	struct bst_set_sock_sync_prop sock_p;
	struct bst_sock_comm_prop_ipv6 prop_ipv6;

	memset(&sock_p, 0, sizeof(sock_p));
	memset(&prop_ipv6, 0, sizeof(prop_ipv6));

	/* Accurating time */
	if (time_after(bsk->bastet_timeout, jiffies)) {
		sk_reset_timer(sk, &bsk->bastet_timer, bsk->bastet_timeout);
		return;
	}

	/*
	 * We must reset timer event,
	 * bastet_delay_sock_sync_notify depends on it
	 * this code must be put after accurating time
	 */
	bsk->bastet_timer_event = BST_TMR_EVT_INVALID;

	/* In repair mode or userspace needs repair, do not sync sock */
	if (unlikely(tcp_sk(sk)->repair || bsk->need_repair)) {
		bastet_loge("proxy_id:%d in repair mode", bsk->proxy_id);
		return;
	}

	if (sk->sk_state != TCP_ESTABLISHED) {
		bastet_loge("proxy_id:%d sk_state is not TCP_ESTABLISHED",
			bsk->proxy_id);
		return;
	}

	if (bsk->bastet_sock_state != BST_SOCK_VALID) {
		bastet_loge("proxy_id:%d state: %d not expected",
			bsk->proxy_id, bsk->bastet_sock_state);
		return;
	}

	/* Sock owner has used since last setup */
	if (time_after(bsk->last_sock_active_time_point +
		bsk->delay_sync_time_section, jiffies)) {
		setup_sock_sync_delay_timer(sk);
		return;
	}

	/*
	 * Sock owner has some data unacked,
	 * Coming ack would trigger delay timer again
	 */
	if (!tcp_write_queue_empty(sk)) {
		if (!is_ipv6_addr(sk)) {
			bastet_get_comm_prop(sk, &sock_p.guide);
			post_indicate_packet(BST_IND_SEND_DATA_NOTACK,
				&sock_p.guide, sizeof(sock_p.guide));
		} else {
			bastet_get_comm_prop_ipv6(sk, &prop_ipv6);
			post_indicate_packet(BST_IND_SEND_DATA_NOTACK_IPV6,
				&prop_ipv6, sizeof(prop_ipv6));
		}
		bastet_logi("proxy_id:%d has sent data not acked",
			bsk->proxy_id);
		post_indicate_packet(BST_IND_TRIGGER_THAW,
			&bsk->pid, sizeof(pid_t));
		return;
	}

	/*
	 * Sock owner has some data to recv, do not sync.
	 * If sock owner has none recv action,delay timer should be stopped.
	 */
	delay_sock_bastet_timeout_ex(sk);
}

static void bastet_store_ts_recent(struct tcp_sock *tp, u32 new_ts_recent,
	u32 new_ts_rencent_tick)
{
	tp->rx_opt.ts_recent = new_ts_recent;

	/* new_ts_rencent_tick has not changed since been passed to modem */
	if (new_ts_rencent_tick != 0) {
		long new_stamp = get_seconds() -
			((tcp_jiffies32 - new_ts_rencent_tick) / HZ);
		bastet_logi("ts_recent_stamp from %u to %u",
			tp->rx_opt.ts_recent_stamp, new_stamp);
		tp->rx_opt.ts_recent_stamp = new_stamp;
	}
}

/*
 * adjust_sock_sync_prop() - Adjust sock sync properties.
 * @sk: point to the sock.
 * @sync_p: point to the sync properties.
 *
 * There may be more than one sk_buff in tcp write queue,
 * Adjust them all.
 *
 * Returns 0 is success,
 * Otherwise is fail.
 */
static int adjust_sock_sync_prop(struct sock *sk,
	struct bst_sock_sync_prop *sync_p)
{
	struct tcp_sock *tp = NULL;
	struct sk_buff *skb = NULL;
	u32 seq_changed = 0;
	u32 new_seq;

	tp = tcp_sk(sk);
	new_seq = sync_p->seq;

	/*
	 * There may be more than one sk_buff in tcp write queue,
	 * Adjust them all.
	 */
	skb = tcp_write_queue_head(sk);
	while (skb != NULL) {
		struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);

		if (skb == tcp_write_queue_head(sk)) {
			if (new_seq < tcb->seq) {
				seq_changed = BST_MAX_SEQ_VALUE - tcb->seq;
				seq_changed += new_seq;
			} else {
				seq_changed = new_seq - tcb->seq;
			}
			new_seq = tp->write_seq + seq_changed;
		}

		tcb->seq += seq_changed;
		tcb->end_seq += seq_changed;

		if (!tcp_skb_is_last(sk, skb))
			skb = tcp_write_queue_next(sk, skb);
		else
			skb = NULL;
	}

	if (tcp_write_queue_head(sk) == NULL)
		seq_changed = new_seq - tp->write_seq;

	tp->write_seq = new_seq;
	tp->rcv_nxt = sync_p->rcv_nxt;
	tp->snd_wnd = sync_p->snd_wnd;
	tp->snd_nxt += seq_changed;

	tp->copied_seq = tp->rcv_nxt;
	tp->rcv_wup = tp->rcv_nxt;
	tp->snd_una += seq_changed;

	if (likely(tp->rx_opt.tstamp_ok)) {
		tcp_mstamp_refresh(tp);
		tp->tsoffset = sync_p->ts_current - tcp_time_stamp(tp);
		bastet_store_ts_recent(tp, sync_p->ts_recent,
			sync_p->ts_recent_tick);
	}

	bastet_logi("prop->seq=%u,prop->rcv_nxt=%u,prop->snd_wnd=%u,"
		"prop->mss=%u,prop->snd_wscale=%u,prop->rcv_wscale=%u",
		tp->write_seq, tp->rcv_nxt, tp->snd_wnd, tp->advmss,
		tp->rx_opt.snd_wscale, tp->rx_opt.rcv_wscale);
	return 0;
}

static void sock_set_internal(struct sock *sk,
	struct bst_sock_sync_prop *sync_p)
{
	struct bastet_sock *bsk = sk->bastet;

	if (bsk->bastet_sock_state == BST_SOCK_INVALID ||
		bsk->bastet_sock_state == BST_SOCK_UPDATING) {
		adjust_sock_sync_prop(sk, sync_p);
		bsk->bastet_sock_state = BST_SOCK_VALID;
		process_sock_send_and_recv(sk);
	}

	if (bsk->user_ctrl == BST_USER_STOP)
		bsk->bastet_sock_state = BST_SOCK_NOT_USED;
}

int check_ack_larger_seq(struct sock *sk, struct sk_buff *skb)
{
	u32 ack = TCP_SKB_CB(skb)->ack_seq;
	struct bst_sock_sync_prop sync_prop;

	ack = TCP_SKB_CB(skb)->ack_seq;
	bastet_get_sock_prop(sk, &sync_prop);
	if (after(ack, sync_prop.seq)) {
		bastet_logi("begin to repaire the ack larger than ack");
		return -EPERM;
	}
	return 0;
}

void fix_sync_with_ack(struct sock *sk, struct sk_buff *skb)
{
	u32 ack;
	struct bastet_sock *bsk = NULL;

	ack = TCP_SKB_CB(skb)->ack_seq;
	bsk = sk->bastet;
	if (bsk == NULL) {
		bastet_loge("can not get bastet from sk");
		return;
	}
	if (bsk->bastet_timer_event == BST_TMR_SET_SOCK_SYNC
		&& bsk->sync_p != NULL) {
		bsk->sync_p->seq = ack;
		bastet_loge("success to change the sync_p.seq with ack");
		return;
	}

	bastet_loge("fail to change the sync seq by tcp ack");
	return;
}

/*
 * bastet_sock_recv_prepare () - prepare the sock.
 * @sk: point to the sock.
 * @skb: point to the buffer of sock.
 *
 * Judge if recv data need to wait.
 * If current sock is invalid, requests socket sync and saves recv data;
 * Otherwise, do nothing.
 *
 * Returns:true means success.
 *         false means false.
 */
/*lint -e666*/
bool bastet_sock_recv_prepare(struct sock *sk, struct sk_buff *skb)
{
	u8 sync_state;
	struct bastet_sock *bsk = sk->bastet;

	if (bsk == NULL)
		return false;

	sync_state = bsk->bastet_sock_state;
	bastet_logi("sync_state:%d", sync_state);
	switch (sync_state) {
	case BST_SOCK_INVALID:
		bsk->bastet_sock_state = BST_SOCK_UPDATING;

		/* Set retry true, if timeout, request sock sync */
		setup_sock_sync_request_timer(sk, true);
		/* fall-through */
	case BST_SOCK_UPDATING:
		if (check_ack_larger_seq(sk, skb)) {
			fix_sync_with_ack(sk, skb);
			bastet_logi("ack lager than seq, begin to repaire before send");
		}
		if (unlikely(add_rcvqueues_queue(sk, skb))) {
			__NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPBACKLOGDROP);
			kfree_skb(skb);
		}
		return true;
	default:
		return false;
	}
}
/*lint +e666*/

/*
 * set_sock_bastet_timeout() - set the sock timeout.
 * @sk: point to the sock.
 * if the event is BST_TMR_SET_SOCK_SYNC timeout, this method
 * will be invoked
 * Returns No
 */
static void set_sock_bastet_timeout(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;
	struct bst_sock_comm_prop prop;
	struct bst_sock_comm_prop_ipv6 prop_ipv6;

	memset(&prop, 0, sizeof(prop));
	memset(&prop_ipv6, 0, sizeof(prop_ipv6));

	bsk->bastet_timer_event = BST_TMR_EVT_INVALID;

	if (bsk->sync_p == NULL) {
		if (!is_ipv6_addr(sk)) {
			bastet_get_comm_prop(sk, &prop);
			post_indicate_packet(BST_IND_NOPENDING_SK_SET,
				&prop, sizeof(prop));
		} else {
			bastet_get_comm_prop_ipv6(sk, &prop_ipv6);
			post_indicate_packet(BST_IND_NOPENDING_SK_SET_IPV6,
				&prop_ipv6, sizeof(prop_ipv6));
		}
		bastet_loge("proxy_id:%d not expected null sync prop",
			bsk->proxy_id);
		return;
	}

	sock_set_internal(sk, bsk->sync_p);

	kfree(bsk->sync_p);
	bsk->sync_p = NULL;
}

/*
 * set_sock_close_internal() - set the internal sock close.
 * @sk: point to the sock.
 *
 * Read tcp_reset in tcp_input.c for reference.
 *
 * Returns No
 */
static void set_sock_close_internal(struct sock *sk)
{
	switch (sk->sk_state) {
	case TCP_SYN_SENT:
		sk->sk_err = ECONNREFUSED;
		break;
	case TCP_CLOSE_WAIT:
		sk->sk_err = EPIPE;
		break;
	case TCP_CLOSE:
		return;
	default:
		sk->sk_err = ECONNRESET;
	}
	/* This barrier is coupled with smp_rmb() in tcp_poll() */
	smp_wmb();

	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_error_report(sk);

	tcp_done(sk);
}

/*
 * close_sock_bastet_timeout() - close sock timeout.
 * @sk: point to the sock.
 *
 * if the event is BST_TMR_CLOSE_SOCK timeout, this method
 * will be invoked.
 *
 * Returns No
 */
static void close_sock_bastet_timeout(struct sock *sk)
{
	sk->bastet->bastet_timer_event = BST_TMR_EVT_INVALID;

	set_sock_close_internal(sk);
}

/*
 * bastet_sock_bastet_timeout() - timeout timer process.
 * @data: message data.
 *
 * Bastet sock timeout, include all bastet time events.
 *
 * Returns No
 */
static void bastet_sock_bastet_timeout(unsigned long data)
{
	int event;
	struct sock *sk = (struct sock *)data;
	struct bastet_sock *bsk = sk->bastet;

	bastet_logi("proxy_id:%d time event: %d", bsk->proxy_id,
		bsk->bastet_timer_event);

	bh_lock_sock(sk);

	/* Include in lock */
	event = bsk->bastet_timer_event;

	if (sock_owned_by_user(sk)) {
		/* Try again later */
		sk_reset_timer(sk, &bsk->bastet_timer,
			jiffies + BST_SKIP_SOCK_OWNER_TIME);
		goto out_unlock;
	}

	switch (event) {
	case BST_TMR_REQ_SOCK_SYNC:
		request_sock_bastet_timeout(sk);
		break;
	case BST_TMR_SET_SOCK_SYNC:
		set_sock_bastet_timeout(sk);
		break;
	case BST_TMR_DELAY_SOCK_SYNC:
		delay_sock_bastet_timeout(sk);
		break;
	case BST_TMR_CLOSE_SOCK:
		close_sock_bastet_timeout(sk);
		break;
	default:
		bastet_loge("proxy_id:%d invalid time event: %d",
			bsk->proxy_id, event);
		break;
	}

	sk_mem_reclaim(sk);
out_unlock:
	bh_unlock_sock(sk);
	sock_put(sk);
}

/*
 * bastet_delay_sock_sync_notify() - notify the bastet delay
 * @sk: sock pointer
 *
 * When sock owner uses sock, we should not sync sock for a while.
 *
 * Return:no
 */
void bastet_delay_sock_sync_notify(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;

	if (!is_bastet_enabled())
		return;

	if (bsk != NULL && sk->sk_state == TCP_ESTABLISHED) {
		bsk->last_sock_active_time_point = jiffies;
		if (bsk->bastet_timer_event != BST_TMR_DELAY_SOCK_SYNC &&
			bsk->bastet_sock_state == BST_SOCK_VALID &&
			bsk->user_ctrl == BST_USER_START)
			setup_sock_sync_delay_timer(sk);
	}
}

/*
 * bastet_sock_repair_prepare_notify() - notify the bastet sock repair
 * @sk: sock pointer
 * @val:to judge if it is need repair.
 *
 * CRIU's socket repair would stop bastet function
 *
 * Returns true, if repair should be stopped.
 * Otherwise return false.
 */
bool bastet_sock_repair_prepare_notify(struct sock *sk, int val)
{
	if (!is_bastet_enabled())
		return false;

	if (!sk || !sk->bastet)
		return false;

	sk->bastet->need_repair = val;

	if (val)
		/* Userspace requires repair */
		return do_sock_send_prepare(sk);

	/* Userspace restore repair */
	spin_lock_bh(&sk->sk_lock.slock);
	bastet_delay_sock_sync_notify(sk);
	spin_unlock_bh(&sk->sk_lock.slock);
	return false;
}

/*
 * set_sock_sync_hold_time() - set the sync socket timer.
 * @hold_delay: sync delay timer value.
 *
 * Check the sync delay timer's value.if it large then orig
 * delay value then caluate the expire value. finally reset the timer.
 *
 * Returns true, if set success.
 * Otherwise return false.
 */
int set_sock_sync_hold_time(struct bst_set_sock_sync_delay hold_delay)
{
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	unsigned long orig_delay;
	unsigned long expire;

	sk = get_sock_by_fd_pid(hold_delay.guide.fd, hold_delay.guide.pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			hold_delay.guide.fd, hold_delay.guide.pid);
		return -ENOENT;
	}
	bsk = sk->bastet;
	if (bsk == NULL) {
		bastet_loge("not expected bastet null");
		sock_put(sk);
		return -EPERM;
	}
	bastet_logi("hold_time=%u", hold_delay.hold_time);
	orig_delay = bsk->delay_sync_time_section;
	bsk->delay_sync_time_section = msecs_to_jiffies(hold_delay.hold_time);
	if (timer_pending(&bsk->bastet_timer)) {
		if (orig_delay < bsk->delay_sync_time_section) {
			bastet_logi("screen off to on");
			expire = bsk->last_sock_active_time_point +
				bsk->delay_sync_time_section;
		} else {
			bastet_logi("screen on to off");
			if (time_after(jiffies,
				bsk->last_sock_active_time_point +
				bsk->delay_sync_time_section)) {
				bastet_logi("need to timeout right now");
				expire = jiffies;
			} else {
				expire = bsk->last_sock_active_time_point +
					bsk->delay_sync_time_section;
			}
		}
		bsk->bastet_timeout = expire;
		sk_reset_timer(sk, &bsk->bastet_timer, expire);
	}
	sock_put(sk);

	return 0;
}

/*
 * create_bastet_sock() - create bastet sock
 * @sk: sock pointer
 * @init_prop: sock parameter which should be set.
 *
 * Generate bastet sock in sock struct.
 *
 * Returns 0, if create sock success.
 * Otherwise return false.
 */
static int create_bastet_sock(struct sock *sk,
	struct bst_set_sock_sync_delay *init_prop)
{
	struct bastet_sock *bsk = NULL;

	bsk = kzalloc(sizeof(*bsk), GFP_ATOMIC);
	if (bsk == NULL) {
		bastet_loge("kmalloc failed");
		return -ENOMEM;
	}
	/* clear bsk memory */
	memset(bsk, 0, sizeof(*bsk));

	bsk->bastet_sock_state = BST_SOCK_NOT_USED;
	bsk->bastet_timer_event = BST_TMR_EVT_INVALID;
	bsk->user_ctrl = BST_USER_STOP;
	bsk->fd = init_prop->guide.fd;
	bsk->pid = init_prop->guide.pid;
	bsk->proxy_id = init_prop->proxy_id;

	/* if wifi proxy is closed, then is_wifi MUST be false */
	bsk->is_wifi = init_prop->is_wifi;
	bsk->bastet_timeout = 0;
	bsk->last_sock_active_time_point = jiffies;
	bsk->sync_p = NULL;
	bsk->need_repair = 0;
	bsk->delay_sync_time_section = msecs_to_jiffies(init_prop->hold_time);

	setup_timer(&bsk->bastet_timer,
		bastet_sock_bastet_timeout, (unsigned long)sk);

	skb_queue_head_init(&bsk->recv_queue);
	bsk->recv_len = 0;
	init_waitqueue_head(&bsk->wq);
	bsk->flag = false;
	atomic_set(&bsk->hbm.reply_filter_cnt, 0);
	atomic_set(&bsk->hbm.reply_matched_cnt, 0);
	atomic_set(&bsk->hbm.frozen, 0);
	spin_lock_init(&bsk->hbm.hbm_lock);

	sk->bastet = bsk;
	return 0;
}

static int do_start_bastet_sock(struct sock *sk,
	struct bst_set_sock_sync_delay *init_prop)
{
	int err = 0;
	struct bastet_sock *bsk = sk->bastet;

	spin_lock(&create_bastet_lock);
	if (bsk == NULL) {
		err = create_bastet_sock(sk, init_prop);
		if (err < 0) {
			spin_unlock(&create_bastet_lock);
			return err;
		}

		bsk = sk->bastet;
	}
	spin_unlock(&create_bastet_lock);
	bsk->last_sock_active_time_point = jiffies;

	spin_lock_bh(&sk->sk_lock.slock);

	bsk->user_ctrl = BST_USER_START;

	if (bsk->bastet_sock_state == BST_SOCK_INVALID ||
		bsk->bastet_sock_state == BST_SOCK_UPDATING) {
		err = -EPERM;
		goto out_unlock;
	}

	bsk->bastet_sock_state = BST_SOCK_VALID;

	setup_sock_sync_delay_timer(sk);

out_unlock:
	spin_unlock_bh(&sk->sk_lock.slock);
	return err;
}

/*
 * start_bastet_sock() - starts bastet sock
 * @init_prop: sock parameter which should be set.
 *
 * Function is called when application starts bastet
 * proxy hearbeat.
 *
 * Returns 0, if start sock success.
 * Otherwise return false.
 */
int start_bastet_sock(struct bst_set_sock_sync_delay *init_prop)
{
	int err;
	struct sock *sk = NULL;
	struct bst_sock_id *guide = &init_prop->guide;

	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}

	if (sk->sk_state != TCP_ESTABLISHED) {
		bastet_loge("sk_state is not TCP_ESTABLISHED");
		sock_put(sk);
		return -EPERM;
	}

	if (sk->sk_protocol != IPPROTO_TCP) {
		bastet_loge("sk_protocol is not IPPROTO_TCP");
		sock_put(sk);
		return -EPERM;
	}

	if (sk->sk_type != SOCK_STREAM) {
		bastet_loge("sk_type is not SOCK_STREAM");
		sock_put(sk);
		return -EPERM;
	}

	if (tcp_sk(sk)->repair) {
		bastet_loge("sk in repair mode");
		sock_put(sk);
		return -EPERM;
	}

	err = do_start_bastet_sock(sk, init_prop);
	sock_put(sk);
	return err;
}

/*
 * stop_bastet_sock() - stops bastet sock
 * @guide: sock information,include pid and fd.
 *
 * Function is called when application stops bastet proxy.
 *
 * Returns 0, if stops sock success.
 * Otherwise return false.
 */
int stop_bastet_sock(struct bst_sock_id *guide)
{
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	u8 sync_state;

	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}

	bsk = sk->bastet;
	if (bsk == NULL) {
		bastet_loge("not expected bastet null");
		sock_put(sk);
		return -EPERM;
	}

	spin_lock_bh(&sk->sk_lock.slock);

	bsk->user_ctrl = BST_USER_STOP;

	sync_state = bsk->bastet_sock_state;
	switch (sync_state) {
	case BST_SOCK_VALID:
		bsk->bastet_sock_state = BST_SOCK_NOT_USED;
		cancel_sock_bastet_timer(sk);
		break;
	case BST_SOCK_INVALID:
		request_sock_sync(sk);
		break;
	default:
		break;
	}

	spin_unlock_bh(&sk->sk_lock.slock);

	sock_put(sk);
	return 0;
}

/*
 * prepare_bastet_sock() - prepare the bastet sock info.
 * @sync_prop: sync sock information,include delay time and proxy id.
 *
 * Function is called when application prepare bastet proxy.
 *
 * Returns 0, if prepare sock success.
 * Otherwise return false.
 */
int prepare_bastet_sock(struct bst_set_sock_sync_delay *sync_prop)
{
	int err = 0;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;

	sk = get_sock_by_fd_pid(sync_prop->guide.fd, sync_prop->guide.pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			sync_prop->guide.fd, sync_prop->guide.pid);
		return -ENOENT;
	}

	if (sk->sk_state != TCP_ESTABLISHED) {
		bastet_loge("sk_state is not TCP_ESTABLISHED");
		sock_put(sk);
		return -EPERM;
	}

	if (tcp_sk(sk)->repair) {
		bastet_loge("sk in repair mode");
		sock_put(sk);
		return -EPERM;
	}

	bsk = sk->bastet;
	spin_lock(&create_bastet_lock);
	if (bsk == NULL) {
		err = create_bastet_sock(sk, sync_prop);
		if (err < 0) {
			sock_put(sk);
			spin_unlock(&create_bastet_lock);
			return err;
		}
		bsk = sk->bastet;
	}
	spin_unlock(&create_bastet_lock);
	sock_put(sk);

	return err;
}

/*
 * setup_sock_sync_set_timer() - Setup sock set timer.
 * @sk: sock pointer
 * @sync_prop: sync sock information,include delay time and proxy id.
 *
 * Function is called when sock was owned by the current user
 *
 * Returns 0, if setup success.
 * Otherwise return false.
 */
static int setup_sock_sync_set_timer(struct sock *sk,
	struct bst_sock_sync_prop *sync_p)
{
	struct bastet_sock *bsk = sk->bastet;

	bsk->sync_p = kzalloc(sizeof(*sync_p), GFP_KERNEL);
	if (bsk->sync_p == NULL)
		return -ENOMEM;

	memcpy(bsk->sync_p, sync_p, sizeof(*sync_p));
	bsk->bastet_timer_event = BST_TMR_SET_SOCK_SYNC;

	sk_reset_timer(sk, &bsk->bastet_timer,
		jiffies + BST_SKIP_SOCK_OWNER_TIME);
	return 0;
}

static int setup_wait_sock_sync_set_timer(struct sock *sk,
	struct bst_sock_sync_prop *sync_p)
{
	struct bastet_sock *bsk = sk->bastet;

	bsk->sync_p = kzalloc(sizeof(*sync_p), GFP_KERNEL);
	if (bsk->sync_p == NULL)
		return -ENOMEM;

	if (bsk->bastet_sock_state == BST_SOCK_INVALID)
		bsk->bastet_sock_state = BST_SOCK_UPDATING;

	memcpy(bsk->sync_p, sync_p, sizeof(*sync_p));
	bsk->bastet_timer_event = BST_TMR_SET_SOCK_SYNC;

	sk_reset_timer(sk, &bsk->bastet_timer,
		jiffies + BST_REQ_SOCK_WAIT_SYNC_TIME);
	bastet_logi("setup_wait_sock_sync_set_timer = 400ms");
	return 0;
}

/*
 * get_sk_and_bsk() - get the sk and bsk.
 * @guide: the sock common properites, such as ip and port value.
 * @sk: the address of sock pointer.
 * @bsk: the address of bsk pointer.
 *
 * Function is called by set_tcp_sock_closed(), set_tcp_sock_sync_prop(),
 * bastet_sync_prop_start(), to cut short the duplicated code. Basiclly,
 * this method will find the sk and bsk, check the state.
 *
 * Returns 0, get sk & bsk success, and the state is ok.
 * -ENOENT, not found the sock.
 * -EPERM, time wait sock.
 * EPERM, not expected bastet null.
 */
static int get_sk_and_bsk(struct bst_sock_comm_prop *guide,
	struct sock **sk, struct bastet_sock **bsk)
{
	int ret = 0;

	*sk = get_sock_by_comm_prop(guide);
	if (*sk == NULL) {
		post_indicate_packet(BST_IND_GET_SK_FAILED,
			guide, sizeof(*guide));
		bastet_loge("can not find sock by lport: %d, lIp: ***,"
			"rport: %d, rIp: ***",
			guide->local_port, guide->remote_port);
		return -ENOENT;
	}

	if ((*sk)->sk_state == TCP_TIME_WAIT) {
		if (!is_ipv6_addr(*sk))
			post_indicate_packet(BST_IND_SOCK_STATE_WAIT,
				guide, sizeof(*guide));
		bastet_loge("not expected time wait sock");
		inet_twsk_put(inet_twsk(*sk));
		return -EPERM;
	}

	*bsk = (*sk)->bastet;
	if (*bsk == NULL) {
		if (!is_ipv6_addr(*sk))
			post_indicate_packet(BST_IND_GET_BSK_FAILED,
				guide, sizeof(*guide));
		bastet_loge("not expected bastet null");
		/*
		 * here should return the 1, not -EPERM.
		 * use this value to judge if it is need to go to out_put.
		 */
		return BST_NULL_RETURN_VALUE;
	}

	/* the normal return. value is 0. */
	return ret;
}
static int get_sk_and_bsk_ipv6(struct bst_sock_comm_prop_ipv6 *guide,
	struct sock **sk, struct bastet_sock **bsk)
{
	int ret = 0;

	*sk = get_sock_by_comm_prop_ipv6(guide);
	if (*sk == NULL) {
		bastet_loge("ipv6 can not find sock by lport: %d,"
			"lIp: ***, rport: %d, rIp: ***",
			guide->local_port, guide->remote_port);
		return -ENOENT;
	}

	if ((*sk)->sk_state == TCP_TIME_WAIT) {
		if (is_ipv6_addr(*sk))
			post_indicate_packet(BST_IND_SOCK_STATE_WAIT_IPV6,
				guide, sizeof(*guide));
		bastet_loge("ipv6 not expected time wait sock");
		inet_twsk_put(inet_twsk(*sk));
		return -EPERM;
	}

	*bsk = (*sk)->bastet;
	if (*bsk == NULL) {
		if (is_ipv6_addr(*sk))
			post_indicate_packet(BST_IND_GET_BSK_FAILED_IPV6,
				guide, sizeof(*guide));
		bastet_loge("ipv6 not expected bastet null");

		/*
		 * here should return the 1, not -EPERM.
		 * use this value to judge if it is need to go to out_put.
		 */
		return BST_NULL_RETURN_VALUE;
	}

	bastet_logi("get ipv6 sk and bastet");

	/* the normal return. value is 0. */
	return ret;
}

/*
 * set_tcp_sock_sync_prop() - Set sock sync properties.
 * @set_prop: the sock sync properties.include bst_sock_comm_prop
 *
 * Function is called by bastet ioctl(), when it start to set the sock
 * properties.
 *
 * Returns 0, if set success.
 * Otherwise return false.
 */
int set_tcp_sock_sync_prop(struct bst_set_sock_sync_prop *set_prop)
{
	int err = 0;
	int ret;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	struct bst_sock_comm_prop *guide = &set_prop->guide;

	ret = get_sk_and_bsk(guide, &sk, &bsk);
	if (ret == -ENOENT || ret == -EPERM)
		return ret;

	if (ret == BST_NULL_RETURN_VALUE) {
		err = -EPERM;
		goto out_put;
	}
	spin_lock_bh(&sk->sk_lock.slock);

	if (bsk->sync_p != NULL) {
		if (!is_ipv6_addr(sk))
			post_indicate_packet(BST_IND_PENDING_SK_SET,
				guide, sizeof(*guide));
		bastet_loge("proxy_id:%d has a pending sock set", bsk->proxy_id);
		err = -EPERM;
		goto out_unlock;
	}
	print_bastet_sock_seq(&set_prop->sync_prop);
	cancel_sock_bastet_timer(sk);

	if (sock_owned_by_user(sk)) {
		err = setup_sock_sync_set_timer(sk, &set_prop->sync_prop);
		goto out_unlock;
	}

	err = setup_wait_sock_sync_set_timer(sk, &set_prop->sync_prop);

out_unlock:
	spin_unlock_bh(&sk->sk_lock.slock);

	adjust_traffic_flow_by_sock(sk,
		set_prop->sync_prop.tx, set_prop->sync_prop.rx);

out_put:
	sock_put(sk);
	return err;
}

int set_tcp_sock_sync_prop_ipv6(
	struct bst_set_sock_sync_prop_ipv6 *set_prop)
{
	int err = 0;
	int ret;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	struct bst_sock_comm_prop_ipv6 *guide = NULL;

	if (set_prop == NULL)
		return -EPERM;

	guide = &set_prop->guide;
	print_bastet_sock_seq(&set_prop->sync_prop);
	ret = get_sk_and_bsk_ipv6(guide, &sk, &bsk);
	if (ret == -ENOENT || ret == -EPERM)
		return ret;

	if (ret == BST_NULL_RETURN_VALUE) {
		err = -EPERM;
		goto out_put;
	}
	spin_lock_bh(&sk->sk_lock.slock);

	if (bsk->sync_p != NULL) {
		if (is_ipv6_addr(sk))
			post_indicate_packet(BST_IND_PENDING_SK_SET_IPV6,
				guide, sizeof(*guide));
		bastet_loge("proxy_id:%d has a pending sock set", bsk->proxy_id);
		err = -EPERM;
		goto out_unlock;
	}

	cancel_sock_bastet_timer(sk);

	if (sock_owned_by_user(sk)) {
		err = setup_sock_sync_set_timer(sk, &set_prop->sync_prop);
		goto out_unlock;
	}

	err = setup_wait_sock_sync_set_timer(sk, &set_prop->sync_prop);

out_unlock:
	spin_unlock_bh(&sk->sk_lock.slock);

	adjust_traffic_flow_by_sock(sk,
		set_prop->sync_prop.tx, set_prop->sync_prop.rx);

out_put:
	sock_put(sk);
	return err;
}

/*
 * bastet_sync_prop_cancel() - cancel set sync properties
 * @sk: sock pointer
 *
 * Function is called by sock state handle. if the sock is vaild.
 *
 * Returns No.
 */
void bastet_sync_prop_cancel(struct sock *sk)
{
	if (sk->bastet) {
		sk->bastet->user_ctrl = BST_USER_STOP;
		cancel_sock_bastet_timer(sk);
	}
}

/*
 * bastet_sync_prop_start() - start sync properties
 * @set_prop: the sock sync properties.include bst_sock_comm_prop
 *
 * Function is called by bastet ioctl.check the sock state. then set
 * the properties.
 *
 * Returns 0, if start success.
 * Otherwise return false.
 */
int bastet_sync_prop_start(struct bst_set_sock_sync_prop *set_prop)
{
	int err = 0;
	int ret;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	struct bst_sock_comm_prop *guide = &set_prop->guide;

	ret = get_sk_and_bsk(guide, &sk, &bsk);
	if (ret == -ENOENT || ret == -EPERM)
		return ret;

	if (ret == BST_NULL_RETURN_VALUE) {
		err = -EPERM;
		goto out_put;
	}
	if (bsk->sync_p != NULL) {
		if (!is_ipv6_addr(sk))
			post_indicate_packet(BST_IND_PENDING_SK_SET,
				guide, sizeof(*guide));
		bastet_loge("proxy_id:%d has a pending sock set", bsk->proxy_id);
		err = -EPERM;
		goto out;
	}

	sock_set_internal(sk, &set_prop->sync_prop);
	bastet_sync_prop_cancel(sk);
	bsk->flag = true;
	bastet_logi("wake up bastet wq");
	wake_up_interruptible(&bsk->wq);

out:
	adjust_traffic_flow_by_sock(sk,
		set_prop->sync_prop.tx, set_prop->sync_prop.rx);

out_put:
	sock_put(sk);
	return err;
}

int bastet_sync_prop_start_ipv6(
	struct bst_set_sock_sync_prop_ipv6 *set_prop)
{
	int err = 0;
	int ret;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	struct bst_sock_comm_prop_ipv6 *guide = NULL;

	if (set_prop == NULL)
		return -EPERM;

	guide = &set_prop->guide;
	ret = get_sk_and_bsk_ipv6(guide, &sk, &bsk);
	if (ret == -ENOENT || ret == -EPERM)
		return ret;

	if (ret == BST_NULL_RETURN_VALUE) {
		err = -EPERM;
		goto out_put;
	}
	if (bsk->sync_p != NULL) {
		if (is_ipv6_addr(sk))
			post_indicate_packet(BST_IND_PENDING_SK_SET_IPV6,
				guide, sizeof(*guide));
		bastet_loge("ipv6 proxy_id:%d has a pending sock set",
			bsk->proxy_id);
		err = -EPERM;
		goto out;
	}

	sock_set_internal(sk, &set_prop->sync_prop);
	bastet_sync_prop_cancel(sk);
	bsk->flag = true;
	bastet_logi("ipv6 wake up bastet wq");
	wake_up_interruptible(&bsk->wq);

out:
	adjust_traffic_flow_by_sock(sk,
		set_prop->sync_prop.tx, set_prop->sync_prop.rx);

out_put:
	sock_put(sk);
	return err;
}

/*
 * bastet_sync_prop_stop() - stop sync properties
 * @comm_prop: the socket common properties.include ip and port.
 *
 * Function is called by bastet ioctl.check the sock state. then stop
 * the bastet sync.
 *
 * Returns 0, if start success.
 * Otherwise return false.
 */
int bastet_sync_prop_stop(struct bst_sock_comm_prop *comm_prop)
{
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;

	sk = get_sock_by_comm_prop(comm_prop);
	if (sk == NULL) {
		bastet_loge("can not find sock by lport: %d, lIp: ***,"
			"rport: %d, rIp: ***", comm_prop->local_port,
			comm_prop->remote_port);
		return -ENOENT;
	}
	if (sk->sk_state == TCP_TIME_WAIT) {
		bastet_loge("not expected time wait sock");
		inet_twsk_put(inet_twsk(sk));
		return -EPERM;
	}
	bsk = sk->bastet;
	if (bsk) {
		if (bsk->bastet_sock_state != BST_SOCK_NOT_USED) {
			bsk->user_ctrl = BST_USER_START;
			bsk->bastet_sock_state = BST_SOCK_VALID;
			setup_sock_sync_delay_timer(sk);
		}
	}

	sock_put(sk);
	return 0;
}

int bastet_sync_prop_stop_ipv6(
	struct bst_sock_comm_prop_ipv6 *comm_prop)
{
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;

	if (comm_prop == NULL)
		return -EPERM;

	sk = get_sock_by_comm_prop_ipv6(comm_prop);
	if (sk == NULL) {
		bastet_loge("ipv6 can not find sock by lport:%d,"
			"lIp: ***, rport: %d, rIp: ***", comm_prop->local_port,
			comm_prop->remote_port);
		return -ENOENT;
	}
	bastet_logi("ipv6 sync prop stop");
	if (sk->sk_state == TCP_TIME_WAIT) {
		bastet_loge("ipv6 not expected time wait sock");
		inet_twsk_put(inet_twsk(sk));
		return -EPERM;
	}
	bsk = sk->bastet;
	if (bsk) {
		if (bsk->bastet_sock_state != BST_SOCK_NOT_USED) {
			bsk->user_ctrl = BST_USER_START;
			bsk->bastet_sock_state = BST_SOCK_VALID;
			setup_sock_sync_delay_timer(sk);
		}
	}

	sock_put(sk);
	return 0;
}

/*
 * get_tcp_sock_comm_prop() - get tcp properties
 * @get_prop the socket common properties.include ip and port.
 *
 * Function is called by bastet ioctl.get the tcp socket common
 * properties such as ip,port.(local and remote)
 *
 * Returns 0, if get success.
 * Otherwise return false.
 */
int get_tcp_sock_comm_prop(struct bst_get_sock_comm_prop *get_prop)
{
	struct sock *sk = NULL;
	struct bst_sock_id *guide = &get_prop->guide;

	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}
	if (sk->sk_state != TCP_ESTABLISHED) {
		bastet_loge("sk_state not expected");
		sock_put(sk);
		return -EPERM;
	}
	bastet_get_comm_prop(sk, &get_prop->comm_prop);
	sock_put(sk);
	return 0;
}

int get_tcp_sock_comm_prop_ipv6(
	struct bst_get_sock_comm_prop_ipv6 *get_prop)
{
	struct sock *sk = NULL;
	struct bst_sock_id *guide = NULL;

	if (get_prop == NULL)
		return -EPERM;

	guide = &get_prop->guide;

	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}

	if (sk->sk_state != TCP_ESTABLISHED) {
		bastet_loge("sk_state not expected");
		sock_put(sk);
		return -EPERM;
	}

	bastet_get_comm_prop_ipv6(sk, &get_prop->comm_prop);
	sock_put(sk);
	return 0;
}

static void setup_sock_sync_close_timer(struct sock *sk)
{
	struct bastet_sock *bsk = sk->bastet;

	bsk->bastet_timer_event = BST_TMR_CLOSE_SOCK;

	sk_reset_timer(sk, &bsk->bastet_timer,
		jiffies + BST_SKIP_SOCK_OWNER_TIME);
}

/*
 * set_tcp_sock_closed() - set tcp sock closed
 * @guide: the socket common properties.include ip and port.
 *
 * Close sock, when modem bastet fails this sock.
 *
 * Returns 0, if set success.
 * Otherwise return false.
 */
int set_tcp_sock_closed(struct bst_sock_comm_prop *guide)
{
	int err = 0;
	int ret;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;

	ret = get_sk_and_bsk(guide, &sk, &bsk);
	if (ret == -ENOENT || ret == -EPERM)
		return ret;

	if (ret == BST_NULL_RETURN_VALUE) {
		err = -EPERM;
		goto out_put;
	}
	spin_lock_bh(&sk->sk_lock.slock);

	/*
	 * wifi network type do not check bastet socket sync state
	 * if wifi proxy is closed, then is_wifi MUST be false
	 */
	if (!bsk->is_wifi)
		if (bsk->bastet_sock_state != BST_SOCK_INVALID
			&& bsk->bastet_sock_state != BST_SOCK_UPDATING) {
			bastet_loge(
				"proxyid:%d sync_current_state: %d not expecte",
				bsk->proxy_id, bsk->bastet_sock_state);
			goto out_unlock;
		}

	cancel_sock_bastet_timer(sk);
	bsk->bastet_sock_state = BST_SOCK_NOT_USED;
	if (sock_owned_by_user(sk)) {
		setup_sock_sync_close_timer(sk);
		goto out_unlock;
	}

	set_sock_close_internal(sk);

out_unlock:
	spin_unlock_bh(&sk->sk_lock.slock);

out_put:
	sock_put(sk);
	return err;
}

int set_tcp_sock_closed_ipv6(struct bst_sock_comm_prop_ipv6 *guide)
{
	int err = 0;
	int ret;
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;

	if (guide == NULL)
		return -EPERM;

	ret = get_sk_and_bsk_ipv6(guide, &sk, &bsk);
	if (ret == -ENOENT || ret == -EPERM)
		return ret;

	if (ret == BST_NULL_RETURN_VALUE) {
		err = -EPERM;
		goto out_put;
	}
	spin_lock_bh(&sk->sk_lock.slock);

	/*
	 * wifi network type do not check bastet socket sync state
	 * if wifi proxy is closed, then is_wifi MUST be false
	 */
	if (!bsk->is_wifi)
		if (bsk->bastet_sock_state != BST_SOCK_INVALID &&
			bsk->bastet_sock_state != BST_SOCK_UPDATING) {
			bastet_loge(
				"ipv6 proxy_id:%d sync_cur_state:%d unexpected",
				bsk->proxy_id, bsk->bastet_sock_state);
			goto out_unlock;
		}

	cancel_sock_bastet_timer(sk);
	bsk->bastet_sock_state = BST_SOCK_NOT_USED;
	if (sock_owned_by_user(sk)) {
		setup_sock_sync_close_timer(sk);
		goto out_unlock;
	}

	set_sock_close_internal(sk);

out_unlock:
	spin_unlock_bh(&sk->sk_lock.slock);

out_put:
	sock_put(sk);
	return err;
}

static void bastet_sock_release_ipv4(struct sock *sk)
{
	struct bst_close_sock_prop prop;
	struct bastet_sock *bsk = NULL;

	bsk = sk->bastet;
	memset(&prop, 0, sizeof(prop));
	if (!is_bastet_enabled())
		return;

	if (bsk != NULL) {
		bastet_get_comm_prop(sk, &prop.comm_prop);
		prop.proxy_id = bsk->proxy_id;
		/*
		 * for wifi proxy, clear heartbeat reply content
		 * if wifi proxy is closed, then is_wifi MUST be false
		 */
		if (bsk->is_wifi)
			bastet_clear_hb_reply(bsk);

		bastet_logi("indicate socket close, proxyId=0x%08X",
			prop.proxy_id);
		post_indicate_packet(BST_IND_SOCK_CLOSED, &prop, sizeof(prop));

		if (bsk->sync_p != NULL) {
			kfree(bsk->sync_p);
			bsk->sync_p = NULL;
		}

		kfree(sk->bastet);
		sk->bastet = NULL;
	}

	if (sk->reconn) {
		bastet_logi("kfree sk reconn");
		kfree(sk->reconn);
		sk->reconn = NULL;
	}
}

static void bastet_sock_release_ipv6(struct sock *sk)
{
	struct bst_close_sock_prop_ipv6 prop;
	struct bastet_sock *bsk = NULL;

	bsk = sk->bastet;
	memset(&prop, 0, sizeof(prop));
	if (!is_bastet_enabled())
		return;

	if (bsk != NULL) {
		bastet_get_comm_prop_ipv6(sk, &prop.comm_prop);
		prop.proxy_id = bsk->proxy_id;

		/*
		 * for wifi proxy, clear heartbeat reply content
		 * if wifi proxy is closed, then is_wifi MUST be false
		 */
		if (bsk->is_wifi)
			bastet_clear_hb_reply(bsk);

		bastet_logi("indicate socket close, proxyId=0x%08X",
			prop.proxy_id);
		post_indicate_packet(BST_IND_SOCK_CLOSED_IPV6, &prop,
			sizeof(prop));

		if (bsk->sync_p != NULL) {
			kfree(bsk->sync_p);
			bsk->sync_p = NULL;
		}

		kfree(sk->bastet);
		sk->bastet = NULL;
	}

	if (sk->reconn) {
		bastet_logi("kfree sk reconn");
		kfree(sk->reconn);
		sk->reconn = NULL;
	}
}

/*
 * bastet_sock_release() - release bastet socket.
 * @sk: the socket pointer.
 *
 * Release bastet sock and Post sock close information.
 *
 * Returns No
 */
void bastet_sock_release(struct sock *sk)
{
	if (IS_ERR_OR_NULL(sk)) {
		bastet_loge("invalid parameter");
		return;
	}

	if (is_ipv6_addr(sk))
		bastet_sock_release_ipv6(sk);
	else
		bastet_sock_release_ipv4(sk);
}

/*
 * get_bastet_sock_state() - get bastet socket state.
 * @sk: the socket pointer.
 *
 * get the bastet socket state by check the sk->bastet.
 *
 * Returns sock state
 * if not exist, BST_SOCK_NOT_CREATED
 */
int get_bastet_sock_state(struct sock *sk)
{
	return sk->bastet ?
		sk->bastet->bastet_sock_state : BST_SOCK_NOT_CREATED;
}

/*
 * get_tcp_bastet_sock_state() - get tcp bastet socket state.
 * @get_prop: the socket state pointer.
 *
 * get the tcp bastet socket state, the value returned from
 * get_bastet_sock_state will filled the .sync_state
 *
 * Returns 0, get success
 * Otherwise failed.
 */
int get_tcp_bastet_sock_state(struct bst_get_bastet_sock_state *get_prop)
{
	struct sock *sk = NULL;
	struct bst_sock_id *guide = &get_prop->guide;

	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}

	get_prop->sync_state = get_bastet_sock_state(sk);

	sock_put(sk);
	return 0;
}
