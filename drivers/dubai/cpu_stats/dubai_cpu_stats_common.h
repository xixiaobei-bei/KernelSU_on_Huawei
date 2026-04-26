#ifndef DUBAI_CPU_STATS_COMMON_H
#define DUBAI_CPU_STATS_COMMON_H

#include <linux/sched.h>
#include <linux/sched/signal.h>

#define KERNEL_TGID				0
#define KERNEL_NAME				"kernel"
#define PREFIX_LEN				32
#define MAX_CPU_FREQ_NR			64

static inline bool dubai_is_kthread(struct task_struct *task)
{
	return task->flags & PF_KTHREAD;
}

static inline bool dubai_is_kernel_tgid(pid_t tgid)
{
	return tgid == KERNEL_TGID;
}

// combine kernel thread to same entry which tgid is 0
static inline int dubai_get_task_normalized_tgid(struct task_struct *task)
{
	return (dubai_is_kthread(task) ? KERNEL_TGID : task->tgid);
}

static inline const char *dubai_get_task_normalized_name(struct task_struct *task)
{
	return (dubai_is_kthread(task) ? KERNEL_NAME : (task->group_leader ?
		task->group_leader->comm : task->comm));
}

static inline bool dubai_thread_group_dying(struct task_struct *task)
{
	return thread_group_leader(task);
}

static inline bool dubai_is_task_alive(struct task_struct *task)
{
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	// uid_sys_stats.c will set @cpu_power as ULLONG_MAX when task is exiting
	if (task->cpu_power == ULLONG_MAX)
		return false;
#endif
	return !(task->flags & PF_EXITING);
}

int dubai_set_proc_decompose(const void __user *argp);
void dubai_remove_proc_decompose(pid_t tgid);
void dubai_set_proc_entry_decompose(pid_t tgid, struct task_struct *task, const char *name);

int dubai_get_cpu_configs(void __user *argp);
int dubai_cpu_configs_init(void);
void dubai_cpu_configs_exit(void);

struct dubai_cpu_config_info {
	uint32_t freq_policy_cpu;
	uint32_t idle_nr;
	uint32_t freq_nr;
	uint32_t avail_freqs[MAX_CPU_FREQ_NR];
} __packed;

#endif // DUBAI_CPU_STATS_COMMON_H
