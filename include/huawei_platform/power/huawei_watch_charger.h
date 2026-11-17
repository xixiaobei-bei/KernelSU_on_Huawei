/*
 * huawei_watch_charger.h
 *
 * huawei watch charger interface for power module
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

#ifndef _HUAWEI_WATCH_CHARGER_
#define _HUAWEI_WATCH_CHARGER_

enum {
	CHARGER_OFFLINE = 0,
	CHARGER_ONLINE = 1,
};

enum {
	CHARGEING_ST = 1,
	DISCHARGEING_ST = 2,
};

enum {
	DISABLE_HIZ_MODE = 0,
	ENABLE_HIZ_MODE  = 1,
};

enum watch_charge_cmd_to_mcu {
	/* coulometer self check */
	CHECK_COULOMTER  = 1,
	/* get battery voltage */
	GET_BAT_VOLT     = 2,
	/* get battery temperature */
	GET_BAT_TEMP     = 3,
	/* get battery capacity */
	GET_BAT_CAP      = 4,
	/* get charging status */
	GET_CHG_ST       = 5,
	/* check whether need battery compensate */
	CHECK_SPLY       = 6,
	/* set enable charging status */
	SET_CHG_ST       = 7,
	/* get charge current */
	GET_CHG_CUR      = 8,
	/* input current limit */
	SET_IIN_LIMIT    = 9,
	/* charge current limit */
	SET_ICHG_LIMIT   = 10,
	/* enable or disable HIZ 0:disable 1:enable */
	SET_ENABLE_HIZ   = 11,
	/* set stop charge cut-off voltage */
	SET_TERM_VOLT    = 12,
	/* defalt cmd */
	DEFAULT_CMD      = 0XFF,
};

enum {
	CHARGE_ST        = 0,
	BAT_VOLTAGE      = 1,
	BAT_CURRENT      = 2,
	BAT_CAPACITY     = 3,
	BAT_TEMPERATURE  = 4,
	BAT_ID           = 5,
	ONLINE_ST        = 6,
	BAT_CAPACITY_RM  = 7,
	BAT_CAPACITY_FCC = 8,
	CHARGE_CYCLE_CNT = 9,
	BAT_CHARGE_FULL  = 10,
	BAT_CHARGE_FULL_DESIGN = 11,
	BAT_VOLTAGE_MAX  = 12,
	BAT_CAPACITY_LVL = 13,
	WIRELESS_ALIGN   = 14,
	INFO_MAX         = 15,
};

enum watch_capacity_level {
	CAPACITY_LVL_UNKNOWN  = 0,
	CAPACITY_LVL_CRITICAL = 1,
	CAPACITY_LVL_LOW      = 2,
	CAPACITY_LVL_NORMAL   = 3,
	CAPACITY_LVL_HIGH     = 4,
	CAPACITY_LVL_FULL     = 5,
};

enum watch_charger_type {
	/* wireless charger */
	CHARGER_TYPE_WIRELESS = 11,
};

enum {
	MCU_RESPONSE_CMD =    0,
	MCU_RESPONSE_SUBCMD = 1,
	MCU_RESPONSE_DATA =   2,
	MCU_RESPONSE_MAX =    3,
};

enum {
	MCU_CHARGE_NOT_DONE = 0,
	MCU_CHARGE_DONE     = 1,
};

enum {
	WIRELESS_CHARGE_ALIGNED    = 0,
	WIRELESS_CHARGE_MISALIGNED = 1,
};

enum watch_charge_sysfs_type {
	WATCH_CHARGE_SYSFS_IIN_RUNNINGTEST,
	WATCH_CHARGE_SYSFS_ICHG_RUNNINGTEST,
	WATCH_CHARGE_SYSFS_REGULATION_VOLTAGE,
	WATCH_CHARGE_SYSFS_HIZ,
	WATCH_CHARGE_SYSFS_CHARGE_TYPE,
	WATCH_CHARGE_SYSFS_CHARGE_DONE_STATUS,
};

struct mcu_chg_info {
	/* charging status:charging or discharging */
	int charge_st;
	/* battery volatge unit:mV */
	int bat_vol;
	/* battery current unit:mA */
	int bat_cur;
	/* battery capcity 0~100 */
	int bat_cap;
	/* battery temprature unit:0.1 degree */
	int bat_temp;
	/* battary ID unit:mV */
	int bat_id;
	/* charge online status 0:offline; 1:online */
	int online;
	/* battery residue captcity unit:mAh unit:mAh */
	int bat_cap_rm;
	/* battery capcity fcc unit:mAh */
	int bat_cap_fcc;
	/* charge cycle count */
	int charge_cycle_cnt;
	/* capacity of charge full unit:mAh */
	int bat_charge_full;
	/* battery stand capacity of design unit:mAh */
	int bat_charge_full_design;
	/* battery stand voltage of design unit:mV */
	int bat_voltage_max;
	/* battery capacity level: Unknown, Critical, Low, Normal, High, Full */
	int bat_capacity_level;
	/* wireless charger aligned status 0:aligned 1:misaligned */
	int wireless_aligned_status;
	/* battery residue captcity unit:mAh */
	int bat_charge_now;
};

struct watch_charge_sysfs_data {
	unsigned int iin_limit;
	unsigned int ichg_limit;
	unsigned int vterm_limit;
	unsigned int hiz_status;
	unsigned int charge_enable;
	unsigned int charge_done_status;
};

struct read_data_st {
	int cmd;
	struct mcu_chg_info info;
};

struct send_cmd_st {
	int cmd;
	int sub_cmd;
};

#define MCU_SET_SUCCESSFUL          0
#define MCU_SET_UNSUCCESSFUL        (-1)

#define STATUS_TRUE                 1
#define STATUS_FALSE                0
#define DEFAULT_BAT_VOLTAGE         3800
#define DEFAULT_BAT_VOLTAGE_MAX     4400
#define DEFAULT_CHARGE_CURRENT      200
#define DEFAULT_BAT_CAPACITY        60
#define CHARGE_DONE_CAPACITY        100
#define DEFAULT_BAT_TEMPERATURE     25
#define DEFAULT_BAT_CAPACITY_RM     210
#define DEFAULT_BAT_CAPACITY_FCC    420
#define DEFAULT_CHARGER_ONLINE_ST   (-1)
#define CHG_SERVICE_ID              0X01
#define CHG_READ_COMMAND_ID         0X86
#define CHG_WRITE_COMMAND_ID        0X81
#define CHG_SID_CID_COUNT           2
#define MCU_RESPONSE_BUFFER_LEN     12
#define MCU_REPORT_BUFFER_LEN       60
#define RESOLVE_STEP_LEN            4
#define POWEROFF_DELAY_MS           (90 * 1000)
#define POWEROFF_CAPACTY            0
#define WAIT_RESP_TIMEOUT_MS        200
#define GET_CHARGE_INFO_PERIOD      5
#define RESET_POWEROFF_CAPACITY     8

#endif /* _HUAWEI_WATCH_CHARGER_ */
