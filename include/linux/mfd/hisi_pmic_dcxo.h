/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo.h
 *
 * driver for pmic dcxo calibration
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#ifndef	__HISI_PMIC_DCXO_H
#define	__HISI_PMIC_DCXO_H

#include <linux/types.h>
#include <linux/mtd/hisi_nve_interface.h>

struct pmic_cali_c1c2 {
	u32 c1_reg;
	u32 c2_reg;
	u32 c2_mask;
	u32 modem_cali_in_mmw_pmu;
};

struct pmu_dcxo {
	uint16_t dcxo_ctrim;
	uint16_t dcxo_c2_fix;
	uint16_t calibra_times;
};

struct pmic_nve_cali {
	char nve_name[NV_NAME_LENGTH];
	uint32_t nve_num;
};

#define CP_CALI 0
#define AP_CALI 1

unsigned int pmic_reg_read(int mode_cali, int modem_cali_in_mmw_pmu, int addr);

void pmic_reg_write_mask(int mode_cali, int modem_cali_in_mmw_pmu, u32 addr,
	u32 value, u32 mask);

int pmu_dcxo_reg_get_set_c1c2(int mode_cali,
	struct pmic_cali_c1c2 *cali_parameter, uint16_t *dcxo_ctrim,
	uint16_t *dcxo_c2_fix, bool get);

int pmu_dcxo_get_set_nv(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get,
	struct pmic_nve_cali *nve_cali);

#ifdef CONFIG_HISI_PMIC_DCXO
int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix);
int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix);
int pmu_dcxo_reg_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix);
int pmu_dcxo_reg_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix);
#else
static inline int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix)
{
	return 0;
}
static inline int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix)
{
	return 0;
}
static inline int pmu_dcxo_reg_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix)
{
	return 0;
}
static inline int pmu_dcxo_reg_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix)
{
	return 0;
}
#endif

#endif
