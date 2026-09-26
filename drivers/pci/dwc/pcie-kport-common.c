/*
 * pcie-kport-common.c
 *
 * PCIe common functions
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include <linux/hisi/pcie-kport-api.h>
#include "pcie-kport-common.h"
#include "pcie-kport-idle.h"

/*
 * config_enable_dbi - make it possible to access the rc configuration registers in the CDM,
 * or the ep configuration registers.
 * @flag: If flag equals 0, you can access the ep configuration registers in the CDM;
 *	  If not, you can access the rc configuration registers in the CDM.
 */
int config_enable_dbi(u32 rc_id, int flag)
{
	u32 ret1, ret2;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pcie = &g_pcie_port[rc_id];

	ret1 = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	ret2 = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);
	if (flag) {
		ret1 = ret1 | PCIE_ELBI_SLV_DBI_ENABLE;
		ret2 = ret2 | PCIE_ELBI_SLV_DBI_ENABLE;
	} else {
		ret1 = ret1 & (~PCIE_ELBI_SLV_DBI_ENABLE);
		ret2 = ret2 & (~PCIE_ELBI_SLV_DBI_ENABLE);
	}
	pcie_apb_ctrl_writel(pcie, ret1, SOC_PCIECTRL_CTRL0_ADDR);
	pcie_apb_ctrl_writel(pcie, ret2, SOC_PCIECTRL_CTRL1_ADDR);

	udelay(10);
	ret1 = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	ret2 = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);

	PCIE_PR_I("PCIeCTRL apb register CTRL0=[0x%x], CTRL1=[0x%x]", ret1, ret2);

	return 0;
}

/*
 * set_bme - enable bus master or not.
 * @flag: If flag equals 0, bus master is disabled. If not, bus master is enabled.
 */
int set_bme(u32 rc_id, int flag)
{
	u32 val;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pcie = &g_pcie_port[rc_id];

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	if (flag) {
		PCIE_PR_I("Enable Bus master!!!");
		val |= PCI_COMMAND_MASTER;
	} else {
		PCIE_PR_I("Disable Bus master!!!");
		val &= ~PCI_COMMAND_MASTER;
	}

	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN, val);

	udelay(5);

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	PCIE_PR_I("Register[0x4] value is [0x%x]", val);

	return 0;
}

/*
 * set_mse - enable mem space or not.
 * @flag: If flag equals 0, mem space is disabled. If not, mem space is enabled.
 */
int set_mse(u32 rc_id, int flag)
{
	u32 val;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pcie = &g_pcie_port[rc_id];

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	if (flag) {
		PCIE_PR_I("Enable MEM space!!!");
		val |= PCI_COMMAND_MEMORY;
	} else {
		PCIE_PR_I("Disable MEM space!!!");
		val &= ~PCI_COMMAND_MEMORY;
	}

	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN, val);

	udelay(5);

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	PCIE_PR_I("CMD_Reg value is [0x%x]", val);

	return 0;
}

int ltssm_enable(u32 rc_id, int yes)
{
	u32 val;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pcie = &g_pcie_port[rc_id];

	if (yes) {
		val = pcie_apb_ctrl_readl(pcie,  SOC_PCIECTRL_CTRL7_ADDR);
		val |= PCIE_LTSSM_ENABLE_BIT;
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);
	} else {
		val = pcie_apb_ctrl_readl(pcie,  SOC_PCIECTRL_CTRL7_ADDR);
		val &= ~PCIE_LTSSM_ENABLE_BIT;
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);
	}

	return 0;
}

void pcie_config_l0sl1(u32 rc_id, enum link_aspm_state aspm_state)
{
	struct pcie_kport *pcie = NULL;
	struct pci_dev *child = NULL;
	struct pci_dev *temp = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return;

	pcie = &g_pcie_port[rc_id];

	if (!pcie->rc_dev || !pcie->ep_dev) {
		PCIE_PR_E("Failed to get dev");
		return;
	}

	if (aspm_state == ASPM_CLOSE) {
		list_for_each_entry_safe(child, temp, &pcie->rc_dev->subordinate->devices, bus_list)
			pcie_capability_clear_and_set_word(child, PCI_EXP_LNKCTL,
							   PCI_EXP_LNKCTL_ASPMC, aspm_state);

		pcie_capability_clear_and_set_word(pcie->rc_dev, PCI_EXP_LNKCTL,
						   PCI_EXP_LNKCTL_ASPMC, aspm_state);
	} else {
		pcie_capability_clear_and_set_word(pcie->rc_dev, PCI_EXP_LNKCTL,
						   PCI_EXP_LNKCTL_ASPMC, aspm_state);

		list_for_each_entry_safe(child, temp, &pcie->rc_dev->subordinate->devices, bus_list)
			pcie_capability_clear_and_set_word(child, PCI_EXP_LNKCTL,
							   PCI_EXP_LNKCTL_ASPMC, aspm_state);
	}
}

void enable_req_clk(struct pcie_kport *pcie, u32 enable_flag)
{
	u32 val;

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);

	if (enable_flag)
		val &= ~PCIE_APB_CLK_REQ;
	else
		val |= PCIE_APB_CLK_REQ;

	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL1_ADDR);
}

void pcie_config_l1ss_ep(struct pcie_kport *pcie, struct pci_dev *dev,
			       u32 enable_flag, enum l1ss_ctrl_state enable)
{
	u32 reg_val;
	int ep_l1ss_pm, ep_ltr_pm;

	PCIE_PR_I("Get EP PCI_EXT_L1SS_CAP_ID");
	ep_l1ss_pm = pci_find_ext_capability(dev, PCI_EXT_L1SS_CAP_ID);
	if (!ep_l1ss_pm) {
		PCIE_PR_E("Failed to get EP PCI_EXT_L1SS_CAP_ID");
		return;
	}

	if (enable_flag) {
		/* EP: Power On Value & Scale bit[7~0] */
		if (pcie->dtsinfo.ep_l1ss_ctrl2) {
			pci_read_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL2, &reg_val);
			reg_val &= ~0xFF;
			reg_val |= pcie->dtsinfo.ep_l1ss_ctrl2;
			pci_write_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL2, reg_val);
		}

		/* EP: LTR Latency */
		if (pcie->dtsinfo.ep_ltr_latency) {
			ep_ltr_pm = pci_find_ext_capability(dev, PCI_EXT_LTR_CAP_ID);
			if (ep_ltr_pm)
				pci_write_config_dword(dev, ep_ltr_pm + LTR_MAX_SNOOP_LATENCY,
						       pcie->dtsinfo.ep_ltr_latency);
		}

		pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &reg_val);
		reg_val |= PCI_EXT_CAP_LTR_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, reg_val);

		/* Enable L1.1&L1.2 bit[3~0] */
		pci_read_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL1, &reg_val);
		reg_val = pcie->dtsinfo.l1ss_ctrl1;
		reg_val &= 0xFFFFFFF0;
		reg_val |= (u32)enable;
		pci_write_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL1, reg_val);
	} else {
		pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &reg_val);
		reg_val &= ~PCI_EXT_CAP_LTR_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, reg_val);

		/* disble L1ss */
		pci_read_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL1, &reg_val);
		reg_val &= ~L1SS_PM_ASPM_ALL;
		pci_write_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL1, reg_val);
	}
}

void pcie_config_l1ss(u32 rc_id, enum l1ss_ctrl_state enable)
{
	u32 reg_val;
	int rc_l1ss_pm;
	struct pcie_kport *pcie = NULL;
	struct pcie_dtsinfo *dtsinfo = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return;

	pcie = &g_pcie_port[rc_id];

	dtsinfo = &pcie->dtsinfo;

	if (!pcie->rc_dev || !pcie->ep_dev) {
		PCIE_PR_E("Failed to get RC_dev or EP_dev");
		return;
	}

	pcie_config_l1ss_ep(pcie, pcie->ep_dev, DISABLE, enable);

	PCIE_PR_I("Get RC PCI_EXT_L1SS_CAP_ID");
	rc_l1ss_pm = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_L1SS_CAP_ID);
	if (!rc_l1ss_pm) {
		PCIE_PR_E("Failed to get RC PCI_EXT_L1SS_CAP_ID");
		return;
	}

	pcie_capability_read_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, &reg_val);
	reg_val &= ~PCI_EXT_CAP_LTR_EN;
	pcie_capability_write_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, reg_val);

	/* disble L1ss */
	pci_read_config_dword(pcie->rc_dev, rc_l1ss_pm + PCI_EXT_L1SS_CTRL1, &reg_val);
	reg_val &= ~L1SS_PM_ASPM_ALL;
	pci_write_config_dword(pcie->rc_dev, rc_l1ss_pm + PCI_EXT_L1SS_CTRL1, reg_val);

	if (enable) {
		enable_req_clk(pcie, DISABLE);

		/* RC: Power On Value & Scale */
		if (dtsinfo->ep_l1ss_ctrl2) {
			pci_read_config_dword(pcie->rc_dev, rc_l1ss_pm + PCI_EXT_L1SS_CTRL2, &reg_val);
			reg_val &= ~0xFF;
			reg_val |= dtsinfo->ep_l1ss_ctrl2;
			pci_write_config_dword(pcie->rc_dev, rc_l1ss_pm + PCI_EXT_L1SS_CTRL2, reg_val);
		}

		pcie_capability_read_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, &reg_val);
		reg_val |= PCI_EXT_CAP_LTR_EN;
		pcie_capability_write_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, reg_val);

		/* Enable */
		pci_read_config_dword(pcie->rc_dev, rc_l1ss_pm + PCI_EXT_L1SS_CTRL1, &reg_val);
		reg_val = dtsinfo->l1ss_ctrl1;
		reg_val &= 0xFFFFFFF0;
		reg_val |= (u32)enable;
		pci_write_config_dword(pcie->rc_dev, rc_l1ss_pm + PCI_EXT_L1SS_CTRL1, reg_val);

		pcie_config_l1ss_ep(pcie, pcie->ep_dev, ENABLE, enable);
	} else {
		enable_req_clk(pcie, ENABLE);
	}
}

static void set_atu_addr(struct dw_pcie *pci, int type, u32 iatu_offset,
			 u64 src_addr, u64 dst_addr, u32 size)
{
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_LOWER_BASE, REG_DWORD_ALIGN, lower_32_bits(src_addr));
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_UPPER_BASE, REG_DWORD_ALIGN, upper_32_bits(src_addr));
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_LIMIT, REG_DWORD_ALIGN, lower_32_bits(src_addr + size - 1));
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_LOWER_TARGET, REG_DWORD_ALIGN, lower_32_bits(dst_addr));
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_UPPER_TARGET, REG_DWORD_ALIGN, upper_32_bits(dst_addr));
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_CR1, REG_DWORD_ALIGN, type);
	pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + ATU_CR2, REG_DWORD_ALIGN, ATU_ENABLE);
}

static void pcie_atu_cfg(struct pcie_kport *pcie, u32 index, u32 direct,
			       int type, u64 src_addr, u64 dst_addr, u32 size)
{
	u32 iatu_offset;

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_E("PCIe is power off");
		return;
	}

	iatu_offset = pcie->dtsinfo.iatu_base_offset;

	if (iatu_offset != ATU_VIEWPORT) {
		iatu_offset += index * PER_ATU_SIZE;
		if (direct & ATU_REGION_INBOUND)
			iatu_offset += INBOUNT_OFFSET;
	} else {
		pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
				     ATU_VIEWPORT, REG_DWORD_ALIGN,
				     (index | direct));
	}

	set_atu_addr(pcie->pci, type, iatu_offset, src_addr, dst_addr, size);
}

int pcie_generate_msg(u32 rc_id, int index, u32 iatu_offset, int msg_type, u32 msg_code)
{
	u32 val;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id)) {
		PCIE_PR_E("generate msg fail, rc_id:%u", rc_id);
		return -1;
	}

	pcie = &g_pcie_port[rc_id];

	/* 0x0 for pci_base_addr */
	pcie_kport_outbound_atu(rc_id, index, msg_type,
				pcie->pci->pp.cfg0_base + MSG_CPU_ADDR_TEM, 0x0,
				MSG_CPU_ADDR_SIZE);

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCIE_GET_ATU_OUTB_UNR_REG_OFFSET((u32)index) +
				  PCIE_ATU_UNR_REGION_CTRL2, REG_DWORD_ALIGN);
	val |= (msg_code | INHIBIT_PAYLOAD);
	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCIE_GET_ATU_OUTB_UNR_REG_OFFSET((u32)index) +
			     PCIE_ATU_UNR_REGION_CTRL2, REG_DWORD_ALIGN, val);

	pcie_refclk_host_vote(&pcie->pci->pp, 1);
	/* 0x0:value to generate message */
	writel(0x0, pcie->pci->pp.va_cfg0_base + MSG_CPU_ADDR_TEM);
	pcie_refclk_host_vote(&pcie->pci->pp, 0);

	return 0;
}

int pcie_power_ctrl(struct pcie_kport *pcie, enum rc_power_status on_flag)
{
	int ret;

	PCIE_PR_I("+%s+", __func__);

	/* power on */
	if (on_flag == RC_POWER_ON || on_flag == RC_POWER_RESUME) {
		ret = pcie->plat_ops->plat_on(pcie, on_flag);
		if (ret < 0)
			return ret;

		pcie->ep_link_status = DEVICE_LINK_UP;
	} else if (on_flag == RC_POWER_OFF || on_flag == RC_POWER_SUSPEND) {
		ret = pcie->plat_ops->plat_off(pcie, on_flag);
	} else {
		PCIE_PR_E("Invalid Param");
		ret = -1;
	}

	PCIE_PR_I("-%s-", __func__);
	return ret;
}

/* change axi_timeout to 48ms */
void pcie_config_axi_timeout(struct pcie_kport *pcie)
{
	u32 val;

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL25_ADDR);
	val &= ~PCIE_CTRL_BUS_TIMEOUT_MASK;
	val |= PCIE_CTRL_BUS_TIMEOUT_VALUE;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL25_ADDR);
}

bool is_pipe_clk_stable(struct pcie_kport *pcie)
{
	u32 reg_val;
	u32 time = 100; /* try 100 times */
	u32 pipe_clk_stable = 0x1 << 19; /* pipe_clk_stable_en */

	if (pcie->dtsinfo.board_type != BOARD_FPGA) {
		reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		while (reg_val & pipe_clk_stable) {
			mdelay(1);
			if (!time)
				return false;
			time--;
			reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		}
	}

	return true;
}

int pcie_noc_power(struct pcie_kport *pcie, int enable)
{
	u32 time = 100; /* try 100 times */
	u32 val;
	int rst;

	if (enable)
		val = NOC_PW_MASK | NOC_PW_SET_BIT;
	else
		val = NOC_PW_MASK;
	rst = enable ? 1 : 0;

	writel(val, pcie->pmctrl_base + NOC_POWER_IDLEREQ_1);

	val = readl(pcie->pmctrl_base + NOC_POWER_IDLE_1);
	while ((val & NOC_PW_SET_BIT) != rst) {
		udelay(10);
		if (!time) {
			PCIE_PR_E("Failed to reverse noc power-status");
			return -1;
		}
		time--;
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE_1);
	}

	return 0;
}

static int pcie_cfg_eco(struct pcie_kport *pcie)
{
	u32 reg_val;
	u32 sram_init_done = 0x1 << 0; /* 0x1 if init done */
	u32 time = 10; /* try 10 times */

	PCIE_PR_I("+%s+", __func__);

	reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_STATE39_ADDR);
	while (!(reg_val & sram_init_done)) {
		udelay(100);
		if (!time) {
			PCIE_PR_E("phy0_sram_init_done fail");
			return -1;
		}
		time--;
		reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_STATE39_ADDR);
	}

	if (pcie->plat_ops->sram_ext_load
		&& pcie->plat_ops->sram_ext_load((void *)pcie)) {
		PCIE_PR_E("Sram extra load Failed");
		return -1;
	}

	/* pull up phy0_sram_ext_ld_done signal, not choose ECO */
	reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL40_ADDR);
	reg_val |= (0x1 << 4);
	pcie_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL40_ADDR);

	PCIE_PR_I("-%s-", __func__);
	return 0;
}

static void pcie_reset_phy(struct pcie_kport *pcie)
{
	u32 reg_val;

	reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	reg_val &= ~(0x1 << 17); /* phy_reset_sel */
	reg_val |= (0x1 << 16); /* phy_reset */
	pcie_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
	udelay(10);
	reg_val &= ~(0x1 << 16); /* phy_dereset */
	pcie_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
}

/* Set Bit0:0'b: pull down phy0_sram_bypass signal, choose ECO */
/* Set Bit0:1'b: pull up phy0_sram_bypass signal, not choose ECO */
#ifndef CONFIG_PCIE_KPORT_PHY
static void pcie_select_eco(struct pcie_kport *pcie)
{
	u32 reg_val;

	reg_val = pcie_apb_phy_readl(pcie, ECO_BYPASS_ADDR);
	if (pcie->dtsinfo.eco)
		reg_val &= ~(0x1 << 0);
	else
		reg_val |= (0x1 << 0);
	pcie_apb_phy_writel(pcie, reg_val, ECO_BYPASS_ADDR);
}
#endif

int pcie_port_phy_init(struct pcie_kport *pcie)
{
	u32 reg_val;

#ifdef CONFIG_PCIE_KPORT_PHY
	pcie_phy_init(pcie);
#else
	pcie_select_eco(pcie);

	/* pull down phy_test_powerdown signal */
	reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	reg_val &= ~PHY_TEST_POWERDOWN;
	pcie_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL0_ADDR);
#endif

	if (pcie_ep_is_modem(pcie) || pcie_ep_is_fpga(pcie)) {
		reg_val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL38_ADDR);
		reg_val &= ~PCIEPHY_PIPE_REFCLK_MODE_BIT;
		pcie_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL38_ADDR);
	}

	if (pcie->dtsinfo.eco)
		pcie_reset_phy(pcie);

	/* deassert controller perst_n */
	reg_val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	if (pcie->dtsinfo.ep_flag)
		reg_val |= PERST_IN_EP;
	else
		reg_val |= PERST_IN_RC;
	pcie_apb_ctrl_writel(pcie, reg_val, SOC_PCIECTRL_CTRL12_ADDR);
	udelay(10);

	if (pcie->dtsinfo.eco && pcie_cfg_eco(pcie)) {
		PCIE_PR_E("eco init fail");
		return -1;
	}

	return 0;
}

void pcie_natural_cfg(struct pcie_kport *pcie)
{
	u32 val;

	/* set clkreq low(not enable l1ss) */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL22_ADDR);
	val &= ~PCIE_CLKREQ_OUT_MASK;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL22_ADDR);

	/* pull up sys_aux_pwr_det bit[10] */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	val |= (0x1 << 10);
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);

	if (pcie->dtsinfo.ep_flag) {
		/* cfg as ep */
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
		val &= 0xFFFFFFF;
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);
		/* input */
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val |= PCIE_PERST_IN_N_CTRL_11B;
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	} else {
		/* cfg as rc */
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
		val &= ~(PCIE_TYPE_MASK << PCIE_TYPE_SHIFT);
		val |= (PCIE_TYPE_RC << PCIE_TYPE_SHIFT);
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);

		/* output, pull down */
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val &= ~PCIE_PERST_IN_N_CTRL_11B;
		val |= PCIE_PERST_OE_EN;
		val &= ~PCIE_PERST_OUT_N;
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	}

	/* Handle phy_reset and lane0_reset to HW */
	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val |= PCIEPHY_RESET_BIT;
	val &= ~PCIEPHY_PIPE_LINE0_RESET_BIT;
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);
}

/* Load FW for PHY Fix */
int pcie_phy_fw_update(struct pcie_kport *pcie, u16 *fw_data, u32 fw_size)
{
	u32 reg_addr, reg_val;

	if (fw_size > PCIE_PHY_SRAM_SIZE)
		return -EINVAL;

	for (reg_addr = 0; reg_addr < fw_size; reg_addr++)
		pcie_ram_phy_writel(pcie, fw_data[reg_addr], reg_addr);

	/* Vboost */
	reg_val = pcie_natural_phy_readl(pcie, SUP_DIG_LVL_OVRD_IN);
	reg_val &= ~SUP_DIG_LVL_MASK;
	reg_val |= SUP_DIG_LVL_VAL;
	pcie_natural_phy_writel(pcie, reg_val, SUP_DIG_LVL_OVRD_IN);

	/* cdr_legacy_en */
	reg_val = pcie_apb_phy_readl(pcie, PCIE_PHY_CTRL150);
	reg_val |= CDR_LEGACY_ENABLE;
	pcie_apb_phy_writel(pcie, reg_val, PCIE_PHY_CTRL150);

	return 0;
}

void pcie_kport_outbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			     u64 pci_addr, u32 size)
{
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return;

	pcie = &g_pcie_port[rc_id];

	if (pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
	    ATU_VIEWPORT, REG_DWORD_ALIGN) == 0xffffffff)
		pcie->pci->iatu_unroll_enabled = 1;
	else
		pcie->pci->iatu_unroll_enabled = 0;

	dw_pcie_prog_outbound_atu(pcie->pci, index, type, cpu_addr, pci_addr, size);
}

/* adjust PCIeIO(diffbuf) driver */
void pcie_io_adjust(struct pcie_kport *pcie)
{
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;
	u32 reg_val;

	/* io_driver[0]: addr, io_driver[1]: mask, io_driver[2]: val */
	if (dtsinfo->io_driver[2]) {
		reg_val = pcie_apb_ctrl_readl(pcie, dtsinfo->io_driver[0]);
		reg_val &= ~dtsinfo->io_driver[1];
		reg_val |= dtsinfo->io_driver[2];
		pcie_apb_ctrl_writel(pcie, reg_val, dtsinfo->io_driver[0]);
	}
}

void set_phy_eye_param(struct pcie_kport *pcie)
{
	u32 i, reg_val;
	u32 *base = NULL;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (!dtsinfo->eye_param_nums)
		return;

	/* *(base + 0): addr *(base + 0):mask *(base + 0):val */
	for (i = 0; i < dtsinfo->eye_param_nums; i++) {
		base = dtsinfo->eye_param_data + (u64)i * OF_DRIVER_PARAM_NUMS;
		if (*(base + 2) != 0xFFFF) {
			reg_val = pcie_natural_phy_readl(pcie, *(base + 0));
			reg_val &= ~(*(base + 1));
			reg_val |= *(base + 2);
			pcie_natural_phy_writel(pcie, reg_val, *(base + 0));
		}
	}
}

void pcie_inbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			    u64 pci_addr, u32 size)
{
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return;

	pcie = &g_pcie_port[rc_id];
	pcie_atu_cfg(pcie, (u32)index, ATU_REGION_INBOUND,
			   type, pci_addr, cpu_addr, size);
}

u32 show_link_state(u32 rc_id)
{
	unsigned int val;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return LTSSM_PWROFF;

	pcie = &g_pcie_port[rc_id];

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);

	PCIE_PR_I("Register sc_pciectrl_stat4 value [0x%x]", val);

	val = val & LTSSM_STATUE_MASK;

	switch (val) {
	case LTSSM_CPLC:
		PCIE_PR_I("L-state: Compliance");
		break;
	case LTSSM_L0:
		PCIE_PR_I("L-state: L0");
		break;
	case LTSSM_L0S:
		PCIE_PR_I("L-state: L0S");
		break;
	case LTSSM_L1:
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE5_ADDR);
		PCIE_PR_I("Register 0x414 value [0x%x]", val);
		val = val & LTSSM_L1SS_MASK;
		if (val == LTSSM_L1_1) {
			PCIE_PR_I("L-state: L1.1");
		} else if (val == LTSSM_L1_2) {
			PCIE_PR_I("L-state: L1.2");
		} else {
			PCIE_PR_I("L-state: L1.0");
			val = LTSSM_L1;
		}
		break;
	case LTSSM_LPBK:
		PCIE_PR_I("L-state: LoopBack");
		break;
	default:
		PCIE_PR_E("LTSSM:%x", val);
		val = LTSSM_OTHERS;
		PCIE_PR_I("Other state");
	}
	return val;
}
EXPORT_SYMBOL_GPL(show_link_state);

void pcie_kport_key_info_dump(void)
{
	u32 i, j, k;
	struct pcie_kport *pcie = NULL;
	u32 val1, val2, val3, val4;

	for (i = 0; i < g_rc_num; i++) {
		pcie = &g_pcie_port[i];

		if (pcie->dtsinfo.ep_device_type != EP_DEVICE_MODEM)
			continue;

		mutex_lock(&pcie->power_lock);
		if (!atomic_read(&pcie->is_power_on)) {
			PCIE_PR_E("PCIe[%u] is Poweroff", pcie->rc_id);
			goto MUTEX_UNLOCK;
		}

		/* 4 register-value per line (base1:0x0 base2:0x400) */
		PCIE_PR_I("####DUMP APB CORE Register :");
		for (j = 0; j < 0x4; j++)
			pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j,
				pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x0),
				pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x4),
				pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x8),
				pcie_apb_ctrl_readl(pcie, 0x10 * j + 0xC));

		for (j = 0; j < 0x2; j++)
			pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x400 + 0x10 * j,
				pcie_apb_ctrl_readl(pcie, 0x400 + 0x10 * j + 0x0),
				pcie_apb_ctrl_readl(pcie, 0x400 + 0x10 * j + 0x4),
				pcie_apb_ctrl_readl(pcie, 0x400 + 0x10 * j + 0x8),
				pcie_apb_ctrl_readl(pcie, 0x400 + 0x10 * j + 0xC));

		/* 4 register-value per line (base1:0x0 base2:0x300000) */
		PCIE_PR_I("####DUMP RC-CFG Register :");
		for (j = 0; j < 0x20; j++) {
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0x0, &val1);
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0x4, &val2);
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0x8, &val3);
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0xC, &val4);
			pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j, val1, val2, val3, val4);
		}
		/* outbound_base:0x0 inbound_base:0x100) */
		for (j = 0; j < 0x4; j++) {
			for (k = 0; k < 0x2; k++) {
				pci_read_config_dword(pcie->rc_dev, (0x3 << 20) + 0x100 * j + 0x10 * k + 0x0, &val1);
				pci_read_config_dword(pcie->rc_dev, (0x3 << 20) + 0x100 * j + 0x10 * k + 0x4, &val2);
				pci_read_config_dword(pcie->rc_dev, (0x3 << 20) + 0x100 * j + 0x10 * k + 0x8, &val3);
				pci_read_config_dword(pcie->rc_dev, (0x3 << 20) + 0x100 * j + 0x10 * k + 0xC, &val4);
				pr_info("0x%-8x: %8x %8x %8x %8x\n",
					(0x3 << 20) + 0x100 * j + 0x10 * k, val1, val2, val3, val4);
			}
		}

		pr_info("\n");

MUTEX_UNLOCK:
		mutex_unlock(&pcie->power_lock);
	}
}
EXPORT_SYMBOL_GPL(pcie_kport_key_info_dump);
