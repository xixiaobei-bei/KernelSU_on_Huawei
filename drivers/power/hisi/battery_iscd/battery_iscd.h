/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: driver for iscd
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

#ifndef _HUAWEI_ISCD_H_
#define _HUAWEI_ISCD_H_

#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <securec.h>
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger/direct_charger.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/coul/coul_event.h>

#define iscd_core_debug(fmt, args...) pr_debug("[ISCD]" fmt, ## args)
#define iscd_core_info(fmt, args...) pr_info("[ISCD]" fmt, ## args)
#define iscd_core_warn(fmt, args...) pr_warn("[ISCD]" fmt, ## args)
#define iscd_core_err(fmt, args...) pr_err("[ISCD]" fmt, ## args)

#define HALF            2
#define QUARTER         4
#define SUCCESS         0
#define ERROR           1
#define ENABLED         1
#define DISABLED        0
#define ISCD_VALID      1
#define ISCD_INVALID    0
#define INVALID_ISC     999999

#define ISCD_SAMPLE_RETYR_CNT           3
#define ISCD_SMAPLE_TIME_MIN            (3 * 3600) /* 3h */
#define ISCD_SMAPLE_LEN_MAX             30
#define ISCD_SAMPLE_INTERVAL_MAX        (12 * 3600)  /* 12h */
#define ISCD_SAMPLE_INTERVAL_DELTA      20 /* s */
#define ISCD_DEFAULT_OCV_MIN            4000000  /* 4V */
#define ISCD_DEFAULT_TBATT_MIN          100
#define ISCD_DEFAULT_TBATT_MAX          500
#define ISCD_DEFAULT_TBATT_DIFF         100
#define ISCD_DEFAULT_SAMPLE_INTERVAL    600 /* 10min */
#define ISCD_DEFAULT_SAMPLE_DELAY       10 /* s */
#define ISCD_DEFAULT_CALC_INTERVAL_MIN  3600 /* 1h */
#define ISCD_CALC_INTERVAL_900S         900 /* 15min */
#define ISCD_INVALID_SAMPLE_CNT_FROM    1
#define ISCD_INVALID_SAMPLE_CNT_TO      2
#define ISCD_STANDBY_SAMPLE_CNT         (-1)  /* for standby mode */
#define ISCD_CHARGE_CYCLE_MIN           10
#define ISCD_CHRG_DELAY_CYCLES          0
#define ISCD_DELAY_CYCLES_ENABLE        0
#define ISCD_DMD_REPORT_TIME_FOR_ABN_RECHARGE 2

#define ISCD_WARNING_LEVEL_THREHOLD     10000 /* uA */
#define ISCD_ERROR_LEVEL_THREHOLD       30000 /* uA */
#define ISCD_CRITICAL_LEVEL_THREHOLD    100000 /* uA */
#define ISCD_FATAL_LEVEL_THREHOLD       200000 /* uA */
#define ISCD_RECHARGE_CC                1000  /* uAh */
#define ISCD_LARGE_ISC_THREHOLD         50000 /* uA */
#define ISCD_LARGE_ISC_VALID_SIZE       10
#define ISCD_LARGE_ISC_VALID_PERCENT    50  /* 50% */
#define ISCD_SMALL_ISC_VALID_SIZE_1     40
#define ISCD_SMALL_ISC_VALID_SIZE_2     30
#define ISCD_SMALL_ISC_VALID_PERCENT    50  /* 50% */

#define ISCD_OCV_FIFO_VALID_CNT         3
#define ISCD_OCV_UV_VAR_THREHOLD        3000000  /* ocv variance threhold */
#define ISCD_OCV_UV_MIN                 2500000 /* 2.5V */
#define ISCD_OCV_UV_MAX                 4500000 /* 4.5V */
#define ISCD_OCV_DELTA_MAX              10000 /* 10mV */
#define ISCD_CC_DELTA_MAX               10000 /* 10mAh */

#define ISCD_ISC_MAX_SIZE               512

#define ISCD_MAX_LEVEL                  10
#define ISCD_DSM_REPORT_CNT_MAX         3
#define ISCD_DSM_REPORT_INTERVAL        (24 * 3600)   /* 24h */
#define ISCD_DSM_LOG_SIZE_MAX           2048
#define ISCD_ERR_NO_STR_SIZE            128

#define MAX_FATAL_ISC_NUM               20
#define MAX_TRIGGER_LEVEL_NUM           5
#define ELEMS_PER_CONDITION             3
#define ISC_DAYS_PER_YEAR               365
#define TM_YEAR_OFFSET                  1900
#define DEFAULT_FATAL_ISC_DEADLINE      15
#define DEFAULT_FATAL_ISC_UPLIMIT_SOC   60
#define DEFAULT_FATAL_ISC_RECHAGE_SOC   55
#define SUCCESSIVE_ISC_JUDGEMENT_TIME   1
#define INVALID_ISC_JUDGEMENT           0
#define UPLIMIT                         0
#define RECHARGE                        1
#define F2FS_MOUNTS_INFO                "/proc/mounts"
#define MOUNTS_INFO_FILE_MAX_SIZE       (PAGE_SIZE - 1)
#define SPLASH2_MOUNT_INFO              "/splash2"
#define ISC_DATA_DIRECTORY              "/splash2/isc"
#define ISC_DATA_FILE                   "/splash2/isc/isc.data"
#define ISC_CONFIG_DATA_FILE            "/splash2/isc/isc_config.data"
#define WAIT_FOR_SPLASH2_START          5000
#define WAIT_FOR_SPLASH2_INTERVAL       1000
#define ISC_SPLASH2_INIT_RETRY          3
#define FATAL_ISC_DMD_RETRY             5
#define FATAL_ISC_DMD_RETRY_INTERVAL    2500
#define FATAL_ISC_DMD_LINE_SIZE         42
#define FATAL_ISC_MAGIC_NUM             0x5a3c7711
#define APP_CLOSE_ISC_MAGIC_NUM         0x55AA55AA
#define FATAL_ISC_TRIGGER_ISC_OFFSET    0
#define FATAL_ISC_TRIGGER_NUM_OFFSET    1
#define FATAL_ISC_TRIGGER_DMD_OFFSET    2
#define FATAL_ISC_DMD_ONLY              1
#define ISC_LIMIT_CHECKING_INTERVAL     5000
#define ISC_LIMIT_START_CHARGING_STAGE  0
#define ISC_LIMIT_UNDER_MONITOR_STAGE   1
#define ISC_LIMIT_STOP_CHARGING_STAGE   2
#define ISC_LIMIT_BOOT_STAGE            3
#define ISC_TRIGGER_WITH_TIME_LIMIT     1
#define ISC_APP_READY                   1
#define FATAL_ISC_OCV_UPDATE_THRESHOLD  20
#define FATAL_ISC_ACTION_DMD_ONLY       0x01
#define CALCULATE_LEAK_MS               20000
#define ISCD_OCV_CURRENT_LIMIT          20000
#define ISCD_CURRENT_OFFSET             (-10000)
#define MOHM_PER_OHM    1000
#define SEC_PER_HOUR    3600
#define TENTH           10
#define PERCENT         100
#define PERMILLAGE      1000
#define FCC_MAX_PERCENT 135
#define UA_PER_MA       1000

#define UOHM_PER_MOHM   1000
#define MAX_RECORDS_CNT 5
#define DEFAULT_BATTERY_OHMIC_RESISTANCE        100 /* mohm */

#define LAST_ONE        1
#define LAST_TWO        2
#define LAST_THREE      3
#define LEAST_SAMPLE_NUM        3
#define TWICE           2
#define LEAST_SAMPLE_NUM_FOR_CHECK_CC           2
#define LEAST_SAMPLE_NUM_FOR_CALC_ISHROT        2

#define DEC     10
#define ISC_TRIGGER_STEP        3
#define FATAL_ISC_SOC_LIMIT_LOWER       0
#define FATAL_ISC_SOC_LIMIT_UPPER       100
#define MAX_DTS_FATAL_ISC_SOC_LIMIT     99

enum ISCD_LEVEL_CONFIG {
	ISCD_ISC_MIN,
	ISCD_ISC_MAX,
	ISCD_DSM_ERR_NO,
	ISCD_DSM_REPORT_CNT,
	ISCD_DSM_REPORT_TIME,
	ISCD_PROTECTION_TYPE,
	ISCD_LEVEL_CONFIG_CNT
};

enum ISCD_CHARGING_EVENT {
	ISCD_CHARGE_START,
	ISCD_CHARGE_DONE,
	ISCD_CHARGE_STOP,
	ISCD_CHARGE_NOT_CHARGE,
	ISCD_CHARGE_MAX
};

struct iscd_sample_info {
	struct timespec sample_time;
	int sample_cnt; /* sample counts since charge/recharge done */
	int ocv_volt_uv; /* uV */
	s64 ocv_soc_uah; /* uAh, capacity get by OCV form battery model parameter table */
	s64 cc_value; /* uAh */
	int tbatt;
};
struct iscd_level_config {
	int isc_min;
	int isc_max;
	int isc_valid_cnt;
	int dsm_err_no;
	int dsm_report_cnt;
	time_t dsm_report_time;
	int protection_type;
};

/*
 * struct isc_history - isc history information
 * @isc_status: isc status(0 means normal)
 * @valid_num: max valid isc(fcc...) number(0 means nothing valid)
 */
struct isc_history {
	unsigned char isc_status;
	unsigned char valid_num;
	unsigned char dmd_report;
	unsigned char trigger_type;
	unsigned int magic_num;
	int isc[MAX_FATAL_ISC_NUM];
	int rm[MAX_FATAL_ISC_NUM];
	int fcc[MAX_FATAL_ISC_NUM];
	int qmax[MAX_FATAL_ISC_NUM];
	unsigned int charge_cycles[MAX_FATAL_ISC_NUM];
	unsigned short year[MAX_FATAL_ISC_NUM];
	unsigned short yday[MAX_FATAL_ISC_NUM];
};

struct isc_config {
	unsigned int write_flag;
	unsigned int delay_cycles;
	unsigned int magic_num;
	unsigned int has_reported;
};

struct isc_trigger {
	unsigned int valid_num;
	unsigned int deadline;
	unsigned int trigger_num[MAX_TRIGGER_LEVEL_NUM];
	int trigger_isc[MAX_TRIGGER_LEVEL_NUM];
	int dmd_no[MAX_TRIGGER_LEVEL_NUM];
};

struct fatal_isc_dmd {
	struct delayed_work work;
	char *buff;
	int err_no;
	int retry;
};

enum fatal_isc_action_type {
	UPLOAD_DMD_ACTION = 0,
	NORAML_CHARGING_ACTION,
	DIRECT_CHARGING_ACTION,
	UPLOAD_UEVENT_ACTION,
	UPDATE_OCV_ACTION,
	__MAX_FATAL_ISC_ACTION_TYPE,
};

struct iscd_info {
	int size;
	int isc; /* internal short current, uA */
	int isc_valid_cycles;
	int isc_valid_delay_cycles;
	int isc_chrg_delay_cycles;
	int isc_delay_cycles_enable;
	int has_reported;
	int write_flag;
	unsigned int isc_status;
	unsigned int fatal_isc_trigger_type;
	unsigned int fatal_isc_soc_limit[2];
	unsigned int fatal_isc_action;
	unsigned int fatal_isc_action_dts;
	struct fatal_isc_dmd dmd_reporter;
	spinlock_t boot_complete;
	unsigned int app_ready;
	unsigned int uevent_wait_for_send;
	int isc_prompt;
	int isc_splash2_ready;
	struct isc_history fatal_isc_hist;
	struct isc_config fatal_isc_config;
	struct isc_trigger fatal_isc_trigger;
	s64 full_update_cc; /* uAh */
	int last_sample_cnt; /* last sample counts since charge /recharge done */
	struct iscd_sample_info sample_info[ISCD_SMAPLE_LEN_MAX];
	int rm_bcd;   /* rm bofore charging done */
	int fcc_bcd;  /* fcc bofore charging done */

	int enable;
	int ocv_min;
	int tbatt_min;
	int tbatt_max;
	int tbatt_diff_max;
	int sample_time_interval;
	int sample_time_delay;
	int calc_time_interval_min;
	int calc_time_interval_max;
	int isc_warning_threhold;
	int isc_error_threhold;
	int isc_critical_threhold;
	int total_level;
	int isc_buff[ISCD_ISC_MAX_SIZE]; /* isc_buff[0]:buffer size */
	struct timespec last_sample_time;
	struct iscd_level_config level_config[ISCD_MAX_LEVEL];
	struct delayed_work delayed_work; /* ISCD: detect_battery short current */
	struct hrtimer timer;
	struct mutex iscd_lock;

	int need_monitor;
	struct delayed_work isc_limit_work;
	struct blocking_notifier_head isc_limit_func_head;
	struct notifier_block fatal_isc_direct_chg_limit_soc_nb;
	struct notifier_block fatal_isc_chg_limit_soc_nb;
	struct work_struct fatal_isc_uevent_work;
	struct notifier_block fatal_isc_uevent_notify_nb;
	struct notifier_block fatal_isc_ocv_update_nb;
	struct notifier_block fatal_isc_dsm_report_nb;
	struct notifier_block isc_listen_to_charge_event_nb;

	struct device *dev;
	int charging_state;
	struct notifier_block charger_nb;
	struct notifier_block ocv_nb;
	struct delayed_work leak_work;

	struct delayed_work isc_splash2_work;
	int isc_datum_init_retry;

	unsigned int ocv_update_interval;
	int iscd_file_magic_num;
};

void update_isc_hist(struct iscd_info *iscd,
		int (*valid)(struct iscd_info *iscd, struct rtc_time *tm));
int smallest_in_oneday(struct iscd_info *iscd, struct rtc_time *tm);
void __fatal_isc_uevent(struct work_struct *work);

#endif /* _BATTERY_ISCD_H_ */

