//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  mfnr_reg_offset.h
// Author        :  HerveDANIEL
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  HerveDANIEL 2018-01-03 Create file
//******************************************************************************

#ifndef __MFNR_REG_OFFSET_H__
#define __MFNR_REG_OFFSET_H__


/******************************************************************************/
/*                      HISTARISP MFNR Registers' Definitions                            */
/******************************************************************************/

#define MFNR_MFNR_CFG_REG                      0x0   /* MFNR Top Config */                                 
#define MFNR_MFNR_SIZE_CFG_REG                 0x4   /* MFNR Input Size Config */                          
#define MFNR_GHOST_DETECT_CFG_REG              0x8   /* MFNR Ghost Detect Config */                        
#define MFNR_GHOST_DETECT_US16_CROP_CFG_REG    0xC   /* MFNR Ghost Detect Upscale 16 Post Crop Config */   
#define MFNR_GHOST_DETECT_STATS_CROP_H_CFG_REG 0x10  /* MFNR Ghost Detect Stats Horizontal Crop Config */  
#define MFNR_TEMPO_FUSION_CFG_REG              0x14  /* MFNR Temporal Fusion Config for Y, U, and V */     
#define MFNR_DEGHOST_SIGMA_STREN_CFG_REG       0x18  /* MFNR Deghost Sigma Strength Config */              
#define MFNR_DEGHOST_BLEND_STREN_CFG_REG       0x1C  /* MFNR Deghost Blend Strength Config */              
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_0_REG      0x20  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_1_REG      0x24  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_2_REG      0x28  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_3_REG      0x2C  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_4_REG      0x30  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_5_REG      0x34  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_6_REG      0x38  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_7_REG      0x3C  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_8_REG      0x40  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_9_REG      0x44  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_10_REG     0x48  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_11_REG     0x4C  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_12_REG     0x50  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_13_REG     0x54  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_14_REG     0x58  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_15_REG     0x5C  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_16_REG     0x60  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_17_REG     0x64  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_18_REG     0x68  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_19_REG     0x6C  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_20_REG     0x70  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_21_REG     0x74  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_22_REG     0x78  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_23_REG     0x7C  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_24_REG     0x80  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2SIGMA_LUT_25_REG     0x84  /* MFNR Deghost mean2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_0_REG      0x88  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_1_REG      0x8C  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_2_REG      0x90  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_3_REG      0x94  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_4_REG      0x98  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_5_REG      0x9C  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_6_REG      0xA0  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_7_REG      0xA4  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_8_REG      0xA8  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_9_REG      0xAC  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_10_REG     0xB0  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_11_REG     0xB4  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_12_REG     0xB8  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_13_REG     0xBC  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_14_REG     0xC0  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_15_REG     0xC4  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_16_REG     0xC8  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_17_REG     0xCC  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_18_REG     0xD0  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_19_REG     0xD4  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_20_REG     0xD8  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_21_REG     0xDC  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_22_REG     0xE0  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_23_REG     0xE4  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_24_REG     0xE8  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_25_REG     0xEC  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_26_REG     0xF0  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_27_REG     0xF4  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_28_REG     0xF8  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_29_REG     0xFC  /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_30_REG     0x100 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_31_REG     0x104 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_32_REG     0x108 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_33_REG     0x10C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_34_REG     0x110 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_35_REG     0x114 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_36_REG     0x118 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_37_REG     0x11C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_38_REG     0x120 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_39_REG     0x124 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_40_REG     0x128 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_41_REG     0x12C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_42_REG     0x130 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_43_REG     0x134 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_44_REG     0x138 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_45_REG     0x13C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_46_REG     0x140 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_47_REG     0x144 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_48_REG     0x148 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_49_REG     0x14C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_50_REG     0x150 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_51_REG     0x154 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_52_REG     0x158 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_53_REG     0x15C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_54_REG     0x160 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_55_REG     0x164 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_56_REG     0x168 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_57_REG     0x16C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_58_REG     0x170 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_59_REG     0x174 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_60_REG     0x178 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_61_REG     0x17C /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_62_REG     0x180 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_63_REG     0x184 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_GRAD2SIGMA_LUT_64_REG     0x188 /* MFNR Deghost grad2sigma LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_0_REG      0x18C /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_1_REG      0x190 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_2_REG      0x194 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_3_REG      0x198 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_4_REG      0x19C /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_5_REG      0x1A0 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_6_REG      0x1A4 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_7_REG      0x1A8 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_8_REG      0x1AC /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_9_REG      0x1B0 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_10_REG     0x1B4 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_11_REG     0x1B8 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_12_REG     0x1BC /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_13_REG     0x1C0 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_14_REG     0x1C4 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_15_REG     0x1C8 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_16_REG     0x1CC /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_17_REG     0x1D0 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_18_REG     0x1D4 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_19_REG     0x1D8 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_20_REG     0x1DC /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_21_REG     0x1E0 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_22_REG     0x1E4 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_23_REG     0x1E8 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_24_REG     0x1EC /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_MEAN2BLEND_LUT_25_REG     0x1F0 /* MFNR Deghost mean2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_0_REG      0x1F4 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_1_REG      0x1F8 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_2_REG      0x1FC /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_3_REG      0x200 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_4_REG      0x204 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_5_REG      0x208 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_6_REG      0x20C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_7_REG      0x210 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_8_REG      0x214 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_9_REG      0x218 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_10_REG     0x21C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_11_REG     0x220 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_12_REG     0x224 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_13_REG     0x228 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_14_REG     0x22C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_15_REG     0x230 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_16_REG     0x234 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_17_REG     0x238 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_18_REG     0x23C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_19_REG     0x240 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_20_REG     0x244 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_21_REG     0x248 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_22_REG     0x24C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_23_REG     0x250 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_24_REG     0x254 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_25_REG     0x258 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_26_REG     0x25C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_27_REG     0x260 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_28_REG     0x264 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_29_REG     0x268 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_30_REG     0x26C /* MFNR Deghost grad2blend LUT */                     
#define MFNR_DEGHOST_GRAD2BLEND_LUT_31_REG     0x270 /* MFNR Deghost grad2blend LUT */                     
#define MFNR_GHOST_DETECT_SUM_DIFF_U_STATS_REG 0x274 /* MFNR Ghost Detect Sum Difference Chroma U Stats */ 
#define MFNR_GHOST_DETECT_SUM_DIFF_V_STATS_REG 0x278 /* MFNR Ghost Detect Sum Difference Chroma V Stats */ 
#define MFNR_GHOST_DETECT_GHOST_SIZE_STATS_REG 0x27C /* MFNR Ghost Detect Ghost Size Stats */              
#define MFNR_GHOST_DETECT_TOP_DEBUG_REG        0x280 /* Ghost Detect Top Debug */                          
#define MFNR_GHOST_DETECT_GAUSS7X7_DEBUG_REG   0x284 /* Ghost Detect Gauss 7x7 Debug */                    
#define MFNR_GHOST_DETECT_DILATION_DEBUG_REG   0x288 /* Ghost Detect Dilation Debug */                     
#define MFNR_GHOST_DETECT_GAUSS3X3_DEBUG_REG   0x28C /* Ghost Detect Gauss 3x3 Debug */                    
#define MFNR_GHOST_DETECT_US16_DEBUG_REG       0x290 /* Ghost Detect Upscale 16 Debug */                   
#define MFNR_DEGHOST_TOP_DEBUG_REG             0x294 /* Deghost Top Debug */                               

#endif // __MFNR_REG_OFFSET_H__
