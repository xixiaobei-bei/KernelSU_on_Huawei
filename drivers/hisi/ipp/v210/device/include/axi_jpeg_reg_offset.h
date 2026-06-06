//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  axi_jpeg_reg_offset.h
// Author        :  AnthonySixta
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  AnthonySixta 2018-01-03 Create file
//******************************************************************************

#ifndef __AXI_JPEG_REG_OFFSET_H__
#define __AXI_JPEG_REG_OFFSET_H__


/******************************************************************************/
/*                      HiStarISP AXI_JPEG Registers' Definitions                            */
/******************************************************************************/

#define AXI_JPEG_AXI_JPEG_CVDR_CFG_REG          0x0    /* CVDR config register. */                                                            
#define AXI_JPEG_AXI_JPEG_CVDR_DEBUG_EN_REG     0x4    /* CVDR debug register enable. */                                                      
#define AXI_JPEG_AXI_JPEG_CVDR_DEBUG_REG        0x8    /* CVDR debug register. */                                                             
#define AXI_JPEG_AXI_JPEG_CVDR_WR_QOS_CFG_REG   0xC    /* AXI Write QOS/Pressure configuration. */                                            
#define AXI_JPEG_AXI_JPEG_CVDR_RD_QOS_CFG_REG   0x10   /* AXI Read QOS/Pressure configuration. */                                             
#define AXI_JPEG_AXI_JPEG_FORCE_CLK_REG         0x14   /* Force clock ON */                                                                   
#define AXI_JPEG_AXI_JPEG_OTHER_RO_REG          0x20   /* Spare Other RO. */                                                                  
#define AXI_JPEG_AXI_JPEG_OTHER_RW_REG          0x24   /* Spare Other RW. */
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_4_REG       0x70   /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_4_REG    0x74   /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_4_REG  0x78   /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_4_REG    0x7C   /* Video port write interface configuration: prefetch or reset or stall capability. */
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_5_REG       0x80   /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_5_REG    0x84   /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_5_REG  0x88   /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_5_REG    0x8C   /* Video port write interface configuration: prefetch or reset or stall capability. */
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_6_REG       0x90   /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_6_REG    0x94   /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_6_REG  0x98   /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_6_REG    0x9C   /* Video port write interface configuration: prefetch or reset or stall capability. */
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_25_REG      0x1C0  /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_25_REG   0x1C4  /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_25_REG 0x1C8  /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_25_REG   0x1CC  /* Video port write interface configuration: prefetch or reset or stall capability. */ 
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_30_REG      0x210  /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_30_REG   0x214  /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_30_REG 0x218  /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_30_REG   0x21C  /* Video port write interface configuration: prefetch or reset or stall capability. */ 
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_4_REG   0x840  /* Video port write Access limiter. */                                                 
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_5_REG   0x844  /* Video port write Access limiter. */                                                 
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_6_REG   0x848  /* Video port write Access limiter. */   
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_25_REG  0x894  /* Video port write Access limiter. */                                                 
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_30_REG  0x8A8  /* Video port write Access limiter. */                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_4_REG       0xAB0  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_4_REG       0xAB4  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_4_REG       0xAB8  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_4_REG    0xABC  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_4_REG  0xAC0  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_4_REG    0xAC4  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_4_REG     0xACC  /* Video Port Read DEBUG information. */    
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_8_REG       0xB30  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_8_REG       0xB34  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_8_REG       0xB38 /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_8_REG    0xB3C  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_8_REG  0xB40  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_8_REG    0xB44  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_8_REG     0xB4C  /* Video Port Read DEBUG information. */  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_4_REG   0x1240 /* Video port read Access limiter. */  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_8_REG   0x1250 /* Video port read Access limiter. */  
#define AXI_JPEG_AXI_JPEG_NR_RD_CFG_4_REG       0x1570 /* Initiator read Configuration. */                                                    
#define AXI_JPEG_AXI_JPEG_NR_RD_DEBUG_4_REG     0x1574 /* Non-Raster Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_LIMITER_NR_RD_4_REG   0x1578 /* NR RD Access limiter. */                                                            
#define AXI_JPEG_AXI_JPEG_SPARE_0_REG           0x1D30 /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_SPARE_1_REG           0x1D34 /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_SPARE_2_REG           0x1D38 /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_SPARE_3_REG           0x1D3C /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_4_REG     0x1D50 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_5_REG     0x1D54 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_6_REG     0x1D58 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_25_REG    0x1DA4 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_30_REG    0x1DB8 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_DEBUG_0_REG           0x1F40 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_1_REG           0x1F44 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_2_REG           0x1F48 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_3_REG           0x1F4C /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_4_REG           0x1F50 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_5_REG           0x1F54 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_6_REG           0x1F58 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_7_REG           0x1F5C /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_8_REG           0x1F60 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_9_REG           0x1F64 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_10_REG          0x1F68 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_11_REG          0x1F6C /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_12_REG          0x1F70 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_13_REG          0x1F74 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_14_REG          0x1F78 /* DEBUG information. */                                                               
#define AXI_JPEG_AXI_JPEG_DEBUG_15_REG          0x1F7C /* DEBUG information. */                                                               

#endif // __AXI_JPEG_REG_OFFSET_H__
