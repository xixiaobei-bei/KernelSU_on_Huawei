/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: temp functions for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _COUL_TEMP_H_
#define _COUL_TEMP_H_

#include "coul_core.h"

void contexthub_thermal_init(void);
void get_temp_dts(struct device_node *np);
int coul_convert_temp_to_adc(int temp);
int coul_retry_temp_permille(enum battery_temp_user user);
int coul_battery_temperature_tenth_degree(enum battery_temp_user user);
int get_temperature_stably(struct smartstar_coul_device *di,
	enum battery_temp_user user);
void update_battery_temperature(struct smartstar_coul_device *di,
	int status);

#endif
