/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo.c
 *
 * driver for pmic dcxo calibration
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

#include <linux/mfd/hisi_pmic_dcxo.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/mtd/hisi_nve_number.h>
#include "../../mtd/hisi_nve.h"
#include <linux/module.h>
#include <linux/printk.h>
#include <securec.h>

unsigned int pmic_reg_read(
	int mode_cali, int modem_cali_in_mmw_pmu, int addr)
{
	if (mode_cali == CP_CALI) {
		if (modem_cali_in_mmw_pmu)
			return mmw_pmic_reg_read(addr);
		else
			return pmic_read_reg(addr);
	} else {
		return pmic_read_reg(addr);
	}
}

void pmic_reg_write(
	int mode_cali, int modem_cali_in_mmw_pmu, int addr, int val)
{
	if (mode_cali == CP_CALI) {
		if (modem_cali_in_mmw_pmu)
			mmw_pmic_reg_write(addr, val);
		else
			pmic_write_reg(addr, val);
	} else {
		pmic_write_reg(addr, val);
	}
}

void pmic_reg_write_mask(
	int mode_cali, int modem_cali_in_mmw_pmu,
	u32 addr, u32 value, u32 mask)
{
	uint32_t reg_tmp;

	reg_tmp = pmic_reg_read(mode_cali, modem_cali_in_mmw_pmu, addr);
	reg_tmp &= ~mask;
	reg_tmp |= value;
	pmic_reg_write(mode_cali, modem_cali_in_mmw_pmu, addr, reg_tmp);
}

int pmu_dcxo_get_set_nv(uint16_t *dcxo_ctrim,
	uint16_t *dcxo_c2_fix, bool get, struct pmic_nve_cali *nve_cali)
{
	int ret;
	struct pmu_dcxo *dcxo = NULL;
	struct hisi_nve_info_user nve = {0};
	errno_t err;

	if (!dcxo_ctrim || !dcxo_c2_fix || !nve_cali)
		return -EFAULT;

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
			nve_cali->nve_name, sizeof(nve_cali->nve_name) - 1);
	if (err != EOK) {
		pr_err("[%s]nve.nv_name strncpy_s failed\n", __func__);
		ret = -1;
		return ret;
	}
	nve.nv_number = nve_cali->nve_num;
	nve.valid_size = sizeof(*dcxo);
	nve.nv_operation = NV_READ;

	ret = hisi_nve_direct_access(&nve);
	if (ret) {
		pr_err("[%s]nve get dcxo failed: 0x%x\n", __func__, ret);
		return ret;
	}

	dcxo = (struct pmu_dcxo *)nve.nv_data;

	if (get) {
		*dcxo_ctrim = dcxo->dcxo_ctrim;
		*dcxo_c2_fix = dcxo->dcxo_c2_fix;
		pr_err("[%s]get dcxo ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
			__func__, *dcxo_ctrim, *dcxo_c2_fix);
		return ret;
	}

	dcxo->calibra_times++;
	dcxo->dcxo_ctrim = *dcxo_ctrim;
	dcxo->dcxo_c2_fix = *dcxo_c2_fix;

	nve.nv_operation = NV_WRITE;

	ret = hisi_nve_direct_access(&nve);
	if (ret)
		pr_err("[%s]nve set dcxo failed: 0x%x\n", __func__, ret);
	else
		pr_err("[%s]%u times setting dcxo\n",
			__func__, dcxo->calibra_times);

	return ret;
}

static void pmu_dcxo_reg_set_c1(int mode_cali,
	struct pmic_cali_c1c2 *cali_parameter, uint16_t *dcxo_ctrim)
{
	uint32_t value;
	uint16_t dcxo_c1;
	uint32_t i;

	value = pmic_reg_read(mode_cali, cali_parameter->modem_cali_in_mmw_pmu,
			cali_parameter->c1_reg);
	dcxo_c1 = *dcxo_ctrim;
	if (value < dcxo_c1) {
		for (i = value + 1; i <= dcxo_c1; i++)
			pmic_reg_write(mode_cali,
				cali_parameter->modem_cali_in_mmw_pmu,
				cali_parameter->c1_reg, i);
	} else if (value > dcxo_c1) {
		for (i = value - 1; i >= dcxo_c1; i--) {
			pmic_reg_write(mode_cali,
				cali_parameter->modem_cali_in_mmw_pmu,
				cali_parameter->c1_reg, i);
			if (i == 0)
				break;
		}
	}

	value = pmic_reg_read(mode_cali,
			cali_parameter->modem_cali_in_mmw_pmu,
			cali_parameter->c1_reg);

	pr_err("%s, pmu reread dcxo c1 = 0x%x\n", __func__, value);
}

static void pmu_dcxo_reg_set_c2(int mode_cali,
	struct pmic_cali_c1c2 *cali_parameter, uint16_t *dcxo_ctrim,
	uint16_t *dcxo_c2_fix)
{
	uint32_t i;
	uint32_t value;
	uint16_t dcxo_c2;

	value = pmic_reg_read(mode_cali,
			cali_parameter->modem_cali_in_mmw_pmu,
			cali_parameter->c2_reg);
	value = value & cali_parameter->c2_mask;
	dcxo_c2 = *dcxo_c2_fix;

	if (value < dcxo_c2) {
		for (i = value + 1; i <= dcxo_c2; i++)
			pmic_reg_write_mask(mode_cali,
				cali_parameter->modem_cali_in_mmw_pmu,
				cali_parameter->c2_reg,
				i, cali_parameter->c2_mask);
	} else if (value > dcxo_c2) {
		for (i = value - 1; i >= dcxo_c2; i--) {
			pmic_reg_write_mask(mode_cali,
				cali_parameter->modem_cali_in_mmw_pmu,
				cali_parameter->c2_reg,
				i, cali_parameter->c2_mask);
			if (i == 0)
				break;
		}
	}
	value = pmic_reg_read(mode_cali,
			cali_parameter->modem_cali_in_mmw_pmu,
			cali_parameter->c2_reg);
	value = value & cali_parameter->c2_mask;

	pr_err("%s, pmu reread dcxo c2 = 0x%x\n", __func__, value);
}

int pmu_dcxo_reg_get_set_c1c2(int mode_cali,
	struct pmic_cali_c1c2 *cali_parameter, uint16_t *dcxo_ctrim,
	uint16_t *dcxo_c2_fix, bool get)
{
	uint32_t value;

	if (!dcxo_ctrim || !dcxo_c2_fix || !cali_parameter)
		return -EFAULT;

	if (get) {
		value = pmic_reg_read(mode_cali,
				cali_parameter->modem_cali_in_mmw_pmu,
				cali_parameter->c1_reg);
		*dcxo_ctrim = (uint16_t)value;
		value = pmic_reg_read(mode_cali,
				cali_parameter->modem_cali_in_mmw_pmu,
				cali_parameter->c2_reg);
		value = value & cali_parameter->c2_mask;
		*dcxo_c2_fix = (uint16_t)value;
		return 0;
	}

	/* PMU_DCXO_CFIX1 */
	pmu_dcxo_reg_set_c1(mode_cali, cali_parameter, dcxo_ctrim);

	/* PMU_DCXO_CFIX2 */
	pmu_dcxo_reg_set_c2(mode_cali, cali_parameter, dcxo_ctrim, dcxo_c2_fix);

	return 0;
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pmic dcxo module");
