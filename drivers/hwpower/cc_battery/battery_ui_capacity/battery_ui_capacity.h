/*
 * battery_ui_capacity.h
 *
 * huawei battery ui capacity
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

#ifndef _BATTERY_UI_CAPACITY_H_
#define _BATTERY_UI_CAPACITY_H_

#include <linux/workqueue.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/coul/coul_interface.h>

#define BUC_WINDOW_LEN                      10
#define BUC_VBAT_MIN                        3450
#define BUC_CAPACITY_EMPTY                  0
#define BUC_CAPACITY_FULL                   100
#define BUC_SOC_CALIBRATION_PARA_LEVEL      2
#define BUC_SOC_JUMP_THRESHOLD              2
#define BUC_RECHG_PROTECT_VOLT_THRESHOLD    150
#define BUC_CURRENT_THRESHOLD               10
#define BUC_LOWTEMP_THRESHOLD               (-50)
#define BUC_LOW_CAPACITY_THRESHOLD          2
#define BUC_LOW_CAPACITY                    3
#define BUC_DEFAULT_CAPACITY                50
#define BUC_CAPACITY_AMPLIFY                100
#define BUC_CAPACITY_DIVISOR                100
#define BUC_CONVERT_VOLTAGE                 12
#define BUC_CONVERT_CURRENT                 100
#define BUC_WORK_INTERVAL_NORMAL            10000
#define BUC_WORK_INTERVAL_LOWTEMP           5000
#define BUC_WORK_INTERVAL_CHARGING          5000
#define BUC_WORK_INTERVAL_FULL              30000
#define BUC_CHG_FORCE_FULL_SOC_THRESHOLD    95
#define BUC_CHG_FORCE_FULL_TIME             (2400000 / BUC_WORK_INTERVAL_FULL)

struct bat_ui_vbat_para {
	int volt;
	int soc;
};

struct bat_ui_soc_calibration_para {
	int soc;
	int volt;
};

struct bat_ui_capacity_device {
	struct device *dev;
	struct delayed_work update_work;
	int charge_status;
	int ui_cap_zero_offset;
	int soc_at_term;
	int ui_capacity;
	int bat_exist;
	int bat_volt;
	int bat_cur;
	int bat_temp;
	int prev_soc;
	int bat_max_volt;
	int ui_prev_capacity;
	int capacity_sum;
	int vth_correct_en;
	int chg_force_full_soc_thld;
	int chg_force_full_count;
	int chg_force_full_wait_time;
	int capacity_filter_count;
	u16 monitoring_interval;
	u32 disable_pre_vol_check;
	struct notifier_block event_nb;
	int capacity_filter[BUC_WINDOW_LEN];
	struct bat_ui_soc_calibration_para vth_soc_calibration_data[BUC_SOC_CALIBRATION_PARA_LEVEL];
};

#ifdef CONFIG_HUAWEI_BATTERY_UI_CAPACTIY
int bat_ui_capacity(void);
int bat_ui_raw_capacity(void);
#else
static inline int bat_ui_capacity(void)
{
	return coul_interface_get_battery_capacity(COUL_TYPE_MAIN);
}

static inline int bat_ui_raw_capacity(void)
{
	return coul_interface_get_battery_capacity(COUL_TYPE_MAIN);
}
#endif /* CONFIG_HUAWEI_BATTERY_UI_CAPACTIY */

#endif /* _BATTERY_UI_CAPACITY_H_ */
