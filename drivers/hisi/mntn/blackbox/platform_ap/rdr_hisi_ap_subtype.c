/*
 *
 * AP reset exception subtype function implementation
 *
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/io.h>
#include <securec.h>

#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <mntn_subtype_exception.h>
#include "../rdr_inner.h"
#include "../rdr_print.h"
#include "rdr_hisi_ap_adapter.h"
#include <pr_log.h>
#define PR_LOG_TAG BLACKBOX_TAG
#define RDR_CATEGORY_TYPE "UNDEF"

static char g_subtype_name[RDR_REBOOT_REASON_LEN] = "undef";
static u32 g_subtype;

/* subtype exception map */
#undef __MMC_EXCEPTION_SUBTYPE_MAP
#define __MMC_EXCEPTION_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __AP_DDRC_SEC_SUBTYPE_MAP
#define __AP_DDRC_SEC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __AP_PANIC_SUBTYPE_MAP
#define __AP_PANIC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __AP_BL31PANIC_SUBTYPE_MAP
#define __AP_BL31PANIC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __AP_VENDOR_PANIC_SUBTYPE_MAP
#define __AP_VENDOR_PANIC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __APWDT_HWEXC_SUBTYPE_MAP
#define __APWDT_HWEXC_SUBTYPE_MAP(x, y, z) { x, #y":hw", #z, z },

#undef __APWDT_EXC_SUBTYPE_MAP
#define __APWDT_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __LPM3_EXC_SUBTYPE_MAP
#define __LPM3_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __LPCPU_EXC_SUBTYPE_MAP
#define __LPCPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __LPGPU_EXC_SUBTYPE_MAP
#define __LPGPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __MEM_REPAIR_EXC_SUBTYPE_MAP
#define __MEM_REPAIR_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __SCHARGER_EXC_SUBTYPE_MAP
#define __SCHARGER_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __PMU_EXC_SUBTYPE_MAP
#define __PMU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __NPU_EXC_SUBTYPE_MAP
#define __NPU_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __CONN_EXC_SUBTYPE_MAP
#define __CONN_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __HISEE_EXC_SUBTYPE_MAP
#define __HISEE_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __IVP_EXC_SUBTYPE_MAP
#define __IVP_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

#undef __DSS_EXC_SUBTYPE_MAP
#define __DSS_EXC_SUBTYPE_MAP(x, y, z) { x, #y, #z, z },

struct exp_subtype exp_subtype_map[] = {
	#include <mntn_subtype_exception_map.h>
};

#undef __MMC_EXCEPTION_SUBTYPE_MAP
#undef __AP_DDRC_SEC_SUBTYPE_MAP
#undef __AP_PANIC_SUBTYPE_MAP
#undef __AP_BL31PANIC_SUBTYPE_MAP
#undef __AP_VENDOR_PANIC_SUBTYPE_MAP
#undef __APWDT_HWEXC_SUBTYPE_MAP
#undef __APWDT_EXC_SUBTYPE_MAP
#undef __LPM3_EXC_SUBTYPE_MAP
#undef __LPCPU_EXC_SUBTYPE_MAP
#undef __LPGPU_EXC_SUBTYPE_MAP
#undef __MEM_REPAIR_EXC_SUBTYPE_MAP
#undef __SCHARGER_EXC_SUBTYPE_MAP
#undef __PMU_EXC_SUBTYPE_MAP
#undef __NPU_EXC_SUBTYPE_MAP
#undef __CONN_EXC_SUBTYPE_MAP
#undef __HISEE_EXC_SUBTYPE_MAP
#undef __IVP_EXC_SUBTYPE_MAP
#undef __DSS_EXC_SUBTYPE_MAP

static u32 get_exception_subtype_map_size(void)
{
	return ARRAY_SIZE(exp_subtype_map);
}

static u32 get_category_value(u32 e_exce_type)
{
	u32 i;
	u32 category = 0;
	const struct cmdword *reboot_reason_map = get_reboot_reason_map();

	if (!reboot_reason_map) {
		pr_err("[%s:%d]: reboot_reason_map is NULL\n", __func__, __LINE__);
		return 0;
	}

	for (i = 0; i < get_reboot_reason_map_size(); i++) {
		if (reboot_reason_map[i].num == e_exce_type) {
			category = reboot_reason_map[i].category_num;
			break;
		}
	}
	return category;
}

/*
 * Description:  get exception subtype name
 * Date:         2017/08/16
 * Modification: Created function
 */
char *rdr_get_subtype_name(u32 e_exce_type, u32 subtype)
{
	u32 i;

	if (get_category_value(e_exce_type) == SUBTYPE)
		for (i = 0; (unsigned int)i < get_exception_subtype_map_size(); i++) {
			if (exp_subtype_map[i].exception == e_exce_type &&
				exp_subtype_map[i].subtype_num == subtype)
				return (char *)exp_subtype_map[i].subtype_name;
		}

	return NULL;
}


/*
 * Description:  get category
 * Date:         2017/08/16
 * Modification: Created function
 */
char *rdr_get_category_name(u32 e_exce_type, u32 subtype)
{
	int i, category;
	const struct cmdword *reboot_reason_map = get_reboot_reason_map();

	if (!reboot_reason_map) {
		pr_err("[%s:%d]: reboot_reason_map is NULL\n", __func__, __LINE__);
		return NULL;
	}

	category = get_category_value(e_exce_type);
	if (category == SUBTYPE) {
		for (i = 0; (unsigned int)i < get_exception_subtype_map_size(); i++) {
			if (exp_subtype_map[i].exception == e_exce_type &&
				exp_subtype_map[i].subtype_num == subtype) {
				return (char *)exp_subtype_map[i].category_name;
			}
		}
	} else {
		for (i = 0; (unsigned int)i < get_reboot_reason_map_size(); i++) {
			if (reboot_reason_map[i].num == e_exce_type)
				return (char *)reboot_reason_map[i].category_name;
		}
	}
	return RDR_CATEGORY_TYPE;
}


/*
 * Description:  set exception subtype to pmu
 * Date:         2017/08/16
 * Modification: Created function
 */
void set_subtype_exception(unsigned int subtype, bool save_value)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;

	if (g_bbox_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_subtype_reg();
		if (pmu_reset_reg)
			value = readl((char *)pmu_reset_reg);
	} else {
		value = pmic_read_reg(PMU_EXCSUBTYPE_REG_OFFSET);
	}
	if (save_value == false)
		value &= (PMU_RESET_REG_MASK);

	subtype &= (RST_FLAG_MASK);
	value |= subtype;
	if (g_bbox_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_subtype_reg();
		if (pmu_reset_reg)
			writel(value, (char *)pmu_reset_reg);
	} else {
		pmic_write_reg(PMU_EXCSUBTYPE_REG_OFFSET, value);
	}
	pr_info("[%s]:set subtype is 0x%x\n", __func__, value);
}

/*
 * Description:  get subtype exception
 * Date:         2017/08/16
 * Modification: Created function
 */
unsigned int get_subtype_exception(void)
{
	unsigned int value = 0;
	uintptr_t pmu_reset_reg;

	if (g_bbox_fpga_flag == FPGA) {
		pmu_reset_reg = get_pmu_reset_reg();
		if (pmu_reset_reg)
			value = readl((char *)pmu_reset_reg);
	} else {
		value = pmic_read_reg(PMU_EXCSUBTYPE_REG_OFFSET);
	}
	value &= RST_FLAG_MASK;
	return value;
}

char *rdr_get_exec_subtype(void)
{
	return g_subtype_name;
}

u32 rdr_get_exec_subtype_value(void)
{
	return g_subtype;
}
static int __init early_parse_exec_subtype_cmdline(char *exec_subtype_cmdline)
{
	int i;

	if (!exec_subtype_cmdline) {
		pr_err("[%s:%d]: param is NULL\n", __func__, __LINE__);
		return -1;
	}

	if (memset_s(g_subtype_name, RDR_REBOOT_REASON_LEN, 0x0, RDR_REBOOT_REASON_LEN) != EOK) {
		pr_err("[%s:%d]: memset_s err\n", __func__, __LINE__);
		return -1;
	}

	if (memcpy_s(g_subtype_name, RDR_REBOOT_REASON_LEN, exec_subtype_cmdline, RDR_REBOOT_REASON_LEN - 1)) {
		BB_PRINT_ERR("failed to memcpy_s exception subtype\n");
		return -1;
	}

	for (i = 0; (u32)i < get_exception_subtype_map_size(); i++) {
		if (!strncmp((char *)exp_subtype_map[i].subtype_name, g_subtype_name, RDR_REBOOT_REASON_LEN)) {
			g_subtype = exp_subtype_map[i].subtype_num;
			break;
		}
	}
	pr_info("[%s][%s][%u]\n", __func__, g_subtype_name, g_subtype);
	return 0;
}

early_param("exception_subtype", early_parse_exec_subtype_cmdline);

