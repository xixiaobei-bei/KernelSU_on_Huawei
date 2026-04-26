// ******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  sub_ctrl_reg_offset.h
// Version       :  1.0
// Date          :  2018-07-02
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// ******************************************************************************

#ifndef __SUB_CTRL_REG_OFFSET_CS_H__
#define __SUB_CTRL_REG_OFFSET_CS_H__


/******************************************************************************/
/*                      HiStarISP SUB_CTRL Registers' Definitions                            */
/******************************************************************************/

#define SUB_CTRL_DMA_CRG_CFG0_REG         0x0   /* configure register for top axi path */
#define SUB_CTRL_DMA_CRG_CFG1_REG         0x4   /* CRG configure register for reset */
#define SUB_CTRL_CVDR_MEM_CFG0_REG        0x8   /* SPSRAM configure register for subsys */
#define SUB_CTRL_CVDR_MEM_CFG1_REG        0xC   /* TPSRAM configure register for subsys */
#define SUB_CTRL_CVDR_IRQ_REG0_REG        0x10  /* IRQ related cfg register */
#define SUB_CTRL_CVDR_IRQ_REG1_REG        0x14  /* IRQ related cfg register */
#define SUB_CTRL_CVDR_IRQ_REG2_REG        0x18  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_CTRL_REG          0x40  /* CMDLST cfg register for channel mapping */
#define SUB_CTRL_CMDLST_CHN0_REG          0x44  /* CMDLST cfg register for channel0 */
#define SUB_CTRL_CMDLST_CHN1_REG          0x48  /* CMDLST cfg register for channel1 */
#define SUB_CTRL_CMDLST_CHN2_REG          0x4C  /* CMDLST cfg register for channel2 */
#define SUB_CTRL_CMDLST_CHN3_REG          0x50  /* CMDLST cfg register for channel3 */
#define SUB_CTRL_CMDLST_CHN4_REG          0x54  /* CMDLST cfg register for channel0 */
#define SUB_CTRL_CMDLST_CHN5_REG          0x58  /* CMDLST cfg register for channel1 */
#define SUB_CTRL_CMDLST_CHN6_REG          0x5C  /* CMDLST cfg register for channel2 */
#define SUB_CTRL_CMDLST_CHN7_REG          0x60  /* CMDLST cfg register for channel3 */
#define SUB_CTRL_CMDLST_R8_IRQ_REG0_REG   0xA0  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_R8_IRQ_REG1_REG   0xA4  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_R8_IRQ_REG2_REG   0xA8  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG 0xAC  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_ACPU_IRQ_REG1_REG 0xB0  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_ACPU_IRQ_REG2_REG 0xB4  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_IVP_IRQ_REG0_REG  0xB8  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_IVP_IRQ_REG1_REG  0xBC  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_IVP_IRQ_REG2_REG  0xC0  /* IRQ related cfg register */
#define SUB_CTRL_JPG_FLUX_CTRL0_0_REG     0x80  /* JPG   flux ctrl grp0 for CVDR RT AXI R */
#define SUB_CTRL_JPG_FLUX_CTRL0_1_REG     0x84  /* JPG   flux ctrl grp1 for CVDR RT AXI R */
#define SUB_CTRL_JPG_FLUX_CTRL1_0_REG     0x88  /* JPG   flux ctrl grp0 for CVDR RT AXI W */
#define SUB_CTRL_JPG_FLUX_CTRL1_1_REG     0x8C  /* JPG   flux ctrl grp1 for CVDR RT AXI W */
#define SUB_CTRL_JPG_FLUX_CTRL2_0_REG     0x90  /* JPG   flux ctrl grp0 for CVDR SRT AXI R */
#define SUB_CTRL_JPG_FLUX_CTRL2_1_REG     0x94  /* JPG   flux ctrl grp1 for CVDR SRT AXI R */
#define SUB_CTRL_JPG_FLUX_CTRL3_0_REG     0x98  /* JPG   flux ctrl grp0 for CVDR SRT AXI W */
#define SUB_CTRL_JPG_FLUX_CTRL3_1_REG     0x9C  /* JPG   flux ctrl grp1 for CVDR SRT AXI W */
#define SUB_CTRL_JPG_RO_STATE_REG         0xFC  /* some read only state */
#define SUB_CTRL_JPGENC_CRG_CFG0_REG      0x100 /* CRG configure register for clk */
#define SUB_CTRL_JPGENC_CRG_CFG1_REG      0x104 /* CRG configure register for reset */
#define SUB_CTRL_JPGENC_MEM_CFG_REG       0x108 /* SRAM configure register */
#define SUB_CTRL_JPGENC_PREF_STOP_REG     0x10C /* JPGENC prefetch stop control and status */
#define SUB_CTRL_JPGENC_IRQ_REG0_REG      0x110 /* IRQ related cfg register */
#define SUB_CTRL_JPGENC_IRQ_REG1_REG      0x114 /* IRQ related cfg register */
#define SUB_CTRL_JPGENC_IRQ_REG2_REG      0x118 /* IRQ related cfg register */
#define SUB_CTRL_JPGDEC_CRG_CFG0_REG      0x180 /* CRG config register for clk */
#define SUB_CTRL_JPGDEC_CRG_CFG1_REG      0x184 /* CRG configure register for reset */
#define SUB_CTRL_JPGDEC_MEM_CFG_REG       0x188 /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_JPGDEC_PREF_STOP_REG     0x18C /* JPGDEC prefetch stop control and status */
#define SUB_CTRL_JPGDEC_IRQ_REG0_REG      0x190 /* IRQ related cfg register */
#define SUB_CTRL_JPGDEC_IRQ_REG1_REG      0x194 /* IRQ related cfg register */
#define SUB_CTRL_JPGDEC_IRQ_REG2_REG      0x198 /* IRQ related cfg register */
#define SUB_CTRL_CPE_CRG_CFG0_REG         0x200 /* CRG config register for clk */
#define SUB_CTRL_CPE_CRG_CFG1_REG         0x204 /* CRG configure register for reset */
#define SUB_CTRL_CPE_MEM_CFG_REG          0x208 /* SRAM configure register for CPE MCF */
#define SUB_CTRL_CPE_IRQ_REG0_REG         0x20C /* IRQ related cfg register */
#define SUB_CTRL_CPE_IRQ_REG1_REG         0x210 /* IRQ related cfg register */
#define SUB_CTRL_CPE_IRQ_REG2_REG         0x214 /* IRQ related cfg register */
#define SUB_CTRL_CPE_IRQ_REG3_REG         0x218 /* IRQ related cfg register */
#define SUB_CTRL_CPE_IRQ_REG4_REG         0x21C /* IRQ related cfg register */
#define SUB_CTRL_CROP_VPWR_0_REG          0x220 /* Crop horizontal offsets. */
#define SUB_CTRL_CROP_VPWR_1_REG          0x224 /* Crop horizontal offsets. */
#define SUB_CTRL_CROP_VPWR_2_REG          0x228 /* Crop horizontal offsets. */
#define SUB_CTRL_CPE_MODE_CFG_REG         0x240 /* configure register for CPE work mode */
#define SUB_CTRL_SLAM_CRG_CFG0_REG        0x280 /* CRG config register for clk */
#define SUB_CTRL_SLAM_CRG_CFG1_REG        0x284 /* CRG configure register for reset */
#define SUB_CTRL_SLAM_MEM_CFG_REG         0x288 /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_SLAM_IRQ_REG0_REG        0x290 /* IRQ related cfg register */
#define SUB_CTRL_SLAM_IRQ_REG1_REG        0x294 /* IRQ related cfg register */
#define SUB_CTRL_SLAM_IRQ_REG2_REG        0x298 /* IRQ related cfg register */
#define SUB_CTRL_RDR_CRG_CFG0_REG         0x300 /* CRG config register for clk */
#define SUB_CTRL_RDR_CRG_CFG1_REG         0x304 /* CRG configure register for reset */
#define SUB_CTRL_RDR_MEM_CFG_REG          0x308 /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_RDR_PREF_STOP_REG        0x30C /* JPGENC prefetch stop control and status */
#define SUB_CTRL_RDR_IRQ_REG0_REG         0x310 /* IRQ related cfg register */
#define SUB_CTRL_RDR_IRQ_REG1_REG         0x314 /* IRQ related cfg register */
#define SUB_CTRL_RDR_IRQ_REG2_REG         0x318 /* IRQ related cfg register */
#define SUB_CTRL_CMP_CRG_CFG0_REG         0x380 /* CRG config register for clk */
#define SUB_CTRL_CMP_CRG_CFG1_REG         0x384 /* CRG configure register for reset */
#define SUB_CTRL_CMP_MEM_CFG_REG          0x388 /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_CMP_PREF_STOP_REG        0x38C /* JPGENC prefetch stop control and status */
#define SUB_CTRL_CMP_IRQ_REG0_REG         0x390 /* IRQ related cfg register */
#define SUB_CTRL_CMP_IRQ_REG1_REG         0x394 /* IRQ related cfg register */
#define SUB_CTRL_CMP_IRQ_REG2_REG         0x398 /* IRQ related cfg register */
#define SUB_CTRL_HIFD_CRG_CFG0_REG        0x400 /* HIFD CRG config register for clk */
#define SUB_CTRL_HIFD_CRG_CFG1_REG        0x404 /* HIFD CRG configure register for reset */
#define SUB_CTRL_HIFD_MEM_CFG_REG         0x408 /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_FD_SMMU_MASTER_REG0_REG  0x410 /* FD frame stream initial signal */
#define SUB_CTRL_FD_SMMU_MASTER_REG1_REG  0x414 /* IRQ related cfg register */
#define SUB_CTRL_FD_SMMU_MASTER_REG2_REG  0x418 /* IRQ related cfg register */
#define SUB_CTRL_JPG_DEBUG_0_REG          0x700 /* debug register 0 */
#define SUB_CTRL_JPG_DEBUG_1_REG          0x704 /* debug register 1 */
#define SUB_CTRL_JPG_DEBUG_2_REG          0x708 /* debug register 2 */
#define SUB_CTRL_JPG_DEBUG_3_REG          0x70C /* debug register 3 */
#define SUB_CTRL_JPG_SEC_CTRL_S_REG       0x800 /* JPG secure cfg register */

#endif // __SUB_CTRL_REG_OFFSET_CS_H__
