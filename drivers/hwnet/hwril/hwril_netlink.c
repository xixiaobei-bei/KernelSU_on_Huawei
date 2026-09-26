/*
 * hwril_netlink.c
 *
 * generic netlink for hwril
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

#include "hwril_netlink.h"
#include <linux/if.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <net/genetlink.h>
#include "vsim.h"

static const struct nla_policy hwril_policy[HWRIL_ATTR_MAX + 1] = {
	[HWRIL_ATTR_VSIM_IFNAME] = {
		.type = NLA_NUL_STRING,
		.len = IFNAMSIZ - 1 },
	[HWRIL_ATTR_VSIM_TX_BYTES] = { .type = NLA_U64 },
	[HWRIL_ATTR_VSIM_RX_BYTES] = { .type = NLA_U64 },
	[HWRIL_ATTR_VSIM_TEE_TASK] = { .type = NLA_U32 },
};

static struct genl_ops hwril_genl_ops[] = {
	{
		.cmd = HWRIL_CMD_VSIM_SET_IFNAME,
		.policy = hwril_policy,
		.doit = vsim_set_ifname
	},
	{
		.cmd = HWRIL_CMD_VSIM_RESET_IFNAME,
		.policy = hwril_policy,
		.doit = vsim_reset_ifname
	},
	{
		.cmd = HWRIL_CMD_VSIM_RESET_COUNTER,
		.policy = hwril_policy,
		.doit = vsim_reset_counter
	},
	{
		.cmd = HWRIL_CMD_VSIM_GET_FLOW,
		.policy = hwril_policy,
		.doit = vsim_get_flow
	},
};

struct genl_multicast_group hwril_mcgrps[] = {
	{ .name = "events" },
};

static struct genl_family hwril_genl_family = {
	.hdrsize = 0,
	.name = HWRIL_GENL_FAMILY,
	.version = 1,
	.maxattr = HWRIL_ATTR_MAX,
	.ops = hwril_genl_ops,
	.n_ops = ARRAY_SIZE(hwril_genl_ops),
	.mcgrps = hwril_mcgrps,
	.n_mcgrps = ARRAY_SIZE(hwril_mcgrps),
};

int hwril_nl_vsim_flow_notify(u64 tx_bytes, u64 rx_bytes)
{
	struct sk_buff *skb = NULL;
	void *hdr = NULL;

	skb = genlmsg_new(GENLMSG_DEFAULT_SIZE, GFP_ATOMIC);
	if (!skb) {
		pr_info("%s: no mem", __func__);
		return -ENOMEM;
	}
	hdr = genlmsg_put(skb, 0, 0, &hwril_genl_family, 0,
		HWRIL_CMD_VSIM_NOTIFY);
	if (!hdr) {
		pr_info("%s: msg size error", __func__);
		nlmsg_free(skb);
		return -EMSGSIZE;
	}
	if (nla_put_u64_64bit(skb, HWRIL_ATTR_VSIM_TX_BYTES, tx_bytes,
		HWRIL_ATTR_PAD)) {
		pr_info("%s: msg size error", __func__);
		genlmsg_cancel(skb, hdr);
		nlmsg_free(skb);
		return -EMSGSIZE;
	}
	if (nla_put_u64_64bit(skb, HWRIL_ATTR_VSIM_RX_BYTES, rx_bytes,
		HWRIL_ATTR_PAD)) {
		pr_info("%s: msg size error", __func__);
		genlmsg_cancel(skb, hdr);
		nlmsg_free(skb);
		return -EMSGSIZE;
	}
	genlmsg_end(skb, hdr);
	return genlmsg_multicast(&hwril_genl_family, skb, 0, 0, GFP_ATOMIC);
}

int hwril_nl_vsim_tee_notify(u32 task_type)
{
	struct sk_buff *skb = NULL;
	void *hdr = NULL;

	skb = genlmsg_new(GENLMSG_DEFAULT_SIZE, GFP_ATOMIC);
	if (!skb) {
		pr_info("%s: no mem", __func__);
		return -ENOMEM;
	}
	hdr = genlmsg_put(skb, 0, 0, &hwril_genl_family, 0,
		HWRIL_CMD_VSIM_NOTIFY);
	if (!hdr) {
		pr_info("%s: msg size error", __func__);
		nlmsg_free(skb);
		return -EMSGSIZE;
	}
	if (nla_put_u32(skb, HWRIL_ATTR_VSIM_TEE_TASK, task_type)) {
		pr_info("%s: msg size error", __func__);
		genlmsg_cancel(skb, hdr);
		nlmsg_free(skb);
		return -EMSGSIZE;
	}
	genlmsg_end(skb, hdr);
	return genlmsg_multicast(&hwril_genl_family, skb, 0, 0, GFP_ATOMIC);
}

static int __init hwril_netlink_init(void)
{
	int err;

	pr_info("hwril: init");
	err = genl_register_family(&hwril_genl_family);
	if (err < 0) {
		pr_err("hwril: genl_register_family failed %d", err);
		return err;
	}

	/* init hwril module here */
	err = vsim_init();
	if (err < 0)
		pr_err("hwril: vsim_init failed %d", err);

	return 0;
}

static void __exit hwril_netlink_exit(void)
{
	int err;

	pr_info("hwril: exit");
	err = genl_unregister_family(&hwril_genl_family);
	if (err < 0)
		pr_err("hwril: genl_unregister_family failed %d", err);
}

module_init(hwril_netlink_init)
module_exit(hwril_netlink_exit)
