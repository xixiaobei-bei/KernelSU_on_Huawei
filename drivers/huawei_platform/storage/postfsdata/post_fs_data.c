/*
 * post_fs_data.c
 *
 * add for kernel to know data is ready
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>

#define POSTFSDATA_NAME "post-fs-data"

static unsigned int postfsdata_flag;

unsigned int get_postfsdata_flag(void)
{
	return postfsdata_flag;
}
EXPORT_SYMBOL(get_postfsdata_flag);

void set_postfsdata_flag(unsigned int value)
{
	postfsdata_flag = value;
}
EXPORT_SYMBOL(set_postfsdata_flag);

int wait_for_postfsdata(unsigned int timeout)
{
	while (postfsdata_flag == 0) {
		/* schedule_timeout(HZ); */
		msleep(1000);
		if (timeout) {
			timeout--;
			if (timeout == 0)
				return 1;
		}
	}
	return 0;
}
EXPORT_SYMBOL(wait_for_postfsdata);

static int postfsdata_info_show(struct seq_file *m, void *v)
{
	(void)seq_printf(m, "%u\n", postfsdata_flag);
	return 0;
}

int postfsdata_write_proc(struct file *file, const char * __user buffer,
			unsigned long count, void *data)
{
	char tmp;

	/* buffer length must 2 */
	if (count > 2)
		return ret;
	/* should ignore character '\n' */
	if (copy_from_user(&tmp, buffer, 1))
		return -EFAULT;

	if (tmp == '1')
		set_postfsdata_flag(1);
	else if (tmp == '0')
		set_postfsdata_flag(0);
	return 1;
}

static int postfsdata_open(struct inode *inode, struct file *file)
{
	return single_open(file, postfsdata_info_show, NULL);
}

static const struct file_operations postfsdata_proc_fops = {
	.open		= postfsdata_open,
	.read		= seq_read,
	.write		= postfsdata_write_proc,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init postfsdata_proc_init(void)
{
	proc_create(POSTFSDATA_NAME, 0600, NULL, &postfsdata_proc_fops);
	return 0;
}

module_init(postfsdata_proc_init);
