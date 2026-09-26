/*
 * pcie-kport-phy-xw.c
 *
 * XW PCIePHY Driver
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
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <linux/of_address.h>

#include <securec.h>

#include "pcie-kport-phy.h"
#include "pcie-kport-phy-xw.h"
#include "pcie-kport-phy-xw-image.h"

static inline void xw_phy_write(struct pcie_phy *phy, uint32_t val, uint32_t reg)
{
	writel(val, phy->phy_base + reg);
}

static inline uint32_t xw_phy_read(struct pcie_phy *phy, uint32_t reg)
{
	return readl(phy->phy_base + reg);
}

static inline void xw_phy_core_write(struct pcie_phy *phy, uint32_t val, uint32_t reg)
{
	writel(val, phy->core_base + reg);
}

static inline uint32_t xw_phy_core_read(struct pcie_phy *phy, uint32_t reg)
{
	return readl(phy->core_base + reg);
}

static void xw_phy_w(struct pcie_phy *phy, uint32_t reg, uint32_t msb, uint32_t lsb, uint32_t val)
{
	uint32_t data = xw_phy_read(phy, reg);

	data = DRV_32BIT_SET_FIELD(data, lsb, msb, val);
	xw_phy_write(phy, data, reg);
}

static uint32_t xw_phy_r(struct pcie_phy *phy, uint32_t reg, uint32_t msb, uint32_t lsb)
{
	uint32_t data = xw_phy_read(phy, reg);

	return DRV_32BIT_READ_FIELD(data, lsb, msb);
}

static void debug_intr_enable(struct pcie_phy *phy)
{
	uint32_t reg_val;

	reg_val = xw_phy_read(phy, PHY_PMAC_CLK_MISS_CHK_ADDR);
	reg_val &= ~PHY_CLK_MISS_CHK_MASK;
	reg_val |= PHY_PMAC_CLK_MISS_VAL;
	xw_phy_write(phy, reg_val, PHY_PMAC_CLK_MISS_CHK_ADDR);

	reg_val = xw_phy_read(phy, PHY_PIPEC_CLK_MISS_CHK_ADDR);
	reg_val &= ~PHY_CLK_MISS_CHK_MASK;
	reg_val |= PHY_PIPEC_CLK_MISS_VAL;
	xw_phy_write(phy, reg_val, PHY_PIPEC_CLK_MISS_CHK_ADDR);

	reg_val = xw_phy_read(phy, PHY_RXREG_CLK_MISS_CHK_ADDR);
	reg_val &= ~PHY_CLK_MISS_CHK_MASK;
	reg_val |= PHY_EXREG_CLK_MISS_VAL;
	xw_phy_write(phy, reg_val, PHY_RXREG_CLK_MISS_CHK_ADDR);
}

#ifdef CONFIG_PCIE_KPORT_BURB
void xw_phy_pllctrl_parm(struct pcie_phy *phy)
{
	u32 reg_val;

	reg_val = xw_phy_read(phy, PHY_PLLCKGCTRLG1_ADDR);
	reg_val &= ~PHY_GEN1_PLL_ICP_MASK;
	reg_val |= PHY_GEN1_PLL_ICP_SET;
	xw_phy_write(phy, reg_val, PHY_PLLCKGCTRLG1_ADDR);

	reg_val = xw_phy_read(phy, PHY_PLLCKGCTRLG2_ADDR);
	reg_val &= ~PHY_GEN2_PLL_ICP_MASK;
	reg_val |= PHY_GEN2_PLL_ICP_SET;
	xw_phy_write(phy, reg_val, PHY_PLLCKGCTRLG2_ADDR);

	reg_val = xw_phy_read(phy, PHY_PLLCTRL_ADDR);
	reg_val &= ~PHY_PLL_LDO_SEL_MASK;
	reg_val |= PHY_PLL_LDO_SEL_SET;
	xw_phy_write(phy, reg_val, PHY_PLLCTRL_ADDR);
}
#endif

static void xw_phy_init(struct pcie_phy *phy)
{
	uint32_t reg_val;

	I("+%s+\n", __func__);
	/* config tx_elecidle entry&exit time */
	xw_phy_write(phy, PHY_TXIDLE_EXIT_SEL, PHY_TXIDLE_EXIT_SEL_ADDR);
	xw_phy_write(phy, PHY_TXIDLE_ENTRY_SEL, PHY_TXIDLE_ENTRY_SEL_ADDR);

	xw_phy_write(phy, PHY_TXEQCOEFF_VAL, PHY_TXEQCOEFF_ADDR);

	reg_val = xw_phy_read(phy, PHY_RXUPDTCTRL_ADDR);
	reg_val &= ~PHY_RX_GSARK_EN;
	reg_val &= ~PHY_RX_EQARK_EN;
	xw_phy_write(phy, reg_val, PHY_RXUPDTCTRL_ADDR);

	reg_val = xw_phy_read(phy, PHY_RXCTRL0_ADDR);
	reg_val &= ~PHY_RX_MANUAL;
	reg_val |= PHY_RX0_EQ_C;
	reg_val |= PHY_RX0_ATT;
	reg_val |= PHY_RX0_EQ_R;
#ifdef CONFIG_PCIE_KPORT_BURB
	reg_val |= PHY_RX0_LDO_SEL;
#endif
	xw_phy_write(phy, reg_val, PHY_RXCTRL0_ADDR);

	reg_val = xw_phy_read(phy, PHY_RXEQACTRL_ADDR);
	reg_val &= ~PHY_RXCTRL_EQA_SLE_C_MASK;
	reg_val |= PHY_RXCTRL_EQA_SLE_C_SET;
	xw_phy_write(phy, reg_val, PHY_RXEQACTRL_ADDR);

	reg_val = xw_phy_read(phy, PHY_RXGSACTRL_ADDR);
	reg_val &= ~PHY_RXCTRL_GSA_RX_ATT_MASK;
	reg_val |= PHY_RXCTRL_GSA_RX_ATT_SET;
	xw_phy_write(phy, reg_val, PHY_RXGSACTRL_ADDR);

#ifndef CONFIG_PCIE_KPORT_BURB /* no adjust this in pcie_burb */
	reg_val = xw_phy_read(phy, PHY_RXCTRL1_ADDR);
	reg_val &= ~PHY_RX_VCM_MASK;
	reg_val |= PHY_RX_VCM_SET;
	xw_phy_write(phy, reg_val, PHY_RXCTRL1_ADDR);
#endif

	reg_val = xw_phy_read(phy, PHY_RXDETTMR_ADDR);
	reg_val &= ~PHY_RXDET_EN_TMR_MASK;
	reg_val |= PHY_RXDET_EN_TMR_SET;
	xw_phy_write(phy, reg_val, PHY_RXDETTMR_ADDR);

	reg_val = xw_phy_read(phy, PHY_TXDRVCTRL_ADDR);
	reg_val &= ~PHY_TX_VCM_MASK;
	reg_val |= PHY_TX_VCM_SET;
	xw_phy_write(phy, reg_val, PHY_TXDRVCTRL_ADDR);

#ifdef CONFIG_PCIE_KPORT_BURB
	xw_phy_pllctrl_parm(phy);
#endif
	debug_intr_enable(phy);

	I("-%s-\n", __func__);
}

static void vcok_termk_osk_show(struct pcie_phy *phy)
{
	uint32_t reg_val;

	E("pllckgctrlg2[0x%x], pllckgctrlg3[0x%x], termkctrl[0x%x]\n",
		xw_phy_read(phy, PHY_PLLCKGCTRLG2_ADDR),
		xw_phy_read(phy, PHY_PLLCKGCTRLG3_ADDR),
		xw_phy_read(phy, PHY_TERMKCTRL_ADDR));

	/* enable RX debug port */
	reg_val = xw_phy_read(phy, PHY_RXCTRL1_ADDR);
	reg_val |= PHY_RX_DBG_PORT_BIT;
	xw_phy_write(phy, reg_val, PHY_RXCTRL1_ADDR);
	/* select EQ OS value */
	reg_val = xw_phy_read(phy, PHY_RXCTRL1_ADDR);
	reg_val &= ~PHY_RX_TEST_MODE_MASK;
	reg_val |= PHY_RX_TEST_MODE_EQ;
	xw_phy_write(phy, reg_val, PHY_RXCTRL1_ADDR);
	E("EQ Offset Calibration result[0x%x]\n",
		xw_phy_read(phy, PHY_PMACDBGCTRL_ADDR));
	/* select SA OS value */
	reg_val = xw_phy_read(phy, PHY_RXCTRL1_ADDR);
	reg_val &= ~PHY_RX_TEST_MODE_MASK;
	reg_val |= PHY_RX_TEST_MODE_SA;
	xw_phy_write(phy, reg_val, PHY_RXCTRL1_ADDR);
	E("SA Offset Calibration result[0x%x]\n",
		xw_phy_read(phy, PHY_PMACDBGCTRL_ADDR));
	/* select GS OS value */
	reg_val = xw_phy_read(phy, PHY_RXCTRL1_ADDR);
	reg_val &= ~PHY_RX_TEST_MODE_MASK;
	reg_val |= PHY_RX_TEST_MODE_GS;
	xw_phy_write(phy, reg_val, PHY_RXCTRL1_ADDR);
	E("GS Offset Calibration result[0x%x]\n",
		xw_phy_read(phy, PHY_PMACDBGCTRL_ADDR));
}

static void down_bit_clear(struct pcie_phy *phy)
{
	uint32_t reg_val;

	reg_val = xw_phy_read(phy, PHY_PLLVCO5GKCTRL_ADDR);
	reg_val |= PHY_CLE_VCOK5G_DONE_BIT;
	xw_phy_write(phy, reg_val, PHY_PLLVCO5GKCTRL_ADDR);

	reg_val = xw_phy_read(phy, PHY_PLLVCO8GKCTRL_ADDR);
	reg_val |= PHY_CLE_VCOK8G_DONE_BIT;
	xw_phy_write(phy, reg_val, PHY_PLLVCO8GKCTRL_ADDR);

	reg_val = xw_phy_read(phy, PHY_RXOFSTUPDTCTRL_ADDR);
	reg_val |= PHY_CLE_EQOFSTK_DONE_BIT;
	reg_val |= PHY_CLE_SAOFSTK_DONE_BIT;
	reg_val |= PHY_CLE_GSOFSTK_DONE_BIT;
	xw_phy_write(phy, reg_val, PHY_RXOFSTUPDTCTRL_ADDR);
}

static bool is_xw_phy_ready(struct pcie_phy *phy)
{
	uint32_t reg_val;

	reg_val = xw_phy_read(phy, PHY_PMACINTRCTRL_ADDR);
	if (reg_val & PHY_INIT_PROC_ERR_MASK) {
		E("XW phy init process err[0x%x]\n", reg_val);
		vcok_termk_osk_show(phy);

		if (reg_val & PHY_INIT_FATAL_ERR_MASK)
			return false;
	}

	down_bit_clear(phy);

	I("XW phy init success\n");
	return true;
}

static void xw_phy_stat_dump(struct pcie_phy *phy)
{
	uint32_t j, val;

	I("+%s+\n", __func__);

	/* 4 register-value per line,0x200: phy_dfx_stat_base_addr(mac/pipe/rx) */
	for (j = 0; j < 0x2; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j + 0x200,
			xw_phy_read(phy, 0x10 * j + 0x200 + 0x0), xw_phy_read(phy, 0x10 * j + 0x200 + 0x4),
			xw_phy_read(phy, 0x10 * j + 0x200 + 0x8), xw_phy_read(phy, 0x10 * j + 0x200 + 0xC));

	val = xw_phy_read(phy, PHY_PMA_DBG_SEL_ADDR);
	val &= ~PHY_PMA_DBG_SEL_MASK;
	val |= PHY_PMA_DBG_SEL0;
	xw_phy_write(phy, val, PHY_PMA_DBG_SEL_ADDR);
	pr_info("0x%-8x: %8x  0x%-8x: %8x\n", PHY_PMA_DBG_SEL_ADDR,
			xw_phy_read(phy, PHY_PMA_DBG_SEL_ADDR),
			PHY_PMA_DBG_ADDR, xw_phy_read(phy, PHY_PMA_DBG_ADDR));

	val = xw_phy_read(phy, PHY_PMA_DBG_SEL_ADDR);
	val &= ~PHY_PMA_DBG_SEL_MASK;
	val |= PHY_PMA_DBG_SEL1;
	xw_phy_write(phy, val, PHY_PMA_DBG_SEL_ADDR);
	pr_info("0x%-8x: %8x  0x%-8x: %8x\n", PHY_PMA_DBG_SEL_ADDR,
			xw_phy_read(phy, PHY_PMA_DBG_SEL_ADDR),
			PHY_PMA_DBG_ADDR, xw_phy_read(phy, PHY_PMA_DBG_ADDR));

	/* 4 register-value per line,0xaa0: phy_pma_stat_base_addr */
	for (j = 0; j < 0x2; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j + 0xaa0,
			xw_phy_read(phy, 0x10 * j + 0xaa0 + 0x0), xw_phy_read(phy, 0x10 * j + 0xaa0 + 0x4),
			xw_phy_read(phy, 0x10 * j + 0xaa0 + 0x8), xw_phy_read(phy, 0x10 * j + 0xaa0 + 0xC));
	/* 4 register-value per line,0xc00: phy_tx_stat_base_addr */
	for (j = 0; j < 0x1; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j + 0xc00,
			xw_phy_read(phy, 0x10 * j + 0xc00 + 0x0), xw_phy_read(phy, 0x10 * j + 0xc00 + 0x4),
			xw_phy_read(phy, 0x10 * j + 0xc00 + 0x8), xw_phy_read(phy, 0x10 * j + 0xc00 + 0xC));
	/* 4 register-value per line,0xe00: phy_pipe_stat_base_addr */
	for (j = 0; j < 0x2; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j + 0xe00,
			xw_phy_read(phy, 0x10 * j + 0xe00 + 0x0), xw_phy_read(phy, 0x10 * j + 0xe00 + 0x4),
			xw_phy_read(phy, 0x10 * j + 0xe00 + 0x8), xw_phy_read(phy, 0x10 * j + 0xe00 + 0xC));
	/* 4 register-value per line,0xf00: phy_pma_stat_base_addr */
	for (j = 0; j < 0x5; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j + 0xf00,
			xw_phy_read(phy, 0x10 * j + 0xf00 + 0x0), xw_phy_read(phy, 0x10 * j + 0xf00 + 0x4),
			xw_phy_read(phy, 0x10 * j + 0xf00 + 0x8), xw_phy_read(phy, 0x10 * j + 0xf00 + 0xC));
	/* 4 register-value per line,0x1c00: phy_rx_stat_base_addr */
	for (j = 0; j < 0x1; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n", 0x10 * j + 0x1c00,
			xw_phy_read(phy, 0x10 * j + 0x1c00 + 0x0), xw_phy_read(phy, 0x10 * j + 0x1c00 + 0x4),
			xw_phy_read(phy, 0x10 * j + 0x1c00 + 0x8), xw_phy_read(phy, 0x10 * j + 0x1c00 + 0xC));

	I("-%s-\n", __func__);
}

static inline bool need_acore_handle_irq(uint32_t sts_val, uint32_t sts_bit,
				  uint32_t mask_val, uint32_t mask_bit)
{
	return ((sts_val & sts_bit) && ((!(mask_val & mask_bit))));
}

static inline void clear_irq(struct pcie_phy *phy, uint32_t sts_reg, uint32_t sts_bit)
{
	uint32_t val = xw_phy_read(phy, sts_reg);

	val |= sts_bit;
	xw_phy_write(phy, val, sts_reg);
}

static inline void mask_irq(struct pcie_phy *phy, uint32_t mask_reg, uint32_t mask_bit)
{
	uint32_t val = xw_phy_read(phy, mask_reg);

	val |= mask_bit;
	xw_phy_write(phy, val, mask_reg);
}

static inline void unmask_irq(struct pcie_phy *phy, uint32_t mask_reg, uint32_t mask_bit)
{
	uint32_t val = xw_phy_read(phy, mask_reg);

	val &= ~mask_bit;
	xw_phy_write(phy, val, mask_reg);
}

/* irq handle flow */
static int32_t base_irq_handler(struct pcie_phy *phy, const struct phy_irq_info *info,
			    const char * const *msg_info, uint32_t cnt)
{
	uint32_t sts_reg, sts_val, mask_reg, mask_val;
	uint32_t sts_bit, mask_bit;
	uint32_t i;

	mutex_lock(&phy->irq_mutex_lock);

	for (i = 0; i < cnt; i++) {
		sts_reg = info[i].status_reg;
		sts_bit = info[i].status_bits;
		mask_reg = info[i].mask_reg;
		mask_bit = info[i].mask_bits;
		sts_val = xw_phy_read(phy, sts_reg);
		mask_val = xw_phy_read(phy, mask_reg);
		if (need_acore_handle_irq(sts_val, sts_bit, mask_val, mask_bit)) {
			mask_irq(phy, mask_reg, mask_bit);

			clear_irq(phy, sts_reg, sts_bit);

			I("Phy_irq: %s\n", msg_info[i]);
			if (info[i].irq_handler)
				(void)info[i].irq_handler(phy);

			/* unmask irq */
			unmask_irq(phy, mask_reg, mask_bit);
		}
	}

	mutex_unlock(&phy->irq_mutex_lock);

	return 0;
}

/* --------------DFX------------------------- */
static const char * const dfx_irq_msg[DFX_IRQ_NUM] = {
	"mac_pcs_handshake_timeout",
	"pipe_crg_handshake_timeout",
	"pipe_mac_handshake_timeout",
	"rxreg_clk_sw_timeout",
	"rxstandy_chg",
	"mac_clk_miss",
	"pipe_clk_miss",
	"rxreg_clk_miss",
};

static const struct phy_irq_info dfx_irq_info[] = {
	{ MAC_PCS_TO_REG, DFX_MASK_BIT, MAC_PCS_TO_REG, DFX_STS_BIT, NULL, },
	{ PIPE_CRG_TO_REG, DFX_MASK_BIT, PIPE_CRG_TO_REG, DFX_STS_BIT, NULL, },
	{ PIPE_MAC_TO_REG, DFX_MASK_BIT, PIPE_MAC_TO_REG, DFX_STS_BIT, NULL, },
	{ RXREG_CLK_SW_TO_REG, DFX_MASK_BIT, RXREG_CLK_SW_TO_REG, DFX_STS_BIT, NULL, },
	{ RXSTD_CHG_REG, DFX_MASK_BIT, RXSTD_CHG_REG, DFX_STS_BIT, NULL, },
	{ MAC_CLK_MISS_REG, DFX_MASK_BIT, MAC_CLK_MISS_REG, DFX_STS_BIT, NULL, },
	{ PIPE_CLK_MISS_REG, DFX_MASK_BIT, PIPE_CLK_MISS_REG, DFX_STS_BIT, NULL, },
	{ RXREG_CLK_MISS_REG, DFX_MASK_BIT, RXREG_CLK_MISS_REG, DFX_STS_BIT, NULL, },
};

static int32_t dfx_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, dfx_irq_info, dfx_irq_msg, DFX_IRQ_NUM);
}

/* --------------Share_irq---------------- */
static const char * const share_irq_msg[] = {
	"vco5g_err",
	"vco8g_err",
	"txtermk_ovudf",
	"rxtermk_ovudf",
};

static const struct phy_irq_info share_irq_info[] = {
	{ SHARE_IRQ_REG, VCO_5G_ERR_MK, SHARE_IRQ_REG, VCO_5G_ERR_STS, NULL, },
	{ SHARE_IRQ_REG, VCO_8G_ERR_MK, SHARE_IRQ_REG, VCO_8G_ERR_STS, NULL, },
	{ SHARE_IRQ_REG, RXTERM_OVUD_MK, SHARE_IRQ_REG, RXTERM_OVUD_STS, NULL, },
	{ SHARE_IRQ_REG, TXTERM_OVUD_MK, SHARE_IRQ_REG, TXTERM_OVUD_STS, NULL, },
};

static int32_t share_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, share_irq_info, share_irq_msg, SHARE_IRQ_NUM);
}

/* --------------GP_irq---------------- */
static const char * const gp_irq_msg[] = {
	"gp0_intr",
	"gp1_intr",
	"gp2_intr",
	"gp3_intr",
	"gp4_intr",
	"gp5_intr",
	"gp6_intr",
	"gp7_intr",
};

static const struct phy_irq_info gp_irq_info[] = {
	{ GP_IRQ_MASK_REG, GP0_BIT, GP_IRQ_STS_REG, GP0_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP1_BIT, GP_IRQ_STS_REG, GP1_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP2_BIT, GP_IRQ_STS_REG, GP2_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP3_BIT, GP_IRQ_STS_REG, GP3_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP4_BIT, GP_IRQ_STS_REG, GP4_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP5_BIT, GP_IRQ_STS_REG, GP5_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP6_BIT, GP_IRQ_STS_REG, GP6_BIT, NULL, },
	{ GP_IRQ_MASK_REG, GP7_BIT, GP_IRQ_STS_REG, GP7_BIT, NULL, },
};

static int32_t gp_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, gp_irq_info, gp_irq_msg, GP_IRQ_NUM);
}

/* --------------rx err irq------------ */
static const char * const rx_irq_msg[RX_IRQ_NUM] = {
	"rx_symbol_align_err",
	"rx_syncheader_err",
	"rx_ebuf_overflow",
	"rx_ebuf_underflow",
};

static const struct phy_irq_info rx_irq_info[RX_IRQ_NUM] = {
	{ RX_IRQ_REG, RX_SYMB_ERR_MK, RX_IRQ_REG, RX_SYMB_ERR_STS, NULL, },
	{ RX_IRQ_REG, RX_SYNCHEADER_ERR_MK, RX_IRQ_REG, RX_SYNCHEADER_ERR_STS, NULL, },
	{ RX_IRQ_REG, RX_EBUF_UD_MK, RX_IRQ_REG, RX_EBUF_OV_STS, NULL, },
	{ RX_IRQ_REG, RX_EBUF_UD_MK, RX_IRQ_REG, RX_EBUF_UD_STS, NULL, },
};

static int32_t rx_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, rx_irq_info, rx_irq_msg, RX_IRQ_NUM);
}

/* --------------tx err irq------------ */
static const char * const tx_irq_msg[TX_IRQ_NUM] = {
	"tx_kcode_err",
	"rx_sym_err_cnt",
	"gen3_encode_buf_overflow",
	"gen3_encode_buf_underflow",
};

static const struct phy_irq_info tx_irq_info[TX_IRQ_NUM] = {
	{ TX_IRQ_REG, TX_KCODE_ERR_MK, TX_IRQ_REG, TX_KCODE_ERR_STS, NULL, },
	{ TX_IRQ_REG, TX_SYMB_ERR_MK, TX_IRQ_REG, TX_SYMB_ERR_STS, NULL, },
	{ TX_IRQ_REG, TX_G3_EBUF_OV_MK, TX_IRQ_REG, TX_G3_EBUF_OV_STS, NULL, },
	{ TX_IRQ_REG, TX_G3_EBUF_UD_MK, TX_IRQ_REG, TX_G3_EBUF_UD_STS, NULL, },
};

static int32_t tx_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, tx_irq_info, tx_irq_msg, TX_IRQ_NUM);
}

/* --------------eios_detect irq------------ */
static const char * const eios_det_irq_msg[EIOS_DET_IRQ_NUM] = {
	"no_eios_detect_before_rxei",
};

static const struct phy_irq_info eios_det_irq_info[EIOS_DET_IRQ_NUM] = {
	{ EIOS_DET_IRQ_REG, EIOS_DET_IRQ_MASK, EIOS_DET_IRQ_REG, EIOS_DET_IRQ_STS, },
};

static int32_t eios_det_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, eios_det_irq_info, eios_det_irq_msg, EIOS_DET_IRQ_NUM);
}

/* --------------pma irq------------ */
static const char * const pma_irq_msg[PMA_IRQ_NUM] = {
	"gsa_timeout",
	"eqa_timeout",
	"fom_timeout",
	"mchg_preproc_intr",
	"mchg_pstproc_intr",
};

static const struct phy_irq_info pma_irq_info[PMA_IRQ_NUM] = {
	{ PMA_IRQ_REG, PMA_GSA_TO_MK, PMA_IRQ_REG, PMA_GSA_TO_STS, NULL, },
	{ PMA_IRQ_REG, PMA_EQA_TO_MK, PMA_IRQ_REG, PMA_EQA_TO_STS, NULL, },
	{ PMA_IRQ_REG, PMA_FOM_TO_MK, PMA_IRQ_REG, PMA_FOM_TO_STS, NULL, },
	{ PMA_IRQ_REG, PMA_MCHG_PREPROC_MK, PMA_IRQ_REG, PMA_MCHG_PREPROC_STS, NULL, },
	{ PMA_IRQ_REG, PMA_MCHG_PSTPROC_MK, PMA_IRQ_REG, PMA_MCHG_PSTPROC_STS, NULL, },
};

static int32_t pma_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, pma_irq_info, pma_irq_msg, PMA_IRQ_NUM);
}

/* --------------phy_core talk to acpu irq------------ */
static const char * const core_talk_irq_msg[CORE_TALK_IRQ_NUM] = {
	"phy_core_to_acore_irq",
};

static const struct phy_irq_info core_talk_irq_info[CORE_TALK_IRQ_NUM] = {
	{ CORE_TALK_IRQ_MASK_REG, CORE_TALK_IRQ_MASK, CORE_TALK_IRQ_STS_REG, CORE_TALK_IRQ_STS, },
};

static int32_t core_talk_irq_handler(struct pcie_phy *phy)
{
	return base_irq_handler(phy, core_talk_irq_info, core_talk_irq_msg, CORE_TALK_IRQ_NUM);
}

/* --------------Top Module------------------------- */
static const struct phy_irq_info xw_phy_irq_info[TOP_IRQ_NUM] = {
	{ CORE_TALK_IRQ_MASK_REG, CORE_TALK_IRQ_MASK, CORE_TALK_IRQ_STS_REG, CORE_TALK_IRQ_STS_REG, core_talk_irq_handler, },
	{ PMA_IRQ_REG, PMA_IRQ_MASK, PMA_IRQ_REG, PMA_IRQ_STS, pma_irq_handler, },
	{ TX_IRQ_REG, TX_IRQ_MASK, TX_IRQ_REG, TX_IRQ_STS, tx_irq_handler, },
	{ EIOS_DET_IRQ_REG, EIOS_DET_IRQ_MASK, EIOS_DET_IRQ_REG, EIOS_DET_IRQ_STS, eios_det_irq_handler, },
	{ RX_IRQ_REG, RX_IRQ_MASK, RX_IRQ_REG, RX_IRQ_STS, rx_irq_handler, },
	{ SHARE_IRQ_REG, SHARE_IRQ_MASK, SHARE_IRQ_REG, SHARE_IRQ_STS, share_irq_handler, },
	{ GP_IRQ_MASK_REG, GP_IRQ_BITS_ALL, GP_IRQ_STS_REG, GP_IRQ_BITS_ALL, gp_irq_handler, },
};

static void xw_phy_irq_handler(struct pcie_phy *phy)
{
	int32_t i;
	int32_t ret = 0;

	E("%s\n", __func__);
	/* dfx */
	dfx_irq_handler(phy);

	/* others */
	for (i = 0; i < TOP_IRQ_NUM; i++) {
		if (xw_phy_irq_info[i].irq_handler)
			ret += xw_phy_irq_info[i].irq_handler(phy);
	}
}

static struct mcu_irq_info xw_phy_irq_mcu[MCU_IRQ_MASK_NUM] = {
	{ 0xF70, 0x7FFFFFF, }, /* OTHERS TOP */
	{ 0x10C, 0x201, },     /* TALK TOP */
};

static void init_phy_irq_to_mcu(struct pcie_phy *phy, bool enable)
{
	uint32_t val;
	uint32_t reg;
	int32_t i;

	for (i = 0; i < MCU_IRQ_MASK_NUM; i++) {
		reg = xw_phy_irq_mcu[i].mask_reg;
		val = xw_phy_read(phy, reg);
		if (enable)
			val &= ~(xw_phy_irq_mcu[i].mask_bits);
		else
			val |= xw_phy_irq_mcu[i].mask_bits;
		xw_phy_write(phy, val, reg);
	}
}

static void init_phy_irq_to_acore(struct pcie_phy *phy, bool enable)
{
	uint32_t val;
	uint32_t reg;

	reg = PCS_TOP_MASK_REG;
	val = xw_phy_read(phy, reg);
	if (enable)
		val &= ~PCS_TOP_MASK_BIT;
	else
		val |= PCS_TOP_MASK_BIT;
	xw_phy_write(phy, val, reg);
}

static void clear_phy_irq_init_status(struct pcie_phy *phy)
{
	uint32_t val;
	uint32_t reg;
	int32_t i;

	for (i = 0; i < TOP_IRQ_NUM; i++) {
		reg = xw_phy_irq_info[i].status_reg;
		val = xw_phy_read(phy, reg);
		val |= xw_phy_irq_info[i].status_bits;
		xw_phy_write(phy, val, reg);
	}

	/* Special: DFX */
	for (reg = DFX_INT_REG_BEG; reg <= DFX_INT_REG_END; reg += REG_DWORD_ALIGN) {
		val = xw_phy_read(phy, reg);
		val |= DFX_INT_STS_BIT;
		xw_phy_write(phy, val, reg);
	}
}

static void xw_phy_irq_init(struct pcie_phy *phy)
{
	I("%s\n", __func__);
	clear_phy_irq_init_status(phy);
	init_phy_irq_to_mcu(phy, DISABLE);
	init_phy_irq_to_acore(phy, ENABLE);
}

static void xw_phy_irq_disable(struct pcie_phy *phy)
{
	I("%s\n", __func__);
	init_phy_irq_to_mcu(phy, DISABLE);
	init_phy_irq_to_acore(phy, DISABLE);
}

/*
 * PHY MCU
 */
#ifdef XW_PHY_MCU
static void xw_phy_core_init(struct pcie_phy *phy)
{
	/* clk for core sub-module */
	xw_phy_core_write(phy, UCE_CRG_GT_BIT,
		PHY_CORE_SCTRL_OFFSET +  UCE_CRG_GT_EN);

	/* unrst core */
	xw_phy_core_write(phy, UCE_CRG_RST_BIT,
		PHY_CORE_SCTRL_OFFSET +  UCE_CRG_RST_DIS);
}

#define REG_DWORD_ALIGN			0x4
static int32_t xw_phy_image_load(struct pcie_phy *phy)
{
	uint32_t reg_addr;

	for (reg_addr = 0; reg_addr < xw_phy_image_size; reg_addr++)
		writel(xw_phy_image[reg_addr],
			phy->core_base + reg_addr * REG_DWORD_ALIGN);
	return 0;
}

static inline void xw_phy_set_core_pc(struct pcie_phy *phy)
{
	xw_phy_core_write(phy, CORE_PC_START_ADDR,
		PHY_CORE_SCTRL_OFFSET +  UCE_MCU_POR_PC);
}

static int32_t xw_phy_core_start(struct pcie_phy *phy)
{
	uint32_t val;

	I("+%s+\n", __func__);

	xw_phy_core_init(phy);

	xw_phy_image_load(phy);

	/* set pc */
	xw_phy_set_core_pc(phy);

	/* exit halt */
	val = xw_phy_core_read(phy, PHY_CORE_SCTRL_OFFSET + UCE_MCU_CTRL);
	val &= ~UCE_MCU_CORE_WAIT;
	xw_phy_core_write(phy, val, PHY_CORE_SCTRL_OFFSET +  UCE_MCU_CTRL);
	I("-%s-\n", __func__);

	return 0;
}

static int32_t xw_phy_get_core_clk(struct pcie_phy *phy, struct device_node *np)
{
	phy->mcu_19p2_clk = of_clk_get_by_name(np, "pcie_mcu_19p2");
	if (IS_ERR(phy->mcu_19p2_clk)) {
		E("Failed to get pcie_mcu_19p2 clock\n");
		return PTR_ERR(phy->mcu_19p2_clk);
	}

	phy->mcu_clk = of_clk_get_by_name(np, "pcie_mcu");
	if (IS_ERR(phy->mcu_clk)) {
		E("Failed to get pcie_mcu clock\n");
		return PTR_ERR(phy->mcu_clk);
	}

	phy->mcu_bus_clk = of_clk_get_by_name(np, "pcie_mcu_bus");
	if (IS_ERR(phy->mcu_bus_clk)) {
		E("Failed to get pcie_mcu_bus clock\n");
		return PTR_ERR(phy->mcu_bus_clk);
	}

	phy->mcu_32k_clk = of_clk_get_by_name(np, "pcie_mcu_32k");
	if (IS_ERR(phy->mcu_32k_clk)) {
		E("Failed to get pcie_mcu_32k clock\n");
		return PTR_ERR(phy->mcu_32k_clk);
	}

	I("Successed to get all mcu clock\n");

	return 0;
}

static int xw_phy_core_clk_config(struct pcie_phy *phy, bool enable)
{
	int ret;

	if (!enable) {
		clk_disable_unprepare(phy->mcu_clk);
		clk_disable_unprepare(phy->mcu_32k_clk);
		clk_disable_unprepare(phy->mcu_19p2_clk);
		clk_disable_unprepare(phy->mcu_bus_clk);
		return 0;
	}

	ret = clk_prepare_enable(phy->mcu_bus_clk);
	if (ret) {
		E("Failed to enable mcu_bus_clk");
		goto MCU_BUS_CLK;
	}

	ret = clk_prepare_enable(phy->mcu_19p2_clk);
	if (ret) {
		E("Failed to enable mcu_19p2_clk");
		goto MCU_19P2_CLK;
	}

	ret = clk_prepare_enable(phy->mcu_32k_clk);
	if (ret) {
		E("Failed to enable mcu_32k_clk");
		goto MCU_32K_CLK;
	}

	ret = clk_prepare_enable(phy->mcu_clk);
	if (ret) {
		E("Failed to enable mcu_clk");
		goto MCU_CLK;
	}

	return 0;
MCU_CLK:
	clk_disable_unprepare(phy->mcu_32k_clk);
MCU_32K_CLK:
	clk_disable_unprepare(phy->mcu_19p2_clk);
MCU_19P2_CLK:
	clk_disable_unprepare(phy->mcu_bus_clk);
MCU_BUS_CLK:
	return ret;
}

#define MCORE_TCM_PROP_SIZE 2
static int32_t xw_phy_parse_tcm_info(struct pcie_phy *phy_info,
		struct device_node *np)
{
	uint64_t val[MCORE_TCM_PROP_SIZE] = { 0 };
	int32_t ret;
	resource_size_t addr;
	resource_size_t size;

	ret = of_property_read_u64_array(np, "mcore_tcm",
			val, MCORE_TCM_PROP_SIZE);
	if (ret) {
		E("get mcore_tcm property\n");
		return ret;
	}

	addr = (resource_size_t)val[0];
	size = (resource_size_t)val[1];

	phy_info->core_base = ioremap(addr, size);
	if (IS_ERR_OR_NULL(phy_info->core_base)) {
		E("mcore_tcm ioremap\n");
		return -EINVAL;
	}

	return 0;
}
#else

static inline int32_t xw_phy_get_core_clk(struct pcie_phy *phy,
		struct device_node *np)
{
	return 0;
}

static inline int xw_phy_core_clk_config(struct pcie_phy *phy, bool enable)
{
	return 0;
}

static inline int32_t xw_phy_core_start(struct pcie_phy *phy)
{
	return 0;
}

static inline int32_t xw_phy_parse_tcm_info(struct pcie_phy *phy_info,
		struct device_node *np)
{
	return 0;
}
#endif /* XW_PHY_MCU */

struct pcie_phy_ops xw_phy_ops = {
	.irq_init = xw_phy_irq_init,
	.irq_disable = xw_phy_irq_disable,
	.irq_handler = xw_phy_irq_handler,
	.phy_core_start = xw_phy_core_start,
	.phy_core_clk_config = xw_phy_core_clk_config,
	.phy_init = xw_phy_init,
	.is_phy_ready = is_xw_phy_ready,
	.phy_stat_dump = xw_phy_stat_dump,
	.phy_r = xw_phy_r,
	.phy_w = xw_phy_w,
};

/* parse rc-id from pcie_port device_node */
static int32_t xw_phy_get_host_id(struct platform_device *pdev,
		struct pcie_phy *phy_info)
{
	int32_t ret;

	ret = of_property_read_u32(pdev->dev.of_node, "rc-id", &phy_info->id);
	if (ret) {
		E("Fail to read Host id\n");
		return ret;
	}

	D("Host id: %u\n", phy_info->id);
	return 0;
}

int32_t pcie_phy_register(struct platform_device *pdev)
{
	int32_t ret;
	struct pcie_phy *phy_info = NULL;
	struct device_node *phy_np = NULL;

	I("+%s+\n", __func__);
	phy_info = devm_kzalloc(&pdev->dev, sizeof(*phy_info), GFP_KERNEL);
	if (!phy_info)
		return -ENOMEM;

	ret = xw_phy_get_host_id(pdev, phy_info);
	if (ret)
		return ret;

	phy_np = of_find_compatible_node(pdev->dev.of_node,
			NULL, "pcie,xw-phy");
	if (!phy_np) {
		E("Get PHY Node\n");
		return -ENODEV;
	}

	ret = xw_phy_get_core_clk(phy_info, phy_np);
	if (ret)
		return ret;

	ret = xw_phy_parse_tcm_info(phy_info, phy_np);
	if (ret)
		return ret;

	ret = pcie_phy_ops_register(phy_info->id, phy_info, &xw_phy_ops);
	if (ret) {
		E("register phy ops\n");
		goto CORE_TCM_BASE_ERR;
	}

	mutex_init(&phy_info->irq_mutex_lock);

	I("-%s-\n", __func__);
	return 0;

CORE_TCM_BASE_ERR:
#ifdef XW_PHY_MCU
	iounmap(phy_info->core_base);
#endif
	E("-%s-\n", __func__);
	return ret;
}

