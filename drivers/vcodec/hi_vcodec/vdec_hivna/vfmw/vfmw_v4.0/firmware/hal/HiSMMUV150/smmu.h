/*
 * smmu.h
 *
 * This is for vdec driver for scd master.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HIVDEC_SMMU_H__
#define __HIVDEC_SMMU_H__
// for VDM_REG_PHY_ADDR, SCD_REG_PHY_ADDR, BPD_REG_PHY_ADDR
#include "sysconfig.h"
#include "vfmw.h"

#define SMMU_OK     0
#define SMMU_ERR   (-1)

#define SECURE_ON   1
#define SECURE_OFF  0
#define SMMU_ON     1
#define SMMU_OFF    0

#define VDH_MASTER_BW 1
#define VDH_MASTER_BS 20
#define COMMON_REG_ADDR_OFFSET 4
#define COMMON_REG_VAR 4
#define COMMON_REG_BW 4
#define COMMON_REG_BS 4

typedef enum {
	MFDE = 0,
	SCD,
} smmu_master_type;

SINT32 smmu_init(void);
void smmu_deinit(void);
void smmu_set_master_reg(
	smmu_master_type master_type, UINT8 secure_en,
	UINT8 mmu_en);
#ifdef PLATFORM_HIVCODECV200
void smmu_set_mem_ctl_reg(void);
#endif
void smmu_init_global_reg(void);
void smmu_int_serv_proc(void);

#endif
