/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: nv operation functions fpr coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_NV_H_
#define _COUL_NV_H_

#include "coul_core.h"

#define DEFAULT_V_OFF_A         1014000
#define DEFAULT_V_OFF_B         0
#define DEFAULT_C_OFF_A         1000000
#define DEFAULT_C_OFF_B         0

struct hw_coul_nv_info {
	int charge_cycles;
	short temp[MAX_TEMPS];
	short real_fcc[MAX_TEMPS];
	/* should reserved buf for removed asw nv to prevent use dirty data */
	short reserved[NV_RESERVE_LEN];
};

void get_coul_cali_params(int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b);
void set_coul_cali_params(int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b);
int save_cali_param(int cur_cal_temp);
unsigned long get_nv_info_addr(void);
struct hw_coul_nv_info get_batt_backup_nv_info(void);
int get_initial_params(struct smartstar_coul_device *di);
void hw_coul_get_nv(struct smartstar_coul_device *di);
int save_nv_info(struct smartstar_coul_device *di);
void hw_coul_update_chargecycles(struct smartstar_coul_device *di);
#endif

