/*
 * smmu_cfg.h
 *
 * provide interface for config smmu
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _INCLUDE_SMMU_CFG_H
#define _INCLUDE_SMMU_CFG_H

#include <linux/iommu.h>
#include <asm/io.h>
#include "hjpeg_common.h"

#define CONFIG_SMMU_RWERRADDR 1

extern int hjpeg_smmu_config(hjpeg_hw_ctl_t *hw_ctl, void* fama_pgd);
extern int is_hjpeg_qos_update(void);

#define SMMU_RW_ERR_ADDR_SIZE (128)

#define SMMU_SMRX_NS(n) (0x0020+n*0x4)
#define SMMU_SMRX_S(n)  (0x0500+n*0x4)
#define SMMU_SMRX_P(n)  (0x10000+n*0x4)

#define SMMU_GLOBAL_BYPASS                   0x00

#define SMMU_SCRS                            0x710

#define SMMU_INTMASK_NS             0x10
#define SMMU_INTCLR_NS              0x1C

#define SMMU_CB_TTBR0               0x204
#define SMMU_CB_TTBCR               0x20C
#define SMMU_FAMA_CTRL0_NS          0x220
#define SMMU_FAMA_CTRL1_NS          0x224
#define SMMU_ADDR_MSB               0x300
#define SMMU_ERR_RDADDR             0x304
#define SMMU_ERR_WRADDR             0x308


#define FAMA_CTRL0_NS_fama_bps_msb_ns_LEN       6
#define FAMA_CTRL0_NS_fama_bps_msb_ns_OFFSET    8
#define FAMA_CTRL0_NS_fama_chn_sel_ns_LEN       1
#define FAMA_CTRL0_NS_fama_chn_sel_ns_OFFSET    7
#define FAMA_CTRL0_NS_fama_sdes_msb_ns_LEN      7
#define FAMA_CTRL0_NS_fama_sdes_msb_ns_OFFSET   0

#define FAMA_CTRL1_NS_fama_ptw_msb_ns_LEN       7
#define FAMA_CTRL1_NS_fama_ptw_msb_ns_OFFSET    0

/* Define the union U_SMMU_SMMU_SCR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t glb_bypass : 1 ; /* [0] */
        uint32_t rqos_en : 1 ; /* [1] */
        uint32_t wqos_en : 1 ; /* [2] */
        uint32_t reserved_0 : 1 ; /* [3] */
        uint32_t cache_l1_en : 1 ; /* [4] */
        uint32_t cache_l2_en : 1 ; /* [5] */
        uint32_t rqos : 4 ; /* [9..6] */
        uint32_t wqos : 4 ; /* [13..10] */
        uint32_t reserved_1 : 1 ; /* [14] */
        uint32_t smr_rd_shadow : 1 ; /* [15] */
        uint32_t ptw_pf : 4 ; /* [19..16] */
        uint32_t ptw_mid : 6 ; /* [25..20] */
        uint32_t reserved_2 : 6 ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;

} U_SMMU_SMMU_SCR;

/* Define the union U_SMMU_SMMU_ADDR_MSB */
typedef union
{
    /* Define the struct bits */
    struct
    {
        uint32_t msb_errrd : 7 ; /* [6..0] */
        uint32_t msb_errwr : 7 ; /* [13..7] */
        uint32_t msb_ova : 7 ; /* [20..14] */
    } bits;

    /* Define an unsigned member */
    uint32_t reg32;

} U_SMMU_SMMU_ADDR_MSB;


#endif  // _INCLUDE_HJPEG_COMMON_H
