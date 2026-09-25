/*
 * battery_asw.h
 *
 * Driver adapter for asw.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_ASW_H_
#define _BATTERY_ASW_H_

#include <huawei_platform/power/battery_soh.h>

struct asw_data {
	unsigned int time_speed;
	unsigned int enable_stub;
	unsigned int charger_type;
	int bat_voltage;
	int bat_temp;
};

void asw_dmd_content_prepare(char *buff, unsigned int size);
int asw_sysfs_create_group(struct bsoh_device *di);
void asw_sysfs_remove_group(struct bsoh_device *di);

#endif /* _BATTERY_ASW_H_ */
