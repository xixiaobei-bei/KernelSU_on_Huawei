/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: polar parameter calc for coul module
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

#ifndef _COUL_POLAR_H_
#define _COUL_POLAR_H_

#include <linux/power/hisi/coul/coul_drv.h>
#include "polar_table.h"

#define polar_debug(fmt, args...) do {} while (0)
#define polar_info(fmt, args...) pr_info("[coul_polar]" fmt, ## args)
#define polar_warn(fmt, args...) pr_warn("[coul_polar]" fmt, ## args)
#define polar_err(fmt, args...) pr_err("[coul_polar]" fmt, ## args)
#define coul_minutes(x) ((x) * 60)

struct polar_com_para {
	int ocv_soc_mv;
	int soc;
	int temp;
	int cur;
};

struct polar_sysfs_info {
	int ocv_old;
	int ori_vol;
	int ori_cur;
	int err_a;
	long vol;
	int curr_5s;
	int curr_peak;
	int curr_ua;
};

#ifdef CONFIG_COUL_POLAR
void stop_polar_sample(void);
void start_polar_sample(void);
void get_resume_polar_info(struct polar_com_para *para, int eco_ibat,
	int duration, int sample_time);
void sync_sample_info(void);
int polar_params_calculate(struct polar_calc_info *polar,
	int ocv_soc_mv, int vol_now, int cur, bool update_a);
int polar_ocv_params_calc(struct polar_calc_info *polar,
	int batt_soc_real, int temp, int cur);
bool is_polar_list_ready(void);
void polar_clear_flash_data(void);
void clear_polar_err_b(void);
struct hisi_polar_device *get_polar_dev(void);
struct polar_ocv_tbl *get_polar_ocv_lut(void);
struct polar_res_tbl *get_polar_res_lut(void);
struct polar_x_y_z_tbl *get_polar_vector_lut(void);
struct polar_learn_tbl *get_polar_learn_lut(void);
int interpolate_polar_ocv(struct polar_ocv_tbl *lut,
	int batt_temp_degc, int pc);
int interpolate_two_dimension(const struct polar_res_tbl *lut,
	int x, int y, int z);
int get_polar_vector_res(struct polar_res_tbl *lut,
	int batt_temp_degc, int soc, int curr);
int get_polar_vector_value(const struct polar_x_y_z_tbl *lut,
	int batt_temp_degc, int soc, int curr, int t_index);
long get_trained_a(const struct polar_learn_tbl *lut,
	int batt_temp_degc, int soc);
short get_trained_polar_vol(const struct polar_learn_tbl *lut,
	int batt_temp_degc, int soc);
void store_trained_a(struct polar_learn_tbl *lut,
	int batt_temp_degc, int soc, long trained_a, long polar_vol_uv);
bool could_vbat_learn_a(struct hisi_polar_device *di,
	struct polar_com_para *para, int vol_now_mv, long polar_vol_uv);
#else
static inline void clear_polar_err_b(void)
{
}

static inline void start_polar_sample(void)
{
}

static inline void sync_sample_info(void)
{
}

static inline void stop_polar_sample(void)
{
}

static inline void polar_clear_flash_data(void)
{
}

#endif
#endif

