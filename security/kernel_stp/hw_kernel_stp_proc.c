/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: the hw_kernel_stp_proc.c for proc file create and destroy
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-03-31
 */

#include "hw_kernel_stp_proc.h"
#ifdef CONFIG_HW_SLUB_DF
#include <linux/slub_def.h> /* for harden double-free check */
#endif
#include <chipset_common/security/kshield.h>

#ifdef CONFIG_HHEE
#include <linux/hisi/hkip_hhee.h> /* for hhee tvm */
#endif

static const char *TAG = "kernel_stp_proc";
static struct proc_dir_entry *g_proc_entry;

static const umode_t file_creat_ro_mode = 0220;
static const kgid_t system_gid = KGIDT_INIT((gid_t)1000);

static inline ssize_t kernel_stp_trigger(unsigned int param)
{
	if (param == KERNEL_STP_TRIGGER_MARK) {
		KSTPLogTrace(TAG, "kernel stp trigger scanner success");
		kernel_stp_scanner();
		return 0;
	}
	KSTPLogError(TAG, "kernel stp trigger scanner fail");
	return -EINVAL;
}

static ssize_t set_hhee_switch(unsigned int param)
{
#ifdef CONFIG_HHEE
	struct arm_smccc_res res = { 0 };
	if (hhee_check_enable() != HHEE_ENABLE)
		return -EINVAL;

	if (param & ENABLE_HHEE_TVM)
		arm_smccc_hvc(HHEE_HVC_ENABLE_TVM,
			0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0) {
		KSTPLogError(TAG, "set hhee switch %u fail", param);
		return -EFAULT;
	}
	KSTPLogTrace(TAG, "set hhee switch %u success", param);
#endif
	return 0;
}

static int handle_stp_proc_write(u32 feature, u32 param, char *str)
{
	switch (feature) {
	case KERNEL_STP_SCAN:
		return kernel_stp_trigger(param);
	case HARDEN_DBLFREE_CHECK:
#ifdef CONFIG_HW_SLUB_DF
		return set_harden_double_free_status(param);
#else
		return 0;
#endif
	case KERNEL_STP_KSHIELD:
		return ks_dev_ioctl(param, str);
	case HHEE_SWITCH:
		return set_hhee_switch(param);
	default:
		return -EINVAL;
	}
}

static ssize_t kernel_stp_proc_write(struct file *file, const char __user *buffer,
				size_t count, loff_t *pos)
{
	char *sub_str = NULL;
	char *str = NULL;
	char *backup_str = NULL;
	int ret = count;
	stp_proc_type data = {
		.val = 0
	};
	(void)file;
	(void)pos;
	str = kzalloc(KERNEL_STP_PROC_MAX_LEN+1, GFP_KERNEL);
	backup_str = str;
	if (!str)
		return -ENOMEM;
	if ((count <= 0) || (count > KERNEL_STP_PROC_MAX_LEN) ||
		copy_from_user(str, buffer, count)) {
		ret = -EFAULT;
		goto end;
	}

	sub_str = strsep(&str, ";");
	if (kstrtoull(sub_str, KERNEL_STP_PROC_HEX_BASE, &data.val)) {
		ret = -EINVAL;
		goto end;
	}
	KSTPLogTrace(TAG, "stp proc feature %u, param %u",
				data.s.feat, data.s.para);

	ret = handle_stp_proc_write(data.s.feat, data.s.para, str);

end:
	kfree(backup_str);
	if (ret < 0) {
		KSTPLogError(TAG, "stp proc process error, %d", ret);
		return ret;
	}
	return count;
}

/*
 * the function is called by kerenl function
 * single_open(struct file *, int (*)(struct seq_file *, void *), void *)
 */
static int kernel_stp_proc_show(struct seq_file *m, void *v)
{
	(void)v;
	seq_printf(m, "%d", 0);
	return 0;
}

static int kernel_stp_proc_open(struct inode *inode, struct file *file)
{
	(void)inode;
	return single_open(file, kernel_stp_proc_show, NULL);
}

static const struct file_operations kernel_stp_proc_fops = {
	.owner          = THIS_MODULE,
	.open           = kernel_stp_proc_open,
	.read           = seq_read,
	.write          = kernel_stp_proc_write,
	.llseek         = seq_lseek,
};

int kernel_stp_proc_init(void)
{
	g_proc_entry = proc_create("kernel_stp", file_creat_ro_mode, NULL,
				&kernel_stp_proc_fops);
	if (g_proc_entry == NULL) {
		KSTPLogError(TAG, "g_proc_entry create is failed");
		return -ENOMEM;
	}

	/* set proc file gid to system gid */
	proc_set_user(g_proc_entry, GLOBAL_ROOT_UID, system_gid);

	KSTPLogTrace(TAG, "g_proc_entry init success");
	return 0;
}

void kernel_stp_proc_exit(void)
{
	remove_proc_entry("kernel_stp", NULL);
	KSTPLogTrace(TAG, "g_proc_entry cleanup success");
}
