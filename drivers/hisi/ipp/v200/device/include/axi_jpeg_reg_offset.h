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
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_8_REG       0xB0   /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_8_REG    0xB4   /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_8_REG  0xB8   /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_8_REG    0xBC   /* Video port write interface configuration: prefetch or reset or stall capability. */ 
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_9_REG       0xC0   /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_9_REG    0xC4   /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_9_REG  0xC8   /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_9_REG    0xCC   /* Video port write interface configuration: prefetch or reset or stall capability. */ 
#define AXI_JPEG_AXI_JPEG_VP_WR_CFG_10_REG      0xD0   /* Video port write Configuration. */                                                  
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_FS_10_REG   0xD4   /* AXI address Frame start. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_WR_AXI_LINE_10_REG 0xD8   /* AXI line wrap and line stride. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_WR_IF_CFG_10_REG   0xDC   /* Video port write interface configuration: prefetch or reset or stall capability. */ 
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
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_8_REG   0x850  /* Video port write Access limiter. */                                                 
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_9_REG   0x854  /* Video port write Access limiter. */                                                 
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_WR_10_REG  0x858  /* Video port write Access limiter. */                                                 
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
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_13_REG      0xBD0  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_13_REG      0xBD4  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_13_REG      0xBD8  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_13_REG   0xBDC  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_13_REG 0xBE0  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_13_REG   0xBE4  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_13_REG    0xBEC  /* Video Port Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_14_REG      0xBF0  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_14_REG      0xBF4  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_14_REG      0xBF8  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_14_REG   0xBFC  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_14_REG 0xC00  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_14_REG   0xC04  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_14_REG    0xC0C  /* Video Port Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_15_REG      0xC10  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_15_REG      0xC14  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_15_REG      0xC18  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_15_REG   0xC1C  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_15_REG 0xC20  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_15_REG   0xC24  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_15_REG    0xC2C  /* Video Port Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_16_REG      0xC30  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_16_REG      0xC34  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_16_REG      0xC38  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_16_REG   0xC3C  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_16_REG 0xC40  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_16_REG   0xC44  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_16_REG    0xC4C  /* Video Port Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_17_REG      0xC50  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_17_REG      0xC54  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_17_REG      0xC58  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_17_REG   0xC5C  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_17_REG 0xC60  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_17_REG   0xC64  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_17_REG    0xC6C  /* Video Port Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_VP_RD_CFG_18_REG      0xC70  /* Video port read Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_VP_RD_LWG_18_REG      0xC74  /* Line width generation. */                                                           
#define AXI_JPEG_AXI_JPEG_VP_RD_FHG_18_REG      0xC78  /* Frame height generation. */                                                         
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_FS_18_REG   0xC7C  /* AXI frame start. */                                                                 
#define AXI_JPEG_AXI_JPEG_VP_RD_AXI_LINE_18_REG 0xC80  /* Line Wrap definition. */                                                            
#define AXI_JPEG_AXI_JPEG_VP_RD_IF_CFG_18_REG   0xC84  /* Video port read interface configuration: prefetch or reset or stall capability. */  
#define AXI_JPEG_AXI_JPEG_VP_RD_DEBUG_18_REG    0xC8C  /* Video Port Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_4_REG   0x1240 /* Video port read Access limiter. */  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_8_REG   0x1250 /* Video port read Access limiter. */  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_13_REG  0x1264 /* Video port read Access limiter. */                                                  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_14_REG  0x1268 /* Video port read Access limiter. */                                                  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_15_REG  0x126C /* Video port read Access limiter. */                                                  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_16_REG  0x1270 /* Video port read Access limiter. */                                                  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_17_REG  0x1274 /* Video port read Access limiter. */                                                  
#define AXI_JPEG_AXI_JPEG_LIMITER_VP_RD_18_REG  0x1278 /* Video port read Access limiter. */                                                  
#define AXI_JPEG_AXI_JPEG_NR_WR_CFG_8_REG       0x13B0 /* Initiator write Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_NR_WR_DEBUG_8_REG     0x13B4 /* Non-Raster Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_LIMITER_NR_WR_8_REG   0x13B8 /* NR WR Access limiter. */                                                            
#define AXI_JPEG_AXI_JPEG_NR_WR_CFG_9_REG       0x13C0 /* Initiator write Configuration. */                                                   
#define AXI_JPEG_AXI_JPEG_NR_WR_DEBUG_9_REG     0x13C4 /* Non-Raster Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_LIMITER_NR_WR_9_REG   0x13C8 /* NR WR Access limiter. */                                                            
#define AXI_JPEG_AXI_JPEG_NR_RD_CFG_4_REG       0x1570 /* Initiator read Configuration. */                                                    
#define AXI_JPEG_AXI_JPEG_NR_RD_DEBUG_4_REG     0x1574 /* Non-Raster Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_LIMITER_NR_RD_4_REG   0x1578 /* NR RD Access limiter. */                                                            
#define AXI_JPEG_AXI_JPEG_NR_RD_CFG_8_REG       0x15B0 /* Initiator read Configuration. */                                                    
#define AXI_JPEG_AXI_JPEG_NR_RD_DEBUG_8_REG     0x15B4 /* Non-Raster Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_LIMITER_NR_RD_8_REG   0x15B8 /* NR RD Access limiter. */                                                            
#define AXI_JPEG_AXI_JPEG_NR_RD_CFG_9_REG       0x15C0 /* Initiator read Configuration. */                                                    
#define AXI_JPEG_AXI_JPEG_NR_RD_DEBUG_9_REG     0x15C4 /* Non-Raster Read DEBUG information. */                                               
#define AXI_JPEG_AXI_JPEG_LIMITER_NR_RD_9_REG   0x15C8 /* NR RD Access limiter. */                                                            
#define AXI_JPEG_AXI_JPEG_SPARE_0_REG           0x1D30 /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_SPARE_1_REG           0x1D34 /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_SPARE_2_REG           0x1D38 /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_SPARE_3_REG           0x1D3C /* Spare. */                                                                           
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_4_REG     0x1D50 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_5_REG     0x1D54 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_6_REG     0x1D58 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_8_REG     0x1D60 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_9_REG     0x1D64 /* Video Port Write DEBUG information. */                                              
#define AXI_JPEG_AXI_JPEG_VP_WR_DEBUG_10_REG    0x1D68 /* Video Port Write DEBUG information. */                                              
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
