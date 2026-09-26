/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: This file used by the netfilter hook for app download monitor
 *              and ad filter.
 * Author: chenzhongxian@huawei.com
 * Create: 2016-05-28
 */

#include "nf_hw_hook.h"

#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <net/ip.h>
#include <uapi/linux/netlink.h>

#include "nf_hw_common.h"
#include "nf_app_dl_monitor.h"
#include "nf_ad_filter.h"

#define REPORTCMD NETLINK_DOWNLOADEVENT_TO_USER

static spinlock_t g_netlink_lock; /* lock for netlink array */

struct tag_hw_msg2knl {
	struct nlmsghdr hdr;
	int opt;
	char data[1];
};

struct appdload_nl_packet_msg {
	int event;
	char url[1];
};

static uid_t find_skb_uid(const struct sk_buff *skb)
{
	const struct file *filp = NULL;
	struct sock *sk = NULL;
	uid_t sock_uid = 0xffffffff;

	if (!skb)
		return sock_uid;
	sk = skb->sk;
	if (!sk)
		return 0xffffffff;
	if (sk && sk->sk_state == TCP_TIME_WAIT)
		return sock_uid;
	filp = sk->sk_socket ? sk->sk_socket->file : NULL;
	if (filp)
		sock_uid = from_kuid(&init_user_ns, filp->f_cred->fsuid);
	else
		return 0xffffffff;
	return sock_uid;
}

static struct sock *g_hw_nlfd = NULL;
static unsigned int g_uspa_pid;

void proc_cmd(int cmd, int opt, const char *data)
{
	if (cmd == NETLINK_SET_AD_RULE_TO_KERNEL)
		add_ad_rule(&g_adlist_info, data, (opt == 0 ? false : true));
	else if (cmd == NETLINK_CLR_AD_RULE_TO_KERNEL)
		clear_ad_rule(&g_adlist_info, opt, data);
	else if (cmd == NETLINK_OUTPUT_AD_TO_KERNEL)
		output_ad_rule(&g_adlist_info);
	else if (cmd == NETLINK_SET_APPDL_RULE_TO_KERNEL)
		add_appdl_rule(data, (opt == 0 ? false : true));
	else if (cmd == NETLINK_CLR_APPDL_RULE_TO_KERNEL)
		clear_appdl_rule(opt, data);
	else if (cmd == NETLINK_OUTPUT_APPDL_TO_KERNEL)
		output_appdl_rule();
	else if (cmd == NETLINK_APPDL_CALLBACK_TO_KERNEL)
		download_notify(opt, data);
	else if (cmd == NETLINK_SET_INSTALL_RULE_TO_KERNEL)
		add_ad_rule(&g_deltalist_info, data, (opt == 0 ? false : true));
	else if (cmd == NETLINK_CLR_INSTALL_RULE_TO_KERNEL)
		clear_ad_rule(&g_deltalist_info, opt, data);
	else if (cmd == NETLINK_OUTPUT_INS_DELTA_TO_KERNEL)
		output_ad_rule(&g_deltalist_info);
	else
		pr_info("hwad:kernel_hw_receive cmd=%d\n", cmd);
}

int check_str(struct nlmsghdr *nlh)
{
	int pos;
	char *data = NULL;
	struct tag_hw_msg2knl *hmsg = NULL;

	hmsg = (struct tag_hw_msg2knl *)nlh;
	data = (char *)&(hmsg->data[0]);
	pos = nlh->nlmsg_len;
	pos -= sizeof(struct nlmsghdr);
	pos -= sizeof(int) + 1;

	if (pos < 0)
		return 1;

	data[pos] = 0;
	return 0;
}
/* receive cmd for netd */
static void kernel_hw_receive(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct tag_hw_msg2knl *hmsg = NULL;
	struct sk_buff *skb = NULL;
	char *data = NULL;

	if (__skb == NULL) {
		pr_err("Invalid parameter: zero pointer reference(__skb)\n");
		return;
	}
	skb = skb_get(__skb);
	if (skb == NULL) {
		pr_err("wifi_tcp_nl_receive skb = NULL\n");
		return;
	}
	if (skb->len < NLMSG_HDRLEN) {
		kfree_skb(skb);
		return;
	}
	nlh = nlmsg_hdr(skb);
	if (nlh == NULL) {
		pr_err("wifi_tcp_nl_receive  nlh = NULL\n");
		kfree_skb(skb);
		return;
	}
	if ((nlh->nlmsg_len >= sizeof(struct nlmsghdr)) &&
		(skb->len >= nlh->nlmsg_len)) {
		if (nlh->nlmsg_type == NETLINK_REG_TO_KERNEL) {
			g_uspa_pid = nlh->nlmsg_pid;
		} else if (nlh->nlmsg_type == NETLINK_UNREG_TO_KERNEL) {
			g_uspa_pid = 0;
		} else {
			hmsg = (struct tag_hw_msg2knl *)nlh;
			data = (char *)&(hmsg->data[0]);
			if (check_str(nlh)) {
				kfree_skb(skb);
				return;
			}
			proc_cmd(nlh->nlmsg_type, hmsg->opt, data);
		}
	}
	kfree_skb(skb);
}

/* notify event to netd  */
static int notify_event(int event, int pid, const char *url)
{
	int ret = -1;
	int size;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct appdload_nl_packet_msg *packet = NULL;
	int urllen = strlen(url);

	if (!pid || !g_hw_nlfd || g_uspa_pid == 0) {
		pr_info("hwad:cannot notify pid 0 or nlfd 0\n");
		ret = -1;
		goto end;
	}
	size = sizeof(struct appdload_nl_packet_msg) + urllen + 1;
	skb = nlmsg_new(size, GFP_ATOMIC);
	if (!skb) {
		pr_info("hwad: alloc skb fail\n");
		ret = -1;
		goto end;
	}
	nlh = nlmsg_put(skb, 0, 0, 0, size, 0);
	if (!nlh) {
		pr_info("hwad: %s fail\n", __func__);
		kfree_skb(skb);
		skb = NULL;
		ret = -1;
		goto end;
	}
	packet = nlmsg_data(nlh);
	memset(packet, 0, size);
	memcpy(packet->url, url, urllen);
	packet->event = event;
	spin_lock_bh(&g_netlink_lock);
	ret = netlink_unicast(g_hw_nlfd, skb, pid, MSG_DONTWAIT);
	spin_unlock_bh(&g_netlink_lock);
end:
	return ret;
}

static void netlink_init(void)
{
	struct netlink_kernel_cfg hwcfg = {
		.input = kernel_hw_receive,
	};
	g_hw_nlfd = netlink_kernel_create(&init_net, NETLINK_HW_NF, &hwcfg);
	if (!g_hw_nlfd)
		pr_info("hwad: %s failed NETLINK_HW_NF\n", __func__);
}

static inline void send_reject_error(unsigned int uid, struct sk_buff *skb)
{
	if (skb->sk) {
		skb->sk->sk_err = ECONNRESET;
		skb->sk->sk_error_report(skb->sk);
	}
}

static bool download_app_pro_pre_check(struct sk_buff *skb,
	const char **tempurl, int *opt, unsigned int *ret_val)
{
	if (!tempurl || !(*tempurl)) {
		*ret_val = NF_ACCEPT;
		return true;
	}
	*opt = get_select(skb);
	if (*opt != DLST_NOT) {
		kfree(*tempurl);
		*tempurl = NULL;
		if (*opt == DLST_REJECT) {
			*ret_val = NF_DROP;
			return true;
		} else if (*opt == DLST_ALLOW) {
			*ret_val = NF_ACCEPT;
			return true;
		} else if (*opt == DLST_WAIT) {
			*ret_val = 0xffffffff;
			return true;
		}
	}

	return false;
}

unsigned int download_app_pro(struct sk_buff *skb, unsigned int uid,
	const char *data, int dlen, char *ip)
{
	char *tempurl = get_url_path(data, dlen); /* new 501 */
	struct dl_info *node = NULL;
	char *buf = NULL;
	int iret;
	char *url = NULL;
	int opt;
	unsigned int ret_val;

	if (download_app_pro_pre_check(skb, &tempurl, &opt, &ret_val))
		return ret_val;
	if (tempurl) {
		node = get_download_monitor(skb, uid, tempurl);
		kfree(tempurl);
		tempurl = NULL;
	}
	if (!node) {
		pr_info("hwad:get_download_monitor=NULL\n ");
		return NF_ACCEPT;
	}
	if (node->bwait)
		return 0xffffffff;
	url = get_url_form_data(data, dlen);
	if (!url) {
		free_node(node);
		return NF_ACCEPT;
	}
	buf = get_report_msg(node->dlid, uid, url, ip);
	kfree(url);
	if (!buf) {
		free_node(node);
		return NF_ACCEPT;
	}
	iret = notify_event(REPORTCMD, g_uspa_pid, buf);
	kfree(buf); /* free 801 */
	if (iret < 0) {
		free_node(node);
		return NF_ACCEPT;
	}
	if (!in_irq() && !in_interrupt())
		opt = wait_user_event(node);
	else
		pr_info("hwad:in_irq=%d in_interrupt=%d",
			(int)in_irq(), (int)in_interrupt());
	if (opt != DLST_NOT) {
		if (opt == DLST_ALLOW)
			return NF_ACCEPT;
		return NF_DROP;
	}
	return 0xffffffff;
}

static bool hw_match_httppack_notify_event(unsigned int uid,
	const char *data, int dlen, bool bmatch, unsigned int *iret, char *ip)
{
	char *tempurl = NULL;
	char *buf = NULL;
	int ret;

	if (match_ad_uid(&g_deltalist_info, uid) &&
		match_ad_url(&g_deltalist_info, uid, data, dlen) &&
		get_cur_time() - g_deltalist_info.lastrepot >
			REPORT_LIMIT_TIMR) {
		/* limit the report time as 1 second */
		g_deltalist_info.lastrepot = get_cur_time();
		tempurl = get_url_form_data(data, dlen);
		if (tempurl) {
			buf = get_report_msg(NETLINK_REPORT_DLID_TO_USER,
				uid, tempurl, ip);
			if (buf) {
				ret = notify_event(REPORTCMD, g_uspa_pid, buf);
				kfree(buf);
			} else {
				pr_info("hwad:report to systemmanager false\n");
			}
			kfree(tempurl);
		} else {
			pr_info("hwad:notify_event delta tempurl=NULL\n");
		}
	}
	if (!bmatch && match_ad_uid(&g_adlist_info, uid) &&
		match_ad_url(&g_adlist_info, uid, data, dlen)) {
		/* limit the report time as 1 second */
		if (get_cur_time() - g_adlist_info.lastrepot >
			REPORT_LIMIT_TIMR) {
			g_adlist_info.lastrepot = get_cur_time();
			/* report the ad block to systemmanager */
			tempurl = get_url_form_data(data, dlen);
			if (!tempurl) {
				pr_info("hwad:notify_event ad tempurl=NULL\n");
				return true;
			}
			buf = get_report_msg(REPORT_MSG_TYPE, uid, tempurl, ip);
			if (buf) {
				ret = notify_event(REPORTCMD, g_uspa_pid, buf);
				kfree(buf);
			} else {
				pr_info("hwad:report to systemmanager false\n");
			}
			kfree(tempurl);
		}
		*iret = NF_DROP;
	}
	return false;
}

unsigned int hw_match_httppack(struct sk_buff *skb, unsigned int uid,
	const char *data, int dlen, char *ip)
{
	unsigned int iret = NF_ACCEPT;
	bool bmatch = false;
	char *tempurl = NULL;

	if (match_appdl_uid(uid)) {
		if (match_appdl_url(data, dlen)) {
			bmatch = true;
			iret = download_app_pro(skb, uid, data, dlen, ip);
		}
	}
	if (hw_match_httppack_notify_event(uid, data, dlen, bmatch, &iret, ip))
		return iret;
	if (iret == NF_DROP || uid == 0xffffffff) {
		tempurl = get_url_path(data, dlen);
		if (!tempurl)
			return iret;
		if (is_droplist(tempurl, strlen(tempurl))) {
			iret = NF_DROP;
		} else {
			if (iret == NF_DROP)
				add_to_droplist(tempurl, strlen(tempurl));
		}
		kfree(tempurl);
	}
	return iret;
}
char g_get_s[] = {'G', 'E', 'T', 0};
char g_post_s[] = {'P', 'O', 'S', 'T', 0};
char g_http_s[] = {'H', 'T', 'T', 'P', 0};

static unsigned int net_hw_hook_localout(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcp = NULL;
	char *pstart = NULL;
	unsigned int iret = NF_ACCEPT;
	int dlen;
	char *data = NULL;

	if (skb_headlen(skb) < sizeof(struct iphdr) ||
		ip_hdrlen(skb) < sizeof(struct iphdr))
		return NF_ACCEPT;

	tcp = tcp_hdr(skb);
	iph = ip_hdr(skb);
	if (!iph || !tcp) {
		pr_info("\nhwad:%s drop NULL==iph\n", __func__);
		return NF_ACCEPT;
	}
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	pstart = skb->data;
	if (pstart) {
		struct tcphdr *th = NULL;

		th = (struct tcphdr *)((__u32 *)iph + iph->ihl);
		data = (char *)((__u32 *)th + th->doff);
		dlen = skb->len - (data - (char *)iph) - skb->data_len;
		if (dlen < SKB_LEN_MIN || dlen > SKB_LEN_MAX || data < pstart ||
			(pstart + skb_headlen(skb)) < (data + dlen))
			return NF_ACCEPT;
		dlen = (dlen > SKB_LEN_AVG ? SKB_LEN_AVG : dlen);
		if (strfindpos(data, g_get_s, SKB_LEN_MIN) ||
			strfindpos(data, g_post_s, SKB_LEN_MIN)) {
			unsigned int uid;
			char ip[IPV4_LEN];

			sprintf(ip, "%d.%d.%d.%d:%d",
				(iph->daddr & 0x000000FF)>>0,
				(iph->daddr & 0x0000FF00)>>SKB_HEAD_MASK_8,
				(iph->daddr & 0x00FF0000)>>SKB_HEAD_MASK_16,
				(iph->daddr & 0xFF000000)>>SKB_HEAD_MASK_24,
				htons(tcp->dest));
			uid = find_skb_uid(skb);
			iret = hw_match_httppack(skb, uid, data, dlen, ip);
			if (iret == NF_DROP)
				send_reject_error(uid, skb);
		}
	}
	if (iret == 0xffffffff)
		iret = NF_DROP;
	return iret;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = net_hw_hook_localout,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
};

static int __init nf_init(void)
{
	int ret;

	init_appdl();
	init_ad();
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	ret = nf_register_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	ret = nf_register_net_hooks(&init_net,
		net_hooks, ARRAY_SIZE(net_hooks));
#endif
	if (ret) {
		pr_info("hwad:%s ret=%d  ", __func__, ret);
		return -1;
	}
	netlink_init();
	spin_lock_init(&g_netlink_lock);
	pr_info("hwad:%s success\n", __func__);
	return 0;
}

static void __exit nf_exit(void)
{
	uninit_ad();
	uninit_appdl();
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	nf_unregister_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	nf_unregister_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
#endif
}

module_init(nf_init);
module_exit(nf_exit);

MODULE_LICENSE("Dual BSD");
MODULE_AUTHOR("c00179874");
MODULE_DESCRIPTION("HW Netfilter NF_HOOK");
MODULE_VERSION("1.0.1");
MODULE_ALIAS("HW Netfilter 01");
