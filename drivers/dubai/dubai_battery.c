/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: DUBAI kernel get battery info
 * Author: zhangguiwen
 * Create: 2019-01-18
 * Notes:
 * History:
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/power_supply.h>

#include <chipset_common/dubai/dubai.h>
#include <chipset_common/dubai/dubai_common.h>

enum {
	DUBAI_BATTERY_PROP_GAS_GAUGE = 0,
	DUBAI_BATTERY_PROP_VOLTAGE_NOW = 1,
	DUBAI_BATTERY_PROP_CURRENT_NOW = 2,
	DUBAI_BATTERY_PROP_CYCLE_COUNT = 3,
	DUBAI_BATTERY_PROP_CHARGE_FULL = 4,
	DUBAI_BATTERY_PROP_LEVEL = 5,
	DUBAI_BATTERY_PROP_BRAND = 6,
	DUBAI_BATTERY_PROP_CHARGE_STATUS = 7,
};

#define BATTERY_BRAND_MAX_LEN 	(32)

union dubai_battery_prop_value {
	int prop;
	int value;
	char brand[BATTERY_BRAND_MAX_LEN];
};

#define UV_PER_MV (1000)
#define DUBAI_ONE_HUNDRED (100)
#ifdef CONFIG_HISI_COUL
#define BATTERY_UNIT (1)
#define BATTERY_SUPPLY_NAME ("Battery")
extern int hisi_battery_rm(void);
#else
#define BATTERY_SUPPLY_NAME ("battery")
#define BATTERY_UNIT (1000)
#endif

#pragma GCC diagnostic ignored "-Wunused-variable"

static int dubai_get_property(struct power_supply *psy, enum power_supply_property psp, union power_supply_propval *val)
{
	int ret = 0;

	ret = power_supply_get_property(psy, psp, val);
	if (ret != 0) {
		DUBAI_LOGE("Failed to get power supply property %d %d.", psp, ret);
		return -EINVAL;
	}

	return ret;
}

static int dubai_get_charge_full(struct power_supply *psy)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_CHARGE_FULL, &propval);
	if (ret == 0) {
		return propval.intval / BATTERY_UNIT;
	}

	return ret;
}

static int dubai_get_level(struct power_supply *psy)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_CAPACITY, &propval);
	if (ret == 0) {
		return propval.intval;
	}

	return ret;
}

#ifndef CONFIG_HISI_COUL
static int dubai_get_gas_gauge(struct power_supply *psy)
{
	int full = 0;
	int level = 0;

	full = dubai_get_charge_full(psy);
	level = dubai_get_level(psy);

	return (full) * (level) / DUBAI_ONE_HUNDRED;
}
#endif

static int dubai_get_voltage(struct power_supply *psy)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &propval);
	if (ret == 0) {
		return propval.intval / UV_PER_MV;
	}

	return ret;
}

static int dubai_get_current(struct power_supply *psy)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW, &propval);
	if (ret == 0) {
		return propval.intval / BATTERY_UNIT;
	}

	return ret;
}

static int dubai_get_cycle_count(struct power_supply *psy)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_CYCLE_COUNT, &propval);
	if (ret == 0) {
		return propval.intval;
	}

	return ret;
}

static int dubai_get_status(struct power_supply *psy)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_STATUS, &propval);
	if (ret == 0) {
		DUBAI_LOGI("Get power supply status %d", propval.intval);
		if ((propval.intval == POWER_SUPPLY_STATUS_CHARGING) || (propval.intval == POWER_SUPPLY_STATUS_FULL)) {
			return 1;
		} else {
			return 0;
		}
	}

	return ret;
}

static void dubai_get_brand(struct power_supply *psy, char *brand)
{
	int ret = 0;
	union power_supply_propval propval;

	ret = dubai_get_property(psy, POWER_SUPPLY_PROP_BRAND, &propval);
	if (ret == 0) {
		strncpy(brand, propval.strval, BATTERY_BRAND_MAX_LEN - 1);
	} else {
		strncpy(brand, "Unknown", BATTERY_BRAND_MAX_LEN - 1);
	}
}

int dubai_get_battery_prop(void __user *argp) {

	int ret = 0;
	int size;
	struct power_supply *psy = NULL;
	struct dev_transmit_t *transmit = NULL;
	union dubai_battery_prop_value *propval = NULL;

	size = sizeof(struct dev_transmit_t) + sizeof(union dubai_battery_prop_value);
	transmit = kzalloc(size, GFP_KERNEL);
	if (transmit == NULL) {
		DUBAI_LOGE("Failed to allocate memory");
		ret = -ENOMEM;
		return ret;
	}
	if (copy_from_user(transmit, argp, size)) {
		DUBAI_LOGE("Failed to get argp.");
		ret = -EFAULT;
		goto exit;
	}

	propval = (union dubai_battery_prop_value *)transmit->data;
	psy = power_supply_get_by_name(BATTERY_SUPPLY_NAME);
	if (psy == NULL) {
		DUBAI_LOGE("Failed to get power supply(%s).", BATTERY_SUPPLY_NAME);
		ret = -EINVAL;
		goto exit;
	}

	switch (propval->prop) {
	case DUBAI_BATTERY_PROP_GAS_GAUGE:
#ifdef CONFIG_HISI_COUL
		propval->value = hisi_battery_rm();
#else
		propval->value = dubai_get_gas_gauge(psy);
#endif
		break;
	case DUBAI_BATTERY_PROP_VOLTAGE_NOW:
		propval->value = dubai_get_voltage(psy);
		break;
	case DUBAI_BATTERY_PROP_CURRENT_NOW:
		propval->value = dubai_get_current(psy);
		break;
	case DUBAI_BATTERY_PROP_CYCLE_COUNT:
		propval->value = dubai_get_cycle_count(psy);
		break;
	case DUBAI_BATTERY_PROP_CHARGE_FULL:
		propval->value = dubai_get_charge_full(psy);
		break;
	case DUBAI_BATTERY_PROP_LEVEL:
		propval->value = dubai_get_level(psy);
		break;
	case DUBAI_BATTERY_PROP_BRAND:
		dubai_get_brand(psy, propval->brand);
		break;
	case DUBAI_BATTERY_PROP_CHARGE_STATUS:
		propval->value = dubai_get_status(psy);
		break;
	default:
		DUBAI_LOGE("Invalid prop %d.", propval->prop);
		ret = -EINVAL;
		goto exit;
	}

	if (copy_to_user(argp, transmit, size)) {
		DUBAI_LOGE("Failed to set prop %d.", propval->prop);
		ret = -EINVAL;
		goto exit;
	}
exit:
	kfree(transmit);

	return ret;
}

