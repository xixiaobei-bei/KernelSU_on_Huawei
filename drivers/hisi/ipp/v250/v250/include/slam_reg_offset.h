//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  slam_reg_offset.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-07-30
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-07-30 Create file
//******************************************************************************

#ifndef __SLAM_REG_OFFSET_CS_H__
#define __SLAM_REG_OFFSET_CS_H__


/******************************************************************************/
/*                      HiStarISP SLAM Registers' Definitions                            */
/******************************************************************************/

#define SLAM_SLAM_CFG_REG           0x0   /* SLAM Top Config */
#define SLAM_IMAGE_SIZE_REG         0x4   /* Image size config */
#define SLAM_TOTAL_KPT_NUM_REG      0x8   /* Total kpt number */
#define SLAM_PYRAMID_INC_CFG_REG    0x10  /* PYRAMID Scaler INC Config */
#define SLAM_PYRAMID_VCROP_CFGB_REG 0x14  /* PYRAMID Scaler Vertical CROP Config bottom */
#define SLAM_PYRAMID_VCROP_CFGT_REG 0x18  /* PYRAMID Scaler Vertical CROP Config top */
#define SLAM_PYRAMID_HCROP_CFGR_REG 0x1C  /* PYRAMID Scaler Horizontal CROP Config R */
#define SLAM_PYRAMID_HCROP_CFGL_REG 0x20  /* Pyramid Scaler Horizontal CROP Config L */
#define SLAM_GSBLUR_COEF_01_REG     0x30  /* Gauss Coefficients 0 and 1 */
#define SLAM_GSBLUR_COEF_23_REG     0x34  /* Gauss Coefficients 2 and 3 */
#define SLAM_THRESHOLD_CFG_REG      0x40  /* Threshold for min and initial */
#define SLAM_NMS_WIN_CFG_REG        0x44  /* NMS window size */
#define SLAM_BLOCK_NUM_CFG_REG      0x50  /* BLOCK Number */
#define SLAM_BLOCK_SIZE_CFG_REG     0x54  /* BLOCK Size */
#define SLAM_OCTREE_CFG_REG         0x58  /* octree limit */
#define SLAM_INC_LUT_CFG_REG        0x5C  /* inc_lut configure */
#define SLAM_UNDISTORT_CX_REG       0x60  /* CX */
#define SLAM_UNDISTORT_CY_REG       0x64  /* CY */
#define SLAM_UNDISTORT_FX_REG       0x68  /* FX */
#define SLAM_UNDISTORT_FY_REG       0x6C  /* FY */
#define SLAM_UNDISTORT_INVFX_REG    0x70  /* INVFX */
#define SLAM_UNDISTORT_INVFY_REG    0x74  /* INVFY */
#define SLAM_UNDISTORT_K1_REG       0x78  /* K1 */
#define SLAM_UNDISTORT_K2_REG       0x7C  /* K2 */
#define SLAM_UNDISTORT_K3_REG       0x80  /* K3 */
#define SLAM_UNDISTORT_P1_REG       0x84  /* P1 */
#define SLAM_UNDISTORT_P2_REG       0x88  /* P2 */
#define SLAM_CVDR_CFG_0_REG         0x90  /* Line width generation. */
#define SLAM_CVDR_CFG_1_REG         0x94  /* Line Wrap definition. */
#define SLAM_DEBUG_0_REG            0xA0  /* debug signal 0 */
#define SLAM_DEBUG_1_REG            0xA4  /* debug signal 1 */
#define SLAM_SCORE_HIST_0_REG       0x100 /* score hist */
#define SLAM_SCORE_HIST_1_REG       0x104 /* score hist */
#define SLAM_SCORE_HIST_2_REG       0x108 /* score hist */
#define SLAM_SCORE_HIST_3_REG       0x10C /* score hist */
#define SLAM_SCORE_HIST_4_REG       0x110 /* score hist */
#define SLAM_SCORE_HIST_5_REG       0x114 /* score hist */
#define SLAM_SCORE_HIST_6_REG       0x118 /* score hist */
#define SLAM_SCORE_HIST_7_REG       0x11C /* score hist */
#define SLAM_SCORE_HIST_8_REG       0x120 /* score hist */
#define SLAM_SCORE_HIST_9_REG       0x124 /* score hist */
#define SLAM_SCORE_HIST_10_REG      0x128 /* score hist */
#define SLAM_SCORE_HIST_11_REG      0x12C /* score hist */
#define SLAM_SCORE_HIST_12_REG      0x130 /* score hist */
#define SLAM_SCORE_HIST_13_REG      0x134 /* score hist */
#define SLAM_SCORE_HIST_14_REG      0x138 /* score hist */
#define SLAM_SCORE_HIST_15_REG      0x13C /* score hist */
#define SLAM_SCORE_HIST_16_REG      0x140 /* score hist */
#define SLAM_SCORE_HIST_17_REG      0x144 /* score hist */
#define SLAM_SCORE_HIST_18_REG      0x148 /* score hist */
#define SLAM_SCORE_HIST_19_REG      0x14C /* score hist */
#define SLAM_SCORE_HIST_20_REG      0x150 /* score hist */
#define SLAM_SCORE_HIST_21_REG      0x154 /* score hist */
#define SLAM_SCORE_HIST_22_REG      0x158 /* score hist */
#define SLAM_SCORE_HIST_23_REG      0x15C /* score hist */
#define SLAM_SCORE_HIST_24_REG      0x160 /* score hist */
#define SLAM_SCORE_HIST_25_REG      0x164 /* score hist */
#define SLAM_SCORE_HIST_26_REG      0x168 /* score hist */
#define SLAM_SCORE_HIST_27_REG      0x16C /* score hist */
#define SLAM_SCORE_HIST_28_REG      0x170 /* score hist */
#define SLAM_SCORE_HIST_29_REG      0x174 /* score hist */
#define SLAM_SCORE_HIST_30_REG      0x178 /* score hist */
#define SLAM_SCORE_HIST_31_REG      0x17C /* score hist */
#define SLAM_SCORE_HIST_32_REG      0x180 /* score hist */
#define SLAM_SCORE_HIST_33_REG      0x184 /* score hist */
#define SLAM_SCORE_HIST_34_REG      0x188 /* score hist */
#define SLAM_SCORE_HIST_35_REG      0x18C /* score hist */
#define SLAM_SCORE_HIST_36_REG      0x190 /* score hist */
#define SLAM_SCORE_HIST_37_REG      0x194 /* score hist */
#define SLAM_SCORE_HIST_38_REG      0x198 /* score hist */
#define SLAM_SCORE_HIST_39_REG      0x19C /* score hist */
#define SLAM_SCORE_HIST_40_REG      0x1A0 /* score hist */
#define SLAM_SCORE_HIST_41_REG      0x1A4 /* score hist */
#define SLAM_SCORE_HIST_42_REG      0x1A8 /* score hist */
#define SLAM_SCORE_HIST_43_REG      0x1AC /* score hist */
#define SLAM_SCORE_HIST_44_REG      0x1B0 /* score hist */
#define SLAM_SCORE_HIST_45_REG      0x1B4 /* score hist */
#define SLAM_SCORE_HIST_46_REG      0x1B8 /* score hist */
#define SLAM_SCORE_HIST_47_REG      0x1BC /* score hist */
#define SLAM_SCORE_HIST_48_REG      0x1C0 /* score hist */
#define SLAM_SCORE_HIST_49_REG      0x1C4 /* score hist */
#define SLAM_BRIEF_PATTERN_0_REG    0x400 /* pattern */
#define SLAM_SCORE_THESHOLD_0_REG   0x800 /* score threshold of each block for octree */
#define SLAM_KPT_NUMBER_0_REG       0xA00 /* Feature number in each block */

#endif // __SLAM_REG_OFFSET_CS_H__
