/*
 * da_separate_v5_driver codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_V5_PMU_REG_DEF_H__
#define __DA_SEPARATE_V5_PMU_REG_DEF_H__

/* PMU_CODEC Base address of Module's Register */
#define DA_SEPARATE_V5_PMU_CODEC_BASE                       (0x0)

/******************************************************************************/
/*                      DA_SEPARATE_V5 PMU_CODEC Registers' Definitions                            */
/******************************************************************************/

#define CLK_EN_CFG_REG         (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC000) /* 时钟使能配置寄存器。 */
#define DAC_ULTR_EN_LEN     1
#define DAC_ULTR_EN_OFFSET  7
#define CODEC_ANA_EN_LEN    1
#define CODEC_ANA_EN_OFFSET 6
#define SIF_EN_LEN          1
#define SIF_EN_OFFSET       5
#define ADC_MIC3_EN_LEN     1
#define ADC_MIC3_EN_OFFSET  4
#define ADCR_EN_LEN         1
#define ADCR_EN_OFFSET      3
#define ADCL_EN_LEN         1
#define ADCL_EN_OFFSET      2
#define DACR_EN_LEN         1
#define DACR_EN_OFFSET      1
#define DACL_EN_LEN         1
#define DACL_EN_OFFSET      0

#define CLK_EDGE_CFG_REG       (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC001) /* 数模接口时钟沿选择。 */
#define DAC_ULTR_CLK_EDGE_SEL_LEN    1
#define DAC_ULTR_CLK_EDGE_SEL_OFFSET 5
#define ADC_MIC3_CLK_EDGE_SEL_LEN    1
#define ADC_MIC3_CLK_EDGE_SEL_OFFSET 4
#define ADCR_CLK_EDGE_SEL_LEN        1
#define ADCR_CLK_EDGE_SEL_OFFSET     3
#define ADCL_CLK_EDGE_SEL_LEN        1
#define ADCL_CLK_EDGE_SEL_OFFSET     2
#define DACR_CLK_EDGE_SEL_LEN        1
#define DACR_CLK_EDGE_SEL_OFFSET     1
#define DACL_CLK_EDGE_SEL_LEN        1
#define DACL_CLK_EDGE_SEL_OFFSET     0

#define SIF_LOOPBACK_CFG_REG   (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC002) /* SIF接口环回控制。 */
#define ADC_LOOPBACK_LEN    2
#define ADC_LOOPBACK_OFFSET 2
#define DAC_LOOPBACK_LEN    2
#define DAC_LOOPBACK_OFFSET 0

#define DAC_CHAN_CTRL_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC003) /* DAC通路配置。 */
#define DAC_ULTR_DWA_EN_LEN    1
#define DAC_ULTR_DWA_EN_OFFSET 6
#define DACR_DIN_SEL_LEN       2
#define DACR_DIN_SEL_OFFSET    4
#define DACL_DIN_SEL_LEN       2
#define DACL_DIN_SEL_OFFSET    2
#define DACR_DWA_EN_LEN        1
#define DACR_DWA_EN_OFFSET     1
#define DACL_DWA_EN_LEN        1
#define DACL_DWA_EN_OFFSET     0

#define ADC_CHAN_CTRL_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC004) /* ADC通路配置。 */
#define ADC_MIC3_DOUT_SEL_LEN    2
#define ADC_MIC3_DOUT_SEL_OFFSET 4
#define ADCR_DOUT_SEL_LEN        2
#define ADCR_DOUT_SEL_OFFSET     2
#define ADCL_DOUT_SEL_LEN        2
#define ADCL_DOUT_SEL_OFFSET     0

#define SIF_MAD_REG            (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC005) /* ADC通路配置。 */
#define MAD_CIC_CLKEN_LEN       1
#define MAD_CIC_CLKEN_OFFSET    4
#define SIF_MAD_ADC_EN_LEN      1
#define SIF_MAD_ADC_EN_OFFSET   3
#define SIF_MAD_EN_LEN          1
#define SIF_MAD_EN_OFFSET       2
#define DAC_ULTR_DIN_SEL_LEN    2
#define DAC_ULTR_DIN_SEL_OFFSET 0

#define SIF_MAD_CIC_GAIN_REG   (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC006) /* CIC增益配置。 */
#define MAD_CIC_GAIN_LEN    6
#define MAD_CIC_GAIN_OFFSET 0

#define ANA_IRQ_SIG_STAT_REG   (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC007) /* 模拟中断原始状态寄存器。 */
#define HS_DET_IRQ_LEN         1
#define HS_DET_IRQ_OFFSET      3
#define HS_MIC_ECO_IRQ_LEN     1
#define HS_MIC_ECO_IRQ_OFFSET  2
#define HS_MIC_NOR2_IRQ_LEN    1
#define HS_MIC_NOR2_IRQ_OFFSET 1
#define HS_MIC_NOR1_IRQ_LEN    1
#define HS_MIC_NOR1_IRQ_OFFSET 0

#define ANA_IRQM_REG0_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC008) /* ANA_IRQ_REG中断屏蔽寄存器。 */
#define ANA_IRQ_MASK_7_LEN    1
#define ANA_IRQ_MASK_7_OFFSET 7
#define ANA_IRQ_MASK_6_LEN    1
#define ANA_IRQ_MASK_6_OFFSET 6
#define ANA_IRQ_MASK_5_LEN    1
#define ANA_IRQ_MASK_5_OFFSET 5
#define ANA_IRQ_MASK_4_LEN    1
#define ANA_IRQ_MASK_4_OFFSET 4
#define ANA_IRQ_MASK_3_LEN    1
#define ANA_IRQ_MASK_3_OFFSET 3
#define ANA_IRQ_MASK_2_LEN    1
#define ANA_IRQ_MASK_2_OFFSET 2
#define ANA_IRQ_MASK_1_LEN    1
#define ANA_IRQ_MASK_1_OFFSET 1
#define ANA_IRQ_MASK_0_LEN    1
#define ANA_IRQ_MASK_0_OFFSET 0

#define ANA_IRQ_REG0_REG       (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC009) /* 模拟中断寄存器。0，写1清，读上报中断信息。 */
#define HS_DET_IRQ_NEG_LEN         1
#define HS_DET_IRQ_NEG_OFFSET      7
#define HS_DET_IRQ_POS_LEN         1
#define HS_DET_IRQ_POS_OFFSET      6
#define HS_MIC_ECO_IRQ_NEG_LEN     1
#define HS_MIC_ECO_IRQ_NEG_OFFSET  5
#define HS_MIC_ECO_IRQ_POS_LEN     1
#define HS_MIC_ECO_IRQ_POS_OFFSET  4
#define HS_MIC_NOR2_IRQ_NEG_LEN    1
#define HS_MIC_NOR2_IRQ_NEG_OFFSET 3
#define HS_MIC_NOR2_IRQ_POS_LEN    1
#define HS_MIC_NOR2_IRQ_POS_OFFSET 2
#define HS_MIC_NOR1_IRQ_NEG_LEN    1
#define HS_MIC_NOR1_IRQ_NEG_OFFSET 1
#define HS_MIC_NOR1_IRQ_POS_LEN    1
#define HS_MIC_NOR1_IRQ_POS_OFFSET 0

#define DEB_CNT_HS_DET_CFG_REG (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC00A) /* hs_det_irq去抖配置。 */
#define BYPASS_DEB_HS_DET_IRQ_LEN    1
#define BYPASS_DEB_HS_DET_IRQ_OFFSET 5
#define DEB_CNT_HS_DET_IRQ_LEN       5
#define DEB_CNT_HS_DET_IRQ_OFFSET    0

#define DEB_CNT_HS_MIC_CFG_REG (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC00B) /* hs_mic_irq去抖配置。 */
#define BYPASS_DEB_HS_MIC_IRQ_LEN    1
#define BYPASS_DEB_HS_MIC_IRQ_OFFSET 5
#define DEB_CNT_HS_MIC_IRQ_LEN       5
#define DEB_CNT_HS_MIC_IRQ_OFFSET    0

#define CODEC_ANA_RW1_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC00C) /* vref/ctcm/ibias模块使能。 */
#define IBIAS_PD_LEN      1
#define IBIAS_PD_OFFSET   2
#define AVREF_SEL_LEN     2
#define AVREF_SEL_OFFSET  0

#define CODEC_ANA_RW2_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC00D) /* micpga使能。 */
#define MAINPGA_PD_LEN              1
#define MAINPGA_PD_OFFSET           6
#define AUXPGA_PD_LEN               1
#define AUXPGA_PD_OFFSET            5
#define MIC3PGA_PD_LEN              1
#define MIC3PGA_PD_OFFSET           4
#define MICPGA_WORK_MODE_SEL_LEN    1
#define MICPGA_WORK_MODE_SEL_OFFSET 3
#define MAINPGA_MUTE_LEN            1
#define MAINPGA_MUTE_OFFSET         2
#define AUXPGA_MUTE_LEN             1
#define AUXPGA_MUTE_OFFSET          1
#define MIC3PGA_MUTE_LEN            1
#define MIC3PGA_MUTE_OFFSET         0

#define CODEC_ANA_RW3_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC00E) /* adc使能。 */
#define ADCL_PD_LEN       1
#define ADCL_PD_OFFSET    6
#define ADCR_PD_LEN       1
#define ADCR_PD_OFFSET    5
#define ADC3_PD_LEN       1
#define ADC3_PD_OFFSET    4
#define ADCL_MUTE_LEN     1
#define ADCL_MUTE_OFFSET  2
#define ADCR_MUTE_LEN     1
#define ADCR_MUTE_OFFSET  1
#define ADC3_MUTE_LEN     1
#define ADC3_MUTE_OFFSET  0

#define CODEC_ANA_RW4_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC00F) /* dac使能。 */
#define DACS_PD_LEN       1
#define DACS_PD_OFFSET    2
#define DACL_PD_LEN       1
#define DACL_PD_OFFSET    1
#define DACR_PD_LEN       1
#define DACR_PD_OFFSET    0

#define CODEC_ANA_RW5_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC010) /* headphone使能。 */
#define CHARGEPUMP_PD_LEN    1
#define CHARGEPUMP_PD_OFFSET 7
#define POP_PULL_EN_LEN      1
#define POP_PULL_EN_OFFSET   6
#define MIXOUT_HPL_PD_LEN    1
#define MIXOUT_HPL_PD_OFFSET 5
#define MIXOUT_HPR_PD_LEN    1
#define MIXOUT_HPR_PD_OFFSET 4
#define HPL_PD_LEN           1
#define HPL_PD_OFFSET        3
#define HPR_PD_LEN           1
#define HPR_PD_OFFSET        2
#define HPL_MUTE_LEN         1
#define HPL_MUTE_OFFSET      1
#define HPR_MUTE_LEN         1
#define HPR_MUTE_OFFSET      0

#define CODEC_ANA_RW6_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC011) /* earphone/lineout使能。 */
#define LOMIXER_PD_LEN       1
#define LOMIXER_PD_OFFSET    6
#define LOUT_PD_LEN          1
#define LOUT_PD_OFFSET       5
#define LOUT_MUTE_LEN        1
#define LOUT_MUTE_OFFSET     4
#define MIXOUT_EAR_PD_LEN    1
#define MIXOUT_EAR_PD_OFFSET 3
#define EPVCM_PD_LEN         1
#define EPVCM_PD_OFFSET      2
#define EAR_PD_LEN           1
#define EAR_PD_OFFSET        1
#define EAR_MUTE_LEN         1
#define EAR_MUTE_OFFSET      0

#define CODEC_ANA_RW7_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC012) /* micbias使能。 */
#define HSMICB_PD_LEN       1
#define HSMICB_PD_OFFSET    6
#define MICB1_PD_LEN        1
#define MICB1_PD_OFFSET     5
#define MICB2_PD_LEN        1
#define MICB2_PD_OFFSET     4
#define HSMICB_DSCHG_LEN    1
#define HSMICB_DSCHG_OFFSET 2
#define MICB1_DSCHG_LEN     1
#define MICB1_DSCHG_OFFSET  1
#define MICB2_DSCHG_LEN     1
#define MICB2_DSCHG_OFFSET  0

#define CODEC_ANA_RW8_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC013) /* mbhd使能。 */
#define MBHD_COMP_HSD_PD_LEN            1
#define MBHD_COMP_HSD_PD_OFFSET         4
#define MBHD_COMP_HSD_VOLTAGE_PD_LEN    1
#define MBHD_COMP_HSD_VOLTAGE_PD_OFFSET 3
#define MBHD_COMP_PD_LEN                1
#define MBHD_COMP_PD_OFFSET             2
#define MBHD_BUFF_PD_LEN                1
#define MBHD_BUFF_PD_OFFSET             1
#define MBHD_ECO_EN_LEN                 1
#define MBHD_ECO_EN_OFFSET              0

#define CODEC_ANA_RW9_REG      (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC014) /* mainpga配置。 */
#define MICPGA_UNLOCK_ENABLE_LEN    1
#define MICPGA_UNLOCK_ENABLE_OFFSET 6
#define MAINPGA_SEL_LEN             2
#define MAINPGA_SEL_OFFSET          4
#define MAINPGA_BOOST_LEN           1
#define MAINPGA_BOOST_OFFSET        3
#define MAINPGA_GAIN_LEN            3
#define MAINPGA_GAIN_OFFSET         0

#define CODEC_ANA_RW10_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC015) /* auxpga配置。 */
#define DISCHARGE_DISABLE_LEN    1
#define DISCHARGE_DISABLE_OFFSET 7
#define HSMIC_VCM_PULL_EN_LEN    1
#define HSMIC_VCM_PULL_EN_OFFSET 6
#define AUXPGA_SEL_LEN           2
#define AUXPGA_SEL_OFFSET        4
#define AUXPGA_BOOST_LEN         1
#define AUXPGA_BOOST_OFFSET      3
#define AUXPGA_GAIN_LEN          3
#define AUXPGA_GAIN_OFFSET       0

#define CODEC_ANA_RW11_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC016) /* mic3pga配置。 */
#define MIC3PGA_SEL_LEN      2
#define MIC3PGA_SEL_OFFSET   4
#define ADC3PGA_BOOST_LEN    1
#define ADC3PGA_BOOST_OFFSET 3
#define MIC3PGA_GAIN_LEN     3
#define MIC3PGA_GAIN_OFFSET  0

#define CODEC_ANA_RW12_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC017) /* adcl配置。 */
#define IDEL_TONE_CTRL_LEN    2
#define IDEL_TONE_CTRL_OFFSET 6
#define ADCL_MIXIN_DACR_LEN       1
#define ADCL_MIXIN_DACR_OFFSET    5
#define ADCL_MIXIN_DACL_LEN       1
#define ADCL_MIXIN_DACL_OFFSET    4
#define ADCL_MIXIN_REV_LEN        2
#define ADCL_MIXIN_REV_OFFSET     2
#define ADCL_MIXIN_MICPGA2_LEN    1
#define ADCL_MIXIN_MICPGA2_OFFSET 1
#define ADCL_MIXIN_MICPGA1_LEN    1
#define ADCL_MIXIN_MICPGA1_OFFSET 0

#define CODEC_ANA_RW13_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC018) /* adcl配置。 */
#define ADCL_DWA_BPS_LEN      1
#define ADCL_DWA_BPS_OFFSET   7
#define ADCL_CLK_DELAY_LEN    3
#define ADCL_CLK_DELAY_OFFSET 4
#define ADCL_DAC_BIAS_LEN     2
#define ADCL_DAC_BIAS_OFFSET  2
#define ADCL_FLSTN_LEN        2
#define ADCL_FLSTN_OFFSET     0

#define CODEC_ANA_RW14_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC019) /* adcr配置。 */
#define ADCR_MIXIN_DACR_LEN       1
#define ADCR_MIXIN_DACR_OFFSET    5
#define ADCR_MIXIN_DACL_LEN       1
#define ADCR_MIXIN_DACL_OFFSET    4
#define ADCR_MIXIN_MICPGA1_LEN    1
#define ADCR_MIXIN_MICPGA1_OFFSET 1
#define ADCR_MIXIN_MICPGA2_LEN    1
#define ADCR_MIXIN_MICPGA2_OFFSET 0

#define CODEC_ANA_RW15_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC01A) /* adcr配置。 */
#define ADCR_DWA_BPS_LEN      1
#define ADCR_DWA_BPS_OFFSET   7
#define ADCR_CLK_DELAY_LEN    3
#define ADCR_CLK_DELAY_OFFSET 4
#define ADCR_DAC_BIAS_LEN     2
#define ADCR_DAC_BIAS_OFFSET  2
#define ADCR_FLSTN_LEN        2
#define ADCR_FLSTN_OFFSET     0

#define CODEC_ANA_RW16_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC01B) /* adc3配置。 */
#define ADC3_MIXIN_ULTRA_LEN      1
#define ADC3_MIXIN_ULTRA_OFFSET   5
#define ADC3_MIXIN_DACL_LEN       1
#define ADC3_MIXIN_DACL_OFFSET    4
#define ADC3_MIXIN_MICPGA1_LEN    1
#define ADC3_MIXIN_MICPGA1_OFFSET 1
#define ADC3_MIXIN_MICPGA3_LEN    1
#define ADC3_MIXIN_MICPGA3_OFFSET 0

#define CODEC_ANA_RW17_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC01C) /* adc3配置。 */
#define ADC3_DWA_BPS_LEN      1
#define ADC3_DWA_BPS_OFFSET   7
#define ADC3_TIMING_LEN       3
#define ADC3_TIMING_OFFSET    4
#define ADC3_CLK_DELAY_LEN    2
#define ADC3_CLK_DELAY_OFFSET 2
#define ADC3_FLSTN_LEN        2
#define ADC3_FLSTN_OFFSET     0

#define CODEC_ANA_RW18_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC01D) /* dac配置。 */
#define PULL_CPN_ENABLE_LEN    1
#define PULL_CPN_ENABLE_OFFSET 7
#define DACS_ICTRL_LEN         2
#define DACS_ICTRL_OFFSET      4
#define DACL_ICTRL_LEN         2
#define DACL_ICTRL_OFFSET      2
#define DACR_ICTRL_LEN         2
#define DACR_ICTRL_OFFSET      0

#define CODEC_ANA_RW19_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC01E) /* chargepump配置。 */
#define CP_CLKSET_LEN          1
#define CP_CLKSET_OFFSET       7
#define CP_AUTOSOFT_DIS_LEN    1
#define CP_AUTOSOFT_DIS_OFFSET 6
#define CP_SOFT_DLY_CFG_LEN    2
#define CP_SOFT_DLY_CFG_OFFSET 4
#define CP_SW_CFG_LEN          2
#define CP_SW_CFG_OFFSET       2
#define CP_DT_CFG_LEN          2
#define CP_DT_CFG_OFFSET       0

#define CODEC_ANA_RW20_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC01F) /* hp mixer输入选择。 */
#define MIXOUT_HSL_DACL_LEN    1
#define MIXOUT_HSL_DACL_OFFSET 7
#define MIXOUT_HSL_DACR_LEN    1
#define MIXOUT_HSL_DACR_OFFSET 6
#define MIXOUT_HSR_DACR_LEN    1
#define MIXOUT_HSR_DACR_OFFSET 3
#define MIXOUT_HSR_DACL_LEN    1
#define MIXOUT_HSR_DACL_OFFSET 2

#define CODEC_ANA_RW21_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC020) /* hp增益选择。 */
#define HSL_GAIN_LEN    4
#define HSL_GAIN_OFFSET 4
#define HSR_GAIN_LEN    4
#define HSR_GAIN_OFFSET 0

#define CODEC_ANA_RW22_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC021) /* hp配置。 */
#define HSL_MIN_GAIN_LEN    1
#define HSL_MIN_GAIN_OFFSET 5
#define HSR_MIN_GAIN_LEN    1
#define HSR_MIN_GAIN_OFFSET 4
#define STB_CTRL_SEC_LEN    2
#define STB_CTRL_SEC_OFFSET 2
#define STB_CTRL_LEN        2
#define STB_CTRL_OFFSET     0

#define CODEC_ANA_RW23_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC022) /* hp配置。 */
#define STB_ACTIVE_CTRL_LEN    1
#define STB_ACTIVE_CTRL_OFFSET 5
#define STB_1N_LEN             1
#define STB_1N_OFFSET          4
#define HPOUT_1K_LOAD_LEN      1
#define HPOUT_1K_LOAD_OFFSET   3
#define STB_N12DB_GAIN_LEN     1
#define STB_N12DB_GAIN_OFFSET  2
#define THD_CTRL_SEL_LEN       2
#define THD_CTRL_SEL_OFFSET    0

#define CODEC_ANA_RW24_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC023) /* hp pop配置。 */
#define POP_DIS_LEN        1
#define POP_DIS_OFFSET     6
#define RST_POP_LEN        1
#define RST_POP_OFFSET     5
#define RL_SYNC_EN_LEN     1
#define RL_SYNC_EN_OFFSET  4
#define POP_DLY_CFG_LEN    2
#define POP_DLY_CFG_OFFSET 2
#define PLO_SPD_CFG_LEN    2
#define PLO_SPD_CFG_OFFSET 0

#define CODEC_ANA_RW25_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC024) /* ep mixer配置。 */
#define MIXOUT_EAR_DACL_LEN          1
#define MIXOUT_EAR_DACL_OFFSET       7
#define MIXOUT_EAR_DACR_LEN          1
#define MIXOUT_EAR_DACR_OFFSET       6
#define MIXOUT_EAR_DAC_ULTRA_LEN     1
#define MIXOUT_EAR_DAC_ULTRA_OFFSET  5
#define CR_MIXER_EP_DACL_LEN         1
#define CR_MIXER_EP_DACL_OFFSET      3
#define CR_MIXER_EP_DACR_LEN         1
#define CR_MIXER_EP_DACR_OFFSET      2
#define CR_MIXER_EP_DAC_ULTRA_LEN    1
#define CR_MIXER_EP_DAC_ULTRA_OFFSET 1

#define CODEC_ANA_RW26_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC025) /* earphone配置。 */
#define EAR_CM_CTRL_LEN    1
#define EAR_CM_CTRL_OFFSET 4
#define EAR_GAIN_LEN       4
#define EAR_GAIN_OFFSET    0

#define CODEC_ANA_RW27_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC026) /* lineout mixer配置。 */
#define LOMIX_SEL_DACL_LEN              1
#define LOMIX_SEL_DACL_OFFSET           7
#define LOMIX_SEL_DACR_LEN              1
#define LOMIX_SEL_DACR_OFFSET           6
#define LOMIX_SEL_DAC_ULTRA_LEN         1
#define LOMIX_SEL_DAC_ULTRA_OFFSET      5
#define LOMIX_BOOSTGAIN_DACL_LEN        1
#define LOMIX_BOOSTGAIN_DACL_OFFSET     3
#define LOMIX_BOOSTGAIN_DACR_LEN        1
#define LOMIX_BOOSTGAIN_DACR_OFFSET     2
#define LOMIX_BOOSTGAIN_DACULTRA_LEN    1
#define LOMIX_BOOSTGAIN_DACULTRA_OFFSET 1

#define CODEC_ANA_RW28_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC027) /* lineout配置。 */
#define LOUT_VCM_CTRL_LEN    1
#define LOUT_VCM_CTRL_OFFSET 4
#define LOUT_GAIN_LEN        4
#define LOUT_GAIN_OFFSET     0

#define CODEC_ANA_RW29_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC028) /* himicbias配置。 */
#define HSMICB_ENB_SDET_LEN       1
#define HSMICB_ENB_SDET_OFFSET    5
#define HSMICB_LONOISE_1P2_LEN    1
#define HSMICB_LONOISE_1P2_OFFSET 4
#define HSMICB_ADJ_LEN            4
#define HSMICB_ADJ_OFFSET         0

#define CODEC_ANA_RW30_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC029) /* micbias1配置。 */
#define MICB1_ENB_SDET_LEN       1
#define MICB1_ENB_SDET_OFFSET    5
#define MICB1_LONOISE_1P2_LEN    1
#define MICB1_LONOISE_1P2_OFFSET 4
#define MICB1_ADJ_LEN            4
#define MICB1_ADJ_OFFSET         0

#define CODEC_ANA_RW31_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC02A) /* micbias2配置。 */
#define MICB2_ENB_SDET_LEN       1
#define MICB2_ENB_SDET_OFFSET    5
#define MICB2_LONOISE_1P2_LEN    1
#define MICB2_LONOISE_1P2_OFFSET 4
#define MICB2_ADJ_LEN            4
#define MICB2_ADJ_OFFSET         0

#define CODEC_ANA_RW32_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC02B) /* 耳机插入检测配置。 */
#define HSD_VTH_SEL_LEN          4
#define HSD_VTH_SEL_OFFSET       4
#define HSD_VTH_CFG_LEN          2
#define HSD_VTH_CFG_OFFSET       2
#define HSD_VTL_SEL_LEN          2
#define HSD_VTL_SEL_OFFSET       0

#define CODEC_ANA_RW33_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC02C) /* 耳机拔出pop配置。 */
#define PLUG_IN_DETECT_MODE_LEN            1
#define PLUG_IN_DETECT_MODE_OFFSET         4
#define ANALOG_LOOP_LEN                    1
#define ANALOG_LOOP_OFFSET                 3
#define PULLOUT_POP_EN1_LEN                1
#define PULLOUT_POP_EN1_OFFSET             2
#define PULLOUT_POP_EN2_LEN                1
#define PULLOUT_POP_EN2_OFFSET             1
#define PULLOUT_HSD_DIR_SEL_LEN            1
#define PULLOUT_HSD_DIR_SEL_OFFSET         0

#define CODEC_ANA_RW34_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC02D) /* mbhd ref配置。 */
#define MBHD_VREF_PD_LEN     1
#define MBHD_VREF_PD_OFFSET  7
#define ECO_VTH_SEL_LEN      3
#define ECO_VTH_SEL_OFFSET   4
#define COMPH_VTH_SEL_LEN    2
#define COMPH_VTH_SEL_OFFSET 2
#define COMPL_VTH_SEL_LEN    2
#define COMPL_VTH_SEL_OFFSET 0

#define CODEC_ANA_RW35_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC02E) /* adc/dac时钟验配置。 */
#define ADC_CLK_SYS_EDGE_LEN    2
#define ADC_CLK_SYS_EDGE_OFFSET 2
#define DAC_CLK_SEL_EDGE_LEN    2
#define DAC_CLK_SEL_EDGE_OFFSET 0

#define CODEC_ANA_RW36_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC02F) /* rx通路chop时钟配置。 */
#define RX_CHOP_BPS_LEN         1
#define RX_CHOP_BPS_OFFSET      6
#define ADC_CHOP_CLK_SEL_LEN    2
#define ADC_CHOP_CLK_SEL_OFFSET 4
#define CTCM_CHOP_BPS_LEN       1
#define CTCM_CHOP_BPS_OFFSET    3
#define CAPLESS_CHOP_BPS_LEN    1
#define CAPLESS_CHOP_BPS_OFFSET 2
#define MICPGA_CHOP_BPS_LEN     1
#define MICPGA_CHOP_BPS_OFFSET  1
#define ADC_SDM_CHOP_BPS_LEN    1
#define ADC_SDM_CHOP_BPS_OFFSET 0

#define CODEC_ANA_RW37_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC030) /* tx通路chop时钟配置。 */
#define LOMIX_CHOP_BPS_LEN      1
#define LOMIX_CHOP_BPS_OFFSET   7
#define TX_CHOP_BPS_LEN         1
#define TX_CHOP_BPS_OFFSET      6
#define DAC_CHOP_CLK_SEL_LEN    2
#define DAC_CHOP_CLK_SEL_OFFSET 4
#define DAC_CHOP_BPS_LEN        1
#define DAC_CHOP_BPS_OFFSET     3
#define HS_CHOP_BPS_LEN         1
#define HS_CHOP_BPS_OFFSET      2
#define EPMIX_CHOP_BPS_LEN      1
#define EPMIX_CHOP_BPS_OFFSET   1
#define LOUT_CHOP_BPS_LEN       1
#define LOUT_CHOP_BPS_OFFSET    0

#define CODEC_ANA_RW38_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC031) /* cp/pop时钟配置。 */
#define HS_SYS_CLK_PD_LEN     1
#define HS_SYS_CLK_PD_OFFSET  5
#define CLKPOP_SEL_LEN        1
#define CLKPOP_SEL_OFFSET     4
#define CLKCP_SRC_FREQ_LEN    1
#define CLKCP_SRC_FREQ_OFFSET 3
#define CLKCP_SEL_LEN         3
#define CLKCP_SEL_OFFSET      0

#define CODEC_ANA_RW39_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC032) /* capless时钟配置。 */
#define CPLS_CLK_FREQ_SEL_LEN     1
#define CPLS_CLK_FREQ_SEL_OFFSET  6
#define CAPLESS_CLK_FQ_SEL_LEN    2
#define CAPLESS_CLK_FQ_SEL_OFFSET 4
#define CLK_CAPLESS_BPS_LEN       1
#define CLK_CAPLESS_BPS_OFFSET    3
#define CAPLESS_CLK_PH_SEL_LEN    1
#define CAPLESS_CLK_PH_SEL_OFFSET 2
#define CAPLESS_CLK_FO_SEL_LEN    2
#define CAPLESS_CLK_FO_SEL_OFFSET 0

#define CODEC_ANA_RW40_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC033) /* 偏置电流配置。 */
#define CODEC_IBIAS_ADJ_LEN     1
#define CODEC_IBIAS_ADJ_OFFSET  7
#define IB05_CTCM_ADJ_LEN       3
#define IB05_CTCM_ADJ_OFFSET    4
#define IB05_CAPLESS_ADJ_LEN    3
#define IB05_CAPLESS_ADJ_OFFSET 0

#define CODEC_ANA_RW41_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC034) /* CODEC_ANA_RW41偏置电流配置。 */
#define IB05_MICPGA_ADJ_LEN         3
#define IB05_MICPGA_ADJ_OFFSET      4
#define IB05_MICPGALOGIC_ADJ_LEN    3
#define IB05_MICPGALOGIC_ADJ_OFFSET 0

#define CODEC_ANA_RW42_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC035) /* CODEC_ANA_RW42偏置电流配置。 */
#define IB05_ADCOP1_ADJ_LEN    3
#define IB05_ADCOP1_ADJ_OFFSET 4
#define IB05_ADCOP2_ADJ_LEN    3
#define IB05_ADCOP2_ADJ_OFFSET 0

#define CODEC_ANA_RW43_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC036) /* CODEC_ANA_RW43偏置电流配置。 */
#define IB05_ADCCOMP_ADJ_LEN    3
#define IB05_ADCCOMP_ADJ_OFFSET 4
#define IB05_DACI2V_ADJ_LEN     3
#define IB05_DACI2V_ADJ_OFFSET  0

#define CODEC_ANA_RW44_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC037) /* CODEC_ANA_RW44偏置电流配置。 */
#define IB05_HPMIX_ADJ_LEN    3
#define IB05_HPMIX_ADJ_OFFSET 4
#define IB05_HP_ADJ_LEN       3
#define IB05_HP_ADJ_OFFSET    0

#define CODEC_ANA_RW45_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC038) /* CODEC_ANA_RW45偏置电流配置。 */
#define IB05_HPLOGIC_ADJ_LEN     3
#define IB05_HPLOGIC_ADJ_OFFSET  4
#define IB05_EARLOGIC_ADJ_LEN    3
#define IB05_EARLOGIC_ADJ_OFFSET 0

#define CODEC_ANA_RW46_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC039) /* CODEC_ANA_RW46偏置电流配置。 */
#define IB05_EARMIX_ADJ_LEN    3
#define IB05_EARMIX_ADJ_OFFSET 4
#define IB05_EAR_ADJ_LEN       3
#define IB05_EAR_ADJ_OFFSET    0

#define CODEC_ANA_RW47_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC03A) /* CODEC_ANA_RW47偏置电流配置。 */
#define IB05_LO_ADJ_LEN         3
#define IB05_LO_ADJ_OFFSET      4
#define IB05_LOLOGIC_ADJ_LEN    3
#define IB05_LOLOGIC_ADJ_OFFSET 0

#define CODEC_ANA_RW48_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC03B) /* CODEC_ANA_RW48偏置电流配置。 */
#define IB05_MICB_ADJ_LEN    3
#define IB05_MICB_ADJ_OFFSET 0

#define CODEC_ANA_RW49_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC03C) /* CODEC_ANA_RW49偏置电流配置。 */
#define IB05_LOMIX_ADJ_LEN       3
#define IB05_LOMIX_ADJ_OFFSET    4
#define IB05_DACS_I2V_ADJ_LEN    3
#define IB05_DACS_I2V_ADJ_OFFSET 0

#define CODEC_ANA_RW50_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC03D) /* CODEC_ANA_RW50偏置电流配置。 */
#define CODEC_ANA_RW_50_LEN    8
#define CODEC_ANA_RW_50_OFFSET 0

#define CODEC_ANA_RW51_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC03E) /* CODEC_ANA_RW51。 */
#define CODEC_ANA_RW_51_LEN    8
#define CODEC_ANA_RW_51_OFFSET 0

#define CODEC_ANA_RW52_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC03F) /* CODEC_ANA_RW52。 */
#define CODEC_ANA_RW_52_LEN    8
#define CODEC_ANA_RW_52_OFFSET 0

#define CODEC_ANA_RW53_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC040) /* MADSDM积分器偏置电流 */
#define CODEC_ANA_RW54_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC041) /* MADSDM PGA偏置电流 */
#define CODEC_ANA_RW55_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC042) /* MAD ADC通路配置。 */
#define CODEC_ANA_RW56_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC043) /* MAD ADC通路配置。 */
#define CODEC_ANA_RW57_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC044) /* MAD ADC通路配置。 */
#define CODEC_ANA_RW58_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC045) /* MAD ADC通路配置。 */
#define VCM_MAD_LP_EN_LEN 1
#define VCM_MAD_LP_EN_OFFSET 6
#define MAD_PGA_SEL_LEN 4
#define MAD_PGA_SEL_OFFSET 0

#define CODEC_ANA_RW59_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC046) /* MAD ADC通路配置。 */
#define PGA_LEN 5
#define PGA_OFFSET 0

#define CODEC_ANA_RW60_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC047) /* MAD ADC通路配置。 */
#define PD_MAD_SDM_LEN 1
#define PD_MAD_SDM_OFFSET 5
#define PD_MAD_PGA_LEN 1
#define PD_MAD_PGA_OFFSET 4
#define PD_MAD_CLK_GEN_LEN 1
#define PD_MAD_CLK_GEN_OFFSET 3
#define PD_DOUT_MAD_LEN 1
#define PD_DOUT_MAD_OFFSET 2
#define PD_MUTE_MAD_LEN 1
#define PD_MUTE_MAD_OFFSET 1
#define PD_MUTE_MAD_PGA_LEN 1
#define PD_MUTE_MAD_PGA_OFFSET 0

#define CODEC_ANA_RW61_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC048) /* MAD CLK通路配置。 */
#define CODEC_ANA_RW62_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC049) /* MAD ADC通路配置。 */
#define DIN_MAD_MIC_LEN 1
#define DIN_MAD_MIC_OFFSET 0

#define CODEC_ANA_RW63_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC04A) /* CODEC_ANA_RW63 */

#define CODEC_ANA_RO01_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC04B) /* CODEC_ANA_RO01。 */
#define CODEC_ANA_RO_01_LEN    8
#define CODEC_ANA_RO_01_OFFSET 0

#define CODEC_ANA_RO02_REG     (DA_SEPARATE_V5_PMU_CODEC_BASE + 0xC04C) /* CODEC_ANA_RO02。 */
#define CODEC_ANA_RO_02_LEN    8
#define CODEC_ANA_RO_02_OFFSET 0

/* CTRL_REGA Base address of Module's Register */
#define DA_SEPARATE_V5_CTRL_REGA_BASE                       (0x0)

#define CTRL_REGA_CLASSD_ONOFF_REG               (DA_SEPARATE_V5_CTRL_REGA_BASE + 0x53)
#define CTRL_REGA_ST_CLASSD_EN_LEN     1
#define CTRL_REGA_ST_CLASSD_EN_OFFSET  1
#define CTRL_REGA_REG_CLASSD_EN_LEN    1
#define CTRL_REGA_REG_CLASSD_EN_OFFSET 0

#define CTRL_REG_PMU_SOFT_RST_REG        (DA_SEPARATE_V5_CTRL_REGA_BASE + 0xFB) /* PMU软复位寄存器。 */
#define CTRL_REG_SOFT_RST_N_LEN    8
#define CTRL_REG_SOFT_RST_N_OFFSET 0

/* CTRL_REGB Base address of Module's Register */
#define DA_SEPARATE_V5_CTRL_REGB_BASE                       (0x400)

#define CTRL_REGB_CLASSD_CTRL0_REG      (DA_SEPARATE_V5_CTRL_REGB_BASE + 0x180) /* CLASSD控制寄存器0 */
#define CTRL_REGB_CLASSD_DRV_EN_LEN    1
#define CTRL_REGB_CLASSD_DRV_EN_OFFSET 7
#define CTRL_REGB_CLASSD_DT_SEL_LEN    1
#define CTRL_REGB_CLASSD_DT_SEL_OFFSET 6
#define CTRL_REGB_CLASSD_GAIN_LEN      2
#define CTRL_REGB_CLASSD_GAIN_OFFSET   4
#define CTRL_REGB_CLASSD_I_OCP_LEN     2
#define CTRL_REGB_CLASSD_I_OCP_OFFSET  2
#define CTRL_REGB_CLASSD_I_PUMP_LEN    2
#define CTRL_REGB_CLASSD_I_PUMP_OFFSET 0

#define CTRL_REGB_CLASSD_CTRL1_REG      (DA_SEPARATE_V5_CTRL_REGB_BASE + 0x181) /* CLASSD控制寄存器1 */
#define CTRL_REGB_CLASSD_MUTE_LEN        1
#define CTRL_REGB_CLASSD_MUTE_OFFSET     7
#define CTRL_REGB_CLASSD_MUTE_SEL_LEN    1
#define CTRL_REGB_CLASSD_MUTE_SEL_OFFSET 6
#define CTRL_REGB_CLASSD_N_SEL_LEN       2
#define CTRL_REGB_CLASSD_N_SEL_OFFSET    4
#define CTRL_REGB_CLASSD_OCP_BPS_LEN     1
#define CTRL_REGB_CLASSD_OCP_BPS_OFFSET  3
#define CTRL_REGB_CLASSD_P_SEL_LEN       2
#define CTRL_REGB_CLASSD_P_SEL_OFFSET    1
#define CTRL_REGB_CLASSD_PLS_BYP_LEN     1
#define CTRL_REGB_CLASSD_PLS_BYP_OFFSET  0

#define CTRL_REGB_CLASSD_CTRL2_REG      (DA_SEPARATE_V5_CTRL_REGB_BASE + 0x182) /* CLASSD控制寄存器2 */
#define CTRL_REGB_CLASSD_RESERVE0_LEN    4
#define CTRL_REGB_CLASSD_RESERVE0_OFFSET 0

/* XOADC Base address of Module's Register */
#define DA_SEPARATE_V5_XOADC_BASE                       (0x0)

#define CTRL_REG_DIG_IO_DS_CODEC_CFG_REG  (DA_SEPARATE_V5_XOADC_BASE + 0x3DC) /* 数字CODEC IO驱动配置寄存器 */

#define CTRL_REG_DIG_IO_DS_SEL1_CODEC_CFG_REG  (DA_SEPARATE_V5_XOADC_BASE + 0x3DE)

#endif /* __DA_SEPARATE_V5_PMU_REG_DEF_H__ */
