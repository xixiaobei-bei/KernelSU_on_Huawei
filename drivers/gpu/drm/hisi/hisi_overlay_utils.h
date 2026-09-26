
#ifndef _HISI_OVERLAY_UTILS_H_
#define _HISI_OVERLAY_UTILS_H_

#include "hisi_overlay_utils_dssv510.h"

#define DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC	(300)

/* GPIO */
#define GPIO_LCD_POWER_1V2     (54)   /* GPIO_6_6 */
#define GPIO_LCD_STANDBY       (67)   /* GPIO_8_3 */
#define GPIO_LCD_RESETN        (65)   /* GPIO_8_1 */
#define GPIO_LCD_GATING        (60)   /* GPIO_7_4 */
#define GPIO_LCD_PCLK_GATING   (58)   /* GPIO_7_2 */
#define GPIO_LCD_REFCLK_GATING (59)   /* GPIO_7_3 */
#define GPIO_LCD_SPICS         (168)  /* GPIO_21_0 */
#define GPIO_LCD_DRV_EN        (73)   /* GPIO_9_1 */

#define GPIO_PG_SEL_A (168)    /* GPIO_21_0 */
#define GPIO_TX_RX_A  (170)    /* GPIO_21_2 */
#define GPIO_PG_SEL_B (133)    /* GPIO_16_5 */
#define GPIO_TX_RX_B  (135)    /* GPIO_16_7 */

struct dss_mctl_ch_base {
	char __iomem *chn_mutex_base;
	char __iomem *chn_flush_en_base;
	char __iomem *chn_ov_en_base;
	char __iomem *chn_starty_base;
	char __iomem *chn_mod_dbg_base;
};
#ifdef CONFIG_HISI_DSS_V510
struct dss_smmu {
	uint32_t smmu_scr;
	uint32_t smmu_memctrl;
	uint32_t smmu_lp_ctrl;
	uint32_t smmu_press_remap;
	uint32_t smmu_intmask_ns;
	uint32_t smmu_intraw_ns;
	uint32_t smmu_intstat_ns;
	uint32_t smmu_intclr_ns;
	uint32_t smmu_smrx_ns[SMMU_SID_NUM];
	uint32_t smmu_rld_en0_ns;
	uint32_t smmu_rld_en1_ns;
	uint32_t smmu_rld_en2_ns;
	uint32_t smmu_cb_sctrl;
	uint32_t smmu_cb_ttbr0;
	uint32_t smmu_cb_ttbr1;
	uint32_t smmu_cb_ttbcr;
	uint32_t smmu_offset_addr_ns;
	uint32_t smmu_scachei_all;
	uint32_t smmu_scachei_l1;
	uint32_t smmu_scachei_l2l3;
	uint32_t smmu_fama_ctrl0_ns;
	uint32_t smmu_fama_ctrl1_ns;
	uint32_t smmu_addr_msb;
	uint32_t smmu_err_rdaddr;
	uint32_t smmu_err_wraddr;
	uint32_t smmu_fault_addr_tcu;
	uint32_t smmu_fault_id_tcu;
	uint32_t smmu_fault_addr_tbux;
	uint32_t smmu_fault_id_tbux;
	uint32_t smmu_fault_infox;
	uint32_t smmu_dbgrptr_tlb;
	uint32_t smmu_dbgrdata_tlb;
	uint32_t smmu_dbgrptr_cache;
	uint32_t smmu_dbgrdata0_cache;
	uint32_t smmu_dbgrdata1_cache;
	uint32_t smmu_dbgaxi_ctrl;
	uint32_t smmu_ova_addr;
	uint32_t smmu_opa_addr;
	uint32_t smmu_ova_ctrl;
	uint32_t smmu_opref_addr;
	uint32_t smmu_opref_ctrl;
	uint32_t smmu_opref_cnt;
	uint32_t smmu_smrx_s[SMMU_SID_NUM];
	uint32_t smmu_rld_en0_s;
	uint32_t smmu_rld_en1_s;
	uint32_t smmu_rld_en2_s;
	uint32_t smmu_intmas_s;
	uint32_t smmu_intraw_s;
	uint32_t smmu_intstat_s;
	uint32_t smmu_intclr_s;
	uint32_t smmu_scr_s;
	uint32_t smmu_scb_sctrl;
	uint32_t smmu_scb_ttbr;
	uint32_t smmu_scb_ttbcr;
	uint32_t smmu_offset_addr_s;

	uint8_t smmu_smrx_ns_used[DSS_CHN_MAX_DEFINE];
};
#else
typedef union {
	unsigned int value;
	struct {
		unsigned int ch_sid          : 8;  /* bit[7:0]  */
		unsigned int reserved_0      : 4;  /* bit[11:8] */
		unsigned int ch_ssid         : 8;  /* bit[19:12]  */
		unsigned int ch_ssidv        : 1;  /* bit[20] */
		unsigned int reserved_1      : 3;  /* bit[23:21] */
		unsigned int ch_sc_hint      : 4;  /* bit[27:24] */
		unsigned int ch_sc_hint_en   : 1;  /* bit[28] */
		unsigned int ch_ptl_as_ful   : 1;  /* bit[29] */
		unsigned int reserved_2      : 2;  /* bit[31:30] */
	} reg;
} soc_dss_mmu_id_attr;

typedef struct dss_smmu {
	unsigned int smmu_smrx_ns[SMMU_SID_NUM]; /* for smmu v2 */
	unsigned char smmu_smrx_ns_used[DSS_CHN_MAX_DEFINE];

	soc_dss_mmu_id_attr ch_mmu_attr[DSS_CHN_MAX_DEFINE]; /* for smmu v3 */
} dss_smmu_t;
#endif
/* V510 */
struct arsr2p_info {
	uint32_t enable;
	/* enable switch */
	uint32_t sharp_enable;
	uint32_t skin_enable;
	uint32_t shoot_enable;

	/* registers */
	uint32_t skin_thres_y;
	uint32_t skin_thres_u;
	uint32_t skin_thres_v;
	uint32_t skin_cfg0;
	uint32_t skin_cfg1;
	uint32_t skin_cfg2;
	uint32_t shoot_cfg1;
	uint32_t shoot_cfg2;
	uint32_t shoot_cfg3;
	uint32_t sharp_cfg3;
	uint32_t sharp_cfg4;
	uint32_t sharp_cfg6;
	uint32_t sharp_cfg7;
	uint32_t sharp_cfg8;
	uint32_t sharp_level;
	uint32_t sharp_gain_low;
	uint32_t sharp_gain_mid;
	uint32_t sharp_gain_high;
	uint32_t sharp_gainctrl_sloph_mf;
	uint32_t sharp_gainctrl_slopl_mf;
	uint32_t sharp_gainctrl_sloph_hf;
	uint32_t sharp_gainctrl_slopl_hf;
	uint32_t sharp_mf_lmt;
	uint32_t sharp_gain_mf;
	uint32_t sharp_mf_b;
	uint32_t sharp_hf_lmt;
	uint32_t sharp_gain_hf;
	uint32_t sharp_hf_b;
	uint32_t sharp_lf_ctrl;
	uint32_t sharp_lf_var;
	uint32_t sharp_lf_ctrl_slop;
	uint32_t sharp_hf_select;
	uint32_t sharp_cfg2_h;
	uint32_t sharp_cfg2_l;
	uint32_t texture_analysis;
	uint32_t intplshootctrl;

	uint32_t update;
};

struct dss_arsr2p {
	uint32_t arsr_input_width_height;
	uint32_t arsr_output_width_height;
	uint32_t ihleft;
	uint32_t ihright;
	uint32_t ivtop;
	uint32_t ivbottom;
	uint32_t ihinc;
	uint32_t ivinc;
	uint32_t offset;
	uint32_t mode;
	struct arsr2p_info arsr2p_effect;
	struct arsr2p_info arsr2p_effect_scale_up;
	struct arsr2p_info arsr2p_effect_scale_down;
	uint32_t ihleft1;
	uint32_t ihright1;
	uint32_t ivbottom1;
};

struct dirty_region_updt {
	uint32_t dbuf_frm_size;
	uint32_t dbuf_frm_hsize;
	uint32_t dpp_img_size_bef_sr;
	uint32_t dpp_img_size_aft_sr;
	uint32_t dpp_img_size_aft_ifbcsw;
	uint32_t hiace_img_size;
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
	dss_arsr1p_t s_arsr1p;
};

/*******************************************************************************
 **dss module reg
 */
struct dss_module_reg {
	char __iomem *mif_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *aif_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *aif1_ch_base[DSS_CHN_MAX_DEFINE];
	struct dss_mctl_ch_base mctl_ch_base[DSS_CHN_MAX_DEFINE];
	char __iomem *dma_base[DSS_CHN_MAX_DEFINE];
	char __iomem *dfc_base[DSS_CHN_MAX_DEFINE];
	char __iomem *scl_base[DSS_CHN_MAX_DEFINE];
	char __iomem *scl_lut_base[DSS_CHN_MAX_DEFINE];
	char __iomem *arsr2p_base[DSS_CHN_MAX_DEFINE];
	char __iomem *arsr2p_lut_base[DSS_CHN_MAX_DEFINE];
	char __iomem *post_clip_base[DSS_CHN_MAX_DEFINE];
	char __iomem *pcsc_base[DSS_CHN_MAX_DEFINE];
	char __iomem *csc_base[DSS_CHN_MAX_DEFINE];

	char __iomem *ov_base[DSS_OVL_IDX_MAX];
	char __iomem *mctl_base[DSS_MCTL_IDX_MAX];
	char __iomem *mctl_sys_base;
	char __iomem *smmu_base;
	char __iomem *post_scf_base;

	dss_mif_t mif[DSS_CHN_MAX_DEFINE];
	dss_aif_t aif[DSS_CHN_MAX_DEFINE];
	dss_aif_t aif1[DSS_CHN_MAX_DEFINE];
	dss_aif_bw_t aif_bw[DSS_CHN_MAX_DEFINE];
	dss_aif_bw_t aif1_bw[DSS_CHN_MAX_DEFINE];
	dss_rdma_t rdma[DSS_CHN_MAX_DEFINE];
	dss_wdma_t wdma[DSS_CHN_MAX_DEFINE];
	dss_dfc_t dfc[DSS_CHN_MAX_DEFINE];
	dss_scl_t scl[DSS_CHN_MAX_DEFINE];
	struct dss_arsr2p arsr2p[DSS_CHN_MAX_DEFINE];
	dss_post_clip_t post_clip[DSS_CHN_MAX_DEFINE];
	dss_csc_t pcsc[DSS_CHN_MAX_DEFINE];
	dss_csc_t csc[DSS_CHN_MAX_DEFINE];
	dss_ovl_t ov[DSS_OVL_IDX_MAX];
	dss_mctl_t mctl[DSS_MCTL_IDX_MAX];
	dss_mctl_ch_t mctl_ch[DSS_CHN_MAX_DEFINE];
	dss_mctl_sys_t mctl_sys;
	struct dss_smmu smmu;
	struct dirty_region_updt dirty_region_updt;
	dss_arsr1p_t post_scf;

	uint8_t mif_used[DSS_CHN_MAX_DEFINE];
	uint8_t aif_ch_used[DSS_CHN_MAX_DEFINE];
	uint8_t aif1_ch_used[DSS_CHN_MAX_DEFINE];
	uint8_t dma_used[DSS_CHN_MAX_DEFINE];
	uint8_t dfc_used[DSS_CHN_MAX_DEFINE];
	uint8_t scl_used[DSS_CHN_MAX_DEFINE];
	uint8_t arsr2p_used[DSS_CHN_MAX_DEFINE];
	uint8_t arsr2p_effect_used[DSS_CHN_MAX_DEFINE];
	uint8_t post_clip_used[DSS_CHN_MAX_DEFINE];
	uint8_t pcsc_used[DSS_CHN_MAX_DEFINE];
	uint8_t csc_used[DSS_CHN_MAX_DEFINE];
	uint8_t ov_used[DSS_OVL_IDX_MAX];
	uint8_t ch_reg_default_used[DSS_CHN_MAX_DEFINE];
	uint8_t mctl_used[DSS_MCTL_IDX_MAX];
	uint8_t mctl_ch_used[DSS_CHN_MAX_DEFINE];
	uint8_t mctl_sys_used;
	uint8_t smmu_used;
	uint8_t dirty_region_updt_used;
	uint8_t post_scf_used;
};

struct dss_mmbuf_info {
	uint32_t mm_base[DSS_CHN_MAX_DEFINE];
	uint32_t mm_size[DSS_CHN_MAX_DEFINE];
	uint32_t mm_base0_y8[DSS_CHN_MAX_DEFINE];
	uint32_t mm_size0_y8[DSS_CHN_MAX_DEFINE];
	uint32_t mm_base1_c8[DSS_CHN_MAX_DEFINE];
	uint32_t mm_size1_c8[DSS_CHN_MAX_DEFINE];
	uint32_t mm_base2_y2[DSS_CHN_MAX_DEFINE];
	uint32_t mm_size2_y2[DSS_CHN_MAX_DEFINE];
	uint32_t mm_base3_c2[DSS_CHN_MAX_DEFINE];
	uint32_t mm_size3_c2[DSS_CHN_MAX_DEFINE];

	uint8_t mm_used[DSS_CHN_MAX_DEFINE];
};
#endif

