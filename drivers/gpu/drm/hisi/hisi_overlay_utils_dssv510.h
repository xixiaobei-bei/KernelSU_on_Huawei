/*
 * Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */
#ifndef _HISI_OVERLAY_UTILS_PLATFORM_H_
#define _HISI_OVERLAY_UTILS_PLATFORM_H_

#define CRGPERI_PLL0_CLK_RATE	(1660000000UL)
#define CRGPERI_PLL2_CLK_RATE	(1920000000UL)
#define CRGPERI_PLL3_CLK_RATE	(1200000000UL)
#define CRGPERI_PLL7_CLK_RATE	(1290000000UL)

#ifdef CONFIG_HISI_DSS_V600
#define DEFAULT_PCLK_DSS_RATE	(238000000UL)

#define DSS_CORE_CLK_RATE_L4	(640000000UL)
#define DSS_CORE_CLK_RATE_L3	(480000000UL)
#define DSS_CORE_CLK_RATE_L2	(400000000UL)
#define DSS_CORE_CLK_RATE_L1	(238000000UL)
#endif

#ifdef CONFIG_HISI_DSS_V510
#define DSS_CORE_CLK_RATE_L4	(554000000UL)
#define DSS_CORE_CLK_RATE_L3	(415000000UL)
#define DSS_CORE_CLK_RATE_L2	(332000000UL)
#define DSS_CORE_CLK_RATE_L1	(208000000UL)
#endif

#define MDC_CORE_CLK_RATE_L4	(554000000UL)
#define MDC_CORE_CLK_RATE_L3	(415000000UL)
#define MDC_CORE_CLK_RATE_L2	(332000000UL)
#define MDC_CORE_CLK_RATE_L1	(238000000UL)

/* pix0_clk: 0.65v-300M, 0.7-415M, 0.8-645M */
#define DSS_PXL0_CLK_RATE_L3	(645000000UL)
#define DSS_PXL0_CLK_RATE_L2	(415000000UL)
#define DSS_PXL0_CLK_RATE_L1	(300000000UL)

/* mm_axi_clk, aclk_mmbuf */
#define DSS_MMBUF_CLK_RATE_L4	(480000000UL)
#define DSS_MMBUF_CLK_RATE_L3	(332000000UL)
#define DSS_MMBUF_CLK_RATE_L2	(277000000UL)
#define DSS_MMBUF_CLK_RATE_L1	(208000000UL)

/* pix1_clk: clk_ldi1_ini */
#define DSS_PXL1_CLK_RATE_L4	(600000000UL)
#define DSS_PXL1_CLK_RATE_L3	(415000000UL)
#define DSS_PXL1_CLK_RATE_L2	(332000000UL)
#define DSS_PXL1_CLK_RATE_L1	(213000000UL)

/* dss clk power off */
#define DSS_CORE_CLK_RATE_OFF	(104000000UL)
#define DSS_MMBUF_CLK_RATE_OFF	(104000000UL)
#define DSS_PXL1_CLK_RATE_OFF	(104000000UL)

#define DEFAULT_PCLK_PCTRL_RATE	(83000000UL)

#define OVL_LAYER_NUM_MAX (9)

/* 10BIT: 0x3ff, 8BIT: 0xFF  RATIO = 0x3ff/0xff */
#define OVL_PATTERN_RATIO (4)

#define HISI_DSS_CMDLIST_MAX	(16)
#define HISI_DSS_CMDLIST_IDXS_MAX (0xFFFF)   // 16 cmdlist, 16bit, 1111,1111,1111,1111=0xFFFF
#define HISI_DSS_COPYBIT_CMDLIST_IDXS	 (0xC000)  // bit14, bit15
#define HISI_DSS_MEDIACOMMON_CMDLIST_IDXS	 (0x204)  // bit2, bit9
#define HISI_DSS_DPP_MAX_SUPPORT_BIT (0x1fff) // 13 dpp module, 13bit, contrast to enmu dpp_module_idx
#define HISIFB_DSS_PLATFORM_TYPE  (FB_ACCEL_DSSV510 | FB_ACCEL_PLATFORM_TYPE_ASIC)

#define DSS_MIF_SMMU_SMRX_IDX_STEP (16)

// PERI REG
#define CRG_PERI_DIS3_DEFAULT_VAL     (0x0002F000)

// scl
#define SCF_LINE_BUF	(2560)

// DSS global
#define DSS_GLB_MODULE_CLK_SEL_DEFAULT_VAL  (0xF0000008)


// LDI0 clk sel
#define DSS_LDI_CLK_SEL_DEFAULT_VAL    (0x00000004)

// DBuf mem ctrl
#define DSS_DBUF_MEM_CTRL_DEFAULT_VAL  (0x00000008)

// SMMU
#define DSS_SMMU_RLD_EN0_DEFAULT_VAL    (0xffffffff)
#define DSS_SMMU_RLD_EN1_DEFAULT_VAL    (0xffffff8f)
#define DSS_SMMU_OUTSTANDING_VAL		(0xf)

// MIF
#define DSS_MIF_CTRL2_INVAL_SEL3_STRIDE_MASK		(0xc)

// AFBCE
#define DSS_AFBCE_ENC_OS_CFG_DEFAULT_VAL			(0x7)

#define TUI_SEC_RCH			(DSS_RCHN_V0)

#define DSS_CHN_MAX_DEFINE (DSS_COPYBIT_MAX)

#define SMMU_SID_NUM	(64)

#define PRE_ENABLED_IN_UEFI             true
#define NEED_PRE_ENABLE_NEXT            false

#endif

