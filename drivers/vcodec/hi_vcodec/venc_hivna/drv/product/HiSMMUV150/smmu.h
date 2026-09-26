/*
 * smmu.h
 *
 * This is for smmu description.
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

#include "hi_type.h"
#include "drv_venc_ioctl.h"

#define SMMU_OK     0
#define SMMU_ERR   (-1)

#define SECURE_ON    1
#define SECURE_OFF   0
#define SMMU_ON      1
#define SMMU_OFF     0

enum {
	REFLD_LUMA = 0,
	REFLD_CHROMA,
	REFLD_H_LUMA,
	REFLD_H_CHROMA,
	VCPI_LD,
	QPG_LD_QP,
	QPG_LD_CELL,
	VLC_LD_PTR_WR,
	VLC_LD_PTR_RD,
	NBI_LD,
	PMV_LD_DATA,
	PMV_LD_CELL,
	LOW_DELAY,
	CURLD_Y,
	CURLD_U,
	CURLD_V,
	CURLD_YH,
	CURLD_CH,
	PMEINFO_LD_FLAG,
	PMEINFO_LD_QPG,
	PMEINFO_LD_SKIP_WGT,
	PME_LD,
	REC_ST_LUMA,
	REC_ST_CHROMA,
	REC_ST_CELL,
	PMV_ST_DATA,
	PMV_ST_CELL,
	QPG_ST_QP,
	QPG_ST_CELL,
	VLC_ST_PTR_WR,
	VLC_ST_PTR_RD,
	VLC_ST_STEAM,
	PMEINFO_ST_FLAG,
	NBI_ST,
	PME_ST,
	REC_ST_H_LUMA,
	REC_ST_H_CHROMA,
};

int32_t venc_smmu_init(bool is_protected, int32_t core_id);
int32_t venc_smmu_cfg(struct encode_info *channelcfg, uint32_t *reg_base);
void venc_smmu_debug(uint32_t *reg_base, bool first_cfg_flag);
#endif
