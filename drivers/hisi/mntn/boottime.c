/*
 * boottime.c
 *
 * boottime module
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/uaccess.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <pr_log.h>
#include <linux/hisi/util.h>
#include <rdr_inner.h>
#define PR_LOG_TAG BOOTTIME_TAG

/* record kernle boot is completed */
#define COMPLETED_MASK 0xABCDEF00
static unsigned int g_bootanim_complete;

/*
 * check kernel boot is completed
 * Return: 1,  kernel boot is completed
 *         0,  no completed
 */
int is_bootanim_completed(void)
{
	return (g_bootanim_complete == COMPLETED_MASK);
}

static ssize_t boot_time_proc_write(struct file *file, const char __user *buf,
				size_t nr, loff_t *off)
{
	if (is_bootanim_completed())
		return nr;

	/* only need the print time */
	pr_err("bootanim has been complete, turn to Lancher!\n");

	g_bootanim_complete = COMPLETED_MASK;

	hisi_dump_bootkmsg();
	return nr;
}

static const struct file_operations boot_time_proc_fops = {
	.write = boot_time_proc_write,
};

static int __init boot_time_proc_init(void)
{
	dfx_create_stats_proc_entry("boot_time", (S_IWUSR), &boot_time_proc_fops, NULL);

	return 0;
}

module_init(boot_time_proc_init);
