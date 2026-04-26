/*
 * soc_mad_reg.h codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _SOC_MAD_REG_H_
#define _SOC_MAD_REG_H_

#define MAD_CLK_EN 0x1C
#define MAD_ANA_EB_BPCTRL 0x48
#define MAD_MIN_CHAN_ENG 0x5C
#define MAD_INE 0x64
#define MAD_SNR_THRE 0x68
#define MAD_BAND_THRE 0x6C
#define MAD_SNR_THRE_SUM 0x70
#define MAD_SNR_MIN 0x74
#define MAD_SCALE 0x78
#define MAD_CNT_THRE 0x7C
#define MAD_SI_CTRL 0x80
#define MAD_MUX_SEL 0x8C
#define ADC_CTRL 0x90
#define BM_SRCDN_12288K_WIND_SEL 0xE0
#define PGA_CTRL 0xE8
#define PGA_DIN_DEL 0xFC
#define MAD_PLL_TIME 0x30
#define MAD_ADC_TIME 0x34
#define MAD_ANA_TIME 0x2C
#define MAD_OMIT_SAMP 0x38
#define MAD_VAD_TIME 0x40
#define MAD_SLEEP_TIME 0x44
#define MAD_BUFFER_CTRL 0x88
#define MAD_VAD_NUM 0x3C
#define CIC_CLK_SEL 0xD0
#define MAD_DATA_DIN_SEL 0xD8
#define DSP_IF_THRE2 0xC4
#define DSP_IF_THRE 0xB8
#define DMA_REQ_MASK 0xC0
#define MAD_CTRL 0x24
#define MAD_IFRAME 0x60
#define MAD_DMIC_DIV 0xA0
#define SIF_6144K_CTRL 0x94

#define AO_SC_SYS_APB_BASE 0x0

#define AO_SC_SYS_APB_SCLBINTPLL1_CTRL0_REG         (AO_SC_SYS_APB_BASE + 0xE0)  /* LBINTPLL控制寄存器0 */
#define SYS_APB_SEL_LBINTPLL_OUT_LEN          1
#define SYS_APB_SEL_LBINTPLL_OUT_OFFSET       30
#define SYS_APB_LBINTPLL_GT_LEN               1
#define SYS_APB_LBINTPLL_GT_OFFSET            29
#define SYS_APB_LBINTPLL_FBDIV_LEN            14
#define SYS_APB_LBINTPLL_FBDIV_OFFSET         15
#define SYS_APB_LBINTPLL_FOUTVCOVEN_LEN       1
#define SYS_APB_LBINTPLL_FOUTVCOVEN_OFFSET    12
#define SYS_APB_LBINTPLL_FOUTPOSTDIVEN_LEN    1
#define SYS_APB_LBINTPLL_FOUTPOSTDIVEN_OFFSET 11
#define SYS_APB_LBINTPLL_LOCKCOUNT_LEN        2
#define SYS_APB_LBINTPLL_LOCKCOUNT_OFFSET     9
#define SYS_APB_LBINTPLL_POSTDIV_LEN          4
#define SYS_APB_LBINTPLL_POSTDIV_OFFSET       5
#define SYS_APB_LBINTPLL_BYPASS_LEN           1
#define SYS_APB_LBINTPLL_BYPASS_OFFSET        1
#define SYS_APB_LBINTPLL_EN_LEN               1
#define SYS_APB_LBINTPLL_EN_OFFSET            0

#define AO_SC_SYS_APB_SCLBINTPLL1_STAT_REG          (AO_SC_SYS_APB_BASE + 0xEC)  /* LBINTPLL状态寄存器 */
#define SYS_APB_LIBINTPLL_FACLOUT_LEN        8
#define SYS_APB_LIBINTPLL_FACLOUT_OFFSET     7
#define SYS_APB_ST_CLK_LBINTPLL_OUT_LEN      1
#define SYS_APB_ST_CLK_LBINTPLL_OUT_OFFSET   3
#define SYS_APB_LIBINTPLL_BYPASS_STAT_LEN    1
#define SYS_APB_LIBINTPLL_BYPASS_STAT_OFFSET 2
#define SYS_APB_LBINTPLL_LOCK_LEN            1
#define SYS_APB_LBINTPLL_LOCK_OFFSET         1
#define SYS_APB_LBINTPLL_EN_LEN              1
#define SYS_APB_LBINTPLL_EN_OFFSET           0

#define AO_SC_SYS_APB_SCPEREN0_REG                  (AO_SC_SYS_APB_BASE + 0x160) /* 外设时钟使能寄存器0 */
#define SYS_APB_GT_CLK_SPLL_SSCG_LEN           1
#define SYS_APB_GT_CLK_SPLL_SSCG_OFFSET        31
#define SYS_APB_GT_CLK_AOBUS_LEN               1
#define SYS_APB_GT_CLK_AOBUS_OFFSET            29
#define SYS_APB_GT_PCLK_BBPDRX_LEN             1
#define SYS_APB_GT_PCLK_BBPDRX_OFFSET          28
#define SYS_APB_GT_CLK_ASP_TCXO_LEN            1
#define SYS_APB_GT_CLK_ASP_TCXO_OFFSET         27
#define SYS_APB_GT_PCLK_AO_GPIO6_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO6_OFFSET        25
#define SYS_APB_GT_CLK_SCI1_LEN                1
#define SYS_APB_GT_CLK_SCI1_OFFSET             24
#define SYS_APB_GT_CLK_SCI0_LEN                1
#define SYS_APB_GT_CLK_SCI0_OFFSET             23
#define SYS_APB_GT_PCLK_AO_GPIO5_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO5_OFFSET        22
#define SYS_APB_GT_PCLK_AO_GPIO4_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO4_OFFSET        21
#define SYS_APB_GT_CLK_SYSCNT_LEN              1
#define SYS_APB_GT_CLK_SYSCNT_OFFSET           20
#define SYS_APB_GT_PCLK_SYSCNT_LEN             1
#define SYS_APB_GT_PCLK_SYSCNT_OFFSET          19
#define SYS_APB_GT_CLK_JTAG_AUTH_LEN           1
#define SYS_APB_GT_CLK_JTAG_AUTH_OFFSET        18
#define SYS_APB_GT_CLK_MAD_ACPU_LEN            1
#define SYS_APB_GT_CLK_MAD_ACPU_OFFSET         17
#define SYS_APB_GT_CLK_ASP_CODEC_BACKUP_LEN    1
#define SYS_APB_GT_CLK_ASP_CODEC_BACKUP_OFFSET 16
#define SYS_APB_GT_PCLK_AO_IOC_LEN             1
#define SYS_APB_GT_PCLK_AO_IOC_OFFSET          15
#define SYS_APB_GT_PCLK_AO_GPIO3_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO3_OFFSET        14
#define SYS_APB_GT_PCLK_AO_GPIO2_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO2_OFFSET        13
#define SYS_APB_GT_PCLK_AO_GPIO1_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO1_OFFSET        12
#define SYS_APB_GT_PCLK_AO_GPIO0_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO0_OFFSET        11
#define SYS_APB_GT_CLK_TIMER3_LEN              1
#define SYS_APB_GT_CLK_TIMER3_OFFSET           10
#define SYS_APB_GT_PCLK_TIMER3_LEN             1
#define SYS_APB_GT_PCLK_TIMER3_OFFSET          9
#define SYS_APB_GT_CLK_TIMER2_LEN              1
#define SYS_APB_GT_CLK_TIMER2_OFFSET           8
#define SYS_APB_GT_PCLK_TIMER2_LEN             1
#define SYS_APB_GT_PCLK_TIMER2_OFFSET          7
#define SYS_APB_GT_CLK_FLL_TEST_SRC_LEN        1
#define SYS_APB_GT_CLK_FLL_TEST_SRC_OFFSET     6
#define SYS_APB_GT_CLK_MAD_32K_LEN             1
#define SYS_APB_GT_CLK_MAD_32K_OFFSET          5
#define SYS_APB_GT_CLK_TIMER0_LEN              1
#define SYS_APB_GT_CLK_TIMER0_OFFSET           4
#define SYS_APB_GT_PCLK_TIMER0_LEN             1
#define SYS_APB_GT_PCLK_TIMER0_OFFSET          3
#define SYS_APB_GT_PCLK_RTC1_LEN               1
#define SYS_APB_GT_PCLK_RTC1_OFFSET            2
#define SYS_APB_GT_PCLK_RTC_LEN                1
#define SYS_APB_GT_PCLK_RTC_OFFSET             1
#define SYS_APB_GT_CLK_REF_CRC_LEN             1
#define SYS_APB_GT_CLK_REF_CRC_OFFSET          0

#define AO_SC_SYS_APB_SCCLKDIV1_REG                 (AO_SC_SYS_APB_BASE + 0x254) /* 时钟分频比控制寄存器1 */
#define SYS_APB_SCCLKDIV1_MSK_LEN           16
#define SYS_APB_SCCLKDIV1_MSK_OFFSET        16
#define SYS_APB_SC_GT_CLK_HIFD_FLL_LEN      1
#define SYS_APB_SC_GT_CLK_HIFD_FLL_OFFSET   15
#define SYS_APB_SEL_CLK_FLL_TEST_SRC_LEN    3
#define SYS_APB_SEL_CLK_FLL_TEST_SRC_OFFSET 12
#define SYS_APB_DIV_PCIE_AUX_LEN            4
#define SYS_APB_DIV_PCIE_AUX_OFFSET         8
#define SYS_APB_SC_GT_CLK_PCIE_AUX_LEN      1
#define SYS_APB_SC_GT_CLK_PCIE_AUX_OFFSET   7
#define SYS_APB_SC_GT_CLK_MAD_SPLL_LEN      1
#define SYS_APB_SC_GT_CLK_MAD_SPLL_OFFSET   6
#define SYS_APB_DIV_AOBUS_LEN               6
#define SYS_APB_DIV_AOBUS_OFFSET            0

#define AO_SC_SYS_APB_SCPEREN3_SEC_REG              (AO_SC_SYS_APB_BASE + 0xC10) /* 外设时钟使能寄存器3（安全） */
#define SYS_APB_GT_CLK_MAD_LPM3_LEN            1
#define SYS_APB_GT_CLK_MAD_LPM3_OFFSET         2
#define SYS_APB_GT_CLK_ASP_CODEC_LPM3_LEN      1
#define SYS_APB_GT_CLK_ASP_CODEC_LPM3_OFFSET   1
#define SYS_APB_GT_CLK_ASP_SUBSYS_LPMCU_LEN    1
#define SYS_APB_GT_CLK_ASP_SUBSYS_LPMCU_OFFSET 0

#define AO_SC_SYS_APB_SCPEREN2_REG                  (AO_SC_SYS_APB_BASE + 0x190) /* 外设时钟使能寄存器2 */
#define SYS_APB_GT_CLK_MAD_AXI_LEN         1
#define SYS_APB_GT_CLK_MAD_AXI_OFFSET      31
#define SYS_APB_FLL_EN_MDM_LEN             1
#define SYS_APB_FLL_EN_MDM_OFFSET          29
#define SYS_APB_FLL_EN_CPU_LEN             1
#define SYS_APB_FLL_EN_CPU_OFFSET          28
#define SYS_APB_SPLL_EN_MDM_LEN            1
#define SYS_APB_SPLL_EN_MDM_OFFSET         27
#define SYS_APB_SPLL_EN_CPU_LEN            1
#define SYS_APB_SPLL_EN_CPU_OFFSET         26
#define SYS_APB_GT_PCLKDBG_TO_IOMCU_LEN    1
#define SYS_APB_GT_PCLKDBG_TO_IOMCU_OFFSET 24
#define SYS_APB_GT_PCLK_AO_WD_LEN          1
#define SYS_APB_GT_PCLK_AO_WD_OFFSET       21
#define SYS_APB_FLL_GT_MDM_LEN             1
#define SYS_APB_FLL_GT_MDM_OFFSET          20
#define SYS_APB_FLL_GT_CPU_LEN             1
#define SYS_APB_FLL_GT_CPU_OFFSET          19
#define SYS_APB_FLL_BP_MDM_LEN             1
#define SYS_APB_FLL_BP_MDM_OFFSET          18
#define SYS_APB_FLL_BP_CPU_LEN             1
#define SYS_APB_FLL_BP_CPU_OFFSET          17
#define SYS_APB_SPLL_GT_MODEM_LEN          1
#define SYS_APB_SPLL_GT_MODEM_OFFSET       16
#define SYS_APB_SPLL_GT_CPU_LEN            1
#define SYS_APB_SPLL_GT_CPU_OFFSET         15
#define SYS_APB_FLL_GT_SOFT_LEN            1
#define SYS_APB_FLL_GT_SOFT_OFFSET         14
#define SYS_APB_SPLL_GT_BBE16_LEN          1
#define SYS_APB_SPLL_GT_BBE16_OFFSET       13
#define SYS_APB_SPLL_GT_CBBE16_LEN         1
#define SYS_APB_SPLL_GT_CBBE16_OFFSET      12
#define SYS_APB_SPLL_EN_BBE16_LEN          1
#define SYS_APB_SPLL_EN_BBE16_OFFSET       11
#define SYS_APB_SPLL_EN_CBBE16_LEN         1
#define SYS_APB_SPLL_EN_CBBE16_OFFSET      10
#define SYS_APB_GT_CLK_HIFD_TCXO_LEN       1
#define SYS_APB_GT_CLK_HIFD_TCXO_OFFSET    2
#define SYS_APB_GT_CLK_HIFD_32K_LEN        1
#define SYS_APB_GT_CLK_HIFD_32K_OFFSET     1
#define SYS_APB_GT_CLK_AONOC2FDBUS_LEN     1
#define SYS_APB_GT_CLK_AONOC2FDBUS_OFFSET  0

#define AO_SC_SYS_APB_SCPERRSTEN1_SEC_REG           (AO_SC_SYS_APB_BASE + 0xA50) /* 外设软复位使能寄存器1（安全） */
#define SYS_APB_IP_RST_HIFD_CRG_LEN          1
#define SYS_APB_IP_RST_HIFD_CRG_OFFSET       5
#define SYS_APB_IP_RST_HIFD_LEN              1
#define SYS_APB_IP_RST_HIFD_OFFSET           4
#define SYS_APB_IP_RST_ASP_CFG_LEN           1
#define SYS_APB_IP_RST_ASP_CFG_OFFSET        3
#define SYS_APB_IP_RST_MAD_LEN               1
#define SYS_APB_IP_RST_MAD_OFFSET            2
#define SYS_APB_IP_RST_ASP_SUBSYS_CRG_LEN    1
#define SYS_APB_IP_RST_ASP_SUBSYS_CRG_OFFSET 1
#define SYS_APB_IP_RST_ASP_SUBSYS_LEN        1
#define SYS_APB_IP_RST_ASP_SUBSYS_OFFSET     0

#define AO_SC_SYS_APB_SCPERDIS0_REG                 (AO_SC_SYS_APB_BASE + 0x164) /* 外设时钟禁止寄存器0 */
#define SYS_APB_GT_CLK_SPLL_SSCG_LEN           1
#define SYS_APB_GT_CLK_SPLL_SSCG_OFFSET        31
#define SYS_APB_GT_CLK_AOBUS_LEN               1
#define SYS_APB_GT_CLK_AOBUS_OFFSET            29
#define SYS_APB_GT_PCLK_BBPDRX_LEN             1
#define SYS_APB_GT_PCLK_BBPDRX_OFFSET          28
#define SYS_APB_GT_CLK_ASP_TCXO_LEN            1
#define SYS_APB_GT_CLK_ASP_TCXO_OFFSET         27
#define SYS_APB_GT_PCLK_AO_GPIO6_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO6_OFFSET        25
#define SYS_APB_GT_CLK_SCI1_LEN                1
#define SYS_APB_GT_CLK_SCI1_OFFSET             24
#define SYS_APB_GT_CLK_SCI0_LEN                1
#define SYS_APB_GT_CLK_SCI0_OFFSET             23
#define SYS_APB_GT_PCLK_AO_GPIO5_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO5_OFFSET        22
#define SYS_APB_GT_PCLK_AO_GPIO4_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO4_OFFSET        21
#define SYS_APB_GT_CLK_SYSCNT_LEN              1
#define SYS_APB_GT_CLK_SYSCNT_OFFSET           20
#define SYS_APB_GT_PCLK_SYSCNT_LEN             1
#define SYS_APB_GT_PCLK_SYSCNT_OFFSET          19
#define SYS_APB_GT_CLK_JTAG_AUTH_LEN           1
#define SYS_APB_GT_CLK_JTAG_AUTH_OFFSET        18
#define SYS_APB_GT_CLK_MAD_ACPU_LEN            1
#define SYS_APB_GT_CLK_MAD_ACPU_OFFSET         17
#define SYS_APB_GT_CLK_ASP_CODEC_BACKUP_LEN    1
#define SYS_APB_GT_CLK_ASP_CODEC_BACKUP_OFFSET 16
#define SYS_APB_GT_PCLK_AO_IOC_LEN             1
#define SYS_APB_GT_PCLK_AO_IOC_OFFSET          15
#define SYS_APB_GT_PCLK_AO_GPIO3_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO3_OFFSET        14
#define SYS_APB_GT_PCLK_AO_GPIO2_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO2_OFFSET        13
#define SYS_APB_GT_PCLK_AO_GPIO1_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO1_OFFSET        12
#define SYS_APB_GT_PCLK_AO_GPIO0_LEN           1
#define SYS_APB_GT_PCLK_AO_GPIO0_OFFSET        11
#define SYS_APB_GT_CLK_TIMER3_LEN              1
#define SYS_APB_GT_CLK_TIMER3_OFFSET           10
#define SYS_APB_GT_PCLK_TIMER3_LEN             1
#define SYS_APB_GT_PCLK_TIMER3_OFFSET          9
#define SYS_APB_GT_CLK_TIMER2_LEN              1
#define SYS_APB_GT_CLK_TIMER2_OFFSET           8
#define SYS_APB_GT_PCLK_TIMER2_LEN             1
#define SYS_APB_GT_PCLK_TIMER2_OFFSET          7
#define SYS_APB_GT_CLK_FLL_TEST_SRC_LEN        1
#define SYS_APB_GT_CLK_FLL_TEST_SRC_OFFSET     6
#define SYS_APB_GT_CLK_MAD_32K_LEN             1
#define SYS_APB_GT_CLK_MAD_32K_OFFSET          5
#define SYS_APB_GT_CLK_TIMER0_LEN              1
#define SYS_APB_GT_CLK_TIMER0_OFFSET           4
#define SYS_APB_GT_PCLK_TIMER0_LEN             1
#define SYS_APB_GT_PCLK_TIMER0_OFFSET          3
#define SYS_APB_GT_PCLK_RTC1_LEN               1
#define SYS_APB_GT_PCLK_RTC1_OFFSET            2
#define SYS_APB_GT_PCLK_RTC_LEN                1
#define SYS_APB_GT_PCLK_RTC_OFFSET             1
#define SYS_APB_GT_CLK_REF_CRC_LEN             1
#define SYS_APB_GT_CLK_REF_CRC_OFFSET          0

#define AO_SC_SYS_APB_SCPERDIS3_SEC_REG             (AO_SC_SYS_APB_BASE + 0xC14) /* 外设时钟禁止寄存器3（安全） */
#define SYS_APB_GT_CLK_MAD_LPM3_LEN            1
#define SYS_APB_GT_CLK_MAD_LPM3_OFFSET         2
#define SYS_APB_GT_CLK_ASP_CODEC_LPM3_LEN      1
#define SYS_APB_GT_CLK_ASP_CODEC_LPM3_OFFSET   1
#define SYS_APB_GT_CLK_ASP_SUBSYS_LPMCU_LEN    1
#define SYS_APB_GT_CLK_ASP_SUBSYS_LPMCU_OFFSET 0

#define AO_SC_SYS_APB_SCPERDIS2_REG                 (AO_SC_SYS_APB_BASE + 0x194) /* 外设时钟禁止寄存器2 */
#define SYS_APB_GT_CLK_MAD_AXI_LEN         1
#define SYS_APB_GT_CLK_MAD_AXI_OFFSET      31
#define SYS_APB_FLL_EN_MDM_LEN             1
#define SYS_APB_FLL_EN_MDM_OFFSET          29
#define SYS_APB_FLL_EN_CPU_LEN             1
#define SYS_APB_FLL_EN_CPU_OFFSET          28
#define SYS_APB_SPLL_EN_MDM_LEN            1
#define SYS_APB_SPLL_EN_MDM_OFFSET         27
#define SYS_APB_SPLL_EN_CPU_LEN            1
#define SYS_APB_SPLL_EN_CPU_OFFSET         26
#define SYS_APB_GT_PCLKDBG_TO_IOMCU_LEN    1
#define SYS_APB_GT_PCLKDBG_TO_IOMCU_OFFSET 24
#define SYS_APB_GT_PCLK_AO_WD_LEN          1
#define SYS_APB_GT_PCLK_AO_WD_OFFSET       21
#define SYS_APB_FLL_GT_MDM_LEN             1
#define SYS_APB_FLL_GT_MDM_OFFSET          20
#define SYS_APB_FLL_GT_CPU_LEN             1
#define SYS_APB_FLL_GT_CPU_OFFSET          19
#define SYS_APB_FLL_BP_MDM_LEN             1
#define SYS_APB_FLL_BP_MDM_OFFSET          18
#define SYS_APB_FLL_BP_CPU_LEN             1
#define SYS_APB_FLL_BP_CPU_OFFSET          17
#define SYS_APB_SPLL_GT_MODEM_LEN          1
#define SYS_APB_SPLL_GT_MODEM_OFFSET       16
#define SYS_APB_SPLL_GT_CPU_LEN            1
#define SYS_APB_SPLL_GT_CPU_OFFSET         15
#define SYS_APB_FLL_GT_SOFT_LEN            1
#define SYS_APB_FLL_GT_SOFT_OFFSET         14
#define SYS_APB_SPLL_GT_BBE16_LEN          1
#define SYS_APB_SPLL_GT_BBE16_OFFSET       13
#define SYS_APB_SPLL_GT_CBBE16_LEN         1
#define SYS_APB_SPLL_GT_CBBE16_OFFSET      12
#define SYS_APB_SPLL_EN_BBE16_LEN          1
#define SYS_APB_SPLL_EN_BBE16_OFFSET       11
#define SYS_APB_SPLL_EN_CBBE16_LEN         1
#define SYS_APB_SPLL_EN_CBBE16_OFFSET      10
#define SYS_APB_GT_CLK_HIFD_TCXO_LEN       1
#define SYS_APB_GT_CLK_HIFD_TCXO_OFFSET    2
#define SYS_APB_GT_CLK_HIFD_32K_LEN        1
#define SYS_APB_GT_CLK_HIFD_32K_OFFSET     1
#define SYS_APB_GT_CLK_AONOC2FDBUS_LEN     1
#define SYS_APB_GT_CLK_AONOC2FDBUS_OFFSET  0

#define AO_SC_SYS_APB_SCPERRSTDIS1_SEC_REG          (AO_SC_SYS_APB_BASE + 0xA54) /* 外设软复位撤离寄存器1（安全） */
#define SYS_APB_IP_RST_HIFD_CRG_LEN          1
#define SYS_APB_IP_RST_HIFD_CRG_OFFSET       5
#define SYS_APB_IP_RST_HIFD_LEN              1
#define SYS_APB_IP_RST_HIFD_OFFSET           4
#define SYS_APB_IP_RST_ASP_CFG_LEN           1
#define SYS_APB_IP_RST_ASP_CFG_OFFSET        3
#define SYS_APB_IP_RST_MAD_LEN               1
#define SYS_APB_IP_RST_MAD_OFFSET            2
#define SYS_APB_IP_RST_ASP_SUBSYS_CRG_LEN    1
#define SYS_APB_IP_RST_ASP_SUBSYS_CRG_OFFSET 1
#define SYS_APB_IP_RST_ASP_SUBSYS_LEN        1
#define SYS_APB_IP_RST_ASP_SUBSYS_OFFSET     0

#define AO_SC_SYS_APB_SCCLKDIV6_REG                 (AO_SC_SYS_APB_BASE + 0x268) /* 时钟分频比控制寄存器6 */
#define SYS_APB_SCCLKDIV6_MSK_LEN      16
#define SYS_APB_SCCLKDIV6_MSK_OFFSET   16
#define SYS_APB_DIV_AOBUS_FLL_LEN      5
#define SYS_APB_DIV_AOBUS_FLL_OFFSET   11
#define SYS_APB_DIV_IOMCU_FLL_LEN      2
#define SYS_APB_DIV_IOMCU_FLL_OFFSET   9
#define SYS_APB_SEL_MAD_MUX_PRE_LEN    1
#define SYS_APB_SEL_MAD_MUX_PRE_OFFSET 8
#define SYS_APB_DIV_SYSCNT_LEN         4
#define SYS_APB_DIV_SYSCNT_OFFSET      4
#define SYS_APB_DIV_MAD_SPLL_LEN       4
#define SYS_APB_DIV_MAD_SPLL_OFFSET    0

#define AO_SC_SYS_APB_SCCLKDIV8_REG                 (AO_SC_SYS_APB_BASE + 0x270) /* 时钟分频比控制寄存器8 */
#define SYS_APB_SCCLKDIV8_MSK_LEN         16
#define SYS_APB_SCCLKDIV8_MSK_OFFSET      16
#define SYS_APB_SEL_SPMI_MST_LEN          1
#define SYS_APB_SEL_SPMI_MST_OFFSET       15
#define SYS_APB_DIV_SPMI_MST_LEN          6
#define SYS_APB_DIV_SPMI_MST_OFFSET       9
#define SYS_APB_SC_GT_CLK_SPMI_MST_LEN    1
#define SYS_APB_SC_GT_CLK_SPMI_MST_OFFSET 8
#define SYS_APB_SEL_MAD_MUX_LEN           1
#define SYS_APB_SEL_MAD_MUX_OFFSET        7
#define SYS_APB_SC_GT_CLK_IOPERI_LEN      1
#define SYS_APB_SC_GT_CLK_IOPERI_OFFSET   6
#define SYS_APB_DIV_IOPERI_LEN            6
#define SYS_APB_DIV_IOPERI_OFFSET         0

#define AO_SC_SYS_APB_SC_MEM_CTRL_SD_REG            (AO_SC_SYS_APB_BASE + 0x3A8) /* 时钟分频比控制寄存器3 */
#define SYS_APB_SC_MEM_CTRL_SD_MSK_LEN               16
#define SYS_APB_SC_MEM_CTRL_SD_MSK_OFFSET            16
#define SYS_APB_ASP_IMGLD_FLAG_LEN                   1
#define SYS_APB_ASP_IMGLD_FLAG_OFFSET                15
#define SYS_APB_HIFI_LOAD_IMAGE_FLAG_LEN             1
#define SYS_APB_HIFI_LOAD_IMAGE_FLAG_OFFSET          14
#define SYS_APB_SC_SPI3_SD_IN_LEN                    1
#define SYS_APB_SC_SPI3_SD_IN_OFFSET                 3
#define SYS_APB_SC_MEM_CTRL_SD_MAD_LEN               1
#define SYS_APB_SC_MEM_CTRL_SD_MAD_OFFSET            2
#define SYS_APB_SC_MEM_CTRL_SD_ASP_LEN               1
#define SYS_APB_SC_MEM_CTRL_SD_ASP_OFFSET            1
#define SYS_APB_MDDRC_SYSCACHE_RETENTION_FLAG_LEN    1
#define SYS_APB_MDDRC_SYSCACHE_RETENTION_FLAG_OFFSET 0

#endif

