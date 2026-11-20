/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: PCIe host controller driver.
 * Create: 2016-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#ifndef _PCIE_KPORT_H
#define _PCIE_KPORT_H

#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/pci.h>
#include <linux/of_pci.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <linux/msi.h>
#include <linux/of_address.h>
#include <linux/pci_regs.h>
#include <linux/regulator/consumer.h>
#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/hisi/pcie-kport-api.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/wait.h>
#include <linux/freezer.h>

#include "pcie-designware.h"
#include "pcie-kport-idle.h"
#include "pcie-kport-phy.h"

#define to_pcie_port(x)	dev_get_drvdata((x)->dev)

#define PCIE_LINK_UP_TIME	200
#define MAX_IRQ_NUM		5
#define IRQ_INTA		0
#define IRQ_INTB		1
#define IRQ_MSI			1
#define IRQ_INTC		2
#define IRQ_INTD		3
#define IRQ_LINKDOWN		4
#define IRQ_CPLTIMEOUT		5
#define EXTRA_IRQ_NUM		1

#define TYPE_POST		0
#define TYPE_NONPOST		1
#define TYPE_CPL			2

#define REF_CLK_FREQ		100000000
#define AXI_CLK_FREQ		207500000

#define TEST_BUS0_OFFSET	0x0
#define TEST_BUS1_OFFSET	0x1000000

/* PCIe ELBI registers */
#define SOC_PCIECTRL_CTRL0_ADDR		0x000
#define SOC_PCIECTRL_CTRL1_ADDR		0x004
#define SOC_PCIECTRL_CTRL2_ADDR		0x008
#define SOC_PCIECTRL_CTRL6_ADDR		0x018
#define SOC_PCIECTRL_CTRL7_ADDR		0x01c
#define SOC_PCIECTRL_CTRL8_ADDR		0x020
#define SOC_PCIECTRL_CTRL10_ADDR	0x028
#define SOC_PCIECTRL_CTRL11_ADDR	0x02c
#define SOC_PCIECTRL_CTRL12_ADDR	0x030
#define SOC_PCIECTRL_CTRL20_ADDR	0x050
#define SOC_PCIECTRL_CTRL21_ADDR	0x054
#define SOC_PCIECTRL_CTRL22_ADDR	0x058
#define SOC_PCIECTRL_CTRL25_ADDR	0x064
#define SOC_PCIECTRL_CTRL30_ADDR	0x078
#define SOC_PCIECTRL_STATE1_ADDR	0x404
#define SOC_PCIECTRL_STATE4_ADDR	0x410
#define SOC_PCIECTRL_STATE5_ADDR	0x414
#define SOC_PCIECTRL_STATE12_ADDR	0x430
#define SOC_PCIECTRL_STATE13_ADDR	0x434
#define SOC_PCIECTRL_STATE14_ADDR	0x438
#define SOC_PCIECTRL_STATE15_ADDR	0x43c

#define SOC_PCIEPHY_CTRL0_ADDR		0x000
#define SOC_PCIEPHY_CTRL1_ADDR		0x004
#define SOC_PCIEPHY_CTRL2_ADDR		0x008
#define SOC_PCIEPHY_CTRL3_ADDR		0x00c
#define SOC_PCIEPHY_CTRL33_ADDR		0x0084
#define SOC_PCIEPHY_CTRL34_ADDR		0x0088
#define SOC_PCIEPHY_CTRL38_ADDR		0x0098
#define SOC_PCIEPHY_CTRL39_ADDR		0x009C
#define SOC_PCIEPHY_CTRL40_ADDR		0x00A0
#define SOC_PCIEPHY_STATE0_ADDR		0x400
#define SOC_PCIEPHY_STATE34_ADDR	0x488
#define SOC_PCIEPHY_STATE39_ADDR	0x049c

#define PCIEPHY_RX_TERMINATION_BIT	(0x1 << 10)
#define PHY_TEST_POWERDOWN		(0x1 << 22)

#define PCIE_APB_CLK_REQ		(0x1 << 23)
#define PERST_FUN_SEC			0x2006
#define PERST_ASSERT_EN			0x1
#define AXI_TIMEOUT_MASK_BIT		(0x1 << 22)
#define AXI_TIMEOUT_CLR_BIT		(0x1 << 22)
#define ENTRY_L23_BIT			(0x1 << 2)
#define PCIE_ELBI_SLV_DBI_ENABLE	(0x1 << 21)
#define APP_DBI_RO_WR_DISABLE		(0x1 << 22)
#define PME_TURN_OFF_BIT		(0x1 << 8)
#define PME_ACK_BIT			(0x1 << 16)
#define PCIE_CLKREQ_OUT_MASK		(0x1 << 0)
#define PCIE_CLKREQ_OUT_HIGH		(0x1 << 0)
#define PCIE_DBI_EP_SPLT_BIT		(0x1 << 0)
#define PCIE_VEN_MSI_REQ		(0x1 << 26)

#define PCIEPHY_POR_N_RESET_BIT	(0x1 << 1)

/* SOC_PCIECTRL_CTRL12_ADDR */
#define PCIE_PERST_IN_N_CTRL_11B	(0x3 << 2)
#define PCIE_PERST_OE_EN		(0x1 << 1)
#define PCIE_PERST_OUT_N		(0x1 << 0)

#define PCI_ANY_ID			(~0)
#define PCIE_VENDOR_ID_HUAWEI		0x19e5
#define PCIE_DEV_ID_SHIFT		16
#define PCIE_INBOUND_OUTBOND_MAX	16
#define PCI_CLASS_CODE_SHIFT		16
#define PCIE_SHUTDOWN_TIMEOUT		1000

#define PCI_SHADOW_REG_BAR0		0x100010

#define RD_FLAG			0
#define WR_FLAG			1
#define IOMG_GPIO		0
#define IOMG_CLKREQ		1
#define BOARD_FPGA		0
#define BOARD_EMU		1
#define BOARD_ASIC		2
#define ENABLE			1
#define DISABLE			0

#define PCI_EXT_LTR_CAP_ID		0x18
#define LTR_MAX_SNOOP_LATENCY		0x04
#define PCI_EXT_L1SS_CAP_ID		0x1E
#define PCI_EXT_L1SS_CAP		0x04
#define PCI_EXT_L1SS_CTRL1		0x08
#define PCI_EXT_L1SS_CTRL2		0x0C
#define PCI_EXT_L1SS_SUP_ALL		0xF
#define PCI_EXT_CAP_LTR_EN		(0x1 << 10)

#define PCIE_APP_LTSSM_ENABLE		0x01c
#define PCIE_ELBI_RDLH_LINKUP		0x400
#define PCIE_LINKUP_ENABLE		0x8020
#define PCIE_LTSSM_ENABLE_BIT		(0x1 << 11)

#define PCIE_CREDIT_HEADER_OFFS		12
#define PCIE_CREDIT_HEADER_MASK		0xFF000
#define PCIE_CREDIT_DATA_MASK		0xFFF
/* SOC_PCIECTRL_STATE15_ADDR */
#define NO_PENDING_DLLP			(1 << 0)
#define NO_EXPECTING_ACK		(1 << 1)
#define HAD_ENOUGH_CREDIT		(1 << 2)
#define NO_PENDING_TLP			(1 << 3)
#define NO_FC				(1 << 4)

/* designware register */
#define ATU_VIEWPORT			0x900
#define ATU_REGION_INBOUND		(0x1u << 31)
#define ATU_REGION_OUTBOUND		(0x0u << 31)
#define ATU_REGION_INDEX0		(0x0 << 0)
#define ATU_CR1			0x4
#define ATU_CR2			0x8
#define ATU_LOWER_BASE		0xC
#define ATU_UPPER_BASE		0x10
#define ATU_LIMIT			0x14
#define ATU_LOWER_TARGET		0x18
#define ATU_UPPER_TARGET		0x1C
#define ATU_TYPE_MEM			(0x0 << 0)
#define ATU_TYPE_CFG0		(0x4 << 0)
#define ATU_TYPE_MSG			0x14
#define ATU_ENABLE			(0x1u << 31)

#define INHIBIT_PAYLOAD			(0x1 << 22)

/* port logic register */
#define PROT_FORCE_LINK_REG		0x708
#define PORT_LINK_CTRL_REG		0x710
#define PORT_MSI_CTRL_ADDR		0x820
#define PORT_MSI_CTRL_UPPER_ADDR	0x824
#define PORT_MSI_CTRL_INT0_ENABLE	0x828
#define PORT_GEN3_CTRL_REG		0x890
#define PORT_PIPE_LOOPBACK_REG		0x8B8

#define MSG_CODE_ASSERT_INTA		0x20
#define MSG_CODE_ASSERT_INTB		0x21
#define MSG_CODE_DEASSERT_INTA		0x24
#define MSG_CODE_DEASSERT_INTB		0x25
#define MSG_CODE_PME_TURN_OFF		25
#define MSG_TYPE_ROUTE_BROADCAST	0x13
#define MSG_CPU_ADDR_SIZE		0x100
#define MSG_CPU_ADDR_TEM		0x900

#define LTSSM_LINK_DOWN			0xFFFFFFFF
#define ADDR_OFFSET_4BYTE		0x4
#define REG_BYTE_ALIGN			0x1
#define REG_WORD_ALIGN			0x2
#define REG_DWORD_ALIGN			0x4
#define OF_DRIVER_PARAM_NUMS		0x3

/* PCIe capability register */
#define ENTER_COMPLIANCE		(0x1 << 4)
#define PCIE_TYPE_MASK			0xFu
#define PCIE_TYPE_SHIFT			28
#define PCIE_TYPE_RC			4
#define PERST_IN_EP			(0x3 << 2)
#define PERST_IN_RC			(0x1 << 2)

#define PCIE_PHY_SRAM_SIZE		0x4000
#define SUP_DIG_LVL_OVRD_IN		0x21
#define SUP_DIG_LVL_MASK		0xFFFF
#define SUP_DIG_LVL_VAL			0xB5
#define PCIE_PHY_CTRL150		0x258
#define CDR_LEGACY_ENABLE		0x1
#define SHUTDOWN_PREPARE_DELAY_10US	10

#define CHIP_TYPE_ES	1
#define CHIP_TYPE_CS	2
#define CHIP_TYPE_CS2	3

struct pcie_irq_info {
	char *name;
	int num;
};

struct pcie_dtsinfo {
	u32 board_type;
	u32 chip_type;
	u32 eco;
	int ep_flag;
	u32 ep_ltr_latency;
	u32 ep_l1ss_ctrl2;
	u32 l1ss_ctrl1;
	u32 aspm_state;
	u32 iso_info[2];
	u32 assert_info[2];
	u32 iatu_base_offset;
	u32 t_ref2perst[2];
	u32 t_perst2access[2];
	u32 t_perst2rst[2];
	u32 eye_param_nums;
	u32 *eye_param_data;
	u32 io_driver[OF_DRIVER_PARAM_NUMS];
	u32 noc_target_id;
	u32 noc_mntn;
	u32 ep_device_type;
	bool sup_s3;
	bool sup_iommus;
};

struct pcie_kport {
	void __iomem *apb_base;
	void __iomem *phy_base;
	void __iomem *crg_base;
	void __iomem *sctrl_base;
	void __iomem *pmctrl_base;
	void __iomem *tbu_base;
	void __iomem *pcie_pll_base;
	void __iomem *hsdtsctrl_base;
	u32 natural_phy_offset;
	u32 apb_phy_offset;
	u32 sram_phy_offset;
	struct clk *apb_sys_clk;
	struct clk *apb_phy_clk;
	struct clk *phy_ref_clk;
	struct clk *pcie_aclk;
	struct clk *pcie_aux_clk;
	struct clk *pcie_tcu_clk;
	struct clk *pcie_tbu_clk;
	struct clk *pcie1_serdes_clk;
	struct clk *pcie_refclk;
	int gpio_id_reset;
	struct dw_pcie *pci;
	struct  pci_dev *rc_dev;
	struct  pci_dev *ep_dev;
	atomic_t usr_suspend;
	atomic_t is_ready;  /* driver is ready */
	atomic_t is_power_on;
	atomic_t is_enumerated;
	atomic_t is_removed;
	struct mutex pm_lock;
	spinlock_t ep_ltssm_lock;
	struct mutex power_lock;
	struct pci_saved_state *rc_saved_state;
	struct work_struct handle_work;
	struct work_struct handle_cpltimeout_work;
	struct work_struct phy_irq_work;
	struct pcie_kport_register_event *event_reg;
	struct pcie_irq_info irq[MAX_IRQ_NUM + EXTRA_IRQ_NUM];
	u32 msi_controller_config[3];
	u32 aer_config;
	u32 rc_id;
	struct pcie_dtsinfo dtsinfo;
	u32 ep_link_status;

	struct pcie_idle_sleep *idle_sleep;

	/* for EP callbacks */
	int (*callback_poweron)(void *data);
	int (*callback_poweroff)(void *data);
	void *callback_data;

	/* Platform Ops */
	struct pcie_platform_ops *plat_ops;

	/* pcie phy ops */
	struct pcie_phy *phy;
	struct pcie_phy_ops *phy_ops;
};

enum rc_power_status {
	RC_POWER_OFF = 0,
	RC_POWER_ON = 1,
	RC_POWER_SUSPEND = 2,
	RC_POWER_RESUME = 3,
	RC_POWER_INVALID = 4,
};

struct pcie_platform_ops {
	int (*sram_ext_load)(void *data);
	int (*plat_on)(struct pcie_kport *pcie, enum rc_power_status on_flag);
	int (*plat_off)(struct pcie_kport *pcie, enum rc_power_status off_flag);
#ifdef CONFIG_PCIE_KPORT_IDLE
	void (*ref_clk_on)(struct pcie_kport *pcie);
	void (*ref_clk_off)(struct pcie_kport *pcie);
	void (*pll_status)(struct pcie_kport *pcie);
#endif
};

enum link_speed {
	GEN1 = 0,
	GEN2 = 1,
	GEN3 = 2,
};

enum link_aspm_state {
	ASPM_CLOSE = 0,         /* disable aspm L0s L1 */
	ASPM_L0S = 1,           /* enable l0s */
	ASPM_L1 = 2,            /* enable l1 */
	ASPM_L0S_L1 = 3,        /* enable l0s & l1 */
};

enum l1ss_ctrl_state {
	L1SS_CLOSE = 0x0,	 /* disable l1ss */
	L1SS_PM_1_2 = 0x1,	 /* pci-pm L1.2 */
	L1SS_PM_1_1 = 0x2,	 /* pci-pm L1.1 */
	L1SS_PM_ALL = 0x3,	 /* pci-pm L1.2 & L1.1 */
	L1SS_ASPM_1_2 = 0x4,	 /* aspm L1.2 */
	L1SS_ASPM_1_1 = 0x8,	 /* aspm L1.1 */
	L1SS_ASPM_ALL = 0xC,	 /* aspm L1.2 & L1.1 */
	L1SS_PM_ASPM_ALL = 0xF,	 /* aspm l1ss & pci-pm l1ss */
};

enum {
	LTSSM_PWROFF = 0x0,
	LTSSM_CPLC = 0x3,
	LTSSM_L0 = 0x11,
	LTSSM_L0S = 0x12,
	LTSSM_L1 = 0x14,
	LTSSM_LPBK = 0x1B,
	LTSSM_L1_1 = 0x4000,
	LTSSM_L1_2 = 0xC000,
	LTSSM_OTHERS = 0xFFFF,
};

enum dsm_err_id {
	DSM_ERR_POWER_ON = 1,
	DSM_ERR_ESTABLISH_LINK,
	DSM_ERR_ENUMERATE,
	DSM_ERR_LINK_DOWN,
	DSM_ERR_CPL_TIMEOUT,
};

enum RST_TYPE {
	RST_ENABLE = 1,
	RST_DISABLE = 2,
};

enum {
	DEVICE_LINK_MIN = 0,
	DEVICE_LINK_UP = 1,
	DEVICE_LINK_ABNORMAL = 2,
	DEVICE_LINK_MAX = 3,
};

enum {
	POWEROFF_BUSON = 0x0,
	POWERON = 0x1,
	POWEROFF_BUSDOWN = 0x2,
	POWERON_CLK = 0x3,
	POWER_MAX = 0x4,
};

enum {
	EP_DEVICE_NODEV = 0,
	EP_DEVICE_BCM = 1,
	EP_DEVICE_HI110X = 2,
	EP_DEVICE_NVME = 3,
	EP_DEVICE_MODEM = 4,
	EP_DEVICE_APR = 5,
	EP_DEVICE_FPGA = 6,
};

enum {
	PLL_TYPE_FN = 1,
	PLL_TYPE_HP = 2,
};

enum {
	REFCLK_FROM_PHY = 1,
	REFCLK_FROM_PLL = 2,
};

enum {
	PCIE_RC0 = 0x0,
	PCIE_RC1 = 0x1,
};

enum pcie_clk_type {
	PCIE_INTERNAL_CLK = 1,
	PCIE_EXTERNAL_CLK = 2,
};

#define PCIE_PR_E(fmt, args ...)	pr_err("[pcie_kport][ERR]:" fmt "\n", ##args)

#define PCIE_PR_I(fmt, args ...)	pr_info("[pcie_kport][INF]:" fmt "\n", ##args)

#define PCIE_PR_D(fmt, args ...)	pr_debug("[pcie_kport][DBG]:" fmt "\n", ##args)

extern struct pcie_kport g_pcie_port[];
extern unsigned int g_rc_num;

void pcie_apb_ctrl_writel(struct pcie_kport *pcie, u32 val, u32 reg);
u32 pcie_apb_ctrl_readl(struct pcie_kport *pcie, u32 reg);

/* Registers in PCIePHY */
void pcie_apb_phy_writel(struct pcie_kport *pcie, u32 val, u32 reg);
u32 pcie_apb_phy_readl(struct pcie_kport *pcie, u32 reg);

void pcie_natural_phy_writel(struct pcie_kport *pcie, u32 val, u32 reg);
u32 pcie_natural_phy_readl(struct pcie_kport *pcie, u32 reg);

void pcie_ram_phy_writel(struct pcie_kport *pcie, u32 val, u32 reg);
u32 pcie_ram_phy_readl(struct pcie_kport *pcie, u32 reg);
u32 pcie_read_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			size_t size);
void pcie_write_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			  size_t size, u32 val);
int pcie_rd_own_conf(struct pcie_port *pp, int where, int size, u32 *val);
int pcie_wr_own_conf(struct pcie_port *pp, int where, int size, u32 val);
int pcie_power_ctrl(struct pcie_kport *pcie, enum rc_power_status on_flag);
bool is_pipe_clk_stable(struct pcie_kport *pcie);
void pcie_iso_ctrl(struct pcie_kport *pcie, int en_flag);
void dsm_pcie_dump_info(struct pcie_kport *pcie, enum dsm_err_id id);
void dsm_pcie_clear_info(void);
void dump_apb_register(struct pcie_kport *pcie);
void pcie_reset_ctrl(struct pcie_kport *pcie, enum RST_TYPE rst);
int pcie_perst_cfg(struct pcie_kport *pcie, int pull_up);
int pcie_plat_init(struct platform_device *pdev, struct pcie_kport *pcie);
int32_t pcie_get_dtsinfo(struct pcie_kport *pcie, struct platform_device *pdev);
int pcie_get_port(struct pcie_kport **pcie, struct platform_device *pdev);
void set_phy_eye_param(struct pcie_kport *pcie);
struct pcie_kport *get_pcie_by_id(uint32_t rc_id);
int pcie_check_rcid(uint32_t rc_id);

static inline bool is_pcie_pwr_on(uint32_t rc_id)
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	return pcie ? !!atomic_read(&(pcie->is_power_on)) : false;
}

#ifdef CONFIG_PCIE_KPORT_PHY
static inline void pcie_phy_irq_init(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->irq_init)
		pcie->phy_ops->irq_init(pcie->phy);
}

static inline void pcie_phy_irq_deinit(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->irq_disable)
		pcie->phy_ops->irq_disable(pcie->phy);
}

static inline void pcie_phy_irq_handle(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->irq_handler)
		pcie->phy_ops->irq_handler(pcie->phy);
}

static inline bool pcie_phy_ready(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->is_phy_ready)
		return pcie->phy_ops->is_phy_ready(pcie->phy);

	return true;
}

static inline void pcie_phy_init(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->phy_init)
		pcie->phy_ops->phy_init(pcie->phy);
}

static inline void pcie_phy_state(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->phy_stat_dump)
		pcie->phy_ops->phy_stat_dump(pcie->phy);
}

static inline int32_t pcie_phy_core_start(struct pcie_kport *pcie)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->phy_core_start)
		return pcie->phy_ops->phy_core_start(pcie->phy);

	return 0;
}

static inline int32_t pcie_phy_core_clk_cfg(struct pcie_kport *pcie, int32_t en)
{
	if (pcie->phy && pcie->phy_ops && pcie->phy_ops->phy_core_clk_config)
		return pcie->phy_ops->phy_core_clk_config(pcie->phy, !!en);

	return 0;
}
#else
static inline void pcie_phy_irq_init(struct pcie_kport *pcie) {}

static inline void pcie_phy_irq_deinit(struct pcie_kport *pcie) {}

static inline void pcie_phy_irq_handle(struct pcie_kport *pcie) {}

static inline bool pcie_phy_ready(struct pcie_kport *pcie)
{
	return true;
}

static inline void pcie_phy_init(struct pcie_kport *pcie) {}

static inline void pcie_phy_state(struct pcie_kport *pcie) {}

static inline int32_t pcie_phy_core_start(struct pcie_kport *pcie)
{
	return 0;
}

static inline int32_t pcie_phy_core_clk_cfg(struct pcie_kport *pcie, int32_t en)
{
	return 0;
}

#endif /* CONFIG_PCIE_KPORT_PHY */

#endif
