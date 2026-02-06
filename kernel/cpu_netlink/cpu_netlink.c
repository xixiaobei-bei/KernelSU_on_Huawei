/*
 * cpu_netlink.c
 *
 * iaware cpu netlink implementation
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <cpu_netlink/cpu_netlink.h>

#define MAX_DATA_LEN 20

static __u32 recv_pid;
static struct sock *netlink_fd;

int send_to_user(int sock_no, size_t len, const int *data)
{
	int ret = -1;
	int i;
	int num = len + SEND_DATA_LEN;
	int size;
	int dt[MAX_DATA_LEN];
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;

	if (!data)
		return ret;

	if (IS_ERR_OR_NULL(netlink_fd))
		return ret;

	if (recv_pid <= 0)
		return ret;

	if (num > MAX_DATA_LEN) {
		printk(KERN_ERR "cpu_netlink send oversize(%d,MAX:%d) data!\n",
			num, MAX_DATA_LEN);
		return ret;
	}

	dt[0] = sock_no;
	dt[1] = len;
	/*lint -save -e440 -e679*/
	for (i = 0; i + SEND_DATA_LEN < num; i++)
		dt[i + SEND_DATA_LEN] = data[i];

	/*lint -restore*/
	size = sizeof(int) * num;

	skb = alloc_skb(NLMSG_SPACE(size), GFP_ATOMIC);
	if (IS_ERR_OR_NULL(skb)) {
		printk(KERN_ERR "cpu_netlink %s: alloc skb failed!\n",
			__func__);
		return -ENOMEM;
	}
	nlh = nlmsg_put(skb, 0, 0, 0,
		NLMSG_SPACE(size) - sizeof(struct nlmsghdr), 0);
	memcpy(NLMSG_DATA(nlh), (void *)dt, size);
	/* send up msg */
	ret = netlink_unicast(netlink_fd, skb, recv_pid, MSG_DONTWAIT);
	if (ret < 0)
		printk(KERN_WARNING
			"cpu_netlink: send_to_user netlink_unicast failed! errno = %d\n",
			ret);

	return ret;
}

static void recv_from_user(struct sk_buff *skb)
{
	struct sk_buff *tmp_skb = NULL;
	struct nlmsghdr *nlh = NULL;

	if (IS_ERR_OR_NULL(skb)) {
		printk(KERN_ERR "cpu_netlink: recv_from_user: skb is NULL!\n");
		return;
	}

	tmp_skb = skb_get(skb);
	if (tmp_skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(tmp_skb);
		recv_pid = nlh->nlmsg_pid;
	}
}

static void create_cpu_netlink(int socket_no)
{
	int ret = 0;
	struct netlink_kernel_cfg cfg = {
		.groups = 0,
		.input = recv_from_user,
	};

	netlink_fd = netlink_kernel_create(&init_net, socket_no, &cfg);
	if (IS_ERR_OR_NULL(netlink_fd)) {
		ret = PTR_ERR(netlink_fd);
		printk(KERN_ERR "cpu_netlink: create cpu netlink error! ret is %d\n",
			ret);
	}
}

static void destroy_cpu_netlink(void)
{
	if (!IS_ERR_OR_NULL(netlink_fd) &&
		!IS_ERR_OR_NULL(netlink_fd->sk_socket)) {
		sock_release(netlink_fd->sk_socket);
		netlink_fd = NULL;
	}
}

static int __init cpu_netlink_init(void)
{
	create_cpu_netlink(NETLINK_HW_IAWARE_CPU);
	return 0;
}

static void __exit cpu_netlink_exit(void)
{
	destroy_cpu_netlink();
}

module_init(cpu_netlink_init);
module_exit(cpu_netlink_exit);
