#ifndef DUBAI_DDR_STATS_H
#define DUBAI_DDR_STATS_H

#include <chipset_common/dubai/dubai_ddr.h>

void dubai_ddr_stats_init(void);
void dubai_ddr_stats_exit(void);
int dubai_ddr_register_ops(struct dubai_ddr_stats_ops *op);
int dubai_ddr_unregister_ops(void);
long dubai_ioctl_ddr(unsigned int cmd, void __user *argp);

#endif // DUBAI_DDR_STATS_H
