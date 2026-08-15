//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  slam_drv_priv.h
// Author        :  HerveDANIEL
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  HerveDANIEL 2018-01-03 Create file
//******************************************************************************

#ifndef __SLAM_DRV_PRIV_H__
#define __SLAM_DRV_PRIV_H__

/* Define the union U_SLAM_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pyramid_en            : 1   ; /* [0]  */
        unsigned int    gsblur_en             : 1   ; /* [1]  */
        unsigned int    fast_en               : 1   ; /* [2]  */
        unsigned int    nms_en                : 1   ; /* [3]  */
        unsigned int    orient_en             : 1   ; /* [4]  */
        unsigned int    brief_en              : 1   ; /* [5]  */
        unsigned int    freak_en              : 1   ; /* [6]  */
        unsigned int    scoremap_en           : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_CFG;

/* Define the union U_IMAGE_SIZE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    width                 : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    height                : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_IMAGE_SIZE;

/* Define the union U_PYRAMID_INC_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    scl_inc               : 20  ; /* [19..0]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PYRAMID_INC_CFG;

/* Define the union U_PYRAMID_VCROP_CFGB */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    scl_vbottom           : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PYRAMID_VCROP_CFGB;

/* Define the union U_PYRAMID_VCROP_CFGT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    scl_vtop              : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PYRAMID_VCROP_CFGT;

/* Define the union U_PYRAMID_HCROP_CFGR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    scl_hright            : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PYRAMID_HCROP_CFGR;

/* Define the union U_PYRAMID_HCROP_CFGL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    scl_hleft             : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_PYRAMID_HCROP_CFGL;

/* Define the union U_GSBLUR_COEF_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coeff_gauss_1         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coeff_gauss_0         : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GSBLUR_COEF_01;

/* Define the union U_GSBLUR_COEF_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    coeff_gauss_3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    coeff_gauss_2         : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_GSBLUR_COEF_23;

/* Define the union U_THRESHOLD_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    min_th                : 8   ; /* [7..0]  */
        unsigned int    ini_th                : 8   ; /* [15..8]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_THRESHOLD_CFG;

/* Define the union U_NMS_WIN_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nmscell_v             : 4   ; /* [3..0]  */
        unsigned int    nmscell_h             : 5   ; /* [8..4]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_NMS_WIN_CFG;

/* Define the union U_SCORE_HIST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    score_histbin         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SCORE_HIST;

/* Define the union U_BRIEF_PATTERN */
typedef union
{
    /* Define the struct bits */
    struct
    {
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

} U_BRIEF_PATTERN;

/* Define the union U_DEBUG_INFO */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_info             : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_DEBUG_INFO;
//==============================================================================
/* Define the global struct */
typedef struct
{
    U_SLAM_CFG             SLAM_CFG                          ; /* 0x0 */
    U_IMAGE_SIZE           IMAGE_SIZE                        ; /* 0x4 */
    U_PYRAMID_INC_CFG      PYRAMID_INC_CFG                   ; /* 0x8 */
    U_PYRAMID_VCROP_CFGB   PYRAMID_VCROP_CFGB                ; /* 0xc */
    U_PYRAMID_VCROP_CFGT   PYRAMID_VCROP_CFGT                ; /* 0x10 */
    U_PYRAMID_HCROP_CFGR   PYRAMID_HCROP_CFGR                ; /* 0x14 */
    U_PYRAMID_HCROP_CFGL   PYRAMID_HCROP_CFGL                ; /* 0x18 */
    U_GSBLUR_COEF_01       GSBLUR_COEF_01                    ; /* 0x1c */
    U_GSBLUR_COEF_23       GSBLUR_COEF_23                    ; /* 0x20 */
    U_THRESHOLD_CFG        THRESHOLD_CFG                     ; /* 0x24 */
    U_NMS_WIN_CFG          NMS_WIN_CFG                       ; /* 0x28 */
    unsigned int           reserved_0[212]                     ; /* 0x2c~0xfc */
    U_SCORE_HIST           SCORE_HIST[252]                   ; /* 0x100~0x1f8 */
    unsigned int           reserved_1[516]                     ; /* 0x1fc~0x3fc */
    U_BRIEF_PATTERN        BRIEF_PATTERN[1024]               ; /* 0x400~0x7fc */
    unsigned int           reserved_2[256]                     ; /* 0x800~0x8fc */
    U_DEBUG_INFO           DEBUG_INFO                        ; /* 0x900 */

} S_SLAM_REGS_TYPE;

/* Declare the struct pointor of the module SLAM */
extern S_SLAM_REGS_TYPE *gopSLAMAllReg;


#endif /* __SLAM_DRV_PRIV_H__ */
