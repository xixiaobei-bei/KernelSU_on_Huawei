#include "pcie-kirin.h"
#include "pcie-kirin-common.h"

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
#define HSDTCRG_PEREN0				0x0
#define HSDTCRG_PERDIS0				0x4
#define HSDTCRG_PEREN1				0x10
#define HSDTCRG_PERDIS1				0x14
#define HSDTCRG_PERRSTEN0			0x60
#define HSDTCRG_PERRSTDIS0			0x64
#define HSDT1CRG_PERRSTEN0			0x20
#define HSDT1CRG_PERRSTDIS0			0x24
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
#define HSDT1CRG_FNPLL_ISOEN			0x280
#define HSDT1CRG_FNPLL_ISODIS			0x284

/* HSDT register bit definition */
#define PCIE0_CLK_HP_GATE			(0x1 << 1)
#define PCIE0_CLK_DEBUNCE_GATE			(0x1 << 2)
#define PCIE0_CLK_PHYREF_GATE			(0x1 << 6)
#define PCIE0_CLK_IO_GATE			(0x1 << 7)
#define PCIE1_CLK_HP_GATE			(0x1 << 10)
#define PCIE1_CLK_DEBUNCE_GATE			(0x1 << 11)
#define PCIE1_CLK_PHYREF_GATE			(0x1 << 15)
#define PCIE1_CLK_IO_GATE			(0x1 << 16)
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
#define PCIE_RST_HSDT_TCU			(0x1 << 3)
#define PCIE_RST_HSDT_TBU			(0x1 << 5)
#define PCIE1_RST_HSDT_TCU			(0x1 << 13)
#define PCIE1_RST_HSDT_TBU			(0x1 << 14)

/* APB PHY register definition */
#define PHY_REF_USE_PAD				(0x1 << 8)
#define PHY_REF_USE_CIO_PAD			(0x1 << 14)

/* pll Bit */
#define FNPLL_EN_BIT				(0x1 << 0)
#define FNPLL_BP_BIT				(0x1 << 1)
#define FNPLL_LOCK_BIT				(0x1 << 4)
#define FNPLL_ISO_BIT				(0x1 << 0)

#define IO_CLK_SEL_CLEAR			(0x3 << 17)
#define IO_CLK_FROM_CIO				(0x0 << 17)
#define IO_CLK_FROM_PHY_MPLLA			(0x1 << 17)

#define NOC_TIMEOUT_VAL				1000
#define FNPLL_LOCK_TIMEOUT			200

#define GEN3_RELATED_OFF			0x890
#define GEN3_ZRXDC_NONCOMPL			(0x1 << 0)

#define PCIE0_TBU_BASE			0xF8160000
#define PCIE1_TBU_BASE			0xF8560000
#define PCIE_TBU_SIZE			0x1000
#define PCIE_TBU_CR			0x000
#define PCIE_TBU_CRACK			0x004
#define PCIE_TBU_EN_REQ_BIT		(0x1 << 0)
#define PCIE_TBU_EN_ACK_BIT		(0x1 << 0)
#define PCIE_TBU_CONNCT_STATUS_BIT	(0x1 << 1)
#define PCIE_TBU_TOK_TRANS_MSK	(0xFF << 8)
#define PCIE_TBU_TOK_TRANS		0x8
#define PCIE_TBU_DEFALUT_TOK_TRANS	0xF


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
		writel(mask_bits | val_bits,
			pcie->pmctrl_base + NOC_POWER_IDLEREQ);
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

/* config fnpll */
static int pcie_fnpll_ctrl(struct kirin_pcie *pcie, u32 enable)
{
	u32 val;
	u32 i = 0;
	u32 pll_cfg6 = PLL_CFG6;
	u32 pll_cfg7 = PLL_CFG7;
	u32 pciepll_state = PCIEPLL_STATE;

	if (pcie->rc_id == 0) {
		pll_cfg6 += 0x4;
		pll_cfg7 += 0x4;
		pciepll_state -= 0x4;
	}

	/* Set bypass and clear en bit */
	val = hsdt_crg_reg_read(pcie, pll_cfg6);
	val |= FNPLL_BP_BIT;
	val &= ~FNPLL_EN_BIT;
	/* FNPLL Disable and Bypass take effect */
	hsdt_crg_reg_write(pcie, val, pll_cfg6);

	if (enable) {
		/* Set VCO=3.2GHz,FOUTPOSTDIV=100MHz,en=0,bypass=1 */
		val = hsdt_crg_reg_read(pcie, pll_cfg6);
		val &= ~(0xFFFFFFFF);
		val |= 0x30537002;
		hsdt_crg_reg_write(pcie, val, pll_cfg6);

		val = hsdt_crg_reg_read(pcie, pll_cfg7);
		val &= ~(0xFFFFFFFF);
		val |= 0x06555555;
		hsdt_crg_reg_write(pcie, val, pll_cfg7);

		udelay(5);

		val = hsdt_crg_reg_read(pcie, pll_cfg6);
		/* Enable Set */
		val |= FNPLL_EN_BIT;
		hsdt_crg_reg_write(pcie, val, pll_cfg6);

		udelay(20);

		val = hsdt_crg_reg_read(pcie, pciepll_state);
		while (i < FNPLL_LOCK_TIMEOUT) {
			if (val & FNPLL_LOCK_BIT) {
				val = hsdt_crg_reg_read(pcie, pll_cfg6);
				/* clear bypass */
				val &= ~FNPLL_BP_BIT;
				hsdt_crg_reg_write(pcie, val, pll_cfg6);
				PCIE_PR_INFO("FNPLL lock in %d us", i);
				return 0;
			}
			udelay(1);
			i++;
			val = hsdt_crg_reg_read(pcie, pciepll_state);
		}

		PCIE_PR_ERR("FNPLL unlock(%d us)", FNPLL_LOCK_TIMEOUT);
		return -1;
	}

	return 0;
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
 *       DISABLE--disable clkreq control phyio clk
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
 *       DISABLE--disable clkreq control phyref clk
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
	u32  mask, reg;

	/* HW bypass cfg */
	pcie_phyref_hard_bypass(pcie, enable);

	/* soft ref cfg,Always disable SW control */
	if (pcie->rc_id == 0) {
		mask = PCIE0_CLK_PHYREF_GATE;
		reg = HSDTCRG_PERDIS1;
	} else {
		mask = PCIE1_CLK_PHYREF_GATE;
		reg = HSDTCRG_PERDIS0;
	}
	hsdt_crg_reg_write(pcie, mask, reg);
}

/*
 * enable: ENABLE--control by pcieio_oe_mux
 *         DISABLE--close
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
 *        DISABLE--close
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

/* ioref clock from mplla */
static void pcie_ioclk_from_phy(struct kirin_pcie *pcie, bool enable)
{
	u32 reg_val, reg_addr;

	PCIE_PR_INFO("IO refclk from PHY");

	if (pcie->rc_id == 0)
		reg_addr = HSDTCRG_PCIECTRL0;
	else
		reg_addr = HSDTCRG_PCIECTRL1;

	/* select mplla */
	reg_val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL21_ADDR);
	reg_val &= ~IO_CLK_SEL_CLEAR;
	reg_val |= IO_CLK_FROM_PHY_MPLLA;
	kirin_elb_writel(pcie, reg_val, SOC_PCIECTRL_CTRL21_ADDR);

	if (enable) {
		/* mplla force en */
		reg_val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL34_ADDR);
		reg_val |= MPLLA_FORCE_EN_MUX_SEL;
		kirin_apb_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL34_ADDR);

		reg_val = hsdt_crg_reg_read(pcie, reg_addr);
		reg_val |= PCIE_MPLLA_FORCE_EN_SEL;
		hsdt_crg_reg_write(pcie, reg_val, reg_addr);

	}
}

/* ioref clock from fnpll */
static void pcie_ioclk_from_pll(struct kirin_pcie *pcie, u32 enable)
{
	u32 val, mask, reg;

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
	if (pcie->rc_id == 0) {
		mask = PCIE0_CLK_IO_GATE;
		reg = HSDTCRG_PERDIS1;
	} else {
		mask = PCIE1_CLK_IO_GATE;
		reg = HSDTCRG_PERDIS0;
	}
	hsdt_crg_reg_write(pcie, mask, reg);

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
	/* from pll as default */
	if (pcie->dtsinfo.ioref_clk_source == REFCLK_FROM_PHY) {
		PCIE_PR_INFO("CIO Use MPLLA");
		pcie_ioclk_from_phy(pcie, enable);
	} else {
		PCIE_PR_INFO("CIO Use FNPLL");
		pcie_ioclk_from_pll(pcie, enable);
	}
}

/* enable/disable hp&debounce clk */
static void pcie_hp_debounce_clk_gt(struct kirin_pcie *pcie, u32 enable)
{
	u32 mask, reg;

	if (pcie->rc_id == 0) {
		mask = PCIE0_CLK_HP_GATE | PCIE0_CLK_DEBUNCE_GATE;
		if (enable)
			reg = HSDTCRG_PEREN1;
		else
			reg = HSDTCRG_PERDIS1;
	} else {
		mask = PCIE1_CLK_HP_GATE | PCIE1_CLK_DEBUNCE_GATE;
		if (enable)
			reg = HSDTCRG_PEREN0;
		else
			reg = HSDTCRG_PERDIS0;
	}

	hsdt_crg_reg_write(pcie, mask, reg);
}

/*
 * For RC, select FNPLL
 * For EP, select CIO
 * Select FNPLL
 */
static int kirin_pcie_refclk_on(struct kirin_pcie *pcie)
{
	u32 val;
	int ret;

	ret = pcie_fnpll_ctrl(pcie, ENABLE);
	if (ret)
		return ret;

	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val &= ~PHY_REF_USE_PAD;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	if (pcie->dtsinfo.ep_flag)
		val |= PHY_REF_USE_CIO_PAD;
	else
		val &= ~PHY_REF_USE_CIO_PAD;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);

	/* enable pcie hp&debounce clk */
	pcie_hp_debounce_clk_gt(pcie, ENABLE);
	/* gate pciephy clk */
	pcie_phy_ref_clk_gt(pcie, ENABLE);
	/* gate pcieio clk */
	kirin_pcie_ioref_gt(pcie, ENABLE);

	return 0;
}

/* turn off refclk */
static void kirin_pcie_refclk_off(struct kirin_pcie *pcie)
{
	pcie_hp_debounce_clk_gt(pcie, DISABLE);
	kirin_pcie_ioref_gt(pcie, DISABLE);
	pcie_phy_ref_clk_gt(pcie, DISABLE);

	pcie_fnpll_ctrl(pcie, DISABLE);
}


static void kirin_pcie_l1ss_fixup(struct kirin_pcie *pcie)
{
	u32 val;
	/* fix l1ss exit issue */
	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
				GEN3_RELATED_OFF, 0x4);
	val &= ~GEN3_ZRXDC_NONCOMPL;
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
				GEN3_RELATED_OFF, 0x4, val);
}

static void kirin_pcie_may_iso_ctrl(struct kirin_pcie *pcie, int en_flag)
{
	if (en_flag) {
		writel((pcie->dtsinfo.iso_info[1]
			| (pcie->dtsinfo.iso_info[1] << 16)),
			pcie->actrl_base + pcie->dtsinfo.iso_info[0]);
	} else {
		writel(pcie->dtsinfo.iso_info[1] << 16,
			pcie->actrl_base + pcie->dtsinfo.iso_info[0]);
		if (pcie->rc_id == 1)
			hsdt_crg_reg_write(pcie, FNPLL_ISO_BIT,
					HSDT1CRG_FNPLL_ISODIS);
	}
}

/* rst/unrst tbu clk */
static void kirin_pcie_tcu_tbu_rst_gt(struct kirin_pcie *pcie, u32 enable)
{
	u32 mask, reg;

	if (pcie->rc_id == 0) {
		mask = PCIE_RST_HSDT_TCU | PCIE_RST_HSDT_TBU;
		if (enable)
			reg = HSDTCRG_PERRSTEN0;
		else
			reg = HSDTCRG_PERRSTDIS0;
	} else {
		mask = PCIE1_RST_HSDT_TCU | PCIE1_RST_HSDT_TBU;
		if (enable)
			reg = HSDT1CRG_PERRSTEN0;
		else
			reg = HSDT1CRG_PERRSTDIS0;
	}

	hsdt_crg_reg_write(pcie, mask, reg);
}

/* config tbu */
static int kirin_pcie_tbu_config(struct kirin_pcie *pcie, u32 enable)
{
	u32 reg_val, tok_trans_gnt;
	u32 time = 100;

	/* enable req */
	reg_val = readl(pcie->tbu_base + PCIE_TBU_CR);
	if (enable)
		reg_val |= PCIE_TBU_EN_REQ_BIT;
	else
		reg_val &= ~PCIE_TBU_EN_REQ_BIT;
	writel(reg_val, pcie->tbu_base + PCIE_TBU_CR);

	reg_val = readl(pcie->tbu_base + PCIE_TBU_CRACK);
	while (!(reg_val & PCIE_TBU_EN_ACK_BIT)) {
		udelay(1);
		if (time == 0) {
			PCIE_PR_ERR("TBU req(en/dis) not been acknowledged");
			return -1;
		}
		time--;
		reg_val = readl(pcie->tbu_base + PCIE_TBU_CRACK);
	}

	reg_val = readl(pcie->tbu_base + PCIE_TBU_CRACK);
	if (enable) {
		if (!(reg_val & PCIE_TBU_CONNCT_STATUS_BIT)) {
			PCIE_PR_ERR("TBU connecting failed.");
			return -1;
		}
	} else {
		if (reg_val & PCIE_TBU_CONNCT_STATUS_BIT) {
			PCIE_PR_ERR("TBU is disconnect from TCU fail");
			return -1;
		}
	}

	if (enable) {
		reg_val = readl(pcie->tbu_base + PCIE_TBU_CRACK);
		tok_trans_gnt = (reg_val & PCIE_TBU_TOK_TRANS_MSK) >>
				PCIE_TBU_TOK_TRANS;
		if (tok_trans_gnt < PCIE_TBU_DEFALUT_TOK_TRANS) {
			PCIE_PR_ERR("tok_trans_gnt is less than setting");
			return -1;
		}
	}

	return 0;
}

static void kirin_pcie_en_dbi_ep_splt(struct kirin_pcie *pcie)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL30_ADDR);
	if (pcie->rc_id == 0)
		val &= ~PCIE_DBI_EP_SPLT_BIT;
	else
		val |= PCIE_DBI_EP_SPLT_BIT;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL30_ADDR);
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
	kirin_pcie_may_iso_ctrl(pcie, DISABLE);

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

	/*set clkreq low */
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

	/* Enable pcie tcu clk */
	ret = clk_prepare_enable(pcie->pcie_tcu_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable tcu_clk");
		goto TCU_CLK;
	}

	/* Enable pcie tbu clk */
	ret = clk_prepare_enable(pcie->pcie_tbu_clk);
	if (ret) {
		PCIE_PR_ERR("Failed to enable tbu_clk");
		goto TBU_CLK;
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

	/* unrst tbu */
	kirin_pcie_tcu_tbu_rst_gt(pcie, DISABLE);

	ret = kirin_pcie_tbu_config(pcie,  ENABLE);
	if (ret) {
		PCIE_PR_ERR("TBU config Failed");
		goto TBU_INIT;
	}
	PCIE_PR_INFO("TBU init Done");

	if (!is_pipe_clk_stable(pcie)) {
		ret = -1;
		PCIE_PR_ERR("PIPE clk is not stable");
		goto GPIO_DISABLE;
	}
	PCIE_PR_INFO("PIPE_clk is stable");

	ret = pcie_noc_power(pcie, 1);
	if (ret) {
		PCIE_PR_ERR("Fail to exit noc idle");
		goto GPIO_DISABLE;
	}

	kirin_pcie_en_dbi_ep_splt(pcie);

	atomic_add(1, &(pcie->is_power_on));

	kirin_pcie_l1ss_fixup(pcie);

	PCIE_PR_INFO("-ON-");
	goto MUTEX_UNLOCK;

GPIO_DISABLE:
	kirin_pcie_perst_cfg(pcie, DISABLE);
TBU_INIT:
	kirin_pcie_tcu_tbu_rst_gt(pcie, ENABLE);
PHY_INIT:
	clk_disable_unprepare(pcie->pcie_aux_clk);
AUX_CLK:
	clk_disable_unprepare(pcie->pcie_aclk);
TBU_CLK:
	clk_disable_unprepare(pcie->pcie_tbu_clk);
TCU_CLK:
	clk_disable_unprepare(pcie->pcie_tcu_clk);
AXI_ACLK:
	kirin_pcie_refclk_off(pcie);
REF_CLK:
	kirin_pcie_reset_ctrl(pcie, RST_ENABLE);
	clk_disable_unprepare(pcie->apb_sys_clk);
APB_SYS_CLK:
	clk_disable_unprepare(pcie->apb_phy_clk);
APB_PHY_CLK:
	kirin_pcie_may_iso_ctrl(pcie, ENABLE);
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

	ret = kirin_pcie_tbu_config(pcie,  DISABLE);
	if (ret)
		PCIE_PR_ERR("SMMU config Failed");

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

	kirin_pcie_tcu_tbu_rst_gt(pcie, ENABLE);
	clk_disable_unprepare(pcie->pcie_aux_clk);
	clk_disable_unprepare(pcie->pcie_tbu_clk);
	clk_disable_unprepare(pcie->pcie_tcu_clk);
	clk_disable_unprepare(pcie->pcie_aclk);
	kirin_pcie_refclk_off(pcie);
	clk_disable_unprepare(pcie->apb_phy_clk);
	clk_disable_unprepare(pcie->apb_sys_clk);

	kirin_pcie_may_iso_ctrl(pcie, ENABLE);

	PCIE_PR_INFO("-OFF-");
MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
	return ret;
}

/* Load FW for PHY Fix */
static int pcie_phy_fw_fix_may(void *data)
{
	return 0;
}

struct pcie_platform_ops plat_ops = {
	.plat_on = kirin_pcie_turn_on,
	.plat_off = kirin_pcie_turn_off,
	.sram_ext_load = pcie_phy_fw_fix_may,
};

/* entry */
int pcie_plat_init(struct platform_device *pdev, struct kirin_pcie *pcie)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,actrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get actrl Node");
		return -1;
	}
	pcie->actrl_base = of_iomap(np, 0);
	if (!pcie->actrl_base) {
		PCIE_PR_ERR("Failed to iomap actrl_base");
		return -1;
	}

	if (!pcie->rc_id)
		np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt-crg");
	else
		np = of_find_compatible_node(NULL, NULL, "hisilicon,hsdt1-crg");
	if (!np) {
		PCIE_PR_ERR("Failed to get hsdt-crg Node");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_ERR("Failed to iomap hsdt_base");
		return -1;
	}

	if (!pcie->rc_id)
		pcie->tbu_base = ioremap(PCIE0_TBU_BASE, PCIE_TBU_SIZE);
	else
		pcie->tbu_base = ioremap(PCIE1_TBU_BASE, PCIE_TBU_SIZE);

	if (!pcie->tbu_base) {
		PCIE_PR_ERR("Failed to iomap tbu_base");
		return -1;
	}

	pcie->pcie_tcu_clk = devm_clk_get(&pdev->dev, "pcie_tcu_clk");
	if (IS_ERR(pcie->pcie_tcu_clk)) {
		PCIE_PR_ERR("Failed to get pcie_tcu_clk");
		return PTR_ERR(pcie->pcie_tcu_clk);
	}

	pcie->pcie_tbu_clk = devm_clk_get(&pdev->dev, "pcie_tbu_clk");
	if (IS_ERR(pcie->pcie_tbu_clk)) {
		PCIE_PR_ERR("Failed to get pcie_tbu_clk");
		return PTR_ERR(pcie->pcie_tbu_clk);
	}

	pcie->plat_ops = &plat_ops;

	return 0;
}
