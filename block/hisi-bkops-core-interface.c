#include <linux/kernel.h>
#include "hisi-bkops-core-interface.h"

void __ufs_bkops_idle_work_func(struct work_struct *work)
{
	bkops_idle_work_func(work);
}

enum blk_busy_idle_callback_return __cfi_bkops_io_busy_idle_notify_handler(
	struct blk_busy_idle_event_node *event_node,
	enum blk_idle_notify_state state)
{
	return bkops_io_busy_idle_notify_handler(event_node, state);
}

int __cfi_bkops_pm_callback(
	struct notifier_block *nb, unsigned long action, void *ptr)
{
	return bkops_pm_callback(nb, action, ptr);
}

int __cfi_bkops_notify_reboot(
	struct notifier_block *this, unsigned long code, void *x)
{
	return bkops_notify_reboot(this, code, x);
}

int __cfi_hisi_bkops_manual_gc_proc_show(struct seq_file *m, void *v)
{
	return hisi_bkops_manual_gc_proc_show(m, v);
}

int __cfi_hisi_bkops_manual_gc_proc_open(
	struct inode *p_inode, struct file *p_file)
{
	return hisi_bkops_manual_gc_proc_open(p_inode, p_file);
}

ssize_t __cfi_hisi_bkops_manual_gc_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos)
{
	return hisi_bkops_manual_gc_proc_write(p_file,userbuf, count, ppos);
}

int __cfi_hisi_bkops_status_proc_show(struct seq_file *m, void *v)
{
	return hisi_bkops_status_proc_show(m, v);
}

int __cfi_hisi_bkops_status_proc_open(
	struct inode *p_inode, struct file *p_file)
{
	return hisi_bkops_status_proc_open(p_inode, p_file);
}


