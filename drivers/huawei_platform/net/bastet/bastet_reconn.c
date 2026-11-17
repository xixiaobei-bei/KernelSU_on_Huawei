/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Bastet TCP reconnection.
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#include <linux/file.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet.h>

extern int inet_release(struct socket *sock);

struct sk_reconn {
	struct list_head list;
	struct file *reconn_file;
	struct socket *old_sock;
	struct socket *orig_sock;
	struct sock *tmp_sk;
	struct bastet_reconn *rc;
	struct bst_sock_comm_prop comm;
};

struct list_head sk_reconn_list;

void bastet_reconn_init(void)
{
	INIT_LIST_HEAD(&sk_reconn_list);
}

/*
 * bastet_reconn_config() - config the bastet reconn stru.
 * @sk: pointer of sock.
 * @val: config value.
 *
 * check the val,if equals to zero or, the reconn was already enable.
 * the method will exit, otherwise kmalloc the memory to create the
 * reconn data.
 *
 * Return:No
 */
void bastet_reconn_config(struct sock *sk, int val)
{
	struct bastet_reconn *reconn = NULL;

	if (IS_ERR_OR_NULL(sk)) {
		bastet_loge("invalid parameter");
		return;
	}

	bastet_logi("val=%d", val);
	if (sk->reconn) {
		if (sk->reconn->enable == val)
			return;
	} else {
		if (val == 0)
			return;

		bastet_logi("new struct bastet_reconn");
		reconn = kzalloc(sizeof(*reconn), GFP_KERNEL);
		if (reconn == NULL) {
			bastet_loge("kmalloc struct bastet_reconn failed");
			return;
		}
		memset(reconn, 0, sizeof(*reconn));
		sk->reconn = reconn;
		sk->reconn->flag = true;
		sk->reconn->err_close = false;
		atomic_set(&sk->reconn->handover, 0);
	}

	sk->reconn->enable = (val != 0) ? true : false;
	if (sk->reconn->auto_connect) {
		if (sk->reconn->enable) {
			init_waitqueue_head(&sk->reconn->wq);
		} else {
			if (!sk->reconn->flag)
				wake_up_interruptible(&sk->reconn->wq);
		}
	}
}

static void bastet_notify_close(struct sock *sk, enum bst_ind_type type)
{
	struct bst_sock_comm_prop comm_prop;
	int ret;

	bastet_logi("type:%d", type);
	ret = bastet_get_comm_prop(sk, &comm_prop);
	if (ret != 0) {
		bastet_loge("Failed to get comm prop, ret=%d", ret);
		return;
	}
	post_indicate_packet(type, &comm_prop, sizeof(comm_prop));
}

static void bastet_notify_close_ipv6(struct sock *sk, enum bst_ind_type type)
{
	struct bst_sock_comm_prop_ipv6 comm_prop;
	int ret;

	bastet_logi("type=%d", type);
	ret = bastet_get_comm_prop_ipv6(sk, &comm_prop);
	if (ret != 0) {
		bastet_loge("Failed to get comm prop, ret=%d", ret);
		return;
	}
	post_indicate_packet(type, &comm_prop, sizeof(comm_prop));
}

static bool check_sk_reconn(struct sk_reconn *reconn,
	struct bst_sock_comm_prop comm)
{
	bool ret = false;
#ifdef CONFIG_SECURITY
	if (reconn && reconn->tmp_sk && reconn->tmp_sk->sk_security) {
		if (memcmp(&reconn->comm, &comm, sizeof(comm)) == 0)
			ret = true;
	}
#endif
	return ret;
}

/*
 * bastet_reconn_failed() - reconnection failed.
 * @prop: socket common info, such as ip and port.
 *
 * iterator the sk_reconn_list, then wake up the interruptible.
 *
 * Return:No
 */
void bastet_reconn_failed(struct bst_sock_comm_prop prop)
{
	struct sk_reconn *reconn = NULL;
	struct bastet_reconn *rc = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	list_for_each_safe(p, n, &sk_reconn_list) {
		reconn = list_entry(p, struct sk_reconn, list);
		if (check_sk_reconn(reconn, prop)) {
			rc = reconn->rc;
			rc->flag = true;
			bastet_logi("reconn break, wake up reconn wq");
			wake_up_interruptible(&rc->wq);
			return;
		}
	}
	bastet_logi("cannot found valid sk_reconn, reset socket");
}

/*
 * bastet_reconn_proxyid_set() - set the proxyid.
 * @id: reconn id.
 *
 * if found the sock by fd&pid, the put the proxy id
 * into sk->reconn->proxy_id.
 *
 * Return: 0, success
 * Otherwise fail.
 */
int bastet_reconn_proxyid_set(struct reconn_id id)
{
	struct sock *sk = NULL;
	struct bastet_reconn *rc = NULL;

	sk = get_sock_by_fd_pid(id.guide.fd, id.guide.pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d pid: %d",
			id.guide.fd, id.guide.pid);
		return -ENOENT;
	}
	rc = sk->reconn;
	if (rc == NULL) {
		bastet_loge("bastet_reconn is NULL");
		sock_put(sk);
		return -ENOENT;
	}
	rc->proxy_id = id.proxy_id;
	rc->auto_connect = id.auto_connect;
	sock_put(sk);

	return 0;
}

/*
 * the following error code is not really error close
 * -EINTR: Interrupted system call
 * -EAGAIN: Try again
 * -ERESTARTSYS: Normal close
 */
static inline bool is_socket_err_close(int err)
{
	return ((err > 0 || err == -EINTR || err == -EAGAIN ||
		err == -ERESTARTSYS) ? false : true);
}

/*
 * bastet_inet_release() - release the inet.
 * @sk: point to the sock.
 *
 * check the sock state, close the related source.
 * set the flag to false.
 *
 * Return:No
 */
void bastet_inet_release(struct sock *sk)
{
	if (IS_ERR_OR_NULL(sk)) {
		bastet_loge("invalid parameter");
		return;
	}

	if (sk->reconn && !sk->reconn->err_close) {
		if (is_ipv6_addr(sk)) {
			bastet_notify_close_ipv6(sk,
				BST_IND_SOCK_NORMAL_CLOSE_IPV6);
		} else {
			bastet_notify_close(sk, BST_IND_SOCK_NORMAL_CLOSE);
		}
	}

	if (sk->prio_channel) {
		if (is_ipv6_addr(sk)) {
			bastet_notify_close_ipv6(sk,
				BST_IND_PRIO_SOCK_CLOSE_IPV6);
		} else {
			bastet_notify_close(sk, BST_IND_PRIO_SOCK_CLOSE);
		}
		sk->prio_channel = false;
	}
}

/*
 * bastet_block_recv() - receive the block data.
 * @sock: point to the socket.
 * @msg: received message.
 * @size: the message size.
 * @flags: message flag.
 * @addr_len: address length.
 * @err: error number.
 *
 * check the sock state, close the related source.
 * set the flag to false.
 *
 * Return: err <= 0, means faile.
 * Otherwise is success.
 */
int bastet_block_recv(struct socket *sock, struct msghdr *msg,
	size_t size, int flags, int *addr_len, int err)
{
	if (err > 0 || !sock->sk)
		/*
		 * check whether receive data is
		 * heartbeat reply in userspace app recv thread
		 */
		return bastet_check_hb_reply(sock->sk, msg, err);

	if (sock->sk->prio_channel && is_socket_err_close(err)) {
		bastet_notify_close(sock->sk, BST_IND_PRIO_SOCK_CLOSE);
		sock->sk->prio_channel = false;
	}
	if (!sock->sk->reconn) {
		if (sock->sk->bastet && is_socket_err_close(err))
			bastet_notify_close(sock->sk, BST_IND_SOCK_ERR_CLOSE);
	} else {
		if (is_socket_err_close(err)) {
			/* net device down returns -ETIMEDOUT */
			if (err == -ETIMEDOUT)
				bastet_notify_close(sock->sk,
					BST_IND_SOCK_TIMEDOUT);
			else
				bastet_notify_close(sock->sk,
					BST_IND_SOCK_DISCONNECT);

			sock->sk->reconn->err_close = true;
		}
	}

	return err;
}
