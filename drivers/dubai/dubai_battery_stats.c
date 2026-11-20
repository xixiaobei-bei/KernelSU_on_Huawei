#include "dubai_battery_stats.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/power_supply.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#include "utils/dubai_utils.h"

#define IOC_BATTERY_PROP DUBAI_BATTERY_DIR_IOC(WR, 1, union dubai_battery_prop_value)

enum {
	DUBAI_BATTERY_PROP_CAPACITY_RM = 0,
	DUBAI_BATTERY_PROP_VOLTAGE_NOW = 1,
	DUBAI_BATTERY_PROP_CURRENT_NOW = 2,
	DUBAI_BATTERY_PROP_CYCLE_COUNT = 3,
	DUBAI_BATTERY_PROP_CHARGE_FULL = 4,
	DUBAI_BATTERY_PROP_CAPACITY = 5,
	DUBAI_BATTERY_PROP_BRAND = 6,
	DUBAI_BATTERY_PROP_CHARGE_STATUS = 7,
	DUBAI_BATTERY_PROP_HEALTH = 8,
	DUBAI_BATTERY_PROP_TEMP = 9,
};

#define UV_PER_MV					1000
#define DEFAULT_CHARGE_FULL_AMP		1000
#define DEFAULT_PSY_BATTERY_NAME	"battery"

static int dubai_get_battery_prop(void __user *argp);
static int dubai_get_capacity_rm_default(void);
static struct dubai_battery_stats_ops dubai_batt_ops_default = {
	.psy_name = DEFAULT_PSY_BATTERY_NAME,
	.charge_full_amp = DEFAULT_CHARGE_FULL_AMP,
	.get_capacity_rm = dubai_get_capacity_rm_default,
};
static struct power_supply *g_batt_psy = NULL;
static struct dubai_battery_stats_ops *g_batt_ops = &dubai_batt_ops_default;

long dubai_ioctl_battery(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_BATTERY_PROP:
		rc = dubai_get_battery_prop(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static bool dubai_check_psy(void)
{
	if (g_batt_psy)
		return true;

	if (!g_batt_ops || !g_batt_ops->psy_name) {
		dubai_err("Battery ops or psy name is null");
		return false;
	}

	g_batt_psy = power_supply_get_by_name(g_batt_ops->psy_name);
	if (!g_batt_psy) {
		dubai_err("Failed to get power supply: %s", g_batt_ops->psy_name);
		return false;
	}
	dubai_info("Get power supply: %s", g_batt_ops->psy_name);
	return true;
}

static int dubai_get_property(struct power_supply *psy, enum power_supply_property psp,
	union power_supply_propval *val)
{
	int ret;

	ret = power_supply_get_property(psy, psp, val);
	if (ret != 0) {
		dubai_err("Failed to get power supply property: %d, ret: %d", psp, ret);
		return -1;
	}
	return 0;
}

int dubai_get_psy_strprop(struct power_supply *psy, enum power_supply_property psp,
	char *buf, size_t len)
{
	int ret;
	union power_supply_propval propval;

	if (!psy || !buf)
		return -1;

	ret = dubai_get_property(psy, psp, &propval);
	if (ret == 0)
		strncpy(buf, propval.strval, len - 1);

	return ret;
}

int dubai_get_psy_intprop(struct power_supply *psy, enum power_supply_property psp, int defval)
{
	int ret;
	union power_supply_propval propval;

	if (!psy)
		return defval;

	ret = dubai_get_property(psy, psp, &propval);
	return (ret == 0) ? propval.intval : defval;
}

static int dubai_get_charge_full(void)
{
	int amp = g_batt_ops ? g_batt_ops->charge_full_amp : DEFAULT_CHARGE_FULL_AMP;
	return dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_CHARGE_FULL, -1) / amp;
}

static int dubai_get_capacity(void)
{
	return dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_CAPACITY, -1);
}

static int dubai_get_capacity_rm_default(void)
{
	int full, level;

	full = dubai_get_charge_full();
	level = dubai_get_capacity();

	return (full * level / 100); // 100 is used for normalizing
}

static int dubai_get_capacity_rm(void)
{
	int rm = -1;

	if (g_batt_ops && g_batt_ops->get_capacity_rm)
		rm = g_batt_ops->get_capacity_rm();
	if (rm <= 0)
		rm = dubai_get_capacity_rm_default();

	return rm;
}

static int dubai_get_charge_status(void)
{
	int val;

	val = dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_STATUS, POWER_SUPPLY_STATUS_UNKNOWN);
	dubai_info("Get power supply status: %d", val);
	return (((val == POWER_SUPPLY_STATUS_CHARGING) || (val == POWER_SUPPLY_STATUS_FULL)) ? 1 : 0);
}

static void dubai_get_brand(char *brand, size_t len)
{
	int ret;

	ret = dubai_get_psy_strprop(g_batt_psy, POWER_SUPPLY_PROP_BRAND, brand, len);
	if (ret != 0)
		strncpy(brand, "Unknown", len - 1);
	brand[len - 1] = '\0';
}

static int dubai_get_battery_prop(void __user *argp) {

	int ret = 0;
	union dubai_battery_prop_value propval;

	if (copy_from_user(&propval, argp, sizeof(union dubai_battery_prop_value))) {
		dubai_err("Failed to get argp");
		return -EFAULT;
	}
	if (!dubai_check_psy())
		return -EINVAL;

	switch (propval.prop) {
	case DUBAI_BATTERY_PROP_CAPACITY_RM:
		propval.value = dubai_get_capacity_rm();
		break;
	case DUBAI_BATTERY_PROP_VOLTAGE_NOW:
		propval.value = dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, -1) / UV_PER_MV;
		break;
	case DUBAI_BATTERY_PROP_CURRENT_NOW:
		propval.value = dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_CURRENT_NOW, -1);
		break;
	case DUBAI_BATTERY_PROP_CYCLE_COUNT:
		propval.value = dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_CYCLE_COUNT, -1);
		break;
	case DUBAI_BATTERY_PROP_CHARGE_FULL:
		propval.value = dubai_get_charge_full();
		break;
	case DUBAI_BATTERY_PROP_CAPACITY:
		propval.value = dubai_get_capacity();
		break;
	case DUBAI_BATTERY_PROP_BRAND:
		dubai_get_brand(propval.strval, BATTERY_STR_VAL_MAX_LEN);
		break;
	case DUBAI_BATTERY_PROP_CHARGE_STATUS:
		propval.value = dubai_get_charge_status();
		break;
	case DUBAI_BATTERY_PROP_HEALTH:
		propval.value = dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_HEALTH, -1);
		break;
	case DUBAI_BATTERY_PROP_TEMP:
		propval.value = dubai_get_psy_intprop(g_batt_psy, POWER_SUPPLY_PROP_TEMP, -1);
		break;
	default:
		dubai_err("Invalid prop %d", propval.prop);
		ret = -EINVAL;
		break;
	}
	if (copy_to_user(argp, &propval, sizeof(union dubai_battery_prop_value))) {
		dubai_err("Failed to set prop: %d", propval.prop);
		return -EFAULT;
	}

	return ret;
}

static void dubai_battery_reset(void)
{
	if (g_batt_psy) {
		power_supply_put(g_batt_psy);
		g_batt_psy = NULL;
	}
	g_batt_ops = NULL;
}

int dubai_battery_register_ops(struct dubai_battery_stats_ops *ops)
{
	if (!ops)
		return -EINVAL;

	g_batt_ops = ops;
	return 0;
}

int dubai_battery_unregister_ops(void)
{
	dubai_battery_reset();
	return 0;
}

void dubai_battery_stats_exit(void)
{
	dubai_battery_reset();
}
