//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  mcf_reg_offset.h
// Author        :  HerveDANIEL
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  HerveDANIEL 2018-01-03 Create file
//******************************************************************************

#ifndef __MCF_REG_OFFSET_H__
#define __MCF_REG_OFFSET_H__


/******************************************************************************/
/*                      HISTARISP MCF Registers' Definitions                            */
/******************************************************************************/

#define MCF_MCF_CFG_REG                       0x0  /* MCF Top Config */                                 
#define MCF_MCF_SIZE_CFG_REG                  0x4  /* MCF Input Size Config */                          
#define MCF_UV_FSN_GET_CAND_CFG_REG           0x8  /* UV Fusion Get Candidate Config */                 
#define MCF_UV_FSN_GET_FUV_RATIO_CFG_REG      0xC  /* UV Fusion Final UV Ratio Config */                
#define MCF_UV_FSN_GET_FUV_MAX_U_DIFF_CFG_REG 0x10 /* UV Fusion Get Final UV Max U Difference Config */ 
#define MCF_UV_FSN_GET_FUV_MAX_V_DIFF_CFG_REG 0x14 /* UV Fusion Get Final UV Max V Difference Config */ 
#define MCF_UV_FSN_GET_CONF_CFG_REG           0x18 /* UV Fusion Get Confidence Config */                
#define MCF_UV_FSN_GET_CONF_MAX_DIFF_CFG_REG  0x1C /* UV Fusion Get Confidence Max Difference Config */ 
#define MCF_FILL_CONFLICT_CFG_REG             0x20 /* Fill Conflict Config */                           
#define MCF_SMOOTH_BLEND_CFG_REG              0x24 /* Smooth Blending Config */                         
#define MCF_SMOOTH_BLEND_GF_CFG_REG           0x28 /* Smooth Blending Guided Filter Config */           
#define MCF_ADJUST_COLOR_GF_CFG_REG           0x2C /* Adjust Color Guided Filter Config */              
#define MCF_ADJUST_COLOR_GET_MASK_CFG_REG     0x30 /* Adjust Color Get Mask Config */                   
#define MCF_ADJUST_COLOR_GET_RATIO_CFG_REG    0x34 /* Adjust Color Get Ratio Config */                  
#define MCF_MCF_DEBUG_REG                     0x38 /* MCF Top Debug */                                  
#define MCF_UV_FSN_DEBUG_REG                  0x3C /* UV Fusion Debug */                                
#define MCF_FILL_CONFLICT_DEBUG_REG           0x40 /* Filled Conflict Debug */                          
#define MCF_SMOOTH_BLEND_DEBUG_REG            0x44 /* Smooth Blend Debug */                             
#define MCF_ADJUST_COLOR_DEBUG_REG            0x48 /* Adjust Color Debug */                             

#endif // __MCF_REG_OFFSET_H__
