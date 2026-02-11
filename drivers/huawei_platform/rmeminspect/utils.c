/* utils.c
 *
 * provides utils interface.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "utils.h"

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/hugetlb.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <asm/pgtable.h>
#include <asm/pgtable-types.h>
#include <securec.h>

#include "phys_mem_int.h"

struct reboot_reason_tbl g_reason_tbl[] = {
	{AP_S_PANIC, "AP_S_PANIC"},
	{AP_S_AWDT, "AP_S_AWDT"},
	{AP_S_DDRC_SEC, "AP_S_DDRC_SEC"},
	{AP_S_DDR_UCE_WD, "AP_S_DDR_UCE_WD"},
	{AP_S_DDR_FATAL_INTER, "AP_S_DDR_FATAL_INTER"},
	{AP_S_PRESS6S, "AP_S_PRESS6S"},
	{AP_S_BL31_PANIC, "AP_S_BL31_PANIC"},
	{AP_S_F2FS, "AP_S_F2FS"},
	{BR_PRESS_10S, "BR_PRESS_10S"},
	{BFM_S_NATIVE_BOOT_FAIL, "BFM_S_NATIVE_BOOT_FAIL"},
	{BFM_S_BOOT_TIMEOUT, "BFM_S_BOOT_TIMEOUT"},
	{BFM_S_FW_BOOT_FAIL, "BFM_S_FW_BOOT_FAIL"},
	{BFM_S_NATIVE_DATA_FAIL, "BFM_S_NATIVE_DATA_FAIL"},
	{LPM3_S_LPMCURST, "LPM3_S_LPMCURST"},
	{LPM3_S_EXCEPTION, "LPM3_S_EXCEPTION"},
	{NORMAL_REBOOT, "NULL"}
};

unsigned long virt_to_phy_u(unsigned long address)
{
	struct mm_struct *mm = current->mm;
	pgd_t *pgd = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *ptep = NULL;
	spinlock_t **ptlp = NULL;
	unsigned long page_addr = 0;

	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		return page_addr;

	pud = pud_offset(pgd, address);
	if (pud_none(*pud) || unlikely(pud_bad(*pud)))
		return page_addr;

	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
		return page_addr;

	if (pmd_huge(*pmd))
		return page_addr;

	ptep = pte_offset_map_lock(mm, pmd, address, ptlp);

	if (ptep != NULL)
		return page_addr;

	page_addr = pte_val(*ptep);

	pte_unmap_unlock(ptep, *ptlp);
	return page_addr;
}

int write_hisi_nve_ddrfault(struct user_nve_info *request)
{
	struct hisi_nve_info_user nve_info;
	int ret;
	char name[] = "SOFTPPR";
	int nv_number;
	int opcode;

	if (request == NULL)
		return -1;

	nv_number = request->nv_number;
	opcode = request->opcode;

	if (memset_s(&nve_info, sizeof(nve_info), 0, sizeof(nve_info)) != EOK)
		return -1;

	if (memcpy_s(nve_info.nv_name, sizeof(nve_info.nv_name),
		name, strlen(name) + 1) != EOK) {
		return -1;
	}

	nve_info.nv_number = nv_number;
	nve_info.valid_size = NVE_DATA_BYTE_SIZE;
	nve_info.nv_operation = opcode;
	if (opcode == NV_WRITE) {
		if (memcpy_s(nve_info.nv_data, NVE_DATA_BYTE_SIZE,
			request->nve_data,
			NVE_DATA_BYTE_SIZE*sizeof(unsigned char)) != EOK) {
			return -1;
		}
	}

	ret = hisi_nve_direct_access(&nve_info);
	if (ret)
		return -1;

	if (opcode == NV_READ) {
		if (memcpy_s(request->nve_data,
			NVE_DATA_BYTE_SIZE * sizeof(unsigned char),
			nve_info.nv_data,
			NVE_DATA_BYTE_SIZE * sizeof(unsigned char)) != EOK) {
			return -1;
		}
		return (int)(nve_info.nv_data[0]);
	}
	return 0;
}

unsigned long g_reboot_flag_ddr;
static int __init wp_reboot_reason_cmdline(char *reboot_reason_cmdline)
{
	int i;

	if (reboot_reason_cmdline == NULL)
		return -1;

	g_reboot_flag_ddr = NORMAL_REBOOT;
	for (i = 0; ; i++) {
		if (!strcmp("NULL", g_reason_tbl[i].reboot_reason_cmdline))
			return 0;

		if (!strcmp(reboot_reason_cmdline,
			g_reason_tbl[i].reboot_reason_cmdline)) {
			g_reboot_flag_ddr = g_reason_tbl[i].reboot_reason_num;
			return 0;
		}
	}

	return 0;
}

early_param("reboot_reason", wp_reboot_reason_cmdline);

