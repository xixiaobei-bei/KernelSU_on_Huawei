/*
 * wireless_direct_charger.h
 *
 * wireless direct charger driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_DIRECT_CHARGER_H_
#define _WIRELESS_DIRECT_CHARGER_H_

#include <linux/of.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/wireless_charge/wireless_dc_cp.h>
#include <chipset_common/hwpower/direct_charge/multi_ic_check.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_comp.h>

#define WLDC_VOLT_GROUP_MAX               10
#define WLDC_BAT_BRAND_LEN_MAX            16
#define WLDC_VOLT_NODE_LEN_MAX            16
#define WLDC_TYPE_LEN_MAX                 8
#define WLDC_DMD_LOG_SIZE                 2048

#define WLDC_MODE_MAX                     2
#define WLDC_VOLT_LEVEL                   5
#define WLDC_ERR_CNT_MAX                  8
#define WLDC_WARNING_CNT_MAX              8
#define WLDC_OPEN_RETRY_CNT_MAX           3

#define WLDC_DEFAULT_VBATT_MAX            4350
#define WLDC_DEFAULT_VBATT_MIN            3550
#define WLDC_VBAT_OVP_TH                  4500

#define WLDC_VOUT_ACCURACY_CHECK_CNT      3
#define WLDC_OPEN_PATH_CNT                6
#define WLDC_OPEN_PATH_IOUT_MIN           500
#define WLDC_LEAK_CURRENT_CHECK_CNT       6
#define WLDC_VDIFF_CHECK_CNT              3

#define WLDC_DFT_MAX_CP_IOUT              6000
#define WLDC_IMPOSSIBLE_PWR               1000000000 /* 1kw */
#define WLDC_DEFAULT_VSTEP                100
#define WLDC_DEFAULT_IOUT_ERR_TH          150
#define WLDC_DEFAULT_IBAT_MAX_TH          10000 /* 10A */
#define WLDC_DEFAULT_IBAT_MIN_TH          10000 /* 10A */
#define WLDC_DFT_IMAX_ERR_TH              50
#define SC_DEFAULT_VOLT_RATIO             2

#define WLDC_DEFAULT_CTRL_INTERVAL        100  /* ms */
#define WLDC_DEFAULT_CALC_INTERVAL        50 /* ms */

#define WLDC_IOUT_LTH                     350  /* ma */
#define WLDC_RATION_MODE                  1  /* 2:1 mode */
#define WLDC_IOUT_AVG_LOW_CNT             10
#define WLDC_NEED_WIRED_SW_ON             1

#define WLDC_IC_ERR_CNT                   8
#define WLDC_IC_IBUS_CHECK_CNT            6
#define WLDC_IC_SC_SW2_CNT                6

enum wldc_ext_pwr_type {
	WLDC_EXT_PWR_TO_SC_IC = BIT(0),
	WLDC_EXT_PWR_TO_RX_IC = BIT(1),
};

enum wldc_batt_stage {
	WLDC_CC_STAGE = 0,
	WLDC_CV_STAGE,
	WLDC_BATT_STG_TOTAL,
};

enum wldc_dis_flag {
	WLDC_EN = 0,
	WLDC_DIS_BY_CHRG_DONE = BIT(0),
	WLDC_DIS_BY_PRIORITY = BIT(1),
};

enum wldc_sysfs_type {
	WLDC_SYSFS_IIN_THERMAL = 0,
};

enum wldc_init_info {
	WLDC_TYPE = 0,
	WLDC_NAME,
	WLDC_EXT_PWR_TYPE,
	WLDC_RX_RATIO,
	WLDC_VBATT_MIN,
	WLDC_VBATT_MAX,
	WLDC_RX_VOUT,
	WLDC_RX_VOUT_TH,
	WLDC_VDIFF_TH,
	WLDC_ILEAK_TH,
	WLDC_VDELT,
	WLDC_RX_VMAX,
	WLDC_INIT_INFO_TOTAL,
};

enum wldc_volt_info {
	WLDC_PARA_VBATT_HTH = 0,
	WLDC_PARA_CP_IOUT_HTH,
	WLDC_PARA_CP_IOUT_LTH,
	WLDC_VOLT_INFO_TOTAL,
};

enum wldc_iin_thermal_channel_type {
	WLDC_CHANNEL_TYPE_BEGIN = 0,
	WLDC_SINGLE_CHANNEL = WLDC_CHANNEL_TYPE_BEGIN,
	WLDC_DUAL_CHANNEL,
	WLDC_CHANNEL_TYPE_END,
};

struct wldc_sysfs_data {
	int enable_charger;
	int cp_iin_thermal_array[WLDC_CHANNEL_TYPE_END];
	int pwr_limit;
};

enum wldc_bat_info {
	WLDC_BAT_ID = 0,
	WLDC_BAT_TEMP_LOW,
	WLDC_BAT_TEMP_HIGH,
	WLDC_BAT_DC_TYPE,
	WLDC_BAT_PARA_INDEX,
	WLDC_BAT_INFO_TOTAL,
};

struct wldc_init_para {
	int dc_type;
	char dc_name[WLDC_TYPE_LEN_MAX];
	enum wldc_ext_pwr_type ext_pwr_type;
	int rx_ratio;
	int vbatt_min;
	int vbatt_max;
	int rx_vout;
	int rx_vout_th;
	int vdiff_th;
	int ileak_th;
	int vdelt;
	int rx_vmax;
};

struct wldc_mode_para {
	enum wldc_dis_flag dis_flag;
	int err_cnt;
	int warn_cnt;
	int dc_open_retry_cnt;
	bool dmd_report_flag;
	struct wldc_init_para init_para;
};

struct wldc_volt_para {
	int vbatt_hth;
	int cp_iout_hth;
	int cp_iout_lth;
};

struct wldc_bat_para {
	int tbatt_lth;
	int tbatt_hth;
	int dc_type;
	int parse_ok;
	char batid[WLDC_BAT_BRAND_LEN_MAX];
	char volt_para_id[WLDC_VOLT_NODE_LEN_MAX];
};

struct wldc_volt_para_group {
	struct wldc_volt_para volt_info[WLDC_VOLT_LEVEL];
	struct wldc_bat_para bat_info;
	int stage_size;
};

enum wldc_ic_ibus_info {
	WLDC_MULTI_IC_IBAT_HTH = 0,
	WLDC_MULTI_IC_HTIME,
	WLDC_MULTI_IC_IBAT_LTH,
	WLDC_MULTI_IC_LTIME,
	WLDC_MULTI_IC_TOTAL,
};

struct wldc_ic_ibus_para {
	u32 hth;
	u32 h_time;
	u32 hth_cnt;
	u32 lth;
	u32 l_time;
	u32 lth_cnt;
	u32 sw_cnt;
};

struct wldc_ic_data {
	int cur_type;
	u32 multi_ic_err_cnt;
	int single_ic_cp_iout_th;
	struct wldc_ic_ibus_para ibus_para;
};

struct wldc_regulation_data {
	int rx_ratio;
	int rx_vrect;
	int rx_vout;
	int rx_iout;
	int rx_iout_avg;
	int cp_iout;
	int cp_iout_avg;
	int ls_vbus;
	int ls_ibus;
	int ls_vbatt;
	int ls_ibatt;
	int rx_max_iout;
	int rx_vmax;
};

enum wldc_tbat_limit_info {
	WLDC_TBAT_LIMIT_LTH = 0,
	WLDC_TBAT_LIMIT_LTH_BACK,
	WLDC_TBAT_LIMIT_HTH,
	WLDC_TBAT_LIMIT_HTH_BACK,
	WLDC_TBAT_LIMIT_TOTAL,
};

struct wldc_tbat_limit_para {
	int lth;
	int lth_back;
	int hth;
	int hth_back;
};

struct wldc_dev_info {
	struct device *dev;
	struct wldc_sysfs_data sysfs_data;
	struct wldc_volt_para volt_para[WLDC_VOLT_LEVEL];
	struct wldc_volt_para orig_volt_para[WLDC_VOLT_LEVEL];
	struct wldc_volt_para_group *orig_volt_para_p;
	struct delayed_work wldc_ctrl_work;
	struct delayed_work wldc_calc_work;
	int total_dc_mode;
	int vstep;
	int max_vgap;
	int stage_size;
	int pre_stage;
	int cur_stage;
	int ctrl_interval;
	int calc_interval;
	int volt_ratio;
	int cur_vbat_hth;
	int cur_cp_iout_hth;
	int cur_cp_iout_lth;
	int cp_iout_err_hth;
	int ibat_max_hth;
	int ibat_min_lth;
	int tbatt_lth;
	int tbatt_hth;
	int rx_vout_set;
	int cur_dc_mode;
	int tx_imax;
	int iout_drop_th;
	struct wldc_tbat_limit_para tbat_limit;
	struct wldc_cp_data cp_data;
	struct wldc_ic_data ic_data;
	struct multi_ic_check_para ic_check_info;
	struct multi_ic_check_mode_para ic_mode_para;
	struct wldc_mode_para *mode_para;
	struct dc_comp_para comp_para;
	char wldc_err_dsm_buff[WLDC_DMD_LOG_SIZE];
	int stage_group_size;
	unsigned int vterm_dec;
	unsigned int ichg_ratio;
	bool stop_flag_error;
	bool stop_flag_warning;
	bool stop_flag_info;
};

void wldc_set_di(struct wldc_dev_info *di);
void wireless_sc_get_di(struct wldc_dev_info **di);

void wldc_extra_power_supply(bool flag);
int wldc_cut_off_normal_charge_path(void);
int wldc_turn_on_direct_charge_channel(void);
int wldc_turn_off_direct_charge_channel(void);

int wldc_prev_check(const char *mode_name);
int wldc_formal_check(const char *mode_name);
int wireless_sc_charge_check(void);

void wldc_parse_dts(struct device_node *np, struct wldc_dev_info *di);
int wldc_chip_init(struct wldc_dev_info *di);
int wldc_security_check(struct wldc_dev_info *di);

void wldc_stop_charging(struct wldc_dev_info *di);

int wldc_set_rx_init_vout(struct wldc_dev_info *di);
int wldc_get_ls_vbus(void);
int wldc_get_cp_avg_iout(void);

void wldc_control_work(struct work_struct *work);
void wldc_calc_work(struct work_struct *work);

int wldc_get_warning_cnt(void);
int wldc_get_error_cnt(void);
void wldc_tx_disconnect_handler(void);
bool wldc_is_stop_charging_complete(void);

bool wldc_msleep_exit(void);
void wldc_ic_para_reset(struct wldc_dev_info *di);
void wldc_ic_select_charge_path(struct wldc_dev_info *di);
void wldc_cp_reset(struct wldc_dev_info *di);
void wldc_cp_select_charge_path(struct wldc_dev_info *di);
void wldc_parse_multi_cp_para(struct device_node *np, struct wldc_dev_info *di);

#endif /* _WIRELESS_DIRECT_CHARGER_H_ */
