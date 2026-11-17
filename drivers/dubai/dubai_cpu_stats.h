#ifndef DUBAI_CPU_STATS_H
#define DUBAI_CPU_STATS_H

#include <linux/sched.h>

long dubai_ioctl_cpu(unsigned int cmd, void __user *argp);
void dubai_proc_cputime_init(void);
void dubai_proc_cputime_exit(void);

#endif // DUBAI_CPU_STATS_H
