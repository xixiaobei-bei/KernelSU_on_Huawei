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
    /* Define the struct bits  */
    struct
    {
        unsigned int jpgenc_mem_ctrl        : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPGENC_MEM_CFG;
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

/* Define the union U_JPGDEC_CRG_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgdec_clken          : 1   ; /* [0]  */
        unsigned int    jpgdec_force_clk_on   : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGDEC_CRG_CFG0;

/* Define the union U_JPGDEC_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgdec_soft_rst       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 15  ; /* [15..1]  */
        unsigned int    prefetch_stop_jpgdec  : 1   ; /* [16]  */
        unsigned int    reserved_1            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGDEC_CRG_CFG1;

/* Define the union U_JPGDEC_MEM_CFG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int jpgdec_mem_ctrl_sp     : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPGDEC_MEM_CFG;
/* Define the union U_JPGDEC_IRQ_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgdec_irq_clr        : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 12  ; /* [15..4]  */
        unsigned int    jpgdec_irq_force      : 4   ; /* [19..16]  */
        unsigned int    reserved_1            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGDEC_IRQ_REG0;

/* Define the union U_JPGDEC_IRQ_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgdec_irq_mask       : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 12  ; /* [15..4]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGDEC_IRQ_REG1;

/* Define the union U_JPGDEC_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    jpgdec_irq_state_mask : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 12  ; /* [15..4]  */
        unsigned int    jpgdec_irq_state_raw  : 4   ; /* [19..16]  */
        unsigned int    reserved_1            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGDEC_IRQ_REG2;

/* Define the union U_JPGDEC_MEM_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [15..0]  */
        unsigned int    jpgdec_mem_ctrl_tp    : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPGDEC_MEM_CFG2;

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
        unsigned int    reserved_0            : 11  ; /* [15..5]  */
        unsigned int    control_disable_axi_data_packing : 1   ; /* [16]  */
        unsigned int    mst_priority_fd       : 1   ; /* [17]  */
        unsigned int    mst_priority_cvdr     : 1   ; /* [18]  */
        unsigned int    reserved_1            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_TOP_AXI_CFG0;

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
        unsigned int    cvdr_irq_clr          : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
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
        unsigned int    cvdr_irq_mask         : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
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
        unsigned int    cvdr_irq_state_mask   : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CVDR_IRQ_REG2;

/* Define the union U_DMA_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_soft_rst         : 1   ; /* [0]  */
        unsigned int    smmu_soft_rst         : 1   ; /* [1]  */
        unsigned int    smmu_master_soft_rst  : 1   ; /* [2]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_DMA_CRG_CFG1;

/* Define the union U_CVDR_IRQ_REG01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_irq_force        : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CVDR_IRQ_REG01;

/* Define the union U_CVDR_IRQ_REG21 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_irq_state_raw    : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CVDR_IRQ_REG21;

/* Define the union U_JPG_OTHER_MERGE_IRQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    merged_irq_state      : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPG_OTHER_MERGE_IRQ;

/* Define the union U_JPG_OTHER_MERGE_IRQ2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 21  ; /* [20..0]  */
        unsigned int    ap_irq_state          : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_JPG_OTHER_MERGE_IRQ2;

/* Define the union U_FD_AXI_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fd_awqos              : 2   ; /* [1..0]  */
        unsigned int    fd_arqos              : 2   ; /* [3..2]  */
        unsigned int    reserved_0            : 4   ; /* [7..4]  */
        unsigned int    fd_rom_ctrl           : 8   ; /* [15..8]  */
        unsigned int    fd_mem_ctrl_tp        : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_AXI_CFG0;

/* Define the union U_FD_CRG_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fd_apbm_clken         : 1   ; /* [0]  */
        unsigned int    fd_axim_clken         : 1   ; /* [1]  */
        unsigned int    fd_func_clken         : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_CRG_CFG0;

/* Define the union U_FD_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fd_apbm_soft_rst      : 1   ; /* [0]  */
        unsigned int    fd_axim_soft_rst      : 1   ; /* [1]  */
        unsigned int    fd_func_soft_rst      : 1   ; /* [2]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_CRG_CFG1;

/* Define the union U_FD_MEM_CFG */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int fd_mem_ctrl_sp         : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_FD_MEM_CFG;
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
/* Define the union U_JPG_FLUX_CTRL2_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl0_fd_r        : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL2_0;
/* Define the union U_JPG_FLUX_CTRL2_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl1_fd_r        : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL2_1;
/* Define the union U_JPG_FLUX_CTRL3_0 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl0_fd_w        : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL3_0;
/* Define the union U_JPG_FLUX_CTRL3_1 */
typedef union
{
    /* Define the struct bits  */
    struct
    {
        unsigned int flux_ctrl1_fd_w        : 32  ; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
        unsigned int    u32;

} U_JPG_FLUX_CTRL3_1;
/* Define the union U_FD_SMMU_MASTER_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fd_prefetch_initial   : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_SMMU_MASTER_REG0;

/* Define the union U_FD_SMMU_MASTER_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fd_stream_end         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_SMMU_MASTER_REG1;

/* Define the union U_FD_SMMU_MASTER_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    fd_stream_ack         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_SMMU_MASTER_REG2;

/* Define the union U_HIFD_CRG_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hifd_apbm_clken       : 1   ; /* [0]  */
        unsigned int    hifd_axim_clken       : 1   ; /* [1]  */
        unsigned int    hifd_func_clken       : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HIFD_CRG_CFG0;

/* Define the union U_HIFD_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hifd_apbm_soft_rst    : 1   ; /* [0]  */
        unsigned int    hifd_axim_soft_rst    : 1   ; /* [1]  */
        unsigned int    hifd_func_soft_rst    : 1   ; /* [2]  */
        unsigned int    reserved_0            : 13  ; /* [15..3]  */
        unsigned int    reserved_1            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_HIFD_CRG_CFG1;

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

/* Define the union U_SLAM_COMP_IRQ_REG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_comp_eof_source_en : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_COMP_IRQ_REG;

/* Define the union U_SLAM_COMP_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    slam_comp_eof_raw_en  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_SLAM_COMP_IRQ_REG2;

/* Define the union U_CPE_CRG_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcf_clken             : 1   ; /* [0]  */
        unsigned int    mfnr_clken            : 1   ; /* [1]  */
        unsigned int    cmdlst_clken          : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_CRG_CFG0;

/* Define the union U_CPE_CRG_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcf_soft_rst          : 1   ; /* [0]  */
        unsigned int    mfnr_soft_rst         : 1   ; /* [1]  */
        unsigned int    cmdlst_soft_rst       : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_CRG_CFG1;

/* Define the union U_CPE_CRG_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcf_force_clk_on      : 1   ; /* [0]  */
        unsigned int    mfnr_force_clk_on     : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_CRG_CFG2;

/* Define the union U_CPE_MEM_CFG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 3   ; /* [2..0]  */
        unsigned int    common_mem_ctrl_sp    : 27  ; /* [29..3]  */
        unsigned int    cmdlst_ec_func_en     : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_MEM_CFG0;

/* Define the union U_CPE_MEM_CFG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcf_pwr_mem_ctrl_sp   : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_MEM_CFG1;

/* Define the union U_CPE_MEM_CFG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mfnr_pwr_mem_ctrl_sp  : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_MEM_CFG2;

/* Define the union U_CROP_VPWR_0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_vpwr0_ihleft      : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    cpe_vpwr0_ihright     : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CROP_VPWR_0;

/* Define the union U_CROP_VPWR_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_vpwr1_ihleft      : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    cpe_vpwr1_ihright     : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CROP_VPWR_1;

/* Define the union U_CROP_VPWR_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_vpwr2_ihleft      : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 5   ; /* [15..11]  */
        unsigned int    cpe_vpwr2_ihright     : 11  ; /* [26..16]  */
        unsigned int    reserved_1            : 5   ; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CROP_VPWR_2;

/* Define the union U_CPE_mode_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_op_mode           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_mode_CFG;

/* Define the union U_CPE_IRQ0_REG0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq0_clr          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ0_REG0;

/* Define the union U_CPE_IRQ0_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq0_force        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ0_REG1;

/* Define the union U_CPE_IRQ0_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq0_mask         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ0_REG2;

/* Define the union U_CPE_IRQ0_REG3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq0_raw          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ0_REG3;

/* Define the union U_CPE_IRQ0_REG4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq0_stat         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ0_REG4;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_cpe_vprd_sof     : 6   ; /* [5..0]  */
        unsigned int    cvdr_cpe_vprd_eof     : 6   ; /* [11..6]  */
        unsigned int    cvdr_cmdlst_vpwr_sof  : 1   ; /* [12]  */
        unsigned int    cvdr_cmdlst_vpwr_eof  : 1   ; /* [13]  */
        unsigned int    cvdr_cmdlst_vpwr_drop : 1   ; /* [14]  */
        unsigned int    cvdr_cmdlst_vprd_sof  : 1   ; /* [15]  */
        unsigned int    cvdr_cmdlst_vprd_eof  : 1   ; /* [16]  */
        unsigned int    cvdr_cpe_vpwr_sof     : 3   ; /* [19..17]  */
        unsigned int    cvdr_cpe_vpwr_eof     : 3   ; /* [22..20]  */
        unsigned int    cvdr_cpe_vpwr_dropped : 3   ; /* [25..23]  */
        unsigned int    cpe_crop_vpwr_eof     : 3   ; /* [28..26]  */
        unsigned int    Reserved_3            : 3   ; /* [31..29]  */
    } bits;
    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ1_REG0;

/* Define the union U_CPE_IRQ1_REG1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq1_force        : 29  ; /* [28..0]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ1_REG1;

/* Define the union U_CPE_IRQ1_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq1_mask         : 29  ; /* [28..0]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ1_REG2;

/* Define the union U_CPE_IRQ1_REG3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq1_raw          : 29  ; /* [28..0]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ1_REG3;

/* Define the union U_CPE_IRQ1_REG4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_irq1_stat         : 29  ; /* [28..0]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_IRQ1_REG4;

typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cvdr_cpe_vprd_eof     : 6   ; /* [5..0]  */
        unsigned int    cvdr_cpe_vpwr_eof     : 3   ; /* [8..6]  */
        unsigned int    cvdr_cmdlst_vpwr_eof  : 1   ; /* [9]  */
        unsigned int    cvdr_cmdlst_vprd_eof  : 1   ; /* [10]  */
        unsigned int    cpe_crop_vpwr_eof     : 3   ; /* [13..11]  */
        unsigned int    Reserved_3            : 18  ; /* [31..14]  */
    } bits;
    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_COMP_IRQ_REG;

/* Define the union U_CPE_COMP_IRQ_REG2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpe_comp_eof_raw_en   : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_CPE_COMP_IRQ_REG2;

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

/* Define the union U_FD_AXI_CFG1_S */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 6   ; /* [5..0]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    fd_axi_prot           : 3   ; /* [10..8]  */
        unsigned int    reserved_2            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_FD_AXI_CFG1_S;

//==============================================================================
/* Define the global struct */
typedef struct
{
    U_JPGENC_CRG_CFG0      JPGENC_CRG_CFG0                   ; /* 0x0 */
    U_JPGENC_CRG_CFG1      JPGENC_CRG_CFG1                   ; /* 0x4 */
    U_JPGENC_MEM_CFG       JPGENC_MEM_CFG                    ; /* 0x8 */
    U_JPG_RO_STATE         JPG_RO_STATE                      ; /* 0xc */
    unsigned int           reserved_0[4]                     ; /* 0x10~0x1c */
    U_JPGDEC_CRG_CFG0      JPGDEC_CRG_CFG0                   ; /* 0x20 */
    U_JPGDEC_CRG_CFG1      JPGDEC_CRG_CFG1                   ; /* 0x24 */
    U_JPGDEC_MEM_CFG       JPGDEC_MEM_CFG                    ; /* 0x28 */
    U_JPGDEC_IRQ_REG0      JPGDEC_IRQ_REG0                   ; /* 0x2c */
    U_JPGDEC_IRQ_REG1      JPGDEC_IRQ_REG1                   ; /* 0x30 */
    U_JPGDEC_IRQ_REG2      JPGDEC_IRQ_REG2                   ; /* 0x34 */
    U_JPGDEC_MEM_CFG2      JPGDEC_MEM_CFG2                   ; /* 0x38 */
    unsigned int           reserved_1                     ; /* 0x3c */
    U_TOP_AXI_CFG0         TOP_AXI_CFG0                      ; /* 0x40 */
    U_CVDR_MEM_CFG         CVDR_MEM_CFG                      ; /* 0x44 */
    U_CVDR_IRQ_REG0        CVDR_IRQ_REG0                     ; /* 0x48 */
    U_CVDR_IRQ_REG1        CVDR_IRQ_REG1                     ; /* 0x4c */
    U_CVDR_IRQ_REG2        CVDR_IRQ_REG2                     ; /* 0x50 */
    U_DMA_CRG_CFG1         DMA_CRG_CFG1                      ; /* 0x54 */
    U_CVDR_IRQ_REG01       CVDR_IRQ_REG01                    ; /* 0x58 */
    U_CVDR_IRQ_REG21       CVDR_IRQ_REG21                    ; /* 0x5c */
    U_JPG_OTHER_MERGE_IRQ   JPG_OTHER_MERGE_IRQ              ; /* 0x60 */
    U_JPG_OTHER_MERGE_IRQ2   JPG_OTHER_MERGE_IRQ2            ; /* 0x64 */
    unsigned int           reserved_2[2]                     ; /* 0x68~0x6c */
    U_FD_AXI_CFG0          FD_AXI_CFG0                       ; /* 0x70 */
    U_FD_CRG_CFG0          FD_CRG_CFG0                       ; /* 0x74 */
    U_FD_CRG_CFG1          FD_CRG_CFG1                       ; /* 0x78 */
    U_FD_MEM_CFG           FD_MEM_CFG                        ; /* 0x7c */
    U_JPG_FLUX_CTRL0_0     JPG_FLUX_CTRL0_0                  ; /* 0x80 */
    U_JPG_FLUX_CTRL0_1     JPG_FLUX_CTRL0_1                  ; /* 0x84 */
    U_JPG_FLUX_CTRL1_0     JPG_FLUX_CTRL1_0                  ; /* 0x88 */
    U_JPG_FLUX_CTRL1_1     JPG_FLUX_CTRL1_1                  ; /* 0x8c */
    U_JPG_FLUX_CTRL2_0     JPG_FLUX_CTRL2_0                  ; /* 0x90 */
    U_JPG_FLUX_CTRL2_1     JPG_FLUX_CTRL2_1                  ; /* 0x94 */
    U_JPG_FLUX_CTRL3_0     JPG_FLUX_CTRL3_0                  ; /* 0x98 */
    U_JPG_FLUX_CTRL3_1     JPG_FLUX_CTRL3_1                  ; /* 0x9c */
    unsigned int           reserved_3[4]                     ; /* 0xa0~0xac */
    U_FD_SMMU_MASTER_REG0   FD_SMMU_MASTER_REG0              ; /* 0xb0 */
    U_FD_SMMU_MASTER_REG1   FD_SMMU_MASTER_REG1              ; /* 0xb4 */
    U_FD_SMMU_MASTER_REG2   FD_SMMU_MASTER_REG2              ; /* 0xb8 */
    U_HIFD_CRG_CFG0        HIFD_CRG_CFG0                     ; /* 0xa0 */
    U_HIFD_CRG_CFG1        HIFD_CRG_CFG1                     ; /* 0xa4 */
    unsigned int           reserved_4[22]                     ; /* 0xa8~0xfc */
    U_JPG_DEBUG_0          JPG_DEBUG_0                       ; /* 0x100 */
    U_JPG_DEBUG_1          JPG_DEBUG_1                       ; /* 0x104 */
    U_JPG_DEBUG_2          JPG_DEBUG_2                       ; /* 0x108 */
    U_JPG_DEBUG_3          JPG_DEBUG_3                       ; /* 0x10c */
    unsigned int           reserved_5[28]                     ; /* 0x110~0x17c */
    U_SLAM_CRG_CFG0        SLAM_CRG_CFG0                     ; /* 0x180 */
    U_SLAM_CRG_CFG1        SLAM_CRG_CFG1                     ; /* 0x184 */
    U_SLAM_CRG_CFG2        SLAM_CRG_CFG2                     ; /* 0x188 */
    U_SLAM_MEM_CFG1        SLAM_MEM_CFG1                     ; /* 0x18c */
    U_SLAM_COMP_IRQ_REG    SLAM_COMP_IRQ_REG                 ; /* 0x190 */
    U_SLAM_COMP_IRQ_REG2   SLAM_COMP_IRQ_REG2                ; /* 0x194 */
    unsigned int           reserved_6[26]                     ; /* 0x198~0x1fc */
    U_CPE_CRG_CFG0         CPE_CRG_CFG0                      ; /* 0x200 */
    U_CPE_CRG_CFG1         CPE_CRG_CFG1                      ; /* 0x204 */
    U_CPE_CRG_CFG2         CPE_CRG_CFG2                      ; /* 0x208 */
    unsigned int           reserved_7                     ; /* 0x20c */
    U_CPE_MEM_CFG0         CPE_MEM_CFG0                      ; /* 0x210 */
    U_CPE_MEM_CFG1         CPE_MEM_CFG1                      ; /* 0x214 */
    U_CPE_MEM_CFG2         CPE_MEM_CFG2                      ; /* 0x218 */
    unsigned int           reserved_8                     ; /* 0x21c */
    U_CROP_VPWR_0          CROP_VPWR_0                       ; /* 0x220 */
    U_CROP_VPWR_1          CROP_VPWR_1                       ; /* 0x224 */
    U_CROP_VPWR_2          CROP_VPWR_2                       ; /* 0x228 */
    unsigned int           reserved_9[5]                     ; /* 0x22c~0x23c */
    U_CPE_mode_CFG         CPE_mode_CFG                      ; /* 0x240 */
    unsigned int           reserved_10[7]                     ; /* 0x244~0x25c */
    U_CPE_IRQ0_REG0        CPE_IRQ0_REG0                     ; /* 0x260 */
    U_CPE_IRQ0_REG1        CPE_IRQ0_REG1                     ; /* 0x264 */
    U_CPE_IRQ0_REG2        CPE_IRQ0_REG2                     ; /* 0x268 */
    U_CPE_IRQ0_REG3        CPE_IRQ0_REG3                     ; /* 0x26c */
    U_CPE_IRQ0_REG4        CPE_IRQ0_REG4                     ; /* 0x270 */
    U_CPE_IRQ1_REG0        CPE_IRQ1_REG0                     ; /* 0x274 */
    U_CPE_IRQ1_REG1        CPE_IRQ1_REG1                     ; /* 0x278 */
    U_CPE_IRQ1_REG2        CPE_IRQ1_REG2                     ; /* 0x27c */
    U_CPE_IRQ1_REG3        CPE_IRQ1_REG3                     ; /* 0x280 */
    U_CPE_IRQ1_REG4        CPE_IRQ1_REG4                     ; /* 0x284 */
    U_CPE_COMP_IRQ_REG     CPE_COMP_IRQ_REG                  ; /* 0x288 */
    U_CPE_COMP_IRQ_REG2    CPE_COMP_IRQ_REG2                 ; /* 0x28c */
    unsigned int           reserved_11[348]                     ; /* 0x290~0x7fc */
    U_JPG_SEC_CTRL_S       JPG_SEC_CTRL_S                    ; /* 0x800 */
    unsigned int           reserved_12[3]                     ; /* 0x804~0x80c */
    U_FD_AXI_CFG1_S        FD_AXI_CFG1_S                     ; /* 0x810 */

} S_SUB_CTRL_REGS_TYPE;

/* Declare the struct pointor of the module SUB_CTRL */
extern S_SUB_CTRL_REGS_TYPE *gopSUB_CTRLAllReg;


#endif /* __SUB_CTRL_DRV_PRIV_H__ */
