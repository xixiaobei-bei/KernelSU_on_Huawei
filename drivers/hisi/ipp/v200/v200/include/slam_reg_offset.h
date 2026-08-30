//****************************************************************************** 
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  slam_reg_offset.h
// Author        :  HerveDANIEL
// Version       :  1.0 
// Date          :  2018-01-03
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  HerveDANIEL 2018-01-03 Create file
//******************************************************************************

#ifndef __SLAM_REG_OFFSET_H__
#define __SLAM_REG_OFFSET_H__


/******************************************************************************/
/*                      HISTARISP SLAM Registers' Definitions                            */
/******************************************************************************/

#define SLAM_SLAM_CFG_REG           0x0   /* SLAM Top Config */                            
#define SLAM_IMAGE_SIZE_REG         0x4   /* Image size config */                          
#define SLAM_PYRAMID_INC_CFG_REG    0x8   /* PYRAMID Scaler INC Config */                  
#define SLAM_PYRAMID_VCROP_CFGB_REG 0xC   /* PYRAMID Scaler Vertical CROP Config bottom */ 
#define SLAM_PYRAMID_VCROP_CFGT_REG 0x10  /* PYRAMID Scaler Vertical CROP Config top */    
#define SLAM_PYRAMID_HCROP_CFGR_REG 0x14  /* PYRAMID Scaler Horizontal CROP Config R */    
#define SLAM_PYRAMID_HCROP_CFGL_REG 0x18  /* Pyramid Scaler Horizontal CROP Config L */    
#define SLAM_GSBLUR_COEF_01_REG     0x1C  /* Gauss Coefficients 0 and 1 */                 
#define SLAM_GSBLUR_COEF_23_REG     0x20  /* Gauss Coefficients 2 and 3 */                 
#define SLAM_THRESHOLD_CFG_REG      0x24  /* Threshold for min and initial */              
#define SLAM_NMS_WIN_CFG_REG        0x28  /* NMS window size */                            
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
#define SLAM_SCORE_HIST_50_REG      0x1C8 /* score hist */                                 
#define SLAM_SCORE_HIST_51_REG      0x1CC /* score hist */                                 
#define SLAM_SCORE_HIST_52_REG      0x1D0 /* score hist */                                 
#define SLAM_SCORE_HIST_53_REG      0x1D4 /* score hist */                                 
#define SLAM_SCORE_HIST_54_REG      0x1D8 /* score hist */                                 
#define SLAM_SCORE_HIST_55_REG      0x1DC /* score hist */                                 
#define SLAM_SCORE_HIST_56_REG      0x1E0 /* score hist */                                 
#define SLAM_SCORE_HIST_57_REG      0x1E4 /* score hist */                                 
#define SLAM_SCORE_HIST_58_REG      0x1E8 /* score hist */                                 
#define SLAM_SCORE_HIST_59_REG      0x1EC /* score hist */                                 
#define SLAM_SCORE_HIST_60_REG      0x1F0 /* score hist */                                 
#define SLAM_SCORE_HIST_61_REG      0x1F4 /* score hist */                                 
#define SLAM_SCORE_HIST_62_REG      0x1F8 /* score hist */                                 
#define SLAM_BRIEF_PATTERN_0_REG    0x400 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_1_REG    0x404 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_2_REG    0x408 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_3_REG    0x40C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_4_REG    0x410 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_5_REG    0x414 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_6_REG    0x418 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_7_REG    0x41C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_8_REG    0x420 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_9_REG    0x424 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_10_REG   0x428 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_11_REG   0x42C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_12_REG   0x430 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_13_REG   0x434 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_14_REG   0x438 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_15_REG   0x43C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_16_REG   0x440 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_17_REG   0x444 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_18_REG   0x448 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_19_REG   0x44C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_20_REG   0x450 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_21_REG   0x454 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_22_REG   0x458 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_23_REG   0x45C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_24_REG   0x460 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_25_REG   0x464 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_26_REG   0x468 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_27_REG   0x46C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_28_REG   0x470 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_29_REG   0x474 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_30_REG   0x478 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_31_REG   0x47C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_32_REG   0x480 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_33_REG   0x484 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_34_REG   0x488 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_35_REG   0x48C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_36_REG   0x490 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_37_REG   0x494 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_38_REG   0x498 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_39_REG   0x49C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_40_REG   0x4A0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_41_REG   0x4A4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_42_REG   0x4A8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_43_REG   0x4AC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_44_REG   0x4B0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_45_REG   0x4B4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_46_REG   0x4B8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_47_REG   0x4BC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_48_REG   0x4C0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_49_REG   0x4C4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_50_REG   0x4C8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_51_REG   0x4CC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_52_REG   0x4D0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_53_REG   0x4D4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_54_REG   0x4D8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_55_REG   0x4DC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_56_REG   0x4E0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_57_REG   0x4E4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_58_REG   0x4E8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_59_REG   0x4EC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_60_REG   0x4F0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_61_REG   0x4F4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_62_REG   0x4F8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_63_REG   0x4FC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_64_REG   0x500 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_65_REG   0x504 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_66_REG   0x508 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_67_REG   0x50C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_68_REG   0x510 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_69_REG   0x514 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_70_REG   0x518 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_71_REG   0x51C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_72_REG   0x520 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_73_REG   0x524 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_74_REG   0x528 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_75_REG   0x52C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_76_REG   0x530 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_77_REG   0x534 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_78_REG   0x538 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_79_REG   0x53C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_80_REG   0x540 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_81_REG   0x544 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_82_REG   0x548 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_83_REG   0x54C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_84_REG   0x550 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_85_REG   0x554 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_86_REG   0x558 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_87_REG   0x55C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_88_REG   0x560 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_89_REG   0x564 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_90_REG   0x568 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_91_REG   0x56C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_92_REG   0x570 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_93_REG   0x574 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_94_REG   0x578 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_95_REG   0x57C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_96_REG   0x580 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_97_REG   0x584 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_98_REG   0x588 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_99_REG   0x58C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_100_REG  0x590 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_101_REG  0x594 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_102_REG  0x598 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_103_REG  0x59C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_104_REG  0x5A0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_105_REG  0x5A4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_106_REG  0x5A8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_107_REG  0x5AC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_108_REG  0x5B0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_109_REG  0x5B4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_110_REG  0x5B8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_111_REG  0x5BC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_112_REG  0x5C0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_113_REG  0x5C4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_114_REG  0x5C8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_115_REG  0x5CC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_116_REG  0x5D0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_117_REG  0x5D4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_118_REG  0x5D8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_119_REG  0x5DC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_120_REG  0x5E0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_121_REG  0x5E4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_122_REG  0x5E8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_123_REG  0x5EC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_124_REG  0x5F0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_125_REG  0x5F4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_126_REG  0x5F8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_127_REG  0x5FC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_128_REG  0x600 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_129_REG  0x604 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_130_REG  0x608 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_131_REG  0x60C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_132_REG  0x610 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_133_REG  0x614 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_134_REG  0x618 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_135_REG  0x61C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_136_REG  0x620 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_137_REG  0x624 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_138_REG  0x628 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_139_REG  0x62C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_140_REG  0x630 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_141_REG  0x634 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_142_REG  0x638 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_143_REG  0x63C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_144_REG  0x640 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_145_REG  0x644 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_146_REG  0x648 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_147_REG  0x64C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_148_REG  0x650 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_149_REG  0x654 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_150_REG  0x658 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_151_REG  0x65C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_152_REG  0x660 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_153_REG  0x664 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_154_REG  0x668 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_155_REG  0x66C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_156_REG  0x670 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_157_REG  0x674 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_158_REG  0x678 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_159_REG  0x67C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_160_REG  0x680 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_161_REG  0x684 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_162_REG  0x688 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_163_REG  0x68C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_164_REG  0x690 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_165_REG  0x694 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_166_REG  0x698 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_167_REG  0x69C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_168_REG  0x6A0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_169_REG  0x6A4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_170_REG  0x6A8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_171_REG  0x6AC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_172_REG  0x6B0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_173_REG  0x6B4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_174_REG  0x6B8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_175_REG  0x6BC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_176_REG  0x6C0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_177_REG  0x6C4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_178_REG  0x6C8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_179_REG  0x6CC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_180_REG  0x6D0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_181_REG  0x6D4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_182_REG  0x6D8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_183_REG  0x6DC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_184_REG  0x6E0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_185_REG  0x6E4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_186_REG  0x6E8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_187_REG  0x6EC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_188_REG  0x6F0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_189_REG  0x6F4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_190_REG  0x6F8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_191_REG  0x6FC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_192_REG  0x700 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_193_REG  0x704 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_194_REG  0x708 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_195_REG  0x70C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_196_REG  0x710 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_197_REG  0x714 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_198_REG  0x718 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_199_REG  0x71C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_200_REG  0x720 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_201_REG  0x724 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_202_REG  0x728 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_203_REG  0x72C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_204_REG  0x730 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_205_REG  0x734 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_206_REG  0x738 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_207_REG  0x73C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_208_REG  0x740 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_209_REG  0x744 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_210_REG  0x748 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_211_REG  0x74C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_212_REG  0x750 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_213_REG  0x754 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_214_REG  0x758 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_215_REG  0x75C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_216_REG  0x760 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_217_REG  0x764 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_218_REG  0x768 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_219_REG  0x76C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_220_REG  0x770 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_221_REG  0x774 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_222_REG  0x778 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_223_REG  0x77C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_224_REG  0x780 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_225_REG  0x784 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_226_REG  0x788 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_227_REG  0x78C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_228_REG  0x790 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_229_REG  0x794 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_230_REG  0x798 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_231_REG  0x79C /* pattern */                                    
#define SLAM_BRIEF_PATTERN_232_REG  0x7A0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_233_REG  0x7A4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_234_REG  0x7A8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_235_REG  0x7AC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_236_REG  0x7B0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_237_REG  0x7B4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_238_REG  0x7B8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_239_REG  0x7BC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_240_REG  0x7C0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_241_REG  0x7C4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_242_REG  0x7C8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_243_REG  0x7CC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_244_REG  0x7D0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_245_REG  0x7D4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_246_REG  0x7D8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_247_REG  0x7DC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_248_REG  0x7E0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_249_REG  0x7E4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_250_REG  0x7E8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_251_REG  0x7EC /* pattern */                                    
#define SLAM_BRIEF_PATTERN_252_REG  0x7F0 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_253_REG  0x7F4 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_254_REG  0x7F8 /* pattern */                                    
#define SLAM_BRIEF_PATTERN_255_REG  0x7FC /* pattern */                                    
#define SLAM_DEBUG_INFO_REG         0x900 

#endif // __SLAM_REG_OFFSET_H__
