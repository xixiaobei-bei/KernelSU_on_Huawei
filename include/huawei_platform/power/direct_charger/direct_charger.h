/*
 * direct_charger.h
 *
 * direct charger driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
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
#include <huawei_platform/power/wireless_charger.h>
#endif
#ifdef CONFIG_DP_AUX_SWITCH
#include "huawei_platform/dp_aux_switch/dp_aux_switch.h"
#endif
#include <huawei_platform/power/direct_charger/direct_charger_power_supply.h>
#include <huawei_platform/power/direct_charger/direct_charger_error_handle.h>
#include <huawei_platform/power/direct_charger/direct_charger_para_parse.h>

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

#define DC_ADAPTER_DETECT             1
#define DC_ADAPTER_NOT_DETECT         0

#define VBUS_ON_THRESHOLD             3000
#define VBAT_VBUS_DIFFERENCE          150
#define KICK_WATCHDOG_TIME            1000
#define WATCHDOG_TIMEOUT              2000
#define BATTERY_CAPACITY_HIGH_TH      95
#define BAT_RATED_VOLT                5000

#define DC_LS_RECOVERY_DELAY          500 /* ms */
#define DC_COUL_CURRENT_UNIT_DEVIDE   1000 /* 1000ua equal 1ma */

#define DC_IN_CHARGING_STAGE          0
#define DC_NOT_IN_CHARGING_STAGE      (-1)

#define DC_IN_CHARGE_DONE_STAGE       0
#define DC_NOT_IN_CHARGE_DONE_STAGE   (-1)

#define DC_SET_DISABLE_FLAGS          1
#define DC_CLEAR_DISABLE_FLAGS        0

#define DC_ADP_DIGEST_LEN             16
#define DC_AF_KEY_LEN                 (DC_ADP_DIGEST_LEN * 2 + 1)
#define DC_AF_WAIT_CT_TIMEOUT         1000

/*
 * define working mode with direct charge
 * LVC is simplified identifier with low-voltage-charging
 * SC is simplified identifier with switch-capacitor
 */
enum direct_charge_working_mode {
	UNDEFINED_MODE = 0x0,
	LVC_MODE = 0x1,
	SC_MODE = 0x2,
};

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
 * define charging path with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_charging_path_type {
	PATH_BEGIN,
	PATH_NORMAL = PATH_BEGIN,
	PATH_LVC,
	PATH_SC,
	PATH_END,
};

/*
 * define stage type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_stage_type {
	DC_STAGE_BEGIN = 0,
	DC_STAGE_DEFAULT = DC_STAGE_BEGIN,
	DC_STAGE_SUPPORT_DETECT,
	DC_STAGE_ADAPTER_DETECT,
	DC_STAGE_SWITCH_DETECT,
	DC_STAGE_CHARGE_INIT,
	DC_STAGE_SECURITY_CHECK,
	DC_STAGE_SUCCESS,
	DC_STAGE_CHARGING,
	DC_STAGE_CHARGE_DONE,
	DC_STAGE_END,
};

/*
 * define error code with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_error_code {
	DC_SUCC,
	DC_ERROR_CHARGE_DISABLED,
	DC_ERROR_ADAPTER_DETECT,
	DC_ERROR_BAT_TEMP,
	DC_ERROR_BAT_VOL,
	DC_ERROR_SWITCH,
	DC_ERROR_INIT,
	DC_ERROR_USB_PORT_LEAKAGE_CURRENT,
	DC_ERROR_ADAPTER_VOLTAGE_ACCURACY,
	DC_ERROR_OPEN_CHARGE_PATH,
	DC_ERROR_FULL_REISISTANCE,
	DC_ERROR_ADAPTER_ANTI_FAKE,
};

/*
 * define sysfs type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_sysfs_type {
	DC_SYSFS_BEGIN = 0,
	DC_SYSFS_IIN_THERMAL = DC_SYSFS_BEGIN,
	DC_SYSFS_ADAPTER_DETECT,
	DC_SYSFS_LOADSWITCH_ID,
	DC_SYSFS_LOADSWITCH_NAME,
	DC_SYSFS_VBAT,
	DC_SYSFS_IBAT,
	DC_SYSFS_VADAPT,
	DC_SYSFS_IADAPT,
	DC_SYSFS_LS_VBUS,
	DC_SYSFS_LS_IBUS,
	DC_SYSFS_FULL_PATH_RESISTANCE,
	DC_SYSFS_DIRECT_CHARGE_SUCC,
	DC_SYSFS_SET_RESISTANCE_THRESHOLD,
	DC_SYSFS_SET_CHARGETYPE_PRIORITY,
	DC_SYSFS_THERMAL_REASON,
	DC_SYSFS_AF,
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
 * define fault type for device with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_fault_type {
	/* for common */
	DC_FAULT_NON = 0,
	DC_FAULT_VBUS_OVP,
	DC_FAULT_REVERSE_OCP,
	DC_FAULT_OTP,
	DC_FAULT_TSBUS_OTP,
	DC_FAULT_TSBAT_OTP,
	DC_FAULT_TDIE_OTP,
	DC_FAULT_INPUT_OCP,
	DC_FAULT_VDROP_OVP,
	DC_FAULT_AC_OVP,
	DC_FAULT_VBAT_OVP,
	DC_FAULT_IBAT_OCP,
	DC_FAULT_IBUS_OCP,
	DC_FAULT_CONV_OCP,
	/* for ltc7820 device */
	DC_FAULT_LTC7820,
	/* for ina231 device */
	DC_FAULT_INA231,
	/* for cc and vbus short */
	DC_FAULT_CC_SHORT,
	DC_FAULT_TOTAL,
};

enum direct_charge_lvc_device_id {
	LOADSWITCH_BEGIN,
	LOADSWITCH_RICHTEK = LOADSWITCH_BEGIN,
	LOADSWITCH_TI,
	LOADSWITCH_FAIRCHILD,
	LOADSWITCH_NXP,
	LOADSWITCH_SCHARGERV600,
	LOADSWITCH_FPF2283,
	LOADSWITCH_TOTAL,
};

enum direct_charge_sc_device_id {
	SWITCHCAP_BEGIN,
	SWITCHCAP_TI_BQ25970 = SWITCHCAP_BEGIN,
	SWITCHCAP_SCHARGERV600,
	SWITCHCAP_LTC7820,
	SWITCHCAP_MULTI_SC,
	SWITCHCAP_TOTAL,
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
#define DC_VOLT_LEVEL           5

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

struct direct_charge_volt_para_group {
	struct direct_charge_volt_para volt_info[DC_VOLT_LEVEL];
	struct direct_charge_bat_para bat_info;
	int stage_size;
};

enum direct_charge_status_notify_list {
	LVC_STATUS_CHARGING,
	SC_STATUS_CHARGING,
	DC_STATUS_STOP_CHARGE,
};

enum direct_charge_info_type {
	CC_CABLE_DETECT_OK,
};

enum direct_charge_cable_type {
	DC_UNKNOWN_CABLE,
	DC_NONSTD_CABLE,
	DC_STD_CABLE,
};

struct nty_data {
	unsigned short addr;
	u8 event1;
	u8 event2;
	u8 event3;
};

struct loadswitch_ops {
	int (*ls_init)(void);
	int (*ls_exit)(void);
	int (*ls_enable)(int);
	int (*ls_discharge)(int);
	int (*is_ls_close)(void);
	int (*get_ls_id)(void);
	int (*watchdog_config_ms)(int time);
	int (*kick_watchdog)(void);
	int (*ls_status)(void);
	int (*ls_enable_prepare)(void);
};

struct batinfo_ops {
	int (*init)(void);
	int (*exit)(void);
	int (*get_bat_btb_voltage)(void);
	int (*get_bat_package_voltage)(void);
	int (*get_vbus_voltage)(int *vol);
	int (*get_bat_current)(int *cur);
	int (*get_ls_ibus)(int *ibus);
	int (*get_ls_temp)(int *temp);
};

/* define protocol power supply oprator for direct charge */
struct direct_charge_pps_ops {
	int (*power_supply_enable)(int);
};

/* define cable detect oprator for direct charge */
struct direct_charge_cd_ops {
	int (*cable_detect)(void);
};

struct direct_charge_device {
	struct device *dev;
	struct direct_charge_pps_ops *pps_ops;
	struct loadswitch_ops *ls_ops;
	struct batinfo_ops *bi_ops;
	struct direct_charge_cd_ops *cd_ops;
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
	struct direct_charge_volt_para_group *orig_volt_para_p;
	struct direct_charge_temp_para temp_para[DC_TEMP_LEVEL];
	struct direct_charge_resist_para nonstd_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para std_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para ctc_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_adp_cur_para adp_10v2p25a[DC_ADP_CUR_LEVEL];
	struct direct_charge_adp_cur_para adp_10v2p25a_car[DC_ADP_CUR_LEVEL];
	struct nty_data *fault_data;
	int stage_need_to_jump[2 * DC_VOLT_LEVEL];
	int error_cnt;
	int working_mode;
	u32 use_5a;
	u32 use_8a;
	int sysfs_enable_charger;
	unsigned int sysfs_disable_charger[DC_DISABLE_END];
	int sysfs_iin_thermal;
	u32 threshold_caculation_interval;
	u32 charge_control_interval;
	int cur_stage;
	int cur_vbat_th;
	int cur_ibat_th_high;
	int cur_ibat_th_low;
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
	int init_adapter_vset;
	int delta_err;
	int delta_err_10v2p25a;
	int vstep;
	int scp_stop_charging_flag_info;
	int stop_charging_flag_error;
	int max_dc_bat_vol;
	int min_dc_bat_vol;
	int super_ico_current;
	u32 is_show_ico_first;
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
	int dc_succ_flag;
	u32 first_cc_stage_timer_in_min;
	int max_adaptor_cur;
	int scp_cable_detect_enable;
	unsigned long first_cc_stage_timeout;
	u32 cc_cable_detect_enable;
	int cc_cable_detect_ok;
	int max_current_for_nonstd_cable;
	int max_current_for_ctc_cable;
	enum direct_charge_fault_type charge_fault;
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
	int adapter_detect_flag;
	char dsm_buff[DC_DMDLOG_SIZE];
	int scp_stop_charging_complete_flag;
	struct wakeup_source charging_lock;
	int adaptor_test_result_type;
	char thermal_reason[DC_THERMAL_REASON_SIZE];
	u32 adp_antifake_key_index;
	u32 adp_antifake_enable;
	int dc_antifake_result;
	struct completion dc_af_completion;
	u8 dc_af_key[DC_AF_KEY_LEN];
	int sc_conv_ocp_count;
	int iin_thermal_default;
	int can_stop_kick_wdt;
	int last_basp_level;
	u32 gain_curr_10v2a;
	u32 gain_curr_10v2p25a;
	u32 gain_curr_10v2p25a_car;
	u32 reset_adap_volt_enabled;
	unsigned int vterm_dec;
	unsigned int ichg_ratio;
	u32 low_temp_hysteresis;
	u32 orig_low_temp_hysteresis;
	u32 high_temp_hysteresis;
	u32 orig_high_temp_hysteresis;
	u32 rt_curr_th;
	u32 rt_test_time;
	bool bat_temp_err_flag;
	bool rt_test_succ;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
	bool nonstd_cable_flag;
	u32 need_wired_sw_off;
	int cable_type;
	int max_pwr;
	int ui_max_pwr;
	bool pri_inversion;
	int cur_inversion;
	u32 adapter_type;
};

#ifdef CONFIG_DIRECT_CHARGER
int direct_charge_pps_ops_register(struct direct_charge_pps_ops *ops);
int direct_charge_cd_ops_register(struct direct_charge_cd_ops *ops);
void direct_charge_get_g_pps_ops(struct direct_charge_pps_ops **ops);
void direct_charge_get_g_cd_ops(struct direct_charge_cd_ops **ops);
int direct_charge_notifier_chain_register(struct notifier_block *nb);
int direct_charge_notifier_chain_unregister(struct notifier_block *nb);
void direct_charge_set_di(struct direct_charge_device *di);
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
int direct_charge_is_failed(void);
void direct_charge_set_disable_flags(int val, int type);
int direct_charge_get_info(enum direct_charge_info_type type, int *value);
void direct_charge_send_normal_charging_uevent(void);
void direct_charge_send_quick_charging_uevent(void);
int direct_charge_get_quick_charging_flag(void);
int direct_charge_get_super_charging_flag(void);
int direct_charge_check_battery_temp(void);
int direct_charge_check_battery_voltage(void);
int direct_charge_update_adapter_info(void);
int direct_charge_set_adapter_output_enable(int enable);
void direct_charge_set_adapter_default_param(void);
void direct_charge_force_disable_dc_path(void);
int direct_charge_get_protocol_register_state(void);
int direct_charge_init_adapter_and_device(void);
int direct_charge_security_check(void);
int direct_charge_check_adapter_antifake(void);
void direct_charge_detect_cable(void);
int direct_charge_switch_charging_path(unsigned int path);
bool direct_charge_get_stop_charging_complete_flag(void);
void direct_charge_set_stop_charging_flag(int value);
int direct_charge_pre_check(void);
void direct_charge_check(void);
int direct_charge_detect_adapter_again(void);
void direct_charge_exit(void);
int direct_charge_get_cutoff_normal_flag(void);
void direct_charge_update_cutoff_normal_flag(void);
void direct_charge_control_work(struct work_struct *work);
void direct_charge_calc_thld_work(struct work_struct *work);
void direct_charge_kick_wtd_work(struct work_struct *work);
enum hrtimer_restart direct_charge_calc_thld_timer_func(struct hrtimer *timer);
enum hrtimer_restart direct_charge_control_timer_func(struct hrtimer *timer);
enum hrtimer_restart direct_charge_kick_wtd_timer_func(struct hrtimer *timer);
int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data);
void direct_charge_start_charging(void);
void direct_charge_stop_charging(void);
int direct_charge_get_vbus(void);
int direct_charge_get_ibus(void);
int lvc_ops_register(struct loadswitch_ops *ops);
int lvc_batinfo_ops_register(struct batinfo_ops *ops);
int lvc_get_di(struct direct_charge_device **di);
void lvc_get_fault_notifier(struct atomic_notifier_head **notifier);
int lvc_set_disable_flags(int val, int type);
void lvc_mode_check(void);
int sc_ops_register(struct loadswitch_ops *ops);
int sc_batinfo_ops_register(struct batinfo_ops *ops);
int sc_get_di(struct direct_charge_device **di);
void sc_get_fault_notifier(struct atomic_notifier_head **notifier);
int sc_set_disable_flags(int val, int type);
void sc_mode_check(void);
#else
static inline int direct_charge_pps_ops_register(
	struct direct_charge_pps_ops *ops)
{
	return -1;
}

static inline int direct_charge_cd_ops_register(
	struct direct_charge_cd_ops *ops)
{
	return -1;
}

static inline void direct_charge_get_g_pps_ops(
	struct direct_charge_pps_ops **ops)
{
}

static inline void direct_charge_get_g_cd_ops(
	struct direct_charge_cd_ops **ops)
{
}

static inline int direct_charge_notifier_chain_register(
	struct notifier_block *nb)
{
	return NOTIFY_OK;
}

static inline int direct_charge_notifier_chain_unregister(
	struct notifier_block *nb)
{
	return NOTIFY_OK;
}

static inline void direct_charge_set_di(struct direct_charge_device *di)
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
	return 0;
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

static inline void direct_charge_send_normal_charging_uevent(void)
{
}

static inline void direct_charge_send_quick_charging_uevent(void)
{
}

static inline int direct_charge_get_quick_charging_flag(void)
{
	return -1;
}

static inline int direct_charge_get_super_charging_flag(void)
{
	return -1;
}

static inline int direct_charge_check_battery_temp(void)
{
	return -1;
}

static inline int direct_charge_check_battery_voltage(void)
{
	return -1;
}

static inline int direct_charge_update_adapter_info(void)
{
	return -1;
}

static inline int direct_charge_set_adapter_output_enable(int enable)
{
	return -1;
}

static inline void direct_charge_set_adapter_default_param(void)
{
}

static inline void direct_charge_force_disable_dc_path(void)
{
}

static inline int direct_charge_get_protocol_register_state(void)
{
	return -1;
}

static inline int direct_charge_init_adapter_and_device(void)
{
	return -1;
}

static inline int direct_charge_security_check(void)
{
	return -1;
}

static inline int direct_charge_check_adapter_antifake(void)
{
	return -1;
}

static inline void direct_charge_detect_cable(void)
{
}

static inline int direct_charge_switch_charging_path(unsigned int path)
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

static inline int direct_charge_pre_check(void)
{
	return -1;
}

static inline void direct_charge_check(void)
{
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

static inline void direct_charge_control_work(struct work_struct *work)
{
}

static inline void direct_charge_calc_thld_work(struct work_struct *work)
{
}

static inline void direct_charge_kick_wtd_work(struct work_struct *work)
{
}

static inline enum hrtimer_restart direct_charge_calc_thld_timer_func(
	struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}

static inline enum hrtimer_restart direct_charge_control_timer_func(
	struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}

static inline enum hrtimer_restart direct_charge_kick_wtd_timer_func(
	struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}

static inline int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	return NOTIFY_OK;
}

static inline void direct_charge_start_charging(void)
{
}

static inline void direct_charge_stop_charging(void)
{
}

static inline int direct_charge_get_vbus(void)
{
	return 0;
}

static inline int direct_charge_get_ibus(void)
{
	return 0;
}

static inline int lvc_ops_register(struct loadswitch_ops *ops)
{
	return -EINVAL;
}

static inline int lvc_batinfo_ops_register(struct batinfo_ops *ops)
{
	return -EINVAL;
}

static inline int lvc_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline void lvc_get_fault_notifier(
	struct atomic_notifier_head **notifier)
{
}

static inline int lvc_set_disable_flags(int val, int type)
{
	return -1;
}

static inline void lvc_mode_check(void)
{
}

static inline int sc_ops_register(struct loadswitch_ops *ops)
{
	return -EINVAL;
}

static inline int sc_batinfo_ops_register(struct batinfo_ops *ops)
{
	return -EINVAL;
}

static inline int sc_get_di(struct direct_charge_device **di)
{
	return -1;
}

static inline void sc_get_fault_notifier(struct atomic_notifier_head **notifier)
{
}

static inline int sc_set_disable_flags(int val, int type)
{
	return -1;
}

static inline void sc_mode_check(void)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#ifdef CONFIG_WIRELESS_CHARGER
extern bool wltx_need_disable_wired_dc(void);
#endif
extern int coul_get_battery_voltage_uv(void);

#endif /* _DIRECT_CHARGER_H_ */
