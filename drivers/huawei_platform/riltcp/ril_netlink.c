/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Communicate between netlink and ril.
 * Author: zhuweichen@huawei.com
 * Create: 2017-04-10
 */

#include "ril_netlink.h"

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netlink.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/types.h>
#include <net/inet_hashtables.h>
#include <net/ip.h>
#include <net/net_namespace.h>
#include <net/netlink.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <uapi/linux/netlink.h>

#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG ril_netlink
HWLOG_REGIST();
MODULE_LICENSE("GPL");

DEFINE_MUTEX(ril_receive_sem);
DEFINE_MUTEX(ril_send_sem);

/* netlink socket fd */
static struct sock *g_ril_nlfd = NULL;

/* save user space progress pid when user space netlink socket registering. */
unsigned int g_ril_uspace_pid = 0;
static struct task_struct *g_ril_netlink_task = NULL;
static int g_ril_module_state = RIL_NETLINK_EXIT;
char g_ril_if_name[MAX_IF_NAME] = {0};

/*
 * tcp protocol use this semaphone to inform chr netlink thread
 * when data speed is slow
 */
static struct semaphore g_ril_netlink_sync_sema;
/* this lock is used to protect global variable. */
static spinlock_t deact_info_lock;

/*
 * netlink socket's callback function,it will be called by system when
 * user space send a message to kernel.
 * this function will save user space progress pid.
 */
static void kernel_ril_receive(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	ril_to_knl_msg *rilmsg = NULL;

	if (__skb == NULL) {
		hwlog_err("Invalid parameter: zero pointer reference(__skb)\n");
		return;
	}
	skb = skb_get(__skb);
	if (skb == NULL)
		return;

	mutex_lock(&ril_receive_sem);

	if (skb->len >= NLMSG_HDRLEN) {
		nlh = nlmsg_hdr(skb);
		if (nlh == NULL || nlh->nlmsg_len < sizeof(struct nlmsghdr) ||
			skb->len < nlh->nlmsg_len) {
			kfree_skb(skb);
			mutex_unlock(&ril_receive_sem);
			return;
		}
		if (nlh->nlmsg_type == NETLINK_TCP_REG) {
			/*
			 * save user space progress pid
			 * when register netlink socket.
			 */
			hwlog_info("receive pid = %d\n", nlh->nlmsg_pid);
			g_ril_uspace_pid = nlh->nlmsg_pid;
		} else if (nlh->nlmsg_type == NETLINK_TCP_UNREG) {
			hwlog_info("%s NETLINK_TCP_UNREG\n", __func__);
			g_ril_uspace_pid = 0;
		} else if (nlh->nlmsg_type == NETLINK_TCP_RST_MSG &&
			nlh->nlmsg_len >= sizeof(struct nlmsghdr) +
			sizeof(ril_to_knl_msg)) {
			rilmsg = (ril_to_knl_msg *)nlmsg_data(nlh);
			if ((rilmsg == NULL) || (rilmsg->if_name == NULL)) {
				hwlog_err("rilmsg or name = NULL\n");
				kfree_skb(skb);
				mutex_unlock(&ril_receive_sem);
				return;
			}
			rilmsg->if_name[MAX_IF_NAME - 1] = '\0';
			hwlog_info("if_name = %s\n", rilmsg->if_name);
			if (spin_trylock_bh(&deact_info_lock)) {
				strncpy(g_ril_if_name, rilmsg->if_name,
					MAX_IF_NAME);
				spin_unlock_bh(&deact_info_lock);
				/* send tcp reset */
				up(&g_ril_netlink_sync_sema);
			}
		}
	}
	kfree_skb(skb);
	mutex_unlock(&ril_receive_sem);
}


int ril_tcp_send_reset(const char *dev)
{
	int bucket = 0;

	for (; bucket <= tcp_hashinfo.ehash_mask; ++bucket) {
		struct sock *sk = NULL;
		struct hlist_nulls_node *node = NULL;
		spinlock_t *lock = inet_ehash_lockp(&tcp_hashinfo, bucket);

		/* Lockless fast path for the common case of empty buckets */
		if (hlist_nulls_empty(&tcp_hashinfo.ehash[bucket].chain))
			continue;

#ifdef CONFIG_HW_WIFIPRO
		spin_lock_bh(lock);
		sk_nulls_for_each(sk, node, &tcp_hashinfo.ehash[bucket].chain) {
		if ((AF_INET == sk->sk_family || AF_INET6 == sk->sk_family) &&
			sk->wifipro_dev_name != NULL &&
			!strncmp(dev, sk->wifipro_dev_name, MAX_IF_NAME)) {
			hwlog_debug("%s: ril_sock_send_reset start dev = %s\n",
				__func__, dev);
			bh_lock_sock(sk);
			tcp_send_active_reset(sk, GFP_ATOMIC);
			bh_unlock_sock(sk);
			}
		}
		spin_unlock_bh(lock);
#endif
	}

	return 0;
}

static int ril_netlink_thread(void *data)
{
	char *if_name[MAX_IF_NAME] = {NULL};

	hwlog_info("%s: start\n", __func__);

	while (1) {
		if (kthread_should_stop())
			break;

		/*
		 * netlink thread will block at this semaphone when data speed
		 * is nomal,only tcp protocol up the sema this thread will
		 * go to next sentence.
		 */
		down(&g_ril_netlink_sync_sema);

		if (g_ril_uspace_pid != 0) {
			spin_lock_bh(&deact_info_lock);
			strncpy(if_name, g_ril_if_name, MAX_IF_NAME);
			spin_unlock_bh(&deact_info_lock);

			hwlog_debug("%s: send tcp reset if_name = %s\n",
				__func__, g_ril_if_name);
			/*
			 * search tcp hash table find sock on if and send
			 * tcp reset
			 */
			ril_tcp_send_reset(if_name);
		}
	}
	return 0;
}

/* netlink init function. */
static void ril_netlink_init(void)
{
	struct netlink_kernel_cfg ril_nl_cfg = {
		.input = kernel_ril_receive,
	};

	g_ril_nlfd = netlink_kernel_create(&init_net, NETLINK_RIL_EVENT_NL,
		&ril_nl_cfg);
	if (!g_ril_nlfd)
		hwlog_info("%s: failed\n", __func__);
	else
		hwlog_info("%s: success\n", __func__);
	sema_init(&g_ril_netlink_sync_sema, 0);
	spin_lock_init(&deact_info_lock);

	g_ril_netlink_task = kthread_run(ril_netlink_thread, NULL,
		"ril_netlink_thread");
}

/* netlink deinit function. */
static void ril_netlink_deinit(void)
{
	if (g_ril_nlfd && g_ril_nlfd->sk_socket) {
		sock_release(g_ril_nlfd->sk_socket);
		g_ril_nlfd = NULL;
	}

	if (g_ril_netlink_task) {
		kthread_stop(g_ril_netlink_task);
		g_ril_netlink_task = NULL;
	}
}

static int __init ril_netlink_module_init(void)
{
	ril_netlink_init();
	g_ril_module_state = RIL_NETLINK_INIT;
	return 0;
}

static void __exit ril_netlink_module_exit(void)
{
	g_ril_module_state = RIL_NETLINK_EXIT;
	ril_netlink_deinit();
}

module_init(ril_netlink_module_init);
module_exit(ril_netlink_module_exit);
