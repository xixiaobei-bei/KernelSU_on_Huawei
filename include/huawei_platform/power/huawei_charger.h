/*
 * huawei_charger.h
 *
 * huawei charger driver
 *
 * Copyright (C) 2012-2015 Huawei Technologies Co., Ltd.
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

#ifndef _HUAWEI_CHARGER
#define _HUAWEI_CHARGER

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/bitops.h>
#include <linux/time.h>
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_common.h>

#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/speaker_charger/series_batt_speaker_charger.h>
#include <huawei_platform/power/speaker_charger/series_batt_speaker_smart_charger.h>
#include <huawei_platform/power/uvdm_charger/uvdm_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <huawei_platform/power/huawei_charger_uevent.h>
#include <chipset_common/hwpower/charger/charger_type.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel_charger.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_scp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_fcp.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_pd.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_uvdm.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/hardware_monitor/water_detect.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_control.h>
#include <chipset_common/hwpower/adapter/adapter_test.h>

/* marco define area */
#ifndef TRUE
#define TRUE                                1
#endif
#ifndef FALSE
#define FALSE                               0
#endif
#define get_index(x)                        ((x) - ERROR_FCP_VOL_OVER_HIGH)
#define ERR_NO_STRING_SIZE                  256
#define CHARGELOG_SIZE                      4096
#define CHARGE_DMDLOG_SIZE                  2048
#define CHARGE_SYSFS_BUF_SIZE               10
#define CHIP_RESP_TIME                      200

#define TERM_ERR_CNT                        120

/* sensor_id#scene_id#stage */
#define THERMAL_REASON_SIZE                 16

#define HIZ_MODE_ENABLE                     1
#define HIZ_MODE_DISABLE                    0

/* charge threshold */
#define NO_CHG_TEMP_LOW                     0
#define NO_CHG_TEMP_HIGH                    50
#define BATT_EXIST_TEMP_LOW                 (-40)
#define DEFAULT_NORMAL_TEMP                 25

/* 0: dbc close charger, 1: dbc open charger, 2: dbc not control charger */
#define CHARGER_NOT_DBC_CONTROL             2

/* fcp detect */
#define FCP_ADAPTER_DETECT_FAIL             1
#define FCP_ADAPTER_DETECT_SUCC             0
#define FCP_ADAPTER_DETECT_OTHER            (-1)

#define FCP_CHECK_CNT_MAX                   3

/* options of charge current(include input current & charge into battery current) */
#define CHARGE_CURRENT_0000_MA              0
#define CHARGE_CURRENT_0100_MA              100
#define CHARGE_CURRENT_0150_MA              150
#define CHARGE_CURRENT_0500_MA              500
#define CHARGE_CURRENT_0800_MA              800
#define CHARGE_CURRENT_1000_MA              1000
#define CHARGE_CURRENT_1200_MA              1200
#define CHARGE_CURRENT_1900_MA              1900
#define CHARGE_CURRENT_2000_MA              2000
#define CHARGE_CURRENT_4000_MA              4000
#define CHARGE_CURRENT_MAX_MA               32767

#define CHARGE_CURRENT_DELAY                100

/* options of battery voltage */
#define BATTERY_VOLTAGE_0000_MV             0
#define BATTERY_VOLTAGE_0200_MV             200

#define BATTERY_VOLTAGE_MIN_MV              (-32767)
#define BATTERY_VOLTAGE_MAX_MV              32767
#define BATTERY_VOLTAGE_3200_MV             3200
#define BATTERY_VOLTAGE_3400_MV             3400
#define BATTERY_VOLTAGE_4100_MV             4200
#define BATTERY_VOLTAGE_4200_MV             4200
#define BATTERY_VOLTAGE_4350_MV             4350
#define BATTERY_VOLTAGE_4500_MV             4500

/* options of NTC battery temperature */
#define BATTERY_TEMPERATURE_MIN             (-32767)
#define BATTERY_TEMPERATURE_MAX             32767
#define BATTERY_TEMPERATURE_0_C             0
#define BATTERY_TEMPERATURE_5_C             5

#define PD_EVENT_NUM_TH1                    3
#define PD_EVENT_NUM_TH2                    10

#define CHARGING_WORK_TIMEOUT               30000
#define CHARGING_WORK_TIMEOUT1              20000
#define CHARGING_WORK_PDTOSCP_TIMEOUT       1000
#define CHARGING_WORK_WAITPD_TIMEOUT        2000
#define MIN_CHARGING_CURRENT_OFFSET         (-10)
#define BATTERY_FULL_CHECK_TIMIES           2
#define IIN_AVG_SAMPLES                     10
#define IMPOSSIBLE_IIN                      999999
#define CURRENT_FULL_CHECK_TIMES            3
#define DELA_ICHG_FOR_CURRENT_FULL          30
#define CURRENT_FULL_VOL_OFFSET             50
#define CURRENT_FULL_VALID_PERCENT          80
#define CAP_TH_FOR_CURRENT_FULL             80

#define WATCHDOG_TIMER_DISABLE              0
#define WATCHDOG_TIMER_40_S                 40
#define WATCHDOG_TIMER_80_S                 80
#define WATCHDOG_TIMER_160_S                160

#define ADAPTER_0V                          0
#define ADAPTER_5V                          5
#define ADAPTER_9V                          9
#define ADAPTER_12V                         12

/* options of charge voltage (for dpm voltage setting, also ovp & uvp protect) */
#define CHARGE_VOLTAGE_4360_MV              4360
#define CHARGE_VOLTAGE_4520_MV              4520
#define CHARGE_VOLTAGE_4600_MV              4600
#define CHARGE_VOLTAGE_5000_MV              5000
#define CHARGE_VOLTAGE_6300_MV              6300
#define CHARGE_VOLTAGE_6500_MV              6500

/* options of charge states from chip */
#define CHAGRE_STATE_NORMAL                 0x00
#define CHAGRE_STATE_VBUS_OVP               0x01
#define CHAGRE_STATE_NOT_PG                 0x02
#define CHAGRE_STATE_WDT_FAULT              0x04
#define CHAGRE_STATE_BATT_OVP               0x08
#define CHAGRE_STATE_CHRG_DONE              0x10
#define CHAGRE_STATE_INPUT_DPM              0x20
#define CHAGRE_STATE_NTC_FAULT              0x40
#define CHAGRE_STATE_CV_MODE                0x80

/* options of vbus voltage */
#define VBUS_VOL_READ_CNT                   3
#define VBUS_VOLTAGE_13400_MV               13400
#define SLEEP_110MS                         110
#define COOL_LIMIT                          11
#define WARM_LIMIT                          44
#define WARM_CUR_RATIO                      35
#define RATIO_BASE                          100
#define IBIS_RATIO                          120

#define VBUS_VOLTAGE_7000_MV                7000
#define VBUS_VOLTAGE_6500_MV                6500
#define VBUS_VOLTAGE_ABNORMAL_MAX_COUNT     2

/* Type C mode current */
#define TYPE_C_HIGH_MODE_CURR               3000
#define TYPE_C_MID_MODE_CURR                1500
#define TYPE_C_DEFAULT_MODE_CURR            500

#define MAX_EVENT_COUNT                     16
#define EVENT_QUEUE_UNIT                    MAX_EVENT_COUNT
#define WEAKSOURCE_CNT                      10

#define CHARGER_SET_DISABLE_FLAGS           1
#define CHARGER_CLEAR_DISABLE_FLAGS         0

#define PD_VOLTAGE_CHANGE_WORK_TIMEOUT      2000

#define OTG_ENABLE                          1
#define OTG_DISABLE                         0
#define IIN_REGL_INTERVAL_DEFAULT           500
#define IIN_REGL_STAGE_MAX                  20

#define NO_EVENT                            (-1)

#define RT_TEST_TEMP_TH                     45

#define BASP_PARA_SCALE                     100

#define SDP_IIN_USB                         475

#define FFC_VTERM_START_FLAG                BIT(0)
#define FFC_VETRM_END_FLAG                  BIT(1)

/* struct define area */
enum charger_event_type {
	START_SINK = 0,
	STOP_SINK,
	START_SOURCE,
	STOP_SOURCE,
	START_SINK_WIRELESS,
	STOP_SINK_WIRELESS,
	CHARGER_MAX_EVENT,
};

enum reset_adapter_source_type {
	RESET_ADAPTER_SYSFS = 0,
	RESET_ADAPTER_WIRELESS_TX,
	RESET_ADAPTER_TOTAL,
};

enum iin_thermal_charge_type {
	IIN_THERMAL_CHARGE_TYPE_BEGIN = 0,
	IIN_THERMAL_WCURRENT_5V = IIN_THERMAL_CHARGE_TYPE_BEGIN,
	IIN_THERMAL_WCURRENT_9V,
	IIN_THERMAL_WLCURRENT_5V,
	IIN_THERMAL_WLCURRENT_9V,
	IIN_THERMAL_CHARGE_TYPE_END,
};

enum charge_sysfs_type {
	CHARGE_SYSFS_IIN_THERMAL = 0,
	CHARGE_SYSFS_ICHG_THERMAL,
	CHARGE_SYSFS_IIN_RUNNINGTEST,
	CHARGE_SYSFS_ICHG_RUNNINGTEST,
	CHARGE_SYSFS_LIMIT_CHARGING,
	CHARGE_SYSFS_REGULATION_VOLTAGE,
	CHARGE_SYSFS_BATFET_DISABLE,
	CHARGE_SYSFS_WATCHDOG_DISABLE,
	CHARGE_SYSFS_IBUS,
	CHARGE_SYSFS_VBUS,
	CHARGE_SYSFS_HIZ,
	CHARGE_SYSFS_CHARGE_TYPE,
	CHARGE_SYSFS_CHARGE_DONE_STATUS,
	CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS,
	CHARGE_SYSFS_INPUTCURRENT,
	CHARGE_SYSFS_VOLTAGE_SYS,
	CHARGE_SYSFS_ADC_CONV_RATE,
	CHARGE_SYSFS_VR_CHARGER_TYPE,
	CHARGE_SYSFS_SUPPORT_ICO,
	CHARGE_SYSFS_CHARGE_TERM_VOLT_DESIGN,
	CHARGE_SYSFS_CHARGE_TERM_CURR_DESIGN,
	CHARGE_SYSFS_CHARGE_TERM_VOLT_SETTING,
	CHARGE_SYSFS_CHARGE_TERM_CURR_SETTING,
	CHARGE_SYSFS_FCP_SUPPORT,
	CHARGE_SYSFS_DBC_CHARGE_CONTROL,
	CHARGE_SYSFS_DBC_CHARGE_DONE,
	CHARGE_SYSFS_ADAPTOR_VOLTAGE,
	CHARGE_SYSFS_PLUGUSB,
	CHARGE_SYSFS_THERMAL_REASON,
	CHARGE_SYSFS_CHARGER_ONLINE,
	CHARGE_SYSFS_CHARGER_CVCAL,
	CHARGE_SYSFS_CHARGER_CVCAL_CLEAR,
	CHARGE_SYSFS_CHARGER_GET_CVSET,
};

enum charge_done_type {
	CHARGE_DONE_NON = 0,
	CHARGE_DONE,
};

enum charge_done_sleep_type {
	CHARGE_DONE_SLEEP_DISABLED = 0,
	CHARGE_DONE_SLEEP_ENABLED,
};

enum fcp_check_stage_type {
	FCP_STAGE_DEFAUTL,
	FCP_STAGE_SUPPORT_DETECT,
	FCP_STAGE_ADAPTER_DETECT,
	FCP_STAGE_ADAPTER_ENABLE,
	FCP_STAGE_SUCESS,
	FCP_STAGE_CHARGE_DONE,
	FCP_STAGE_RESET_ADAPTOR,
	FCP_STAGE_ERROR,
};

enum fcp_retry_operate_type {
	FCP_RETRY_OPERATE_DEFAUTL,
	FCP_RETRY_OPERATE_RESET_ADAPTER,
	FCP_RETRY_OPERATE_RESET_SWITCH,
	FCP_RETRY_OPERATE_UNVALID,
};

enum fcp_test_result {
	FCP_TEST_SUCC,
	FCP_TEST_FAIL,
	FCP_NOT_SUPPORT,
};

enum disable_charger_type {
	CHARGER_SYS_NODE = 0,
	CHARGER_FATAL_ISC_TYPE,
	CHARGER_WIRELESS_TX,
	BATT_CERTIFICATION_TYPE,
	__MAX_DISABLE_CHAGER,
};

enum soft_recharge_para {
	RECHARGE_SOC_TH = 0,
	RECHARGE_VOL_TH,
	RECHARGE_CC_TH,
	RECHARGE_TIMES_TH,
	RECHARGE_DMD_SWITCH,
	RECHARGE_NT_SWITCH, /* night time switch */
	RECHARGE_PARA_TOTAL,
};

static const char *const fcp_check_stage[] = {
	[0] = "FCP_STAGE_DEFAUTL",
	[1] = "FCP_STAGE_SUPPORT_DETECT",
	[2] = "FCP_STAGE_ADAPTER_DETECT",
	[3] = "FCP_STAGE_ADAPTER_ENABLE",
	[4] = "FCP_STAGE_SUCESS",
};

struct charger_event_queue {
	enum charger_event_type *event;
	unsigned int num_event;
	unsigned int max_event;
	unsigned int enpos, depos;
	unsigned int overlay, overlay_index;
};

struct ico_input {
	unsigned int charger_type;
	unsigned int iin_max;
	unsigned int ichg_max;
	unsigned int vterm;
};

struct ico_output {
	unsigned int input_current;
	unsigned int charge_current;
};

struct chip_init_crit {
	unsigned int charger_type;
	int vbus;
};

struct charge_sysfs_data {
	unsigned int adc_conv_rate;
	unsigned int iin_thl;
	unsigned int iin_thl_array[IIN_THERMAL_CHARGE_TYPE_END];
	unsigned int ichg_thl;
	unsigned int iin_rt;
	unsigned int ichg_rt;
	unsigned int vterm_rt;
	unsigned int charge_limit;
	unsigned int wdt_disable;
	unsigned int charge_enable;
	unsigned int fcp_charge_enable;
	unsigned int disable_charger[__MAX_DISABLE_CHAGER];
	unsigned int batfet_disable;
	unsigned int vr_charger_type;
	unsigned int dbc_charge_control;
	enum charge_done_type charge_done_status;
	enum charge_done_sleep_type charge_done_sleep_status;
	int ibus;
	int vbus;
	int inputcurrent;
	int voltage_sys;
	unsigned int support_ico;
	unsigned int fcp_support;
	int charger_cvcal_value;
	int charger_cvcal_clear;
	int charger_get_cvset;
};

struct charge_core_data {
	unsigned int iin;
	unsigned int ichg;
	unsigned int vterm;
	unsigned int iin_ac;
	unsigned int ichg_ac;
	unsigned int iin_usb;
	unsigned int ichg_usb;
	unsigned int iin_nonstd;
	unsigned int ichg_nonstd;
	unsigned int iin_bc_usb;
	unsigned int ichg_bc_usb;
	unsigned int iin_vr;
	unsigned int ichg_vr;
	unsigned int iin_pd;
	unsigned int ichg_pd;
	unsigned int iin_fcp;
	unsigned int ichg_fcp;
	unsigned int iin_nor_scp;
	unsigned int ichg_nor_scp;
	unsigned int iin_weaksource;
	unsigned int iterm;
	unsigned int vdpm;
	unsigned int vdpm_control_type;
	unsigned int vdpm_buf_limit;
	unsigned int iin_max;
	unsigned int ichg_max;
	unsigned int otg_curr;
	unsigned int iin_typech;
	unsigned int ichg_typech;
	unsigned int typec_support;
	unsigned int segment_type;
	unsigned int segment_level;
	unsigned int temp_level;
	unsigned int high_temp_limit;
	bool warm_triggered;
#ifdef CONFIG_WIRELESS_CHARGER
	unsigned int iin_wireless;
	unsigned int ichg_wireless;
#endif
	unsigned int vterm_bsoh;
	unsigned int ichg_bsoh;
	unsigned int battery_cell_num;
	unsigned int vterm_low_th;
	unsigned int vterm_high_th;
};

struct charge_iin_regl_lut {
	int total_stage;
	int *iin_regl_para;
};

struct charge_device_ops {
	int (*chip_init)(struct chip_init_crit *init_crit);
	int (*set_adc_conv_rate)(int rate_mode);
	int (*set_input_current)(int value);
	void (*set_input_current_thermal)(int val1, int val2);
	int (*set_charge_current)(int value);
	void (*set_charge_current_thermal)(int val1, int val2);
	int (*dev_check)(void);
	int (*set_terminal_voltage)(int value);
	int (*set_dpm_voltage)(int value);
	int (*set_terminal_current)(int value);
	int (*set_charge_enable)(int enable);
	int (*get_charge_enable_status)(void);
	int (*set_otg_enable)(int enable);
	int (*set_term_enable)(int enable);
	int (*get_charge_state)(unsigned int *state);
	int (*reset_watchdog_timer)(void);
	int (*set_watchdog_timer)(int value);
	int (*set_batfet_disable)(int disable);
	int (*get_ibus)(void);
	int (*get_vbus)(unsigned int *value);
	int (*check_charger_plugged)(void);
	int (*check_input_dpm_state)(void);
	int (*check_input_vdpm_state)(void);
	int (*check_input_idpm_state)(void);
	int (*set_covn_start)(int enable);
	int (*set_charger_hiz)(int enable);
	int (*set_otg_current)(int value);
	int (*stop_charge_config)(void);
	int (*set_otg_switch_mode_enable)(int enable);
	int (*get_vbat_sys)(void);
	int (*set_vbus_vset)(u32);
	int (*set_uvp_ovp)(void);
	int (*turn_on_ico)(struct ico_input *, struct ico_output *);
	int (*set_force_term_enable)(int enable);
	int (*get_charger_state)(void);
	int (*soft_vbatt_ovp_protect)(void);
	int (*rboost_buck_limit)(void);
	int (*get_charge_current)(void);
	int (*get_iin_set)(void);
	int (*set_boost_voltage)(int voltage);
	int (*get_dieid)(char *dieid, unsigned int len);
	int (*get_vbat)(void);
	int (*get_terminal_voltage)(void);
	int (*get_vusb)(int *value);
	int (*set_pretrim_code)(int val);
	int (*get_dieid_for_nv)(u8 *dieid, unsigned int len);
};

struct charge_switch_ops {
	enum hisi_charger_type (*get_charger_type)(void);
};

#ifdef CONFIG_TCPC_CLASS
struct tcpc_device;
#endif
struct charge_device_info {
	struct device *dev;
	struct notifier_block usb_nb;
	struct notifier_block fault_nb;
	struct notifier_block typec_nb;
	struct delayed_work charge_work;
	struct delayed_work plugout_uscp_work;
	struct delayed_work pd_voltage_change_work;
	struct work_struct usb_work;
	struct work_struct fault_work;
	struct charge_device_ops *ops;
	struct charge_switch_ops *sw_ops;
	struct charge_core_data *core_data;
	struct charge_sysfs_data sysfs_data;
	struct hrtimer timer;
	struct mutex mutex_hiz;
#ifdef CONFIG_TCPC_CLASS
	struct notifier_block tcpc_nb;
	struct tcpc_device *tcpc;
	unsigned int tcpc_otg;
	struct mutex tcpc_otg_lock;
	struct pd_dpm_vbus_state *vbus_state;
#endif
	unsigned int recharge_para[RECHARGE_PARA_TOTAL];
	unsigned int pd_input_current;
	unsigned int pd_charge_current;
	unsigned int charger_type;
	enum typec_input_current typec_current_mode;
	enum power_supply_type charger_source;
	unsigned int charge_fault;
	unsigned int charge_enable;
	unsigned int input_current;
	unsigned int charge_current;
	unsigned int input_typec_current;
	unsigned int charge_typec_current;
	unsigned int enable_current_full;
	unsigned int check_current_full_count;
	unsigned int check_full_count;
	unsigned int start_attemp_ico;
	unsigned int support_standard_ico;
	unsigned int ico_current_mode;
	unsigned int ico_all_the_way;
	unsigned int fcp_vindpm;
	unsigned int hiz_ref;
	unsigned int check_ibias_sleep_time;
	unsigned int need_filter_pd_event;
	u32 force_disable_dc_path;
	u32 scp_adp_normal_chg;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
#ifdef CONFIG_DIRECT_CHARGER
	int ignore_pluggin_and_plugout_flag;
	int support_scp_power;
#endif
	unsigned int reset_adapter;
#ifdef CONFIG_WIRELESS_CHARGER
	struct notifier_block wireless_nb;
	int wireless_vbus;
	int otg_channel;
	int gpio_otg_switch;
	unsigned int iin_limit;
#endif
	int weaksource_cnt;
	struct mutex event_type_lock;
	unsigned int charge_done_maintain_fcp;
	unsigned int term_exceed_time;
	struct work_struct event_work;
	spinlock_t event_spin_lock;
	enum charger_event_type event;
	struct charger_event_queue event_queue;
	unsigned int clear_water_intrused_flag_after_read;
	char thermal_reason[THERMAL_REASON_SIZE];
	int avg_iin_ma;
	int max_iin_ma;
	int current_full_status;
#ifdef CONFIG_HUAWEI_YCABLE
	struct notifier_block ycable_nb;
#endif
#ifdef CONFIG_POGO_PIN
	struct notifier_block pogopin_nb;
	struct notifier_block pogopin_otg_typec_chg_nb;
#endif
	int iin_regulation_enabled;
	int iin_regl_interval;
	int iin_now;
	int iin_target;
	struct mutex iin_regl_lock;
	struct charge_iin_regl_lut iin_regl_lut;
	struct delayed_work iin_regl_work;
	u32 rt_curr_th;
	u32 rt_test_time;
	bool rt_test_succ;
	u32 increase_term_volt_en;
	u32 ffc_vterm_flag;
	bool is_dc_enable_hiz;
#ifdef CONFIG_HUAWEI_SPEAKER_CHARGER
	u32 scp_cur_trans_ratio;
	u32 scp_vindpm;
#endif /* CONFIG_HUAWEI_SPEAKER_CHARGER */
	struct timeval charge_start_time;
	struct timeval charge_done_time;
	u32 smart_charge_support;
};
enum charge_wakelock_flag {
	CHARGE_NEED_WAKELOCK,
	CHARGE_NO_NEED_WAKELOCK,
};

#define WDT_TIME_80_S                       80
#define WDT_STOP                            0
#define CHARGE_IC_GOOD                      0
#define CHARGE_IC_BAD                       1
#define MAIN_CHARGER                        1
#define AUX_CHARGER                         0
#define WEAKSOURCE_TRUE                     1
#define WEAKSOURCE_FALSE                    0
#define FCP_DETECT_DELAY_IN_POWEROFF_CHARGE 2000
#define ASW_PROTECT_IIN_LIMIT               100
/* variable and function declarationn area */
extern struct blocking_notifier_head charge_wake_unlock_list;
extern unsigned int get_pd_charge_flag(void);
#ifdef CONFIG_HISI_ASW
extern int asw_get_iin_limit(void);
#else
static inline int asw_get_iin_limit(void)
{
	return 0;
}
#endif /* CONFIG_HISI_ASW */

enum fcp_check_stage_type fcp_get_stage_status(void);
#ifdef CONFIG_HUAWEI_CHARGER_AP
int charge_ops_register(struct charge_device_ops *ops);
int charge_switch_ops_register(struct charge_switch_ops *ops);
void charge_type_dcp_detected_notify(void);
int fcp_test_is_support(void);
int fcp_test_detect_adapter(void);
int get_fcp_charging_flag(void);
int charge_get_vusb(int *value);
int charge_set_input_current(int iset);
int charge_get_charger_iinlim_regval(void);
int huawei_charger_get_dieid(char *dieid, unsigned int len);
enum charge_done_type get_charge_done_type(void);
#else
static inline int charge_ops_register(struct charge_device_ops *ops)
{
	return 0;
}
static inline int charge_switch_ops_register(struct charge_switch_ops *ops)
{
	return 0;
}
static inline void charge_type_dcp_detected_notify(void) { }
static inline int fcp_test_is_support(void)
{
	return 0;
}
static inline int fcp_test_detect_adapter(void)
{
	return 0;
}
static inline int get_fcp_charging_flag(void)
{
	return 0;
}
static inline int charge_get_vusb(int *value)
{
	return 0;
}
static inline int charge_set_input_current(int iset)
{
	return 0;
}
static inline int charge_get_charger_iinlim_regval(void)
{
	return 0;
}
static inline int huawei_charger_get_dieid(char *dieid, unsigned int len)
{
	return 0;
}
static inline enum charge_done_type get_charge_done_type(void)
{
	return CHARGE_DONE_NON;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP */

#ifdef CONFIG_TCPC_CLASS
#ifdef CONFIG_HUAWEI_CHARGER_AP
int is_pd_supported(void);
#else
static inline int is_pd_supported(void)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP */
#endif /* CONFIG_TCPC_CLASS */

#ifdef CONFIG_DIRECT_CHARGER
#ifdef CONFIG_HUAWEI_CHARGER_AP
void direct_charger_disconnect_update_charger_type(void);
void ignore_pluggin_and_pluggout_interrupt(void);
void restore_pluggin_pluggout_interrupt(void);
int get_direct_charge_flag(void);
int charger_disable_usbpd(bool disable);
#else
static inline void direct_charger_disconnect_update_charger_type(void) { }
static inline void ignore_pluggin_and_pluggout_interrupt(void) { }
static inline void restore_pluggin_pluggout_interrupt(void) { }
static inline int get_direct_charge_flag(void)
{
	return 0;
}
static inline int charger_disable_usbpd(bool disable)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP */
#endif /* CONFIG_DIRECT_CHARGER */

#ifdef CONFIG_WIRELESS_CHARGER
void charge_set_input_current_prop(int iin_step, int iin_delay);
void wireless_channel_off_update_charger_type(void);
#endif

#ifdef CONFIG_HUAWEI_CHARGER_AP
int set_charger_disable_flags(int, int);
int adaptor_cfg_for_wltx_vbus(int vol, int cur);

void water_detection_entrance(void);
int charge_get_vbus(void);
void charge_set_charger_type(unsigned int type);
void charger_source_sink_event(enum charger_event_type event);
bool get_stop_charge_sync_flag(void);
int charge_otg_mode_enable(int enable, unsigned int user);
void charge_set_adapter_voltage(int val, enum reset_adapter_source_type type,
	unsigned int delay_time);
int charge_set_input_current_max(void);
bool charge_get_hiz_state(void);
int charge_set_hiz_enable(int enable);
int charge_set_hiz_enable_by_direct_charge(int enable);
void charge_set_batfet_disable(int val);
void charge_reset_hiz_state(void);
void charge_request_charge_monitor(void);
bool get_cancel_work_flag(void);
void set_sysfs_water_intruded_flag(int val);
struct charge_device_info *huawei_charge_get_di(void);
int charge_get_charger_online(void);
void wireless_charge_wired_vbus_handler(void);
void emark_detect_complete(void);
unsigned int charge_get_bsoh_vterm(void);
#else
static inline int set_charger_disable_flags(int val, int type)
{
	return 0;
}
static inline int adaptor_cfg_for_wltx_vbus(int vol, int cur)
{
	return 0;
}

static inline void water_detection_entrance(void) { }
static inline int charge_get_vbus(void)
{
	return 0;
}
static inline void charge_set_charger_type(unsigned int type) { }
static inline void charger_source_sink_event(enum charger_event_type event) { }
static inline void send_water_intrused_event(bool flag) { }
static inline bool get_stop_charge_sync_flag(void)
{
	return false;
}
static inline int charge_otg_mode_enable(int enable, unsigned int user)
{
	return 0;
}
static inline void charge_set_adapter_voltage(int val, enum reset_adapter_source_type type,
	unsigned int delay_time) { }
static inline int charge_set_input_current_max(void)
{
	return 0;
}
static inline bool charge_get_hiz_state(void)
{
	return false;
}
static inline int charge_set_hiz_enable(int enable)
{
	return 0;
}
static inline void charge_set_batfet_disable(int val) { }
static inline void charge_reset_hiz_state(void) { }
static inline void charge_request_charge_monitor(void) { }
static inline bool get_cancel_work_flag(void)
{
	return false;
}
static inline void set_sysfs_water_intruded_flag(int val) { }
static inline struct charge_device_info *huawei_charge_get_di(void)
{
	return NULL;
}
static inline int charge_get_charger_online(void)
{
	return 0;
}
static inline void wireless_charge_wired_vbus_handler(void) { }
static inline void emark_detect_complete(void) { }
#endif /* CONFIG_HUAWEI_CHARGER_AP */

#endif /* _HUAWEI_CHARGER */
