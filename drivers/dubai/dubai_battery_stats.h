#ifndef DUBAI_BATTERY_STATS_H
#define DUBAI_BATTERY_STATS_H

#include <chipset_common/dubai/dubai_battery.h>

#define BATTERY_STR_VAL_MAX_LEN 	32

union dubai_battery_prop_value {
	int prop;
	int value;
	char strval[BATTERY_STR_VAL_MAX_LEN];
} __packed;

long dubai_ioctl_battery(unsigned int cmd, void __user *argp);
void dubai_battery_stats_exit(void);
int dubai_battery_register_ops(struct dubai_battery_stats_ops *ops);
int dubai_battery_unregister_ops(void);

#endif // DUBAI_BATTERY_STATS_H
