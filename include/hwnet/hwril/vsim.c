/*
 * vsim.c
 *
 * monitor vsim traffic
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

#include "vsim.h"
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include "hwril_netlink.h"

extern int TC_NS_RegisterServiceCallbackFunc(const char *uuid, void *func,
	const void *private_data);

struct teec_timer_property_stru {
	unsigned int time_type; /* Timer Type */
	unsigned int time_id;   /* Timer ID */
	unsigned int rev1;
	unsigned int rev2;
};

static unsigned int vsim_hook(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state);

#define NOTIFY_THRESH 1024 // 1KB
#define NOTIFY_INTERVAL HZ

static DEFINE_SPINLOCK(vsim_lock);
static char vsim_ifname[IFNAMSIZ];
static unsigned long rx_bytes;
static unsigned long tx_bytes;
static unsigned long total_bytes;
static unsigned long prev_bytes;
static unsigned long prev_jiffies;
static struct timer_list notify_timer;

static struct nf_hook_ops vsim_hooks[] = {
	{
		.hook = vsim_hook,
		.pf = PF_INET,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_RAW - 1,
	},
	{
		.hook = vsim_hook,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_MANGLE + 1,
	},
	{
		.hook = vsim_hook,
		.pf = PF_INET6,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP6_PRI_RAW - 1,
	},
	{
		.hook = vsim_hook,
		.pf = PF_INET6,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP6_PRI_MANGLE + 1,
	},
};

int vsim_set_ifname(struct sk_buff *skb, struct genl_info *info)
{
	char *ifname = NULL;

	if (!info || !info->attrs) {
		pr_info("vsim: null attr");
		return 0;
	}
	if (info->attrs[HWRIL_ATTR_VSIM_IFNAME])
		ifname = nla_data(info->attrs[HWRIL_ATTR_VSIM_IFNAME]);

	if (!ifname) {
		pr_info("vsim: null ifname");
		return 0;
	}
	spin_lock_bh(&vsim_lock);
	strlcpy(vsim_ifname, ifname, IFNAMSIZ);
	spin_unlock_bh(&vsim_lock);
	pr_info("vsim: set ifname %s", ifname);
	return 0;
}

int vsim_reset_ifname(struct sk_buff *skb, struct genl_info *info)
{
	spin_lock_bh(&vsim_lock);
	vsim_ifname[0] = '\0';
	spin_unlock_bh(&vsim_lock);
	return 0;
}

void vsim_flow_notify(unsigned long data)
{
	int err;

	spin_lock_bh(&vsim_lock);
	err = hwril_nl_vsim_flow_notify(tx_bytes, rx_bytes);
	pr_info("vsim: tx=%llu rx=%llu err=%d", tx_bytes, rx_bytes, err);
	prev_bytes = total_bytes;
	prev_jiffies = jiffies;
	spin_unlock_bh(&vsim_lock);
}

int vsim_get_flow(struct sk_buff *skb, struct genl_info *info)
{
	vsim_flow_notify(0);
	return 0;
}

bool vsim_match_ifname(const char *ifname)
{
	if (!ifname)
		return false;
	return !strncmp(vsim_ifname, ifname, IFNAMSIZ);
}

void vsim_maybe_notify(void)
{
	if (total_bytes - prev_bytes < NOTIFY_THRESH)
		return;
	if (timer_pending(&notify_timer))
		return;
	pr_info("vsim: add timer");
	if (time_after(jiffies, prev_jiffies + NOTIFY_INTERVAL))
		mod_timer(&notify_timer, jiffies);
	else
		mod_timer(&notify_timer, prev_jiffies + NOTIFY_INTERVAL);
}

int vsim_reset_counter(struct sk_buff *skb, struct genl_info *info)
{
	spin_lock_bh(&vsim_lock);
	rx_bytes = 0;
	tx_bytes = 0;
	total_bytes = 0;
	prev_bytes = 0;
	spin_unlock_bh(&vsim_lock);
	return 0;
}

static unsigned int vsim_hook(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	if (skb == NULL || skb->dev == NULL || state == NULL)
		return NF_ACCEPT;

	if (!spin_trylock_bh(&vsim_lock))
		return NF_ACCEPT;
	if (!vsim_match_ifname(skb->dev->name)) {
		spin_unlock_bh(&vsim_lock);
		return NF_ACCEPT;
	}
	if (state->in)
		rx_bytes += skb->len;
	else
		tx_bytes += skb->len;
	total_bytes += skb->len;
	vsim_maybe_notify();
	spin_unlock_bh(&vsim_lock);
	return NF_ACCEPT;
}

void vsim_tee_timeout_callback(void *timer_data_callback)
{
	struct teec_timer_property_stru *timer_data = NULL;

	pr_info("%s enter", __func__);
	if (timer_data_callback == NULL) {
		pr_err("%s: para error!", __func__);
		return;
	}

	timer_data = (struct teec_timer_property_stru *)timer_data_callback;
	spin_lock_bh(&vsim_lock);
	hwril_nl_vsim_tee_notify(timer_data->time_type);
	spin_unlock_bh(&vsim_lock);
	pr_info("%s, time_type = %d", __func__, timer_data->time_type);
}

void regist_vsim_tcc_timeout(void)
{
	// this uuid must same as skytone APK, used in tee OS to check validity
	const unsigned char uuid[] = {
		0x47, 0x91, 0xe8, 0xab, 0x61, 0xcd, 0x3f, 0xf4, 0x71, 0xc4,
			0x1a, 0x31, 0x7e, 0x40, 0x53, 0x12
	};
	pr_info("%s\n", __func__);

	if (TC_NS_RegisterServiceCallbackFunc((const char *)uuid,
		vsim_tee_timeout_callback, NULL) != 0) {
		pr_err("%s regist tee timeout cb Fail", __func__);
		return;
	}
	pr_info("%s regist tee timeout cb ok", __func__);
}

int vsim_init(void)
{
	int err;

	pr_info("vsim: init");
	init_timer(&notify_timer);
	notify_timer.function = vsim_flow_notify;
	prev_jiffies = jiffies;
	err = nf_register_net_hooks(&init_net, vsim_hooks,
		ARRAY_SIZE(vsim_hooks));
	regist_vsim_tcc_timeout();
	return err;
}
