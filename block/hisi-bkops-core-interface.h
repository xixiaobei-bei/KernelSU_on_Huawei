#ifndef __HISI_BKOPS_CORE_INTERFACE_H__
#define __HISI_BKOPS_CORE_INTERFACE_H__
#include <linux/blkdev.h>
#include <linux/hisi-bkops-core.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>

extern void bkops_idle_work_func(struct work_struct *work);
extern void __ufs_bkops_idle_work_func(struct work_struct *work);
extern enum blk_busy_idle_callback_return bkops_io_busy_idle_notify_handler(
	struct blk_busy_idle_event_node *event_node,
	enum blk_idle_notify_state state);
extern enum blk_busy_idle_callback_return __cfi_bkops_io_busy_idle_notify_handler(
	struct blk_busy_idle_event_node *event_node,
	enum blk_idle_notify_state state);
extern int __cfi_bkops_pm_callback(
	struct notifier_block *nb, unsigned long action, void *ptr);
extern int bkops_pm_callback(
	struct notifier_block *nb, unsigned long action, void *ptr);
extern int bkops_notify_reboot(
	struct notifier_block *this, unsigned long code, void *x);
extern int __cfi_bkops_notify_reboot(
	struct notifier_block *this, unsigned long code, void *x);
extern int __cfi_hisi_bkops_manual_gc_proc_show(struct seq_file *m, void *v);
extern int hisi_bkops_manual_gc_proc_show(struct seq_file *m, void *v);
extern int __cfi_hisi_bkops_manual_gc_proc_open(
	struct inode *p_inode, struct file *p_file);
extern int hisi_bkops_manual_gc_proc_open(
	struct inode *p_inode, struct file *p_file);
extern ssize_t __cfi_hisi_bkops_manual_gc_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos);
extern ssize_t hisi_bkops_manual_gc_proc_write(struct file *p_file,
	const char __user *userbuf, size_t count, loff_t *ppos);
extern int __cfi_hisi_bkops_status_proc_show(struct seq_file *m, void *v);
extern int hisi_bkops_status_proc_show(struct seq_file *m, void *v);
extern int __cfi_hisi_bkops_status_proc_open(
	struct inode *p_inode, struct file *p_file);
extern int hisi_bkops_status_proc_open(
	struct inode *p_inode, struct file *p_file);
#endif /* __HISI_BKOPS_CORE_INTERFACE_H__ */

