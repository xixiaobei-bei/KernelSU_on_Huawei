#include "hisi-bkops-sysfs-interface.h"

#ifdef CONFIG_HISI_DEBUG_FS
int __cfi_hisi_bkops_stat_open(struct inode *inode, struct file *filp)
{
	return hisi_bkops_stat_open(inode, filp);
}

ssize_t __cfi_hisi_bkops_stat_read(
	struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
	return hisi_bkops_stat_read(filp, ubuf, cnt, ppos);
}

int __cfi_hisi_bkops_stat_release(struct inode *inode, struct file *file)
{
	return hisi_bkops_stat_release(inode, file);
}

int __cfi_hisi_bkops_force_query_open(struct inode *inode, struct file *filp)
{
	return hisi_bkops_force_query_open(inode, filp);
}

ssize_t __cfi_hisi_bkops_force_query_read(
	struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
	return hisi_bkops_force_query_read(filp, ubuf, cnt, ppos);
}

int __cfi_hisi_bkops_force_query_release(struct inode *inode, struct file *file)
{
	return hisi_bkops_force_query_release(inode, file);
}
#endif
