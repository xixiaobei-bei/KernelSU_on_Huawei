/*
 * hisi_vbat_drop_protect_bal.h
 *
 * head file for vbat drop protect
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HISI_VBAT_DROP_PROTECT_BAL_H
#define __HISI_VBAT_DROP_PROTECT_BAL_H

#include <soc_corecrg_b_m_interface.h>
#include <soc_corecrg_l_l3_interface.h>
#include <soc_npucrg_interface.h>
#include <soc_gpucrg_interface.h>

/* pmic vsys drop reg */
#define PMIC_VSYS_DROP_VOL_SET			PMIC_NP_VSYS_DROP_SET_ADDR(0)
#define PMIC_VSYS_DROP_IRQ_MASK_REG		PMIC_IRQ_MASK_20_ADDR(0)
#define PMIC_VBAT_DROP_VOL_SET_REG		PMIC_NP_VSYS_DROP_SET_ADDR(0)

#define VBAT_DROP_TEST /* for get core auto div status */

/* core enable reg in pmctrl */
#define LITTLE_VOL_DROP_EN_ADDR(base)                                          \
	SOC_CORECRG_L_L3_VS_SVFD_CTRL2_LITTLE_ADDR(base)
#define MIDDLE_VOL_DROP_EN_ADDR(base)                                          \
	SOC_CORECRG_B_M_VS_SVFD_CTRL2_MIDDLE0_ADDR(base)
#define BIG_VOL_DROP_EN_ADDR(base) SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_ADDR(base)
#define L3_VOL_DROP_EN_ADDR(base)  SOC_CORECRG_L_L3_VS_SVFD_CTRL2_L3_ADDR(base)
#define GPU_VOL_DROP_EN_ADDR(base) SOC_GPUCRG_VS_SVFD_CTRL2_GPU_ADDR(base)
#define NPU_VOL_DROP_EN_ADDR(base) SOC_NPUCRG_VS_SVFD_CTRL2_NPU_ADDR(base)

#define LITTLE_VOL_DROP_EN_BIT                                                 \
	BIT(SOC_CORECRG_L_L3_VS_SVFD_CTRL2_LITTLE_vol_drop_en_little_START)
#define MIDDLE_VOL_DROP_EN_BIT                                                 \
	BIT(SOC_CORECRG_B_M_VS_SVFD_CTRL2_MIDDLE0_vol_drop_en_middle0_START)
#define BIG_VOL_DROP_EN_BIT                                                    \
	BIT(SOC_CORECRG_B_M_VS_SVFD_CTRL2_BIG0_vol_drop_en_big0_START)
#define L3_VOL_DROP_EN_BIT                                                     \
	BIT(SOC_CORECRG_L_L3_VS_SVFD_CTRL2_L3_vol_drop_en_l3_START)
#define GPU_VOL_DROP_EN_BIT                                                    \
	BIT(SOC_GPUCRG_VS_SVFD_CTRL2_GPU_vol_drop_en_gpu_START)

/* reg bit write en */
#define LITTLE_VOL_DROP_MASK (LITTLE_VOL_DROP_EN_BIT << 16)
#define MIDDLE_VOL_DROP_MASK (MIDDLE_VOL_DROP_EN_BIT << 16)
#define BIG_VOL_DROP_MASK (BIG_VOL_DROP_EN_BIT << 16)
#define L3_VOL_DROP_MASK (L3_VOL_DROP_EN_BIT << 16)
#define GPU_VOL_DROP_MASK (GPU_VOL_DROP_EN_BIT << 16)

#ifdef VBAT_DROP_TEST
/* core enable status register in pmctrl */
#define LITTLE_VOL_DROP_EN_STAT_ADDR(base)                                     \
	SOC_CORECRG_L_L3_VS_TEST_STAT_LITTLE_ADDR(base)
#define MIDDLE_VOL_DROP_EN_STAT_ADDR(base)                                     \
	SOC_CORECRG_B_M_VS_TEST_STAT_MIDDLE0_ADDR(base)
#define BIG_VOL_DROP_EN_STAT_ADDR(base)                                        \
	SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_ADDR(base)
#define L3_VOL_DROP_EN_STAT_ADDR(base)                                         \
	SOC_CORECRG_L_L3_VS_TEST_STAT_L3_ADDR(base)
#define GPU_VOL_DROP_EN_STAT_ADDR(base) SOC_GPUCRG_VS_TEST_STAT_GPU_ADDR(base)

#define LITTLE_VOL_DROP_EN_STAT_BIT                                            \
	BIT(SOC_CORECRG_L_L3_VS_TEST_STAT_LITTLE_vbat_drop_protect_ind_little_START)
#define MIDDLE_VOL_DROP_EN_STAT_BIT                                            \
	BIT(SOC_CORECRG_B_M_VS_TEST_STAT_MIDDLE0_vbat_drop_protect_ind_middle0_START)
#define BIG_VOL_DROP_EN_STAT_BIT                                               \
	BIT(SOC_CORECRG_B_M_VS_TEST_STAT_BIG0_vbat_drop_protect_ind_big0_START)
#define L3_VOL_DROP_EN_STAT_BIT                                                \
	BIT(SOC_CORECRG_L_L3_VS_TEST_STAT_L3_vbat_drop_protect_ind_l3_START)
#define GPU_VOL_DROP_EN_STAT_BIT                                               \
	BIT(SOC_GPUCRG_VS_TEST_STAT_GPU_vbat_drop_protect_ind_gpu_START)
#endif
#endif
