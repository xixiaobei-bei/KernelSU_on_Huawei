
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is detect whether the BPF
 *              and iptables rule drop the skb.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2019-04-18
 */

#include "hw_packet_filter_bypass.h"

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/uidgid.h>
#include <net/sock.h>
#include <net/inet_sock.h>

#include "hw_booster_common.h"

#define NOTIFY_BUF_LEN 512
#define MAX_LISTENING_UID_NUM 10

#define REPORT_THRESHOLD_PKTS 1
#define IP_REPORT_THRESHOLD_PKTS 10
#define DETECT_PEIROID (30 * HZ)

enum report_hooks {
	HOOK_IP = 0,
	HOOK_EGRESS,
	HOOK_INGRESS,
	HOOK_LOCAL_OUT,
	HOOK_LOCAL_IN,
};

struct packet_drop_stats {
	int hook;
	u32 pkts;
	unsigned long ts_reset;

	bool reported_drop;
	bool reported_recovery;
};

struct uid_info {
	int uid;
	bool bypass;

	struct packet_drop_stats ip_in_stats;
	struct packet_drop_stats local_out_drop_stats;
	struct packet_drop_stats bpf_egress_drop_stats;
	struct packet_drop_stats local_in_drop_stats;
	struct packet_drop_stats bpf_ingress_drop_stats;
};

struct listening_uids {
	struct uid_info infos[MAX_LISTENING_UID_NUM];
	int nums;

	spinlock_t lock;
};

static struct listening_uids listen_uids;
static notify_event *notifier = NULL;

static void reset_uid_info(struct uid_info *info, int uid)
{
	memset(info, 0, sizeof(struct uid_info));
	info->uid = uid;
	info->ip_in_stats.hook = HOOK_IP;
	info->bpf_egress_drop_stats.hook = HOOK_EGRESS;
	info->bpf_ingress_drop_stats.hook = HOOK_INGRESS;
	info->local_in_drop_stats.hook = HOOK_LOCAL_IN;
	info->local_out_drop_stats.hook = HOOK_LOCAL_OUT;
}

static bool add_listening_uid(int uid)
{
	int i;

	if (listen_uids.nums >= MAX_LISTENING_UID_NUM)
		return false;
	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid) {
			reset_uid_info(&listen_uids.infos[i], uid);
			return true;
		}
	}
	i = listen_uids.nums;
	reset_uid_info(&listen_uids.infos[i], uid);
	++listen_uids.nums;
	return true;
}

static bool del_listening_uid(int uid)
{
	int i;

	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid) {
			for (; i < (listen_uids.nums - 1); ++i)
				listen_uids.infos[i] = listen_uids.infos[i + 1];
			--listen_uids.nums;
			return true;
		}
	}
	return false;
}

static bool upate_bypass_flag(int uid, bool bypass)
{
	int i;

	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid) {
			listen_uids.infos[i].bypass = bypass;
			return true;
		}
	}
	return false;
}

static void do_commands(struct req_msg_head *msg)
{
	int uid;
	int num;
	int len;
	int i;
	char *p = NULL;
	bool ret = false;

	if (!msg || msg->len <= sizeof(struct req_msg_head)) {
		pr_err("too small\n");
		return;
	}

	len = msg->len - sizeof(struct req_msg_head);
	p = (char *)msg + sizeof(struct req_msg_head);
	if (len <= sizeof(int))
		return;
	num = *(int *)p;
	len -= sizeof(int);
	p += sizeof(int);
	if (len < (sizeof(int) * num))
		return;

	spin_lock_bh(&listen_uids.lock);
	for (i = 0; i < num; ++i) {
		uid = *(int *)p;
		if (uid < 0)
			break;
		switch (msg->type) {
		case ADD_FG_UID:
			ret = add_listening_uid(uid);
			break;
		case DEL_FG_UID:
			ret = del_listening_uid(uid);
			break;
		case BYPASS_FG_UID:
			ret = upate_bypass_flag(uid, true);
			break;
		case NOPASS_FG_UID:
			ret = upate_bypass_flag(uid, false);
			break;
		default:
			ret = false;
			break;
		}
		if (!ret)
			break;
		p += sizeof(int);
	}
	spin_unlock_bh(&listen_uids.lock);
}

static void notify_drop_event(int uid, struct packet_drop_stats *stats)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;

	if (!notifier)
		return;

	// type
	assign_short(p, PACKET_FILTER_DROP_RPT);
	skip_byte(p, sizeof(s16));
	// 16 eq len(2B type + 2B len + 4B uid + 4B hook + 4B pkts)
	assign_short(p, 16);
	skip_byte(p, sizeof(s16));
	// uid
	assign_int(p, uid);
	skip_byte(p, sizeof(int));
	// hook
	assign_int(p, stats->hook);
	skip_byte(p, sizeof(int));
	// pkts
	assign_int(p, stats->pkts);
	skip_byte(p, sizeof(int));

	notifier((struct res_msg_head *)event);
}

static void notify_recovery_event(int uid, struct packet_drop_stats *stats)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;

	if (!notifier)
		return;

	// type
	assign_short(p, PACKET_FILTER_RECOVERY_RPT);
	skip_byte(p, sizeof(s16));
	// 12 eq len((2B type + 2B len + 4B uid + 4B hook))
	assign_short(p, 12);
	skip_byte(p, sizeof(s16));
	// uid
	assign_int(p, uid);
	skip_byte(p, sizeof(int));
	// hook
	assign_int(p, stats->hook);
	notifier((struct res_msg_head *)event);
}

static void update_packet_count(int uid, struct packet_drop_stats *stats)
{
	unsigned long now = jiffies;
	int threshold;

	if (stats->hook == HOOK_IP)
		threshold = IP_REPORT_THRESHOLD_PKTS;
	else
		threshold = REPORT_THRESHOLD_PKTS;
	/*
	 * If the dropped packets num exceeds the REPORT_THRESHOLD
	 *  within DETECTION_PERIOD seconds, think it as an exception
	 */
	stats->pkts = stats->pkts + 1;
	if ((stats->pkts >= threshold) && (!stats->reported_drop)) {
		notify_drop_event(uid, stats);
		stats->reported_drop = true;
		stats->reported_recovery = false;
		stats->pkts = 0;
	}

	if (!stats->ts_reset)
		stats->ts_reset = now;
	if (time_after(now, stats->ts_reset + DETECT_PEIROID)) {
		stats->ts_reset = now + DETECT_PEIROID;
		if (stats->pkts >= threshold && stats->reported_drop)
			notify_drop_event(uid, stats);
		stats->pkts = 0;
	}
}

static int update_filter_pkts(struct uid_info *info,
	struct packet_drop_stats *stats, int pass)
{
	int ret = 0;

	if (pass == PASS) {
		stats->pkts = 0;
		/*
	 	 * cond1: already notify the drop event to user
	 	 * cond2: not notify the recovery event
	 	 */
		if (stats->reported_drop && !stats->reported_recovery) {
			notify_recovery_event(info->uid, stats);
			stats->reported_recovery = true;
			stats->reported_drop = false;
			stats->ts_reset = 0;
		}
	} else {
		update_packet_count(info->uid, stats);
		ret = info->bypass;
	}
	return ret;
}

static int update_ip_in_pkts(struct uid_info *info,
	struct packet_drop_stats *stats, int pass)
{
	if (pass != PASS)
		return 0;

	update_packet_count(info->uid, stats);
	return 0;
}

static int update_ip_out_pkts(struct uid_info *info,
	struct packet_drop_stats *stats, int pass)
{
	if (pass != PASS)
		return 0;

	stats->pkts = 0;
	/*
	 * cond1: already notify the drop event to user
	 * cond2: not notify the recovery event
	 * cond3: pass is not lead by bypass
	 */
	if (stats->reported_drop && (!stats->reported_recovery) && (!info->bypass)) {
		notify_recovery_event(info->uid, stats);
		stats->reported_recovery = true;
		stats->reported_drop = false;
		stats->ts_reset = 0;
	}
	return 0;
}

int hw_translate_hook_num(int af, int hook)
{
	int ret;

	switch (hook) {
	case NF_INET_PRE_ROUTING:
		ret = HW_PFB_INET_PRE_ROUTING;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_PRE_ROUTING;
		break;
	case NF_INET_LOCAL_IN:
		ret = HW_PFB_INET_LOCAL_IN;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_LOCAL_IN;
		break;
	case NF_INET_FORWARD:
		ret = HW_PFB_INET_FORWARD;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_FORWARD;
		break;
	case NF_INET_LOCAL_OUT:
		ret = HW_PFB_INET_LOCAL_OUT;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_LOCAL_OUT;
		break;
	case NF_INET_POST_ROUTING:
		ret = HW_PFB_INET_POST_ROUTING;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_POST_ROUTING;
		break;
	default:
		ret = HW_PFB_HOOK_INVALID;
		break;
	}
	return ret;
}

int hw_translate_verdict(u32 verdict)
{
	if (verdict == NF_ACCEPT || verdict == NF_STOP)
		return PASS;
	else if ((verdict & NF_VERDICT_MASK) == NF_DROP)
		return DROP;
	else
		return IGNORE;
}

bool hw_hook_bypass_skb(int af, int hook, struct sk_buff *skb)
{
	/* 0 means do not influence the orignal procedure */
	struct net_device *in = NULL;
	struct net_device *out = NULL;
	struct dst_entry *dst = NULL;

	if (notifier == NULL)
		return 0;
	if (af != AF_INET && af != AF_INET6)
		return 0;
	if (!skb)
		return 0;

	switch (hook) {
	case NF_INET_LOCAL_OUT:
		dst = skb_dst(skb);
		if (likely(dst))
			out = dst->dev;
		break;
	case NF_INET_LOCAL_IN:
		out = skb->dev;
		break;
	default:
		break;
	}
	if (!in && !out)
		return 0;
	return hw_bypass_skb(af, HW_PFB_HOOK_ICMP, NULL, skb, in, out, PASS);
}

static int hw_pfb_hooks_info_judge(int i, int hook, int pass)
{
	int ret = 0;
	if (i < listen_uids.nums) {
		switch (hook) {
		case HW_PFB_INET_LOCAL_OUT:
		case HW_PFB_INET6_LOCAL_OUT:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].local_out_drop_stats, pass);
			break;
		case HW_PFB_INET_BPF_EGRESS:
		case HW_PFB_INET6_BPF_EGRESS:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].bpf_egress_drop_stats, pass);
			break;
		case HW_PFB_INET_IP_XMIT:
		case HW_PFB_INET6_IP_XMIT:
			ret = update_ip_in_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].ip_in_stats, pass);
			break;
		case HW_PFB_INET_DEV_XMIT:
		case HW_PFB_INET6_DEV_XMIT:
			ret = update_ip_out_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].ip_in_stats, pass);
			break;
		case HW_PFB_INET_LOCAL_IN:
		case HW_PFB_INET6_LOCAL_IN:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].local_in_drop_stats, pass);
			break;
		case HW_PFB_INET_BPF_INGRESS:
		case HW_PFB_INET6_BPF_INGRESS:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].bpf_ingress_drop_stats, pass);
			break;
		case HW_PFB_HOOK_ICMP:
			ret = listen_uids.infos[i].bypass;
			break;
		default:
			break;
		}
	}

	return ret;
}

/* if we should bypass this skb, return 1, else return 0 */
bool hw_bypass_skb(int af, int hook, const struct sock *sk,
	struct sk_buff *skb, struct net_device *in,
	struct net_device *out, int pass)
{
	uid_t uid;
	int i;
	int ret = 0;

	if (notifier == NULL)
		return ret;
	if (!skb)
		return ret;
	if (af != AF_INET && af != AF_INET6)
		return ret;
	if (pass == IGNORE)
		return ret;
	if (sk)
		sk = sk_to_full_sk((struct sock *)sk);
	else
		sk = sk_to_full_sk(skb->sk);
	uid = hw_get_sock_uid((struct sock *)sk);
	if (uid == overflowuid)
		return ret;
	if (!is_ds_rnic(in) && !is_ds_rnic(out))
		return ret;

	spin_lock_bh(&listen_uids.lock);
	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid)
			break;
	}
	ret = hw_pfb_hooks_info_judge(i, hook, pass);
	spin_unlock_bh(&listen_uids.lock);

	return ret;
}

msg_process* __init hw_packet_filter_bypass_init(notify_event *notify)
{
	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}
	spin_lock_init(&listen_uids.lock);
	notifier = notify;
	return do_commands;
}
