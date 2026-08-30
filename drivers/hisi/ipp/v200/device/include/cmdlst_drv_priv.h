//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  cmdlst_drv_priv.h
// Author        :  AnthonySixta
// Version       :  1.0 
// Date          :  2018-01-16
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  AnthonySixta 2018-01-16 Create file
//******************************************************************************

#ifndef __CMDLST_DRV_PRIV_H__
#define __CMDLST_DRV_PRIV_H__

/* Define the union U_CMDLST_ID */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ip_id                  : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_ID;
/* Define the union U_CMDLST_VERSION */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ip_version             : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_VERSION;
/* Define the union U_CMDLST_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    prefetch              : 1   ; /* [0]  */
        unsigned int    slowdown_nrt_channel  : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_CFG;

/* Define the union U_CMDLST_DEBUG_0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
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

} U_CMDLST_DEBUG_0;

/* Define the union U_CMDLST_DEBUG_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbg_sw_start          : 16  ; /* [15..0]  */
        unsigned int    dbg_hw_start          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_DEBUG_1;

/* Define the union U_CMDLST_DEBUG_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
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

} U_CMDLST_DEBUG_2;

/* Define the union U_CMDLST_DEBUG_3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
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

} U_CMDLST_DEBUG_3;

/* Define the union U_CMDLST_DEBUG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    last_lb_addr          : 21  ; /* [20..0]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_DEBUG_4;

/* Define the union U_CMDLST_VCD_TRACE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vcd_out_select        : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_VCD_TRACE;

/* Define the union U_CMDLST_CH_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    active_token_nbr      : 7   ; /* [6..0]  */
        unsigned int    active_token_nbr_enable : 1   ; /* [7]  */
        unsigned int    nrt_channel           : 1   ; /* [8]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_CH_CFG;

/* Define the union U_CMDLST_SW_CH_MNGR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sw_link_channel       : 4   ; /* [3..0]  */
        unsigned int    sw_link_token_nbr     : 4   ; /* [7..4]  */
        unsigned int    sw_resource           : 10  ; /* [17..8]  */
        unsigned int    reserved_0            : 13  ; /* [30..18]  */
        unsigned int    sw_priority           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_CH_MNGR;

/* Define the union U_CMDLST_SW_CVDR_RD_ADDR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_sw_0         : 2   ; /* [1..0]  */
        unsigned int    sw_cvdr_rd_address    : 19  ; /* [20..2]  */
        unsigned int    reserved_sw_1         : 3   ; /* [23..21]  */
        unsigned int    sw_cvdr_rd_size       : 2   ; /* [25..24]  */
        unsigned int    reserved_sw_2         : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_CVDR_RD_ADDR;

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format     : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion  : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du     : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page        : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_CVDR_RD_DATA_0;

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size        : 13  ; /* [12..0]  */
        unsigned int    reserved_1_0          : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking : 8   ; /* [23..16]  */
        unsigned int    reserved_1_1          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_CVDR_RD_DATA_1;

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size       : 13  ; /* [12..0]  */
        unsigned int    reserved_2_0          : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking : 8   ; /* [23..16]  */
        unsigned int    reserved_2_1          : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_CVDR_RD_DATA_2;

/* Define the union U_CMDLST_SW_CVDR_RD_DATA_3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_3_0          : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_0 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_CVDR_RD_DATA_3;

/* Define the union U_CMDLST_SW_BRANCH */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    branching             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_SW_BRANCH;

/* Define the union U_CMDLST_LAST_EXEC_RD_DATA_3 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int shadowed_rd_data_3     : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_LAST_EXEC_RD_DATA_3;
/* Define the union U_CMDLST_HW_CH_MNGR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hw_link_channel       : 4   ; /* [3..0]  */
        unsigned int    hw_link_token_nbr     : 4   ; /* [7..4]  */
        unsigned int    hw_resource           : 10  ; /* [17..8]  */
        unsigned int    reserved_0            : 13  ; /* [30..18]  */
        unsigned int    hw_priority           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_HW_CH_MNGR;

/* Define the union U_CMDLST_HW_CVDR_RD_ADDR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_hw_0         : 2   ; /* [1..0]  */
        unsigned int    hw_cvdr_rd_address    : 19  ; /* [20..2]  */
        unsigned int    reserved_hw_1         : 3   ; /* [23..21]  */
        unsigned int    hw_cvdr_rd_size       : 2   ; /* [25..24]  */
        unsigned int    reserved_hw_2         : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_HW_CVDR_RD_ADDR;

/* Define the union U_CMDLST_HW_CVDR_RD_DATA_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int hw_cvdr_rd_data_0      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_HW_CVDR_RD_DATA_0;
/* Define the union U_CMDLST_HW_CVDR_RD_DATA_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int hw_cvdr_rd_data_1      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_HW_CVDR_RD_DATA_1;
/* Define the union U_CMDLST_HW_CVDR_RD_DATA_2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int hw_cvdr_rd_data_2      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_HW_CVDR_RD_DATA_2;
/* Define the union U_CMDLST_HW_CVDR_RD_DATA_3 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int hw_cvdr_rd_data_3      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CMDLST_HW_CVDR_RD_DATA_3;
/* Define the union U_CMDLST_CHANNEL_DEBUG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbg_fsm_ch            : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 1   ; /* [3]  */
        unsigned int    dbg_sw_start          : 1   ; /* [4]  */
        unsigned int    reserved_1            : 3   ; /* [7..5]  */
        unsigned int    dbg_hw_start          : 1   ; /* [8]  */
        unsigned int    reserved_2            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_CHANNEL_DEBUG;

//==============================================================================
/* Define the global struct */
typedef struct
{
    U_CMDLST_ID            CMDLST_ID                         ; /* 0x0 */
    U_CMDLST_VERSION       CMDLST_VERSION                    ; /* 0x4 */
    unsigned int           reserved_0[6]                     ; /* 0x8~0x1c */
    U_CMDLST_CFG           CMDLST_CFG                        ; /* 0x20 */
    unsigned int           reserved_1[7]                     ; /* 0x24~0x3c */
    U_CMDLST_DEBUG_0       CMDLST_DEBUG_0                    ; /* 0x40 */
    U_CMDLST_DEBUG_1       CMDLST_DEBUG_1                    ; /* 0x44 */
    U_CMDLST_DEBUG_2       CMDLST_DEBUG_2                    ; /* 0x48 */
    U_CMDLST_DEBUG_3       CMDLST_DEBUG_3                    ; /* 0x4c */
    U_CMDLST_DEBUG_4       CMDLST_DEBUG_4                    ; /* 0x50 */
    U_CMDLST_VCD_TRACE     CMDLST_VCD_TRACE                  ; /* 0x54 */
    unsigned int           reserved_2[10]                     ; /* 0x58~0x7c */
    U_CMDLST_CH_CFG        CMDLST_CH_CFG[449]                ; /* 0x80~0x780 */
    U_CMDLST_SW_CH_MNGR    CMDLST_SW_CH_MNGR[449]            ; /* 0x84~0x784 */
    U_CMDLST_SW_CVDR_RD_ADDR   CMDLST_SW_CVDR_RD_ADDR[449]   ; /* 0x88~0x788 */
    U_CMDLST_SW_CVDR_RD_DATA_0   CMDLST_SW_CVDR_RD_DATA_0[449] ; /* 0x8c~0x78c */
    U_CMDLST_SW_CVDR_RD_DATA_1   CMDLST_SW_CVDR_RD_DATA_1[449] ; /* 0x90~0x790 */
    U_CMDLST_SW_CVDR_RD_DATA_2   CMDLST_SW_CVDR_RD_DATA_2[449] ; /* 0x94~0x794 */
    U_CMDLST_SW_CVDR_RD_DATA_3   CMDLST_SW_CVDR_RD_DATA_3[449] ; /* 0x98~0x798 */
    U_CMDLST_SW_BRANCH     CMDLST_SW_BRANCH[449]             ; /* 0xa0~0x7a0 */
    U_CMDLST_LAST_EXEC_RD_DATA_3   CMDLST_LAST_EXEC_RD_DATA_3[449] ; /* 0xa4~0x7a4 */
    U_CMDLST_HW_CH_MNGR    CMDLST_HW_CH_MNGR[449]            ; /* 0xbc~0x7bc */
    U_CMDLST_HW_CVDR_RD_ADDR   CMDLST_HW_CVDR_RD_ADDR[449]   ; /* 0xc0~0x7c0 */
    U_CMDLST_HW_CVDR_RD_DATA_0   CMDLST_HW_CVDR_RD_DATA_0[449] ; /* 0xc4~0x7c4 */
    U_CMDLST_HW_CVDR_RD_DATA_1   CMDLST_HW_CVDR_RD_DATA_1[449] ; /* 0xc8~0x7c8 */
    U_CMDLST_HW_CVDR_RD_DATA_2   CMDLST_HW_CVDR_RD_DATA_2[449] ; /* 0xcc~0x7cc */
    U_CMDLST_HW_CVDR_RD_DATA_3   CMDLST_HW_CVDR_RD_DATA_3[449] ; /* 0xd0~0x7d0 */
    U_CMDLST_CHANNEL_DEBUG   CMDLST_CHANNEL_DEBUG[449]       ; /* 0xe0~0x7e0 */

} S_CMDLST_REGS_TYPE;

/* Declare the struct pointor of the module CMDLST */
extern S_CMDLST_REGS_TYPE *gopCMDLSTAllReg;


#endif /* __CMDLST_DRV_PRIV_H__ */
