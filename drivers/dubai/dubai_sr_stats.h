#ifndef DUBAI_SR_STATS_H
#define DUBAI_SR_STATS_H

#include <chipset_common/dubai/dubai_wakeup.h>

long dubai_ioctl_sr(unsigned int cmd, void __user *argp);
void dubai_sr_stats_init(void);
void dubai_sr_stats_exit(void);
int dubai_wakeup_register_ops(struct dubai_wakeup_stats_ops *op);
int dubai_wakeup_unregister_ops(void);

#endif // DUBAI_SR_STATS_H
