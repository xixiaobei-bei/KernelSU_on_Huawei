/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description:  battery driver for Linux
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/power_supply.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/version.h>
#ifdef CONFIG_HUAWEI_BATTERY_TEMP
#include <chipset_common/hwpower/battery/battery_temp.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <chipset_common/hwpower/common_module/power_dsm.h>
#endif
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
#include <huawei_platform/power/battery_voltage.h>
#endif
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger/direct_charger.h>
#endif
#ifdef CONFIG_HUAWEI_BATTERY_CAPACITY
#include <huawei_platform/power/huawei_battery_capacity.h>
#endif
#ifdef CONFIG_HUAWEI_CHARGER_AP
#include <huawei_platform/power/huawei_charger.h>
#endif
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif
#ifdef CONFIG_HUAWEI_TYPEC
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>
#endif
#ifdef CONFIG_WIRELESS_CHARGER
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#endif
#include <securec.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/coul/coul_event.h>
#include <linux/power/hisi/bci_battery.h>
#ifdef CONFIG_CHARGE_TIME
#include "charge_time/hisi_charge_time.h"
#endif

#define bci_debug(fmt, args...) pr_debug("[batt_bci]" fmt, ## args)
#define bci_info(fmt, args...) pr_info("[batt_bci]" fmt, ## args)
#define bci_warn(fmt, args...) pr_warn("[batt_bci]" fmt, ## args)
#define bci_err(fmt, args...) pr_err("[batt_bci]" fmt, ## args)

#define WINDOW_LEN 10
#define INTERVAL_SIZE \
	(WORK_INTERVAL_PARA_LEVEL * sizeof(struct work_interval_para))

struct batt_dsm {
	int error_no;
	bool notify_enable;
	void (*dump)(char *);
	int (*check_error)(char *buf);
};

static struct wakeup_source g_low_power_lock;
static int g_is_low_power_locked;
static unsigned int g_capacity_filter[WINDOW_LEN];
static unsigned int g_capacity_sum;
static unsigned int g_suspend_capacity;
static bool g_capacity_debounce_flag;
static int g_removable_batt_flag;
static int g_is_board_type;
static int is_fake_battery;
static int g_google_battery_node;
static int g_charge_current_overhigh_th;
struct kobject *g_sysfs_bq_bci;
module_param(is_fake_battery, int, 0644);

enum bci_info_dump {
	BATT_ID,
	BATT_CYCLE,
	FCC_DESIGN,
	FCC,
	RM,
	SOC,
	VBATT,
	VBUS,
	IBUS,
	ICHG,
	CURR_NOW,
	AVG_CURR,
	TBATT,
	TUSB,
	MAX,
};

struct bci_device_info {
	int bat_voltage;
	int bat_max_volt;
	int bat_temperature;
	int bat_exist;
	int bat_health;
	int bat_capacity;
	int bat_capacity_level;
	int bat_technolog;
	int bat_design_fcc;
	int bat_rm;
	int bat_fcc;
	int bci_soc_at_term;
	int bat_current;
	unsigned int bat_err;
	int charge_status;
	int power_supply_status;
	u8 usb_online;
	u8 ac_online;
	u8 chargedone_stat;
	u16 monitoring_interval;
	int watchdog_timer_status;
	int capacity;
	unsigned long event;
	unsigned int capacity_filter_count;
	unsigned int prev_capacity;
	unsigned int charge_full_count;
	unsigned int chg_full_rpt_thr;
	unsigned int chg_full_wait_times;
	unsigned int wakelock_enabled;
	struct power_supply *bat;
	struct power_supply *usb;
	struct power_supply *bat_google;
	struct power_supply *ac;
	struct power_supply *wireless;
	struct power_supply *bk_bat;
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work bci_monitor_work;
	struct work_interval_para interval_data[WORK_INTERVAL_PARA_LEVEL];
	struct cap_lock_para vth_correct_data[CAP_LOCK_PARA_LEVEL];
	unsigned int vth_correct_en;
	u32 disable_pre_vol_check;
	u32 sc_uisoc_update_speed;
};

static struct bci_device_info *g_bci_dev;

static BLOCKING_NOTIFIER_HEAD(g_notifier_list);

void get_notifier_list(struct blocking_notifier_head **charge_evt_notifier_list)
{
	if (charge_evt_notifier_list == NULL)
		return;
	*charge_evt_notifier_list = &g_notifier_list;
}

static enum power_supply_property bci_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_LIMIT_FCC,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CAPACITY_RM,
	POWER_SUPPLY_PROP_CAPACITY_FCC,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_ID_VOLTAGE,
	POWER_SUPPLY_PROP_BRAND,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_BAT_QMAX,
	POWER_SUPPLY_PROP_CHARGE_TIME_REMAINING,
};

static enum power_supply_property usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
};

static enum power_supply_property ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_HEALTH,
};

#ifdef CONFIG_WIRELESS_CHARGER
static enum power_supply_property wireless_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};
#endif

static enum power_supply_property bk_bci_battery_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

#ifdef CONFIG_HUAWEI_BATTERY_CAPACITY
static int bci_health(void)
{
	return huawei_battery_health();
}

static int bci_capacity(void)
{
	return huawei_battery_capacity();
}
#else
static int bci_health(void)
{
	return coul_drv_battery_health();
}

static int bci_capacity(void)
{
	return coul_drv_battery_capacity();
}
#endif

static int bci_temp(void)
{
	int temp = 0;
#ifdef CONFIG_HUAWEI_BATTERY_TEMP
	bat_temp_get_temperature(BAT_TEMP_MIXED, &temp);
#else
	temp = coul_drv_battery_temperature();
#endif
	return temp;
}

static int calc_capacity_from_voltage(void)
{
	int data;
	int batt_voltage;
	int batt_current;

	batt_current = -coul_drv_battery_current();
	batt_voltage = coul_drv_battery_voltage();
	if (batt_voltage <= BAT_VOL_3200) {
		data = 0;
		return data;
	}
	batt_voltage = coul_drv_battery_voltage() -
		DEFAULT_RESISTANCE * batt_current / PERMILLAGE;
	/* calculating the capacity based on the voltage */
	if (batt_voltage < BAT_VOL_3500)
		data = CAPACITY_2;
	else if (batt_voltage < BAT_VOL_3550)
		data = CAPACITY_10;
	else if (batt_voltage < BAT_VOL_3600)
		data = CAPACITY_20;
	else if (batt_voltage < BAT_VOL_3700)
		data = CAPACITY_30;
	else if (batt_voltage < BAT_VOL_3800)
		data = CAPACITY_40;
	else if (batt_voltage < BAT_VOL_3850)
		data = CAPACITY_50;
	else if (batt_voltage < BAT_VOL_3900)
		data = CAPACITY_60;
	else if (batt_voltage < BAT_VOL_3950)
		data = CAPACITY_65;
	else if (batt_voltage < BAT_VOL_4000)
		data = CAPACITY_70;
	else if (batt_voltage < BAT_VOL_4250)
		data = CAPACITY_85;
	else
		data = CAPACITY_FULL;
	return data;
}

/* only charge-work can not reach full(95%).Change capacity to full after 40min. */
static int force_full_timer(int curr_capacity,
	struct bci_device_info *di)
{
	static u64 chg_full_thr_enter_time;
	u64 chg_full_now_time;

	if (curr_capacity > (int)di->chg_full_rpt_thr) {
		if (di->charge_full_count == 0)
			chg_full_thr_enter_time = get_jiffies_64();

		chg_full_now_time = get_jiffies_64();

		if (time_after64(chg_full_now_time,
			chg_full_thr_enter_time + msecs_to_jiffies(di->chg_full_wait_times * MIN2MSEC))) {
			bci_info("FORCE_CHARGE_FULL = %d\n", curr_capacity);
			curr_capacity = CAPACITY_FULL;
		} else {
			di->charge_full_count++;
		}
	} else {
		di->charge_full_count = 0;
	}
	return curr_capacity;
}

static int capacity_pulling_filter(int curr_capacity,
	struct bci_device_info *di)
{
	int index;
	int retval_cap;

	di->bat_exist = coul_drv_is_battery_exist();

	if ((!di->bat_exist) || (is_fake_battery)) {
		retval_cap = calc_capacity_from_voltage();
		return retval_cap;
	}
	index = di->capacity_filter_count % WINDOW_LEN;

	g_capacity_sum -= g_capacity_filter[index];
	g_capacity_filter[index] = curr_capacity;
	g_capacity_sum += g_capacity_filter[index];

	if (++di->capacity_filter_count >= WINDOW_LEN)
		di->capacity_filter_count = 0;

	/* rounding-off 0.5 method */
	retval_cap = (g_capacity_sum) / WINDOW_LEN;

	return retval_cap;
}

/* exit scp (example : 87.5% to 87%) */
static void reset_capacity_fifo(int curr_capacity)
{
	unsigned int i;

	g_capacity_sum = 0;

	for (i = 0; i < WINDOW_LEN; i++) {
		g_capacity_filter[i] = curr_capacity;
		g_capacity_sum += g_capacity_filter[i];
	}
}

static void adjust_bci_work_interval(int capacity,
	struct bci_device_info *di)
{
	/* if temp is lower than 5, reduce interval to NORMAL / 2 */
	if (coul_drv_low_temp_opt() && (bci_temp() < LOW_TEMP_FIVE_DEGREE) &&
		(di->monitoring_interval > (WORK_INTERVAL_NOARMAL / HALF)))
		di->monitoring_interval = WORK_INTERVAL_NOARMAL / HALF;
#ifdef CONFIG_DIRECT_CHARGER
	if (dc_get_super_charging_flag() && di->ac_online &&
		(di->monitoring_interval > (WORK_INTERVAL_NOARMAL / HALF)))
		di->monitoring_interval = WORK_INTERVAL_NOARMAL / HALF;
#endif
#ifdef CONFIG_WIRELESS_CHARGER
	if (wlrx_get_charge_stage() >= WLRX_STAGE_REGULATION_DC)
		di->monitoring_interval = WORK_INTERVAL_NOARMAL / HALF;
#endif

	if (capacity > CHG_CANT_FULL_THRESHOLD)
		di->monitoring_interval = di->sc_uisoc_update_speed;

	if (di->chargedone_stat && (capacity <= CHG_CANT_FULL_THRESHOLD))
		di->monitoring_interval = WORK_INTERVAL_MAX;
}

static void bci_set_work_interval(int capacity, struct bci_device_info *di)
{
	int i;

	for (i = 0; i < WORK_INTERVAL_PARA_LEVEL; i++) {
		if ((capacity >= di->interval_data[i].cap_min) &&
			(capacity <= di->interval_data[i].cap_max)) {
			di->monitoring_interval =
				di->interval_data[i].work_interval;
			break;
		}
	}

	if (!di->monitoring_interval)
		di->monitoring_interval = WORK_INTERVAL_NOARMAL;

	adjust_bci_work_interval(capacity, di);
}

static int check_curr_low_capacity(int curr_capacity,
	struct bci_device_info *di)
{
	int battery_volt;
	int i;

	for (i = 0; i < CAP_LOCK_PARA_LEVEL; i++) {
		if (curr_capacity < di->vth_correct_data[i].cap &&
			di->prev_capacity >= di->vth_correct_data[i].cap) {
#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
			/* if multi battery, get MIN voltage */
			battery_volt = hw_battery_voltage(BAT_ID_MIN);
#else
			battery_volt = coul_drv_battery_voltage();
#endif
			if (battery_volt >= di->vth_correct_data[i].level_vol) {
				bci_info("low capacity reported, battery_vol = %d mv, prev_capacity = %d, capacity = %d, lock_cap:%d\n",
					battery_volt, di->prev_capacity, curr_capacity,
					di->vth_correct_data[i].cap);
				return di->vth_correct_data[i].cap;
			}
		}
	}
	return curr_capacity;
}

/* correct soc according to voltage threshhold */
static int vth_correct_soc(int curr_capacity, struct bci_device_info *di)
{
	if (!di->vth_correct_en)
		return curr_capacity;
	if ((di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) ||
		 (di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING))
		return check_curr_low_capacity(curr_capacity, di);
	return curr_capacity;
}

static int check_curr_capacity(const int curr_capacity,
	struct bci_device_info *di)
{
	int low_bat_flag = coul_drv_is_battery_reach_threshold();
	int battery_volt;

	if ((!di->bat_exist) &&
		strstr(saved_command_line, "androidboot.swtype=factory")) {
		/*
		 * when in facotry mode and battery is not exist,
		 * keep capacity > 2 to prevent system shutdown
		 */
		if (curr_capacity <= FACTOEY_CAPACITY_TOO_LOW) { /* limit */
			di->capacity = LEAST_FACTORY_CAPACITY; /* limit */
			di->prev_capacity = LEAST_FACTORY_CAPACITY; /* limit */
			return 1;
		}
	}

	if ((low_bat_flag != BQ_FLAG_LOCK) &&
		g_is_low_power_locked) {
		__pm_relax(&g_low_power_lock);
		g_is_low_power_locked = 0;
	}

	/* Debouncing of power on init. */
	if (di->capacity == -1) {
		di->capacity = curr_capacity;
		di->prev_capacity = curr_capacity;
		return 1;
	}

	/* Only availability if the capacity changed */
	if (curr_capacity != di->prev_capacity) {
		if (abs(di->prev_capacity - curr_capacity) >=
			CHG_ODD_CAPACITY)
			bci_info("prev_capacity = %u\n"
				 "curr_capacity = %d\n"
				 "curr_voltage = %d\n", di->prev_capacity,
				 curr_capacity, coul_drv_battery_voltage());
	}

	/* if SOC < 2% check voltage is also low */
	if ((curr_capacity < LOW_CAPACITY) && !di->disable_pre_vol_check) {
#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
		/* if multi battery, get MIN voltage */
		battery_volt = hw_battery_voltage(BAT_ID_MIN);
#else
		battery_volt = coul_drv_battery_voltage();
#endif
		if (battery_volt >= BAT_VOL_3500) {
			bci_info("low capacity reported, battery_vol = %d mv, capacity = %d\n",
				 battery_volt, curr_capacity);
			return 0;
		}
	}
	return -1;
}

static int capacity_correct_check(int curr_capacity,
	struct bci_device_info *di)
{
	int low_temp_capacity_record;
	int correct_capacity;

	bci_set_work_interval(curr_capacity, di);
	correct_capacity = vth_correct_soc(curr_capacity, di);
	low_temp_capacity_record = correct_capacity;

	/* filter */
	correct_capacity = capacity_pulling_filter(correct_capacity, di);
	/* if temp < -5 degree, must only reduce 1 */
	if (coul_drv_low_temp_opt() && (bci_temp() < TEMP_MINUS_5) &&
			(correct_capacity - low_temp_capacity_record > 1)) {
		bci_info("low_temp_opt:filter_curr_capacity = %d, low_temp_capacity_record= %d\n",
			correct_capacity, low_temp_capacity_record);
		correct_capacity -= 1;
		bci_info("low_temp_opt: low_temp_capacity = %d\n",
			correct_capacity);
	}

	if (di->prev_capacity == correct_capacity)
		return 0;

	if ((di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) ||
		 (di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING)) {
		if ((int)di->prev_capacity < correct_capacity)
			return 0;
	}

	if (di->charge_status == POWER_SUPPLY_STATUS_CHARGING &&
		(-coul_drv_battery_current() >= CURRENT_THRESHOLD)) {
		if ((int)di->prev_capacity > correct_capacity)
			return 0;
	}

	bci_info("Capacity Updated = %d, charge_full_count = %u, charge_status = %d\n",
		 correct_capacity, di->charge_full_count, di->charge_status);

	di->capacity = correct_capacity;
	di->prev_capacity = correct_capacity;
	return 1;
}

static int judge_charge_status(struct bci_device_info *di,
	int *curr_capacity)
{
	int battery_volt;

	switch (di->charge_status) {
	case POWER_SUPPLY_STATUS_CHARGING:
		*curr_capacity = force_full_timer(*curr_capacity, di);
		break;
	case POWER_SUPPLY_STATUS_FULL:
		if (coul_drv_battery_current_avg() >= 0) {
#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
			/* if multi battery, get MAX voltage */
			battery_volt = hw_battery_voltage(BAT_ID_MAX);
#else
			battery_volt = coul_drv_battery_voltage();
#endif
			/* if multi battery, get MAX voltage */
			if (battery_volt >=
				(di->bat_max_volt - RECHG_PROTECT_THRESHOLD)) {
				*curr_capacity = CAPACITY_FULL;
				bci_info("Force soc=100\n");
			}
		}
		di->charge_full_count = 0;
		break;
	case POWER_SUPPLY_STATUS_DISCHARGING:
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		/* capacity-count will always decrease when discharging or notcharging */
		if ((int)di->prev_capacity <= *curr_capacity)
			return 0;
		di->charge_full_count = 0;
		break;
	default:
		bci_err("%s defualt run\n", __func__);
		break;
	}
	return 1;
}

static int capacity_changed(struct bci_device_info *di)
{
	int curr_capacity, cap, ret;

	di->bat_exist = coul_drv_is_battery_exist();

	/*
	 * if battery is not present we assume it is on battery simulator
	 * if we are in factory mode, BAT FW is not updated yet, we use volt2Capacity
	 */
	if ((!di->bat_exist) || (is_fake_battery) ||
		(strstr(saved_command_line, "androidboot.swtype=factory") &&
			(coul_drv_coulometer_type() == COUL_BQ27510))) {
		curr_capacity = calc_capacity_from_voltage();
	} else {
		cap = bci_capacity();
		if (di->charge_status == POWER_SUPPLY_STATUS_CHARGING)
			curr_capacity = DIV_ROUND_CLOSEST((cap * PERCENT),
				di->bci_soc_at_term); /* multiplier */
		else
			curr_capacity = cap;
		if (curr_capacity > CAPACITY_FULL)
			curr_capacity = CAPACITY_FULL;
	}

	ret = check_curr_capacity(curr_capacity, di);
	if (ret != -1)
		return ret;

	if (judge_charge_status(di, &curr_capacity) == 0)
		return 0;

	ret = capacity_correct_check(curr_capacity, di);
	return ret;
}

static void update_charging_status(
	struct bci_device_info *di, unsigned long event)
{
	if ((di->usb_online || di->ac_online) &&
		(di->capacity == CAPACITY_FULL) &&
		(di->charge_status != POWER_SUPPLY_STATUS_NOT_CHARGING))
		di->charge_status = POWER_SUPPLY_STATUS_FULL;

	if (event == VCHRG_CHARGE_DONE_EVENT)
		di->chargedone_stat = 1;
	else
		di->chargedone_stat = 0;
}

static void judge_charging_event(struct bci_device_info *di,
	unsigned long event)
{
	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
		di->usb_online = 1;
		di->ac_online = 0;
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
		if (g_sysfs_bq_bci)
			sysfs_notify(g_sysfs_bq_bci,
				NULL, "poll_charge_start_event");
		break;

	case VCHRG_START_AC_CHARGING_EVENT:
		di->ac_online = 1;
		di->usb_online = 0;
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
		if (g_sysfs_bq_bci)
			sysfs_notify(g_sysfs_bq_bci,
				NULL, "poll_charge_start_event");
		break;

	case VCHRG_STOP_CHARGING_EVENT:
		di->usb_online = 0;
		di->ac_online = 0;
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_UNKNOWN;
		di->charge_full_count = 0;
		if (g_sysfs_bq_bci)
			sysfs_notify(g_sysfs_bq_bci,
				NULL, "poll_charge_start_event");
		break;

	case VCHRG_START_CHARGING_EVENT:
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
		break;

	case VCHRG_NOT_CHARGING_EVENT:
		di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case VCHRG_CHARGE_DONE_EVENT:
		/*
		 * Do not need charge status change to full when bq24192 chargedone.
		 * because bq27510 will insure the charge status to full when capacity is 100.
		 */
		break;
	default:
		bci_err("%s defualt run\n", __func__);
		break;
	}
}

static int bci_charger_event(struct notifier_block *nb,
	unsigned long event, void *priv_data)
{
	struct bci_device_info *di = NULL;
	int low_bat_flag;

	di = container_of(nb, struct bci_device_info, nb);
	di->event = event;
	switch (event) {
	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		break;

	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		di->power_supply_status = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	case BATTERY_LOW_WARNING:
		break;
	case BATTERY_LOW_SHUTDOWN:
		low_bat_flag = coul_drv_is_battery_reach_threshold();
		if (low_bat_flag == BQ_FLAG_LOCK) {
			__pm_stay_awake(&g_low_power_lock);
			g_is_low_power_locked = 1;
		}
		mod_delayed_work(system_wq,
			&di->bci_monitor_work, msecs_to_jiffies(0));
		break;
	case VCHRG_STATE_WDT_TIMEOUT:
		di->watchdog_timer_status =
			POWER_SUPPLY_HEALTH_WATCHDOG_TIMER_EXPIRE;
		break;
	case BATTERY_MOVE:
		di->bat_exist = coul_drv_is_battery_exist();
		break;
	case WIRELESS_TX_STATUS_CHANGED:
		break;
	case WIRELESS_COVER_DETECTED:
		break;
	default:
		judge_charging_event(di, event);
		break;
	}

	update_charging_status(di, event);

	if (event != VCHRG_START_CHARGING_EVENT)
		bci_info("received event = %lx, charge_status = %d\n",
			event, di->charge_status);
	power_supply_changed(di->bat);
	return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-larger-than="
/* for dmd */
#if defined(CONFIG_HUAWEI_DSM)
static int check_batt_not_exist(char *buf)
{
#ifndef CONFIG_HLTHERM_RUNTEST
	if (!coul_drv_is_battery_exist())
		return 1;
#endif
	return 0;
}

static int check_batt_temp_overlow(char *buf)
{
	int batt_temp = bci_temp();
#ifndef CONFIG_HLTHERM_RUNTEST
	if (batt_temp < BATT_TEMP_OVERLOW_TH)
		return 1;
#endif
	return 0;
}

static int check_batt_volt_overhigh(char *buf)
{
	int volt, check_cnt, avg_volt, ret;

	volt = coul_drv_battery_voltage();
	if (volt > BATT_VOLT_OVERHIGH_TH) {
		for (check_cnt = 0;
			check_cnt < MAX_CONFIRM_CNT - 1; check_cnt++) {
			msleep(CONFIRM_INTERVAL);
			volt += coul_drv_battery_voltage();
		}
		avg_volt = (int)(volt / (MAX_CONFIRM_CNT));
		if (avg_volt > BATT_VOLT_OVERHIGH_TH) {
			ret = snprintf_s(buf,
				DSM_BATTERY_MAX_SIZE, DSM_BATTERY_MAX_SIZE - 1,
				"avg_batt_volt = %dmV\n", avg_volt);
			if (ret < 0) {
				bci_err("[%s]snprintf_s fail, err=%d\n",
					__func__, ret);
				return ret;
			}
			return 1;
		}
	}
	return 0;
}

static int check_batt_terminate_too_early(char *buf)
{
	struct bci_device_info *di = g_bci_dev;
	int capacity = bci_capacity();

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -1;
	}
	if (di->chargedone_stat && capacity <= CHG_CANT_FULL_THRESHOLD)
		return 1;

	return 0;
}

static int check_batt_not_terminate(char *buf)
{
	int curr_by_coultype = 1;
	int current_avg;
	int current_now;
	static int times;
	int dpm_state = 0;
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -1;
	}
	if (coul_drv_coulometer_type() == KCOUL)
		curr_by_coultype = -1;
#ifdef CONFIG_HUAWEI_CHARGER_AP
	dpm_state = charge_check_input_dpm_state();
#endif
	if ((di->charge_status == POWER_SUPPLY_STATUS_FULL) && !dpm_state) {
		current_avg = coul_drv_battery_current_avg();
		current_now = curr_by_coultype * coul_drv_battery_current();
		if ((current_avg <= BATT_NOT_TERMINATE_TH) &&
			(current_avg >= CURRENT_THRESHOLD) &&
			(current_now <= BATT_NOT_TERMINATE_TH) &&
			(current_now >= CURRENT_THRESHOLD))
			times++;
		else
			times = 0;
		if (times == DSM_CHECK_TIMES_LV3) {
			times = 0;
			snprintf(buf, DSM_BATTERY_MAX_SIZE,
				"current_avg = %dmA, current_now = %dmA, "
				"charge_event = %ld\n", current_avg,
				current_now, di->event);
			return 1;
		}
	}
	return 0;
}

static int check_batt_bad_curr_sensor(char *buf)
{
	static int times;
	int current_now;
	struct bci_device_info *di = g_bci_dev;
	int plugged = 0;
	int ret;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -1;
	}
	current_now = -coul_drv_battery_current();
#ifdef CONFIG_HUAWEI_CHARGER_AP
	plugged = charge_check_charger_plugged();
#endif
	if ((!di->usb_online) && (!di->ac_online) && !plugged &&
		(current_now > CURRENT_OFFSET))
		times++;
	else
		times = 0;

	if (times == DSM_CHECK_TIMES_LV2) {
		times = 0;
		ret = snprintf_s(buf,
			DSM_BATTERY_MAX_SIZE, DSM_BATTERY_MAX_SIZE - 1,
			"current = %dmA, charge_status = %d\n",
			current_now, di->charge_status);
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
	return 0;
}

static int check_vbus_volt_overhigh(char *buf)
{
	static int times;
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -1;
	}
	if (di->power_supply_status == POWER_SUPPLY_HEALTH_OVERVOLTAGE)
		times++;
	else
		times = 0;

	if (times == DSM_CHECK_TIMES_LV2) {
		times = 0;
		return 1;
	}
	return 0;
}

static int check_watchdog_timer_expiration(char *buf)
{
	int ret;
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -1;
	}
	if (di->watchdog_timer_status ==
		POWER_SUPPLY_HEALTH_WATCHDOG_TIMER_EXPIRE) {
		ret = snprintf_s(buf, DSM_BATTERY_MAX_SIZE,
			DSM_BATTERY_MAX_SIZE - 1,
			"WDT_status=%s\n", "watch dog timer expiration");
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
	return 0;
}

static int check_charge_curr_overhigh(char *buf)
{
	int i;
	int current_now[DSM_CHECK_TIMES_LV1];
	int ret;

	for (i = 0; i < DSM_CHECK_TIMES_LV1; i++) {
		current_now[i] = -coul_drv_battery_current();
		if (current_now[i] > g_charge_current_overhigh_th)
			msleep(SLEEP_TIME_MS); /* ms */
		else
			break;
	}
	if (i == DSM_CHECK_TIMES_LV1) {
		ret = snprintf_s(buf, DSM_BATTERY_MAX_SIZE,
			DSM_BATTERY_MAX_SIZE - 1, "current = %d, %d, %d\n",
			current_now[0], current_now[1], current_now[2]); /* 0, 1, 2 current index */
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
	return 0;
}

static int check_discharge_curr_overhigh(char *buf)
{
	int i;
	int current_now[DSM_CHECK_TIMES_LV1];
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -1;
	}
	for (i = 0; i < DSM_CHECK_TIMES_LV1; i++) {
		current_now[i] = -coul_drv_battery_current();
		if ((!di->usb_online) && (!di->ac_online) &&
			current_now[i] < DISCHARGE_CURRENT_OVERHIGH_TH) {
			msleep(SLEEP_TIME_MS);
		} else {
			break;
		}
	}

	if (i == DSM_CHECK_TIMES_LV1) {
		snprintf(buf, DSM_BATTERY_MAX_SIZE,
			"current = %d, %d, %d\n",
			current_now[0], current_now[1], current_now[2]); /* 0, 1, 2 current index */
		return 1;
	}
	return 0;
}

/*
 * During continuous suspend and resume in 30 seconds, and suspend current < 10ma test,
 * g_curr_zero_times will not exceeding 1 ,it will be cleared when resume
 */
static int g_curr_zero_times;
static int check_charge_curr_zero(char *buf)
{
	int plugged = 0;
	int current_now;

	current_now = -coul_drv_battery_current();
#ifdef CONFIG_HUAWEI_CHARGER_AP
	plugged = charge_check_charger_plugged();
#endif
	if ((current_now < CURRENT_THRESHOLD) &&
		(current_now > (-CURRENT_THRESHOLD)) && !plugged)
		g_curr_zero_times++;
	else
		g_curr_zero_times = 0;

	if (g_curr_zero_times == DSM_CHECK_TIMES_LV2) {
		g_curr_zero_times = 0;
		return 1;
	}
	return 0;
}

#define HIGH_TEMP 50
#define LOW_TEMP  0
static int check_charge_temp_fault(char *buf)
{
#ifndef CONFIG_HLTHERM_RUNTEST
	struct bci_device_info *di = g_bci_dev;
	int batt_temp;
	int ret;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return 0;
	}
	batt_temp = bci_temp();
	if ((batt_temp >= HIGH_TEMP || batt_temp < LOW_TEMP) &&
		(di->usb_online || di->ac_online)) {
		ret = snprintf_s(buf,
			DSM_BATTERY_MAX_SIZE, DSM_BATTERY_MAX_SIZE - 1,
			"%s", "temp fault cause not charging\n");
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
#endif
	return 0;
}

#define WARM_TEMP 45
static int check_charge_warm_status(char *buf)
{
#ifndef CONFIG_HLTHERM_RUNTEST
	struct bci_device_info *di = g_bci_dev;
	int temp = bci_temp();
	int ret;

	if (temp >= WARM_TEMP && temp < HIGH_TEMP &&
		(di->usb_online || di->ac_online)) {
		ret = snprintf_s(buf,
			DSM_BATTERY_MAX_SIZE, DSM_BATTERY_MAX_SIZE - 1,
			"warm temp triggered: temp = %d\n", temp);
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
#endif
	return 0;
}

#define SHUTDOWN_TEMP 68
static int check_charge_batt_shutdown_temp(char *buf)
{
#ifndef CONFIG_HLTHERM_RUNTEST
	int temp = bci_temp();
	int ret;

	if (temp >= SHUTDOWN_TEMP) {
		ret = snprintf_s(buf, DSM_BATTERY_MAX_SIZE,
			DSM_BATTERY_MAX_SIZE - 1,
			"shutdown because batt_temp = %d\n", temp);
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
#endif
	return 0;
}

static int check_charge_batt_capacity(char *buf)
{
	struct bci_device_info *di = g_bci_dev;
	int full_rm = di->bat_rm;
	int fcc_design = coul_drv_battery_fcc_design();
	int batt_temp = bci_temp();
	int ret;

	if ((di->event == VCHRG_CHARGE_DONE_EVENT) &&
		(full_rm < fcc_design * BATT_CAPACITY_REDUCE_TH / CAPACITY_FULL) &&
		(batt_temp > LOW_TEMP)) {
		ret = snprintf_s(buf, DSM_BATTERY_MAX_SIZE,
			DSM_BATTERY_MAX_SIZE - 1,
			"full_rm = %dmAh, fcc_design = %dmAh\n",
			di->bat_rm, fcc_design);
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
	return 0;
}

#ifdef CONFIG_HUAWEI_CHARGER_AP
static int check_charge_charger_ts(char *buf)
{
	struct bci_device_info *di = g_bci_dev;
	int ret;

	if (charge_check_charger_ts()) {
		ret = snprintf_s(buf, DSM_BATTERY_MAX_SIZE,
			DSM_BATTERY_MAX_SIZE - 1,
			"charger ts error!charge status = %d\n",
			di->charge_status);
		if (ret < 0) {
			bci_err("[%s]snprintf_s fail, err=%d\n", __func__, ret);
			return ret;
		}
		return 1;
	}
	return 0;
}
#else
static int check_charge_charger_ts(char *buf)
{
	return 0;
}
#endif

static int check_charge_otg_ture(char *buf)
{
#ifdef CONFIG_HUAWEI_TYPEC
	int typec_mode;
	int ret;
	bool otg_state = false;
#ifdef CONFIG_HUAWEI_CHARGER_AP
	otg_state = charge_check_charger_otg_state();
#endif
	if (otg_state && (chip_get_charger_type() == PLEASE_PROVIDE_POWER)) {
		typec_mode = typec_detect_port_mode();
		if (typec_mode < 0) {
			bci_err("typec not ready %d\n", typec_mode);
			return 0;
		}
		if (typec_mode != TYPEC_DEV_PORT_MODE_DFP &&
			typec_mode != TYPEC_DEV_PORT_MODE_NOT_READY) {
			ret = snprintf_s(buf, DSM_BATTERY_MAX_SIZE,
				DSM_BATTERY_MAX_SIZE - 1,
				"charger in otg mode!typec mode = %d\n",
				typec_mode);
			if (ret < 0) {
				bci_err("[%s]snprintf_s fail, err=%d\n",
					__func__, ret);
				return ret;
			}
			return 1;
		}
	}
#endif
	return 0;
}

/* monitor the varity of SOC */
#define SOC_MONITOR_INTERVAL 60000 /* 1 min */
static int check_soc_vary_err(char *buf)
{
	int report_flag = 0;
#ifndef CONFIG_HLTHERM_RUNTEST
	static int data_invalid;
	static int monitor_cnt;
	int deta_soc = 0;
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}

	data_invalid |= coul_drv_battery_soc_vary_flag(0, &deta_soc);
	if (monitor_cnt %
		(SOC_MONITOR_INTERVAL / di->monitoring_interval) == 0) {
		data_invalid |= coul_drv_battery_soc_vary_flag(1, &deta_soc);
		if (data_invalid) {
			report_flag = 0;
		} else {
			snprintf(buf, DSM_BATTERY_MAX_SIZE,
				"soc change fast: %d\n", deta_soc);
			report_flag = 1;
		}
		monitor_cnt = 0; /* reset the counter */
		data_invalid = 0;
	}
	monitor_cnt++;
#endif
	return report_flag;
}

void batt_info_dump(char *pstr)
{
	char buf[CHARGE_DMDLOG_SIZE];
	unsigned int charger_type = 0;
	char *batt_brand = coul_drv_battery_brand();
	int info_dump[MAX] = {0};

#ifdef CONFIG_HUAWEI_CHARGER_AP
	charger_type = charge_get_charger_type();
#endif
	info_dump[BATT_ID] = coul_drv_battery_id_voltage();
	info_dump[BATT_CYCLE] = coul_drv_battery_cycle_count();
	info_dump[FCC_DESIGN] = coul_drv_battery_fcc_design();
	info_dump[FCC] = coul_drv_battery_fcc();
	info_dump[RM] = coul_drv_battery_rm();
	info_dump[SOC] = bci_capacity();

	info_dump[VBATT] = coul_drv_battery_voltage();
#ifdef CONFIG_HUAWEI_CHARGER_AP
	info_dump[VBUS] = get_charger_vbus_vol();

	info_dump[IBUS] = get_charger_ibus_curr();
#endif
	info_dump[ICHG] = -coul_drv_battery_current();
	info_dump[CURR_NOW] = -coul_drv_battery_current();
	info_dump[AVG_CURR] = coul_drv_battery_current_avg();

	info_dump[TBATT] = bci_temp();
	info_dump[TUSB] = power_temp_get_average_value(
		POWER_TEMP_USB_PORT) / POWER_TEMP_UNIT;

	if (!pstr) {
		bci_err("%s: para null\n", __func__);
		return;
	}
	memset(buf, 0, CHARGE_DMDLOG_SIZE);

	snprintf(buf, sizeof(buf) - 1,
		"battery: %s, batt_id = %d, batt_cycle = %d, fcc_design = %dmAh, fcc = %dmAh, rm = %dmAh, soc = %d, "
		"batt_volt = %dmV, charger_vbus = %dmV, charger_ibus = %dmA, ichrg = %dmA, curr_now = %dmA, avg_curr = %dmA, "
		"tbatt = %d, tusb = %d, charger_type = %u\n",
		batt_brand, info_dump[BATT_ID], info_dump[BATT_CYCLE],
		info_dump[FCC_DESIGN], info_dump[FCC], info_dump[RM],
		info_dump[SOC], info_dump[VBATT], info_dump[VBUS],
		info_dump[IBUS], info_dump[ICHG], info_dump[CURR_NOW],
		info_dump[AVG_CURR], info_dump[TBATT], info_dump[TUSB],
		charger_type);
	strncat(pstr, buf, strlen(buf));
}

struct batt_dsm batt_dsm_array[] = {
	{ERROR_BATT_NOT_EXIST, true,
		.dump = batt_info_dump, check_batt_not_exist},
	{ERROR_BATT_TEMP_LOW, true,
		.dump = batt_info_dump, check_batt_temp_overlow},
	{ERROR_BATT_VOLT_HIGH, true,
		.dump = batt_info_dump, check_batt_volt_overhigh},
	{ERROR_BATT_TERMINATE_TOO_EARLY, true,
		.dump = batt_info_dump, check_batt_terminate_too_early},
	{ERROR_BATT_NOT_TERMINATE, true,
		.dump = batt_info_dump, check_batt_not_terminate},
	{ERROR_BATT_BAD_CURR_SENSOR, true,
		.dump = batt_info_dump, check_batt_bad_curr_sensor},
	{ERROR_VBUS_VOLT_HIGH, true,
		.dump = batt_info_dump, check_vbus_volt_overhigh},
	{ERROR_WATCHDOG_RESET, true,
		.dump = batt_info_dump, check_watchdog_timer_expiration},
	{ERROR_CHARGE_CURR_OVERHIGH, true,
		.dump = batt_info_dump, check_charge_curr_overhigh},
	{ERROR_DISCHARGE_CURR_OVERHIGH, true,
		.dump = batt_info_dump, check_discharge_curr_overhigh},
	{ERROR_CHARGE_CURR_ZERO, true,
		.dump = batt_info_dump, check_charge_curr_zero},
	{ERROR_CHARGE_TEMP_FAULT, true,
		.dump = batt_info_dump, check_charge_temp_fault},
	{ERROR_CHARGE_BATT_TEMP_SHUTDOWN, true,
		.dump = batt_info_dump, check_charge_batt_shutdown_temp},
	{ERROR_CHARGE_BATT_CAPACITY, true,
		.dump = batt_info_dump, check_charge_batt_capacity},
	{ERROR_CHARGE_CHARGER_TS, true,
		.dump = batt_info_dump, check_charge_charger_ts},
	{ERROR_CHARGE_OTG, true,
		.dump = batt_info_dump, check_charge_otg_ture},
	{ERROR_BATT_SOC_CHANGE_FAST, true,
		.dump = batt_info_dump, check_soc_vary_err},
	{ERROR_CHARGE_TEMP_WARM, true,
		.dump = batt_info_dump, check_charge_warm_status},
};

static int check_error_info(struct bci_device_info *di,
	char *buf, int i)
{
	if (batt_dsm_array[i].check_error(buf)) {
		di->bat_err = batt_dsm_array[i].error_no;
#if defined(CONFIG_HUAWEI_DSM)
		if (!dsm_client_ocuppy(
			power_dsm_get_dclient(POWER_DSM_BATTERY))) {
			buf[DSM_BATTERY_MAX_SIZE - 1] = 0;
			dev_err(di->dev, "%s", buf);
			if (batt_dsm_array[i].dump)
				batt_dsm_array[i].dump(buf);
			dsm_client_record(
				power_dsm_get_dclient(POWER_DSM_BATTERY),
				"%s", buf);
			dsm_client_notify(
				power_dsm_get_dclient(POWER_DSM_BATTERY),
				di->bat_err);
			batt_dsm_array[i].notify_enable = false;
			return 1;
		}
#else
		buf[DSM_BATTERY_MAX_SIZE - 1] = 0;
		bci_err("error_no = %u!!!\n", di->bat_err);
		bci_err("error_buf = %s\n", buf);
		return 1;
#endif
	}
	return 0;
}

static void get_error_info(struct bci_device_info *di)
{
	int i;
	char buf[DSM_BATTERY_MAX_SIZE];

	memset(buf, 0, sizeof(buf));
	for (i = 0; i < (int)(sizeof(batt_dsm_array) / sizeof(struct batt_dsm));
		++i) {
		if (batt_dsm_array[i].notify_enable &&
			(g_is_board_type == BAT_BOARD_ASIC)) {
			if (check_error_info(di, buf, i))
				break;
		}
	}
}
#endif
#pragma GCC diagnostic pop

static void get_battery_info(struct bci_device_info *di)
{
	di->bat_rm = coul_drv_battery_rm();
	di->bat_fcc = coul_drv_battery_fcc();

	if (!(di->bat_exist)) {
		di->bat_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
		di->bat_technolog = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
		di->bat_temperature = 0;
	} else {
		di->bat_health = bci_health();
		di->bat_capacity_level = coul_drv_battery_capacity_level();
		di->bat_technolog = coul_drv_battery_technology();
		di->bat_temperature = bci_temp();
	}
}

unsigned int get_bci_soc(void)
{
	/* if bci driver load uncorrectly, full soc will be returned */
	return g_bci_dev ? g_bci_dev->capacity : CAPACITY_FULL;
}

static void bci_battery_work(struct work_struct *work)
{
	struct bci_device_info *di =
		container_of(work, struct bci_device_info,
		bci_monitor_work.work);

	get_battery_info(di);
#if defined CONFIG_HUAWEI_DSM
	get_error_info(di);
#endif
	queue_delayed_work(system_power_efficient_wq,
		&di->bci_monitor_work,
		msecs_to_jiffies(di->monitoring_interval));

	if (capacity_changed(di))
		power_supply_changed(di->bat);
}

static int ac_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
#ifdef CONFIG_WIRELESS_CHARGER
		if (di->ac_online && charge_get_charger_type() != CHARGER_TYPE_WIRELESS)
#else
		if (di->ac_online)
#endif
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = di->power_supply_status;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_WIRELESS_CHARGER
static int wireless_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS)
			val->intval = 1;
		else
			val->intval = 0;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
#endif

#define MILLI_TO_MICRO 1000
static int usb_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->usb_online;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = di->power_supply_status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = 0;
#ifdef CONFIG_HUAWEI_CHARGER
		val->intval = get_charger_vbus_vol() * MILLI_TO_MICRO;
#endif
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = 0;
#ifdef CONFIG_HUAWEI_CHARGER
		val->intval = get_charge_current_max() * MILLI_TO_MICRO;
#endif
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int bk_bci_battery_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* Use gpadc channel for measuring bk battery voltage */
		val->intval = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int get_battery_current(void)
{
	int bat_current;

	if (strstr(saved_command_line, "androidboot.swtype=factory"))
		bat_current = -coul_drv_battery_current();
	else
		bat_current = -coul_drv_fifo_avg_current();

	return bat_current;
}

static int get_property_extra(enum power_supply_property psp,
	union power_supply_propval *val, struct bci_device_info *di)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CAPACITY_RM:
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = di->bat_rm;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_FCC:
	case POWER_SUPPLY_PROP_CHARGE_FULL:
#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
		val->intval = di->bat_fcc * hw_battery_get_series_num();
#else
		val->intval = di->bat_fcc;
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = di->bat_design_fcc;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = di->bat_max_volt;
		break;
	case POWER_SUPPLY_PROP_ID_VOLTAGE:
		val->intval = coul_drv_battery_id_voltage();
		break;
	case POWER_SUPPLY_PROP_BRAND:
		val->strval = coul_drv_battery_brand();
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = coul_drv_battery_cycle_count();
		break;
	case POWER_SUPPLY_PROP_LIMIT_FCC:
		val->intval = coul_drv_battery_get_limit_fcc();
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
		val->intval = coul_drv_battery_cc();
		val->intval = abs(val->intval) + 1;
		val->intval *= PERMILLAGE; /* 1000 is the unit of mah to uah */
		break;
	case POWER_SUPPLY_PROP_BAT_QMAX:
		val->intval = coul_drv_battery_get_qmax();
		break;
	case POWER_SUPPLY_PROP_CHARGE_TIME_REMAINING:
#ifdef CONFIG_CHARGE_TIME
		val->intval = hisi_chg_time_remaining(1);
#else
		val->intval = -1;
#endif
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int bci_battery_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = di->charge_status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		di->bat_voltage = coul_drv_battery_voltage();
		if (coul_drv_coulometer_type() == KCOUL)
			val->intval = coul_drv_battery_voltage_uv();
		else
			val->intval = di->bat_voltage * PERMILLAGE;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		di->bat_current = get_battery_current();
		val->intval = di->bat_current;
		break;
	case POWER_SUPPLY_PROP_TEMP:
#ifdef CONFIG_HLTHERM_RUNTEST
		val->intval = DEFAULT_TEMP_TENTH;
#else
		val->intval = di->bat_temperature * TENTH;
#endif
		break;
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->bat_exist;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = di->bat_health;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = di->capacity;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = di->bat_capacity_level;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = di->bat_technolog;
		break;
	default:
		return get_property_extra(psp, val, di);
	}
	return 0;
}

int bci_capacity_get_filter_sum(int base)
{
	return (g_capacity_sum * base) / WINDOW_LEN;
}

void bci_capacity_sync_filter(int rep_soc, int round_soc, int base)
{
	struct bci_device_info *di = g_bci_dev;
	int new_soc;
	static int prev_soc;

	if (!di) {
		bci_err("di is null\n");
		return;
	}

	if (base == 0) {
		bci_err("base is 0\n");
		return;
	}

	/* step1: reset capacity fifo */
	reset_capacity_fifo(round_soc);
	new_soc = (rep_soc * WINDOW_LEN / base) - round_soc * (WINDOW_LEN - 1);
	capacity_pulling_filter(new_soc, di);

	/* step2: capacity changed (example: 86% to 87%) */
	if (prev_soc != round_soc) {
		di->capacity = round_soc;
		di->prev_capacity = round_soc;
		power_supply_changed(di->bat);
	}
	prev_soc = round_soc;
}

int bci_show_capacity(void)
{
	struct bci_device_info *di = g_bci_dev;

	if (!di) {
		bci_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}

	if (di->capacity > CAPACITY_FULL) {
		bci_err("error capacity, will rewrite from %d to 100\n",
			di->capacity);
		di->capacity = CAPACITY_FULL;
	}
	return di->capacity;
}

int bci_get_charge_current_overhigh_th(void)
{
	return g_charge_current_overhigh_th;
}

int bci_register_notifier(struct notifier_block *nb, unsigned int events)
{
	return blocking_notifier_chain_register(&g_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(bci_register_notifier);

int bci_unregister_notifier(struct notifier_block *nb, unsigned int events)
{
	return blocking_notifier_chain_unregister(&g_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(bci_unregister_notifier);

static ssize_t bci_show_batt_removable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf_s(buf, PAGE_SIZE, "%d\n", g_removable_batt_flag);
}

static ssize_t bci_poll_charge_start_event(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bci_device_info *di = dev_get_drvdata(dev);

	if (di)
		return sprintf_s(buf, PAGE_SIZE, "%ld\n", di->event);
	else
		return 0;
}

static ssize_t bci_set_charge_event(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bci_device_info *di = dev_get_drvdata(dev);
	long val = 0;

	if ((kstrtol(buf, DEC, &val) < 0) || (val < 0) || (val > 3000)) /* mv */
		return -EINVAL;
	di->event = val;
	sysfs_notify(g_sysfs_bq_bci, NULL, "poll_charge_start_event");
	return count;
}

static DEVICE_ATTR(batt_removable, (S_IWUSR | S_IRUGO),
	bci_show_batt_removable, NULL);

static DEVICE_ATTR(poll_charge_start_event, (S_IWUSR | S_IRUGO),
	bci_poll_charge_start_event, bci_set_charge_event);

static struct attribute *bci_attributes[] = {
	&dev_attr_batt_removable.attr,
	&dev_attr_poll_charge_start_event.attr,
	NULL,
};

static const struct attribute_group bci_attr_group = {
	.attrs = bci_attributes,
};

static char *bci_supplied_to[] = {
	"hisi_bci_battery",
};

static const struct power_supply_config bci_battery_cfg = {
	.supplied_to = bci_supplied_to,
	.num_supplicants = ARRAY_SIZE(bci_supplied_to),
};

static const struct power_supply_desc bci_battery_desc = {
	.name = "Battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = bci_battery_props,
	.num_properties = ARRAY_SIZE(bci_battery_props),
	.get_property = bci_battery_get_property,
};

static const struct power_supply_desc bci_usb_desc = {
	.name = "USB",
	.type = POWER_SUPPLY_TYPE_USB,
	.properties = usb_props,
	.num_properties = ARRAY_SIZE(usb_props),
	.get_property = usb_get_property,
};

static const struct power_supply_desc bci_mains_desc = {
	.name = "Mains",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = ac_props,
	.num_properties = ARRAY_SIZE(ac_props),
	.get_property = ac_get_property,
};

#ifdef CONFIG_WIRELESS_CHARGER
static const struct power_supply_desc bci_wireless_desc = {
	.name = "Wireless",
	.type = POWER_SUPPLY_TYPE_WIRELESS,
	.properties = wireless_props,
	.num_properties = ARRAY_SIZE(wireless_props),
	.get_property = wireless_get_property,
};
#endif

static const struct power_supply_desc bci_bk_battery_desc = {
	.name = "hisi_bk_battery",
	.type = POWER_SUPPLY_TYPE_UPS,
	.properties = bk_bci_battery_props,
	.num_properties = ARRAY_SIZE(bk_bci_battery_props),
	.get_property = bk_bci_battery_get_property,
};

static const struct power_supply_desc bci_bat_google_desc = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_UNKNOWN,
	.properties = bci_battery_props,
	.num_properties = ARRAY_SIZE(bci_battery_props),
	.get_property = bci_battery_get_property,
};

static int bci_supply_register_extra(
	struct platform_device *pdev, struct bci_device_info *di)
{
#ifdef CONFIG_WIRELESS_CHARGER
	di->wireless = power_supply_register(&pdev->dev,
		&bci_wireless_desc, NULL);
	if (IS_ERR(di->wireless)) {
		bci_debug("failed to register wireless power supply\n");
		goto wireless_failed;
	}
#endif
	di->bk_bat = power_supply_register(&pdev->dev,
		&bci_bk_battery_desc, NULL);
	if (IS_ERR(di->bk_bat)) {
		bci_debug("failed to register backup battery\n");
		goto bk_batt_failed;
	}

	if (g_google_battery_node) {
		di->bat_google = power_supply_register(
		  &pdev->dev, &bci_bat_google_desc, &bci_battery_cfg);
		if (IS_ERR(di->bat_google)) {
			bci_debug("failed to register google battery\n");
			goto bat_google_failed;
		}
	}
	return 0;
bat_google_failed:
	cancel_delayed_work(&di->bci_monitor_work);
	power_supply_unregister(di->bk_bat);
bk_batt_failed:
	if (!g_google_battery_node)
		cancel_delayed_work(&di->bci_monitor_work);
#ifdef CONFIG_WIRELESS_CHARGER
	power_supply_unregister(di->wireless);
wireless_failed:
#endif
	return -1;
}

static int bci_supply_register(
	struct platform_device *pdev, struct bci_device_info *di)
{
	int ret;

	di->bat = power_supply_register(&pdev->dev,
		&bci_battery_desc, &bci_battery_cfg);
	if (IS_ERR(di->bat)) {
		bci_debug("failed to register main battery\n");
		goto batt_failed;
	}
	di->usb = power_supply_register(&pdev->dev,
		&bci_usb_desc, NULL);
	if (IS_ERR(di->usb)) {
		bci_debug("failed to register usb power supply\n");
		goto usb_failed;
	}

	di->ac = power_supply_register(&pdev->dev,
		&bci_mains_desc, NULL);
	if (IS_ERR(di->ac)) {
		bci_debug("failed to register ac power supply\n");
		goto ac_failed;
	}

	ret = bci_supply_register_extra(pdev, di);
	if (ret)
		goto supply_extra_failed;

	return 0;
supply_extra_failed:
	power_supply_unregister(di->ac);
ac_failed:
	power_supply_unregister(di->usb);
usb_failed:
	power_supply_unregister(di->bat);
batt_failed:
	return -1;
}

static void capacity_info_init(struct bci_device_info *di)
{
	int i;

	di->capacity = -1;
	di->capacity_filter_count = 0;
	di->charge_full_count = 0;
	di->bat_max_volt = coul_drv_battery_vbat_max();
	di->monitoring_interval = WORK_INTERVAL_NOARMAL;
	di->bat_health = POWER_SUPPLY_HEALTH_GOOD;
	di->bat_exist = coul_drv_is_battery_exist();
	di->bat_err = 0;
	di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
	di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;

	for (i = 0; i < WINDOW_LEN; i++) {
		g_capacity_filter[i] = (unsigned int)bci_capacity();
		g_capacity_sum += g_capacity_filter[i];
		bci_info("capacity_filter[%d] = %u\n", i, g_capacity_filter[i]);
	}
}

static void bci_sysfs_creat(struct bci_device_info *di)
{
	int ret;
	struct device *new_dev = NULL;
	struct class *power_class = NULL;

	power_class = power_sysfs_get_class("hw_power");
	if (power_class) {
		new_dev = device_create(power_class, NULL, 0, NULL, "bq_bci");
		if (IS_ERR(new_dev))
			new_dev = NULL;

		if (new_dev) {
			dev_set_drvdata(new_dev, di);
			g_sysfs_bq_bci = &new_dev->kobj;
			ret = sysfs_create_group(&new_dev->kobj,
				&bci_attr_group);
			if (ret)
				bci_err("%s, could not create sysfs files bci!\n",
					__func__);
		} else {
			bci_err("%s, could not create dev bci!\n", __func__);
		}
	}
}

static int parse_vth_correct_dts(const struct device_node *np,
	struct bci_device_info *di)
{
	int i;

	if (of_property_read_u32(np,
		"vth_correct_en", (u32 *)&di->vth_correct_en)) {
		di->vth_correct_en = 0;
		bci_err("error:get vth_correct_en value failed, used default value!\n");
	}

	if (of_property_read_u32_array(np, "vth_correct_para",
			(u32 *)&di->vth_correct_data[0],
			sizeof(di->vth_correct_data) / sizeof(int))) {
		di->vth_correct_en = 0;
		bci_err("get vth_correct_para fail!!\n");
		return -EINVAL;
	}

	for (i = 0; i < CAP_LOCK_PARA_LEVEL; i++)
		bci_info("di->vth_correct_data[%d]:%d,%d\n", i,
			di->vth_correct_data[i].cap,
			di->vth_correct_data[i].level_vol);
	return 0;
}

/* parse the module dts config size of work interval */
static int parse_interval_size_dts(const struct device_node *np)
{
	int array_len;

	/* bci work interval para */
	array_len = of_property_count_strings(np, "bci_work_interval_para");
	if ((array_len <= 0) || (array_len % WORK_INTERVAL_PARA_TOTAL != 0)) {
		bci_err("bci_work_interval_para is invaild,please check iput number!!\n");
		return -EINVAL;
	}

	if (array_len > WORK_INTERVAL_PARA_LEVEL * WORK_INTERVAL_PARA_TOTAL) {
		bci_err("bci_work_interval_para is too long,use only front %d paras!!\n",
			array_len);
		return -EINVAL;
	}
	return array_len;
}

static void parse_work_inter_dts(const struct device_node *np,
	struct bci_device_info *di)
{
	int i, array_len, ret;
	long idata = 0;
	const char *bci_data_string = NULL;

	array_len = parse_interval_size_dts(np);
	if (array_len < 0) {
		bci_err("get bci_work_interval_para  size failed\n");
		return;
	}
	(void)memset_s(di->interval_data, INTERVAL_SIZE, 0, INTERVAL_SIZE);

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"bci_work_interval_para", i, &bci_data_string);
		if (ret) {
			bci_err("get bci_work_interval_para failed\n");
			return;
		}

		kstrtol(bci_data_string, DEC, &idata); /* length */
		switch (i % WORK_INTERVAL_PARA_TOTAL) {
		case WORK_INTERVAL_CAP_MIN:
			if ((idata < CAPACITY_MIN) || (idata > CAPACITY_MAX)) {
				bci_err("the bci_work_interval_para cap_min is out of range!!\n");
				return;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].cap_min = idata;
			break;
		case WORK_INTERVAL_CAP_MAX:
			if ((idata < CAPACITY_MIN) || (idata > CAPACITY_MAX)) {
				bci_err("the bci_work_interval_para cap_max is out of range!!\n");
				return;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].cap_max = idata;
			break;
		case WORK_INTERVAL_VALUE:
			if ((idata < WORK_INTERVAL_MIN) ||
					(idata > WORK_INTERVAL_MAX)) {
				bci_err("the bci_work_interval_para work_interval is out of range!!\n");
				return;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].work_interval = idata;
			break;
		default:
			bci_err("%s default run\n", __func__);
			break;
		}
		bci_info("di->interval_data[%d][%d] = %ld\n", i / (WORK_INTERVAL_PARA_TOTAL),
			 i % (WORK_INTERVAL_PARA_TOTAL), idata);
	}
}

/* parse the module dts config value, return value: 0-success or others-fail */
static int bci_parse_dts(const struct device_node *np,
	struct bci_device_info *di)
{
	int ret = 0;

	if (of_property_read_u32(np, "battery_design_fcc",
		(u32 *)&di->bat_design_fcc)) {
		di->bat_design_fcc = coul_drv_battery_fcc_design();
		bci_err("error:get battery_design_fcc value failed, used default value from batt_parm!\n");
	}

	if (of_property_read_u32(np, "disable_pre_vol_check",
		&di->disable_pre_vol_check))
		di->disable_pre_vol_check = 0;
	bci_info("disable_pre_vol_check:%d", di->disable_pre_vol_check);

	if (of_property_read_u32(np, "bci_soc_at_term",
		(u32 *)&di->bci_soc_at_term)) {
		di->bci_soc_at_term = SOC_FULL; /* full */
		bci_err("error:get bci_soc_at_term value failed, no early term in bci !\n");
	}
	bci_info("bci_soc_at_term =%d", di->bci_soc_at_term);

	if (of_property_read_u32(np, "battery_is_removable",
		(u32 *)&g_removable_batt_flag)) {
		g_removable_batt_flag = 0;
		bci_err("error:get removable_batt_flag value failed!\n");
	}

	if (of_property_read_u32(np, "battery_board_type",
		(u32 *)&g_is_board_type)) {
		g_is_board_type = BAT_BOARD_SFT;
		bci_err("error:get battery_board_type value failed!\n");
	}

	if (of_property_read_u32(np, "google_battery_node",
		(u32 *)&g_google_battery_node)) {
		g_google_battery_node = 0;
		bci_err("error:get google_battery_node value failed!\n");
	}

	if (of_property_read_u32(np, "chg_full_rpt_thr",
		(u32 *)&di->chg_full_rpt_thr))
		di->chg_full_rpt_thr = CHG_CANT_FULL_THRESHOLD;

	if (of_property_read_u32(np, "chg_full_wait_times",
		(u32 *)&di->chg_full_wait_times))
		di->chg_full_wait_times = CHARGE_FULL_TIME;

	if (of_property_read_u32(np, "charge_current_overhigh_th",
		(u32 *)&g_charge_current_overhigh_th))
		g_charge_current_overhigh_th = CHARGE_CURRENT_OVERHIGH_TH;

	bci_info("chg_full_rpt_thr = %u, chg_full_wait_times = %u, charge_current_overhigh_th = %d\n",
		di->chg_full_rpt_thr, di->chg_full_wait_times, g_charge_current_overhigh_th);

	if (of_property_read_u32(np, "sc_uisoc_update_speed", &di->sc_uisoc_update_speed))
		di->sc_uisoc_update_speed = WORK_INTERVAL_REACH_FULL;
	bci_info("sc_uisoc_update_speed:%u\n", di->sc_uisoc_update_speed);

	parse_work_inter_dts(np, di);
	ret |= parse_vth_correct_dts(np, di);
	return ret;
}

static int bci_battery_probe(struct platform_device *pdev)
{
	struct bci_device_info *di = NULL;
	int low_bat_flag;
	int ret;
	struct device_node *np = NULL;

	np = pdev->dev.of_node;
	if (!np)
		return -1;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	g_bci_dev = di;

	di->dev = &pdev->dev;

	capacity_info_init(di);
	get_battery_info(di);

	platform_set_drvdata(pdev, di);

	wakeup_source_init(&g_low_power_lock, "low_power_wake_lock");

	low_bat_flag = coul_drv_is_battery_reach_threshold();
	if (low_bat_flag == BQ_FLAG_LOCK) {
		__pm_stay_awake(&g_low_power_lock);
		g_is_low_power_locked = 1;
	}

	bci_parse_dts(np, di);
	ret = bci_supply_register(pdev, di);
	if (ret)
		goto supply_regist_failed;

	bci_sysfs_creat(di);

	INIT_DELAYED_WORK(&di->bci_monitor_work, bci_battery_work);
	queue_delayed_work(system_power_efficient_wq,
				&di->bci_monitor_work, 0);

	di->nb.notifier_call = bci_charger_event;
	bci_register_notifier(&di->nb, 1);
	bci_info("bci probe ok!\n");

	return 0;

supply_regist_failed:
	wakeup_source_trash(&g_low_power_lock);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	g_bci_dev = NULL;
	return ret;
}

static int bci_battery_remove(struct platform_device *pdev)
{
	struct bci_device_info *di = platform_get_drvdata(pdev);

	if (!di) {
		bci_err("di is NULL!\n");
		return -ENODEV;
	}

	bci_unregister_notifier(&di->nb, 1);
	cancel_delayed_work(&di->bci_monitor_work);
	wakeup_source_trash(&g_low_power_lock);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;

	return 0;
}

static void bci_battery_shutdown(struct platform_device *pdev)
{
	struct bci_device_info *di = platform_get_drvdata(pdev);

	bci_info("%s ++\n", __func__);
	if (!di) {
		bci_err("di is NULL!\n");
		return;
	}

	bci_unregister_notifier(&di->nb, 1);
	cancel_delayed_work(&di->bci_monitor_work);
	wakeup_source_trash(&g_low_power_lock);
	bci_info("%s --\n", __func__);
}

#ifdef CONFIG_PM
static int bci_battery_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct bci_device_info *di = platform_get_drvdata(pdev);

	if (!di) {
		bci_err("di is NULL!\n");
		return -ENODEV;
	}
	bci_info("%s:+\n", __func__);
	g_suspend_capacity = bci_capacity();
	cancel_delayed_work(&di->bci_monitor_work);
	bci_info("%s:-\n", __func__);
	return 0;
}

static int bci_battery_resume(struct platform_device *pdev)
{
	struct bci_device_info *di = platform_get_drvdata(pdev);
	int i;
	int resume_capacity;

	if (!di) {
		bci_err("di is NULL!\n");
		return -ENODEV;
	}

	bci_info("%s:+\n", __func__);
	g_capacity_debounce_flag = true;

	resume_capacity = bci_capacity();
	if ((di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) ||
		(di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING)) {
		if ((g_suspend_capacity - resume_capacity) >= DELTA_CAPACITY) { /* limit cap */
			g_capacity_sum = 0;
			for (i = 0; i < WINDOW_LEN; i++) {
				g_capacity_filter[i] = resume_capacity;
				g_capacity_sum += g_capacity_filter[i];
			}
		}
	}
#if defined CONFIG_HUAWEI_DSM
	g_curr_zero_times = 0;
#endif
	queue_delayed_work(system_power_efficient_wq,
		&di->bci_monitor_work, 0);
	bci_info("%s:-\n", __func__);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id bci_battery_match_table[] = {
	{
		.compatible = "hisilicon,bci_battery",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver bci_battery_driver = {
	.probe = bci_battery_probe,
	.remove = bci_battery_remove,
#ifdef CONFIG_PM
	.suspend = bci_battery_suspend,
	.resume = bci_battery_resume,
#endif
	.shutdown = bci_battery_shutdown,
	.driver = {
		.name = "bci_battery",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bci_battery_match_table),
	},
};

static int __init bci_battery_init(void)
{
	return platform_driver_register(&bci_battery_driver);
}

module_init(bci_battery_init);

static void __exit bci_battery_exit(void)
{
	platform_driver_unregister(&bci_battery_driver);
}

module_exit(bci_battery_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("battery_bci");

