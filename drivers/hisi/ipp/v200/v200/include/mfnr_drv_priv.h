//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  mfnr_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  HerveDANIEL 2018-01-03 Create file
//******************************************************************************

#ifndef __MFNR_DRV_PRIV_H__
#define __MFNR_DRV_PRIV_H__

/* Define the union U_MFNR_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    op_mode               : 2   ; /* [1..0]  */
        unsigned int    uv_unpack             : 1   ; /* [2]  */
        unsigned int    uv_pack               : 1   ; /* [3]  */
        unsigned int    ghost_mask_wr_en      : 1   ; /* [4]  */
        unsigned int    ghost_stats_crop_h_en : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MFNR_CFG;

/* Define the union U_MFNR_SIZE_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    width                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    height                : 13  ; /* [28..16]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_MFNR_SIZE_CFG;

/* Define the union U_GHOST_DETECT_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ghost_mask_th         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GHOST_DETECT_CFG;

/* Define the union U_GHOST_DETECT_US16_CROP_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    us16_crop_h_start     : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    us16_crop_h_end       : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GHOST_DETECT_US16_CROP_CFG;

/* Define the union U_GHOST_DETECT_STATS_CROP_H_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    stats_crop_h_start    : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 7   ; /* [15..9]  */
        unsigned int    stats_crop_h_end      : 9   ; /* [24..16]  */
        unsigned int    reserved_1            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GHOST_DETECT_STATS_CROP_H_CFG;

/* Define the union U_TEMPO_FUSION_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    weight                : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 3   ; /* [11..9]  */
        unsigned int    ghost_region_th       : 8   ; /* [19..12]  */
        unsigned int    subscale              : 8   ; /* [27..20]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_TEMPO_FUSION_CFG;

/* Define the union U_DEGHOST_SIGMA_STREN_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    max_grad2sigma_scale_bkpt : 8   ; /* [7..0]  */
        unsigned int    min_grad2sigma_scale_bkpt : 8   ; /* [15..8]  */
        unsigned int    judge_grad_region_th  : 8   ; /* [23..16]  */
        unsigned int    sigma                 : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DEGHOST_SIGMA_STREN_CFG;

/* Define the union U_DEGHOST_BLEND_STREN_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    max_grad2blend_scale_bkpt : 8   ; /* [7..0]  */
        unsigned int    min_grad2blend_scale_bkpt : 8   ; /* [15..8]  */
        unsigned int    sup_mean2blend_th     : 5   ; /* [20..16]  */
        unsigned int    reserved_0            : 3   ; /* [23..21]  */
        unsigned int    sup_blend_scale       : 5   ; /* [28..24]  */
        unsigned int    reserved_1            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DEGHOST_BLEND_STREN_CFG;

/* Define the union U_DEGHOST_MEAN2SIGMA_LUT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mean2sigma            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DEGHOST_MEAN2SIGMA_LUT;

/* Define the union U_DEGHOST_GRAD2SIGMA_LUT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    grad2sigma            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DEGHOST_GRAD2SIGMA_LUT;

/* Define the union U_DEGHOST_MEAN2BLEND_LUT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mean2blend            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DEGHOST_MEAN2BLEND_LUT;

/* Define the union U_DEGHOST_GRAD2BLEND_LUT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    grad2blend            : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DEGHOST_GRAD2BLEND_LUT;

/* Define the union U_GHOST_DETECT_SUM_DIFF_U_STATS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sum_diff_u            : 31  ; /* [30..0]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GHOST_DETECT_SUM_DIFF_U_STATS;

/* Define the union U_GHOST_DETECT_SUM_DIFF_V_STATS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sum_diff_v            : 31  ; /* [30..0]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GHOST_DETECT_SUM_DIFF_V_STATS;

/* Define the union U_GHOST_DETECT_GHOST_SIZE_STATS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ghost_size            : 23  ; /* [22..0]  */
        unsigned int    reserved_0            : 9   ; /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GHOST_DETECT_GHOST_SIZE_STATS;

/* Define the union U_GHOST_DETECT_TOP_DEBUG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ghost_detect_top_debug : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GHOST_DETECT_TOP_DEBUG;
/* Define the union U_GHOST_DETECT_GAUSS7x7_DEBUG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ghost_detect_gauss7x7_debug : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GHOST_DETECT_GAUSS7x7_DEBUG;
/* Define the union U_GHOST_DETECT_DILATION_DEBUG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ghost_detect_dilation_debug : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GHOST_DETECT_DILATION_DEBUG;
/* Define the union U_GHOST_DETECT_GAUSS3x3_DEBUG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ghost_detect_gauss3x3_debug : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GHOST_DETECT_GAUSS3x3_DEBUG;
/* Define the union U_GHOST_DETECT_US16_DEBUG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int ghost_detect_us16_debug : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_GHOST_DETECT_US16_DEBUG;
/* Define the union U_DEGHOST_TOP_DEBUG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int deghost_top_debug      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DEGHOST_TOP_DEBUG;
//==============================================================================
/* Define the global struct */
typedef struct
{
    U_MFNR_CFG             MFNR_CFG                          ; /* 0x0 */
    U_MFNR_SIZE_CFG        MFNR_SIZE_CFG                     ; /* 0x4 */
    U_GHOST_DETECT_CFG     GHOST_DETECT_CFG                  ; /* 0x8 */
    U_GHOST_DETECT_US16_CROP_CFG   GHOST_DETECT_US16_CROP_CFG ; /* 0xc */
    U_GHOST_DETECT_STATS_CROP_H_CFG   GHOST_DETECT_STATS_CROP_H_CFG ; /* 0x10 */
    U_TEMPO_FUSION_CFG     TEMPO_FUSION_CFG                  ; /* 0x14 */
    U_DEGHOST_SIGMA_STREN_CFG   DEGHOST_SIGMA_STREN_CFG      ; /* 0x18 */
    U_DEGHOST_BLEND_STREN_CFG   DEGHOST_BLEND_STREN_CFG      ; /* 0x1c */
    U_DEGHOST_MEAN2SIGMA_LUT   DEGHOST_MEAN2SIGMA_LUT[104]   ; /* 0x20~0x84 */
    U_DEGHOST_GRAD2SIGMA_LUT   DEGHOST_GRAD2SIGMA_LUT[260]   ; /* 0x88~0x188 */
    U_DEGHOST_MEAN2BLEND_LUT   DEGHOST_MEAN2BLEND_LUT[104]   ; /* 0x18c~0x1f0 */
    U_DEGHOST_GRAD2BLEND_LUT   DEGHOST_GRAD2BLEND_LUT[128]   ; /* 0x1f4~0x270 */
    U_GHOST_DETECT_SUM_DIFF_U_STATS   GHOST_DETECT_SUM_DIFF_U_STATS ; /* 0x274 */
    U_GHOST_DETECT_SUM_DIFF_V_STATS   GHOST_DETECT_SUM_DIFF_V_STATS ; /* 0x278 */
    U_GHOST_DETECT_GHOST_SIZE_STATS   GHOST_DETECT_GHOST_SIZE_STATS ; /* 0x27c */
    U_GHOST_DETECT_TOP_DEBUG   GHOST_DETECT_TOP_DEBUG        ; /* 0x280 */
    U_GHOST_DETECT_GAUSS7x7_DEBUG   GHOST_DETECT_GAUSS7x7_DEBUG ; /* 0x284 */
    U_GHOST_DETECT_DILATION_DEBUG   GHOST_DETECT_DILATION_DEBUG ; /* 0x288 */
    U_GHOST_DETECT_GAUSS3x3_DEBUG   GHOST_DETECT_GAUSS3x3_DEBUG ; /* 0x28c */
    U_GHOST_DETECT_US16_DEBUG   GHOST_DETECT_US16_DEBUG      ; /* 0x290 */
    U_DEGHOST_TOP_DEBUG    DEGHOST_TOP_DEBUG                 ; /* 0x294 */

} S_MFNR_REGS_TYPE;

/* Declare the struct pointor of the module MFNR */
extern S_MFNR_REGS_TYPE *gopMFNRAllReg;


#endif /* __MFNR_DRV_PRIV_H__ */
