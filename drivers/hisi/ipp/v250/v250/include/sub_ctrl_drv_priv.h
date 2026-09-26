// ******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  sub_ctrl_drv_priv.h
// Version       :  1.0
// Date          :  2018-07-02
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// ******************************************************************************

#ifndef __SUB_CTRL_DRV_PRIV_CS_H__
#define __SUB_CTRL_DRV_PRIV_CS_H__

/* Define the union U_DMA_CRG_CFG0 */
union u_dma_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    jpg_dw_axi_gatedclock_en : 1   ; /* [0]  */
		unsigned int    jpg_top_apb_force_clk_on : 1   ; /* [1]  */
		unsigned int    reserved_0            : 14  ; /* [15..2]  */
		unsigned int    control_disable_axi_data_packing : 1   ; /* [16]  */
		unsigned int    mst_priority_fd       : 1   ; /* [17]  */
		unsigned int    mst_priority_cvdr     : 1   ; /* [18]  */
		unsigned int    apb_overf_prot        : 2   ; /* [20..19]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_DMA_CRG_CFG1 */
union u_dma_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_soft_rst         : 1   ; /* [0]  */
		unsigned int    smmu_soft_rst         : 1   ; /* [1]  */
		unsigned int    smmu_master_soft_rst  : 1   ; /* [2]  */
		unsigned int    cmdlst_soft_rst       : 1   ; /* [3]  */
		unsigned int    reserved_0            : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_MEM_CFG0 */
union u_cvdr_mem_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_mem_ctrl_sp      : 3   ; /* [2..0]  */
		unsigned int    mem_ctrl_sp           : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_MEM_CFG1 */
union u_cvdr_mem_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 3   ; /* [2..0]  */
		unsigned int    mem_ctrl_tp           : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_IRQ_REG0 */
union u_cvdr_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_irq_clr          : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_IRQ_REG1 */
union u_cvdr_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_irq_mask         : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_IRQ_REG2 */
union u_cvdr_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    cvdr_irq_state_mask   : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cvdr_irq_state_raw    : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CTRL */
union u_cmdlst_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ctrl_chn0      : 3   ; /* [2..0]  */
		unsigned int    cmdlst_ctrl_chn1      : 3   ; /* [5..3]  */
		unsigned int    cmdlst_ctrl_chn2      : 3   ; /* [8..6]  */
		unsigned int    cmdlst_ctrl_chn3      : 3   ; /* [11..9]  */
		unsigned int    cmdlst_ctrl_chn4      : 3   ; /* [14..12]  */
		unsigned int    cmdlst_ctrl_chn5      : 3   ; /* [17..15]  */
		unsigned int    cmdlst_ctrl_chn6      : 3   ; /* [20..18]  */
		unsigned int    cmdlst_ctrl_chn7      : 3   ; /* [23..21]  */
		unsigned int    reserved_0            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN0 */
union u_cmdlst_chn0 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn0  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN1 */
union u_cmdlst_chn1 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn1  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN2 */
union u_cmdlst_chn2 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn2  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN3 */
union u_cmdlst_chn3 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn3  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN4 */
union u_cmdlst_chn4 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn4  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN5 */
union u_cmdlst_chn5 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn5  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN6 */
union u_cmdlst_chn6 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn6  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CHN7 */
union u_cmdlst_chn7 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_eof_mask_chn7  : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_R8_IRQ_REG0 */
union u_cmdlst_r8_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_r8_irq_clr     : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_R8_IRQ_REG1 */
union u_cmdlst_r8_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_r8_irq_mask    : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_R8_IRQ_REG2 */
union u_cmdlst_r8_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_r8_irq_state_mask : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cmdlst_r8_irq_state_raw : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_ACPU_IRQ_REG0 */
union u_cmdlst_acpu_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_acpu_irq_clr   : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_ACPU_IRQ_REG1 */
union u_cmdlst_acpu_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_acpu_irq_mask  : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_ACPU_IRQ_REG2 */
union u_cmdlst_acpu_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_acpu_irq_state_mask : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cmdlst_acpu_irq_state_raw : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_IVP_IRQ_REG0 */
union u_cmdlst_ivp_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ivp_irq_clr    : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_IVP_IRQ_REG1 */
union u_cmdlst_ivp_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ivp_irq_mask   : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_IVP_IRQ_REG2 */
union u_cmdlst_ivp_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    cmdlst_ivp_irq_state_mask : 8   ; /* [7..0]  */
		unsigned int    reserved_0            : 8   ; /* [15..8]  */
		unsigned int    cmdlst_ivp_irq_state_raw : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPG_FLUX_CTRL0_0 */
union u_jpg_flux_ctrl0_0 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_cvdr_r      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL0_1 */
union u_jpg_flux_ctrl0_1 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_cvdr_r      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL1_0 */
union u_jpg_flux_ctrl1_0 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_cvdr_w      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL1_1 */
union u_jpg_flux_ctrl1_1 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_cvdr_w      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL2_0 */
union u_jpg_flux_ctrl2_0 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_fd_r        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL2_1 */
union u_jpg_flux_ctrl2_1 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_fd_r        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL3_0 */
union u_jpg_flux_ctrl3_0 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl0_fd_w        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_FLUX_CTRL3_1 */
union u_jpg_flux_ctrl3_1 {
	/* Define the struct bits  */
	struct {
		unsigned int flux_ctrl1_fd_w        : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_RO_STATE */
union u_jpg_ro_state {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 16  ; /* [15..0]  */
		unsigned int    jpg_axi_dlock_irq     : 1   ; /* [16]  */
		unsigned int    jpg_axi_dlock_wr      : 1   ; /* [17]  */
		unsigned int    jpg_axi_dlock_slv     : 1   ; /* [18]  */
		unsigned int    jpg_axi_dlock_mst     : 1   ; /* [19]  */
		unsigned int    jpg_axi_dlock_id      : 8   ; /* [27..20]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_CRG_CFG0 */
union u_jpgenc_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_clken          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgenc_force_clk_on   : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_CRG_CFG1 */
union u_jpgenc_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_soft_rst       : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_MEM_CFG */
union u_jpgenc_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_mem_ctrl_sp    : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_PREF_STOP */
union u_jpgenc_pref_stop {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_prefetch_stop  : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgenc_prefetch_stop_ok : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_IRQ_REG0 */
union u_jpgenc_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_irq_clr        : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    jpgenc_irq_force      : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_IRQ_REG1 */
union u_jpgenc_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_irq_mask       : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGENC_IRQ_REG2 */
union u_jpgenc_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgenc_irq_state_mask : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    jpgenc_irq_state_raw  : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_CRG_CFG0 */
union u_jpgdec_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_clken          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgdec_force_clk_on   : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_CRG_CFG1 */
union u_jpgdec_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_soft_rst       : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_MEM_CFG */
union u_jpgdec_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_mem_ctrl_sp    : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    jpgdec_mem_ctrl_tp    : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_PREF_STOP */
union u_jpgdec_pref_stop {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_prefetch_stop  : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    jpgdec_prefetch_stop_ok : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_IRQ_REG0 */
union u_jpgdec_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_irq_clr        : 4   ; /* [3..0]  */
		unsigned int    reserved_0            : 12  ; /* [15..4]  */
		unsigned int    jpgdec_irq_force      : 4   ; /* [19..16]  */
		unsigned int    reserved_1            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_IRQ_REG1 */
union u_jpgdec_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_irq_mask       : 4   ; /* [3..0]  */
		unsigned int    reserved_0            : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPGDEC_IRQ_REG2 */
union u_jpgdec_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    jpgdec_irq_state_mask : 4   ; /* [3..0]  */
		unsigned int    reserved_0            : 12  ; /* [15..4]  */
		unsigned int    jpgdec_irq_state_raw  : 4   ; /* [19..16]  */
		unsigned int    reserved_1            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_CRG_CFG0 */
union u_cpe_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_clken             : 1   ; /* [0]  */
		unsigned int    mfnr_clken            : 1   ; /* [1]  */
		unsigned int    vbk_clken             : 1   ; /* [2]  */
		unsigned int    reserved_0            : 13  ; /* [15..3]  */
		unsigned int    mcf_force_clk_on      : 1   ; /* [16]  */
		unsigned int    mfnr_force_clk_on     : 1   ; /* [17]  */
		unsigned int    vbk_force_clk_on      : 1   ; /* [18]  */
		unsigned int    reserved_1            : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_CRG_CFG1 */
union u_cpe_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_soft_rst          : 1   ; /* [0]  */
		unsigned int    mfnr_soft_rst         : 1   ; /* [1]  */
		unsigned int    vbk_soft_rst          : 1   ; /* [2]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_MEM_CFG */
union u_cpe_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_mem_ctrl_sp       : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    mfnr_mem_ctrl_sp      : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    vbk_mem_ctrl_sp       : 3   ; /* [10..8]  */
		unsigned int    reserved_2            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_IRQ_REG0 */
union u_cpe_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_clr           : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_IRQ_REG1 */
union u_cpe_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_mask          : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_IRQ_REG2 */
union u_cpe_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    mcf_irq_outen         : 2   ; /* [1..0]  */
		unsigned int    mfnr_irq_outen        : 2   ; /* [3..2]  */
		unsigned int    vbk_irq_outen         : 2   ; /* [5..4]  */
		unsigned int    reserved_0            : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_IRQ_REG3 */
union u_cpe_irq_reg3 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_state_mask    : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_IRQ_REG4 */
union u_cpe_irq_reg4 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_irq_state_raw     : 23  ; /* [22..0]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CROP_VPWR_0 */
union u_crop_vpwr_0 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_vpwr0_ihleft      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    cpe_vpwr0_ihright     : 11  ; /* [26..16]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CROP_VPWR_1 */
union u_crop_vpwr_1 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_vpwr1_ihleft      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    cpe_vpwr1_ihright     : 11  ; /* [26..16]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CROP_VPWR_2 */
union u_crop_vpwr_2 {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_vpwr2_ihleft      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    cpe_vpwr2_ihright     : 11  ; /* [26..16]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CPE_mode_CFG */
union u_cpe_mode_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    cpe_op_mode           : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SLAM_CRG_CFG0 */
union u_slam_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    slam_clken            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    slam_force_clk_on     : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SLAM_CRG_CFG1 */
union u_slam_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    slam_soft_rst         : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SLAM_MEM_CFG */
union u_slam_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    slam_mem_ctrl_sp      : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SLAM_IRQ_REG0 */
union u_slam_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    slam_irq_clr          : 14  ; /* [13..0]  */
		unsigned int    reserved_0            : 2   ; /* [15..14]  */
		unsigned int    slam_irq_force        : 14  ; /* [29..16]  */
		unsigned int    reserved_1            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SLAM_IRQ_REG1 */
union u_slam_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    slam_irq_mask         : 14  ; /* [13..0]  */
		unsigned int    reserved_0            : 2   ; /* [15..14]  */
		unsigned int    slam_irq_outen        : 2   ; /* [17..16]  */
		unsigned int    reserved_1            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SLAM_IRQ_REG2 */
union u_slam_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    slam_irq_state_mask   : 14  ; /* [13..0]  */
		unsigned int    reserved_0            : 2   ; /* [15..14]  */
		unsigned int    slam_irq_state_raw    : 14  ; /* [29..16]  */
		unsigned int    reserved_1            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_CRG_CFG0 */
union u_rdr_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_clken             : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    rdr_force_clk_on      : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_CRG_CFG1 */
union u_rdr_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_soft_rst          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_MEM_CFG */
union u_rdr_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_mem_ctrl_sp       : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_PREF_STOP */
union u_rdr_pref_stop {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_prefetch_stop     : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    rdr_prefetch_stop_ok  : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_IRQ_REG0 */
union u_rdr_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_irq_clr           : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    rdr_irq_force         : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_IRQ_REG1 */
union u_rdr_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_irq_mask          : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    rdr_irq_outen         : 2   ; /* [17..16]  */
		unsigned int    reserved_1            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_RDR_IRQ_REG2 */
union u_rdr_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    rdr_irq_state_mask    : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    rdr_irq_state_raw     : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_CRG_CFG0 */
union u_cmp_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_clken             : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    cmp_force_clk_on      : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_CRG_CFG1 */
union u_cmp_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_soft_rst          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_MEM_CFG */
union u_cmp_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_mem_ctrl_sp       : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_PREF_STOP */
union u_cmp_pref_stop {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_prefetch_stop     : 1   ; /* [0]  */
		unsigned int    reserved_0            : 15  ; /* [15..1]  */
		unsigned int    cmp_prefetch_stop_ok  : 1   ; /* [16]  */
		unsigned int    reserved_1            : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_IRQ_REG0 */
union u_cmp_irq_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_irq_clr           : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    cmp_irq_force         : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_IRQ_REG1 */
union u_cmp_irq_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_irq_mask          : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    cmp_irq_outen         : 2   ; /* [17..16]  */
		unsigned int    reserved_1            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMP_IRQ_REG2 */
union u_cmp_irq_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    cmp_irq_state_mask    : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 11  ; /* [15..5]  */
		unsigned int    cmp_irq_state_raw     : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_HIFD_CRG_CFG0 */
union u_hifd_crg_cfg0 {
	/* Define the struct bits */
	struct {
		unsigned int    hifd_clken            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;


};

/* Define the union U_HIFD_CRG_CFG1 */
union u_hifd_crg_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int    hifd_soft_rst         : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_HIFD_MEM_CFG */

union u_hifd_mem_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    hifd_mem_ctrl_sp      : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    hifd_mem_ctrl_sp2     : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    hifd_mem_ctrl_tp      : 3   ; /* [10..8]  */
		unsigned int    reserved_2            : 5   ; /* [15..11]  */
		unsigned int    hifd_rom_ctrl         : 8   ; /* [23..16]  */
		unsigned int    reserved_3            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_FD_SMMU_MASTER_REG0 */
union u_fd_smmu_master_reg0 {
	/* Define the struct bits */
	struct {
		unsigned int    fd_prefetch_initial   : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_FD_SMMU_MASTER_REG1 */
union u_fd_smmu_master_reg1 {
	/* Define the struct bits */
	struct {
		unsigned int    fd_stream_end         : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_FD_SMMU_MASTER_REG2 */
union u_fd_smmu_master_reg2 {
	/* Define the struct bits */
	struct {
		unsigned int    fd_stream_ack         : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_JPG_DEBUG_0 */
union u_jpg_debug_0 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_0           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_DEBUG_1 */
union u_jpg_debug_1 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_1           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_DEBUG_2 */
union u_jpg_debug_2 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_2           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_DEBUG_3 */
union u_jpg_debug_3 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_info_3           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_JPG_SEC_CTRL_S */
union u_jpg_sec_ctrl_s {
	/* Define the struct bits */
	struct {
		unsigned int    top_tz_secure_n       : 1   ; /* [0]  */
		unsigned int    jpgenc_tz_secure_n    : 1   ; /* [1]  */
		unsigned int    jpgdec_tz_secure_n    : 1   ; /* [2]  */
		unsigned int    fd_tz_secure_n        : 1   ; /* [3]  */
		unsigned int    cpe_tz_secure_n       : 1   ; /* [4]  */
		unsigned int    slam_tz_secure_n      : 1   ; /* [5]  */
		unsigned int    orb_tz_secure_n       : 1   ; /* [6]  */
		unsigned int    cmdlst_tz_secure_n    : 1   ; /* [7]  */
		unsigned int    reserved_0            : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

//==============================================================================
/* Define the global struct */
struct s_sub_ctrl_regs_type {
	union u_dma_crg_cfg0         dma_crg_cfg0;
	union u_dma_crg_cfg1         dma_crg_cfg1;
	union u_cvdr_mem_cfg0        cvdr_mem_cfg0;
	union u_cvdr_mem_cfg1        cvdr_mem_cfg1;
	union u_cvdr_irq_reg0        cvdr_irq_reg0;
	union u_cvdr_irq_reg1        cvdr_irq_reg1;
	union u_cvdr_irq_reg2        cvdr_irq_reg2;
	union u_cmdlst_ctrl          cmdlst_ctrl;
	union u_cmdlst_chn0          cmdlst_chn0;
	union u_cmdlst_chn1          cmdlst_chn1;
	union u_cmdlst_chn2          cmdlst_chn2;
	union u_cmdlst_chn3          cmdlst_chn3;
	union u_cmdlst_chn4          cmdlst_chn4;
	union u_cmdlst_chn5          cmdlst_chn5;
	union u_cmdlst_chn6          cmdlst_chn6;
	union u_cmdlst_chn7          cmdlst_chn7;
	union u_cmdlst_r8_irq_reg0   cmdlst_r8_irq_reg0;
	union u_cmdlst_r8_irq_reg1   cmdlst_r8_irq_reg1;
	union u_cmdlst_r8_irq_reg2   cmdlst_r8_irq_reg2;
	union u_cmdlst_acpu_irq_reg0 cmdlst_acpu_irq_reg0;
	union u_cmdlst_acpu_irq_reg1 cmdlst_acpu_irq_reg1;
	union u_cmdlst_acpu_irq_reg2 cmdlst_acpu_irq_reg2;
	union u_cmdlst_ivp_irq_reg0  cmdlst_ivp_irq_reg0;
	union u_cmdlst_ivp_irq_reg1  cmdlst_ivp_irq_reg1;
	union u_cmdlst_ivp_irq_reg2  cmdlst_ivp_irq_reg2;
	union u_jpg_flux_ctrl0_0     jpg_flux_ctrl0_0;
	union u_jpg_flux_ctrl0_1     jpg_flux_ctrl0_1;
	union u_jpg_flux_ctrl1_0     jpg_flux_ctrl1_0;
	union u_jpg_flux_ctrl1_1     jpg_flux_ctrl1_1;
	union u_jpg_flux_ctrl2_0     jpg_flux_ctrl2_0;
	union u_jpg_flux_ctrl2_1     jpg_flux_ctrl2_1;
	union u_jpg_flux_ctrl3_0     jpg_flux_ctrl3_0;
	union u_jpg_flux_ctrl3_1     jpg_flux_ctrl3_1;
	union u_jpg_ro_state         jpg_ro_state;
	union u_jpgenc_crg_cfg0      jpgenc_crg_cfg0;
	union u_jpgenc_crg_cfg1      jpgenc_crg_cfg1;
	union u_jpgenc_mem_cfg       jpgenc_mem_cfg;
	union u_jpgenc_pref_stop     jpgenc_pref_stop;
	union u_jpgenc_irq_reg0      jpgenc_irq_reg0;
	union u_jpgenc_irq_reg1      jpgenc_irq_reg1;
	union u_jpgenc_irq_reg2      jpgenc_irq_reg2;
	union u_jpgdec_crg_cfg0      jpgdec_crg_cfg0;
	union u_jpgdec_crg_cfg1      jpgdec_crg_cfg1;
	union u_jpgdec_mem_cfg       jpgdec_mem_cfg;
	union u_jpgdec_pref_stop     jpgdec_pref_stop;
	union u_jpgdec_irq_reg0      jpgdec_irq_reg0;
	union u_jpgdec_irq_reg1      jpgdec_irq_reg1;
	union u_jpgdec_irq_reg2      jpgdec_irq_reg2;
	union u_cpe_crg_cfg0         cpe_crg_cfg0;
	union u_cpe_crg_cfg1         cpe_crg_cfg1;
	union u_cpe_mem_cfg          cpe_mem_cfg;
	union u_cpe_irq_reg0         cpe_irq_reg0;
	union u_cpe_irq_reg1         cpe_irq_reg1;
	union u_cpe_irq_reg2         cpe_irq_reg2;
	union u_cpe_irq_reg3         cpe_irq_reg3;
	union u_cpe_irq_reg4         cpe_irq_reg4;
	union u_crop_vpwr_0          crop_vpwr_0;
	union u_crop_vpwr_1          crop_vpwr_1;
	union u_crop_vpwr_2          crop_vpwr_2;
	union u_cpe_mode_cfg         cpe_mode_cfg;
	union u_slam_crg_cfg0        slam_crg_cfg0;
	union u_slam_crg_cfg1        slam_crg_cfg1;
	union u_slam_mem_cfg         slam_mem_cfg;
	union u_slam_irq_reg0        slam_irq_reg0;
	union u_slam_irq_reg1        slam_irq_reg1;
	union u_slam_irq_reg2        slam_irq_reg2;
	union u_rdr_crg_cfg0         rdr_crg_cfg0;
	union u_rdr_crg_cfg1         rdr_crg_cfg1;
	union u_rdr_mem_cfg          rdr_mem_cfg;
	union u_rdr_pref_stop        rdr_pref_stop;
	union u_rdr_irq_reg0         rdr_irq_reg0;
	union u_rdr_irq_reg1         rdr_irq_reg1;
	union u_rdr_irq_reg2         rdr_irq_reg2;
	union u_cmp_crg_cfg0         cmp_crg_cfg0;
	union u_cmp_crg_cfg1         cmp_crg_cfg1;
	union u_cmp_mem_cfg          cmp_mem_cfg;
	union u_cmp_pref_stop        cmp_pref_stop;
	union u_cmp_irq_reg0         cmp_irq_reg0;
	union u_cmp_irq_reg1         cmp_irq_reg1;
	union u_cmp_irq_reg2         cmp_irq_reg2;
	union u_hifd_crg_cfg0        hifd_crg_cfg0;
	union u_hifd_crg_cfg1        hifd_crg_cfg1;
	union u_hifd_mem_cfg         hifd_mem_cfg;
	union u_fd_smmu_master_reg0  fd_smmu_master_reg0;
	union u_fd_smmu_master_reg1  fd_smmu_master_reg1;
	union u_fd_smmu_master_reg2  fd_smmu_master_reg2;
	union u_jpg_debug_0          jpg_debug_0;
	union u_jpg_debug_1          jpg_debug_1;
	union u_jpg_debug_2          jpg_debug_2;
	union u_jpg_debug_3          jpg_debug_3;
	union u_jpg_sec_ctrl_s       jpg_sec_ctrl_s;

};

/* Declare the struct pointor of the module SUB_CTRL */
extern struct s_sub_ctrl_regs_type *gopsub_ctrlallreg;


#endif /* __SUB_CTRL_DRV_PRIV_CS_H__ */
