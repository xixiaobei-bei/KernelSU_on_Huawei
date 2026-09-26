/*
 * storage_rofa.c
 *
 * storage Read Only First Aid
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/string.h>

#include <chipset_common/storage_rofa/storage_rofa.h>
#include <hwbootfail/core/boot_recovery.h>

#define STORAGE_ROFA_STR_CHECK      "check"
#define STORAGE_ROFA_STR_BYPASS     "bypass"
#define ANDROID_BOOT_MODE_NORMAL    "normal"

static unsigned int g_storage_rochk;
static unsigned int g_storage_row;
static unsigned int g_androidboot_normalmode;
static unsigned int g_bopd_enable_row;
#ifdef CONFIG_STORAGE_ROW_SKIP_PART
static unsigned int g_bopd_row_skip_part;
#endif

static int __init early_parse_storage_rofa_cmdline(char *cmdline_opt)
{
	if (cmdline_opt == NULL)
		return 0;

	pr_info("early_param storage_rofa option is [%s]\n", cmdline_opt);

	if (strncmp(cmdline_opt, STORAGE_ROFA_STR_CHECK,
	    strlen(STORAGE_ROFA_STR_CHECK)) == 0)
		g_storage_rochk = 0x01;
	else if (strncmp(cmdline_opt, STORAGE_ROFA_STR_BYPASS,
		 strlen(STORAGE_ROFA_STR_BYPASS)) == 0)
		g_storage_row = 0x01;

	return 0;
}
early_param("storage_rofa", early_parse_storage_rofa_cmdline);

static int __init early_parse_androidboot_mode_cmdline(char *cmdline_opt)
{
	if (cmdline_opt == NULL)
		return 0;

	pr_info("early_param androidboot.mode option is [%s]\n", cmdline_opt);

	if (strncmp(cmdline_opt, ANDROID_BOOT_MODE_NORMAL,
	    strlen(ANDROID_BOOT_MODE_NORMAL)) == 0)
		g_androidboot_normalmode = 0x01;

	return 0;
}
early_param("androidboot.mode", early_parse_androidboot_mode_cmdline);

static int __init early_parse_bopd_mode_cmdline(char *bopd_mode_cmdline)
{
	unsigned long long value = 0;

	pr_info("early_param bopd.mode option is [%s]\n", bopd_mode_cmdline);

#ifdef CONFIG_STORAGE_ROW_SKIP_PART
	if (kstrtoull(bopd_mode_cmdline, 16, &value) == 0) {
/*
 * If we enter BOPD with the disk is Read-Only, we enable ROW on the
 * whole disk; If we enter BOPD with the disk is NOT Read-Only, we still
 * enable ROW on the whole disk, but skip ROW on DFX partitions, such as
 * log, misc, splash2, rrecord. So the system can still write to DFX
 * partitions.
 */
		if (is_bopd_run_mode(value))
			g_bopd_enable_row = 1;

		if (value != BOPD_STG_RO_BEFORE_DL &&
			value != BOPD_STG_RO_HW_DEG_BEFORE_DL)
			g_bopd_row_skip_part = 1;
	}
#else
	if (kstrtoull(bopd_mode_cmdline, 16, &value) == 0 &&
		(value == BOPD_STG_RO_BEFORE_DL ||
		value == BOPD_STG_RO_HW_DEG_BEFORE_DL))
		g_bopd_enable_row = 0x01;
#endif

	return 0;
}
early_param("bopd.mode", early_parse_bopd_mode_cmdline);

unsigned int get_storage_rofa_bootopt(void)
{
	if (g_androidboot_normalmode == 0)
		return STORAGE_ROFA_BOOTOPT_NOP;
	else if (g_bopd_enable_row)
		return STORAGE_ROFA_BOOTOPT_BYPASS;
	else if (g_storage_rochk)
		return STORAGE_ROFA_BOOTOPT_CHECK;
	else
		return STORAGE_ROFA_BOOTOPT_NOP;
}

#ifdef CONFIG_STORAGE_ROW_SKIP_PART
unsigned int is_bopd_row_skip_part(void)
{
	return g_bopd_row_skip_part;
}
#endif
