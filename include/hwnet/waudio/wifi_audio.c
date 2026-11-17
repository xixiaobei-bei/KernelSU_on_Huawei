/*
 * wifi_audio.c
 *
 * wifi audio kernel module implementation
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "wifi_audio.h"
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/netlink.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <securec.h>
#include "wifi_audio_utils.h"
#include "wifi_audio_ptp.h"

struct netlink_ctrl {
	struct sock *dev_sock;
	struct work_struct receive_handle_work;
	struct list_queue receive_queue;
	unsigned int user_pid;
	struct work_struct report_handle_work;
	struct list_queue report_queue;
};

static int netdev_notifier_event(struct notifier_block *this, unsigned long event,
	void *ptr);
static void netlink_event_send_tasket(unsigned long arg);
DECLARE_TASKLET(netlink_event_report_tasklet,
	netlink_event_send_tasket, 0);

static struct netlink_ctrl *wifi_audio_netlink = NULL;
static struct notifier_block wifi_audio_netdev_notifier = {
	.notifier_call = netdev_notifier_event,
};

static int netdev_notifier_event(struct notifier_block *this, unsigned long event,
	void *ptr)
{
	struct net_device *dev = NULL;
	if ((wifi_audio_netlink == NULL) || (ptr == NULL)) {
		wifi_audio_log_info("param is invalid");
		return NOTIFY_DONE;
	}
	dev = netdev_notifier_info_to_dev(ptr);
	wifi_audio_log_info("netdev notifier event %lu", event);
	switch (event) {
	case NETDEV_DOWN:
	case NETDEV_CHANGEMTU:
	case NETDEV_CHANGEADDR:
	case NETDEV_CHANGENAME:
		wifi_audio_log_info("ptp clear by dev handle");
		ptp_clear_by_dev(dev);
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static void netlink_msg_receive_dispatch(const struct netlink_data *msg_rev)
{
	int ret;

	if (msg_rev == NULL) {
		wifi_audio_log_err("msg_rev is NULL, error");
		return;
	}

	switch (msg_rev->id) {
	case NETLINK_MSG_SET_CONFIG_INFOR:
	case NETLINK_MSG_START_TIMESYNC:
	case NETLINK_MSG_STOP_TIMESYNC: {
		ret = netlink_msg_ptp_handle(msg_rev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp netlink msg handle error");
		break;
	}
	default:
		wifi_audio_log_err("msg_rev id is invalid, error");
		break;
	}
	return;
}

static void netlink_msg_receive_handle(struct work_struct *work)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;
	struct netlink_data msg_rev = {0};
	int sec_ret;

	if (work == NULL) {
		wifi_audio_log_err("work is NULL, error");
		return;
	}

	ctrl = container_of(work, struct netlink_ctrl, receive_handle_work);
	while (list_queue_is_empty(&(ctrl->receive_queue)) == 0) {
		node = list_queue_dequeue(&(ctrl->receive_queue));
		if (node == NULL) {
			wifi_audio_log_err("dequeue a null node");
			break;
		}
		sec_ret = memcpy_s(&msg_rev, sizeof(msg_rev), node->data, node->len);
		free_list_node(node);
		node = NULL;
		if (sec_ret != 0) {
			wifi_audio_log_err("memcpy node->data fail, error");
			break;
		}
		netlink_msg_receive_dispatch(&msg_rev);
	}
	return;
}

static void netlink_msg_set(struct netlink_ctrl *pctrl,
	const struct netlink_data *msg_rev)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;

	if ((msg_rev == NULL) || (pctrl == NULL)) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	ctrl = pctrl;
	wifi_audio_log_info("id = %d, len = %u", msg_rev->id,
		msg_rev->msg_len);

	node = alloc_list_node((void *)msg_rev, sizeof(*msg_rev));
	if (unlikely(node == NULL)) {
		wifi_audio_log_err("alloc node failed, error");
		return;
	}
	list_queue_enqueue(&(ctrl->receive_queue), node);
	schedule_work(&(ctrl->receive_handle_work));
	return;
}

static void netlink_event_receive(struct sk_buff *netlink_skb)
{
	int ret;
	int sec_ret;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct netlink_ctrl *pctrl = NULL;
	struct netlink_data msg_rev = {0};

	if ((wifi_audio_netlink == NULL) ||
		(netlink_skb == NULL)) {
		wifi_audio_log_err("param is NULL Pointer");
		return;
	}
	pctrl = wifi_audio_netlink;
	skb = skb_get(netlink_skb);
	if (skb->len < NLMSG_SPACE(0)) {
		wifi_audio_log_err("skb->len isn't enough");
		kfree_skb(skb);
		return;
	}

	nlh = nlmsg_hdr(skb);
	if (nlh == NULL) {
		wifi_audio_log_err("nlh is NULL");
		kfree_skb(skb);
		return;
	}
	sec_ret = memcpy_s(&msg_rev, sizeof(msg_rev), NLMSG_DATA(nlh), sizeof(msg_rev));
	if (sec_ret != 0) {
		wifi_audio_log_err("memcpy_s ERROR! bt_netlink_event_receive return");
		kfree_skb(skb);
		return;
	}

	if(msg_rev.id == NETLINK_MSG_SET_USER_ID) {
		/* Store the user space daemon pid */
		pctrl->user_pid = nlh->nlmsg_pid;
		wifi_audio_log_info("set netlink user ID pctrl->user_pid %u", pctrl->user_pid);
		ret = wifi_audio_time_sync_init();
		if (ret != SUCCESS)
			wifi_audio_log_err("WiFi audio time sync init failed");
	} else {
		wifi_audio_log_info("set data msg_rev.id %d, nlmsg_pid %d", msg_rev.id, nlh->nlmsg_pid);
		netlink_msg_set(pctrl, &msg_rev);
	}
	schedule_work(&pctrl->report_handle_work);
	kfree_skb(skb);
	return;
}

int netlink_event_report(const struct netlink_data *event)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;

	if ((event == NULL) || (wifi_audio_netlink == NULL)) {
		wifi_audio_log_err("NULL Pointer");
		return ERROR;
	}
	ctrl = wifi_audio_netlink;

	node = alloc_list_node((void *)event, sizeof(*event));
	if (unlikely(node == NULL)) {
		wifi_audio_log_err("alloc node failed");
		return  ERROR;
	}
	list_queue_enqueue(&(ctrl->report_queue), node);
	tasklet_schedule(&netlink_event_report_tasklet);
	return SUCCESS;
}

static void netlink_event_send(struct work_struct *work)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	unsigned int msg_size;
	int ret;
	int sec_ret;

	if (work == NULL) {
		wifi_audio_log_err("work NULL Pointer");
		return;
	}
	ctrl = container_of(work, struct netlink_ctrl, report_handle_work);
	while (list_queue_is_empty(&(ctrl->report_queue)) == 0) {
		node = list_queue_dequeue(&(ctrl->report_queue));
		if (node == NULL) {
			wifi_audio_log_err("dequeue a null node");
			break;
		}
		wifi_audio_log_info("dequeue a node");

		msg_size = NLMSG_SPACE(node->len);
		skb = alloc_skb(msg_size, GFP_ATOMIC);
		if (skb == NULL) {
			wifi_audio_log_err("alloc skb failed");
			free_list_node(node);
			node = NULL;
			return;
		}
		nlh = nlmsg_put(skb, 0, 0, 0, msg_size - sizeof(*nlh), 0);
		NETLINK_CB(skb).portid = 0; //lint !e545
		NETLINK_CB(skb).dst_group = 0; //lint !e545
		sec_ret = memcpy_s(NLMSG_DATA(nlh), node->len, node->data,
			node->len);
		free_list_node(node);
		node = NULL;
		if (sec_ret != 0) {
			wifi_audio_log_err("memcpy node->data fail");
			kfree_skb(skb);
			return;
		}
		ret = netlink_unicast(ctrl->dev_sock, skb, ctrl->user_pid, 0);
		if (ret < 0) {
			wifi_audio_log_err("msg send failed!");
			return;
		}
	}
	return;
}

static void netlink_event_send_tasket(unsigned long arg)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	unsigned int msg_size;
	int ret;
	int sec_ret;

	if (unlikely(wifi_audio_netlink == NULL)) {
		wifi_audio_log_err("wifi_audio_netlink is NULL Pointer");
		return;
	}
	ctrl = wifi_audio_netlink;

	while (list_queue_is_empty(&(ctrl->report_queue)) == 0) {
		node = list_queue_dequeue(&(ctrl->report_queue));
		if (node == NULL) {
			wifi_audio_log_err("dequeue a null node");
			break;
		}
		msg_size = NLMSG_SPACE(node->len);
		skb = alloc_skb(msg_size, GFP_ATOMIC);
		if (skb == NULL) {
			wifi_audio_log_err("alloc skb failed");
			free_list_node(node);
			node = NULL;
			return;
		}
		nlh = nlmsg_put(skb, 0, 0, 0, msg_size - sizeof(*nlh), 0);
		NETLINK_CB(skb).portid = 0; //lint !e545
		NETLINK_CB(skb).dst_group = 0; //lint !e545
		sec_ret = memcpy_s(NLMSG_DATA(nlh), node->len, node->data,
			node->len);
		free_list_node(node);
		node = NULL;
		if (sec_ret != 0) {
			wifi_audio_log_err("memcpy node->data fail");
			kfree_skb(skb);
			return;
		}
		ret = netlink_unicast(ctrl->dev_sock, skb, ctrl->user_pid, 0);
		if (ret < 0) {
			wifi_audio_log_err("msg send failed!");
			return;
		}
	}
	return;
}

static void netlink_uninit(void)
{
	struct netlink_ctrl *pctrl = NULL;

	if (unlikely(wifi_audio_netlink == NULL)) {
		wifi_audio_log_err("wifi_audio_netlink is NULL Pointer");
		return;
	}
	wifi_audio_log_info("uinit netlink");
	pctrl = wifi_audio_netlink;
	if (pctrl != NULL) {
		cancel_work_sync(&(pctrl->receive_handle_work));
		cancel_work_sync(&(pctrl->report_handle_work));
		if (pctrl->dev_sock != NULL)
			netlink_kernel_release(pctrl->dev_sock);
		kfree(pctrl);
		pctrl = NULL;
		wifi_audio_netlink = NULL;
	}
	return;
}

static int netlink_init(void)
{
	struct netlink_ctrl *pctrl = NULL;
	struct netlink_kernel_cfg config_cfg = {0};

	wifi_audio_log_info("netlink init");
	pctrl = (struct netlink_ctrl *)kmalloc(sizeof(*pctrl), GFP_ATOMIC);
	if (pctrl == NULL)
	{
		wifi_audio_log_err("kmalloc failed");
		return ERROR;
	}

	config_cfg.groups = 0;
	config_cfg.input = (void *)netlink_event_receive;
	config_cfg.cb_mutex = NULL;
	pctrl->dev_sock = netlink_kernel_create(&init_net,
		NETLINK_WIFI_AUDIO_EVENT, &config_cfg);
	if (pctrl->dev_sock == NULL) {
		kfree(pctrl);
		pctrl = NULL;
		wifi_audio_log_err("netlink creat failed!");
		return ERROR;
	}
	list_queue_init(&(pctrl->receive_queue));
	INIT_WORK(&(pctrl->receive_handle_work), netlink_msg_receive_handle);
	pctrl->user_pid = (unsigned int)(USER_ID_MAX);
	list_queue_init(&(pctrl->report_queue));
	INIT_WORK(&(pctrl->report_handle_work), netlink_event_send);
	wifi_audio_netlink = pctrl;
	return SUCCESS;
}

static int __init wifi_audio_init(void)
{
	int ret;

	ret = netlink_init();
	if (ret != SUCCESS)
		wifi_audio_log_err("netlink init failed");

	ret = register_netdevice_notifier(&wifi_audio_netdev_notifier);
	if (ret != SUCCESS)
		wifi_audio_log_err("register netdevice notifier failed");

	return ret;
}

static void __exit wifi_audio_exit(void)
{
	netlink_uninit();
	wifi_audio_time_sync_uninit();
	unregister_netdevice_notifier(&wifi_audio_netdev_notifier);
}

module_init(wifi_audio_init);
module_exit(wifi_audio_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(WIFI_AUDIO_MODULE);
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
