/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: coul core headfile
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _COUL_CORE_H_
#define _COUL_CORE_H_

#include <asm/bug.h>
#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/power_supply.h>
#include <linux/reboot.h>
#include <linux/rtc.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/syscalls.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#ifdef CONFIG_HUAWEI_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger/direct_charger.h>
#endif
#ifdef CONFIG_HUAWEI_CHARGER_AP
#include <huawei_platform/power/huawei_charger.h>
#endif
#include <linux/hisi/hisi_adc.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/battery_data.h>
#include <linux/power/hisi/bci_battery.h>

#define coul_core_debug(fmt, args...) pr_debug("[coul_core]" fmt, ## args)
#define coul_core_info(fmt, args...) pr_info("[coul_core]" fmt, ## args)
#define coul_core_warn(fmt, args...) pr_warn("[coul_core]" fmt, ## args)
#define coul_core_err(fmt, args...) pr_err("[coul_core]" fmt, ## args)

enum basp_fcc_learn_state {
	LS_UNKNOWN,
	LS_INIT,
	LS_GOOD,
	LS_BAD,
};

enum basp_fcc_learn_event {
	EVT_START,
	EVT_PER_CHECK, /* periodic check */
	EVT_DONE,
};

enum battery_temp_user {
	USER_COUL,
	USER_CHARGER,
	USER_CNT,
};

enum ocv_level {
	OCV_LEVEL_0 = 0,
	OCV_LEVEL_1,
	OCV_LEVEL_2,
	OCV_LEVEL_3,
	OCV_LEVEL_4,
	OCV_LEVEL_5,
	OCV_LEVEL_6,
	OCV_LEVEL_7,
	OCV_LEVEL_MAX,
};

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define RBATT_ADJ       1

#define di_lock() mutex_lock(&di->soc_mutex)
#define di_unlock() mutex_unlock(&di->soc_mutex)

#define SUCCESS 0
#define ERROR   1
#define LOW_TEMP_OPT_OPEN       1
#define LOW_TEMP_OPT_CLOSE      0
#define UUC_MIN_CURRENT_MA      400
#define NOT_UPDATE_FCC  1
#define IS_UPDATE_FCC   0
#define DEFAULT_TEMP            25 /* set for SFT/UDP */
#define TEMP_OCV_ALLOW_CLEAR    5
#define TEMP_THRESHOLD_CALI     2
#define ABNORMAL_DELTA_SOC      10
#define CHARGING_STATE_CHARGE_START             1
#define CHARGING_STATE_CHARGE_DONE              2
#define CHARGING_STATE_CHARGE_STOP              3
#define CHARGING_STATE_CHARGE_RECHARGE          4
#define CHARGING_STATE_CHARGE_NOT_CHARGE        5
#define CHARGING_STATE_CHARGE_UNKNOW            0
#define IAVG_SAMPLES            10
#define CHARGING_IAVG_MA        250
#define CHANGE_BATTERY_MOVE             1 /* bettery move */
#define CHANGE_BATTERY_NEED_RESTORE     2 /* need restore chargecycle after battery plug in */
#define RESTORE_CYCLE_MAX               (40 * 100) /* need not restore chargecycle, because overtime */
#define DELTA_FCC_INVALID_RADIO         5 /* maximum error standard for two times learning */
#define NEW_BATT_DIFF_RADIO             97 /* the standard near the new battery */
#define OLD_BATT_DIFF_RADIO             3 /* maximum error standard for original battery */
#define BASP_FCC_LERAN_TEMP_MIN         200 /* tenth degree */
#define BASP_FCC_LERAN_TEMP_MAX         450 /* tenth degree */
#define RARIO_MIN        100
#define INVALID_SAVE_OCV_LEVEL          (OCV_LEVEL_MAX)
#define LOW_PRECISION_OCV_LEVEL         (OCV_LEVEL_4)
#define DELTA_SAFE_FCC_PERCENT          5
#define DELTA_MAX_DECR_FCC_PERCENT      5
#define DELTA_MAX_INCR_FCC_PERCENT      10
#define CALCULATE_SOC_MS                (20 * 1000)
#define READ_TEMPERATURE_MS             (5 * 1000)
#define TEMPERATURE_INIT_STATUS         1
#define TEMPERATURE_UPDATE_STATUS       0
#define CALIBRATE_INTERVAL              (5 * 60 * 1000)    /* 5 min */
#define CHARGED_OCV_UPDATE_INTERVAL_S   (10 * 60)
#define DELAY_REPORT_DMD_TIME           (100 * 1000)    /* 100s */
#define BATTERY_CC_LOW_LEV              3
#define BATTERY_CC_WARNING_LEV          10
#define BATTERY_SWITCH_ON_VOLTAGE       3250
#define LOW_INT_STATE_RUNNING   1
#define LOW_INT_STATE_SLEEP     0
#define BATTERY_VOL_2_PERCENT   3350
#define LOW_INT_TEMP_THRED      (-50) /* -5 */
#define MAX_TEMPS               10
#define MAX_RECORDS_CNT         5
#define SR_DEVICE_WAKEUP        1
#define SR_DEVICE_SLEEP         2
#define MIN_BEGIN_PERCENT_FOR_LEARNING  60
#define MIN_BEGIN_PERCENT_FOR_SAFE      20
#define MAX_DELTA_RC_PC                 1
#define FCC_UPDATE_TEMP_MIN             100 /* tenth degree */
#define FCC_UPDATE_TEMP_MAX             450 /* tenth degree */
#define FCC_UPDATE_CHARGING_CURR_MIN_MA 200
#define MIN_BEGIN_PERCENT_FOR_QMAX      20 /* high pre qmax */
#define CURRENT_LIMIT           (20 * 1000)
#define CHARGING_CURRENT_OFFSET (-10 * 1000)
#define BATTERY_PLUG_OUT        1
#define BATTERY_PLUG_IN         0
#define BATTERY_CHECK_TIME_MS   (1 * 1000)
#define NV_RESERVE_LEN          (2 * MAX_TEMPS + 3) /* removed asw nv len */
#define NV_SAVE_SUCCESS         1
#define NV_SAVE_FAIL            0
#define NV_READ_SUCCESS         1
#define NV_READ_FAIL            0
#define UVOLT_PER_MVOLT         1000
#define UA_PER_MA               1000
#define SEC_PER_HOUR            3600
#define TENTH                   10
#define PERCENT                 100
#define PERMILLAGE              1000
#define FCC_UPDATE_MAX_OCV_INTERVAL     12
#define CURRENT_FULL_TERM_SOC           95
#define SUPPLY_PARA_LEVEL               5
#define ZERO_V_MIN              2800
#define BASP_FATAL_VDEC_TH      100
#define ZERO_V_COPM_MV          10
#define ZERO_V_ADJ_START_T      5
#define ZERO_V_ADJ_END_T        (-10)
#define LOW_INT_VOL_COUNT       3
#define TEMP_UUC_STEP           5
#define TIMESTAMP_STR_SIZE      32
#define DSM_BUFF_SIZE_MAX       1024
#define SOC_JUMP_MAX            5
#define LOW_TEMP_UPPER          15
#define DEFAULT_ZERO_VOL        3200
#define UUC_PC_STEP_ADD         20
#define UUC_PC_STEP_SUB         20
#define UUC_PC_MAX_DIFF         50
#define LOW_TEMP_UUC_PC_MAX_DIFF        700
#define LOW_TEMP_UUC_PC_STEP_ADD        40
#define LOW_TEMP_UUC_PC_STEP_SUB        5
#define NORMAL_SOC_LIMIT        30
#define HALF    2
#define QUARTER 4
#define SOC_FULL_TENTH  1000
#define FCC_UPPER_LIMIT_PERCENT 106
#define LIMIT_FCC_CAL_RATIO     101
#define CHARGING_CUR_MA         (-50)
#define FIFO_AVG_CUR             50
#define LOW_TEMP_FIVE_DEG               5
#define LOW_TEMP_MINUS_FIVE_TENTH       (-50)
#define LOW_TEMP_FIVE_TENTH             50
#define LOW_TEMP_MINUS_TWENTY_DEG       (-20)
#define THREE_PERCENT_SOC_VOL           3500
#define CUR_FULL_CHG_BEGIN_SOC_UPPER    90
#define INVALID_FCC_UAH         100
#define DEFAULT_SOC             50
#define CALI_ADC_DELAY_MS       2500
#define OCV_3200_UV     3200000
#define OCV_3670_UV     3670000
#define OCV_3690_UV     3690000
#define OCV_3730_UV     3730000
#define OCV_3800_UV     3800000
#define OCV_3900_UV     3900000
#define CHG_DONE_SLEEP_CUR_UPPER        20
#define SEC_PER_HOUR_DIV_200    18
#define PERMILLAGE_DIV_200      5
#define IAVG_MA_200     200
#define IAVG_MA_100     100
#define IAVG_MA_50      50
#define FOUR_HOUR       4
#define EIGHT_HOUR      8
#define SOC_TO_CAL_LIMIT_FCC    950
#define MODIFY_NEW_BATT_CYCLE   40
#define POWER_ON_CNT_MAX        3
#define SOC_VARY_MAX    2
#define SOC_EST_NUM     3
#define SLEEP_1500_MS   1500
#define SOC_FULL        100

enum soc_percent {
	SOC_ZERO = 0,
	SOC_ONE,
	SOC_TWO,
	SOC_THREE,
};

enum coul_status {
	STATUS_START = 0,
	STATUS_RUNNING,
	STATUS_WAKEUP,
};

enum coul_fault_type {
	COUL_FAULT_NON = 0,
	COUL_FAULT_LOW_VOL,
	COUL_FAULT_CL_INT,
	COUL_FAULT_CL_IN,
	COUL_FAULT_CL_OUT,
	COUL_FAULT_I_OUT,
	COUL_FAULT_I_IN,
	COUL_FAULT_TOTAL,
};

/* max 104 bytes */
struct ss_coul_nv_info {
	int charge_cycles;
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
	int limit_fcc;
	short temp[MAX_TEMPS];
	short real_fcc[MAX_TEMPS];
	/* below get data from fastboot,not need to save */
	short calc_ocv_reg_v;
	short calc_ocv_reg_c;
	short hkadc_batt_id_voltage;
	short real_fcc_record[MAX_RECORDS_CNT];
	short latest_record_index;
	short basp_delay_config;
	int fcc_check_sum_ext;
	short change_battery_learn_flag;
	short qmax; /* 92byte */
	int report_full_fcc_real;
};

struct vcdata {
	int avg_v; /* mv */
	int avg_c; /* ma */
	int min_c; /* ma */
	int max_c; /* ma */
};

struct coul_device_ops {
	int (*calculate_cc_uah)(void);
	void (*save_cc_uah)(int cc_uah);
	int (*convert_ocv_regval2ua)(short reg_val);
	int (*convert_ocv_regval2uv)(short reg_val);
	int (*is_battery_moved)(void);
	void (*set_battery_moved_magic_num)(int);
	void (*get_fifo_avg_data)(struct vcdata *vc);
	int (*get_fifo_depth)(void);
	int (*get_eco_fifo_depth)(void);
	int (*get_delta_rc_ignore_flag)(void);
	int (*get_nv_read_flag)(void);
	void (*set_nv_save_flag)(int nv_flag);
	int (*set_hltherm_flag)(int temp_protect_flag);
	int (*get_hltherm_flag)(void);
	int (*get_use_saved_ocv_flag)(void);
	int (*get_fcc_invalid_up_flag)(void);
	void (*save_ocv)(int ocv, int invalid_fcc_up_flag);
	short (*get_ocv)(void);
	void (*clear_ocv)(void);
	void (*save_ocv_temp)(short ocv_temp);
	short (*get_ocv_temp)(void);
	void (*clear_ocv_temp)(void);
	void (*set_low_low_int_val)(int vol_mv);
	int (*get_abs_cc)(void);
	unsigned int (*get_coul_time)(void);
	void (*clear_coul_time)(void);
	void (*clear_cc_register)(void);
	void (*cali_adc)(void);
	int (*get_battery_voltage_uv)(void);
	int (*get_battery_current_ua)(void);
	int (*get_battery_vol_uv_from_fifo)(short fifo_order);
	int (*get_battery_cur_ua_from_fifo)(short fifo_order);
	int (*get_eco_vol_uv_from_fifo)(short fifo_order);
	int (*get_eco_cur_ua_from_fifo)(short fifo_order);
	int (*get_eco_temp_from_fifo)(short fifo_order);
	short (*get_offset_current_mod)(void);
	short (*get_offset_vol_mod)(void);
	void (*set_offset_vol_mod)(void);
	int (*get_ate_a)(void);
	int (*get_ate_b)(void);
	void (*irq_enable)(void);
	void (*irq_disable)(void);
	void (*show_key_reg)(void);
	void (*enter_eco)(void);
	void (*exit_eco)(void);
	int (*calculate_eco_leak_uah)(void);
	void (*save_last_soc)(short soc);
	void (*get_last_soc)(short *soc);
	void (*clear_last_soc_flag)(void);
	void (*get_last_soc_flag)(bool *valid);
	void (*cali_auto_off)(void);
	void (*save_ocv_level)(u8 level);
	void (*get_ocv_level)(u8 *level);
	void (*set_i_in_event_gate)(int ma);
	void (*set_i_out_event_gate)(int ma);
	int (*get_chip_temp)(void);
	int (*get_bat_temp)(void);
	int (*convert_regval2uv)(unsigned int reg_val);
	int (*convert_regval2ua)(unsigned int reg_val);
	int (*convert_regval2temp)(unsigned int reg_val);
	unsigned int (*convert_uv2regval)(int uv_val);
	int (*convert_regval2uah)(u64 reg_val);
	void (*set_eco_sample_flag)(u8 set_val);
	void (*get_eco_sample_flag)(u8 *get_val);
	void (*clr_eco_data)(u8 set_val);
	int (*get_coul_calibration_status)(void);
	int (*get_drained_battery_flag)(void);
	void (*clear_drained_battery_flag)(void);
	void (*set_bootocv_sample)(u8 set_val);
};

struct polar_supply_data {
	int soc_min;
	int soc_max;
	int supply_curr1;
	int supply_curr2;
	int temp_back;
};

struct smartstar_coul_device {
	int batt_exist;
	int prev_pc_unusable;
	int batt_ocv; /* ocv in uv */
	int batt_ocv_temp;
	int batt_ocv_valid_to_refresh_fcc;
	int batt_changed_flag;
	int batt_reset_flag;
	int soc_limit_flag;
	int batt_temp; /* temperature in degree * 10 */
	int qmax; /* uAh */
	int batt_fcc;
	int batt_limit_fcc;
	int batt_limit_fcc_begin;
	int batt_report_full_fcc_real;
	int batt_report_full_fcc_cal;
	int batt_rm;
	int batt_ruc;
	int batt_uuc;
	int batt_delta_rc;
	int batt_pre_delta_rc;
	int rbatt;
	int rbatt_ratio;
	int r_pcb;
	int soc_work_interval;
	int charging_begin_soc;
	int charging_state;
	int charging_stop_soc; /* the unit is uah */
	int charging_stop_time;
	unsigned int dischg_ocv_enable;
	int dischg_ocv_soc;
	int batt_soc;
	int last_powerdown_soc;
	int batt_soc_real;
	int batt_soc_est;
	unsigned int batt_id_vol;
	unsigned int batt_chargecycles; /* chargecycle in percent */
	int last_cali_temp; /* temperature in degree * 10 */
	int cc_end_value;
	unsigned int v_cutoff;
	unsigned int v_cutoff_sleep;
	int v_cutoff_low_temp;
	unsigned int v_low_int_value;
	unsigned int get_cc_end_time;
	unsigned int suspend_time; /* time when suspend */
	int sr_suspend_time;  /* sr time when suspend */
	int sr_resume_time;   /* sr time when resume */
	int charging_begin_cc;  /* the unit is uah */
	int charging_begin_time;
	int suspend_cc; /* cc when suspend */
	int resume_cc; /* cc when suspend */
	unsigned int last_time;
	int last_cc;
	int last_iavg_ma;
	int last_fifo_iavg_ma;
	int fcc_real_mah;
	int ntc_compensation_is;
	int work_init_flag;
	struct ntc_temp_compensation_para_data ntc_temp_compensation_para[COMPENSATION_PARA_LEVEL];
	struct mutex soc_mutex;
	struct coul_battery_data *batt_data;
	struct single_row_lut *adjusted_fcc_temp_lut;
	struct single_row_lut adjusted_fcc_temp_lut_tbl1;
	struct single_row_lut adjusted_fcc_temp_lut_tbl2;
	struct delayed_work calculate_soc_delayed_work;
	struct delayed_work battery_check_delayed_work;
	struct delayed_work read_temperature_delayed_work;
	struct work_struct fault_work;
	struct notifier_block fault_nb;
	struct notifier_block reboot_nb;
	struct notifier_block pm_notify;
	struct coulometer_ops *ops;
	struct ss_coul_nv_info nv_info;
	struct coul_device_ops *coul_dev_ops;
	enum coul_fault_type coul_fault;
	struct device *dev;
	int is_nv_read;
	int is_nv_need_save;
	int last_soc_enable;
	int startup_delta_soc;
	int soc_at_term;
	unsigned int enable_current_full;
	unsigned int basp_level;
	unsigned int nondc_volt_dec;
	int soc_unlimited;
	int soc_monitor_flag;
	int soc_monitor_limit;
	unsigned char last_ocv_level;
	unsigned int low_vol_filter_cnt;
	unsigned int polar_ocv_enable;
	unsigned int supply_enable;
	struct polar_supply_data supply_info[SUPPLY_PARA_LEVEL];
	int chg_done_max_avg_cur_flag; /* acr max currrent flag */
	int high_pre_qmax;
	int qmax_start_pc;
	int qmax_cc;
	int qmax_refresh_flag;
	int qmax_end_pc;
	unsigned int dbg_ocv_cng_0; /* ocv change count by wake up */
	unsigned int dbg_ocv_cng_1; /* ocv change count by full charged */
	unsigned int dbg_valid_vol; /* vaild voltage from FIFO vol registers */
	unsigned int dbg_invalid_vol; /* invaild voltage from FIFO vol registers */
	unsigned int dbg_ocv_fc_failed; /* full charged can't update OCV */
	s64 iscd_full_update_cc;
	int low_temp_opt_flag;
	int multi_ocv_open_flag;
	int fcc_update_limit_flag;
	int check_ocv_data_enable;
	int uuc_zero_vol;
	u32 is_board_type;
	u32 battery_is_removable;
	u32 dec_state;
	u32 adc_batt_id;
	u32 adc_batt_temp;
	int dts_c_offset_a;
	int batt_backup_nv_flag;
	int need_restore_cycle_flag;
	u32 batt_under_voltage_flag;
	s32 batt_temp_too_hot;
	s32 batt_temp_too_cold;
	u32 wakelock_low_batt_soc;
};

int coul_notify_fault(unsigned long val, void *v);
struct smartstar_coul_device *get_coul_dev(void);
struct platform_device *get_coul_pdev(void);
int get_sr_cur_state(void);
void coul_charging_begin(struct smartstar_coul_device *di);
void coul_charging_stop(struct smartstar_coul_device *di);
void coul_charging_done(struct smartstar_coul_device *di);
void get_ocv_by_vol(struct smartstar_coul_device *di);
int coul_resume(struct platform_device *pdev);
int coul_suspend(struct platform_device *pdev, pm_message_t state);

#endif

