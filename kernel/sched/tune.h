
#ifdef CONFIG_SCHED_TUNE

#include <linux/reciprocal_div.h>

/*
 * System energy normalization constants
 */
struct target_nrg {
	unsigned long min_power;
	unsigned long max_power;
	struct reciprocal_value rdiv;
};

int schedtune_cpu_boost(int cpu);
int schedtune_task_boost(struct task_struct *tsk);

int schedtune_prefer_idle(struct task_struct *tsk);

void schedtune_enqueue_task(struct task_struct *p, int cpu);
void schedtune_dequeue_task(struct task_struct *p, int cpu);
#ifdef CONFIG_HISI_CPU_FREQ_GOV_SCHEDUTIL
int schedtune_freq_boost(int cpu);
int schedtune_top_task(struct task_struct *tsk);
#endif

#else /* CONFIG_SCHED_TUNE */

#define schedtune_cpu_boost(cpu)  0
#define schedtune_task_boost(tsk) 0

#define schedtune_prefer_idle(tsk) 0

#define schedtune_enqueue_task(task, cpu) do { } while (0)
#define schedtune_dequeue_task(task, cpu) do { } while (0)

#ifdef CONFIG_HISI_CPU_FREQ_GOV_SCHEDUTIL
static inline int schedtune_freq_boost(int cpu) {
	return 0;
}
static inline int schedtune_top_task(struct task_struct *tsk) {
	return 0;
}
#endif

#endif /* CONFIG_SCHED_TUNE */
