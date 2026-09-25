/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: This module is for sending and receiving netlink messages
 *              for data service app qoe.
 *              If you want to add a new module, you need to add a module
 *              initialization function to this module initialization function
 *              and set the message callback function. If you want to receive
 *              messages from other modules, you need to modify the
 *              cmd_map_model mapping table.
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#include "netlink_handle.h"
#ifdef CONFIG_HW_NETWORK_QOE
#include "ip_para_collec_ex.h"
#endif

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/time.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/netlink.h>
#include <uapi/linux/netlink.h>
#include <linux/kthread.h>
#include <linux/list.h>

#include "tcp_para_collec.h"

#ifdef CONFIG_HW_PACKET_FILTER_BYPASS
#include "hw_packet_filter_bypass.h"
#endif

#ifdef CONFIG_HW_NETWORK_SLICE
#include "network_slice_management.h"
#endif
#include "sock_destroy_handler.h"

#ifdef CONFIG_HW_ICMP_PING_DETECT
#include "icmp_ping_detect.h"
#endif

#ifdef CONFIG_TCP_CONG_ALGO_CTRL
#include "tcp_cong_algo_ctrl.h"
#endif
#ifdef CONFIG_HW_PACKET_TRACKER
#include "hw_pt.h"
#endif

#ifdef CONFIG_CHR_NETLINK_MODULE
#include "chr_manager.h"
#endif

#undef HWLOG_TAG
#define HWLOG_TAG monitor_handle
HWLOG_REGIST();
MODULE_LICENSE("GPL");

#define NETLINK_HANDLE_EXIT 0
#define NETLINK_HANDLE_INIT 1

#ifndef NETLINK_OLLIE
#define NETLINK_OLLIE 41
#endif

static DEFINE_MUTEX(g_recv_mtx);
static DEFINE_MUTEX(g_send_mtx);
static DEFINE_SPINLOCK(g_rpt_lock);

/* The context of the message handle */
struct handle_ctx {
	/* Netlink socket fd */
	struct sock *nlfd;

	/* Save user space progress pid when user space netlink registering. */
	unsigned int native_pid;

	/* Tasks for sending messages. */
	struct task_struct *task;

	/* Channel status */
	int chan_state;

	/* Semaphore of the message sent */
	struct semaphore sema;

	/* Message queue header */
	struct list_head msg_head;

	/* Message processing callback functions */
	msg_process * model_cb[MODEL_NUM];
};
static struct handle_ctx g_ctx;

/*Message list structure*/
struct msg_entity {
	struct list_head head;
	struct res_msg_head msg;
};

/*Message mapping table for external modules*/
const u16 cmd_map_model[CMD_NUM_MAX][MAP_ENTITY_NUM] = {
	{APP_QOE_SYNC_CMD, IP_PARA_COLLEC},
	{UPDATE_APP_INFO_CMD, IP_PARA_COLLEC},
	/* hw_packet_filter_bypass begin */
	{ADD_FG_UID, PACKET_FILTER_BYPASS},
	{DEL_FG_UID, PACKET_FILTER_BYPASS},
	{BYPASS_FG_UID, PACKET_FILTER_BYPASS},
	{NOPASS_FG_UID, PACKET_FILTER_BYPASS},
	/* hw_packet_filter_bypass end */
	{TCP_PKT_COLLEC_CMD, TCP_PARA_COLLEC},
	{BIND_UID_PROCESS_TO_NETWORK_CMD, NETWORK_SLICE_MANAGEMENT},
	{DEL_UID_BIND_CMD, NETWORK_SLICE_MANAGEMENT},
	{SLICE_RPT_CTRL_CMD, NETWORK_SLICE_MANAGEMENT},
	{SOCK_DESTROY_HANDLER_CMD, SOCK_DESTROY_HANDLER},
	{ ICMP_PING_DETECT_CMD, ICMP_PING_DETECT },
	/* tcp_cong_algo_ctrl ops */
	{ UPDATE_TCP_CA_CONFIG, TCP_CONG_ALGO_CTRL },
	{ UPDATE_UID_STATE, TCP_CONG_ALGO_CTRL },
	{ UPDATE_DS_STATE, TCP_CONG_ALGO_CTRL },
	{ BIND_PROCESS_TO_SLICE_CMD, NETWORK_SLICE_MANAGEMENT },
	{ DEL_SLICE_BIND_CMD, NETWORK_SLICE_MANAGEMENT },
};

static void nl_notify_event(struct res_msg_head *msg)
{
	struct msg_entity *p = NULL;
	u32 len;

	if (!msg) {
		hwlog_err("%s:null data\n", __func__);
		return;
	}

	if (g_ctx.chan_state != NETLINK_HANDLE_INIT) {
		hwlog_err("%s:module not inited\n", __func__);
		return;
	}

	len = sizeof(struct list_head) + msg->len;
	p = kmalloc(len, GFP_ATOMIC);
	if (!p) {
		printk(KERN_ERR "%s: kmalloc() failed!\n", __func__);
		return;
	}
	memcpy(&p->msg, msg, msg->len);
	spin_lock_bh(&g_rpt_lock);
	list_add((struct list_head *)(p), &g_ctx.msg_head);
	spin_unlock_bh(&g_rpt_lock);

	up(&g_ctx.sema);
}

static void process_cmd(struct req_msg_head *cmd)
{
	int i;

	if (cmd == NULL)
		return;

	for (i = 0; i < CMD_NUM_MAX; i++) {
		if (cmd_map_model[i][MAP_KEY_INDEX] != cmd->type)
			continue;
		if (g_ctx.model_cb[cmd_map_model[i][MAP_VALUE_INDEX]] == NULL)
			break;
		g_ctx.model_cb[cmd_map_model[i][MAP_VALUE_INDEX]](cmd);
	}
}

static void netlink_handle_rcv(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;

	if (g_ctx.chan_state != NETLINK_HANDLE_INIT) {
		hwlog_err("%s:module not inited\n", __func__);
		return;
	}
	if (__skb == NULL) {
		hwlog_err("Invalid param: zero pointer reference(__skb)\n");
		return;
	}
	skb = skb_get(__skb);
	if (skb == NULL) {
		hwlog_err("netlink_handle_rcv: skb = NULL\n");
		return;
	}
	mutex_lock(&g_recv_mtx);
	if (skb->len < NLMSG_HDRLEN) {
		hwlog_err("netlink_handle_rcv:skb len error\n");
		goto skb_free;
	}
	nlh = nlmsg_hdr(skb);
	if (nlh == NULL) {
		hwlog_err("netlink_handle_rcv:nlh = NULL\n");
		goto skb_free;
	}
	if ((nlh->nlmsg_len < sizeof(struct nlmsghdr))
		|| (skb->len < nlh->nlmsg_len)) {
		hwlog_err("netlink_handle_rcv:nlmsg len error\n");
		goto skb_free;
	}
	switch (nlh->nlmsg_type) {
	case NL_MSG_REG:
		g_ctx.native_pid = nlh->nlmsg_pid;
		break;
	case NL_MSG_JNI_REQ:
		process_cmd((struct req_msg_head *)NLMSG_DATA(nlh));
		break;
	default:
		hwlog_err("invalid nlmsg_type %d\n", nlh->nlmsg_type);
		break;
	}

skb_free:
	kfree_skb(skb);
	mutex_unlock(&g_recv_mtx);
}

static struct msg_entity *get_msg(void)
{
	struct msg_entity *p = NULL;

	spin_lock_bh(&g_rpt_lock);
	if (!list_empty_careful(&g_ctx.msg_head)) {
		p = list_last_entry(&g_ctx.msg_head, struct msg_entity, head);
		list_del((struct list_head *)p);
	}
	spin_unlock_bh(&g_rpt_lock);
	return p;
}

/* send a message to user space */
static int __nl_notify_event(struct res_msg_head *msg)
{
	int ret = 0;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	void *pdata = NULL;

	mutex_lock(&g_send_mtx);
	if (!g_ctx.native_pid || !g_ctx.nlfd) {
		hwlog_err("%s: err pid = %d\n", __func__, g_ctx.native_pid);
		ret = -1;
		goto nty_end;
	}

	skb = nlmsg_new(msg->len, GFP_ATOMIC);
	if (!skb) {
		hwlog_info("%s: alloc skb fail\n", __func__);
		ret = -1;
		goto nty_end;
	}
	nlh = nlmsg_put(skb, 0, 0, msg->type, msg->len, 0);
	if (!nlh) {
		kfree_skb(skb);
		skb = NULL;
		ret = -1;
		goto nty_end;
	}

	pdata = nlmsg_data(nlh);
	memcpy(pdata, msg, msg->len);

	/* skb will be freed in netlink_unicast */
	ret = netlink_unicast(g_ctx.nlfd, skb, g_ctx.native_pid, MSG_DONTWAIT);

nty_end:
	mutex_unlock(&g_send_mtx);
	return ret;
}

static int netlink_handle_thread(void *data)
{
	struct msg_entity *p = NULL;

	while (!kthread_should_stop()) {

		down(&g_ctx.sema);

		if (g_ctx.native_pid == 0)
			continue;

		p = get_msg();
		while (p != NULL) {
			if (p->msg.type < INTER_MSG_BASE)
				__nl_notify_event(&p->msg);
			else
				process_cmd((struct req_msg_head *)&p->msg);

			kfree(p);
			p = get_msg();
		}
	}
	return 0;
}

/* netlink init function */
static int netlink_handle_init(void)
{
	struct netlink_kernel_cfg nb_nl_cfg = {
		.input = netlink_handle_rcv,
	};

	g_ctx.nlfd = netlink_kernel_create(&init_net,
		NETLINK_OLLIE, &nb_nl_cfg);

	if (!g_ctx.nlfd) {
		hwlog_info("%s: netlink_handle_init failed\n", __func__);
		return -1;
	}
	hwlog_info("%s: netlink_handle_init success\n", __func__);

	sema_init(&g_ctx.sema, 0);

	g_ctx.task = kthread_run(netlink_handle_thread, NULL, "netlink_handle");
	if (IS_ERR(g_ctx.task)) {
		hwlog_err("%s:failed to create thread\n", __func__);
		g_ctx.task = NULL;
		return -1;
	}
	g_ctx.chan_state = NETLINK_HANDLE_INIT;
	g_ctx.msg_head.prev = &g_ctx.msg_head;
	g_ctx.msg_head.next = &g_ctx.msg_head;

	return 0;
}

/* netlink deinit function */
static void netlink_handle_exit(void)
{
	if (g_ctx.nlfd && g_ctx.nlfd->sk_socket) {
		sock_release(g_ctx.nlfd->sk_socket);
		g_ctx.nlfd = NULL;
	}

	if (g_ctx.task) {
		kthread_stop(g_ctx.task);
		g_ctx.task = NULL;
	}
}

static int __init netlink_handle_module_init(void)
{
	msg_process *fn = NULL;

	if (netlink_handle_init()) {
		hwlog_err("%s:init netlink_handle module failed\n", __func__);
		g_ctx.chan_state = NETLINK_HANDLE_EXIT;
		return -1;
	}

	memset(&g_ctx.model_cb[0], 0, MODEL_NUM * sizeof(msg_process *));
#ifdef CONFIG_HW_NETWORK_QOE
	fn = ip_para_collec_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init ip para failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[IP_PARA_COLLEC] = fn;
#endif

#ifdef CONFIG_HW_PACKET_FILTER_BYPASS
	/* init the packet filter bypass module */
	fn = hw_packet_filter_bypass_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init packet filter bypass failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[PACKET_FILTER_BYPASS] = fn;
#endif

	fn = tcp_para_collec_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init tcp para failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[TCP_PARA_COLLEC] = fn;

#ifdef CONFIG_HW_NETWORK_SLICE
	fn = network_slice_management_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init uid process bind failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[NETWORK_SLICE_MANAGEMENT] = fn;
#endif
	fn = sock_destroy_handler_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init sock destroy handler failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[SOCK_DESTROY_HANDLER] = fn;

#ifdef CONFIG_HW_ICMP_PING_DETECT
	fn = icmp_ping_detect_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init icmp ping detect failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[ICMP_PING_DETECT] = fn;
#endif

#ifdef CONFIG_TCP_CONG_ALGO_CTRL
	/* init the dynamic control tcp congestion algorithm module */
	fn = hw_tcp_cong_algo_ctrl_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init tcp Ca controller failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[TCP_CONG_ALGO_CTRL] = fn;
#endif

#ifdef CONFIG_HW_PACKET_TRACKER
	fn = hw_pt_init(nl_notify_event);
	if (fn == NULL) {
		hwlog_err("%s:init PACKET TRACKER failed\n", __func__);
		return -1;
	}
	g_ctx.model_cb[PACKET_TRACKER] = fn;
#endif

#ifdef CONFIG_CHR_NETLINK_MODULE
	chr_manager_init(nl_notify_event);
#endif

	hwlog_info("%s:netlink_handle module inited:\n", __func__);
	return 0;
}

static void __exit netlink_handle_module_exit(void)
{
#ifdef CONFIG_HW_NETWORK_QOE
	ip_para_collec_exit();
#endif
	g_ctx.chan_state = NETLINK_HANDLE_EXIT;
	netlink_handle_exit();
}

module_init(netlink_handle_module_init);
module_exit(netlink_handle_module_exit);
