/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: Bastet heartbeat reply content management.
 * Author: zhuweichen@huawei.com
 * Create: 2015-09-01
 */

#include <linux/file.h>
#include <linux/net.h>
#include <net/sock.h>

#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet.h>

/*
 * bastet_send_msg() - send data by socket
 * @guide: pid and fd to find sock
 * @data: data to be sent
 * @len: the length of data
 *
 * find sock according to fd and pid, then send data from kernel directly
 *
 * Return: 0, success
 *         negative, failed
 */
int bastet_send_msg(struct bst_sock_id *guide, uint8_t *data, uint32_t len)
{
	int ret;
	struct sock *sk = NULL;
	struct msghdr msg;
	struct kvec iov[1];

	/* parameters check */
	if (guide == NULL || data == NULL || len == 0) {
		bastet_loge("invalid parameters");
		return -EFAULT;
	}
	/* find sock according to fd & pid */
	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}
	memset(&msg, 0, sizeof(msg));
	memset(&iov, 0, sizeof(iov));
	msg.msg_flags  |= MSG_DONTWAIT;
	iov[0].iov_base = data;
	iov[0].iov_len  = len;

	/* send data from kernel directly */
	ret = kernel_sendmsg(sk->sk_socket, &msg, iov, 1, len);
	if (ret != len) {
		bastet_loge("kernel_sendmsg failed, ret=%d", ret);
		sock_put(sk);
		return -EFAULT;
	}
	sock_put(sk);

	return 0;
}

/*
 * bastet_set_hb_reply() - set heartbeat reply.
 * @guide: pid and fd to find sock
 * @data: heartbeat reply content
 * @len: the length of heartbeat reply content
 *
 * set heartbeat reply content to struct sock.
 *
 * Return: 0, success
 *         negative, failed
 */
int bastet_set_hb_reply(struct bst_sock_id *guide, uint8_t *data, uint32_t len)
{
	struct sock *sk = NULL;
	struct bastet_sock *bsk = NULL;
	int err = 0;

	/* parameters check */
	if (guide == NULL || data == NULL || len == 0) {
		bastet_loge("Invalid input parameters");
		err = -EFAULT;
		goto out;
	}

	/* find sock according to fd & pid */
	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		err = -ENOENT;
		goto out;
	}

	bsk = sk->bastet;
	if (!bsk) {
		bastet_loge("no bastet in sock");
		err = -ENOENT;
		goto out_put;
	}
	spin_lock_bh(&bsk->hbm.hbm_lock);
	if (bsk->hbm.reply_content != NULL) {
		if ((len == bsk->hbm.reply_len) &&
			(memcmp(bsk->hbm.reply_content, data,
			bsk->hbm.reply_len) == 0)) {
			bastet_logi("heartbeat reply content is exist");
			spin_unlock_bh(&bsk->hbm.hbm_lock);
			goto out_put;
		} else {
			/*
			 * heartbeat reply content is different,
			 * clear original one
			 */
			bastet_logi("update heartbeat reply content");
			kfree(bsk->hbm.reply_content);
			bsk->hbm.reply_content = NULL;
			bsk->hbm.reply_len = 0;
		}
	}

	bsk->hbm.reply_len = len;
	bsk->hbm.reply_content = data;
	spin_unlock_bh(&bsk->hbm.hbm_lock);
	bastet_logi("add heartbeat reply content");
out_put:
	sock_put(sk);
out:
	return err;
}

/*
 * bastet_clear_hb_reply() - clear the heartbeat reply.
 * @bsk: bastet sock which in struct sock
 *
 * clear heartbeat reply content when sock release
 *
 * Return: No
 */
void bastet_clear_hb_reply(struct bastet_sock *bsk)
{
	if (bsk && bsk->hbm.reply_content) {
		bastet_logi("kfree heartbeat reply content");
		kfree(bsk->hbm.reply_content);
		bsk->hbm.reply_content = NULL;
		bsk->hbm.reply_len = 0;
	}
}

/*
 * bastet_notify_reply_recv() - notify the reply.
 * @sk: struct sock pointer
 * @type: bastet indication type
 *
 * notify bastetd socket has received heartbeat reply packet
 *
 * Return:No
 */
static void bastet_notify_reply_recv(struct sock *sk, enum bst_ind_type type)
{
	struct bst_sock_comm_prop comm_prop;
	int ret;

	ret = bastet_get_comm_prop(sk, &comm_prop);
	if (ret != 0) {
		bastet_loge("failed to get comm prop, ret=%d", ret);
		return;
	}
	post_indicate_packet(type, &comm_prop, sizeof(comm_prop));
}

/*
 * bastet_drop_hb_reply() - drop the heartbeat reply.
 * @sk: struct sock pointer
 * @msg: receive message header
 * @len: length of receive message header
 *
 * if receive data is heartbeat reply packet, then drop it by return -EINTR
 *
 * Return: -EINTR, drop packet
 *         others, socket original recv return value
 */
static int bastet_drop_hb_reply(struct sock *sk, struct msghdr *msg, int len)
{
	struct bastet_sock *bsk = sk->bastet;
	int offset;
	int ret = len;
	bool not_matched = 1;
	unsigned int min_len;

	if (len < 0)
		return -EFAULT;
	if ((msg == NULL) || (bsk == NULL))
		return -EFAULT;
	if (!access_ok(VERIFY_READ, msg->msg_iter.iov->iov_base, len))
		return -EFAULT;
	min_len = len > bsk->hbm.reply_len ? bsk->hbm.reply_len : len;
	offset = bsk->hbm.reply_offset;

	if (bsk->hbm.reply_content) {
		if (msg->msg_iter.iov->iov_base != NULL)
			not_matched = memcmp(bsk->hbm.reply_content + offset,
				msg->msg_iter.iov->iov_base, min_len);
		if (!not_matched) {
			bsk->hbm.reply_offset += len;
			/*
			 * check whether heartbeat reply
			 * matching has been completed
			 */
			if (bsk->hbm.reply_offset == bsk->hbm.reply_len) {
				bsk->hbm.reply_offset = 0;
				atomic_dec(&bsk->hbm.reply_matched_cnt);
			}
			/*
			 * return fake interrupt error to
			 * make userspace ignores received heartbeat reply
			 */
			ret = -EINTR;
		}
	}

	return ret;
}

/*
 * bastet_check_hb_reply() - check the heartbeat reply.
 * @sk: struct sock pointer
 * @msg: receive message header
 * @err:recv return value
 *
 * check heartbeat reply data in recv thread
 *
 * Return: -EINTR, drop packet
 *         others, socket original recv return value
 */
inline int bastet_check_hb_reply(struct sock *sk, struct msghdr *msg, int err)
{
	/* check wifi proxy socket when heartbeat reply matched */
	return ((is_wifi_proxy(sk) &&
		(atomic_read(&sk->bastet->hbm.reply_matched_cnt) > 0)) ?
		bastet_drop_hb_reply(sk, msg, err) : err);
}

/*
 * bastet_match_hb_reply() - match the heartbeat reply
 * @sk: struct sock pointer
 * @data: receive data content
 * @len: length of receive data content
 *
 * compare heartbeat reply content,if matched,notify heartbeat to relay.
 * if not matched, then notify heartbeat.
 *
 * Return:No
 */
static void bastet_match_hb_reply(struct sock *sk, uint8_t *data, int len)
{
	struct bastet_sock *bsk = sk->bastet;

	if (!data || len <= 0) {
		bastet_loge("Invalid parameters");
		return;
	}
	if (bsk->hbm.reply_content) {
		/* compare heartbeat reply content */
		if (memcmp(bsk->hbm.reply_content, data, len) == 0) {
			bastet_logi("len: %d", len);
			/* notify bastetd heartbeat reply packet has received */
			bastet_notify_reply_recv(sk, BST_IND_HB_REPLY_RECV);
			atomic_dec(&bsk->hbm.reply_filter_cnt);
			atomic_inc(&sk->bastet->hbm.reply_matched_cnt);
		} else if (atomic_read(&sk->bastet->hbm.frozen) > 0) {
			/*
			 * it is not heartbeat reply,
			 * notify bastetd to thaw the process
			 */
			atomic_set(&sk->bastet->hbm.frozen, 0);
			post_indicate_packet(BST_IND_TRIGGER_THAW,
				&sk->bastet->pid, sizeof(pid_t));
		}
	}
}

/*
 * bastet_mark_hb_reply() - mark the heartbeat reply.
 * @sk: struct sock pointer
 * @skb: struct socket buffer pointer
 * @len: length of struct socket buffer
 *
 * mark receive data as heartbeat reply packet in network stack thread.
 *
 * Return:No
 */
void bastet_mark_hb_reply(struct sock *sk, struct sk_buff *skb, int len)
{
	/* only check wifi proxy socket */
	if (is_wifi_proxy(sk)) {
		/*
		 * only has sent heartbeat packet
		 * needs match heartbeat reply packet
		 */
		if (atomic_read(&sk->bastet->hbm.reply_filter_cnt) > 0) {
			bastet_match_hb_reply(sk,
				skb->data + len, skb->len - len);
		} else if (atomic_read(&sk->bastet->hbm.frozen) > 0) {
			/*
			 * it is not heartbeat reply,
			 * notify bastetd to thaw the process
			 */
			atomic_set(&sk->bastet->hbm.frozen, 0);
			post_indicate_packet(BST_IND_TRIGGER_THAW,
				&sk->bastet->pid, sizeof(pid_t));
		}
	}
}

/*
 * bastet_filter_hb_reply() - filter the heartbeat reply.
 * @guide: pid and fd to find sock
 *
 * find sock according to fd & pid, if found then invoke
 * sock_put().
 *
 * Return: 0, success
 *         negative, failed
 */
int bastet_filter_hb_reply(struct bst_sock_id *guide)
{
	struct sock *sk = NULL;
	int err = 0;

	if (guide == NULL) {
		bastet_loge("Invalid input parameters");
		return -EFAULT;
	}
	/* find sock according to fd & pid */
	sk = get_sock_by_fd_pid(guide->fd, guide->pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			guide->fd, guide->pid);
		return -ENOENT;
	}
	if (is_wifi_proxy(sk)) {
		bastet_logi("filter heartbeat reply");
		atomic_inc(&sk->bastet->hbm.reply_filter_cnt);
	}
	sock_put(sk);

	return err;
}
