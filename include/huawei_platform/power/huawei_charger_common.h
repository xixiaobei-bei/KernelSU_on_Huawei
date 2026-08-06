/*
 * huawei_charger_common.h
 *
 * common interface for huawei charger driver
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

#ifndef _HUAWEI_CHARGER_COMMON_H_
#define _HUAWEI_CHARGER_COMMON_H_

#include <linux/types.h>

struct charge_extra_ops {
	bool (*check_ts)(void);
	bool (*check_otg_state)(void);
	enum fcp_check_stage_type (*get_stage)(void);
	enum huawei_usb_charger_type (*get_charger_type)(void);
	int (*set_state)(int state);
	int (*get_charge_current)(void);
};

int charge_extra_ops_register(struct charge_extra_ops *ops);

int get_charger_vbus_vol(void);
int get_charger_ibus_curr(void);
int charge_check_input_dpm_state(void);
int charge_check_charger_plugged(void);

bool charge_check_charger_ts(void);
bool charge_check_charger_otg_state(void);
enum fcp_check_stage_type fcp_get_stage_status(void);
enum huawei_usb_charger_type charge_get_charger_type(void);
int charge_set_charge_state(int state);
int get_charge_current_max(void);

#endif /* _HUAWEI_CHARGER_COMMON_H_ */
