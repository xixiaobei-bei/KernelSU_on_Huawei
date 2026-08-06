#ifndef DUBAI_MISC_STATS_H
#define DUBAI_MISC_STATS_H

long dubai_ioctl_misc(unsigned int cmd, void __user *argp);
void dubai_misc_stats_init(void);
void dubai_misc_stats_exit(void);

#endif // DUBAI_MISC_STATS_H
