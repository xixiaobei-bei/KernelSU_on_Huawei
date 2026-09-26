/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: provide function to find out mmap_sem's owner.
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2020-11-11
 */
#include <linux/huawei_check_mmap_sem.h>

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/rwsem.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>

#ifdef CONFIG_DETECT_HUAWEI_MMAP_SEM_DBG
static int g_mmap_sem_debug;
#endif

enum rwsem_waiter_type {
	RWSEM_WAITING_FOR_WRITE,
	RWSEM_WAITING_FOR_READ
};

struct rwsem_waiter {
	struct list_head list;
	struct task_struct *task;
	enum rwsem_waiter_type type;
};

static int is_waiting_mmap_sem(struct task_struct *task)
{
	struct rwsem_waiter *waiter = NULL;
	struct rw_semaphore *sem = &task->mm->mmap_sem;

	raw_spin_lock_irq(&sem->wait_lock);
	list_for_each_entry(waiter, &sem->wait_list, list) {
		if (task == waiter->task) {
			raw_spin_unlock_irq(&sem->wait_lock);
			return 1;
		}
	}
	raw_spin_unlock_irq(&sem->wait_lock);

	return 0;
}

void check_mmap_sem(pid_t pid)
{
	struct task_struct *task = NULL;
	struct task_struct *t = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task) {
		pr_err("%s : can't find task!\n", __func__);
		goto out;
	}

	task_lock(task);

	pr_err("hungtask:name=%s,PID=%d,tgid=%d,tgname=%s\n",
		task->comm, task->pid, task->tgid, task->group_leader->comm);
	sched_show_task(task);

	if (!task->mm) {
		pr_err("%s has no mm!\n", task->comm);
		goto unlock;
	}

	if (!is_waiting_mmap_sem(task)) {
		pr_err("%s is not waiting for mmap_sem!\n", task->comm);
		goto unlock;
	}

	pr_err("%s is waiting for mmap_sem!\n", task->comm);

	for_each_thread(task, t) {
		if ((t->state == TASK_RUNNING ||
			t->state == TASK_UNINTERRUPTIBLE) &&
			!is_waiting_mmap_sem(t)) {
			pr_err("hungtask:name=%s,PID=%d, may hold mmap_sem\n",
				t->comm, t->pid);
			sched_show_task(t);
		}
	}

unlock:
	task_unlock(task);

out:
	rcu_read_unlock();
}

#ifdef CONFIG_DETECT_HUAWEI_MMAP_SEM_DBG
void mmap_sem_debug(const struct rw_semaphore *sem)
{
	int i;

	if (likely(g_mmap_sem_debug == 0))
		return;

	if (!current->mm || &current->mm->mmap_sem != sem)
		return;

	if (strcmp(current->comm, "system_server") == 0 ||
		strcmp(current->comm, "surfaceflinger") == 0) {
		i = 0;
		while (1) {
			ssleep(1);
			i++;
			pr_err("%s : holding %s's mmap_sem for %d seconds\n",
				__func__, current->comm, i);
		}
	}
}

static ssize_t mmap_sem_debug_write(struct file *file,
				    const char __user *buf,
				    size_t size,
				    loff_t *ppos)
{
	char val;
	int ret;

	ret = simple_write_to_buffer(&val, 1, ppos, buf, size);
	if (ret < 0)
		return ret;

	if (val == '1') {
		g_mmap_sem_debug = 1;
		pr_err("start mmap_sem debug\n");
	} else {
		pr_err("invalid value, can't start mmap_sem debug\n");
	}
	return size;
}

static int mmap_sem_debug_show(struct seq_file *s, void *unused)
{
	seq_puts(s, "set 1 to start mmap_sem_debug\n");
	return 0;
}

static int mmap_sem_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, mmap_sem_debug_show, NULL);
}

static const struct file_operations mmap_sem_debug_proc_fops = {
	.open    = mmap_sem_debug_open,
	.read    = seq_read,
	.write   = mmap_sem_debug_write,
	.llseek  = seq_lseek,
	.release = single_release,
};

static int __init mmap_sem_debug_init(void)
{
	struct dentry *d = NULL;

	d = debugfs_create_file("mmap_sem_debug", 0600, NULL, NULL,
		&mmap_sem_debug_proc_fops);
	if (d == NULL) {
		pr_err("%s: failed to create inode\n", __func__);
		return -EIO;
	}
	return 0;
}
module_init(mmap_sem_debug_init);
#endif
