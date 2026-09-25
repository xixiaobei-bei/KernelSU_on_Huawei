/*
 * hi64xx_regs.h
 *
 * codec regs for hi64xx driver
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HI64XX_REGS_H__
#define __HI64XX_REGS_H__

/* page base addr def begin */
#define CODEC_BASE_ADDR    0x20000000
#define BASE_ADDR_PAGE_CFG 0x7000
#define BASE_ADDR_PAGE_IO  0x1000
#define BASE_ADDR_PAGE_DIG 0x7200
#define BASE_ADDR_PAGE_MM  0xD000

/* page base addr def end */
#define HI64XX_VERSION_CS 0x11
#define HI64XX_VERSION_ES 0x10
#define HI64XX_VERSION_REG  0x20007000
#define HI64XX_CHIP_ID_REG0 0X20007092
#define HI64XX_CHIP_ID_REG1 0x20007093
#define HI64XX_CHIP_ID_REG2 0x20007094
#define HI64XX_CHIP_ID_REG3 0x20007095

#define HI64XX_HS_PLUGIN_BIT        6
#define HI64XX_VAD_BIT              4
#define HI64XX_MBHC_VREF_BIT        7
#define HI64XX_MICBIAS_ECO_ON_BIT   0
#define HI64XX_PLUGIN_IRQ_BIT       3
#define HI64XX_SARADC_CMP_BIT       7
#define HI64XX_SARADC_RD_BIT        2
#define HI64XX_MASK_PLL_UNLOCK_BIT  4
#define HI64XX_PLL_RST_BIT          2
#define HI64XX_SARADC_PD_BIT        6
#define HI64XX_SAR_START_BIT        5
#define HI64XX_MBHC_ON_BIT          4
#define HI64XX_SARADC_RDY_BIT       0
#define PLL_LOCK_VALUE              0x02
/* SARADC */
#define HI64XX_SAR_AUTO_EN_BIT              0
#define HI64XX_MBHC_MASK_PLUGOUT_BIT        7
#define HI64XX_MBHC_MASK_PLUGIN_BIT         6
#define HI64XX_MBHC_MASK_ECODOWN_BIT        5
#define HI64XX_MBHC_MASK_ECOUP_BIT          4
#define HI64XX_MBHC_MASK_DOWN2_BIT          3
#define HI64XX_MBHC_MASK_UP2_BIT            2
#define HI64XX_MBHC_MASK_DOWN_BIT           1
#define HI64XX_MBHC_MASK_UP_BIT             0
#define PLL_PD_VALUE                        0x02
/* sctrl_reg */
#define HI64XX_VAD_INT_SET                  0x20003000

/* cfg reg begin */
/* CFG CLK */
#define HI64XX_CFG_REG_CLK_CTRL_REG        (BASE_ADDR_PAGE_CFG + 0x038)
#define HI64XX_CFG_REG_CLK_SEL_BIT         0
#define HI64XX_CFG_REG_CLK_SW_REQ_BIT      1
#define HI64XX_CFG_REG_TMUX_CLKB_BP_BIT    2
#define HI64XX_CFG_REG_CLK_STATUS          (BASE_ADDR_PAGE_CFG + 0x039)

#define HI64XX_SC_MAD_CTRL0                (BASE_ADDR_PAGE_CFG + 0x013)
#define HI64XX_CODEC_MAINPGA_SEL           (BASE_ADDR_PAGE_CFG + 0x0AA)
#define HI64XX_CODEC_MAINPGA_SEL_BIT       1

#define HI64XX_DSP_SW_RST_REQ              (BASE_ADDR_PAGE_CFG + 0x001)
#define HI64XX_DSP_SC_DSP_CTRL0            (BASE_ADDR_PAGE_CFG + 0x004)
#define HI64XX_DSP_SC_MAD_CTRL0            (BASE_ADDR_PAGE_CFG + 0x013)
#define HI64XX_DSP_RAM2AXI_CTRL            (BASE_ADDR_PAGE_CFG + 0x020)
#define HI64XX_SLIM_CTRL1                  (BASE_ADDR_PAGE_CFG + 0x031)
#define HI64XX_AUDIO_CLK_EN                (BASE_ADDR_PAGE_CFG + 0x040)
#define HI64XX_S2_DP_CLK_EN                (BASE_ADDR_PAGE_CFG + 0x042)

#define HI64XX_DSP_DAC_DP_CLK_EN_1         (BASE_ADDR_PAGE_CFG + 0x047)
#define HI64XX_DSP_LP_CTRL                 (BASE_ADDR_PAGE_CFG + 0x050)
#define HI64XX_AXI_CSYSREQ_BIT             4
#define HI64XX_DSP_CLK_CFG                 (BASE_ADDR_PAGE_CFG + 0x052)
#define HI64XX_DSP_HIFI_DIV_NUM_START_BIT  0
#define HI64XX_DSP_HIFI_DIV_NUM_END_BIT    3
#define HI64XX_DSP_OCDHALTON_RST           (BASE_ADDR_PAGE_CFG + 0x5D)
#define HI64XX_DSP_DEBUG_RST               (BASE_ADDR_PAGE_CFG + 0x5E)

#define HI64XX_DSP_APB_CLK_CFG             (BASE_ADDR_PAGE_CFG + 0x053)
#define HI64XX_DSP_DSP_NMI                 (BASE_ADDR_PAGE_CFG + 0x06d)
#define HI64XX_DSP_DSP_STATUS0             (BASE_ADDR_PAGE_CFG + 0x06e)
#define HI64XX_DSP_CMD_STATUS_VLD          (BASE_ADDR_PAGE_CFG + 0x078)
#define HI64XX_DSP_CMD_STATUS              (BASE_ADDR_PAGE_CFG + 0x079)
/* DSP SC */
#define HI64XX_IRQ_SC_DSP_CTRL0            (BASE_ADDR_PAGE_CFG + 0x004)
#define HI64XX_CODEC_ANA_PLL               (BASE_ADDR_PAGE_CFG + 0x0F6)


#define HI64XX_AP_GPIO0_SEL                (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x027)
#define HI64XX_REG_IRQ_0                   (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x014)
#define HI64XX_REG_IRQ_1                   (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x015)
#define HI64XX_REG_IRQ_2                   (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x016)
#define HI64XX_REG_IRQ_3                   (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x02D)
#define HI64XX_REG_IRQM_0                  (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x017)
#define HI64XX_REG_IRQM_1                  (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x018)
#define HI64XX_REG_IRQM_2                  (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x019)
#define HI64XX_REG_IRQM_3                  (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x02C)
#define HI64XX_REG_AXI_DLOCK_IRQ_1         (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x01E)
#define HI64XX_REG_AXI_DLOCK_IRQ_2         (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x01F)
#define HI64XX_REG_WRITE_DSP_STATUS        (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x021)
#define HI64XX_REG_READ_DSP_STATUS         (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + 0x022)
/* cfg reg end */

/* reg bit musk */
#define HI64XX_WRITE_DSP_STATUS_BIT_MUSK   0x10
#define HI64XX_READ_DSP_STATUS_BIT_MUSK    0x10
#define HI64XX_AXI_DLOCK_IRQ_BIT_MUSK_1    0xF3
#define HI64XX_AXI_DLOCK_IRQ_BIT_MUSK_2    0x1F

/* io reg begin */
#define HI64XX_DSP_IOS_AF_CTRL0            (BASE_ADDR_PAGE_IO + 0x100)
#define HI64XX_DSP_IOS_IOM_I2S2_SDO        (BASE_ADDR_PAGE_IO + 0x238)
#define HI64XX_DSP_IOS_IOM_UART_TXD        (BASE_ADDR_PAGE_IO + 0x278)

/* io reg end */

/* dig reg begin */
#define HI64XX_SC_S4_IF_L                  (BASE_ADDR_PAGE_DIG + 0x006)
#define HI64XX_SC_CODEC_MUX_SEL3_0         (BASE_ADDR_PAGE_DIG + 0x014)
#define HI64XX_SC_S1_SRC_LR_CTRL_M         (BASE_ADDR_PAGE_DIG + 0x037)
#define HI64XX_SC_S2_SRC_LR_CTRL_M         (BASE_ADDR_PAGE_DIG + 0x049)
#define HI64XX_SC_S3_SRC_LR_CTRL_M         (BASE_ADDR_PAGE_DIG + 0x058)
#define HI64XX_SC_S4_SRC_LR_CTRL_M         (BASE_ADDR_PAGE_DIG + 0x067)
#define HI64XX_SC_MISC_SRC_CTRL_H          (BASE_ADDR_PAGE_DIG + 0x088)
#define HI64XX_SC_FS_S1_CTRL_H             (BASE_ADDR_PAGE_DIG + 0x0AF)
#define HI64XX_SC_FS_S2_CTRL_H             (BASE_ADDR_PAGE_DIG + 0x0B1)
#define HI64XX_SC_FS_S3_CTRL_H             (BASE_ADDR_PAGE_DIG + 0x0B3)
#define HI64XX_SC_FS_S4_CTRL_L             (BASE_ADDR_PAGE_DIG + 0x0B4)
#define HI64XX_SC_FS_S4_CTRL_H             (BASE_ADDR_PAGE_DIG + 0x0B5)
#define HI64XX_SC_FS_MISC_CTRL             (BASE_ADDR_PAGE_DIG + 0x0B6)
#define HI64XX_SC_FS_SELC_TRL              (BASE_ADDR_PAGE_DIG + 0x0F2)
#define HI64XX_CODEC_DP_CLK_EN             (BASE_ADDR_PAGE_DIG + 0x1D8)

/* dig reg end */

#endif

