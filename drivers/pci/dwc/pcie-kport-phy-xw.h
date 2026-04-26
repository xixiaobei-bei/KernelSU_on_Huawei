/*
 * pcie-kport-phy-xw.h
 *
 * XW PHY Driver
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

#ifndef _PCIE_KPORT_PHY_XW_H
#define _PCIE_KPORT_PHY_XW_H

#include <linux/bitops.h>
#include <linux/types.h>

/* phy_core layout */
#define CORE_PC_START_ADDR	0x280
#define CORE_RAM_SIZE		0x6000

/*
 * core memory layout
 * item		Offset	Size
 * TCM		0	32KB
 * IPC		32KB	4KB
 * UART		36KB	4KB
 * SCTRL	40KB	4KB
 */
#define PHY_CORE_SCTRL_OFFSET	(SIZE_32KB + SIZE_4KB + SIZE_4KB)

/* UCE Sctrl */
#define UCE_MCU_CTRL		0x0
#define UCE_MCU_CORE_WAIT	(1 << 3)
#define UCE_MCU_POR_PC		0x18
#define UCE_CRG_GT_EN		0x100
#define UCE_CRG_GT_BIT		0x1FF
#define UCE_CRG_RST_DIS		0x114
#define UCE_CRG_RST_BIT		0x1FF

/* The following info may diff with SoC */
/* -------------MCU BEGIN---------------- */
struct mcu_irq_info {
	uint32_t mask_reg;
	uint32_t mask_bits;
};

#define MCU_IRQ_MASK_NUM 2
/* -------------MCU END---------------- */


/* -------------ACPU BEGIN---------------- */
/* TOP LVL */
#define PCS_TOP_MASK_REG	(0x4)
#define PCS_TOP_MASK_BIT	(0x1 << 0)

#define LANE_INT_MASK_REG	(0xC04)
#define PER_LANE_MASK_BIT	(0x1 << 12)
#define PMA_SUB_MASK_REG	(0xF60)
#define PMA_SUB_MASK_BITS	(0x1FFF)

#define DFX_INT_REG_BEG		(0x200)
#define DFX_INT_REG_END		(0x21C)
#define DFX_INT_MASK_BIT	(0x1 << 1)
#define DFX_INT_STS_BIT		(0x1 << 0)

struct phy_irq_info {
	uint32_t mask_reg;
	uint32_t mask_bits;
	uint32_t status_reg;
	uint32_t status_bits;
	int32_t (*irq_handler)(struct pcie_phy *phy);
};

#define REG_DWORD_ALIGN		4

#define TOP_IRQ_NUM		7

#define DFX_IRQ_NUM		8
#define MAC_PCS_TO_REG		0x200
#define PIPE_CRG_TO_REG		0x204
#define PIPE_MAC_TO_REG		0x208
#define RXREG_CLK_SW_TO_REG	0x20C
#define RXSTD_CHG_REG		0x210
#define MAC_CLK_MISS_REG	0x214
#define PIPE_CLK_MISS_REG	0x218
#define RXREG_CLK_MISS_REG	0x21C
#define DFX_STS_BIT		BIT(0)
#define DFX_MASK_BIT		BIT(1)

#define SHARE_IRQ_NUM		4
#define SHARE_IRQ_REG		0xAA0
#define VCO_5G_ERR_STS		BIT(0)
#define VCO_5G_ERR_MK		BIT(8)
#define VCO_8G_ERR_STS		BIT(1)
#define VCO_8G_ERR_MK		BIT(9)
#define RXTERM_OVUD_STS		BIT(2)
#define RXTERM_OVUD_MK		BIT(10)
#define TXTERM_OVUD_STS		BIT(3)
#define TXTERM_OVUD_MK		BIT(11)
#define SHARE_IRQ_STS		(VCO_5G_ERR_STS | VCO_8G_ERR_STS | \
				RXTERM_OVUD_STS | TXTERM_OVUD_STS)
#define SHARE_IRQ_MASK		(VCO_5G_ERR_MK | VCO_8G_ERR_MK | \
				RXTERM_OVUD_MK | TXTERM_OVUD_MK)

#define GP_IRQ_NUM		8
#define GP_IRQ_STS_REG		0x95C
#define GP_IRQ_MASK_REG		0x960
#define GP0_BIT			BIT(0)
#define GP1_BIT			BIT(1)
#define GP2_BIT			BIT(2)
#define GP3_BIT			BIT(3)
#define GP4_BIT			BIT(4)
#define GP5_BIT			BIT(5)
#define GP6_BIT			BIT(6)
#define GP7_BIT			BIT(7)
#define GP_IRQ_BITS_ALL		(GP0_BIT | GP1_BIT | GP2_BIT | \
				GP3_BIT | GP4_BIT | GP5_BIT | \
				GP6_BIT | GP7_BIT)

#define RX_IRQ_NUM 		4
#define RX_IRQ_REG		0xC04
#define RX_SYMB_ERR_STS		BIT(0)
#define RX_SYMB_ERR_MK		BIT(8)
#define RX_SYNCHEADER_ERR_STS	BIT(1)
#define RX_SYNCHEADER_ERR_MK	BIT(9)
#define RX_EBUF_OV_STS		BIT(2)
#define RX_EBUF_OV_MK		BIT(10)
#define RX_EBUF_UD_STS		BIT(3)
#define RX_EBUF_UD_MK		BIT(11)
#define RX_IRQ_STS		(RX_SYMB_ERR_STS | RX_SYNCHEADER_ERR_STS | \
				RX_EBUF_OV_STS | RX_EBUF_UD_STS)
#define RX_IRQ_MASK		(RX_SYMB_ERR_MK | RX_SYNCHEADER_ERR_MK | \
				RX_EBUF_OV_MK | RX_EBUF_UD_MK)

#define TX_IRQ_NUM		4
#define TX_IRQ_REG		0xC00
#define TX_KCODE_ERR_STS	BIT(0)
#define TX_KCODE_ERR_MK		BIT(16)
#define TX_SYMB_ERR_STS		BIT(1)
#define TX_SYMB_ERR_MK		BIT(17)
#define TX_G3_EBUF_OV_STS	BIT(2)
#define TX_G3_EBUF_OV_MK	BIT(18)
#define TX_G3_EBUF_UD_STS	BIT(3)
#define TX_G3_EBUF_UD_MK	BIT(19)
#define TX_IRQ_STS		(TX_KCODE_ERR_STS | TX_SYMB_ERR_STS | \
				TX_G3_EBUF_OV_STS | TX_G3_EBUF_UD_STS)
#define TX_IRQ_MASK		(TX_KCODE_ERR_MK | TX_SYMB_ERR_MK | \
				TX_G3_EBUF_OV_MK | TX_G3_EBUF_UD_MK)

#define EIOS_DET_IRQ_NUM	1
#define EIOS_DET_IRQ_REG	0xE1C
#define EIOS_DET_IRQ_STS	BIT(0)
#define EIOS_DET_IRQ_MASK	BIT(16)

#define PMA_IRQ_NUM		5
#define PMA_IRQ_REG		0xF00
#define PMA_GSA_TO_STS		BIT(0)
#define PMA_GSA_TO_MK		BIT(8)
#define PMA_EQA_TO_STS		BIT(1)
#define PMA_EQA_TO_MK		BIT(9)
#define PMA_FOM_TO_STS		BIT(2)
#define PMA_FOM_TO_MK		BIT(10)
#define PMA_MCHG_PREPROC_STS	BIT(3)
#define PMA_MCHG_PREPROC_MK	BIT(11)
#define PMA_MCHG_PSTPROC_STS	BIT(4)
#define PMA_MCHG_PSTPROC_MK	BIT(12)
#define PMA_IRQ_STS		(PMA_GSA_TO_STS | PMA_EQA_TO_STS | \
				PMA_FOM_TO_STS | PMA_MCHG_PREPROC_STS | \
				PMA_MCHG_PSTPROC_STS)
#define PMA_IRQ_MASK		(PMA_GSA_TO_MK | PMA_EQA_TO_MK | \
				PMA_FOM_TO_MK | PMA_MCHG_PREPROC_MK | \
				PMA_MCHG_PSTPROC_MK)

#define CORE_TALK_IRQ_NUM	1
#define CORE_TALK_IRQ_STS_REG	0x10
#define CORE_TALK_IRQ_MASK_REG	0x10C
#define CORE_TALK_IRQ_STS	BIT(0)
#define CORE_TALK_IRQ_MASK	BIT(10)
/* -------------ACPU END---------------- */

#define PHY_PMAC_CLK_MISS_CHK_ADDR	(0x200 + 0x014)
#define PHY_PIPEC_CLK_MISS_CHK_ADDR	(0x200 + 0x018)
#define PHY_RXREG_CLK_MISS_CHK_ADDR	(0x200 + 0x01C)
#define PHY_TXIDLE_EXIT_SEL_ADDR	0x500
#define PHY_TXIDLE_ENTRY_SEL_ADDR	0x504
#define PHY_PLLCKGCTRLG1_ADDR	0xA14
#define PHY_PLLCKGCTRLG2_ADDR	0xA18
#define PHY_PLLCKGCTRLG3_ADDR	0xA1C
#define PHY_PLLCTRL_ADDR	0xA20
#define PHY_PLLVCO5GKCTRL_ADDR	0xA34
#define PHY_PLLVCO8GKCTRL_ADDR	0xA38
#define PHY_RXDETTMR_ADDR	0xA44
#define PHY_TERMKCTRL_ADDR	0xA78
#define PHY_PMACINTRCTRL_ADDR	0xAA0
#define PHY_PMACDBGCTRL_ADDR	0xAB0
#define PHY_PCS_STATUS_ADDR	(0xC00 + 0x200)
#define PHY_TXDRVCTRL_ADDR	(0xC00 + 0x314)
#define PHY_TXEQCOEFF_ADDR	(0xC00 + 0x318)
#define PHY_RXOFSTUPDTCTRL_ADDR	(0xC00 + 0x320)
#define PHY_RXUPDTCTRL_ADDR	(0xC00 + 0x32C)
#define PHY_RXGSACTRL_ADDR	(0xC00 + 0x330)
#define PHY_RXEQACTRL_ADDR	(0xC00 + 0x334)
#define PHY_RXCTRL0_ADDR		(0xC00 + 0x340)
#define PHY_RXCTRL1_ADDR		(0xC00 + 0x344)
#define PHY_PMA_DBG_ADDR	0x934
#define PHY_PMA_DBG_SEL_ADDR	0x938

#define PHY_GEN1_PLL_ICP_MASK	(0x1F << 23)
#define PHY_GEN1_PLL_ICP_SET	(0xA << 23)
#define PHY_GEN2_PLL_ICP_MASK	(0x1F << 23)
#define PHY_GEN2_PLL_ICP_SET	(0xA << 23)
#define PHY_PLL_LDO_SEL_MASK	(0xF << 4)
#define PHY_PLL_LDO_SEL_SET	(0x6 << 4)

#define PHY_TXIDLE_EXIT_SEL	0x0
#define PHY_TXIDLE_ENTRY_SEL	0x0
#define PHY_TXEQCOEFF_VAL	0x00003001
#define PHY_RX_GSARK_EN	(0x1 << 8)
#define PHY_RX_EQARK_EN	(0x1 << 9)
#define PHY_RX_MANUAL		(0xFu << 28)
#define PHY_RX0_EQ_C		(0x1 << 28)
#define PHY_RX0_GAIN_SEL	(0x1 << 29)
#define PHY_RX0_ATT		(0x1 << 30)
#define PHY_RX0_EQ_R		(0x1u << 31)
#define PHY_RX0_LDO_SEL		(0x1u << 12)
#define PHY_RXCTRL_EQA_SLE_C_MASK	(0x1F << 16)
#define PHY_RXCTRL_EQA_SLE_C_SET	(0x10 << 16)
#define PHY_RXCTRL_GSA_SLE_R_MASK	(0xF << 16)
#define PHY_RXCTRL_GSA_SLE_R_SET	(0x8 << 16)
#define PHY_RXCTRL_GSA_RX_ATT_MASK	(0x7 << 8)
#define PHY_RXCTRL_GSA_RX_ATT_SET	(0x2 << 8)
#define PHY_RX_VCM_MASK	(0x7 << 16)
#define PHY_RX_VCM_SET		(0x5 << 16)
#define PHY_RXDET_EN_TMR_MASK	(0xFF << 16)
#define PHY_RXDET_EN_TMR_SET		(0x80 << 16)
#define PHY_TX_VCM_MASK	(0x3 << 0)
#define PHY_TX_VCM_SET		(0x0 << 0)
#define PHY_CLK_MISS_CHK_MASK	(0x1FFFF << 1)
#define PHY_PMAC_CLK_MISS_VAL	(0x17D02 << 1)
#define PHY_PIPEC_CLK_MISS_VAL	(0x1FD02 << 1)
#define PHY_EXREG_CLK_MISS_VAL	(0x1FD02 << 1)
#define PHY_PMA_DBG_SEL_MASK	(0x7 << 0)
#define PHY_PMA_DBG_SEL0	(0x6 << 0)
#define PHY_PMA_DBG_SEL1	(0x1 << 0)

#define PHY_POWERON_STATUS_BIT	(0x1 << 7)
#define PHY_INIT_PROC_ERR_MASK	0xF
#define PHY_INIT_FATAL_ERR_MASK	0x1
#define PHY_RX_DBG_PORT_BIT	0x1
#define PHY_RX_TEST_MODE_MASK	(0x7 << 20)
#define PHY_RX_TEST_MODE_EQ	(0x5 << 20)
#define PHY_RX_TEST_MODE_SA	(0x6 << 20)
#define PHY_RX_TEST_MODE_GS	(0x7 << 20)
#define PHY_CLE_VCOK5G_DONE_BIT	(0x1 << 24)
#define PHY_CLE_VCOK8G_DONE_BIT	(0x1 << 24)
#define PHY_CLE_EQOFSTK_DONE_BIT	(0x1 << 24)
#define PHY_CLE_SAOFSTK_DONE_BIT	(0x1 << 25)
#define PHY_CLE_GSOFSTK_DONE_BIT	(0x1 << 26)

enum phy_test_mode {
	PHY_TEST_MODE_0 = 0x1,
	PHY_TEST_MODE_1 = 0x2,
	PHY_TEST_MODE_2 = 0x4,
	PHY_TEST_MODE_3 = 0x8,
	PHY_TEST_MODE_4 = 0x10,
	PHY_TEST_MODE_5 = 0x20,
	PHY_TEST_MODE_6 = 0x40,
};

enum phy_rate {
	PHY_RATE_GEN1 = 0x0,
	PHY_RATE_GEN2 = 0x1,
	PHY_RATE_GEN3 = 0x2,
};

#endif
