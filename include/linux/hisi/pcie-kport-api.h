/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Description: PCIe host controller driver.
 * Create: 2021-2-1
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef _PCIE_KPORT_API_H
#define _PCIE_KPORT_API_H

#include <linux/types.h>
#include <linux/pci.h>
#ifdef CONFIG_PCIE_KPORT_V1
#include <linux/hisi/pcie-kirin-api.h>
#endif

enum pcie_kport_event {
	PCIE_KPORT_EVENT_MIN_INVALID = 0x0, /* min invalid value */
	PCIE_KPORT_EVENT_LINKUP = 0x1, /* linkup event */
	PCIE_KPORT_EVENT_LINKDOWN = 0x2, /* linkdown event */
	PCIE_KPORT_EVENT_WAKE = 0x4, /* wake event */
	PCIE_KPORT_EVENT_L1SS = 0x8, /* l1ss event */
	PCIE_KPORT_EVENT_CPL_TIMEOUT = 0x10, /* completion timeout event */
	PCIE_KPORT_EVENT_MAX_INVALID = 0x1F, /* max invalid value */
};

enum pcie_kport_trigger {
	PCIE_KPORT_TRIGGER_CALLBACK,
	PCIE_KPORT_TRIGGER_COMPLETION,
};

#ifndef CONFIG_PCIE_KPORT_V1
struct pcie_kport_notify {
	enum pcie_kport_event event;
	void *user;
	void *data;
	u32 options;
};
#else
#define pcie_kport_notify kirin_pcie_notify
#endif

struct pcie_kport_register_event {
	u32 events;
	void *user;
	enum pcie_kport_trigger mode;
	void (*callback)(struct pcie_kport_notify *notify);
	struct pcie_kport_notify notify;
	struct completion *completion;
	u32 options;
};

#ifdef CONFIG_PCIE_KPORT
int pcie_kport_register_event(struct pcie_kport_register_event *reg);
int pcie_kport_deregister_event(struct pcie_kport_register_event *reg);
int pcie_kport_pm_control(int power_ops, u32 rc_idx);
int pcie_kport_ep_off(u32 rc_idx);
int pcie_kport_lp_ctrl(u32 rc_idx, u32 enable);
int pcie_kport_enumerate(u32 rc_idx);
int pcie_kport_remove_ep(u32 rc_idx);
int pcie_kport_rescan_ep(u32 rc_idx);
int pcie_kport_ep_link_ltssm_notify(u32 rc_id, u32 link_status);
int pcie_kport_power_notifiy_register(u32 rc_id, int (*poweron)(void *data),
				int (*poweroff)(void *data), void *data);
void pcie_kport_key_info_dump(void);
void pcie_kport_outbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			     u64 pci_addr, u32 size);
#else
static inline int pcie_kport_register_event(struct pcie_kport_register_event *reg)
{
	return -EINVAL;
}

static inline int pcie_kport_deregister_event(struct pcie_kport_register_event *reg)
{
	return -EINVAL;
}

static inline int pcie_kport_pm_control(int power_ops, u32 rc_idx)
{
	return -EINVAL;
}

static inline int pcie_kport_ep_off(u32 rc_idx)
{
	return -EINVAL;
}

static inline int pcie_kport_lp_ctrl(u32 rc_idx, u32 enable)
{
	return -EINVAL;
}

static inline int pcie_kport_enumerate(u32 rc_idx)
{
	return -EINVAL;
}

static inline int pcie_kport_remove_ep(u32 rc_idx)
{
	return -EINVAL;
}

static inline int pcie_kport_rescan_ep(u32 rc_idx)
{
	return -EINVAL;
}

static inline int pcie_kport_ep_link_ltssm_notify(u32 rc_id, u32 link_status)
{
	return -EINVAL;
}

static inline int pcie_kport_power_notifiy_register(u32 rc_id,
				int (*poweron)(void *data),
				int (*poweroff)(void *data), void *data)
{
	return -EINVAL;
}

static inline void pcie_kport_key_info_dump(void) {}
static inline void pcie_kport_outbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			     u64 pci_addr, u32 size) {}
#endif /* CONFIG_PCIE_KPORT */

#ifdef CONFIG_PCIE_KPORT_TEST
u64 pcie_set_mem_outbound(u32 rc_id, struct pci_dev *dev, int bar, u64 target);
#else
static inline u64 pcie_set_mem_outbound(u32 rc_id, struct pci_dev *dev, int bar, u64 target)
{
	return -EINVAL;
}
#endif /* CONFIG_PCIE_KPORT_TEST */

#endif


