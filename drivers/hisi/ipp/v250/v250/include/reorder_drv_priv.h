//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  reorder_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-08-09
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-08-09 Create file
//******************************************************************************

#ifndef __REORDER_DRV_PRIV_CS_H__
#define __REORDER_DRV_PRIV_CS_H__

/* Define the union U_REORDER_CFG */
union u_reorder_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    reorder_en            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 3   ; /* [3..1]  */
		unsigned int    descriptor_type       : 1   ; /* [4]  */
		unsigned int    reserved_1            : 3   ; /* [7..5]  */
		unsigned int    total_kpts            : 11  ; /* [18..8]  */
		unsigned int    reserved_2            : 13  ; /* [31..19]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_REORDER_BLOCK_CFG */
union u_reorder_block_cfg {
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

/* Define the union U_REORDER_PREFETCH_CFG */
union u_reorder_prefetch_cfg {
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

/* Define the union U_REORDER_DEBUG_0 */
union u_reorder_debug_0 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_0                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_REORDER_DEBUG_1 */
union u_reorder_debug_1 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_1                : 32  ; /* [31..0]  */
	} bits;
	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_REORDER_EC_0 */
union u_reorder_ec_0 {
	/* Define the struct bits  */
	struct {
		unsigned int ec_0                   : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_REORDER_EC_1 */
union u_reorder_ec_1 {
	/* Define the struct bits  */
	struct {
		unsigned int ec_1                   : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_REORDER_KPT_NUMBER */
union u_reorder_kpt_number {
	/* Define the struct bits  */
	struct {
		unsigned int kpt_num                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
//==============================================================================
/* Define the global struct */

struct s_reorder_regs_type {
	union u_reorder_cfg          reorder_cfg;
	union u_reorder_block_cfg    block_cfg;
	union u_reorder_prefetch_cfg prefetch_cfg;
	union u_reorder_debug_0      debug_0;
	union u_reorder_debug_1      debug_1;
	union u_reorder_ec_0         ec_0;
	union u_reorder_ec_1         ec_1;
	union u_reorder_kpt_number   kpt_number[94];

};

/* Declare the struct pointor of the module REORDER */
extern struct s_reorder_regs_type *gopreorderallreg;


#endif /* __REORDER_DRV_PRIV_CS_H__ */
