/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#define CONFIG_DSS_LP_USED
#define CONFIG_BACKLIGHT_10000

//GPIO
#define GPIO_LCD_POWER_1V2  (54)     //GPIO_6_6
#define GPIO_LCD_STANDBY    (67)     //GPIO_8_3
#define GPIO_LCD_RESETN     (65)     //GPIO_8_1
#define GPIO_LCD_GATING     (60)     //GPIO_7_4
#define GPIO_LCD_PCLK_GATING (58)    //GPIO_7_2
#define GPIO_LCD_REFCLK_GATING (59)  //GPIO_7_3
#define GPIO_LCD_SPICS         (168) //GPIO_21_0
#define GPIO_LCD_DRV_EN        (73)  //GPIO_9_1

#define GPIO_PG_SEL_A (40)    //GPIO_5_0
#define GPIO_TX_RX_A (42)    //GPIO_5_2
#define GPIO_PG_SEL_B (44)    //GPIO_5_4
#define GPIO_TX_RX_B (46)    //GPIO_5_6

#define OVL_LAYER_NUM_MAX (7)
#define OVL_PATTERN_RATIO (1)

//400 KB
#define MMBUF_SIZE_MAX	(400 * 1024)
#define MMBUF_SIZE_MDC_MAX (0)
#define HISI_DSS_CMDLIST_MAX	(14)
#define HISI_DSS_CMDLIST_IDXS_MAX (0x3FFF)   //14 cmdlist, 14bit, 11111111111111=0x3FFF
#define HISI_DSS_DPP_MAX_SUPPORT_BIT (0x7ff) //10 dpp module, 10bit, contrast to enmu dpp_module_idx
#define HISIFB_DSS_PLATFORM_TYPE  (FB_ACCEL_HI365x | FB_ACCEL_PLATFORM_TYPE_ASIC)

/* for DFS */
/* 1480 * 144bits */
#define DFS_TIME	(100)
#define DBUF0_DEPTH	(1408)

/* scl */
#define SCF_LINE_BUF	(2560)

#define DSS_MIF_SMMU_SMRX_IDX_STEP (16)

//PERI REG
#define CRG_PERI_DIS3_DEFAULT_VAL     (0x0002F000)

//SYSCTRL
#define SCPWREN 	(0x0D0)
#define SCPEREN1    (0x040)
#define SCPERDIS1   (0x044)
#define SCPERCLKEN1 (0x048)
#define SCPERRSTDIS1	(0x090)
#define SCISODIS	    (0x0C4)

//DSS global
#define DSS_GLB_MODULE_CLK_SEL_DEFAULT_VAL  (0xF0000008)


//LDI0 clk sel
#define DSS_LDI_CLK_SEL_DEFAULT_VAL    (0x00000004)

//DBuf mem ctrl
#define DSS_DBUF_MEM_CTRL_DEFAULT_VAL  (0x00000008)

//SMMU
#define DSS_SMMU_RLD_EN0_DEFAULT_VAL    (0xfffefffe)
#define DSS_SMMU_RLD_EN1_DEFAULT_VAL    (0xfffefffe)
#define DSS_SMMU_OUTSTANDING_VAL		(0xf)

//MIF
#define DSS_MIF_CTRL2_INVAL_SEL3_STRIDE_MASK		(0xc)

//AFBCE
#define DSS_AFBCE_ENC_OS_CFG_DEFAULT_VAL			(0x7)

#define TUI_SEC_RCH			(DSS_RCHN_V0)

#define DSS_CHN_MAX_DEFINE (DSS_CHN_MAX)

/*******************************************************************************
**
*/
#define CRGPERI_PLL0_CLK_RATE	(1440000000UL)
#define CRGPERI_PLL2_CLK_RATE	(1290000000UL)
#define CRGPERI_PLL3_CLK_RATE	(1600000000UL)

#define DEFAULT_DSS_CORE_CLK_RATE	(480000000UL)
#define DEFAULT_PCLK_DSS_RATE	(120000000UL)
#define DEFAULT_PCLK_PCTRL_RATE	(120000000UL)

#define SMMU_SID_NUM	(64)

/*******************************************************************************
**
*/
typedef struct dss_smmu {
	uint32_t smmu_scr;
	uint32_t smmu_memctrl;
	uint32_t smmu_lp_ctrl;
	uint32_t smmu_intmas_s;
	uint32_t smmu_intraw_s;
	uint32_t smmu_intstat_s;
	uint32_t smmu_intclr_s;
	uint32_t smmu_intmask_ns;
	uint32_t smmu_intraw_ns;
	uint32_t smmu_intstat_ns;
	uint32_t smmu_intclr_ns;
	uint32_t smmu_smrx[SMMU_SID_NUM];
	uint32_t smmu_rld_en0;
	uint32_t smmu_rld_en1;
	uint32_t smmu_scb_sctrl;
	uint32_t smmu_cb_sctrl;
	uint32_t smmu_scb_ttbr;
	uint32_t smmu_cb_ttbr0;
	uint32_t smmu_cb_ttbr1;
	uint32_t smmu_scb_ttbcr;
	uint32_t smmu_cb_ttbcr;
	uint32_t smmu_offset_addr_s;
	uint32_t smmu_offset_addr_ns;
	uint32_t smmu_addr_msb;
	uint32_t smmu_err_rdaddr;
	uint32_t smmu_err_wraddr;
	uint32_t smmu_fault_addr;
	uint32_t smmu_fault_id;
	uint32_t smmu_fault_info;
	uint32_t smmu_dbgrptr_tlb;
	uint32_t smmu_dbgrdata_tlb;
	uint32_t smmu_dbgrptr_cache;
	uint32_t smmu_dbgrdata0_cache;
	uint32_t smmu_dbgrdata1_cache;
	uint32_t smmu_scachei_all;
	uint32_t smmu_scachei_l1;
	uint32_t smmu_scachei_l2l3;
	uint32_t smmu_ova_addr;
	uint32_t smmu_opa_addr;
	uint32_t smmu_ova_ctrl;
	uint32_t smmu_opref_addr;
	uint32_t smmu_opref_ctrl;
	uint32_t smmu_opref_cnt;

	uint8_t smmu_smrx_used[DSS_CHN_MAX_DEFINE];
} dss_smmu_t;

typedef struct dss_ce {
	uint32_t hist_reset;
	uint32_t hist_ctl;
	uint32_t hist_frame_cnt;
	uint32_t size;
	uint32_t no_stat_lines;
	uint32_t black_region_thre;
	uint32_t white_region_thre;
	uint32_t lut_sel;
	uint32_t lut_enable;
	uint32_t mem_ctrl;

	uint32_t totalgray;
	uint32_t totalpixl;
	uint32_t mean;
	uint32_t pec0_3;
	uint32_t pec4_7;
	uint32_t pec8;
	uint32_t black_num;
	uint32_t white_num;
	uint32_t hist_rpt_ind;
	char __iomem *lut_base;
	uint8_t *lut_table;
} dss_ce_t;

typedef struct dss_mctl_ch_base {
	char __iomem *chn_mutex_base;
	char __iomem *chn_flush_en_base;
	char __iomem *chn_ov_en_base;
	char __iomem *chn_starty_base;
	char __iomem *chn_mod_dbg_base;
} dss_mctl_ch_base_t;

typedef struct dirty_region_updt {
	uint32_t dbuf_frm_size;
	uint32_t dbuf_frm_hsize;
	uint32_t dpp_img_hrz;
	uint32_t dpp_img_vrt;
	uint32_t ldi_dpi0_hrz_ctrl0;
	uint32_t ldi_dpi0_hrz_ctrl1;
	uint32_t ldi_dpi0_hrz_ctrl2;
	uint32_t ldi_dpi1_hrz_ctrl0;
	uint32_t ldi_dpi1_hrz_ctrl1;
	uint32_t ldi_dpi1_hrz_ctrl2;
	uint32_t ldi_vrt_ctrl0;
	uint32_t ldi_vrt_ctrl1;
	uint32_t ldi_vrt_ctrl2;
	uint32_t ldi_ctrl;
	uint32_t ifbc_size;
	uint32_t edpi_cmd_size;
}dirty_region_updt_t;

typedef struct dss_module_reg {
	char __iomem *aif_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *aif1_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *mif_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *dma_base[DSS_CHN_MAX_DEFINE];
	char __iomem *dfc_base[DSS_CHN_MAX_DEFINE];
	char __iomem *scl_base[DSS_CHN_MAX_DEFINE];
	char __iomem *sharp_base[DSS_CHN_MAX_DEFINE];
	char __iomem *post_clip_base[DSS_CHN_MAX_DEFINE];
	char __iomem *ce_base[DSS_CHN_MAX_DEFINE];
	char __iomem *csc_base[DSS_CHN_MAX_DEFINE];
	char __iomem *ov_base[DSS_OVL_IDX_MAX];
	char __iomem *mctl_base[DSS_MCTL_IDX_MAX];
	dss_mctl_ch_base_t mctl_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *mctl_sys_base;
	char __iomem *smmu_base;
	char __iomem *post_scf_base;

	dss_aif_t aif[DSS_CHN_MAX_DEFINE];
	dss_aif_t aif1[DSS_CHN_MAX_DEFINE];
	dss_aif_bw_t aif_bw[DSS_CHN_MAX_DEFINE];
	dss_aif_bw_t aif1_bw[DSS_CHN_MAX_DEFINE];
	dss_mif_t mif[DSS_CHN_MAX_DEFINE];
	dss_rdma_t rdma[DSS_CHN_MAX_DEFINE];
	dss_wdma_t wdma[DSS_CHN_MAX_DEFINE];
	dss_dfc_t dfc[DSS_CHN_MAX_DEFINE];
	dss_scl_t scl[DSS_CHN_MAX_DEFINE];
	dss_sharpness_t sharp[DSS_CHN_MAX_DEFINE];
	dss_post_clip_t post_clip[DSS_CHN_MAX_DEFINE];
	dss_ce_t ce[DSS_CHN_MAX_DEFINE];
	dss_csc_t csc[DSS_CHN_MAX_DEFINE];
	dss_ovl_t ov[DSS_OVL_IDX_MAX];
	dss_mctl_t mctl[DSS_MCTL_IDX_MAX];
	dss_mctl_ch_t mctl_ch[DSS_CHN_MAX_DEFINE];
	dss_mctl_sys_t mctl_sys;
	dss_smmu_t smmu;
	dirty_region_updt_t dirty_region_updt;
	dss_scl_t post_scf;

	uint8_t aif_ch_used[DSS_CHN_MAX_DEFINE];
	uint8_t aif1_ch_used[DSS_CHN_MAX_DEFINE];
	uint8_t mif_used[DSS_CHN_MAX_DEFINE];
	uint8_t dma_used[DSS_CHN_MAX_DEFINE];
	uint8_t dfc_used[DSS_CHN_MAX_DEFINE];
	uint8_t scl_used[DSS_CHN_MAX_DEFINE];
	uint8_t sharp_used[DSS_CHN_MAX_DEFINE];
	uint8_t post_clip_used[DSS_CHN_MAX_DEFINE];
	uint8_t ce_used[DSS_CHN_MAX_DEFINE];
	uint8_t csc_used[DSS_CHN_MAX_DEFINE];
	uint8_t ov_used[DSS_OVL_IDX_MAX];
	uint8_t ch_reg_default_used[DSS_CHN_MAX_DEFINE];
	uint8_t mctl_used[DSS_MCTL_IDX_MAX];
	uint8_t mctl_ch_used[DSS_CHN_MAX_DEFINE];
	uint8_t mctl_sys_used;
	uint8_t smmu_used;
	uint8_t dirty_region_updt_used;
	uint8_t post_scf_used;
} dss_module_reg_t;

typedef struct dss_mmbuf_info {
	uint32_t mm_base[DSS_CHN_MAX_DEFINE];
	uint32_t mm_size[DSS_CHN_MAX_DEFINE];

	uint8_t mm_used[DSS_CHN_MAX_DEFINE];
} dss_mmbuf_info_t;


#endif
