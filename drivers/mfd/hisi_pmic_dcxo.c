/*
 * All other module's reference of nve.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/mtd/hisi_nve_number.h>
#include "../mtd/hisi_nve.h"
#include "securec.h"
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/mfd/hisi_pmic_dcxo.h>

EXPORT_SYMBOL(pmu_dcxo_get);
EXPORT_SYMBOL(pmu_dcxo_set);
EXPORT_SYMBOL(pmu_dcxo_reg_get);
EXPORT_SYMBOL(pmu_dcxo_reg_set);

#ifndef CONFIG_HISI_PMIC_DCXO
int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix){ return 0;}
int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix){ return 0;}
int pmu_dcxo_reg_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix){ return 0;}
int pmu_dcxo_reg_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix){ return 0;}
#else

struct pmu_dcxo {
	uint16_t dcxo_ctrim;
	uint16_t dcxo_c2_fix;
	uint16_t calibra_times;
};

static int pmu_dcxo_get_set(
	uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get)
{
	int ret;
	struct pmu_dcxo *dcxo;
	struct hisi_nve_info_user nve = {0};
	errno_t err = EOK;

	if (!dcxo_ctrim || !dcxo_c2_fix)
		return -EFAULT;

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
			NVE_CALC1C2_NAME, sizeof(nve.nv_name));
	if (EOK != err) {
		pr_err("[%s]nve.nv_name strncpy_s failed\n", __FUNCTION__);
	}
	nve.nv_number = NVE_CALC1C2_NUM;
	nve.valid_size = sizeof(struct pmu_dcxo);
	nve.nv_operation = NV_READ;

	ret = hisi_nve_direct_access(&nve);
	if (ret) {
		pr_err("[%s]nve get dcxo failed: 0x%x\n", __FUNCTION__, ret);
		return ret;
	}

	dcxo = (struct pmu_dcxo *)nve.nv_data;

	if (get) {
		*dcxo_ctrim = dcxo->dcxo_ctrim;
		*dcxo_c2_fix = dcxo->dcxo_c2_fix;
		pr_err("[%s]get dcxo ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
			__FUNCTION__, *dcxo_ctrim, *dcxo_c2_fix);
		return ret;
	}

	dcxo->calibra_times++;
	dcxo->dcxo_ctrim = *dcxo_ctrim;
	dcxo->dcxo_c2_fix = *dcxo_c2_fix;

	nve.nv_operation = NV_WRITE;

	ret = hisi_nve_direct_access(&nve);
	if (ret)
		pr_err("[%s]nve set dcxo failed: 0x%x\n", __FUNCTION__, ret);
	else
		pr_err("[%s]%d times setting dcxo\n", __FUNCTION__, dcxo->calibra_times);

	return ret;
}

void pmic_reg_write_mask(u32 addr, u32 value, u32 mask)
{
	uint32_t reg_tmp = 0;

	reg_tmp = hisi_pmic_reg_read(addr);
	reg_tmp &= ~mask;
	reg_tmp |= value;
	hisi_pmic_reg_write(addr, reg_tmp);
}

static int pmu_dcxo_reg_get_set(
	uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix, bool get)
{
	uint32_t i;
	uint32_t value;
	uint16_t dcxo_c1;
	uint16_t dcxo_c2;

	if (!dcxo_ctrim || !dcxo_c2_fix)
		return -EFAULT;

	if (get) {
		value = hisi_pmic_reg_read(PMIC_DCXO_CFIX1);
		*dcxo_ctrim = (uint16_t)value;
		value = hisi_pmic_reg_read(PMIC_DCXO_CFIX2);
		value = value & PMIC_DCXO_CFIX2_MASK;
		*dcxo_c2_fix = (uint16_t)value;
		return 0;
	}

	/* HISI_PMU_DCXO_CFIX1 */
	value = hisi_pmic_reg_read(PMIC_DCXO_CFIX1);
	dcxo_c1 = *dcxo_ctrim;
	if (value < dcxo_c1) {
		for (i = value + 1; i <= dcxo_c1; i++) {
			hisi_pmic_reg_write(PMIC_DCXO_CFIX1, i);
		}
	} else if (value > dcxo_c1) {
		for (i = value - 1; i >= dcxo_c1; i--) {
			hisi_pmic_reg_write(PMIC_DCXO_CFIX1, i);
			if (i == 0)
				break;
		}
	}

	value = hisi_pmic_reg_read(PMIC_DCXO_CFIX1);
	pr_err("%s, pmu reread dcxo c1 = 0x%x\n", __func__, value);

	/* HISI_PMU_DCXO_CFIX2 */
	value = hisi_pmic_reg_read(PMIC_DCXO_CFIX2);
	value = value & PMIC_DCXO_CFIX2_MASK;
	dcxo_c2 = *dcxo_c2_fix;

	if (value < dcxo_c2) {
		for (i = value + 1; i <= dcxo_c2; i++) {
			pmic_reg_write_mask(PMIC_DCXO_CFIX2, i,
				PMIC_DCXO_CFIX2_MASK);
		}
	} else if (value > dcxo_c2) {
		for (i = value - 1; i >= dcxo_c2; i--) {
			pmic_reg_write_mask(PMIC_DCXO_CFIX2, i,
				PMIC_DCXO_CFIX2_MASK);
			if (i == 0)
				break;
		}
	}
	value = hisi_pmic_reg_read(PMIC_DCXO_CFIX2);
	value = value & PMIC_DCXO_CFIX2_MASK;
	pr_err("%s, pmu reread dcxo c2 = 0x%x\n", __func__, value);

	return 0;
}

int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix)
{
	pr_err("[%s]set dcxo ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
		__FUNCTION__, dcxo_ctrim, dcxo_c2_fix);

	return pmu_dcxo_get_set(&dcxo_ctrim, &dcxo_c2_fix, false);
}

int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix)
{
	return pmu_dcxo_get_set(dcxo_ctrim, dcxo_c2_fix, true);
}

int pmu_dcxo_reg_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix)
{
	pr_err("[%s]set dcxo reg ctrim = 0x%x, dcxo_c2_fix = 0x%x\n",
		__FUNCTION__, dcxo_ctrim, dcxo_c2_fix);
	return pmu_dcxo_reg_get_set(&dcxo_ctrim, &dcxo_c2_fix, false);
}

int pmu_dcxo_reg_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix)
{
	return pmu_dcxo_reg_get_set(dcxo_ctrim, dcxo_c2_fix, true);
}

int pmu_dcxo_get_test(int get, uint16_t v1, uint16_t v2)
{
	int ret;
	uint16_t dcxo_ctrim = v1;
	uint16_t dcxo_c2_fix = v2;

	ret = pmu_dcxo_get_set(&dcxo_ctrim, &dcxo_c2_fix, get);

	pr_err("[%s]test get trim value, ctrim = 0x%x, dcxo_c2_fix = "
	       "0x%x\n",
		__FUNCTION__, dcxo_ctrim, dcxo_c2_fix);

	return ret;
}

#endif
