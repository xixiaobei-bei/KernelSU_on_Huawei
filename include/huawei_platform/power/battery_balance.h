/*
 * battery_balance.h
 *
 * battery balance driver
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

#ifndef _BATTERY_BALANCE_H_
#define _BATTERY_BALANCE_H_

enum bal_batt_id {
	BATT_ID_0 = 0,
	BATT_ID_1 = 1,
};

enum bal_mode {
	BAL_MODE_CHG,
	BAL_MODE_PULSE,
	BAL_MODE_DISCHG,
	BAL_MODE_MAX,
};

enum bal_state {
	BAL_STATE_BALANCE,
	BAL_STATE_RELAX,
	BAL_STATE_IDLE,
};

enum bal_event_type {
	BAL_STOP,
	BAL_START_FAC,
	BAL_START_CHG,
	BAL_START_PULSE,
	BAL_DONE_FAC,
	BAL_DONE_CHG,
	BAL_DONE_PULSE,
	BAL_E_MAX,
};

struct bal_info {
	enum bal_state bal_state;
	enum bal_mode bal_mode;
	enum bal_batt_id bat_id;
	int signal_time;
	int sum_time;
	s64 signal_gauge;
	s64 sum_gauge;
};

#ifdef CONFIG_BATTERY_BALANCE
int batt_bal_state_notifier_register(struct notifier_block *nb);
int batt_bal_state_notifier_unregister(struct notifier_block *nb);
int batt_bal_get_info(struct bal_info *info);
void batt_bal_reset_info(void);

#else

static inline int batt_bal_state_notifier_register(
	struct notifier_block *nb)
{
	return 0;
}

static inline int batt_bal_state_notifier_unregister(
	struct notifier_block *nb)
{
	return 0;
}

static inline int batt_bal_get_info(struct bal_info *info)
{
	return -EIO;
}

static inline void batt_bal_reset_info(void)
{
}
#endif /* CONFIG_BATTERY_BALANCE */

#endif /* _BATTERY_BALANCE_H_ */
