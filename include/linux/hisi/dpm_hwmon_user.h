/*
 * dpm_hwmon_user.h
 *
 * dpm user interface
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
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

#ifndef __DPM_HWMON_USER_H__
#define __DPM_HWMON_USER_H__

#include <linux/hisi/dpm_hwmon.h>

#define DPM_NAME_SIZE		64

#pragma pack(1)
struct ip_energy {
	char name[DPM_NAME_SIZE];
	unsigned long long energy;
};

struct dubai_transmit_t {
	unsigned int length;
	char data[0];
};
#pragma pack()

unsigned long long get_dpm_chdmod_power(int dpm_id);
void dpm_parse_switch_cmd(unsigned int cmd);
unsigned int dpm_peri_num(void);
int get_dpm_peri_data(struct dubai_transmit_t *peri_data,
		      unsigned int peri_num);
/* interface for its */
void update_dpm_power(int dpm_id);
bool check_dpm_enabled(int dpm_type);
#endif
