/*
 * pcie-kirin-common.c
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

#include "pcie-kirin-common.h"
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
#include <linux/hisi/hisi_pcie_idle_sleep.h>
#endif

int kirin_pcie_valid(u32 rc_id)
{
	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_id);
		return KIRIN_PCIE_INVALID;
	}

	if (!atomic_read(&(g_kirin_pcie[rc_id].is_power_on))) {
		PCIE_PR_ERR("PCIe%u is power off", rc_id);
		return KIRIN_PCIE_INVALID;
	}

	return KIRIN_PCIE_VALID;
}

/*
 * config_enable_dbi - make it possible to access the rc configuration registers in the CDM,
 * or the ep configuration registers.
 * @flag: If flag equals 0, you can access the ep configuration registers in the CDM;
 *	  If not, you can access the rc configuration registers in the CDM.
 */
int config_enable_dbi(u32 rc_id, int flag)
{
	u32 ret1, ret2;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];

	ret1 = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	ret2 = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);
	if (flag) {
		ret1 = ret1 | PCIE_ELBI_SLV_DBI_ENABLE;
		ret2 = ret2 | PCIE_ELBI_SLV_DBI_ENABLE;
	} else {
		ret1 = ret1 & (~PCIE_ELBI_SLV_DBI_ENABLE);
		ret2 = ret2 & (~PCIE_ELBI_SLV_DBI_ENABLE);
	}
	kirin_elb_writel(pcie, ret1, SOC_PCIECTRL_CTRL0_ADDR);
	kirin_elb_writel(pcie, ret2, SOC_PCIECTRL_CTRL1_ADDR);

	udelay(10);
	ret1 = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	ret2 = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);

	PCIE_PR_INFO("PCIeCTRL apb register CTRL0=[0x%x], CTRL1=[0x%x]", ret1, ret2);

	return 0;
}

/*
 * set_bme - enable bus master or not.
 * @flag: If flag equals 0, bus master is disabled. If not, bus master is enabled.
 */
int set_bme(u32 rc_id, int flag)
{
	u32 val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	if (flag) {
		PCIE_PR_INFO("Enable Bus master!!!");
		val |= PCI_COMMAND_MASTER;
	} else {
		PCIE_PR_INFO("Disable Bus master!!!");
		val &= ~PCI_COMMAND_MASTER;
	}

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN, val);

	udelay(5);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	PCIE_PR_INFO("Register[0x4] value is [0x%x]", val);

	return 0;
}

/*
 * set_mse - enable mem space or not.
 * @flag: If flag equals 0, mem space is disabled. If not, mem space is enabled.
 */
int set_mse(u32 rc_id, int flag)
{
	u32 val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	if (flag) {
		PCIE_PR_INFO("Enable MEM space!!!");
		val |= PCI_COMMAND_MEMORY;
	} else {
		PCIE_PR_INFO("Disable MEM space!!!");
		val &= ~PCI_COMMAND_MEMORY;
	}

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN, val);

	udelay(5);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, REG_DWORD_ALIGN);

	PCIE_PR_INFO("CMD_Reg value is [0x%x]", val);

	return 0;
}

int ltssm_enable(u32 rc_id, int yes)
{
	u32 val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];

	if (yes) {
		val = kirin_elb_readl(pcie,  SOC_PCIECTRL_CTRL7_ADDR);
		val |= PCIE_LTSSM_ENABLE_BIT;
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);
	} else {
		val = kirin_elb_readl(pcie,  SOC_PCIECTRL_CTRL7_ADDR);
		val &= ~PCIE_LTSSM_ENABLE_BIT;
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);
	}

	return 0;
}

void kirin_pcie_config_l0sl1(u32 rc_id, enum link_aspm_state aspm_state)
{
	struct kirin_pcie *pcie = NULL;
	struct pci_dev *child = NULL;
	struct pci_dev *temp = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];

	if (!pcie->rc_dev || !pcie->ep_dev) {
		PCIE_PR_ERR("Failed to get dev");
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

void enable_req_clk(struct kirin_pcie *pcie, u32 enable_flag)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);

	if (enable_flag)
		val &= ~PCIE_APB_CLK_REQ;
	else
		val |= PCIE_APB_CLK_REQ;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL1_ADDR);
}

void kirin_pcie_config_l1ss_ep(struct kirin_pcie *pcie, struct pci_dev *dev,
			       u32 enable_flag, enum l1ss_ctrl_state enable)
{
	u32 reg_val;
	int ep_l1ss_pm, ep_ltr_pm;

	PCIE_PR_INFO("Get EP PCI_EXT_L1SS_CAP_ID");
	ep_l1ss_pm = pci_find_ext_capability(dev, PCI_EXT_L1SS_CAP_ID);
	if (!ep_l1ss_pm) {
		PCIE_PR_ERR("Failed to get EP PCI_EXT_L1SS_CAP_ID");
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

void kirin_pcie_config_l1ss(u32 rc_id, enum l1ss_ctrl_state enable)
{
	u32 reg_val;
	int rc_l1ss_pm;
	struct kirin_pcie *pcie = NULL;
	struct kirin_pcie_dtsinfo *dtsinfo = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];

	dtsinfo = &pcie->dtsinfo;

	if (!pcie->rc_dev || !pcie->ep_dev) {
		PCIE_PR_ERR("Failed to get RC_dev or EP_dev");
		return;
	}

	kirin_pcie_config_l1ss_ep(pcie, pcie->ep_dev, DISABLE, enable);

	PCIE_PR_INFO("Get RC PCI_EXT_L1SS_CAP_ID");
	rc_l1ss_pm = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_L1SS_CAP_ID);
	if (!rc_l1ss_pm) {
		PCIE_PR_ERR("Failed to get RC PCI_EXT_L1SS_CAP_ID");
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

		kirin_pcie_config_l1ss_ep(pcie, pcie->ep_dev, ENABLE, enable);
	} else {
		enable_req_clk(pcie, ENABLE);
	}
}

static void set_atu_addr(struct dw_pcie *pci, int type, u32 iatu_offset,
			 u64 src_addr, u64 dst_addr, u32 size)
{
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_LOWER_BASE, REG_DWORD_ALIGN, lower_32_bits(src_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_UPPER_BASE, REG_DWORD_ALIGN, upper_32_bits(src_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_LIMIT, REG_DWORD_ALIGN, lower_32_bits(src_addr + size - 1));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_LOWER_TARGET, REG_DWORD_ALIGN, lower_32_bits(dst_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_UPPER_TARGET, REG_DWORD_ALIGN, upper_32_bits(dst_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_CR1, REG_DWORD_ALIGN, type);
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_CR2, REG_DWORD_ALIGN, KIRIN_PCIE_ATU_ENABLE);
}

static void kirin_pcie_atu_cfg(struct kirin_pcie *pcie, u32 index, u32 direct,
			       int type, u64 src_addr, u64 dst_addr, u32 size)
{
	u32 iatu_offset;

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return;
	}

	iatu_offset = pcie->dtsinfo.iatu_base_offset;

	if (iatu_offset != KIRIN_PCIE_ATU_VIEWPORT) {
		iatu_offset += index * PER_ATU_SIZE;
		if (direct & KIRIN_PCIE_ATU_REGION_INBOUND)
			iatu_offset += INBOUNT_OFFSET;
	} else {
		kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
				     KIRIN_PCIE_ATU_VIEWPORT, REG_DWORD_ALIGN,
				     (index | direct));
	}

	set_atu_addr(pcie->pci, type, iatu_offset, src_addr, dst_addr, size);
}

void kirin_pcie_generate_msg(u32 rc_id, int index, u32 iatu_offset, int msg_type, u32 msg_code)
{
	u32 val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];

	/* 0x0 for pci_base_addr */
	kirin_pcie_outbound_atu(rc_id, index, msg_type,
				pcie->pci->pp.cfg0_base + MSG_CPU_ADDR_TEM, 0x0,
				MSG_CPU_ADDR_SIZE);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCIE_GET_ATU_OUTB_UNR_REG_OFFSET((u32)index) +
				  PCIE_ATU_UNR_REGION_CTRL2, REG_DWORD_ALIGN);
	val |= (msg_code | INHIBIT_PAYLOAD);
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCIE_GET_ATU_OUTB_UNR_REG_OFFSET((u32)index) +
			     PCIE_ATU_UNR_REGION_CTRL2, REG_DWORD_ALIGN, val);

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	kirin_pcie_refclk_host_vote(&pcie->pci->pp, 1);
#endif
	/* 0x0:value to generate message */
	writel(0x0, pcie->pci->pp.va_cfg0_base + MSG_CPU_ADDR_TEM);

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	kirin_pcie_refclk_host_vote(&pcie->pci->pp, 0);
#endif
}

int kirin_pcie_power_ctrl(struct kirin_pcie *pcie, enum rc_power_status on_flag)
{
	int ret;
	u32 val;

	PCIE_PR_INFO("+%s+", __func__);

	/* power on */
	if (on_flag == RC_POWER_ON || on_flag == RC_POWER_RESUME) {
		ret = pcie->plat_ops->plat_on(pcie, on_flag);
		if (ret < 0)
			return ret;

		pcie->ep_link_status = DEVICE_LINK_UP;

		if (pcie->dtsinfo.board_type == BOARD_FPGA) {
			val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
						  KIRIN_PCIE_LNKCTL2, REG_DWORD_ALIGN);
			val &= ~SPEED_MASK;
			val |= SPEED_GEN1;
			kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
					     KIRIN_PCIE_LNKCTL2, REG_DWORD_ALIGN, val);
		}
	} else if (on_flag == RC_POWER_OFF || on_flag == RC_POWER_SUSPEND) {
		ret = pcie->plat_ops->plat_off(pcie, on_flag);
	} else {
		PCIE_PR_ERR("Invalid Param");
		ret = -1;
	}

	PCIE_PR_INFO("-%s-", __func__);
	return ret;
}

/* change axi_timeout to 48ms */
void kirin_pcie_config_axi_timeout(struct kirin_pcie *pcie)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL25_ADDR);
	val &= ~PCIE_CTRL_BUS_TIMEOUT_MASK;
	val |= PCIE_CTRL_BUS_TIMEOUT_VALUE;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL25_ADDR);
}

bool is_pipe_clk_stable(struct kirin_pcie *pcie)
{
	u32 reg_val;
	u32 time = 100; /* try 100 times */
	u32 pipe_clk_stable = 0x1 << 19; /* pipe_clk_stable_en */

	if (pcie->dtsinfo.board_type != BOARD_FPGA) {
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		while (reg_val & pipe_clk_stable) {
			mdelay(1);
			if (!time)
				return false;
			time--;
			reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		}
	}

	return true;
}

int kirin_pcie_noc_power(struct kirin_pcie *pcie, int enable)
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
			PCIE_PR_ERR("Failed to reverse noc power-status");
			return -1;
		}
		time--;
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE_1);
	}

	return 0;
}

static int kirin_pcie_cfg_eco(struct kirin_pcie *pcie)
{
	u32 reg_val;
	u32 sram_init_done = 0x1 << 0; /* 0x1 if init done */
	u32 time = 10; /* try 10 times */

	PCIE_PR_INFO("+%s+", __func__);

	reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_STATE39_ADDR);
	while (!(reg_val & sram_init_done)) {
		udelay(100);
		if (!time) {
			PCIE_PR_ERR("phy0_sram_init_done fail");
			return -1;
		}
		time--;
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_STATE39_ADDR);
	}

	if (pcie->plat_ops->sram_ext_load
		&& pcie->plat_ops->sram_ext_load((void *)pcie)) {
		PCIE_PR_ERR("Sram extra load Failed");
		return -1;
	}

	/* pull up phy0_sram_ext_ld_done signal, not choose ECO */
	reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL40_ADDR);
	reg_val |= (0x1 << 4);
	kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL40_ADDR);

	PCIE_PR_INFO("-%s-", __func__);
	return 0;
}

static void kirin_pcie_reset_phy(struct kirin_pcie *pcie)
{
	u32 reg_val;

	reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	reg_val &= ~(0x1 << 17); /* phy_reset_sel */
	reg_val |= (0x1 << 16); /* phy_reset */
	kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
	udelay(10);
	reg_val &= ~(0x1 << 16); /* phy_dereset */
	kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
}

/* Set Bit0:0'b: pull down phy0_sram_bypass signal, choose ECO */
/* Set Bit0:1'b: pull up phy0_sram_bypass signal, not choose ECO */
static void kirin_pcie_select_eco(struct kirin_pcie *pcie)
{
	u32 reg_val;

	reg_val = kirin_apb_phy_readl(pcie, ECO_BYPASS_ADDR);
	if (pcie->dtsinfo.eco)
		reg_val &= ~(0x1 << 0);
	else
		reg_val |= (0x1 << 0);
	kirin_apb_phy_writel(pcie, reg_val, ECO_BYPASS_ADDR);
}

int kirin_pcie_phy_init(struct kirin_pcie *pcie)
{
	u32 reg_val;

	kirin_pcie_select_eco(pcie);

	/* pull down phy_test_powerdown signal */
	reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	reg_val &= ~PHY_TEST_POWERDOWN;
	kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL0_ADDR);

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL38_ADDR);
		reg_val &= ~PCIEPHY_PIPE_REFCLK_MODE_BIT;
		kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL38_ADDR);
	}

	if (pcie->dtsinfo.eco)
		kirin_pcie_reset_phy(pcie);

	/* deassert controller perst_n */
	reg_val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	if (pcie->dtsinfo.ep_flag)
		reg_val |= PERST_IN_EP;
	else
		reg_val |= PERST_IN_RC;
	kirin_elb_writel(pcie, reg_val, SOC_PCIECTRL_CTRL12_ADDR);
	udelay(10);

	if (pcie->dtsinfo.eco && kirin_pcie_cfg_eco(pcie)) {
		PCIE_PR_ERR("eco init fail");
		return -1;
	}

	return 0;
}

void kirin_pcie_natural_cfg(struct kirin_pcie *pcie)
{
	u32 val;

	/* pull up sys_aux_pwr_det bit[10] */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	val |= (0x1 << 10);
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);

	if (pcie->dtsinfo.ep_flag) {
		/* cfg as ep */
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
		val &= 0xFFFFFFF;
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);
		/* input */
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val |= PCIE_PERST_IN_N_CTRL_11B;
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	} else {
		/* cfg as rc */
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
		val &= ~(PCIE_TYPE_MASK << PCIE_TYPE_SHIFT);
		val |= (PCIE_TYPE_RC << PCIE_TYPE_SHIFT);
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);

		/* output, pull down */
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val &= ~PCIE_PERST_IN_N_CTRL_11B;
		val |= PCIE_PERST_OE_EN;
		val &= ~PCIE_PERST_OUT_N;
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	}

	/* Handle phy_reset and lane0_reset to HW */
	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val |= PCIEPHY_RESET_BIT;
	val &= ~PCIEPHY_PIPE_LINE0_RESET_BIT;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);
}

/* Load FW for PHY Fix */
int pcie_phy_fw_update(struct kirin_pcie *pcie, u16 *fw_data, u32 fw_size)
{
	u32 reg_addr, reg_val;

	if (fw_size > PCIE_PHY_SRAM_SIZE)
		return -EINVAL;

	for (reg_addr = 0; reg_addr < fw_size; reg_addr++)
		kirin_sram_phy_writel(pcie, fw_data[reg_addr], reg_addr);

	/* Vboost */
	reg_val = kirin_natural_phy_readl(pcie, SUP_DIG_LVL_OVRD_IN);
	reg_val &= ~SUP_DIG_LVL_MASK;
	reg_val |= SUP_DIG_LVL_VAL;
	kirin_natural_phy_writel(pcie, reg_val, SUP_DIG_LVL_OVRD_IN);

	/* cdr_legacy_en */
	reg_val = kirin_apb_phy_readl(pcie, PCIE_PHY_CTRL150);
	reg_val |= CDR_LEGACY_ENABLE;
	kirin_apb_phy_writel(pcie, reg_val, PCIE_PHY_CTRL150);

	return 0;
}

void kirin_pcie_outbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			     u64 pci_addr, u32 size)
{
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];

	if (kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
	    KIRIN_PCIE_ATU_VIEWPORT, REG_DWORD_ALIGN) == 0xffffffff)
		pcie->pci->iatu_unroll_enabled = 1;
	else
		pcie->pci->iatu_unroll_enabled = 0;

	dw_pcie_prog_outbound_atu(pcie->pci, index, type, cpu_addr, pci_addr, size);
}

/* adjust PCIeIO(diffbuf) driver */
void pcie_io_adjust(struct kirin_pcie *pcie)
{
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;
	u32 reg_val;

	/* io_driver[0]: addr, io_driver[1]: mask,io_driver[0]: val */
	if (dtsinfo->io_driver[2]) {
		reg_val = kirin_elb_readl(pcie, dtsinfo->io_driver[0]);
		reg_val &= ~dtsinfo->io_driver[1];
		reg_val |= dtsinfo->io_driver[2];
		kirin_elb_writel(pcie, reg_val, dtsinfo->io_driver[0]);
	}
}

void set_phy_eye_param(struct kirin_pcie *pcie)
{
	u32 i, reg_val;
	u32 *base = NULL;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (!dtsinfo->eye_param_nums)
		return;

	/* *(base + 0): addr *(base + 0):mask *(base + 0):val */
	for (i = 0; i < dtsinfo->eye_param_nums; i++) {
		base = dtsinfo->eye_param_data + (u64)i * OF_DRIVER_PARAM_NUMS;
		if (*(base + 2) != 0xFFFF) {
			reg_val = kirin_natural_phy_readl(pcie, *(base + 0));
			reg_val &= ~(*(base + 1));
			reg_val |= *(base + 2);
			kirin_natural_phy_writel(pcie, reg_val, *(base + 0));
		}
	}
}

void kirin_pcie_inbound_atu(u32 rc_id, int index, int type, u64 cpu_addr,
			    u64 pci_addr, u32 size)
{
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];
	kirin_pcie_atu_cfg(pcie, (u32)index, KIRIN_PCIE_ATU_REGION_INBOUND,
			   type, pci_addr, cpu_addr, size);
}

u32 show_link_state(u32 rc_id)
{
	unsigned int val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return LTSSM_PWROFF;

	pcie = &g_kirin_pcie[rc_id];

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);

	PCIE_PR_INFO("Register sc_pciectrl_stat4 value [0x%x]", val);

	val = val & LTSSM_STATUE_MASK;

	switch (val) {
	case LTSSM_CPLC:
		PCIE_PR_INFO("L-state: Compliance");
		break;
	case LTSSM_L0:
		PCIE_PR_INFO("L-state: L0");
		break;
	case LTSSM_L0S:
		PCIE_PR_INFO("L-state: L0S");
		break;
	case LTSSM_L1:
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE5_ADDR);
		PCIE_PR_INFO("Register 0x414 value [0x%x]", val);
		val = val & LTSSM_L1SS_MASK;
		if (val == LTSSM_L1_1) {
			PCIE_PR_INFO("L-state: L1.1");
		} else if (val == LTSSM_L1_2) {
			PCIE_PR_INFO("L-state: L1.2");
		} else {
			PCIE_PR_INFO("L-state: L1.0");
			val = LTSSM_L1;
		}
		break;
	case LTSSM_LPBK:
		PCIE_PR_INFO("L-state: LoopBack");
		break;
	default:
		PCIE_PR_ERR("LTSSM:%x", val);
		val = LTSSM_OTHERS;
		PCIE_PR_INFO("Other state");
	}
	return val;
}
EXPORT_SYMBOL_GPL(show_link_state);

void kirin_pcie_apb_info_dump(void)
{
	u32 i, j, k;
	struct kirin_pcie *pcie = NULL;
	u32 val1, val2, val3, val4;

	for (i = 0; i < g_rc_num; i++) {
		pcie = &g_kirin_pcie[i];

		if (pcie->dtsinfo.ep_device_type != EP_DEVICE_MODEM)
			return;

		mutex_lock(&pcie->power_lock);
		if (!atomic_read(&pcie->is_power_on)) {
			PCIE_PR_ERR("PCIe is Poweroff");
			goto MUTEX_UNLOCK;
		}

		/* 4 register-value per line (base1:0x0 base2:0x400) */
		PCIE_PR_INFO("####DUMP APB CORE Register : ");
		for (j = 0; j < 0x4; j++)
			pr_info("0x%-8x: %8x %8x %8x %8x\n",
				0x10 * j,
				kirin_elb_readl(pcie, 0x10 * j + 0x0),
				kirin_elb_readl(pcie, 0x10 * j + 0x4),
				kirin_elb_readl(pcie, 0x10 * j + 0x8),
				kirin_elb_readl(pcie, 0x10 * j + 0xC));

		for (j = 0; j < 0x2; j++)
			pr_info("0x%-8x: %8x %8x %8x %8x\n",
				0x400 + 0x10 * j,
				kirin_elb_readl(pcie, 0x400 + 0x10 * j + 0x0),
				kirin_elb_readl(pcie, 0x400 + 0x10 * j + 0x4),
				kirin_elb_readl(pcie, 0x400 + 0x10 * j + 0x8),
				kirin_elb_readl(pcie, 0x400 + 0x10 * j + 0xC));

		/* 4 register-value per line (base1:0x0 base2:0x300000) */
		PCIE_PR_INFO("####DUMP RC-CFG Register : ");
		for (j = 0; j < 0x20; j++) {
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0x0, &val1);
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0x4, &val2);
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0x8, &val3);
			pci_read_config_dword(pcie->rc_dev, 0x10 * j + 0xC, &val4);
			pr_info("0x%-8x: %8x %8x %8x %8x\n",
				0x10 * j, val1, val2, val3, val4);
		}
		/* outbound_base:0x0 inbound_base:0x100) */
		for (j = 0; j < 0x4; j++) {
			for (k = 0; k < 0x2; k++) {
				pci_read_config_dword(pcie->rc_dev,
				(0x3 << 20) + 0x100 * j + 0x10 * k + 0x0, &val1);
				pci_read_config_dword(pcie->rc_dev,
				(0x3 << 20) + 0x100 * j + 0x10 * k + 0x4, &val2);
				pci_read_config_dword(pcie->rc_dev,
				(0x3 << 20) + 0x100 * j + 0x10 * k + 0x8, &val3);
				pci_read_config_dword(pcie->rc_dev,
				(0x3 << 20) + 0x100 * j + 0x10 * k + 0xC, &val4);
				pr_info("0x%-8x: %8x %8x %8x %8x\n",
				(0x3 << 20) + 0x100 * j + 0x10 * k, val1, val2, val3, val4);
			}
		}

		pr_info("\n");

MUTEX_UNLOCK:
		mutex_unlock(&pcie->power_lock);
	}
}
EXPORT_SYMBOL_GPL(kirin_pcie_apb_info_dump);

#ifdef CONFIG_KIRIN_PCIE_TEST
int retrain_link(u32 rc_id)
{
	u32 val = 0;
	u32 cap_pos;
	unsigned long start_jiffies;
	struct pcie_port *pp = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pp = &(g_kirin_pcie[rc_id].pci->pp);

	cap_pos = kirin_pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL, REG_DWORD_ALIGN, &val);
	/* Retrain link */
	val |= PCI_EXP_LNKCTL_RL;
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL, REG_DWORD_ALIGN, val);

	/* Wait for link training end. Break out after waiting for timeout */
	start_jiffies = jiffies;
	for (;;) {
		kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKSTA, REG_DWORD_ALIGN, &val);
		val &= 0xffff;
		if (!(val & PCI_EXP_LNKSTA_LT))
			break;
		if (time_after(jiffies, start_jiffies + HZ))
			break;
		msleep(1);
	}
	if (!(val & PCI_EXP_LNKSTA_LT))
		return 0;
	return -1;
}

int set_link_speed(u32 rc_id, enum link_speed gen)
{
	u32 val = 0x1;
	u32 reg_val = 0x0;
	int ret = 0;
	u32 cap_pos;
	struct pcie_port *pp = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pp = &(g_kirin_pcie[rc_id].pci->pp);

	/* link speed:gen1(0x1) gen2(0x2) gen3(0x3) */
	switch (gen) {
	case GEN1:
		val = 0x1;
		break;
	case GEN2:
		val = 0x2;
		break;
	case GEN3:
		val = 0x3;
		break;
	default:
		ret = -1;
	}

	cap_pos = kirin_pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	/* PCIE_CAP_TARGET_LINK_SPEED bit[3~0] */
	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, REG_DWORD_ALIGN, &reg_val);
	reg_val &= ~(0x3 << 0);
	reg_val |= val;
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, REG_DWORD_ALIGN, reg_val);

	if (!ret)
		return retrain_link(rc_id);
	return ret;
}

int show_link_speed(u32 rc_id)
{
	unsigned int val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);
	val = val & 0xc0;
	switch (val) {
	case 0x0:
		PCIE_PR_INFO("Link speed: gen1");
		break;
	case 0x40:
		PCIE_PR_INFO("Link speed: gen2");
		break;
	case 0x80:
		PCIE_PR_INFO("Link speed: gen3");
		break;
	default:
		PCIE_PR_INFO("Link speed info unknow");
	}

	return val;
}

u32 kirin_pcie_find_capability(struct pcie_port *pp, int cap)
{
	u8 id;
	int ttl = 0x30;
	u32 pos = 0;
	u32 ent = 0;

	kirin_pcie_rd_own_conf(pp, PCI_CAPABILITY_LIST, REG_BYTE_ALIGN, &pos);

	/* bit[1~0]:capability-id, bit[15~8]:offset */
	while (ttl--) {
		if (pos < 0x40)
			break;
		pos &= ~3;
		kirin_pcie_rd_own_conf(pp, pos, REG_WORD_ALIGN, &ent);
		id = ent & 0xff;
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos = (ent >> 8);
	}
	return 0;
}

#endif

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
static void kirin_pcie_ref_clk_on(struct kirin_pcie *pcie)
{
	if (pcie && pcie->plat_ops && pcie->plat_ops->ref_clk_on)
		pcie->plat_ops->ref_clk_on(pcie);
}

static void kirin_pcie_ref_clk_off(struct kirin_pcie *pcie)
{
	if (pcie && pcie->plat_ops && pcie->plat_ops->ref_clk_off)
		pcie->plat_ops->ref_clk_off(pcie);
}

static void kirin_pcie_refclk_vote(struct kirin_pcie *pcie, u32 vote)
{
	if (unlikely(!pcie || !pcie->idle_sleep->idle_sleep_enable))
		return;

	spin_lock_irq(&(pcie->idle_sleep->idle_sleep_lock));
	if (vote) {
		if (!atomic_read(&(pcie->idle_sleep->ref_clk_vote)))
			kirin_pcie_ref_clk_on(pcie);
		atomic_inc(&(pcie->idle_sleep->ref_clk_vote));
	} else {
		if (!atomic_read(&(pcie->idle_sleep->ref_clk_vote))) {
			/* should NOT run to this flow */
			PCIE_PR_ERR("pcie ref clk vote already 0!");
			goto SPIN_UNLOCK;
		}
		atomic_dec(&(pcie->idle_sleep->ref_clk_vote));
	}
SPIN_UNLOCK:
	spin_unlock_irq(&(pcie->idle_sleep->idle_sleep_lock));
}

void kirin_pcie_refclk_host_vote(struct pcie_port *pp, u32 vote)
{
	struct dw_pcie *pci = NULL;
	struct kirin_pcie *pcie = NULL;

	if (unlikely(!pp))
		return;

	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);
	kirin_pcie_refclk_vote(pcie, vote);
}
EXPORT_SYMBOL_GPL(kirin_pcie_refclk_host_vote);

void kirin_pcie_refclk_device_vote(u32 ep_type, u32 rc_id, u32 vote)
{
	struct kirin_pcie *pcie = NULL;

	if (unlikely(rc_id >= g_rc_num || ep_type >= PCIE_DEVICE_MAX)) {
		PCIE_PR_ERR("Invalid ep_type %u or rc_id %u!\n",
			    ep_type, rc_id);
		return;
	}

	pcie = &g_kirin_pcie[rc_id];
	if (unlikely(!pcie->idle_sleep->idle_sleep_enable))
		return;

	PCIE_PR_INFO("%s: ep_type: %u [caller: %pF]",
		     __func__, vote, (void *)_RET_IP_);

	spin_lock_irq(&(pcie->idle_sleep->idle_sleep_lock));
	if (vote) {
		atomic_set(&(pcie->idle_sleep->ref_clk_device_vote), 1);
		kirin_pcie_ref_clk_on(pcie);
	} else {
		atomic_set(&(pcie->idle_sleep->ref_clk_device_vote), 0);
	}
	spin_unlock_irq(&(pcie->idle_sleep->idle_sleep_lock));
}
EXPORT_SYMBOL_GPL(kirin_pcie_refclk_device_vote);

static irqreturn_t kirin_pcie_idle_timeout_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;
	u32 val;

	if (!atomic_read(&(pcie->is_power_on)))
		return IRQ_HANDLED;

	spin_lock_irq(&(pcie->idle_sleep->idle_sleep_lock));

	if (!atomic_read(&(pcie->idle_sleep->ref_clk_vote)) &&
	    !atomic_read(&(pcie->idle_sleep->ref_clk_device_vote))) {
		PCIE_PR_INFO("Allow idle sleep! Ref clk will off");
		kirin_pcie_ref_clk_off(pcie);
	}

	/* clear pcie idle timeout int */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_timeout_clr_addr);
	val |= BIT(pcie->idle_sleep->idle_timeout_clr_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_timeout_clr_addr);

	/* clear pcie idle wake int */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_clr_addr);
	val |= BIT(pcie->idle_sleep->idle_wake_clr_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_clr_addr);

	/* clear pcie l1ss wake int mask */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_int_addr);
	val &= ~(BIT(pcie->idle_sleep->idle_wake_mask_bit));
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_int_addr);

	spin_unlock_irq(&(pcie->idle_sleep->idle_sleep_lock));

	return IRQ_HANDLED;
}

static irqreturn_t kirin_pcie_idle_wake_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;
	u32 val;

	if (!atomic_read(&(pcie->is_power_on)))
		return IRQ_HANDLED;

	spin_lock_irq(&(pcie->idle_sleep->idle_sleep_lock));

	/* clear pcie idle wake int */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_clr_addr);
	val |= BIT(pcie->idle_sleep->idle_wake_clr_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_clr_addr);

	/* add pcie l1ss wake int mask */
	val = kirin_elb_readl(pcie, pcie->idle_sleep->idle_wake_int_addr);
	val |= BIT(pcie->idle_sleep->idle_wake_mask_bit);
	kirin_elb_writel(pcie, val, pcie->idle_sleep->idle_wake_int_addr);

	/* ref clk on */
	if (atomic_read(&(pcie->idle_sleep->ref_clk_vote)) ||
	    atomic_read(&(pcie->idle_sleep->ref_clk_device_vote)))
		kirin_pcie_ref_clk_on(pcie);

	spin_unlock_irq(&(pcie->idle_sleep->idle_sleep_lock));

	return IRQ_HANDLED;
}

int kirin_add_idle_sleep_pcie_port(struct kirin_pcie *pcie,
				   struct platform_device *pdev)
{
	int ret;

	pcie->irq[IRQ_IDLE_TIMEOUT].num = platform_get_irq(pdev, IRQ_IDLE_TIMEOUT);
	if (!pcie->irq[IRQ_IDLE_TIMEOUT].num) {
		PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]",
			    pcie->irq[IRQ_IDLE_TIMEOUT].name,
			    pcie->irq[IRQ_IDLE_TIMEOUT].num);
		return -ENODEV;
	}
	PCIE_PR_INFO("L1ss idle timeout interrupt number is %d",
		     pcie->irq[IRQ_IDLE_TIMEOUT].num);

	ret = devm_request_irq(&pdev->dev,
				(unsigned int)pcie->irq[IRQ_IDLE_TIMEOUT].num,
				kirin_pcie_idle_timeout_irq_handler,
				IRQF_TRIGGER_RISING,
				pcie->irq[IRQ_IDLE_TIMEOUT].name,
				pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to request l1ss idle timeout irq");
		return ret;
	}

	PCIE_PR_INFO("pcie->irq[%d].name = [%s]",
		     IRQ_IDLE_TIMEOUT, pcie->irq[IRQ_IDLE_TIMEOUT].name);

	pcie->irq[IRQ_IDLE_WAKE].num = platform_get_irq(pdev, IRQ_IDLE_WAKE);
	if (!pcie->irq[IRQ_IDLE_WAKE].num) {
		PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]",
			    pcie->irq[IRQ_IDLE_WAKE].name,
			    pcie->irq[IRQ_IDLE_WAKE].num);
		return -ENODEV;
	}
	PCIE_PR_INFO("L1ss idle wake interrupt number is %d",
		     pcie->irq[IRQ_IDLE_WAKE].num);

	ret = devm_request_irq(&pdev->dev,
				(unsigned int)pcie->irq[IRQ_IDLE_WAKE].num,
				kirin_pcie_idle_wake_irq_handler,
				IRQF_TRIGGER_RISING,
				pcie->irq[IRQ_IDLE_WAKE].name,
				pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to request l1ss idle wake irq");
		return ret;
	}

	PCIE_PR_INFO("pcie->irq[%d].name = [%s]",
		     IRQ_IDLE_WAKE, pcie->irq[IRQ_IDLE_WAKE].name);

	return 0;
}

#ifdef CONFIG_KIRIN_PCIE_TEST
void kirin_pcie_refclk_vote_test(u32 rc_id, u32 vote)
{
	struct kirin_pcie *pcie = NULL;

	if (unlikely(rc_id >= g_rc_num)) {
		PCIE_PR_ERR("Invalid rc_id %u!\n", rc_id);
		return;
	}

	pcie = &g_kirin_pcie[rc_id];
	kirin_pcie_refclk_vote(pcie, vote);
}

void kirin_pcie_idle_sleep_enable_test(u32 rc_id, u32 endis)
{
	struct kirin_pcie *pcie = NULL;

	if (unlikely(rc_id >= g_rc_num)) {
		PCIE_PR_ERR("Invalid rc_id %u!\n", rc_id);
		return;
	}

	pcie = &g_kirin_pcie[rc_id];
	pcie->idle_sleep->idle_sleep_enable = (!!endis);
}
#endif

#ifdef CONFIG_PCIE_KIRIN_MNTN
void kirin_pcie_dump_idle_sleep_state(u32 rc_id)
{
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;
	pcie = &g_kirin_pcie[rc_id];

	/* idle sleep state dump */
	pr_info("[PCIe%u] Refclk onoff state  : %d\n",
		pcie->rc_id,
		atomic_read(&(pcie->idle_sleep->ref_clk_onoff)));
	pr_info("[PCIe%u] Refclk device vote  : %d\n",
		pcie->rc_id,
		atomic_read(&(pcie->idle_sleep->ref_clk_device_vote)));
	pr_info("[PCIe%u] Refclk other vote   : %d\n",
		pcie->rc_id,
		atomic_read(&(pcie->idle_sleep->ref_clk_vote)));
	pr_info("[PCIe%u] Refclk turn on cnt  : %llu\n",
		pcie->rc_id,
		pcie->idle_sleep->ref_clk_on_cnt);
	pr_info("[PCIe%u] Refclk turn off cnt : %llu\n",
		pcie->rc_id,
		pcie->idle_sleep->ref_clk_off_cnt);

	/* pciepll_state dump */
	pr_info("[PCIe%u] PCIEPLL STATE 0x%-8x: %8x\n",
		pcie->rc_id,
		pcie->idle_sleep->pciepll_state,
		readl(pcie->crg_base + pcie->idle_sleep->pciepll_state));

	/* PCIECTRL state dump */
	pr_info("0x%-8x: %8x %8x %8x %8x\n",
		0x43C, /* pciectrl_stat15 */
		kirin_elb_readl(pcie, 0x43C),  /* pciectrl_stat15 */
		kirin_elb_readl(pcie, 0x440),  /* pciectrl_stat16 */
		kirin_elb_readl(pcie, 0x444),  /* pciectrl_stat17 */
		kirin_elb_readl(pcie, 0x448)); /* pciectrl_stat18 */
}
#endif /* CONFIG_PCIE_KIRIN_MNTN */

#endif /* CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP */

