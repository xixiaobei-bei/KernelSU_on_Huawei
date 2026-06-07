#ifndef __HISI_BKOPS_SYSFS_INTERFACE_H__
#define __HISI_BKOPS_SYSFS_INTERFACE_H__
#include <linux/debugfs.h>
#include <linux/hisi-bkops-core.h>

extern int __cfi_hisi_bkops_stat_open(struct inode *inode, struct file *filp);
extern int hisi_bkops_stat_open(struct inode *inode, struct file *filp);
extern ssize_t __cfi_hisi_bkops_stat_read(struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos);
extern ssize_t hisi_bkops_stat_read(struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos);
extern int __cfi_hisi_bkops_stat_release(struct inode *inode, struct file *file);
extern int hisi_bkops_stat_release(struct inode *inode, struct file *file);
extern int __cfi_hisi_bkops_force_query_open(struct inode *inode, struct file *filp);
extern int hisi_bkops_force_query_open(struct inode *inode, struct file *filp);
extern ssize_t __cfi_hisi_bkops_force_query_read(struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos);
extern ssize_t hisi_bkops_force_query_read(struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos);
extern int __cfi_hisi_bkops_force_query_release(struct inode *inode, struct file *file);
extern int hisi_bkops_force_query_release(struct inode *inode, struct file *file);
#endif /* __HISI_BKOPS_SYSFS_INTERFACE_H__ */
