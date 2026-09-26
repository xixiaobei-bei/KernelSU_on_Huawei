/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: PCIe host controller driver.
 * Create: 2016-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/pci-aspm.h>
#include "pcie-kport.h"
#include "pcie-kport-common.h"
#include "pcie-kport-idle.h"
#include "pcie-kport-phy.h"

unsigned int g_rc_num;

struct pcie_kport g_pcie_port[] = {
	{
		.irq = {
				{
					.name = "pcie0-inta",
				},
				{
					.name = "pcie0-msi",
				},
				{
					.name = "pcie0-intc",
				},
				{
					.name = "pcie0-intd",
				},
				{
					.name = "pcie0-linkdown",
				},
				{
					.name = "pcie0-cpltimeout",
				},
			},
		.rc_dev = NULL,
		.ep_dev = NULL,
		.is_ready = ATOMIC_INIT(0),
		.is_enumerated = ATOMIC_INIT(0),
		.is_power_on = ATOMIC_INIT(0),
		.usr_suspend = ATOMIC_INIT(0),
		.is_removed = ATOMIC_INIT(0),
	},
	{
		.irq = {
				{
					.name = "pcie1-inta",
				},
				{
					.name = "pcie1-msi",
				},
				{
					.name = "pcie1-intc",
				},
				{
					.name = "pcie1-intd",
				},
				{
					.name = "pcie1-linkdown",
				},
				{
					.name = "pcie1-cpltimeout",
				},
			},
		.rc_dev = NULL,
		.ep_dev = NULL,
		.is_ready = ATOMIC_INIT(0),
		.is_enumerated = ATOMIC_INIT(0),
		.is_power_on = ATOMIC_INIT(0),
		.usr_suspend = ATOMIC_INIT(0),
		.is_removed = ATOMIC_INIT(0),
	},
};

int pcie_check_rcid(uint32_t rc_id)
{
	return rc_id < g_rc_num;
}

struct pcie_kport *get_pcie_by_id(uint32_t rc_id)
{
	if (rc_id < g_rc_num)
		return &(g_pcie_port[rc_id]);
	return NULL;
}

void pcie_apb_ctrl_writel(struct pcie_kport *pcie, u32 val, u32 reg)
{
	writel(val, pcie->apb_base + reg);
}

u32 pcie_apb_ctrl_readl(struct pcie_kport *pcie, u32 reg)
{
	return readl(pcie->apb_base + reg);
}

void pcie_apb_phy_writel(struct pcie_kport *pcie, u32 val, u32 reg)
{
	writel(val, pcie->phy_base + pcie->apb_phy_offset + reg);
}

u32 pcie_apb_phy_readl(struct pcie_kport *pcie, u32 reg)
{
	return readl(pcie->phy_base + pcie->apb_phy_offset + reg);
}

void pcie_natural_phy_writel(struct pcie_kport *pcie, u32 val, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		writel(val, pcie->phy_base + pcie->natural_phy_offset + reg * REG_DWORD_ALIGN);
}

u32 pcie_natural_phy_readl(struct pcie_kport *pcie, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		return readl(pcie->phy_base + pcie->natural_phy_offset + reg * REG_DWORD_ALIGN);
	return 0;
}

void pcie_ram_phy_writel(struct pcie_kport *pcie, u32 val, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		writel(val, pcie->phy_base + pcie->sram_phy_offset + reg * REG_DWORD_ALIGN);
}

u32 pcie_ram_phy_readl(struct pcie_kport *pcie, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		return readl(pcie->phy_base + pcie->sram_phy_offset + reg * REG_DWORD_ALIGN);
	return 0;
}

static int32_t pcie_get_clk(struct pcie_kport *pcie, struct platform_device *pdev)
{
	pcie->pcie_aux_clk = devm_clk_get(&pdev->dev, "pcie_aux");
	if (IS_ERR(pcie->pcie_aux_clk)) {
		PCIE_PR_E("Failed to get pcie_aux clock");
		return PTR_ERR(pcie->pcie_aux_clk);
	}

	pcie->apb_phy_clk = devm_clk_get(&pdev->dev, "pcie_apb_phy");
	if (IS_ERR(pcie->apb_phy_clk)) {
		PCIE_PR_E("Failed to get pcie_apb_phy clock");
		return PTR_ERR(pcie->apb_phy_clk);
	}

	pcie->apb_sys_clk = devm_clk_get(&pdev->dev, "pcie_apb_sys");
	if (IS_ERR(pcie->apb_sys_clk)) {
		PCIE_PR_E("Failed to get pcie_apb_sys clock");
		return PTR_ERR(pcie->apb_sys_clk);
	}

	pcie->pcie_aclk = devm_clk_get(&pdev->dev, "pcie_aclk");
	if (IS_ERR(pcie->pcie_aclk)) {
		PCIE_PR_E("Failed to get pcie_aclk clock");
		return PTR_ERR(pcie->pcie_aclk);
	}

	PCIE_PR_I("Successed to get all clock");

	return 0;
}

static int32_t get_phy_layout(struct pcie_kport *pcie, struct device_node *np)
{
	/* get three offset from dts info */
	u32 val[3] = {0};
	const size_t size = 3;

	if (of_property_read_u32_array(np, "phy_layout_info", val, size)) {
		PCIE_PR_E("Failed to get phy layout info");
		return -1;
	}

	pcie->natural_phy_offset = val[0];
	pcie->sram_phy_offset = val[1];
	pcie->apb_phy_offset = val[2];

	return 0;
}

static int32_t pcie_get_baseaddr(struct pcie_kport *pcie,
				       struct platform_device *pdev)
{
	struct resource *apb = NULL;
	struct resource *phy = NULL;
	struct resource *dbi = NULL;
	struct device_node *np = NULL;

	apb = platform_get_resource_byname(pdev, IORESOURCE_MEM, "apb");
	pcie->apb_base = devm_ioremap_resource(&pdev->dev, apb);
	if (IS_ERR(pcie->apb_base)) {
		PCIE_PR_E("Failed to get PCIeCTRL apb base");
		return PTR_ERR(pcie->apb_base);
	}

	phy = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy");
	pcie->phy_base = devm_ioremap_resource(&pdev->dev, phy);
	if (IS_ERR(pcie->phy_base)) {
		PCIE_PR_E("Failed to get PCIePHY base");
		return PTR_ERR(pcie->phy_base);
	}

	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	pcie->pci->dbi_base = devm_ioremap_resource(&pdev->dev, dbi);
	if (IS_ERR(pcie->pci->dbi_base)) {
		PCIE_PR_E("Failed to get PCIe dbi base");
		return PTR_ERR(pcie->pci->dbi_base);
	}

	np = pdev->dev.of_node;
	if (of_property_read_u32(np, "iatu_base_offset", &pcie->dtsinfo.iatu_base_offset)) {
		PCIE_PR_E("Failed to get iatu_base_offset info");
		return -1;
	}

	if (get_phy_layout(pcie, np))
		return -1;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np) {
		PCIE_PR_E("Failed to get sysctrl Node");
		return -1;
	}
	pcie->sctrl_base = of_iomap(np, 0);
	if (!pcie->sctrl_base) {
		PCIE_PR_E("Failed to iomap sctrl_base");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		PCIE_PR_E("Failed to get pmctrl Node");
		goto SCTRL_BASE_UNMAP;
	}
	pcie->pmctrl_base = of_iomap(np, 0);
	if (!pcie->pmctrl_base) {
		PCIE_PR_E("Failed to iomap sctrl_base");
		goto SCTRL_BASE_UNMAP;
	}

	PCIE_PR_I("Successed to get all resource");
	return 0;

SCTRL_BASE_UNMAP:
	iounmap(pcie->sctrl_base);
	return -1;
}

static int32_t pcie_get_pinctrl(struct pcie_kport *pcie,
				      struct platform_device *pdev)
{
	pcie->gpio_id_reset = of_get_named_gpio(pdev->dev.of_node, "reset-gpio", 0);
	if (pcie->gpio_id_reset < 0) {
		PCIE_PR_E("Failed to get perst gpio number");
		return -1;
	}

	return 0;
}

static void pcie_get_boardtype(struct pcie_kport *pcie,
				     struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32(np, "board_type", &dtsinfo->board_type)) {
		dtsinfo->board_type = BOARD_ASIC;
		PCIE_PR_I("Failed to get board_type, set default[0x%x]", dtsinfo->board_type);
	}

	if (of_property_read_u32(np, "chip_type", &dtsinfo->chip_type)) {
		dtsinfo->chip_type = CHIP_TYPE_CS;
		PCIE_PR_I("Failed to get chip_type, set default[0x%x]", dtsinfo->chip_type);
	}

	if (of_property_read_u32(np, "ep_device_type", &dtsinfo->ep_device_type)) {
		dtsinfo->ep_device_type = EP_DEVICE_NODEV;
		PCIE_PR_I("Failed to get ep_device_type, set default[0x%x]", dtsinfo->ep_device_type);
	}

	if (of_property_read_bool(np, "ep_flag"))
		dtsinfo->ep_flag = 1;
	else
		dtsinfo->ep_flag = 0;
}

static int32_t pcie_get_isoinfo(struct pcie_kport *pcie,
				      struct platform_device *pdev)
{
	const size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "iso_info", dtsinfo->iso_info, array_num)) {
		PCIE_PR_E("Failed to get isoen info");
		return -1;
	}

	return 0;
}

static void pcie_get_sups3info(struct pcie_kport *pcie,
					struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;

	pcie->dtsinfo.sup_s3 = of_property_read_bool(np, "sup_s3");
}

void pcie_iso_ctrl(struct pcie_kport *pcie, int en_flag)
{
	if (en_flag)
		writel(pcie->dtsinfo.iso_info[1],
		       pcie->sctrl_base + pcie->dtsinfo.iso_info[0]);
	else
		writel(pcie->dtsinfo.iso_info[1],
		       pcie->sctrl_base + pcie->dtsinfo.iso_info[0] + ADDR_OFFSET_4BYTE);
}

static int32_t pcie_get_assertinfo(struct pcie_kport *pcie,
					 struct platform_device *pdev)
{
	const size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "assert_info", dtsinfo->assert_info, array_num)) {
		PCIE_PR_E("Failed to get assert info");
		return -1;
	}

	return 0;
}

void pcie_reset_ctrl(struct pcie_kport *pcie, enum RST_TYPE rst)
{
	if (rst == RST_DISABLE)
		writel(pcie->dtsinfo.assert_info[1],
		       pcie->crg_base + pcie->dtsinfo.assert_info[0] + ADDR_OFFSET_4BYTE);
	else
		writel(pcie->dtsinfo.assert_info[1],
		       pcie->crg_base + pcie->dtsinfo.assert_info[0]);
}

static void pcie_get_linkstate(struct pcie_kport *pcie,
				     struct platform_device *pdev)
{
	int ret;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node,
				   "ep_ltr_latency", &dtsinfo->ep_ltr_latency);
	if (ret) {
		dtsinfo->ep_ltr_latency = 0x0;
		PCIE_PR_I("Not set ep_ltr_latency, set default[0x%x]",
			  dtsinfo->ep_ltr_latency);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				   "ep_l1ss_ctrl2", &dtsinfo->ep_l1ss_ctrl2);
	if (ret) {
		dtsinfo->ep_l1ss_ctrl2 = 0x0;
		PCIE_PR_I("Not set ep_l1ss_ctrl2, set default[0x%x]",
			  dtsinfo->ep_l1ss_ctrl2);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				   "l1ss_ctrl1", &dtsinfo->l1ss_ctrl1);
	if (ret) {
		dtsinfo->l1ss_ctrl1 = 0x0;
		PCIE_PR_I("Not set L1ss_ctrl1, set default[0x%x]",
			  dtsinfo->l1ss_ctrl1);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				   "aspm_state", &dtsinfo->aspm_state);
	if (ret) {
		dtsinfo->aspm_state = ASPM_L1;
		PCIE_PR_I("Not set aspm_state, set default[0x%x]",
			  dtsinfo->aspm_state);
	}
}

static void pcie_get_eco(struct pcie_kport *pcie,
			       struct platform_device *pdev)
{
	int ret;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node, "eco", &dtsinfo->eco);
	if (ret)
		dtsinfo->eco = 0x0;

	PCIE_PR_I("SRAM ECO [0x%x]", dtsinfo->eco);
}

static void pcie_get_time_params(struct pcie_kport *pcie,
				 struct platform_device *pdev)
{
	const size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;

	if (of_property_read_u32_array(np, "t_ref2perst",
		pcie->dtsinfo.t_ref2perst, array_num)) {
		PCIE_PR_I("Fail: ref2perst time");
		pcie->dtsinfo.t_ref2perst[0] = PCIE_REF2PERST1_DELAY_MIN;
		pcie->dtsinfo.t_ref2perst[1] = PCIE_REF2PERST2_DELAY_MAX;
	}

	if (of_property_read_u32_array(np, "t_perst2access",
		pcie->dtsinfo.t_perst2access, array_num)) {
		PCIE_PR_I("Fail: perst2access time");
		pcie->dtsinfo.t_perst2access[0] = PCIE_PERST2ACCESS1_DELAY_MIN;
		pcie->dtsinfo.t_perst2access[1] = PCIE_PERST2ACCESS2_DELAY_MAX;
	}

	if (of_property_read_u32_array(np, "t_perst2rst",
		pcie->dtsinfo.t_perst2rst, array_num)) {
		PCIE_PR_I("Fail: perst2rst time");
		pcie->dtsinfo.t_perst2rst[0] = PCIE_PERST2RST1_DELAY_MIN;
		pcie->dtsinfo.t_perst2rst[1] = PCIE_PERST2RST2_DELAY_MAX;
	}
}

static int get_rc_num(void)
{
	struct device_node *np = of_find_node_by_name(NULL, "pcie_kport");

	g_rc_num = 0;
	if (!np) {
		PCIE_PR_E("Failed to getstruct pcie_kportinfo");
		return -1;
	}

	if (of_property_read_u32(np, "rc_num", &g_rc_num)) {
		PCIE_PR_E("Failed to get rc_num info");
		return -1;
	}

	return 0;
}

static void pcie_get_noc_id(struct pcie_kport *pcie,
			    struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;

	if (of_property_read_u32(np, "noc_target_id", &pcie->dtsinfo.noc_target_id)) {
		pcie->dtsinfo.noc_target_id = 0x0;
		PCIE_PR_I("Fail: noc target id, set default[0x%x]",
			     pcie->dtsinfo.noc_target_id);
	}

	pcie->dtsinfo.noc_mntn = 0;
}

static void pcie_get_eyeparam(struct pcie_kport *pcie, struct platform_device *pdev)
{
	int ret;
	struct device_node *np = pdev->dev.of_node;
	struct pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32_array(np, "io_driver", dtsinfo->io_driver, OF_DRIVER_PARAM_NUMS);
	if (ret)
		dtsinfo->io_driver[2] = 0x0;

	ret = of_property_read_u32(np, "eye_param_nums", &dtsinfo->eye_param_nums);
	if (ret) {
		dtsinfo->eye_param_nums = 0;
		PCIE_PR_I("Default eye params");
		return;
	}

	/* All params: default val */
	if (!dtsinfo->eye_param_nums)
		return;

	PCIE_PR_I("Update eye params: %u", dtsinfo->eye_param_nums);

	dtsinfo->eye_param_data = kzalloc((u64)(dtsinfo->eye_param_nums) *
						OF_DRIVER_PARAM_NUMS * sizeof(u32), GFP_KERNEL);
	if (!dtsinfo->eye_param_data)  {
		PCIE_PR_E("Failed to alloc mem");
		return;
	}

	ret = of_property_read_u32_array(np, "eye_param_details", dtsinfo->eye_param_data,
					 (u64)(dtsinfo->eye_param_nums) * OF_DRIVER_PARAM_NUMS);
	if (ret) {
		kfree(dtsinfo->eye_param_data);
		dtsinfo->eye_param_data = NULL;
	}
}

static void pcie_get_iommus(struct pcie_kport *pcie, struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;

	pcie->dtsinfo.sup_iommus = of_property_read_bool(np, "iommus");
}

int32_t pcie_get_dtsinfo(struct pcie_kport *pcie, struct platform_device *pdev)
{
	if (!pdev->dev.of_node) {
		PCIE_PR_E("Of_node is null");
		return -EINVAL;
	}

	pcie_get_boardtype(pcie, pdev);

	pcie_get_linkstate(pcie, pdev);

	pcie_get_eco(pcie, pdev);

	pcie_get_time_params(pcie, pdev);

	pcie_get_noc_id(pcie, pdev);

	pcie_get_eyeparam(pcie, pdev);

	pcie_get_sups3info(pcie, pdev);

	pcie_get_iommus(pcie, pdev);

	if (pcie_get_isoinfo(pcie, pdev))
		return -ENODEV;

	if (pcie_get_assertinfo(pcie, pdev))
		return -ENODEV;

	if (pcie_get_clk(pcie, pdev))
		return -ENODEV;

	if (pcie_get_pinctrl(pcie, pdev))
		return -ENODEV;

	if (pcie_get_baseaddr(pcie, pdev))
		return -ENODEV;

	return 0;
}

static int perst_from_pciectrl(struct pcie_kport *pcie, int pull_up)
{
	u32 val;

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val |= PERST_FUN_SEC;
	if (pull_up)
		val |= PERST_ASSERT_EN;
	else
		val &= ~PERST_ASSERT_EN;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	return 0;
}

static int perst_from_gpio(struct pcie_kport *pcie, int pull_up)
{
	return gpio_direction_output((unsigned int)pcie->gpio_id_reset, pull_up);
}

int pcie_perst_cfg(struct pcie_kport *pcie, int pull_up)
{
	int ret;

	if (pull_up)
		usleep_range(pcie->dtsinfo.t_ref2perst[0], pcie->dtsinfo.t_ref2perst[1]);

	if (pcie->dtsinfo.board_type == BOARD_FPGA)
		ret = perst_from_pciectrl(pcie, pull_up);
	else
		ret = perst_from_gpio(pcie, pull_up);

	if (ret)
		PCIE_PR_E("Failed to pulse perst signal");

	if (pull_up)
		usleep_range(pcie->dtsinfo.t_perst2access[0], pcie->dtsinfo.t_perst2access[1]);
	else
		usleep_range(pcie->dtsinfo.t_perst2rst[0], pcie->dtsinfo.t_perst2rst[1]);

	return ret;
}

static void pcie_sideband_dbi_w_mode(struct pcie_kport *pcie, bool on)
{
	u32 val;

	if (IS_ENABLED(CONFIG_PCIE_KPORT_DBI_SPLT))
		return;

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);
}

static void pcie_sideband_dbi_r_mode(struct pcie_kport *pcie, bool on)
{
	u32 val;

	if (IS_ENABLED(CONFIG_PCIE_KPORT_DBI_SPLT))
		return;

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL1_ADDR);
}

static int pcie_establish_link(struct pcie_kport *pcie)
{
	int count = 0;

	PCIE_PR_I("+%s+", __func__);

	if (dw_pcie_link_up(pcie->pci)) {
		PCIE_PR_E("Link already up");
		return 0;
	}

	/* setup root complex */
	dw_pcie_setup_rc(&(pcie->pci->pp));
	PCIE_PR_I("Setup rc done");

	/* FPGA/EMU or UDP+FPGA need to limit speed */
	if (pcie->dtsinfo.board_type != BOARD_ASIC || pcie_ep_is_fpga(pcie))
		if (set_link_speed(pcie->rc_id, GEN1))
			PCIE_PR_E("Fail to limit speed");

	/* assert LTSSM enable */
	pcie_apb_ctrl_writel(pcie, PCIE_LTSSM_ENABLE_BIT, PCIE_APP_LTSSM_ENABLE);
	/* check if the link is up or not */
	while (!dw_pcie_link_up(pcie->pci)) {
		mdelay(1);
		count++;
		if (count == PCIE_LINK_UP_TIME) {
			PCIE_PR_E("Link Fail, status is [0x%x]",
				    pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE4_ADDR));
			dsm_pcie_dump_info(pcie, DSM_ERR_ESTABLISH_LINK);
			return -ETIMEDOUT;
		}
	}
	PCIE_PR_I("PCIe Link success after %dms", count);

	return 0;
}

/* EP register hook fun for link event notification */
int pcie_kport_register_event(struct pcie_kport_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev = NULL;
	struct pcie_port *pp = NULL;
	struct pcie_kport *pcie = NULL;
	struct dw_pcie *pci = NULL;

	if (!reg || !reg->user) {
		PCIE_PR_I("Event registration or user of event is null");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);

	pci = to_dw_pcie_from_pp(pp);
	pcie = to_pcie_port(pci);

	if (pp) {
		pcie->event_reg = reg;
		PCIE_PR_I("Event 0x%x is registered for RC", reg->events);
	} else {
		PCIE_PR_I("did not find RC for pci endpoint device");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(pcie_kport_register_event);

int pcie_kport_deregister_event(struct pcie_kport_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev = NULL;
	struct pcie_port *pp = NULL;
	struct pcie_kport *pcie = NULL;
	struct dw_pcie *pci = NULL;

	if (!reg || !reg->user) {
		PCIE_PR_I("Event registration or user of event is NULL");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);

	pci = to_dw_pcie_from_pp(pp);
	pcie = to_pcie_port(pci);

	if (pp) {
		if (reg->notify.event == PCIE_KPORT_EVENT_LINKDOWN)
			(void)flush_work(&pcie->handle_work);
		else if (reg->notify.event == PCIE_KPORT_EVENT_CPL_TIMEOUT)
			(void)flush_work(&pcie->handle_cpltimeout_work);

		pcie->event_reg = NULL;
		PCIE_PR_I("deregistered");
	} else {
		PCIE_PR_I("No RC for this EP device");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(pcie_kport_deregister_event);

/* notify EP about link-down event */
static void pcie_notify_callback(struct pcie_kport *pcie, u32 event)
{
	if ((pcie->event_reg) && (pcie->event_reg->callback) &&
	    (pcie->event_reg->events & event)) {
		struct pcie_kport_notify *notify = &pcie->event_reg->notify;

		notify->event = event;
		notify->user = pcie->event_reg->user;
		PCIE_PR_I("Callback for the event : %u", event);
		pcie->event_reg->callback(notify);
	} else {
		PCIE_PR_I("EP does not register this event : %u", event);
	}
}

static void pcie_handle_work(struct work_struct *work)
{
	struct pcie_kport *pcie = container_of(work, struct pcie_kport, handle_work);

	dsm_pcie_dump_info(pcie, DSM_ERR_LINK_DOWN);

	dump_apb_register(pcie);

	pcie_notify_callback(pcie, PCIE_KPORT_EVENT_LINKDOWN);
}

static void pcie_phy_irq_work(struct work_struct *work)
{
	struct pcie_kport *pcie = container_of(work, struct pcie_kport, phy_irq_work);

	mutex_lock(&pcie->power_lock);

	if (!atomic_read(&pcie->is_power_on)) {
		PCIE_PR_E("PCIe[%u] is Poweroff", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	pcie_phy_irq_handle(pcie);

MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
}

static irqreturn_t pcie_linkdown_irq_handler(int irq, void *arg)
{
	struct pcie_kport *pcie = arg;

	PCIE_PR_E("RC[%u], Triggle linkdown irq[%d]", pcie->rc_id, irq);

	pcie_dump_ilde_sw_stat(pcie->rc_id);

	schedule_work(&pcie->phy_irq_work);

	schedule_work(&pcie->handle_work);

	return IRQ_HANDLED;
}

static irqreturn_t pcie_msi_irq_handler(int irq, void *arg)
{
	struct pcie_kport *pcie = arg;

	return dw_handle_msi_irq(&(pcie->pci->pp));
}

static void pcie_handle_cpltimeout_work(struct work_struct *work)
{
	struct pcie_kport *pcie = container_of(work, struct pcie_kport, handle_cpltimeout_work);

	dsm_pcie_dump_info(pcie, DSM_ERR_CPL_TIMEOUT);

	dump_apb_register(pcie);

	pcie_notify_callback(pcie, PCIE_KPORT_EVENT_CPL_TIMEOUT);
}

#ifdef CONFIG_PCIE_KPORT_CPLTIMEOUT_INT
static irqreturn_t pcie_cpltimeout_irq_handler(int irq, void *arg)
{
	struct pcie_kport *pcie = arg;
	u32 val;

	PCIE_PR_E("RC[%u], Triggle CPL timeout irq[%d]", pcie->rc_id, irq);
	/* clear interrupt */
	if (atomic_read(&(pcie->is_power_on))) {
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL11_ADDR);
		val |= AXI_TIMEOUT_CLR_BIT;
		pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL11_ADDR);
	}

	schedule_work(&pcie->handle_cpltimeout_work);

	return IRQ_HANDLED;
}
#endif

#ifdef CONFIG_PCIE_KPORT_TEST
static irqreturn_t pcie_intx_irq_handler(int irq, void *arg)
{
	PCIE_PR_E("Triggle intx irq[%d]", irq);
	return IRQ_HANDLED;
}
#endif

u32 pcie_read_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			size_t size)
{
	struct pcie_kport *pcie = to_pcie_port(pci);
	u32 val = 0xFFFFFFFF;

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_E("PCIe is power off");
		return val;
	}

	pcie_sideband_dbi_r_mode(pcie, true);
	dw_pcie_read(base + reg, size, &val);
	pcie_sideband_dbi_r_mode(pcie, false);
	return val;
}

void pcie_write_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			  size_t size, u32 val)
{
	struct pcie_kport *pcie = to_pcie_port(pci);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_E("PCIe is power off");
		return;
	}

	pcie_sideband_dbi_w_mode(pcie, true);
	dw_pcie_write(base + reg, size, val);
	pcie_sideband_dbi_w_mode(pcie, false);
}

int pcie_rd_own_conf(struct pcie_port *pp, int where, int size,
			   u32 *val)
{
	int ret;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct pcie_kport *pcie = to_pcie_port(pci);

	if (!atomic_read(&(pcie->is_power_on)))
		return -EINVAL;

	pcie_sideband_dbi_r_mode(pcie, true);
	ret = dw_pcie_read(pcie->pci->dbi_base + where, size, val);
	pcie_sideband_dbi_r_mode(pcie, false);
	return ret;
}

int pcie_wr_own_conf(struct pcie_port *pp, int where, int size,
			   u32 val)
{
	int ret;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct pcie_kport *pcie = to_pcie_port(pci);

	if (!atomic_read(&(pcie->is_power_on)))
		return -EINVAL;

	pcie_sideband_dbi_w_mode(pcie, true);
	ret = dw_pcie_write(pcie->pci->dbi_base + where, size, val);
	pcie_sideband_dbi_w_mode(pcie, false);
	return ret;
}

static int pcie_host_init(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct pcie_kport *pcie = to_pcie_port(pci);

	if (pcie_establish_link(pcie))
		return -1;

	if (IS_ENABLED(CONFIG_PCI_MSI))
		dw_pcie_msi_init(pp);

	return 0;
}

static int pcie_link_up(struct dw_pcie *pci)
{
	struct pcie_kport *pcie = to_pcie_port(pci);
	u32 val;

	if (!atomic_read(&(pcie->is_power_on)) || atomic_read(&(pcie->usr_suspend)))
		return 0;

	val = pcie_apb_ctrl_readl(pcie, PCIE_ELBI_RDLH_LINKUP);
	if (((val & PCIE_LINKUP_ENABLE) == PCIE_LINKUP_ENABLE)
		&& pcie->ep_link_status == DEVICE_LINK_UP)
		return 1;

	return 0;
}

static struct dw_pcie_host_ops pcie_host_ops = {
	.rd_own_conf = pcie_rd_own_conf,
	.wr_own_conf = pcie_wr_own_conf,
	.host_init = pcie_host_init,
};

static const struct dw_pcie_ops dw_pcie_ops = {
	.read_dbi = pcie_read_dbi,
	.write_dbi = pcie_write_dbi,
	.link_up = pcie_link_up,
};

static int __init pcie_add_port(struct pcie_kport *pcie,
				      struct platform_device *pdev)
{
	int ret;
	int index;
	struct pcie_port *pp = NULL;

	pp = &(pcie->pci->pp);

	PCIE_PR_I("+%s+", __func__);
	for (index = 0; index < MAX_IRQ_NUM; index++) {
		pcie->irq[index].num = platform_get_irq(pdev, index);
		if (!pcie->irq[index].num) {
			PCIE_PR_E("Failed to get [%s] irq ,num = [%d]", pcie->irq[index].name,
				  pcie->irq[index].num);
			return -ENODEV;
		}
	PCIE_PR_I("Succeed to get [%s] irq ,num = [%d]", pcie->irq[index].name, pcie->irq[index].num);
	}
#ifdef CONFIG_PCIE_KPORT_TEST
	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTC].num,
			       pcie_intx_irq_handler, (unsigned long)IRQF_TRIGGER_RISING,
			       pcie->irq[IRQ_INTC].name, pp);
	if (ret) {
		PCIE_PR_E("Failed to request %s irq", pcie->irq[IRQ_INTC].name);
		return ret;
	}

	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTD].num,
			       pcie_intx_irq_handler, (unsigned long)IRQF_TRIGGER_RISING,
			       pcie->irq[IRQ_INTD].name, pp);
	if (ret) {
		PCIE_PR_E("Failed to request %s irq", pcie->irq[IRQ_INTD].name);
		return ret;
	}
#endif

#ifdef CONFIG_PCIE_KPORT_CPLTIMEOUT_INT
	pcie->irq[IRQ_CPLTIMEOUT].num = platform_get_irq(pdev, IRQ_CPLTIMEOUT);
	if (!pcie->irq[IRQ_CPLTIMEOUT].num) {
		PCIE_PR_E("Failed to get [%s] irq ,num = [%d]", pcie->irq[IRQ_CPLTIMEOUT].name,
			    pcie->irq[IRQ_CPLTIMEOUT].num);
		return -ENODEV;
	}
	PCIE_PR_I("Completion timeout interrupt number is %d", pcie->irq[IRQ_CPLTIMEOUT].num);

	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_CPLTIMEOUT].num,
			       pcie_cpltimeout_irq_handler,
			       (unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_CPLTIMEOUT].name, pcie);
	if (ret) {
		PCIE_PR_E("Failed to request cpltimeout irq");
		return ret;
	}

	PCIE_PR_I("Request completion timeout interrupt done!");
#endif

	ret = devm_request_irq(&pdev->dev, pcie->irq[IRQ_LINKDOWN].num,
			       pcie_linkdown_irq_handler, IRQF_TRIGGER_RISING,
			       pcie->irq[IRQ_LINKDOWN].name, pcie);
	if (ret) {
		PCIE_PR_E("Failed to request linkdown irq");
		return ret;
	}

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = pcie->irq[IRQ_MSI].num;
		ret = devm_request_irq(&pdev->dev, pp->msi_irq, pcie_msi_irq_handler,
				       IRQF_SHARED | IRQF_NO_THREAD, pcie->irq[IRQ_MSI].name, pcie);
		if (ret) {
			PCIE_PR_E("Failed to request msi irq");
			return ret;
		}
	}

	PCIE_PR_I("-%s-", __func__);
	return 0;
}

static int pcie_port_pwron_default(void *data)
{
	struct pcie_kport *pcie = data;

	return pcie_perst_cfg(pcie, ENABLE);
}

static int pcie_port_pwroff_default(void *data)
{
	struct pcie_kport *pcie = (struct pcie_kport *)data;

	return pcie_perst_cfg(pcie, DISABLE);
}

int pcie_get_port(struct pcie_kport **pcie, struct platform_device *pdev)
{
	u32 rc_id;

	if (!pdev->dev.of_node) {
		PCIE_PR_E("Of_node is null");
		return -EINVAL;
	}

	if (of_property_read_u32(pdev->dev.of_node, "rc-id", &rc_id)) {
		PCIE_PR_E("Failed to get rc_id info");
		return -EINVAL;
	}

	if (get_rc_num()) {
		PCIE_PR_E("Failed to get rc_num");
		return -EINVAL;
	}

	*pcie = get_pcie_by_id(rc_id);
	if (!(*pcie)) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	(*pcie)->rc_id = rc_id;

	return 0;
}

static int pcie_probe(struct platform_device *pdev)
{
	struct pcie_kport *pcie = NULL;
	struct pcie_port *pp = NULL;
	struct dw_pcie *pci = NULL;
	int ret;

	PCIE_PR_I("+%s+", __func__);

	ret = pcie_get_port(&pcie, pdev);
	if (ret) {
		PCIE_PR_E("Failed to get pcie from dts");
		return ret;
	}

	PCIE_PR_I("PCIe No.%u probe", pcie->rc_id);

	pci = devm_kzalloc(&pdev->dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;
	pcie->pci = pci;

	ret = pcie_get_dtsinfo(pcie, pdev);
	if (ret) {
		PCIE_PR_E("Failed to get dts info");
		goto FAIL;
	}

	pp = &(pcie->pci->pp);
	pcie->pci->dev = &(pdev->dev);
	pcie->pci->ops = &dw_pcie_ops;

	if (pcie->dtsinfo.ep_device_type != EP_DEVICE_HI110X &&
	    pcie->dtsinfo.ep_device_type != EP_DEVICE_MODEM) {
		ret = pcie_kport_power_notifiy_register(pcie->rc_id,
							pcie_port_pwron_default,
							pcie_port_pwroff_default,
							(void *)pcie);
		if (ret)
			PCIE_PR_I("Failed to register default pwr_callback_funcs");

		ret = gpio_request((unsigned int)pcie->gpio_id_reset, "pcie_reset");
		if (ret) {
			PCIE_PR_E("Failed to request gpio-%d", pcie->gpio_id_reset);
			goto FAIL;
		}
	}

	pp->ops = &pcie_host_ops;

	INIT_WORK(&pcie->handle_work, pcie_handle_work);
	INIT_WORK(&pcie->handle_cpltimeout_work, pcie_handle_cpltimeout_work);
	INIT_WORK(&pcie->phy_irq_work, pcie_phy_irq_work);

	platform_set_drvdata(pdev, pcie);

	ret = pcie_phy_register(pdev);
	if (ret)
		goto FAIL;

	ret = pcie_plat_init(pdev, pcie);
	if (ret) {
		PCIE_PR_E("Failed to get platform info");
		goto FAIL;
	}

	ret = pcie_add_port(pcie, pdev);
	if (ret < 0) {
		PCIE_PR_E("Failed to assign resource, ret=[%d]", ret);
		goto FAIL;
	}

	register_pcie_idle_hook(pdev);

	atomic_set(&(pcie->is_ready), 1);
	spin_lock_init(&pcie->ep_ltssm_lock);
	mutex_init(&pcie->power_lock);
	mutex_init(&pcie->pm_lock);

	PCIE_PR_I("-%s-", __func__);
	return 0;
FAIL:
	devm_kfree(&pdev->dev, pcie->pci);

	return ret;
}

static int pcie_save_rc_cfg(struct pcie_kport *pcie)
{
	int ret, aer_pos;
	u32 val = 0;
	struct pcie_port *pp = &(pcie->pci->pp);

	pcie_rd_own_conf(pp, PORT_MSI_CTRL_ADDR, ADDR_OFFSET_4BYTE, &val);
	pcie->msi_controller_config[0] = val;
	pcie_rd_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR, ADDR_OFFSET_4BYTE, &val);
	pcie->msi_controller_config[1] = val;
	pcie_rd_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE, ADDR_OFFSET_4BYTE, &val);
	pcie->msi_controller_config[2] = val;

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos) {
		PCIE_PR_E("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return -EINVAL;
	}

	pci_read_config_dword(pcie->rc_dev, aer_pos + PCI_ERR_ROOT_COMMAND,
			      &pcie->aer_config);

	ret = pci_save_state(pcie->rc_dev);
	if (ret) {
		PCIE_PR_E("Failed to save state of RC");
		return -EINVAL;
	}
	pcie->rc_saved_state = pci_store_saved_state(pcie->rc_dev);

	return 0;
}

static int pcie_restore_rc_cfg(struct pcie_kport *pcie)
{
	struct pcie_port *pp = &(pcie->pci->pp);
	int aer_pos;

	if (!pcie->rc_dev) {
		PCIE_PR_E("Failed to get RC dev");
		return -EINVAL;
	}

	pcie_wr_own_conf(pp, PORT_MSI_CTRL_ADDR,
			       ADDR_OFFSET_4BYTE, pcie->msi_controller_config[0]);
	pcie_wr_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR,
			       ADDR_OFFSET_4BYTE, pcie->msi_controller_config[1]);
	pcie_wr_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE,
			       ADDR_OFFSET_4BYTE, pcie->msi_controller_config[2]);

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos) {
		PCIE_PR_E("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return -EINVAL;
	}

	pci_write_config_dword(pcie->rc_dev, aer_pos + PCI_ERR_ROOT_COMMAND,
			       pcie->aer_config);

	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);
	pci_restore_state(pcie->rc_dev);
	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);

	return 0;
}

static int pcie_shutdown_prepare(struct pci_dev *dev)
{
	u32 val = 0;
	u32 pm;
	int index = 0;
	int ret;
	struct pcie_port *pp = NULL;
	struct pcie_kport *pcie = NULL;
	struct dw_pcie *pci = NULL;

	PCIE_PR_I("+%s+", __func__);

	if (!dev) {
		PCIE_PR_E("pci_dev is null");
		return -1;
	}
	pp = dev->sysdata;
	pci = to_dw_pcie_from_pp(pp);
	pcie = to_pcie_port(pci);

	/* Enable PME */
	pm = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (!pm) {
		PCIE_PR_E("Failed to get PCI_CAP_ID_PM");
		return -1;
	}
	pcie_rd_own_conf(pp, pm + PCI_PM_CTRL, ADDR_OFFSET_4BYTE, &val);
	val |= PME_TURN_OFF_BIT;
	pcie_wr_own_conf(pp, pm + PCI_PM_CTRL, ADDR_OFFSET_4BYTE, val);

	ret = pcie_generate_msg(pcie->rc_id, ATU_REGION_INDEX0,
				pcie->dtsinfo.iatu_base_offset,
				MSG_TYPE_ROUTE_BROADCAST, MSG_CODE_PME_TURN_OFF);
	if (ret) {
		PCIE_PR_E("Failed to generate msg in shutdown prepare");
		return -1;
	}

	do {
		if (index >= PCIE_SHUTDOWN_TIMEOUT) {
			PCIE_PR_E("Failed to get PME_TO_ACK");
			return -1;
		}
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE1_ADDR);
		val = val & PME_ACK_BIT;
		index++;
		udelay(SHUTDOWN_PREPARE_DELAY_10US);
	} while (val != PME_ACK_BIT);

	PCIE_PR_I("Get PME ACK");

	PCIE_PR_I("-%s-", __func__);
	return 0;
}

static void pcie_shutdown(struct platform_device *pdev)
{
	struct pcie_kport *pcie = dev_get_drvdata(&pdev->dev);

	if (!pcie) {
		PCIE_PR_E("Failed to get drvdata");
		return;
	}

	PCIE_PR_I("+%s[%u]+", __func__, pcie->rc_id);

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_I("ep_device_type is modem, skip");
		return;
	}

	if (atomic_read(&(pcie->is_power_on))) {
		if (pcie_power_ctrl(pcie, RC_POWER_OFF)) {
			PCIE_PR_E("Failed to power off");
			return;
		}
	}

	PCIE_PR_I("-%s-", __func__);
}

#ifdef CONFIG_PM
static int pcie_resume_noirq(struct device *dev)
{
	struct pcie_kport *pcie = dev_get_drvdata(dev);

	if (!pcie) {
		PCIE_PR_E("Failed to get drvdata");
		return -EINVAL;
	}

	PCIE_PR_I("+%s[%u]+", __func__, pcie->rc_id);

	if (pcie->dtsinfo.sup_s3)
		return 0;

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_I("ep_device_type is modem, skip");
		return 0;
	}

	mutex_lock(&(pcie->pm_lock));
	if (atomic_read(&(pcie->is_enumerated)) &&
	    (!atomic_read(&(pcie->usr_suspend)))) {
		if (pcie_power_ctrl(pcie, RC_POWER_RESUME)) {
			PCIE_PR_E("Failed to power on");
			goto FAIL;
		}

		if (pcie_establish_link(pcie)) {
			PCIE_PR_E("Failed to link up");
			goto FAIL;
		}

		PCIE_PR_I("Begin to recover RC cfg");
		if (pcie->rc_dev)
			pcie_restore_rc_cfg(pcie);
	}

	PCIE_PR_I("-%s-", __func__);
	mutex_unlock(&(pcie->pm_lock));
	return 0;

FAIL:
	mutex_unlock(&(pcie->pm_lock));
	schedule_work(&pcie->handle_work);

	return -EINVAL;
}

static int pcie_suspend_noirq(struct device *dev)
{
	struct pcie_kport *pcie = dev_get_drvdata(dev);

	if (!pcie) {
		PCIE_PR_E("Failed to get drvdata");
		return -EINVAL;
	}

	PCIE_PR_I("+%s[%u]+", __func__, pcie->rc_id);

	if (pcie->dtsinfo.sup_s3)
		return 0;

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_I("ep_device_type is modem, skip");
		return 0;
	}

	mutex_lock(&(pcie->pm_lock));
	if (atomic_read(&(pcie->is_power_on))) {
		if (!atomic_read(&(pcie->usr_suspend))) {
			pcie_kport_lp_ctrl(pcie->rc_id, DISABLE);
			(void)pcie_shutdown_prepare(pcie->rc_dev);
		}
		if (pcie_power_ctrl(pcie, RC_POWER_SUSPEND)) {
			PCIE_PR_E("Failed to power off");
			mutex_unlock(&(pcie->pm_lock));
			return -EINVAL;
		}
	}

	PCIE_PR_I("-%s-", __func__);
	mutex_unlock(&(pcie->pm_lock));
	return 0;
}
#endif

static const struct dev_pm_ops pcie_dev_pm_ops = {
#ifdef CONFIG_PM
	.suspend_noirq	= pcie_suspend_noirq,
	.resume_noirq	= pcie_resume_noirq,
#endif
};

static const struct of_device_id pcie_match_table[] = {
	{
		.compatible = "pcie-kport,rc",
		.data = NULL,
	},
	{},
};

static struct platform_driver pcie_driver = {
	.probe    = pcie_probe,
	.shutdown = pcie_shutdown,
	.driver   = { .name		   = "pcie-kport-rc",
		      .pm		   = &pcie_dev_pm_ops,
		      .of_match_table      = pcie_match_table,
		      .suppress_bind_attrs = true
	},
};

static int pcie_usr_suspend(struct pcie_kport *pcie, int power_off_ops)
{
	int ret;
	u32 val;

	PCIE_PR_I("+%s+", __func__);

	if (atomic_read(&(pcie->usr_suspend)) ||
	    !atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_E("Already suspend by EP");
		return -EINVAL;
	}

	if (!pcie->rc_dev) {
		PCIE_PR_E("Failed to get RC dev");
		return -1;
	}

	if (power_off_ops != POWEROFF_BUSDOWN) {
		pcie_kport_lp_ctrl(pcie->rc_id, DISABLE);
		(void)pcie_shutdown_prepare(pcie->rc_dev);
	}
	/* phy rst from sys to pipe */
	val = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val |= 0x1 << 17; /* rst sel bit */
	pcie_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	ret = pcie_power_ctrl(pcie, RC_POWER_OFF);
	if (ret) {
		PCIE_PR_E("Failed to power off");
		return -EINVAL;
	}

	atomic_set(&(pcie->usr_suspend), 1);

	PCIE_PR_I("-%s-", __func__);
	return 0;
}

static int pcie_usr_resume(struct pcie_kport *pcie)
{
	int ret;

	PCIE_PR_I("+%s+", __func__);

	atomic_set(&(pcie->usr_suspend), 0);

	ret = pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_E("Failed to power on");

		atomic_set(&(pcie->usr_suspend), 1);
		return -EINVAL;
	}

	ret = pcie_establish_link(pcie);
	if (ret) {
		if (pcie_power_ctrl(pcie, RC_POWER_OFF))
			PCIE_PR_E("Failed to power off");

		atomic_set(&(pcie->usr_suspend), 1);
		return -EINVAL;
	}

	pcie_restore_rc_cfg(pcie);

	pcie_kport_lp_ctrl(pcie->rc_id, ENABLE);

	PCIE_PR_I("-%s-", __func__);

	return 0;
}

/*
 * pm control - EP Power ON/OFF callback Function.
 * @rc_idx: [in] which rc the EP link with
 * @power_ops: [in] 0---PowerOff normally
 *                  1---Poweron normally
 *                  2---PowerOFF without PME
 *                  3---Poweron without LINK
 */
int pcie_kport_pm_control(int power_ops, u32 rc_idx)
{
	int ret = 0;
	struct pcie_kport *pcie = get_pcie_by_id(rc_idx);

	PCIE_PR_I("RC[%u], power_ops[%d]", rc_idx, power_ops);

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_E("PCIe driver is not ready");
		return -1;
	}

	mutex_lock(&(pcie->pm_lock));

	switch (power_ops) {
	case POWERON:
		dsm_pcie_clear_info();
		ret = pcie_usr_resume(pcie);
		break;

	case POWEROFF_BUSON:
	case POWEROFF_BUSDOWN:
		ret = pcie_usr_suspend(pcie, power_ops);
		break;

	case POWERON_CLK:
		ret = pcie->plat_ops->plat_on(pcie, RC_POWER_ON);
		break;

	default:
		PCIE_PR_E("Invalid power_ops[%d]", power_ops);
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&(pcie->pm_lock));
	return ret;
}
EXPORT_SYMBOL_GPL(pcie_kport_pm_control);

static void pcie_aspm_enable(struct pcie_kport *pcie)
{
#ifdef CONFIG_PCIE_KPORT_APR
	u32 reg_val;
	int ep_l1ss_pm;
#endif

	pcie_config_l0sl1(pcie->rc_id,
				(enum link_aspm_state)pcie->dtsinfo.aspm_state);

	if (IS_ENABLED(CONFIG_PCIE_KPORT_MAR) &&
		pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_I("Bypass L1ss");
		return;
	}

#ifdef CONFIG_PCIE_KPORT_APR
	if ((pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) &&
	    pcie->ep_dev) {
		ep_l1ss_pm = pci_find_ext_capability(pcie->ep_dev, PCI_EXT_L1SS_CAP_ID);
		if (!ep_l1ss_pm) {
			PCIE_PR_E("Failed to get EP PCI_EXT_L1SS_CAP_ID");
			return;
		}
		pci_read_config_dword(pcie->ep_dev, ep_l1ss_pm + PCI_EXT_L1SS_CAP, &reg_val);
		if ((reg_val & PCI_EXT_L1SS_SUP_ALL) != PCI_EXT_L1SS_SUP_ALL) {
			PCIE_PR_I("Bypass L1ss, EP not support 0x%x", reg_val);
			return;
		}
	}
#endif
	pcie_config_l1ss(pcie->rc_id, L1SS_PM_ASPM_ALL);
}

/*
 * API FOR EP to control L1&L1-substate
 * param: rc_idx---which rc the EP link with
 * enable: PCIE_KPORT_LP_ON---enable L1 and L1-substate,
 *         PCIE_KPORT_LP_Off---disable,
 *         others---illegal
 */
int pcie_kport_lp_ctrl(u32 rc_idx, u32 enable)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_idx);

	PCIE_PR_I("+%s+", enable == ENABLE ? "enable" : "disable");

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	if (pcie_ep_is_fpga(pcie)) {
		PCIE_PR_I("PCIe do not support lowpower");
		return 0;
	}

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_E("PCIe driver is not ready");
		return -EINVAL;
	}

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_E("PCIe%u is power off ", rc_idx);
		return -EINVAL;
	}

	if (enable) {
		if (pcie->dtsinfo.board_type == BOARD_ASIC)
			pcie_aspm_enable(pcie);
	} else {
		pcie_config_l1ss(pcie->rc_id, L1SS_CLOSE);
		pcie_config_l0sl1(pcie->rc_id, ASPM_CLOSE);
	}

	PCIE_PR_I("-%s-", enable == ENABLE ? "enable" : "disable");

	return 0;
}
EXPORT_SYMBOL_GPL(pcie_kport_lp_ctrl);

static int pcie_reenumerate(u32 rc_idx)
{
	int ret;

	ret = pcie_kport_pm_control(POWERON, rc_idx);
	if (!ret)
		ret = pcie_kport_rescan_ep(rc_idx);

	return ret;
}

/*
 * pcie_kport_enumerate - Enumerate Function.
 * @rc_idx: [in] which rc the EP link with
 */
int pcie_kport_enumerate(u32 rc_idx)
{
	int ret;
	u32 val, dev_id, vendor_id;
	struct pcie_port *pp = NULL;
	struct pci_dev *dev = NULL;
	struct pcie_kport *pcie = get_pcie_by_id(rc_idx);

#ifdef CONFIG_PCIE_KPORT_PC
	struct pci_saved_state *state = NULL;
#endif

	PCIE_PR_I("+RC[%u]+", rc_idx);

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	pp = &(pcie->pci->pp);

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_E("PCIe driver is not ready");
		return -1;
	}

	if (atomic_read(&(pcie->is_enumerated))) {
		if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM)
			return pcie_reenumerate(rc_idx);

		PCIE_PR_E("Enumeration was done successed before");
		return 0;
	}

	/* clk on */
	ret = pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_E("Failed to power RC");
		dsm_pcie_dump_info(pcie, DSM_ERR_POWER_ON);
		return ret;
	}

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, 0, 4); /* dbi size is 4 */
	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, 0, 4,
			     val + ((pcie->rc_id) << PCIE_DEV_ID_SHIFT));

	ret = dw_pcie_host_init(pp);
	if (ret) {
		PCIE_PR_E("Failed to initialize host");
		dsm_pcie_dump_info(pcie, DSM_ERR_ENUMERATE);
		goto FAIL_TO_POWEROFF;
	}

	pcie_rd_own_conf(pp, PCI_VENDOR_ID, 2, &vendor_id); /* config size is 2 */
	pcie_rd_own_conf(pp, PCI_DEVICE_ID, 2, &dev_id);
	pcie->rc_dev = pci_get_device(vendor_id, dev_id, pcie->rc_dev);
	if (!pcie->rc_dev) {
		PCIE_PR_E("Failed to get RC device");
		goto FAIL_TO_POWEROFF;
	}

	ret = pcie_save_rc_cfg(pcie);
	if (ret)
		goto FAIL_TO_POWEROFF;

	if (pcie->rc_dev->subordinate) {
		list_for_each_entry(dev, &pcie->rc_dev->subordinate->devices, bus_list) {
			if (pci_is_pcie(dev)) {
				pcie->ep_dev = dev;
				atomic_set(&(pcie->is_enumerated), 1);
			} else {
				PCIE_PR_E("No PCIe EP found!");
				pci_stop_and_remove_bus_device(pcie->rc_dev);
				goto FAIL_TO_POWEROFF;
			}
		}
	} else {
		PCIE_PR_E("Bus1 is null");
		pcie->ep_dev = NULL;
		pci_stop_and_remove_bus_device(pcie->rc_dev);
		goto FAIL_TO_POWEROFF;
	}

#ifdef CONFIG_PCIE_KPORT_PC
	ret = pci_save_state(pcie->ep_dev);
	if (ret) {
		PCIE_PR_E("Failed to save state of EP");
		goto FAIL_TO_POWEROFF;
	}
	state = pci_store_saved_state(pcie->ep_dev);
#endif

	pcie_kport_lp_ctrl(pcie->rc_id, ENABLE);

	atomic_set(&(pcie->usr_suspend), 0);

	PCIE_PR_I("-RC[%u]-", rc_idx);
	return 0;

FAIL_TO_POWEROFF:
	if (pcie_power_ctrl(pcie, RC_POWER_OFF))
		PCIE_PR_E("Failed to power off");

	return -1;
}
EXPORT_SYMBOL(pcie_kport_enumerate);

/*
 * pcie_kport_remove_ep - Remove EP Function.
 * @rc_idx: [in]  which rc the EP link with
 */
int pcie_kport_remove_ep(u32 rc_idx)
{
	struct pci_dev *dev = NULL;
	struct pci_dev *temp = NULL;
	struct pcie_kport *pcie = get_pcie_by_id(rc_idx);

	PCIE_PR_I("+RC[%u]+", rc_idx);

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_E("PCIe driver is not ready");
		return -1;
	}

	if (!atomic_read(&(pcie->is_enumerated))) {
		PCIE_PR_E("Enumeration was not done");
		return -1;
	}

	if (atomic_read(&(pcie->is_removed))) {
		PCIE_PR_E("Remove was done before");
		return 0;
	}

	(void)pcie_kport_lp_ctrl(pcie->rc_id, DISABLE);
	list_for_each_entry_safe(dev, temp, &pcie->rc_dev->subordinate->devices, bus_list) {
		if (pci_is_pcie(dev)) {
			pci_stop_and_remove_bus_device_locked(dev);
			if (pcie->ep_dev)
				pcie->ep_dev = NULL;
		} else {
			PCIE_PR_E("No PCIe EP found!");
			return -1;
		}
	}

	(void)atomic_inc_return(&(pcie->is_removed));

	PCIE_PR_I("-RC[%u]-", rc_idx);
	return 0;
}
EXPORT_SYMBOL(pcie_kport_remove_ep);

/*
 * pcie_kport_rescan_ep - Rescan EP Function.
 * @rc_idx: [in] which rc the EP link with
 */
int pcie_kport_rescan_ep(u32 rc_idx)
{
	struct pci_dev *dev = NULL;
	struct pci_dev *temp = NULL;
	struct pcie_kport *pcie = get_pcie_by_id(rc_idx);
	u32 max;

	PCIE_PR_I("+RC[%u]+", rc_idx);

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_E("PCIe driver is not ready");
		return -1;
	}

	if (!atomic_read(&(pcie->is_enumerated))) {
		PCIE_PR_E("Enumeration was not done");
		return -1;
	}

	if (!atomic_read(&(pcie->is_removed))) {
		PCIE_PR_E("Rescan was done before or Remove was not done");
		return 0;
	}

	pci_lock_rescan_remove();
	max = pci_rescan_bus_bridge_resize(pcie->rc_dev);
	pci_unlock_rescan_remove();

	if (!max) {
		PCIE_PR_E("Bus1 is null");
		pcie->ep_dev = NULL;
		return -1;
	}

	if (pcie_save_rc_cfg(pcie))
		return -1;

	list_for_each_entry_safe(dev, temp, &pcie->rc_dev->subordinate->devices, bus_list) {
		if (pci_is_pcie(dev)) {
			if (!pcie->ep_dev)
				pcie->ep_dev = dev;
		} else {
			PCIE_PR_E("No PCIe EP found!");
			return -1;
		}
	}

	(void)pcie_kport_lp_ctrl(pcie->rc_id, ENABLE);
	(void)atomic_dec_return(&(pcie->is_removed));

	PCIE_PR_I("-RC[%u]-", rc_idx);
	return 0;
}
EXPORT_SYMBOL(pcie_kport_rescan_ep);

int pcie_kport_ep_link_ltssm_notify(u32 rc_id, u32 link_status)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	if (link_status >= DEVICE_LINK_MAX || link_status <= DEVICE_LINK_MIN) {
		PCIE_PR_E("Invalid Device link status[%u]", link_status);
		return -EINVAL;
	}

	if (!atomic_read(&pcie->is_power_on)) {
		PCIE_PR_E("PCIe is Poweroff");
		return -EINVAL;
	}

	pcie->ep_link_status = link_status;

	return 0;
}
EXPORT_SYMBOL(pcie_kport_ep_link_ltssm_notify);

int pcie_kport_power_notifiy_register(u32 rc_id, int (*poweron)(void *data),
				      int (*poweroff)(void *data), void *data)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	pcie->callback_poweron  = poweron;
	pcie->callback_poweroff = poweroff;
	pcie->callback_data = data;
	return 0;
}
EXPORT_SYMBOL_GPL(pcie_kport_power_notifiy_register);

bool kport_pcie_bypass_s4(struct pci_dev *dev)
{
#ifdef CONFIG_PCIE_KPORT_PC
	struct pci_dev *ep_dev = NULL;

	if (!dev) {
		PCIE_PR_E("NULL Param");
		return false;
	}

	ep_dev = dev;
	if (pci_is_root_bus(dev->bus))
		ep_dev = list_first_entry(&dev->subordinate->devices,
					  struct pci_dev, bus_list);

	PCIE_PR_I("EP device id: 0x%x", ep_dev->device);

	if ((ep_dev->vendor == PCI_VENDOR_ID_REALTEK) &&
	    (ep_dev->device == PCI_DEVICE_ID_REALTEK_8168))
		return true;
#endif
	return false;
}
EXPORT_SYMBOL_GPL(kport_pcie_bypass_s4);

#define PCIE_PROGRAM_INTERFACE 8
bool kport_pcie_bypass_pm(struct pci_dev *dev)
{
	struct pci_dev *ep_dev = NULL;
	u32 class;

	if (!dev) {
		PCIE_PR_E("NULL Param");
		return false;
	}

	ep_dev = dev;
	if (pci_is_root_bus(dev->bus))
		ep_dev = list_first_entry(&dev->subordinate->devices,
					  struct pci_dev, bus_list);

	class = ep_dev->class >> PCIE_PROGRAM_INTERFACE;
	PCIE_PR_I("EP class version: 0x%x", class);

	return class == PCI_CLASS_COMMUNICATION_MODEM;
}
EXPORT_SYMBOL_GPL(kport_pcie_bypass_pm);

builtin_platform_driver(pcie_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PCIe kport driver");
