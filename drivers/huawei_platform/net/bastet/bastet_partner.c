/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014. All rights reserved.
 * Description: Bastet intereact with user space bastetd.
 *
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/ip.h>

#include <huawei_platform/net/bastet/bastet_utils.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif

#define BASTET_UID_APP 10000

struct bastet_partner_sock {
	struct list_head list;
	struct sock *sk;
	struct bst_sock_comm_prop comm;
	bool identified;
};

struct bastet_partner_uid {
	struct list_head list;
	struct list_head socks_head;
	uid_t uid;
	bool identified;
};

static spinlock_t partner_uids_lock;
static struct list_head partner_uids_head;

static inline bool invalid_uid(uid_t uid)
{
	/* if uid less than 10000, it is not an Android apk */
	return (uid < BASTET_UID_APP);
}

static int add_sock_info(struct list_head *socks_head,
	struct sock *sk, bool need_report)
{
	struct bastet_partner_sock *sock_node = NULL;
	struct list_head *p = NULL, *n = NULL;
	struct bst_sock_comm_prop comm;
	uid_t uid = sock_i_uid(sk).val;

	if (!list_empty(socks_head)) {
		/* check if there is same sock in the list */
		list_for_each_safe(p, n, socks_head) {
			sock_node = list_entry(p,
				struct bastet_partner_sock, list);
			if (sock_node->sk == sk) {
				bastet_logi("sk is already in the list");
				return 0;
			}
		}
	}
	/* create a new sock_node */
	sock_node = kmalloc(sizeof(*sock_node),
		GFP_ATOMIC);
	if (sock_node == NULL) {
		bastet_loge("Failed to kmalloc struct bastet_partner_sock");
		return -ENOMEM;
	}
	bastet_get_comm_prop(sk, &comm);
	sock_node->comm = comm;
	sock_node->sk = sk;
	sock_node->identified = false;
	list_add_tail(&sock_node->list, socks_head);
	bastet_logi("add sock_node to list, uid=%d", uid);
	if (need_report) {
		post_indicate_packet(BST_IND_SOCK_STATE_CHANGED,
			&uid, sizeof(uid_t));
	}

	return 0;
}

static void bastet_check_connect(struct sock *sk)
{
	uid_t uid = sock_i_uid(sk).val;
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p_head = NULL;
	struct list_head *n_head = NULL;

	if (invalid_uid(uid))
		return;

	spin_lock_bh(&partner_uids_lock);
	if (!list_empty(&partner_uids_head)) {
		/* check if there is same uid in the list */
		list_for_each_safe(p_head, n_head, &partner_uids_head) {
			uid_node = list_entry(p_head,
				struct bastet_partner_uid, list);
			if (uid_node->uid == uid) {
				/*
				 * If socket is identified, then it is
				 * no need to report.
				 */
				add_sock_info(&uid_node->socks_head,
					sk, !uid_node->identified);
				break;
			}
		}
	}
	spin_unlock_bh(&partner_uids_lock);
}

static bool del_sock_info(struct sock *sk,
	struct list_head *socks_head, bool need_report)
{
	struct bastet_partner_sock *sock_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	bool ret = false;
	uid_t uid = 0;

	if (!list_empty(socks_head)) {
		/* check if there is same sock in the list */
		list_for_each_safe(p, n, socks_head) {
			sock_node = list_entry(p,
				struct bastet_partner_sock, list);
			if (sock_node->sk == sk) {
				if (sock_node->identified)
					ret = true;

				list_del(&sock_node->list);
				uid = sock_i_uid(sk).val;
				if (need_report || ret)
					post_indicate_packet(
						BST_IND_SOCK_STATE_CHANGED,
						&uid, sizeof(uid_t));

				bastet_logi(
					"kfree struct bastet_partner_sock,uid=%u",
					uid);
				kfree(sock_node);
				break;
			}
		}
	}

	return ret;
}

static void bastet_check_disconnect(struct sock *sk)
{
	uid_t uid = sock_i_uid(sk).val;
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	if (invalid_uid(uid))
		return;

	spin_lock_bh(&partner_uids_lock);
	if (!list_empty(&partner_uids_head)) {
		/* check if there is same uid in the list */
		list_for_each_safe(p, n, &partner_uids_head) {
			uid_node = list_entry(p,
				struct bastet_partner_uid, list);
			if (uid_node->uid != uid)
				continue;
			/*
			 * If socket is identified,
			 * then it is no need to report.
			 */
			if (del_sock_info(sk, &uid_node->socks_head,
				!uid_node->identified))
				uid_node->identified = false;
			break;
		}
	}
	spin_unlock_bh(&partner_uids_lock);
}

void bastet_check_partner(struct sock *sk, int state)
{
	uid_t uid = sock_i_uid(sk).val;

	if (invalid_uid(uid) || !is_proxy_available())
		return;

	switch (state) {
	case TCP_ESTABLISHED:
		bastet_check_connect(sk);
		break;
	case TCP_FIN_WAIT1:
	case TCP_FIN_WAIT2:
	case TCP_TIME_WAIT:
	case TCP_CLOSE:
	case TCP_CLOSE_WAIT:
	case TCP_LAST_ACK:
	case TCP_CLOSING:
		bastet_check_disconnect(sk);
		break;
	default:
		break;
	}
}

static bool check_sock_valid(int fd, pid_t pid,
	struct bst_sock_comm_prop *comm)
{
	bool ret = false;
	struct sock *sk = NULL;

	sk = get_sock_by_fd_pid(fd, pid);
	if (sk == NULL)
		goto out;

	if (sk->sk_state != TCP_ESTABLISHED)
		goto out_put;
	if (sk->sk_socket == NULL)
		goto out_put;
	if (sk->sk_socket->type != SOCK_STREAM)
		goto out_put;

	bastet_get_comm_prop(sk, comm);
	if (comm->remote_ip == 0 && comm->remote_port == 0)
		goto out_put;

	ret = true;
out_put:
	sock_put(sk);
out:
	return ret;
}

static bool set_sock_identified(struct bst_sock_comm_prop *comm,
	struct list_head *socks_head)
{
	struct bastet_partner_sock *sock_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	bool ret = false;

	if (!list_empty(socks_head)) {
		/* check if there is same comm prop in the list */
		list_for_each_safe(p, n, socks_head) {
			sock_node = list_entry(p,
				struct bastet_partner_sock, list);
			if (memcpy(&sock_node->comm, comm,
				sizeof(*comm)) == 0) {
				sock_node->identified = true;
				ret = true;
				break;
			}
		}
	}

	return ret;
}

static void set_uid_identified(uid_t uid,
	struct bst_sock_comm_prop *comm)
{
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	spin_lock_bh(&partner_uids_lock);
	if (!list_empty(&partner_uids_head)) {
		/* check if there is same uid in the list */
		list_for_each_safe(p, n, &partner_uids_head) {
			uid_node = list_entry(p,
				struct bastet_partner_uid, list);
			if (uid_node->uid == uid) {
				if (set_sock_identified(comm,
					&uid_node->socks_head))
					uid_node->identified = true;
			}
		}
	}
	spin_unlock_bh(&partner_uids_lock);
}

static int check_found_sock_result(bool found,
	struct bst_monitor_ind_prop prop, uid_t uid)
{
	if (!found) {
		bastet_loge("no sock found, exit");
		return -ENOENT;
	}
	prop.uid = uid;
	bastet_logi("Found uid=%d has one tcp sock, pid=%d, fd=%d",
		prop.uid, prop.sock_id.pid, prop.sock_id.fd);
	set_uid_identified(uid, &prop.comm);

	return post_indicate_packet(BST_IND_UID_SOCK_PROP,
		&prop, sizeof(prop));
}

static int post_unique_sock_prop(uid_t uid,
	int32_t tid_num, int32_t *tids)
{
	struct task_struct *task = NULL;
	struct files_struct *files = NULL;
	struct fdtable *fdt = NULL;
	struct bst_monitor_ind_prop prop;
	struct bst_sock_comm_prop comm;
	int i, fd;
	bool found = false;

	for (i = 0; i < tid_num; i++) {
		rcu_read_lock();
		task = find_task_by_vpid(tids[i]);
		if (!task) {
			rcu_read_unlock();
			return -EFAULT;
		}
		get_task_struct(task);
		rcu_read_unlock();
		files = task->files;
		if (files == NULL) {
			put_task_struct(task);
			return -EFAULT;
		}
		put_task_struct(task);
		fdt = files_fdtable(files);
		for (fd = 0; fd < fdt->max_fds; fd++) {
			if (!check_sock_valid(fd, tids[i], &comm))
				continue;
			if (!found) {
				prop.comm = comm;
				prop.sock_id.fd = fd;
				prop.sock_id.pid = tids[i];
				found = true;
			} else {
				if ((fd == prop.sock_id.fd) &&
					(comm.local_port ==
					prop.comm.local_port))
					continue;

				bastet_loge("More than one socket found fd=%d, pid=%d", fd, tids[i]);
				return -EEXIST;
			}
		}
	}

	return check_found_sock_result(found, prop, uid);
}

static int check_uid_sock(struct app_monitor_prop *prop, int32_t *tids)
{
	if (invalid_uid(prop->uid) || IS_ERR_OR_NULL(tids))
		return -EINVAL;

	if (prop->tid_count > 0) {
		/* if pid has one tcp sock */
		post_unique_sock_prop(prop->uid, prop->tid_count, tids);
	}

	return 0;
}

static int add_uid(uid_t uid)
{
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	if (invalid_uid(uid))
		return -EINVAL;

	spin_lock_bh(&partner_uids_lock);
	/* check if there is same uid in the list */
	list_for_each_safe(p, n, &partner_uids_head) {
		uid_node = list_entry(p, struct bastet_partner_uid, list);
		if (uid_node->uid == uid) {
			bastet_logi("uid=%u is already in the list", uid);
			spin_unlock_bh(&partner_uids_lock);
			return 0;
		}
	}
	/* create a new uid_node */
	uid_node = kmalloc(sizeof(*uid_node), GFP_ATOMIC);
	if (uid_node == NULL) {
		spin_unlock_bh(&partner_uids_lock);
		bastet_loge("Failed to kmalloc struct bastet_partner_uid");
		return -ENOMEM;
	}
	uid_node->uid = uid;
	uid_node->identified = false;
	INIT_LIST_HEAD(&uid_node->socks_head);
	list_add_tail(&uid_node->list, &partner_uids_head);
	bastet_logi("add uid=%d", uid);
	spin_unlock_bh(&partner_uids_lock);

	return 0;
}

static void del_socks(struct list_head *socks_head)
{
	struct bastet_partner_sock *sock_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	if (!list_empty(socks_head)) {
		/* clear all sock_node in uid_node */
		list_for_each_safe(p, n, socks_head) {
			sock_node = list_entry(p,
				struct bastet_partner_sock, list);
			list_del(&sock_node->list);
			bastet_logi("kfree struct bastet_partner_sock, uid=%d",
				from_kuid(&init_user_ns,
				sock_i_uid(sock_node->sk)));
			kfree(sock_node);
		}
	}
}

static int del_uid(uid_t uid)
{
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	if (invalid_uid(uid))
		return -EINVAL;

	spin_lock_bh(&partner_uids_lock);
	if (!list_empty(&partner_uids_head)) {
		/* check if there is same uid in the list */
		list_for_each_safe(p, n, &partner_uids_head) {
			uid_node = list_entry(p,
				struct bastet_partner_uid, list);
			if (uid_node->uid == uid) {
				del_socks(&uid_node->socks_head);
				list_del(&uid_node->list);
				kfree(uid_node);
				bastet_logi("kfree struct bastet_partner_uid uid=%u", uid);
				break;
			}
		}
	}
	spin_unlock_bh(&partner_uids_lock);

	return 0;
}

/**
 * bastet_partner_process_cmd() - process the command.
 * @prop: pid and fd to find sock
 * @tids: the tid of the sock.
 *
 * to check the uid or add/delete the uid due to different commands.
 *
 * Return: 0, success
 * Otherwise is failed.
 */
int bastet_partner_process_cmd(struct app_monitor_prop *prop, int32_t *tids)
{
	int rc = 0;

	if (IS_ERR_OR_NULL(prop)) {
		bastet_loge("Invalid struct app_monitor_prop");
		return -EINVAL;
	}

	switch (prop->cmd) {
	case CMD_CHECK_UID_SOCK:
		rc = check_uid_sock(prop, tids);
		break;
	case CMD_ADD_UID:
		rc = add_uid(prop->uid);
		break;
	case CMD_DEL_UID:
		rc = del_uid(prop->uid);
		break;
	default:
		bastet_logi("Unknown cmd");
		rc = -ENOENT;
		break;
	}

	return rc;
}

/**
 * bastet_partner_init() - init partner.
 *
 * to initialize the global variables.
 * partner_uids_lock and partner_uids_head.
 *
 * Return: 0, success
 */
int bastet_partner_init(void)
{
	spin_lock_init(&partner_uids_lock);
	INIT_LIST_HEAD(&partner_uids_head);

	return 0;
}

/**
 * bastet_partner_clear() - clear partner.
 *
 * to clear the socks.
 *
 * Return:No
 */
void bastet_partner_clear(void)
{
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p_head = NULL;
	struct list_head *n_head = NULL;

	spin_lock_bh(&partner_uids_lock);
	if (!list_empty(&partner_uids_head)) {
		/* clear all uid_node */
		list_for_each_safe(p_head, n_head, &partner_uids_head) {
			uid_node = list_entry(p_head,
				struct bastet_partner_uid, list);
			del_socks(&uid_node->socks_head);
		}
	}
	spin_unlock_bh(&partner_uids_lock);
}

/**
 * bastet_partner_release() - release the partner.
 *
 * to release the socks list and free the memorys.
 *
 * Return: No
 */
void bastet_partner_release(void)
{
	struct bastet_partner_uid *uid_node = NULL;
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	spin_lock_bh(&partner_uids_lock);
	if (!list_empty(&partner_uids_head)) {
		/* clear all uid_node */
		list_for_each_safe(p, n, &partner_uids_head) {
			uid_node = list_entry(p,
				struct bastet_partner_uid, list);
			del_socks(&uid_node->socks_head);
			list_del(&uid_node->list);
			bastet_logi("kfree struct bastet_partner_uid uid=%d",
				uid_node->uid);
			kfree(uid_node);
		}
	}
	spin_unlock_bh(&partner_uids_lock);
}
