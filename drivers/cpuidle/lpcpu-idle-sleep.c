/*
 * lpcpu-idle-sleep.c
 *
 * information about lpcpu cpuidle vote
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "Idlesleep: " fmt

#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/hisi/lpcpu_idle_sleep.h>
#include <linux/spinlock.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_sctrl_interface.h>
#include <linux/of_platform.h>

#define IDLE_SLEEP_VOTE_ADDR_CNT	1UL
#define IDLE_SLEEP_VOTE_ADDR_LEN	8UL

static DEFINE_SPINLOCK(s_idle_sleep_lock);//lint !e64 !e570 !e708 !e785
/* sync with lpm3 */
static u32 *g_idle_sleep_vote_addr;

/*
 * idle_sleep_vote - vote for idle sleep in lpm3
 *
 * modid: modid
 * val:  0, can enter idle sleep;1 for not
 *
 * Called from the device that need access peripheral zone.
 */
s32 lpcpu_idle_sleep_vote(u32 modid, u32 val)
{
	unsigned long flags;

	if (modid >= ID_MUX)
		return -EINVAL;
	if (g_idle_sleep_vote_addr == NULL)
		return -EFAULT;

	spin_lock_irqsave(&s_idle_sleep_lock, flags);//lint !e550
	if (val != 0)
		*g_idle_sleep_vote_addr |= (u32)BIT(modid);
	else
		*g_idle_sleep_vote_addr &= ~(u32)BIT(modid);
	spin_unlock_irqrestore(&s_idle_sleep_lock, flags);//lint !e550

	return 0;
}
EXPORT_SYMBOL(lpcpu_idle_sleep_vote);

u32 lpcpu_idle_sleep_getval(void)
{
	return *g_idle_sleep_vote_addr;
}

static int __init lpcpu_idle_sleep_init(void)
{
	struct device_node *np = NULL;
	u32 vote_addr;
	int ret;

	g_idle_sleep_vote_addr = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,idle-sleep");//lint !e838
	if (np == NULL) {
		pr_err("maybe idle sleep vote not supported\n");
		return -ENODEV;
	}

	ret = of_property_read_u32_array(np, "vote-addr", &vote_addr,
					 IDLE_SLEEP_VOTE_ADDR_CNT);
	if (ret != 0) {
		pr_err("failed to find vote-addr node\n");
		of_node_put(np);
		return -EFAULT;
	}

	g_idle_sleep_vote_addr = (u32 *)ioremap((u64)vote_addr,
						IDLE_SLEEP_VOTE_ADDR_LEN);
	if (g_idle_sleep_vote_addr == NULL) {
		pr_err("failed to remap vote_addr\n");
		of_node_put(np);
		return -EIO;
	}

	of_node_put(np);
	return 0;
}

subsys_initcall(lpcpu_idle_sleep_init);
