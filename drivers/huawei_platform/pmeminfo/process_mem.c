/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: get process meminfo for TopN
 * Author: PangNana pangnana@huawei.com
 * Create: 2019-05-27
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/oom.h>
#include <linux/list.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/fdtable.h>

#define MAX_LENGTH 64
#define TOPN_DEFAULT 3
#define TUNIT_SIZE 1024
#define PROCNAME    "driver/process_mem"

struct proc_meminfo {
	char task_name[MAX_LENGTH];
	int task_size;
	struct list_head node_task;
};

static unsigned int top_n = TOPN_DEFAULT;
static struct kmem_cache *process_cache;

static int process_mem_write(const struct file *file, const char __user *buffer,
			unsigned long count, void *data)
{
	int rv;

	rv = kstrtouint_from_user(buffer, count, 10, &top_n);
	if (rv < 0)
		return rv;

	return count;
}

int process_get_ion_size(const struct task_struct *task)
{
	int n = 0;
	size_t ion_size = 0;
	struct files_struct *files;
	struct fdtable *fdt;

	if (task->flags & PF_KTHREAD)
		return ion_size;

	files = task->files;
	if (!files)
		return ion_size;

	spin_lock(&files->file_lock);
	for (fdt = files_fdtable(files); n < fdt->max_fds; n++) {
		struct dma_buf *dbuf;
		struct file *f = rcu_dereference_check_fdtable(files,
					fdt->fd[n]);

		if (!f)
			continue;
		if (!is_dma_buf_file(f))
			continue;
		dbuf = file_to_dma_buf(f);
		if (!dbuf)
			continue;
		if (dbuf->owner != THIS_MODULE)
			continue;
		if (!dbuf->priv)
			continue;
		ion_size += dbuf->size;
	}
	spin_unlock(&files->file_lock);
	return (int)(ion_size / TUNIT_SIZE);
}

struct proc_meminfo *process_info_add(const struct task_struct *task, int tsize)
{
	struct proc_meminfo *dump_info;

	dump_info = kmem_cache_zalloc(process_cache, GFP_ATOMIC);
	if (dump_info) {
		strcpy(dump_info->task_name, task->comm);
		dump_info->task_size = tsize;
	}
	return dump_info;
}

static int process_mem_show(struct seq_file *m, void *v)
{
	struct task_struct *p;
	struct task_struct *task;
	int count;
	int tasksize;
	struct proc_meminfo *dump_info;
	struct proc_meminfo *temp_info;
	struct list_head dumpclass;

	rcu_read_lock();
	INIT_LIST_HEAD(&dumpclass);

	for_each_process(p) {
		task = find_lock_task_mm(p);
		/*
		 * This is a kthread or all of p's threads have already
		 * detached their mm's.  There's no need to report
		 * them; they can't be oom killed anyway.
		 */
		if (!task)
			continue;

		tasksize = get_mm_rss(task->mm) +
					get_mm_counter(task->mm, MM_SWAPENTS);
		tasksize *= (int)(PAGE_SIZE / TUNIT_SIZE);
		tasksize += process_get_ion_size(task);

		if (list_empty(&dumpclass)) {
			dump_info = process_info_add(task, tasksize);
			if (dump_info)
				list_add(&dump_info->node_task, &dumpclass);
			goto t_unlock;
		}

		count = 0;
		list_for_each_entry(temp_info, &dumpclass, node_task) {
			count++;
			if (count >= top_n)
				break;
			if (tasksize > temp_info->task_size) {
				dump_info = process_info_add(task, tasksize);
				if (dump_info)
					list_add_tail(&dump_info->node_task,
							&temp_info->node_task);
				break;

			} else if (list_is_last(&temp_info->node_task,
						&dumpclass)) {
				dump_info = process_info_add(task, tasksize);
				if (dump_info)
					list_add(&dump_info->node_task,
							&temp_info->node_task);
				break;
			}
		}
t_unlock:
		task_unlock(task);
	}
	count = 0;
	seq_printf(m, "%u\n", top_n);
	list_for_each_entry(temp_info, &dumpclass, node_task) {
		if (count++ < top_n) {
			seq_printf(m, "%s ", temp_info->task_name);
			seq_printf(m, "%dKB\n", temp_info->task_size);
		}
		kmem_cache_free(process_cache, temp_info);
	}
	rcu_read_unlock();

	return 0;
}

static int process_mem_open(struct inode *inode, struct file *file)
{
	return single_open(file, process_mem_show, NULL);
}

static const struct file_operations process_mem_fops = {
	.owner		= THIS_MODULE,
	.open		= process_mem_open,
	.write		= process_mem_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init process_mem_procfs_init(void)
{
	struct proc_dir_entry *proc_entry_mem = NULL;

	proc_entry_mem = proc_create(PROCNAME, 0660, NULL, &process_mem_fops);
	if (!proc_entry_mem) {
		pr_err("can't create /proc/%s\n", PROCNAME);
		return -ENOMEM;
	}

	process_cache = kmem_cache_create("pinfo_cache",
			sizeof(struct proc_meminfo), 0, 0, NULL);
	if (!process_cache)
		return -ENOMEM;

	return 0;
}

static void __exit process_mem_procfs_exit(void)
{
	kmem_cache_destroy(process_cache);
	remove_proc_entry(PROCNAME, NULL);
}

module_init(process_mem_procfs_init)
module_exit(process_mem_procfs_exit)
