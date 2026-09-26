/*
 * pcie-kport-phy.h
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

#ifndef _PCIE_KPORT_PHY_H
#define _PCIE_KPORT_PHY_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>

#define TCM_SIZE_24K 0x6000
#define SIZE_32KB 0x8000
#define SIZE_4KB 0x1000
#define DISABLE 0
#define ENABLE 1

#define DRV_BIT(n) ((uint64_t)((uint64_t)0x1 << (n)))
#define DRV_32BIT_MASK(n1, n2) (DRV_BIT((uint64_t)(n2) + 1) - DRV_BIT((uint64_t)(n1)))
#define DRV_32BIT_READ_FIELD(v, n1, n2) (((uint64_t)(v) & DRV_32BIT_MASK((n1), (n2))) >> (n1))
#define DRV_32BIT_SET_FIELD(v1, n1, n2, v2) (((uint64_t)(v1) & (~DRV_32BIT_MASK((n1), (n2)))) | \
		(((uint64_t)(v2) << (uint64_t)(n1)) & DRV_32BIT_MASK((n1), (n2))))

#define D(format, arg...) pr_err("[k_pcie_phy][DBG]" format, ##arg)
#define I(format, arg...) pr_err("[k_pcie_phy][INF]" format, ##arg)
#define E(format, arg...) pr_err("[k_pcie_phy][ERR]" format, ##arg)

struct pcie_phy {
	uint32_t id;
	void __iomem *phy_base;
	void __iomem *core_base; /* RiscV core in PHY */

	/* mcu info for XW-PHY */
	struct clk *mcu_19p2_clk;
	struct clk *mcu_clk;
	struct clk *mcu_bus_clk;
	struct clk *mcu_32k_clk;

	struct mutex irq_mutex_lock;
};

struct pcie_phy_ops {
	void (*irq_init)(struct pcie_phy *phy);
	void (*irq_disable)(struct pcie_phy *phy);
	void (*irq_handler)(struct pcie_phy *phy);
	int32_t (*phy_core_clk_config)(struct pcie_phy *phy, bool enable);
	int32_t (*phy_core_start)(struct pcie_phy *phy);
	void (*phy_init)(struct pcie_phy *phy);
	bool (*is_phy_ready)(struct pcie_phy *phy);
	void (*phy_stat_dump)(struct pcie_phy *phy);
	void (*phy_w)(struct pcie_phy *phy, uint32_t reg, uint32_t msb, uint32_t lsb, uint32_t val);
	uint32_t (*phy_r)(struct pcie_phy *phy, uint32_t reg, uint32_t msb, uint32_t lsb);
};

#ifdef CONFIG_PCIE_KPORT_PHY
int32_t pcie_phy_ops_register(uint32_t rc_id, struct pcie_phy *phy,
		struct pcie_phy_ops *ops);

int32_t pcie_phy_register(struct platform_device *pdev);

#else
static inline int32_t pcie_phy_ops_register(uint32_t rc_id, struct pcie_phy *phy,
		struct pcie_phy_ops *ops)
{
	return -1;
}

static inline int32_t pcie_phy_register(struct platform_device *pdev)
{
	return 0;
}
#endif

#endif /* _PCIE_KPORT_PHY_H */
