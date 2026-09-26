//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  cmdlst_drv_priv.h
// Author        :  AnthonySixta
// Version       :  1.0
// Date          :  2018-06-30
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  AnthonySixta 2018-06-30 Create file
//******************************************************************************

#ifndef __CMDLST_DRV_PRIV_CS_H__
#define __CMDLST_DRV_PRIV_CS_H__

/* Define the union U_CMDLST_ID */
union u_cmdlst_id {
	/* Define the struct bits  */
	struct {
		unsigned int ip_id                  : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_VERSION */
union u_cmdlst_version {
	/* Define the struct bits  */
	struct {
		unsigned int ip_version             : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_CFG */
union u_cmdlst_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    prefetch              : 1   ; /* [0]  */
		unsigned int    slowdown_nrt_channel  : 2   ; /* [2..1]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_DEBUG_0 */
union u_cmdlst_debug_0 {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_fifo_nb_elem      : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    dbg_lb_master_fsm     : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    dbg_vp_wr_fsm         : 2   ; /* [9..8]  */
		unsigned int    reserved_2            : 2   ; /* [11..10]  */
		unsigned int    dbg_arb_fsm           : 1   ; /* [12]  */
		unsigned int    reserved_3            : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_DEBUG_1 */
union u_cmdlst_debug_1 {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_sw_start          : 16  ; /* [15..0]  */
		unsigned int    dbg_hw_start          : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_DEBUG_2 */
union u_cmdlst_debug_2 {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_fsm_ch_0          : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    dbg_fsm_ch_1          : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    dbg_fsm_ch_2          : 3   ; /* [10..8]  */
		unsigned int    reserved_2            : 1   ; /* [11]  */
		unsigned int    dbg_fsm_ch_3          : 3   ; /* [14..12]  */
		unsigned int    reserved_3            : 1   ; /* [15]  */
		unsigned int    dbg_fsm_ch_4          : 3   ; /* [18..16]  */
		unsigned int    reserved_4            : 1   ; /* [19]  */
		unsigned int    dbg_fsm_ch_5          : 3   ; /* [22..20]  */
		unsigned int    reserved_5            : 1   ; /* [23]  */
		unsigned int    dbg_fsm_ch_6          : 3   ; /* [26..24]  */
		unsigned int    reserved_6            : 1   ; /* [27]  */
		unsigned int    dbg_fsm_ch_7          : 3   ; /* [30..28]  */
		unsigned int    reserved_7            : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_DEBUG_3 */
union u_cmdlst_debug_3 {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_fsm_ch_8          : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    dbg_fsm_ch_9          : 3   ; /* [6..4]  */
		unsigned int    reserved_1            : 1   ; /* [7]  */
		unsigned int    dbg_fsm_ch_10         : 3   ; /* [10..8]  */
		unsigned int    reserved_2            : 1   ; /* [11]  */
		unsigned int    dbg_fsm_ch_11         : 3   ; /* [14..12]  */
		unsigned int    reserved_3            : 1   ; /* [15]  */
		unsigned int    dbg_fsm_ch_12         : 3   ; /* [18..16]  */
		unsigned int    reserved_4            : 1   ; /* [19]  */
		unsigned int    dbg_fsm_ch_13         : 3   ; /* [22..20]  */
		unsigned int    reserved_5            : 1   ; /* [23]  */
		unsigned int    dbg_fsm_ch_14         : 3   ; /* [26..24]  */
		unsigned int    reserved_6            : 1   ; /* [27]  */
		unsigned int    dbg_fsm_ch_15         : 3   ; /* [30..28]  */
		unsigned int    reserved_7            : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_DEBUG_4 */
union u_cmdlst_debug_4 {
	/* Define the struct bits */
	struct {
		unsigned int    last_lb_addr          : 21  ; /* [20..0]  */
		unsigned int    reserved_0            : 11  ; /* [31..21]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_VCD_TRACE */
union u_cmdlst_vcd_trace {
	/* Define the struct bits */
	struct {
		unsigned int    vcd_out_select        : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 29  ; /* [31..3]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_CH_CFG */
union u_cmdlst_ch_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    active_token_nbr      : 7   ; /* [6..0]  */
		unsigned int    active_token_nbr_enable : 1   ; /* [7]  */
		unsigned int    nrt_channel           : 1   ; /* [8]  */
		unsigned int    reserved_0            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_CH_MNGR */
union u_cmdlst_sw_ch_mngr {
	/* Define the struct bits */
	struct {
		unsigned int    sw_link_channel       : 4   ; /* [3..0]  */
		unsigned int    sw_link_token_nbr     : 4   ; /* [7..4]  */
		unsigned int    sw_resource           : 23  ; /* [30..8]  */
		unsigned int    sw_priority           : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_CVDR_RD_ADDR */
union u_cmdlst_sw_cvdr_rd_addr {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_sw_0         : 2   ; /* [1..0]  */
		unsigned int    sw_cvdr_rd_address    : 19  ; /* [20..2]  */
		unsigned int    reserved_sw_1         : 3   ; /* [23..21]  */
		unsigned int    sw_cvdr_rd_size       : 2   ; /* [25..24]  */
		unsigned int    reserved_sw_2         : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_0 */
union u_cmdlst_sw_cvdr_rd_data_0 {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_pixel_format     : 4   ; /* [3..0]  */
		unsigned int    vprd_pixel_expansion  : 1   ; /* [4]  */
		unsigned int    vprd_allocated_du     : 5   ; /* [9..5]  */
		unsigned int    reserved_0            : 3   ; /* [12..10]  */
		unsigned int    vprd_last_page        : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_1 */
union  u_cmdlst_sw_cvdr_rd_data_1{
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_size        : 13  ; /* [12..0]  */
		unsigned int    reserved_1_0          : 3   ; /* [15..13]  */
		unsigned int    vprd_horizontal_blanking : 8   ; /* [23..16]  */
		unsigned int    reserved_1_1          : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_2 */
union u_cmdlst_sw_cvdr_rd_data_2 {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_frame_size       : 13  ; /* [12..0]  */
		unsigned int    reserved_2_0          : 3   ; /* [15..13]  */
		unsigned int    vprd_vertical_blanking : 8   ; /* [23..16]  */
		unsigned int    reserved_2_1          : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_3 */
union u_cmdlst_sw_cvdr_rd_data_3 {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_3_0          : 2   ; /* [1..0]  */
		unsigned int    vprd_axi_frame_start_0 : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_SW_BRANCH */
union u_cmdlst_sw_branch {
	/* Define the struct bits */
	struct {
		unsigned int    branching             : 1   ; /* [0]  */
		unsigned int    reserved_0            : 31  ; /* [31..1]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_LAST_EXEC_RD_DATA_3 */
union u_cmdlst_last_exec_rd_data_3 {
	/* Define the struct bits  */
	struct {
		unsigned int shadowed_rd_data_3     : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_HW_CH_MNGR */
union u_cmdlst_hw_ch_mngr {
	/* Define the struct bits */
	struct {
		unsigned int    hw_link_channel       : 4   ; /* [3..0]  */
		unsigned int    hw_link_token_nbr     : 4   ; /* [7..4]  */
		unsigned int    hw_resource           : 23  ; /* [30..8]  */
		unsigned int    hw_priority           : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_HW_CVDR_RD_ADDR */
union u_cmdlst_hw_cvdr_rd_addr {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_hw_0         : 2   ; /* [1..0]  */
		unsigned int    hw_cvdr_rd_address    : 19  ; /* [20..2]  */
		unsigned int    reserved_hw_1         : 3   ; /* [23..21]  */
		unsigned int    hw_cvdr_rd_size       : 2   ; /* [25..24]  */
		unsigned int    reserved_hw_2         : 6   ; /* [31..26]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_CMDLST_HW_CVDR_RD_DATA_0 */
union u_cmdlst_hw_cvdr_rd_data_0 {
	/* Define the struct bits  */
	struct {
		unsigned int hw_cvdr_rd_data_0      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_HW_CVDR_RD_DATA_1 */
union  u_cmdlst_hw_cvdr_rd_data_1{
	/* Define the struct bits  */
	struct {
		unsigned int hw_cvdr_rd_data_1      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_HW_CVDR_RD_DATA_2 */
union u_cmdlst_hw_cvdr_rd_data_2 {
	/* Define the struct bits  */
	struct {
		unsigned int hw_cvdr_rd_data_2      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_HW_CVDR_RD_DATA_3 */
union u_cmdlst_hw_cvdr_rd_data_3 {
	/* Define the struct bits  */
	struct {
		unsigned int hw_cvdr_rd_data_3      : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_CMDLST_CHANNEL_DEBUG */
union u_cmdlst_channel_debug{
	/* Define the struct bits */
	struct {
		unsigned int    dbg_fsm_ch            : 3   ; /* [2..0]  */
		unsigned int    reserved_0            : 1   ; /* [3]  */
		unsigned int    dbg_sw_start          : 1   ; /* [4]  */
		unsigned int    reserved_1            : 3   ; /* [7..5]  */
		unsigned int    dbg_hw_start          : 1   ; /* [8]  */
		unsigned int    reserved_2            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

//==============================================================================
/* Define the global struct */
struct s_cmdlst_regs_type {
	union u_cmdlst_id            cmdlst_id;
	union u_cmdlst_version       cmdlst_version;
	union u_cmdlst_cfg           cmdlst_cfg;
	union u_cmdlst_debug_0       cmdlst_debug_0;
	union u_cmdlst_debug_1       cmdlst_debug_1;
	union u_cmdlst_debug_2       cmdlst_debug_2;
	union u_cmdlst_debug_3       cmdlst_debug_3;
	union u_cmdlst_debug_4       cmdlst_debug_4;
	union u_cmdlst_vcd_trace     cmdlst_vcd_trace;
	union u_cmdlst_ch_cfg        cmdlst_ch_cfg[15];
	union u_cmdlst_sw_ch_mngr    cmdlst_sw_ch_mngr[15];
	union u_cmdlst_sw_cvdr_rd_addr cmdlst_sw_cvdr_rd_addr[15];
	union u_cmdlst_sw_cvdr_rd_data_0 cmdlst_sw_cvdr_rd_data_0[15];
	union u_cmdlst_sw_cvdr_rd_data_1 cmdlst_sw_cvdr_rd_data_1[15];
	union u_cmdlst_sw_cvdr_rd_data_2 cmdlst_sw_cvdr_rd_data_2[15];
	union u_cmdlst_sw_cvdr_rd_data_3 cmdlst_sw_cvdr_rd_data_3[15];
	union u_cmdlst_sw_branch     cmdlst_sw_branch[15];
	union u_cmdlst_last_exec_rd_data_3 cmdlst_last_exec_rd_data_3[15];
	union u_cmdlst_hw_ch_mngr    cmdlst_hw_ch_mngr[15];
	union u_cmdlst_hw_cvdr_rd_addr cmdlst_hw_cvdr_rd_addr[15];
	union u_cmdlst_hw_cvdr_rd_data_0 cmdlst_hw_cvdr_rd_data_0[15];
	union u_cmdlst_hw_cvdr_rd_data_1 cmdlst_hw_cvdr_rd_data_1[15];
	union u_cmdlst_hw_cvdr_rd_data_2 cmdlst_hw_cvdr_rd_data_2[15];
	union u_cmdlst_hw_cvdr_rd_data_3 cmdlst_hw_cvdr_rd_data_3[15];
	union u_cmdlst_channel_debug cmdlst_channel_debug[15];

};

/* Declare the struct pointor of the module CMDLST */
extern struct s_cmdlst_regs_type *gopcmdlstallreg;


#endif /* __CMDLST_DRV_PRIV_CS_H__ */
