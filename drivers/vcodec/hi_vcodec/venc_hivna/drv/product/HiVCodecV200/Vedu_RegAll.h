// ******************************************************************************
//  Copyright (C), 2007-2016, Hisilicon Technologies Co., Ltd.
//
// ******************************************************************************
// File name   : c_union_define.h
// Version     : 2.0
// Created     : 2016-10-31
// Last Modified :
// Description   : ±àÂëÆ÷¼Ä´æÆ÷½á¹¹Ìå
// Function List :
// History       :
// 1 Date        :
// Modification  : Create file
// ******************************************************************************
#ifndef __C_UNION_DEFINE_VEDU_REG_KIRIN970_H__
#define __C_UNION_DEFINE_VEDU_REG_KIRIN970_H__



/* Define the union U_VEDU_VCPI_INTMASK */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_enable_ve_eop    : 1   ; /* [0]  */
		unsigned int    vcpi_enable_vedu_slice_end  : 1   ; /* [1]  */
		unsigned int    vcpi_enable_ve_buffull  : 1   ; /* [2]  */
		unsigned int    vcpi_enable_ve_pbitsover  : 1   ; /* [3]  */
		unsigned int    vcpi_enable_vedu_brkpt  : 1   ; /* [4]  */
		unsigned int    vcpi_enable_vedu_step  : 1   ; /* [5]  */
		unsigned int    vcpi_enable_vedu_timeout  : 1   ; /* [6]  */
		unsigned int    vcpi_enable_cfg_err   : 1   ; /* [7]  */
		unsigned int    Reserved_0    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTMASK;

/* Define the union U_VEDU_VCPI_INTCLR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_clr_ve_eop    : 1   ; /* [0]  */
		unsigned int    vcpi_clr_vedu_slice_end  : 1   ; /* [1]  */
		unsigned int    vcpi_clr_ve_buffull   : 1   ; /* [2]  */
		unsigned int    vcpi_clr_ve_pbitsover  : 1   ; /* [3]  */
		unsigned int    vcpi_clr_vedu_brkpt   : 1   ; /* [4]  */
		unsigned int    vcpi_clr_vedu_step    : 1   ; /* [5]  */
		unsigned int    vcpi_clr_vedu_timeout  : 1   ; /* [6]  */
		unsigned int    vcpi_clr_cfg_err    : 1   ; /* [7]  */
		unsigned int    Reserved_1    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTCLR;

/* Define the union U_VEDU_VCPI_START */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_vstart    : 1   ; /* [0]  */
		unsigned int    Reserved_3    : 15  ; /* [15..1]  */
		unsigned int    vcpi_vstep    : 1   ; /* [16]  */
		unsigned int    Reserved_2    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_START;

/* Define the union U_VEDU_VCPI_CNTCLR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_cnt_clr    : 1   ; /* [0]  */
		unsigned int    vctrl_cnt_clr    : 1   ; /* [1]  */
		unsigned int    curld_cnt_clr    : 1   ; /* [2]  */
		unsigned int    pme_cnt_clr    : 1   ; /* [3]  */
		unsigned int    refld_cnt_clr    : 1   ; /* [4]  */
		unsigned int    ime_cnt_clr    : 1   ; /* [5]  */
		unsigned int    pintra_cnt_clr    : 1   ; /* [6]  */
		unsigned int    mrg_cnt_clr    : 1   ; /* [7]  */
		unsigned int    fme_cnt_clr    : 1   ; /* [8]  */
		unsigned int    intra_cnt_clr    : 1   ; /* [9]  */
		unsigned int    pmv_cnt_clr    : 1   ; /* [10]  */
		unsigned int    tqitq_cnt_clr    : 1   ; /* [11]  */
		unsigned int    sel_cnt_clr    : 1   ; /* [12]  */
		unsigned int    dblk_cnt_clr    : 1   ; /* [13]  */
		unsigned int    sao_cnt_clr    : 1   ; /* [14]  */
		unsigned int    pack_cnt_clr    : 1   ; /* [15]  */
		unsigned int    cabac_cnt_clr    : 1   ; /* [16]  */
		unsigned int    pmeld_cnt_clr    : 1   ; /* [17]  */
		unsigned int    pmest_cnt_clr    : 1   ; /* [18]  */
		unsigned int    nbi_cnt_clr    : 1   ; /* [19]  */
		unsigned int    lfldst_cnt_clr    : 1   ; /* [20]  */
		unsigned int    recst_cnt_clr    : 1   ; /* [21]  */
		unsigned int    csst_cnt_clr    : 1   ; /* [22]  */
		unsigned int    qpg_cnt_clr    : 1   ; /* [23]  */
		unsigned int    Reserved_4    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_CNTCLR;

/* Define the union U_VEDU_VCPI_BP_POS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bp_lcu_x    : 9   ; /* [8..0]  */
		unsigned int    Reserved_6    : 7   ; /* [15..9]  */
		unsigned int    vcpi_bp_lcu_y    : 9   ; /* [24..16]  */
		unsigned int    Reserved_5    : 5   ; /* [29..25]  */
		unsigned int    vcpi_bkp_en    : 1   ; /* [30]  */
		unsigned int    vcpi_dbgmod    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BP_POS;

/* Define the union U_VEDU_VCPI_MODE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_vedsel    : 1   ; /* [0]  */
		unsigned int    vcpi_lcu_time_sel    : 1   ; /* [1]  */
		unsigned int    vcpi_protocol    : 2   ; /* [3..2]  */
		unsigned int    vcpi_cfg_mode    : 1   ; /* [4]  */
		unsigned int    vcpi_slice_int_en    : 1   ; /* [5]  */
		unsigned int    vcpi_sao_luma    : 1   ; /* [6]  */
		unsigned int    vcpi_sao_chroma    : 1   ; /* [7]  */
		unsigned int    vcpi_rec_cmp_en    : 1   ; /* [8]  */
		unsigned int    vcpi_img_improve_en   : 1   ; /* [9]  */
		unsigned int    vcpi_frame_type    : 2   ; /* [11..10]  */
		unsigned int    vcpi_entropy_mode    : 1   ; /* [12]  */
		unsigned int    vcpi_long_term_refpic  : 1   ; /* [13]  */
		unsigned int    vcpi_ref_num    : 1   ; /* [14]  */
		unsigned int    vcpi_2line_paral_enc  : 1   ; /* [15]  */
		unsigned int    vcpi_idr_pic    : 1   ; /* [16]  */
		unsigned int    vcpi_pskip_en    : 1   ; /* [17]  */
		unsigned int    vcpi_trans_mode    : 2   ; /* [19..18]  */
		unsigned int    vcpi_blk8_inter    : 1   ; /* [20]  */
		unsigned int    vcpi_sobel_weight_en  : 1   ; /* [21]  */
		unsigned int    vcpi_high_speed_en    : 1   ; /* [22]  */
		unsigned int    vcpi_tiles_en    : 1   ; /* [23]  */
		unsigned int    vcpi_10bit_mode    : 2   ; /* [25..24]  */
		unsigned int    vcpi_lcu_size    : 2   ; /* [27..26]  */
		unsigned int    vcpi_time_en    : 2   ; /* [29..28]  */
		unsigned int    vcpi_ref_cmp_en    : 1   ; /* [30]  */
		unsigned int    vcpi_refc_nload    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_MODE;

/* Define the union U_VEDU_VCPI_TILE_SIZE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_tile_width    : 9   ; /* [8..0]  */
		unsigned int    Reserved_8    : 7   ; /* [15..9]  */
		unsigned int    vcpi_tile_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_7    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_TILE_SIZE;

/* Define the union U_VEDU_VCPI_PICSIZE_PIX */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_imgwidth_pix    : 13  ; /* [12..0]  */
		unsigned int    Reserved_10    : 3   ; /* [15..13]  */
		unsigned int    vcpi_imgheight_pix    : 13  ; /* [28..16]  */
		unsigned int    Reserved_9    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PICSIZE_PIX;

/* Define the union U_VEDU_VCPI_MULTISLC */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_slice_size    : 16  ; /* [15..0]  */
		unsigned int    vcpi_slcspilt_mod    : 1   ; /* [16]  */
		unsigned int    vcpi_multislc_en    : 1   ; /* [17]  */
		unsigned int    Reserved_11    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_MULTISLC;

/* Define the union U_VEDU_VCPI_QPCFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_cr_qp_offset    : 5   ; /* [4..0]  */
		unsigned int    vcpi_cb_qp_offset    : 5   ; /* [9..5]  */
		unsigned int    vcpi_frm_qp    : 6   ; /* [15..10]  */
		unsigned int    Reserved_14    : 1   ; /* [16]  */
		unsigned int    Reserved_13    : 1   ; /* [17]  */
		unsigned int    vcpi_intra_det_qp_en  : 1   ; /* [18]  */
		unsigned int    vcpi_rc_cu_madi_en    : 1   ; /* [19]  */
		unsigned int    Reserved_12    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_QPCFG;

/* Define the union U_VEDU_VCPI_DBLKCFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_dblk_beta    : 4   ; /* [3..0]  */
		unsigned int    vcpi_dblk_alpha    : 4   ; /* [7..4]  */
		unsigned int    vcpi_dblk_filter_flag  : 2   ; /* [9..8]  */
		unsigned int    Reserved_15    : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_DBLKCFG;

/* Define the union U_VEDU_VCPI_LOW_POWER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_intra_lowpow_en  : 1   ; /* [0]  */
		unsigned int    vcpi_fme_lowpow_en    : 1   ; /* [1]  */
		unsigned int    vcpi_ime_lowpow_en    : 1   ; /* [2]  */
		unsigned int    Reserved_18    : 1   ; /* [3]  */
		unsigned int    vcpi_ddr_cross_idx    : 11  ; /* [14..4]  */
		unsigned int    Reserved_17    : 1   ; /* [15]  */
		unsigned int    vcpi_tqitq_gtck_en    : 1   ; /* [16]  */
		unsigned int    vcpi_mrg_gtck_en    : 1   ; /* [17]  */
		unsigned int    vcpi_fme_gtck_en    : 1   ; /* [18]  */
		unsigned int    vcpi_clkgate_en    : 2   ; /* [20..19]  */
		unsigned int    vcpi_mem_clkgate_en   : 1   ; /* [21]  */
		unsigned int    vcpi_hfbc_clkgate_en  : 1   ; /* [22]  */
		unsigned int    vcpi_ddr_cross_en    : 1   ; /* [23]  */
		unsigned int    vcpi_10bit_addr_mode  : 2   ; /* [25..24]  */
		unsigned int    Reserved_16    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_LOW_POWER;

/* Define the union U_VEDU_VCPI_OUTSTD */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_r_outstanding    : 5   ; /* [4..0]  */
		unsigned int    Reserved_20    : 3   ; /* [7..5]  */
		unsigned int    vcpi_w_outstanding    : 5   ; /* [12..8]  */
		unsigned int    Reserved_19    : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OUTSTD;

/* Define the union U_VEDU_VCPI_TMV_LOAD */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_tmv_wr_rd_avail  : 2   ; /* [1..0]  */
		unsigned int    Reserved_21    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_TMV_LOAD;

/* Define the union U_VEDU_VCPI_CROSS_TILE_SLC */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_cross_slice    : 1   ; /* [0]  */
		unsigned int    vcpi_cross_tile    : 1   ; /* [1]  */
		unsigned int    Reserved_22    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_CROSS_TILE_SLC;

/* Define the union U_VEDU_VCPI_MEM_CTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ema    : 3   ; /* [2..0]  */
		unsigned int    vcpi_emaw    : 2   ; /* [4..3]  */
		unsigned int    vcpi_emaa    : 3   ; /* [7..5]  */
		unsigned int    vcpi_emab    : 3   ; /* [10..8]  */
		unsigned int    Reserved_23    : 21  ; /* [31..11]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_MEM_CTRL;

/* Define the union U_VEDU_VCPI_INTRA_INTER_CU_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_intra_cu_en    : 4   ; /* [3..0]  */
		unsigned int    vcpi_ipcm_en    : 1   ; /* [4]  */
		unsigned int    vcpi_intra_h264_cutdiag  : 1   ; /* [5]  */
		unsigned int    Reserved_25    : 2   ; /* [7..6]  */
		unsigned int    vcpi_fme_cu_en    : 4   ; /* [11..8]  */
		unsigned int    vcpi_mrg_cu_en    : 4   ; /* [15..12]  */
		unsigned int    Reserved_24    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTRA_INTER_CU_EN;

/* Define the union U_VEDU_VCPI_VLC_CONFIG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ref_idc    : 2   ; /* [1..0]  */
		unsigned int    Reserved_28    : 2   ; /* [3..2]  */
		unsigned int    vcpi_cabac_init_idc   : 2   ; /* [5..4]  */
		unsigned int    Reserved_27    : 6   ; /* [11..6]  */
		unsigned int    vcpi_byte_stuffing    : 1   ; /* [12]  */
		unsigned int    Reserved_26    : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_VLC_CONFIG;

/* Define the union U_VEDU_VCPI_PRE_JUDGE_EXT_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_iblk_pre_en    : 1   ; /* [0]  */
		unsigned int    vcpi_pblk_pre_en    : 1   ; /* [1]  */
		unsigned int    vcpi_force_inter    : 1   ; /* [2]  */
		unsigned int    vcpi_pintra_inter_flag_disable  : 1   ; /* [3]  */
		unsigned int    vcpi_ext_edge_en    : 1   ; /* [4]  */
		unsigned int    Reserved_29    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PRE_JUDGE_EXT_EN;

/* Define the union U_VEDU_VCPI_PRE_JUDGE_COST_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_iblk_pre_cost_thr  : 16  ; /* [15..0]  */
		unsigned int    vcpi_pblk_pre_cost_thr  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PRE_JUDGE_COST_THR;

/* Define the union U_VEDU_VCPI_IBLK_PRE_MV_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_iblk_pre_mv_dif_thr0  : 8   ; /* [7..0]  */
		unsigned int    vcpi_iblk_pre_mv_dif_thr1  : 8   ; /* [15..8]  */
		unsigned int    vcpi_iblk_pre_mvx_thr  : 8   ; /* [23..16]  */
		unsigned int    vcpi_iblk_pre_mvy_thr  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_IBLK_PRE_MV_THR;

/* Define the union U_VEDU_VCPI_PME_PARAM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_31    : 1   ; /* [0]  */
		unsigned int    vcpi_move_sad_en    : 1   ; /* [1]  */
		unsigned int    Reserved_30    : 14  ; /* [15..2]  */
		unsigned int    vcpi_pblk_pre_mvx_thr  : 8   ; /* [23..16]  */
		unsigned int    vcpi_pblk_pre_mvy_thr  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PME_PARAM;

/* Define the union U_VEDU_VCPI_PIC_STRONG_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_skin_en    : 1   ; /* [0]  */
		unsigned int    vcpi_strong_edge_en   : 1   ; /* [1]  */
		unsigned int    vcpi_still_en    : 1   ; /* [2]  */
		unsigned int    vcpi_skin_close_angle  : 1   ; /* [3]  */
		unsigned int    vcpi_rounding_sobel_en  : 1   ; /* [4]  */
		unsigned int    Reserved_32    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PIC_STRONG_EN;

/* Define the union U_VEDU_VCPI_REF_FLAG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_34    : 1   ; /* [0]  */
		unsigned int    vcpi_col_from_l0_flag  : 1   ; /* [1]  */
		unsigned int    vcpi_curr_ref_long_flag  : 2   ; /* [3..2]  */
		unsigned int    vcpi_col_long_flag    : 2   ; /* [5..4]  */
		unsigned int    vcpi_predflag_sel    : 2   ; /* [7..6]  */
		unsigned int    Reserved_33    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REF_FLAG;

/* Define the union U_VEDU_VCPI_RC_ENABLE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_rc_cu_qp_en    : 1   ; /* [0]  */
		unsigned int    vcpi_rc_row_qp_en    : 1   ; /* [1]  */
		unsigned int    vcpi_move_scene_en    : 1   ; /* [2]  */
		unsigned int    Reserved_38    : 1   ; /* [3]  */
		unsigned int    vcpi_strong_edge_move_en  : 1   ; /* [4]  */
		unsigned int    Reserved_37    : 1   ; /* [5]  */
		unsigned int    Reserved_36    : 2   ; /* [7..6]  */
		unsigned int    vcpi_rc_low_luma_en   : 1   ; /* [8]  */
		unsigned int    vcpi_rc_h264_smooth_mb_en  : 1   ; /* [9]  */
		unsigned int    vcpi_rd_smooth_mb_en  : 1   ; /* [10]  */
		unsigned int    vcpi_rd_min_sad_flag_en  : 1   ; /* [11]  */
		unsigned int    vcpi_wr_min_sad_flag_en  : 1   ; /* [12]  */
		unsigned int    vcpi_high_min_sad_en  : 1   ; /* [13]  */
		unsigned int    vcpi_low_min_sad_en   : 1   ; /* [14]  */
		unsigned int    vcpi_prev_min_sad_en  : 1   ; /* [15]  */
		unsigned int    vcpi_qpgld_en    : 1   ; /* [16]  */
		unsigned int    vcpi_map_roikeep_en   : 1   ; /* [17]  */
		unsigned int    vcpi_flat_region_en   : 1   ; /* [18]  */
		unsigned int    vcpi_qp_restrain_large_sad  : 1   ; /* [19]  */
		unsigned int    Reserved_35    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_RC_ENABLE;

/* Define the union U_VEDU_VCPI_PINTRA_THRESH0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_pintra_pu16_amp_th  : 8   ; /* [7..0]  */
		unsigned int    vcpi_pintra_pu32_amp_th  : 8   ; /* [15..8]  */
		unsigned int    vcpi_pintra_pu64_amp_th  : 8   ; /* [23..16]  */
		unsigned int    Reserved_39    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PINTRA_THRESH0;

/* Define the union U_VEDU_VCPI_PINTRA_THRESH1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_pintra_pu16_std_th  : 8   ; /* [7..0]  */
		unsigned int    vcpi_pintra_pu32_std_th  : 8   ; /* [15..8]  */
		unsigned int    Reserved_40    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PINTRA_THRESH1;

/* Define the union U_VEDU_VCPI_PINTRA_THRESH2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_pintra_pu16_angel_cost_th  : 10  ; /* [9..0]  */
		unsigned int    vcpi_pintra_pu32_angel_cost_th  : 10  ; /* [19..10]  */
		unsigned int    Reserved_41    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_PINTRA_THRESH2;

/* Define the union U_VEDU_VCPI_I_SLC_INSERT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_insert_i_slc_idx  : 8   ; /* [7..0]  */
		unsigned int    vcpi_insert_i_slc_en  : 1   ; /* [8]  */
		unsigned int    Reserved_42    : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_I_SLC_INSERT;

/* Define the union U_VEDU_VCPI_CLKDIV_ENABLE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_clkdiv_en    : 1   ; /* [0]  */
		unsigned int    Reserved_44    : 15  ; /* [15..1]  */
		unsigned int    vcpi_down_freq_en    : 1   ; /* [16]  */
		unsigned int    Reserved_43    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_CLKDIV_ENABLE;

/* Define the union U_VEDU_VCPI_OSD_ENABLE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd0_en    : 1   ; /* [0]  */
		unsigned int    vcpi_osd1_en    : 1   ; /* [1]  */
		unsigned int    vcpi_osd2_en    : 1   ; /* [2]  */
		unsigned int    vcpi_osd3_en    : 1   ; /* [3]  */
		unsigned int    vcpi_osd4_en    : 1   ; /* [4]  */
		unsigned int    vcpi_osd5_en    : 1   ; /* [5]  */
		unsigned int    vcpi_osd6_en    : 1   ; /* [6]  */
		unsigned int    vcpi_osd7_en    : 1   ; /* [7]  */
		unsigned int    vcpi_osd0_absqp    : 1   ; /* [8]  */
		unsigned int    vcpi_osd1_absqp    : 1   ; /* [9]  */
		unsigned int    vcpi_osd2_absqp    : 1   ; /* [10]  */
		unsigned int    vcpi_osd3_absqp    : 1   ; /* [11]  */
		unsigned int    vcpi_osd4_absqp    : 1   ; /* [12]  */
		unsigned int    vcpi_osd5_absqp    : 1   ; /* [13]  */
		unsigned int    vcpi_osd6_absqp    : 1   ; /* [14]  */
		unsigned int    vcpi_osd7_absqp    : 1   ; /* [15]  */
		unsigned int    vcpi_roi_osd_sel_0    : 1   ; /* [16]  */
		unsigned int    vcpi_roi_osd_sel_1    : 1   ; /* [17]  */
		unsigned int    vcpi_roi_osd_sel_2    : 1   ; /* [18]  */
		unsigned int    vcpi_roi_osd_sel_3    : 1   ; /* [19]  */
		unsigned int    vcpi_roi_osd_sel_4    : 1   ; /* [20]  */
		unsigned int    vcpi_roi_osd_sel_5    : 1   ; /* [21]  */
		unsigned int    vcpi_roi_osd_sel_6    : 1   ; /* [22]  */
		unsigned int    vcpi_roi_osd_sel_7    : 1   ; /* [23]  */
		unsigned int    vcpi_osd_en    : 1   ; /* [24]  */
		unsigned int    Reserved_45    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_ENABLE;

/* Define the union U_VEDU_VCPI_OSD_POS_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd0_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_47    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd0_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_46    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_0;

/* Define the union U_VEDU_VCPI_OSD_POS_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd1_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_49    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd1_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_48    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_1;

/* Define the union U_VEDU_VCPI_OSD_POS_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd2_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_51    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd2_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_50    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_2;

/* Define the union U_VEDU_VCPI_OSD_POS_3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd3_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_53    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd3_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_52    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_3;

/* Define the union U_VEDU_VCPI_OSD_POS_4 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd4_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_55    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd4_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_54    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_4;

/* Define the union U_VEDU_VCPI_OSD_POS_5 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd5_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_57    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd5_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_56    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_5;

/* Define the union U_VEDU_VCPI_OSD_POS_6 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd6_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_59    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd6_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_58    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_6;

/* Define the union U_VEDU_VCPI_OSD_POS_7 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd7_x    : 13  ; /* [12..0]  */
		unsigned int    Reserved_61    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd7_y    : 13  ; /* [28..16]  */
		unsigned int    Reserved_60    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_POS_7;

/* Define the union U_VEDU_VCPI_OSD_SIZE_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd0_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_63    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd0_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_62    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_0;

/* Define the union U_VEDU_VCPI_OSD_SIZE_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd1_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_65    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd1_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_64    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_1;

/* Define the union U_VEDU_VCPI_OSD_SIZE_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd2_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_67    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd2_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_66    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_2;

/* Define the union U_VEDU_VCPI_OSD_SIZE_3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd3_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_69    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd3_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_68    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_3;

/* Define the union U_VEDU_VCPI_OSD_SIZE_4 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd4_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_71    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd4_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_70    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_4;

/* Define the union U_VEDU_VCPI_OSD_SIZE_5 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd5_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_73    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd5_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_72    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_5;

/* Define the union U_VEDU_VCPI_OSD_SIZE_6 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd6_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_75    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd6_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_74    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_6;

/* Define the union U_VEDU_VCPI_OSD_SIZE_7 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd7_w    : 13  ; /* [12..0]  */
		unsigned int    Reserved_77    : 3   ; /* [15..13]  */
		unsigned int    vcpi_osd7_h    : 13  ; /* [28..16]  */
		unsigned int    Reserved_76    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_SIZE_7;

/* Define the union U_VEDU_VCPI_OSD_LAYERID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd0_layer_id    : 3   ; /* [2..0]  */
		unsigned int    Reserved_85    : 1   ; /* [3]  */
		unsigned int    vcpi_osd1_layer_id    : 3   ; /* [6..4]  */
		unsigned int    Reserved_84    : 1   ; /* [7]  */
		unsigned int    vcpi_osd2_layer_id    : 3   ; /* [10..8]  */
		unsigned int    Reserved_83    : 1   ; /* [11]  */
		unsigned int    vcpi_osd3_layer_id    : 3   ; /* [14..12]  */
		unsigned int    Reserved_82    : 1   ; /* [15]  */
		unsigned int    vcpi_osd4_layer_id    : 3   ; /* [18..16]  */
		unsigned int    Reserved_81    : 1   ; /* [19]  */
		unsigned int    vcpi_osd5_layer_id    : 3   ; /* [22..20]  */
		unsigned int    Reserved_80    : 1   ; /* [23]  */
		unsigned int    vcpi_osd6_layer_id    : 3   ; /* [26..24]  */
		unsigned int    Reserved_79    : 1   ; /* [27]  */
		unsigned int    vcpi_osd7_layer_id    : 3   ; /* [30..28]  */
		unsigned int    Reserved_78    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_LAYERID;

/* Define the union U_VEDU_VCPI_OSD_QP0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd0_qp    : 7   ; /* [6..0]  */
		unsigned int    Reserved_89    : 1   ; /* [7]  */
		unsigned int    vcpi_osd1_qp    : 7   ; /* [14..8]  */
		unsigned int    Reserved_88    : 1   ; /* [15]  */
		unsigned int    vcpi_osd2_qp    : 7   ; /* [22..16]  */
		unsigned int    Reserved_87    : 1   ; /* [23]  */
		unsigned int    vcpi_osd3_qp    : 7   ; /* [30..24]  */
		unsigned int    Reserved_86    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_QP0;

/* Define the union U_VEDU_VCPI_OSD_QP1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_osd4_qp    : 7   ; /* [6..0]  */
		unsigned int    Reserved_93    : 1   ; /* [7]  */
		unsigned int    vcpi_osd5_qp    : 7   ; /* [14..8]  */
		unsigned int    Reserved_92    : 1   ; /* [15]  */
		unsigned int    vcpi_osd6_qp    : 7   ; /* [22..16]  */
		unsigned int    Reserved_91    : 1   ; /* [23]  */
		unsigned int    vcpi_osd7_qp    : 7   ; /* [30..24]  */
		unsigned int    Reserved_90    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_OSD_QP1;

/* Define the union U_VEDU_VCPI_SW_L0_SIZE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_sw_l0_width    : 7   ; /* [6..0]  */
		unsigned int    Reserved_95    : 9   ; /* [15..7]  */
		unsigned int    vcpi_sw_l0_height    : 6   ; /* [21..16]  */
		unsigned int    Reserved_94    : 10  ; /* [31..22]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_SW_L0_SIZE;

/* Define the union U_VEDU_VCPI_SW_L1_SIZE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_sw_l1_width    : 7   ; /* [6..0]  */
		unsigned int    Reserved_97    : 9   ; /* [15..7]  */
		unsigned int    vcpi_sw_l1_height    : 6   ; /* [21..16]  */
		unsigned int    Reserved_96    : 10  ; /* [31..22]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_SW_L1_SIZE;

/* Define the union U_VEDU_VCPI_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_curld_c_stride   : 16  ; /* [15..0]  */
		unsigned int    vcpi_curld_y_stride   : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_STRIDE;

/* Define the union U_VEDU_VCPI_REC_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_recst_ystride    : 16  ; /* [15..0]  */
		unsigned int    vcpi_recst_cstride    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REC_STRIDE;

/* Define the union U_VEDU_VCPI_REC_HEAD_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_recst_ch_stride  : 16  ; /* [15..0]  */
		unsigned int    vcpi_recst_yh_stride  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REC_HEAD_STRIDE;

/* Define the union U_VEDU_VCPI_REF_L0_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_refc_l0_stride   : 16  ; /* [15..0]  */
		unsigned int    vcpi_refy_l0_stride   : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REF_L0_STRIDE;

/* Define the union U_VEDU_VCPI_REFH_L0_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_refch_l0_stride  : 16  ; /* [15..0]  */
		unsigned int    vcpi_refyh_l0_stride  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REFH_L0_STRIDE;

/* Define the union U_VEDU_VCPI_REF_L1_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_refc_l1_stride   : 16  ; /* [15..0]  */
		unsigned int    vcpi_refy_l1_stride   : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REF_L1_STRIDE;

/* Define the union U_VEDU_VCPI_REFH_L1_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_refch_l1_stride  : 16  ; /* [15..0]  */
		unsigned int    vcpi_refyh_l1_stride  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_REFH_L1_STRIDE;

/* Define the union U_VEDU_VCPI_STRFMT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_crop_en    : 1   ; /* [0]  */
		unsigned int    vcpi_scale_en    : 1   ; /* [1]  */
		unsigned int    vcpi_store_mode    : 1   ; /* [2]  */
		unsigned int    vcpi_blk_type    : 3   ; /* [5..3]  */
		unsigned int    vcpi_str_fmt    : 4   ; /* [9..6]  */
		unsigned int    vcpi_package_sel    : 4   ; /* [13..10]  */
		unsigned int    vcpi_recst_disable    : 1   ; /* [14]  */
		unsigned int    Reserved_98    : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_STRFMT;

/* Define the union U_VEDU_VCPI_CROP_START */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_crop_xstart    : 13  ; /* [12..0]  */
		unsigned int    Reserved_100    : 3   ; /* [15..13]  */
		unsigned int    vcpi_crop_ystart    : 13  ; /* [28..16]  */
		unsigned int    Reserved_99    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_CROP_START;

/* Define the union U_VEDU_VCPI_CROP_END */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_crop_xend    : 13  ; /* [12..0]  */
		unsigned int    Reserved_102    : 3   ; /* [15..13]  */
		unsigned int    vcpi_crop_yend    : 13  ; /* [28..16]  */
		unsigned int    Reserved_101    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_CROP_END;

/* Define the union U_VEDU_VCPI_SCALE_PARA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_xscale    : 16  ; /* [15..0]  */
		unsigned int    vcpi_yscale    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_SCALE_PARA;

/* Define the union U_VEDU_VCPI_ORI_PICSIZE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_oriwidth_pix    : 13  ; /* [12..0]  */
		unsigned int    Reserved_104    : 3   ; /* [15..13]  */
		unsigned int    vcpi_oriheight_pix    : 13  ; /* [28..16]  */
		unsigned int    Reserved_103    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_ORI_PICSIZE;

/* Define the union U_VEDU_VCPI_BG_ENABLE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bg_info_st_en    : 1   ; /* [0]  */
		unsigned int    vcpi_bg_refresh_st_en  : 1   ; /* [1]  */
		unsigned int    vcpi_bg_ld_en    : 1   ; /* [2]  */
		unsigned int    vcpi_bg_en    : 1   ; /* [3]  */
		unsigned int    vcpi_bg_stat_frame    : 1   ; /* [4]  */
		unsigned int    vcpi_bg_th_frame    : 1   ; /* [5]  */
		unsigned int    vcpi_bg_percentage_en  : 1   ; /* [6]  */
		unsigned int    vcpi_bg_start_idc    : 1   ; /* [7]  */
		unsigned int    vcpi_bg_start_frame   : 1   ; /* [8]  */
		unsigned int    vcpi_bg_reset_diff_en  : 1   ; /* [9]  */
		unsigned int    Reserved_105    : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_ENABLE;

/* Define the union U_VEDU_VCPI_BG_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bgl_stride    : 16  ; /* [15..0]  */
		unsigned int    vcpi_bgc_stride    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_STRIDE;

/* Define the union U_VEDU_VCPI_BG_FLT_PARA0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bg_alpha_fix_0   : 11  ; /* [10..0]  */
		unsigned int    Reserved_107    : 5   ; /* [15..11]  */
		unsigned int    vcpi_bg_alpha_fix_1   : 11  ; /* [26..16]  */
		unsigned int    Reserved_106    : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_FLT_PARA0;

/* Define the union U_VEDU_VCPI_BG_FLT_PARA1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bg_belta_fix_0   : 11  ; /* [10..0]  */
		unsigned int    Reserved_109    : 5   ; /* [15..11]  */
		unsigned int    vcpi_bg_belta_fix_1   : 11  ; /* [26..16]  */
		unsigned int    Reserved_108    : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_FLT_PARA1;

/* Define the union U_VEDU_VCPI_BG_FLT_PARA2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bg_delta_fix_0   : 11  ; /* [10..0]  */
		unsigned int    Reserved_111    : 5   ; /* [15..11]  */
		unsigned int    vcpi_bg_delta_fix_1   : 11  ; /* [26..16]  */
		unsigned int    Reserved_110    : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_FLT_PARA2;

/* Define the union U_VEDU_VCPI_BG_THR0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bg_th_ave    : 8   ; /* [7..0]  */
		unsigned int    vcpi_bg_stat_th    : 13  ; /* [20..8]  */
		unsigned int    vcpi_bg_ave_update_th  : 8   ; /* [28..21]  */
		unsigned int    Reserved_112    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_THR0;

/* Define the union U_VEDU_VCPI_BG_THR1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bg_dist_th    : 8   ; /* [7..0]  */
		unsigned int    vcpi_bg_frame_num    : 8   ; /* [15..8]  */
		unsigned int    vcpi_bg_min_diff    : 8   ; /* [23..16]  */
		unsigned int    Reserved_113    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_BG_THR1;

/* Define the union U_VEDU_VCPI_MEM_CTRL_T16 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mem_ctrl_s    : 16  ; /* [15..0]  */
		unsigned int    mem_ctrl_d1w2r    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_MEM_CTRL_T16;

/* Define the union U_VEDU_VCPI_INTRA32_LOW_POWER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_intra32_low_power_thr  : 16  ; /* [15..0]  */
		unsigned int    vcpi_intra32_low_power_en  : 1   ; /* [16]  */
		unsigned int    Reserved_114    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTRA32_LOW_POWER;

/* Define the union U_VEDU_VCPI_INTRA16_LOW_POWER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_intra16_low_power_thr  : 16  ; /* [15..0]  */
		unsigned int    vcpi_intra16_low_power_en  : 1   ; /* [16]  */
		unsigned int    Reserved_115    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTRA16_LOW_POWER;

/* Define the union U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_intra_reduce_rdo_num_thr  : 12  ; /* [11..0]  */
		unsigned int    Reserved_117    : 4   ; /* [15..12]  */
		unsigned int    vcpi_intra_reduce_rdo_num_en  : 1   ; /* [16]  */
		unsigned int    Reserved_116    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM;

/* Define the union U_VEDU_VCPI_NOFORCEZERO */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bislayer0flag    : 1   ; /* [0]  */
		unsigned int    vcpi_bnoforcezero_flag  : 1   ; /* [1]  */
		unsigned int    vcpi_bnoforcezero_posx  : 4   ; /* [5..2]  */
		unsigned int    vcpi_bnoforcezero_posy  : 4   ; /* [9..6]  */
		unsigned int    Reserved_118    : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_NOFORCEZERO;

/* Define the union U_VEDU_VCPI_INTMASK_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_enable_ve_eop_s  : 1   ; /* [0]  */
		unsigned int    vcpi_enable_vedu_slice_end_s  : 1   ; /* [1]  */
		unsigned int    vcpi_enable_ve_buffull_s  : 1   ; /* [2]  */
		unsigned int    vcpi_enable_ve_pbitsover_s  : 1   ; /* [3]  */
		unsigned int    vcpi_enable_vedu_brkpt_s  : 1   ; /* [4]  */
		unsigned int    vcpi_enable_vedu_step_s  : 1   ; /* [5]  */
		unsigned int    vcpi_enable_vedu_timeout_s  : 1   ; /* [6]  */
		unsigned int    vcpi_enable_cfg_err_s  : 1   ; /* [7]  */
		unsigned int    Reserved_119    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTMASK_S;

/* Define the union U_VEDU_VCPI_INTCLR_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_clr_ve_eop_s    : 1   ; /* [0]  */
		unsigned int    vcpi_clr_vedu_slice_end_s  : 1   ; /* [1]  */
		unsigned int    vcpi_clr_ve_buffull_s  : 1   ; /* [2]  */
		unsigned int    vcpi_clr_ve_pbitsover_s  : 1   ; /* [3]  */
		unsigned int    vcpi_clr_vedu_brkpt_s  : 1   ; /* [4]  */
		unsigned int    vcpi_clr_vedu_step_s  : 1   ; /* [5]  */
		unsigned int    vcpi_clr_vedu_timeout_s  : 1   ; /* [6]  */
		unsigned int    vcpi_clr_cfg_err_s    : 1   ; /* [7]  */
		unsigned int    Reserved_120    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCPI_INTCLR_S;

/* Define the union U_VEDU_VCTRL_ROI_CFG0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_region0en    : 1   ; /* [0]  */
		unsigned int    vctrl_region1en    : 1   ; /* [1]  */
		unsigned int    vctrl_region2en    : 1   ; /* [2]  */
		unsigned int    vctrl_region3en    : 1   ; /* [3]  */
		unsigned int    vctrl_region4en    : 1   ; /* [4]  */
		unsigned int    vctrl_region5en    : 1   ; /* [5]  */
		unsigned int    vctrl_region6en    : 1   ; /* [6]  */
		unsigned int    vctrl_region7en    : 1   ; /* [7]  */
		unsigned int    vctrl_absqp0    : 1   ; /* [8]  */
		unsigned int    vctrl_absqp1    : 1   ; /* [9]  */
		unsigned int    vctrl_absqp2    : 1   ; /* [10]  */
		unsigned int    vctrl_absqp3    : 1   ; /* [11]  */
		unsigned int    vctrl_absqp4    : 1   ; /* [12]  */
		unsigned int    vctrl_absqp5    : 1   ; /* [13]  */
		unsigned int    vctrl_absqp6    : 1   ; /* [14]  */
		unsigned int    vctrl_absqp7    : 1   ; /* [15]  */
		unsigned int    vctrl_region0keep    : 1   ; /* [16]  */
		unsigned int    vctrl_region1keep    : 1   ; /* [17]  */
		unsigned int    vctrl_region2keep    : 1   ; /* [18]  */
		unsigned int    vctrl_region3keep    : 1   ; /* [19]  */
		unsigned int    vctrl_region4keep    : 1   ; /* [20]  */
		unsigned int    vctrl_region5keep    : 1   ; /* [21]  */
		unsigned int    vctrl_region6keep    : 1   ; /* [22]  */
		unsigned int    vctrl_region7keep    : 1   ; /* [23]  */
		unsigned int    vctrl_roi_en    : 1   ; /* [24]  */
		unsigned int    Reserved_122    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_CFG0;

/* Define the union U_VEDU_VCTRL_ROI_CFG1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roiqp0    : 7   ; /* [6..0]  */
		unsigned int    Reserved_126    : 1   ; /* [7]  */
		unsigned int    vctrl_roiqp1    : 7   ; /* [14..8]  */
		unsigned int    Reserved_125    : 1   ; /* [15]  */
		unsigned int    vctrl_roiqp2    : 7   ; /* [22..16]  */
		unsigned int    Reserved_124    : 1   ; /* [23]  */
		unsigned int    vctrl_roiqp3    : 7   ; /* [30..24]  */
		unsigned int    Reserved_123    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_CFG1;

/* Define the union U_VEDU_VCTRL_ROI_CFG2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roiqp4    : 7   ; /* [6..0]  */
		unsigned int    Reserved_130    : 1   ; /* [7]  */
		unsigned int    vctrl_roiqp5    : 7   ; /* [14..8]  */
		unsigned int    Reserved_129    : 1   ; /* [15]  */
		unsigned int    vctrl_roiqp6    : 7   ; /* [22..16]  */
		unsigned int    Reserved_128    : 1   ; /* [23]  */
		unsigned int    vctrl_roiqp7    : 7   ; /* [30..24]  */
		unsigned int    Reserved_127    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_CFG2;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size0_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_132    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size0_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_131    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_0;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size1_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_134    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size1_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_133    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_1;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size2_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_136    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size2_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_135    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_2;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size3_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_138    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size3_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_137    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_3;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_4 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size4_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_140    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size4_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_139    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_4;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_5 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size5_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_142    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size5_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_141    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_5;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_6 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size6_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_144    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size6_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_143    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_6;

/* Define the union U_VEDU_VCTRL_ROI_SIZE_7 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_size7_roiwidth  : 10  ; /* [9..0]  */
		unsigned int    Reserved_146    : 6   ; /* [15..10]  */
		unsigned int    vctrl_size7_roiheight  : 10  ; /* [25..16]  */
		unsigned int    Reserved_145    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_SIZE_7;

/* Define the union U_VEDU_VCTRL_ROI_START_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start0_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_148    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start0_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_147    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_0;

/* Define the union U_VEDU_VCTRL_ROI_START_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start1_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_150    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start1_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_149    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_1;

/* Define the union U_VEDU_VCTRL_ROI_START_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start2_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_152    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start2_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_151    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_2;

/* Define the union U_VEDU_VCTRL_ROI_START_3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start3_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_154    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start3_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_153    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_3;

/* Define the union U_VEDU_VCTRL_ROI_START_4 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start4_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_156    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start4_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_155    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_4;

/* Define the union U_VEDU_VCTRL_ROI_START_5 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start5_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_158    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start5_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_157    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_5;

/* Define the union U_VEDU_VCTRL_ROI_START_6 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start6_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_160    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start6_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_159    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_6;

/* Define the union U_VEDU_VCTRL_ROI_START_7 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_start7_roistartx  : 9   ; /* [8..0]  */
		unsigned int    Reserved_162    : 7   ; /* [15..9]  */
		unsigned int    vctrl_start7_roistarty  : 9   ; /* [24..16]  */
		unsigned int    Reserved_161    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_ROI_START_7;

/* Define the union U_VEDU_VCTRL_LCU_TARGET_BIT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_lcu_target_bit  : 20  ; /* [19..0]  */
		unsigned int    Reserved_163    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_LCU_TARGET_BIT;

/* Define the union U_VEDU_VCTRL_NARROW_THRESHOLD */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_narrow_tile_width  : 4   ; /* [3..0]  */
		unsigned int    Reserved_164    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_NARROW_THRESHOLD;

/* Define the union U_VEDU_VCTRL_LCU_BASELINE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_lcu_performance_baseline  : 16  ; /* [15..0]  */
		unsigned int    Reserved_165    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_LCU_BASELINE;

/* Define the union U_VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_norm32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_168    : 6   ; /* [15..10]  */
		unsigned int    vctrl_norm32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_167    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingoffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_166    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_norm16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_171    : 8   ; /* [15..8]  */
		unsigned int    vctrl_norm16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_170    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingoffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_169    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_NORM_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingmechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingdegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundinglowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundinglowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_172    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_NORM_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_NORM_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_norm_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_norm_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_norm_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_norm_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_norm_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_norm_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_173    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_NORM_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_skin32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_176    : 6   ; /* [15..10]  */
		unsigned int    vctrl_skin32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_175    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingskinoffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_174    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_skin16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_179    : 8   ; /* [15..8]  */
		unsigned int    vctrl_skin16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_178    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingskinoffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_177    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SKIN_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingskinmechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingskindegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingskinforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingskinac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingskinac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundingskinlowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundingskinlowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_180    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SKIN_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SKIN_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_skin_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_skin_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_skin_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_skin_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_skin_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_skin_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_181    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SKIN_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_hedge32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_184    : 6   ; /* [15..10]  */
		unsigned int    vctrl_hedge32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_183    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingedgeoffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_182    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_hedge16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_187    : 8   ; /* [15..8]  */
		unsigned int    vctrl_hedge16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_186    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingedgeoffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_185    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGE_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingedgemechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingedgedegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingedgeforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingedgeac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingedgeac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundingedgelowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundingedgelowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_188    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGE_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGE_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_stredge_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_stredge_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_stredge_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_stredge_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_stredge_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_stredge_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_189    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGE_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_edgemov32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_192    : 6   ; /* [15..10]  */
		unsigned int    vctrl_edgemov32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_191    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingedgemovoffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_190    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_edgemov16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_195    : 8   ; /* [15..8]  */
		unsigned int    vctrl_edgemov16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_194    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingedgemovoffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_193    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingedgemovmechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingedgemovdegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingedgemovforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingedgemovac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingedgemovac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundingedgemovlowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundingedgemovlowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_196    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_HEDGEMOV_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_edgemov_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_edgemov_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_edgemov_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_edgemov_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_edgemov_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_edgemov_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_197    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_HEDGEMOV_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_static32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_200    : 6   ; /* [15..10]  */
		unsigned int    vctrl_static32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_199    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingstilloffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_198    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_static16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_203    : 8   ; /* [15..8]  */
		unsigned int    vctrl_static16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_202    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingstilloffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_201    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_STATIC_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingstillmechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingstilldegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingstillforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingstillac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingstillac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundingstilllowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundingstilllowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_204    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_STATIC_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_STATIC_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_still_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_still_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_still_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_still_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_still_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_still_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_205    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_STATIC_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_sobelstr32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_208    : 6   ; /* [15..10]  */
		unsigned int    vctrl_sobelstr32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_207    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingsobelstroffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_206    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_sobelstr16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_211    : 8   ; /* [15..8]  */
		unsigned int    vctrl_sobelstr16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_210    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingsobelstroffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_209    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELSTR_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingsobelstrmechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingsobelstrdegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingsobelstrforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingsobelstrac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingsobelstrac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundingsobelstrlowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundingsobelstrlowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_212    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELSTR_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELSTR_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_sobelstr_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_sobelstr_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_sobelstr_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_sobelstr_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_sobelstr_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_sobelstr_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_213    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELSTR_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_sobelweak32_coeff_protect_num  : 10  ; /* [9..0]  */
		unsigned int    Reserved_216    : 6   ; /* [15..10]  */
		unsigned int    vctrl_sobelweak32_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_215    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingsobelweakoffset32x32  : 4   ; /* [27..24]  */
		unsigned int    Reserved_214    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_sobelweak16_coeff_protect_num  : 8   ; /* [7..0]  */
		unsigned int    Reserved_219    : 8   ; /* [15..8]  */
		unsigned int    vctrl_sobelweak16_tr1_denois_max_num  : 5   ; /* [20..16]  */
		unsigned int    Reserved_218    : 3   ; /* [23..21]  */
		unsigned int    vctrl_roundingsobelweakoffset16x16  : 4   ; /* [27..24]  */
		unsigned int    Reserved_217    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_roundingsobelweakmechanism  : 3   ; /* [2..0]  */
		unsigned int    vctrl_roundingsobelweakdegreethresh  : 7   ; /* [9..3]  */
		unsigned int    vctrl_roundingsobelweakforcezeroresidthresh  : 3   ; /* [12..10]  */
		unsigned int    vctrl_roundingsobelweakac32sum  : 5   ; /* [17..13]  */
		unsigned int    vctrl_roundingsobelweakac16sum  : 4   ; /* [21..18]  */
		unsigned int    vctrl_roundingsobelweaklowfreqacblk32  : 4   ; /* [25..22]  */
		unsigned int    vctrl_roundingsobelweaklowfreqacblk16  : 4   ; /* [29..26]  */
		unsigned int    Reserved_220    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE;

/* Define the union U_VEDU_VCTRL_SOBELWEAK_ENG_DENOISE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_sobelwk_isolate_ac_enable  : 1   ; /* [0]  */
		unsigned int    vctrl_sobelwk_force_zero_cnt  : 3   ; /* [3..1]  */
		unsigned int    vctrl_sobelwk_engsum_32  : 6   ; /* [9..4]  */
		unsigned int    vctrl_sobelwk_engcnt_32  : 5   ; /* [14..10]  */
		unsigned int    vctrl_sobelwk_engsum_16  : 6   ; /* [20..15]  */
		unsigned int    vctrl_sobelwk_engcnt_16  : 5   ; /* [25..21]  */
		unsigned int    Reserved_221    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_SOBELWEAK_ENG_DENOISE;

/* Define the union U_VEDU_VCTRL_INTRA_RDO_FACTOR_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_norm_intra_cu4_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_norm_intra_cu8_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_norm_intra_cu16_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_norm_intra_cu32_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_strmov_intra_cu4_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_strmov_intra_cu8_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_strmov_intra_cu16_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_strmov_intra_cu32_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_INTRA_RDO_FACTOR_0;

/* Define the union U_VEDU_VCTRL_INTRA_RDO_FACTOR_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_skin_intra_cu4_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_skin_intra_cu8_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_skin_intra_cu16_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_skin_intra_cu32_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_sobel_str_intra_cu4_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_sobel_str_intra_cu8_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_sobel_str_intra_cu16_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_sobel_str_intra_cu32_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_INTRA_RDO_FACTOR_1;

/* Define the union U_VEDU_VCTRL_INTRA_RDO_FACTOR_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_hedge_intra_cu4_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_hedge_intra_cu8_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_hedge_intra_cu16_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_hedge_intra_cu32_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_sobel_tex_intra_cu4_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_sobel_tex_intra_cu8_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_sobel_tex_intra_cu16_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_sobel_tex_intra_cu32_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_INTRA_RDO_FACTOR_2;

/* Define the union U_VEDU_VCTRL_MRG_RDO_FACTOR_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_norm_mrg_cu8_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_norm_mrg_cu16_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_norm_mrg_cu32_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_norm_mrg_cu64_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_strmov_mrg_cu8_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_strmov_mrg_cu16_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_strmov_mrg_cu32_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_strmov_mrg_cu64_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_MRG_RDO_FACTOR_0;

/* Define the union U_VEDU_VCTRL_MRG_RDO_FACTOR_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_skin_mrg_cu8_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_skin_mrg_cu16_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_skin_mrg_cu32_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_skin_mrg_cu64_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_sobel_str_mrg_cu8_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_sobel_str_mrg_cu16_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_sobel_str_mrg_cu32_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_sobel_str_mrg_cu64_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_MRG_RDO_FACTOR_1;

/* Define the union U_VEDU_VCTRL_MRG_RDO_FACTOR_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_hedge_mrg_cu8_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_hedge_mrg_cu16_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_hedge_mrg_cu32_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_hedge_mrg_cu64_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_sobel_tex_mrg_cu8_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_sobel_tex_mrg_cu16_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_sobel_tex_mrg_cu32_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_sobel_tex_mrg_cu64_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_MRG_RDO_FACTOR_2;

/* Define the union U_VEDU_VCTRL_FME_RDO_FACTOR_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_norm_fme_cu8_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_norm_fme_cu16_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_norm_fme_cu32_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_norm_fme_cu64_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_strmov_fme_cu8_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_strmov_fme_cu16_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_strmov_fme_cu32_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_strmov_fme_cu64_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_FME_RDO_FACTOR_0;

/* Define the union U_VEDU_VCTRL_FME_RDO_FACTOR_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_skin_fme_cu8_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_skin_fme_cu16_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_skin_fme_cu32_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_skin_fme_cu64_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_sobel_str_fme_cu8_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_sobel_str_fme_cu16_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_sobel_str_fme_cu32_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_sobel_str_fme_cu64_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_FME_RDO_FACTOR_1;

/* Define the union U_VEDU_VCTRL_FME_RDO_FACTOR_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vctrl_hedge_fme_cu8_rdcost_offset  : 4   ; /* [3..0]  */
		unsigned int    vctrl_hedge_fme_cu16_rdcost_offset  : 4   ; /* [7..4]  */
		unsigned int    vctrl_hedge_fme_cu32_rdcost_offset  : 4   ; /* [11..8]  */
		unsigned int    vctrl_hedge_fme_cu64_rdcost_offset  : 4   ; /* [15..12]  */
		unsigned int    vctrl_sobel_tex_fme_cu8_rdcost_offset  : 4   ; /* [19..16]  */
		unsigned int    vctrl_sobel_tex_fme_cu16_rdcost_offset  : 4   ; /* [23..20]  */
		unsigned int    vctrl_sobel_tex_fme_cu32_rdcost_offset  : 4   ; /* [27..24]  */
		unsigned int    vctrl_sobel_tex_fme_cu64_rdcost_offset  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VCTRL_FME_RDO_FACTOR_2;

/* Define the union U_VEDU_CURLD_GCFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd0_global_en  : 1   ; /* [0]  */
		unsigned int    curld_osd1_global_en  : 1   ; /* [1]  */
		unsigned int    curld_osd2_global_en  : 1   ; /* [2]  */
		unsigned int    curld_osd3_global_en  : 1   ; /* [3]  */
		unsigned int    curld_osd4_global_en  : 1   ; /* [4]  */
		unsigned int    curld_osd5_global_en  : 1   ; /* [5]  */
		unsigned int    curld_osd6_global_en  : 1   ; /* [6]  */
		unsigned int    curld_osd7_global_en  : 1   ; /* [7]  */
		unsigned int    Reserved_226    : 6   ; /* [13..8]  */
		unsigned int    curld_col2gray_en    : 1   ; /* [14]  */
		unsigned int    curld_clip_en    : 1   ; /* [15]  */
		unsigned int    Reserved_225    : 4   ; /* [19..16]  */
		unsigned int    curld_read_interval   : 8   ; /* [27..20]  */
		unsigned int    curld_lowdly_en    : 1   ; /* [28]  */
		unsigned int    Reserved_224    : 1   ; /* [29]  */
		unsigned int    curld_osd_rgbfmt    : 1   ; /* [30]  */
		unsigned int    Reserved_223    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_GCFG;

/* Define the union U_VEDU_CURLD_OSD01_ALPHA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd0_alpha0    : 8   ; /* [7..0]  */
		unsigned int    curld_osd0_alpha1    : 8   ; /* [15..8]  */
		unsigned int    curld_osd1_alpha0    : 8   ; /* [23..16]  */
		unsigned int    curld_osd1_alpha1    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD01_ALPHA;

/* Define the union U_VEDU_CURLD_OSD23_ALPHA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd2_alpha0    : 8   ; /* [7..0]  */
		unsigned int    curld_osd2_alpha1    : 8   ; /* [15..8]  */
		unsigned int    curld_osd3_alpha0    : 8   ; /* [23..16]  */
		unsigned int    curld_osd3_alpha1    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD23_ALPHA;

/* Define the union U_VEDU_CURLD_OSD45_ALPHA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd4_alpha0    : 8   ; /* [7..0]  */
		unsigned int    curld_osd4_alpha1    : 8   ; /* [15..8]  */
		unsigned int    curld_osd5_alpha0    : 8   ; /* [23..16]  */
		unsigned int    curld_osd5_alpha1    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD45_ALPHA;

/* Define the union U_VEDU_CURLD_OSD67_ALPHA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd6_alpha0    : 8   ; /* [7..0]  */
		unsigned int    curld_osd6_alpha1    : 8   ; /* [15..8]  */
		unsigned int    curld_osd7_alpha0    : 8   ; /* [23..16]  */
		unsigned int    curld_osd7_alpha1    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD67_ALPHA;

/* Define the union U_VEDU_CURLD_OSD_GALPHA0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd0_global_alpha  : 8   ; /* [7..0]  */
		unsigned int    curld_osd1_global_alpha  : 8   ; /* [15..8]  */
		unsigned int    curld_osd2_global_alpha  : 8   ; /* [23..16]  */
		unsigned int    curld_osd3_global_alpha  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD_GALPHA0;

/* Define the union U_VEDU_CURLD_OSD_GALPHA1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd4_global_alpha  : 8   ; /* [7..0]  */
		unsigned int    curld_osd5_global_alpha  : 8   ; /* [15..8]  */
		unsigned int    curld_osd6_global_alpha  : 8   ; /* [23..16]  */
		unsigned int    curld_osd7_global_alpha  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD_GALPHA1;

/* Define the union U_VEDU_CURLD_OSD01_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd0_stride    : 16  ; /* [15..0]  */
		unsigned int    curld_osd1_stride    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD01_STRIDE;

/* Define the union U_VEDU_CURLD_OSD23_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd2_stride    : 16  ; /* [15..0]  */
		unsigned int    curld_osd3_stride    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD23_STRIDE;

/* Define the union U_VEDU_CURLD_OSD45_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd4_stride    : 16  ; /* [15..0]  */
		unsigned int    curld_osd5_stride    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD45_STRIDE;

/* Define the union U_VEDU_CURLD_OSD67_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_osd6_stride    : 16  ; /* [15..0]  */
		unsigned int    curld_osd7_stride    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_OSD67_STRIDE;

/* Define the union U_VEDU_CURLD_CLIP_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_clip_luma_min   : 8   ; /* [7..0]  */
		unsigned int    curld_clip_luma_max   : 8   ; /* [15..8]  */
		unsigned int    curld_clip_chrm_min   : 8   ; /* [23..16]  */
		unsigned int    curld_clip_chrm_max   : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_CLIP_THR;

/* Define the union U_VEDU_CURLD_HOR_FILTER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_filter_h0    : 4   ; /* [3..0]  */
		unsigned int    curld_filter_h1    : 4   ; /* [7..4]  */
		unsigned int    curld_filter_h2    : 4   ; /* [11..8]  */
		unsigned int    curld_filter_h3    : 4   ; /* [15..12]  */
		unsigned int    curld_filter_hrnd    : 4   ; /* [19..16]  */
		unsigned int    curld_filter_hshift   : 3   ; /* [22..20]  */
		unsigned int    Reserved_227    : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_HOR_FILTER;

/* Define the union U_VEDU_CURLD_VER_FILTER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_filter_v0    : 4   ; /* [3..0]  */
		unsigned int    curld_filter_v1    : 4   ; /* [7..4]  */
		unsigned int    curld_filter_v2    : 4   ; /* [11..8]  */
		unsigned int    curld_filter_v3    : 4   ; /* [15..12]  */
		unsigned int    curld_filter_vrnd    : 4   ; /* [19..16]  */
		unsigned int    curld_filter_vshift   : 3   ; /* [22..20]  */
		unsigned int    Reserved_228    : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_VER_FILTER;

/* Define the union U_VEDU_CURLD_ARGB_YUV_0COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_coeff_gy    : 15  ; /* [14..0]  */
		unsigned int    Reserved_230    : 1   ; /* [15]  */
		unsigned int    vcpi_coeff_ry    : 15  ; /* [30..16]  */
		unsigned int    Reserved_229    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_0COEFF;

/* Define the union U_VEDU_CURLD_ARGB_YUV_1COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_coeff_rcb    : 15  ; /* [14..0]  */
		unsigned int    Reserved_232    : 1   ; /* [15]  */
		unsigned int    vcpi_coeff_by    : 15  ; /* [30..16]  */
		unsigned int    Reserved_231    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_1COEFF;

/* Define the union U_VEDU_CURLD_ARGB_YUV_2COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_coeff_bcb    : 15  ; /* [14..0]  */
		unsigned int    Reserved_234    : 1   ; /* [15]  */
		unsigned int    vcpi_coeff_gcb    : 15  ; /* [30..16]  */
		unsigned int    Reserved_233    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_2COEFF;

/* Define the union U_VEDU_CURLD_ARGB_YUV_3COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_coeff_gcr    : 15  ; /* [14..0]  */
		unsigned int    Reserved_236    : 1   ; /* [15]  */
		unsigned int    vcpi_coeff_rcr    : 15  ; /* [30..16]  */
		unsigned int    Reserved_235    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_3COEFF;

/* Define the union U_VEDU_CURLD_ARGB_YUV_4COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_coeff_bcr    : 15  ; /* [14..0]  */
		unsigned int    Reserved_237    : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_4COEFF;

/* Define the union U_VEDU_CURLD_ARGB_YUV_5COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_rgb_rndy    : 16  ; /* [15..0]  */
		unsigned int    vcpi_rgb_shift    : 4   ; /* [19..16]  */
		unsigned int    Reserved_238    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_5COEFF;

/* Define the union U_VEDU_CURLD_ARGB_YUV_6COEFF */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_rgb_rndcr    : 16  ; /* [15..0]  */
		unsigned int    vcpi_rgb_rndcb    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_YUV_6COEFF;

/* Define the union U_VEDU_CURLD_ARGB_CLIP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_rgb_clpmin    : 8   ; /* [7..0]  */
		unsigned int    vcpi_rgb_clpmax    : 8   ; /* [15..8]  */
		unsigned int    vcpi_rgb_clip_en    : 1   ; /* [16]  */
		unsigned int    Reserved_239    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_ARGB_CLIP;

/* Define the union U_VEDU_CURLD_NARROW_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    curld_y_wide_narrow_en  : 1   ; /* [0]  */
		unsigned int    curld_c_wide_narrow_en  : 1   ; /* [1]  */
		unsigned int    Reserved_240    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_NARROW_EN;

/* Define the union U_VEDU_CURLD_SRCH_STRIDE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_curld_srcch_stride  : 16  ; /* [15..0]  */
		unsigned int    vcpi_curld_srcyh_stride  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_SRCH_STRIDE;

/* Define the union U_VEDU_CURLD_HFBCD */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_curld_hfbcd_bypass_en  : 1   ; /* [0]  */
		unsigned int    vcpi_curld_hfbcd_raw_en  : 1   ; /* [1]  */
		unsigned int    Reserved_241    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CURLD_HFBCD;

/* Define the union U_VEDU_PME_SW_ADAPT_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_psw_adapt_en   : 1   ; /* [0]  */
		unsigned int    pme_l1_psw_adapt_en   : 1   ; /* [1]  */
		unsigned int    Reserved_244    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SW_ADAPT_EN;

/* Define the union U_VEDU_PME_SW_THR0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_psw_thr0    : 16  ; /* [15..0]  */
		unsigned int    pme_l1_psw_thr0    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SW_THR0;

/* Define the union U_VEDU_PME_SW_THR1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_psw_thr1    : 16  ; /* [15..0]  */
		unsigned int    pme_l1_psw_thr1    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SW_THR1;

/* Define the union U_VEDU_PME_SW_THR2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_psw_thr2    : 16  ; /* [15..0]  */
		unsigned int    pme_l1_psw_thr2    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SW_THR2;

/* Define the union U_VEDU_PME_SKIP_PRE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_skipblk_pre_cost_thr  : 16  ; /* [15..0]  */
		unsigned int    pme_skipblk_pre_en    : 1   ; /* [16]  */
		unsigned int    Reserved_245    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SKIP_PRE;

/* Define the union U_VEDU_PME_TR_WEIGHTX */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_tr_weightx_0    : 9   ; /* [8..0]  */
		unsigned int    pme_tr_weightx_1    : 9   ; /* [17..9]  */
		unsigned int    pme_tr_weightx_2    : 9   ; /* [26..18]  */
		unsigned int    Reserved_246    : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_TR_WEIGHTX;

/* Define the union U_VEDU_PME_TR_WEIGHTY */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_tr_weighty_0    : 8   ; /* [7..0]  */
		unsigned int    pme_tr_weighty_1    : 8   ; /* [15..8]  */
		unsigned int    pme_tr_weighty_2    : 8   ; /* [23..16]  */
		unsigned int    Reserved_247    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_TR_WEIGHTY;

/* Define the union U_VEDU_PME_SR_WEIGHT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_sr_weight_0    : 4   ; /* [3..0]  */
		unsigned int    pme_sr_weight_1    : 4   ; /* [7..4]  */
		unsigned int    pme_sr_weight_2    : 4   ; /* [11..8]  */
		unsigned int    pme_pskip_strongedge_madi_thr  : 8   ; /* [19..12]  */
		unsigned int    pme_pskip_strongedge_madi_times  : 8   ; /* [27..20]  */
		unsigned int    Reserved_248    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SR_WEIGHT;

/* Define the union U_VEDU_PME_INTRABLK_DET */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_intrablk_det_cost_thr0  : 16  ; /* [15..0]  */
		unsigned int    pme_pskip_mvy_consistency_thr  : 8   ; /* [23..16]  */
		unsigned int    pme_pskip_mvx_consistency_thr  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_INTRABLK_DET;

/* Define the union U_VEDU_PME_INTRABLK_DET_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_intrablk_det_mv_dif_thr1  : 8   ; /* [7..0]  */
		unsigned int    pme_intrablk_det_mv_dif_thr0  : 8   ; /* [15..8]  */
		unsigned int    pme_intrablk_det_mvy_thr  : 8   ; /* [23..16]  */
		unsigned int    pme_intrablk_det_mvx_thr  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_INTRABLK_DET_THR;

/* Define the union U_VEDU_PME_SKIN_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_skin_u_max_thr    : 8   ; /* [7..0]  */
		unsigned int    pme_skin_u_min_thr    : 8   ; /* [15..8]  */
		unsigned int    pme_skin_v_max_thr    : 8   ; /* [23..16]  */
		unsigned int    pme_skin_v_min_thr    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SKIN_THR;

/* Define the union U_VEDU_PME_INTRA_LOWPOW */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_intra16_madi_thr  : 8   ; /* [7..0]  */
		unsigned int    pme_intra32_madi_thr  : 8   ; /* [15..8]  */
		unsigned int    pme_intra_lowpow_en   : 1   ; /* [16]  */
		unsigned int    pme_inter_first    : 1   ; /* [17]  */
		unsigned int    Reserved_249    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_INTRA_LOWPOW;

/* Define the union U_VEDU_PME_IBLK_COST_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_iblk_pre_cost_thr_h264  : 16  ; /* [15..0]  */
		unsigned int    pme_intrablk_det_cost_thr1  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_IBLK_COST_THR;

/* Define the union U_VEDU_PME_STRONG_EDGE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_skin_num    : 9   ; /* [8..0]  */
		unsigned int    pme_strong_edge_thr   : 8   ; /* [16..9]  */
		unsigned int    pme_strong_edge_cnt   : 5   ; /* [21..17]  */
		unsigned int    pme_still_scene_thr   : 9   ; /* [30..22]  */
		unsigned int    Reserved_250    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_STRONG_EDGE;

/* Define the union U_VEDU_PME_LARGE_MOVE_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_move_scene_thr    : 8   ; /* [7..0]  */
		unsigned int    pme_move_sad_thr    : 14  ; /* [21..8]  */
		unsigned int    Reserved_251    : 10  ; /* [31..22]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_LARGE_MOVE_THR;

/* Define the union U_VEDU_PME_INTER_STRONG_EDGE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_interdiff_max_min_madi_abs  : 8   ; /* [7..0]  */
		unsigned int    pme_interdiff_max_min_madi_times  : 8   ; /* [15..8]  */
		unsigned int    pme_interstrongedge_madi_thr  : 8   ; /* [23..16]  */
		unsigned int    Reserved_252    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_INTER_STRONG_EDGE;

/* Define the union U_VEDU_PME_NEW_COST */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_cost_lamda0    : 4   ; /* [3..0]  */
		unsigned int    pme_cost_lamda1    : 4   ; /* [7..4]  */
		unsigned int    pme_cost_lamda2    : 4   ; /* [11..8]  */
		unsigned int    pme_new_cost_en    : 2   ; /* [13..12]  */
		unsigned int    Reserved_254    : 2   ; /* [15..14]  */
		unsigned int    pme_cost_lamda_en    : 2   ; /* [17..16]  */
		unsigned int    pme_mvp3median_en    : 1   ; /* [18]  */
		unsigned int    Reserved_253    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_NEW_COST;

/* Define the union U_VEDU_PME_WINDOW_SIZE0_L0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_win0_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_256    : 6   ; /* [15..10]  */
		unsigned int    pme_l0_win0_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_255    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE0_L0;

/* Define the union U_VEDU_PME_WINDOW_SIZE1_L0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_win1_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_258    : 6   ; /* [15..10]  */
		unsigned int    pme_l0_win1_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_257    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE1_L0;

/* Define the union U_VEDU_PME_WINDOW_SIZE2_L0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_win2_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_260    : 6   ; /* [15..10]  */
		unsigned int    pme_l0_win2_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_259    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE2_L0;

/* Define the union U_VEDU_PME_WINDOW_SIZE3_L0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_win3_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_262    : 6   ; /* [15..10]  */
		unsigned int    pme_l0_win3_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_261    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE3_L0;

/* Define the union U_VEDU_PME_WINDOW_SIZE0_L1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l1_win0_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_264    : 6   ; /* [15..10]  */
		unsigned int    pme_l1_win0_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_263    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE0_L1;

/* Define the union U_VEDU_PME_WINDOW_SIZE1_L1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l1_win1_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_266    : 6   ; /* [15..10]  */
		unsigned int    pme_l1_win1_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_265    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE1_L1;

/* Define the union U_VEDU_PME_WINDOW_SIZE2_L1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l1_win2_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_268    : 6   ; /* [15..10]  */
		unsigned int    pme_l1_win2_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_267    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE2_L1;

/* Define the union U_VEDU_PME_WINDOW_SIZE3_L1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l1_win3_width    : 10  ; /* [9..0]  */
		unsigned int    Reserved_270    : 6   ; /* [15..10]  */
		unsigned int    pme_l1_win3_height    : 9   ; /* [24..16]  */
		unsigned int    Reserved_269    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_WINDOW_SIZE3_L1;

/* Define the union U_VEDU_PME_COST_OFFSET */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_l0_cost_offset    : 16  ; /* [15..0]  */
		unsigned int    pme_l1_cost_offset    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_COST_OFFSET;

/* Define the union U_VEDU_PME_SAFE_CFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_safe_line    : 13  ; /* [12..0]  */
		unsigned int    pme_safe_line_val    : 1   ; /* [13]  */
		unsigned int    Reserved_271    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SAFE_CFG;

/* Define the union U_VEDU_PME_IBLK_REFRESH */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_iblk_refresh_start_num  : 18  ; /* [17..0]  */
		unsigned int    Reserved_272    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_IBLK_REFRESH;

/* Define the union U_VEDU_PME_IBLK_REFRESH_NUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_iblk_refresh_num  : 18  ; /* [17..0]  */
		unsigned int    Reserved_273    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_IBLK_REFRESH_NUM;

/* Define the union U_VEDU_PME_QPG_RC_THR0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_275    : 16  ; /* [15..0]  */
		unsigned int    pme_madi_dif_thr    : 7   ; /* [22..16]  */
		unsigned int    pme_cur_madi_dif_thr  : 7   ; /* [29..23]  */
		unsigned int    Reserved_274    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_QPG_RC_THR0;

/* Define the union U_VEDU_PME_QPG_RC_THR1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_min_sad_thr_offset  : 8   ; /* [7..0]  */
		unsigned int    pme_min_sad_thr_gain  : 4   ; /* [11..8]  */
		unsigned int    pme_smooth_madi_thr   : 8   ; /* [19..12]  */
		unsigned int    pme_min_sad_thr_offset_cur  : 8   ; /* [27..20]  */
		unsigned int    pme_min_sad_thr_gain_cur  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_QPG_RC_THR1;

/* Define the union U_VEDU_PME_LOW_LUMA_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_low_luma_thr    : 8   ; /* [7..0]  */
		unsigned int    pme_low_luma_madi_thr  : 8   ; /* [15..8]  */
		unsigned int    pme_high_luma_thr    : 8   ; /* [23..16]  */
		unsigned int    Reserved_276    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_LOW_LUMA_THR;

/* Define the union U_VEDU_PME_PBLK_PRE1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_pblk_pre_mv_dif_thr1  : 8   ; /* [7..0]  */
		unsigned int    pme_pblk_pre_mv_dif_thr0  : 8   ; /* [15..8]  */
		unsigned int    pme_pblk_pre_mv_dif_cost_thr  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_PBLK_PRE1;

/* Define the union U_VEDU_PME_CHROMA_FLAT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_flat_v_thr_high   : 8   ; /* [7..0]  */
		unsigned int    pme_flat_v_thr_low    : 8   ; /* [15..8]  */
		unsigned int    pme_flat_u_thr_high   : 8   ; /* [23..16]  */
		unsigned int    pme_flat_u_thr_low    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_CHROMA_FLAT;

/* Define the union U_VEDU_PME_LUMA_FLAT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_flat_pmemv_thr    : 8   ; /* [7..0]  */
		unsigned int    pme_flat_luma_madi_thr  : 8   ; /* [15..8]  */
		unsigned int    pme_flat_low_luma_thr  : 8   ; /* [23..16]  */
		unsigned int    pme_flat_high_luma_thr  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_LUMA_FLAT;

/* Define the union U_VEDU_PME_MADI_FLAT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_flat_pmesad_thr   : 14  ; /* [13..0]  */
		unsigned int    pme_flat_icount_thr   : 9   ; /* [22..14]  */
		unsigned int    pme_flat_region_cnt   : 5   ; /* [27..23]  */
		unsigned int    pme_flat_madi_times   : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_MADI_FLAT;

/* Define the union U_VEDU_PME_SKIP_LARGE_RES */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_skip_sad_thr_offset  : 8   ; /* [7..0]  */
		unsigned int    pme_skip_sad_thr_gain  : 4   ; /* [11..8]  */
		unsigned int    pme_skip_large_res_det  : 1   ; /* [12]  */
		unsigned int    Reserved_277    : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PME_SKIP_LARGE_RES;

/* Define the union U_VEDU_QPG_MAX_MIN_QP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_min_qp    : 6   ; /* [5..0]  */
		unsigned int    Reserved_282    : 2   ; /* [7..6]  */
		unsigned int    qpg_max_qp    : 6   ; /* [13..8]  */
		unsigned int    Reserved_281    : 2   ; /* [15..14]  */
		unsigned int    qpg_cu_qp_delta_enable_flag  : 1   ; /* [16]  */
		unsigned int    Reserved_280    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_MAX_MIN_QP;

/* Define the union U_VEDU_QPG_ROW_TARGET_BITS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_qp_delta    : 6   ; /* [5..0]  */
		unsigned int    qpg_row_target_bits   : 25  ; /* [30..6]  */
		unsigned int    Reserved_283    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_ROW_TARGET_BITS;

/* Define the union U_VEDU_QPG_AVERAGE_LCU_BITS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_ave_lcu_bits    : 16  ; /* [15..0]  */
		unsigned int    Reserved_284    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_AVERAGE_LCU_BITS;

/* Define the union U_VEDU_QPG_LOWLUMA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lowluma_min_qp    : 6   ; /* [5..0]  */
		unsigned int    Reserved_287    : 2   ; /* [7..6]  */
		unsigned int    qpg_lowluma_max_qp    : 6   ; /* [13..8]  */
		unsigned int    Reserved_286    : 2   ; /* [15..14]  */
		unsigned int    qpg_lowluma_qp_delta  : 4   ; /* [19..16]  */
		unsigned int    Reserved_285    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_LOWLUMA;

/* Define the union U_VEDU_QPG_HEDGE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_hedge_min_qp    : 6   ; /* [5..0]  */
		unsigned int    Reserved_290    : 2   ; /* [7..6]  */
		unsigned int    qpg_hedge_max_qp    : 6   ; /* [13..8]  */
		unsigned int    Reserved_289    : 2   ; /* [15..14]  */
		unsigned int    qpg_hedge_qp_delta    : 4   ; /* [19..16]  */
		unsigned int    Reserved_288    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_HEDGE;

/* Define the union U_VEDU_QPG_HEDGE_MOVE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_hedge_move_min_qp  : 6   ; /* [5..0]  */
		unsigned int    Reserved_293    : 2   ; /* [7..6]  */
		unsigned int    qpg_hedge_move_max_qp  : 6   ; /* [13..8]  */
		unsigned int    Reserved_292    : 2   ; /* [15..14]  */
		unsigned int    qpg_hedge_move_qp_delta  : 4   ; /* [19..16]  */
		unsigned int    Reserved_291    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_HEDGE_MOVE;

/* Define the union U_VEDU_QPG_LARGE_MOVE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_large_move_min_qp  : 6   ; /* [5..0]  */
		unsigned int    Reserved_296    : 2   ; /* [7..6]  */
		unsigned int    qpg_large_move_max_qp  : 6   ; /* [13..8]  */
		unsigned int    Reserved_295    : 2   ; /* [15..14]  */
		unsigned int    qpg_large_move_qp_delta  : 4   ; /* [19..16]  */
		unsigned int    Reserved_294    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_LARGE_MOVE;

/* Define the union U_VEDU_QPG_SKIN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_skin_min_qp    : 6   ; /* [5..0]  */
		unsigned int    Reserved_299    : 2   ; /* [7..6]  */
		unsigned int    qpg_skin_max_qp    : 6   ; /* [13..8]  */
		unsigned int    Reserved_298    : 2   ; /* [15..14]  */
		unsigned int    qpg_skin_qp_delta    : 4   ; /* [19..16]  */
		unsigned int    Reserved_297    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_SKIN;

/* Define the union U_VEDU_QPG_INTRA_DET */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_intra_det_min_qp  : 6   ; /* [5..0]  */
		unsigned int    Reserved_302    : 2   ; /* [7..6]  */
		unsigned int    qpg_intra_det_max_qp  : 6   ; /* [13..8]  */
		unsigned int    Reserved_301    : 2   ; /* [15..14]  */
		unsigned int    qpg_intra_det_qp_delta  : 4   ; /* [19..16]  */
		unsigned int    Reserved_300    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_INTRA_DET;

/* Define the union U_VEDU_QPG_H264_SMOOTH */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_h264_smooth_min_qp  : 6   ; /* [5..0]  */
		unsigned int    Reserved_305    : 2   ; /* [7..6]  */
		unsigned int    qpg_h264_smooth_max_qp  : 6   ; /* [13..8]  */
		unsigned int    Reserved_304    : 2   ; /* [15..14]  */
		unsigned int    qpg_h264_smooth_qp_delta  : 4   ; /* [19..16]  */
		unsigned int    qpg_h264_smooth_qp_delta1  : 4   ; /* [23..20]  */
		unsigned int    Reserved_303    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_H264_SMOOTH;

/* Define the union U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_cu_qp_delta_thresh0  : 8   ; /* [7..0]  */
		unsigned int    qpg_cu_qp_delta_thresh1  : 8   ; /* [15..8]  */
		unsigned int    qpg_cu_qp_delta_thresh2  : 8   ; /* [23..16]  */
		unsigned int    qpg_cu_qp_delta_thresh3  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0;

/* Define the union U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_cu_qp_delta_thresh4  : 8   ; /* [7..0]  */
		unsigned int    qpg_cu_qp_delta_thresh5  : 8   ; /* [15..8]  */
		unsigned int    qpg_cu_qp_delta_thresh6  : 8   ; /* [23..16]  */
		unsigned int    qpg_cu_qp_delta_thresh7  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1;

/* Define the union U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_cu_qp_delta_thresh8  : 8   ; /* [7..0]  */
		unsigned int    qpg_cu_qp_delta_thresh9  : 8   ; /* [15..8]  */
		unsigned int    qpg_cu_qp_delta_thresh10  : 8   ; /* [23..16]  */
		unsigned int    qpg_cu_qp_delta_thresh11  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2;

/* Define the union U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_cu_qp_delta_thresh12  : 8   ; /* [7..0]  */
		unsigned int    qpg_cu_qp_delta_thresh13  : 8   ; /* [15..8]  */
		unsigned int    qpg_cu_qp_delta_thresh14  : 8   ; /* [23..16]  */
		unsigned int    qpg_cu_qp_delta_thresh15  : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3;

/* Define the union U_VEDU_QPG_DELTA_LEVEL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_qp_delta_level_0  : 2   ; /* [1..0]  */
		unsigned int    qpg_qp_delta_level_1  : 2   ; /* [3..2]  */
		unsigned int    qpg_qp_delta_level_2  : 2   ; /* [5..4]  */
		unsigned int    qpg_qp_delta_level_3  : 2   ; /* [7..6]  */
		unsigned int    qpg_qp_delta_level_4  : 2   ; /* [9..8]  */
		unsigned int    qpg_qp_delta_level_5  : 2   ; /* [11..10]  */
		unsigned int    qpg_qp_delta_level_6  : 2   ; /* [13..12]  */
		unsigned int    qpg_qp_delta_level_7  : 2   ; /* [15..14]  */
		unsigned int    qpg_qp_delta_level_8  : 2   ; /* [17..16]  */
		unsigned int    qpg_qp_delta_level_9  : 2   ; /* [19..18]  */
		unsigned int    qpg_qp_delta_level_10  : 2   ; /* [21..20]  */
		unsigned int    qpg_qp_delta_level_11  : 2   ; /* [23..22]  */
		unsigned int    qpg_qp_delta_level_12  : 2   ; /* [25..24]  */
		unsigned int    qpg_qp_delta_level_13  : 2   ; /* [27..26]  */
		unsigned int    qpg_qp_delta_level_14  : 2   ; /* [29..28]  */
		unsigned int    qpg_qp_delta_level_15  : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_DELTA_LEVEL;

/* Define the union U_VEDU_QPG_MADI_SWITCH_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_qp_madi_switch_thr  : 5   ; /* [4..0]  */
		unsigned int    Reserved_306    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_MADI_SWITCH_THR;

/* Define the union U_VEDU_QPG_CU32_DELTA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_cu32_delta_low    : 4   ; /* [3..0]  */
		unsigned int    qpg_cu32_delta_high   : 4   ; /* [7..4]  */
		unsigned int    Reserved_307    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_CU32_DELTA;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG00 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda00    : 20  ; /* [19..0]  */
		unsigned int    Reserved_308    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG00;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG01 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda01    : 20  ; /* [19..0]  */
		unsigned int    Reserved_309    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG01;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG02 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda02    : 20  ; /* [19..0]  */
		unsigned int    Reserved_310    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG02;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG03 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda03    : 20  ; /* [19..0]  */
		unsigned int    Reserved_311    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG03;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG04 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda04    : 20  ; /* [19..0]  */
		unsigned int    Reserved_312    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG04;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG05 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda05    : 20  ; /* [19..0]  */
		unsigned int    Reserved_313    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG05;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG06 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda06    : 20  ; /* [19..0]  */
		unsigned int    Reserved_314    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG06;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG07 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda07    : 20  ; /* [19..0]  */
		unsigned int    Reserved_315    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG07;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG08 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda08    : 20  ; /* [19..0]  */
		unsigned int    Reserved_316    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG08;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG09 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda09    : 20  ; /* [19..0]  */
		unsigned int    Reserved_317    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG09;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG10 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda10    : 20  ; /* [19..0]  */
		unsigned int    Reserved_318    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG10;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG11 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda11    : 20  ; /* [19..0]  */
		unsigned int    Reserved_319    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG11;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG12 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda12    : 20  ; /* [19..0]  */
		unsigned int    Reserved_320    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG12;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG13 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda13    : 20  ; /* [19..0]  */
		unsigned int    Reserved_321    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG13;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG14 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda14    : 20  ; /* [19..0]  */
		unsigned int    Reserved_322    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG14;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG15 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda15    : 20  ; /* [19..0]  */
		unsigned int    Reserved_323    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG15;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG16 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda16    : 20  ; /* [19..0]  */
		unsigned int    Reserved_324    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG16;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG17 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda17    : 20  ; /* [19..0]  */
		unsigned int    Reserved_325    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG17;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG18 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda18    : 20  ; /* [19..0]  */
		unsigned int    Reserved_326    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG18;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG19 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda19    : 20  ; /* [19..0]  */
		unsigned int    Reserved_327    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG19;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG20 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda20    : 20  ; /* [19..0]  */
		unsigned int    Reserved_328    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG20;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG21 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda21    : 20  ; /* [19..0]  */
		unsigned int    Reserved_329    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG21;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG22 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda22    : 20  ; /* [19..0]  */
		unsigned int    Reserved_330    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG22;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG23 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda23    : 20  ; /* [19..0]  */
		unsigned int    Reserved_331    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG23;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG24 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda24    : 20  ; /* [19..0]  */
		unsigned int    Reserved_332    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG24;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG25 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda25    : 20  ; /* [19..0]  */
		unsigned int    Reserved_333    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG25;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG26 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda26    : 20  ; /* [19..0]  */
		unsigned int    Reserved_334    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG26;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG27 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda27    : 20  ; /* [19..0]  */
		unsigned int    Reserved_335    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG27;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG28 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda28    : 20  ; /* [19..0]  */
		unsigned int    Reserved_336    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG28;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG29 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda29    : 20  ; /* [19..0]  */
		unsigned int    Reserved_337    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG29;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG30 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda30    : 20  ; /* [19..0]  */
		unsigned int    Reserved_338    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG30;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG31 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda31    : 20  ; /* [19..0]  */
		unsigned int    Reserved_339    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG31;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG32 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda32    : 20  ; /* [19..0]  */
		unsigned int    Reserved_340    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG32;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG33 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda33    : 20  ; /* [19..0]  */
		unsigned int    Reserved_341    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG33;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG34 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda34    : 20  ; /* [19..0]  */
		unsigned int    Reserved_342    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG34;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG35 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda35    : 20  ; /* [19..0]  */
		unsigned int    Reserved_343    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG35;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG36 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda36    : 20  ; /* [19..0]  */
		unsigned int    Reserved_344    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG36;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG37 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda37    : 20  ; /* [19..0]  */
		unsigned int    Reserved_345    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG37;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG38 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda38    : 20  ; /* [19..0]  */
		unsigned int    Reserved_346    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG38;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG39 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda39    : 20  ; /* [19..0]  */
		unsigned int    Reserved_347    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG39;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG40 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda40    : 20  ; /* [19..0]  */
		unsigned int    Reserved_348    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG40;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG41 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda41    : 20  ; /* [19..0]  */
		unsigned int    Reserved_349    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG41;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG42 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda42    : 20  ; /* [19..0]  */
		unsigned int    Reserved_350    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG42;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG43 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda43    : 20  ; /* [19..0]  */
		unsigned int    Reserved_351    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG43;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG44 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda44    : 20  ; /* [19..0]  */
		unsigned int    Reserved_352    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG44;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG45 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda45    : 20  ; /* [19..0]  */
		unsigned int    Reserved_353    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG45;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG46 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda46    : 20  ; /* [19..0]  */
		unsigned int    Reserved_354    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG46;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG47 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda47    : 20  ; /* [19..0]  */
		unsigned int    Reserved_355    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG47;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG48 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda48    : 20  ; /* [19..0]  */
		unsigned int    Reserved_356    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG48;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG49 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda49    : 20  ; /* [19..0]  */
		unsigned int    Reserved_357    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG49;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG50 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda50    : 20  ; /* [19..0]  */
		unsigned int    Reserved_358    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG50;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG51 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda51    : 20  ; /* [19..0]  */
		unsigned int    Reserved_359    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG51;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG52 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda52    : 20  ; /* [19..0]  */
		unsigned int    Reserved_360    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG52;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG53 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda53    : 20  ; /* [19..0]  */
		unsigned int    Reserved_361    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG53;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG54 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda54    : 20  ; /* [19..0]  */
		unsigned int    Reserved_362    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG54;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG55 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda55    : 20  ; /* [19..0]  */
		unsigned int    Reserved_363    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG55;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG56 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda56    : 20  ; /* [19..0]  */
		unsigned int    Reserved_364    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG56;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG57 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda57    : 20  ; /* [19..0]  */
		unsigned int    Reserved_365    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG57;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG58 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda58    : 20  ; /* [19..0]  */
		unsigned int    Reserved_366    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG58;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG59 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda59    : 20  ; /* [19..0]  */
		unsigned int    Reserved_367    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG59;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG60 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda60    : 20  ; /* [19..0]  */
		unsigned int    Reserved_368    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG60;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG61 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda61    : 20  ; /* [19..0]  */
		unsigned int    Reserved_369    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG61;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG62 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda62    : 20  ; /* [19..0]  */
		unsigned int    Reserved_370    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG62;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG63 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda63    : 20  ; /* [19..0]  */
		unsigned int    Reserved_371    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG63;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG64 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda64    : 20  ; /* [19..0]  */
		unsigned int    Reserved_372    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG64;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG65 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda65    : 20  ; /* [19..0]  */
		unsigned int    Reserved_373    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG65;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG66 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda66    : 20  ; /* [19..0]  */
		unsigned int    Reserved_374    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG66;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG67 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda67    : 20  ; /* [19..0]  */
		unsigned int    Reserved_375    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG67;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG68 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda68    : 20  ; /* [19..0]  */
		unsigned int    Reserved_376    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG68;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG69 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda69    : 20  ; /* [19..0]  */
		unsigned int    Reserved_377    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG69;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG70 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda70    : 20  ; /* [19..0]  */
		unsigned int    Reserved_378    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG70;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG71 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda71    : 20  ; /* [19..0]  */
		unsigned int    Reserved_379    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG71;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG72 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda72    : 20  ; /* [19..0]  */
		unsigned int    Reserved_380    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG72;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG73 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda73    : 20  ; /* [19..0]  */
		unsigned int    Reserved_381    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG73;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG74 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda74    : 20  ; /* [19..0]  */
		unsigned int    Reserved_382    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG74;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG75 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda75    : 20  ; /* [19..0]  */
		unsigned int    Reserved_383    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG75;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG76 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda76    : 20  ; /* [19..0]  */
		unsigned int    Reserved_384    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG76;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG77 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda77    : 20  ; /* [19..0]  */
		unsigned int    Reserved_385    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG77;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG78 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda78    : 20  ; /* [19..0]  */
		unsigned int    Reserved_386    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG78;

/* Define the union U_VEDU_QPG_QP_LAMBDA_CTRL_REG79 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda79    : 20  ; /* [19..0]  */
		unsigned int    Reserved_387    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_LAMBDA_CTRL_REG79;

/* Define the union U_VEDU_QPG_LAMBDA_MODE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_lambda_qp_offset  : 5   ; /* [4..0]  */
		unsigned int    Reserved_390    : 3   ; /* [7..5]  */
		unsigned int    qpg_rdo_lambda_choose_mode  : 2   ; /* [9..8]  */
		unsigned int    Reserved_389    : 2   ; /* [11..10]  */
		unsigned int    qpg_lambda_inter_stredge_en  : 1   ; /* [12]  */
		unsigned int    Reserved_388    : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_LAMBDA_MODE;

/* Define the union U_VEDU_QPG_QP_RESTRAIN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_qp_restrain_madi_thr  : 6   ; /* [5..0]  */
		unsigned int    Reserved_393    : 2   ; /* [7..6]  */
		unsigned int    qpg_qp_restrain_en    : 1   ; /* [8]  */
		unsigned int    qpg_qp_restrain_mode  : 2   ; /* [10..9]  */
		unsigned int    Reserved_392    : 1   ; /* [11]  */
		unsigned int    qpg_qp_restrain_delta_blk16  : 4   ; /* [15..12]  */
		unsigned int    qpg_qp_restrain_delta_blk32  : 4   ; /* [19..16]  */
		unsigned int    Reserved_391    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_QP_RESTRAIN;

/* Define the union U_VEDU_QPG_CU_MIN_SAD_REG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_min_sad_level    : 16  ; /* [15..0]  */
		unsigned int    qpg_low_min_sad_mode  : 2   ; /* [17..16]  */
		unsigned int    qpg_high_min_sad_mode  : 2   ; /* [19..18]  */
		unsigned int    qpg_min_sad_madi_en   : 1   ; /* [20]  */
		unsigned int    qpg_min_sad_qp_restrain_en  : 1   ; /* [21]  */
		unsigned int    Reserved_394    : 10  ; /* [31..22]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_CU_MIN_SAD_REG;

/* Define the union U_VEDU_QPG_SMART_REG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_396    : 1   ; /* [0]  */
		unsigned int    qpg_smart_abs_qp_mode  : 1   ; /* [1]  */
		unsigned int    qpg_smart_get_cu32_qp_mode  : 2   ; /* [3..2]  */
		unsigned int    qpg_smart_get_cu64_qp_mode  : 2   ; /* [5..4]  */
		unsigned int    qpg_qp_detlta_size_cu64  : 1   ; /* [6]  */
		unsigned int    Reserved_395    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_SMART_REG;

/* Define the union U_VEDU_QPG_FLAT_REGION */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    qpg_flat_region_qp_delta  : 4   ; /* [3..0]  */
		unsigned int    qpg_flat_region_max_qp  : 6   ; /* [9..4]  */
		unsigned int    Reserved_399    : 2   ; /* [11..10]  */
		unsigned int    qpg_flat_region_min_qp  : 6   ; /* [17..12]  */
		unsigned int    Reserved_398    : 6   ; /* [23..18]  */
		unsigned int    vcpi_cu32_use_cu16_mean_en  : 1   ; /* [24]  */
		unsigned int    Reserved_397    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_QPG_FLAT_REGION;

/* Define the union U_VEDU_IME_INTER_MODE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_layer3to2_en    : 1   ; /* [0]  */
		unsigned int    ime_inter8x8_en    : 1   ; /* [1]  */
		unsigned int    ime_flat_region_force_low3layer  : 1   ; /* [2]  */
		unsigned int    Reserved_404    : 1   ; /* [3]  */
		unsigned int    ime_high3pre_en    : 2   ; /* [5..4]  */
		unsigned int    Reserved_403    : 2   ; /* [7..6]  */
		unsigned int    ime_intra4_lowpow_en  : 1   ; /* [8]  */
		unsigned int    Reserved_402    : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_INTER_MODE;

/* Define the union U_VEDU_IME_RDOCFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_lambdaoff8    : 16  ; /* [15..0]  */
		unsigned int    ime_lambdaoff16    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_RDOCFG;

/* Define the union U_VEDU_IME_FME_LPOW_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_lowpow_fme_thr0   : 6   ; /* [5..0]  */
		unsigned int    ime_lowpow_fme_thr1   : 6   ; /* [11..6]  */
		unsigned int    Reserved_405    : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_FME_LPOW_THR;

/* Define the union U_VEDU_IME_LAYER3TO2_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_layer3to2_thr0    : 10  ; /* [9..0]  */
		unsigned int    Reserved_407    : 6   ; /* [15..10]  */
		unsigned int    ime_layer3to2_thr1    : 10  ; /* [25..16]  */
		unsigned int    Reserved_406    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_LAYER3TO2_THR;

/* Define the union U_VEDU_IME_LAYER3TO2_THR1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_layer3to2_cost_diff_thr  : 10  ; /* [9..0]  */
		unsigned int    Reserved_408    : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_LAYER3TO2_THR1;

/* Define the union U_VEDU_IME_LAYER3TO1_THR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_layer3to1_en    : 1   ; /* [0]  */
		unsigned int    Reserved_410    : 7   ; /* [7..1]  */
		unsigned int    ime_layer3to1_pu64_madi_thr  : 7   ; /* [14..8]  */
		unsigned int    Reserved_409    : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_LAYER3TO1_THR;

/* Define the union U_VEDU_IME_LAYER3TO1_THR1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ime_layer3to1_pu32_cost_thr  : 16  ; /* [15..0]  */
		unsigned int    ime_layer3to1_pu64_cost_thr  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_IME_LAYER3TO1_THR1;

/* Define the union U_VEDU_FME_BIAS_COST0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_pu8_bias_cost    : 16  ; /* [15..0]  */
		unsigned int    fme_pu16_bias_cost    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_FME_BIAS_COST0;

/* Define the union U_VEDU_FME_BIAS_COST1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_pu32_bias_cost    : 16  ; /* [15..0]  */
		unsigned int    fme_pu64_bias_cost    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_FME_BIAS_COST1;

/* Define the union U_VEDU_FME_PU64_LWP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_pu64_lwp_flag    : 1   ; /* [0]  */
		unsigned int    Reserved_412    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_FME_PU64_LWP;

/* Define the union U_VEDU_MRG_FORCE_ZERO_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_force_zero_en    : 1   ; /* [0]  */
		unsigned int    mrg_force_y_zero_en   : 1   ; /* [1]  */
		unsigned int    mrg_force_u_zero_en   : 1   ; /* [2]  */
		unsigned int    mrg_force_v_zero_en   : 1   ; /* [3]  */
		unsigned int    fme_lpw_en    : 1   ; /* [4]  */
		unsigned int    dct4_en    : 1   ; /* [5]  */
		unsigned int    force_adapt_en    : 1   ; /* [6]  */
		unsigned int    Reserved_415    : 5   ; /* [11..7]  */
		unsigned int    rqt_bias_weight    : 4   ; /* [15..12]  */
		unsigned int    fme_lpw_th    : 10  ; /* [25..16]  */
		unsigned int    Reserved_414    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_FORCE_ZERO_EN;

/* Define the union U_VEDU_MRG_FORCE_SKIP_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_417    : 4   ; /* [3..0]  */
		unsigned int    mrg_force_skip_en    : 1   ; /* [4]  */
		unsigned int    Reserved_416    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_FORCE_SKIP_EN;

/* Define the union U_VEDU_MRG_BIAS_COST0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_pu8_bias_cost    : 16  ; /* [15..0]  */
		unsigned int    mrg_pu16_bias_cost    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_BIAS_COST0;

/* Define the union U_VEDU_MRG_BIAS_COST1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_pu32_bias_cost    : 16  ; /* [15..0]  */
		unsigned int    mrg_pu64_bias_cost    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_BIAS_COST1;

/* Define the union U_VEDU_MRG_ABS_OFFSET0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_pu8_abs_offset    : 16  ; /* [15..0]  */
		unsigned int    mrg_pu16_abs_offset   : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_ABS_OFFSET0;

/* Define the union U_VEDU_MRG_ABS_OFFSET1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_pu32_abs_offset   : 16  ; /* [15..0]  */
		unsigned int    mrg_pu64_abs_offset   : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_ABS_OFFSET1;

/* Define the union U_VEDU_MRG_ADJ_WEIGHT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cu8_fz_weight    : 4   ; /* [3..0]  */
		unsigned int    cu16_fz_weight    : 4   ; /* [7..4]  */
		unsigned int    cu32_fz_weight    : 4   ; /* [11..8]  */
		unsigned int    cu64_fz_weight    : 4   ; /* [15..12]  */
		unsigned int    cu8_fz_adapt_weight   : 4   ; /* [19..16]  */
		unsigned int    cu16_fz_adapt_weight  : 4   ; /* [23..20]  */
		unsigned int    cu32_fz_adapt_weight  : 4   ; /* [27..24]  */
		unsigned int    cu64_fz_adapt_weight  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_MRG_ADJ_WEIGHT;

/* Define the union U_VEDU_INTRA_CFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    constrained_intra_pred_flag  : 1   ; /* [0]  */
		unsigned int    Reserved_421    : 3   ; /* [3..1]  */
		unsigned int    vcpi_force_cu16_only_dc_pl  : 1   ; /* [4]  */
		unsigned int    Reserved_420    : 3   ; /* [7..5]  */
		unsigned int    vcpi_force_cu32_only_dc_pl  : 1   ; /* [8]  */
		unsigned int    Reserved_419    : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CFG;

/* Define the union U_VEDU_INTRA_SMOOTH */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_smooth    : 1   ; /* [0]  */
		unsigned int    Reserved_422    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_SMOOTH;

/* Define the union U_VEDU_INTRA_BIT_WEIGHT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_bit_weight    : 4   ; /* [3..0]  */
		unsigned int    Reserved_423    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_BIT_WEIGHT;

/* Define the union U_VEDU_INTRA_RDO_COST_OFFSET_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu16_rdo_cost_offset  : 16  ; /* [15..0]  */
		unsigned int    intra_cu32_rdo_cost_offset  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_RDO_COST_OFFSET_0;

/* Define the union U_VEDU_INTRA_RDO_COST_OFFSET_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu4_rdo_cost_offset  : 16  ; /* [15..0]  */
		unsigned int    intra_cu8_rdo_cost_offset  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_RDO_COST_OFFSET_1;

/* Define the union U_VEDU_INTRA_NO_DC_COST_OFFSET_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu16_non_dc_mode_offset  : 16  ; /* [15..0]  */
		unsigned int    intra_cu32_non_dc_mode_offset  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_NO_DC_COST_OFFSET_0;

/* Define the union U_VEDU_INTRA_NO_DC_COST_OFFSET_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu4_non_dc_mode_offset  : 16  ; /* [15..0]  */
		unsigned int    intra_cu8_non_dc_mode_offset  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_NO_DC_COST_OFFSET_1;

/* Define the union U_VEDU_INTRA_CHNL4_ANG_0EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu4_mode0_en    : 1   ; /* [0]  */
		unsigned int    intra_cu4_mode1_en    : 1   ; /* [1]  */
		unsigned int    intra_cu4_mode2_en    : 1   ; /* [2]  */
		unsigned int    intra_cu4_mode3_en    : 1   ; /* [3]  */
		unsigned int    intra_cu4_mode4_en    : 1   ; /* [4]  */
		unsigned int    intra_cu4_mode5_en    : 1   ; /* [5]  */
		unsigned int    intra_cu4_mode6_en    : 1   ; /* [6]  */
		unsigned int    intra_cu4_mode7_en    : 1   ; /* [7]  */
		unsigned int    intra_cu4_mode8_en    : 1   ; /* [8]  */
		unsigned int    intra_cu4_mode9_en    : 1   ; /* [9]  */
		unsigned int    intra_cu4_mode10_en   : 1   ; /* [10]  */
		unsigned int    intra_cu4_mode11_en   : 1   ; /* [11]  */
		unsigned int    intra_cu4_mode12_en   : 1   ; /* [12]  */
		unsigned int    intra_cu4_mode13_en   : 1   ; /* [13]  */
		unsigned int    intra_cu4_mode14_en   : 1   ; /* [14]  */
		unsigned int    intra_cu4_mode15_en   : 1   ; /* [15]  */
		unsigned int    intra_cu4_mode16_en   : 1   ; /* [16]  */
		unsigned int    intra_cu4_mode17_en   : 1   ; /* [17]  */
		unsigned int    intra_cu4_mode18_en   : 1   ; /* [18]  */
		unsigned int    intra_cu4_mode19_en   : 1   ; /* [19]  */
		unsigned int    intra_cu4_mode20_en   : 1   ; /* [20]  */
		unsigned int    intra_cu4_mode21_en   : 1   ; /* [21]  */
		unsigned int    intra_cu4_mode22_en   : 1   ; /* [22]  */
		unsigned int    intra_cu4_mode23_en   : 1   ; /* [23]  */
		unsigned int    intra_cu4_mode24_en   : 1   ; /* [24]  */
		unsigned int    intra_cu4_mode25_en   : 1   ; /* [25]  */
		unsigned int    intra_cu4_mode26_en   : 1   ; /* [26]  */
		unsigned int    intra_cu4_mode27_en   : 1   ; /* [27]  */
		unsigned int    intra_cu4_mode28_en   : 1   ; /* [28]  */
		unsigned int    intra_cu4_mode29_en   : 1   ; /* [29]  */
		unsigned int    intra_cu4_mode30_en   : 1   ; /* [30]  */
		unsigned int    intra_cu4_mode31_en   : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL4_ANG_0EN;

/* Define the union U_VEDU_INTRA_CHNL4_ANG_1EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu4_mode32_en   : 1   ; /* [0]  */
		unsigned int    intra_cu4_mode33_en   : 1   ; /* [1]  */
		unsigned int    intra_cu4_mode34_en   : 1   ; /* [2]  */
		unsigned int    Reserved_424    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL4_ANG_1EN;

/* Define the union U_VEDU_INTRA_CHNL8_ANG_0EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu8_mode0_en    : 1   ; /* [0]  */
		unsigned int    intra_cu8_mode1_en    : 1   ; /* [1]  */
		unsigned int    intra_cu8_mode2_en    : 1   ; /* [2]  */
		unsigned int    intra_cu8_mode3_en    : 1   ; /* [3]  */
		unsigned int    intra_cu8_mode4_en    : 1   ; /* [4]  */
		unsigned int    intra_cu8_mode5_en    : 1   ; /* [5]  */
		unsigned int    intra_cu8_mode6_en    : 1   ; /* [6]  */
		unsigned int    intra_cu8_mode7_en    : 1   ; /* [7]  */
		unsigned int    intra_cu8_mode8_en    : 1   ; /* [8]  */
		unsigned int    intra_cu8_mode9_en    : 1   ; /* [9]  */
		unsigned int    intra_cu8_mode10_en   : 1   ; /* [10]  */
		unsigned int    intra_cu8_mode11_en   : 1   ; /* [11]  */
		unsigned int    intra_cu8_mode12_en   : 1   ; /* [12]  */
		unsigned int    intra_cu8_mode13_en   : 1   ; /* [13]  */
		unsigned int    intra_cu8_mode14_en   : 1   ; /* [14]  */
		unsigned int    intra_cu8_mode15_en   : 1   ; /* [15]  */
		unsigned int    intra_cu8_mode16_en   : 1   ; /* [16]  */
		unsigned int    intra_cu8_mode17_en   : 1   ; /* [17]  */
		unsigned int    intra_cu8_mode18_en   : 1   ; /* [18]  */
		unsigned int    intra_cu8_mode19_en   : 1   ; /* [19]  */
		unsigned int    intra_cu8_mode20_en   : 1   ; /* [20]  */
		unsigned int    intra_cu8_mode21_en   : 1   ; /* [21]  */
		unsigned int    intra_cu8_mode22_en   : 1   ; /* [22]  */
		unsigned int    intra_cu8_mode23_en   : 1   ; /* [23]  */
		unsigned int    intra_cu8_mode24_en   : 1   ; /* [24]  */
		unsigned int    intra_cu8_mode25_en   : 1   ; /* [25]  */
		unsigned int    intra_cu8_mode26_en   : 1   ; /* [26]  */
		unsigned int    intra_cu8_mode27_en   : 1   ; /* [27]  */
		unsigned int    intra_cu8_mode28_en   : 1   ; /* [28]  */
		unsigned int    intra_cu8_mode29_en   : 1   ; /* [29]  */
		unsigned int    intra_cu8_mode30_en   : 1   ; /* [30]  */
		unsigned int    intra_cu8_mode31_en   : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL8_ANG_0EN;

/* Define the union U_VEDU_INTRA_CHNL8_ANG_1EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu8_mode32_en   : 1   ; /* [0]  */
		unsigned int    intra_cu8_mode33_en   : 1   ; /* [1]  */
		unsigned int    intra_cu8_mode34_en   : 1   ; /* [2]  */
		unsigned int    Reserved_425    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL8_ANG_1EN;

/* Define the union U_VEDU_INTRA_CHNL16_ANG_0EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu16_mode0_en   : 1   ; /* [0]  */
		unsigned int    intra_cu16_mode1_en   : 1   ; /* [1]  */
		unsigned int    intra_cu16_mode2_en   : 1   ; /* [2]  */
		unsigned int    intra_cu16_mode3_en   : 1   ; /* [3]  */
		unsigned int    intra_cu16_mode4_en   : 1   ; /* [4]  */
		unsigned int    intra_cu16_mode5_en   : 1   ; /* [5]  */
		unsigned int    intra_cu16_mode6_en   : 1   ; /* [6]  */
		unsigned int    intra_cu16_mode7_en   : 1   ; /* [7]  */
		unsigned int    intra_cu16_mode8_en   : 1   ; /* [8]  */
		unsigned int    intra_cu16_mode9_en   : 1   ; /* [9]  */
		unsigned int    intra_cu16_mode10_en  : 1   ; /* [10]  */
		unsigned int    intra_cu16_mode11_en  : 1   ; /* [11]  */
		unsigned int    intra_cu16_mode12_en  : 1   ; /* [12]  */
		unsigned int    intra_cu16_mode13_en  : 1   ; /* [13]  */
		unsigned int    intra_cu16_mode14_en  : 1   ; /* [14]  */
		unsigned int    intra_cu16_mode15_en  : 1   ; /* [15]  */
		unsigned int    intra_cu16_mode16_en  : 1   ; /* [16]  */
		unsigned int    intra_cu16_mode17_en  : 1   ; /* [17]  */
		unsigned int    intra_cu16_mode18_en  : 1   ; /* [18]  */
		unsigned int    intra_cu16_mode19_en  : 1   ; /* [19]  */
		unsigned int    intra_cu16_mode20_en  : 1   ; /* [20]  */
		unsigned int    intra_cu16_mode21_en  : 1   ; /* [21]  */
		unsigned int    intra_cu16_mode22_en  : 1   ; /* [22]  */
		unsigned int    intra_cu16_mode23_en  : 1   ; /* [23]  */
		unsigned int    intra_cu16_mode24_en  : 1   ; /* [24]  */
		unsigned int    intra_cu16_mode25_en  : 1   ; /* [25]  */
		unsigned int    intra_cu16_mode26_en  : 1   ; /* [26]  */
		unsigned int    intra_cu16_mode27_en  : 1   ; /* [27]  */
		unsigned int    intra_cu16_mode28_en  : 1   ; /* [28]  */
		unsigned int    intra_cu16_mode29_en  : 1   ; /* [29]  */
		unsigned int    intra_cu16_mode30_en  : 1   ; /* [30]  */
		unsigned int    intra_cu16_mode31_en  : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL16_ANG_0EN;

/* Define the union U_VEDU_INTRA_CHNL16_ANG_1EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu16_mode32_en  : 1   ; /* [0]  */
		unsigned int    intra_cu16_mode33_en  : 1   ; /* [1]  */
		unsigned int    intra_cu16_mode34_en  : 1   ; /* [2]  */
		unsigned int    Reserved_426    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL16_ANG_1EN;

/* Define the union U_VEDU_INTRA_CHNL32_ANG_0EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu32_mode0_en   : 1   ; /* [0]  */
		unsigned int    intra_cu32_mode1_en   : 1   ; /* [1]  */
		unsigned int    intra_cu32_mode2_en   : 1   ; /* [2]  */
		unsigned int    intra_cu32_mode3_en   : 1   ; /* [3]  */
		unsigned int    intra_cu32_mode4_en   : 1   ; /* [4]  */
		unsigned int    intra_cu32_mode5_en   : 1   ; /* [5]  */
		unsigned int    intra_cu32_mode6_en   : 1   ; /* [6]  */
		unsigned int    intra_cu32_mode7_en   : 1   ; /* [7]  */
		unsigned int    intra_cu32_mode8_en   : 1   ; /* [8]  */
		unsigned int    intra_cu32_mode9_en   : 1   ; /* [9]  */
		unsigned int    intra_cu32_mode10_en  : 1   ; /* [10]  */
		unsigned int    intra_cu32_mode11_en  : 1   ; /* [11]  */
		unsigned int    intra_cu32_mode12_en  : 1   ; /* [12]  */
		unsigned int    intra_cu32_mode13_en  : 1   ; /* [13]  */
		unsigned int    intra_cu32_mode14_en  : 1   ; /* [14]  */
		unsigned int    intra_cu32_mode15_en  : 1   ; /* [15]  */
		unsigned int    intra_cu32_mode16_en  : 1   ; /* [16]  */
		unsigned int    intra_cu32_mode17_en  : 1   ; /* [17]  */
		unsigned int    intra_cu32_mode18_en  : 1   ; /* [18]  */
		unsigned int    intra_cu32_mode19_en  : 1   ; /* [19]  */
		unsigned int    intra_cu32_mode20_en  : 1   ; /* [20]  */
		unsigned int    intra_cu32_mode21_en  : 1   ; /* [21]  */
		unsigned int    intra_cu32_mode22_en  : 1   ; /* [22]  */
		unsigned int    intra_cu32_mode23_en  : 1   ; /* [23]  */
		unsigned int    intra_cu32_mode24_en  : 1   ; /* [24]  */
		unsigned int    intra_cu32_mode25_en  : 1   ; /* [25]  */
		unsigned int    intra_cu32_mode26_en  : 1   ; /* [26]  */
		unsigned int    intra_cu32_mode27_en  : 1   ; /* [27]  */
		unsigned int    intra_cu32_mode28_en  : 1   ; /* [28]  */
		unsigned int    intra_cu32_mode29_en  : 1   ; /* [29]  */
		unsigned int    intra_cu32_mode30_en  : 1   ; /* [30]  */
		unsigned int    intra_cu32_mode31_en  : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL32_ANG_0EN;

/* Define the union U_VEDU_INTRA_CHNL32_ANG_1EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_cu32_mode32_en  : 1   ; /* [0]  */
		unsigned int    intra_cu32_mode33_en  : 1   ; /* [1]  */
		unsigned int    intra_cu32_mode34_en  : 1   ; /* [2]  */
		unsigned int    Reserved_427    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_CHNL32_ANG_1EN;

/* Define the union U_VEDU_INTRA_RDO_COST_OFFSET_3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_h264_rdo_cost_offset  : 16  ; /* [15..0]  */
		unsigned int    Reserved_428    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_INTRA_RDO_COST_OFFSET_3;

/* Define the union U_VEDU_PMV_TMV_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pmv_tmv_en    : 1   ; /* [0]  */
		unsigned int    Reserved_430    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PMV_TMV_EN;

/* Define the union U_VEDU_TQITQ_DEADZONE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    tqitq_deadzone_intra_slice  : 8   ; /* [7..0]  */
		unsigned int    tqitq_deadzone_inter_slice  : 8   ; /* [15..8]  */
		unsigned int    Reserved_432    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_TQITQ_DEADZONE;

/* Define the union U_VEDU_SEL_OFFSET_STRENGTH */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sel_offset_strength   : 3   ; /* [2..0]  */
		unsigned int    Reserved_435    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_SEL_OFFSET_STRENGTH;

/* Define the union U_VEDU_SEL_CU32_DC_AC_TH_OFFSET */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sel_cu32_dc_ac_th_offset  : 2   ; /* [1..0]  */
		unsigned int    Reserved_436    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_SEL_CU32_DC_AC_TH_OFFSET;

/* Define the union U_VEDU_SEL_CU32_QP_TH */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sel_cu32_qp0_th    : 6   ; /* [5..0]  */
		unsigned int    Reserved_438    : 2   ; /* [7..6]  */
		unsigned int    sel_cu32_qp1_th    : 6   ; /* [13..8]  */
		unsigned int    Reserved_437    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_SEL_CU32_QP_TH;

/* Define the union U_VEDU_SEL_RES_DC_AC_TH */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sel_res16_luma_dc_th  : 4   ; /* [3..0]  */
		unsigned int    sel_res16_chroma_dc_th  : 4   ; /* [7..4]  */
		unsigned int    sel_res16_luma_ac_th  : 4   ; /* [11..8]  */
		unsigned int    sel_res16_chroma_ac_th  : 4   ; /* [15..12]  */
		unsigned int    sel_res32_luma_dc_th  : 4   ; /* [19..16]  */
		unsigned int    sel_res32_chroma_dc_th  : 4   ; /* [23..20]  */
		unsigned int    sel_res32_luma_ac_th  : 4   ; /* [27..24]  */
		unsigned int    sel_res32_chroma_ac_th  : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_SEL_RES_DC_AC_TH;

/* Define the union U_VEDU_EMAR_WAIT_TIM_OUT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_wtmax    : 10  ; /* [9..0]  */
		unsigned int    vcpi_rtmax    : 10  ; /* [19..10]  */
		unsigned int    Reserved_442    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_WAIT_TIM_OUT;

/* Define the union U_VEDU_EMAR_RCH_RPT_TH0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ch00_rrmax    : 5   ; /* [4..0]  */
		unsigned int    vcpi_ch01_rrmax    : 5   ; /* [9..5]  */
		unsigned int    vcpi_ch02_rrmax    : 5   ; /* [14..10]  */
		unsigned int    vcpi_ch03_rrmax    : 5   ; /* [19..15]  */
		unsigned int    vcpi_ch04_rrmax    : 5   ; /* [24..20]  */
		unsigned int    vcpi_ch05_rrmax    : 5   ; /* [29..25]  */
		unsigned int    Reserved_443    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_RCH_RPT_TH0;

/* Define the union U_VEDU_EMAR_RCH_RPT_TH1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ch06_rrmax    : 5   ; /* [4..0]  */
		unsigned int    vcpi_ch07_rrmax    : 5   ; /* [9..5]  */
		unsigned int    vcpi_ch08_rrmax    : 5   ; /* [14..10]  */
		unsigned int    vcpi_ch09_rrmax    : 5   ; /* [19..15]  */
		unsigned int    vcpi_ch10_rrmax    : 5   ; /* [24..20]  */
		unsigned int    vcpi_ch11_rrmax    : 5   ; /* [29..25]  */
		unsigned int    Reserved_444    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_RCH_RPT_TH1;

/* Define the union U_VEDU_EMAR_RCH_RPT_TH2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ch12_rrmax    : 5   ; /* [4..0]  */
		unsigned int    Reserved_445    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_RCH_RPT_TH2;

/* Define the union U_VEDU_EMAR_WCH_RPT_TH0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ch00_wrmax    : 5   ; /* [4..0]  */
		unsigned int    vcpi_ch01_wrmax    : 5   ; /* [9..5]  */
		unsigned int    vcpi_ch02_wrmax    : 5   ; /* [14..10]  */
		unsigned int    vcpi_ch03_wrmax    : 5   ; /* [19..15]  */
		unsigned int    vcpi_ch04_wrmax    : 5   ; /* [24..20]  */
		unsigned int    vcpi_ch05_wrmax    : 5   ; /* [29..25]  */
		unsigned int    Reserved_446    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_WCH_RPT_TH0;

/* Define the union U_VEDU_EMAR_WCH_RPT_TH1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ch06_wrmax    : 5   ; /* [4..0]  */
		unsigned int    vcpi_ch07_wrmax    : 5   ; /* [9..5]  */
		unsigned int    vcpi_ch08_wrmax    : 5   ; /* [14..10]  */
		unsigned int    vcpi_ch09_wrmax    : 5   ; /* [19..15]  */
		unsigned int    vcpi_ch10_wrmax    : 5   ; /* [24..20]  */
		unsigned int    vcpi_ch11_wrmax    : 5   ; /* [29..25]  */
		unsigned int    Reserved_447    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_WCH_RPT_TH1;

/* Define the union U_VEDU_EMAR_WCH_RPT_TH2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_ch12_wrmax    : 5   ; /* [4..0]  */
		unsigned int    vcpi_ch13_wrmax    : 5   ; /* [9..5]  */
		unsigned int    vcpi_ch14_wrmax    : 5   ; /* [14..10]  */
		unsigned int    Reserved_448    : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_WCH_RPT_TH2;

/* Define the union U_VEDU_EMAR_SCRAMBLE_TYPE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    emar_rec_b7_scramble  : 1   ; /* [0]  */
		unsigned int    emar_rec_b8_scramble  : 2   ; /* [2..1]  */
		unsigned int    emar_ori_y_b7_scramble  : 1   ; /* [3]  */
		unsigned int    emar_ori_y_b8_scramble  : 2   ; /* [5..4]  */
		unsigned int    emar_ori_uv_b7_scramble  : 1   ; /* [6]  */
		unsigned int    Reserved_449    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_EMAR_SCRAMBLE_TYPE;

/* Define the union U_VEDU_PACK_CU_PARAMETER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pack_vcpi2cu_tq_bypass_enabled_flag  : 1   ; /* [0]  */
		unsigned int    reserved1    : 3   ; /* [3..1]  */
		unsigned int    pack_vcpi2cu_qp_min_cu_size  : 3   ; /* [6..4]  */
		unsigned int    Reserved_451    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PACK_CU_PARAMETER;

/* Define the union U_VEDU_PACK_PCM_PARAMETER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pack_vcpi2pu_log2_min_ipcm_cbsizey  : 3   ; /* [2..0]  */
		unsigned int    Reserved_453    : 1   ; /* [3]  */
		unsigned int    pack_vcpi2pu_log2_max_ipcm_cbsizey  : 3   ; /* [6..4]  */
		unsigned int    Reserved_452    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PACK_PCM_PARAMETER;

/* Define the union U_VEDU_PACK_TF_SKIP_FLAG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pack_vcpi2res_tf_skip_enabled_flag  : 1   ; /* [0]  */
		unsigned int    Reserved_454    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PACK_TF_SKIP_FLAG;

/* Define the union U_VEDU_CABAC_GLB_CFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_457    : 8   ; /* [7..0]  */
		unsigned int    cabac_max_num_mergecand  : 3   ; /* [10..8]  */
		unsigned int    Reserved_456    : 5   ; /* [15..11]  */
		unsigned int    cabac_nal_unit_head   : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CABAC_GLB_CFG;

/* Define the union U_VEDU_CABAC_SLCHDR_SIZE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cabac_slchdr_size_part1  : 5   ; /* [4..0]  */
		unsigned int    Reserved_458    : 11  ; /* [15..5]  */
		unsigned int    cabac_slchdr_size_part2  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CABAC_SLCHDR_SIZE;

/* Define the union U_VEDU_CABAC_SLCHDR_PART1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cabac_slchdr_part1    : 16  ; /* [15..0]  */
		unsigned int    Reserved_459    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CABAC_SLCHDR_PART1;

/* Define the union U_VEDU_CABAC_SLCHDR_SIZE_I */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cabac_slchdr_size_part1_i  : 5   ; /* [4..0]  */
		unsigned int    Reserved_460    : 11  ; /* [15..5]  */
		unsigned int    cabac_slchdr_size_part2_i  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CABAC_SLCHDR_SIZE_I;

/* Define the union U_VEDU_CABAC_SLCHDR_PART1_I */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cabac_slchdr_part1_i  : 16  ; /* [15..0]  */
		unsigned int    Reserved_461    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_CABAC_SLCHDR_PART1_I;

/* Define the union U_VEDU_VLC_SLCHDRPARA */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vlc_markingbit    : 6   ; /* [5..0]  */
		unsigned int    reserved1    : 2   ; /* [7..6]  */
		unsigned int    vlc_reorderbit    : 6   ; /* [13..8]  */
		unsigned int    reserved0    : 2   ; /* [15..14]  */
		unsigned int    vlc_parabit    : 7   ; /* [22..16]  */
		unsigned int    Reserved_463    : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VLC_SLCHDRPARA;

/* Define the union U_VEDU_VLC_SVC */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vlc_svc_en    : 1   ; /* [0]  */
		unsigned int    vlc_svc_strm    : 24  ; /* [24..1]  */
		unsigned int    Reserved_464    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VLC_SVC;

/* Define the union U_VEDU_VLC_SLCHDRPARA_I */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vlc_markingbit_i    : 6   ; /* [5..0]  */
		unsigned int    reserved1    : 2   ; /* [7..6]  */
		unsigned int    vlc_reorderbit_i    : 6   ; /* [13..8]  */
		unsigned int    reserved0    : 2   ; /* [15..14]  */
		unsigned int    vlc_parabit_i    : 7   ; /* [22..16]  */
		unsigned int    Reserved_465    : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VLC_SLCHDRPARA_I;

/* Define the union U_VEDU_VLCST_PTBITS_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vlcst_ptbits_en    : 1   ; /* [0]  */
		unsigned int    Reserved_468    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VLCST_PTBITS_EN;

/* Define the union U_VEDU_VLCST_PARAMETER */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vlcst_para_set_len    : 8   ; /* [7..0]  */
		unsigned int    vlcst_para_sprat_en   : 1   ; /* [8]  */
		unsigned int    Reserved_470    : 7   ; /* [15..9]  */
		unsigned int    vlcst_para_set_en    : 1   ; /* [16]  */
		unsigned int    Reserved_469    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_VLCST_PARAMETER;

/* Define the union U_VEDU_PPFD_ST_CFG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ppfd_st_cmp_en    : 1   ; /* [0]  */
		unsigned int    Reserved_473    : 3   ; /* [3..1]  */
		unsigned int    ppfd_st_bypass_en    : 1   ; /* [4]  */
		unsigned int    Reserved_472    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_PPFD_ST_CFG;

/* Define the union U_VEDU_ENV_CHN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    env_chn_curr_id    : 4   ; /* [3..0]  */
		unsigned int    env_chn_goptype    : 4   ; /* [7..4]  */
		unsigned int    env_chn_bfrm_num    : 8   ; /* [15..8]  */
		unsigned int    Reserved_475    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_VEDU_ENV_CHN;

/* Define the union U_FUNC_VCPI_INTSTAT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_int_ve_eop    : 1   ; /* [0]  */
		unsigned int    vcpi_int_vedu_slice_end  : 1   ; /* [1]  */
		unsigned int    vcpi_int_ve_buffull   : 1   ; /* [2]  */
		unsigned int    vcpi_int_ve_pbitsover  : 1   ; /* [3]  */
		unsigned int    vcpi_int_vedu_brkpt   : 1   ; /* [4]  */
		unsigned int    vcpi_int_vedu_step    : 1   ; /* [5]  */
		unsigned int    vcpi_int_vedu_timeout  : 1   ; /* [6]  */
		unsigned int    vcpi_int_cfg_err    : 1   ; /* [7]  */
		unsigned int    Reserved_477    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VCPI_INTSTAT;

/* Define the union U_FUNC_VCPI_RAWINT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_rint_ve_eop    : 1   ; /* [0]  */
		unsigned int    vcpi_rint_vedu_slice_end  : 1   ; /* [1]  */
		unsigned int    vcpi_rint_ve_buffull  : 1   ; /* [2]  */
		unsigned int    vcpi_rint_ve_pbitsover  : 1   ; /* [3]  */
		unsigned int    vcpi_rint_vedu_brkpt  : 1   ; /* [4]  */
		unsigned int    vcpi_rint_vedu_step   : 1   ; /* [5]  */
		unsigned int    vcpi_rint_vedu_timeout  : 1   ; /* [6]  */
		unsigned int    vcpi_rint_cfg_err    : 1   ; /* [7]  */
		unsigned int    Reserved_478    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VCPI_RAWINT;

/* Define the union U_FUNC_VCPI_INTSTAT_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_int_ve_eop_s    : 1   ; /* [0]  */
		unsigned int    vcpi_int_vedu_slice_end_s  : 1   ; /* [1]  */
		unsigned int    vcpi_int_ve_buffull_s  : 1   ; /* [2]  */
		unsigned int    vcpi_int_ve_pbitsover_s  : 1   ; /* [3]  */
		unsigned int    vcpi_int_vedu_brkpt_s  : 1   ; /* [4]  */
		unsigned int    vcpi_int_vedu_step_s  : 1   ; /* [5]  */
		unsigned int    vcpi_int_vedu_timeout_s  : 1   ; /* [6]  */
		unsigned int    vcpi_int_cfg_err_s    : 1   ; /* [7]  */
		unsigned int    Reserved_479    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VCPI_INTSTAT_S;

/* Define the union U_FUNC_VCPI_RAWINT_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_rint_ve_eop_s    : 1   ; /* [0]  */
		unsigned int    vcpi_rint_vedu_slice_end_s  : 1   ; /* [1]  */
		unsigned int    vcpi_rint_ve_buffull_s  : 1   ; /* [2]  */
		unsigned int    vcpi_rint_ve_pbitsover_s  : 1   ; /* [3]  */
		unsigned int    vcpi_rint_vedu_brkpt_s  : 1   ; /* [4]  */
		unsigned int    vcpi_rint_vedu_step_s  : 1   ; /* [5]  */
		unsigned int    vcpi_rint_vedu_timeout_s  : 1   ; /* [6]  */
		unsigned int    vcpi_rint_cfg_err_s   : 1   ; /* [7]  */
		unsigned int    Reserved_480    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VCPI_RAWINT_S;

/* Define the union U_FUNC_PME_MADI_SUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_madi_sum    : 25  ; /* [24..0]  */
		unsigned int    Reserved_488    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_PME_MADI_SUM;

/* Define the union U_FUNC_PME_MADP_SUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_madp_sum    : 25  ; /* [24..0]  */
		unsigned int    Reserved_489    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_PME_MADP_SUM;

/* Define the union U_FUNC_PME_MADI_NUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_madi_num    : 17  ; /* [16..0]  */
		unsigned int    Reserved_490    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_PME_MADI_NUM;

/* Define the union U_FUNC_PME_MADP_NUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pme_madp_num    : 17  ; /* [16..0]  */
		unsigned int    Reserved_491    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_PME_MADP_NUM;

/* Define the union U_FUNC_BGGEN_BLOCK_COUNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_bggen_block_count  : 18  ; /* [17..0]  */
		unsigned int    Reserved_495    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_BGGEN_BLOCK_COUNT;

/* Define the union U_FUNC_BGGEN_FRAME_BGM_DIST */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    vcpi_frame_bgm_dist   : 31  ; /* [30..0]  */
		unsigned int    Reserved_496    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_BGGEN_FRAME_BGM_DIST;

/* Define the union U_FUNC_VLCST_DSRPTR00 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len0    : 29  ; /* [28..0]  */
		unsigned int    Reserved_520    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR00;

/* Define the union U_FUNC_VLCST_DSRPTR01 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum0    : 7   ; /* [6..0]  */
		unsigned int    Reserved_521    : 24  ; /* [30..7]  */
		unsigned int    islastslc0    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR01;

/* Define the union U_FUNC_VLCST_DSRPTR10 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len1    : 29  ; /* [28..0]  */
		unsigned int    Reserved_522    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR10;

/* Define the union U_FUNC_VLCST_DSRPTR11 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum1    : 7   ; /* [6..0]  */
		unsigned int    Reserved_523    : 24  ; /* [30..7]  */
		unsigned int    islastslc1    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR11;

/* Define the union U_FUNC_VLCST_DSRPTR20 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len2    : 29  ; /* [28..0]  */
		unsigned int    Reserved_524    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR20;

/* Define the union U_FUNC_VLCST_DSRPTR21 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum2    : 7   ; /* [6..0]  */
		unsigned int    Reserved_525    : 24  ; /* [30..7]  */
		unsigned int    islastslc2    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR21;

/* Define the union U_FUNC_VLCST_DSRPTR30 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len3    : 29  ; /* [28..0]  */
		unsigned int    Reserved_526    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR30;

/* Define the union U_FUNC_VLCST_DSRPTR31 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum3    : 7   ; /* [6..0]  */
		unsigned int    Reserved_527    : 24  ; /* [30..7]  */
		unsigned int    islastslc3    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR31;

/* Define the union U_FUNC_VLCST_DSRPTR40 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len4    : 29  ; /* [28..0]  */
		unsigned int    Reserved_528    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR40;

/* Define the union U_FUNC_VLCST_DSRPTR41 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum4    : 7   ; /* [6..0]  */
		unsigned int    Reserved_529    : 24  ; /* [30..7]  */
		unsigned int    islastslc4    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR41;

/* Define the union U_FUNC_VLCST_DSRPTR50 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len5    : 29  ; /* [28..0]  */
		unsigned int    Reserved_530    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR50;

/* Define the union U_FUNC_VLCST_DSRPTR51 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum5    : 7   ; /* [6..0]  */
		unsigned int    Reserved_531    : 24  ; /* [30..7]  */
		unsigned int    islastslc5    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR51;

/* Define the union U_FUNC_VLCST_DSRPTR60 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len6    : 29  ; /* [28..0]  */
		unsigned int    Reserved_532    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR60;

/* Define the union U_FUNC_VLCST_DSRPTR61 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum6    : 7   ; /* [6..0]  */
		unsigned int    Reserved_533    : 24  ; /* [30..7]  */
		unsigned int    islastslc6    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR61;

/* Define the union U_FUNC_VLCST_DSRPTR70 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len7    : 29  ; /* [28..0]  */
		unsigned int    Reserved_534    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR70;

/* Define the union U_FUNC_VLCST_DSRPTR71 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum7    : 7   ; /* [6..0]  */
		unsigned int    Reserved_535    : 24  ; /* [30..7]  */
		unsigned int    islastslc7    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR71;

/* Define the union U_FUNC_VLCST_DSRPTR80 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len8    : 29  ; /* [28..0]  */
		unsigned int    Reserved_536    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR80;

/* Define the union U_FUNC_VLCST_DSRPTR81 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum8    : 7   ; /* [6..0]  */
		unsigned int    Reserved_537    : 24  ; /* [30..7]  */
		unsigned int    islastslc8    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR81;

/* Define the union U_FUNC_VLCST_DSRPTR90 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len9    : 29  ; /* [28..0]  */
		unsigned int    Reserved_538    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR90;

/* Define the union U_FUNC_VLCST_DSRPTR91 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum9    : 7   ; /* [6..0]  */
		unsigned int    Reserved_539    : 24  ; /* [30..7]  */
		unsigned int    islastslc9    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR91;

/* Define the union U_FUNC_VLCST_DSRPTR100 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len10    : 29  ; /* [28..0]  */
		unsigned int    Reserved_540    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR100;

/* Define the union U_FUNC_VLCST_DSRPTR101 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum10    : 7   ; /* [6..0]  */
		unsigned int    Reserved_541    : 24  ; /* [30..7]  */
		unsigned int    islastslc10    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR101;

/* Define the union U_FUNC_VLCST_DSRPTR110 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len11    : 29  ; /* [28..0]  */
		unsigned int    Reserved_542    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR110;

/* Define the union U_FUNC_VLCST_DSRPTR111 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum11    : 7   ; /* [6..0]  */
		unsigned int    Reserved_543    : 24  ; /* [30..7]  */
		unsigned int    islastslc11    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR111;

/* Define the union U_FUNC_VLCST_DSRPTR120 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len12    : 29  ; /* [28..0]  */
		unsigned int    Reserved_544    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR120;

/* Define the union U_FUNC_VLCST_DSRPTR121 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum12    : 7   ; /* [6..0]  */
		unsigned int    Reserved_545    : 24  ; /* [30..7]  */
		unsigned int    islastslc12    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR121;

/* Define the union U_FUNC_VLCST_DSRPTR130 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len13    : 29  ; /* [28..0]  */
		unsigned int    Reserved_546    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR130;

/* Define the union U_FUNC_VLCST_DSRPTR131 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum13    : 7   ; /* [6..0]  */
		unsigned int    Reserved_547    : 24  ; /* [30..7]  */
		unsigned int    islastslc13    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR131;

/* Define the union U_FUNC_VLCST_DSRPTR140 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len14    : 29  ; /* [28..0]  */
		unsigned int    Reserved_548    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR140;

/* Define the union U_FUNC_VLCST_DSRPTR141 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum14    : 7   ; /* [6..0]  */
		unsigned int    Reserved_549    : 24  ; /* [30..7]  */
		unsigned int    islastslc14    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR141;

/* Define the union U_FUNC_VLCST_DSRPTR150 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    slc_len15    : 29  ; /* [28..0]  */
		unsigned int    Reserved_550    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR150;

/* Define the union U_FUNC_VLCST_DSRPTR151 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    invalidnum15    : 7   ; /* [6..0]  */
		unsigned int    Reserved_551    : 24  ; /* [30..7]  */
		unsigned int    islastslc15    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_VLCST_DSRPTR151;

/* Define the union U_FUNC_SEL_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    opt_8x8_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_555    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTRA_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_opt_8x8_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_556    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTRA_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_normal_opt_8x8_cnt  : 19  ; /* [18..0]  */
		unsigned int    Reserved_557    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pcm_opt_8x8_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_558    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTER_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    inter_opt_8x8_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_559    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTER_FME_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_opt_8x8_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_560    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_FME_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTER_MERGE_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_opt_8x8_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_561    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_MERGE_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_skip_opt_8x8_cnt  : 19  ; /* [18..0]  */
		unsigned int    Reserved_562    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;

/* Define the union U_FUNC_SEL_OPT_16X16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    opt_16x16_cnt    : 17  ; /* [16..0]  */
		unsigned int    Reserved_563    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_OPT_16X16_CNT;

/* Define the union U_FUNC_SEL_INTRA_OPT_16X16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_opt_16x16_cnt   : 17  ; /* [16..0]  */
		unsigned int    Reserved_564    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTRA_OPT_16X16_CNT;

/* Define the union U_FUNC_SEL_OPT_4X4_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    opt_4x4_cnt    : 19  ; /* [18..0]  */
		unsigned int    Reserved_565    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_OPT_4X4_CNT;

/* Define the union U_FUNC_SEL_INTER_OPT_16X16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    inter_opt_16x16_cnt   : 17  ; /* [16..0]  */
		unsigned int    Reserved_567    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_OPT_16X16_CNT;

/* Define the union U_FUNC_SEL_INTER_FME_OPT_16X16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_opt_16x16_cnt    : 17  ; /* [16..0]  */
		unsigned int    Reserved_568    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_FME_OPT_16X16_CNT;

/* Define the union U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_opt_16x16_cnt    : 17  ; /* [16..0]  */
		unsigned int    Reserved_569    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;

/* Define the union U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_skip_opt_16x16_cnt  : 17  ; /* [16..0]  */
		unsigned int    Reserved_570    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;

/* Define the union U_FUNC_SEL_OPT_32X32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    opt_32x32_cnt    : 16  ; /* [15..0]  */
		unsigned int    Reserved_571    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_OPT_32X32_CNT;

/* Define the union U_FUNC_SEL_INTRA_OPT_32X32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intra_opt_32x32_cnt   : 16  ; /* [15..0]  */
		unsigned int    Reserved_572    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTRA_OPT_32X32_CNT;

/* Define the union U_FUNC_SEL_INTER_OPT_32X32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    inter_opt_32x32_cnt   : 16  ; /* [15..0]  */
		unsigned int    Reserved_574    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_OPT_32X32_CNT;

/* Define the union U_FUNC_SEL_INTER_FME_OPT_32X32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_opt_32x32_cnt    : 16  ; /* [15..0]  */
		unsigned int    Reserved_575    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_FME_OPT_32X32_CNT;

/* Define the union U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_opt_32x32_cnt    : 16  ; /* [15..0]  */
		unsigned int    Reserved_576    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;

/* Define the union U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_skip_opt_32x32_cnt  : 16  ; /* [15..0]  */
		unsigned int    Reserved_577    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;

/* Define the union U_FUNC_SEL_OPT_64X64_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    opt_64x64_cnt    : 16  ; /* [15..0]  */
		unsigned int    Reserved_578    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_OPT_64X64_CNT;

/* Define the union U_FUNC_SEL_INTER_FME_OPT_64X64_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fme_opt_64x64_cnt    : 16  ; /* [15..0]  */
		unsigned int    Reserved_579    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_FME_OPT_64X64_CNT;

/* Define the union U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_opt_64x64_cnt    : 16  ; /* [15..0]  */
		unsigned int    Reserved_580    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;

/* Define the union U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mrg_skip_opt_64x64_cnt  : 16  ; /* [15..0]  */
		unsigned int    Reserved_581    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;

/* Define the union U_FUNC_SEL_TOTAL_LUMA_QP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    total_luma_qp    : 26  ; /* [25..0]  */
		unsigned int    Reserved_582    : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_TOTAL_LUMA_QP;

/* Define the union U_FUNC_SEL_MAX_MIN_LUMA_QP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    min_luma_qp    : 6   ; /* [5..0]  */
		unsigned int    Reserved_584    : 2   ; /* [7..6]  */
		unsigned int    max_luma_qp    : 6   ; /* [13..8]  */
		unsigned int    Reserved_583    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_MAX_MIN_LUMA_QP;

/* Define the union U_FUNC_SEL_LUMA_QP0_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp0_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_586    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP0_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP1_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp1_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_587    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP1_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP2_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp2_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_588    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP2_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP3_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp3_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_589    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP3_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP4_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp4_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_590    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP4_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP5_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp5_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_591    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP5_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP6_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp6_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_592    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP6_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP7_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp7_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_593    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP7_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP8_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp8_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_594    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP8_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP9_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp9_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_595    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP9_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP10_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp10_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_596    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP10_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP11_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp11_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_597    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP11_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP12_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp12_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_598    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP12_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP13_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp13_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_599    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP13_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP14_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp14_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_600    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP14_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP15_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp15_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_601    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP15_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP16_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp16_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_602    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP16_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP17_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp17_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_603    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP17_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP18_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp18_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_604    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP18_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP19_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp19_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_605    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP19_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP20_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp20_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_606    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP20_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP21_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp21_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_607    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP21_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP22_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp22_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_608    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP22_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP23_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp23_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_609    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP23_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP24_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp24_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_610    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP24_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP25_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp25_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_611    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP25_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP26_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp26_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_612    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP26_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP27_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp27_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_613    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP27_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP28_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp28_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_614    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP28_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP29_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp29_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_615    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP29_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP30_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp30_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_616    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP30_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP31_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp31_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_617    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP31_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP32_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp32_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_618    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP32_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP33_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp33_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_619    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP33_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP34_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp34_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_620    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP34_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP35_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp35_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_621    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP35_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP36_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp36_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_622    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP36_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP37_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp37_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_623    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP37_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP38_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp38_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_624    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP38_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP39_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp39_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_625    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP39_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP40_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp40_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_626    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP40_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP41_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp41_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_627    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP41_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP42_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp42_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_628    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP42_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP43_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp43_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_629    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP43_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP44_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp44_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_630    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP44_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP45_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp45_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_631    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP45_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP46_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp46_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_632    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP46_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP47_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp47_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_633    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP47_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP48_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp48_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_634    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP48_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP49_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp49_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_635    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP49_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP50_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp50_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_636    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP50_CNT;

/* Define the union U_FUNC_SEL_LUMA_QP51_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    luma_qp51_cnt    : 21  ; /* [20..0]  */
		unsigned int    Reserved_637    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SEL_LUMA_QP51_CNT;

/* Define the union U_FUNC_SAO_MSE_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sao2vcpi_mse_cnt    : 17  ; /* [16..0]  */
		unsigned int    Reserved_639    : 15  ; /* [31..17]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SAO_MSE_CNT;

/* Define the union U_FUNC_SAO_MSE_MAX */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sao2vcpi_mse_max    : 28  ; /* [27..0]  */
		unsigned int    Reserved_640    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SAO_MSE_MAX;

/* Define the union U_FUNC_SAO_OFF_NUM */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    saooff_luma_num    : 14  ; /* [13..0]  */
		unsigned int    Reserved_643    : 2   ; /* [15..14]  */
		unsigned int    saooff_chroma_num    : 14  ; /* [29..16]  */
		unsigned int    Reserved_642    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SAO_OFF_NUM;

/* Define the union U_FUNC_SAO_LCU_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    sao_lcu_cnt    : 18  ; /* [17..0]  */
		unsigned int    Reserved_644    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_FUNC_SAO_LCU_CNT;

/* Define the union U_MMU_PRE_GLB_SCR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    glb_scr    : 1   ; /* [0]  */
		unsigned int    Reserved_647    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_MMU_PRE_GLB_SCR;

/* Define the union U_MMU_PRE_DFX_ARERR_FLAG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arerr_ar_flag    : 1   ; /* [0]  */
		unsigned int    Reserved_650    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_MMU_PRE_DFX_ARERR_FLAG;

/* Define the union U_MMU_PRE_DFX_ARERR_ID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_ar_id    : 5   ; /* [4..0]  */
		unsigned int    Reserved_651    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_MMU_PRE_DFX_ARERR_ID;

/* Define the union U_MMU_PRE_DFX_AWERR_FLAG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arerr_aw_flag    : 1   ; /* [0]  */
		unsigned int    Reserved_653    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_MMU_PRE_DFX_AWERR_FLAG;

/* Define the union U_MMU_PRE_DFX_AWERR_ID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_aw_id    : 5   ; /* [4..0]  */
		unsigned int    Reserved_654    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_MMU_PRE_DFX_AWERR_ID;

/* Define the union U_AXIDFX_ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_ar_r_err    : 1   ; /* [0]  */
		unsigned int    dfx_aw_w_err    : 1   ; /* [1]  */
		unsigned int    dfx_aw_b_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid_err    : 1   ; /* [3]  */
		unsigned int    dfx_rid_err    : 1   ; /* [4]  */
		unsigned int    dfx_awid_err    : 1   ; /* [5]  */
		unsigned int    dfx_wid_err    : 1   ; /* [6]  */
		unsigned int    dfx_bid_err    : 1   ; /* [7]  */
		unsigned int    dfx_arid_tx_err    : 1   ; /* [8]  */
		unsigned int    dfx_rid_rx_err    : 1   ; /* [9]  */
		unsigned int    dfx_awid_tx_err    : 1   ; /* [10]  */
		unsigned int    dfx_bid_rx_err    : 1   ; /* [11]  */
		unsigned int    dfx_arid_len_err    : 1   ; /* [12]  */
		unsigned int    dfx_awid_len    : 1   ; /* [13]  */
		unsigned int    dfx_rresp_err    : 1   ; /* [14]  */
		unsigned int    dfx_bresp_err    : 1   ; /* [15]  */
		unsigned int    dfx_ar_ovr_err    : 1   ; /* [16]  */
		unsigned int    dfx_r_ovr_err    : 1   ; /* [17]  */
		unsigned int    dfx_aw_ovr_err    : 1   ; /* [18]  */
		unsigned int    dfx_w_ovr_err    : 1   ; /* [19]  */
		unsigned int    dfx_b_ovr_err    : 1   ; /* [20]  */
		unsigned int    dfx_ar_outstanding_err  : 1   ; /* [21]  */
		unsigned int    dfx_aw_outstanding_err  : 1   ; /* [22]  */
		unsigned int    dfx_arlen_err    : 1   ; /* [23]  */
		unsigned int    dfx_awlen_err    : 1   ; /* [24]  */
		unsigned int    Reserved_656    : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR;

/* Define the union U_AXIDFX_AR_R_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ar_r_cnt    : 7   ; /* [6..0]  */
		unsigned int    Reserved_658    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_AR_R_CNT;

/* Define the union U_AXIDFX_AW_W_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    aw_w_cnt    : 6   ; /* [5..0]  */
		unsigned int    Reserved_659    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_AW_W_CNT;

/* Define the union U_AXIDFX_AW_B_CNT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    aw_b_cnt    : 6   ; /* [5..0]  */
		unsigned int    Reserved_660    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_AW_B_CNT;

/* Define the union U_AXIDFX_AR_R_ID_ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid_err    : 1   ; /* [0]  */
		unsigned int    Reserved_663    : 3   ; /* [3..1]  */
		unsigned int    dfx_rid_err    : 1   ; /* [4]  */
		unsigned int    Reserved_662    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_AR_R_ID_ERR;

/* Define the union U_AXIDFX_ERR_ARID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_arid    : 7   ; /* [6..0]  */
		unsigned int    Reserved_664    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_ARID;

/* Define the union U_AXIDFX_ERR_RID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_rid    : 7   ; /* [6..0]  */
		unsigned int    Reserved_665    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_RID;

/* Define the union U_AXIDFX_AW_W_B_ID_ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_awid_err    : 1   ; /* [0]  */
		unsigned int    Reserved_669    : 3   ; /* [3..1]  */
		unsigned int    dfx_wid_err    : 1   ; /* [4]  */
		unsigned int    Reserved_668    : 3   ; /* [7..5]  */
		unsigned int    dfx_bid_err    : 1   ; /* [8]  */
		unsigned int    Reserved_667    : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_AW_W_B_ID_ERR;

/* Define the union U_AXIDFX_ERR_AWID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_awid    : 6   ; /* [5..0]  */
		unsigned int    Reserved_670    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_AWID;

/* Define the union U_AXIDFX_ERR_WID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_wid    : 6   ; /* [5..0]  */
		unsigned int    Reserved_671    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_WID;

/* Define the union U_AXIDFX_ERR_BID */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_bid    : 6   ; /* [5..0]  */
		unsigned int    Reserved_672    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_BID;

/* Define the union U_AXIDFX_ARID_TX_0ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid0_tx_err    : 1   ; /* [0]  */
		unsigned int    dfx_arid1_tx_err    : 1   ; /* [1]  */
		unsigned int    dfx_arid2_tx_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid3_tx_err    : 1   ; /* [3]  */
		unsigned int    dfx_arid4_tx_err    : 1   ; /* [4]  */
		unsigned int    dfx_arid5_tx_err    : 1   ; /* [5]  */
		unsigned int    dfx_arid6_tx_err    : 1   ; /* [6]  */
		unsigned int    dfx_arid7_tx_err    : 1   ; /* [7]  */
		unsigned int    dfx_arid8_tx_err    : 1   ; /* [8]  */
		unsigned int    dfx_arid9_tx_err    : 1   ; /* [9]  */
		unsigned int    dfx_arid10_tx_err    : 1   ; /* [10]  */
		unsigned int    dfx_arid11_tx_err    : 1   ; /* [11]  */
		unsigned int    dfx_arid12_tx_err    : 1   ; /* [12]  */
		unsigned int    dfx_arid13_tx_err    : 1   ; /* [13]  */
		unsigned int    dfx_arid14_tx_err    : 1   ; /* [14]  */
		unsigned int    dfx_arid15_tx_err    : 1   ; /* [15]  */
		unsigned int    dfx_arid16_tx_err    : 1   ; /* [16]  */
		unsigned int    dfx_arid17_tx_err    : 1   ; /* [17]  */
		unsigned int    dfx_arid18_tx_err    : 1   ; /* [18]  */
		unsigned int    dfx_arid19_tx_err    : 1   ; /* [19]  */
		unsigned int    dfx_arid20_tx_err    : 1   ; /* [20]  */
		unsigned int    dfx_arid21_tx_err    : 1   ; /* [21]  */
		unsigned int    dfx_arid22_tx_err    : 1   ; /* [22]  */
		unsigned int    dfx_arid23_tx_err    : 1   ; /* [23]  */
		unsigned int    dfx_arid24_tx_err    : 1   ; /* [24]  */
		unsigned int    dfx_arid25_tx_err    : 1   ; /* [25]  */
		unsigned int    dfx_arid26_tx_err    : 1   ; /* [26]  */
		unsigned int    dfx_arid27_tx_err    : 1   ; /* [27]  */
		unsigned int    dfx_arid28_tx_err    : 1   ; /* [28]  */
		unsigned int    dfx_arid29_tx_err    : 1   ; /* [29]  */
		unsigned int    dfx_arid30_tx_err    : 1   ; /* [30]  */
		unsigned int    dfx_arid31_tx_err    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_TX_0ERR;

/* Define the union U_AXIDFX_ARID_TX_1ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid32_tx_err    : 1   ; /* [0]  */
		unsigned int    dfx_arid33_tx_err    : 1   ; /* [1]  */
		unsigned int    dfx_arid34_tx_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid35_tx_err    : 1   ; /* [3]  */
		unsigned int    dfx_arid36_tx_err    : 1   ; /* [4]  */
		unsigned int    dfx_arid37_tx_err    : 1   ; /* [5]  */
		unsigned int    dfx_arid38_tx_err    : 1   ; /* [6]  */
		unsigned int    dfx_arid39_tx_err    : 1   ; /* [7]  */
		unsigned int    dfx_arid40_tx_err    : 1   ; /* [8]  */
		unsigned int    dfx_arid41_tx_err    : 1   ; /* [9]  */
		unsigned int    dfx_arid42_tx_err    : 1   ; /* [10]  */
		unsigned int    dfx_arid43_tx_err    : 1   ; /* [11]  */
		unsigned int    dfx_arid44_tx_err    : 1   ; /* [12]  */
		unsigned int    dfx_arid45_tx_err    : 1   ; /* [13]  */
		unsigned int    dfx_arid46_tx_err    : 1   ; /* [14]  */
		unsigned int    dfx_arid47_tx_err    : 1   ; /* [15]  */
		unsigned int    dfx_arid48_tx_err    : 1   ; /* [16]  */
		unsigned int    dfx_arid49_tx_err    : 1   ; /* [17]  */
		unsigned int    dfx_arid50_tx_err    : 1   ; /* [18]  */
		unsigned int    dfx_arid51_tx_err    : 1   ; /* [19]  */
		unsigned int    dfx_arid52_tx_err    : 1   ; /* [20]  */
		unsigned int    dfx_arid53_tx_err    : 1   ; /* [21]  */
		unsigned int    dfx_arid54_tx_err    : 1   ; /* [22]  */
		unsigned int    dfx_arid55_tx_err    : 1   ; /* [23]  */
		unsigned int    dfx_arid56_tx_err    : 1   ; /* [24]  */
		unsigned int    dfx_arid57_tx_err    : 1   ; /* [25]  */
		unsigned int    dfx_arid58_tx_err    : 1   ; /* [26]  */
		unsigned int    dfx_arid59_tx_err    : 1   ; /* [27]  */
		unsigned int    dfx_arid60_tx_err    : 1   ; /* [28]  */
		unsigned int    dfx_arid61_tx_err    : 1   ; /* [29]  */
		unsigned int    dfx_arid62_tx_err    : 1   ; /* [30]  */
		unsigned int    dfx_arid63_tx_err    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_TX_1ERR;

/* Define the union U_AXIDFX_ARID_TX_2ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid64_tx_err    : 1   ; /* [0]  */
		unsigned int    dfx_arid65_tx_err    : 1   ; /* [1]  */
		unsigned int    dfx_arid66_tx_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid67_tx_err    : 1   ; /* [3]  */
		unsigned int    Reserved_673    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_TX_2ERR;

/* Define the union U_AXIDFX_RID_RX_0ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_rid0_rx_err    : 1   ; /* [0]  */
		unsigned int    dfx_rid1_rx_err    : 1   ; /* [1]  */
		unsigned int    dfx_rid2_rx_err    : 1   ; /* [2]  */
		unsigned int    dfx_rid3_rx_err    : 1   ; /* [3]  */
		unsigned int    dfx_rid4_rx_err    : 1   ; /* [4]  */
		unsigned int    dfx_rid5_rx_err    : 1   ; /* [5]  */
		unsigned int    dfx_rid6_rx_err    : 1   ; /* [6]  */
		unsigned int    dfx_rid7_rx_err    : 1   ; /* [7]  */
		unsigned int    dfx_rid8_rx_err    : 1   ; /* [8]  */
		unsigned int    dfx_rid9_rx_err    : 1   ; /* [9]  */
		unsigned int    dfx_rid10_rx_err    : 1   ; /* [10]  */
		unsigned int    dfx_rid11_rx_err    : 1   ; /* [11]  */
		unsigned int    dfx_rid12_rx_err    : 1   ; /* [12]  */
		unsigned int    dfx_rid13_rx_err    : 1   ; /* [13]  */
		unsigned int    dfx_rid14_rx_err    : 1   ; /* [14]  */
		unsigned int    dfx_rid15_rx_err    : 1   ; /* [15]  */
		unsigned int    dfx_rid16_rx_err    : 1   ; /* [16]  */
		unsigned int    dfx_rid17_rx_err    : 1   ; /* [17]  */
		unsigned int    dfx_rid18_rx_err    : 1   ; /* [18]  */
		unsigned int    dfx_rid19_rx_err    : 1   ; /* [19]  */
		unsigned int    dfx_rid20_rx_err    : 1   ; /* [20]  */
		unsigned int    dfx_rid21_rx_err    : 1   ; /* [21]  */
		unsigned int    dfx_rid22_rx_err    : 1   ; /* [22]  */
		unsigned int    dfx_rid23_rx_err    : 1   ; /* [23]  */
		unsigned int    dfx_rid24_rx_err    : 1   ; /* [24]  */
		unsigned int    dfx_rid25_rx_err    : 1   ; /* [25]  */
		unsigned int    dfx_rid26_rx_err    : 1   ; /* [26]  */
		unsigned int    dfx_rid27_rx_err    : 1   ; /* [27]  */
		unsigned int    dfx_rid28_rx_err    : 1   ; /* [28]  */
		unsigned int    dfx_rid29_rx_err    : 1   ; /* [29]  */
		unsigned int    dfx_rid30_rx_err    : 1   ; /* [30]  */
		unsigned int    dfx_rid31_rx_err    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_RID_RX_0ERR;

/* Define the union U_AXIDFX_RID_RX_1ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_rid32_rx_err    : 1   ; /* [0]  */
		unsigned int    dfx_rid33_rx_err    : 1   ; /* [1]  */
		unsigned int    dfx_rid34_rx_err    : 1   ; /* [2]  */
		unsigned int    dfx_rid35_rx_err    : 1   ; /* [3]  */
		unsigned int    dfx_rid36_rx_err    : 1   ; /* [4]  */
		unsigned int    dfx_rid37_rx_err    : 1   ; /* [5]  */
		unsigned int    dfx_rid38_rx_err    : 1   ; /* [6]  */
		unsigned int    dfx_rid39_rx_err    : 1   ; /* [7]  */
		unsigned int    dfx_rid40_rx_err    : 1   ; /* [8]  */
		unsigned int    dfx_rid41_rx_err    : 1   ; /* [9]  */
		unsigned int    dfx_rid42_rx_err    : 1   ; /* [10]  */
		unsigned int    dfx_rid43_rx_err    : 1   ; /* [11]  */
		unsigned int    dfx_rid44_rx_err    : 1   ; /* [12]  */
		unsigned int    dfx_rid45_rx_err    : 1   ; /* [13]  */
		unsigned int    dfx_rid46_rx_err    : 1   ; /* [14]  */
		unsigned int    dfx_rid47_rx_err    : 1   ; /* [15]  */
		unsigned int    dfx_rid48_rx_err    : 1   ; /* [16]  */
		unsigned int    dfx_rid49_rx_err    : 1   ; /* [17]  */
		unsigned int    dfx_rid50_rx_err    : 1   ; /* [18]  */
		unsigned int    dfx_rid51_rx_err    : 1   ; /* [19]  */
		unsigned int    dfx_rid52_rx_err    : 1   ; /* [20]  */
		unsigned int    dfx_rid53_rx_err    : 1   ; /* [21]  */
		unsigned int    dfx_rid54_rx_err    : 1   ; /* [22]  */
		unsigned int    dfx_rid55_rx_err    : 1   ; /* [23]  */
		unsigned int    dfx_rid56_rx_err    : 1   ; /* [24]  */
		unsigned int    dfx_rid57_rx_err    : 1   ; /* [25]  */
		unsigned int    dfx_rid58_rx_err    : 1   ; /* [26]  */
		unsigned int    dfx_rid59_rx_err    : 1   ; /* [27]  */
		unsigned int    dfx_rid60_rx_err    : 1   ; /* [28]  */
		unsigned int    dfx_rid61_rx_err    : 1   ; /* [29]  */
		unsigned int    dfx_rid62_rx_err    : 1   ; /* [30]  */
		unsigned int    dfx_rid63_rx_err    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_RID_RX_1ERR;

/* Define the union U_AXIDFX_RID_RX_2ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_rid64_rx_err    : 1   ; /* [0]  */
		unsigned int    dfx_rid65_rx_err    : 1   ; /* [1]  */
		unsigned int    dfx_rid66_rx_err    : 1   ; /* [2]  */
		unsigned int    dfx_rid67_rx_err    : 1   ; /* [3]  */
		unsigned int    Reserved_675    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_RID_RX_2ERR;

/* Define the union U_AXIDFX_ARID_RX_0ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_awid0_rx_err    : 1   ; /* [0]  */
		unsigned int    dfx_awid1_rx_err    : 1   ; /* [1]  */
		unsigned int    dfx_awid2_rx_err    : 1   ; /* [2]  */
		unsigned int    dfx_awid3_rx_err    : 1   ; /* [3]  */
		unsigned int    dfx_awid4_rx_err    : 1   ; /* [4]  */
		unsigned int    dfx_awid5_rx_err    : 1   ; /* [5]  */
		unsigned int    dfx_awid6_rx_err    : 1   ; /* [6]  */
		unsigned int    dfx_awid7_rx_err    : 1   ; /* [7]  */
		unsigned int    Reserved_677    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_RX_0ERR;

/* Define the union U_AXIDFX_BID_RX_ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_bid0_rx_err    : 1   ; /* [0]  */
		unsigned int    dfx_bid1_rx_err    : 1   ; /* [1]  */
		unsigned int    dfx_bid2_rx_err    : 1   ; /* [2]  */
		unsigned int    dfx_bid3_rx_err    : 1   ; /* [3]  */
		unsigned int    dfx_bid4_rx_err    : 1   ; /* [4]  */
		unsigned int    dfx_bid5_rx_err    : 1   ; /* [5]  */
		unsigned int    dfx_bid6_rx_err    : 1   ; /* [6]  */
		unsigned int    dfx_bid7_rx_err    : 1   ; /* [7]  */
		unsigned int    Reserved_678    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_BID_RX_ERR;

/* Define the union U_AXIDFX_ARID_LEN_0ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid0_len_err    : 1   ; /* [0]  */
		unsigned int    dfx_arid1_len_err    : 1   ; /* [1]  */
		unsigned int    dfx_arid2_len_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid3_len_err    : 1   ; /* [3]  */
		unsigned int    dfx_arid4_len_err    : 1   ; /* [4]  */
		unsigned int    dfx_arid5_len_err    : 1   ; /* [5]  */
		unsigned int    dfx_arid6_len_err    : 1   ; /* [6]  */
		unsigned int    dfx_arid7_len_err    : 1   ; /* [7]  */
		unsigned int    dfx_arid8_len_err    : 1   ; /* [8]  */
		unsigned int    dfx_arid9_len_err    : 1   ; /* [9]  */
		unsigned int    dfx_arid10_len_err    : 1   ; /* [10]  */
		unsigned int    dfx_arid11_len_err    : 1   ; /* [11]  */
		unsigned int    dfx_arid12_len_err    : 1   ; /* [12]  */
		unsigned int    dfx_arid13_len_err    : 1   ; /* [13]  */
		unsigned int    dfx_arid14_len_err    : 1   ; /* [14]  */
		unsigned int    dfx_arid15_len_err    : 1   ; /* [15]  */
		unsigned int    dfx_arid16_len_err    : 1   ; /* [16]  */
		unsigned int    dfx_arid17_len_err    : 1   ; /* [17]  */
		unsigned int    dfx_arid18_len_err    : 1   ; /* [18]  */
		unsigned int    dfx_arid19_len_err    : 1   ; /* [19]  */
		unsigned int    dfx_arid20_len_err    : 1   ; /* [20]  */
		unsigned int    dfx_arid21_len_err    : 1   ; /* [21]  */
		unsigned int    dfx_arid22_len_err    : 1   ; /* [22]  */
		unsigned int    dfx_arid23_len_err    : 1   ; /* [23]  */
		unsigned int    dfx_arid24_len_err    : 1   ; /* [24]  */
		unsigned int    dfx_arid25_len_err    : 1   ; /* [25]  */
		unsigned int    dfx_arid26_len_err    : 1   ; /* [26]  */
		unsigned int    dfx_arid27_len_err    : 1   ; /* [27]  */
		unsigned int    dfx_arid28_len_err    : 1   ; /* [28]  */
		unsigned int    dfx_arid29_len_err    : 1   ; /* [29]  */
		unsigned int    dfx_arid30_len_err    : 1   ; /* [30]  */
		unsigned int    dfx_arid31_len_err    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_LEN_0ERR;

/* Define the union U_AXIDFX_ARID_LEN_1ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid32_len_err    : 1   ; /* [0]  */
		unsigned int    dfx_arid33_len_err    : 1   ; /* [1]  */
		unsigned int    dfx_arid34_len_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid35_len_err    : 1   ; /* [3]  */
		unsigned int    dfx_arid36_len_err    : 1   ; /* [4]  */
		unsigned int    dfx_arid37_len_err    : 1   ; /* [5]  */
		unsigned int    dfx_arid38_len_err    : 1   ; /* [6]  */
		unsigned int    dfx_arid39_len_err    : 1   ; /* [7]  */
		unsigned int    dfx_arid40_len_err    : 1   ; /* [8]  */
		unsigned int    dfx_arid41_len_err    : 1   ; /* [9]  */
		unsigned int    dfx_arid42_len_err    : 1   ; /* [10]  */
		unsigned int    dfx_arid43_len_err    : 1   ; /* [11]  */
		unsigned int    dfx_arid44_len_err    : 1   ; /* [12]  */
		unsigned int    dfx_arid45_len_err    : 1   ; /* [13]  */
		unsigned int    dfx_arid46_len_err    : 1   ; /* [14]  */
		unsigned int    dfx_arid47_len_err    : 1   ; /* [15]  */
		unsigned int    dfx_arid48_len_err    : 1   ; /* [16]  */
		unsigned int    dfx_arid49_len_err    : 1   ; /* [17]  */
		unsigned int    dfx_arid50_len_err    : 1   ; /* [18]  */
		unsigned int    dfx_arid51_len_err    : 1   ; /* [19]  */
		unsigned int    dfx_arid52_len_err    : 1   ; /* [20]  */
		unsigned int    dfx_arid53_len_err    : 1   ; /* [21]  */
		unsigned int    dfx_arid54_len_err    : 1   ; /* [22]  */
		unsigned int    dfx_arid55_len_err    : 1   ; /* [23]  */
		unsigned int    dfx_arid56_len_err    : 1   ; /* [24]  */
		unsigned int    dfx_arid57_len_err    : 1   ; /* [25]  */
		unsigned int    dfx_arid58_len_err    : 1   ; /* [26]  */
		unsigned int    dfx_arid59_len_err    : 1   ; /* [27]  */
		unsigned int    dfx_arid60_len_err    : 1   ; /* [28]  */
		unsigned int    dfx_arid61_len_err    : 1   ; /* [29]  */
		unsigned int    dfx_arid62_len_err    : 1   ; /* [30]  */
		unsigned int    dfx_arid63_len_err    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_LEN_1ERR;

/* Define the union U_AXIDFX_ARID_LEN_2ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arid64_len_err    : 1   ; /* [0]  */
		unsigned int    dfx_arid65_len_err    : 1   ; /* [1]  */
		unsigned int    dfx_arid66_len_err    : 1   ; /* [2]  */
		unsigned int    dfx_arid67_len_err    : 1   ; /* [3]  */
		unsigned int    Reserved_680    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ARID_LEN_2ERR;

/* Define the union U_AXIDFX_RESP_ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_rresp_err    : 1   ; /* [0]  */
		unsigned int    Reserved_684    : 3   ; /* [3..1]  */
		unsigned int    dfx_bresp_err    : 1   ; /* [4]  */
		unsigned int    Reserved_683    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_RESP_ERR;

/* Define the union U_AXIDFX_ERR_RESP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_rresp    : 2   ; /* [1..0]  */
		unsigned int    dfx_err_bresp    : 2   ; /* [3..2]  */
		unsigned int    Reserved_685    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_RESP;

/* Define the union U_AXIDFX_LEN_ERR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_arlen_err    : 1   ; /* [0]  */
		unsigned int    Reserved_687    : 3   ; /* [3..1]  */
		unsigned int    dfx_awlen_err    : 1   ; /* [4]  */
		unsigned int    Reserved_686    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_LEN_ERR;

/* Define the union U_AXIDFX_ERR_LEN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_err_arlen    : 4   ; /* [3..0]  */
		unsigned int    dfx_err_awlen    : 4   ; /* [7..4]  */
		unsigned int    Reserved_688    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_ERR_LEN;

/* Define the union U_AXIDFX_2RID_FLAG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_2rid_flag    : 4   ; /* [3..0]  */
		unsigned int    Reserved_689    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_2RID_FLAG;

/* Define the union U_AXIDFX_WID_FLAG */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dfx_wid_flag    : 8   ; /* [7..0]  */
		unsigned int    Reserved_690    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_WID_FLAG;

/* Define the union U_AXIDFX_AXI_ST */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    arvalid    : 1   ; /* [0]  */
		unsigned int    arready    : 1   ; /* [1]  */
		unsigned int    rvalid    : 1   ; /* [2]  */
		unsigned int    rready    : 1   ; /* [3]  */
		unsigned int    awvalid    : 1   ; /* [4]  */
		unsigned int    awready    : 1   ; /* [5]  */
		unsigned int    wvalid    : 1   ; /* [6]  */
		unsigned int    wready    : 1   ; /* [7]  */
		unsigned int    bvalid    : 1   ; /* [8]  */
		unsigned int    bready    : 1   ; /* [9]  */
		unsigned int    Reserved_691    : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_AXI_ST;

/* Define the union U_AXIDFX_SOFT_RST_REQ */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    axi_soft_rst_req    : 1   ; /* [0]  */
		unsigned int    Reserved_693    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_SOFT_RST_REQ;

/* Define the union U_AXIDFX_SOFT_RST_ACK */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    axi_soft_rst_ack    : 1   ; /* [0]  */
		unsigned int    Reserved_694    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_SOFT_RST_ACK;

/* Define the union U_AXIDFX_SOFT_RST_FORCE_REQ_ACK */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    axi_soft_rst_force_req_ack  : 1   ; /* [0]  */
		unsigned int    Reserved_696    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_AXIDFX_SOFT_RST_FORCE_REQ_ACK;

/* Define the union U_SMMU_MSTR_GLB_BYPASS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    glb_bypass    : 1   ; /* [0]  */
		unsigned int    Reserved_698    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_GLB_BYPASS;

/* Define the union U_SMMU_MSTR_DEBUG_MODE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    debug_mode    : 1   ; /* [0]  */
		unsigned int    axilock_en    : 1   ; /* [1]  */
		unsigned int    Reserved_699    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_DEBUG_MODE;

/* Define the union U_SMMU_MSTR_MEM_CTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mem_ctrl_s    : 16  ; /* [15..0]  */
		unsigned int    mem_ctrl_d1w2r    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_MEM_CTRL;

/* Define the union U_SMMU_MSTR_CLK_EN */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    apb_clk_en    : 2   ; /* [1..0]  */
		unsigned int    core_clk_en    : 2   ; /* [3..2]  */
		unsigned int    Reserved_700    : 28  ; /* [31..4]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_CLK_EN;

/* Define the union U_SMMU_MSTR_END_REQ_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    end_req_2    : 24  ; /* [23..0]  */
		unsigned int    Reserved_701    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_END_REQ_2;

/* Define the union U_SMMU_MSTR_END_ACK_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    end_ack_2    : 24  ; /* [23..0]  */
		unsigned int    Reserved_702    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_END_ACK_2;

/* Define the union U_SMMU_MSTR_SMRX_START_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    stream_start_2    : 24  ; /* [23..0]  */
		unsigned int    Reserved_703    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_SMRX_START_2;

/* Define the union U_SMMU_MSTR_INPT_SEL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_start_sel    : 1   ; /* [0]  */
		unsigned int    end_req_sel    : 1   ; /* [1]  */
		unsigned int    Reserved_704    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_INPT_SEL;

/* Define the union U_SMMU_MSTR_INTMASK */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    rd_va_err0_msk    : 1   ; /* [0]  */
		unsigned int    rd_va_err1_msk    : 1   ; /* [1]  */
		unsigned int    wr_va_err0_msk    : 1   ; /* [2]  */
		unsigned int    wr_va_err1_msk    : 1   ; /* [3]  */
		unsigned int    wdata_burst_msk    : 1   ; /* [4]  */
		unsigned int    Reserved_706    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_INTMASK;

/* Define the union U_SMMU_MSTR_INTRAW */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    rd_va_err0_raw    : 1   ; /* [0]  */
		unsigned int    rd_va_err1_raw    : 1   ; /* [1]  */
		unsigned int    wr_va_err0_raw    : 1   ; /* [2]  */
		unsigned int    wr_va_err1_raw    : 1   ; /* [3]  */
		unsigned int    wdata_burst_raw    : 1   ; /* [4]  */
		unsigned int    Reserved_707    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_INTRAW;

/* Define the union U_SMMU_MSTR_INTSTAT */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    rd_va_err0_stat    : 1   ; /* [0]  */
		unsigned int    rd_va_err1_stat    : 1   ; /* [1]  */
		unsigned int    wr_va_err0_stat    : 1   ; /* [2]  */
		unsigned int    wr_va_err1_stat    : 1   ; /* [3]  */
		unsigned int    wdata_burst_stat    : 1   ; /* [4]  */
		unsigned int    Reserved_708    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_INTSTAT;

/* Define the union U_SMMU_MSTR_INTCLR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    rd_va_err0_clr    : 1   ; /* [0]  */
		unsigned int    rd_va_err1_clr    : 1   ; /* [1]  */
		unsigned int    wr_va_err0_clr    : 1   ; /* [2]  */
		unsigned int    wr_va_err1_clr    : 1   ; /* [3]  */
		unsigned int    wdata_burst_clr    : 1   ; /* [4]  */
		unsigned int    Reserved_709    : 27  ; /* [31..5]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_INTCLR;

/* Define the union U_SMMU_MSTR_DBG_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    rd_sid    : 8   ; /* [7..0]  */
		unsigned int    Reserved_710    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_DBG_0;

/* Define the union U_SMMU_MSTR_DBG_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    wr_sid    : 8   ; /* [7..0]  */
		unsigned int    Reserved_711    : 24  ; /* [31..8]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_DBG_2;

/* Define the union U_SMMU_MSTR_DBG_4 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    awid    : 8   ; /* [7..0]  */
		unsigned int    awlen    : 8   ; /* [15..8]  */
		unsigned int    wid    : 8   ; /* [23..16]  */
		unsigned int    wdata_len    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_DBG_4;

/* Define the union U_SMMU_MSTR_DBG_PORT_IN_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_cfg_addr    : 16  ; /* [15..0]  */
		unsigned int    dbg_cfg_cs    : 1   ; /* [16]  */
		unsigned int    dbg_cfg_wr    : 1   ; /* [17]  */
		unsigned int    Reserved_713    : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_DBG_PORT_IN_0;

/* Define the union U_SMMU_MSTR_SMRX_0 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    bypass    : 1   ; /* [0]  */
		unsigned int    Reserved_716    : 3   ; /* [3..1]  */
		unsigned int    upwin    : 8   ; /* [11..4]  */
		unsigned int    len    : 8   ; /* [19..12]  */
		unsigned int    Reserved_715    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_SMRX_0;

/* Define the union U_SMMU_MSTR_SMRX_1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_717    : 15  ; /* [14..0]  */
		unsigned int    va_str    : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_SMRX_1;

/* Define the union U_SMMU_MSTR_SMRX_2 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_718    : 15  ; /* [14..0]  */
		unsigned int    va_end    : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MSTR_SMRX_2;

/* Define the union U_RD_CMD_TRANS_LATENCY */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    rd_cmd_max_latency    : 16  ; /* [15..0]  */
		unsigned int    rd_cmd_avg_latency    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_RD_CMD_TRANS_LATENCY;

/* Define the union U_WR_CMD_TRANS_LATENCY */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    wr_cmd_max_latency    : 16  ; /* [15..0]  */
		unsigned int    wr_cmd_avg_latency    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_WR_CMD_TRANS_LATENCY;

/* Define the union U_SMMU_SCR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    glb_bypass    : 1   ; /* [0]  */
		unsigned int    rqos_en    : 1   ; /* [1]  */
		unsigned int    wqos_en    : 1   ; /* [2]  */
		unsigned int    Reserved_723    : 1   ; /* [3]  */
		unsigned int    cache_l1_en    : 1   ; /* [4]  */
		unsigned int    cache_l2_en    : 1   ; /* [5]  */
		unsigned int    rqos    : 4   ; /* [9..6]  */
		unsigned int    wqos    : 4   ; /* [13..10]  */
		unsigned int    Reserved_722    : 1   ; /* [14]  */
		unsigned int    smr_rd_shadow    : 1   ; /* [15]  */
		unsigned int    ptw_pf    : 4   ; /* [19..16]  */
		unsigned int    Reserved_721    : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCR;

/* Define the union U_SMMU_MEMCTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    mem_ctrl_wr    : 16  ; /* [15..0]  */
		unsigned int    mem_ctrl_rd    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_MEMCTRL;

/* Define the union U_SMMU_LP_CTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    auto_clk_gt_en    : 2   ; /* [1..0]  */
		unsigned int    Reserved_724    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_LP_CTRL;

/* Define the union U_SMMU_PRESS_REMAP */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    remap_sel0    : 2   ; /* [1..0]  */
		unsigned int    remap_sel1    : 2   ; /* [3..2]  */
		unsigned int    remap_sel2    : 2   ; /* [5..4]  */
		unsigned int    remap_sel3    : 2   ; /* [7..6]  */
		unsigned int    remap_sel4    : 2   ; /* [9..8]  */
		unsigned int    remap_sel5    : 2   ; /* [11..10]  */
		unsigned int    remap_sel6    : 2   ; /* [13..12]  */
		unsigned int    remap_sel7    : 2   ; /* [15..14]  */
		unsigned int    remap_sel8    : 2   ; /* [17..16]  */
		unsigned int    remap_sel9    : 2   ; /* [19..18]  */
		unsigned int    remap_sel10    : 2   ; /* [21..20]  */
		unsigned int    remap_sel11    : 2   ; /* [23..22]  */
		unsigned int    remap_sel12    : 2   ; /* [25..24]  */
		unsigned int    remap_sel13    : 2   ; /* [27..26]  */
		unsigned int    remap_sel14    : 2   ; /* [29..28]  */
		unsigned int    remap_sel15    : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_PRESS_REMAP;

/* Define the union U_SMMU_INTMASK_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intns_permis_msk    : 1   ; /* [0]  */
		unsigned int    intns_ext_msk    : 1   ; /* [1]  */
		unsigned int    intns_tlbmiss_msk    : 1   ; /* [2]  */
		unsigned int    intns_ptw_trans_msk   : 1   ; /* [3]  */
		unsigned int    intns_ptw_invalid_msk  : 1   ; /* [4]  */
		unsigned int    intns_ptw_ns_msk    : 1   ; /* [5]  */
		unsigned int    Reserved_725    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTMASK_NS;

/* Define the union U_SMMU_INTRAW_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intns_permis_raw    : 1   ; /* [0]  */
		unsigned int    intns_ext_raw    : 1   ; /* [1]  */
		unsigned int    intns_tlbmiss_raw    : 1   ; /* [2]  */
		unsigned int    intns_ptw_trans_raw   : 1   ; /* [3]  */
		unsigned int    intns_ptw_invalid_raw  : 1   ; /* [4]  */
		unsigned int    intns_ptw_ns_raw    : 1   ; /* [5]  */
		unsigned int    Reserved_726    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTRAW_NS;

/* Define the union U_SMMU_INTSTAT_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intns_permis_stat    : 1   ; /* [0]  */
		unsigned int    intns_ext_stat    : 1   ; /* [1]  */
		unsigned int    intns_tlbmiss_stat    : 1   ; /* [2]  */
		unsigned int    intns_ptw_trans_stat  : 1   ; /* [3]  */
		unsigned int    intns_ptw_invalid_stat  : 1   ; /* [4]  */
		unsigned int    intns_ptw_ns_stat    : 1   ; /* [5]  */
		unsigned int    Reserved_727    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTSTAT_NS;

/* Define the union U_SMMU_INTCLR_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intns_permis_clr    : 1   ; /* [0]  */
		unsigned int    intns_ext_clr    : 1   ; /* [1]  */
		unsigned int    intns_tlbmiss_clr    : 1   ; /* [2]  */
		unsigned int    intns_ptw_trans_clr   : 1   ; /* [3]  */
		unsigned int    intns_ptw_invalid_clr  : 1   ; /* [4]  */
		unsigned int    intns_ptw_ns_clr    : 1   ; /* [5]  */
		unsigned int    Reserved_728    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTCLR_NS;

/* Define the union U_SMMU_SMRX_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_bypass    : 1   ; /* [0]  */
		unsigned int    Reserved_730    : 1   ; /* [1]  */
		unsigned int    smr_ptw_qos    : 2   ; /* [3..2]  */
		unsigned int    smr_invld_en    : 1   ; /* [4]  */
		unsigned int    Reserved_729    : 7   ; /* [11..5]  */
		unsigned int    smr_offset_addr    : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SMRX_NS;

/* Define the union U_SMMU_RLD_EN2_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_rld_en2    : 24  ; /* [23..0]  */
		unsigned int    Reserved_731    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_RLD_EN2_NS;

/* Define the union U_SMMU_CB_SCTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cb_bypass    : 1   ; /* [0]  */
		unsigned int    Reserved_732    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_CB_SCTRL;

/* Define the union U_SMMU_CB_TTBCR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cb_ttbcr_des    : 1   ; /* [0]  */
		unsigned int    cb_ttbcr_t0sz    : 3   ; /* [3..1]  */
		unsigned int    cb_ttbcr_n    : 3   ; /* [6..4]  */
		unsigned int    Reserved_734    : 9   ; /* [15..7]  */
		unsigned int    cb_ttbcr_t1sz    : 3   ; /* [18..16]  */
		unsigned int    Reserved_733    : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_CB_TTBCR;

/* Define the union U_SMMU_OFFSET_ADDR_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    offset_addr_ns    : 14  ; /* [13..0]  */
		unsigned int    Reserved_735    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OFFSET_ADDR_NS;

/* Define the union U_SMMU_SCACHEI_ALL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cache_all_invalid    : 1   ; /* [0]  */
		unsigned int    cache_all_level    : 2   ; /* [2..1]  */
		unsigned int    Reserved_736    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCACHEI_ALL;

/* Define the union U_SMMU_SCACHEI_L1 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cache_l1_invalid    : 1   ; /* [0]  */
		unsigned int    cache_l1_security    : 2   ; /* [2..1]  */
		unsigned int    Reserved_737    : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCACHEI_L1;

/* Define the union U_SMMU_SCACHEI_L2L3 */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    cache_l2l3_invalid    : 1   ; /* [0]  */
		unsigned int    cache_l2l3_strmid    : 15  ; /* [15..1]  */
		unsigned int    Reserved_738    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCACHEI_L2L3;

/* Define the union U_SMMU_FAMA_CTRL0_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fama_sdes_msb_ns    : 7   ; /* [6..0]  */
		unsigned int    fama_chn_sel_ns    : 1   ; /* [7]  */
		unsigned int    fama_bps_msb_ns    : 6   ; /* [13..8]  */
		unsigned int    Reserved_739    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAMA_CTRL0_NS;

/* Define the union U_SMMU_FAMA_CTRL1_NS */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fama_ptw_msb_ns    : 7   ; /* [6..0]  */
		unsigned int    Reserved_740    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAMA_CTRL1_NS;

/* Define the union U_SMMU_ADDR_MSB */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    msb_errrd    : 7   ; /* [6..0]  */
		unsigned int    msb_errwr    : 7   ; /* [13..7]  */
		unsigned int    msb_ova    : 7   ; /* [20..14]  */
		unsigned int    Reserved_741    : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_ADDR_MSB;

/* Define the union U_SMMU_FAULT_ID_TCU */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fault_index_id_tcu    : 16  ; /* [15..0]  */
		unsigned int    fault_strm_id_tcu    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAULT_ID_TCU;

/* Define the union U_SMMU_FAULT_ID_TBUX */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fault_index_id_tbu    : 16  ; /* [15..0]  */
		unsigned int    fault_strm_id_tbu    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAULT_ID_TBUX;

/* Define the union U_SMMU_FAULT_INFOX */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fault_tlbmiss_err_tbu  : 2   ; /* [1..0]  */
		unsigned int    fault_permis_err_tbu  : 3   ; /* [4..2]  */
		unsigned int    fautl_ext_err_tbu    : 2   ; /* [6..5]  */
		unsigned int    fault_ext_err_id_tbu  : 9   ; /* [15..7]  */
		unsigned int    Reserved_742    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAULT_INFOX;

/* Define the union U_SMMU_DBGRPTR_TLB */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_tlbword_pointer   : 3   ; /* [2..0]  */
		unsigned int    dbg_tlbentry_pointer  : 13  ; /* [15..3]  */
		unsigned int    Reserved_744    : 14  ; /* [29..16]  */
		unsigned int    dbg_tlb_type    : 1   ; /* [30]  */
		unsigned int    Reserved_743    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRPTR_TLB;

/* Define the union U_SMMU_DBGRDATA_TLB */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_tlb_rdata    : 31  ; /* [30..0]  */
		unsigned int    Reserved_745    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRDATA_TLB;

/* Define the union U_SMMU_DBGRPTR_CACHE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_cache_level    : 2   ; /* [1..0]  */
		unsigned int    dbg_cache_l1_pointer  : 2   ; /* [3..2]  */
		unsigned int    dbg_cache_l1_ns    : 1   ; /* [4]  */
		unsigned int    dbg_cache_l2_strmid   : 11  ; /* [15..5]  */
		unsigned int    Reserved_746    : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRPTR_CACHE;

/* Define the union U_SMMU_DBGRDATA0_CACHE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_cache_rdata0    : 29  ; /* [28..0]  */
		unsigned int    Reserved_747    : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRDATA0_CACHE;

/* Define the union U_SMMU_DBGRDATA1_CACHE */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_cache_rdata1    : 12  ; /* [11..0]  */
		unsigned int    Reserved_748    : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRDATA1_CACHE;

/* Define the union U_SMMU_DBGAXI_CTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_axilock_en    : 1   ; /* [0]  */
		unsigned int    Reserved_749    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGAXI_CTRL;

/* Define the union U_SMMU_OPA_ADDR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    override_acquire_pa   : 27  ; /* [26..0]  */
		unsigned int    Reserved_750    : 4   ; /* [30..27]  */
		unsigned int    override_pa_done    : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OPA_ADDR;

/* Define the union U_SMMU_OVA_CTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    override_va_cfg    : 1   ; /* [0]  */
		unsigned int    Reserved_751    : 1   ; /* [1]  */
		unsigned int    override_va_type    : 1   ; /* [2]  */
		unsigned int    override_va_indexid   : 13  ; /* [15..3]  */
		unsigned int    override_va_strmid    : 12  ; /* [27..16]  */
		unsigned int    override_tbu_num    : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OVA_CTRL;

/* Define the union U_SMMU_OPREF_CTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    override_pref_cfg    : 1   ; /* [0]  */
		unsigned int    Reserved_752    : 1   ; /* [1]  */
		unsigned int    override_pref_type    : 1   ; /* [2]  */
		unsigned int    override_pref_initial  : 1   ; /* [3]  */
		unsigned int    override_pref_indexid  : 12  ; /* [15..4]  */
		unsigned int    override_pref_strmid  : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OPREF_CTRL;

/* Define the union U_SMMU_SMRX_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_nscfg    : 1   ; /* [0]  */
		unsigned int    smr_nscfg_en    : 1   ; /* [1]  */
		unsigned int    smr_bypass_s    : 1   ; /* [2]  */
		unsigned int    Reserved_753    : 2   ; /* [4..3]  */
		unsigned int    smr_mid_en_s    : 1   ; /* [5]  */
		unsigned int    smr_mid_s    : 6   ; /* [11..6]  */
		unsigned int    smr_offset_addr_s    : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SMRX_S;

/* Define the union U_SMMU_RLD_EN2_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_rld_en2_s    : 24  ; /* [23..0]  */
		unsigned int    Reserved_754    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_RLD_EN2_S;

/* Define the union U_SMMU_INTMAS_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ints_permis_msk    : 1   ; /* [0]  */
		unsigned int    ints_ext_msk    : 1   ; /* [1]  */
		unsigned int    ints_tlbmiss_msk    : 1   ; /* [2]  */
		unsigned int    ints_ptw_trans_msk    : 1   ; /* [3]  */
		unsigned int    ints_ptw_invalid_msk  : 1   ; /* [4]  */
		unsigned int    ints_ptw_ns_msk    : 1   ; /* [5]  */
		unsigned int    Reserved_755    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTMAS_S;

/* Define the union U_SMMU_INTRAW_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ints_permis_raw    : 1   ; /* [0]  */
		unsigned int    ints_ext_raw    : 1   ; /* [1]  */
		unsigned int    ints_tlbmiss_raw    : 1   ; /* [2]  */
		unsigned int    ints_ptw_trans_raw    : 1   ; /* [3]  */
		unsigned int    ints_ptw_invalid_raw  : 1   ; /* [4]  */
		unsigned int    ints_ptw_ns_raw    : 1   ; /* [5]  */
		unsigned int    Reserved_756    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTRAW_S;

/* Define the union U_SMMU_INTSTAT_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ints_permis_stat    : 1   ; /* [0]  */
		unsigned int    ints_ext_stat    : 1   ; /* [1]  */
		unsigned int    ints_tlbmiss_stat    : 1   ; /* [2]  */
		unsigned int    ints_ptw_trans_stat   : 1   ; /* [3]  */
		unsigned int    ints_ptw_invalid_stat  : 1   ; /* [4]  */
		unsigned int    ints_ptw_ns_stat    : 1   ; /* [5]  */
		unsigned int    Reserved_757    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTSTAT_S;

/* Define the union U_SMMU_INTCLR_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    ints_permis_clr    : 1   ; /* [0]  */
		unsigned int    ints_ext_clr    : 1   ; /* [1]  */
		unsigned int    ints_tlbmiss_clr    : 1   ; /* [2]  */
		unsigned int    ints_ptw_trans_clr    : 1   ; /* [3]  */
		unsigned int    ints_ptw_invalid_clr  : 1   ; /* [4]  */
		unsigned int    ints_ptw_ns_clr    : 1   ; /* [5]  */
		unsigned int    Reserved_758    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTCLR_S;

/* Define the union U_SMMU_SCR_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    glb_nscfg    : 2   ; /* [1..0]  */
		unsigned int    mid_s    : 6   ; /* [7..2]  */
		unsigned int    glb_bypass_s    : 1   ; /* [8]  */
		unsigned int    mid_en_s    : 1   ; /* [9]  */
		unsigned int    Reserved_759    : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCR_S;

/* Define the union U_SMMU_SCB_SCTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    scb_bypass    : 1   ; /* [0]  */
		unsigned int    Reserved_760    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCB_SCTRL;

/* Define the union U_SMMU_SCB_TTBCR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    scb_ttbcr_des    : 1   ; /* [0]  */
		unsigned int    Reserved_761    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCB_TTBCR;

/* Define the union U_SMMU_OFFSET_ADDR_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    offset_addr_s    : 14  ; /* [13..0]  */
		unsigned int    Reserved_762    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OFFSET_ADDR_S;

/* Define the union U_SMMU_FAMA_CTRL0_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fama_sdes_msb_s    : 7   ; /* [6..0]  */
		unsigned int    fama_chn_sel_s    : 1   ; /* [7]  */
		unsigned int    fama_bps_msb_s    : 6   ; /* [13..8]  */
		unsigned int    Reserved_763    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAMA_CTRL0_S;

/* Define the union U_SMMU_FAMA_CTRL1_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fama_ptw_msb_s    : 7   ; /* [6..0]  */
		unsigned int    Reserved_764    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAMA_CTRL1_S;

/* Define the union U_SMMU_DBGRPTR_TLB_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_tlb_en    : 1   ; /* [0]  */
		unsigned int    Reserved_765    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRPTR_TLB_S;

/* Define the union U_SMMU_DBGRPTR_CACHE_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    dbg_cache_en    : 1   ; /* [0]  */
		unsigned int    Reserved_766    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_DBGRPTR_CACHE_S;

/* Define the union U_SMMU_OVERRIDE_CTRL_S */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    Reserved_768    : 1   ; /* [0]  */
		unsigned int    override_va_security  : 1   ; /* [1]  */
		unsigned int    Reserved_767    : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OVERRIDE_CTRL_S;

/* Define the union U_SMMU_SMRX_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_protect_en    : 1   ; /* [0]  */
		unsigned int    Reserved_770    : 1   ; /* [1]  */
		unsigned int    smr_bypass_p    : 1   ; /* [2]  */
		unsigned int    Reserved_769    : 9   ; /* [11..3]  */
		unsigned int    smr_offset_addr_p    : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SMRX_P;

/* Define the union U_SMMU_RLD_EN2_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    smr_rld_en2_p    : 24  ; /* [23..0]  */
		unsigned int    Reserved_771    : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_RLD_EN2_P;

/* Define the union U_SMMU_INTMAS_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intp_permis_msk    : 1   ; /* [0]  */
		unsigned int    intp_ext_msk    : 1   ; /* [1]  */
		unsigned int    intp_tlbmiss_msk    : 1   ; /* [2]  */
		unsigned int    intp_ptw_trans_msk    : 1   ; /* [3]  */
		unsigned int    intp_ptw_invalid_msk  : 1   ; /* [4]  */
		unsigned int    intp_ptw_ns_msk    : 1   ; /* [5]  */
		unsigned int    Reserved_772    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTMAS_P;

/* Define the union U_SMMU_INTRAW_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intp_permis_raw    : 1   ; /* [0]  */
		unsigned int    intp_ext_raw    : 1   ; /* [1]  */
		unsigned int    intp_tlbmiss_raw    : 1   ; /* [2]  */
		unsigned int    intp_ptw_trans_raw    : 1   ; /* [3]  */
		unsigned int    intp_ptw_invalid_raw  : 1   ; /* [4]  */
		unsigned int    intp_ptw_ns_raw    : 1   ; /* [5]  */
		unsigned int    Reserved_773    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTRAW_P;

/* Define the union U_SMMU_INTSTAT_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intp_permis_stat    : 1   ; /* [0]  */
		unsigned int    intp_ext_stat    : 1   ; /* [1]  */
		unsigned int    intp_tlbmiss_stat    : 1   ; /* [2]  */
		unsigned int    intp_ptw_trans_stat   : 1   ; /* [3]  */
		unsigned int    intp_ptw_invalid_stat  : 1   ; /* [4]  */
		unsigned int    intp_ptw_ns_stat    : 1   ; /* [5]  */
		unsigned int    Reserved_774    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTSTAT_P;

/* Define the union U_SMMU_INTCLR_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    intp_permis_clr    : 1   ; /* [0]  */
		unsigned int    intp_ext_clr    : 1   ; /* [1]  */
		unsigned int    intp_tlbmiss_clr    : 1   ; /* [2]  */
		unsigned int    intp_ptw_trans_clr    : 1   ; /* [3]  */
		unsigned int    intp_ptw_invalid_clr  : 1   ; /* [4]  */
		unsigned int    intp_ptw_ns_clr    : 1   ; /* [5]  */
		unsigned int    Reserved_775    : 26  ; /* [31..6]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_INTCLR_P;

/* Define the union U_SMMU_SCR_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    glb_prot_cfg    : 1   ; /* [0]  */
		unsigned int    Reserved_777    : 7   ; /* [7..1]  */
		unsigned int    glb_bypass_p    : 1   ; /* [8]  */
		unsigned int    Reserved_776    : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_SCR_P;

/* Define the union U_SMMU_PCB_SCTRL */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pcb_bypass    : 1   ; /* [0]  */
		unsigned int    Reserved_778    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_PCB_SCTRL;

/* Define the union U_SMMU_PCB_TTBCR */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    pcb_ttbcr_des    : 1   ; /* [0]  */
		unsigned int    Reserved_779    : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_PCB_TTBCR;

/* Define the union U_SMMU_OFFSET_ADDR_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    offset_addr_p    : 14  ; /* [13..0]  */
		unsigned int    Reserved_780    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_OFFSET_ADDR_P;

/* Define the union U_SMMU_FAMA_CTRL0_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fama_sdes_msb_p    : 7   ; /* [6..0]  */
		unsigned int    fama_chn_sel_p    : 1   ; /* [7]  */
		unsigned int    fama_bps_msb_p    : 6   ; /* [13..8]  */
		unsigned int    Reserved_781    : 18  ; /* [31..14]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAMA_CTRL0_P;

/* Define the union U_SMMU_FAMA_CTRL1_P */
typedef union
{
	/* Define the struct bits */
	struct
	{
		unsigned int    fama_ptw_msb_p    : 7   ; /* [6..0]  */
		unsigned int    Reserved_782    : 25  ; /* [31..7]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

} U_SMMU_FAMA_CTRL1_P;

//==============================================================================
/* Define the global struct */
typedef struct
{
	volatile U_VEDU_VCPI_INTMASK    VEDU_VCPI_INTMASK;
	volatile U_VEDU_VCPI_INTCLR    VEDU_VCPI_INTCLR;
	volatile U_VEDU_VCPI_START    VEDU_VCPI_START;
	volatile U_VEDU_VCPI_CNTCLR    VEDU_VCPI_CNTCLR;
	volatile unsigned int    VEDU_VCPI_FRAMENO;
	volatile U_VEDU_VCPI_BP_POS    VEDU_VCPI_BP_POS;
	volatile unsigned int    VEDU_VCPI_TIMEOUT;
	volatile U_VEDU_VCPI_MODE    VEDU_VCPI_MODE;
	volatile U_VEDU_VCPI_TILE_SIZE   VEDU_VCPI_TILE_SIZE;
	volatile U_VEDU_VCPI_PICSIZE_PIX   VEDU_VCPI_PICSIZE_PIX;
	volatile U_VEDU_VCPI_MULTISLC   VEDU_VCPI_MULTISLC;
	volatile U_VEDU_VCPI_QPCFG    VEDU_VCPI_QPCFG;
	volatile U_VEDU_VCPI_DBLKCFG    VEDU_VCPI_DBLKCFG;
	volatile U_VEDU_VCPI_LOW_POWER   VEDU_VCPI_LOW_POWER;
	volatile U_VEDU_VCPI_OUTSTD    VEDU_VCPI_OUTSTD;
	volatile U_VEDU_VCPI_TMV_LOAD   VEDU_VCPI_TMV_LOAD;
	volatile U_VEDU_VCPI_CROSS_TILE_SLC   VEDU_VCPI_CROSS_TILE_SLC;
	volatile U_VEDU_VCPI_MEM_CTRL   VEDU_VCPI_MEM_CTRL;
	volatile U_VEDU_VCPI_INTRA_INTER_CU_EN   VEDU_VCPI_INTRA_INTER_CU_EN;
	volatile U_VEDU_VCPI_VLC_CONFIG   VEDU_VCPI_VLC_CONFIG;
	volatile U_VEDU_VCPI_PRE_JUDGE_EXT_EN   VEDU_VCPI_PRE_JUDGE_EXT_EN;
	volatile U_VEDU_VCPI_PRE_JUDGE_COST_THR   VEDU_VCPI_PRE_JUDGE_COST_THR;
	volatile U_VEDU_VCPI_IBLK_PRE_MV_THR   VEDU_VCPI_IBLK_PRE_MV_THR;
	volatile U_VEDU_VCPI_PME_PARAM   VEDU_VCPI_PME_PARAM;
	volatile U_VEDU_VCPI_PIC_STRONG_EN   VEDU_VCPI_PIC_STRONG_EN;
	volatile U_VEDU_VCPI_REF_FLAG   VEDU_VCPI_REF_FLAG;
	volatile U_VEDU_VCPI_RC_ENABLE   VEDU_VCPI_RC_ENABLE;
	volatile U_VEDU_VCPI_PINTRA_THRESH0   VEDU_VCPI_PINTRA_THRESH0;
	volatile U_VEDU_VCPI_PINTRA_THRESH1   VEDU_VCPI_PINTRA_THRESH1;
	volatile U_VEDU_VCPI_PINTRA_THRESH2   VEDU_VCPI_PINTRA_THRESH2;
	volatile U_VEDU_VCPI_I_SLC_INSERT   VEDU_VCPI_I_SLC_INSERT;
	volatile U_VEDU_VCPI_CLKDIV_ENABLE   VEDU_VCPI_CLKDIV_ENABLE;
	volatile U_VEDU_VCPI_OSD_ENABLE   VEDU_VCPI_OSD_ENABLE;
	volatile U_VEDU_VCPI_OSD_POS_0   VEDU_VCPI_OSD_POS_0;
	volatile U_VEDU_VCPI_OSD_POS_1   VEDU_VCPI_OSD_POS_1;
	volatile U_VEDU_VCPI_OSD_POS_2   VEDU_VCPI_OSD_POS_2;
	volatile U_VEDU_VCPI_OSD_POS_3   VEDU_VCPI_OSD_POS_3;
	volatile U_VEDU_VCPI_OSD_POS_4   VEDU_VCPI_OSD_POS_4;
	volatile U_VEDU_VCPI_OSD_POS_5   VEDU_VCPI_OSD_POS_5;
	volatile U_VEDU_VCPI_OSD_POS_6   VEDU_VCPI_OSD_POS_6;
	volatile U_VEDU_VCPI_OSD_POS_7   VEDU_VCPI_OSD_POS_7;
	volatile U_VEDU_VCPI_OSD_SIZE_0   VEDU_VCPI_OSD_SIZE_0;
	volatile U_VEDU_VCPI_OSD_SIZE_1   VEDU_VCPI_OSD_SIZE_1;
	volatile U_VEDU_VCPI_OSD_SIZE_2   VEDU_VCPI_OSD_SIZE_2;
	volatile U_VEDU_VCPI_OSD_SIZE_3   VEDU_VCPI_OSD_SIZE_3;
	volatile U_VEDU_VCPI_OSD_SIZE_4   VEDU_VCPI_OSD_SIZE_4;
	volatile U_VEDU_VCPI_OSD_SIZE_5   VEDU_VCPI_OSD_SIZE_5;
	volatile U_VEDU_VCPI_OSD_SIZE_6   VEDU_VCPI_OSD_SIZE_6;
	volatile U_VEDU_VCPI_OSD_SIZE_7   VEDU_VCPI_OSD_SIZE_7;
	volatile U_VEDU_VCPI_OSD_LAYERID   VEDU_VCPI_OSD_LAYERID;
	volatile U_VEDU_VCPI_OSD_QP0    VEDU_VCPI_OSD_QP0;
	volatile U_VEDU_VCPI_OSD_QP1    VEDU_VCPI_OSD_QP1;
	volatile U_VEDU_VCPI_SW_L0_SIZE   VEDU_VCPI_SW_L0_SIZE;
	volatile U_VEDU_VCPI_SW_L1_SIZE   VEDU_VCPI_SW_L1_SIZE;
	volatile unsigned int    VEDU_VCPI_PMEINFO_ST_ADDR;
	volatile unsigned int    VEDU_VCPI_PMEINFO_LD0_ADDR;
	volatile unsigned int    VEDU_VCPI_PMEINFO_LD1_ADDR;
	volatile unsigned int    VEDU_VCPI_QPGLD_INF_ADDR;
	volatile unsigned int    VEDU_VCPI_TUNLCELL_ADDR;
	volatile unsigned int    VEDU_VCPI_SRC_YADDR;
	volatile unsigned int    VEDU_VCPI_SRC_CADDR;
	volatile unsigned int    VEDU_VCPI_SRC_VADDR;
	volatile unsigned int    VEDU_VCPI_YH_ADDR;
	volatile unsigned int    VEDU_VCPI_CH_ADDR;
	volatile U_VEDU_VCPI_STRIDE    VEDU_VCPI_STRIDE;
	volatile unsigned int    VEDU_VCPI_REC_YADDR;
	volatile unsigned int    VEDU_VCPI_REC_CADDR;
	volatile U_VEDU_VCPI_REC_STRIDE   VEDU_VCPI_REC_STRIDE;
	volatile unsigned int    VEDU_VCPI_REC_YH_ADDR;
	volatile unsigned int    VEDU_VCPI_REC_CH_ADDR;
	volatile U_VEDU_VCPI_REC_HEAD_STRIDE   VEDU_VCPI_REC_HEAD_STRIDE;
	volatile unsigned int    VEDU_VCPI_REFY_L0_ADDR;
	volatile unsigned int    VEDU_VCPI_REFC_L0_ADDR;
	volatile U_VEDU_VCPI_REF_L0_STRIDE   VEDU_VCPI_REF_L0_STRIDE;
	volatile unsigned int    VEDU_VCPI_REFYH_L0_ADDR;
	volatile unsigned int    VEDU_VCPI_REFCH_L0_ADDR;
	volatile U_VEDU_VCPI_REFH_L0_STRIDE   VEDU_VCPI_REFH_L0_STRIDE;
	volatile unsigned int    VEDU_VCPI_PMELD_L0_ADDR;
	volatile unsigned int    VEDU_VCPI_REFY_L1_ADDR;
	volatile unsigned int    VEDU_VCPI_REFC_L1_ADDR;
	volatile U_VEDU_VCPI_REF_L1_STRIDE   VEDU_VCPI_REF_L1_STRIDE;
	volatile unsigned int    VEDU_VCPI_REFYH_L1_ADDR;
	volatile unsigned int    VEDU_VCPI_REFCH_L1_ADDR;
	volatile U_VEDU_VCPI_REFH_L1_STRIDE   VEDU_VCPI_REFH_L1_STRIDE;
	volatile unsigned int    VEDU_VCPI_PMELD_L1_ADDR;
	volatile unsigned int    VEDU_VCPI_PMEST_ADDR;
	volatile unsigned int    VEDU_VCPI_NBI_UPST_ADDR;
	volatile unsigned int    VEDU_VCPI_NBI_MVST_ADDR;
	volatile unsigned int    VEDU_VCPI_NBI_MVLD_ADDR;
	volatile unsigned int    VEDU_VCPI_STRMADDR;
	volatile unsigned int    VEDU_VCPI_SWPTRADDR;
	volatile unsigned int    VEDU_VCPI_SRPTRADDR;
	volatile unsigned int    VEDU_VCPI_LLILD_ADDR;
	volatile U_VEDU_VCPI_STRFMT    VEDU_VCPI_STRFMT;
	volatile U_VEDU_VCPI_CROP_START   VEDU_VCPI_CROP_START;
	volatile U_VEDU_VCPI_CROP_END   VEDU_VCPI_CROP_END;
	volatile U_VEDU_VCPI_SCALE_PARA   VEDU_VCPI_SCALE_PARA;
	volatile U_VEDU_VCPI_ORI_PICSIZE   VEDU_VCPI_ORI_PICSIZE;
	volatile U_VEDU_VCPI_BG_ENABLE   VEDU_VCPI_BG_ENABLE;
	volatile unsigned int    VEDU_VCPI_BGL_ADDR;
	volatile unsigned int    VEDU_VCPI_BGC_ADDR;
	volatile unsigned int    VEDU_VCPI_BGINF_ADDR;
	volatile U_VEDU_VCPI_BG_STRIDE   VEDU_VCPI_BG_STRIDE;
	volatile U_VEDU_VCPI_BG_FLT_PARA0   VEDU_VCPI_BG_FLT_PARA0;
	volatile U_VEDU_VCPI_BG_FLT_PARA1   VEDU_VCPI_BG_FLT_PARA1;
	volatile U_VEDU_VCPI_BG_FLT_PARA2   VEDU_VCPI_BG_FLT_PARA2;
	volatile U_VEDU_VCPI_BG_THR0    VEDU_VCPI_BG_THR0;
	volatile U_VEDU_VCPI_BG_THR1    VEDU_VCPI_BG_THR1;
	volatile U_VEDU_VCPI_MEM_CTRL_T16   VEDU_VCPI_MEM_CTRL_T16;
	volatile unsigned int    VEDU_VCPI_PMEST_STRIDE;
	volatile unsigned int    VEDU_VCPI_PMELD_STRIDE;
	volatile U_VEDU_VCPI_INTRA32_LOW_POWER   VEDU_VCPI_INTRA32_LOW_POWER;
	volatile U_VEDU_VCPI_INTRA16_LOW_POWER   VEDU_VCPI_INTRA16_LOW_POWER;
	volatile U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM   VEDU_VCPI_INTRA_REDUCE_RDO_NUM;
	volatile unsigned int    VEDU_VCPI_DBLK_INFO_ADDR;
	volatile U_VEDU_VCPI_NOFORCEZERO   VEDU_VCPI_NOFORCEZERO;
	volatile U_VEDU_VCPI_INTMASK_S   VEDU_VCPI_INTMASK_S;
	volatile U_VEDU_VCPI_INTCLR_S   VEDU_VCPI_INTCLR_S;
	volatile unsigned int    RESERVED_VCPI[10];
	volatile U_VEDU_VCTRL_ROI_CFG0   VEDU_VCTRL_ROI_CFG0;
	volatile U_VEDU_VCTRL_ROI_CFG1   VEDU_VCTRL_ROI_CFG1;
	volatile U_VEDU_VCTRL_ROI_CFG2   VEDU_VCTRL_ROI_CFG2;
	volatile U_VEDU_VCTRL_ROI_SIZE_0   VEDU_VCTRL_ROI_SIZE_0;
	volatile U_VEDU_VCTRL_ROI_SIZE_1   VEDU_VCTRL_ROI_SIZE_1;
	volatile U_VEDU_VCTRL_ROI_SIZE_2   VEDU_VCTRL_ROI_SIZE_2;
	volatile U_VEDU_VCTRL_ROI_SIZE_3   VEDU_VCTRL_ROI_SIZE_3;
	volatile U_VEDU_VCTRL_ROI_SIZE_4   VEDU_VCTRL_ROI_SIZE_4;
	volatile U_VEDU_VCTRL_ROI_SIZE_5   VEDU_VCTRL_ROI_SIZE_5;
	volatile U_VEDU_VCTRL_ROI_SIZE_6   VEDU_VCTRL_ROI_SIZE_6;
	volatile U_VEDU_VCTRL_ROI_SIZE_7   VEDU_VCTRL_ROI_SIZE_7;
	volatile U_VEDU_VCTRL_ROI_START_0   VEDU_VCTRL_ROI_START_0;
	volatile U_VEDU_VCTRL_ROI_START_1   VEDU_VCTRL_ROI_START_1;
	volatile U_VEDU_VCTRL_ROI_START_2   VEDU_VCTRL_ROI_START_2;
	volatile U_VEDU_VCTRL_ROI_START_3   VEDU_VCTRL_ROI_START_3;
	volatile U_VEDU_VCTRL_ROI_START_4   VEDU_VCTRL_ROI_START_4;
	volatile U_VEDU_VCTRL_ROI_START_5   VEDU_VCTRL_ROI_START_5;
	volatile U_VEDU_VCTRL_ROI_START_6   VEDU_VCTRL_ROI_START_6;
	volatile U_VEDU_VCTRL_ROI_START_7   VEDU_VCTRL_ROI_START_7;
	volatile U_VEDU_VCTRL_LCU_TARGET_BIT   VEDU_VCTRL_LCU_TARGET_BIT;
	volatile U_VEDU_VCTRL_NARROW_THRESHOLD   VEDU_VCTRL_NARROW_THRESHOLD;
	volatile U_VEDU_VCTRL_LCU_BASELINE   VEDU_VCTRL_LCU_BASELINE;
	volatile U_VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE   VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE   VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_NORM_COEFF_DENOISE   VEDU_VCTRL_NORM_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_NORM_ENG_DENOISE   VEDU_VCTRL_NORM_ENG_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE   VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE   VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_COEFF_DENOISE   VEDU_VCTRL_SKIN_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_ENG_DENOISE   VEDU_VCTRL_SKIN_ENG_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE   VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE   VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_COEFF_DENOISE   VEDU_VCTRL_HEDGE_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_ENG_DENOISE   VEDU_VCTRL_HEDGE_ENG_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE   VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE   VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE   VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_ENG_DENOISE   VEDU_VCTRL_HEDGEMOV_ENG_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE   VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE   VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_COEFF_DENOISE   VEDU_VCTRL_STATIC_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_ENG_DENOISE   VEDU_VCTRL_STATIC_ENG_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE   VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE   VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_COEFF_DENOISE   VEDU_VCTRL_SOBELSTR_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_ENG_DENOISE   VEDU_VCTRL_SOBELSTR_ENG_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE   VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE   VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE   VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_ENG_DENOISE   VEDU_VCTRL_SOBELWEAK_ENG_DENOISE;
	volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_0   VEDU_VCTRL_INTRA_RDO_FACTOR_0;
	volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_1   VEDU_VCTRL_INTRA_RDO_FACTOR_1;
	volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_2   VEDU_VCTRL_INTRA_RDO_FACTOR_2;
	volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_0   VEDU_VCTRL_MRG_RDO_FACTOR_0;
	volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_1   VEDU_VCTRL_MRG_RDO_FACTOR_1;
	volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_2   VEDU_VCTRL_MRG_RDO_FACTOR_2;
	volatile U_VEDU_VCTRL_FME_RDO_FACTOR_0   VEDU_VCTRL_FME_RDO_FACTOR_0;
	volatile U_VEDU_VCTRL_FME_RDO_FACTOR_1   VEDU_VCTRL_FME_RDO_FACTOR_1;
	volatile U_VEDU_VCTRL_FME_RDO_FACTOR_2   VEDU_VCTRL_FME_RDO_FACTOR_2;
	volatile unsigned int    RESERVED_VCTRL[69];
	volatile U_VEDU_CURLD_GCFG    VEDU_CURLD_GCFG;
	volatile U_VEDU_CURLD_OSD01_ALPHA   VEDU_CURLD_OSD01_ALPHA;
	volatile U_VEDU_CURLD_OSD23_ALPHA   VEDU_CURLD_OSD23_ALPHA;
	volatile U_VEDU_CURLD_OSD45_ALPHA   VEDU_CURLD_OSD45_ALPHA;
	volatile U_VEDU_CURLD_OSD67_ALPHA   VEDU_CURLD_OSD67_ALPHA;
	volatile U_VEDU_CURLD_OSD_GALPHA0   VEDU_CURLD_OSD_GALPHA0;
	volatile U_VEDU_CURLD_OSD_GALPHA1   VEDU_CURLD_OSD_GALPHA1;
	volatile unsigned int    VEDU_CURLD_OSD0_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD1_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD2_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD3_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD4_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD5_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD6_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD7_ADDR;
	volatile U_VEDU_CURLD_OSD01_STRIDE   VEDU_CURLD_OSD01_STRIDE;
	volatile U_VEDU_CURLD_OSD23_STRIDE   VEDU_CURLD_OSD23_STRIDE;
	volatile U_VEDU_CURLD_OSD45_STRIDE   VEDU_CURLD_OSD45_STRIDE;
	volatile U_VEDU_CURLD_OSD67_STRIDE   VEDU_CURLD_OSD67_STRIDE;
	volatile U_VEDU_CURLD_CLIP_THR   VEDU_CURLD_CLIP_THR;
	volatile U_VEDU_CURLD_HOR_FILTER   VEDU_CURLD_HOR_FILTER;
	volatile U_VEDU_CURLD_VER_FILTER   VEDU_CURLD_VER_FILTER;
	volatile U_VEDU_CURLD_ARGB_YUV_0COEFF   VEDU_CURLD_ARGB_YUV_0COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_1COEFF   VEDU_CURLD_ARGB_YUV_1COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_2COEFF   VEDU_CURLD_ARGB_YUV_2COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_3COEFF   VEDU_CURLD_ARGB_YUV_3COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_4COEFF   VEDU_CURLD_ARGB_YUV_4COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_5COEFF   VEDU_CURLD_ARGB_YUV_5COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_6COEFF   VEDU_CURLD_ARGB_YUV_6COEFF;
	volatile U_VEDU_CURLD_ARGB_CLIP   VEDU_CURLD_ARGB_CLIP;
	volatile U_VEDU_CURLD_NARROW_EN   VEDU_CURLD_NARROW_EN;
	volatile U_VEDU_CURLD_SRCH_STRIDE   VEDU_CURLD_SRCH_STRIDE;
	volatile U_VEDU_CURLD_HFBCD    VEDU_CURLD_HFBCD;
	volatile unsigned int    RESERVED_CURLD[31];
	volatile unsigned int    RESERVED_NBI[64];
	volatile U_VEDU_PME_SW_ADAPT_EN   VEDU_PME_SW_ADAPT_EN;
	volatile U_VEDU_PME_SW_THR0    VEDU_PME_SW_THR0;
	volatile U_VEDU_PME_SW_THR1    VEDU_PME_SW_THR1;
	volatile U_VEDU_PME_SW_THR2    VEDU_PME_SW_THR2;
	volatile U_VEDU_PME_SKIP_PRE    VEDU_PME_SKIP_PRE;
	volatile U_VEDU_PME_TR_WEIGHTX   VEDU_PME_TR_WEIGHTX;
	volatile U_VEDU_PME_TR_WEIGHTY   VEDU_PME_TR_WEIGHTY;
	volatile U_VEDU_PME_SR_WEIGHT   VEDU_PME_SR_WEIGHT;
	volatile U_VEDU_PME_INTRABLK_DET   VEDU_PME_INTRABLK_DET;
	volatile U_VEDU_PME_INTRABLK_DET_THR   VEDU_PME_INTRABLK_DET_THR;
	volatile U_VEDU_PME_SKIN_THR    VEDU_PME_SKIN_THR;
	volatile U_VEDU_PME_INTRA_LOWPOW   VEDU_PME_INTRA_LOWPOW;
	volatile U_VEDU_PME_IBLK_COST_THR   VEDU_PME_IBLK_COST_THR;
	volatile U_VEDU_PME_STRONG_EDGE   VEDU_PME_STRONG_EDGE;
	volatile U_VEDU_PME_LARGE_MOVE_THR   VEDU_PME_LARGE_MOVE_THR;
	volatile U_VEDU_PME_INTER_STRONG_EDGE   VEDU_PME_INTER_STRONG_EDGE;
	volatile U_VEDU_PME_NEW_COST    VEDU_PME_NEW_COST;
	volatile U_VEDU_PME_WINDOW_SIZE0_L0   VEDU_PME_WINDOW_SIZE0_L0;
	volatile U_VEDU_PME_WINDOW_SIZE1_L0   VEDU_PME_WINDOW_SIZE1_L0;
	volatile U_VEDU_PME_WINDOW_SIZE2_L0   VEDU_PME_WINDOW_SIZE2_L0;
	volatile U_VEDU_PME_WINDOW_SIZE3_L0   VEDU_PME_WINDOW_SIZE3_L0;
	volatile U_VEDU_PME_WINDOW_SIZE0_L1   VEDU_PME_WINDOW_SIZE0_L1;
	volatile U_VEDU_PME_WINDOW_SIZE1_L1   VEDU_PME_WINDOW_SIZE1_L1;
	volatile U_VEDU_PME_WINDOW_SIZE2_L1   VEDU_PME_WINDOW_SIZE2_L1;
	volatile U_VEDU_PME_WINDOW_SIZE3_L1   VEDU_PME_WINDOW_SIZE3_L1;
	volatile U_VEDU_PME_COST_OFFSET   VEDU_PME_COST_OFFSET;
	volatile U_VEDU_PME_SAFE_CFG    VEDU_PME_SAFE_CFG;
	volatile U_VEDU_PME_IBLK_REFRESH   VEDU_PME_IBLK_REFRESH;
	volatile U_VEDU_PME_IBLK_REFRESH_NUM   VEDU_PME_IBLK_REFRESH_NUM;
	volatile U_VEDU_PME_QPG_RC_THR0   VEDU_PME_QPG_RC_THR0;
	volatile U_VEDU_PME_QPG_RC_THR1   VEDU_PME_QPG_RC_THR1;
	volatile U_VEDU_PME_LOW_LUMA_THR   VEDU_PME_LOW_LUMA_THR;
	volatile U_VEDU_PME_PBLK_PRE1   VEDU_PME_PBLK_PRE1;
	volatile U_VEDU_PME_CHROMA_FLAT   VEDU_PME_CHROMA_FLAT;
	volatile U_VEDU_PME_LUMA_FLAT   VEDU_PME_LUMA_FLAT;
	volatile U_VEDU_PME_MADI_FLAT   VEDU_PME_MADI_FLAT;
	volatile U_VEDU_PME_SKIP_LARGE_RES   VEDU_PME_SKIP_LARGE_RES;
	volatile unsigned int    RESERVED_PME[91];
	volatile unsigned int    RESERVED_PMEST[64];
	volatile U_VEDU_QPG_MAX_MIN_QP   VEDU_QPG_MAX_MIN_QP;
	volatile U_VEDU_QPG_ROW_TARGET_BITS   VEDU_QPG_ROW_TARGET_BITS;
	volatile U_VEDU_QPG_AVERAGE_LCU_BITS   VEDU_QPG_AVERAGE_LCU_BITS;
	volatile U_VEDU_QPG_LOWLUMA    VEDU_QPG_LOWLUMA;
	volatile U_VEDU_QPG_HEDGE    VEDU_QPG_HEDGE;
	volatile U_VEDU_QPG_HEDGE_MOVE   VEDU_QPG_HEDGE_MOVE;
	volatile U_VEDU_QPG_LARGE_MOVE   VEDU_QPG_LARGE_MOVE;
	volatile U_VEDU_QPG_SKIN    VEDU_QPG_SKIN;
	volatile U_VEDU_QPG_INTRA_DET   VEDU_QPG_INTRA_DET;
	volatile U_VEDU_QPG_H264_SMOOTH   VEDU_QPG_H264_SMOOTH;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0   VEDU_QPG_CU_QP_DELTA_THRESH_REG0;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1   VEDU_QPG_CU_QP_DELTA_THRESH_REG1;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2   VEDU_QPG_CU_QP_DELTA_THRESH_REG2;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3   VEDU_QPG_CU_QP_DELTA_THRESH_REG3;
	volatile U_VEDU_QPG_DELTA_LEVEL   VEDU_QPG_DELTA_LEVEL;
	volatile U_VEDU_QPG_MADI_SWITCH_THR   VEDU_QPG_MADI_SWITCH_THR;
	volatile U_VEDU_QPG_CU32_DELTA   VEDU_QPG_CU32_DELTA;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG00   VEDU_QPG_QP_LAMBDA_CTRL_REG00;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG01   VEDU_QPG_QP_LAMBDA_CTRL_REG01;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG02   VEDU_QPG_QP_LAMBDA_CTRL_REG02;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG03   VEDU_QPG_QP_LAMBDA_CTRL_REG03;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG04   VEDU_QPG_QP_LAMBDA_CTRL_REG04;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG05   VEDU_QPG_QP_LAMBDA_CTRL_REG05;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG06   VEDU_QPG_QP_LAMBDA_CTRL_REG06;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG07   VEDU_QPG_QP_LAMBDA_CTRL_REG07;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG08   VEDU_QPG_QP_LAMBDA_CTRL_REG08;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG09   VEDU_QPG_QP_LAMBDA_CTRL_REG09;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG10   VEDU_QPG_QP_LAMBDA_CTRL_REG10;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG11   VEDU_QPG_QP_LAMBDA_CTRL_REG11;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG12   VEDU_QPG_QP_LAMBDA_CTRL_REG12;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG13   VEDU_QPG_QP_LAMBDA_CTRL_REG13;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG14   VEDU_QPG_QP_LAMBDA_CTRL_REG14;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG15   VEDU_QPG_QP_LAMBDA_CTRL_REG15;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG16   VEDU_QPG_QP_LAMBDA_CTRL_REG16;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG17   VEDU_QPG_QP_LAMBDA_CTRL_REG17;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG18   VEDU_QPG_QP_LAMBDA_CTRL_REG18;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG19   VEDU_QPG_QP_LAMBDA_CTRL_REG19;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG20   VEDU_QPG_QP_LAMBDA_CTRL_REG20;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG21   VEDU_QPG_QP_LAMBDA_CTRL_REG21;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG22   VEDU_QPG_QP_LAMBDA_CTRL_REG22;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG23   VEDU_QPG_QP_LAMBDA_CTRL_REG23;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG24   VEDU_QPG_QP_LAMBDA_CTRL_REG24;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG25   VEDU_QPG_QP_LAMBDA_CTRL_REG25;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG26   VEDU_QPG_QP_LAMBDA_CTRL_REG26;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG27   VEDU_QPG_QP_LAMBDA_CTRL_REG27;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG28   VEDU_QPG_QP_LAMBDA_CTRL_REG28;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG29   VEDU_QPG_QP_LAMBDA_CTRL_REG29;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG30   VEDU_QPG_QP_LAMBDA_CTRL_REG30;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG31   VEDU_QPG_QP_LAMBDA_CTRL_REG31;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG32   VEDU_QPG_QP_LAMBDA_CTRL_REG32;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG33   VEDU_QPG_QP_LAMBDA_CTRL_REG33;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG34   VEDU_QPG_QP_LAMBDA_CTRL_REG34;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG35   VEDU_QPG_QP_LAMBDA_CTRL_REG35;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG36   VEDU_QPG_QP_LAMBDA_CTRL_REG36;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG37   VEDU_QPG_QP_LAMBDA_CTRL_REG37;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG38   VEDU_QPG_QP_LAMBDA_CTRL_REG38;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG39   VEDU_QPG_QP_LAMBDA_CTRL_REG39;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG40   VEDU_QPG_QP_LAMBDA_CTRL_REG40;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG41   VEDU_QPG_QP_LAMBDA_CTRL_REG41;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG42   VEDU_QPG_QP_LAMBDA_CTRL_REG42;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG43   VEDU_QPG_QP_LAMBDA_CTRL_REG43;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG44   VEDU_QPG_QP_LAMBDA_CTRL_REG44;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG45   VEDU_QPG_QP_LAMBDA_CTRL_REG45;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG46   VEDU_QPG_QP_LAMBDA_CTRL_REG46;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG47   VEDU_QPG_QP_LAMBDA_CTRL_REG47;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG48   VEDU_QPG_QP_LAMBDA_CTRL_REG48;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG49   VEDU_QPG_QP_LAMBDA_CTRL_REG49;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG50   VEDU_QPG_QP_LAMBDA_CTRL_REG50;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG51   VEDU_QPG_QP_LAMBDA_CTRL_REG51;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG52   VEDU_QPG_QP_LAMBDA_CTRL_REG52;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG53   VEDU_QPG_QP_LAMBDA_CTRL_REG53;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG54   VEDU_QPG_QP_LAMBDA_CTRL_REG54;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG55   VEDU_QPG_QP_LAMBDA_CTRL_REG55;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG56   VEDU_QPG_QP_LAMBDA_CTRL_REG56;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG57   VEDU_QPG_QP_LAMBDA_CTRL_REG57;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG58   VEDU_QPG_QP_LAMBDA_CTRL_REG58;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG59   VEDU_QPG_QP_LAMBDA_CTRL_REG59;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG60   VEDU_QPG_QP_LAMBDA_CTRL_REG60;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG61   VEDU_QPG_QP_LAMBDA_CTRL_REG61;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG62   VEDU_QPG_QP_LAMBDA_CTRL_REG62;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG63   VEDU_QPG_QP_LAMBDA_CTRL_REG63;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG64   VEDU_QPG_QP_LAMBDA_CTRL_REG64;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG65   VEDU_QPG_QP_LAMBDA_CTRL_REG65;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG66   VEDU_QPG_QP_LAMBDA_CTRL_REG66;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG67   VEDU_QPG_QP_LAMBDA_CTRL_REG67;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG68   VEDU_QPG_QP_LAMBDA_CTRL_REG68;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG69   VEDU_QPG_QP_LAMBDA_CTRL_REG69;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG70   VEDU_QPG_QP_LAMBDA_CTRL_REG70;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG71   VEDU_QPG_QP_LAMBDA_CTRL_REG71;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG72   VEDU_QPG_QP_LAMBDA_CTRL_REG72;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG73   VEDU_QPG_QP_LAMBDA_CTRL_REG73;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG74   VEDU_QPG_QP_LAMBDA_CTRL_REG74;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG75   VEDU_QPG_QP_LAMBDA_CTRL_REG75;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG76   VEDU_QPG_QP_LAMBDA_CTRL_REG76;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG77   VEDU_QPG_QP_LAMBDA_CTRL_REG77;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG78   VEDU_QPG_QP_LAMBDA_CTRL_REG78;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG79   VEDU_QPG_QP_LAMBDA_CTRL_REG79;
	volatile U_VEDU_QPG_LAMBDA_MODE   VEDU_QPG_LAMBDA_MODE;
	volatile U_VEDU_QPG_QP_RESTRAIN   VEDU_QPG_QP_RESTRAIN;
	volatile unsigned int    VEDU_QPG_CU_MIN_SAD_THRESH_0;
	volatile unsigned int    VEDU_QPG_CU_MIN_SAD_THRESH_1;
	volatile U_VEDU_QPG_CU_MIN_SAD_REG   VEDU_QPG_CU_MIN_SAD_REG;
	volatile U_VEDU_QPG_SMART_REG   VEDU_QPG_SMART_REG;
	volatile U_VEDU_QPG_FLAT_REGION   VEDU_QPG_FLAT_REGION;
	volatile unsigned int    RESERVED_QPG[88];
	volatile unsigned int    RESERVED_REFLD[64];
	volatile U_VEDU_IME_INTER_MODE   VEDU_IME_INTER_MODE;
	volatile U_VEDU_IME_RDOCFG    VEDU_IME_RDOCFG;
	volatile U_VEDU_IME_FME_LPOW_THR   VEDU_IME_FME_LPOW_THR;
	volatile U_VEDU_IME_LAYER3TO2_THR   VEDU_IME_LAYER3TO2_THR;
	volatile U_VEDU_IME_LAYER3TO2_THR1   VEDU_IME_LAYER3TO2_THR1;
	volatile U_VEDU_IME_LAYER3TO1_THR   VEDU_IME_LAYER3TO1_THR;
	volatile U_VEDU_IME_LAYER3TO1_THR1   VEDU_IME_LAYER3TO1_THR1;
	volatile unsigned int    RESERVED_IME[57];
	volatile U_VEDU_FME_BIAS_COST0   VEDU_FME_BIAS_COST0;
	volatile U_VEDU_FME_BIAS_COST1   VEDU_FME_BIAS_COST1;
	volatile U_VEDU_FME_PU64_LWP    VEDU_FME_PU64_LWP;
	volatile unsigned int    RESERVED_FME[61];
	volatile U_VEDU_MRG_FORCE_ZERO_EN   VEDU_MRG_FORCE_ZERO_EN;
	volatile U_VEDU_MRG_FORCE_SKIP_EN   VEDU_MRG_FORCE_SKIP_EN;
	volatile U_VEDU_MRG_BIAS_COST0   VEDU_MRG_BIAS_COST0;
	volatile U_VEDU_MRG_BIAS_COST1   VEDU_MRG_BIAS_COST1;
	volatile U_VEDU_MRG_ABS_OFFSET0   VEDU_MRG_ABS_OFFSET0;
	volatile U_VEDU_MRG_ABS_OFFSET1   VEDU_MRG_ABS_OFFSET1;
	volatile U_VEDU_MRG_ADJ_WEIGHT   VEDU_MRG_ADJ_WEIGHT;
	volatile unsigned int    RESERVED_MRG[57];
	volatile U_VEDU_INTRA_CFG    VEDU_INTRA_CFG;
	volatile U_VEDU_INTRA_SMOOTH    VEDU_INTRA_SMOOTH;
	volatile U_VEDU_INTRA_BIT_WEIGHT   VEDU_INTRA_BIT_WEIGHT;
	volatile U_VEDU_INTRA_RDO_COST_OFFSET_0   VEDU_INTRA_RDO_COST_OFFSET_0;
	volatile U_VEDU_INTRA_RDO_COST_OFFSET_1   VEDU_INTRA_RDO_COST_OFFSET_1;
	volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_0   VEDU_INTRA_NO_DC_COST_OFFSET_0;
	volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_1   VEDU_INTRA_NO_DC_COST_OFFSET_1;
	volatile U_VEDU_INTRA_CHNL4_ANG_0EN   VEDU_INTRA_CHNL4_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL4_ANG_1EN   VEDU_INTRA_CHNL4_ANG_1EN;
	volatile U_VEDU_INTRA_CHNL8_ANG_0EN   VEDU_INTRA_CHNL8_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL8_ANG_1EN   VEDU_INTRA_CHNL8_ANG_1EN;
	volatile U_VEDU_INTRA_CHNL16_ANG_0EN   VEDU_INTRA_CHNL16_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL16_ANG_1EN   VEDU_INTRA_CHNL16_ANG_1EN;
	volatile U_VEDU_INTRA_CHNL32_ANG_0EN   VEDU_INTRA_CHNL32_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL32_ANG_1EN   VEDU_INTRA_CHNL32_ANG_1EN;
	volatile U_VEDU_INTRA_RDO_COST_OFFSET_3   VEDU_INTRA_RDO_COST_OFFSET_3;
	volatile unsigned int    RESERVED_INTRA[48];
	volatile unsigned int    VEDU_PMV_POC_0;
	volatile unsigned int    VEDU_PMV_POC_1;
	volatile unsigned int    VEDU_PMV_POC_2;
	volatile unsigned int    VEDU_PMV_POC_3;
	volatile unsigned int    VEDU_PMV_POC_4;
	volatile unsigned int    VEDU_PMV_POC_5;
	volatile U_VEDU_PMV_TMV_EN    VEDU_PMV_TMV_EN;
	volatile unsigned int    RESERVED_PMV[57];
	volatile U_VEDU_TQITQ_DEADZONE   VEDU_TQITQ_DEADZONE;
	volatile unsigned int    RESERVED_TQITQ[191];
	volatile unsigned int    RESERVED0_SEL[4];
	volatile U_VEDU_SEL_OFFSET_STRENGTH   VEDU_SEL_OFFSET_STRENGTH;
	volatile U_VEDU_SEL_CU32_DC_AC_TH_OFFSET   VEDU_SEL_CU32_DC_AC_TH_OFFSET;
	volatile U_VEDU_SEL_CU32_QP_TH   VEDU_SEL_CU32_QP_TH;
	volatile U_VEDU_SEL_RES_DC_AC_TH   VEDU_SEL_RES_DC_AC_TH;
	volatile unsigned int    RESERVED1_SEL[56];
	volatile unsigned int    RESERVED_DBLK[64];
	volatile unsigned int    VEDU_SAO_SSD_AERA0;
	volatile unsigned int    VEDU_SAO_SSD_AERA1;
	volatile unsigned int    VEDU_SAO_SSD_AERA2;
	volatile unsigned int    VEDU_SAO_SSD_AERA3;
	volatile unsigned int    VEDU_SAO_SSD_AERA4;
	volatile unsigned int    VEDU_SAO_SSD_AERA5;
	volatile unsigned int    VEDU_SAO_SSD_AERA6;
	volatile unsigned int    VEDU_SAO_SSD_AERA7;
	volatile unsigned int    RESERVED_SAO[56];
	volatile U_VEDU_EMAR_WAIT_TIM_OUT   VEDU_EMAR_WAIT_TIM_OUT;
	volatile U_VEDU_EMAR_RCH_RPT_TH0   VEDU_EMAR_RCH_RPT_TH0;
	volatile U_VEDU_EMAR_RCH_RPT_TH1   VEDU_EMAR_RCH_RPT_TH1;
	volatile U_VEDU_EMAR_RCH_RPT_TH2   VEDU_EMAR_RCH_RPT_TH2;
	volatile U_VEDU_EMAR_WCH_RPT_TH0   VEDU_EMAR_WCH_RPT_TH0;
	volatile U_VEDU_EMAR_WCH_RPT_TH1   VEDU_EMAR_WCH_RPT_TH1;
	volatile U_VEDU_EMAR_WCH_RPT_TH2   VEDU_EMAR_WCH_RPT_TH2;
	volatile U_VEDU_EMAR_SCRAMBLE_TYPE   VEDU_EMAR_SCRAMBLE_TYPE;
	volatile unsigned int    RESERVED_EMAR[56];
	volatile unsigned int    VEDU_PACK_SYNTAX_CONFIG;
	volatile U_VEDU_PACK_CU_PARAMETER   VEDU_PACK_CU_PARAMETER;
	volatile U_VEDU_PACK_PCM_PARAMETER   VEDU_PACK_PCM_PARAMETER;
	volatile U_VEDU_PACK_TF_SKIP_FLAG   VEDU_PACK_TF_SKIP_FLAG;
	volatile unsigned int    RESERVED_PACK[60];
	volatile U_VEDU_CABAC_GLB_CFG   VEDU_CABAC_GLB_CFG;
	volatile U_VEDU_CABAC_SLCHDR_SIZE   VEDU_CABAC_SLCHDR_SIZE;
	volatile U_VEDU_CABAC_SLCHDR_PART1   VEDU_CABAC_SLCHDR_PART1;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG1;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG2;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG3;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG4;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG5;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG6;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG7;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG8;
	volatile U_VEDU_CABAC_SLCHDR_SIZE_I   VEDU_CABAC_SLCHDR_SIZE_I;
	volatile U_VEDU_CABAC_SLCHDR_PART1_I   VEDU_CABAC_SLCHDR_PART1_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG1_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG2_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG3_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG4_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG5_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG6_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG7_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG8_I;
	volatile unsigned int    RESERVED_CABAC[43];
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM0;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM1;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM2;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM3;
	volatile unsigned int    VEDU_VLC_REORDERSTRM0;
	volatile unsigned int    VEDU_VLC_REORDERSTRM1;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM0;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM1;
	volatile U_VEDU_VLC_SLCHDRPARA   VEDU_VLC_SLCHDRPARA;
	volatile U_VEDU_VLC_SVC    VEDU_VLC_SVC;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM0_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM1_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM2_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM3_I;
	volatile unsigned int    VEDU_VLC_REORDERSTRM0_I;
	volatile unsigned int    VEDU_VLC_REORDERSTRM1_I;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM0_I;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM1_I;
	volatile U_VEDU_VLC_SLCHDRPARA_I   VEDU_VLC_SLCHDRPARA_I;
	volatile unsigned int    RESERVED_VLC[45];
	volatile unsigned int    RESERVED_VLCST0;
	volatile U_VEDU_VLCST_PTBITS_EN   VEDU_VLCST_PTBITS_EN;
	volatile unsigned int    VEDU_VLCST_PTBITS;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN0;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN1;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN2;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN3;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN4;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN5;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN6;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN7;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN8;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN9;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN10;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN11;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN12;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN13;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN14;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN15;
	volatile unsigned int    VEDU_VLCST_STRMADDR0;
	volatile unsigned int    VEDU_VLCST_STRMADDR1;
	volatile unsigned int    VEDU_VLCST_STRMADDR2;
	volatile unsigned int    VEDU_VLCST_STRMADDR3;
	volatile unsigned int    VEDU_VLCST_STRMADDR4;
	volatile unsigned int    VEDU_VLCST_STRMADDR5;
	volatile unsigned int    VEDU_VLCST_STRMADDR6;
	volatile unsigned int    VEDU_VLCST_STRMADDR7;
	volatile unsigned int    VEDU_VLCST_STRMADDR8;
	volatile unsigned int    VEDU_VLCST_STRMADDR9;
	volatile unsigned int    VEDU_VLCST_STRMADDR10;
	volatile unsigned int    VEDU_VLCST_STRMADDR11;
	volatile unsigned int    VEDU_VLCST_STRMADDR12;
	volatile unsigned int    VEDU_VLCST_STRMADDR13;
	volatile unsigned int    VEDU_VLCST_STRMADDR14;
	volatile unsigned int    VEDU_VLCST_STRMADDR15;
	volatile unsigned int    VEDU_VLCST_PARA_ADDR;
	volatile U_VEDU_VLCST_PARAMETER   VEDU_VLCST_PARAMETER;
	volatile unsigned int    VEDU_VLCST_PARA_DATA0;
	volatile unsigned int    VEDU_VLCST_PARA_DATA1;
	volatile unsigned int    VEDU_VLCST_PARA_DATA2;
	volatile unsigned int    VEDU_VLCST_PARA_DATA3;
	volatile unsigned int    VEDU_VLCST_PARA_DATA4;
	volatile unsigned int    VEDU_VLCST_PARA_DATA5;
	volatile unsigned int    VEDU_VLCST_PARA_DATA6;
	volatile unsigned int    VEDU_VLCST_PARA_DATA7;
	volatile unsigned int    VEDU_VLCST_PARA_DATA8;
	volatile unsigned int    VEDU_VLCST_PARA_DATA9;
	volatile unsigned int    VEDU_VLCST_PARA_DATA10;
	volatile unsigned int    VEDU_VLCST_PARA_DATA11;
	volatile unsigned int    VEDU_VLCST_PARA_DATA12;
	volatile unsigned int    VEDU_VLCST_PARA_DATA13;
	volatile unsigned int    VEDU_VLCST_PARA_DATA14;
	volatile unsigned int    VEDU_VLCST_PARA_DATA15;
	volatile unsigned int    VEDU_VLCST_PARA_DATA16;
	volatile unsigned int    VEDU_VLCST_PARA_DATA17;
	volatile unsigned int    VEDU_VLCST_PARA_DATA18;
	volatile unsigned int    VEDU_VLCST_PARA_DATA19;
	volatile unsigned int    VEDU_VLCST_PARA_DATA20;
	volatile unsigned int    VEDU_VLCST_PARA_DATA21;
	volatile unsigned int    VEDU_VLCST_PARA_DATA22;
	volatile unsigned int    VEDU_VLCST_PARA_DATA23;
	volatile unsigned int    VEDU_VLCST_PARA_DATA24;
	volatile unsigned int    VEDU_VLCST_PARA_DATA25;
	volatile unsigned int    VEDU_VLCST_PARA_DATA26;
	volatile unsigned int    VEDU_VLCST_PARA_DATA27;
	volatile unsigned int    VEDU_VLCST_PARA_DATA28;
	volatile unsigned int    VEDU_VLCST_PARA_DATA29;
	volatile unsigned int    VEDU_VLCST_PARA_DATA30;
	volatile unsigned int    VEDU_VLCST_PARA_DATA31;
	volatile unsigned int    VEDU_VLCST_PARA_DATA32;
	volatile unsigned int    VEDU_VLCST_PARA_DATA33;
	volatile unsigned int    VEDU_VLCST_PARA_DATA34;
	volatile unsigned int    VEDU_VLCST_PARA_DATA35;
	volatile unsigned int    VEDU_VLCST_PARA_DATA36;
	volatile unsigned int    VEDU_VLCST_PARA_DATA37;
	volatile unsigned int    VEDU_VLCST_PARA_DATA38;
	volatile unsigned int    VEDU_VLCST_PARA_DATA39;
	volatile unsigned int    VEDU_VLCST_PARA_DATA40;
	volatile unsigned int    VEDU_VLCST_PARA_DATA41;
	volatile unsigned int    VEDU_VLCST_PARA_DATA42;
	volatile unsigned int    VEDU_VLCST_PARA_DATA43;
	volatile unsigned int    VEDU_VLCST_PARA_DATA44;
	volatile unsigned int    VEDU_VLCST_PARA_DATA45;
	volatile unsigned int    VEDU_VLCST_PARA_DATA46;
	volatile unsigned int    VEDU_VLCST_PARA_DATA47;
	volatile unsigned int    RESERVED_VLCST1[43];
	volatile unsigned int    VEDU_PPFD_ST_ADDR0;
	volatile unsigned int    VEDU_PPFD_ST_ADDR1;
	volatile unsigned int    VEDU_PPFD_ST_LEN0;
	volatile unsigned int    VEDU_PPFD_ST_LEN1;
	volatile U_VEDU_PPFD_ST_CFG    VEDU_PPFD_ST_CFG;
	volatile unsigned int    RESERVED_PPFD[59];
	volatile U_VEDU_ENV_CHN    VEDU_ENV_CHN;
	volatile unsigned int    RESERVED_ENV[1087];
	volatile U_FUNC_VCPI_INTSTAT    FUNC_VCPI_INTSTAT;
	volatile U_FUNC_VCPI_RAWINT     FUNC_VCPI_RAWINT;
	volatile unsigned int    FUNC_VCPI_VEDU_TIMER;
	volatile unsigned int    FUNC_VCPI_IDLE_TIMER;
	volatile U_FUNC_VCPI_INTSTAT_S   FUNC_VCPI_INTSTAT_S;
	volatile U_FUNC_VCPI_RAWINT_S   FUNC_VCPI_RAWINT_S;
	volatile unsigned int    RESERVED_FUNC_VCPI[10];
	volatile unsigned int    RESERVED_FUNC_VCTRL[16];
	volatile unsigned int    RESERVED_FUNC_QPGLD[16];
	volatile unsigned int    RESERVED_FUNC_CURLD[16];
	volatile unsigned int    RESERVED_FUNC_NBI[16];
	volatile unsigned int    RESERVED_FUNC_PMELD[16];
	volatile unsigned int    RESERVED_FUNC_PMEINFO_LD[16];
	volatile U_FUNC_PME_MADI_SUM    FUNC_PME_MADI_SUM;
	volatile U_FUNC_PME_MADP_SUM    FUNC_PME_MADP_SUM;
	volatile U_FUNC_PME_MADI_NUM    FUNC_PME_MADI_NUM;
	volatile U_FUNC_PME_MADP_NUM    FUNC_PME_MADP_NUM;
	volatile unsigned int    RESERVED_FUNC_PME[12];
	volatile unsigned int    RESERVED_FUNC_PMEST[16];
	volatile unsigned int    RESERVED_FUNC_PMEINFO_ST[16];
	volatile U_FUNC_BGGEN_BLOCK_COUNT   FUNC_BGGEN_BLOCK_COUNT;
	volatile U_FUNC_BGGEN_FRAME_BGM_DIST   FUNC_BGGEN_FRAME_BGM_DIST;
	volatile unsigned int    RESERVED_FUNC_BGST[14];
	volatile unsigned int    RESERVED_FUNC_QPG[16];
	volatile unsigned int    RESERVED_FUNC_REFLD[16];
	volatile unsigned int    RESERVED_FUNC_PINTRA[16];
	volatile unsigned int    RESERVED_FUNC_IME[16];
	volatile unsigned int    RESERVED_FUNC_FME[16];
	volatile unsigned int    RESERVED_FUNC_MRG0[16];
	volatile unsigned int    RESERVED_FUNC_MRG1[16];
	volatile unsigned int    RESERVED_FUNC_INTRA0[16];
	volatile unsigned int    RESERVED_FUNC_INTRA1[16];
	volatile unsigned int    RESERVED_FUNC_PMV0[16];
	volatile unsigned int    RESERVED_FUNC_PMV1[16];
	volatile unsigned int    RESERVED_FUNC_TQITQ0[16];
	volatile unsigned int    RESERVED_FUNC_TQITQ1[16];
	volatile unsigned int    RESERVED_FUNC_XXX0[16];
	volatile unsigned int    RESERVED_FUNC_LFLDST[16];
	volatile unsigned int    RESERVED_FUNC_DBLK[16];
	volatile unsigned int    RESERVED_FUNC_XXX1[16];
	volatile unsigned int    RESERVED_FUNC_RECST[16];
	volatile unsigned int    RESERVED_FUNC_PACK0[16];
	volatile unsigned int    RESERVED_FUNC_PACK1[16];
	volatile unsigned int    FUNC_CABAC_PIC_STRMSIZE;
	volatile unsigned int    FUNC_CABAC_BIT_NUM;
	volatile unsigned int    RESERVED_FUNC_CABAC[15];
	volatile unsigned int    VLC_SLC_TTBITS;
	volatile unsigned int    VLC_PIC_TTBITS;
	volatile unsigned int    RESERVED_FUNC_VLC[13];
	volatile unsigned int    FUNC_VLCST_SLC_LEN_CNT;
	volatile U_FUNC_VLCST_DSRPTR00   FUNC_VLCST_DSRPTR00;
	volatile U_FUNC_VLCST_DSRPTR01   FUNC_VLCST_DSRPTR01;
	volatile U_FUNC_VLCST_DSRPTR10   FUNC_VLCST_DSRPTR10;
	volatile U_FUNC_VLCST_DSRPTR11   FUNC_VLCST_DSRPTR11;
	volatile U_FUNC_VLCST_DSRPTR20   FUNC_VLCST_DSRPTR20;
	volatile U_FUNC_VLCST_DSRPTR21   FUNC_VLCST_DSRPTR21;
	volatile U_FUNC_VLCST_DSRPTR30   FUNC_VLCST_DSRPTR30;
	volatile U_FUNC_VLCST_DSRPTR31   FUNC_VLCST_DSRPTR31;
	volatile U_FUNC_VLCST_DSRPTR40   FUNC_VLCST_DSRPTR40;
	volatile U_FUNC_VLCST_DSRPTR41   FUNC_VLCST_DSRPTR41;
	volatile U_FUNC_VLCST_DSRPTR50   FUNC_VLCST_DSRPTR50;
	volatile U_FUNC_VLCST_DSRPTR51   FUNC_VLCST_DSRPTR51;
	volatile U_FUNC_VLCST_DSRPTR60   FUNC_VLCST_DSRPTR60;
	volatile U_FUNC_VLCST_DSRPTR61   FUNC_VLCST_DSRPTR61;
	volatile U_FUNC_VLCST_DSRPTR70   FUNC_VLCST_DSRPTR70;
	volatile U_FUNC_VLCST_DSRPTR71   FUNC_VLCST_DSRPTR71;
	volatile U_FUNC_VLCST_DSRPTR80   FUNC_VLCST_DSRPTR80;
	volatile U_FUNC_VLCST_DSRPTR81   FUNC_VLCST_DSRPTR81;
	volatile U_FUNC_VLCST_DSRPTR90   FUNC_VLCST_DSRPTR90;
	volatile U_FUNC_VLCST_DSRPTR91   FUNC_VLCST_DSRPTR91;
	volatile U_FUNC_VLCST_DSRPTR100   FUNC_VLCST_DSRPTR100;
	volatile U_FUNC_VLCST_DSRPTR101   FUNC_VLCST_DSRPTR101;
	volatile U_FUNC_VLCST_DSRPTR110   FUNC_VLCST_DSRPTR110;
	volatile U_FUNC_VLCST_DSRPTR111   FUNC_VLCST_DSRPTR111;
	volatile U_FUNC_VLCST_DSRPTR120   FUNC_VLCST_DSRPTR120;
	volatile U_FUNC_VLCST_DSRPTR121   FUNC_VLCST_DSRPTR121;
	volatile U_FUNC_VLCST_DSRPTR130   FUNC_VLCST_DSRPTR130;
	volatile U_FUNC_VLCST_DSRPTR131   FUNC_VLCST_DSRPTR131;
	volatile U_FUNC_VLCST_DSRPTR140   FUNC_VLCST_DSRPTR140;
	volatile U_FUNC_VLCST_DSRPTR141   FUNC_VLCST_DSRPTR141;
	volatile U_FUNC_VLCST_DSRPTR150   FUNC_VLCST_DSRPTR150;
	volatile U_FUNC_VLCST_DSRPTR151   FUNC_VLCST_DSRPTR151;
	volatile unsigned int    RESERVED_FUNC_VLCST[95];
	volatile unsigned int    RESERVED_FUNC_EMAR[16];
	volatile unsigned int    RESERVED_FUNC_PPFD[16];
	volatile U_FUNC_SEL_OPT_8X8_CNT   FUNC_SEL_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTRA_OPT_8X8_CNT   FUNC_SEL_INTRA_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT   FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT   FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_OPT_8X8_CNT   FUNC_SEL_INTER_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_8X8_CNT   FUNC_SEL_INTER_FME_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_8X8_CNT   FUNC_SEL_INTER_MERGE_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT   FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
	volatile U_FUNC_SEL_OPT_16X16_CNT   FUNC_SEL_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTRA_OPT_16X16_CNT   FUNC_SEL_INTRA_OPT_16X16_CNT;
	volatile U_FUNC_SEL_OPT_4X4_CNT   FUNC_SEL_OPT_4X4_CNT;
	volatile unsigned int    RESERVED0_FUNC_SEL;
	volatile U_FUNC_SEL_INTER_OPT_16X16_CNT   FUNC_SEL_INTER_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_16X16_CNT   FUNC_SEL_INTER_FME_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT   FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT   FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
	volatile U_FUNC_SEL_OPT_32X32_CNT   FUNC_SEL_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTRA_OPT_32X32_CNT   FUNC_SEL_INTRA_OPT_32X32_CNT;
	volatile unsigned int    RESERVED1_FUNC_SEL;
	volatile U_FUNC_SEL_INTER_OPT_32X32_CNT   FUNC_SEL_INTER_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_32X32_CNT   FUNC_SEL_INTER_FME_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT   FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT   FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
	volatile U_FUNC_SEL_OPT_64X64_CNT   FUNC_SEL_OPT_64X64_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_64X64_CNT   FUNC_SEL_INTER_FME_OPT_64X64_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT   FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT   FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
	volatile U_FUNC_SEL_TOTAL_LUMA_QP   FUNC_SEL_TOTAL_LUMA_QP;
	volatile U_FUNC_SEL_MAX_MIN_LUMA_QP   FUNC_SEL_MAX_MIN_LUMA_QP;
	volatile unsigned int    RESERVED2_FUNC_SEL[35];
	volatile U_FUNC_SEL_LUMA_QP0_CNT   FUNC_SEL_LUMA_QP0_CNT;
	volatile U_FUNC_SEL_LUMA_QP1_CNT   FUNC_SEL_LUMA_QP1_CNT;
	volatile U_FUNC_SEL_LUMA_QP2_CNT   FUNC_SEL_LUMA_QP2_CNT;
	volatile U_FUNC_SEL_LUMA_QP3_CNT   FUNC_SEL_LUMA_QP3_CNT;
	volatile U_FUNC_SEL_LUMA_QP4_CNT   FUNC_SEL_LUMA_QP4_CNT;
	volatile U_FUNC_SEL_LUMA_QP5_CNT   FUNC_SEL_LUMA_QP5_CNT;
	volatile U_FUNC_SEL_LUMA_QP6_CNT   FUNC_SEL_LUMA_QP6_CNT;
	volatile U_FUNC_SEL_LUMA_QP7_CNT   FUNC_SEL_LUMA_QP7_CNT;
	volatile U_FUNC_SEL_LUMA_QP8_CNT   FUNC_SEL_LUMA_QP8_CNT;
	volatile U_FUNC_SEL_LUMA_QP9_CNT   FUNC_SEL_LUMA_QP9_CNT;
	volatile U_FUNC_SEL_LUMA_QP10_CNT   FUNC_SEL_LUMA_QP10_CNT;
	volatile U_FUNC_SEL_LUMA_QP11_CNT   FUNC_SEL_LUMA_QP11_CNT;
	volatile U_FUNC_SEL_LUMA_QP12_CNT   FUNC_SEL_LUMA_QP12_CNT;
	volatile U_FUNC_SEL_LUMA_QP13_CNT   FUNC_SEL_LUMA_QP13_CNT;
	volatile U_FUNC_SEL_LUMA_QP14_CNT   FUNC_SEL_LUMA_QP14_CNT;
	volatile U_FUNC_SEL_LUMA_QP15_CNT   FUNC_SEL_LUMA_QP15_CNT;
	volatile U_FUNC_SEL_LUMA_QP16_CNT   FUNC_SEL_LUMA_QP16_CNT;
	volatile U_FUNC_SEL_LUMA_QP17_CNT   FUNC_SEL_LUMA_QP17_CNT;
	volatile U_FUNC_SEL_LUMA_QP18_CNT   FUNC_SEL_LUMA_QP18_CNT;
	volatile U_FUNC_SEL_LUMA_QP19_CNT   FUNC_SEL_LUMA_QP19_CNT;
	volatile U_FUNC_SEL_LUMA_QP20_CNT   FUNC_SEL_LUMA_QP20_CNT;
	volatile U_FUNC_SEL_LUMA_QP21_CNT   FUNC_SEL_LUMA_QP21_CNT;
	volatile U_FUNC_SEL_LUMA_QP22_CNT   FUNC_SEL_LUMA_QP22_CNT;
	volatile U_FUNC_SEL_LUMA_QP23_CNT   FUNC_SEL_LUMA_QP23_CNT;
	volatile U_FUNC_SEL_LUMA_QP24_CNT   FUNC_SEL_LUMA_QP24_CNT;
	volatile U_FUNC_SEL_LUMA_QP25_CNT   FUNC_SEL_LUMA_QP25_CNT;
	volatile U_FUNC_SEL_LUMA_QP26_CNT   FUNC_SEL_LUMA_QP26_CNT;
	volatile U_FUNC_SEL_LUMA_QP27_CNT   FUNC_SEL_LUMA_QP27_CNT;
	volatile U_FUNC_SEL_LUMA_QP28_CNT   FUNC_SEL_LUMA_QP28_CNT;
	volatile U_FUNC_SEL_LUMA_QP29_CNT   FUNC_SEL_LUMA_QP29_CNT;
	volatile U_FUNC_SEL_LUMA_QP30_CNT   FUNC_SEL_LUMA_QP30_CNT;
	volatile U_FUNC_SEL_LUMA_QP31_CNT   FUNC_SEL_LUMA_QP31_CNT;
	volatile U_FUNC_SEL_LUMA_QP32_CNT   FUNC_SEL_LUMA_QP32_CNT;
	volatile U_FUNC_SEL_LUMA_QP33_CNT   FUNC_SEL_LUMA_QP33_CNT;
	volatile U_FUNC_SEL_LUMA_QP34_CNT   FUNC_SEL_LUMA_QP34_CNT;
	volatile U_FUNC_SEL_LUMA_QP35_CNT   FUNC_SEL_LUMA_QP35_CNT;
	volatile U_FUNC_SEL_LUMA_QP36_CNT   FUNC_SEL_LUMA_QP36_CNT;
	volatile U_FUNC_SEL_LUMA_QP37_CNT   FUNC_SEL_LUMA_QP37_CNT;
	volatile U_FUNC_SEL_LUMA_QP38_CNT   FUNC_SEL_LUMA_QP38_CNT;
	volatile U_FUNC_SEL_LUMA_QP39_CNT   FUNC_SEL_LUMA_QP39_CNT;
	volatile U_FUNC_SEL_LUMA_QP40_CNT   FUNC_SEL_LUMA_QP40_CNT;
	volatile U_FUNC_SEL_LUMA_QP41_CNT   FUNC_SEL_LUMA_QP41_CNT;
	volatile U_FUNC_SEL_LUMA_QP42_CNT   FUNC_SEL_LUMA_QP42_CNT;
	volatile U_FUNC_SEL_LUMA_QP43_CNT   FUNC_SEL_LUMA_QP43_CNT;
	volatile U_FUNC_SEL_LUMA_QP44_CNT   FUNC_SEL_LUMA_QP44_CNT;
	volatile U_FUNC_SEL_LUMA_QP45_CNT   FUNC_SEL_LUMA_QP45_CNT;
	volatile U_FUNC_SEL_LUMA_QP46_CNT   FUNC_SEL_LUMA_QP46_CNT;
	volatile U_FUNC_SEL_LUMA_QP47_CNT   FUNC_SEL_LUMA_QP47_CNT;
	volatile U_FUNC_SEL_LUMA_QP48_CNT   FUNC_SEL_LUMA_QP48_CNT;
	volatile U_FUNC_SEL_LUMA_QP49_CNT   FUNC_SEL_LUMA_QP49_CNT;
	volatile U_FUNC_SEL_LUMA_QP50_CNT   FUNC_SEL_LUMA_QP50_CNT;
	volatile U_FUNC_SEL_LUMA_QP51_CNT   FUNC_SEL_LUMA_QP51_CNT;
	volatile unsigned int    RESERVED3_FUNC_SEL[140];
	volatile unsigned int    FUNC_SAO_MSE_SUM;
	volatile U_FUNC_SAO_MSE_CNT    FUNC_SAO_MSE_CNT;
	volatile U_FUNC_SAO_MSE_MAX    FUNC_SAO_MSE_MAX;
	volatile unsigned int    FUNC_SAO_SSD_AERA0_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA1_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA2_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA3_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA4_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA5_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA6_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA7_SUM;
	volatile unsigned int    RESERVED_FUNC_0SAO[8];
	volatile U_FUNC_SAO_OFF_NUM    FUNC_SAO_OFF_NUM;
	volatile U_FUNC_SAO_LCU_CNT    FUNC_SAO_LCU_CNT;
	volatile unsigned int    RESERVED_FUNC_1SAO[11];
	volatile unsigned int    RESERVED_FUNC[48];
	volatile unsigned int    RESERVED_MIDDLE[4096];

	volatile U_MMU_PRE_GLB_SCR    MMU_PRE_GLB_SCR;  //8000
	volatile unsigned int    MMU_PRE_RESERVED_0[3];
	volatile unsigned int    MMU_PRE_NBI_MVST_ADDR_STR;
	volatile unsigned int    MMU_PRE_NBI_MVST_ADDR_END;
	volatile unsigned int    MMU_PRE_NBI_MVLD_ADDR_STR;
	volatile unsigned int    MMU_PRE_NBI_MVLD_ADDR_END;
	volatile unsigned int    MMU_PRE_PMEST_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEST_ADDR_END;
	volatile unsigned int    MMU_PRE_PMELD_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMELD_ADDR_END;
	volatile unsigned int    MMU_PRE_PMEINFOST_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEINFOST_ADDR_END;
	volatile unsigned int    MMU_PRE_PMEINFOLD0_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEINFOLD0_ADDR_END;
	volatile unsigned int    MST_PRE_RESERVED_1[2];
	volatile unsigned int    MMU_PRE_PMEINFOLD1_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEINFOLD1_ADDR_END;
	volatile unsigned int    MMU_PRE_QPGLD_ADDR_STR;
	volatile unsigned int    MMU_PRE_QPGLD_ADDR_END;
	volatile unsigned int    MMU_PRE_REC_YH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REC_YH_ADDR_END;
	volatile unsigned int    MMU_PRE_REC_CH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REC_CH_ADDR_END;
	volatile unsigned int    MMU_PRE_REC_YADDR_STR;
	volatile unsigned int    MMU_PRE_REC_YADDR_END;
	volatile unsigned int    MMU_PRE_REC_CADDR_STR;
	volatile unsigned int    MMU_PRE_REC_CADDR_END;
	volatile unsigned int    MMU_PRE_REF_YH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REF_YH_ADDR_END;
	volatile unsigned int    MMU_PRE_REF_CH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REF_CH_ADDR_END;
	volatile unsigned int    MMU_PRE_REF_YADDR_STR;
	volatile unsigned int    MMU_PRE_REF_YADDR_END;
	volatile unsigned int    MMU_PRE_REF_CADDR_STR;
	volatile unsigned int    MMU_PRE_REF_CADDR_END;
	volatile unsigned int    MMU_PRE_SRC_YHADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_YHADDR_END;
	volatile unsigned int    MMU_PRE_SRC_CHADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_CHADDR_END;
	volatile unsigned int    MMU_PRE_SRC_YADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_YADDR_END;
	volatile unsigned int    MMU_PRE_SRC_CADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_CADDR_END;
	volatile unsigned int    MMU_PRE_SRC_VADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_VADDR_END;
	volatile unsigned int    MMU_PRE_LOWDLY_ADDR_STR;
	volatile unsigned int    MMU_PRE_LOWDLY_ADDR_END;
	volatile unsigned int    MMU_PRE_PPS_ADDR_STR;
	volatile unsigned int    MMU_PRE_PPS_ADDR_END;
	volatile unsigned int    MMU_PRE_STRMADDR0_STR;
	volatile unsigned int    MMU_PRE_STRMADDR0_END;
	volatile unsigned int    MMU_PRE_STRMADDR1_STR;
	volatile unsigned int    MMU_PRE_STRMADDR1_END;
	volatile unsigned int    MMU_PRE_STRMADDR2_STR;
	volatile unsigned int    MMU_PRE_STRMADDR2_END;
	volatile unsigned int    MMU_PRE_STRMADDR3_STR;
	volatile unsigned int    MMU_PRE_STRMADDR3_END;
	volatile unsigned int    MMU_PRE_STRMADDR4_STR;
	volatile unsigned int    MMU_PRE_STRMADDR4_END;
	volatile unsigned int    MMU_PRE_STRMADDR5_STR;
	volatile unsigned int    MMU_PRE_STRMADDR5_END;
	volatile unsigned int    MMU_PRE_STRMADDR6_STR;
	volatile unsigned int    MMU_PRE_STRMADDR6_END;
	volatile unsigned int    MMU_PRE_STRMADDR7_STR;
	volatile unsigned int    MMU_PRE_STRMADDR7_END;
	volatile unsigned int    MMU_PRE_STRMADDR8_STR;
	volatile unsigned int    MMU_PRE_STRMADDR8_END;
	volatile unsigned int    MMU_PRE_STRMADDR9_STR;
	volatile unsigned int    MMU_PRE_STRMADDR9_END;
	volatile unsigned int    MMU_PRE_STRMADDR10_STR;
	volatile unsigned int    MMU_PRE_STRMADDR10_END;
	volatile unsigned int    MMU_PRE_STRMADDR11_STR;
	volatile unsigned int    MMU_PRE_STRMADDR11_END;
	volatile unsigned int    MMU_PRE_STRMADDR12_STR;
	volatile unsigned int    MMU_PRE_STRMADDR12_END;
	volatile unsigned int    MMU_PRE_STRMADDR13_STR;
	volatile unsigned int    MMU_PRE_STRMADDR13_END;
	volatile unsigned int    MMU_PRE_STRMADDR14_STR;
	volatile unsigned int    MMU_PRE_STRMADDR14_END;
	volatile unsigned int    MMU_PRE_STRMADDR15_STR;
	volatile unsigned int    MMU_PRE_STRMADDR15_END;
	volatile U_MMU_PRE_DFX_ARERR_FLAG   MMU_PRE_DFX_ARERR_FLAG;
	volatile U_MMU_PRE_DFX_ARERR_ID   MMU_PRE_DFX_ARERR_ID;
	volatile unsigned int    MMU_PRE_DFX_ARERR_ADDR;
	volatile unsigned int    MST_PRE_RESERVED_2;
	volatile U_MMU_PRE_DFX_AWERR_FLAG   MMU_PRE_DFX_AWERR_FLAG;
	volatile U_MMU_PRE_DFX_AWERR_ID   MMU_PRE_DFX_AWERR_ID;
	volatile unsigned int    MMU_PRE_DFX_AWERR_ADDR;
	volatile unsigned int    MMU_PRE_RESERVED_3[933];
	volatile U_AXIDFX_ERR    AXIDFX_ERR;  //9000
	volatile unsigned int    RESERVED_1[3];
	volatile U_AXIDFX_AR_R_CNT    AXIDFX_AR_R_CNT;
	volatile U_AXIDFX_AW_W_CNT    AXIDFX_AW_W_CNT;
	volatile U_AXIDFX_AW_B_CNT    AXIDFX_AW_B_CNT;
	volatile unsigned int    RESERVED_2;
	volatile U_AXIDFX_AR_R_ID_ERR   AXIDFX_AR_R_ID_ERR;
	volatile U_AXIDFX_ERR_ARID    AXIDFX_ERR_ARID;
	volatile U_AXIDFX_ERR_RID    AXIDFX_ERR_RID;
	volatile unsigned int    RESERVED_3;
	volatile U_AXIDFX_AW_W_B_ID_ERR   AXIDFX_AW_W_B_ID_ERR;
	volatile U_AXIDFX_ERR_AWID    AXIDFX_ERR_AWID;
	volatile U_AXIDFX_ERR_WID    AXIDFX_ERR_WID;
	volatile U_AXIDFX_ERR_BID    AXIDFX_ERR_BID;
	volatile U_AXIDFX_ARID_TX_0ERR   AXIDFX_ARID_TX_0ERR;
	volatile U_AXIDFX_ARID_TX_1ERR   AXIDFX_ARID_TX_1ERR;
	volatile U_AXIDFX_ARID_TX_2ERR   AXIDFX_ARID_TX_2ERR;
	volatile unsigned int    RESERVED_4;
	volatile U_AXIDFX_RID_RX_0ERR   AXIDFX_RID_RX_0ERR;
	volatile U_AXIDFX_RID_RX_1ERR   AXIDFX_RID_RX_1ERR;
	volatile U_AXIDFX_RID_RX_2ERR   AXIDFX_RID_RX_2ERR;
	volatile unsigned int    RESERVED_5;
	volatile U_AXIDFX_ARID_RX_0ERR   AXIDFX_ARID_RX_0ERR;
	volatile U_AXIDFX_BID_RX_ERR    AXIDFX_BID_RX_ERR;
	volatile unsigned int    RESERVED_6[2];
	volatile U_AXIDFX_ARID_LEN_0ERR   AXIDFX_ARID_LEN_0ERR;
	volatile U_AXIDFX_ARID_LEN_1ERR   AXIDFX_ARID_LEN_1ERR;
	volatile U_AXIDFX_ARID_LEN_2ERR   AXIDFX_ARID_LEN_2ERR;
	volatile unsigned int    RESERVED_7;
	volatile unsigned int    AXIDFX_AWLEN_CNT;
	volatile unsigned int    AXIDFX_WLEN_CNT;
	volatile unsigned int    RESERVED_8[2];
	volatile U_AXIDFX_RESP_ERR    AXIDFX_RESP_ERR;
	volatile U_AXIDFX_ERR_RESP    AXIDFX_ERR_RESP;
	volatile U_AXIDFX_LEN_ERR    AXIDFX_LEN_ERR;
	volatile U_AXIDFX_ERR_LEN    AXIDFX_ERR_LEN;
	volatile unsigned int    AXIDFX_0RID_FLAG;
	volatile unsigned int    AXIDFX_1RID_FLAG;
	volatile U_AXIDFX_2RID_FLAG    AXIDFX_2RID_FLAG;
	volatile U_AXIDFX_WID_FLAG    AXIDFX_WID_FLAG;
	volatile U_AXIDFX_AXI_ST    AXIDFX_AXI_ST;
	volatile unsigned int    RESERVED_9[19];
	volatile U_AXIDFX_SOFT_RST_REQ   AXIDFX_SOFT_RST_REQ;
	volatile U_AXIDFX_SOFT_RST_ACK   AXIDFX_SOFT_RST_ACK;
	volatile unsigned int    RESERVED_11[2];
	volatile U_AXIDFX_SOFT_RST_FORCE_REQ_ACK   AXIDFX_SOFT_RST_FORCE_REQ_ACK;
	volatile unsigned int    AXIDFX_SOFT_RST_STATE0;
	volatile unsigned int    AXIDFX_SOFT_RST_STATE1;
	volatile unsigned int    RESERVED_12[7097];
	volatile U_SMMU_MSTR_GLB_BYPASS   SMMU_MSTR_GLB_BYPASS;  //30000
	volatile U_SMMU_MSTR_DEBUG_MODE   SMMU_MSTR_DEBUG_MODE;
	volatile U_SMMU_MSTR_MEM_CTRL   SMMU_MSTR_MEM_CTRL;
	volatile U_SMMU_MSTR_CLK_EN    SMMU_MSTR_CLK_EN;
	volatile unsigned int    SMMU_MSTR_END_REQ_0;
	volatile unsigned int    SMMU_MSTR_END_REQ_1;
	volatile U_SMMU_MSTR_END_REQ_2   SMMU_MSTR_END_REQ_2;
	volatile unsigned int    SMMU_MSTR_END_ACK_0;
	volatile unsigned int    SMMU_MSTR_END_ACK_1;
	volatile U_SMMU_MSTR_END_ACK_2   SMMU_MSTR_END_ACK_2;
	volatile unsigned int    SMMU_MSTR_SMRX_START_0;
	volatile unsigned int    SMMU_MSTR_SMRX_START_1;
	volatile U_SMMU_MSTR_SMRX_START_2   SMMU_MSTR_SMRX_START_2;
	volatile U_SMMU_MSTR_INPT_SEL   SMMU_MSTR_INPT_SEL;
	volatile unsigned int    SMMU_MSTR_RESERVED_0[2];
	volatile U_SMMU_MSTR_INTMASK    SMMU_MSTR_INTMASK;
	volatile U_SMMU_MSTR_INTRAW    SMMU_MSTR_INTRAW;
	volatile U_SMMU_MSTR_INTSTAT    SMMU_MSTR_INTSTAT;
	volatile U_SMMU_MSTR_INTCLR    SMMU_MSTR_INTCLR;
	volatile U_SMMU_MSTR_DBG_0    SMMU_MSTR_DBG_0;
	volatile unsigned int    SMMU_MSTR_DBG_1;
	volatile U_SMMU_MSTR_DBG_2    SMMU_MSTR_DBG_2;
	volatile unsigned int    SMMU_MSTR_DBG_3;
	volatile U_SMMU_MSTR_DBG_4    SMMU_MSTR_DBG_4;
	volatile unsigned int    SMMU_MSTR_DBG_5;
	volatile unsigned int    SMMU_MSTR_RESERVED_1[2];
	volatile U_SMMU_MSTR_DBG_PORT_IN_0   SMMU_MSTR_DBG_PORT_IN_0;
	volatile unsigned int    SMMU_MSTR_DBG_PORT_IN_1;
	volatile unsigned int    SMMU_MSTR_DBG_PORT_OUT;
	volatile unsigned int    SMMU_MSTR_RESERVED_2[33];
	volatile U_SMMU_MSTR_SMRX_0    SMMU_MSTR_SMRX_0[88];
	volatile U_SMMU_MSTR_SMRX_1    SMMU_MSTR_SMRX_1[88];
	volatile U_SMMU_MSTR_SMRX_2    SMMU_MSTR_SMRX_2[144];
	volatile unsigned int    RD_CMD_TOTAL_CNT[88];
	volatile unsigned int    RD_CMD_MISS_CNT[88];
	volatile unsigned int    RD_DATA_TOTAL_CNT[88];
	volatile unsigned int    RD_CMD_CASE_CNT[6];
	volatile U_RD_CMD_TRANS_LATENCY   RD_CMD_TRANS_LATENCY[50];
	volatile unsigned int    WR_CMD_TOTAL_CNT[88];
	volatile unsigned int    WR_CMD_MISS_CNT[88];
	volatile unsigned int    WR_DATA_TOTAL_CNT[88];
	volatile unsigned int    WR_CMD_CASE_CNT[6];
	volatile U_WR_CMD_TRANS_LATENCY   WR_CMD_TRANS_LATENCY[50];
	volatile unsigned int    RESERVED_MSTR_RESERVED_4[15360];
	volatile U_SMMU_SCR    SMMU_SCR; //40000
	volatile U_SMMU_MEMCTRL    SMMU_MEMCTRL;
	volatile U_SMMU_LP_CTRL    SMMU_LP_CTRL;
	volatile U_SMMU_PRESS_REMAP    SMMU_PRESS_REMAP;
	volatile U_SMMU_INTMASK_NS    SMMU_INTMASK_NS;
	volatile U_SMMU_INTRAW_NS    SMMU_INTRAW_NS;
	volatile U_SMMU_INTSTAT_NS    SMMU_INTSTAT_NS;
	volatile U_SMMU_INTCLR_NS    SMMU_INTCLR_NS;
	volatile U_SMMU_SMRX_NS    SMMU_SMRX_NS[116];
	volatile unsigned int    SMMU_RLD_EN0_NS;
	volatile unsigned int    SMMU_RLD_EN1_NS;
	volatile U_SMMU_RLD_EN2_NS    SMMU_RLD_EN2_NS[2];
	volatile U_SMMU_CB_SCTRL    SMMU_CB_SCTRL;
	volatile unsigned int    SMMU_CB_TTBR0;
	volatile unsigned int    SMMU_CB_TTBR1;
	volatile U_SMMU_CB_TTBCR    SMMU_CB_TTBCR;
	volatile U_SMMU_OFFSET_ADDR_NS   SMMU_OFFSET_ADDR_NS;
	volatile U_SMMU_SCACHEI_ALL    SMMU_SCACHEI_ALL;
	volatile U_SMMU_SCACHEI_L1    SMMU_SCACHEI_L1;
	volatile U_SMMU_SCACHEI_L2L3    SMMU_SCACHEI_L2L3;
	volatile U_SMMU_FAMA_CTRL0_NS   SMMU_FAMA_CTRL0_NS;
	volatile U_SMMU_FAMA_CTRL1_NS   SMMU_FAMA_CTRL1_NS[55];
	volatile U_SMMU_ADDR_MSB    SMMU_ADDR_MSB;
	volatile unsigned int    SMMU_ERR_RDADDR;
	volatile unsigned int    SMMU_ERR_WRADDR[2];
	volatile unsigned int    SMMU_FAULT_ADDR_TCU;
	volatile U_SMMU_FAULT_ID_TCU    SMMU_FAULT_ID_TCU[3];
	volatile unsigned int    SMMU_FAULT_ADDR_TBUX;
	volatile U_SMMU_FAULT_ID_TBUX   SMMU_FAULT_ID_TBUX;
	volatile U_SMMU_FAULT_INFOX    SMMU_FAULT_INFOX[22];
	volatile U_SMMU_DBGRPTR_TLB    SMMU_DBGRPTR_TLB;
	volatile U_SMMU_DBGRDATA_TLB    SMMU_DBGRDATA_TLB;
	volatile U_SMMU_DBGRPTR_CACHE   SMMU_DBGRPTR_CACHE;
	volatile U_SMMU_DBGRDATA0_CACHE   SMMU_DBGRDATA0_CACHE;
	volatile U_SMMU_DBGRDATA1_CACHE   SMMU_DBGRDATA1_CACHE;
	volatile U_SMMU_DBGAXI_CTRL    SMMU_DBGAXI_CTRL;
	volatile unsigned int    SMMU_OVA_ADDR;
	volatile U_SMMU_OPA_ADDR    SMMU_OPA_ADDR;
	volatile U_SMMU_OVA_CTRL    SMMU_OVA_CTRL;
	volatile unsigned int    SMMU_OPREF_ADDR;
	volatile U_SMMU_OPREF_CTRL    SMMU_OPREF_CTRL;
	volatile unsigned int    SMMU_OPREF_CNT[85];
	volatile U_SMMU_SMRX_S    SMMU_SMRX_S[124];
	volatile unsigned int    SMMU_RLD_EN0_S;
	volatile unsigned int    SMMU_RLD_EN1_S;
	volatile U_SMMU_RLD_EN2_S    SMMU_RLD_EN2_S[2];
	volatile U_SMMU_INTMAS_S    SMMU_INTMAS_S;
	volatile U_SMMU_INTRAW_S    SMMU_INTRAW_S;
	volatile U_SMMU_INTSTAT_S    SMMU_INTSTAT_S;
	volatile U_SMMU_INTCLR_S    SMMU_INTCLR_S;
	volatile U_SMMU_SCR_S    SMMU_SCR_S;
	volatile U_SMMU_SCB_SCTRL    SMMU_SCB_SCTRL;
	volatile unsigned int    SMMU_SCB_TTBR;
	volatile U_SMMU_SCB_TTBCR    SMMU_SCB_TTBCR;
	volatile U_SMMU_OFFSET_ADDR_S   SMMU_OFFSET_ADDR_S;
	volatile U_SMMU_FAMA_CTRL0_S    SMMU_FAMA_CTRL0_S;
	volatile U_SMMU_FAMA_CTRL1_S    SMMU_FAMA_CTRL1_S;
	volatile U_SMMU_DBGRPTR_TLB_S   SMMU_DBGRPTR_TLB_S;
	volatile U_SMMU_DBGRPTR_CACHE_S   SMMU_DBGRPTR_CACHE_S;
	volatile U_SMMU_OVERRIDE_CTRL_S   SMMU_OVERRIDE_CTRL_S[15923];
	volatile U_SMMU_SMRX_P    SMMU_SMRX_P[124];
	volatile unsigned int    SMMU_RLD_EN0_P;
	volatile unsigned int    SMMU_RLD_EN1_P;
	volatile U_SMMU_RLD_EN2_P    SMMU_RLD_EN2_P[2];
	volatile U_SMMU_INTMAS_P    SMMU_INTMAS_P;
	volatile U_SMMU_INTRAW_P    SMMU_INTRAW_P;
	volatile U_SMMU_INTSTAT_P    SMMU_INTSTAT_P;
	volatile U_SMMU_INTCLR_P    SMMU_INTCLR_P;
	volatile U_SMMU_SCR_P    SMMU_SCR_P;
	volatile U_SMMU_PCB_SCTRL    SMMU_PCB_SCTRL;
	volatile unsigned int    SMMU_PCB_TTBR;
	volatile U_SMMU_PCB_TTBCR    SMMU_PCB_TTBCR;
	volatile U_SMMU_OFFSET_ADDR_P   SMMU_OFFSET_ADDR_P;
	volatile U_SMMU_FAMA_CTRL0_P    SMMU_FAMA_CTRL0_P;
	volatile U_SMMU_FAMA_CTRL1_P    SMMU_FAMA_CTRL1_P;
} S_HEVC_AVC_REGS_TYPE;

typedef struct
{
	volatile U_VEDU_VCPI_INTMASK    VEDU_VCPI_INTMASK;
	volatile U_VEDU_VCPI_INTCLR    VEDU_VCPI_INTCLR;
	volatile U_VEDU_VCPI_START    VEDU_VCPI_START;
	volatile U_VEDU_VCPI_CNTCLR    VEDU_VCPI_CNTCLR;
	volatile unsigned int    VEDU_VCPI_FRAMENO;
	volatile U_VEDU_VCPI_BP_POS    VEDU_VCPI_BP_POS;
	volatile unsigned int    VEDU_VCPI_TIMEOUT;
	volatile U_VEDU_VCPI_MODE    VEDU_VCPI_MODE;
	volatile U_VEDU_VCPI_TILE_SIZE   VEDU_VCPI_TILE_SIZE;
	volatile U_VEDU_VCPI_PICSIZE_PIX   VEDU_VCPI_PICSIZE_PIX;
	volatile U_VEDU_VCPI_MULTISLC   VEDU_VCPI_MULTISLC;
	volatile U_VEDU_VCPI_QPCFG    VEDU_VCPI_QPCFG;
	volatile U_VEDU_VCPI_DBLKCFG    VEDU_VCPI_DBLKCFG;
	volatile U_VEDU_VCPI_LOW_POWER   VEDU_VCPI_LOW_POWER;
	volatile U_VEDU_VCPI_OUTSTD    VEDU_VCPI_OUTSTD;
	volatile U_VEDU_VCPI_TMV_LOAD   VEDU_VCPI_TMV_LOAD;
	volatile U_VEDU_VCPI_CROSS_TILE_SLC   VEDU_VCPI_CROSS_TILE_SLC;
	volatile U_VEDU_VCPI_MEM_CTRL   VEDU_VCPI_MEM_CTRL;
	volatile U_VEDU_VCPI_INTRA_INTER_CU_EN   VEDU_VCPI_INTRA_INTER_CU_EN;
	volatile U_VEDU_VCPI_VLC_CONFIG   VEDU_VCPI_VLC_CONFIG;
	volatile U_VEDU_VCPI_PRE_JUDGE_EXT_EN   VEDU_VCPI_PRE_JUDGE_EXT_EN;
	volatile U_VEDU_VCPI_PRE_JUDGE_COST_THR   VEDU_VCPI_PRE_JUDGE_COST_THR;
	volatile U_VEDU_VCPI_IBLK_PRE_MV_THR   VEDU_VCPI_IBLK_PRE_MV_THR;
	volatile U_VEDU_VCPI_PME_PARAM   VEDU_VCPI_PME_PARAM;
	volatile U_VEDU_VCPI_PIC_STRONG_EN   VEDU_VCPI_PIC_STRONG_EN;
	volatile U_VEDU_VCPI_REF_FLAG   VEDU_VCPI_REF_FLAG;
	volatile U_VEDU_VCPI_RC_ENABLE   VEDU_VCPI_RC_ENABLE;
	volatile U_VEDU_VCPI_PINTRA_THRESH0   VEDU_VCPI_PINTRA_THRESH0;
	volatile U_VEDU_VCPI_PINTRA_THRESH1   VEDU_VCPI_PINTRA_THRESH1;
	volatile U_VEDU_VCPI_PINTRA_THRESH2   VEDU_VCPI_PINTRA_THRESH2;
	volatile U_VEDU_VCPI_I_SLC_INSERT   VEDU_VCPI_I_SLC_INSERT;
	volatile U_VEDU_VCPI_CLKDIV_ENABLE   VEDU_VCPI_CLKDIV_ENABLE;
	volatile U_VEDU_VCPI_OSD_ENABLE   VEDU_VCPI_OSD_ENABLE;
	volatile U_VEDU_VCPI_OSD_POS_0   VEDU_VCPI_OSD_POS_0;
	volatile U_VEDU_VCPI_OSD_POS_1   VEDU_VCPI_OSD_POS_1;
	volatile U_VEDU_VCPI_OSD_POS_2   VEDU_VCPI_OSD_POS_2;
	volatile U_VEDU_VCPI_OSD_POS_3   VEDU_VCPI_OSD_POS_3;
	volatile U_VEDU_VCPI_OSD_POS_4   VEDU_VCPI_OSD_POS_4;
	volatile U_VEDU_VCPI_OSD_POS_5   VEDU_VCPI_OSD_POS_5;
	volatile U_VEDU_VCPI_OSD_POS_6   VEDU_VCPI_OSD_POS_6;
	volatile U_VEDU_VCPI_OSD_POS_7   VEDU_VCPI_OSD_POS_7;
	volatile U_VEDU_VCPI_OSD_SIZE_0   VEDU_VCPI_OSD_SIZE_0;
	volatile U_VEDU_VCPI_OSD_SIZE_1   VEDU_VCPI_OSD_SIZE_1;
	volatile U_VEDU_VCPI_OSD_SIZE_2   VEDU_VCPI_OSD_SIZE_2;
	volatile U_VEDU_VCPI_OSD_SIZE_3   VEDU_VCPI_OSD_SIZE_3;
	volatile U_VEDU_VCPI_OSD_SIZE_4   VEDU_VCPI_OSD_SIZE_4;
	volatile U_VEDU_VCPI_OSD_SIZE_5   VEDU_VCPI_OSD_SIZE_5;
	volatile U_VEDU_VCPI_OSD_SIZE_6   VEDU_VCPI_OSD_SIZE_6;
	volatile U_VEDU_VCPI_OSD_SIZE_7   VEDU_VCPI_OSD_SIZE_7;
	volatile U_VEDU_VCPI_OSD_LAYERID   VEDU_VCPI_OSD_LAYERID;
	volatile U_VEDU_VCPI_OSD_QP0    VEDU_VCPI_OSD_QP0;
	volatile U_VEDU_VCPI_OSD_QP1    VEDU_VCPI_OSD_QP1;
	volatile U_VEDU_VCPI_SW_L0_SIZE   VEDU_VCPI_SW_L0_SIZE;
	volatile U_VEDU_VCPI_SW_L1_SIZE   VEDU_VCPI_SW_L1_SIZE;
	volatile unsigned int    VEDU_VCPI_PMEINFO_ST_ADDR;
	volatile unsigned int    VEDU_VCPI_PMEINFO_LD0_ADDR;
	volatile unsigned int    VEDU_VCPI_PMEINFO_LD1_ADDR;
	volatile unsigned int    VEDU_VCPI_QPGLD_INF_ADDR;
	volatile unsigned int    VEDU_VCPI_TUNLCELL_ADDR;
	volatile unsigned int    VEDU_VCPI_SRC_YADDR;
	volatile unsigned int    VEDU_VCPI_SRC_CADDR;
	volatile unsigned int    VEDU_VCPI_SRC_VADDR;
	volatile unsigned int    VEDU_VCPI_YH_ADDR;
	volatile unsigned int    VEDU_VCPI_CH_ADDR;
	volatile U_VEDU_VCPI_STRIDE    VEDU_VCPI_STRIDE;
	volatile unsigned int    VEDU_VCPI_REC_YADDR;
	volatile unsigned int    VEDU_VCPI_REC_CADDR;
	volatile U_VEDU_VCPI_REC_STRIDE   VEDU_VCPI_REC_STRIDE;
	volatile unsigned int    VEDU_VCPI_REC_YH_ADDR;
	volatile unsigned int    VEDU_VCPI_REC_CH_ADDR;
	volatile U_VEDU_VCPI_REC_HEAD_STRIDE   VEDU_VCPI_REC_HEAD_STRIDE;
	volatile unsigned int    VEDU_VCPI_REFY_L0_ADDR;
	volatile unsigned int    VEDU_VCPI_REFC_L0_ADDR;
	volatile U_VEDU_VCPI_REF_L0_STRIDE   VEDU_VCPI_REF_L0_STRIDE;
	volatile unsigned int    VEDU_VCPI_REFYH_L0_ADDR;
	volatile unsigned int    VEDU_VCPI_REFCH_L0_ADDR;
	volatile U_VEDU_VCPI_REFH_L0_STRIDE   VEDU_VCPI_REFH_L0_STRIDE;
	volatile unsigned int    VEDU_VCPI_PMELD_L0_ADDR;
	volatile unsigned int    VEDU_VCPI_REFY_L1_ADDR;
	volatile unsigned int    VEDU_VCPI_REFC_L1_ADDR;
	volatile U_VEDU_VCPI_REF_L1_STRIDE   VEDU_VCPI_REF_L1_STRIDE;
	volatile unsigned int    VEDU_VCPI_REFYH_L1_ADDR;
	volatile unsigned int    VEDU_VCPI_REFCH_L1_ADDR;
	volatile U_VEDU_VCPI_REFH_L1_STRIDE   VEDU_VCPI_REFH_L1_STRIDE;
	volatile unsigned int    VEDU_VCPI_PMELD_L1_ADDR;
	volatile unsigned int    VEDU_VCPI_PMEST_ADDR;
	volatile unsigned int    VEDU_VCPI_NBI_UPST_ADDR;
	volatile unsigned int    VEDU_VCPI_NBI_MVST_ADDR;
	volatile unsigned int    VEDU_VCPI_NBI_MVLD_ADDR;
	volatile unsigned int    VEDU_VCPI_STRMADDR;
	volatile unsigned int    VEDU_VCPI_SWPTRADDR;
	volatile unsigned int    VEDU_VCPI_SRPTRADDR;
	volatile unsigned int    VEDU_VCPI_LLILD_ADDR;
	volatile U_VEDU_VCPI_STRFMT    VEDU_VCPI_STRFMT;
	volatile U_VEDU_VCPI_CROP_START   VEDU_VCPI_CROP_START;
	volatile U_VEDU_VCPI_CROP_END   VEDU_VCPI_CROP_END;
	volatile U_VEDU_VCPI_SCALE_PARA   VEDU_VCPI_SCALE_PARA;
	volatile U_VEDU_VCPI_ORI_PICSIZE   VEDU_VCPI_ORI_PICSIZE;
	volatile U_VEDU_VCPI_BG_ENABLE   VEDU_VCPI_BG_ENABLE;
	volatile unsigned int    VEDU_VCPI_BGL_ADDR;
	volatile unsigned int    VEDU_VCPI_BGC_ADDR;
	volatile unsigned int    VEDU_VCPI_BGINF_ADDR;
	volatile U_VEDU_VCPI_BG_STRIDE   VEDU_VCPI_BG_STRIDE;
	volatile U_VEDU_VCPI_BG_FLT_PARA0   VEDU_VCPI_BG_FLT_PARA0;
	volatile U_VEDU_VCPI_BG_FLT_PARA1   VEDU_VCPI_BG_FLT_PARA1;
	volatile U_VEDU_VCPI_BG_FLT_PARA2   VEDU_VCPI_BG_FLT_PARA2;
	volatile U_VEDU_VCPI_BG_THR0    VEDU_VCPI_BG_THR0;
	volatile U_VEDU_VCPI_BG_THR1    VEDU_VCPI_BG_THR1;
	volatile U_VEDU_VCPI_MEM_CTRL_T16   VEDU_VCPI_MEM_CTRL_T16;
	volatile unsigned int    VEDU_VCPI_PMEST_STRIDE;
	volatile unsigned int    VEDU_VCPI_PMELD_STRIDE;
	volatile U_VEDU_VCPI_INTRA32_LOW_POWER   VEDU_VCPI_INTRA32_LOW_POWER;
	volatile U_VEDU_VCPI_INTRA16_LOW_POWER   VEDU_VCPI_INTRA16_LOW_POWER;
	volatile U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM   VEDU_VCPI_INTRA_REDUCE_RDO_NUM;
	volatile unsigned int    VEDU_VCPI_DBLK_INFO_ADDR;
	volatile U_VEDU_VCPI_NOFORCEZERO   VEDU_VCPI_NOFORCEZERO;
	volatile U_VEDU_VCPI_INTMASK_S   VEDU_VCPI_INTMASK_S;
	volatile U_VEDU_VCPI_INTCLR_S   VEDU_VCPI_INTCLR_S;

	volatile U_VEDU_VCTRL_ROI_CFG0   VEDU_VCTRL_ROI_CFG0;
	volatile U_VEDU_VCTRL_ROI_CFG1   VEDU_VCTRL_ROI_CFG1;
	volatile U_VEDU_VCTRL_ROI_CFG2   VEDU_VCTRL_ROI_CFG2;
	volatile U_VEDU_VCTRL_ROI_SIZE_0   VEDU_VCTRL_ROI_SIZE_0;
	volatile U_VEDU_VCTRL_ROI_SIZE_1   VEDU_VCTRL_ROI_SIZE_1;
	volatile U_VEDU_VCTRL_ROI_SIZE_2   VEDU_VCTRL_ROI_SIZE_2;
	volatile U_VEDU_VCTRL_ROI_SIZE_3   VEDU_VCTRL_ROI_SIZE_3;
	volatile U_VEDU_VCTRL_ROI_SIZE_4   VEDU_VCTRL_ROI_SIZE_4;
	volatile U_VEDU_VCTRL_ROI_SIZE_5   VEDU_VCTRL_ROI_SIZE_5;
	volatile U_VEDU_VCTRL_ROI_SIZE_6   VEDU_VCTRL_ROI_SIZE_6;
	volatile U_VEDU_VCTRL_ROI_SIZE_7   VEDU_VCTRL_ROI_SIZE_7;
	volatile U_VEDU_VCTRL_ROI_START_0   VEDU_VCTRL_ROI_START_0;
	volatile U_VEDU_VCTRL_ROI_START_1   VEDU_VCTRL_ROI_START_1;
	volatile U_VEDU_VCTRL_ROI_START_2   VEDU_VCTRL_ROI_START_2;
	volatile U_VEDU_VCTRL_ROI_START_3   VEDU_VCTRL_ROI_START_3;
	volatile U_VEDU_VCTRL_ROI_START_4   VEDU_VCTRL_ROI_START_4;
	volatile U_VEDU_VCTRL_ROI_START_5   VEDU_VCTRL_ROI_START_5;
	volatile U_VEDU_VCTRL_ROI_START_6   VEDU_VCTRL_ROI_START_6;
	volatile U_VEDU_VCTRL_ROI_START_7   VEDU_VCTRL_ROI_START_7;
	volatile U_VEDU_VCTRL_LCU_TARGET_BIT   VEDU_VCTRL_LCU_TARGET_BIT;
	volatile U_VEDU_VCTRL_NARROW_THRESHOLD   VEDU_VCTRL_NARROW_THRESHOLD;
	volatile U_VEDU_VCTRL_LCU_BASELINE   VEDU_VCTRL_LCU_BASELINE;
	volatile U_VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE   VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE   VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_NORM_COEFF_DENOISE   VEDU_VCTRL_NORM_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_NORM_ENG_DENOISE   VEDU_VCTRL_NORM_ENG_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE   VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE   VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_COEFF_DENOISE   VEDU_VCTRL_SKIN_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SKIN_ENG_DENOISE   VEDU_VCTRL_SKIN_ENG_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE   VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE   VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_COEFF_DENOISE   VEDU_VCTRL_HEDGE_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGE_ENG_DENOISE   VEDU_VCTRL_HEDGE_ENG_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE   VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE   VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE   VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_HEDGEMOV_ENG_DENOISE   VEDU_VCTRL_HEDGEMOV_ENG_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE   VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE   VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_COEFF_DENOISE   VEDU_VCTRL_STATIC_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_STATIC_ENG_DENOISE   VEDU_VCTRL_STATIC_ENG_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE   VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE   VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_COEFF_DENOISE   VEDU_VCTRL_SOBELSTR_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELSTR_ENG_DENOISE   VEDU_VCTRL_SOBELSTR_ENG_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE   VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE   VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE   VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE;
	volatile U_VEDU_VCTRL_SOBELWEAK_ENG_DENOISE   VEDU_VCTRL_SOBELWEAK_ENG_DENOISE;
	volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_0   VEDU_VCTRL_INTRA_RDO_FACTOR_0;
	volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_1   VEDU_VCTRL_INTRA_RDO_FACTOR_1;
	volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_2   VEDU_VCTRL_INTRA_RDO_FACTOR_2;
	volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_0   VEDU_VCTRL_MRG_RDO_FACTOR_0;
	volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_1   VEDU_VCTRL_MRG_RDO_FACTOR_1;
	volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_2   VEDU_VCTRL_MRG_RDO_FACTOR_2;
	volatile U_VEDU_VCTRL_FME_RDO_FACTOR_0   VEDU_VCTRL_FME_RDO_FACTOR_0;
	volatile U_VEDU_VCTRL_FME_RDO_FACTOR_1   VEDU_VCTRL_FME_RDO_FACTOR_1;
	volatile U_VEDU_VCTRL_FME_RDO_FACTOR_2   VEDU_VCTRL_FME_RDO_FACTOR_2;
	volatile U_VEDU_CURLD_GCFG    VEDU_CURLD_GCFG;
	volatile U_VEDU_CURLD_OSD01_ALPHA   VEDU_CURLD_OSD01_ALPHA;
	volatile U_VEDU_CURLD_OSD23_ALPHA   VEDU_CURLD_OSD23_ALPHA;
	volatile U_VEDU_CURLD_OSD45_ALPHA   VEDU_CURLD_OSD45_ALPHA;
	volatile U_VEDU_CURLD_OSD67_ALPHA   VEDU_CURLD_OSD67_ALPHA;
	volatile U_VEDU_CURLD_OSD_GALPHA0   VEDU_CURLD_OSD_GALPHA0;
	volatile U_VEDU_CURLD_OSD_GALPHA1   VEDU_CURLD_OSD_GALPHA1;
	volatile unsigned int    VEDU_CURLD_OSD0_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD1_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD2_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD3_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD4_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD5_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD6_ADDR;
	volatile unsigned int    VEDU_CURLD_OSD7_ADDR;
	volatile U_VEDU_CURLD_OSD01_STRIDE   VEDU_CURLD_OSD01_STRIDE;
	volatile U_VEDU_CURLD_OSD23_STRIDE   VEDU_CURLD_OSD23_STRIDE;
	volatile U_VEDU_CURLD_OSD45_STRIDE   VEDU_CURLD_OSD45_STRIDE;
	volatile U_VEDU_CURLD_OSD67_STRIDE   VEDU_CURLD_OSD67_STRIDE;
	volatile U_VEDU_CURLD_CLIP_THR   VEDU_CURLD_CLIP_THR;
	volatile U_VEDU_CURLD_HOR_FILTER   VEDU_CURLD_HOR_FILTER;
	volatile U_VEDU_CURLD_VER_FILTER   VEDU_CURLD_VER_FILTER;
	volatile U_VEDU_CURLD_ARGB_YUV_0COEFF   VEDU_CURLD_ARGB_YUV_0COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_1COEFF   VEDU_CURLD_ARGB_YUV_1COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_2COEFF   VEDU_CURLD_ARGB_YUV_2COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_3COEFF   VEDU_CURLD_ARGB_YUV_3COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_4COEFF   VEDU_CURLD_ARGB_YUV_4COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_5COEFF   VEDU_CURLD_ARGB_YUV_5COEFF;
	volatile U_VEDU_CURLD_ARGB_YUV_6COEFF   VEDU_CURLD_ARGB_YUV_6COEFF;
	volatile U_VEDU_CURLD_ARGB_CLIP   VEDU_CURLD_ARGB_CLIP;
	volatile U_VEDU_CURLD_NARROW_EN   VEDU_CURLD_NARROW_EN;
	volatile U_VEDU_CURLD_SRCH_STRIDE   VEDU_CURLD_SRCH_STRIDE;
	volatile U_VEDU_CURLD_HFBCD    VEDU_CURLD_HFBCD;
	volatile U_VEDU_PME_SW_ADAPT_EN   VEDU_PME_SW_ADAPT_EN;
	volatile U_VEDU_PME_SW_THR0    VEDU_PME_SW_THR0;
	volatile U_VEDU_PME_SW_THR1    VEDU_PME_SW_THR1;
	volatile U_VEDU_PME_SW_THR2    VEDU_PME_SW_THR2;
	volatile U_VEDU_PME_SKIP_PRE    VEDU_PME_SKIP_PRE;
	volatile U_VEDU_PME_TR_WEIGHTX   VEDU_PME_TR_WEIGHTX;
	volatile U_VEDU_PME_TR_WEIGHTY   VEDU_PME_TR_WEIGHTY;
	volatile U_VEDU_PME_SR_WEIGHT   VEDU_PME_SR_WEIGHT;
	volatile U_VEDU_PME_INTRABLK_DET   VEDU_PME_INTRABLK_DET;
	volatile U_VEDU_PME_INTRABLK_DET_THR   VEDU_PME_INTRABLK_DET_THR;
	volatile U_VEDU_PME_SKIN_THR    VEDU_PME_SKIN_THR;
	volatile U_VEDU_PME_INTRA_LOWPOW   VEDU_PME_INTRA_LOWPOW;
	volatile U_VEDU_PME_IBLK_COST_THR   VEDU_PME_IBLK_COST_THR;
	volatile U_VEDU_PME_STRONG_EDGE   VEDU_PME_STRONG_EDGE;
	volatile U_VEDU_PME_LARGE_MOVE_THR   VEDU_PME_LARGE_MOVE_THR;
	volatile U_VEDU_PME_INTER_STRONG_EDGE   VEDU_PME_INTER_STRONG_EDGE;
	volatile U_VEDU_PME_NEW_COST    VEDU_PME_NEW_COST;
	volatile U_VEDU_PME_WINDOW_SIZE0_L0   VEDU_PME_WINDOW_SIZE0_L0;
	volatile U_VEDU_PME_WINDOW_SIZE1_L0   VEDU_PME_WINDOW_SIZE1_L0;
	volatile U_VEDU_PME_WINDOW_SIZE2_L0   VEDU_PME_WINDOW_SIZE2_L0;
	volatile U_VEDU_PME_WINDOW_SIZE3_L0   VEDU_PME_WINDOW_SIZE3_L0;
	volatile U_VEDU_PME_WINDOW_SIZE0_L1   VEDU_PME_WINDOW_SIZE0_L1;
	volatile U_VEDU_PME_WINDOW_SIZE1_L1   VEDU_PME_WINDOW_SIZE1_L1;
	volatile U_VEDU_PME_WINDOW_SIZE2_L1   VEDU_PME_WINDOW_SIZE2_L1;
	volatile U_VEDU_PME_WINDOW_SIZE3_L1   VEDU_PME_WINDOW_SIZE3_L1;
	volatile U_VEDU_PME_COST_OFFSET   VEDU_PME_COST_OFFSET;
	volatile U_VEDU_PME_SAFE_CFG    VEDU_PME_SAFE_CFG;
	volatile U_VEDU_PME_IBLK_REFRESH   VEDU_PME_IBLK_REFRESH;
	volatile U_VEDU_PME_IBLK_REFRESH_NUM   VEDU_PME_IBLK_REFRESH_NUM;
	volatile U_VEDU_PME_QPG_RC_THR0   VEDU_PME_QPG_RC_THR0;
	volatile U_VEDU_PME_QPG_RC_THR1   VEDU_PME_QPG_RC_THR1;
	volatile U_VEDU_PME_LOW_LUMA_THR   VEDU_PME_LOW_LUMA_THR;
	volatile U_VEDU_PME_PBLK_PRE1   VEDU_PME_PBLK_PRE1;
	volatile U_VEDU_PME_CHROMA_FLAT   VEDU_PME_CHROMA_FLAT;
	volatile U_VEDU_PME_LUMA_FLAT   VEDU_PME_LUMA_FLAT;
	volatile U_VEDU_PME_MADI_FLAT   VEDU_PME_MADI_FLAT;
	volatile U_VEDU_PME_SKIP_LARGE_RES   VEDU_PME_SKIP_LARGE_RES;
	volatile U_VEDU_QPG_MAX_MIN_QP   VEDU_QPG_MAX_MIN_QP;
	volatile U_VEDU_QPG_ROW_TARGET_BITS   VEDU_QPG_ROW_TARGET_BITS;
	volatile U_VEDU_QPG_AVERAGE_LCU_BITS   VEDU_QPG_AVERAGE_LCU_BITS;
	volatile U_VEDU_QPG_LOWLUMA    VEDU_QPG_LOWLUMA;
	volatile U_VEDU_QPG_HEDGE    VEDU_QPG_HEDGE;
	volatile U_VEDU_QPG_HEDGE_MOVE   VEDU_QPG_HEDGE_MOVE;
	volatile U_VEDU_QPG_LARGE_MOVE   VEDU_QPG_LARGE_MOVE;
	volatile U_VEDU_QPG_SKIN    VEDU_QPG_SKIN;
	volatile U_VEDU_QPG_INTRA_DET   VEDU_QPG_INTRA_DET;
	volatile U_VEDU_QPG_H264_SMOOTH   VEDU_QPG_H264_SMOOTH;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0   VEDU_QPG_CU_QP_DELTA_THRESH_REG0;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1   VEDU_QPG_CU_QP_DELTA_THRESH_REG1;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2   VEDU_QPG_CU_QP_DELTA_THRESH_REG2;
	volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3   VEDU_QPG_CU_QP_DELTA_THRESH_REG3;
	volatile U_VEDU_QPG_DELTA_LEVEL   VEDU_QPG_DELTA_LEVEL;
	volatile U_VEDU_QPG_MADI_SWITCH_THR   VEDU_QPG_MADI_SWITCH_THR;
	volatile U_VEDU_QPG_CU32_DELTA   VEDU_QPG_CU32_DELTA;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG00   VEDU_QPG_QP_LAMBDA_CTRL_REG00;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG01   VEDU_QPG_QP_LAMBDA_CTRL_REG01;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG02   VEDU_QPG_QP_LAMBDA_CTRL_REG02;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG03   VEDU_QPG_QP_LAMBDA_CTRL_REG03;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG04   VEDU_QPG_QP_LAMBDA_CTRL_REG04;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG05   VEDU_QPG_QP_LAMBDA_CTRL_REG05;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG06   VEDU_QPG_QP_LAMBDA_CTRL_REG06;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG07   VEDU_QPG_QP_LAMBDA_CTRL_REG07;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG08   VEDU_QPG_QP_LAMBDA_CTRL_REG08;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG09   VEDU_QPG_QP_LAMBDA_CTRL_REG09;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG10   VEDU_QPG_QP_LAMBDA_CTRL_REG10;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG11   VEDU_QPG_QP_LAMBDA_CTRL_REG11;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG12   VEDU_QPG_QP_LAMBDA_CTRL_REG12;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG13   VEDU_QPG_QP_LAMBDA_CTRL_REG13;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG14   VEDU_QPG_QP_LAMBDA_CTRL_REG14;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG15   VEDU_QPG_QP_LAMBDA_CTRL_REG15;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG16   VEDU_QPG_QP_LAMBDA_CTRL_REG16;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG17   VEDU_QPG_QP_LAMBDA_CTRL_REG17;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG18   VEDU_QPG_QP_LAMBDA_CTRL_REG18;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG19   VEDU_QPG_QP_LAMBDA_CTRL_REG19;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG20   VEDU_QPG_QP_LAMBDA_CTRL_REG20;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG21   VEDU_QPG_QP_LAMBDA_CTRL_REG21;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG22   VEDU_QPG_QP_LAMBDA_CTRL_REG22;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG23   VEDU_QPG_QP_LAMBDA_CTRL_REG23;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG24   VEDU_QPG_QP_LAMBDA_CTRL_REG24;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG25   VEDU_QPG_QP_LAMBDA_CTRL_REG25;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG26   VEDU_QPG_QP_LAMBDA_CTRL_REG26;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG27   VEDU_QPG_QP_LAMBDA_CTRL_REG27;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG28   VEDU_QPG_QP_LAMBDA_CTRL_REG28;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG29   VEDU_QPG_QP_LAMBDA_CTRL_REG29;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG30   VEDU_QPG_QP_LAMBDA_CTRL_REG30;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG31   VEDU_QPG_QP_LAMBDA_CTRL_REG31;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG32   VEDU_QPG_QP_LAMBDA_CTRL_REG32;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG33   VEDU_QPG_QP_LAMBDA_CTRL_REG33;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG34   VEDU_QPG_QP_LAMBDA_CTRL_REG34;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG35   VEDU_QPG_QP_LAMBDA_CTRL_REG35;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG36   VEDU_QPG_QP_LAMBDA_CTRL_REG36;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG37   VEDU_QPG_QP_LAMBDA_CTRL_REG37;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG38   VEDU_QPG_QP_LAMBDA_CTRL_REG38;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG39   VEDU_QPG_QP_LAMBDA_CTRL_REG39;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG40   VEDU_QPG_QP_LAMBDA_CTRL_REG40;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG41   VEDU_QPG_QP_LAMBDA_CTRL_REG41;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG42   VEDU_QPG_QP_LAMBDA_CTRL_REG42;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG43   VEDU_QPG_QP_LAMBDA_CTRL_REG43;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG44   VEDU_QPG_QP_LAMBDA_CTRL_REG44;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG45   VEDU_QPG_QP_LAMBDA_CTRL_REG45;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG46   VEDU_QPG_QP_LAMBDA_CTRL_REG46;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG47   VEDU_QPG_QP_LAMBDA_CTRL_REG47;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG48   VEDU_QPG_QP_LAMBDA_CTRL_REG48;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG49   VEDU_QPG_QP_LAMBDA_CTRL_REG49;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG50   VEDU_QPG_QP_LAMBDA_CTRL_REG50;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG51   VEDU_QPG_QP_LAMBDA_CTRL_REG51;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG52   VEDU_QPG_QP_LAMBDA_CTRL_REG52;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG53   VEDU_QPG_QP_LAMBDA_CTRL_REG53;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG54   VEDU_QPG_QP_LAMBDA_CTRL_REG54;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG55   VEDU_QPG_QP_LAMBDA_CTRL_REG55;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG56   VEDU_QPG_QP_LAMBDA_CTRL_REG56;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG57   VEDU_QPG_QP_LAMBDA_CTRL_REG57;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG58   VEDU_QPG_QP_LAMBDA_CTRL_REG58;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG59   VEDU_QPG_QP_LAMBDA_CTRL_REG59;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG60   VEDU_QPG_QP_LAMBDA_CTRL_REG60;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG61   VEDU_QPG_QP_LAMBDA_CTRL_REG61;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG62   VEDU_QPG_QP_LAMBDA_CTRL_REG62;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG63   VEDU_QPG_QP_LAMBDA_CTRL_REG63;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG64   VEDU_QPG_QP_LAMBDA_CTRL_REG64;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG65   VEDU_QPG_QP_LAMBDA_CTRL_REG65;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG66   VEDU_QPG_QP_LAMBDA_CTRL_REG66;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG67   VEDU_QPG_QP_LAMBDA_CTRL_REG67;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG68   VEDU_QPG_QP_LAMBDA_CTRL_REG68;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG69   VEDU_QPG_QP_LAMBDA_CTRL_REG69;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG70   VEDU_QPG_QP_LAMBDA_CTRL_REG70;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG71   VEDU_QPG_QP_LAMBDA_CTRL_REG71;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG72   VEDU_QPG_QP_LAMBDA_CTRL_REG72;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG73   VEDU_QPG_QP_LAMBDA_CTRL_REG73;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG74   VEDU_QPG_QP_LAMBDA_CTRL_REG74;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG75   VEDU_QPG_QP_LAMBDA_CTRL_REG75;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG76   VEDU_QPG_QP_LAMBDA_CTRL_REG76;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG77   VEDU_QPG_QP_LAMBDA_CTRL_REG77;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG78   VEDU_QPG_QP_LAMBDA_CTRL_REG78;
	volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG79   VEDU_QPG_QP_LAMBDA_CTRL_REG79;
	volatile U_VEDU_QPG_LAMBDA_MODE   VEDU_QPG_LAMBDA_MODE;
	volatile U_VEDU_QPG_QP_RESTRAIN   VEDU_QPG_QP_RESTRAIN;
	volatile unsigned int    VEDU_QPG_CU_MIN_SAD_THRESH_0;
	volatile unsigned int    VEDU_QPG_CU_MIN_SAD_THRESH_1;
	volatile U_VEDU_QPG_CU_MIN_SAD_REG   VEDU_QPG_CU_MIN_SAD_REG;
	volatile U_VEDU_QPG_SMART_REG   VEDU_QPG_SMART_REG;
	volatile U_VEDU_QPG_FLAT_REGION   VEDU_QPG_FLAT_REGION;
	volatile U_VEDU_IME_INTER_MODE   VEDU_IME_INTER_MODE;
	volatile U_VEDU_IME_RDOCFG    VEDU_IME_RDOCFG;
	volatile U_VEDU_IME_FME_LPOW_THR   VEDU_IME_FME_LPOW_THR;
	volatile U_VEDU_IME_LAYER3TO2_THR   VEDU_IME_LAYER3TO2_THR;
	volatile U_VEDU_IME_LAYER3TO2_THR1   VEDU_IME_LAYER3TO2_THR1;
	volatile U_VEDU_IME_LAYER3TO1_THR   VEDU_IME_LAYER3TO1_THR;
	volatile U_VEDU_IME_LAYER3TO1_THR1   VEDU_IME_LAYER3TO1_THR1;
	volatile U_VEDU_FME_BIAS_COST0   VEDU_FME_BIAS_COST0;
	volatile U_VEDU_FME_BIAS_COST1   VEDU_FME_BIAS_COST1;
	volatile U_VEDU_FME_PU64_LWP    VEDU_FME_PU64_LWP;
	volatile U_VEDU_MRG_FORCE_ZERO_EN   VEDU_MRG_FORCE_ZERO_EN;
	volatile U_VEDU_MRG_FORCE_SKIP_EN   VEDU_MRG_FORCE_SKIP_EN;
	volatile U_VEDU_MRG_BIAS_COST0   VEDU_MRG_BIAS_COST0;
	volatile U_VEDU_MRG_BIAS_COST1   VEDU_MRG_BIAS_COST1;
	volatile U_VEDU_MRG_ABS_OFFSET0   VEDU_MRG_ABS_OFFSET0;
	volatile U_VEDU_MRG_ABS_OFFSET1   VEDU_MRG_ABS_OFFSET1;
	volatile U_VEDU_MRG_ADJ_WEIGHT   VEDU_MRG_ADJ_WEIGHT;
	volatile U_VEDU_INTRA_CFG    VEDU_INTRA_CFG;
	volatile U_VEDU_INTRA_SMOOTH    VEDU_INTRA_SMOOTH;
	volatile U_VEDU_INTRA_BIT_WEIGHT   VEDU_INTRA_BIT_WEIGHT;
	volatile U_VEDU_INTRA_RDO_COST_OFFSET_0   VEDU_INTRA_RDO_COST_OFFSET_0;
	volatile U_VEDU_INTRA_RDO_COST_OFFSET_1   VEDU_INTRA_RDO_COST_OFFSET_1;
	volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_0   VEDU_INTRA_NO_DC_COST_OFFSET_0;
	volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_1   VEDU_INTRA_NO_DC_COST_OFFSET_1;
	volatile U_VEDU_INTRA_CHNL4_ANG_0EN   VEDU_INTRA_CHNL4_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL4_ANG_1EN   VEDU_INTRA_CHNL4_ANG_1EN;
	volatile U_VEDU_INTRA_CHNL8_ANG_0EN   VEDU_INTRA_CHNL8_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL8_ANG_1EN   VEDU_INTRA_CHNL8_ANG_1EN;
	volatile U_VEDU_INTRA_CHNL16_ANG_0EN   VEDU_INTRA_CHNL16_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL16_ANG_1EN   VEDU_INTRA_CHNL16_ANG_1EN;
	volatile U_VEDU_INTRA_CHNL32_ANG_0EN   VEDU_INTRA_CHNL32_ANG_0EN;
	volatile U_VEDU_INTRA_CHNL32_ANG_1EN   VEDU_INTRA_CHNL32_ANG_1EN;
	volatile U_VEDU_INTRA_RDO_COST_OFFSET_3   VEDU_INTRA_RDO_COST_OFFSET_3;
	volatile unsigned int    VEDU_PMV_POC_0;
	volatile unsigned int    VEDU_PMV_POC_1;
	volatile unsigned int    VEDU_PMV_POC_2;
	volatile unsigned int    VEDU_PMV_POC_3;
	volatile unsigned int    VEDU_PMV_POC_4;
	volatile unsigned int    VEDU_PMV_POC_5;
	volatile U_VEDU_PMV_TMV_EN    VEDU_PMV_TMV_EN;
	volatile U_VEDU_TQITQ_DEADZONE   VEDU_TQITQ_DEADZONE;
	volatile U_VEDU_SEL_OFFSET_STRENGTH   VEDU_SEL_OFFSET_STRENGTH;
	volatile U_VEDU_SEL_CU32_DC_AC_TH_OFFSET   VEDU_SEL_CU32_DC_AC_TH_OFFSET;
	volatile U_VEDU_SEL_CU32_QP_TH   VEDU_SEL_CU32_QP_TH;
	volatile U_VEDU_SEL_RES_DC_AC_TH   VEDU_SEL_RES_DC_AC_TH;
	volatile unsigned int    VEDU_SAO_SSD_AERA0;
	volatile unsigned int    VEDU_SAO_SSD_AERA1;
	volatile unsigned int    VEDU_SAO_SSD_AERA2;
	volatile unsigned int    VEDU_SAO_SSD_AERA3;
	volatile unsigned int    VEDU_SAO_SSD_AERA4;
	volatile unsigned int    VEDU_SAO_SSD_AERA5;
	volatile unsigned int    VEDU_SAO_SSD_AERA6;
	volatile unsigned int    VEDU_SAO_SSD_AERA7;
	volatile U_VEDU_EMAR_WAIT_TIM_OUT   VEDU_EMAR_WAIT_TIM_OUT;
	volatile U_VEDU_EMAR_RCH_RPT_TH0   VEDU_EMAR_RCH_RPT_TH0;
	volatile U_VEDU_EMAR_RCH_RPT_TH1   VEDU_EMAR_RCH_RPT_TH1;
	volatile U_VEDU_EMAR_RCH_RPT_TH2   VEDU_EMAR_RCH_RPT_TH2;
	volatile U_VEDU_EMAR_WCH_RPT_TH0   VEDU_EMAR_WCH_RPT_TH0;
	volatile U_VEDU_EMAR_WCH_RPT_TH1   VEDU_EMAR_WCH_RPT_TH1;
	volatile U_VEDU_EMAR_WCH_RPT_TH2   VEDU_EMAR_WCH_RPT_TH2;
	volatile U_VEDU_EMAR_SCRAMBLE_TYPE   VEDU_EMAR_SCRAMBLE_TYPE;
	volatile unsigned int    VEDU_PACK_SYNTAX_CONFIG;
	volatile U_VEDU_PACK_CU_PARAMETER   VEDU_PACK_CU_PARAMETER;
	volatile U_VEDU_PACK_PCM_PARAMETER   VEDU_PACK_PCM_PARAMETER;
	volatile U_VEDU_PACK_TF_SKIP_FLAG   VEDU_PACK_TF_SKIP_FLAG;
	volatile U_VEDU_CABAC_GLB_CFG   VEDU_CABAC_GLB_CFG;
	volatile U_VEDU_CABAC_SLCHDR_SIZE   VEDU_CABAC_SLCHDR_SIZE;
	volatile U_VEDU_CABAC_SLCHDR_PART1   VEDU_CABAC_SLCHDR_PART1;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG1;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG2;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG3;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG4;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG5;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG6;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG7;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG8;
	volatile U_VEDU_CABAC_SLCHDR_SIZE_I   VEDU_CABAC_SLCHDR_SIZE_I;
	volatile U_VEDU_CABAC_SLCHDR_PART1_I   VEDU_CABAC_SLCHDR_PART1_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG1_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG2_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG3_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG4_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG5_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG6_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG7_I;
	volatile unsigned int    VEDU_CABAC_SLCHDR_PART2_SEG8_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM0;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM1;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM2;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM3;
	volatile unsigned int    VEDU_VLC_REORDERSTRM0;
	volatile unsigned int    VEDU_VLC_REORDERSTRM1;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM0;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM1;
	volatile U_VEDU_VLC_SLCHDRPARA   VEDU_VLC_SLCHDRPARA;
	volatile U_VEDU_VLC_SVC    VEDU_VLC_SVC;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM0_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM1_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM2_I;
	volatile unsigned int    VEDU_VLC_SLCHDRSTRM3_I;
	volatile unsigned int    VEDU_VLC_REORDERSTRM0_I;
	volatile unsigned int    VEDU_VLC_REORDERSTRM1_I;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM0_I;
	volatile unsigned int    VEDU_VLC_MARKINGSTRM1_I;
	volatile U_VEDU_VLC_SLCHDRPARA_I   VEDU_VLC_SLCHDRPARA_I;
	volatile U_VEDU_VLCST_PTBITS_EN   VEDU_VLCST_PTBITS_EN;
	volatile unsigned int    VEDU_VLCST_PTBITS;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN0;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN1;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN2;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN3;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN4;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN5;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN6;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN7;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN8;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN9;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN10;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN11;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN12;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN13;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN14;
	volatile unsigned int    VEDU_VLCST_STRMBUFLEN15;
	volatile unsigned int    VEDU_VLCST_STRMADDR0;
	volatile unsigned int    VEDU_VLCST_STRMADDR1;
	volatile unsigned int    VEDU_VLCST_STRMADDR2;
	volatile unsigned int    VEDU_VLCST_STRMADDR3;
	volatile unsigned int    VEDU_VLCST_STRMADDR4;
	volatile unsigned int    VEDU_VLCST_STRMADDR5;
	volatile unsigned int    VEDU_VLCST_STRMADDR6;
	volatile unsigned int    VEDU_VLCST_STRMADDR7;
	volatile unsigned int    VEDU_VLCST_STRMADDR8;
	volatile unsigned int    VEDU_VLCST_STRMADDR9;
	volatile unsigned int    VEDU_VLCST_STRMADDR10;
	volatile unsigned int    VEDU_VLCST_STRMADDR11;
	volatile unsigned int    VEDU_VLCST_STRMADDR12;
	volatile unsigned int    VEDU_VLCST_STRMADDR13;
	volatile unsigned int    VEDU_VLCST_STRMADDR14;
	volatile unsigned int    VEDU_VLCST_STRMADDR15;
	volatile unsigned int    VEDU_VLCST_PARA_ADDR;
	volatile U_VEDU_VLCST_PARAMETER   VEDU_VLCST_PARAMETER;
	volatile unsigned int    VEDU_VLCST_PARA_DATA0;
	volatile unsigned int    VEDU_VLCST_PARA_DATA1;
	volatile unsigned int    VEDU_VLCST_PARA_DATA2;
	volatile unsigned int    VEDU_VLCST_PARA_DATA3;
	volatile unsigned int    VEDU_VLCST_PARA_DATA4;
	volatile unsigned int    VEDU_VLCST_PARA_DATA5;
	volatile unsigned int    VEDU_VLCST_PARA_DATA6;
	volatile unsigned int    VEDU_VLCST_PARA_DATA7;
	volatile unsigned int    VEDU_VLCST_PARA_DATA8;
	volatile unsigned int    VEDU_VLCST_PARA_DATA9;
	volatile unsigned int    VEDU_VLCST_PARA_DATA10;
	volatile unsigned int    VEDU_VLCST_PARA_DATA11;
	volatile unsigned int    VEDU_VLCST_PARA_DATA12;
	volatile unsigned int    VEDU_VLCST_PARA_DATA13;
	volatile unsigned int    VEDU_VLCST_PARA_DATA14;
	volatile unsigned int    VEDU_VLCST_PARA_DATA15;
	volatile unsigned int    VEDU_VLCST_PARA_DATA16;
	volatile unsigned int    VEDU_VLCST_PARA_DATA17;
	volatile unsigned int    VEDU_VLCST_PARA_DATA18;
	volatile unsigned int    VEDU_VLCST_PARA_DATA19;
	volatile unsigned int    VEDU_VLCST_PARA_DATA20;
	volatile unsigned int    VEDU_VLCST_PARA_DATA21;
	volatile unsigned int    VEDU_VLCST_PARA_DATA22;
	volatile unsigned int    VEDU_VLCST_PARA_DATA23;
	volatile unsigned int    VEDU_VLCST_PARA_DATA24;
	volatile unsigned int    VEDU_VLCST_PARA_DATA25;
	volatile unsigned int    VEDU_VLCST_PARA_DATA26;
	volatile unsigned int    VEDU_VLCST_PARA_DATA27;
	volatile unsigned int    VEDU_VLCST_PARA_DATA28;
	volatile unsigned int    VEDU_VLCST_PARA_DATA29;
	volatile unsigned int    VEDU_VLCST_PARA_DATA30;
	volatile unsigned int    VEDU_VLCST_PARA_DATA31;
	volatile unsigned int    VEDU_VLCST_PARA_DATA32;
	volatile unsigned int    VEDU_VLCST_PARA_DATA33;
	volatile unsigned int    VEDU_VLCST_PARA_DATA34;
	volatile unsigned int    VEDU_VLCST_PARA_DATA35;
	volatile unsigned int    VEDU_VLCST_PARA_DATA36;
	volatile unsigned int    VEDU_VLCST_PARA_DATA37;
	volatile unsigned int    VEDU_VLCST_PARA_DATA38;
	volatile unsigned int    VEDU_VLCST_PARA_DATA39;
	volatile unsigned int    VEDU_VLCST_PARA_DATA40;
	volatile unsigned int    VEDU_VLCST_PARA_DATA41;
	volatile unsigned int    VEDU_VLCST_PARA_DATA42;
	volatile unsigned int    VEDU_VLCST_PARA_DATA43;
	volatile unsigned int    VEDU_VLCST_PARA_DATA44;
	volatile unsigned int    VEDU_VLCST_PARA_DATA45;
	volatile unsigned int    VEDU_VLCST_PARA_DATA46;
	volatile unsigned int    VEDU_VLCST_PARA_DATA47;
	volatile unsigned int    VEDU_PPFD_ST_ADDR0;
	volatile unsigned int    VEDU_PPFD_ST_ADDR1;
	volatile unsigned int    VEDU_PPFD_ST_LEN0;
	volatile unsigned int    VEDU_PPFD_ST_LEN1;
	volatile U_VEDU_PPFD_ST_CFG    VEDU_PPFD_ST_CFG;
	volatile U_VEDU_ENV_CHN    VEDU_ENV_CHN;
	volatile U_FUNC_VCPI_INTSTAT    FUNC_VCPI_INTSTAT;
	volatile U_FUNC_VCPI_RAWINT     FUNC_VCPI_RAWINT;
	volatile unsigned int    FUNC_VCPI_VEDU_TIMER;
	volatile unsigned int    FUNC_VCPI_IDLE_TIMER;
	volatile U_FUNC_VCPI_INTSTAT_S   FUNC_VCPI_INTSTAT_S;
	volatile U_FUNC_VCPI_RAWINT_S   FUNC_VCPI_RAWINT_S;
	volatile U_FUNC_PME_MADI_SUM    FUNC_PME_MADI_SUM;
	volatile U_FUNC_PME_MADP_SUM    FUNC_PME_MADP_SUM;
	volatile U_FUNC_PME_MADI_NUM    FUNC_PME_MADI_NUM;
	volatile U_FUNC_PME_MADP_NUM    FUNC_PME_MADP_NUM;
	volatile U_FUNC_BGGEN_BLOCK_COUNT   FUNC_BGGEN_BLOCK_COUNT;
	volatile U_FUNC_BGGEN_FRAME_BGM_DIST   FUNC_BGGEN_FRAME_BGM_DIST;
	volatile unsigned int    FUNC_CABAC_PIC_STRMSIZE;
	volatile unsigned int    FUNC_CABAC_BIT_NUM;
	volatile unsigned int    VLC_SLC_TTBITS;
	volatile unsigned int    VLC_PIC_TTBITS;
	volatile unsigned int    FUNC_VLCST_SLC_LEN_CNT;
	volatile U_FUNC_VLCST_DSRPTR00   FUNC_VLCST_DSRPTR00;
	volatile U_FUNC_VLCST_DSRPTR01   FUNC_VLCST_DSRPTR01;
	volatile U_FUNC_VLCST_DSRPTR10   FUNC_VLCST_DSRPTR10;
	volatile U_FUNC_VLCST_DSRPTR11   FUNC_VLCST_DSRPTR11;
	volatile U_FUNC_VLCST_DSRPTR20   FUNC_VLCST_DSRPTR20;
	volatile U_FUNC_VLCST_DSRPTR21   FUNC_VLCST_DSRPTR21;
	volatile U_FUNC_VLCST_DSRPTR30   FUNC_VLCST_DSRPTR30;
	volatile U_FUNC_VLCST_DSRPTR31   FUNC_VLCST_DSRPTR31;
	volatile U_FUNC_VLCST_DSRPTR40   FUNC_VLCST_DSRPTR40;
	volatile U_FUNC_VLCST_DSRPTR41   FUNC_VLCST_DSRPTR41;
	volatile U_FUNC_VLCST_DSRPTR50   FUNC_VLCST_DSRPTR50;
	volatile U_FUNC_VLCST_DSRPTR51   FUNC_VLCST_DSRPTR51;
	volatile U_FUNC_VLCST_DSRPTR60   FUNC_VLCST_DSRPTR60;
	volatile U_FUNC_VLCST_DSRPTR61   FUNC_VLCST_DSRPTR61;
	volatile U_FUNC_VLCST_DSRPTR70   FUNC_VLCST_DSRPTR70;
	volatile U_FUNC_VLCST_DSRPTR71   FUNC_VLCST_DSRPTR71;
	volatile U_FUNC_VLCST_DSRPTR80   FUNC_VLCST_DSRPTR80;
	volatile U_FUNC_VLCST_DSRPTR81   FUNC_VLCST_DSRPTR81;
	volatile U_FUNC_VLCST_DSRPTR90   FUNC_VLCST_DSRPTR90;
	volatile U_FUNC_VLCST_DSRPTR91   FUNC_VLCST_DSRPTR91;
	volatile U_FUNC_VLCST_DSRPTR100   FUNC_VLCST_DSRPTR100;
	volatile U_FUNC_VLCST_DSRPTR101   FUNC_VLCST_DSRPTR101;
	volatile U_FUNC_VLCST_DSRPTR110   FUNC_VLCST_DSRPTR110;
	volatile U_FUNC_VLCST_DSRPTR111   FUNC_VLCST_DSRPTR111;
	volatile U_FUNC_VLCST_DSRPTR120   FUNC_VLCST_DSRPTR120;
	volatile U_FUNC_VLCST_DSRPTR121   FUNC_VLCST_DSRPTR121;
	volatile U_FUNC_VLCST_DSRPTR130   FUNC_VLCST_DSRPTR130;
	volatile U_FUNC_VLCST_DSRPTR131   FUNC_VLCST_DSRPTR131;
	volatile U_FUNC_VLCST_DSRPTR140   FUNC_VLCST_DSRPTR140;
	volatile U_FUNC_VLCST_DSRPTR141   FUNC_VLCST_DSRPTR141;
	volatile U_FUNC_VLCST_DSRPTR150   FUNC_VLCST_DSRPTR150;
	volatile U_FUNC_VLCST_DSRPTR151   FUNC_VLCST_DSRPTR151;
	volatile U_FUNC_SEL_OPT_8X8_CNT   FUNC_SEL_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTRA_OPT_8X8_CNT   FUNC_SEL_INTRA_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT   FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT   FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_OPT_8X8_CNT   FUNC_SEL_INTER_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_8X8_CNT   FUNC_SEL_INTER_FME_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_8X8_CNT   FUNC_SEL_INTER_MERGE_OPT_8X8_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT   FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
	volatile U_FUNC_SEL_OPT_16X16_CNT   FUNC_SEL_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTRA_OPT_16X16_CNT   FUNC_SEL_INTRA_OPT_16X16_CNT;
	volatile U_FUNC_SEL_OPT_4X4_CNT   FUNC_SEL_OPT_4X4_CNT;
	volatile unsigned int    RESERVED0_FUNC_SEL;
	volatile U_FUNC_SEL_INTER_OPT_16X16_CNT   FUNC_SEL_INTER_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_16X16_CNT   FUNC_SEL_INTER_FME_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT   FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT   FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
	volatile U_FUNC_SEL_OPT_32X32_CNT   FUNC_SEL_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTRA_OPT_32X32_CNT   FUNC_SEL_INTRA_OPT_32X32_CNT;
	volatile unsigned int    RESERVED1_FUNC_SEL;
	volatile U_FUNC_SEL_INTER_OPT_32X32_CNT   FUNC_SEL_INTER_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_32X32_CNT   FUNC_SEL_INTER_FME_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT   FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT   FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
	volatile U_FUNC_SEL_OPT_64X64_CNT   FUNC_SEL_OPT_64X64_CNT;
	volatile U_FUNC_SEL_INTER_FME_OPT_64X64_CNT   FUNC_SEL_INTER_FME_OPT_64X64_CNT;
	volatile U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT   FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
	volatile U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT   FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
	volatile U_FUNC_SEL_TOTAL_LUMA_QP   FUNC_SEL_TOTAL_LUMA_QP;
	volatile U_FUNC_SEL_MAX_MIN_LUMA_QP   FUNC_SEL_MAX_MIN_LUMA_QP;
	volatile U_FUNC_SEL_LUMA_QP0_CNT   FUNC_SEL_LUMA_QP0_CNT;
	volatile U_FUNC_SEL_LUMA_QP1_CNT   FUNC_SEL_LUMA_QP1_CNT;
	volatile U_FUNC_SEL_LUMA_QP2_CNT   FUNC_SEL_LUMA_QP2_CNT;
	volatile U_FUNC_SEL_LUMA_QP3_CNT   FUNC_SEL_LUMA_QP3_CNT;
	volatile U_FUNC_SEL_LUMA_QP4_CNT   FUNC_SEL_LUMA_QP4_CNT;
	volatile U_FUNC_SEL_LUMA_QP5_CNT   FUNC_SEL_LUMA_QP5_CNT;
	volatile U_FUNC_SEL_LUMA_QP6_CNT   FUNC_SEL_LUMA_QP6_CNT;
	volatile U_FUNC_SEL_LUMA_QP7_CNT   FUNC_SEL_LUMA_QP7_CNT;
	volatile U_FUNC_SEL_LUMA_QP8_CNT   FUNC_SEL_LUMA_QP8_CNT;
	volatile U_FUNC_SEL_LUMA_QP9_CNT   FUNC_SEL_LUMA_QP9_CNT;
	volatile U_FUNC_SEL_LUMA_QP10_CNT   FUNC_SEL_LUMA_QP10_CNT;
	volatile U_FUNC_SEL_LUMA_QP11_CNT   FUNC_SEL_LUMA_QP11_CNT;
	volatile U_FUNC_SEL_LUMA_QP12_CNT   FUNC_SEL_LUMA_QP12_CNT;
	volatile U_FUNC_SEL_LUMA_QP13_CNT   FUNC_SEL_LUMA_QP13_CNT;
	volatile U_FUNC_SEL_LUMA_QP14_CNT   FUNC_SEL_LUMA_QP14_CNT;
	volatile U_FUNC_SEL_LUMA_QP15_CNT   FUNC_SEL_LUMA_QP15_CNT;
	volatile U_FUNC_SEL_LUMA_QP16_CNT   FUNC_SEL_LUMA_QP16_CNT;
	volatile U_FUNC_SEL_LUMA_QP17_CNT   FUNC_SEL_LUMA_QP17_CNT;
	volatile U_FUNC_SEL_LUMA_QP18_CNT   FUNC_SEL_LUMA_QP18_CNT;
	volatile U_FUNC_SEL_LUMA_QP19_CNT   FUNC_SEL_LUMA_QP19_CNT;
	volatile U_FUNC_SEL_LUMA_QP20_CNT   FUNC_SEL_LUMA_QP20_CNT;
	volatile U_FUNC_SEL_LUMA_QP21_CNT   FUNC_SEL_LUMA_QP21_CNT;
	volatile U_FUNC_SEL_LUMA_QP22_CNT   FUNC_SEL_LUMA_QP22_CNT;
	volatile U_FUNC_SEL_LUMA_QP23_CNT   FUNC_SEL_LUMA_QP23_CNT;
	volatile U_FUNC_SEL_LUMA_QP24_CNT   FUNC_SEL_LUMA_QP24_CNT;
	volatile U_FUNC_SEL_LUMA_QP25_CNT   FUNC_SEL_LUMA_QP25_CNT;
	volatile U_FUNC_SEL_LUMA_QP26_CNT   FUNC_SEL_LUMA_QP26_CNT;
	volatile U_FUNC_SEL_LUMA_QP27_CNT   FUNC_SEL_LUMA_QP27_CNT;
	volatile U_FUNC_SEL_LUMA_QP28_CNT   FUNC_SEL_LUMA_QP28_CNT;
	volatile U_FUNC_SEL_LUMA_QP29_CNT   FUNC_SEL_LUMA_QP29_CNT;
	volatile U_FUNC_SEL_LUMA_QP30_CNT   FUNC_SEL_LUMA_QP30_CNT;
	volatile U_FUNC_SEL_LUMA_QP31_CNT   FUNC_SEL_LUMA_QP31_CNT;
	volatile U_FUNC_SEL_LUMA_QP32_CNT   FUNC_SEL_LUMA_QP32_CNT;
	volatile U_FUNC_SEL_LUMA_QP33_CNT   FUNC_SEL_LUMA_QP33_CNT;
	volatile U_FUNC_SEL_LUMA_QP34_CNT   FUNC_SEL_LUMA_QP34_CNT;
	volatile U_FUNC_SEL_LUMA_QP35_CNT   FUNC_SEL_LUMA_QP35_CNT;
	volatile U_FUNC_SEL_LUMA_QP36_CNT   FUNC_SEL_LUMA_QP36_CNT;
	volatile U_FUNC_SEL_LUMA_QP37_CNT   FUNC_SEL_LUMA_QP37_CNT;
	volatile U_FUNC_SEL_LUMA_QP38_CNT   FUNC_SEL_LUMA_QP38_CNT;
	volatile U_FUNC_SEL_LUMA_QP39_CNT   FUNC_SEL_LUMA_QP39_CNT;
	volatile U_FUNC_SEL_LUMA_QP40_CNT   FUNC_SEL_LUMA_QP40_CNT;
	volatile U_FUNC_SEL_LUMA_QP41_CNT   FUNC_SEL_LUMA_QP41_CNT;
	volatile U_FUNC_SEL_LUMA_QP42_CNT   FUNC_SEL_LUMA_QP42_CNT;
	volatile U_FUNC_SEL_LUMA_QP43_CNT   FUNC_SEL_LUMA_QP43_CNT;
	volatile U_FUNC_SEL_LUMA_QP44_CNT   FUNC_SEL_LUMA_QP44_CNT;
	volatile U_FUNC_SEL_LUMA_QP45_CNT   FUNC_SEL_LUMA_QP45_CNT;
	volatile U_FUNC_SEL_LUMA_QP46_CNT   FUNC_SEL_LUMA_QP46_CNT;
	volatile U_FUNC_SEL_LUMA_QP47_CNT   FUNC_SEL_LUMA_QP47_CNT;
	volatile U_FUNC_SEL_LUMA_QP48_CNT   FUNC_SEL_LUMA_QP48_CNT;
	volatile U_FUNC_SEL_LUMA_QP49_CNT   FUNC_SEL_LUMA_QP49_CNT;
	volatile U_FUNC_SEL_LUMA_QP50_CNT   FUNC_SEL_LUMA_QP50_CNT;
	volatile U_FUNC_SEL_LUMA_QP51_CNT   FUNC_SEL_LUMA_QP51_CNT;
	volatile unsigned int    FUNC_SAO_MSE_SUM;
	volatile U_FUNC_SAO_MSE_CNT    FUNC_SAO_MSE_CNT;
	volatile U_FUNC_SAO_MSE_MAX    FUNC_SAO_MSE_MAX;
	volatile unsigned int    FUNC_SAO_SSD_AERA0_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA1_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA2_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA3_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA4_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA5_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA6_SUM;
	volatile unsigned int    FUNC_SAO_SSD_AERA7_SUM;
	volatile U_FUNC_SAO_OFF_NUM    FUNC_SAO_OFF_NUM;
	volatile U_FUNC_SAO_LCU_CNT    FUNC_SAO_LCU_CNT;

	volatile U_MMU_PRE_GLB_SCR    MMU_PRE_GLB_SCR;  //8000

	volatile unsigned int    MMU_PRE_NBI_MVST_ADDR_STR;
	volatile unsigned int    MMU_PRE_NBI_MVST_ADDR_END;
	volatile unsigned int    MMU_PRE_NBI_MVLD_ADDR_STR;
	volatile unsigned int    MMU_PRE_NBI_MVLD_ADDR_END;
	volatile unsigned int    MMU_PRE_PMEST_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEST_ADDR_END;
	volatile unsigned int    MMU_PRE_PMELD_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMELD_ADDR_END;
	volatile unsigned int    MMU_PRE_PMEINFOST_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEINFOST_ADDR_END;
	volatile unsigned int    MMU_PRE_PMEINFOLD0_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEINFOLD0_ADDR_END;

	volatile unsigned int    MMU_PRE_PMEINFOLD1_ADDR_STR;
	volatile unsigned int    MMU_PRE_PMEINFOLD1_ADDR_END;
	volatile unsigned int    MMU_PRE_QPGLD_ADDR_STR;
	volatile unsigned int    MMU_PRE_QPGLD_ADDR_END;
	volatile unsigned int    MMU_PRE_REC_YH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REC_YH_ADDR_END;
	volatile unsigned int    MMU_PRE_REC_CH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REC_CH_ADDR_END;
	volatile unsigned int    MMU_PRE_REC_YADDR_STR;
	volatile unsigned int    MMU_PRE_REC_YADDR_END;
	volatile unsigned int    MMU_PRE_REC_CADDR_STR;
	volatile unsigned int    MMU_PRE_REC_CADDR_END;
	volatile unsigned int    MMU_PRE_REF_YH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REF_YH_ADDR_END;
	volatile unsigned int    MMU_PRE_REF_CH_ADDR_STR;
	volatile unsigned int    MMU_PRE_REF_CH_ADDR_END;
	volatile unsigned int    MMU_PRE_REF_YADDR_STR;
	volatile unsigned int    MMU_PRE_REF_YADDR_END;
	volatile unsigned int    MMU_PRE_REF_CADDR_STR;
	volatile unsigned int    MMU_PRE_REF_CADDR_END;
	volatile unsigned int    MMU_PRE_SRC_YHADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_YHADDR_END;
	volatile unsigned int    MMU_PRE_SRC_CHADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_CHADDR_END;
	volatile unsigned int    MMU_PRE_SRC_YADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_YADDR_END;
	volatile unsigned int    MMU_PRE_SRC_CADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_CADDR_END;
	volatile unsigned int    MMU_PRE_SRC_VADDR_STR;
	volatile unsigned int    MMU_PRE_SRC_VADDR_END;
	volatile unsigned int    MMU_PRE_LOWDLY_ADDR_STR;
	volatile unsigned int    MMU_PRE_LOWDLY_ADDR_END;
	volatile unsigned int    MMU_PRE_PPS_ADDR_STR;
	volatile unsigned int    MMU_PRE_PPS_ADDR_END;
	volatile unsigned int    MMU_PRE_STRMADDR0_STR;
	volatile unsigned int    MMU_PRE_STRMADDR0_END;
	volatile unsigned int    MMU_PRE_STRMADDR1_STR;
	volatile unsigned int    MMU_PRE_STRMADDR1_END;
	volatile unsigned int    MMU_PRE_STRMADDR2_STR;
	volatile unsigned int    MMU_PRE_STRMADDR2_END;
	volatile unsigned int    MMU_PRE_STRMADDR3_STR;
	volatile unsigned int    MMU_PRE_STRMADDR3_END;
	volatile unsigned int    MMU_PRE_STRMADDR4_STR;
	volatile unsigned int    MMU_PRE_STRMADDR4_END;
	volatile unsigned int    MMU_PRE_STRMADDR5_STR;
	volatile unsigned int    MMU_PRE_STRMADDR5_END;
	volatile unsigned int    MMU_PRE_STRMADDR6_STR;
	volatile unsigned int    MMU_PRE_STRMADDR6_END;
	volatile unsigned int    MMU_PRE_STRMADDR7_STR;
	volatile unsigned int    MMU_PRE_STRMADDR7_END;
	volatile unsigned int    MMU_PRE_STRMADDR8_STR;
	volatile unsigned int    MMU_PRE_STRMADDR8_END;
	volatile unsigned int    MMU_PRE_STRMADDR9_STR;
	volatile unsigned int    MMU_PRE_STRMADDR9_END;
	volatile unsigned int    MMU_PRE_STRMADDR10_STR;
	volatile unsigned int    MMU_PRE_STRMADDR10_END;
	volatile unsigned int    MMU_PRE_STRMADDR11_STR;
	volatile unsigned int    MMU_PRE_STRMADDR11_END;
	volatile unsigned int    MMU_PRE_STRMADDR12_STR;
	volatile unsigned int    MMU_PRE_STRMADDR12_END;
	volatile unsigned int    MMU_PRE_STRMADDR13_STR;
	volatile unsigned int    MMU_PRE_STRMADDR13_END;
	volatile unsigned int    MMU_PRE_STRMADDR14_STR;
	volatile unsigned int    MMU_PRE_STRMADDR14_END;
	volatile unsigned int    MMU_PRE_STRMADDR15_STR;
	volatile unsigned int    MMU_PRE_STRMADDR15_END;
	volatile U_MMU_PRE_DFX_ARERR_FLAG   MMU_PRE_DFX_ARERR_FLAG;
	volatile U_MMU_PRE_DFX_ARERR_ID   MMU_PRE_DFX_ARERR_ID;
	volatile unsigned int    MMU_PRE_DFX_ARERR_ADDR;
	volatile unsigned int    MST_PRE_RESERVED_2;
	volatile U_MMU_PRE_DFX_AWERR_FLAG   MMU_PRE_DFX_AWERR_FLAG;
	volatile U_MMU_PRE_DFX_AWERR_ID   MMU_PRE_DFX_AWERR_ID;
	volatile unsigned int    MMU_PRE_DFX_AWERR_ADDR;

	volatile U_AXIDFX_ERR    AXIDFX_ERR;  //9000

	volatile U_AXIDFX_AR_R_CNT    AXIDFX_AR_R_CNT;
	volatile U_AXIDFX_AW_W_CNT    AXIDFX_AW_W_CNT;
	volatile U_AXIDFX_AW_B_CNT    AXIDFX_AW_B_CNT;
	volatile U_AXIDFX_AR_R_ID_ERR   AXIDFX_AR_R_ID_ERR;
	volatile U_AXIDFX_ERR_ARID    AXIDFX_ERR_ARID;
	volatile U_AXIDFX_ERR_RID    AXIDFX_ERR_RID;
	volatile U_AXIDFX_AW_W_B_ID_ERR   AXIDFX_AW_W_B_ID_ERR;
	volatile U_AXIDFX_ERR_AWID    AXIDFX_ERR_AWID;
	volatile U_AXIDFX_ERR_WID    AXIDFX_ERR_WID;
	volatile U_AXIDFX_ERR_BID    AXIDFX_ERR_BID;
	volatile U_AXIDFX_ARID_TX_0ERR   AXIDFX_ARID_TX_0ERR;
	volatile U_AXIDFX_ARID_TX_1ERR   AXIDFX_ARID_TX_1ERR;
	volatile U_AXIDFX_ARID_TX_2ERR   AXIDFX_ARID_TX_2ERR;
	volatile U_AXIDFX_RID_RX_0ERR   AXIDFX_RID_RX_0ERR;
	volatile U_AXIDFX_RID_RX_1ERR   AXIDFX_RID_RX_1ERR;
	volatile U_AXIDFX_RID_RX_2ERR   AXIDFX_RID_RX_2ERR;
	volatile U_AXIDFX_ARID_RX_0ERR   AXIDFX_ARID_RX_0ERR;
	volatile U_AXIDFX_BID_RX_ERR    AXIDFX_BID_RX_ERR;
	volatile U_AXIDFX_ARID_LEN_0ERR   AXIDFX_ARID_LEN_0ERR;
	volatile U_AXIDFX_ARID_LEN_1ERR   AXIDFX_ARID_LEN_1ERR;
	volatile U_AXIDFX_ARID_LEN_2ERR   AXIDFX_ARID_LEN_2ERR;
	volatile unsigned int    AXIDFX_AWLEN_CNT;
	volatile unsigned int    AXIDFX_WLEN_CNT;
	volatile U_AXIDFX_RESP_ERR    AXIDFX_RESP_ERR;
	volatile U_AXIDFX_ERR_RESP    AXIDFX_ERR_RESP;
	volatile U_AXIDFX_LEN_ERR    AXIDFX_LEN_ERR;
	volatile U_AXIDFX_ERR_LEN    AXIDFX_ERR_LEN;
	volatile unsigned int    AXIDFX_0RID_FLAG;
	volatile unsigned int    AXIDFX_1RID_FLAG;
	volatile U_AXIDFX_2RID_FLAG    AXIDFX_2RID_FLAG;
	volatile U_AXIDFX_WID_FLAG    AXIDFX_WID_FLAG;
	volatile U_AXIDFX_AXI_ST    AXIDFX_AXI_ST;
	volatile U_AXIDFX_SOFT_RST_REQ   AXIDFX_SOFT_RST_REQ;
	volatile U_AXIDFX_SOFT_RST_ACK   AXIDFX_SOFT_RST_ACK;
	volatile U_AXIDFX_SOFT_RST_FORCE_REQ_ACK   AXIDFX_SOFT_RST_FORCE_REQ_ACK;
	volatile unsigned int    AXIDFX_SOFT_RST_STATE0;
	volatile unsigned int    AXIDFX_SOFT_RST_STATE1;

	volatile U_SMMU_MSTR_GLB_BYPASS   SMMU_MSTR_GLB_BYPASS;  //30000
	volatile U_SMMU_MSTR_DEBUG_MODE   SMMU_MSTR_DEBUG_MODE;
	volatile U_SMMU_MSTR_MEM_CTRL   SMMU_MSTR_MEM_CTRL;
	volatile U_SMMU_MSTR_CLK_EN    SMMU_MSTR_CLK_EN;
	volatile unsigned int    SMMU_MSTR_END_REQ_0;
	volatile unsigned int    SMMU_MSTR_END_REQ_1;
	volatile U_SMMU_MSTR_END_REQ_2   SMMU_MSTR_END_REQ_2;
	volatile unsigned int    SMMU_MSTR_END_ACK_0;
	volatile unsigned int    SMMU_MSTR_END_ACK_1;
	volatile U_SMMU_MSTR_END_ACK_2   SMMU_MSTR_END_ACK_2;
	volatile unsigned int    SMMU_MSTR_SMRX_START_0;
	volatile unsigned int    SMMU_MSTR_SMRX_START_1;
	volatile U_SMMU_MSTR_SMRX_START_2   SMMU_MSTR_SMRX_START_2;
	volatile U_SMMU_MSTR_INPT_SEL   SMMU_MSTR_INPT_SEL;
	volatile U_SMMU_MSTR_INTMASK    SMMU_MSTR_INTMASK;
	volatile U_SMMU_MSTR_INTRAW    SMMU_MSTR_INTRAW;
	volatile U_SMMU_MSTR_INTSTAT    SMMU_MSTR_INTSTAT;
	volatile U_SMMU_MSTR_INTCLR    SMMU_MSTR_INTCLR;
	volatile U_SMMU_MSTR_DBG_0    SMMU_MSTR_DBG_0;
	volatile unsigned int    SMMU_MSTR_DBG_1;
	volatile U_SMMU_MSTR_DBG_2    SMMU_MSTR_DBG_2;
	volatile unsigned int    SMMU_MSTR_DBG_3;
	volatile U_SMMU_MSTR_DBG_4    SMMU_MSTR_DBG_4;
	volatile unsigned int    SMMU_MSTR_DBG_5;
	volatile U_SMMU_MSTR_DBG_PORT_IN_0   SMMU_MSTR_DBG_PORT_IN_0;
	volatile unsigned int    SMMU_MSTR_DBG_PORT_IN_1;
	volatile unsigned int    SMMU_MSTR_DBG_PORT_OUT;
	volatile U_SMMU_MSTR_SMRX_0    SMMU_MSTR_SMRX_0[88];
	volatile U_SMMU_MSTR_SMRX_1    SMMU_MSTR_SMRX_1[88];
	volatile U_SMMU_MSTR_SMRX_2    SMMU_MSTR_SMRX_2[144];
	volatile unsigned int    RD_CMD_TOTAL_CNT[88];
	volatile unsigned int    RD_CMD_MISS_CNT[88];
	volatile unsigned int    RD_DATA_TOTAL_CNT[88];
	volatile unsigned int    RD_CMD_CASE_CNT[6];
	volatile U_RD_CMD_TRANS_LATENCY   RD_CMD_TRANS_LATENCY[50];
	volatile unsigned int    WR_CMD_TOTAL_CNT[88];
	volatile unsigned int    WR_CMD_MISS_CNT[88];
	volatile unsigned int    WR_DATA_TOTAL_CNT[88];
	volatile unsigned int    WR_CMD_CASE_CNT[6];
	volatile U_WR_CMD_TRANS_LATENCY   WR_CMD_TRANS_LATENCY[50];
	volatile U_SMMU_SCR    SMMU_SCR; //40000
	volatile U_SMMU_MEMCTRL    SMMU_MEMCTRL;
	volatile U_SMMU_LP_CTRL    SMMU_LP_CTRL;
	volatile U_SMMU_PRESS_REMAP    SMMU_PRESS_REMAP;
	volatile U_SMMU_INTMASK_NS    SMMU_INTMASK_NS;
	volatile U_SMMU_INTRAW_NS    SMMU_INTRAW_NS;
	volatile U_SMMU_INTSTAT_NS    SMMU_INTSTAT_NS;
	volatile U_SMMU_INTCLR_NS    SMMU_INTCLR_NS;
	volatile U_SMMU_SMRX_NS    SMMU_SMRX_NS[88];
	volatile unsigned int    SMMU_RLD_EN0_NS;
	volatile unsigned int    SMMU_RLD_EN1_NS;
	volatile U_SMMU_RLD_EN2_NS    SMMU_RLD_EN2_NS[2];
	volatile U_SMMU_CB_SCTRL    SMMU_CB_SCTRL;
	volatile unsigned int    SMMU_CB_TTBR0;
	volatile unsigned int    SMMU_CB_TTBR1;
	volatile U_SMMU_CB_TTBCR    SMMU_CB_TTBCR;
	volatile U_SMMU_OFFSET_ADDR_NS   SMMU_OFFSET_ADDR_NS;
	volatile U_SMMU_SCACHEI_ALL    SMMU_SCACHEI_ALL;
	volatile U_SMMU_SCACHEI_L1    SMMU_SCACHEI_L1;
	volatile U_SMMU_SCACHEI_L2L3    SMMU_SCACHEI_L2L3;
	volatile U_SMMU_FAMA_CTRL0_NS   SMMU_FAMA_CTRL0_NS;
	volatile U_SMMU_FAMA_CTRL1_NS   SMMU_FAMA_CTRL1_NS;
	volatile U_SMMU_ADDR_MSB    SMMU_ADDR_MSB;
	volatile unsigned int    SMMU_ERR_RDADDR;
	volatile unsigned int    SMMU_ERR_WRADDR;
	volatile unsigned int    SMMU_FAULT_ADDR_TCU;
	volatile U_SMMU_FAULT_ID_TCU    SMMU_FAULT_ID_TCU[3];
	volatile unsigned int    SMMU_FAULT_ADDR_TBUX;
	volatile U_SMMU_FAULT_ID_TBUX   SMMU_FAULT_ID_TBUX;
	volatile U_SMMU_FAULT_INFOX    SMMU_FAULT_INFOX[22];
	volatile U_SMMU_DBGRPTR_TLB    SMMU_DBGRPTR_TLB;
	volatile U_SMMU_DBGRDATA_TLB    SMMU_DBGRDATA_TLB;
	volatile U_SMMU_DBGRPTR_CACHE   SMMU_DBGRPTR_CACHE;
	volatile U_SMMU_DBGRDATA0_CACHE   SMMU_DBGRDATA0_CACHE;
	volatile U_SMMU_DBGRDATA1_CACHE   SMMU_DBGRDATA1_CACHE;
	volatile U_SMMU_DBGAXI_CTRL    SMMU_DBGAXI_CTRL;
	volatile unsigned int    SMMU_OVA_ADDR;
	volatile U_SMMU_OPA_ADDR    SMMU_OPA_ADDR;
	volatile U_SMMU_OVA_CTRL    SMMU_OVA_CTRL;
	volatile unsigned int    SMMU_OPREF_ADDR;
	volatile U_SMMU_OPREF_CTRL    SMMU_OPREF_CTRL;
	volatile unsigned int    SMMU_OPREF_CNT[85];
	volatile U_SMMU_SMRX_S    SMMU_SMRX_S[88];
	volatile unsigned int    SMMU_RLD_EN0_S;
	volatile unsigned int    SMMU_RLD_EN1_S;
	volatile U_SMMU_RLD_EN2_S    SMMU_RLD_EN2_S[2];
	volatile U_SMMU_INTMAS_S    SMMU_INTMAS_S;
	volatile U_SMMU_INTRAW_S    SMMU_INTRAW_S;
	volatile U_SMMU_INTSTAT_S    SMMU_INTSTAT_S;
	volatile U_SMMU_INTCLR_S    SMMU_INTCLR_S;
	volatile U_SMMU_SCR_S    SMMU_SCR_S;
	volatile U_SMMU_SCB_SCTRL    SMMU_SCB_SCTRL;
	volatile unsigned int    SMMU_SCB_TTBR;
	volatile U_SMMU_SCB_TTBCR    SMMU_SCB_TTBCR;
	volatile U_SMMU_OFFSET_ADDR_S   SMMU_OFFSET_ADDR_S;
	volatile U_SMMU_FAMA_CTRL0_S    SMMU_FAMA_CTRL0_S;
	volatile U_SMMU_FAMA_CTRL1_S    SMMU_FAMA_CTRL1_S;
	volatile U_SMMU_DBGRPTR_TLB_S   SMMU_DBGRPTR_TLB_S;
	volatile U_SMMU_DBGRPTR_CACHE_S   SMMU_DBGRPTR_CACHE_S;
	volatile U_SMMU_OVERRIDE_CTRL_S   SMMU_OVERRIDE_CTRL_S;
	volatile U_SMMU_SMRX_P    SMMU_SMRX_P[88];
	volatile unsigned int    SMMU_RLD_EN0_P;
	volatile unsigned int    SMMU_RLD_EN1_P;
	volatile U_SMMU_RLD_EN2_P    SMMU_RLD_EN2_P[2];
	volatile U_SMMU_INTMAS_P    SMMU_INTMAS_P;
	volatile U_SMMU_INTRAW_P    SMMU_INTRAW_P;
	volatile U_SMMU_INTSTAT_P    SMMU_INTSTAT_P;
	volatile U_SMMU_INTCLR_P    SMMU_INTCLR_P;
	volatile U_SMMU_SCR_P    SMMU_SCR_P;
	volatile U_SMMU_PCB_SCTRL    SMMU_PCB_SCTRL;
	volatile unsigned int    SMMU_PCB_TTBR;
	volatile U_SMMU_PCB_TTBCR    SMMU_PCB_TTBCR;
	volatile U_SMMU_OFFSET_ADDR_P   SMMU_OFFSET_ADDR_P;
	volatile U_SMMU_FAMA_CTRL0_P    SMMU_FAMA_CTRL0_P;
	volatile U_SMMU_FAMA_CTRL1_P    SMMU_FAMA_CTRL1_P;

} S_HEVC_AVC_REGS_TYPE_CFG;

#endif
