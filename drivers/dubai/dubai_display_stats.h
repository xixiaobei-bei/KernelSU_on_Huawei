#ifndef DUBAI_DISPLAY_STATS_H
#define DUBAI_DISPLAY_STATS_H

#include <chipset_common/dubai/dubai_display.h>

long dubai_ioctl_display(unsigned int cmd, void __user *argp);
void dubai_display_stats_init(void);

int dubai_display_register_ops(struct dubai_display_hist_ops *op);
int dubai_display_unregister_ops(void);

#endif // DUBAI_DISPLAY_STATS_H
