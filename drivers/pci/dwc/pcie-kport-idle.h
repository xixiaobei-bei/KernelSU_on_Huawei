/*
 * pcie-kport-idle.h
 *
 * PCIe Idle Feature
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef _PCIE_KPORT_IDLE_H
#define _PCIE_KPORT_IDLE_H

#include <linux/types.h>
#include <linux/platform_device.h>

#include "pcie-designware.h"

struct pcie_idle_sleep {
	void __iomem	*idle_reg_base;
	u32		host_id;
	u32		idle_sleep_enable;
	u32		l1ss_cnt_base;
	u32		l1ss_cnt_base_addr;
	u32		idle_timeout_en_bit;
	u32		idle_timeout_mask_bit;
	u32		idle_timeout_int_addr;
	u32		idle_timeout_clr_bit;
	u32		idle_timeout_clr_addr;
	u32		idle_wake_en_bit;
	u32		idle_wake_mask_bit;
	u32		idle_wake_int_addr;
	u32		idle_wake_clr_bit;
	u32		idle_wake_clr_addr;
	atomic_t	ref_clk_onoff;
	atomic_t	ref_clk_vote;
	atomic_t	ref_clk_device_vote;
	spinlock_t	idle_sleep_lock;
	u32		irq[2];
	void 		(*init)(struct pcie_idle_sleep *idle_sleep);
	void 		(*deinit)(struct pcie_idle_sleep *idle_sleep);
};

#ifdef CONFIG_PCIE_KPORT_IDLE
void pcie_refclk_host_vote(struct pcie_port *pp, u32 vote);
void pcie_dump_ilde_hw_stat(u32 rc_id);
void pcie_dump_ilde_sw_stat(u32 rc_id);

static inline void pcie_32k_idle_init(struct pcie_idle_sleep *idle)
{
	if (idle && idle->init)
		idle->init(idle);
}

static inline void pcie_32k_idle_deinit(struct pcie_idle_sleep *idle)
{
	if (idle && idle->deinit)
		idle->deinit(idle);
}

static inline unsigned long pcie_idle_spin_lock(struct pcie_idle_sleep *idle,
		unsigned long flags)
{
	if (idle)
		spin_lock_irqsave(&(idle->idle_sleep_lock), flags);
	return flags;
}

static inline void pcie_idle_spin_unlock(struct pcie_idle_sleep *idle,
		unsigned long flags)
{
	if (idle)
		spin_unlock_irqrestore(&(idle->idle_sleep_lock), flags);
}

int32_t register_pcie_idle_hook(struct platform_device *dev);
#else
static inline void pcie_refclk_host_vote(struct pcie_port *pp, u32 vote)
{
}

static inline void pcie_dump_ilde_hw_stat(u32 rc_id)
{
}

static inline void pcie_dump_ilde_sw_stat(u32 rc_id)
{
}

static inline void pcie_32k_idle_init(struct pcie_idle_sleep *idle)
{
}

static inline void pcie_32k_idle_deinit(struct pcie_idle_sleep *idle)
{
}

static inline unsigned long pcie_idle_spin_lock(struct pcie_idle_sleep *idle,
		unsigned long flags)
{
	return 0;
}

static inline unsigned long pcie_idle_spin_unlock(struct pcie_idle_sleep *idle,
		unsigned long flags)
{
	return 0;
}

static inline int32_t register_pcie_idle_hook(struct platform_device *dev)
{
	return 0;
}
#endif /* CONFIG_PCIE_KPORT_IDLE */

#endif
