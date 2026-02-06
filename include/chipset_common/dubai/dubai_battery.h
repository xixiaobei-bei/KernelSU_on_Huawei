#ifndef DUBAI_BATTERY_H
#define DUBAI_BATTERY_H

#include <linux/power_supply.h>
#include <linux/types.h>

typedef int (*dubai_battery_get_capacity_rm_t)(void);

struct dubai_battery_stats_ops {
	const char *psy_name;
	int charge_full_amp;
	dubai_battery_get_capacity_rm_t get_capacity_rm;
};

#ifdef CONFIG_HUAWEI_DUBAI
int dubai_get_psy_intprop(struct power_supply *psy, enum power_supply_property psp, int defval);
int dubai_get_psy_strprop(struct power_supply *psy, enum power_supply_property psp, char *buf, size_t len);
#else // !CONFIG_HUAWEI_DUBAI
static inline int dubai_get_psy_intprop(struct power_supply *psy, enum power_supply_property psp, int defval)
{
	return defval;
}

static inline int dubai_get_psy_strprop(struct power_supply *psy, enum power_supply_property psp,
	char *buf, size_t len)
{
	return -1;
}
#endif // CONFIG_HUAWEI_DUBAI

#endif // DUBAI_BATTERY_H
