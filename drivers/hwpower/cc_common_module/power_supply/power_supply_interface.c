/*
 * power_supply_interface.c
 *
 * power supply interface for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_psy
HWLOG_REGIST();

void power_supply_sync_changed(const char *name)
{
	struct power_supply *psy = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return;
	}

	psy = power_supply_get_by_name(name);
	if (!psy) {
		hwlog_err("power supply %s not exist\n", name);
		return;
	}

	power_supply_changed(psy);
	power_supply_put(psy);
}

bool power_supply_check_psy_available(const char *name,
	struct power_supply **psy)
{
	if (!name || !psy) {
		hwlog_err("name or psy is null\n");
		return false;
	}

	if (*psy)
		return true;

	*psy = power_supply_get_by_name(name);
	if (!*psy) {
		hwlog_err("power supply %s not exist\n", name);
		return false;
	}

	power_supply_put(*psy);
	return true;
}

int power_supply_check_property_writeable(const char *name,
	enum power_supply_property psp)
{
	int ret;
	struct power_supply *psy = NULL;

	if (!name) {
		hwlog_err("name is null\n");
		return -EINVAL;
	}

	psy = power_supply_get_by_name(name);
	if (!psy) {
		hwlog_err("power supply %s not exist\n", name);
		return -EINVAL;
	}

	ret = power_supply_property_is_writeable(psy, psp);
	power_supply_put(psy);

	return ret;
}

int power_supply_get_property_value_with_psy(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int ret;

	if (!psy || !val) {
		hwlog_err("psy or val is null\n");
		return -EINVAL;
	}

	ret = power_supply_get_property(psy, psp, val);
	if (ret) {
		hwlog_err("get %u property fail\n", psp);
		return ret;
	}

	return 0;
}

int power_supply_set_property_value_with_psy(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	int ret;

	if (!psy || !val) {
		hwlog_err("psy or val is null\n");
		return -EINVAL;
	}

	ret = power_supply_set_property(psy, psp, val);
	if (ret) {
		hwlog_err("set %u property fail\n", psp);
		return ret;
	}

	return 0;
}

int power_supply_get_int_property_value_with_psy(struct power_supply *psy,
	enum power_supply_property psp, int *val)
{
	int ret;
	union power_supply_propval union_val;

	if (!psy || !val) {
		hwlog_err("psy or val is null\n");
		return -EINVAL;
	}

	ret = power_supply_get_property_value_with_psy(psy, psp, &union_val);
	if (!ret)
		*val = union_val.intval;

	return ret;
}

int power_supply_set_int_property_value_with_psy(struct power_supply *psy,
	enum power_supply_property psp, int val)
{
	union power_supply_propval union_val;

	if (!psy) {
		hwlog_err("psy is null\n");
		return -EINVAL;
	}

	union_val.intval = val;
	return power_supply_set_property_value_with_psy(psy, psp, &union_val);
}

int power_supply_return_int_property_value_with_psy(struct power_supply *psy,
	enum power_supply_property psp)
{
	int ret;
	union power_supply_propval union_val;

	if (!psy) {
		hwlog_err("psy is null\n");
		return -EINVAL;
	}

	ret = power_supply_get_property_value_with_psy(psy, psp, &union_val);
	if (!ret)
		return union_val.intval;

	return -EINVAL;
}

int power_supply_get_property_value(const char *name,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int ret;
	struct power_supply *psy = NULL;

	if (!name || !val) {
		hwlog_err("name or val is null\n");
		return -EINVAL;
	}

	psy = power_supply_get_by_name(name);
	if (!psy) {
		hwlog_err("power supply %s not exist\n", name);
		return -EINVAL;
	}

	ret = power_supply_get_property(psy, psp, val);
	power_supply_put(psy);

	return ret;
}

int power_supply_set_property_value(const char *name,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	int ret;
	struct power_supply *psy = NULL;

	if (!name || !val) {
		hwlog_err("name or val is null\n");
		return -EINVAL;
	}

	psy = power_supply_get_by_name(name);
	if (!psy) {
		hwlog_err("power supply %s not exist\n", name);
		return -EINVAL;
	}

	ret = power_supply_set_property(psy, psp, val);
	power_supply_put(psy);

	return ret;
}

int power_supply_get_int_property_value(const char *name,
	enum power_supply_property psp, int *val)
{
	int ret;
	union power_supply_propval union_val;

	if (!name || !val) {
		hwlog_err("name or val is null\n");
		return -EINVAL;
	}

	ret = power_supply_get_property_value(name, psp, &union_val);
	if (!ret)
		*val = union_val.intval;

	return ret;
}

int power_supply_set_int_property_value(const char *name,
	enum power_supply_property psp, int val)
{
	union power_supply_propval union_val;

	if (!name) {
		hwlog_err("name is null\n");
		return -EINVAL;
	}

	union_val.intval = val;
	return power_supply_set_property_value(name, psp, &union_val);
}

int power_supply_get_str_property_value(const char *name,
	enum power_supply_property psp, char *val, unsigned int len)
{
	int ret;
	union power_supply_propval union_val;
	unsigned int str_len;

	if (!name || !val) {
		hwlog_err("name or val is null\n");
		return -EINVAL;
	}

	ret = power_supply_get_property_value(name, psp, &union_val);
	if (!ret) {
		str_len = strlen(union_val.strval);
		if (str_len > len)
			strncat(val, union_val.strval, len);
		else
			strncat(val, union_val.strval, str_len);
	}

	return ret;
}
