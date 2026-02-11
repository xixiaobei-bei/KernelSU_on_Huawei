#include "pcie-kirin.h"
#include "pcie-kirin-common.h"
#include "pcie_phy_firmware_apr.h"

/* PCIE CTRL Register bit definition */
#define PCIE_PERST_CONFIG_MASK			0x4

/* PMC register offset */
#define NOC_POWER_IDLEREQ			0x380
#define NOC_POWER_IDLE				0x388

/* PMC register bit definition */
#define NOC_PCIE0_POWER				(0x1 << 10)
#define NOC_PCIE1_POWER				(0x1 << 11)
#define NOC_PCIE0_POWER_MASK			(0x1 << 26)
#define NOC_PCIE1_POWER_MASK			(0x1 << 27)

/* HSDT register offset */
#define HSDTCRG_PEREN1				0x10
#define HSDTCRG_PERDIS1				0x14
#define PCIEPLL_STATE				0x208
#define PLL_CFG0				0x224
#define PLL_CFG1				0x228
#define PLL_CFG2				0x22C
#define PLL_CFG3				0x230
#define PLL_CFG4				0x234
#define PLL_CFG5				0x238
#define PLL_CFG6				0x23C
#define PLL_CFG7				0x240
#define HSDTCRG_PCIECTRL0			0x300
#define HSDTCRG_PCIECTRL1			0x304

/* HSDT register bit definition */
#define PCIE0_CLK_HP_GATE			(0x1 << 1)
#define PCIE0_CLK_DEBUNCE_GATE			(0x1 << 2)
#define PCIE0_CLK_PHYREF_GATE			(0x1 << 6)
#define PCIE0_CLK_IO_GATE			(0x1 << 7)
#define PCIE1_CLK_HP_GATE			(0x1 << 13)
#define PCIE1_CLK_DEBUNCE_GATE			(0x1 << 14)
#define PCIE1_CLK_PHYREF_GATE			(0x1 << 18)
#define PCIE1_CLK_IO_GATE			(0x1 << 19)
#define PCIEIO_HW_BYPASS			(0x1 << 0)
#define PCIEPHY_REF_HW_BYPASS			(0x1 << 1)
#define PCIEIO_OE_EN_SOFT			(0x1 << 6)
#define PCIEIO_OE_POLAR				(0x1 << 9)
#define PCIEIO_OE_EN_HARD_BYPASS		(0x1 << 11)
#define PCIEIO_IE_EN_HARD_BYPASS		(0x1 << 27)
#define PCIEIO_IE_EN_SOFT			(0x1 << 28)
#define PCIEIO_IE_POLAR				(0x1 << 29)
#define MPLLA_FORCE_EN_MUX_SEL			(0x1 << 4)
#define PCIE_MPLLA_FORCE_EN_SEL			(0x1 << 30)

/* APB PHY register definition */
#define PHY_REF_USE_PAD				(0x1 << 8)
#define PHY_REF_USE_CIO_PAD			(0x1 << 14)

/* pll Bit */
#define FNPLL_EN_BIT				(0x1 << 0)
#define FNPLL_BP_BIT				(0x1 << 1)
#define FNPLL_LOCK_BIT				(0x1 << 4)

#define IO_CLK_SEL_CLEAR			(0x3 << 17)
#define IO_CLK_FROM_CIO				(0x0 << 17)
#define IO_CLK_FROM_PHY_MPLLA			(0x1 << 17)

#define NOC_TIMEOUT_VAL				1000
#define FNPLL_LOCK_TIMEOUT			200

#define GEN3_RELATED_OFF			0x890
#define GEN3_ZRXDC_NONCOMPL			(1 << 0)

atomic_t pll_init_cnt = ATOMIC_INIT(0);

static u32 hsdt_crg_reg_read(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->crg_base + reg);
}

static void hsdt_crg_reg_write(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->crg_base + reg);
}

/*
 * exit or enter noc power idle
 * exit: 1, exit noc power idle
 *	0, enter noc power idle
 */
static int pcie_noc_power(struct kirin_pcie *pcie, u32 exit)
{
	u32 mask_bits, val_bits, val;
	u32 time = NOC_TIMEOUT_VAL;

	if (pcie->rc_id == 0) {
		val_bits = NOC_PCIE0_POWER;
		mask_bits = NOC_PCIE0_POWER_MASK;
	} else {
		val_bits = NOC_PCIE1_POWER;
		mask_bits = NOC_PCIE1_POWER_MASK;
	}

	/*
	 * bits in mask_bits set to write the bit
	 * if bit in val_bits is 0, exit noc power idle
	 * or enter noc power idle
	 */
	if (exit) {
		writel(mask_bits, pcie->pmctrl_base + NOC_POWER_IDLEREQ);
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		while ((val & val_bits)) {
			udelay(1);
			if (!time) {
				PCIE_PR_ERR("Exit failed :%d", val);
				return -1;
			}
			time--;
			val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		}
	} else {
		writel(mask_bits | val_bits, pcie->pmctrl_base + NOC_POWER_IDLEREQ);
		val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		while ((val & val_bits) != val_bits) {
			udelay(1);
			if (!time) {
				PCIE_PR_ERR("Enter failed :%d", val);
				return -1;
			}
			time--;
			val = readl(pcie->pmctrl_base + NOC_POWER_IDLE);
		}
	}

	return 0;
}

/* Enable fnpll */
static int pcie_pll_init(struct kirin_pcie *pcie)
{
	u32 val;
	u32 i = 0;

	val = hsdt_crg_reg_read(pcie, PLL_CFG6);

	/* en clear */
	val &= ~FNPLL_EN_BIT;
	/* Bypass set */
	val |= FNPLL_BP_BIT;
	/* FNPLL Disable and Bypass take effect */
	hsdt_crg_reg_write(pcie, val, PLL_CFG6);

	/* fnpll init */
	if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES) {
		hsdt_crg_reg_write(pcie, 0x00000000, PLL_CFG0);
		hsdt_crg_reg_write(pcie, 0x00B50000, PLL_CFG1);
		hsdt_crg_reg_write(pcie, 0x20101fa0, PLL_CFG2);
		hsdt_crg_reg_write(pcie, 0x2404FF20, PLL_CFG3);
		hsdt_crg_reg_write(pcie, 0x0034013F, PLL_CFG4);
		hsdt_crg_reg_write(pcie, 0x00000046, PLL_CFG5);
	} else {
		hsdt_crg_reg_write(pcie, 0x00042000, PLL_CFG0);
		hsdt_crg_reg_write(pcie, 0x000000B5, PLL_CFG1);
		hsdt_crg_reg_write(pcie, 0x1FA02010, PLL_CFG2);
		hsdt_crg_reg_write(pcie, 0xFF202404, PLL_CFG3);
		hsdt_crg_reg_write(pcie, 0x013F0004, PLL_CFG4);
	}

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES) {
		/* 3.0G-100M */
		/* Clear to 0 */
		val = 0;
		/* fnpll_pll_mode:bit[30]=0 */
		val |= (0x0 << 30);
		/* fnpll_dll_en:bit[29]=1 */
		val |= (0x1 << 29);
		/* fnpll_fout4phasepd:bit[28]=1 */
		val |= (0x1 << 28);
		/* fnpll_fout2xpd:bit[27]=0 */
		val |= (0x0 << 27);
		/* fnpll_foutpostdivpd:bit[26]=0 */
		val |= (0x0 << 26);
		/* fnpll_postdiv2:bit[25:23]=0x5 */
		val |= (0x5 << 23);
		/* fnpll_postdiv1:bit[22:20]=0x6 */
		val |= (0x6 << 20);
		/* fnpll_fbdiv:bit[19:8]=0x4e */
		val |= (0x4e << 8);
		/* fnpll_rfdiv:bit[7:2]=0x1 */
		val |= (0x1 << 2);
		/* fnpll_bypass:bit[1]=1 */
		val |= (0x1 << 1);
		/* fnpll_en:bit[0]=0 */
		val |= (0x0 << 0);
		hsdt_crg_reg_write(pcie, val, PLL_CFG6);

		/* Frac:bit[23:0]=0x200000 */
		val = hsdt_crg_reg_read(pcie, PLL_CFG7);
		val &= ~(0xFFFFFF);
		val |= 0x200000;
		hsdt_crg_reg_write(pcie, val, PLL_CFG7);
	} else {
		/* 2.4G-100M */
		/* Clear to 0 */
		val = 0;
		/* fnpll_pll_mode:bit[30]=0 */
		val |= (0x0 << 30);
		/* fnpll_dll_en:bit[29]=1 */
		val |= (0x1 << 29);
		/* fnpll_fout4phasepd:bit[28]=1 */
		val |= (0x1 << 28);
		/* fnpll_fout2xpd:bit[27]=1 */
		val |= (0x1 << 27);
		/* fnpll_foutpostdivpd:bit[26]=0 */
		val |= (0x0 << 26);
		/* fnpll_postdiv2:bit[25:23]=0x4 */
		val |= (0x4 << 23);
		/* fnpll_postdiv1:bit[22:20]=0x6 */
		val |= (0x6 << 20);
		/* fnpll_fbdiv:bit[19:8]=0x3e */
		val |= (0x3e << 8);
		/* fnpll_rfdiv:bit[7:2]=0x1 */
		val |= (0x1 << 2);
		/* fnpll_bypass:bit[1]=1 */
		val |= (0x1 << 1);
		/* fnpll_en:bit[0]=0 */
		val |= (0x0 << 0);
		hsdt_crg_reg_write(pcie, val, PLL_CFG6);

		/* Frac:bit[23:0]=0x800000 */
		val = hsdt_crg_reg_read(pcie, PLL_CFG7);
		val &= ~(0xFFFFFF);
		val |= 0x800000;
		hsdt_crg_reg_write(pcie, val, PLL_CFG7);
	}

	udelay(5);

	val = hsdt_crg_reg_read(pcie, PLL_CFG6);
	/* Enable Set */
	val |= FNPLL_EN_BIT;
	hsdt_crg_reg_write(pcie, val, PLL_CFG6);

	val = hsdt_crg_reg_read(pcie, PCIEPLL_STATE);
	while (i < FNPLL_LOCK_TIMEOUT) {
		if (val & FNPLL_LOCK_BIT) {
			val = hsdt_crg_reg_read(pcie, PLL_CFG6);
			/* clear bypass */
			val &= ~FNPLL_BP_BIT;
			hsdt_crg_reg_write(pcie, val, PLL_CFG6);
			PCIE_PR_INFO("FNPLL lock in %d us", i);
			return 0;
		}
		udelay(1);
		i++;
		val = hsdt_crg_reg_read(pcie, PCIEPLL_STATE);
	}

	PCIE_PR_ERR("FNPLL unlock(%d us)", FNPLL_LOCK_TIMEOUT);
	return -1;
}

/* Disable FNPLL */
static void pcie_pll_shutdown(struct kirin_pcie *pcie)
{
	u32 val;

	if (atomic_sub_return(1, &pll_init_cnt) == 0) {
		val = hsdt_crg_reg_read(pcie, PLL_CFG6);
		/* en clear */
		val &= ~FNPLL_EN_BIT;
		hsdt_crg_reg_write(pcie, val, PLL_CFG6);

		/* Bypass set */
		val |= FNPLL_BP_BIT;
		hsdt_crg_reg_write(pcie, val, PLL_CFG6);
	}
}

/* change clkreq_n signal to low */
static void pcie_config_clkreq_low(struct kirin_pcie *pcie)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL22_ADDR);
	val &= ~PCIE_CLKREQ_OUT_MASK;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL22_ADDR);
}

/*
 * enable:ENABLE--enable clkreq control phyio clk
 *	DISABLE--disable clkreq control phyio clk
 */
static void pcie_phyio_hard_bypass(struct kirin_pcie *pcie, bool enable)
{
	u32 val, reg_addr;

	if (pcie->rc_id == 0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	if (enable)
		val &= ~PCIEIO_HW_BYPASS;
	else
		val |= PCIEIO_HW_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * enable:ENABLE--enable clkreq control phyref clk
 *	DISABLE--disable clkreq control phyref clk
 */
static void pcie_phyref_hard_bypass(struct kirin_pcie *pcie, bool enable)
{
	u32 val, reg_addr;

	if (pcie->rc_id == 0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	if (enable)
		val &= ~PCIEPHY_REF_HW_BYPASS;
	else
		val |= PCIEPHY_REF_HW_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}


/*
 * Config gt_clk_pciephy_ref_inuse
 * enable: ENABLE--controlled by ~pcie_clkreq_in
 *         FALSE--clock down
 */
static void pcie_phy_ref_clk_gt(struct kirin_pcie *pcie, u32 enable)
{
	u32  mask;

	/* HW bypass cfg*/
	pcie_phyref_hard_bypass(pcie, enable);

	/* soft ref cfg */
	if (pcie->rc_id == 0)
		mask = PCIE0_CLK_PHYREF_GATE;
	else
		mask = PCIE1_CLK_PHYREF_GATE;

	/* Always disable SW control */
	hsdt_crg_reg_write(pcie, mask, HSDTCRG_PERDIS1);
}

/*
 * enable: ENABLE--control by pcieio_oe_mux
 *	 DISABLE--close
 */
static void pcie_oe_config(struct kirin_pcie *pcie, bool enable)
{
	/* HW bypass */
	u32 val, reg_addr;

	if (pcie->rc_id == 0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	val &= ~PCIEIO_OE_POLAR;
	val &= ~PCIEIO_OE_EN_SOFT;
	if (enable)
		val &= ~PCIEIO_OE_EN_HARD_BYPASS;
	else
		val |= PCIEIO_OE_EN_HARD_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/*
 * enable: ENABLE--control by pcie_clkreq_in_n
 *	  DISABLE--close
 */
static void pcie_ie_config(struct kirin_pcie *pcie, bool enable)
{
	/* HW bypass */
	u32 val, reg_addr;

	if (pcie->rc_id == 0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	val = hsdt_crg_reg_read(pcie, reg_addr);
	val &= ~PCIEIO_IE_POLAR;
	val &= ~PCIEIO_IE_EN_SOFT;
	if (enable)
		val &= ~PCIEIO_IE_EN_HARD_BYPASS;
	else
		val |= PCIEIO_IE_EN_HARD_BYPASS;
	hsdt_crg_reg_write(pcie, val, reg_addr);
}

/* ioref clock from fnpll */
static void pcie_ioclk_from_pll(struct kirin_pcie *pcie, u32 enable)
{
	u32 val, mask;

	/* selcet cio */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	val &= ~IO_CLK_SEL_CLEAR;
	val |= IO_CLK_FROM_CIO;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL21_ADDR);

	/*
	 * HW bypass: DISABLE:HW don't control
	 *            ENABLE:clkreq_n is one of controller
	 */
	pcie_phyio_hard_bypass(pcie, ENABLE);

	/* disable SW control */
	if (pcie->rc_id == 0)
		mask = PCIE0_CLK_IO_GATE;
	else
		mask = PCIE1_CLK_IO_GATE;
	hsdt_crg_reg_write(pcie, mask, HSDTCRG_PERDIS1);

	/* enable/disable ie/oe according mode */
	if (unlikely(pcie->dtsinfo.ep_flag)) {
		pcie_oe_config(pcie, DISABLE);
		pcie_ie_config(pcie, enable);
	} else {
		pcie_oe_config(pcie, enable);
		pcie_ie_config(pcie, DISABLE);
	}
}

/* phyio clk configuration */
static void kirin_pcie_ioref_gt(struct kirin_pcie *pcie, bool enable)
{
	PCIE_PR_INFO("CIO Use FNPLL");
	pcie_ioclk_from_pll(pcie, enable);
}

/* enable/disable hp clk */
static void pcie_hp_clk_gt(struct kirin_pcie *pcie, u32 enable)
{
	u32 val, mask, reg;

	if (pcie->rc_id == 0)
		mask = PCIE0_CLK_HP_GATE;
	else
		mask = PCIE1_CLK_HP_GATE;

	if (enable)
		reg = HSDTCRG_PEREN1;
	else
		reg = HSDTCRG_PERDIS1;

	val = hsdt_crg_reg_read(pcie, reg);
	val |= mask;
	hsdt_crg_reg_write(pcie, val, reg);
}

/* enable/disable debounce clk*/
static void pcie_debounce_clk_gt(struct kirin_pcie *pcie, u32 enable)
{
	u32 val, mask, reg;

	if (pcie->rc_id == 0)
		mask = PCIE0_CLK_DEBUNCE_GATE;
	else
		mask = PCIE1_CLK_DEBUNCE_GATE;

	if (enable)
		reg = HSDTCRG_PEREN1;
	else
		reg = HSDTCRG_PERDIS1;

	val = hsdt_crg_reg_read(pcie, reg);
	val |= mask;
	hsdt_crg_reg_write(pcie, val, reg);
}

/*
 * For RC, select FNPLL
 * For EP, select CIO
 * Select FNPLL
 */
static int kirin_pcie_refclk_on(struct kirin_pcie *pcie)
{
	u32 val;
	int ret = 0;

	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val &= ~PHY_REF_USE_PAD;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	if (pcie->dtsinfo.ep_flag)
		val |= PHY_REF_USE_CIO_PAD;
	else
		val &= ~PHY_REF_USE_CIO_PAD;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	if (atomic_add_return(1, &pll_init_cnt) == 1) {
		ret = pcie_pll_init(pcie);
		if (ret)
			return ret;
	}

	/* enable pcie hp clk */
	pcie_hp_clk_gt(pcie, ENABLE);
	/* enable pcie debounce clk */
	pcie_debounce_clk_gt(pcie, ENABLE);
	/* gate pciephy clk */
	pcie_phy_ref_clk_gt(pcie, ENABLE);
	/* gate pcieio clk */
	kirin_pcie_ioref_gt(pcie, ENABLE);

	return 0;
}

/* turn off refclk */
static void kirin_pcie_refclk_off(struct kirin_pcie *pcie)
{
	pcie_hp_clk_gt(pcie, DISABLE);
	pcie_debounce_clk_gt(pcie, DISABLE);
	kirin_pcie_ioref_gt(pcie, DISABLE);
	pcie_phy_ref_clk_gt(pcie, DISABLE);

	pcie_pll_shutdown(pcie);
}


static void kirin_pcie_fixup(struct kirin_pcie *pcie)
{
	u32 val;
	/* fix l1ss exit issue */
	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, GEN3_RELATED_OFF, 0x4);
	val &= ~GEN3_ZRXDC_NONCOMPL;
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, GEN3_RELATED_OFF, 0x4, val);
}

/* Turn on pcie */
static int kirin_pcie_turn_on(struct kirin_pcie *pcie,
		enum rc_power_status on_flag)
{
	int ret = 0;

	PCIE_PR_INFO("+ON+");

	mutex_lock(&pcie->power_lock);

	if (atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_INFO("PCIe%d already power on", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}

	/* pull down phy ISO */
	kirin_pcie_iso_ctrl(pcie, DISABLE);

	/* rst pcie_phy_apb_presetn pcie_ctrl_apb_presetn pcie_ctrl_por_n */
	kirin_pcie_reset_ctrl(pcie, RST_ENABLE);

	/* pclk for phy */
	ret = clk_prepare_enable(pcie->apb_phy_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable apb_phy_clk");
		goto APB_PHY_CLK;
	}

	/* pclk for ctrl */
	ret = clk_prepare_enable(pcie->apb_sys_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable apb_sys_clk");
		goto APB_SYS_CLK;
	}

	/* unset module */
	kirin_pcie_reset_ctrl(pcie, RST_DISABLE);

	/* adjust output refclk amplitude, currently no adjust */
	pcie_io_adjust(pcie);

	/* set clkreq low */
	pcie_config_clkreq_low(pcie);
	PCIE_PR_INFO("set clkreq low Done");

	/* sys_aux_pwr_det, perst */
	kirin_pcie_natural_cfg(pcie);

	ret = kirin_pcie_refclk_on(pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to enable 100MHz ref_clk");
		goto REF_CLK;
	}
	PCIE_PR_INFO("100MHz refclks enable Done");

	/* Enable pcie axi clk */
	ret = clk_prepare_enable(pcie->pcie_aclk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable axi_aclk");
		goto AXI_ACLK;
	}

	/* enable pcie aux clk */
	ret = clk_prepare_enable(pcie->pcie_aux_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable aux_clk");
		goto AUX_CLK;
	}

	ret = kirin_pcie_phy_init(pcie);
	if (ret) {
		PCIE_PR_ERR("PHY init Failed");
		goto PHY_INIT;
	}
	PCIE_PR_INFO("PHY init Done");

	/* Call EP poweron callback */
	PCIE_PR_INFO("Device +");
	if (pcie->callback_poweron &&
		pcie->callback_poweron(pcie->callback_data)) {
		PCIE_PR_ERR("Failed: Device callback");
		ret = -1;
		goto PHY_INIT;
	}
	PCIE_PR_INFO("Device -");

	if (!is_pipe_clk_stable(pcie)) {
		ret = -1;
		PCIE_PR_ERR("PIPE clk is not stable");
		goto GPIO_DISABLE;
	}
	PCIE_PR_INFO("PIPE_clk is stable");

	set_phy_eye_param(pcie);

	ret = pcie_noc_power(pcie, 1);
	if (ret) {
		PCIE_PR_ERR("Fail to exit noc idle");
		goto GPIO_DISABLE;
	}

	atomic_add(1, &(pcie->is_power_on));

	kirin_pcie_fixup(pcie);

	PCIE_PR_INFO("-ON-");
	goto MUTEX_UNLOCK;

GPIO_DISABLE:
	kirin_pcie_perst_cfg(pcie, DISABLE);
PHY_INIT:
	clk_disable_unprepare(pcie->pcie_aux_clk);
AUX_CLK:
	clk_disable_unprepare(pcie->pcie_aclk);
AXI_ACLK:
	kirin_pcie_refclk_off(pcie);
REF_CLK:
	kirin_pcie_reset_ctrl(pcie, RST_ENABLE);
	clk_disable_unprepare(pcie->apb_sys_clk);
APB_SYS_CLK:
	clk_disable_unprepare(pcie->apb_phy_clk);
APB_PHY_CLK:
	kirin_pcie_iso_ctrl(pcie, ENABLE);
	PCIE_PR_ERR("Failed to PowerOn");
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

/* Turn off PCIe */
static int kirin_pcie_turn_off(struct kirin_pcie *pcie,
		enum rc_power_status on_flag)
{
	int ret = 0;
	u32 val;

	PCIE_PR_INFO("+OFF+");

	mutex_lock(&pcie->power_lock);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_INFO("PCIe%d already power off", pcie->rc_id);
		goto MUTEX_UNLOCK;
	}
	atomic_set(&(pcie->is_power_on), 0);

	/* mask pcie_axi_timeout */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL10_ADDR);
	val |= AXI_TIMEOUT_MASK_BIT;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL10_ADDR);

	/* Enter NOC Power Idle */
	ret = pcie_noc_power(pcie, 0);
	if (ret)
		PCIE_PR_ERR("Fail to enter noc idle");

	PCIE_PR_INFO("Device +");
	if (pcie->callback_poweroff &&
		pcie->callback_poweroff(pcie->callback_data))
		PCIE_PR_ERR("Failed: Device callback");
	PCIE_PR_INFO("Device -");

	/* rst controller perst_n */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~PCIE_PERST_CONFIG_MASK;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	/* close SIGDET modules */
	/* RAWAONLANEN_DIG_RX_OVRD_OUT_3[5:0]--0x2A */
	val = kirin_natural_phy_readl(pcie, 0x4035);
	val &= ~0x3F;
	val |= 0x2A;
	kirin_natural_phy_writel(pcie, val, 0x4035);

	/* pull up phy_test_powerdown signal */
	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	val |= PHY_TEST_POWERDOWN;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	clk_disable_unprepare(pcie->pcie_aux_clk);
	clk_disable_unprepare(pcie->pcie_aclk);
	kirin_pcie_refclk_off(pcie);
	clk_disable_unprepare(pcie->apb_phy_clk);
	clk_disable_unprepare(pcie->apb_sys_clk);

	kirin_pcie_iso_ctrl(pcie, ENABLE);

	PCIE_PR_INFO("-OFF-");
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

/* Load FW for PHY Fix */
static int pcie_phy_fw_fix_apr(void *data)
{
	struct kirin_pcie *pcie = (struct kirin_pcie *)data;

	return pcie_phy_fw_update(pcie, g_pcie_phy_data_apr,
			ARRAY_SIZE(g_pcie_phy_data_apr));
}

struct pcie_platform_ops plat_ops = {
	.plat_on = kirin_pcie_turn_on,
	.plat_off = kirin_pcie_turn_off,
	.sram_ext_load = pcie_phy_fw_fix_apr,
};

/* entry */
int pcie_plat_init(struct platform_device *pdev, struct kirin_pcie *pcie)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt-crg");
	if (!np) {
		PCIE_PR_ERR("Failed to get hsdt-crg Node");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_ERR("Failed to iomap hsdt_base");
		return -1;
	}
	pcie->plat_ops = &plat_ops;

	return 0;
}
