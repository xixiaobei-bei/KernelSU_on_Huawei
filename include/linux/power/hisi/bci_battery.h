/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description:  battery driver for Linux
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

#ifndef _LINUX_BCI_BATTERY_H
#define _LINUX_BCI_BATTERY_H

enum charge_status_event {
	VCHRG_POWER_NONE_EVENT = 0,
	VCHRG_NOT_CHARGING_EVENT,
	VCHRG_START_CHARGING_EVENT,
	VCHRG_START_AC_CHARGING_EVENT,
	VCHRG_START_USB_CHARGING_EVENT,
	VCHRG_CHARGE_DONE_EVENT,
	VCHRG_STOP_CHARGING_EVENT,
	VCHRG_POWER_SUPPLY_OVERVOLTAGE,
	VCHRG_POWER_SUPPLY_WEAKSOURCE,
	VCHRG_STATE_WDT_TIMEOUT,
	BATTERY_LOW_WARNING,
	BATTERY_LOW_SHUTDOWN,
	BATTERY_MOVE,
	SWITCH_INTB_WORK_EVENT,
	VCHRG_THERMAL_POWER_OFF,
	WIRELESS_TX_STATUS_CHANGED,
	WIRELESS_COVER_DETECTED,
	VCHRG_CURRENT_FULL_EVENT,
};

#define CURRENT_OFFSET             10
#define DSM_BATTERY_MAX_SIZE       2048
#define IMPOSSIBLE_IAVG            9999
/* State-of-Charge-Threshold 1 bit */
#define BQ_FLAG_SOC1    (1<<2)
/* State-of-Charge-Threshold Final bit */
#define BQ_FLAG_SOCF    (1<<1)
#define BQ_FLAG_LOCK    (BQ_FLAG_SOC1 | BQ_FLAG_SOCF)

#define BAT_VOL_3200               3200
#define BAT_VOL_3500               3500
#define BAT_VOL_3550               3550
#define BAT_VOL_3600               3600
#define BAT_VOL_3700               3700
#define BAT_VOL_3800               3800
#define BAT_VOL_3850               3850
#define BAT_VOL_3900               3900
#define BAT_VOL_3950               3950
#define BAT_VOL_4000               4000
#define BAT_VOL_4200               4200
#define BAT_VOL_4250               4250

#define CAPACITY_2                 2
#define CAPACITY_10                10
#define CAPACITY_20                20
#define CAPACITY_30                30
#define CAPACITY_40                40
#define CAPACITY_50                50
#define CAPACITY_60                60
#define CAPACITY_65                65
#define CAPACITY_70                70
#define CAPACITY_85                85

#define RECHG_PROTECT_THRESHOLD    150
#define WORK_INTERVAL_PARA_LEVEL   5
#define CAPACITY_MIN               0
#define CAPACITY_MAX               100
#define CAPACITY_FULL              100
#define WORK_INTERVAL_MIN          1
#define WORK_INTERVAL_MAX          60000 /* 60s */
#define WORK_INTERVAL_NOARMAL      10000 /* 10s */
#define WORK_INTERVAL_REACH_FULL   30000 /* 30s */

#define CHARGE_CURRENT_OVERHIGH_TH 10000
#define DISCHARGE_CURRENT_OVERHIGH_TH (-7000)

#define BATT_VOLT_OVERLOW_TH       2800
#define BATT_VOLT_OVERHIGH_TH      4550

#define BATT_TEMP_OVERLOW_TH       (-10)

#define BATT_NOT_TERMINATE_TH      50

#define BATT_CAPACITY_REDUCE_TH    80

#define DSM_CHECK_TIMES_LV1        3
#define DSM_CHECK_TIMES_LV2        5
#define DSM_CHECK_TIMES_LV3        10

#define CHG_ODD_CAPACITY           2
#define CHG_CANT_FULL_THRESHOLD    95
#define CHARGE_FULL_TIME           40 /* 40min */
#define MIN2MSEC                   60000
#define CURRENT_THRESHOLD          10

#define MAX_CONFIRM_CNT            3
#define CONFIRM_INTERVAL           100 /* 100 ms */

#define CAP_LOCK_PARA_LEVEL        2

#define PERMILLAGE                 1000
#define PERCENT                    100
#define TENTH                      10
#define HALF                       2

#define LOW_TEMP_FIVE_DEGREE       5
#define LEAST_FACTORY_CAPACITY     3
#define FACTOEY_CAPACITY_TOO_LOW   2
#define LOW_CAPACITY               2
#define TEMP_MINUS_5               (-5)
#define SLEEP_TIME_MS              100
#define DEFAULT_TEMP_TENTH         250
#define DEC                        10
#define DELTA_CAPACITY             2
#define DEFAULT_RESISTANCE         120
#define SOC_FULL                   100

enum plugin_status {
	/* no charger plugin */
	PLUGIN_DEVICE_NONE,
	/* plugin usb charger */
	PLUGIN_USB_CHARGER,
	/* plugin ac charger */
	PLUGIN_AC_CHARGER,
};

enum work_interval_para_info {
	WORK_INTERVAL_CAP_MIN = 0,
	WORK_INTERVAL_CAP_MAX,
	WORK_INTERVAL_VALUE,
	WORK_INTERVAL_PARA_TOTAL,
};

struct work_interval_para {
	int cap_min;
	int cap_max;
	int work_interval;
};

struct cap_lock_para {
	int cap;
	int level_vol;
};

enum bat_board_type {
	BAT_BOARD_SFT = 0,
	BAT_BOARD_UDP = 1,
	BAT_BOARD_ASIC = 2,
	BAT_BOARD_MAX
};

void get_notifier_list(struct blocking_notifier_head **charge_evt_notifier_list);
int bci_get_charge_current_overhigh_th(void);
int bci_register_notifier(struct notifier_block *nb, unsigned int events);
int bci_unregister_notifier(struct notifier_block *nb, unsigned int events);
int bci_show_capacity(void);
enum fcp_check_stage_type fcp_get_stage_status(void);
unsigned int get_bci_soc(void);
void batt_info_dump(char *pstr);
int bci_capacity_get_filter_sum(int base);
void bci_capacity_sync_filter(int rep_soc, int round_soc, int base);

#endif

