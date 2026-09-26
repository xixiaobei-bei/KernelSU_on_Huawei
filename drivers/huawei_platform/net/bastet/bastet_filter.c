/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: Provide bastet filter.
 * Author: zhangkaige@huawei.com
 * Create: 2015-12-01
 */

#include <linux/fdtable.h>
#include <linux/file.h>
#include <net/tcp.h>
#include <linux/list.h>
#include <uapi/linux/netfilter_ipv4.h>
#include <uapi/linux/netfilter_ipv6.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <net/icmp.h> /* icmp_send */
#include <uapi/linux/icmp.h> /* ICMP_NET_ANO ICMP_DEST_UNREACH */

extern int get_uid_by_pid(struct set_process_info *info);

struct st_proc_info_node {
	struct set_process_info info;
	struct list_head list;
};

struct list_head g_white_list_head;
static spinlock_t proc_info_lock;
static int g_special_uid;

/*
 * get_process_info_by_skb() - get uid and pid by skb
 * @skb: sock buffer
 * @info: process info buffer.
 *
 * get uid and pid from sock buffer, then fill them into
 * the set_process_info.
 *
 * Return:No
 */
static void get_process_info_by_skb(struct sk_buff *skb,
	struct set_process_info *info)
{
	if ((skb == NULL) || (info == NULL)) {
		bastet_loge("invalid parameter");
		return;
	}

	if (skb->sk == NULL) {
		bastet_loge("skb->sk is NULL");
		return;
	}
	info->uid = (int32_t)sock_i_uid(skb->sk).val;
	info->pid = current->pid;
}

/*
 * hook_local_out_cb() - hook local out cb.
 * @*ops: point to the data.
 * @*skb: sock buffer.
 * @*state: hook state.
 *
 * netfilter hook call back function on ipv4 LOCAL_OUT
 * hook position, here is NF_INET_LOCAL_OUT
 * net_device info, available on in hook
 * net_device info, available on out hook
 * netfilter call back function
 *
 * Return:NF_ACCEPT -- we should all return this to make sure that
 * sk_buff continue to be transfered
 */
static unsigned int hook_local_out_cb(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct set_process_info info;
	struct list_head *pos = NULL;
	struct st_proc_info_node *tmp_node = NULL;
	bool found = false;

	info.pid = -1;
	info.uid = -1;
	get_process_info_by_skb(skb, &info);

	bastet_logi("uid=%d,pid=%d", info.uid, info.pid);
	spin_lock_bh(&proc_info_lock);
	if (list_empty(&g_white_list_head)) {
		spin_unlock_bh(&proc_info_lock);
		bastet_loge("ipv4 NF_DROP1");
		return NF_DROP;
	}
	list_for_each(pos, &g_white_list_head) {
		tmp_node = list_entry(pos, struct st_proc_info_node, list);
		bastet_logi("uid=%d,pid=%d",
			tmp_node->info.uid, tmp_node->info.pid);
		if (tmp_node->info.uid != info.uid)
			continue;
		if (tmp_node->info.uid != g_special_uid) {
			bastet_logi("uid=%d not %d match success",
				info.uid, g_special_uid);
			found = true;
			break;
		}
		if (tmp_node->info.pid == info.pid) {
			bastet_logi("uid=%d,pid=%d match success",
				info.uid, info.pid);
			found = true;
			break;
		}
	}

	if (found) {
		bastet_loge("ipv4 NF_ACCEPT");
		spin_unlock_bh(&proc_info_lock);
		return NF_ACCEPT;
	}
	bastet_loge("ipv4 NF_DROP2");
	if (skb->sk) {
		skb->sk->sk_err = icmp_err_convert[ICMP_NET_ANO].errno;
		skb->sk->sk_error_report(skb->sk);
	}
	spin_unlock_bh(&proc_info_lock);
	return NF_DROP;
}

static struct nf_hook_ops ipv4_local_out_ops = {
	.hook = hook_local_out_cb,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_LOCAL_OUT,
	.priority = NF_IP_PRI_FIRST,
};

static struct nf_hook_ops ipv6_local_out_ops = {
	.hook = hook_local_out_cb,
	.pf = NFPROTO_IPV6,
	.hooknum = NF_INET_LOCAL_OUT,
	.priority = NF_IP6_PRI_FIRST,
};

/*
 * add_proc_info_to_white_list() - add proc info to list.
 * @info: set_process_info.
 *
 * add process_info to the white list
 *
 * Return:0 is success
 * Otherwise is fail.
 */
static int add_proc_info_to_white_list(struct set_process_info *info)
{
	struct st_proc_info_node *new_node = NULL;
	struct st_proc_info_node *tmp_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (info == NULL) {
		bastet_loge("invalid parameter");
		return -EFAULT;
	}
	spin_lock_bh(&proc_info_lock);
	if (!list_empty(&g_white_list_head)) {
		list_for_each_safe(pos, n, &g_white_list_head) {
			tmp_node = list_entry(pos,
				struct st_proc_info_node, list);
			if (tmp_node->info.pid == info->pid) {
				bastet_logi("pid =%d has existed in white list",
					info->pid);
				spin_unlock_bh(&proc_info_lock);
				return 0;
			}
		}
	}
	new_node = kzalloc(sizeof(*new_node), GFP_ATOMIC);
	if (new_node == NULL) {
		bastet_loge("kzalloc proc_info_node failed");
		spin_unlock_bh(&proc_info_lock);
		return -EFAULT;
	}

	if (get_uid_by_pid(info) != 0) {
		bastet_loge("get_uid_by_pid fail");
		kfree(new_node);
		spin_unlock_bh(&proc_info_lock);
		return -EFAULT;
	}
	new_node->info.uid = info->uid;
	new_node->info.pid = info->pid;

	bastet_logi("uid=%d, pid=%d add to list",
		new_node->info.uid, new_node->info.pid);
	list_add(&(new_node->list), &g_white_list_head);
	spin_unlock_bh(&proc_info_lock);
	return 0;
}

/*
 * del_proc_info_from_white_list() - delete the info from list.
 * @info: set_process_info.
 *
 * delete the process_info in the white list
 *
 * Return: 0 is success
 * Otherwise is fail.
 */
static int del_proc_info_from_white_list(struct set_process_info *info)
{
	struct st_proc_info_node *tmp_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (info == NULL) {
		bastet_loge("invalid parameter");
		return -EFAULT;
	}
	spin_lock_bh(&proc_info_lock);
	if (!list_empty(&g_white_list_head)) {
		list_for_each_safe(pos, n, &g_white_list_head) {
			tmp_node = list_entry(pos,
				struct st_proc_info_node, list);
			if (tmp_node->info.pid  == info->pid) {
				bastet_logi("uid=%d, pid=%d delete",
					tmp_node->info.uid, tmp_node->info.pid);
				list_del(pos);
				kfree(tmp_node);
				break;
			}
		}
	}
	spin_unlock_bh(&proc_info_lock);
	return 0;
}

/*
 * del_all_proc_info_from_white_list() - delete all proc
 *
 * delete all process_info in the white list
 *
 * Return: 0 is success
 * Otherwise is fail.
 */
static int del_all_proc_info_from_white_list(void)
{
	struct st_proc_info_node *tmp_node = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	spin_lock_bh(&proc_info_lock);
	if (!list_empty(&g_white_list_head)) {
		list_for_each_safe(pos, n, &g_white_list_head) {
			tmp_node = list_entry(pos,
				struct st_proc_info_node, list);
			list_del(pos);
			kfree(tmp_node);
		}
	}
	spin_unlock_bh(&proc_info_lock);
	return 0;
}

/*
 * set_proc_info() - set the info.
 * @info: set_process_info.
 *
 * add/del/del all proc into/from the white list
 * due to different cmd.
 *
 * Return: -EFAULT is invalid cmd.
 * Otherwise is normal.
 */
int set_proc_info(struct set_process_info *info)
{
	int rc;

	if (IS_ERR_OR_NULL(info)) {
		bastet_loge("invalid parameter");
		return -EFAULT;
	}
	switch (info->cmd) {
	case CMD_ADD_PROC_INFO:
		rc = add_proc_info_to_white_list(info);
		break;
	case CMD_DEL_PROC_INFO:
		rc = del_proc_info_from_white_list(info);
		break;
	case CMD_DEL_ALL_INFO:
		rc = del_all_proc_info_from_white_list();
		break;
	default:
		bastet_loge("invalid cmd");
		rc = -EFAULT;
		break;
	}
	return rc;
}

int set_special_uid(int32_t uid)
{
	g_special_uid = uid;
	return 0;
}

/*
 * set_netfilter() - set the filter
 * @state: bool value to set, state true register,false unregister
 *
 * to register or unregister the hook by the state.
 *
 * Return: 0 is success.
 * Otherwise is fail.
 */
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
int set_netfilter(bool state)
{
	static bool is_registered;

	bastet_logi("is_registered=%d", state);

	if (!state) {
		if (is_registered) {
			nf_unregister_hook(&ipv4_local_out_ops);
			nf_unregister_hook(&ipv6_local_out_ops);
			is_registered = false;
		}
		return 0;
	}

	if (nf_register_hook(&ipv4_local_out_ops) < 0) {
		bastet_loge("register ipv4 fail");
		nf_unregister_hook(&ipv4_local_out_ops);
		return -EFAULT;
	}
	if (nf_register_hook(&ipv6_local_out_ops) < 0) {
		bastet_loge("register ipv6 fail");
		nf_unregister_hook(&ipv4_local_out_ops);
		nf_unregister_hook(&ipv6_local_out_ops);
		return -EFAULT;
	}
	is_registered = true;
	return 0;
}
#else
/*
 * set_netfilter() - set the filter
 * @state: bool value to set, state true register,false unregister
 *
 * to register or unregister the hook by the state.
 *
 * Return: 0 is success.
 * Otherwise is fail.
 */
int set_netfilter(bool state)
{
	static bool is_registered;

	bastet_logi("is_registered=%d", state);

	if (!state) {
		if (is_registered) {
			nf_unregister_net_hook(&init_net, &ipv4_local_out_ops);
			nf_unregister_net_hook(&init_net, &ipv6_local_out_ops);
			is_registered = false;
		}
		return 0;
	}

	if (nf_register_net_hook(&init_net, &ipv4_local_out_ops) < 0) {
		bastet_loge("register ipv4 fail");
		nf_unregister_net_hook(&init_net, &ipv4_local_out_ops);
		return -EFAULT;
	}
	if (nf_register_net_hook(&init_net, &ipv6_local_out_ops) < 0) {
		bastet_loge("register ipv6 fail");
		nf_unregister_net_hook(&init_net, &ipv4_local_out_ops);
		nf_unregister_net_hook(&init_net, &ipv6_local_out_ops);
		return -EFAULT;
	}
	is_registered = true;
	return 0;
}
#endif

void bastet_filter_init(void)
{
	spin_lock_init(&proc_info_lock);
	INIT_LIST_HEAD(&g_white_list_head);
}
