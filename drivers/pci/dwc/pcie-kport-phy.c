/*
 * pcie-kport-phy.c
 *
 * PCIe kport PHY adapter
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

#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "pcie-kport.h"
#include "pcie-kport-phy.h"

int32_t pcie_phy_ops_register(uint32_t rc_id, struct pcie_phy *phy,
			struct pcie_phy_ops *ops)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie) {
		E("get rc node\n");
		return -1;
	}

	pcie->phy = phy;
	pcie->phy_ops = ops;

	phy->phy_base = pcie->phy_base;

	return 0;
}

#ifdef CONFIG_PCIE_KPORT_TEST
void pcie_phy_write(uint32_t rc_id, uint32_t reg, uint32_t msb, uint32_t lsb, uint32_t val)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie || !atomic_read(&(pcie->is_power_on))) {
		E("Invalid PCIe or PWR off\n");
		return;
	}

	if (!pcie->phy || !pcie->phy_ops || !pcie->phy_ops->phy_w) {
		E("Invalid PHY or PHY Ops\n");
		return;
	}

	pcie->phy_ops->phy_w(pcie->phy, reg, msb, lsb, val);
}

uint32_t pcie_phy_read(uint32_t rc_id, uint32_t reg, uint32_t msb, uint32_t lsb)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie || !atomic_read(&(pcie->is_power_on))) {
		E("Invalid PCIe or PWR off\n");
		return 0xffffffff;
	}

	if (!pcie->phy || !pcie->phy_ops || !pcie->phy_ops->phy_w) {
		E("Invalid PHY or PHY Ops\n");
		return 0xffffffff;
	}

	return pcie->phy_ops->phy_r(pcie->phy, reg, msb, lsb);
}

void pcie_apb_phy_w(uint32_t rc_id, uint32_t reg, uint32_t msb, uint32_t lsb, uint32_t val)
{
	uint32_t data;
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie || !atomic_read(&(pcie->is_power_on))) {
		E("get rc node\n");
		return;
	}

	data = pcie_apb_phy_readl(pcie, reg);
	data = DRV_32BIT_SET_FIELD(data, lsb, msb, val);
	pcie_apb_phy_writel(pcie, data, reg);
}

uint32_t pcie_apb_phy_r(uint32_t rc_id, uint32_t reg, uint32_t msb, uint32_t lsb)
{
	uint32_t data;
	struct pcie_kport *pcie  = get_pcie_by_id(rc_id);

	if (!pcie || !atomic_read(&(pcie->is_power_on))) {
		E("get rc node\n");
		return 0xffffffff;
	}

	data = pcie_apb_phy_readl(pcie, reg);
	data = DRV_32BIT_READ_FIELD(data, lsb, msb);

	return data;
}
#endif

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PCIe kport phy driver");
