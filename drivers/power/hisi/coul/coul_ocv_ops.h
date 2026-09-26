/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ocv operation functions for coul module
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

#ifndef _COUL_OCV_OPS_H_
#define _COUL_OCV_OPS_H_

#include "coul_core.h"

#define dbg_cnt_inc(xxx)        (di->xxx++)

enum avg_curr_level {
	AVG_CURR_250MA = 0,
	AVG_CURR_500MA,
	AVG_CURR_MAX,
};

void judge_eco_leak_uah(int soc);
void set_ocv_cali_level(unsigned char last_ocv_level);
void update_ocv_cali_rbatt(struct smartstar_coul_device *di, int avg_c,
	int rbatt_calc);
void record_ocv_cali_info(struct smartstar_coul_device *di);
int coul_dsm_report_ocv_cali_info(struct smartstar_coul_device *di,
	int err_num, const char *buff);
void coul_clear_cc_register(void);
void coul_clear_coul_time(void);
void coul_get_initial_ocv(struct smartstar_coul_device *di);
void get_ocv_by_fcc(struct smartstar_coul_device *di);
int get_ocv_vol_from_fifo(struct smartstar_coul_device *di);

#ifdef CONFIG_PM
void record_wakeup_info(struct smartstar_coul_device *di, int wakeup_time);
void record_sleep_info(int sr_sleep_time);
void update_ocv_after_resume(struct smartstar_coul_device *di);

#endif
#endif

