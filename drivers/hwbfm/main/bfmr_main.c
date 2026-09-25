/*
 * bfmr_main.c
 *
 * It is the main entry for BFMR(Boot Fail Monitor and Recovery)
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/semaphore.h>
#include <chipset_common/bfmr/common/bfmr_common.h>
#include <chipset_common/bfmr/bfm/core/bfm_core.h>
#include <chipset_common/bfmr/bfr/core/bfr_core.h>

#define BOOT_LOCK_INFO_NUM 32

static char s_boot_lock_info[BOOT_LOCK_INFO_NUM] = {0};
static int s_is_bfmr_enabled;
static int s_is_bfr_enabled;
static DEFINE_SEMAPHORE(s_bfmr_enable_ctl_sem);

static int __init bfm_early_parse_bootlock_cmdline(char *p)
{
	if (!p)
		return -1;

	BFMR_PRINT_KEY_INFO(BFMR_BOOTLOCK_FIELD_NAME "=%s\n", p);
	memset((void *)s_boot_lock_info, 0, sizeof(s_boot_lock_info));
	snprintf(s_boot_lock_info, sizeof(s_boot_lock_info) - 1,
		BFMR_BOOTLOCK_FIELD_NAME "=%s", p);

	return 0;
}
early_param(BFMR_BOOTLOCK_FIELD_NAME, bfm_early_parse_bootlock_cmdline);

static int __init early_parse_bfmr_enable_flag(char *p)
{
	if (!p)
		return -1;

	if (!strncmp(p, "1", strlen("1")))
		s_is_bfmr_enabled = 1;
	else
		s_is_bfmr_enabled = 0;

	BFMR_PRINT_KEY_INFO(BFMR_ENABLE_FIELD_NAME "=%s\n", p);
	return 0;
}
early_param(BFMR_ENABLE_FIELD_NAME, early_parse_bfmr_enable_flag);

static int __init early_parse_bfr_enable_flag(char *p)
{
	if (!p)
		return -1;

	if (!strncmp(p, "1", strlen("1")))
		s_is_bfr_enabled = 1;
	else
		s_is_bfr_enabled = 0;

	BFMR_PRINT_KEY_INFO(BFR_ENABLE_FIELD_NAME "=%s\n", p);
	return 0;
}
early_param(BFR_ENABLE_FIELD_NAME, early_parse_bfr_enable_flag);

bool bfmr_has_been_enabled(void)
{
	int bfmr_enable_flag;

	down(&s_bfmr_enable_ctl_sem);
	bfmr_enable_flag = s_is_bfmr_enabled;
	up(&s_bfmr_enable_ctl_sem);
	return (bfmr_enable_flag == 0) ? (false) : (true);
}

bool bfr_has_been_enabled(void)
{
	return (s_is_bfr_enabled == 0) ? (false) : (true);
}

void bfmr_enable_ctl(int enable_flag)
{
	down(&s_bfmr_enable_ctl_sem);
	s_is_bfmr_enabled = enable_flag;
	up(&s_bfmr_enable_ctl_sem);
}

char *bfm_get_bootlock_value_from_cmdline(void)
{
	return s_boot_lock_info;
}


/*
 * function: int __init bfmr_init(void)
 * brief: init bfmr module in kernel.
 *
 * param: none.
 *
 * return: 0 - succeeded; -1 - failed.
 *
 * note: it need be initialized in bootloader and kernel.
 */
int __init bfmr_init(void)
{
	bfmr_common_init();
	bfm_init();
	bfr_init();

	return 0;
}

static void __exit bfmr_exit(void)
{}

fs_initcall(bfmr_init);
module_exit(bfmr_exit);
MODULE_LICENSE("GPL");

