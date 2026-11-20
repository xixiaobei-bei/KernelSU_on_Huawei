#ifndef DUBAI_PERI_STATS_H
#define DUBAI_PERI_STATS_H

#include <chipset_common/dubai/dubai_peri.h>

void dubai_peri_stats_init(void);
void dubai_peri_stats_exit(void);
int dubai_peri_register_ops(struct dubai_peri_stats_ops *op);
int dubai_peri_unregister_ops(void);
long dubai_ioctl_peri(unsigned int cmd, void __user *argp);

#endif // DUBAI_PERI_STATS_H
