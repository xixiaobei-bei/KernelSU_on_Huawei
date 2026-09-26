//******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co., Ltd.
// File name     :  compare_reg_offset.h
// Author        :  HerveDANIEL
// Version       :  1.0
// Date          :  2018-08-09
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0
// History       :  HerveDANIEL 2018-08-09 Create file
//******************************************************************************

#ifndef __COMPARE_REG_OFFSET_CS_H__
#define __COMPARE_REG_OFFSET_CS_H__


/******************************************************************************/
/*                      HISTARISP COMPARE Registers' Definitions                            */
/******************************************************************************/

#define COMPARE_COMPARE_CFG_REG       0x0    /* Compare Config */
#define COMPARE_BLOCK_CFG_REG         0x4    /* BLOCK Config */
#define COMPARE_SEARCH_CFG_REG        0x8    /* Search */
#define COMPARE_STAT_CFG_REG          0xC    /* Statistic */
#define COMPARE_PREFETCH_CFG_REG      0x10   /* Configure the prefetch */
#define COMPARE_DEBUG_0_REG           0x20   /* Debug 0 */
#define COMPARE_DEBUG_1_REG           0x24   /* Debug 1 */
#define COMPARE_EC_0_REG              0x28   /* ec 0 */
#define COMPARE_EC_1_REG              0x2C   /* ec 1 */
#define COMPARE_REF_KPT_NUMBER_0_REG  0x200  /* Reference feature number in each block */
#define COMPARE_CUR_KPT_NUMBER_0_REG  0x380  /* Current feature number in each block */
#define COMPARE_INDEX_0_REG           0x500  /* feature offset for each block */
#endif // __COMPARE_REG_OFFSET_CS_H__
