//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  mcf_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-01-03 Create file
//******************************************************************************

#ifndef __MCF_DRV_PRIV_CS_H__
#define __MCF_DRV_PRIV_CS_H__

/* Define the union U_MCF_CFG */
union u_mcf_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    op_mode               : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 2   ; /* [3..2]  */
		unsigned int    y_gf_mode_in          : 1   ; /* [4]  */
		unsigned int    reserved_1            : 1   ; /* [5]  */
		unsigned int    y_gf_mode_out         : 2   ; /* [7..6]  */
		unsigned int    uv_pack               : 1   ; /* [8]  */
		unsigned int    reserved_2            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_MCF_SIZE_CFG */
union u_mcf_size_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    width                 : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    height                : 13  ; /* [28..16]  */
		unsigned int    reserved_1            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UV_FSN_GET_CAND_CFG */
union u_uv_fsn_get_cand_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    th_u                  : 8   ; /* [7..0]  */
		unsigned int    th_v                  : 8   ; /* [15..8]  */
		unsigned int    const_value           : 8   ; /* [23..16]  */
		unsigned int    reserved_0            : 4   ; /* [27..24]  */
		unsigned int    input_cnt             : 2   ; /* [29..28]  */
		unsigned int    reserved_1            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UV_FSN_GET_FUV_RATIO_CFG */
union u_uv_fsn_get_fuv_ratio_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    ratio_dist_uv_to_conf : 9   ; /* [8..0]  */
		unsigned int    reserved_0            : 3   ; /* [11..9]  */
		unsigned int    ratio_dist_to_conf    : 9   ; /* [20..12]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UV_FSN_GET_FUV_MAX_U_DIFF_CFG */
union u_uv_fsn_get_fuv_max_u_diff_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    max_u_diff_const_inv  : 18  ; /* [17..0]  */
		unsigned int    reserved_0            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UV_FSN_GET_FUV_MAX_V_DIFF_CFG */
union u_uv_fsn_get_fuv_max_v_diff_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    max_v_diff_const_inv  : 18  ; /* [17..0]  */
		unsigned int    reserved_0            : 14  ; /* [31..18]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UV_FSN_GET_CONF_CFG */
union u_uv_fsn_get_conf_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    diff_hue_shift        : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    diff_hue_shift_ex     : 6   ; /* [9..4]  */
		unsigned int    reserved_1            : 6   ; /* [15..10]  */
		unsigned int    diff_hue_coff         : 11  ; /* [26..16]  */
		unsigned int    reserved_2            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UV_FSN_GET_CONF_MAX_DIFF_CFG */
union u_uv_fsn_get_conf_max_diff_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    max_diff_inv          : 18  ; /* [17..0]  */
		unsigned int    reserved_0            : 2   ; /* [19..18]  */
		unsigned int    max_diff              : 8   ; /* [27..20]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_FILL_CONFLICT_CFG */
union u_fill_conflict_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    conflict_th           : 8   ; /* [7..0]  */
		unsigned int    luma_th               : 8   ; /* [15..8]  */
		unsigned int    reserved_0            : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMOOTH_BLEND_CFG */
union u_smooth_blend_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    gauss_radius          : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 2   ; /* [3..2]  */
		unsigned int    mask_blend            : 8   ; /* [11..4]  */
		unsigned int    reserved_1            : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMOOTH_BLEND_GF_CFG */
union u_smooth_blend_gf_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    uv_gf_radius          : 1   ; /* [0]  */
		unsigned int    reserved_0            : 3   ; /* [3..1]  */
		unsigned int    uv_gf_eps             : 21  ; /* [24..4]  */
		unsigned int    reserved_1            : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_ADJUST_COLOR_GF_CFG */
union u_adjust_color_gf_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    y_gf_radius           : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 2   ; /* [3..2]  */
		unsigned int    y_gf_eps              : 23  ; /* [26..4]  */
		unsigned int    reserved_1            : 5   ; /* [31..27]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_ADJUST_COLOR_GET_MASK_CFG */
union u_adjust_color_get_mask_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    mask_sub              : 8   ; /* [7..0]  */
		unsigned int    mask_mult             : 8   ; /* [15..8]  */
		unsigned int    reserved_0            : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_ADJUST_COLOR_GET_RATIO_CFG */
union u_adjust_color_get_ratio_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    ratio_par_down        : 9   ; /* [8..0]  */
		unsigned int    reserved_0            : 3   ; /* [11..9]  */
		unsigned int    ratio_par_up          : 9   ; /* [20..12]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_MCF_DEBUG */
union u_mcf_debug {
	/* Define the struct bits  */
	struct {
		unsigned int mcf_debug              : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_UV_FSN_DEBUG */
union u_uv_fsn_debug {
	/* Define the struct bits  */
	struct {
		unsigned int uv_fsn_debug           : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_FILL_CONFLICT_DEBUG */
union u_fill_conflict_debug {
	/* Define the struct bits  */
	struct {
		unsigned int filled_conflict_debug  : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_SMOOTH_BLEND_DEBUG */
union u_smooth_blend_debug {
	/* Define the struct bits  */
	struct {
		unsigned int smooth_blend_debug     : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_ADJUST_COLOR_DEBUG */
union u_adjust_color_debug {
	/* Define the struct bits  */
	struct {
		unsigned int adjust_color_debug     : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
//==============================================================================
/* Define the global struct */
struct s_mcf_regs_type {
	union u_mcf_cfg              mcf_cfg                           ; /* 0x0 */
	union u_mcf_size_cfg         mcf_size_cfg                      ; /* 0x4 */
	union u_uv_fsn_get_cand_cfg   uv_fsn_get_cand_cfg              ; /* 0x8 */
	union u_uv_fsn_get_fuv_ratio_cfg   uv_fsn_get_fuv_ratio_cfg    ; /* 0xc */
	union u_uv_fsn_get_fuv_max_u_diff_cfg   uv_fsn_get_fuv_max_u_diff_cfg ; /* 0x10 */
	union u_uv_fsn_get_fuv_max_v_diff_cfg   uv_fsn_get_fuv_max_v_diff_cfg ; /* 0x14 */
	union u_uv_fsn_get_conf_cfg   uv_fsn_get_conf_cfg              ; /* 0x18 */
	union u_uv_fsn_get_conf_max_diff_cfg   uv_fsn_get_conf_max_diff_cfg ; /* 0x1c */
	union u_fill_conflict_cfg    fill_conflict_cfg                 ; /* 0x20 */
	union u_smooth_blend_cfg     smooth_blend_cfg                  ; /* 0x24 */
	union u_smooth_blend_gf_cfg   smooth_blend_gf_cfg              ; /* 0x28 */
	union u_adjust_color_gf_cfg   adjust_color_gf_cfg              ; /* 0x2c */
	union u_adjust_color_get_mask_cfg   adjust_color_get_mask_cfg  ; /* 0x30 */
	union u_adjust_color_get_ratio_cfg   adjust_color_get_ratio_cfg ; /* 0x34 */
	union u_mcf_debug            mcf_debug                         ; /* 0x38 */
	union u_uv_fsn_debug         uv_fsn_debug                      ; /* 0x3c */
	union u_fill_conflict_debug   fill_conflict_debug              ; /* 0x40 */
	union u_smooth_blend_debug   smooth_blend_debug                ; /* 0x44 */
	union u_adjust_color_debug   adjust_color_debug                ; /* 0x48 */

};

/* Declare the struct pointor of the module MCF */
extern struct s_mcf_regs_type *gopmcfallreg;


#endif /* __MCF_DRV_PRIV_CS_H__ */
