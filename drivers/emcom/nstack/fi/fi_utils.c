/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
* Description: This module is a map utils for FI
* Author: songqiubin songqiubin@huawei.com
* Create: 2019-09-18
*/

#include "fi_utils.h"
#include <linux/stddef.h>
#include <linux/string.h>
#include "fi.h"

struct sk_buff *fi_get_netlink_skb(int type, int len, char **data)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *pskb_out = NULL;

	pskb_out = nlmsg_new(len, GFP_ATOMIC);
	if (pskb_out == NULL) {
		fi_loge("Out of memry");
		return NULL;
	}
	nlh = nlmsg_put(pskb_out, 0, 0, type, len, 0);
	if (nlh == NULL) {
		kfree_skb(pskb_out);
		return NULL;
	}
	*data = nlmsg_data(nlh);
	return pskb_out;
}

void fi_enqueue_netlink_skb(struct sk_buff *pskb)
{
	if (skb_queue_len(&g_fi_ctx.skb_queue) > FI_NL_SKB_QUEUE_MAXLEN) {
		fi_logi("skb_queue len too many, discard the skb");
		kfree_skb(pskb);
		return;
	}
	NETLINK_CB(pskb).dst_group = 0; /* For unicast */ /*lint !e545*/
	skb_queue_tail(&g_fi_ctx.skb_queue, pskb);
	up(&g_fi_ctx.netlink_sync_sema);
}

int fi_netlink_thread(void *data)
{
	struct sk_buff *skb = NULL;

	while (1) {
		if (kthread_should_stop())
			break;
		/* netlink thread will block at this semaphone when no data coming. */
		down(&g_fi_ctx.netlink_sync_sema);
		fi_logd("fi_netlink_thread get sema success!");
		do {
			skb = skb_dequeue(&g_fi_ctx.skb_queue);
			if (skb != NULL) {
				if (g_fi_ctx.nl_pid)
					netlink_unicast(g_fi_ctx.nlfd, skb, g_fi_ctx.nl_pid, MSG_DONTWAIT);
				else
					kfree_skb(skb);
			}
		} while (!skb_queue_empty(&g_fi_ctx.skb_queue));
	}
	return 0;
}

void fi_empty_netlink_skb_queue(void)
{
	struct sk_buff *skb = NULL;
	while (!skb_queue_empty(&g_fi_ctx.skb_queue)) {
		skb = skb_dequeue(&g_fi_ctx.skb_queue);
		if (skb != NULL)
			kfree_skb(skb);
	}
}
