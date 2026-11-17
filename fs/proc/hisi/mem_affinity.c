#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>

static size_t mem_affinity_proc_write(struct file *filp,
		const char __user *user_buf, size_t cnt, loff_t *ppos)
{
	size_t ret = cnt;
	pid_t upid = 0;
	struct pid *pid = NULL;
	struct task_struct *tsk = NULL;
	struct mm_struct *mm = NULL;

	if (kstrtoint_from_user(user_buf, cnt, 0, &upid))
		return  -EINVAL;

	pid = find_get_pid(upid);
	if (!pid) {
		ret =  -EINVAL;
		goto out;
	}

	tsk = get_pid_task(pid, PIDTYPE_PID);
	if (!tsk) {
		ret =  -ESRCH;
		goto pid_put;
	}

	mm = get_task_mm(tsk);
	if (!mm) {
		ret =  -EINVAL;
		goto tsk_put;
	}

	mm->dma_zone_tag = 1;

	mmput(mm);
tsk_put:
	put_task_struct(tsk);
pid_put:
	put_pid(pid);
out:
	return ret;
}

static int mem_affinity_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations mem_affinity_proc_fops = {
	.open		= mem_affinity_proc_open,
	.write		= mem_affinity_proc_write,
};

static int __init proc_mem_affinity_init(void)
{
	proc_create("mem_affinity", 0220, NULL, &mem_affinity_proc_fops);
	return 0;
}
fs_initcall(proc_mem_affinity_init);
