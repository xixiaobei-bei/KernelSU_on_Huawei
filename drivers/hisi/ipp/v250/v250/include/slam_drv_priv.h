//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  slam_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-07-30
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-07-30 Create file
//******************************************************************************

#ifndef __SLAM_DRV_PRIV_CS_H__
#define __SLAM_DRV_PRIV_CS_H__

/* Define the union U_SLAM_CFG */
union u_slam_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    pyramid_en            : 1   ; /* [0]  */
		unsigned int    gsblur_en             : 1   ; /* [1]  */
		unsigned int    fast_en               : 1   ; /* [2]  */
		unsigned int    nms_en                : 1   ; /* [3]  */
		unsigned int    orient_en             : 1   ; /* [4]  */
		unsigned int    brief_en              : 1   ; /* [5]  */
		unsigned int    freak_en              : 1   ; /* [6]  */
		unsigned int    scoremap_en           : 1   ; /* [7]  */
		unsigned int    gridstat_en           : 1   ; /* [8]  */
		unsigned int    undistort_en          : 1   ; /* [9]  */
		unsigned int    reserved_0            : 22  ; /* [31..10]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_IMAGE_SIZE */
union u_image_size {
	/* Define the struct bits */
	struct {
		unsigned int    width                 : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    height                : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_TOTAL_KPT_NUM */
union u_total_kpt_num {
	/* Define the struct bits */
	struct {
		unsigned int    total_kpt_num         : 15  ; /* [14..0]  */
		unsigned int    reserved_0            : 17  ; /* [31..15]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_PYRAMID_INC_CFG */
union u_pyramid_inc_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    scl_inc               : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_PYRAMID_VCROP_CFGB */
union u_pyramid_vcrop_cfgb {
	/* Define the struct bits */
	struct {
		unsigned int    scl_vbottom           : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_PYRAMID_VCROP_CFGT */
union u_pyramid_vcrop_cfgt {
	/* Define the struct bits */
	struct {
		unsigned int    scl_vtop              : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_PYRAMID_HCROP_CFGR */
union u_pyramid_hcrop_cfgr {
	/* Define the struct bits */
	struct {
		unsigned int    scl_hright            : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_PYRAMID_HCROP_CFGL */
union u_pyramid_hcrop_cfgl {
	/* Define the struct bits */
	struct {
		unsigned int    scl_hleft             : 28  ; /* [27..0]  */
		unsigned int    reserved_0            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_GSBLUR_COEF_01 */
union u_gsblur_coef_01 {
	/* Define the struct bits */
	struct {
		unsigned int    coeff_gauss_1         : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    coeff_gauss_0         : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_GSBLUR_COEF_23 */
union u_gsblur_coef_23 {
	/* Define the struct bits */
	struct {
		unsigned int    coeff_gauss_3         : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 6   ; /* [15..10]  */
		unsigned int    coeff_gauss_2         : 10  ; /* [25..16]  */
		unsigned int    reserved_1            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_THRESHOLD_CFG */
union u_threshold_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    min_th                : 8   ; /* [7..0]  */
		unsigned int    ini_th                : 8   ; /* [15..8]  */
		unsigned int    reserved_0            : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_NMS_WIN_CFG */
union u_nms_win_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    nmscell_v             : 4   ; /* [3..0]  */
		unsigned int    nmscell_h             : 5   ; /* [8..4]  */
		unsigned int    reserved_0            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_BLOCK_NUM_CFG */
union u_block_num_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    blk_v_num             : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    blk_h_num             : 5   ; /* [12..8]  */
		unsigned int    reserved_1            : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_BLOCK_SIZE_CFG */
union u_block_size_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    blk_v_size            : 10  ; /* [9..0]  */
		unsigned int    reserved_0            : 2   ; /* [11..10]  */
		unsigned int    blk_h_size            : 10  ; /* [21..12]  */
		unsigned int    reserved_1            : 10  ; /* [31..22]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_OCTREE_CFG */
union u_octree_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    grid_max_kpnum        : 8   ; /* [7..0]  */
		unsigned int    max_kpnum             : 15  ; /* [22..8]  */
		unsigned int    reserved_0            : 9   ; /* [31..23]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_INC_LUT_CFG */
union u_inc_lut_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    inc_level             : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_CX */
union u_undistort_cx {
	/* Define the struct bits */
	struct {
		unsigned int    cx                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_CY */
union u_undistort_cy {
	/* Define the struct bits */
	struct {
		unsigned int    cy                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_FX */
union u_undistort_fx {
	/* Define the struct bits */
	struct {
		unsigned int    fx                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_FY */
union u_undistort_fy {
	/* Define the struct bits */
	struct {
		unsigned int    fy                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_INVFX */
union u_undistort_invfx {
	/* Define the struct bits */
	struct {
		unsigned int    invfx                 : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_INVFY */
union u_undistort_invfy {
	/* Define the struct bits */
	struct {
		unsigned int    invfy                 : 20  ; /* [19..0]  */
		unsigned int    reserved_0            : 12  ; /* [31..20]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_K1 */
union u_undistort_k1 {
	/* Define the struct bits */
	struct {
		unsigned int    k1                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_K2 */
union u_undistort_k2 {
	/* Define the struct bits */
	struct {
		unsigned int    k2                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_K3 */
union u_undistort_k3 {
	/* Define the struct bits */
	struct {
		unsigned int    k3                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_P1 */
union u_undistort_p1 {
	/* Define the struct bits */
	struct {
		unsigned int    p1                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_UNDISTORT_P2 */
union u_undistort_p2 {
	/* Define the struct bits */
	struct {
		unsigned int    p2                    : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_CFG_0 */
union u_cvdr_cfg_0 {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_size        : 13  ; /* [12..0]  */
		unsigned int    reserved_0            : 3   ; /* [15..13]  */
		unsigned int    vprd_horizontal_blanking : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CVDR_CFG_1 */
union u_cvdr_cfg_1 {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_stride      : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    vprd_line_wrap        : 13  ; /* [28..16]  */
		unsigned int    reserved_1            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_DEBUG 0 */
union u_debug_0 {
	/* Define the struct bits  */
	struct {
		unsigned int debug0                 : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_DEBUG 1 */
union u_debug_1 {
	/* Define the struct bits  */
	struct {
		unsigned int debug1                 : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_SCORE_HIST */
union u_score_hist {
	/* Define the struct bits */
	struct {
		unsigned int    score_histbin         : 12  ; /* [11..0]  */
		unsigned int    reserved_0            : 20  ; /* [31..12]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_BRIEF_PATTERN */
union u_brief_pattern {
	/* Define the struct bits */
	struct {
		unsigned int    pattern_x0            : 6   ; /* [5..0]  */
		unsigned int    reserved_0            : 2   ; /* [7..6]  */
		unsigned int    pattern_y0            : 6   ; /* [13..8]  */
		unsigned int    reserved_1            : 2   ; /* [15..14]  */
		unsigned int    pattern_x1            : 6   ; /* [21..16]  */
		unsigned int    reserved_2            : 2   ; /* [23..22]  */
		unsigned int    pattern_y1            : 6   ; /* [29..24]  */
		unsigned int    reserved_3            : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SCORE_THESHOLD */
union u_score_theshold {
	/* Define the struct bits  */
	struct {
		unsigned int score_th               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_KPT_NUMBER */
union u_kpt_number {
	/* Define the struct bits  */
	struct {
		unsigned int kpt_num                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
//==============================================================================
/* Define the global struct */
struct s_slam_regs_type {
	union u_slam_cfg             slam_cfg;
	union u_image_size           image_size;
	union u_total_kpt_num        total_kpt_num;
	union u_pyramid_inc_cfg      pyramid_inc_cfg;
	union u_pyramid_vcrop_cfgb   pyramid_vcrop_cfgb;
	union u_pyramid_vcrop_cfgt   pyramid_vcrop_cfgt;
	union u_pyramid_hcrop_cfgr   pyramid_hcrop_cfgr;
	union u_pyramid_hcrop_cfgl   pyramid_hcrop_cfgl;
	union u_gsblur_coef_01       gsblur_coef_01;
	union u_gsblur_coef_23       gsblur_coef_23;
	union u_threshold_cfg        threshold_cfg;
	union u_nms_win_cfg          nms_win_cfg;
	union u_block_num_cfg        block_num_cfg;
	union u_block_size_cfg       block_size_cfg;
	union u_octree_cfg           octree_cfg;
	union u_inc_lut_cfg          inc_lut_cfg;
	union u_undistort_cx         undistort_cx;
	union u_undistort_cy         undistort_cy;
	union u_undistort_fx         undistort_fx;
	union u_undistort_fy         undistort_fy;
	union u_undistort_invfx      undistort_invfx;
	union u_undistort_invfy      undistort_invfy;
	union u_undistort_k1         undistort_k1;
	union u_undistort_k2         undistort_k2;
	union u_undistort_k3         undistort_k3;
	union u_undistort_p1         undistort_p1;
	union u_undistort_p2         undistort_p2;
	union u_cvdr_cfg_0           cvdr_cfg_0;
	union u_cvdr_cfg_1           cvdr_cfg_1;
	union u_debug_0              debug_0;
	union u_debug_1              debug_1;
	union u_score_hist           score_hist[63];
	union u_brief_pattern        brief_pattern[256];
	union u_score_theshold       score_theshold[94];
	union u_kpt_number           kpt_number[94];

};

/* Declare the struct pointor of the module SLAM */
extern struct s_slam_regs_type *gopslamallreg;


#endif /* __SLAM_DRV_PRIV_CS_H__ */
