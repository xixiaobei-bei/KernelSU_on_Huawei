/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: send tcp reset on iface.
 * Author: zhuweichen@huawei.com
 * Create: 2019-10-24
 */

#include "sock_destroy_handler.h"

#include <linux/errno.h>
#include <linux/if.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>

#include <net/inet_hashtables.h>
#include <net/ip.h>
#include <net/sock.h>
#include <net/tcp.h>

static char g_ifname[IFNAMSIZ];
static spinlock_t g_ifname_lock;
static struct semaphore g_sock_destroy_cmd_sema;

struct sock_destroy_req_msg {
	struct req_msg_head header;
	char ifname[IFNAMSIZ];
};

static void sock_destroy_cmd(struct sock_destroy_req_msg *msg)
{
	pr_info("%s:ifname=%s\n", __func__, msg->ifname);
	if (msg->ifname[0] == '\0')
		return;

	if (spin_trylock_bh(&g_ifname_lock)) {
		strlcpy(g_ifname, msg->ifname, IFNAMSIZ);
		spin_unlock_bh(&g_ifname_lock);
		up(&g_sock_destroy_cmd_sema);
	}
}

static void sock_destroy_msg_process(struct req_msg_head *msg)
{
	if (msg == NULL)
		return;

	if (msg->type == SOCK_DESTROY_HANDLER_CMD &&
		msg->len >= sizeof(struct sock_destroy_req_msg))
		sock_destroy_cmd((struct sock_destroy_req_msg *)msg);
}

#ifdef CONFIG_HW_WIFIPRO
static bool sock_destroy_check(struct sock *sk, const char *ifname)
{
	return sk_fullsock(sk) && !sock_flag(sk, SOCK_DEAD) &&
		(sk->sk_family == AF_INET || sk->sk_family == AF_INET6) &&
		(sk->sk_protocol == IPPROTO_TCP) &&
		tcp_need_reset(sk->sk_state) &&
		!sock_owned_by_user(sk) &&
		!strncmp(ifname, sk->wifipro_dev_name, IFNAMSIZ);
}
#endif

static int sock_destroy_send_tcp_rst(const char *ifname)
{
	int bucket = 0;
	int total = 0;
	int destroy = 0;
#ifdef CONFIG_HW_WIFIPRO
	struct sock *sk = NULL;
	struct hlist_nulls_node *node = NULL;
	spinlock_t *lock = NULL;
#endif

	pr_info("%s:ifname=%s\n", __func__, ifname);
	for (; bucket <= tcp_hashinfo.ehash_mask; ++bucket) {
		if (hlist_nulls_empty(&tcp_hashinfo.ehash[bucket].chain))
			continue;
#ifdef CONFIG_HW_WIFIPRO
		lock = inet_ehash_lockp(&tcp_hashinfo, bucket);
		spin_lock_bh(lock);
		sk_nulls_for_each(sk, node, &tcp_hashinfo.ehash[bucket].chain) {
			total++;
			if (sock_destroy_check(sk, ifname)) {
				bh_lock_sock(sk);
				tcp_send_active_reset(sk, GFP_ATOMIC);
				bh_unlock_sock(sk);
				destroy++;
				pr_info("%s:sk=%p do send\n", __func__, sk);
			}
		}
		spin_unlock_bh(lock);
#endif
	}
	pr_info("%s:done total=%d destroy=%d\n", __func__, total, destroy);
	return 0;
}

static int sock_destroy_thread(void *data)
{
	char ifname[IFNAMSIZ];

	pr_info("%s:start\n", __func__);
	while (1) {
		if (kthread_should_stop())
			break;
		down(&g_sock_destroy_cmd_sema);
		spin_lock_bh(&g_ifname_lock);
		strlcpy(ifname, g_ifname, IFNAMSIZ);
		spin_unlock_bh(&g_ifname_lock);
		sock_destroy_send_tcp_rst(ifname);
	}
	return 0;
}

msg_process *sock_destroy_handler_init(notify_event *notify)
{
	sema_init(&g_sock_destroy_cmd_sema, 0);
	spin_lock_init(&g_ifname_lock);
	kthread_run(sock_destroy_thread, NULL, "sock_destroy_th");
	return sock_destroy_msg_process;
}
