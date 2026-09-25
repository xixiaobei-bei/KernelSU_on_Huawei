/*
 * hisi_charge_time.h  --  Calculate the remaining time of charging.
 *
 * Copyright (c) 2019 Huawei Technologies CO., Ltd.
 *
 * Author: yangshunlong
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation; either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __HISI_CHARGE_DURATION_H__
#define __HISI_CHARGE_DURATION_H__

#define CHG_TYPE_DETECT_TIME	2000
#define DURATION_PARA_MAX	5

#define CHG_DUR_FILE_LIMIT	0660
#define SOC_FULL		100

#define CHG_DUR_SIZE		0x1000
#define CHG_DUR_OFFSET		0x19000 // offset from 100K

#define CHG_DUR_BUF_SHOW_LEN	128

#define NSEC_PRE_SECOND		(1000 * 1000 * 1000)

#define MAX_NUM			102

#define PARAM_SIZE		401
#define MULT_100		100
#define FULL_SOC		100
#define NORMAL_TEMP		25

#define TEMP_HIGH		50
#define TEMP_LOW		10

#define ADJUST_FCC_SOC		95
#define ADJUST_FCC_CURRENT	300
#define SEC_PRE_HOUR		3600
#define MIN_START_TH		(60 * 5)
#define SOC_START_TH		40
#define CYCLE_TH		5

#define ROW			10
#define COL			10

/* Add valid flag for framework to check*/
#define VALID_FLAG		0x55000000

static char *charge_type_str_enum[] = {
	"CHG_NONE", "CHG_USB", "CHG_5V2A", "CHG_FCP", "CHG_LVC", "CHG_SC",
	"CHG_OTHERS",
};

enum charge_type_enum {
	CHG_NONE,
	CHG_USB,
	CHG_5V2A,
	CHG_FCP,
	CHG_LVC,
	CHG_SC,
	CHG_OTHERS,
};

struct hisi_soc_durations {
	int soc;
	int durations;
};

struct hisi_chg_time_param {
	int step_time[SOC_FULL + 1];
	int volt[SOC_FULL + 1];
	int start_soc;
	int batt_cycles;
	int temp_high;
	int temp_low;
	int fcc;
	int reserved[5];
};

struct hisi_chg_time_info {
	struct hisi_chg_time_param standard;
	struct hisi_chg_time_param fcp;
	struct hisi_chg_time_param lvc;
	struct hisi_chg_time_param lvc_none_standard;
	struct hisi_chg_time_param sc;
	struct hisi_chg_time_param sc_none_standard;
};

struct hisi_batt_info {
	int fcc;
	int soc;
	int volt_mv;
	int curr_ma;
	int batt_temp;
	int batt_cycles;
};

struct hisi_chg_info {
	enum charge_type_enum charge_type;
	int cc_cable_detect_ok;
	int direct_charge_type;
	bool direct_charge_done;
	int direct_charge_done_soc;
};
struct param {
	int adjust_by_current;
	int adjust_by_fcc;
	int adjust_by_temp;
	int adjust_by_volt;
};

struct hisi_chg_time_device {
	int remaining_duration;
	int remaining_duration_with_valid;
	int test_flag;
	int pre_capacity;
	u64 pre_timestemp;
	int start_capacity;
	u64 start_timestemp;
	int charge_type_change;
	int work_running;
	struct hisi_chg_info chg_info;
	struct hisi_batt_info batt_info;

	struct hisi_chg_time_info flash_param;
	struct hisi_chg_time_param cur_curve;
	struct hisi_chg_time_param *ref_curve;
	struct delayed_work charge_time_work;
	struct param param_dts;
	struct notifier_block nb;
	struct notifier_block direct_charger_nb;
	struct device *dev;
};

#ifdef CONFIG_HISI_CHARGE_TIME
extern int hisi_chg_time_remaining(bool with_valid);
#else
static inline int hisi_chg_time_remaining(bool with_vaild)
{
	return -1;
}
#endif

#define CHGDUR_LOG_INFO
#ifndef CHGDUR_LOG_INFO
#define ct_dbg(fmt, args...)\
	do {\
	} while (0)
#define ct_info(fmt, args...)\
	do {\
	} while (0)
#define ct_warn(fmt, args...)\
	do {\
	} while (0)
#define chg_time_err(fmt, args...)\
	do {\
	} while (0)
#else
#define ct_dbg(fmt, args...)\
		pr_info("[hisi_chg_time]" fmt, ##args)
#define ct_info(fmt, args...)\
		pr_info("[hisi_chg_time]" fmt, ##args)
#define ct_warn(fmt, args...)\
		pr_warn("[hisi_chg_time]" fmt, ##args)
#define ct_err(fmt, args...)\
		pr_err("[hisi_chg_time]" fmt, ##args)
#endif

#endif
