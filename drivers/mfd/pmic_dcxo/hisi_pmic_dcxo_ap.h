/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo_ap.h
 *
 * driver for pmic dcxo ap calibration
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

#ifndef	__HISI_PMIC_DCXO_AP_H
#define	__HISI_PMIC_DCXO_AP_H

#include <pmic_interface.h>
#include <linux/types.h>

#define NVE_CALC1C2_AP_NAME "APDCXO"
#define DCXO_MAX_DEVICES 5

#define PMIC_DCXO_CFIX1  PMIC_NP_CLK_TOP_CTRL1_ADDR(0)
#define PMIC_DCXO_CFIX2  PMIC_NP_CLK_TOP_CTRL0_ADDR(0)
#define PMIC_DCXO_CFIX2_MASK    0xF

#define PMIC_DCXO_CLK_EN  PMIC_CLK_SYS_EN_ADDR(0)
#define PMIC_DCXO_CLK_EN_SHIFT PMIC_CLK_SYS_EN_reg_xo_sys_en_START
#define PMIC_DCXO_CLK_EN_MASK (1 << PMIC_CLK_SYS_EN_reg_xo_sys_en_START)

#define PMIC_DCXO_CALI_RC_EN PMIC_NP_BACKUP_CHG_ADDR(0)
#define PMIC_DCXO_CALI_RC_EN_SHIFT PMIC_NP_BACKUP_CHG_np_chg_bypass_START
#define PMIC_DCXO_CALI_RC_EN_MASK (1 << PMIC_NP_BACKUP_CHG_np_chg_bypass_START)

#define PMIC_DCXO_RF0_CLK_EN PMIC_CLK_RF0_EN_ADDR(0)
#define PMIC_DCXO_RF0_CLK_EN_SHIFT 0
#define PMIC_DCXO_RF0_CLK_EN_MASK (1 << PMIC_DCXO_RF0_CLK_EN_SHIFT)

#define PMIC_DCXO_RF1_CLK_EN PMIC_CLK_RF1_EN_ADDR(0)
#define PMIC_DCXO_RF1_CLK_EN_SHIFT 0
#define PMIC_DCXO_RF1_CLK_EN_MASK (1 << PMIC_DCXO_RF0_CLK_EN_SHIFT)

#define PMU_DCXO_GET_CMD 0x10
#define PMU_DCXO_SET_CMD 0x20
#define HKADC_XOADC "dcxo0"

#define DCXO_CTRIM_MAX 255
#define DCXO_CTRIM_MIN 0
#define DCXO_C2_FIX_MAX 15
#define DCXO_C2_FIX_MIN 0

#define DEFAULT_TEMP 20000 /* 20C */

struct pmic_dcxo_clk {
	u32    clk_en;
	u32    clk_shift;
	u32    clk_mask;
};

struct pmic_dcxo_device {
	struct device *dev;
	int     dcxoid_major;
	struct class *dcxoid_class;
	struct pmic_dcxo_clk dcxo_clk;
};
enum  pmu_dcxo_msg_type {
	PMU_DCXO_CLK_EN = 0,     /* serdes_clk_38M4 clk enable */
	PMU_DCXO_CLK_DIS,        /* serdes_clk_38M4 clk disable */
	PMU_DCXO_CLK_STATUS_GET, /* serdes_clk_38M4 clk status */
	PMU_DCXO_TEMP_GET,       /* get xoadc temp */
	PMU_DCXO_CALI_REG_SET,   /* set cali_reg */
	PMU_DCXO_CALI_REG_GET,   /* get cali_reg */
	PMU_DCXO_CALI_NV_SET,    /* set cali value to nv */
	PMU_DCXO_CALI_NV_GET,    /* get cali value to nv */
	PMU_DCXO_CALI_RC_EN,     /* RC cali enable */
	PMU_DCXO_CALI_RC_DIS,    /* RC cali disable */
};
struct  pmu_dcxo_cali_val {
	uint16_t  dcxo_ctrim; /* c1 */
	uint16_t  dcxo_c2_fix; /* c2 */
};

struct pmu_dcxo_cali_msg {
	enum  pmu_dcxo_msg_type  msg_type;
	short msg_len;
	struct  pmu_dcxo_cali_val  msg_data;
	int get_val;
};

#endif
