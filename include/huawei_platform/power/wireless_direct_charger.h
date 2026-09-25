/*
 * wireless_direct_charger.h
 *
 * wireless direct charger driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef BIT
#define BIT(x)                            (1 << (x))
#endif

#define WLDC_VOLT_GROUP_MAX               10
#define WLDC_BAT_BRAND_LEN_MAX            16
#define WLDC_VOLT_NODE_LEN_MAX            16
#define WLDC_TYPE_LEN_MAX                 8
#define WLDC_DMD_LOG_SIZE                 2048

#define WLDC_MODE_MAX                     2
#define WLDC_VOLT_LEVEL                   3
#define WLDC_ERR_CNT_MAX                  8
#define WLDC_WARNING_CNT_MAX              8
#define WLDC_OPEN_RETRY_CNT_MAX           3

#define WLDC_TBATT_MIN                    10
#define WLDC_TBATT_MIN_BACK               13
#define WLDC_TBATT_MAX                    45
#define WLDC_TBATT_MAX_BACK               42
#define WLDC_DEFAULT_VBATT_MAX            4350
#define WLDC_DEFAULT_VBATT_MIN            3550
#define WLDC_VBAT_OVP_TH                  4500
#define WLDC_TX_PWR_RATIO                 75

#define WLDC_VOUT_ACCURACY_CHECK_CNT      3
#define WLDC_OPEN_PATH_CNT                6
#define WLDC_OPEN_PATH_IOUT_MIN           500
#define WLDC_LEAK_CURRENT_CHECK_CNT       6
#define WLDC_VDIFF_CHECK_CNT              3

#define WLDC_DFT_MAX_CP_IOUT              5000
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

enum wldc_stage {
	WLDC_STAGE_DEFAULT = 0,
	WLDC_STAGE_CHECK,
	WLDC_STAGE_SUCCESS,
	WLDC_STAGE_CHARGING,
	WLDC_STAGE_CHARGE_DONE,
	WLDC_STAGE_STOP_CHARGING,
	WLDC_STAGE_TOTAL,
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

struct wldc_sysfs_data {
	int enable_charger;
	int cp_iin_thermal;
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

struct wldc_dev_info {
	struct device *dev;
	struct loadswitch_ops *ls_ops;
	struct batinfo_ops *bi_ops;
	struct wldc_sysfs_data sysfs_data;
	struct wldc_volt_para volt_para[WLDC_VOLT_LEVEL];
	struct wldc_volt_para orig_volt_para[WLDC_VOLT_LEVEL];
	struct wldc_volt_para_group *orig_volt_para_p;
	struct delayed_work wldc_ctrl_work;
	struct delayed_work wldc_calc_work;
	int total_dc_mode;
	int vstep;
	int wldc_stage;
	int stage_size;
	int pre_stage;
	int cur_stage;
	int ctrl_interval;
	int calc_interval;
	int stop_flag_error;
	int stop_flag_warning;
	int stop_flag_info;
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
	struct wldc_mode_para *mode_para;
	char wldc_err_dsm_buff[WLDC_DMD_LOG_SIZE];
	int stage_group_size;
	unsigned int vterm_dec;
	unsigned int ichg_ratio;
};

void wldc_set_di(struct wldc_dev_info *di);
void wireless_sc_get_di(struct wldc_dev_info **di);

#ifdef CONFIG_WIRELESS_CHARGER
int wldc_rx_ops_register(struct wireless_charge_device_ops *ops);
int wireless_sc_ops_register(struct loadswitch_ops *ops);
int wireless_sc_batinfo_ops_register(struct batinfo_ops *ops);
#else
static inline int wldc_rx_ops_register(struct wireless_charge_device_ops *ops)
{
	return 0;
}

static inline int wireless_sc_ops_register(struct loadswitch_ops *ops)
{
	return 0;
}

static inline int wireless_sc_batinfo_ops_register(struct batinfo_ops *ops)
{
	return 0;
}
#endif /* CONFIG_WIRELESS_CHARGER */

void wldc_extra_power_supply(int flag);
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

void wldc_set_charge_stage(enum wldc_stage sc_stage);
int wldc_set_rx_init_vout(struct wldc_dev_info *di);
int wldc_get_ls_vbus(void);

void wldc_control_work(struct work_struct *work);
void wldc_calc_work(struct work_struct *work);

int wldc_get_warning_cnt(void);
int wldc_get_error_cnt(void);
void wldc_tx_disconnect_handler(void);
bool wldc_is_stop_charging_complete(void);

#endif /* _WIRELESS_DIRECT_CHARGER_H_ */
