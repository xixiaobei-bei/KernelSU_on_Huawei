/*
 * pcie-kirin.c
 *
 * PCIe host controller driver for Kirin SoCs
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

#include "pcie-kirin.h"
#include "pcie-kirin-common.h"
#include <linux/module.h>
#include <linux/pci-aspm.h>

unsigned int g_rc_num;
unsigned int dbi_debug_flag;
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
struct kirin_pcie_idle_sleep g_pcie0_idle_sleep = {
	.ref_clk_onoff       = ATOMIC_INIT(0),
	.ref_clk_vote        = ATOMIC_INIT(0),
	.ref_clk_device_vote = ATOMIC_INIT(0),
#ifdef CONFIG_PCIE_KIRIN_MNTN
	.ref_clk_on_cnt      = 0,
	.ref_clk_off_cnt     = 0,
#endif
};
struct kirin_pcie_idle_sleep g_pcie1_idle_sleep = {
	.ref_clk_onoff       = ATOMIC_INIT(0),
	.ref_clk_vote        = ATOMIC_INIT(0),
	.ref_clk_device_vote = ATOMIC_INIT(0),
#ifdef CONFIG_PCIE_KIRIN_MNTN
	.ref_clk_on_cnt      = 0,
	.ref_clk_off_cnt     = 0,
#endif
};
#endif
struct kirin_pcie g_kirin_pcie[] = {
	{
		.irq = {
				{
					.name = "kirin-pcie0-inta",
				},
				{
					.name = "kirin-pcie0-msi",
				},
				{
					.name = "kirin-pcie0-intc",
				},
				{
					.name = "kirin-pcie0-intd",
				},
				{
					.name = "kirin-pcie0-linkdown",
				},
				{
					.name = "kirin-pcie0-cpltimeout",
				},
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
				{
					.name = "kirin-pcie0-idletimeout",
				},
				{
					.name = "kirin-pcie0-idlewake",
				},
#endif
			},
		.rc_dev = NULL,
		.ep_dev = NULL,
		.is_ready = ATOMIC_INIT(0),
		.is_enumerated = ATOMIC_INIT(0),
		.is_power_on = ATOMIC_INIT(0),
		.usr_suspend = ATOMIC_INIT(0),
		.is_removed = ATOMIC_INIT(0),
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
		.idle_sleep = &g_pcie0_idle_sleep,
#endif
	},
	{
		.irq = {
				{
					.name = "kirin-pcie1-inta",
				},
				{
					.name = "kirin-pcie1-msi",
				},
				{
					.name = "kirin-pcie1-intc",
				},
				{
					.name = "kirin-pcie1-intd",
				},
				{
					.name = "kirin-pcie1-linkdown",
				},
				{
					.name = "kirin-pcie1-cpltimeout",
				},
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
				{
					.name = "kirin-pcie1-idletimeout",
				},
				{
					.name = "kirin-pcie1-idlewake",
				},
#endif
			},
		.rc_dev = NULL,
		.ep_dev = NULL,
		.is_ready = ATOMIC_INIT(0),
		.is_enumerated = ATOMIC_INIT(0),
		.is_power_on = ATOMIC_INIT(0),
		.usr_suspend = ATOMIC_INIT(0),
		.is_removed = ATOMIC_INIT(0),
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
		.idle_sleep = &g_pcie1_idle_sleep,
#endif
	},
};

void kirin_elb_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->apb_base + reg);
}

u32 kirin_elb_readl(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->apb_base + reg);
}

void kirin_apb_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->phy_base + pcie->apb_phy_offset + reg);
}

u32 kirin_apb_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->phy_base + pcie->apb_phy_offset + reg);
}

void kirin_natural_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		writel(val, pcie->phy_base + pcie->natural_phy_offset + reg * REG_DWORD_ALIGN);
}

u32 kirin_natural_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		return readl(pcie->phy_base + pcie->natural_phy_offset + reg * REG_DWORD_ALIGN);
	return 0;
}

void kirin_sram_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		writel(val, pcie->phy_base + pcie->sram_phy_offset + reg * REG_DWORD_ALIGN);
}

u32 kirin_sram_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	if (pcie->dtsinfo.board_type == BOARD_ASIC)
		return readl(pcie->phy_base + pcie->sram_phy_offset + reg * REG_DWORD_ALIGN);
	return 0;
}

static int32_t kirin_pcie_get_clk(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	pcie->pcie_aux_clk = devm_clk_get(&pdev->dev, "pcie_aux");
	if (IS_ERR(pcie->pcie_aux_clk)) {
		PCIE_PR_ERR("Failed to get pcie_aux clock");
		return PTR_ERR(pcie->pcie_aux_clk);
	}

	pcie->apb_phy_clk = devm_clk_get(&pdev->dev, "pcie_apb_phy");
	if (IS_ERR(pcie->apb_phy_clk)) {
		PCIE_PR_ERR("Failed to get pcie_apb_phy clock");
		return PTR_ERR(pcie->apb_phy_clk);
	}

	pcie->apb_sys_clk = devm_clk_get(&pdev->dev, "pcie_apb_sys");
	if (IS_ERR(pcie->apb_sys_clk)) {
		PCIE_PR_ERR("Failed to get pcie_apb_sys clock");
		return PTR_ERR(pcie->apb_sys_clk);
	}

	pcie->pcie_aclk = devm_clk_get(&pdev->dev, "pcie_aclk");
	if (IS_ERR(pcie->pcie_aclk)) {
		PCIE_PR_ERR("Failed to get pcie_aclk clock");
		return PTR_ERR(pcie->pcie_aclk);
	}

	PCIE_PR_INFO("Successed to get all clock");

	return 0;
}

static int32_t get_phy_layout(struct kirin_pcie *pcie, struct device_node *np)
{
	/* get three offset from dts info */
	u32 val[3] = {0};
	const size_t size = 3;

	if (of_property_read_u32_array(np, "phy_layout_info", val, size)) {
		PCIE_PR_ERR("Failed to get phy layout info");
		return -1;
	}

	pcie->natural_phy_offset = val[0];
	pcie->sram_phy_offset = val[1];
	pcie->apb_phy_offset = val[2];

	return 0;
}

static int32_t kirin_pcie_get_baseaddr(struct kirin_pcie *pcie,
				       struct platform_device *pdev)
{
	struct resource *apb = NULL;
	struct resource *phy = NULL;
	struct resource *dbi = NULL;
	struct device_node *np = NULL;
	void __iomem *dbi_vir_base = NULL;

	apb = platform_get_resource_byname(pdev, IORESOURCE_MEM, "apb");
	pcie->apb_base = devm_ioremap_resource(&pdev->dev, apb);
	if (IS_ERR(pcie->apb_base)) {
		PCIE_PR_ERR("Failed to get PCIeCTRL apb base");
		return PTR_ERR(pcie->apb_base);
	}

	phy = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy");
	pcie->phy_base = devm_ioremap_resource(&pdev->dev, phy);
	if (IS_ERR(pcie->phy_base)) {
		PCIE_PR_ERR("Failed to get PCIePHY base");
		return PTR_ERR(pcie->phy_base);
	}

	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi) {
		PCIE_PR_ERR("Failed to get PCIe dbi resource");
		return -1;
	}
	pcie->dtsinfo.dbi_phy_base = lower_32_bits(dbi->start);

	dbi_vir_base = devm_ioremap_resource(&pdev->dev, dbi);
	if (IS_ERR(dbi_vir_base)) {
		PCIE_PR_ERR("Failed to get PCIe dbi base");
		return PTR_ERR(dbi_vir_base);
	}

	pcie->pci->dbi_base = dbi_vir_base;

	np = pdev->dev.of_node;
	if (of_property_read_u32(np, "iatu_base_offset", &pcie->dtsinfo.iatu_base_offset)) {
		PCIE_PR_ERR("Failed to get iatu_base_offset info");
		return -1;
	}

	if (get_phy_layout(pcie, np))
		return -1;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get sysctrl Node");
		return -1;
	}
	pcie->sctrl_base = of_iomap(np, 0);
	if (!pcie->sctrl_base) {
		PCIE_PR_ERR("Failed to iomap sctrl_base");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get pmctrl Node");
		goto SCTRL_BASE_UNMAP;
	}
	pcie->pmctrl_base = of_iomap(np, 0);
	if (!pcie->pmctrl_base) {
		PCIE_PR_ERR("Failed to iomap sctrl_base");
		goto SCTRL_BASE_UNMAP;
	}

	PCIE_PR_INFO("Successed to get all resource");
	return 0;

SCTRL_BASE_UNMAP:
	iounmap(pcie->sctrl_base);
	return -1;
}

static int32_t kirin_pcie_get_pinctrl(struct kirin_pcie *pcie,
				      struct platform_device *pdev)
{
	pcie->gpio_id_reset = of_get_named_gpio(pdev->dev.of_node, "reset-gpio", 0);
	if (pcie->gpio_id_reset < 0) {
		PCIE_PR_ERR("Failed to get perst gpio number");
		return -1;
	}

	return 0;
}
static void kirin_pcie_get_boardtype(struct kirin_pcie *pcie,
				     struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32(np, "board_type", &dtsinfo->board_type)) {
		dtsinfo->board_type = BOARD_ASIC;
		PCIE_PR_INFO("Failed to get board_type, set default[0x%x]", dtsinfo->board_type);
	}

	if (of_property_read_u32(np, "chip_type", &dtsinfo->chip_type)) {
		dtsinfo->chip_type = CHIP_TYPE_CS;
		PCIE_PR_INFO("Failed to get chip_type, set default[0x%x]", dtsinfo->chip_type);
	}

	if (of_property_read_u32(np, "ep_device_type", &dtsinfo->ep_device_type)) {
		dtsinfo->ep_device_type = EP_DEVICE_NODEV;
		PCIE_PR_INFO("Failed to get ep_device_type, set default[0x%x]", dtsinfo->ep_device_type);
	}

	if (of_property_read_bool(np, "ep_flag"))
		dtsinfo->ep_flag = 1;
	else
		dtsinfo->ep_flag = 0;
}

static int32_t kirin_pcie_get_isoinfo(struct kirin_pcie *pcie,
				      struct platform_device *pdev)
{
	const size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "iso_info", dtsinfo->iso_info, array_num)) {
		PCIE_PR_ERR("Failed to get isoen info");
		return -1;
	}

	return 0;
}

void kirin_pcie_iso_ctrl(struct kirin_pcie *pcie, int en_flag)
{
	if (en_flag)
		writel(pcie->dtsinfo.iso_info[1],
		       pcie->sctrl_base + pcie->dtsinfo.iso_info[0]);
	else
		writel(pcie->dtsinfo.iso_info[1],
		       pcie->sctrl_base + pcie->dtsinfo.iso_info[0] + ADDR_OFFSET_4BYTE);
}

static int32_t kirin_pcie_get_assertinfo(struct kirin_pcie *pcie,
					 struct platform_device *pdev)
{
	const size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "assert_info", dtsinfo->assert_info, array_num)) {
		PCIE_PR_ERR("Failed to get assert info");
		return -1;
	}

	return 0;
}

void kirin_pcie_reset_ctrl(struct kirin_pcie *pcie, enum RST_TYPE rst)
{
	if (rst == RST_DISABLE)
		writel(pcie->dtsinfo.assert_info[1],
		       pcie->crg_base + pcie->dtsinfo.assert_info[0] + ADDR_OFFSET_4BYTE);
	else
		writel(pcie->dtsinfo.assert_info[1],
			pcie->crg_base + pcie->dtsinfo.assert_info[0]);
}

static void kirin_pcie_get_linkstate(struct kirin_pcie *pcie,
				     struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node,
				   "ep_ltr_latency", &dtsinfo->ep_ltr_latency);
	if (ret) {
		dtsinfo->ep_ltr_latency = 0x0;
		PCIE_PR_INFO("Not set ep_ltr_latency, set default[0x%x]",
			     dtsinfo->ep_ltr_latency);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				   "ep_l1ss_ctrl2", &dtsinfo->ep_l1ss_ctrl2);
	if (ret) {
		dtsinfo->ep_l1ss_ctrl2 = 0x0;
		PCIE_PR_INFO("Not set ep_l1ss_ctrl2, set default[0x%x]",
			     dtsinfo->ep_l1ss_ctrl2);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				   "l1ss_ctrl1", &dtsinfo->l1ss_ctrl1);
	if (ret) {
		dtsinfo->l1ss_ctrl1 = 0x0;
		PCIE_PR_INFO("Not set L1ss_ctrl1, set default[0x%x]",
			     dtsinfo->l1ss_ctrl1);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				   "aspm_state", &dtsinfo->aspm_state);
	if (ret) {
		dtsinfo->aspm_state = ASPM_L1;
		PCIE_PR_INFO("Not set aspm_state, set default[0x%x]",
			     dtsinfo->aspm_state);
	}
}

static void kirin_pcie_get_eco(struct kirin_pcie *pcie,
			       struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node, "eco", &dtsinfo->eco);
	if (ret)
		dtsinfo->eco = 0x0;

	PCIE_PR_INFO("SRAM ECO [0x%x]", dtsinfo->eco);
}

static void pcie_get_clk_source(struct kirin_pcie *pcie,
				struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;
	struct device_node *np = pdev->dev.of_node;

	ret = of_property_read_u32(np, "pll_source", &dtsinfo->pll_source);
	if (ret)
		dtsinfo->pll_source = PLL_TYPE_FN;

	ret = of_property_read_u32(np, "ioref_clk_source", &dtsinfo->ioref_clk_source);
	if (ret)
		dtsinfo->ioref_clk_source = REFCLK_FROM_PLL;

	PCIE_PR_INFO("Pll source:%s, Refclk from:%s",
		     dtsinfo->pll_source == PLL_TYPE_FN ? "FN_pll" : "HP_pll",
		     dtsinfo->ioref_clk_source == REFCLK_FROM_PLL ? "FromPll" : "FromPhy");
}

static void pcie_get_time_params(struct kirin_pcie *pcie,
				 struct platform_device *pdev)
{
	const size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;

	if (of_property_read_u32_array(np, "t_ref2perst",
		pcie->dtsinfo.t_ref2perst, array_num)) {
		PCIE_PR_INFO("Fail: ref2perst time");
		pcie->dtsinfo.t_ref2perst[0] = PCIE_REF2PERST1_DELAY_MIN;
		pcie->dtsinfo.t_ref2perst[1] = PCIE_REF2PERST2_DELAY_MAX;
	}

	if (of_property_read_u32_array(np, "t_perst2access",
		pcie->dtsinfo.t_perst2access, array_num)) {
		PCIE_PR_INFO("Fail: perst2access time");
		pcie->dtsinfo.t_perst2access[0] = PCIE_PERST2ACCESS1_DELAY_MIN;
		pcie->dtsinfo.t_perst2access[1] = PCIE_PERST2ACCESS2_DELAY_MAX;
	}

	if (of_property_read_u32_array(np, "t_perst2rst",
		pcie->dtsinfo.t_perst2rst, array_num)) {
		PCIE_PR_INFO("Fail: perst2rst time");
		pcie->dtsinfo.t_perst2rst[0] = PCIE_PERST2RST1_DELAY_MIN;
		pcie->dtsinfo.t_perst2rst[1] = PCIE_PERST2RST2_DELAY_MAX;
	}
}

static int get_rc_num(void)
{
	struct device_node *np = of_find_node_by_name(NULL, "kirin_pcie");
	g_rc_num = 0;
	if (!np) {
		PCIE_PR_ERR("Failed to get kirin_pcie info");
		return -1;
	}

	if (of_property_read_u32(np, "rc_num", &g_rc_num)) {
		PCIE_PR_ERR("Failed to get rc_num info");
		return -1;
	}

	return 0;
}

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
static int32_t get_pcie_idle_sleep_dtsinfo(struct kirin_pcie *pcie,
					   struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	u32 val[3] = {0}; /* get three offset from dts info */

	if (of_property_read_u32(np, "idle_sleep_enable",
				 &(pcie->idle_sleep->idle_sleep_enable)))
		pcie->idle_sleep->idle_sleep_enable = 1;

	/* get dts info : enable bit, mask bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_timeout_en", val, 3)) {
		PCIE_PR_ERR("Failed to get pcie idle timeout info");
		return -1;
	}
	pcie->idle_sleep->idle_timeout_en_bit = val[0];
	pcie->idle_sleep->idle_timeout_mask_bit = val[1];
	pcie->idle_sleep->idle_timeout_int_addr = val[2];

	/* get dts info : clear  bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_timeout_clr", val, 2)) {
		PCIE_PR_ERR("Failed to get pcie idle timeout clear info");
		return -1;
	}
	pcie->idle_sleep->idle_timeout_clr_bit = val[0];
	pcie->idle_sleep->idle_timeout_clr_addr = val[1];

	/* get dts info : enable bit, mask bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_wake_en", val, 3)) {
		PCIE_PR_ERR("Failed to get pcie idle wake info");
		return -1;
	}
	pcie->idle_sleep->idle_wake_en_bit = val[0];
	pcie->idle_sleep->idle_wake_mask_bit = val[1];
	pcie->idle_sleep->idle_wake_int_addr = val[2];

	/* get dts info : clear  bit, reg_addr */
	if (of_property_read_u32_array(np, "idle_wake_clr", val, 2)) {
		PCIE_PR_ERR("Failed to get pcie idle wake clear info");
		return -1;
	}
	pcie->idle_sleep->idle_wake_clr_bit = val[0];
	pcie->idle_sleep->idle_wake_clr_addr = val[1];

	/*
	 * get dts info :
	 *    l1ss cnt set val(if 0, then use default), l1ss cnt reg addr
	 */
	if (of_property_read_u32_array(np, "l1ss_cnt_base", val, 2)) {
		PCIE_PR_ERR("Failed to get pcie l1ss cnt info");
		return -1;
	}
	pcie->idle_sleep->l1ss_cnt_base = val[0];
	pcie->idle_sleep->l1ss_cnt_base_addr = val[1];

	return 0;
}
#endif

static void pcie_get_noc_id(struct kirin_pcie *pcie,
			    struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;

	if (of_property_read_u32(np, "noc_target_id", &pcie->dtsinfo.noc_target_id)) {
		pcie->dtsinfo.noc_target_id = 0x0;
		PCIE_PR_INFO("Fail: noc target id, set default[0x%x]",
			     pcie->dtsinfo.noc_target_id);
	}

	pcie->dtsinfo.noc_mntn = 0;
}

static void kirin_pcie_get_eyeparam(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int ret;
	struct device_node *np = pdev->dev.of_node;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32_array(np, "io_driver", dtsinfo->io_driver, OF_DRIVER_PARAM_NUMS);
	if (ret)
		dtsinfo->io_driver[2] = 0x0;

	ret = of_property_read_u32(np, "eye_param_nums", &dtsinfo->eye_param_nums);
	if (ret) {
		dtsinfo->eye_param_nums = 0;
		PCIE_PR_INFO("Default eye params");
		return;
	}

	/* All params: default val */
	if (!dtsinfo->eye_param_nums)
		return;

	PCIE_PR_INFO("Update eye params: %u", dtsinfo->eye_param_nums);

	dtsinfo->eye_param_data = kzalloc((u64)(dtsinfo->eye_param_nums) *
						OF_DRIVER_PARAM_NUMS * sizeof(u32), GFP_KERNEL);
	if (!dtsinfo->eye_param_data)  {
		PCIE_PR_ERR("Failed to alloc mem");
		return;
	}

	ret = of_property_read_u32_array(np, "eye_param_details", dtsinfo->eye_param_data,
					 (u64)(dtsinfo->eye_param_nums) * OF_DRIVER_PARAM_NUMS);
	if (ret) {
		kfree(dtsinfo->eye_param_data);
		dtsinfo->eye_param_data = NULL;
	}
}

int32_t kirin_pcie_get_dtsinfo(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	if (!pdev->dev.of_node) {
		PCIE_PR_ERR("Of_node is null");
		return -EINVAL;
	}

	kirin_pcie_get_boardtype(pcie, pdev);

	kirin_pcie_get_linkstate(pcie, pdev);

	kirin_pcie_get_eco(pcie, pdev);

	pcie_get_clk_source(pcie, pdev);

	pcie_get_time_params(pcie, pdev);

	pcie_get_noc_id(pcie, pdev);

	kirin_pcie_get_eyeparam(pcie, pdev);

	if (kirin_pcie_get_isoinfo(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_assertinfo(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_clk(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_pinctrl(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_baseaddr(pcie, pdev))
		return -ENODEV;

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	if (get_pcie_idle_sleep_dtsinfo(pcie, pdev))
		return -ENODEV;
#endif

	return 0;
}

static int perst_from_pciectrl(struct kirin_pcie *pcie, int pull_up)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val |= PERST_FUN_SEC;
	if (pull_up)
		val |= PERST_ASSERT_EN;
	else
		val &= ~PERST_ASSERT_EN;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	return 0;
}

static int perst_from_gpio(struct kirin_pcie *pcie, int pull_up)
{
	return gpio_direction_output((unsigned int)pcie->gpio_id_reset, pull_up);
}

int kirin_pcie_perst_cfg(struct kirin_pcie *pcie, int pull_up)
{
	int ret;

	if (pull_up)
		usleep_range(pcie->dtsinfo.t_ref2perst[0], pcie->dtsinfo.t_ref2perst[1]);

	if (pcie->dtsinfo.board_type == BOARD_FPGA)
		ret = perst_from_pciectrl(pcie, pull_up);
	else
		ret = perst_from_gpio(pcie, pull_up);

	if (ret)
		PCIE_PR_ERR("Failed to pulse perst signal");

	if (pull_up)
		usleep_range(pcie->dtsinfo.t_perst2access[0], pcie->dtsinfo.t_perst2access[1]);
	else
		usleep_range(pcie->dtsinfo.t_perst2rst[0], pcie->dtsinfo.t_perst2rst[1]);

	return ret;
}

static void kirin_pcie_sideband_dbi_w_mode(struct kirin_pcie *pcie, bool on)
{
	u32 val;

	if (IS_ENABLED(CONFIG_KIRIN_PCIE_MAY))
		return;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);
}

static void kirin_pcie_sideband_dbi_r_mode(struct kirin_pcie *pcie, bool on)
{
	u32 val;

	if (IS_ENABLED(CONFIG_KIRIN_PCIE_MAY))
		return;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL1_ADDR);
}

static int kirin_pcie_establish_link(struct kirin_pcie *pcie)
{
	int count = 0;

	PCIE_PR_INFO("+%s+", __func__);

	if (dw_pcie_link_up(pcie->pci)) {
		PCIE_PR_ERR("Link already up");
		return 0;
	}

	/* setup root complex */
	dw_pcie_setup_rc(&(pcie->pci->pp));
	PCIE_PR_INFO("Setup rc done");

	/* assert LTSSM enable */
	kirin_elb_writel(pcie, PCIE_LTSSM_ENABLE_BIT, PCIE_APP_LTSSM_ENABLE);
	/* check if the link is up or not */
	while (!dw_pcie_link_up(pcie->pci)) {
		mdelay(1);
		count++;
		if (count == PCIE_LINK_UP_TIME) {
			PCIE_PR_ERR("Link Fail, status is [0x%x]",
				    kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR));
			dsm_pcie_dump_info(pcie, DSM_ERR_ESTABLISH_LINK);
			return -ETIMEDOUT;
		}
	}
	PCIE_PR_INFO("PCIe Link success after %dms", count);

	return 0;
}

/* EP register hook fun for link event notification */
int kirin_pcie_register_event(struct kirin_pcie_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev = NULL;
	struct pcie_port *pp = NULL;
	struct kirin_pcie *pcie = NULL;
	struct dw_pcie *pci = NULL;

	if (!reg || !reg->user) {
		PCIE_PR_INFO("Event registration or user of event is null");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);

	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);

	if (pp) {
		pcie->event_reg = reg;
		PCIE_PR_INFO("Event 0x%x is registered for RC", reg->events);
	} else {
		PCIE_PR_INFO("did not find RC for pci endpoint device");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_register_event);

int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev = NULL;
	struct pcie_port *pp = NULL;
	struct kirin_pcie *pcie = NULL;
	struct dw_pcie *pci = NULL;

	if (!reg || !reg->user) {
		PCIE_PR_INFO("Event registration or user of event is NULL");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);

	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);

	if (pp) {
		if (reg->notify.event == KIRIN_PCIE_EVENT_LINKDOWN)
			(void)flush_work(&pcie->handle_work);
		else if (reg->notify.event == KIRIN_PCIE_EVENT_CPL_TIMEOUT)
			(void)flush_work(&pcie->handle_cpltimeout_work);

		pcie->event_reg = NULL;
		PCIE_PR_INFO("deregistered");
	} else {
		PCIE_PR_INFO("No RC for this EP device");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_deregister_event);

/* notify EP about link-down event */
static void kirin_pcie_notify_callback(struct kirin_pcie *pcie,
				       enum kirin_pcie_event event)
{
	if ((pcie->event_reg) && (pcie->event_reg->callback) &&
	    (pcie->event_reg->events & (u32)event)) {
		struct kirin_pcie_notify *notify = &pcie->event_reg->notify;

		notify->event = event;
		notify->user = pcie->event_reg->user;
		PCIE_PR_INFO("Callback for the event : %d", event);
		pcie->event_reg->callback(notify);
	} else {
		PCIE_PR_INFO("EP does not register this event : %d", event);
	}
}

static void kirin_handle_work(struct work_struct *work)
{
	struct kirin_pcie *pcie = container_of(work, struct kirin_pcie, handle_work);

	dsm_pcie_dump_info(pcie, DSM_ERR_LINK_DOWN);

	dump_apb_register(pcie);

	kirin_pcie_notify_callback(pcie, KIRIN_PCIE_EVENT_LINKDOWN);
}

static irqreturn_t kirin_pcie_linkdown_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;

	PCIE_PR_ERR("RC[%u], Triggle linkdown irq[%d]", pcie->rc_id, irq);

	schedule_work(&pcie->handle_work);

	return IRQ_HANDLED;
}

static irqreturn_t kirin_pcie_msi_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;

	return dw_handle_msi_irq(&(pcie->pci->pp));
}

static void kirin_handle_cpltimeout_work(struct work_struct *work)
{
	struct kirin_pcie *pcie = container_of(work, struct kirin_pcie, handle_cpltimeout_work);

	dsm_pcie_dump_info(pcie, DSM_ERR_CPL_TIMEOUT);

	dump_apb_register(pcie);

	kirin_pcie_notify_callback(pcie, KIRIN_PCIE_EVENT_CPL_TIMEOUT);
}

#ifdef CONFIG_KIRIN_PCIE_CPLTIMEOUT_INT
static irqreturn_t kirin_pcie_cpltimeout_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;
	u32 val;

	PCIE_PR_ERR("RC[%u], Triggle CPL timeout irq[%d]", pcie->rc_id, irq);
	/* clear interrupt */
	if (atomic_read(&(pcie->is_power_on))) {
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL11_ADDR);
		val |= AXI_TIMEOUT_CLR_BIT;
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL11_ADDR);
	}

	schedule_work(&pcie->handle_cpltimeout_work);

	return IRQ_HANDLED;
}
#endif

#ifdef CONFIG_KIRIN_PCIE_TEST
static irqreturn_t kirin_pcie_intx_irq_handler(int irq, void *arg)
{
	PCIE_PR_ERR("Triggle intx irq[%d]", irq);
	return IRQ_HANDLED;
}
#endif

u32 kirin_pcie_read_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			size_t size)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pci);
	u32 val = 0xFFFFFFFF;

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return val;
	}

	kirin_pcie_sideband_dbi_r_mode(pcie, true);
	dw_pcie_read(base + reg, size, &val);
	kirin_pcie_sideband_dbi_r_mode(pcie, false);
	return val;
}

void kirin_pcie_write_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			  size_t size, u32 val)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pci);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return;
	}

	kirin_pcie_sideband_dbi_w_mode(pcie, true);
	dw_pcie_write(base + reg, size, val);
	kirin_pcie_sideband_dbi_w_mode(pcie, false);
}

int kirin_pcie_rd_own_conf(struct pcie_port *pp, int where, int size,
			   u32 *val)
{
	int ret;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct kirin_pcie *pcie = to_kirin_pcie(pci);

	if (!atomic_read(&(pcie->is_power_on)))
		return -EINVAL;

	kirin_pcie_sideband_dbi_r_mode(pcie, true);
	ret = dw_pcie_read(pcie->pci->dbi_base + where, size, val);
	kirin_pcie_sideband_dbi_r_mode(pcie, false);
	return ret;
}

int kirin_pcie_wr_own_conf(struct pcie_port *pp, int where, int size,
			   u32 val)
{
	int ret;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct kirin_pcie *pcie = to_kirin_pcie(pci);

	if (!atomic_read(&(pcie->is_power_on)))
		return -EINVAL;

	kirin_pcie_sideband_dbi_w_mode(pcie, true);
	ret = dw_pcie_write(pcie->pci->dbi_base + where, size, val);
	kirin_pcie_sideband_dbi_w_mode(pcie, false);
	return ret;
}

static int kirin_pcie_host_init(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct kirin_pcie *pcie = to_kirin_pcie(pci);

	if (kirin_pcie_establish_link(pcie))
		return -1;

	if (IS_ENABLED(CONFIG_PCI_MSI))
		dw_pcie_msi_init(pp);

	return 0;
}

static int kirin_pcie_link_up(struct dw_pcie *pci)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pci);
	u32 val;

	if (!atomic_read(&(pcie->is_power_on)) || atomic_read(&(pcie->usr_suspend)))
		return 0;

	val = kirin_elb_readl(pcie, PCIE_ELBI_RDLH_LINKUP);
	if (((val & PCIE_LINKUP_ENABLE) == PCIE_LINKUP_ENABLE)
		&& pcie->ep_link_status == DEVICE_LINK_UP)
		return 1;

	return 0;
}

static struct dw_pcie_host_ops kirin_pcie_host_ops = {
	.rd_own_conf = kirin_pcie_rd_own_conf,
	.wr_own_conf = kirin_pcie_wr_own_conf,
	.host_init = kirin_pcie_host_init,
};

static const struct dw_pcie_ops dw_pcie_ops = {
	.read_dbi = kirin_pcie_read_dbi,
	.write_dbi = kirin_pcie_write_dbi,
	.link_up = kirin_pcie_link_up,
};

static int __init kirin_add_pcie_port(struct kirin_pcie *pcie,
				      struct platform_device *pdev)
{
	int ret;
	int index;
	struct pcie_port *pp = NULL;

	pp = &(pcie->pci->pp);

	PCIE_PR_INFO("+%s+", __func__);
	for (index = 0; index < MAX_IRQ_NUM; index++) {
		pcie->irq[index].num = platform_get_irq(pdev, index);
		if (!pcie->irq[index].num) {
			PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]", pcie->irq[index].name,
				    pcie->irq[index].num);
			return -ENODEV;
		}
	PCIE_PR_INFO("Succeed to get [%s] irq ,num = [%d]", pcie->irq[index].name,
		     pcie->irq[index].num);
	}
#ifdef CONFIG_KIRIN_PCIE_TEST
	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTC].num,
			       kirin_pcie_intx_irq_handler, (unsigned long)IRQF_TRIGGER_RISING,
			       pcie->irq[IRQ_INTC].name, pp);
	if (ret) {
		PCIE_PR_ERR("Failed to request %s irq", pcie->irq[IRQ_INTC].name);
		return ret;
	}

	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTD].num,
			       kirin_pcie_intx_irq_handler, (unsigned long)IRQF_TRIGGER_RISING,
			       pcie->irq[IRQ_INTD].name, pp);
	if (ret) {
		PCIE_PR_ERR("Failed to request %s irq", pcie->irq[IRQ_INTD].name);
		return ret;
	}
#endif

#ifdef CONFIG_KIRIN_PCIE_CPLTIMEOUT_INT
	pcie->irq[IRQ_CPLTIMEOUT].num = platform_get_irq(pdev, IRQ_CPLTIMEOUT);
	if (!pcie->irq[IRQ_CPLTIMEOUT].num) {
		PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]", pcie->irq[IRQ_CPLTIMEOUT].name,
			    pcie->irq[IRQ_CPLTIMEOUT].num);
		return -ENODEV;
	}
	PCIE_PR_INFO("Completion timeout interrupt number is %d", pcie->irq[IRQ_CPLTIMEOUT].num);

	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_CPLTIMEOUT].num,
			       kirin_pcie_cpltimeout_irq_handler,
			       (unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_CPLTIMEOUT].name, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to request cpltimeout irq");
		return ret;
	}

	PCIE_PR_INFO("Request completion timeout interrupt done!");
#endif

	ret = devm_request_irq(&pdev->dev, pcie->irq[IRQ_LINKDOWN].num,
			       kirin_pcie_linkdown_irq_handler, IRQF_TRIGGER_RISING,
			       pcie->irq[IRQ_LINKDOWN].name, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to request linkdown irq");
		return ret;
	}

	PCIE_PR_INFO("pcie->irq[%d].name = [%s], pcie->irq[%d].name = [%s]", IRQ_LINKDOWN,
		     pcie->irq[IRQ_LINKDOWN].name, IRQ_LINKDOWN, pcie->irq[IRQ_LINKDOWN].name);

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = pcie->irq[IRQ_MSI].num;
		ret = devm_request_irq(&pdev->dev, pp->msi_irq,
				       kirin_pcie_msi_irq_handler,
				       IRQF_SHARED | IRQF_NO_THREAD,
				       pcie->irq[IRQ_MSI].name, pcie);
		if (ret) {
			PCIE_PR_ERR("Failed to request msi irq");
			return ret;
		}
	}

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	ret = kirin_add_idle_sleep_pcie_port(pcie, pdev);
	if (ret)
		return ret;
#endif

	PCIE_PR_INFO("-%s-", __func__);
	return 0;
}

static int kirin_pcie_pwron_default(void *data)
{
	struct kirin_pcie *pcie = data;

	return kirin_pcie_perst_cfg(pcie, ENABLE);
}

static int kirin_pcie_pwroff_default(void *data)
{
	struct kirin_pcie *pcie = (struct kirin_pcie *)data;

	return kirin_pcie_perst_cfg(pcie, DISABLE);
}

int kirin_pcie_get_pcie(struct kirin_pcie **pcie, struct platform_device *pdev)
{
	u32 rc_id;

	if (!pdev->dev.of_node) {
		PCIE_PR_ERR("Of_node is null");
		return -EINVAL;
	}

	if (of_property_read_u32(pdev->dev.of_node, "rc-id", &rc_id)) {
		PCIE_PR_ERR("Failed to get rc_id info");
		return -EINVAL;
	}

	if (get_rc_num()) {
		PCIE_PR_ERR("Failed to get rc_num");
		return -EINVAL;
	}

	if ((rc_id >= g_rc_num) || (rc_id >= ARRAY_SIZE(g_kirin_pcie))) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	*pcie = &g_kirin_pcie[rc_id];

	(*pcie)->rc_id = rc_id;

	return 0;
}

static int kirin_pcie_probe(struct platform_device *pdev)
{
	struct kirin_pcie *pcie = NULL;
	struct pcie_port *pp = NULL;
	struct dw_pcie *pci = NULL;
	int ret;

	PCIE_PR_INFO("+%s+", __func__);

	ret = kirin_pcie_get_pcie(&pcie, pdev);
	if (ret) {
		PCIE_PR_ERR("Failed to get kirin pcie from dts");
		return ret;
	}

	PCIE_PR_INFO("PCIe No.%u probe", pcie->rc_id);

	pci = devm_kzalloc(&pdev->dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;
	pcie->pci = pci;

	ret = kirin_pcie_get_dtsinfo(pcie, pdev);
	if (ret) {
		PCIE_PR_ERR("Failed to get dts info");
		goto FAIL;
	}

	pp = &(pcie->pci->pp);
	pcie->pci->dev = &(pdev->dev);
	pcie->pci->ops = &dw_pcie_ops;

	if (pcie->dtsinfo.ep_device_type != EP_DEVICE_HI110X &&
	    pcie->dtsinfo.ep_device_type != EP_DEVICE_MODEM) {
		ret = kirin_pcie_power_notifiy_register(pcie->rc_id,
							kirin_pcie_pwron_default,
							kirin_pcie_pwroff_default,
							(void *)pcie);
		if (ret)
			PCIE_PR_INFO("Failed to register default pwr_callback_funcs");

		ret = gpio_request((unsigned int)pcie->gpio_id_reset, "pcie_reset");
		if (ret) {
			PCIE_PR_ERR("Failed to request gpio-%d", pcie->gpio_id_reset);
			goto FAIL;
		}
	}

	pp->ops = &kirin_pcie_host_ops;

	INIT_WORK(&pcie->handle_work, kirin_handle_work);
	INIT_WORK(&pcie->handle_cpltimeout_work, kirin_handle_cpltimeout_work);

	platform_set_drvdata(pdev, pcie);

	ret = pcie_plat_init(pdev, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to get platform info");
		goto FAIL;
	}

	ret = kirin_add_pcie_port(pcie, pdev);
	if (ret < 0) {
		PCIE_PR_ERR("Failed to assign resource, ret=[%d]", ret);
		goto FAIL;
	}

	atomic_set(&(pcie->is_ready), 1);
	spin_lock_init(&pcie->ep_ltssm_lock);
	mutex_init(&pcie->power_lock);
	mutex_init(&pcie->pm_lock);
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
	spin_lock_init(&(pcie->idle_sleep->idle_sleep_lock));
#endif

	PCIE_PR_INFO("-%s-", __func__);
	return 0;
FAIL:
	devm_kfree(&pdev->dev, pcie->pci);

	return ret;
}

static int kirin_pcie_save_rc_cfg(struct kirin_pcie *pcie)
{
	int ret, aer_pos;
	u32 val = 0;
	struct pcie_port *pp = &(pcie->pci->pp);

	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_ADDR, ADDR_OFFSET_4BYTE, &val);
	pcie->msi_controller_config[0] = val;
	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR, ADDR_OFFSET_4BYTE, &val);
	pcie->msi_controller_config[1] = val;
	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE, ADDR_OFFSET_4BYTE, &val);
	pcie->msi_controller_config[2] = val;

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos) {
		PCIE_PR_ERR("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return -EINVAL;
	}

	pci_read_config_dword(pcie->rc_dev, aer_pos + PCI_ERR_ROOT_COMMAND,
			      &pcie->aer_config);

	ret = pci_save_state(pcie->rc_dev);
	if (ret) {
		PCIE_PR_ERR("Failed to save state of RC");
		return -EINVAL;
	}
	pcie->rc_saved_state = pci_store_saved_state(pcie->rc_dev);

	return 0;
}

static int kirin_pcie_restore_rc_cfg(struct kirin_pcie *pcie)
{
	struct pcie_port *pp = &(pcie->pci->pp);
	int aer_pos;

	if (!pcie->rc_dev) {
		PCIE_PR_ERR("Failed to get RC dev");
		return -EINVAL;
	}

	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_ADDR,
			       ADDR_OFFSET_4BYTE, pcie->msi_controller_config[0]);
	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR,
			       ADDR_OFFSET_4BYTE, pcie->msi_controller_config[1]);
	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE,
			       ADDR_OFFSET_4BYTE, pcie->msi_controller_config[2]);

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos) {
		PCIE_PR_ERR("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return -EINVAL;
	}

	pci_write_config_dword(pcie->rc_dev, aer_pos + PCI_ERR_ROOT_COMMAND,
			       pcie->aer_config);

	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);
	pci_restore_state(pcie->rc_dev);
	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);

	return 0;
}

static int kirin_pcie_shutdown_prepare(struct pci_dev *dev)
{
	u32 val = 0;
	u32 pm, iatu_offset;
	int index = 0;
	struct pcie_port *pp = NULL;
	struct kirin_pcie *pcie = NULL;
	struct dw_pcie *pci = NULL;

	PCIE_PR_INFO("+%s+", __func__);

	if (!dev) {
		PCIE_PR_ERR("pci_dev is null");
		return -1;
	}
	pp = dev->sysdata;
	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);
	iatu_offset = pcie->dtsinfo.iatu_base_offset;

	/* Enable PME */
	pm = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (!pm) {
		PCIE_PR_ERR("Failed to get PCI_CAP_ID_PM");
		return -1;
	}
	kirin_pcie_rd_own_conf(pp, pm + PCI_PM_CTRL, ADDR_OFFSET_4BYTE, &val);
	val |= PME_TURN_OFF_BIT;
	kirin_pcie_wr_own_conf(pp, pm + PCI_PM_CTRL, ADDR_OFFSET_4BYTE, val);

	kirin_pcie_generate_msg(pcie->rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, iatu_offset,
				MSG_TYPE_ROUTE_BROADCAST, MSG_CODE_PME_TURN_OFF);

	do {
		if (index >= PCIE_SHUTDOWN_TIMEOUT) {
			PCIE_PR_ERR("Failed to get PME_TO_ACK");
			return -1;
		}
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE1_ADDR);
		val = val & PME_ACK_BIT;
		index++;
		udelay(10);
	} while (val != PME_ACK_BIT);

	PCIE_PR_INFO("Get PME ACK");

	PCIE_PR_INFO("-%s-", __func__);
	return 0;
}

static void kirin_pcie_shutdown(struct platform_device *pdev)
{
	struct kirin_pcie *pcie = dev_get_drvdata(&pdev->dev);

	if (!pcie) {
		PCIE_PR_ERR("Failed to get drvdata");
		return;
	}

	PCIE_PR_INFO("+%s[%u]+", __func__, pcie->rc_id);

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_INFO("ep_device_type is modem, skip");
		return;
	}

	if (atomic_read(&(pcie->is_power_on))) {
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_OFF)) {
			PCIE_PR_ERR("Failed to power off");
			return;
		}
	}

	PCIE_PR_INFO("-%s-", __func__);
}

#ifdef CONFIG_PM
static int kirin_pcie_resume_noirq(struct device *dev)
{
	struct kirin_pcie *pcie = dev_get_drvdata(dev);

	if (!pcie) {
		PCIE_PR_ERR("Failed to get drvdata");
		return -EINVAL;
	}

	PCIE_PR_INFO("+%s[%u]+", __func__, pcie->rc_id);

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_INFO("ep_device_type is modem, skip");
		return 0;
	}

	mutex_lock(&(pcie->pm_lock));
	if (atomic_read(&(pcie->is_enumerated)) &&
	    (!atomic_read(&(pcie->usr_suspend)))) {
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_RESUME)) {
			PCIE_PR_ERR("Failed to power on");
			goto FAIL;
		}

		if (kirin_pcie_establish_link(pcie)) {
			PCIE_PR_ERR("Failed to link up");
			goto FAIL;
		}

		PCIE_PR_INFO("Begin to recover RC cfg");
		if (pcie->rc_dev)
			kirin_pcie_restore_rc_cfg(pcie);
	}

	PCIE_PR_INFO("-%s-", __func__);
	mutex_unlock(&(pcie->pm_lock));
	return 0;

FAIL:
	mutex_unlock(&(pcie->pm_lock));
	schedule_work(&pcie->handle_work);

	return -EINVAL;
}

static int kirin_pcie_suspend_noirq(struct device *dev)
{
	struct kirin_pcie *pcie = dev_get_drvdata(dev);

	if (!pcie) {
		PCIE_PR_ERR("Failed to get drvdata");
		return -EINVAL;
	}

	PCIE_PR_INFO("+%s[%u]+", __func__, pcie->rc_id);

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_INFO("ep_device_type is modem, skip");
		return 0;
	}

	mutex_lock(&(pcie->pm_lock));
	if (atomic_read(&(pcie->is_power_on))) {
		if (!atomic_read(&(pcie->usr_suspend))) {
			kirin_pcie_lp_ctrl(pcie->rc_id, DISABLE);
			kirin_pcie_shutdown_prepare(pcie->rc_dev);
		}
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_SUSPEND)) {
			PCIE_PR_ERR("Failed to power off");
			mutex_unlock(&(pcie->pm_lock));
			return -EINVAL;
		}
	}

	PCIE_PR_INFO("-%s-", __func__);
	mutex_unlock(&(pcie->pm_lock));
	return 0;
}
#endif

static const struct dev_pm_ops kirin_pcie_dev_pm_ops = {
#ifdef CONFIG_PM
	.suspend_noirq	= kirin_pcie_suspend_noirq,
	.resume_noirq	= kirin_pcie_resume_noirq,
#endif
};

static const struct of_device_id kirin_pcie_match_table[] = {
	{
		.compatible = "hisilicon,kirin-pcie",
		.data = NULL,
	},
	{},
};

static struct platform_driver kirin_pcie_driver = {
	.probe    = kirin_pcie_probe,
	.shutdown = kirin_pcie_shutdown,
	.driver   = { .name		   = "Kirin-pcie",
		      .pm		   = &kirin_pcie_dev_pm_ops,
		      .of_match_table      = kirin_pcie_match_table,
		      .suppress_bind_attrs = true
	},
};

static int kirin_pcie_usr_suspend(u32 rc_idx, int power_off_ops)
{
	int ret;
	u32 val;
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	PCIE_PR_INFO("+%s+", __func__);

	if (atomic_read(&(pcie->usr_suspend)) ||
	    !atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("Already suspend by EP");
		return -EINVAL;
	}

	if (!pcie->rc_dev) {
		PCIE_PR_ERR("Failed to get RC dev");
		return -1;
	}

	if (power_off_ops != POWEROFF_BUSDOWN) {
		kirin_pcie_lp_ctrl(rc_idx, DISABLE);
		ret = kirin_pcie_shutdown_prepare(pcie->rc_dev);
		if (ret)
			PCIE_PR_ERR("device do not reply ACK");
	}
	/* phy rst from sys to pipe */
	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val |= 0x1 << 17;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_OFF);
	if (ret) {
		PCIE_PR_ERR("Failed to power off");
		return -EINVAL;
	}

	atomic_set(&(pcie->usr_suspend), 1);

	PCIE_PR_INFO("-%s-", __func__);
	return 0;
}

static int kirin_pcie_usr_resume(u32 rc_idx)
{
	int ret;
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	PCIE_PR_INFO("+%s+", __func__);

	atomic_set(&(pcie->usr_suspend), 0);

	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power on");

		atomic_set(&(pcie->usr_suspend), 1);
		return -EINVAL;
	}

	ret = kirin_pcie_establish_link(pcie);
	if (ret) {
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_OFF))
			PCIE_PR_ERR("Failed to power off");

		atomic_set(&(pcie->usr_suspend), 1);
		return -EINVAL;
	}

	kirin_pcie_restore_rc_cfg(pcie);

	kirin_pcie_lp_ctrl(rc_idx, ENABLE);

	PCIE_PR_INFO("-%s-", __func__);

	return 0;
}

/*
 * EP Power ON/OFF callback Function:
 * param: rc_idx---which rc the EP link with
 * power_ops: 0---PowerOff normally
 *            1---Poweron normally
 *            2---PowerOFF without PME
 *            3---Poweron without LINK
 */
int kirin_pcie_pm_control(int power_ops, u32 rc_idx)
{
	int ret = 0;
	struct kirin_pcie *pcie = NULL;

	PCIE_PR_INFO("RC = [%u], power_ops[%d]", rc_idx, power_ops);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -1;
	}

	mutex_lock(&(pcie->pm_lock));

	switch (power_ops) {
	case POWERON:
		dsm_pcie_clear_info();
		ret = kirin_pcie_usr_resume(rc_idx);
		break;

	case POWEROFF_BUSON:
	case POWEROFF_BUSDOWN:
		ret = kirin_pcie_usr_suspend(rc_idx, power_ops);
		break;

	case POWERON_CLK:
		ret = pcie->plat_ops->plat_on(pcie, RC_POWER_ON);
		break;

	default:
		PCIE_PR_ERR("Invalid power_ops[%d]", power_ops);
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&(pcie->pm_lock));
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_pm_control);

#ifndef CONFIG_PCIEASPM_POWER_SUPERSAVE
static void kirin_pcie_aspm_enable(struct kirin_pcie *pcie)
{
#ifdef CONFIG_KIRIN_PCIE_APR
	u32 reg_val;
	int ep_l1ss_pm;
#endif

	kirin_pcie_config_l0sl1(pcie->rc_id,
				(enum link_aspm_state)pcie->dtsinfo.aspm_state);

	if (IS_ENABLED(CONFIG_KIRIN_PCIE_MAR) &&
		pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_INFO("Bypass L1ss");
		return;
	}

#ifdef CONFIG_KIRIN_PCIE_APR
	if ((pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) &&
	    pcie->ep_dev) {
		ep_l1ss_pm = pci_find_ext_capability(pcie->ep_dev, PCI_EXT_L1SS_CAP_ID);
		if (!ep_l1ss_pm) {
			PCIE_PR_ERR("Failed to get EP PCI_EXT_L1SS_CAP_ID");
			return;
		}
		pci_read_config_dword(pcie->ep_dev, ep_l1ss_pm + PCI_EXT_L1SS_CAP, &reg_val);
		if ((reg_val & PCI_EXT_L1SS_SUP_ALL) != PCI_EXT_L1SS_SUP_ALL) {
			PCIE_PR_INFO("Bypass L1ss, EP not support 0x%x", reg_val);
			return;
		}
	}
#endif
	kirin_pcie_config_l1ss(pcie->rc_id, L1SS_PM_ASPM_ALL);
}
#endif

#define ASPM_STATE_L0S_L1	7

/*
 * API FOR EP to control L1&L1-substate
 * param: rc_idx---which rc the EP link with
 * enable: KIRIN_PCIE_LP_ON---enable L1 and L1-substate,
 *         KIRIN_PCIE_LP_Off---disable,
 *         others---illegal
 */
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable)
{
#ifdef CONFIG_PCIEASPM_POWER_SUPERSAVE
	u32 val;
#endif
	struct kirin_pcie *pcie = NULL;
	struct kirin_pcie_dtsinfo *dtsinfo = NULL;

	PCIE_PR_INFO("+%s+", enable == ENABLE ? "enable" : "disable");

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -EINVAL;
	}

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe%u is power off ", rc_idx);
		return -EINVAL;
	}

	dtsinfo = &(pcie->dtsinfo);

#ifndef CONFIG_PCIEASPM_POWER_SUPERSAVE
	if (enable) {
		if (pcie->dtsinfo.board_type == BOARD_ASIC)
			kirin_pcie_aspm_enable(pcie);
	} else {
		kirin_pcie_config_l1ss(pcie->rc_id, L1SS_CLOSE);
		kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_CLOSE);
	}
#else
	if (enable) {
		enable_req_clk(pcie, DISABLE);

		pci_enable_link_state(pcie->rc_dev, ASPM_STATE_L0S_L1);

		pcie_capability_read_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, &val);
		val |= PCI_EXT_CAP_LTR_EN;
		pcie_capability_write_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, val);

		pcie_capability_read_dword(pcie->ep_dev, PCI_EXP_DEVCTL2, &val);
		val |= PCI_EXT_CAP_LTR_EN;
		pcie_capability_write_dword(pcie->ep_dev, PCI_EXP_DEVCTL2, val);
	} else {
		pci_disable_link_state(pcie->rc_dev, ASPM_STATE_L0S_L1);
		enable_req_clk(pcie, ENABLE);
	}
#endif

	PCIE_PR_INFO("-%s-", enable == ENABLE ? "enable" : "disable");

	return 0;
}
EXPORT_SYMBOL_GPL(kirin_pcie_lp_ctrl);

static int kirin_pcie_reenumerate(u32 rc_idx)
{
	int ret;

	ret = kirin_pcie_pm_control(POWERON, rc_idx);
	if (!ret)
		ret = kirin_pcie_rescan_ep(rc_idx);

	return ret;
}

/*
 * Enumerate Function:
 * param: rc_idx---which rc the EP link with
 */
int kirin_pcie_enumerate(u32 rc_idx)
{
	int ret;
	u32 val, dev_id, vendor_id;
	struct pcie_port *pp = NULL;
	struct pci_dev *dev = NULL;
	struct kirin_pcie *pcie = NULL;

	PCIE_PR_INFO("+RC[%u]+", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];
	pp = &(pcie->pci->pp);

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -1;
	}

	if (atomic_read(&(pcie->is_enumerated))) {
		if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM)
			return kirin_pcie_reenumerate(rc_idx);

		PCIE_PR_ERR("Enumeration was done successed before");
		return 0;
	}

	/* clk on */
	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power RC");
		dsm_pcie_dump_info(pcie, DSM_ERR_POWER_ON);
		return ret;
	}

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, 0, 4);
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, 0, 4,
			     val + ((pcie->rc_id) << PCIE_DEV_ID_SHIFT));

	ret = dw_pcie_host_init(pp);
	if (ret) {
		PCIE_PR_ERR("Failed to initialize host");
		dsm_pcie_dump_info(pcie, DSM_ERR_ENUMERATE);
		goto FAIL_TO_POWEROFF;
	}

	kirin_pcie_rd_own_conf(pp, PCI_VENDOR_ID, 2, &vendor_id);
	kirin_pcie_rd_own_conf(pp, PCI_DEVICE_ID, 2, &dev_id);
	pcie->rc_dev = pci_get_device(vendor_id, dev_id, pcie->rc_dev);
	if (!pcie->rc_dev) {
		PCIE_PR_ERR("Failed to get RC device");
		goto FAIL_TO_POWEROFF;
	}

	ret = kirin_pcie_save_rc_cfg(pcie);
	if (ret)
		goto FAIL_TO_POWEROFF;

	if (pcie->rc_dev->subordinate) {
		list_for_each_entry(dev, &pcie->rc_dev->subordinate->devices, bus_list) {
			if (pci_is_pcie(dev)) {
				pcie->ep_dev = dev;
				atomic_set(&(pcie->is_enumerated), 1);
			} else {
				PCIE_PR_ERR("No PCIe EP found!");
				pci_stop_and_remove_bus_device(pcie->rc_dev);
				goto FAIL_TO_POWEROFF;
			}
		}
	} else {
		PCIE_PR_ERR("Bus1 is null");
		pcie->ep_dev = NULL;
		pci_stop_and_remove_bus_device(pcie->rc_dev);
		goto FAIL_TO_POWEROFF;
	}

	kirin_pcie_lp_ctrl(pcie->rc_id, ENABLE);

	atomic_set(&(pcie->usr_suspend), 0);

	PCIE_PR_INFO("-RC[%u]-", rc_idx);
	return 0;

FAIL_TO_POWEROFF:
	if (kirin_pcie_power_ctrl(pcie, RC_POWER_OFF))
		PCIE_PR_ERR("Failed to power off");

	return -1;
}
EXPORT_SYMBOL(kirin_pcie_enumerate);

/*
 * Remove EP Function:
 * param: rc_idx---which rc the EP link with
 */
int kirin_pcie_remove_ep(u32 rc_idx)
{
	struct kirin_pcie *pcie = NULL;
	struct pci_dev *dev = NULL;
	struct pci_dev *temp = NULL;

	PCIE_PR_INFO("+RC[%u]+", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -1;
	}

	if (!atomic_read(&(pcie->is_enumerated))) {
		PCIE_PR_ERR("Enumeration was not done");
		return -1;
	}

	if (atomic_read(&(pcie->is_removed))) {
		PCIE_PR_ERR("Remove was done before");
		return 0;
	}

	(void)kirin_pcie_lp_ctrl(pcie->rc_id, DISABLE);
	list_for_each_entry_safe(dev, temp, &pcie->rc_dev->subordinate->devices, bus_list) {
		if (pci_is_pcie(dev)) {
			pci_stop_and_remove_bus_device_locked(dev);
			if (pcie->ep_dev)
				pcie->ep_dev = NULL;
		} else {
			PCIE_PR_ERR("No PCIe EP found!");
			return -1;
		}
	}

	(void)atomic_inc_return(&(pcie->is_removed));

	PCIE_PR_INFO("-RC[%u]-", rc_idx);
	return 0;
}
EXPORT_SYMBOL(kirin_pcie_remove_ep);

/*
 * Rescan EP Function:
 * param: rc_idx---which rc the EP link with
 */
int kirin_pcie_rescan_ep(u32 rc_idx)
{
	struct kirin_pcie *pcie = NULL;
	struct pci_dev *dev = NULL;
	struct pci_dev *temp = NULL;
	u32 max;

	PCIE_PR_INFO("+RC[%u]+", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -1;
	}

	if (!atomic_read(&(pcie->is_enumerated))) {
		PCIE_PR_ERR("Enumeration was not done");
		return -1;
	}

	if (!atomic_read(&(pcie->is_removed))) {
		PCIE_PR_ERR("Rescan was done before or Remove was not done");
		return 0;
	}

	pci_lock_rescan_remove();
	max = pci_rescan_bus_bridge_resize(pcie->rc_dev);
	pci_unlock_rescan_remove();

	if (!max) {
		PCIE_PR_ERR("Bus1 is null");
		pcie->ep_dev = NULL;
		return -1;
	}

	if (kirin_pcie_save_rc_cfg(pcie))
		return -1;

	list_for_each_entry_safe(dev, temp, &pcie->rc_dev->subordinate->devices, bus_list) {
		if (pci_is_pcie(dev)) {
			if (!pcie->ep_dev)
				pcie->ep_dev = dev;
		} else {
			PCIE_PR_ERR("No PCIe EP found!");
			return -1;
		}
	}

	(void)kirin_pcie_lp_ctrl(pcie->rc_id, ENABLE);
	(void)atomic_dec_return(&(pcie->is_removed));

	PCIE_PR_INFO("-RC[%u]-", rc_idx);
	return 0;
}
EXPORT_SYMBOL(kirin_pcie_rescan_ep);

int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status)
{
	struct kirin_pcie *pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	if (link_status >= DEVICE_LINK_MAX || link_status <= DEVICE_LINK_MIN) {
		PCIE_PR_ERR("Invalid Device link status[%u]", link_status);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];

	if (!atomic_read(&pcie->is_power_on)) {
		PCIE_PR_ERR("PCIe is Poweroff");
		return -EINVAL;
	}

	pcie->ep_link_status = link_status;

	return 0;
}
EXPORT_SYMBOL(pcie_ep_link_ltssm_notify);

int kirin_pcie_power_notifiy_register(u32 rc_id, int (*poweron)(void *data),
				      int (*poweroff)(void *data), void *data)
{
	struct kirin_pcie *pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];
	pcie->callback_poweron  = poweron;
	pcie->callback_poweroff = poweroff;
	pcie->callback_data = data;
	return 0;
}
EXPORT_SYMBOL_GPL(kirin_pcie_power_notifiy_register);

void pcie_set_dbi_flag(void)
{
	if (IS_ENABLED(CONFIG_KIRIN_PCIE_TEST)) {
		PCIE_PR_ERR("set dbi_debug_flag 0x5a5aa5a5");
		dbi_debug_flag = PCIE_ENABLE_DBI_READ_FLAG;
	}
}
EXPORT_SYMBOL_GPL(pcie_set_dbi_flag);

#define PCIE_PROGRAM_INTERFACE 8
bool kirin_pcie_bypass_pm(struct pci_dev *dev)
{
	struct pci_dev *ep_dev = NULL;
	u32 class;

	if (!dev) {
		PCIE_PR_ERR("NULL Param");
		return false;
	}

	ep_dev = dev;
	if (pci_is_root_bus(dev->bus))
		ep_dev = list_first_entry(&dev->subordinate->devices,
					  struct pci_dev, bus_list);

	class = ep_dev->class >> PCIE_PROGRAM_INTERFACE;
	PCIE_PR_INFO("EP class version: 0x%x", class);

	return class == PCI_CLASS_COMMUNICATION_MODEM;
}
EXPORT_SYMBOL_GPL(kirin_pcie_bypass_pm);

builtin_platform_driver(kirin_pcie_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisilicon Kirin pcie driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
