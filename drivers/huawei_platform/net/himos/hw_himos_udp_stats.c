/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: This module use for himos udp state.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2018-07-19
 */
#include "hw_himos_udp_stats.h"
#include <linux/errno.h>
#include <linux/genetlink.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <net/genetlink.h>
#include <net/sock.h>

#include "hw_himos_stats_common.h"
#include "hw_himos_stats_report.h"

/*
 * When the udp_stats_info.keepalive exceed this value,
 * the kernel will delete this uid, this mechanism can keep
 * the kernel clean when the user application abnormal
 */
#define KEEPALIVE_MAX 10

struct himos_udp_stats_info {
	/* this is must the first member */
	struct himos_stats_common comm;

	__u32 interval;

	__u32 in_pkts;
	__u32 out_pkts;
	__u32 in_bytes;
	__u32 out_bytes;

	int keepalive;
};

/* timer */
static struct timer_list timer_report;

static void himos_udp_stats_cb(unsigned long arg);

/*
 * Function: himos_alloc_udp_stats_info
 * Description: Allocate the struct stats_info node and init it.
 * return: the new stats info pointer or NULL when failed
 */
static struct himos_udp_stats_info *himos_alloc_udp_stats_info(int flags)
{
	struct himos_udp_stats_info *info = NULL;

	info = kmalloc(sizeof(struct himos_udp_stats_info), flags);
	if (info == NULL)
		return NULL;
	himos_stats_common_reset(&info->comm);
	info->comm.type = HIMOS_STATS_TYPE_UDP;

	info->in_pkts = 0;
	info->out_pkts = 0;
	info->in_bytes = 0;
	info->out_bytes = 0;
	info->interval = 0;
	info->keepalive = 0;

	return info;
}

/*
 * Function: himos_free_udp_stats_info
 * Description: Free the struct stats_info.
 * return: None
 */
static void himos_free_udp_stats_info(struct himos_udp_stats_info *info)
{
	if (info == NULL)
		return;

	kfree(info);
}
static size_t himos_calculate_msg_size(void)
{
	size_t size = 0;

	/* calculate the reply message size */
	size += nla_total_size(sizeof(__s32)); /* uid attr */
	size += nla_total_size(sizeof(__u32)); /* inPkts attr */
	size += nla_total_size(sizeof(__u32)); /* outPkts attr */
	size += nla_total_size(sizeof(__u32)); /* inBytes attr */
	size += nla_total_size(sizeof(__u32)); /* outBytes attr */

	return size;
}

static void himos_clear_counter(struct himos_udp_stats_info *info)
{
	if (info == NULL)
		return;

	/* clear counter */
	info->in_pkts = 0;
	info->out_pkts = 0;
	info->in_bytes = 0;
	info->out_bytes = 0;
}

/*
 * Function: himos_udp_stats_cb
 * Description: The timer callback
 *      NOTE: this function maybe called nested, so it must be reentrant
 * Arguments:
 *  @arg: the uid
 * return: NONE
 */
static void himos_udp_stats_cb(unsigned long arg)
{
	struct himos_aweme_id id = {0};
	struct himos_udp_stats_info *info = NULL;
	possible_net_t net;
	struct sk_buff *skb = NULL;
	size_t size;
	void *reply = NULL;
	struct genlmsghdr *genlhdr = NULL;

	spin_lock_bh(&stats_info_lock);
	id.uid = (__s32)arg;
	info = (struct himos_udp_stats_info *)himos_get_stats_info_by_uid(
		id.uid);
	if (info == NULL)
		goto unlock;

	info->keepalive = info->keepalive + 1;
	if (info->keepalive > KEEPALIVE_MAX) {
		list_del(&info->comm.node);
		himos_free_udp_stats_info(info);
		goto unlock;
	}

	id.portid = info->comm.portid;
	net = info->comm.net;

	size = himos_calculate_msg_size();

	skb = himos_get_nlmsg(id.portid, HIMOS_STATS_UDP_THROUGHPUT_NOTIFY,
		size);
	if (skb == NULL)
		goto unlock;

	/* add the attrs */
	if (nla_put_s32(skb, HIMOS_STATS_ATTR_UID, id.uid) < 0 ||
		nla_put_u32(skb, HIMOS_STATS_ATTR_INPKTS, info->in_pkts) < 0 ||
		nla_put_u32(skb, HIMOS_STATS_ATTR_OUTPKTS,
			info->out_pkts) < 0 ||
		nla_put_u32(skb, HIMOS_STATS_ATTR_INBYTES,
			info->in_bytes) < 0 ||
		nla_put_u32(skb, HIMOS_STATS_ATTR_OUTBYTES,
			info->out_bytes) < 0) {
		pr_err("Bug, the memory error");
		nlmsg_free(skb);
		goto unlock;
	}
	/* send the message */
	genlhdr = nlmsg_data(nlmsg_hdr(skb));
	reply = genlmsg_data(genlhdr);
	genlmsg_end(skb, reply);

	himos_clear_counter(info);

	timer_report.expires = jiffies + (info->interval * HZ / HIMOS_US_MS);
	add_timer(&timer_report);

	spin_unlock_bh(&stats_info_lock);

	himos_notify_result(id.portid, net, skb);
	goto out;

unlock:
	spin_unlock_bh(&stats_info_lock);
out:
	return;
}

int himos_start_udp_stats(struct genl_info *info)
{
	struct himos_udp_stats_info *stats_info = NULL;
	__s32 uid;
	__u32 interval;
	__s32 type;
	struct nlattr *na = NULL;
	int ret = 0;

	/* get the type */
	na = info->attrs[HIMOS_STATS_ATTR_TYPE];
	type = nla_get_s32(na);
	if (type != HIMOS_STATS_TYPE_UDP)
		return -EINVAL;

	/* get the uid */
	na = info->attrs[HIMOS_STATS_ATTR_UID];
	uid = nla_get_s32(na);

	/* get the interval */
	na = info->attrs[HIMOS_STATS_ATTR_INTERVAL];
	interval = nla_get_u32(na);

	spin_lock_bh(&stats_info_lock);
	stats_info = (struct himos_udp_stats_info *)himos_get_stats_info_by_uid(
		uid);
	if (stats_info == NULL) {
		stats_info = himos_alloc_udp_stats_info(GFP_KERNEL);
		if (stats_info == NULL) {
			pr_err("There is no memory when alloc stats_info");
			ret = -ENOMEM;
			goto out;
		}
		/* add the stats_info to global list */
		list_add(&stats_info->comm.node, &stats_info_head);
		stats_info->comm.uid = uid;

		del_timer_sync(&timer_report);

		/* add the timer */
		timer_report.data = uid;
		timer_report.expires = jiffies + (interval * HZ / HIMOS_US_MS);
		add_timer(&timer_report);
	}
	stats_info->comm.type = HIMOS_STATS_TYPE_UDP;
	stats_info->interval = interval;
	stats_info->comm.portid = info->snd_portid;
	stats_info->keepalive = 0;
	write_pnet(&stats_info->comm.net, genl_info_net(info));
	ret = 0;

out:
	spin_unlock_bh(&stats_info_lock);
	return ret;
}

/*
 * Function: himos_stop_udp_stats
 * Description: stop udp statistic
 * return: 0: success, -1: when the uid is invalid
 */
int himos_stop_udp_stats(const struct genl_info *info)
{
	struct himos_udp_stats_info *stats_info = NULL;
	int ret = 0;
	__s32 type;
	__s32 uid;
	struct nlattr *na = NULL;

	/* get the type */
	if (info == NULL || info->attrs == NULL ||
		info->attrs[HIMOS_STATS_ATTR_TYPE] == NULL) {
		pr_err("info ptr are null");
		return -ENOENT;
	}
	na = info->attrs[HIMOS_STATS_ATTR_TYPE];
	type = nla_get_s32(na);
	if (type != HIMOS_STATS_TYPE_UDP)
		return -EINVAL;

	/* get the uid */
	na = info->attrs[HIMOS_STATS_ATTR_UID];
	uid = nla_get_s32(na);

	spin_lock_bh(&stats_info_lock);

	stats_info = (struct himos_udp_stats_info *)himos_get_stats_info_by_uid(
		uid);
	if (stats_info == NULL) {
		pr_err("BUG: stop a invalid uid:%d", uid);
		ret = -ENOENT;
		goto out;
	}
	/* delete the stats from the list */
	list_del(&stats_info->comm.node);
	spin_unlock_bh(&stats_info_lock);

	/* free the stats memory */
	himos_free_udp_stats_info(stats_info);
	return 0;

out:
	spin_unlock_bh(&stats_info_lock);
	return ret;
}

int himos_keepalive_udp_stats(const struct genl_info *info)
{
	struct himos_udp_stats_info *stats_info = NULL;
	int ret = 0;
	__s32 type;
	__s32 uid;
	struct nlattr *na = NULL;

	/* get the type */
	na = info->attrs[HIMOS_STATS_ATTR_TYPE];
	type = nla_get_s32(na);
	if (type != HIMOS_STATS_TYPE_UDP)
		return -EINVAL;

	/* get the uid */
	na = info->attrs[HIMOS_STATS_ATTR_UID];
	uid = nla_get_s32(na);

	spin_lock_bh(&stats_info_lock);

	stats_info = (struct himos_udp_stats_info *)himos_get_stats_info_by_uid(
		uid);
	if (stats_info == NULL) {
		pr_err("BUG: keepalive a invalid uid:%d", uid);
		ret = -ENOENT;
		goto out;
	}
	stats_info->keepalive = 0;
	ret = 0;

out:
	spin_unlock_bh(&stats_info_lock);
	return ret;
}

/*
 * Function: himos_udp_stats
 * Description: The himos_udp_stats callback when the UDP packet is sent or
 * received
 * Arguments:
 *  @sk: socket controller block
 *    @inbytes&&outbytes: the size of this UDP packet
 * return: NONE
 */
void himos_udp_stats(const struct sock *sk, __u32 in_bytes, __u32 out_bytes)
{
	struct himos_udp_stats_info *info = NULL;
	__s32 uid;

	if (!sk)
		return;
	uid = (__s32)(sk->sk_uid.val);

	spin_lock_bh(&stats_info_lock);
	info = (struct himos_udp_stats_info *)himos_get_stats_info_by_uid(uid);
	if (info != NULL && info->comm.type == HIMOS_STATS_TYPE_UDP) {
		if (in_bytes > 0) {
			info->in_pkts = info->in_pkts + 1;
			info->in_bytes += (in_bytes + HIMOS_BYTE_OFFSET);
		}
		if (out_bytes > 0) {
			info->out_pkts = info->out_pkts + 1;
			info->out_bytes += (out_bytes - HIMOS_BYTE_OFFSET);
		}
	}
	spin_unlock_bh(&stats_info_lock);
}

int __init himos_udp_stats_init(void)
{
	init_timer(&timer_report);
	timer_report.function = himos_udp_stats_cb;

	return 0;
}
