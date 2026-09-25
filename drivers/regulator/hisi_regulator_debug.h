/*
 * hisi_regulator_debug.h
 *
 * regulator for regualtor trace
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HISI_REGULATOR_DEBUG_H
#define __HISI_REGULATOR_DEBUG_H

#include <linux/regulator/driver.h>

#ifdef CONFIG_HISI_REGULATOR_TRACE
enum track_regulator_type {
	TRACK_ON_OFF,
	TRACK_VOL,
	TRACK_MODE,
	TRACK_REGULATOR_MAX
};

void track_regulator_onoff(struct regulator_dev *rdev,
	enum track_regulator_type track_item);
void track_regulator_set_vol(struct regulator_dev *rdev,
	enum track_regulator_type track_item, int max_uv, int min_uv);
void track_regulator_set_mode(struct regulator_dev *rdev,
	enum track_regulator_type track_item, u8 mode);
#endif

extern struct list_head *get_regulator_list(void);

#ifdef CONFIG_HISI_PMIC_DEBUG
void get_current_regulator_dev(struct seq_file *s);
void set_regulator_state(const char *ldo_name, int value);
void get_regulator_state(const char *ldo_name, int length);
int set_regulator_voltage(const char *ldo_name, unsigned int vol_value);
#endif

#endif

