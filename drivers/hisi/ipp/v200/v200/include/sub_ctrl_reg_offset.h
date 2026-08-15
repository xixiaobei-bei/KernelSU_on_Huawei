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

#define SUB_CTRL_JPGENC_CRG_CFG0_REG      0x0   /* CRG configure register for clk */            
#define SUB_CTRL_JPGENC_CRG_CFG1_REG      0x4   /* CRG configure register for reset */          
#define SUB_CTRL_JPGENC_MEM_CFG_REG       0x8   /* SRAM configure register for jpgenc */        
#define SUB_CTRL_JPG_RO_STATE_REG         0xC   /* some read only state */                      
#define SUB_CTRL_JPGDEC_CRG_CFG0_REG      0x20  /* CRG config register for clk */               
#define SUB_CTRL_JPGDEC_CRG_CFG1_REG      0x24  /* CRG configure register for reset */          
#define SUB_CTRL_JPGDEC_MEM_CFG_REG       0x28  /* SRAM configure register for jpgdec */        
#define SUB_CTRL_JPGDEC_IRQ_REG0_REG      0x2C  /* IRQ related cfg register */                  
#define SUB_CTRL_JPGDEC_IRQ_REG1_REG      0x30  /* IRQ related cfg register */                  
#define SUB_CTRL_JPGDEC_IRQ_REG2_REG      0x34  /* IRQ related cfg register */                  
#define SUB_CTRL_JPGDEC_MEM_CFG2_REG      0x38  /* SRAM configure register group2 for jpgdec */ 
#define SUB_CTRL_TOP_AXI_CFG0_REG         0x40  /* configure register for top axi path */       
#define SUB_CTRL_CVDR_MEM_CFG_REG         0x44  /* SRAM configure register for CVDR */          
#define SUB_CTRL_CVDR_IRQ_REG0_REG        0x48  /* IRQ related cfg register */                  
#define SUB_CTRL_CVDR_IRQ_REG1_REG        0x4C  /* IRQ related cfg register */                  
#define SUB_CTRL_CVDR_IRQ_REG2_REG        0x50  /* IRQ related cfg register */                  
#define SUB_CTRL_DMA_CRG_CFG1_REG         0x54  /* CRG configure register for reset */          
#define SUB_CTRL_CVDR_IRQ_REG01_REG       0x58  /* IRQ related cfg register */                  
#define SUB_CTRL_CVDR_IRQ_REG21_REG       0x5C  /* IRQ related cfg register */                  
#define SUB_CTRL_JPG_OTHER_MERGE_IRQ_REG  0x60  /* merged IRQ status cfg register */            
#define SUB_CTRL_JPG_OTHER_MERGE_IRQ2_REG 0x64  /* merged IRQ status cfg register */            
#define SUB_CTRL_FD_AXI_CFG0_REG          0x70  /* configure register for FD axi path */        
#define SUB_CTRL_FD_CRG_CFG0_REG          0x74  /* CRG config register for clk */               
#define SUB_CTRL_FD_CRG_CFG1_REG          0x78  /* CRG configure register for reset */          
#define SUB_CTRL_FD_MEM_CFG_REG           0x7C  /* SRAM configure register for FD */            
#define SUB_CTRL_JPG_FLUX_CTRL0_0_REG     0x80  /* JPG   flux ctrl grp0 for CVDR RT AXI R */    
#define SUB_CTRL_JPG_FLUX_CTRL0_1_REG     0x84  /* JPG   flux ctrl grp1 for CVDR RT AXI R */    
#define SUB_CTRL_JPG_FLUX_CTRL1_0_REG     0x88  /* JPG   flux ctrl grp0 for CVDR RT AXI W */    
#define SUB_CTRL_JPG_FLUX_CTRL1_1_REG     0x8C  /* JPG   flux ctrl grp1 for CVDR RT AXI W */    
#define SUB_CTRL_JPG_FLUX_CTRL2_0_REG     0x90  /* JPG   flux ctrl grp0 for CVDR SRT AXI R */   
#define SUB_CTRL_JPG_FLUX_CTRL2_1_REG     0x94  /* JPG   flux ctrl grp1 for CVDR SRT AXI R */   
#define SUB_CTRL_JPG_FLUX_CTRL3_0_REG     0x98  /* JPG   flux ctrl grp0 for CVDR SRT AXI W */   
#define SUB_CTRL_JPG_FLUX_CTRL3_1_REG     0x9C  /* JPG   flux ctrl grp1 for CVDR SRT AXI W */   
#define SUB_CTRL_FD_SMMU_MASTER_REG0_REG  0xB0  /* FD frame stream initial signal */            
#define SUB_CTRL_FD_SMMU_MASTER_REG1_REG  0xB4  /* IRQ related cfg register */                  
#define SUB_CTRL_FD_SMMU_MASTER_REG2_REG  0xB8  /* IRQ related cfg register */                  
#define SUB_CTRL_HIFD_CRG_CFG0_REG        0xA0  /* HIFD CRG config register for clk */          
#define SUB_CTRL_HIFD_CRG_CFG1_REG        0xA4  /* HIFD CRG configure register for reset */     
#define SUB_CTRL_JPG_DEBUG_0_REG          0x100 /* debug register 0 */                          
#define SUB_CTRL_JPG_DEBUG_1_REG          0x104 /* debug register 1 */                          
#define SUB_CTRL_JPG_DEBUG_2_REG          0x108 /* debug register 2 */                          
#define SUB_CTRL_JPG_DEBUG_3_REG          0x10C /* debug register 3 */                          
#define SUB_CTRL_SLAM_CRG_CFG0_REG        0x180 /* CRG config register for clk */               
#define SUB_CTRL_SLAM_CRG_CFG1_REG        0x184 /* CRG configure register for reset */          
#define SUB_CTRL_SLAM_CRG_CFG2_REG        0x188 /* CRG config register for clk force on */      
#define SUB_CTRL_SLAM_MEM_CFG1_REG        0x18C /* SRAM configure register for SLAM MCF */      
#define SUB_CTRL_SLAM_COMP_IRQ_REG_REG    0x190 /* IRQ related cfg register */                  
#define SUB_CTRL_SLAM_COMP_IRQ_REG2_REG   0x194 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_CRG_CFG0_REG         0x200 /* CRG config register for clk */               
#define SUB_CTRL_CPE_CRG_CFG1_REG         0x204 /* CRG configure register for reset */          
#define SUB_CTRL_CPE_CRG_CFG2_REG         0x208 /* CRG config register for clk force on */      
#define SUB_CTRL_CPE_MEM_CFG0_REG         0x210 /* SRAM configure register0 for CPE */          
#define SUB_CTRL_CPE_MEM_CFG1_REG         0x214 /* SRAM configure register for CPE MCF */       
#define SUB_CTRL_CPE_MEM_CFG2_REG         0x218 /* SRAM configure register for CPE MFNR */      
#define SUB_CTRL_CROP_VPWR_0_REG          0x220 /* Crop horizontal offsets. */                  
#define SUB_CTRL_CROP_VPWR_1_REG          0x224 /* Crop horizontal offsets. */                  
#define SUB_CTRL_CROP_VPWR_2_REG          0x228 /* Crop horizontal offsets. */                  
#define SUB_CTRL_CPE_MODE_CFG_REG         0x240 /* configure register for CPE work mode */      
#define SUB_CTRL_CPE_IRQ0_REG0_REG        0x260 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ0_REG1_REG        0x264 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ0_REG2_REG        0x268 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ0_REG3_REG        0x26C /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ0_REG4_REG        0x270 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ1_REG0_REG        0x274 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ1_REG1_REG        0x278 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ1_REG2_REG        0x27C /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ1_REG3_REG        0x280 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_IRQ1_REG4_REG        0x284 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_COMP_IRQ_REG_REG     0x288 /* IRQ related cfg register */                  
#define SUB_CTRL_CPE_COMP_IRQ_REG2_REG    0x28C /* IRQ related cfg register */                  
#define SUB_CTRL_JPG_SEC_CTRL_S_REG       0x800 /* JPG secure cfg register */                   
#define SUB_CTRL_FD_AXI_CFG1_S_REG        0x810 /* configure register for FD axi path */        

#endif // __SUB_CTRL_REG_OFFSET_H__
