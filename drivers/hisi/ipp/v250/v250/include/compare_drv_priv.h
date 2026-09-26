//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  compare_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-08-09
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-08-09 Create file
//******************************************************************************

#ifndef __COMPARE_DRV_PRIV_CS_H__
#define __COMPARE_DRV_PRIV_CS_H__

/* Define the union U_COMPARE_CFG */
union u_compare_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    compare_en            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 3   ; /* [3..1]  */
		unsigned int    descriptor_type       : 1   ; /* [4]  */
		unsigned int    reserved_1            : 3   ; /* [7..5]  */
		unsigned int    matched_kpts          : 11  ; /* [18..8]  */
		unsigned int    reserved_2            : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_BLOCK_CFG */
union u_compare_block_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    blk_v_num             : 5   ; /* [4..0]  */
		unsigned int    reserved_0            : 3   ; /* [7..5]  */
		unsigned int    blk_h_num             : 5   ; /* [12..8]  */
		unsigned int    reserved_1            : 3   ; /* [15..13]  */
		unsigned int    blk_num               : 10  ; /* [25..16]  */
		unsigned int    reserved_2            : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SEARCH_CFG */
union u_compare_search_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    v_radius              : 2   ; /* [1..0]  */
		unsigned int    reserved_0            : 2   ; /* [3..2]  */
		unsigned int    h_radius              : 2   ; /* [5..4]  */
		unsigned int    reserved_1            : 2   ; /* [7..6]  */
		unsigned int    dis_ratio             : 7   ; /* [14..8]  */
		unsigned int    reserved_2            : 1   ; /* [15]  */
		unsigned int    dis_threshold         : 8   ; /* [23..16]  */
		unsigned int    reserved_3            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_STAT_CFG */
union u_compare_stat_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    bin_num               : 6   ; /* [5..0]  */
		unsigned int    reserved_0            : 2   ; /* [7..6]  */
		unsigned int    bin_factor            : 8   ; /* [15..8]  */
		unsigned int    max3_ratio            : 5   ; /* [20..16]  */
		unsigned int    reserved_1            : 3   ; /* [23..21]  */
		unsigned int    stat_en               : 1   ; /* [24]  */
		unsigned int    reserved_2            : 7   ; /* [31..25]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_PREFETCH_CFG */
union u_compare_prefetch_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    first_32k_page        : 19  ; /* [18..0]  */
		unsigned int    reserved_0            : 1   ; /* [19]  */
		unsigned int    prefetch_enable       : 1   ; /* [20]  */
		unsigned int    reserved_1            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_DEBUG_0 */
union u_compare_debug_0 {
	/* Define the struct bits  */
	struct {
		unsigned int debug0                 : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_DEBUG_1 */
union u_compare_debug_1 {
	/* Define the struct bits  */
	struct {
		unsigned int debug1                 : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_EC_0 */
union u_compare_ec_0 {
	/* Define the struct bits  */
	struct {
		unsigned int ec_0                   : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_EC_1 */
union u_compare_ec_1 {
	/* Define the struct bits  */
	struct {
		unsigned int ec_1                   : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_REF_KPT_NUMBER */
union u_compare_ref_kpt_number {
	/* Define the struct bits  */
	struct {
		unsigned int ref_kpt_num            : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CUR_KPT_NUMBER */
union u_compare_cur_kpt_number {
	/* Define the struct bits  */
	struct {
		unsigned int cur_kpt_num            : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_INDEX */
union u_compare_index {
	/* Define the struct bits */
	struct {
		unsigned int    cur_index             : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 1   ; /* [11]  */
		unsigned int    ref_index             : 11  ; /* [22..12]  */
		unsigned int    reserved_1            : 1   ; /* [23]  */
		unsigned int    best_dist             : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

//==============================================================================
/* Define the global struct */
struct s_compare_regs_type {
	union u_compare_cfg            compare_cfg;
	union u_compare_block_cfg      block_cfg;
	union u_compare_search_cfg     search_cfg;
	union u_compare_stat_cfg       stat_cfg;
	union u_compare_prefetch_cfg   prefetch_cfg;
	union u_compare_debug_0        debug_0;
	union u_compare_debug_1        debug_1;
	union u_compare_ec_0           ec_0;
	union u_compare_ec_1           ec_1;
	union u_compare_ref_kpt_number ref_kpt_number[94];
	union u_compare_cur_kpt_number cur_kpt_number[94];
	union u_compare_index          index[1600];
};

/* Declare the struct pointor of the module COMPARE */
extern struct s_compare_regs_type *gopcompareallreg;


#endif /* __COMPARE_DRV_PRIV_CS_H__ */
