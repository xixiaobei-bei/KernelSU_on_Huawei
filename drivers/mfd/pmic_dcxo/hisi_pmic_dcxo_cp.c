/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo_cp.c
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

#include <linux/mfd/hisi_pmic_dcxo.h>
#include "hisi_pmic_dcxo_cp.h"
#include <linux/mfd/hisi_pmic.h>
#include <linux/mtd/hisi_nve_number.h>
#include "../../mtd/hisi_nve.h"
#include <linux/module.h>
#include <linux/printk.h>
#include <securec.h>

static u32 c1_reg;
static u32 c2_reg;
static u32 c2_mask;
static u32 modem_cali_in_mmw_pmu;

static int pmu_dcxo_get_set(
	uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get)
{
	int ret;
	struct pmic_nve_cali nve_cali;
	errno_t err;

	err = strncpy_s(nve_cali.nve_name, sizeof(nve_cali.nve_name),
		NVE_CALC1C2_NAME, sizeof(NVE_CALC1C2_NAME) - 1);
	if (err != EOK) {
		pr_err("[%s]nve.nv_name strncpy_s failed\n", __func__);
		return err;
	}

	nve_cali.nve_num = NVE_CALC1C2_NUM;
	ret = pmu_dcxo_get_set_nv(dcxo_ctrim, dcxo_c2_fix, get, &nve_cali);
	if (ret)
		pr_err("[%s]%d pmu_dcxo_get_set_nv error\n", __func__, get);
	return ret;
}


static int pmu_dcxo_reg_get_set(
	uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get)
{
	int ret;
	struct pmic_cali_c1c2 cali_parameter;

	cali_parameter.c1_reg = c1_reg;
	cali_parameter.c2_reg = c2_reg;
	cali_parameter.c2_mask = c2_mask;
	cali_parameter.modem_cali_in_mmw_pmu = modem_cali_in_mmw_pmu;

	ret = pmu_dcxo_reg_get_set_c1c2(CP_CALI, &cali_parameter,
		dcxo_ctrim, dcxo_c2_fix, get);
	if (ret)
		pr_err("[%s]error, %d\n", __func__, get);

	return ret;
}

int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix)
{
	pr_err("[%s]set dcxo ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
		__func__, dcxo_ctrim, dcxo_c2_fix);

	return pmu_dcxo_get_set(&dcxo_ctrim, &dcxo_c2_fix, false);
}
EXPORT_SYMBOL(pmu_dcxo_set);

int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix)
{
	return pmu_dcxo_get_set(dcxo_ctrim, dcxo_c2_fix, true);
}
EXPORT_SYMBOL(pmu_dcxo_get);

int pmu_dcxo_reg_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix)
{
	pr_err("[%s]set dcxo reg ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
		__func__, dcxo_ctrim, dcxo_c2_fix);
	return pmu_dcxo_reg_get_set(&dcxo_ctrim, &dcxo_c2_fix, false);
}
EXPORT_SYMBOL(pmu_dcxo_reg_set);

int pmu_dcxo_reg_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix)
{
	return pmu_dcxo_reg_get_set(dcxo_ctrim, dcxo_c2_fix, true);
}
EXPORT_SYMBOL(pmu_dcxo_reg_get);

#ifdef CONFIG_HISI_PMIC_DEBUG
int pmu_dcxo_get_test(int get, uint16_t v1, uint16_t v2)
{
	int ret;
	uint16_t dcxo_ctrim = v1;
	uint16_t dcxo_c2_fix = v2;

	ret = pmu_dcxo_get_set(&dcxo_ctrim, &dcxo_c2_fix, get);

	pr_err("[%s]test get trim value, ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
		__func__, dcxo_ctrim, dcxo_c2_fix);

	return ret;
}
#endif
static int __init pmu_dcxo_cp_cali_init(void)
{
	struct device_node *np = NULL;
	const char *status = NULL;
	int statlen = 0;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_dcxo_cp");
	if (np != NULL) {
		status = of_get_property(np, "status", &statlen);
		if (status == NULL) {
			pr_err("[%s]of_get_property status err\n", __func__);
			return -EINVAL;
		}
		if (strstr(status, "disabled"))
			goto use_default;
	} else {
		goto use_default;
	}

	modem_cali_in_mmw_pmu = 1;
	c1_reg = PMIC_MMW_DCXO_CFIX1;
	c2_reg = PMIC_MMW_DCXO_CFIX2;
	c2_mask = PMIC_MMW_DCXO_CFIX2_MASK;
	pr_info("[%s] in mmw pmu: c1 reg 0x%x, c2 reg 0x%x, c2 mask 0x%x\n",
		__func__, c1_reg, c2_reg, c2_mask);

	return 0;

use_default:
	c1_reg = PMIC_DCXO_CFIX1;
	c2_reg = PMIC_DCXO_CFIX2;
	c2_mask = PMIC_DCXO_CFIX2_MASK;
	pr_info("[%s]use_default: c1 reg 0x%x, c2 reg 0x%x, c2 mask 0x%x\n",
		__func__, c1_reg, c2_reg, c2_mask);

	return 0;
}

late_initcall_sync(pmu_dcxo_cp_cali_init);
