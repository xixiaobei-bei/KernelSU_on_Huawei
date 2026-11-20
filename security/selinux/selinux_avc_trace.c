/*
 * selinux_avc_trace.c
 *
 * print trace log for selinux
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
 *
 */

#ifdef CONFIG_SECURITY_SELINUX_TRACE_LOG
#include "selinux_avc_trace.h"
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/segment.h>

#define AVC_FILE  "/system/avc_context.txt"

char avc_scontext[CONTEXT_LEN];
char avc_tcontext[CONTEXT_LEN];
char avc_tclass[CONTEXT_LEN];
static int avc_file_flag = -1;
static int avc_proc_flag = -1;
static struct proc_dir_entry *avc_entry;

enum {
	AVC_SUCC = 1,
	AVC_FAIL = 0,
	AVC_FILE_ERR = -1,
};

void __exit avc_proc_exit(void)
{
	if (avc_entry) {
		proc_remove(avc_entry);
		avc_entry = NULL;
	}
}

static int avc_readline(struct file *filp, char *buf, int len)
{
	int ret;
	int readlen = 0;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	for (; readlen < len; buf++, readlen++) {
		ret = vfs_read(filp, buf, 1, &filp->f_pos);
		if (ret <= 0)
			break;
		if (*buf == ' ' || *buf == '\r' || *buf == '\n' || *buf == '\0') {
			*buf = '\0';
			readlen++;
			break;
		}
	}
	if (!readlen) {
		set_fs(oldfs);
		return AVC_FILE_ERR;
	}
	set_fs(oldfs);
	pr_info("avc_readline completed\n");
	return 0;
}

static int avc_readfile(void)
{
	int ret;
	struct file *file = NULL;
	file = filp_open(AVC_FILE, O_RDONLY, 0);
	if (IS_ERR(file)) {
		pr_err("error occurred while opening file %s\n", AVC_FILE);
		return AVC_FILE_ERR;
	}
	pr_info("==========avc trace settings==========");
	ret = avc_readline(file, avc_scontext, CONTEXT_LEN);
	if (ret) {
		pr_err("error occurred while read scontext\n");
		filp_close(file, NULL);
		return AVC_FILE_ERR;
	}
	ret = avc_readline(file, avc_tcontext, CONTEXT_LEN);
	if (ret) {
		pr_err("error occurred while read tcontext\n");
		filp_close(file, NULL);
		return AVC_FILE_ERR;
	}
	ret = avc_readline(file, avc_tclass, CONTEXT_LEN);
	if (ret) {
		pr_err("error occurred while read tclass\n");
		filp_close(file, NULL);
		return AVC_FILE_ERR;
	}

	avc_file_flag = 1;
	avc_proc_flag = 1;
	filp_close(file, NULL);
	return AVC_SUCC;
}

static ssize_t avc_openfile(struct file *file, char __user *buf,
			    size_t size, loff_t *ppos)
{
	if (avc_proc_flag < 0)
		avc_proc_flag = 0;
	avc_readfile();
	return 0;
}

static const struct file_operations trace_fops = {
	.read = avc_openfile,
};

int avc_getcontext(void)
{
	int ret;
	if (avc_proc_flag < 0)
		return AVC_SUCC;
	if (avc_file_flag > 0)
		return AVC_SUCC;
	ret = avc_readfile();
	if (ret == 1)
		return AVC_SUCC;
	return AVC_FAIL;
}

int __init avc_proc_init(void)
{
	avc_entry = proc_create("avc_trace", 0440, NULL, &trace_fops);
	return 0;
}

module_init(avc_proc_init);
module_exit(avc_proc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("print log for selinux");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
#endif
