/*
 * hw_kernel_stp_proc.c
 *
 * the hw_kernel_stp_proc.c for proc file create and destroy
 *
 * sunhongqing <sunhongqing@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include "hw_kernel_stp_proc.h"

static const char *TAG = "kernel_stp_proc";
static struct proc_dir_entry *proc_entry;

static const umode_t FILE_CREAT_RO_MODE = 0220;
static const kgid_t SYSTEM_GID = KGIDT_INIT((gid_t)1000);

static ssize_t kernel_stp_proc_write(struct file *file,const char __user *buffer,size_t count,loff_t *pos)
{
	char *mode;

	if(KERNEL_STP_TRIGGER_MARK_LEN == count) {
		mode = kzalloc((count + 1) * sizeof(char), GFP_KERNEL);
		if(NULL == mode){
			KSTPLogError(TAG, "proc mode init failed");
			return -ENOMEM;
		}

		if(copy_from_user(mode, buffer, count)) {
			KSTPLogError(TAG, "mode copy from user failed");
			kfree(mode);
			return -EFAULT;
		}

		if(0 == strncmp(mode, KERNEL_STP_TRIGGER_MARK, KERNEL_STP_TRIGGER_MARK_LEN))
		{	
				kernel_stp_scanner();
				KSTPLogTrace(TAG, "kernel stp trigger scanner success");
		}

		kfree(mode);
		return count;
	}
	return -EIO;
}

static const struct file_operations kernel_stp_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= simple_open,
	.read		= seq_read,
	.write		= kernel_stp_proc_write,
	.llseek		= seq_lseek,
};

int kernel_stp_proc_init(void)
{
	int ret = 0;

	proc_entry = proc_create("kernel_stp", FILE_CREAT_RO_MODE, NULL,
							&kernel_stp_proc_fops);

	if (proc_entry == NULL) {
		KSTPLogError(TAG, "proc_entry create is failed");
		return -ENOMEM;
	}

	/* set proc file gid to system gid */
	proc_set_user(proc_entry, GLOBAL_ROOT_UID, SYSTEM_GID);

	KSTPLogTrace(TAG, "proc_entry init success");
	return ret;
}

void kernel_stp_proc_exit(void)
{
	remove_proc_entry("kernel_stp", NULL);
	KSTPLogTrace(TAG, "proc_entry cleanup success");
}
