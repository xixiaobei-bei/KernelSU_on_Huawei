//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  axi_jpeg_drv_priv.h
// Author        :  AnthonySixta
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  AnthonySixta 2018-01-03 Create file
//******************************************************************************

#ifndef __AXI_JPEG_DRV_PRIV_H__
#define __AXI_JPEG_DRV_PRIV_H__

/* Define the union U_AXI_JPEG_CVDR_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axiwrite_du_threshold : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    du_threshold_reached  : 8   ; /* [15..8]  */
        unsigned int    max_axiread_id        : 5   ; /* [20..16]  */
        unsigned int    reserved_1            : 3   ; /* [23..21]  */
        unsigned int    max_axiwrite_id       : 5   ; /* [28..24]  */
        unsigned int    reserved_2            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_CVDR_CFG;

/* Define the union U_AXI_JPEG_CVDR_DEBUG_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wr_peak_en            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 7   ; /* [7..1]  */
        unsigned int    rd_peak_en            : 1   ; /* [8]  */
        unsigned int    reserved_1            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_CVDR_DEBUG_EN;

/* Define the union U_AXI_JPEG_CVDR_DEBUG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wr_peak               : 8   ; /* [7..0]  */
        unsigned int    rd_peak               : 8   ; /* [15..8]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_CVDR_DEBUG;

/* Define the union U_AXI_JPEG_CVDR_WR_QOS_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wr_qos_threshold_01_stop : 4   ; /* [3..0]  */
        unsigned int    wr_qos_threshold_01_start : 4   ; /* [7..4]  */
        unsigned int    wr_qos_threshold_10_stop : 4   ; /* [11..8]  */
        unsigned int    wr_qos_threshold_10_start : 4   ; /* [15..12]  */
        unsigned int    wr_qos_threshold_11_stop : 4   ; /* [19..16]  */
        unsigned int    wr_qos_threshold_11_start : 4   ; /* [23..20]  */
        unsigned int    reserved_0            : 2   ; /* [25..24]  */
        unsigned int    wr_qos_min            : 2   ; /* [27..26]  */
        unsigned int    wr_qos_max            : 2   ; /* [29..28]  */
        unsigned int    wr_qos_sr             : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_CVDR_WR_QOS_CFG;

/* Define the union U_AXI_JPEG_CVDR_RD_QOS_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rd_qos_threshold_01_stop : 4   ; /* [3..0]  */
        unsigned int    rd_qos_threshold_01_start : 4   ; /* [7..4]  */
        unsigned int    rd_qos_threshold_10_stop : 4   ; /* [11..8]  */
        unsigned int    rd_qos_threshold_10_start : 4   ; /* [15..12]  */
        unsigned int    rd_qos_threshold_11_stop : 4   ; /* [19..16]  */
        unsigned int    rd_qos_threshold_11_start : 4   ; /* [23..20]  */
        unsigned int    reserved_0            : 2   ; /* [25..24]  */
        unsigned int    rd_qos_min            : 2   ; /* [27..26]  */
        unsigned int    rd_qos_max            : 2   ; /* [29..28]  */
        unsigned int    rd_qos_sr             : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_CVDR_RD_QOS_CFG;

/* Define the union U_AXI_JPEG_FORCE_CLK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    force_vprd_clk_on     : 1   ; /* [0]  */
        unsigned int    force_vpwr_clk_on     : 1   ; /* [1]  */
        unsigned int    force_nrrd_clk_on     : 1   ; /* [2]  */
        unsigned int    force_nrwr_clk_on     : 1   ; /* [3]  */
        unsigned int    force_axi_rd_clk_on   : 1   ; /* [4]  */
        unsigned int    force_axi_wr_clk_on   : 1   ; /* [5]  */
        unsigned int    force_du_rd_clk_on    : 1   ; /* [6]  */
        unsigned int    force_du_wr_clk_on    : 1   ; /* [7]  */
        unsigned int    force_cfg_clk_on      : 1   ; /* [8]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_FORCE_CLK;

/* Define the union U_AXI_JPEG_OTHER_RO */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int other_ro               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_OTHER_RO;
/* Define the union U_AXI_JPEG_OTHER_RW */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int other_rw               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_OTHER_RW;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format   : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap      : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok       : 1   ; /* [24]  */
        unsigned int    vp_wr_stop          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD;

/* Define the union U_AXI_JPEG_VP_WR_CFG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_4   : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_4 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_4      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_4;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_4 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_4;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_4    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_4      : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_4;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_4 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_4 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_4 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_4       : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_4          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_4 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_4;

/* Define the union U_AXI_JPEG_VP_WR_CFG_5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_5   : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_5 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_5      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_5;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_5 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_5;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_5    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_5      : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_5;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_5 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_5 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_5 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_5       : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_5          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_5 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_5;

/* Define the union U_AXI_JPEG_VP_WR_CFG_6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_6   : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_6 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_6      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_6;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_6 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_6;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_6    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_6      : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_6;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_6 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_6 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_6 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_6       : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_6          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_6 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_6;

/* Define the union U_AXI_JPEG_VP_WR_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_8   : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_8 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_8      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_8;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_8 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_8;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_8    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_8      : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_8;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_8 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_8 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_8 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_8       : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_8          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_8 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_8;

/* Define the union U_AXI_JPEG_VP_WR_CFG_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_9   : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_9 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_9      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_9;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_9 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_9;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_9    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_9      : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_9;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_9 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_9 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_9 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_9       : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_9          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_9 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_9;

/* Define the union U_AXI_JPEG_VP_WR_CFG_10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_10  : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_10 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_10     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_10;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_10 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_10;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_10   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_10     : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_10;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_10 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_10 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_10 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_10      : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_10         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_10 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_10;

/* Define the union U_AXI_JPEG_VP_WR_CFG_25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_25  : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_25 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_25     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_25;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_25 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_25;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_25   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_25     : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_25;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_25 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_25 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_25 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_25      : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_25         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_25 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_25;

/* Define the union U_AXI_JPEG_VP_WR_CFG_30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_pixel_format_30  : 4   ; /* [3..0]  */
        unsigned int    vpwr_pixel_expansion_30 : 1   ; /* [4]  */
        unsigned int    reserved_0            : 8   ; /* [12..5]  */
        unsigned int    vpwr_last_page_30     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_CFG_30;

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS_30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vpwr_address_frame_start_30 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_FS_30;

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_line_stride_30   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 5   ; /* [14..10]  */
        unsigned int    vpwr_line_wrap_30     : 14  ; /* [28..15]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_AXI_LINE_30;

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG_30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_wr_stop_enable_du_threshold_reached_30 : 1   ; /* [16]  */
        unsigned int    vp_wr_stop_enable_flux_ctrl_30 : 1   ; /* [17]  */
        unsigned int    vp_wr_stop_enable_pressure_30 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_wr_stop_ok_30      : 1   ; /* [24]  */
        unsigned int    vp_wr_stop_30         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vpwr_prefetch_bypass_30 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_WR_IF_CFG_30;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_4 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_4 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_4 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_4 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_4 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_4;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_5 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_5 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_5 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_5 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_5 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_5;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_6 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_6 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_6 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_6 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_6 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_6;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_8 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_8 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_8 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_8 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_8 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_8;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_9 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_9 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_9 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_9 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_9 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_9;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_10 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_10 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_10 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_10 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_10 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_10;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_25 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_25 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_25 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_25 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_25 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_25;

/* Define the union U_AXI_JPEG_LIMITER_VP_WR_30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vpwr_access_limiter_0_30 : 4   ; /* [3..0]  */
        unsigned int    vpwr_access_limiter_1_30 : 4   ; /* [7..4]  */
        unsigned int    vpwr_access_limiter_2_30 : 4   ; /* [11..8]  */
        unsigned int    vpwr_access_limiter_3_30 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vpwr_access_limiter_reload_30 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_WR_30;

/* Define the union U_AXI_JPEG_VP_RD_CFG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_4   : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_4 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_4   : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_4      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_4;

/* Define the union U_AXI_JPEG_VP_RD_LWG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_4      : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_4 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_4;

/* Define the union U_AXI_JPEG_VP_RD_FHG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_4     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_4 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_4;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_4 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_4;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_4    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_4      : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_4;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_4 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_4 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_4 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_4       : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_4          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_4 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_4;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_4 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_4          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_4;
/* Define the union U_AXI_JPEG_VP_RD_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_8   : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_8 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_8   : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_8      : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_8;

/* Define the union U_AXI_JPEG_VP_RD_LWG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_8      : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_8 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_8;

/* Define the union U_AXI_JPEG_VP_RD_FHG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_8     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_8 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_8;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_8 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_8;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_8    : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_8      : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_8;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_8 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_8 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_8 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_8       : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_8          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_8 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_8;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_8 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_8          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_8;
/* Define the union U_AXI_JPEG_VP_RD_CFG_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_13  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_13 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_13  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_13     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_13;

/* Define the union U_AXI_JPEG_VP_RD_LWG_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_13     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_13 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_13;

/* Define the union U_AXI_JPEG_VP_RD_FHG_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_13    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_13 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_13;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_13 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_13;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_13   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_13     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_13;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_13 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_13 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_13 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_13      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_13         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_13 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_13;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_13 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_13         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_13;
/* Define the union U_AXI_JPEG_VP_RD_CFG_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_14  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_14 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_14  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_14     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_14;

/* Define the union U_AXI_JPEG_VP_RD_LWG_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_14     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_14 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_14;

/* Define the union U_AXI_JPEG_VP_RD_FHG_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_14    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_14 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_14;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_14 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_14;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_14   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_14     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_14;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_14 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_14 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_14 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_14      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_14         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_14 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_14;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_14 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_14         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_14;
/* Define the union U_AXI_JPEG_VP_RD_CFG_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_15  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_15 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_15  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_15     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_15;

/* Define the union U_AXI_JPEG_VP_RD_LWG_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_15     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_15 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_15;

/* Define the union U_AXI_JPEG_VP_RD_FHG_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_15    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_15 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_15;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_15 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_15;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_15   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_15     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_15;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_15 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_15 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_15 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_15      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_15         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_15 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_15;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_15 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_15         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_15;
/* Define the union U_AXI_JPEG_VP_RD_CFG_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_16  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_16 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_16  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_16     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_16;

/* Define the union U_AXI_JPEG_VP_RD_LWG_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_16     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_16 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_16;

/* Define the union U_AXI_JPEG_VP_RD_FHG_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_16    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_16 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_16;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_16 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_16;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_16   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_16     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_16;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_16 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_16 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_16 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_16      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_16         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_16 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_16;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_16 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_16         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_16;
/* Define the union U_AXI_JPEG_VP_RD_CFG_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_17  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_17 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_17  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_17     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_17;

/* Define the union U_AXI_JPEG_VP_RD_LWG_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_17     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_17 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_17;

/* Define the union U_AXI_JPEG_VP_RD_FHG_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_17    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_17 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_17;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_17 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_17;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_17   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_17     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_17;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_17 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_17 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_17 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_17      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_17         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_17 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_17;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_17 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_17         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_17;
/* Define the union U_AXI_JPEG_VP_RD_CFG_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_pixel_format_18  : 4   ; /* [3..0]  */
        unsigned int    vprd_pixel_expansion_18 : 1   ; /* [4]  */
        unsigned int    vprd_allocated_du_18  : 5   ; /* [9..5]  */
        unsigned int    reserved_0            : 3   ; /* [12..10]  */
        unsigned int    vprd_last_page_18     : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_CFG_18;

/* Define the union U_AXI_JPEG_VP_RD_LWG_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_size_18     : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_horizontal_blanking_18 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_LWG_18;

/* Define the union U_AXI_JPEG_VP_RD_FHG_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_frame_size_18    : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 3   ; /* [15..13]  */
        unsigned int    vprd_vertical_blanking_18 : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_FHG_18;

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    vprd_axi_frame_start_18 : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_FS_18;

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_line_stride_18   : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    vprd_line_wrap_18     : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_AXI_LINE_18;

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    vp_rd_stop_enable_du_threshold_reached_18 : 1   ; /* [16]  */
        unsigned int    vp_rd_stop_enable_flux_ctrl_18 : 1   ; /* [17]  */
        unsigned int    vp_rd_stop_enable_pressure_18 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    vp_rd_stop_ok_18      : 1   ; /* [24]  */
        unsigned int    vp_rd_stop_18         : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    vprd_prefetch_bypass_18 : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_VP_RD_IF_CFG_18;

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_18 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_rd_debug_18         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_RD_DEBUG_18;
/* Define the union U_AXI_JPEG_LIMITER_VP_RD_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_4 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_4 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_4 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_4 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_4 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_4;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_8 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_8 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_8 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_8 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_8 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_8;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_13 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_13 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_13 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_13 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_13 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_13;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_14 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_14 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_14 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_14 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_14 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_14;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_15 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_15 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_15 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_15 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_15 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_15;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_16 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_16 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_16 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_16 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_16 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_16;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_17 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_17 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_17 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_17 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_17 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_17;

/* Define the union U_AXI_JPEG_LIMITER_VP_RD_18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vprd_access_limiter_0_18 : 4   ; /* [3..0]  */
        unsigned int    vprd_access_limiter_1_18 : 4   ; /* [7..4]  */
        unsigned int    vprd_access_limiter_2_18 : 4   ; /* [11..8]  */
        unsigned int    vprd_access_limiter_3_18 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    vprd_access_limiter_reload_18 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_VP_RD_18;

/* Define the union U_AXI_JPEG_NR_WR_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    nr_wr_stop_enable_du_threshold_reached_8 : 1   ; /* [16]  */
        unsigned int    nr_wr_stop_enable_flux_ctrl_8 : 1   ; /* [17]  */
        unsigned int    nr_wr_stop_enable_pressure_8 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    nr_wr_stop_ok_8       : 1   ; /* [24]  */
        unsigned int    nr_wr_stop_8          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    nrwr_enable_8         : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_NR_WR_CFG_8;

/* Define the union U_AXI_JPEG_NR_WR_DEBUG_8 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int nr_wr_debug_8          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_NR_WR_DEBUG_8;
/* Define the union U_AXI_JPEG_LIMITER_NR_WR_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrwr_access_limiter_0_8 : 4   ; /* [3..0]  */
        unsigned int    nrwr_access_limiter_1_8 : 4   ; /* [7..4]  */
        unsigned int    nrwr_access_limiter_2_8 : 4   ; /* [11..8]  */
        unsigned int    nrwr_access_limiter_3_8 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    nrwr_access_limiter_reload_8 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_NR_WR_8;

/* Define the union U_AXI_JPEG_NR_WR_CFG_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    nr_wr_stop_enable_du_threshold_reached_9 : 1   ; /* [16]  */
        unsigned int    nr_wr_stop_enable_flux_ctrl_9 : 1   ; /* [17]  */
        unsigned int    nr_wr_stop_enable_pressure_9 : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [23..19]  */
        unsigned int    nr_wr_stop_ok_9       : 1   ; /* [24]  */
        unsigned int    nr_wr_stop_9          : 1   ; /* [25]  */
        unsigned int    reserved_2            : 5   ; /* [30..26]  */
        unsigned int    nrwr_enable_9         : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_NR_WR_CFG_9;

/* Define the union U_AXI_JPEG_NR_WR_DEBUG_9 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int nr_wr_debug_9          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_NR_WR_DEBUG_9;
/* Define the union U_AXI_JPEG_LIMITER_NR_WR_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrwr_access_limiter_0_9 : 4   ; /* [3..0]  */
        unsigned int    nrwr_access_limiter_1_9 : 4   ; /* [7..4]  */
        unsigned int    nrwr_access_limiter_2_9 : 4   ; /* [11..8]  */
        unsigned int    nrwr_access_limiter_3_9 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    nrwr_access_limiter_reload_9 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_NR_WR_9;

/* Define the union U_AXI_JPEG_NR_RD_CFG_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 5   ; /* [4..0]  */
        unsigned int    nrrd_allocated_du_4   : 5   ; /* [9..5]  */
        unsigned int    reserved_1            : 6   ; /* [15..10]  */
        unsigned int    nr_rd_stop_enable_du_threshold_reached_4 : 1   ; /* [16]  */
        unsigned int    nr_rd_stop_enable_flux_ctrl_4 : 1   ; /* [17]  */
        unsigned int    nr_rd_stop_enable_pressure_4 : 1   ; /* [18]  */
        unsigned int    reserved_2            : 5   ; /* [23..19]  */
        unsigned int    nr_rd_stop_ok_4       : 1   ; /* [24]  */
        unsigned int    nr_rd_stop_4          : 1   ; /* [25]  */
        unsigned int    reserved_3            : 5   ; /* [30..26]  */
        unsigned int    nrrd_enable_4         : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_NR_RD_CFG_4;

/* Define the union U_AXI_JPEG_NR_RD_DEBUG_4 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int nr_rd_debug_4          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_NR_RD_DEBUG_4;
/* Define the union U_AXI_JPEG_LIMITER_NR_RD_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrrd_access_limiter_0_4 : 4   ; /* [3..0]  */
        unsigned int    nrrd_access_limiter_1_4 : 4   ; /* [7..4]  */
        unsigned int    nrrd_access_limiter_2_4 : 4   ; /* [11..8]  */
        unsigned int    nrrd_access_limiter_3_4 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    nrrd_access_limiter_reload_4 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_NR_RD_4;

/* Define the union U_AXI_JPEG_NR_RD_CFG_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 5   ; /* [4..0]  */
        unsigned int    nrrd_allocated_du_8   : 5   ; /* [9..5]  */
        unsigned int    reserved_1            : 6   ; /* [15..10]  */
        unsigned int    nr_rd_stop_enable_du_threshold_reached_8 : 1   ; /* [16]  */
        unsigned int    nr_rd_stop_enable_flux_ctrl_8 : 1   ; /* [17]  */
        unsigned int    nr_rd_stop_enable_pressure_8 : 1   ; /* [18]  */
        unsigned int    reserved_2            : 5   ; /* [23..19]  */
        unsigned int    nr_rd_stop_ok_8       : 1   ; /* [24]  */
        unsigned int    nr_rd_stop_8          : 1   ; /* [25]  */
        unsigned int    reserved_3            : 5   ; /* [30..26]  */
        unsigned int    nrrd_enable_8         : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_NR_RD_CFG_8;

/* Define the union U_AXI_JPEG_NR_RD_DEBUG_8 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int nr_rd_debug_8          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_NR_RD_DEBUG_8;
/* Define the union U_AXI_JPEG_LIMITER_NR_RD_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrrd_access_limiter_0_8 : 4   ; /* [3..0]  */
        unsigned int    nrrd_access_limiter_1_8 : 4   ; /* [7..4]  */
        unsigned int    nrrd_access_limiter_2_8 : 4   ; /* [11..8]  */
        unsigned int    nrrd_access_limiter_3_8 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    nrrd_access_limiter_reload_8 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_NR_RD_8;

/* Define the union U_AXI_JPEG_NR_RD_CFG_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 5   ; /* [4..0]  */
        unsigned int    nrrd_allocated_du_9   : 5   ; /* [9..5]  */
        unsigned int    reserved_1            : 6   ; /* [15..10]  */
        unsigned int    nr_rd_stop_enable_du_threshold_reached_9 : 1   ; /* [16]  */
        unsigned int    nr_rd_stop_enable_flux_ctrl_9 : 1   ; /* [17]  */
        unsigned int    nr_rd_stop_enable_pressure_9 : 1   ; /* [18]  */
        unsigned int    reserved_2            : 5   ; /* [23..19]  */
        unsigned int    nr_rd_stop_ok_9       : 1   ; /* [24]  */
        unsigned int    nr_rd_stop_9          : 1   ; /* [25]  */
        unsigned int    reserved_3            : 5   ; /* [30..26]  */
        unsigned int    nrrd_enable_9         : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_NR_RD_CFG_9;

/* Define the union U_AXI_JPEG_NR_RD_DEBUG_9 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int nr_rd_debug_9          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_NR_RD_DEBUG_9;
/* Define the union U_AXI_JPEG_LIMITER_NR_RD_9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrrd_access_limiter_0_9 : 4   ; /* [3..0]  */
        unsigned int    nrrd_access_limiter_1_9 : 4   ; /* [7..4]  */
        unsigned int    nrrd_access_limiter_2_9 : 4   ; /* [11..8]  */
        unsigned int    nrrd_access_limiter_3_9 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 8   ; /* [23..16]  */
        unsigned int    nrrd_access_limiter_reload_9 : 4   ; /* [27..24]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_AXI_JPEG_LIMITER_NR_RD_9;

/* Define the union U_AXI_JPEG_SPARE_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int spare_0                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_SPARE_0;
/* Define the union U_AXI_JPEG_SPARE_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int spare_1                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_SPARE_1;
/* Define the union U_AXI_JPEG_SPARE_2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int spare_2                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_SPARE_2;
/* Define the union U_AXI_JPEG_SPARE_3 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int spare_3                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_SPARE_3;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_4 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_4          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_4;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_5 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_5          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_5;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_6 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_6          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_6;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_8 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_8          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_8;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_9 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_9          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_9;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_10 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_10         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_10;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_25 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_25         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_25;
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_30 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int vp_wr_debug_30         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_VP_WR_DEBUG_30;
/* Define the union U_AXI_JPEG_DEBUG_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_0                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_0;
/* Define the union U_AXI_JPEG_DEBUG_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_1                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_1;
/* Define the union U_AXI_JPEG_DEBUG_2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_2                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_2;
/* Define the union U_AXI_JPEG_DEBUG_3 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_3                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_3;
/* Define the union U_AXI_JPEG_DEBUG_4 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_4                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_4;
/* Define the union U_AXI_JPEG_DEBUG_5 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_5                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_5;
/* Define the union U_AXI_JPEG_DEBUG_6 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_6                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_6;
/* Define the union U_AXI_JPEG_DEBUG_7 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_7                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_7;
/* Define the union U_AXI_JPEG_DEBUG_8 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_8                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_8;
/* Define the union U_AXI_JPEG_DEBUG_9 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_9                : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_9;
/* Define the union U_AXI_JPEG_DEBUG_10 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_10               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_10;
/* Define the union U_AXI_JPEG_DEBUG_11 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_11               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_11;
/* Define the union U_AXI_JPEG_DEBUG_12 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_12               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_12;
/* Define the union U_AXI_JPEG_DEBUG_13 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_13               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_13;
/* Define the union U_AXI_JPEG_DEBUG_14 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_14               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_14;
/* Define the union U_AXI_JPEG_DEBUG_15 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_15               : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_AXI_JPEG_DEBUG_15;
//==============================================================================
/* Define the global struct */
typedef struct
{
    U_AXI_JPEG_CVDR_CFG    AXI_JPEG_CVDR_CFG                 ; /* 0x0 */
    U_AXI_JPEG_CVDR_DEBUG_EN   AXI_JPEG_CVDR_DEBUG_EN        ; /* 0x4 */
    U_AXI_JPEG_CVDR_DEBUG   AXI_JPEG_CVDR_DEBUG              ; /* 0x8 */
    U_AXI_JPEG_CVDR_WR_QOS_CFG   AXI_JPEG_CVDR_WR_QOS_CFG    ; /* 0xc */
    U_AXI_JPEG_CVDR_RD_QOS_CFG   AXI_JPEG_CVDR_RD_QOS_CFG    ; /* 0x10 */
    U_AXI_JPEG_FORCE_CLK   AXI_JPEG_FORCE_CLK                ; /* 0x14 */
    unsigned int           reserved_0[2]                     ; /* 0x18~0x1c */
    U_AXI_JPEG_OTHER_RO    AXI_JPEG_OTHER_RO                 ; /* 0x20 */
    U_AXI_JPEG_OTHER_RW    AXI_JPEG_OTHER_RW                 ; /* 0x24 */
    unsigned int           reserved_1[18]                     ; /* 0x28~0x6c */
    U_AXI_JPEG_VP_WR_CFG_4   AXI_JPEG_VP_WR_CFG_4            ; /* 0x70 */
    U_AXI_JPEG_VP_WR_AXI_FS_4   AXI_JPEG_VP_WR_AXI_FS_4      ; /* 0x74 */
    U_AXI_JPEG_VP_WR_AXI_LINE_4   AXI_JPEG_VP_WR_AXI_LINE_4  ; /* 0x78 */
    U_AXI_JPEG_VP_WR_IF_CFG_4   AXI_JPEG_VP_WR_IF_CFG_4      ; /* 0x7c */
    U_AXI_JPEG_VP_WR_CFG_5   AXI_JPEG_VP_WR_CFG_5            ; /* 0x80 */
    U_AXI_JPEG_VP_WR_AXI_FS_5   AXI_JPEG_VP_WR_AXI_FS_5      ; /* 0x84 */
    U_AXI_JPEG_VP_WR_AXI_LINE_5   AXI_JPEG_VP_WR_AXI_LINE_5  ; /* 0x88 */
    U_AXI_JPEG_VP_WR_IF_CFG_5   AXI_JPEG_VP_WR_IF_CFG_5      ; /* 0x8c */
    U_AXI_JPEG_VP_WR_CFG_6   AXI_JPEG_VP_WR_CFG_6            ; /* 0x90 */
    U_AXI_JPEG_VP_WR_AXI_FS_6   AXI_JPEG_VP_WR_AXI_FS_6      ; /* 0x94 */
    U_AXI_JPEG_VP_WR_AXI_LINE_6   AXI_JPEG_VP_WR_AXI_LINE_6  ; /* 0x98 */
    U_AXI_JPEG_VP_WR_IF_CFG_6   AXI_JPEG_VP_WR_IF_CFG_6      ; /* 0x9c */
    unsigned int           reserved_2[4]                     ; /* 0xa0~0xac */
    U_AXI_JPEG_VP_WR_CFG_8   AXI_JPEG_VP_WR_CFG_8            ; /* 0xb0 */
    U_AXI_JPEG_VP_WR_AXI_FS_8   AXI_JPEG_VP_WR_AXI_FS_8      ; /* 0xb4 */
    U_AXI_JPEG_VP_WR_AXI_LINE_8   AXI_JPEG_VP_WR_AXI_LINE_8  ; /* 0xb8 */
    U_AXI_JPEG_VP_WR_IF_CFG_8   AXI_JPEG_VP_WR_IF_CFG_8      ; /* 0xbc */
    U_AXI_JPEG_VP_WR_CFG_9   AXI_JPEG_VP_WR_CFG_9            ; /* 0xc0 */
    U_AXI_JPEG_VP_WR_AXI_FS_9   AXI_JPEG_VP_WR_AXI_FS_9      ; /* 0xc4 */
    U_AXI_JPEG_VP_WR_AXI_LINE_9   AXI_JPEG_VP_WR_AXI_LINE_9  ; /* 0xc8 */
    U_AXI_JPEG_VP_WR_IF_CFG_9   AXI_JPEG_VP_WR_IF_CFG_9      ; /* 0xcc */
    U_AXI_JPEG_VP_WR_CFG_10   AXI_JPEG_VP_WR_CFG_10          ; /* 0xd0 */
    U_AXI_JPEG_VP_WR_AXI_FS_10   AXI_JPEG_VP_WR_AXI_FS_10    ; /* 0xd4 */
    U_AXI_JPEG_VP_WR_AXI_LINE_10   AXI_JPEG_VP_WR_AXI_LINE_10 ; /* 0xd8 */
    U_AXI_JPEG_VP_WR_IF_CFG_10   AXI_JPEG_VP_WR_IF_CFG_10    ; /* 0xdc */
    unsigned int           reserved_3[56]                     ; /* 0xe0~0x1bc */
    U_AXI_JPEG_VP_WR_CFG_25   AXI_JPEG_VP_WR_CFG_25          ; /* 0x1c0 */
    U_AXI_JPEG_VP_WR_AXI_FS_25   AXI_JPEG_VP_WR_AXI_FS_25    ; /* 0x1c4 */
    U_AXI_JPEG_VP_WR_AXI_LINE_25   AXI_JPEG_VP_WR_AXI_LINE_25 ; /* 0x1c8 */
    U_AXI_JPEG_VP_WR_IF_CFG_25   AXI_JPEG_VP_WR_IF_CFG_25    ; /* 0x1cc */
    unsigned int           reserved_4[16]                     ; /* 0x1d0~0x20c */
    U_AXI_JPEG_VP_WR_CFG_30   AXI_JPEG_VP_WR_CFG_30          ; /* 0x210 */
    U_AXI_JPEG_VP_WR_AXI_FS_30   AXI_JPEG_VP_WR_AXI_FS_30    ; /* 0x214 */
    U_AXI_JPEG_VP_WR_AXI_LINE_30   AXI_JPEG_VP_WR_AXI_LINE_30 ; /* 0x218 */
    U_AXI_JPEG_VP_WR_IF_CFG_30   AXI_JPEG_VP_WR_IF_CFG_30    ; /* 0x21c */
    unsigned int           reserved_5[392]                     ; /* 0x220~0x83c */
    U_AXI_JPEG_LIMITER_VP_WR_4   AXI_JPEG_LIMITER_VP_WR_4    ; /* 0x840 */
    U_AXI_JPEG_LIMITER_VP_WR_5   AXI_JPEG_LIMITER_VP_WR_5    ; /* 0x844 */
    U_AXI_JPEG_LIMITER_VP_WR_6   AXI_JPEG_LIMITER_VP_WR_6    ; /* 0x848 */
    unsigned int           reserved_6                     ; /* 0x84c */
    U_AXI_JPEG_LIMITER_VP_WR_8   AXI_JPEG_LIMITER_VP_WR_8    ; /* 0x850 */
    U_AXI_JPEG_LIMITER_VP_WR_9   AXI_JPEG_LIMITER_VP_WR_9    ; /* 0x854 */
    U_AXI_JPEG_LIMITER_VP_WR_10   AXI_JPEG_LIMITER_VP_WR_10  ; /* 0x858 */
    unsigned int           reserved_7[14]                     ; /* 0x85c~0x890 */
    U_AXI_JPEG_LIMITER_VP_WR_25   AXI_JPEG_LIMITER_VP_WR_25  ; /* 0x894 */
    unsigned int           reserved_8[4]                     ; /* 0x898~0x8a4 */
    U_AXI_JPEG_LIMITER_VP_WR_30   AXI_JPEG_LIMITER_VP_WR_30  ; /* 0x8a8 */
    unsigned int           reserved_9[129]                     ; /* 0x8ac~0xaac */
    U_AXI_JPEG_VP_RD_CFG_4   AXI_JPEG_VP_RD_CFG_4            ; /* 0xab0 */
    U_AXI_JPEG_VP_RD_LWG_4   AXI_JPEG_VP_RD_LWG_4            ; /* 0xab4 */
    U_AXI_JPEG_VP_RD_FHG_4   AXI_JPEG_VP_RD_FHG_4            ; /* 0xab8 */
    U_AXI_JPEG_VP_RD_AXI_FS_4   AXI_JPEG_VP_RD_AXI_FS_4      ; /* 0xabc */
    U_AXI_JPEG_VP_RD_AXI_LINE_4   AXI_JPEG_VP_RD_AXI_LINE_4  ; /* 0xac0 */
    U_AXI_JPEG_VP_RD_IF_CFG_4   AXI_JPEG_VP_RD_IF_CFG_4      ; /* 0xac4 */
    unsigned int           reserved_10                     ; /* 0xac8 */
    U_AXI_JPEG_VP_RD_DEBUG_4   AXI_JPEG_VP_RD_DEBUG_4        ; /* 0xacc */
    unsigned int           reserved_11[24]                     ; /* 0xad0~0xb2c */
    U_AXI_JPEG_VP_RD_CFG_8   AXI_JPEG_VP_RD_CFG_8            ; /* 0xb30 */
    U_AXI_JPEG_VP_RD_LWG_8   AXI_JPEG_VP_RD_LWG_8            ; /* 0xb34 */
    U_AXI_JPEG_VP_RD_FHG_8   AXI_JPEG_VP_RD_FHG_8            ; /* 0xb38 */
    U_AXI_JPEG_VP_RD_AXI_FS_8   AXI_JPEG_VP_RD_AXI_FS_8      ; /* 0xb3c */
    U_AXI_JPEG_VP_RD_AXI_LINE_8   AXI_JPEG_VP_RD_AXI_LINE_8  ; /* 0xb40 */
    U_AXI_JPEG_VP_RD_IF_CFG_8   AXI_JPEG_VP_RD_IF_CFG_8      ; /* 0xb44 */
    unsigned int           reserved_12                     ; /* 0xb48 */
    U_AXI_JPEG_VP_RD_DEBUG_8   AXI_JPEG_VP_RD_DEBUG_8        ; /* 0xb4c */
    unsigned int           reserved_13[32]                     ; /* 0xb50~0xbcc */
    U_AXI_JPEG_VP_RD_CFG_13   AXI_JPEG_VP_RD_CFG_13          ; /* 0xbd0 */
    U_AXI_JPEG_VP_RD_LWG_13   AXI_JPEG_VP_RD_LWG_13          ; /* 0xbd4 */
    U_AXI_JPEG_VP_RD_FHG_13   AXI_JPEG_VP_RD_FHG_13          ; /* 0xbd8 */
    U_AXI_JPEG_VP_RD_AXI_FS_13   AXI_JPEG_VP_RD_AXI_FS_13    ; /* 0xbdc */
    U_AXI_JPEG_VP_RD_AXI_LINE_13   AXI_JPEG_VP_RD_AXI_LINE_13 ; /* 0xbe0 */
    U_AXI_JPEG_VP_RD_IF_CFG_13   AXI_JPEG_VP_RD_IF_CFG_13    ; /* 0xbe4 */
    unsigned int           reserved_14                     ; /* 0xbe8 */
    U_AXI_JPEG_VP_RD_DEBUG_13   AXI_JPEG_VP_RD_DEBUG_13      ; /* 0xbec */
    U_AXI_JPEG_VP_RD_CFG_14   AXI_JPEG_VP_RD_CFG_14          ; /* 0xbf0 */
    U_AXI_JPEG_VP_RD_LWG_14   AXI_JPEG_VP_RD_LWG_14          ; /* 0xbf4 */
    U_AXI_JPEG_VP_RD_FHG_14   AXI_JPEG_VP_RD_FHG_14          ; /* 0xbf8 */
    U_AXI_JPEG_VP_RD_AXI_FS_14   AXI_JPEG_VP_RD_AXI_FS_14    ; /* 0xbfc */
    U_AXI_JPEG_VP_RD_AXI_LINE_14   AXI_JPEG_VP_RD_AXI_LINE_14 ; /* 0xc00 */
    U_AXI_JPEG_VP_RD_IF_CFG_14   AXI_JPEG_VP_RD_IF_CFG_14    ; /* 0xc04 */
    unsigned int           reserved_15                     ; /* 0xc08 */
    U_AXI_JPEG_VP_RD_DEBUG_14   AXI_JPEG_VP_RD_DEBUG_14      ; /* 0xc0c */
    U_AXI_JPEG_VP_RD_CFG_15   AXI_JPEG_VP_RD_CFG_15          ; /* 0xc10 */
    U_AXI_JPEG_VP_RD_LWG_15   AXI_JPEG_VP_RD_LWG_15          ; /* 0xc14 */
    U_AXI_JPEG_VP_RD_FHG_15   AXI_JPEG_VP_RD_FHG_15          ; /* 0xc18 */
    U_AXI_JPEG_VP_RD_AXI_FS_15   AXI_JPEG_VP_RD_AXI_FS_15    ; /* 0xc1c */
    U_AXI_JPEG_VP_RD_AXI_LINE_15   AXI_JPEG_VP_RD_AXI_LINE_15 ; /* 0xc20 */
    U_AXI_JPEG_VP_RD_IF_CFG_15   AXI_JPEG_VP_RD_IF_CFG_15    ; /* 0xc24 */
    unsigned int           reserved_16                     ; /* 0xc28 */
    U_AXI_JPEG_VP_RD_DEBUG_15   AXI_JPEG_VP_RD_DEBUG_15      ; /* 0xc2c */
    U_AXI_JPEG_VP_RD_CFG_16   AXI_JPEG_VP_RD_CFG_16          ; /* 0xc30 */
    U_AXI_JPEG_VP_RD_LWG_16   AXI_JPEG_VP_RD_LWG_16          ; /* 0xc34 */
    U_AXI_JPEG_VP_RD_FHG_16   AXI_JPEG_VP_RD_FHG_16          ; /* 0xc38 */
    U_AXI_JPEG_VP_RD_AXI_FS_16   AXI_JPEG_VP_RD_AXI_FS_16    ; /* 0xc3c */
    U_AXI_JPEG_VP_RD_AXI_LINE_16   AXI_JPEG_VP_RD_AXI_LINE_16 ; /* 0xc40 */
    U_AXI_JPEG_VP_RD_IF_CFG_16   AXI_JPEG_VP_RD_IF_CFG_16    ; /* 0xc44 */
    unsigned int           reserved_17                     ; /* 0xc48 */
    U_AXI_JPEG_VP_RD_DEBUG_16   AXI_JPEG_VP_RD_DEBUG_16      ; /* 0xc4c */
    U_AXI_JPEG_VP_RD_CFG_17   AXI_JPEG_VP_RD_CFG_17          ; /* 0xc50 */
    U_AXI_JPEG_VP_RD_LWG_17   AXI_JPEG_VP_RD_LWG_17          ; /* 0xc54 */
    U_AXI_JPEG_VP_RD_FHG_17   AXI_JPEG_VP_RD_FHG_17          ; /* 0xc58 */
    U_AXI_JPEG_VP_RD_AXI_FS_17   AXI_JPEG_VP_RD_AXI_FS_17    ; /* 0xc5c */
    U_AXI_JPEG_VP_RD_AXI_LINE_17   AXI_JPEG_VP_RD_AXI_LINE_17 ; /* 0xc60 */
    U_AXI_JPEG_VP_RD_IF_CFG_17   AXI_JPEG_VP_RD_IF_CFG_17    ; /* 0xc64 */
    unsigned int           reserved_18                     ; /* 0xc68 */
    U_AXI_JPEG_VP_RD_DEBUG_17   AXI_JPEG_VP_RD_DEBUG_17      ; /* 0xc6c */
    U_AXI_JPEG_VP_RD_CFG_18   AXI_JPEG_VP_RD_CFG_18          ; /* 0xc70 */
    U_AXI_JPEG_VP_RD_LWG_18   AXI_JPEG_VP_RD_LWG_18          ; /* 0xc74 */
    U_AXI_JPEG_VP_RD_FHG_18   AXI_JPEG_VP_RD_FHG_18          ; /* 0xc78 */
    U_AXI_JPEG_VP_RD_AXI_FS_18   AXI_JPEG_VP_RD_AXI_FS_18    ; /* 0xc7c */
    U_AXI_JPEG_VP_RD_AXI_LINE_18   AXI_JPEG_VP_RD_AXI_LINE_18 ; /* 0xc80 */
    U_AXI_JPEG_VP_RD_IF_CFG_18   AXI_JPEG_VP_RD_IF_CFG_18    ; /* 0xc84 */
    unsigned int           reserved_19                     ; /* 0xc88 */
    U_AXI_JPEG_VP_RD_DEBUG_18   AXI_JPEG_VP_RD_DEBUG_18      ; /* 0xc8c */
    unsigned int           reserved_20[364]                     ; /* 0xc90~0x123c */
    U_AXI_JPEG_LIMITER_VP_RD_4   AXI_JPEG_LIMITER_VP_RD_4    ; /* 0x1240 */
    unsigned int           reserved_21[3]                     ; /* 0x1244~0x124c */
    U_AXI_JPEG_LIMITER_VP_RD_8   AXI_JPEG_LIMITER_VP_RD_8    ; /* 0x1250 */
    unsigned int           reserved_22[4]                     ; /* 0x1254~0x1260 */
    U_AXI_JPEG_LIMITER_VP_RD_13   AXI_JPEG_LIMITER_VP_RD_13  ; /* 0x1264 */
    U_AXI_JPEG_LIMITER_VP_RD_14   AXI_JPEG_LIMITER_VP_RD_14  ; /* 0x1268 */
    U_AXI_JPEG_LIMITER_VP_RD_15   AXI_JPEG_LIMITER_VP_RD_15  ; /* 0x126c */
    U_AXI_JPEG_LIMITER_VP_RD_16   AXI_JPEG_LIMITER_VP_RD_16  ; /* 0x1270 */
    U_AXI_JPEG_LIMITER_VP_RD_17   AXI_JPEG_LIMITER_VP_RD_17  ; /* 0x1274 */
    U_AXI_JPEG_LIMITER_VP_RD_18   AXI_JPEG_LIMITER_VP_RD_18  ; /* 0x1278 */
    unsigned int           reserved_23[77]                     ; /* 0x127c~0x13ac */
    U_AXI_JPEG_NR_WR_CFG_8   AXI_JPEG_NR_WR_CFG_8            ; /* 0x13b0 */
    U_AXI_JPEG_NR_WR_DEBUG_8   AXI_JPEG_NR_WR_DEBUG_8        ; /* 0x13b4 */
    U_AXI_JPEG_LIMITER_NR_WR_8   AXI_JPEG_LIMITER_NR_WR_8    ; /* 0x13b8 */
    unsigned int           reserved_24                     ; /* 0x13bc */
    U_AXI_JPEG_NR_WR_CFG_9   AXI_JPEG_NR_WR_CFG_9            ; /* 0x13c0 */
    U_AXI_JPEG_NR_WR_DEBUG_9   AXI_JPEG_NR_WR_DEBUG_9        ; /* 0x13c4 */
    U_AXI_JPEG_LIMITER_NR_WR_9   AXI_JPEG_LIMITER_NR_WR_9    ; /* 0x13c8 */
    unsigned int           reserved_25[105]                     ; /* 0x13cc~0x156c */
    U_AXI_JPEG_NR_RD_CFG_4   AXI_JPEG_NR_RD_CFG_4            ; /* 0x1570 */
    U_AXI_JPEG_NR_RD_DEBUG_4   AXI_JPEG_NR_RD_DEBUG_4        ; /* 0x1574 */
    U_AXI_JPEG_LIMITER_NR_RD_4   AXI_JPEG_LIMITER_NR_RD_4    ; /* 0x1578 */
    unsigned int           reserved_26[13]                     ; /* 0x157c~0x15ac */
    U_AXI_JPEG_NR_RD_CFG_8   AXI_JPEG_NR_RD_CFG_8            ; /* 0x15b0 */
    U_AXI_JPEG_NR_RD_DEBUG_8   AXI_JPEG_NR_RD_DEBUG_8        ; /* 0x15b4 */
    U_AXI_JPEG_LIMITER_NR_RD_8   AXI_JPEG_LIMITER_NR_RD_8    ; /* 0x15b8 */
    unsigned int           reserved_27                     ; /* 0x15bc */
    U_AXI_JPEG_NR_RD_CFG_9   AXI_JPEG_NR_RD_CFG_9            ; /* 0x15c0 */
    U_AXI_JPEG_NR_RD_DEBUG_9   AXI_JPEG_NR_RD_DEBUG_9        ; /* 0x15c4 */
    U_AXI_JPEG_LIMITER_NR_RD_9   AXI_JPEG_LIMITER_NR_RD_9    ; /* 0x15c8 */
    unsigned int           reserved_28[473]                     ; /* 0x15cc~0x1d2c */
    U_AXI_JPEG_SPARE_0     AXI_JPEG_SPARE_0                  ; /* 0x1d30 */
    U_AXI_JPEG_SPARE_1     AXI_JPEG_SPARE_1                  ; /* 0x1d34 */
    U_AXI_JPEG_SPARE_2     AXI_JPEG_SPARE_2                  ; /* 0x1d38 */
    U_AXI_JPEG_SPARE_3     AXI_JPEG_SPARE_3                  ; /* 0x1d3c */
    unsigned int           reserved_29[4]                     ; /* 0x1d40~0x1d4c */
    U_AXI_JPEG_VP_WR_DEBUG_4   AXI_JPEG_VP_WR_DEBUG_4        ; /* 0x1d50 */
    U_AXI_JPEG_VP_WR_DEBUG_5   AXI_JPEG_VP_WR_DEBUG_5        ; /* 0x1d54 */
    U_AXI_JPEG_VP_WR_DEBUG_6   AXI_JPEG_VP_WR_DEBUG_6        ; /* 0x1d58 */
    unsigned int           reserved_30                     ; /* 0x1d5c */
    U_AXI_JPEG_VP_WR_DEBUG_8   AXI_JPEG_VP_WR_DEBUG_8        ; /* 0x1d60 */
    U_AXI_JPEG_VP_WR_DEBUG_9   AXI_JPEG_VP_WR_DEBUG_9        ; /* 0x1d64 */
    U_AXI_JPEG_VP_WR_DEBUG_10   AXI_JPEG_VP_WR_DEBUG_10      ; /* 0x1d68 */
    unsigned int           reserved_31[14]                     ; /* 0x1d6c~0x1da0 */
    U_AXI_JPEG_VP_WR_DEBUG_25   AXI_JPEG_VP_WR_DEBUG_25      ; /* 0x1da4 */
    unsigned int           reserved_32[4]                     ; /* 0x1da8~0x1db4 */
    U_AXI_JPEG_VP_WR_DEBUG_30   AXI_JPEG_VP_WR_DEBUG_30      ; /* 0x1db8 */
    unsigned int           reserved_33[97]                     ; /* 0x1dbc~0x1f3c */
    U_AXI_JPEG_DEBUG_0     AXI_JPEG_DEBUG_0                  ; /* 0x1f40 */
    U_AXI_JPEG_DEBUG_1     AXI_JPEG_DEBUG_1                  ; /* 0x1f44 */
    U_AXI_JPEG_DEBUG_2     AXI_JPEG_DEBUG_2                  ; /* 0x1f48 */
    U_AXI_JPEG_DEBUG_3     AXI_JPEG_DEBUG_3                  ; /* 0x1f4c */
    U_AXI_JPEG_DEBUG_4     AXI_JPEG_DEBUG_4                  ; /* 0x1f50 */
    U_AXI_JPEG_DEBUG_5     AXI_JPEG_DEBUG_5                  ; /* 0x1f54 */
    U_AXI_JPEG_DEBUG_6     AXI_JPEG_DEBUG_6                  ; /* 0x1f58 */
    U_AXI_JPEG_DEBUG_7     AXI_JPEG_DEBUG_7                  ; /* 0x1f5c */
    U_AXI_JPEG_DEBUG_8     AXI_JPEG_DEBUG_8                  ; /* 0x1f60 */
    U_AXI_JPEG_DEBUG_9     AXI_JPEG_DEBUG_9                  ; /* 0x1f64 */
    U_AXI_JPEG_DEBUG_10    AXI_JPEG_DEBUG_10                 ; /* 0x1f68 */
    U_AXI_JPEG_DEBUG_11    AXI_JPEG_DEBUG_11                 ; /* 0x1f6c */
    U_AXI_JPEG_DEBUG_12    AXI_JPEG_DEBUG_12                 ; /* 0x1f70 */
    U_AXI_JPEG_DEBUG_13    AXI_JPEG_DEBUG_13                 ; /* 0x1f74 */
    U_AXI_JPEG_DEBUG_14    AXI_JPEG_DEBUG_14                 ; /* 0x1f78 */
    U_AXI_JPEG_DEBUG_15    AXI_JPEG_DEBUG_15                 ; /* 0x1f7c */

} S_AXI_JPEG_REGS_TYPE;

/* Declare the struct pointor of the module AXI_JPEG */
extern S_AXI_JPEG_REGS_TYPE *gopAXI_JPEGAllReg;


#endif /* __AXI_JPEG_DRV_PRIV_H__ */
