// ****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  sub_ctrl_drv_priv.h
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// ******************************************************************************

#ifndef __SUB_CTRL_DRV_PRIV_H__
#define __SUB_CTRL_DRV_PRIV_H__

/* Define the union U_TOP_AXI_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpg_dw_axi_gatedclock_en : 1   ; /* [0]  */
        unsigned int    jpg_smmu_force_clk_on : 1   ; /* [1]  */
        unsigned int    jpg_dma_force_clk_on  : 1   ; /* [2]  */
        unsigned int    jpg_top_apb_force_clk_on : 1   ; /* [3]  */
        unsigned int    jpg_smmu_master_clken : 1   ; /* [4]  */
        unsigned int    cmdlst_clken          : 1   ; /* [5]  */
        unsigned int    reserved_0            : 10  ; /* [15..6]  */
        unsigned int    control_disable_axi_data_packing : 1   ; /* [16]  */
        unsigned int    mst_priority_fd       : 1   ; /* [17]  */
        unsigned int    mst_priority_cvdr     : 1   ; /* [18]  */
        unsigned int    reserved_1            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_TOP_AXI_CFG0;

/* Define the union U_DMA_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_soft_rst         : 1   ; /* [0]  */
        unsigned int    smmu_soft_rst         : 1   ; /* [1]  */
        unsigned int    smmu_master_soft_rst  : 1   ; /* [2]  */
        unsigned int    cmdlst_soft_rst       : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DMA_CRG_CFG1;

/* Define the union U_CVDR_MEM_CFG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int cvdr_mem_ctrl          : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_CVDR_MEM_CFG;
/* Define the union U_CVDR_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_irq_clr          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CVDR_IRQ_REG0;

/* Define the union U_CVDR_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_irq_mask         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CVDR_IRQ_REG1;

/* Define the union U_CVDR_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_irq_state_mask   : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 8   ; /* [15..8]  */
        unsigned int    cvdr_irq_state_raw    : 8   ; /* [23..16]  */
        unsigned int    reserved_1            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CVDR_IRQ_REG2;

/* Define the union U_CMDLST_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_ctrl_chn0      : 2   ; /* [1..0]  */
        unsigned int    cmdlst_ctrl_chn1      : 2   ; /* [3..2]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_CTRL;

/* Define the union U_CMDLST_CHN0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_eof_mask_chn0  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_CHN0;

/* Define the union U_CMDLST_CHN1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_eof_mask_chn1  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_CHN1;

/* Define the union U_CMDLST_R8_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_r8_irq_clr     : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_R8_IRQ_REG0;

/* Define the union U_CMDLST_R8_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_r8_irq_mask    : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_R8_IRQ_REG1;

/* Define the union U_CMDLST_R8_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_r8_irq_state_mask : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 14  ; /* [15..2]  */
        unsigned int    cmdlst_r8_irq_state_raw : 2   ; /* [17..16]  */
        unsigned int    reserved_1            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_R8_IRQ_REG2;

/* Define the union U_CMDLST_ACPU_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_acpu_irq_clr   : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_ACPU_IRQ_REG0;

/* Define the union U_CMDLST_ACPU_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_acpu_irq_mask  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_ACPU_IRQ_REG1;

/* Define the union U_CMDLST_ACPU_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_acpu_irq_state_mask : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 14  ; /* [15..2]  */
        unsigned int    cmdlst_acpu_irq_state_raw : 2   ; /* [17..16]  */
        unsigned int    reserved_1            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_ACPU_IRQ_REG2;

/* Define the union U_CMDLST_IVP_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_ivp_irq_clr    : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_IVP_IRQ_REG0;

/* Define the union U_CMDLST_IVP_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_ivp_irq_mask   : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_IVP_IRQ_REG1;

/* Define the union U_CMDLST_IVP_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cmdlst_ivp_irq_state_mask : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 14  ; /* [15..2]  */
        unsigned int    cmdlst_ivp_irq_state_raw : 2   ; /* [17..16]  */
        unsigned int    reserved_1            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CMDLST_IVP_IRQ_REG2;

/* Define the union U_JPG_FLUX_CTRL0_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl0_cvdr_r      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL0_0;
/* Define the union U_JPG_FLUX_CTRL0_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl1_cvdr_r      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL0_1;
/* Define the union U_JPG_FLUX_CTRL1_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl0_cvdr_w      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL1_0;
/* Define the union U_JPG_FLUX_CTRL1_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl1_cvdr_w      : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL1_1;
/* Define the union U_JPG_RO_STATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    prefetch_stop_ok_jpgenc : 1   ; /* [0]  */
        unsigned int    prefetch_stop_ok_jpgdec : 1   ; /* [1]  */
        unsigned int    reserved_0            : 6   ; /* [7..2]  */
        unsigned int    jpg_smmu_idle         : 1   ; /* [8]  */
        unsigned int    jpg_cvdr_busy         : 1   ; /* [9]  */
        unsigned int    reserved_1            : 6   ; /* [15..10]  */
        unsigned int    jpg_axi_dlock_irq     : 1   ; /* [16]  */
        unsigned int    jpg_axi_dlock_wr      : 1   ; /* [17]  */
        unsigned int    jpg_axi_dlock_slv     : 1   ; /* [18]  */
        unsigned int    jpg_axi_dlock_mst     : 1   ; /* [19]  */
        unsigned int    jpg_axi_dlock_id      : 8   ; /* [27..20]  */
        unsigned int    reserved_2            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPG_RO_STATE;

/* Define the union U_JPGENC_CRG_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgenc_clken          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGENC_CRG_CFG0;

/* Define the union U_JPGENC_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgenc_soft_rst       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 15  ; /* [15..1]  */
        unsigned int    prefetch_stop_jpgenc  : 1   ; /* [16]  */
        unsigned int    reserved_1            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGENC_CRG_CFG1;

/* Define the union U_JPGENC_MEM_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgenc_mem_ctrl       : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGENC_MEM_CFG;

/* Define the union U_JPGENC_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgenc_irq_clr        : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 11  ; /* [15..5]  */
        unsigned int    jpgenc_irq_force      : 5   ; /* [20..16]  */
        unsigned int    reserved_1            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGENC_IRQ_REG0;

/* Define the union U_JPGENC_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgenc_irq_mask       : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGENC_IRQ_REG1;

/* Define the union U_JPGENC_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgenc_irq_state_mask : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 11  ; /* [15..5]  */
        unsigned int    jpgenc_irq_state_raw  : 5   ; /* [20..16]  */
        unsigned int    reserved_1            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGENC_IRQ_REG2;

/* Define the union U_SLAM_CRG_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_clken            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_CRG_CFG0;

/* Define the union U_SLAM_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_soft_rst         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_CRG_CFG1;

/* Define the union U_SLAM_CRG_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_force_clk_on     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_CRG_CFG2;

/* Define the union U_SLAM_MEM_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_pwr_mem_ctrl_sp  : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_MEM_CFG1;

/* Define the union U_SLAM_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_irq_clr          : 14  ; /* [13..0]  */
        unsigned int    reserved_0            : 2   ; /* [15..14]  */
        unsigned int    slam_irq_force        : 14  ; /* [29..16]  */
        unsigned int    reserved_1            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_IRQ_REG0;

/* Define the union U_SLAM_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_irq_mask         : 14  ; /* [13..0]  */
        unsigned int    reserved_0            : 2   ; /* [15..14]  */
        unsigned int    slam_irq_outen        : 2   ; /* [17..16]  */
        unsigned int    reserved_1            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_IRQ_REG1;

/* Define the union U_SLAM_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_irq_state_mask   : 14  ; /* [13..0]  */
        unsigned int    reserved_0            : 2   ; /* [15..14]  */
        unsigned int    slam_irq_state_raw    : 14  ; /* [29..16]  */
        unsigned int    reserved_1            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_IRQ_REG2;

/* Define the union U_JPG_DEBUG_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_info_0           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_DEBUG_0;
/* Define the union U_JPG_DEBUG_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_info_1           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_DEBUG_1;
/* Define the union U_JPG_DEBUG_2 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_info_2           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_DEBUG_2;
/* Define the union U_JPG_DEBUG_3 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int debug_info_3           : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_DEBUG_3;
/* Define the union U_JPG_SEC_CTRL_S */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    top_tz_secure_n       : 1   ; /* [0]  */
        unsigned int    jpgenc_tz_secure_n    : 1   ; /* [1]  */
        unsigned int    jpgdec_tz_secure_n    : 1   ; /* [2]  */
        unsigned int    fd_tz_secure_n        : 1   ; /* [3]  */
        unsigned int    cpe_tz_secure_n       : 1   ; /* [4]  */
        unsigned int    slam_tz_secure_n      : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPG_SEC_CTRL_S;

//==============================================================================
/* Define the global struct */
typedef struct
{
    U_TOP_AXI_CFG0                  TOP_AXI_CFG0                     ; /* 0x0 */
    U_DMA_CRG_CFG1                  DMA_CRG_CFG1                     ; /* 0x4 */
    U_CVDR_MEM_CFG                  CVDR_MEM_CFG                     ; /* 0x8 */
    unsigned int                    reserved_0                       ; /* 0xc */
    U_CVDR_IRQ_REG0                 CVDR_IRQ_REG0                    ; /* 0x10 */
    U_CVDR_IRQ_REG1                 CVDR_IRQ_REG1                    ; /* 0x14 */
    U_CVDR_IRQ_REG2                 CVDR_IRQ_REG2                    ; /* 0x18 */
    unsigned int                    reserved_1[9]                    ; /* 0x1c~0x3c */
    U_CMDLST_CTRL                   CMDLST_CTRL                      ; /* 0x40 */
    U_CMDLST_CHN0                   CMDLST_CHN0                      ; /* 0x44 */
    U_CMDLST_CHN1                   CMDLST_CHN1                      ; /* 0x48 */
    unsigned int                    reserved_2[4]                    ; /* 0x4c~0x58 */
    U_CMDLST_R8_IRQ_REG0            CMDLST_R8_IRQ_REG0               ; /* 0x5c */
    U_CMDLST_R8_IRQ_REG1            CMDLST_R8_IRQ_REG1               ; /* 0x60 */
    U_CMDLST_R8_IRQ_REG2            CMDLST_R8_IRQ_REG2               ; /* 0x64 */
    U_CMDLST_ACPU_IRQ_REG0          CMDLST_ACPU_IRQ_REG0             ; /* 0x68 */
    U_CMDLST_ACPU_IRQ_REG1          CMDLST_ACPU_IRQ_REG1             ; /* 0x6c */
    U_CMDLST_ACPU_IRQ_REG2          CMDLST_ACPU_IRQ_REG2             ; /* 0x70 */
    U_CMDLST_IVP_IRQ_REG0           CMDLST_IVP_IRQ_REG0              ; /* 0x74 */
    U_CMDLST_IVP_IRQ_REG1           CMDLST_IVP_IRQ_REG1              ; /* 0x78 */
    U_CMDLST_IVP_IRQ_REG2           CMDLST_IVP_IRQ_REG2              ; /* 0x7c */
    U_JPG_FLUX_CTRL0_0              JPG_FLUX_CTRL0_0                 ; /* 0x80 */
    U_JPG_FLUX_CTRL0_1              JPG_FLUX_CTRL0_1                 ; /* 0x84 */
    U_JPG_FLUX_CTRL1_0              JPG_FLUX_CTRL1_0                 ; /* 0x88 */
    U_JPG_FLUX_CTRL1_1              JPG_FLUX_CTRL1_1                 ; /* 0x8c */
    unsigned int                    reserved_3[12]                   ; /* 0x90~0xbc */
    U_JPG_RO_STATE                  JPG_RO_STATE                     ; /* 0xc0 */
    unsigned int                    reserved_4[15]                   ; /* 0xc4~0xfc */
    U_JPGENC_CRG_CFG0               JPGENC_CRG_CFG0                  ; /* 0x100 */
    U_JPGENC_CRG_CFG1               JPGENC_CRG_CFG1                  ; /* 0x104 */
    U_JPGENC_MEM_CFG                JPGENC_MEM_CFG                   ; /* 0x108 */
    unsigned int                    reserved_5                       ; /* 0x10c */
    U_JPGENC_IRQ_REG0               JPGENC_IRQ_REG0                  ; /* 0x110 */
    U_JPGENC_IRQ_REG1               JPGENC_IRQ_REG1                  ; /* 0x114 */
    U_JPGENC_IRQ_REG2               JPGENC_IRQ_REG2                  ; /* 0x118 */
    unsigned int                    reserved_6[89]                   ; /* 0x11c~0x27c */
    U_SLAM_CRG_CFG0                 SLAM_CRG_CFG0                    ; /* 0x280 */
    U_SLAM_CRG_CFG1                 SLAM_CRG_CFG1                    ; /* 0x284 */
    U_SLAM_CRG_CFG2                 SLAM_CRG_CFG2                    ; /* 0x288 */
    U_SLAM_MEM_CFG1                 SLAM_MEM_CFG1                    ; /* 0x28c */
    U_SLAM_IRQ_REG0                 SLAM_IRQ_REG0                    ; /* 0x290 */
    U_SLAM_IRQ_REG1                 SLAM_IRQ_REG1                    ; /* 0x294 */
    U_SLAM_IRQ_REG2                 SLAM_IRQ_REG2                    ; /* 0x298 */
    unsigned int                    reserved_7[89]                   ; /* 0x29c~0x3fc */
    U_JPG_DEBUG_0                   JPG_DEBUG_0                      ; /* 0x400 */
    U_JPG_DEBUG_1                   JPG_DEBUG_1                      ; /* 0x404 */
    U_JPG_DEBUG_2                   JPG_DEBUG_2                      ; /* 0x408 */
    U_JPG_DEBUG_3                   JPG_DEBUG_3                      ; /* 0x40c */
    unsigned int                    reserved_8[252]                  ; /* 0x410~0x7fc */
    U_JPG_SEC_CTRL_S                JPG_SEC_CTRL_S                   ; /* 0x800 */

} S_SUB_CTRL_REGS_TYPE;

/* Declare the struct pointor of the module SUB_CTRL */
extern S_SUB_CTRL_REGS_TYPE *gopSUB_CTRLAllReg;


#endif /* __SUB_CTRL_DRV_PRIV_H__ */
