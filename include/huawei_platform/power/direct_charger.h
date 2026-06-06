#ifndef __DIRECT_CHARGER_H_
#define __DIRECT_CHARGER_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <huawei_platform/power/huawei_charger.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

/*move from direct_charger.c*/
#include <huawei_platform/log/hw_log.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/wakelock.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
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

#include <huawei_platform/power/direct_charger_error_handle.h>

#define CHARGE_DMDLOG_SIZE      (2048)
#define REVERSE_OCP_CNT          3
#define OTP_CNT                  3
#define ADAPTOR_OTP_CNT          3

#ifndef BIT
#define BIT(x) (1 << x)
#endif
/*scp adaptor register*/
#define ADAPTOR_SCP_DETECT_FAIL -1

#define SCP_ADP_TYPE 0x80
#define SCP_ADP_TYPE_B_MASK (BIT(5)|BIT(4))
#define SCP_ADP_TYPE_B 0x10

#define SCP_ADP_TYPE0 0x7E
#define SCP_ADP_TYPE0_B_MASK (BIT(4))
#define SCP_ADP_TYPE0_B_SC_MASK (BIT(3))
#define SCP_ADP_TYPE0_B_LVC_MASK (BIT(2))

#define SCP_B_ADP_TYPE 0x81
#define SCP_B_DIRECT_ADP 0x10
#define DOUBLE_SIZE 2

/* sensor_id#scene_id#stage */
#define SCP_THERMAL_REASON_SIZE    16

#define SCP_VENDOR_ID_H 0x82
#define SCP_VENDOR_ID_L 0x83
#define SCP_MODULE_ID_H 0x84
#define SCP_MODULE_ID_L 0x85
#define SCP_SERRIAL_NO_H 0x86
#define SCP_SERRIAL_NO_L 0x87
#define SCP_PCHIP_ID 0x88
#define SCP_HWVER 0x89
#define SCP_FWVER_H 0x8a
#define SCP_FWVER_L 0x8b
#define SCP_SPID 0x8c
#define SCP_NEED_OPEN_OUTPUT2 0x31
#define SCP_MAX_POWER 0x90
#define SCP_CNT_POWER 0x91
#define SCP_MIN_VOUT 0x92
#define SCP_MAX_VOUT 0x93
#define SCP_MIN_IOUT 0x94
#define SCP_MAX_IOUT 0x95
#define SCP_MAX_IOUT_A_MASK (BIT(7) | BIT(6))
#define SCP_MAX_IOUT_A_SHIFT 6
#define SCP_MAX_IOUT_B_MASK (BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SCP_VSTEP 0x96
#define SCP_ISTEP 0x97
#define SCP_MAX_VERR 0x98
#define SCP_MAX_IVRR 0x99
#define SCP_MAX_STTIME 0x9a
#define SCP_MAX_RSPTIME 0x9b
#define SCP_SCTRL 0x9e
#define SCP_OUTPUT2_ENABLE_MASK BIT(7)
#define SCP_OUTPUT2_ENABLE BIT(7)
#define SCP_CTRL_BYTE0 0xa0
#define SCP_OUTPUT_MODE_MASK BIT(6)
#define SCP_OUTPUT_MODE_ENABLE BIT(6)
#define SCP_OUTPUT_MODE_DISABLE 0
#define SCP_OUTPUT_MASK BIT(7)
#define SCP_OUTPUT_ENABLE BIT(7)
#define SCP_OUTPUT_DISABLE 0
#define SCP_ADAPTOR_RESET_MASK BIT(5)
#define SCP_ADAPTOR_RESET_ENABLE BIT(5)
#define SCP_ADAPTOR_RESET_DISABLE 0
#define SCP_CTRL_BYTE1 0xa1
#define SCP_DP_DELITCH_MASK (BIT(3) | BIT(4))
#define SCP_DP_DELITCH_5_MS (BIT(3) | BIT(4))
#define SCP_WATCHDOG_BITS_PER_SECOND (2)
#define SCP_WATCHDOG_MASK (BIT(2) | BIT(1) | BIT(0))
#define SCP_STATUS_BYTE0 0xa2
#define SCP_CABLE_STS_MASK BIT(3)
#define SCP_PORT_LEAKAGE_INFO BIT(4)
#define SCP_PORT_LEAKAGE_SHIFT 4
#define SCP_STATUS_BYTE1 0xa3
#define SCP_SSTS 0xa5
#define SCP_SSTS_A_MASK (BIT(3) | BIT(2) | BIT(1))
#define SCP_SSTS_B_MASK (BIT(0))
#define SCP_SSTS_B_SHIFT 0
#define SCP_INSIDE_TMP 0xa6
#define SCP_PORT_TMP 0xa7
#define SCP_READ_VOLT_L 0xa8
#define SCP_READ_VOLT_H 0xa9
#define SCP_READ_IOLT_L 0xaa
#define SCP_READ_IOLT_H 0xab
#define SCP_SREAD_VOUT 0xc8
#define SCP_SREAD_VOUT_OFFSET 3000
#define SCP_SREAD_VOUT_STEP 10
#define SCP_SREAD_IOUT 0xc9
#define SCP_SREAD_IOUT_STEP 50
#define SCP_DAC_VSET_L 0xac
#define SCP_DAC_VSET_H 0xad
#define SCP_DAC_ISET_L 0xae
#define SCP_DAC_ISET_H 0xaf
#define SCP_VSET_BOUNDARY_L 0xb0
#define SCP_VSET_BOUNDARY_H 0xb1
#define SCP_ISET_BOUNDARY_L 0xb2
#define SCP_ISET_BOUNDARY_H 0xb3
#define SCP_MAX_VSET_OFFSET 0xb4
#define SCP_MAX_ISET_OFFSET 0xb5
#define SCP_VSET_L 0xb8
#define SCP_VSET_H 0xb9
#define SCP_ISET_L 0xba
#define SCP_ISET_H 0xbb
#define SCP_VSSET 0xca
#define VSSET_OFFSET 3000
#define VSSET_STEP 10
#define SCP_ISSET 0xcb
#define ISSET_STEP 50
#define SCP_VSET_OFFSET_L 0xbc
#define SCP_VSET_OFFSET_H 0xbd
#define SCP_ISET_OFFSET_L 0xbe
#define SCP_ISET_OFFSET_H 0xbf
#define SCP_STEP_VSET_OFFSET 0xcc
#define SCP_STEP_ISET_OFFSET 0xcd

#define SCP_ADAPTOR_DIGEST_LEN  16
#define SCP_ADAPTOR_DETECT_FAIL 1
#define SCP_ADAPTOR_DETECT_SUCC 0
#define SCP_ADAPTOR_DETECT_OTHER -1
#define SCP_CABLE_DETECT_FAIL 1
#define SCP_CABLE_DETECT_SUCC 0
#define SCP_CABLE_DETECT_ERROR -1

#define ERROR_RESISTANCE -99999
#define MAX_RESISTANCE 10000
#define SCP_CHARGE_TYPE_LVC 3
#define SCP_CHARGE_TYPE_SC 2
#define SCP_CHARGE_TYPE_FCP 4

#define DC_ERR_CNT_MAX            (8)
#define DC_OPEN_RETRY_CNT_MAX      (3)

#define STRTOL_MAX_LEN     10

#define FAIL -1
#define SUCC 0

#define INVALID -1
#define VALID 0
#define WAIT_LS_DISCHARGE 200
#define MAX_TIMES_FOR_SET_ADAPTER_VOL_30 30
#define ADAPTER_VOL_DIFFRENCE_300_MV 300
#define MAX_ADAPTER_VOL_4400_MV 4400
#define MIN_ADAPTER_VOL_STEP_20_MV 20
#define MIN_CURRENT_FOR_RES_DETECT_800_MA 800
#define MAX_VOL_FOR_BATTERY_4360_MV 4360
#define CURRENT_SET_FOR_RES_DETECT_1000_MA 1000

#define VENDOR_ID_RICHTEK 0x01
#define VENDOR_ID_WELTREND 0x02
#define VENDOR_ID_IWATT 0x03
#define VENDOR_ID_0X32 0x32

#define MAX_IOUT_EXP_0 0
#define MAX_IOUT_EXP_1 1
#define MAX_IOUT_EXP_2 2
#define MAX_IOUT_EXP_3 3
#define TEN_EXP_0 1
#define TEN_EXP_1 10
#define TEN_EXP_2 100
#define TEN_EXP_3 1000
#define DROP_POWER_FLAG 1
#define DROP_POWER_FACTOR 8
#define SCP_NO_ERR 0
#define SCP_IS_ERR 1
#define ONE_BYTE_LEN 8
#define ONE_BYTE_MASK 0xff
#define ONE_BIT_EQUAL_TWO_SECONDS 2
#define WAIT_FOR_ADAPTOR_RESET 50
#define OUTPUT_MODE_ENABLE 1
#define OUTPUT_MODE_DISABLE 0
#define ADAPTOR_RESET 1
#define GPIO_ENABLE 1
#define GPIO_DISABLE 0
#define IN_SCP_CHARGING_STAGE 0
#define NOT_IN_SCP_CHARGING_STAGE -1
#define ENABLE 1
#define DISABLE 0


#define SCP_ADAPTOR_DETECT 1
#define SCP_ADAPTOR_NOT_DETECT 0

#define VBUS_ON_THRESHOLD 3000
#define VBAT_VBUS_DIFFERENCE 150
#define KICK_WATCHDOG_TIME 1000
#define WATCHDOG_TIMEOUT 2000
#define BATTERY_CAPACITY_HIGH_TH 95

#define DIRECT_CHARGER_SET_DISABLE_FLAGS        1
#define DIRECT_CHARGER_CLEAR_DISABLE_FLAGS      0

#define DIRECT_CHARGER_LS_RECOVERY_DELAY      500 /*ms*/
#define DIRECT_CHARGER_COUL_CURRENT_UNIT_DEVIDE	1000 /*1000 ua equal 1ma*/

#define SCCAL_NV_CMDLINE_MAX_LEN                30
#define C_OFFSET_MAX_LEN                        20
#define C_OFFSET_A_MAX                          1200000
#define C_OFFSET_A_MIN                          800000

#define DECIMAL_BASE                            10

#define ADP_B_TYPE1                             3
#define ADP_B_TYPE1_65W                         5

/*
 * define working mode with direct charge
 * LVC is simplified identifier with low-voltage-charging
 * SC is simplified identifier with switch-capacitor
 */
enum direct_charge_mode {
	UNDEFINED_MODE = 0x0,
	LVC_MODE = 0x1,
	SC_MODE = 0x2,
};

enum direct_charge_mode_set {
	AND_SET = 0x0,
	OR_SET = 0x1,
};

enum adapter_vendor{
	RICHTEK_ADAPTER,
	IWATT_ADAPTER,
	WELTREND_ADAPTER,
	ID0X32_ADAPTER,
};

/*
 * define error code with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_error_code {
	DC_SUCC,
	DC_ERROR_CHARGE_DISABLED,
	DC_ERROR_ADAPTOR_DETECT,
	DC_ERROR_BAT_TEMP,
	DC_ERROR_BAT_VOL,
	DC_ERROR_SWITCH,
	DC_ERROR_INIT,
	DC_ERROR_USB_PORT_LEAKAGE_CURRENT,
	DC_ERROR_ADAPTOR_VOLTAGE_ACCURACY,
	DC_ERROR_OPEN_CHARGE_PATH,
	DC_ERROR_FULL_REISISTANCE,
	DC_ERROR_ANTI_FAKE_ADAPTOR,
};

/*
 * define sysfs type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum direct_charge_sysfs_type {
	DC_SYSFS_BEGIN = 0,

	DC_SYSFS_ENABLE_CHARGER = DC_SYSFS_BEGIN,
	DC_SYSFS_IIN_THERMAL,
	DC_SYSFS_ADAPTOR_DETECT,
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
	DC_SYSFS_DIRECT_CHARGE_TYPE,
	DC_SYSFS_THERMAL_REASON,

	DC_SYSFS_END,
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

enum scp_retry_operate_type {
	SCP_RETRY_OPERATE_DEFAUTL,
	SCP_RETRY_OPERATE_RESET_ADAPTER,
	SCP_RETRY_OPERATE_RESET_CHIP,
	SCP_RETRY_OPERATE_UNVALID,
};

enum scp_stage_type {
	SCP_STAGE_DEFAULT,
	SCP_STAGE_SUPPORT_DETECT,
	SCP_STAGE_ADAPTER_DETECT,
	SCP_STAGE_SWITCH_DETECT,
	SCP_STAGE_CHARGE_INIT,
	SCP_STAGE_SECURITY_CHECK,
	SCP_STAGE_SUCCESS,
	SCP_STAGE_CHARGING,
	SCP_STAGE_CHARGE_DONE,
};

enum dc_status_notify_list {
	LVC_STATUS_CHARGING,
	SC_STATUS_CHARGING,
	DC_STATUS_STOP_CHARGE,
};

enum disable_direct_charger_type {
	DIRECT_CHARGER_SYS_NODE = 0,
	DIRECT_CHARGER_FATAL_ISC_TYPE,
	DIRECT_CHARGER_WIRELESS_TX,
	DIRECT_CHARGER_BATT_CERTIFICATION_TYPE,
	__MAX_DISABLE_DIRECT_CHAGER,
};

static const char *const scp_check_stage[] = {
	[0] = "SCP_STAGE_DEFAULT",
	[1] = "SCP_STAGE_SUPPORT_DETECT",
	[2] = "SCP_STAGE_ADAPTER_DETECT",
	[3] = "SCP_STAGE_SWITCH_DETECT",
	[4] = "SCP_STAGE_CHARGE_INIT",
	[5] = "SCP_STAGE_SECURITY_CHECK",
	[6] = "SCP_STAGE_SUCCESS",
	[7] = "SCP_STAGE_CHARGING",
	[8] = "SCP_STAGE_CHARGE_DONE",
};

struct scp_init_data {
	int scp_mode_enable;
	int vset_boundary;
	int iset_boundary;
	int init_adaptor_voltage;
	int watchdog_timer;
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
};
struct batinfo_ops {
	int (*init)(void);
	int (*exit)(void);
	int (*get_bat_btb_voltage)(void);
	int (*get_bat_package_voltage)(void);
	int (*get_vbus_voltage)(int* vol);
	int (*get_bat_current)(int* cur);
	int (*get_ls_ibus)(int *ibus);
	int (*get_ls_temp)(int *temp);
};

struct adaptor_info {
	int b_adp_type;
	int vendor_id_h;
	int vendor_id_l;
	int module_id_h;
	int module_id_l;
	int serrial_no_h;
	int serrial_no_l;
	int pchip_id;
	int hwver;
	int fwver_h;
	int fwver_l;
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
	struct loadswitch_ops* ls_ops;
	struct batinfo_ops* bi_ops;
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
	struct direct_charge_resist_para resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para std_resist_para[DC_RESIST_LEVEL];
	struct direct_charge_resist_para ctc_resist_para[DC_RESIST_LEVEL];
	struct adaptor_info adp_info;
	struct nty_data *fault_data;
	int stage_need_to_jump[2*DC_VOLT_LEVEL];
	int error_cnt;
	enum direct_charge_mode scp_mode;
	unsigned int use_5a;
	unsigned int use_8a;
	int sysfs_enable_charger;
	int sysfs_disable_charger[__MAX_DISABLE_DIRECT_CHAGER];
	int sysfs_iin_thermal;
	int threshold_caculation_interval;
	int charge_control_interval;
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
	int max_adaptor_vset;
	unsigned int max_tadapt;
	unsigned int max_tls;
	int adaptor_iset;
	int max_adaptor_iset;
	int init_delt_vset;
	int init_adapter_vset;
	int delta_err;
	int vstep;
	int scp_stop_charging_flag_info;
	int stop_charging_flag_error;
	int max_dc_bat_vol;
	int min_dc_bat_vol;
	int super_ico_current;
	int is_show_ico_first;
	int bst_ctrl;
	int scp_power_en;
	int compensate_r;
	int compensate_v;
	int cc_protect;
	int ls_id;
	const char *ls_name;
	int vol_err_th;
	int full_path_res_max;
	int standard_cable_full_path_res_max;
	int ctc_cable_full_path_res_max;
	int full_path_res_threshold;
	int adaptor_leakage_current_th;
	int adaptor_detect_by_voltage;
	int direct_charge_succ_flag;
	int first_cc_stage_timer_in_min;
	int max_adaptor_cur;
	int scp_cable_detect_enable;
	unsigned long first_cc_stage_timeout;
	int cc_cable_detect_enable;
	int cc_cable_detect_ok;
	int max_current_for_none_standard_cable;
	int max_current_for_ctc_cable;
	enum direct_charge_fault_type charge_fault;
	int adaptor_vendor_id;
	int scp_work_on_charger;
	int anti_fake_adaptor;
	int dc_err_report_flag;
	int dc_open_retry_cnt;
	int reverse_ocp_cnt;
	int otp_cnt;
	int adaptor_otp_cnt;
	int dc_volt_ratio;
	enum scp_stage_type scp_stage;
	int cutoff_normal_flag;
	int quick_charge_flag;
	int super_charge_flag;
	int scp_adaptor_detect_flag;
	char dsm_buff[CHARGE_DMDLOG_SIZE];
	int scp_stop_charging_complete_flag;
	struct wake_lock charging_lock;
	int adaptor_test_result_type;
	char thermal_reason[SCP_THERMAL_REASON_SIZE];
	int adaptor_antifake_key_index;
	int adaptor_antifake_check_enable;
	int dc_antifake_result;
	struct completion dc_af_completion;
	int sc_conv_ocp_count;
	int iin_thermal_default;
	int can_stop_kick_wdt;
	int last_basp_level;
	int gain_curr;
	int reset_adap_volt_enabled;
	u32 open_path_vol_select;
};

struct smart_charge_ops {
	int (*is_support_scp)(void);
	int (*scp_init)(struct scp_init_data*);
	int (*scp_exit)(struct direct_charge_device*);
	int (*scp_adaptor_detect)(void);
	int (*scp_set_adaptor_voltage)(int);
	int (*scp_get_adaptor_voltage)(void);
	int (*scp_set_adaptor_current)(int);
	int (*scp_get_adaptor_current)(void);
	int (*scp_get_adaptor_current_set)(void);
	int (*scp_get_adaptor_max_current)(void);
	int (*scp_adaptor_output_enable)(int);
	int (*scp_stop_charge_config)(void);
	int (*is_scp_charger_type)(void);
	int (*scp_get_adaptor_status)(void);
	int (*scp_get_chip_status)(void);
	int (*scp_get_adaptor_info)(void*);
	int (*scp_cable_detect)(void);
	int (*scp_get_adaptor_temp)(int*);
	int (*scp_get_adapter_vendor_id)(void);
	int (*scp_get_usb_port_leakage_current_info)(void);
	void (*scp_set_direct_charge_mode)(int);
	int (*scp_get_adaptor_type)(void);
};

int direct_charge_pps_ops_register(struct direct_charge_pps_ops *ops);
int direct_charge_cd_ops_register(struct direct_charge_cd_ops *ops);
void direct_charge_get_g_pps_ops(struct direct_charge_pps_ops **ops);
void direct_charge_get_g_cd_ops(struct direct_charge_cd_ops **ops);

int direct_charge_notifier_chain_register(struct notifier_block *nb);
int direct_charge_notifier_chain_unregister(struct notifier_block *nb);

void direct_charge_set_di(struct direct_charge_device *di);

enum scp_stage_type direct_charge_get_stage_status(void);
void direct_charge_set_stage_status(enum scp_stage_type stage_type);
void direct_charge_set_stage_status_default(void);
void direct_charge_set_adapter_default_param(void);

void direct_charge_set_local_mode(int set, int dc_mode);
int direct_charge_get_local_mode(void);
int direct_charge_get_adapter_type(void);

int direct_charge_ops_is_valid(struct direct_charge_device *di);
int direct_charge_is_failed(void);

void direct_charge_send_normal_charging_uevent(void);
void direct_charge_send_quick_charging_uevent(void);
void direct_charge_send_super_charging_uevent(void);
int direct_charge_get_quick_charging_flag(void);
int direct_charge_get_super_charging_flag(void);

int direct_charge_pre_check_battery_temp(void);
int direct_charge_pre_check_battery_voltage(void);
int direct_charge_init_adapter_and_device(void);
int direct_charge_security_check(void);

void direct_charge_set_stop_charging_flag(int value);

int direct_charge_parse_dts(struct device_node *np,
	struct direct_charge_device *di);
int direct_charge_fault_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data);
void direct_charge_control_work(struct work_struct *work);
void direct_charge_calc_thld_work(struct work_struct *work);
void direct_charge_kick_wtd_work(struct work_struct *work);
enum hrtimer_restart direct_charge_calc_thld_timer_func(struct hrtimer *timer);
enum hrtimer_restart direct_charge_control_timer_func(struct hrtimer *timer);
enum hrtimer_restart direct_charge_kick_wtd_timer_func(struct hrtimer *timer);

void battery_aging_safe_policy(struct direct_charge_device *di,
				bool force_update);
void scp_start_charging(void);
void scp_stop_charging(void);
int cutoff_normal_charge(void);
void scp_cable_detect(void);
int is_support_scp(void);

int lvc_ops_register(struct loadswitch_ops *ops);
int lvc_batinfo_ops_register(struct batinfo_ops *ops);
int lvc_get_di(struct direct_charge_device **di);
void lvc_get_fault_notifier(struct atomic_notifier_head **notifier);
void lvc_mode_check(void);

int sc_ops_register(struct loadswitch_ops *ops);
int sc_batinfo_ops_register(struct batinfo_ops *ops);
int sc_get_di(struct direct_charge_device **di);
void sc_get_fault_notifier(struct atomic_notifier_head **notifier);
void sc_mode_check(void);

int scp_adaptor_type_detect(int *mode);
void direct_charge_check(void);
void direct_charge_update_cutoff_flag(void);
void direct_charge_stop_charging(void);
int is_scp_stop_charging_complete(void);
int scp_adaptor_detect(void);
int scp_adaptor_set_output_enable(int enable);
int direct_charge_get_cutoff_normal_flag(void);
int is_in_scp_charging_stage(void);
#ifdef CONFIG_SCHARGER_V300
#define HI6523_CV_CUT 150
extern bool is_hi6523_cv_limit(void);
#endif
extern int set_direct_charger_disable_flags(int, int);
extern struct blocking_notifier_head direct_charger_control_head;
bool direct_charge_check_sc_mode(void);
int direct_charge_gen_nl_init(struct platform_device *pdev);
int do_adpator_antifake_check(void);
int direct_charge_disable_usbpd(bool disable);
void direct_charge_force_disable_dc_path(void);
int direct_charge_pre_check(void);
enum direct_charge_mode direct_charge_get_adaptor_mode(void);

#ifdef CONFIG_WIRELESS_CHARGER
extern bool wltx_need_disable_wired_dc(void);
#endif

void direct_charge_send_ico_uevent(void);

#ifdef CONFIG_DIRECT_CHARGER
bool direct_charge_get_abnormal_adp_flag(void);
void direct_charge_set_abnormal_adp_flag(bool flag);
#else
static inline bool direct_charge_get_abnormal_adp_flag(void)
{
	return false;
}

static inline void direct_charge_set_abnormal_adp_flag(bool flag)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif
