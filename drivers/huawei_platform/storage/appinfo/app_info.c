/*
 * app_info.c
 *
 * for app_info file operation
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
 *
 */

#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/printk.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/module.h>

unsigned int get_pd_charge_flag(void);
unsigned int get_boot_into_recovery_flag(void);

static int app_info_show(struct seq_file *m, void *v)
{
	(void)seq_printf(m, "recovery_flag:\n%d\n"
			"charge_flag:\n%d\n",
			(int)get_boot_into_recovery_flag(),
			(int)get_pd_charge_flag());
	return 0;
}

static int appinfo_open(struct inode *inode, struct file *file)
{
	return single_open(file, app_info_show, NULL);
}

static const struct file_operations proc_appinfo_operations = {
	.open		= appinfo_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_app_info_init(void)
{
	/* fixme::: should change "app_info_1" to "app_info" ;
	 * now: in touch scream driver , app_info is create ;
	 * but it's not correct;
	 */
	proc_create("app_info", 0000, NULL, &proc_appinfo_operations);
	return 0;
}

module_init(proc_app_info_init);
