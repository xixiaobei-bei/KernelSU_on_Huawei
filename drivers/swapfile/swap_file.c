/*
 * swap_file.c
 *
 * Hisilicon swapfile test
 *
 * Copyright (c) 2019-2020, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: pengrucheng@huawei.com
 * Create: 2020-04-08
 *
 */
#include "swap_file.h"
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/suspend.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/err.h>
#include <securec.h>

static int run_cmd(const char *cmd)
{
	char **argv;
	int ret;

	static char *envp[] = {
		"HOME=/",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
		NULL
	};

	argv = argv_split(GFP_KERNEL, cmd, NULL);
	if (argv) {
		ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
		argv_free(argv);
	} else {
		swap_file_err("argv_split failed, cmd[%s]\n", cmd);
		ret = -ENOMEM;
	}

	return ret;
}

static int run_swap_cmd(const char *swapcmd, const char *filename)
{
	int ret;
	char cmd[SWAP_CMD_LEN] = {0};

	swap_file_info("swap_file run swap_cmd filename %s", filename);
	ret = sprintf_s(cmd, SWAP_CMD_LEN, "%s %s", swapcmd, filename);
	if (ret <= 0) {
		return -1;
	}
	ret = run_cmd(cmd);
	swap_file_info("cmd(%s) exec (%d)\n", cmd, ret);
	return ret;
}

/*
 * swapon swapfile for hibernate
 * @filename:  file pointer, for the required swapfile
 *
 * Returns 0 success, else fail.
 */
static int swapon(const char *filename)
{
	int ret;

	struct file *fp = NULL;

	fp = filp_open(filename, O_RDONLY | O_LARGEFILE, 0);

	if (IS_ERR(fp)) {
		swap_file_info("swapfile %s is not exist", filename);
		return -ENOTBLK;
	}
	filp_close(fp, NULL);
	ret = run_swap_cmd(SWAPON_CMD, filename);

	if (ret != 0) {
		swap_file_err("swap_file swapon filename %s fail", filename);
		return ret;
	}

	swap_file_info("swap_file swapon filename %s", filename);
	return 0;
}

/*
 * swapoff swapfile for hibernate
 * @filename:  file pointer, for the required swapfile
 *
 * Returns 0 success, else fail.
 */
static int swapoff(char *filename)
{
	int ret;

	struct file *fp = NULL;

	fp = filp_open(filename, O_RDONLY | O_LARGEFILE, 0);
	if (IS_ERR(fp)) {
		swap_file_info("swapfile %s is not exist", filename);
		return -ENOTBLK;
	}
	filp_close(fp, NULL);
	ret = run_swap_cmd(SWAPOFF_CMD, filename);

	if (ret != 0) {
		swap_file_err("swap_file swapoff filename %s fail", filename);
		return ret;
	}

	swap_file_info("swap_file swapoff filename %s", filename);
	return 0;
}

/*
 * the hisi pm_callback for hibernate
 * @nb: a pointer to notify_call
 * @action: refers to system events received
 * @ptr: default input function param
 */
static int swapfile_pm_callback(struct notifier_block *nb, unsigned long action, void *ptr)
{
	if (action == PM_HIBERNATION_PREPARE)
		return swapon(SWAPFILE_NAME);
	else if (action == PM_POST_HIBERNATION)
		return swapoff(SWAPFILE_NAME);

	return NOTIFY_OK;
}

static struct notifier_block g_swapfile_pm_callback_nb = {
	.notifier_call = swapfile_pm_callback,
	.priority = 0
};

/*
 * the hisi swapfile module init function.
 */
static int __init swap_file_init(void)
{
	register_pm_notifier(&g_swapfile_pm_callback_nb);
	return 0;
}

static void __exit swap_file_clean(void)
{
	unregister_pm_notifier(&g_swapfile_pm_callback_nb);
}

module_init(swap_file_init);
module_exit(swap_file_clean);
MODULE_LICENSE("GPL v2");
