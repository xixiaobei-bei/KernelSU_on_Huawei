/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: PCIe host controller driver for Kirin SoCs.
 * Create: 2016-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef _KIRIN_PCIE_API_H
#define _KIRIN_PCIE_API_H

#include <linux/types.h>
#include <linux/pci.h>

enum kirin_pcie_event {
	KIRIN_PCIE_EVENT_MIN_INVALID = 0x0, /* min invalid value */
	KIRIN_PCIE_EVENT_LINKUP = 0x1, /* linkup event */
	KIRIN_PCIE_EVENT_LINKDOWN = 0x2, /* linkdown event */
	KIRIN_PCIE_EVENT_WAKE = 0x4, /* wake event */
	KIRIN_PCIE_EVENT_L1SS = 0x8, /* l1ss event */
	KIRIN_PCIE_EVENT_CPL_TIMEOUT = 0x10, /* completion timeout event */
	KIRIN_PCIE_EVENT_MAX_INVALID = 0x1F, /* max invalid value */
};

enum kirin_pcie_trigger {
	KIRIN_PCIE_TRIGGER_CALLBACK,
	KIRIN_PCIE_TRIGGER_COMPLETION,
};

struct kirin_pcie_notify {
	enum kirin_pcie_event event;
	void *user;
	void *data;
	u32 options;
};


struct kirin_pcie_register_event {
	u32 events;
	void *user;
	enum kirin_pcie_trigger mode;
	void (*callback)(struct kirin_pcie_notify *notify);
	struct kirin_pcie_notify notify;
	struct completion *completion;
	u32 options;
};

#ifdef CONFIG_PCIE_KPORT_V1
int kirin_pcie_register_event(struct kirin_pcie_register_event *reg);
int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg);
int kirin_pcie_pm_control(int power_ops, u32 rc_idx);
int kirin_pcie_ep_off(u32 rc_idx);
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable);
int kirin_pcie_enumerate(u32 rc_idx);
int kirin_pcie_remove_ep(u32 rc_idx);
int kirin_pcie_rescan_ep(u32 rc_idx);
int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status);
int kirin_pcie_power_notifiy_register(u32 rc_id, int (*poweron)(void *data),
				int (*poweroff)(void *data), void *data);
void kirin_pcie_apb_info_dump(void);
#else
static inline int kirin_pcie_register_event(struct kirin_pcie_register_event *reg)
{
	return -EINVAL;
}

static inline int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg)
{
	return -EINVAL;
}

static inline int kirin_pcie_pm_control(int power_ops, u32 rc_idx)
{
	return -EINVAL;
}

static inline int kirin_pcie_ep_off(u32 rc_idx)
{
	return -EINVAL;
}

static inline int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable)
{
	return -EINVAL;
}

static inline int kirin_pcie_enumerate(u32 rc_idx)
{
	return -EINVAL;
}

static inline int kirin_pcie_remove_ep(u32 rc_idx)
{
	return -EINVAL;
}

static inline int kirin_pcie_rescan_ep(u32 rc_idx)
{
	return -EINVAL;
}

static inline int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status)
{
	return -EINVAL;
}

static inline int kirin_pcie_power_notifiy_register(u32 rc_id,
				int (*poweron)(void *data),
				int (*poweroff)(void *data), void *data)
{
	return -EINVAL;
}

static inline void kirin_pcie_apb_info_dump(void) {}
#endif /* CONFIG_PCIE_KPORT */

#endif
