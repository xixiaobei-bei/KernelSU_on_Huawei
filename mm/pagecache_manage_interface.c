#include "pagecache_manage_interface.h"

void __cfi_pch_timeout_work(struct work_struct *work)
{
	return pch_timeout_work(work);
}

void __cfi_pch_timeout_timer(unsigned long data)
{
	return pch_timeout_timer(data);
}

int __cfi_pch_emui_bootstat_proc_show(struct seq_file *m, void *v)
{
	return pch_emui_bootstat_proc_show(m, v);
}

int __cfi_pch_emui_bootstat_proc_open(struct inode *p_inode, struct file *p_file)
{
	return pch_emui_bootstat_proc_open(p_inode, p_file);
}

ssize_t __cfi_pch_emui_bootstat_proc_write(struct file *p_file,
		const char __user *userbuf, size_t count, loff_t *ppos)
{
	return pch_emui_bootstat_proc_write(p_file, userbuf, count, ppos);
}

int __init __cfi_hisi_page_cache_manage_init(void)
{
	return hisi_page_cache_manage_init();
}

void __exit __cfi_hisi_page_cache_manage_exit(void)
{
	return hisi_page_cache_manage_exit();
}

