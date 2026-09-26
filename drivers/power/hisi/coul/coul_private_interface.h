/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: private interface for coul module
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

#ifndef _COUL_PRIVATE_INTERFACE_H_
#define _COUL_PRIVATE_INTERFACE_H_

#include <linux/power/hisi/coul/coul_event.h>
#include "coul_core.h"
#ifdef CONFIG_COUL_POLAR
#include "coul_polar.h"
#endif

struct static_soc_data {
	int cali_cnt;
	int charged_cnt;
	int last_cc;
	int last_time;
	int charging_done_ocv_enter_time;
};

struct soc_param_data {
	int fcc_uah;
	int unusable_charge_uah;
	int remaining_charge_uah;
	int cc_uah;
	int delta_rc_uah;
	int rbatt;
};

unsigned int coul_get_pd_charge_flag(void);
int coul_get_battery_temperature(void);
int coul_is_battery_exist(void);
int coul_get_battery_capacity(void);
void coul_get_battery_voltage_and_current(
	struct smartstar_coul_device *di, int *ibat_ua, int *vbat_uv);
int coul_get_battery_voltage_mv(void);
int coul_get_battery_current_ma(void);
int coul_battery_unfiltered_capacity(void);
int coul_get_battery_rm(void);
int coul_get_battery_fcc(void);
int coul_get_battery_uuc(void);
int coul_get_battery_cc(void);
int coul_get_battery_delta_rc(void);
int coul_get_battery_ocv(void);
int coul_get_battery_resistance(void);
int coul_core_ops_register(struct coul_device_ops *ops);
int coul_get_chip_temp(void);
void coul_cali_adc(struct smartstar_coul_device *di);
void get_last_cali_info(int *last_cali_temp, int *last_cali_time);
int coul_get_fifo_avg_current_ma(void);
int is_between(int left, int right, int value);
int linear_interpolate(int y0, int x0, int y1, int x1, int x);
int interpolate_single_lut(const struct single_row_lut *lut, int x);
int interpolate_single_y_lut(const struct single_row_lut *lut, int y);
int interpolate_scalingfactor(const struct sf_lut *sf_lut,
	int row_entry, int pc);
int interpolate_fcc(struct smartstar_coul_device *di, int batt_temp);
int calculate_fcc_uah(struct smartstar_coul_device *di);
int interpolate_ocv(const struct pc_temp_ocv_lut *lut,
	int batt_temp_degc, int pc);
int interpolate_pc_high_precision(const struct pc_temp_ocv_lut *lut,
	int batt_temp, int ocv);
int interpolate_pc(const struct pc_temp_ocv_lut *lut, int batt_temp, int ocv);
int calculate_pc(struct smartstar_coul_device *di, int ocv_uv,
	int batt_temp, int chargecycles);
int get_rbatt(struct smartstar_coul_device *di, int soc_rbatt, int batt_temp);
int basp_full_pc_by_voltage(struct smartstar_coul_device *di);
int coul_get_qmax(struct smartstar_coul_device *di);
int coul_get_battery_qmax(void);
int battery_para_changed(struct smartstar_coul_device *di);
void force_ocv_update(void);
bool could_cc_update_ocv(void);
bool is_in_capacity_dense_area(int ocv_uv);
int get_timestamp(char *str, int len);
void batt_coul_notify(struct smartstar_coul_device *di,
	enum batt_coul_event event);
void coul_start_soh_check(void);
void coul_set_low_vol_int(struct smartstar_coul_device *di, int state);
void get_battery_id_voltage(struct smartstar_coul_device *di);
int bound_soc(int soc);
void check_chg_done_max_avg_cur_flag(struct smartstar_coul_device *di);
void clear_moved_battery_data(struct smartstar_coul_device *di);
int probe_check_coul_dev_ops(struct smartstar_coul_device *di);
int check_batt_data(struct smartstar_coul_device *di);
int get_zero_cap_vol(struct smartstar_coul_device *di);
void calculate_iavg_ma(struct smartstar_coul_device *di, int iavg_ua);
void coul_set_work_interval(struct smartstar_coul_device *di);
#ifdef CONFIG_COUL_POLAR
void set_info_to_update_polar_info(struct smartstar_coul_device *di,
	struct info_to_update_polar_info *info, int sr_sleep_time, int sleep_cc);
#endif
#endif
