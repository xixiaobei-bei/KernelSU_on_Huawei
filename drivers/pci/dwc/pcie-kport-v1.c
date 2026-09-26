/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Description: PCIe host controller driver.
 * Create: 2021
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/hisi/pcie-kport-api.h>
#include <linux/hisi/pcie-kirin-api.h>

/* EP register hook fun for link event notification */
int kirin_pcie_register_event(struct kirin_pcie_register_event *reg)
{
	return pcie_kport_register_event((struct pcie_kport_register_event *)reg);
}
EXPORT_SYMBOL_GPL(kirin_pcie_register_event);

int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg)
{
	return pcie_kport_deregister_event((struct pcie_kport_register_event *)reg);
}
EXPORT_SYMBOL_GPL(kirin_pcie_deregister_event);

/*
 * kirin_pcie_pm_control - EP Power ON/OFF callback Function.
 * @rc_idx: [in] which rc the EP link with
 * @power_ops: [in] 0---PowerOff normally
 *                  1---Poweron normally
 *                  2---PowerOFF without PME
 *                  3---Poweron without LINK
 */
int kirin_pcie_pm_control(int power_ops, u32 rc_idx)
{
	return pcie_kport_pm_control(power_ops, rc_idx);
}
EXPORT_SYMBOL_GPL(kirin_pcie_pm_control);

/*
 * API FOR EP to control L1&L1-substate
 * param: rc_idx---which rc the EP link with
 * enable: PCIE_KPORT_LP_ON---enable L1 and L1-substate,
 *         PCIE_KPORT_LP_Off---disable,
 *         others---illegal
 */
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable)
{
	return pcie_kport_lp_ctrl(rc_idx, enable);
}
EXPORT_SYMBOL_GPL(kirin_pcie_lp_ctrl);

/*
 * kirin_pcie_enumerate - Enumerate Function.
 * @rc_idx: [in] which rc the EP link with
 */
int kirin_pcie_enumerate(u32 rc_idx)
{
	return pcie_kport_enumerate(rc_idx);
}
EXPORT_SYMBOL(kirin_pcie_enumerate);

/*
 * kirin_pcie_remove_ep - Remove EP Function.
 * @rc_idx: [in]  which rc the EP link with
 */
int kirin_pcie_remove_ep(u32 rc_idx)
{
	return pcie_kport_remove_ep(rc_idx);
}
EXPORT_SYMBOL(kirin_pcie_remove_ep);

/*
 * kirin_pcie_rescan_ep - Rescan EP Function.
 * @rc_idx: [in] which rc the EP link with
 */
int kirin_pcie_rescan_ep(u32 rc_idx)
{
	return pcie_kport_rescan_ep(rc_idx);
}
EXPORT_SYMBOL(kirin_pcie_rescan_ep);

int kirin_pcie_power_notifiy_register(u32 rc_id, int (*poweron)(void *data),
				      int (*poweroff)(void *data), void *data)
{
	return pcie_kport_power_notifiy_register(rc_id, poweron, poweroff, data);
}
EXPORT_SYMBOL_GPL(kirin_pcie_power_notifiy_register);

int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status)
{
	return pcie_kport_ep_link_ltssm_notify(rc_id, link_status);
}
EXPORT_SYMBOL(pcie_ep_link_ltssm_notify);

void kirin_pcie_apb_info_dump(void)
{
	pcie_kport_key_info_dump();
}
EXPORT_SYMBOL_GPL(kirin_pcie_apb_info_dump);

void kirin_pcie_outbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			     u64 pci_addr, u32 size)
{
	pcie_kport_outbound_atu(rc_id, index, type, cpu_addr, pci_addr, size);
}
EXPORT_SYMBOL_GPL(kirin_pcie_outbound_atu);

