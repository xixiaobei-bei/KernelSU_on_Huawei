/*
 * direct_charger.h
 *
 * direct charger driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGER_H_
#define _DIRECT_CHARGER_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>

#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_COUL_DRV
#include <linux/power/hisi/coul/coul_drv.h>
#endif
#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#ifdef CONFIG_SUPERSWITCH_FSC
#include <huawei_platform/usb/superswitch/fsc/core/hw_scp.h>
#endif
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless/wireless_charger.h>
#endif
#ifdef CONFIG_DP_AUX_SWITCH
#include "huawei_platform/dp_aux_switch/dp_aux_switch.h"
#endif
#include <chipset_common/hwpower/direct_charge/direct_charge_power_supply.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_para_parse.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_path_switch.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_adapter.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_uevent.h>
#include <huawei_platform/power/direct_charger/direct_charger_check.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_cable.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_mmi.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_comp.h>
#include <chipset_common/hwpower/direct_charge/multi_ic_check.h>

#define DC_DMDLOG_SIZE                2048
#define REVERSE_OCP_CNT               3
#define OTP_CNT                       3
#define ADP_OTP_CNT                   3
#define DOUBLE_SIZE                   2

/* sensor_id#scene_id#stage */
#define DC_THERMAL_REASON_SIZE        16

#define DC_ERROR_RESISTANCE           (-99999)
#define DC_MAX_RESISTANCE             10000
#define DC_CHARGE_TYPE_LVC            3
#define DC_CHARGE_TYPE_SC             2
#define DC_CHARGE_TYPE_FCP            4

#define DC_ERR_CNT_MAX                8
#define DC_MULTI_ERR_CNT_MAX          4
#define DC_OPEN_RETRY_CNT_MAX         3

#define INVALID                       (-1)
#define VALID                         0
#define WAIT_LS_DISCHARGE             200
#define MAX_TIMES_FOR_SET_ADAPTER_VOL 30
#define MIN_CURRENT_FOR_RES_DETECT    800
#define CURRENT_SET_FOR_RES_DETECT    1000
#define STEP_VOL_START                6000

#define ENABLE                        1
#define DISABLE                       0

#define VBUS_ON_THRESHOLD             3000
#define VBAT_VBUS_DIFFERENCE          150
#define KICK_WATCHDOG_TIME            1000
#define WATCHDOG_TIMEOUT              2000
#define BATTERY_CAPACITY_HIGH_TH      95
#define BAT_RATED_VOLT                5000
#define POWER_TH_IGNORE_ANTIFAKE      22500

#define DC_LS_RECOVERY_DELAY          500 /* ms */
#define DC_COUL_CURRENT_UNIT_DEVIDE   1000 /* 1000ua equal 1ma */

#define DC_IN_CHARGING_STAGE          0
#define DC_NOT_IN_CHARGING_STAGE      (-1)

#define DC_IN_CHARGE_DONE_STAGE       0
#define DC_NOT_IN_CHARGE_DONE_STAGE   (-1)

#define DC_SET_DISABLE_FLAGS          1
#define DC_CLEAR_DISABLE_FLAGS        0

#define DC_SC_CUR_LEN                 64

#define BASP_VTERM_DEC_DEFAULT        0
#define BASP_ICHG_RATIO_DEFAULT       100

#define DC_SINGLEIC_CURRENT_LIMIT     8000
#define DC_MULTI_IC_IBAT_TH           4000
#define DC_CURRENT_OFFSET             300
#define MIN_CURRENT_FOR_MULTI_IC      500
#define DC_MULTI_IC_INFO_IBAT_TH      7000

/* for charge temp hysteresis */
#define DC_LOW_TEMP_MAX               10
#define DC_HIGH_TEMP_MAX              45

#define ADP_ANTIFAKE_FAIL_MAX_CURRENT 2250
#define ADP_ANTIFAKE_CHECK_THLD       4

enum direct_charge_mode_set {
	AND_SET = 0x0,
	OR_SET = 0x1,
};

/*
 * define operate type for retry with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_retry_operate_type {
	DC_RESET_ADAPTER = 0,
	DC_RESET_MASTER,
};

/*
 * define stage type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_stage_type {
	DC_STAGE_BEGIN = 0,
	DC_STAGE_DEFAULT = DC_STAGE_BEGIN,
	DC_STAGE_ADAPTER_DETECT,
	DC_STAGE_SWITCH_DETECT,
	DC_STAGE_CHARGE_INIT,
	DC_STAGE_SECURITY_CHECK,
	DC_STAGE_SUCCESS,
	DC_STAGE_CHARGING,
	DC_STAGE_CHARGE_DONE,
	DC_STAGE_END,
};

enum direct_charge_succ_flag {
	DC_SUCCESS,
	DC_ERROR,
};

/*
 * define sysfs type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_sysfs_type {
	DC_SYSFS_BEGIN = 0,
	DC_SYSFS_IIN_THERMAL = DC_SYSFS_BEGIN,
	DC_SYSFS_IIN_THERMAL_ICHG_CONTROL,
	DC_SYSFS_ICHG_CONTROL_ENABLE,
	DC_SYSFS_ADAPTER_DETECT,
	DC_SYSFS_IADAPT,
	DC_SYSFS_FULL_PATH_RESISTANCE,
	DC_SYSFS_DIRECT_CHARGE_SUCC,
	DC_SYSFS_SET_RESISTANCE_THRESHOLD,
	DC_SYSFS_SET_CHARGETYPE_PRIORITY,
	DC_SYSFS_THERMAL_REASON,
	DC_SYSFS_MULTI_SC_CUR,
	DC_SYSFS_MULTI_IBAT,
	DC_SYSFS_SC_STATE,
	DC_SYSFS_SC_FREQ,
	DC_SYSFS_END,
};

/*
 * define disable type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_disable_type {
	DC_DISABLE_BEGIN = 0,
	DC_DISABLE_SYS_NODE = DC_DISABLE_BEGIN,
	DC_DISABLE_FATAL_ISC_TYPE,
	DC_DISABLE_WIRELESS_TX,
	DC_DISABLE_BATT_CERTIFICATION_TYPE,
	DC_DISABLE_END,
};

/*
 * define temprature threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_TEMP_LEVEL           5

enum direct_charge_temp_info {
	DC_TEMP_MIN = 0,
	DC_TEMP_MAX,
	DC_TEMP_CUR_MAX,
	DC_TEMP_TOTAL,
};

struct direct_charge_temp_para {
	int temp_min;
	int temp_max;
	int temp_cur_max;
};

/*
 * define resistance threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_RESIST_LEVEL         5

enum direct_charge_resist_info {
	DC_RESIST_MIN = 0,
	DC_RESIST_MAX,
	DC_RESIST_CUR_MAX,
	DC_RESIST_TOTAL,
};

struct direct_charge_resist_para {
	int resist_min;
	int resist_max;
	int resist_cur_max;
};

/*
 * define multistage (cc)constant current and (cv)constant voltage
 * support up to 5 parameters list on dts
 */
#define DC_VOLT_LEVEL           8

enum direct_charge_volt_info {
	DC_PARA_VOL_TH = 0,
	DC_PARA_CUR_TH_HIGH,
	DC_PARA_CUR_TH_LOW,
	DC_PARA_VOLT_TOTAL,
};

struct direct_charge_volt_para {
	int vol_th;
	int cur_th_high;
	int cur_th_low;
};

/*
 * define adapter current threshold at different voltages
 * support up to 6 parameters list on dts
 */
#define DC_ADP_CUR_LEVEL       6

enum direct_charge_adp_cur_info {
	DC_ADP_VOL_MIN,
	DC_ADP_VOL_MAX,
	DC_ADP_CUR_TH,
	DC_ADP_TOTAL,
};

struct direct_charge_adp_cur_para {
	int vol_min;
	int vol_max;
	int cur_th;
};

/* define rt test time parameters */
enum direct_charge_rt_test_info {
	DC_RT_CURR_TH,
	DC_RT_TEST_TIME,
	DC_RT_TEST_INFO_TOTAL,
};

enum direct_charge_rt_test_mode {
	DC_NORMAL_MODE,
	DC_CHAN1_MODE, /* channel1: single main sc mode */
	DC_CHAN2_MODE, /* channel2: single aux sc mode */
	DC_MODE_TOTAL,
};

struct direct_charge_rt_test_para {
	u32 rt_curr_th;
	u32 rt_test_time;
	bool rt_test_result;
};

/*
 * define voltage parameters of different batteries
 * at different temperature threshold
 * support up to 8 parameters list on dts
 */
#define DC_VOLT_GROUP_MAX       8
#define DC_BAT_BRAND_LEN_MAX    16
#define DC_VOLT_NODE_LEN_MAX    16

enum direct_charge_bat_info {
	DC_PARA_BAT_ID = 0,
	DC_PARA_TEMP_LOW,
	DC_PARA_TEMP_HIGH,
	DC_PARA_INDEX,
	DC_PARA_BAT_TOTAL,
};

struct direct_charge_bat_para {
	int temp_low;
	int temp_high;
	int parse_ok;
	char batid[DC_BAT_BRAND_LEN_MAX];
	char volt_para_index[DC_VOLT_NODE_LEN_MAX];
};

/*
 * define dc time threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_TIME_PARA_LEVEL               5

enum direct_charge_time_info {
	DC_TIME_INFO_TIME_TH,
	DC_TIME_INFO_IBAT_MAX,
	DC_TIME_INFO_MAX,
};

struct direct_charge_time_para {
	int time_th;
	int ibat_max;
};

/*
* define dc first cc charge time with max power
* support up to 7 parameters list on dts
*/
#define DC_MAX_POWER_TIME_PARA_LEVEL    20

enum direct_charge_max_power_time_info {
	DC_ADAPTER_TYPE,
	DC_MAX_POWER_TIME,
	DC_MAX_POWER_LIMIT_CURRENT,
	DC_MAX_POWER_PARA_MAX,
};

struct direct_charge_max_power_time_para {
	int adatper_type;
	int max_power_time;
	int limit_current;
};

struct direct_charge_limit_max_power_para {
	int limit_current;
	int start_time;
	int run_time;
	bool start_timer_flag;
	int max_power_timeout;
};

struct direct_charge_volt_para_group {
	struct direct_charge_volt_para volt_info[DC_VOLT_LEVEL];
	struct direct_charge_bat_para bat_info;
	int stage_size;
};

enum direct_charge_info_type {
	CC_CABLE_DETECT_OK,
};

enum direct_charge_cable_type {
	DC_UNKNOWN_CABLE,
	DC_NONSTD_CABLE,
	DC_STD_CABLE,
};

enum dc_iin_thermal_channel_type {
	DC_CHANNEL_TYPE_BEGIN = 0,
	DC_SINGLE_CHANNEL = DC_CHANNEL_TYPE_BEGIN,
	DC_DUAL_CHANNEL,
	DC_CHANNEL_TYPE_END,
};

struct direct_charge_charge_info {
	int succ_flag;
	const char *ic_name[CHARGE_IC_MAX_NUM];
	int channel_num;
	int ibat_max;
	int pmu_ibat;
	int ibus[CHARGE_IC_MAX_NUM];
	int ibat[CHARGE_IC_MAX_NUM];
	int vbat[CHARGE_IC_MAX_NUM];
	int vout[CHARGE_IC_MAX_NUM];
	int tbat[CHARGE_IC_MAX_NUM];
};

#define DC_VSTEP_PARA_LEVEL    4

enum direct_charge_vstep_info {
	DC_VSTEP_INFO_CURR_GAP,
	DC_VSTEP_INFO_VSTEP,
	DC_VSTEP_INFO_MAX,
};

struct direct_charge_vstep_para {
	int curr_gap;
	int vstep;
};

/* define adapter power curve */
#define DC_ADP_PC_LEVEL              16
#define DC_ADP_PC_PARA_SIZE          2

struct direct_charge_adp_pc_para {
	int volt;
	int cur;
};

struct direct_charge_device {
	struct device *dev;
	struct hrtimer calc_thld_timer;
	struct hrtimer control_timer;
	struct hrtimer kick_wtd_timer;
	struct workqueue_struct *charging_wq;
	struct workqueue_struct *kick_wtd_wq;
	struct work_struct calc_thld_work;
	struct work_struct control_work;
	struct work_struct fault_work;
	struct work_struct kick_wtd_work;
	struct notifier_block fault_nb;
	struct direct_charge_volt_para volt_para[DC_VOLT_LEVEL];
	struct direct_charge_volt_para orig_volt_para[DC_VOLT_LEVEL];
	struct direct_charge_volt_para_group orig_volt_para_p[DC_VOLT_GROUP_MAX];
	struct direct_charge_temp_para temp_para[DC_TEMP_LEVEL];
	struct direct_charge_resist_para nonstd_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para std_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para second_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para ctc_second_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para ctc_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_adp_cur_para adp_10v2p25a[DC_ADP_CUR_LEVEL];
	struct direct_charge_adp_cur_para adp_10v2p25a_car[DC_ADP_CUR_LEVEL];
	struct direct_charge_adp_cur_para adp_qtr_a_10v2p25a[DC_ADP_CUR_LEVEL];
	struct direct_charge_adp_cur_para adp_qtr_c_20v3a[DC_ADP_CUR_LEVEL];
	struct direct_charge_adp_cur_para adp_10v4a[DC_ADP_CUR_LEVEL];
	struct direct_charge_adp_pc_para adp_pwr_curve[DC_ADP_PC_LEVEL];
	struct direct_charge_time_para time_para[DC_TIME_PARA_LEVEL];
	struct direct_charge_max_power_time_para max_power_time[DC_MAX_POWER_TIME_PARA_LEVEL];
	struct direct_charge_limit_max_power_para limit_max_pwr;
	struct direct_charge_vstep_para vstep_para[DC_VSTEP_PARA_LEVEL];
	struct direct_charge_rt_test_para rt_test_para[DC_MODE_TOTAL];
	struct multi_ic_check_para multi_ic_check_info;
	struct multi_ic_check_mode_para multi_ic_mode_para;
	struct dc_comp_para comp_para;
	struct nty_data *fault_data;
	bool force_single_path_flag;
	int time_para_parse_ok;
	int stage_need_to_jump[2 * DC_VOLT_LEVEL];
	int cam_flash_stop;
	int error_cnt;
	u32 multi_ic_error_cnt;
	int working_mode;
	int cur_mode;
	int local_mode;
	int curr_offset;
	int multi2single_ibat_delta;
	u32 use_5a;
	u32 use_8a;
	int sysfs_enable_charger;
	u32 sysfs_mainsc_enable_charger;
	u32 sysfs_auxsc_enable_charger;
	unsigned int sysfs_disable_charger[DC_DISABLE_END];
	int sysfs_iin_thermal;
	int sysfs_iin_thermal_array[DC_CHANNEL_TYPE_END];
	int sysfs_iin_thermal_ichg_control;
	int ichg_control_enable;
	u32 threshold_caculation_interval;
	u32 charge_control_interval;
	int cur_stage;
	int cur_vbat_th;
	int cur_ibat_th_high;
	int cur_ibat_th_low;
	int multi_ic_ibat_th;
	int single_ic_ibat_th;
	int vbat;
	int ibat;
	int ibat_abnormal_cnt;
	int ibat_abnormal_th;
	int vadapt;
	int iadapt;
	int tadapt;
	int ls_vbus;
	int ls_ibus;
	int tls;
	int full_path_resistance;
	int second_path_res_report_th;
	int second_resist_check_en;
	int second_path_resistance;
	bool second_resist_check_ok;
	int stage_size;
	int stage_group_size;
	int stage_group_cur;
	int pre_stage;
	int adaptor_vset;
	int max_adapter_vset;
	int max_tadapt;
	int max_tls;
	int adaptor_iset;
	int max_adapter_iset;
	int init_delt_vset;
	int init_delt_iset;
	int init_adapter_vset;
	int delta_err;
	int delta_err_10v2p25a;
	int delta_err_10v4a;
	int vstep;
	int scp_stop_charging_flag_info;
	int stop_charging_flag_error;
	int max_dc_bat_vol;
	int min_dc_bat_vol;
	int super_ico_current;
	u32 is_show_ico_first;
	u32 is_send_cable_type;
	int bst_ctrl;
	int scp_power_en;
	int compensate_r;
	int compensate_v;
	u32 cc_protect;
	int ls_id;
	const char *ls_name;
	int vol_err_th;
	int nonstd_cable_full_path_res_max;
	int std_cable_full_path_res_max;
	int ctc_cable_full_path_res_max;
	int full_path_res_thld;
	int adaptor_leakage_current_th;
	u32 adaptor_detect_by_voltage;
	int lvc_in_aux_mode;
	int dc_succ_flag;
	u32 first_cc_stage_timer_in_min;
	int max_adaptor_cur;
	int scp_cable_detect_enable;
	unsigned long first_cc_stage_timeout;
	u32 direct_charge_start_time;
	u32 dc_multi_ic_start_time;
	u32 cc_cable_detect_enable;
	int cc_cable_detect_ok;
	int max_current_for_nonstd_cable;
	int max_current_for_ctc_cable;
	unsigned int charge_fault;
	int adaptor_vendor_id;
	u32 scp_work_on_charger;
	int dc_err_report_flag;
	int dc_open_retry_cnt;
	int reverse_ocp_cnt;
	int otp_cnt;
	int adp_otp_cnt;
	int dc_volt_ratio;
	unsigned int dc_stage;
	int cutoff_normal_flag;
	int quick_charge_flag;
	int super_charge_flag;
	char dsm_buff[DC_DMDLOG_SIZE];
	int scp_stop_charging_complete_flag;
	struct wakeup_source charging_lock;
	int adaptor_test_result_type;
	char thermal_reason[DC_THERMAL_REASON_SIZE];
	u32 adp_antifake_key_index;
	u32 adp_antifake_execute_enable;
	u32 adp_antifake_enable;
	int adp_antifake_failed_cnt;
	int adp_antifake_result;
	int sc_conv_ocp_count;
	int iin_thermal_default;
	int can_stop_kick_wdt;
	u32 gain_curr_10v2a;
	u32 reset_adap_volt_enabled;
	unsigned int vterm_dec;
	unsigned int ichg_ratio;
	int bat_temp_before_charging;
	u32 low_temp_hysteresis;
	u32 orig_low_temp_hysteresis;
	u32 high_temp_hysteresis;
	u32 orig_high_temp_hysteresis;
	bool bat_temp_err_flag;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
	bool nonstd_cable_flag;
	u32 need_wired_sw_off;
	int cable_type;
	int orig_cable_type;
	int max_pwr;
	int ui_max_pwr;
	int product_max_pwr;
	bool pri_inversion;
	int cur_inversion;
	u32 adapter_type;
	bool ignore_full_path_res;
	bool adapter_power_curve_flag;
	int use_force_sleep;
	struct direct_charge_charge_info curr_info;
	u32 cc_unsafe_sc_enable;
	bool cc_safe;
	u32 use_higher_vbat;
	u32 tbat_id;
	u32 adapter_watchdog_time;
	u32 adapter_iwatt_iset;
	u32 support_series_bat_mode;
	u32 bat_cell_num;
};

#ifdef CONFIG_DIRECT_CHARGER
void direct_charge_set_di(struct direct_charge_device *di);
struct direct_charge_device *direct_charge_get_di(void);
struct direct_charge_device *direct_charge_get_di_by_mode(int mode);
void direct_charge_set_start_time(void);
void direct_charge_set_abnormal_adp_flag(bool flag);
bool direct_charge_get_abnormal_adp_flag(void);
unsigned int direct_charge_get_stage_status(void);
const char *direct_charge_get_stage_status_string(unsigned int stage);
void direct_charge_set_stage_status(unsigned int stage);
void direct_charge_set_stage_status_default(void);
int direct_charge_in_charging_stage(void);
int direct_charge_get_working_mode(void);
void direct_charge_set_local_mode(int set, unsigned int dc_mode);
unsigned int direct_charge_get_local_mode(void);
unsigned int direct_charge_update_local_mode(unsigned int mode);
int direct_charge_is_failed(void);
void direct_charge_set_disable_flags(int val, int type);
int direct_charge_get_info(enum direct_charge_info_type type, int *value);
int direct_charge_get_battery_max_current(void);
int direct_charge_battery_temp_handler(int value);
int direct_charge_get_adapter_gain_current(struct direct_charge_device *di);
void direct_charge_force_disable_dc_path(void);
int direct_charge_get_device_vbus(int *value);
int direct_charge_get_device_ibus(int *value);
int direct_charge_get_device_close_status(void);
int direct_charge_init_adapter_and_device(void);
int direct_charge_switch_path_to_normal_charging(void);
int direct_charge_switch_path_to_dc_charging(void);
int direct_charge_open_charging_path(void);
bool direct_charge_get_stop_charging_complete_flag(void);
void direct_charge_set_stop_charging_flag(int value);
unsigned int direct_charge_detect_adapter_support_mode(void);
int direct_charge_detect_adapter_again(void);
void direct_charge_exit(void);
int direct_charge_get_cutoff_normal_flag(void);
void direct_charge_update_cutoff_normal_flag(void);
void direct_charge_update_charge_info(void);
void direct_charge_get_ffc_para(void);
int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data);
int direct_charge_start_charging(void);
void direct_charge_stop_charging(void);
int direct_charge_get_stop_charging_flag(void);
int direct_charge_get_vbus(void);
int direct_charge_get_ibus(void);
int direct_charge_get_bat_current(int *value);
void direct_charge_select_charging_param(void);
void direct_charge_select_charging_stage(void);
void direct_charge_soh_policy(void);
void direct_charge_regulation(void);
bool direct_charge_check_priority_inversion(void);
bool direct_charge_is_priority_inversion(void);
int lvc_get_di(struct direct_charge_device **di);
int lvc_set_disable_flags(int val, int type);
int sc_get_di(struct direct_charge_device **di);
int sc_set_disable_flags(int val, int type);
#else
static inline void direct_charge_set_di(struct direct_charge_device *di)
{
}

static inline struct direct_charge_device *direct_charge_get_di(void)
{
	return NULL;
}

static inline struct direct_charge_device *direct_charge_get_di_by_mode(int mode)
{
	return NULL;
}

static inline void direct_charge_set_start_time(void)
{
}

static inline void direct_charge_set_abnormal_adp_flag(bool flag)
{
}

static inline bool direct_charge_get_abnormal_adp_flag(void)
{
	return false;
}

static inline unsigned int direct_charge_get_stage_status(void)
{
	return DC_STAGE_DEFAULT;
}

static inline const char *direct_charge_get_stage_status_string(
	unsigned int stage)
{
	return "illegal stage status";
}

static inline void direct_charge_set_stage_status(unsigned int stage)
{
}

static inline void direct_charge_set_stage_status_default(void)
{
}

static inline int direct_charge_in_charging_stage(void)
{
	return -1;
}

static inline int direct_charge_get_working_mode(void)
{
	return 0;
}

static inline void direct_charge_set_local_mode(int set, unsigned int dc_mode)
{
}

static inline unsigned int direct_charge_get_local_mode(void)
{
	return 0;
}

static inline unsigned int direct_charge_update_local_mode(unsigned int mode)
{
	return 0;
}

static inline int direct_charge_is_failed(void)
{
	return -1;
}

static inline void direct_charge_set_disable_flags(int val, int type)
{
}

static inline int direct_charge_get_info(
	enum direct_charge_info_type type, int *value)
{
	return -1;
}

static inline int direct_charge_get_battery_max_current(void)
{
	return -1;
}

static inline int direct_charge_battery_temp_handler(int value)
{
	return 0;
}

static inline int direct_charge_get_adapter_gain_current(
	struct direct_charge_device *di)
{
	return 0;
}

static inline void direct_charge_force_disable_dc_path(void)
{
}

static inline int direct_charge_get_device_vbus(int *value)
{
	return -1;
}

static inline int direct_charge_get_device_ibus(int *value)
{
	return -1;
}

static inline int direct_charge_get_device_close_status(void)
{
	return -1;
}

static inline int direct_charge_init_adapter_and_device(void)
{
	return -1;
}

static inline int direct_charge_switch_path_to_normal_charging(void)
{
	return -1;
}

static inline int direct_charge_switch_path_to_dc_charging(void)
{
	return -1;
}

static inline int direct_charge_open_charging_path(void)
{
	return -1;
}

static inline bool direct_charge_get_stop_charging_complete_flag(void)
{
	return false;
}

static inline void direct_charge_set_stop_charging_flag(int value)
{
}

static inline unsigned int direct_charge_detect_adapter_support_mode(void)
{
	return 0;
}

static inline int direct_charge_detect_adapter_again(void)
{
	return -1;
}

static inline void direct_charge_exit(void)
{
}

static inline int direct_charge_get_cutoff_normal_flag(void)
{
	return -1;
}

static inline void direct_charge_update_cutoff_normal_flag(void)
{
}

static inline void direct_charge_update_charge_info(void)
{
}

static inline void direct_charge_get_ffc_para(void)
{
}

static inline int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	return NOTIFY_OK;
}

static inline int direct_charge_start_charging(void)
{
	return -1;
}

static inline void direct_charge_stop_charging(void)
{
}

static inline int direct_charge_get_stop_charging_flag(void)
{
	return -1;
}

static inline int direct_charge_get_vbus(void)
{
	return 0;
}

static inline int direct_charge_get_ibus(void)
{
	return 0;
}

static inline int direct_charge_get_bat_current(int *value)
{
	return 0;
}

static inline void direct_charge_select_charging_param(void)
{
}

static inline void direct_charge_select_charging_stage(void)
{
}

static inline void direct_charge_soh_policy(void)
{
}

static inline void direct_charge_regulation(void)
{
}

static inline bool direct_charge_check_priority_inversion(void)
{
	return false;
}

static inline bool direct_charge_is_priority_inversion(void)
{
	return false;
}

static inline int lvc_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline int lvc_set_disable_flags(int val, int type)
{
	return -1;
}

static inline int sc_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline int sc_set_disable_flags(int val, int type)
{
	return -1;
}
#endif /* CONFIG_DIRECT_CHARGER */

extern int coul_get_battery_voltage_uv(void);

#endif /* _DIRECT_CHARGER_H_ */
