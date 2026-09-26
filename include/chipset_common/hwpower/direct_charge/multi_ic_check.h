/*
 * multi_ic_check.h
 *
 * multi ic check interface for power module
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

#ifndef _MULTI_IC_CHECK_H_
#define _MULTI_IC_CHECK_H_

#include <linux/device.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>

#define MULTI_IC_CHECK_ERR_CNT_MAX     4
#define MULTI_IC_CHECK_TIMEOUT         30
#define MULTI_IC_INFO_IBAT_TH_DEFAULT  7000
#define MULTI_IC_CHECK_VBAT_LOW_TH     2500
#define MULTI_IC_CHEKC_VBAT_HIGH_TH    4700

enum multi_ic_error_type {
	MULTI_IC_ERROR_TYPE_INFO = 0,
	MULTI_IC_ERROR_TYPE_IBUS,
	MULTI_IC_ERROR_TYPE_VBAT,
	MULTI_IC_ERROR_TYPE_TBAT,
};

/* multi ic dmd info */
enum multi_ic_dmd_level {
	MULTI_IC_DMD_LEVEL_BEGIN = 0,
	MULTI_IC_DMD_LEVEL_INFO = MULTI_IC_DMD_LEVEL_BEGIN,
	MULTI_IC_DMD_LEVEL_WARNING,
	MULTI_IC_DMD_LEVEL_ERROR,
	MULTI_IC_DMD_LEVEL_END,
};

enum mult_ic_dsm_type {
	MULTI_IC_VBAT_ERROR_TYPE,
	MULTI_IC_IBAT_RATIO_TYPE,
	MULTI_IC_TBAT_ERROR_TYPE,
	MULTI_IC_MULTI_IC_DSM_TYPE_MAX,
};

#define MULTI_IC_CURR_RATIO_PARA_LEVEL   5

enum multi_ic_current_ratio_info {
	MULTI_IC_CURR_RATIO_ERR_CHECK_CNT,
	MULTI_IC_CURR_RATIO_MIN,
	MULTI_IC_CURR_RATIO_MAX,
	MULTI_IC_CURR_RATIO_DMD_LEVEL,
	MULTI_IC_CURR_RATIO_LIMIT_CURRENT,
	MULTI_IC_CURR_RATIO_ERR_MAX,
};

struct multi_ic_curr_ratio_para {
	u32 error_cnt;
	u32 current_ratio_min;
	u32 current_ratio_max;
	u32 dmd_level;
	int limit_current;
};

#define MULTI_IC_VBAT_ERROR_PARA_LEVEL     5

enum multi_ic_vbat_error_info {
	MULTI_IC_VBAT_ERROR_CHECK_CNT,
	MULTI_IC_VBAT_ERROR_DELTA,
	MULTI_IC_VBAT_ERROR_DMD_LEVEL,
	MULTI_IC_VBAT_ERROR_LIMIT_CURRENT,
	MULTI_IC_VBAT_ERROR_MAX,
};

struct multi_ic_vbat_error_para {
	u32 error_cnt;
	u32 vbat_error;
	u32 dmd_level;
	int limit_current;
};

#define MULTI_IC_TBAT_ERROR_PARA_LEVEL   5

enum multi_ic_tbat_error_info {
	MULTI_IC_TBAT_ERROR_CHECK_CNT,
	MULTI_IC_TBAT_ERROR_DELTA,
	MULTI_IC_TBAT_ERROR_DMD_LEVEL,
	MULTI_IC_TBAT_ERROR_LIMIT_CURRENT,
	MULTI_IC_TBAT_ERROR_MAX,
};

struct multi_ic_tbat_error_para {
	u32 error_cnt;
	u32 tbat_error;
	u32 dmd_level;
	int limit_current;
};

struct multi_ic_check_para {
	u32 multi_ic_start_time;
	int ibat_th;
	int limit_current;
	int report_info[MULTI_IC_DMD_LEVEL_END];
	unsigned int ibus_error_num[MULTI_IC_CURR_RATIO_PARA_LEVEL];
	unsigned int vbat_error_num[MULTI_IC_VBAT_ERROR_PARA_LEVEL];
	unsigned int tbat_error_num[MULTI_IC_TBAT_ERROR_PARA_LEVEL];
	int vbat_comp[CHARGE_PATH_MAX_NUM];
	struct multi_ic_curr_ratio_para curr_ratio[MULTI_IC_CURR_RATIO_PARA_LEVEL];
	struct multi_ic_vbat_error_para vbat_error[MULTI_IC_VBAT_ERROR_PARA_LEVEL];
	struct multi_ic_tbat_error_para tbat_error[MULTI_IC_TBAT_ERROR_PARA_LEVEL];
};

struct multi_ic_check_mode_para {
	int support_multi_ic;
	u32 support_select_temp;
	int ic_error_cnt[CHARGE_PATH_MAX_NUM];
};

int mulit_ic_check(int working_mode, int mode, struct multi_ic_check_para *info, int volt_ratio);
int multi_ic_check_select_tbat_id(struct multi_ic_check_mode_para *para);
int multi_ic_check_select_init_mode(struct multi_ic_check_mode_para *para, int *mode);
void multi_ic_check_set_ic_error_flag(int flag, struct multi_ic_check_mode_para *para);
int multi_ic_check_ic_status(struct multi_ic_check_mode_para *para);
void multi_ic_parse_check_para(struct device_node *np, struct multi_ic_check_para *info);

#endif /* _MULTI_IC_CHECK_H_ */
