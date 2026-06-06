// ****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  sub_ctrl_reg_offset.h
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// ******************************************************************************

#ifndef __SUB_CTRL_REG_OFFSET_H__
#define __SUB_CTRL_REG_OFFSET_H__


/******************************************************************************/
/*                      HiStarISP SUB_CTRL Registers' Definitions                            */
/******************************************************************************/

#define SUB_CTRL_TOP_AXI_CFG0_REG         0x0   /* configure register for top axi path */
#define SUB_CTRL_DMA_CRG_CFG1_REG         0x4   /* CRG configure register for reset */
#define SUB_CTRL_CVDR_MEM_CFG_REG         0x8   /* SRAM configure register for CVDR */
#define SUB_CTRL_CVDR_IRQ_REG0_REG        0x10  /* IRQ related cfg register */  //for debug???
#define SUB_CTRL_CVDR_IRQ_REG1_REG        0x14  /* IRQ related cfg register */
#define SUB_CTRL_CVDR_IRQ_REG2_REG        0x18  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_CTRL_REG          0x40  /* CMDLST cfg register for channel mapping */
#define SUB_CTRL_CMDLST_CHN0_REG          0x44  /* CMDLST cfg register for channel0 */
#define SUB_CTRL_CMDLST_CHN1_REG          0x48  /* CMDLST cfg register for channel1 */
#define SUB_CTRL_CMDLST_R8_IRQ_REG0_REG   0x5C  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_R8_IRQ_REG1_REG   0x60  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_R8_IRQ_REG2_REG   0x64  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG 0x68  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_ACPU_IRQ_REG1_REG 0x6C  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_ACPU_IRQ_REG2_REG 0x70  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_IVP_IRQ_REG0_REG  0x74  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_IVP_IRQ_REG1_REG  0x78  /* IRQ related cfg register */
#define SUB_CTRL_CMDLST_IVP_IRQ_REG2_REG  0x7C  /* IRQ related cfg register */
#define SUB_CTRL_JPG_FLUX_CTRL0_0_REG     0x80  /* JPG   flux ctrl grp0 for CVDR RT AXI R */
#define SUB_CTRL_JPG_FLUX_CTRL0_1_REG     0x84  /* JPG   flux ctrl grp1 for CVDR RT AXI R */
#define SUB_CTRL_JPG_FLUX_CTRL1_0_REG     0x88  /* JPG   flux ctrl grp0 for CVDR RT AXI W */
#define SUB_CTRL_JPG_FLUX_CTRL1_1_REG     0x8C  /* JPG   flux ctrl grp1 for CVDR RT AXI W */
#define SUB_CTRL_JPG_RO_STATE_REG         0xC0  /* some read only state */
#define SUB_CTRL_JPGENC_CRG_CFG0_REG      0x100 /* CRG configure register for clk */
#define SUB_CTRL_JPGENC_CRG_CFG1_REG      0x104 /* CRG configure register for reset */
#define SUB_CTRL_JPGENC_MEM_CFG_REG       0x108 /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_JPGENC_IRQ_REG0_REG      0x110 /* IRQ related cfg register */
#define SUB_CTRL_JPGENC_IRQ_REG1_REG      0x114 /* IRQ related cfg register */
#define SUB_CTRL_JPGENC_IRQ_REG2_REG      0x118 /* IRQ related cfg register */
#define SUB_CTRL_SLAM_CRG_CFG0_REG        0x280 /* CRG config register for clk */
#define SUB_CTRL_SLAM_CRG_CFG1_REG        0x284 /* CRG configure register for reset */
#define SUB_CTRL_SLAM_CRG_CFG2_REG        0x288 /* CRG config register for clk force on */
#define SUB_CTRL_SLAM_MEM_CFG1_REG        0x28C /* SRAM configure register for SLAM MCF */
#define SUB_CTRL_SLAM_IRQ_REG0_REG        0x290 /* IRQ related cfg register */
#define SUB_CTRL_SLAM_IRQ_REG1_REG        0x294 /* IRQ related cfg register */
#define SUB_CTRL_SLAM_IRQ_REG2_REG        0x298 /* IRQ related cfg register */
#define SUB_CTRL_JPG_DEBUG_0_REG          0x400 /* debug register 0 */
#define SUB_CTRL_JPG_DEBUG_1_REG          0x404 /* debug register 1 */
#define SUB_CTRL_JPG_DEBUG_2_REG          0x408 /* debug register 2 */
#define SUB_CTRL_JPG_DEBUG_3_REG          0x40C /* debug register 3 */
#define SUB_CTRL_JPG_SEC_CTRL_S_REG       0x800 /* JPG secure cfg register */

#endif // __SUB_CTRL_REG_OFFSET_H__
