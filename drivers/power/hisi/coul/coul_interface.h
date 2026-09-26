/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: interface for coul module
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

#ifndef _COUL_INTERFACE_H_
#define _COUL_INTERFACE_H_

#include "coul_core.h"

void get_battery_id_voltage_real(struct smartstar_coul_device *di);
int coul_get_battery_voltage_uv(void);
int coul_get_polar_table_val(int temp, int soc);
struct coulometer_ops *get_coul_ops(void);
bool check_coul_dev(void);
unsigned int get_nondc_vol_dec(void);
int get_coul_dev_batt_temp(void);
int update_coul_dev_qmax(void);
int get_coul_dev_batt_rm(void);
int get_coul_dev_batt_fcc(void);
int get_coul_dev_qmax(void);
int get_coul_dev_batt_chargecycles(void);
int get_coul_dev_batt_ruc(void);
s64 get_iscd_full_update_cc(void);
#endif

