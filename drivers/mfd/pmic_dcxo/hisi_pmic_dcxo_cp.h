/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo_cp.h
 *
 * driver for pmic dcxo cp calibration
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef	__HISI_PMIC_CP_DCXO_H
#define	__HISI_PMIC_CP_DCXO_H

#include <pmic_interface.h>
#ifdef CONFIG_HISI_PMIC_AP_DCXO
#include <pmic_mmw_interface.h>
#endif
#include <linux/types.h>

#define NVE_CALC1C2_NAME "CALC1C2"
#define PMIC_DCXO_CFIX2_MASK 0xF

#ifdef PMIC_NP_CLK_TOP_CTRL1_ADDR
#define PMIC_DCXO_CFIX1  PMIC_NP_CLK_TOP_CTRL1_ADDR(0)
#define PMIC_DCXO_CFIX2  PMIC_NP_CLK_TOP_CTRL0_ADDR(0)
#else
#define PMIC_DCXO_CFIX1  PMIC_CLK_TOP_CTRL1_1_ADDR(0)
#define PMIC_DCXO_CFIX2  PMIC_CLK_TOP_CTRL1_0_ADDR(0)
#endif

#ifdef PMIC_MMW_CLK_TOP_CTRL1_1_ADDR
#define PMIC_MMW_DCXO_CFIX1  PMIC_MMW_CLK_TOP_CTRL1_1_ADDR(0)
#define PMIC_MMW_DCXO_CFIX2  PMIC_MMW_CLK_TOP_CTRL1_0_ADDR(0)
#define PMIC_MMW_DCXO_CFIX2_MASK 0xF
#else
#define PMIC_MMW_DCXO_CFIX1  0
#define PMIC_MMW_DCXO_CFIX2  0
#define PMIC_MMW_DCXO_CFIX2_MASK 0
#endif

#endif
