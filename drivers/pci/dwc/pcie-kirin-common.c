#include "pcie-kirin-common.h"

int kirin_pcie_valid(u32 rc_id)
{
	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return 0;
	}

	if (!atomic_read(&(g_kirin_pcie[rc_id].is_power_on))) {
		PCIE_PR_ERR("PCIe%d is power off", rc_id);
		return 0;
	}

	return -1;
}

/*
 * config_enable_dbi - make it possible to access the rc configuration registers in the CDM,
 * or the ep configuration registers.
 * @flag: If flag equals 0, you can access the ep configuration registers in the CDM;
 *  If not, you can access the rc configuration registers in the CDM.
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

	PCIE_PR_INFO("PCIeCTRL apb register 0x0=[0x%x], 0x4=[0x%x]", ret1, ret2);

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

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, 0x4);

	if (flag) {
		PCIE_PR_INFO("Enable Bus master!!!");
		val |= PCI_COMMAND_MASTER;
	} else {
		PCIE_PR_INFO("Disable Bus master!!!");
		val &= ~PCI_COMMAND_MASTER;
	}

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, 0x4, val);

	udelay(5);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, 0x4);

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

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, 0x4);

	if (flag) {
		PCIE_PR_INFO("Enable MEM space!!!");
		val |= PCI_COMMAND_MEMORY;
	} else {
		PCIE_PR_INFO("Disable MEM space!!!");
		val &= ~PCI_COMMAND_MEMORY;
	}

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, 0x4, val);

	udelay(5);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_COMMAND, 0x4);

	PCIE_PR_INFO("Register[0x4] value is [0x%x]", val);

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
		/* EP: Power On Value & Scale */
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
		reg_val |= (0x1 << 10);
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, reg_val);

		/* Enable */
		pci_read_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL1, &reg_val);
		reg_val = pcie->dtsinfo.l1ss_ctrl1;
		reg_val &= 0xFFFFFFF0;
		reg_val |= (u32)enable;
		pci_write_config_dword(dev, ep_l1ss_pm + PCI_EXT_L1SS_CTRL1, reg_val);
	} else {
		pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &reg_val);
		reg_val &= ~(0x1 << 10);
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

	if ((pcie->rc_dev == NULL) || (pcie->ep_dev == NULL)) {
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
	reg_val &= ~(0x1 << 10);
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
		reg_val |= (0x1 << 10);
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
			iatu_offset + KIRIN_PCIE_ATU_LOWER_BASE, 0x4, lower_32_bits(src_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_UPPER_BASE, 0x4, upper_32_bits(src_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_LIMIT, 0x4, lower_32_bits(src_addr + size - 1));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_LOWER_TARGET, 0x4, lower_32_bits(dst_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_UPPER_TARGET, 0x4, upper_32_bits(dst_addr));
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_CR1, 0x4, type);
	kirin_pcie_write_dbi(pci, pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_CR2, 0x4, KIRIN_PCIE_ATU_ENABLE);
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
		iatu_offset += index * 0x200;
		if (direct & KIRIN_PCIE_ATU_REGION_INBOUND)
			iatu_offset += INBOUNT_OFFSET;
	} else
		kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
				KIRIN_PCIE_ATU_VIEWPORT, 0x4, (index | direct));

	set_atu_addr(pcie->pci, type, iatu_offset, src_addr, dst_addr, size);
}

void kirin_pcie_generate_msg(u32 rc_id, int index, u32 iatu_offset, int msg_type, u32 msg_code)
{
	u32 val;
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];

	kirin_pcie_outbound_atu(rc_id, index, msg_type,
			pcie->dtsinfo.dbi_phy_base + MSG_CPU_ADDR, 0x0, MSG_CPU_ADDR_SIZE);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCIE_GET_ATU_OUTB_UNR_REG_OFFSET((u32)index) +
			PCIE_ATU_UNR_REGION_CTRL2, 0x4);
	val |= (msg_code | INHIBIT_PAYLOAD);
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCIE_GET_ATU_OUTB_UNR_REG_OFFSET((u32)index) +
			PCIE_ATU_UNR_REGION_CTRL2, 0x4, val);

	writel(0x0, pcie->pci->dbi_base + MSG_CPU_ADDR);
}

int kirin_pcie_power_ctrl(struct kirin_pcie *pcie, enum rc_power_status on_flag)
{
	int ret;
	u32 val;

	PCIE_PR_INFO("++");

	/* power on */
	if (on_flag == RC_POWER_ON || on_flag == RC_POWER_RESUME) {
		ret = pcie->plat_ops->plat_on(pcie, on_flag);
		if (ret < 0)
			return ret;

		pcie->ep_link_status = DEVICE_LINK_UP;

		if (pcie->dtsinfo.board_type == BOARD_FPGA) {
			val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
					KIRIN_PCIE_LNKCTL2, 0x4);
			val &= ~SPEED_MASK;
			val |= SPEED_GEN1;
			kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
					KIRIN_PCIE_LNKCTL2, 0x4, val);
		}
	} else if (on_flag == RC_POWER_OFF || on_flag == RC_POWER_SUSPEND) {
		ret = pcie->plat_ops->plat_off(pcie, on_flag);
	} else {
		PCIE_PR_ERR("Invalid Param");
		ret = -1;
	}

	PCIE_PR_INFO("--");
	return ret;
}

bool is_pipe_clk_stable(struct kirin_pcie *pcie)
{
	u32 reg_val;
	u32 time = 100;
	u32 pipe_clk_stable = 0x1 << 19;

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
	u32 time = 100;
	u32 val = NOC_PW_MASK;
	int rst;

	if (enable)
		val = NOC_PW_MASK | NOC_PW_SET_BIT;
	else
		val = NOC_PW_MASK;
	rst = enable ? 1 : 0;

	writel(val, pcie->pmctrl_base + NOC_POWER_IDLEREQ_1);

	time = 100;
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
	u32 sram_init_done = 0x1 << 0;
	u32 time = 10;
	struct kirin_pcie_dtsinfo *dtsinfo = NULL;

	PCIE_PR_INFO("+");
	dtsinfo = &(pcie->dtsinfo);

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

	PCIE_PR_INFO("-");
	return 0;
}

static void kirin_pcie_reset_phy(struct kirin_pcie *pcie)
{
	u32 reg_val;

	reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	reg_val &= ~(0x1 << 17);//phy_reset_sel
	reg_val |= (0x1 << 16);//phy_reset
	kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
	udelay(10);
	reg_val &= ~(0x1 << 16);//phy_dereset
	kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
}

/* Set Bit0=0'b: pull down phy0_sram_bypass signal, choose ECO */
/* Set Bit0=1'b: pull up phy0_sram_bypass signal, not choose ECO */
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

	/* pull up sys_aux_pwr_det */
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
		val |= (0x3 << 2);
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	} else {
		/* cfg as rc */
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
		val &= ~(PCIE_TYPE_MASK << PCIE_TYPE_SHIFT);
		val |= (PCIE_TYPE_RC << PCIE_TYPE_SHIFT);
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);

		/* output, pull down */
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val &= ~(0x3 << 2);
		val |= (0x1 << 1);
		val &= ~(0x1 << 0);
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

void kirin_pcie_outbound_atu(u32 rc_id, int index,
		int type, u64 cpu_addr, u64 pci_addr, u32 size)
{
	struct kirin_pcie *pcie = NULL;

	if (!kirin_pcie_valid(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];

	if (kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
				KIRIN_PCIE_ATU_VIEWPORT, 0x4) == 0xffffffff)
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

	for (i = 0; i < dtsinfo->eye_param_nums; i++) {
		base = dtsinfo->eye_param_data + (u64)i * TRIPLE_TUPLE;
		if (*(base + 2) != 0xFFFF) {
			reg_val = kirin_natural_phy_readl(pcie, *(base + 0));
			reg_val &= ~(*(base + 1));
			reg_val |= *(base + 2);
			kirin_natural_phy_writel(pcie, reg_val, *(base + 0));
		}
	}
}

void kirin_pcie_inbound_atu(u32 rc_id, int index,
		int type, u64 cpu_addr, u64 pci_addr, u32 size)
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

	PCIE_PR_INFO("Register 0x410 value [0x%x]", val);

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
	case LTSSM_L1: {
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE5_ADDR);
		PCIE_PR_INFO("Register 0x414 value [0x%x]", val);
		val = val & LTSSM_L1SS_MASK;
		if (val == LTSSM_L1_1)
			PCIE_PR_INFO("L-state: L1.1");
		else if (val == LTSSM_L1_2)
			PCIE_PR_INFO("L-state: L1.2");
		else {
			PCIE_PR_INFO("L-state: L1.0");
			val = LTSSM_L1;
		}
		break;
	}
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

void pcie_data_cpy(ulong dst, ulong src, uint32_t size)
{
	int ret;
	int dst_size = size;

	ret = memcpy_s((void *)(uintptr_t)dst, dst_size, (void *)(uintptr_t)src, size);
	if (ret) {
		PCIE_PR_ERR("Fail to do memcpy\n");
		return;
	}
}

void kirin_pcie_apb_info_dump(void)
{
	u32 i, j, k;
	struct kirin_pcie *pcie = NULL;
	u32 val1, val2, val3, val4;

	for (i = 0; i < g_rc_num; i++) {
		pcie = &g_kirin_pcie[i];

		if (!pcie)
			return;

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
			pr_info("0x%-8x: %8x %8x %8x %8x \n",
				0x10 * j,
				kirin_elb_readl(pcie, 0x10 * j + 0x0),
				kirin_elb_readl(pcie, 0x10 * j + 0x4),
				kirin_elb_readl(pcie, 0x10 * j + 0x8),
				kirin_elb_readl(pcie, 0x10 * j + 0xC));

		for (j = 0; j < 0x2; j++)
			pr_info("0x%-8x: %8x %8x %8x %8x \n",
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
			pr_info("0x%-8x: %8x %8x %8x %8x \n",
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
				pr_info("0x%-8x: %8x %8x %8x %8x \n",
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

	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL, 4, &val);
	/* Retrain link */
	val |= PCI_EXP_LNKCTL_RL;
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL, 4, val);

	/* Wait for link training end. Break out after waiting for timeout */
	start_jiffies = jiffies;
	for (;;) {
		kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKSTA, 4, &val);
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
	u32 cap_pos = 0x0;
	struct pcie_port *pp = NULL;

	if (!kirin_pcie_valid(rc_id))
		return -1;

	pp = &(g_kirin_pcie[rc_id].pci->pp);

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

	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, &reg_val);
	reg_val &= ~(0x3 << 0);
	reg_val |= val;
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, reg_val);

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

	kirin_pcie_rd_own_conf(pp, PCI_CAPABILITY_LIST, 1, &pos);

	while (ttl--) {
		if (pos < 0x40)
			break;
		pos &= ~3;
		kirin_pcie_rd_own_conf(pp, pos, 2, &ent);
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
