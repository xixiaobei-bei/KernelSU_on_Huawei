/*
 * hardware_info.c
 *
 * show hardware info
 *
 * Copyright (c) 2010-2019 Huawei Technologies Co., Ltd.
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
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_fdt.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define HARDWARE_FILE_MODE        (S_IRUSR | S_IRGRP)

static const char *hardware_name = NULL;
static int hardware_info_show(struct seq_file *m, void *v)
{
	if (m == NULL || v == NULL)
		return -1;

	if (hardware_name != NULL)
		seq_printf(m, "%s\n", hardware_name);
	else
		seq_printf(m, "%s\n", "unknown");

	return 0;
}

static int hardware_info_open(struct inode *inode, struct file *file)
{
	if (inode == NULL || file == NULL)
		return -1;

	return single_open(file, hardware_info_show, NULL);
}

const struct file_operations hardware_info_fops = {
	.owner = THIS_MODULE,
	.open = hardware_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init proc_hardware_info_init(void)
{
	hardware_name = of_flat_dt_get_machine_name();
	proc_create("hardware", HARDWARE_FILE_MODE, NULL, &hardware_info_fops);

	return 0;
}
fs_initcall(proc_hardware_info_init);
