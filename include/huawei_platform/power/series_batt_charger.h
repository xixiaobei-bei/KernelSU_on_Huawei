/*
 * series_batt_charger.h
 *
 * series batt charger driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _SERIES_BATT_CHARGER_H_
#define _SERIES_BATT_CHARGER_H_

#define CHARGERLOG_SIZE  256
#define CHARGE_ENABLE    1
#define CHARGE_DISABLE   0
#define DEFAULT_VALUE    0

enum series_batt_chgstate {
	SERIES_BATT_CHG_DISABLE,
	SERIES_BATT_CHG_EN,
	SERIES_BATT_HIZ_EN,
	SERIES_BATT_HIZ_DISABLE,
	SERIES_BATT_CHG_DONE,
};

#ifdef CONFIG_SERIES_BATT
int series_batt_ops_register(struct charge_device_ops *ops);
int get_series_batt_chargelog_head(char *chargelog, int size);
int get_series_batt_chargelog(char *chargelog, int size);
int series_batt_set_charge_en(int enable);
int series_batt_set_term_curr(int value);
int series_batt_get_term_curr(void);
int series_batt_chg_state_notifier_register(struct notifier_block *nb);
int series_batt_chg_state_notifier_unregister(struct notifier_block *nb);

#else

static inline int series_batt_ops_register(struct charge_device_ops *ops)
{
	return 0;
}

static inline int get_series_batt_chargelog_head(char *chargelog, int size)
{
	return 0;
}

static inline int get_series_batt_chargelog(char *chargelog, int size)
{
	return 0;
}

static inline int series_batt_set_charge_en(int enable)
{
	return 0;
}

static inline int series_batt_set_term_curr(int value)
{
	return 0;
}

static inline int series_batt_get_term_curr(void)
{
	return 0;
}

static inline int series_batt_chg_state_notifier_register(
	struct notifier_block *nb)
{
	return 0;
}

static inline int series_batt_chg_state_notifier_unregister(
	struct notifier_block *nb)
{
	return 0;
}

#endif /* CONFIG_SERIES_BATT */

#endif /* _SERIES_BATT_CHARGER_H_ */
