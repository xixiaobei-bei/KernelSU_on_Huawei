/*
 * battery_temp_fitting.h
 *
 * battery temperature fitting head file
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_TEMP_FITTING_
#define _BATTERY_TEMP_FITTING_

#include <linux/types.h>
#include <linux/workqueue.h>

#define BTF_TEMP_LTH                      (-5) /* -5 degree */
#define BTF_DO_LINE_FIT_TEMP_GAP          10
#define BTF_DSM_BUFF_SIZE_MAX             128
#define BTF_RESOULUTION                   10000
#define BTF_IMPOSSIBE_IBAT                1000000 /* 1000A */
#define BTF_IMPOSSIBLE_CHG_PWR            1000000000 /* 1kw */
#define MDEGREE_P_DEGREE                  1000

/* Wait 3s for thermal_zone being ready */
#define BTF_WAIT_BTF_READY                3000

/* Linear fitting */
#define BTF_MAX_LINEAR_FIT_CASE           8
#define BTF_MAX_LINEAR_COEF_NUM           8
#define BTF_DMD_REPORT_FIT_TIME_MAX       30
#define BTF_TZ_GROUP_SELECT_TH            10

#define BTF_TEMP_PARA_LEVEL               10

enum btf_mode {
	BTF_MODE_BEGIN = 0,
	BTF_MODE_LINEAR,
};

enum btf_tz_group {
	BTF_TZ_GROUP_ILLEGAL = -1,
	BTF_TZ_GROUP_USB_CHARGER,
	BTF_TZ_GROUP_CHARGER_RFBOARD,
	BTF_TZ_GROUP_USB_RFBOARD,
	BTF_TZ_GROUP_END,
};

struct btf_tz_group_lkup_tbl {
	enum btf_tz_group tz_group;
	const char *tz_name1; /* sensor name */
	const char *tz_name2; /* sensor name */
};

struct btf_tz_temp {
	int temp;
	const char *name; /* sensor name */
};

struct btf_linear_para {
	int coef_num;
	int tz_group_num;
	int coef[BTF_MAX_LINEAR_FIT_CASE][BTF_MAX_LINEAR_COEF_NUM];
	const char *tz_name[BTF_MAX_LINEAR_COEF_NUM - 1]; /* sensor name */
};

struct btf_charge_temp_data {
	int temp_min;
	int temp_max;
	int iin_temp;
	int ichg_temp;
	int vterm_temp;
	int temp_back;
};

struct btf_dev {
	struct device *dev;
	int t_wait_for_ready;
	int fitting_mode;
	int fake_btf_val;
	int fake_btf_by_unready;
	int dc_i_limit;
	int wldc_pwr_limit;
	bool ready;
	bool dmd_reported;
	bool bat_temp_fitted;
	enum btf_tz_group tz_group;
	struct btf_linear_para linear_fit_para;
	struct btf_charge_temp_data temp_para[BTF_TEMP_PARA_LEVEL];
	struct delayed_work check_ready;
};

#ifdef CONFIG_HUAWEI_BATTERY_TEMP_FITTING
bool btf_temp_fitted_eqr(void);
int btf_try_fit(int temp_in);
struct btf_charge_temp_data *btf_charge_temp_data_eqr(void);
#else
static inline bool btf_temp_fitted_eqr(void)
{
	return false;
}

static inline int btf_try_fit(int temp_in)
{
	return temp_in;
}

static inline struct btf_charge_temp_data *btf_charge_temp_data_eqr(void)
{
	return NULL;
}
#endif /* CONFIG_HUAWEI_BATTERY_TEMP_FITTING */

#endif /* _BATTERY_TEMP_FITTING_ */
