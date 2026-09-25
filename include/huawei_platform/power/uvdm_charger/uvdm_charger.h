/*
 * uvdm_charger.h
 *
 * uvdm charger driver
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

#ifndef _UVDM_CHARGER_H_
#define _UVDM_CHARGER_H_

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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>

#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif

#define UVDM_CHARGE_EVENT_LEN                 15
#define UVDM_VBUS_THRESHOLD                   3000
#define UVDM_CLEAR_DISABLE_FLAGS              0
#define UVDM_ERR_CNT_MAX                      8
#define UVDM_OPEN_RETRY_CNT_MAX               3
#define UVDM_LIMIT_CURRENT_BY_COVER_OTP       1800
#define UVDM_MAX_POWER                        22500
#define UVDM_MAX_CURRENT                      8000

#define UVDM_IN_CHARGING_STAGE                0
#define UVDM_NOT_IN_CHARGING_STAGE           (-1)

#define UVDM_RANDOM_ADD_HASH_LEN              16
#define UVDM_AF_WAIT_CT_TIMEOUT               1000

enum uvdm_charge_sysfs_type {
	UVDM_CHARGE_SYSFS_BEGIN,
	UVDM_CHARGE_SYSFS_KEY_ID,
	UVDM_CHARGE_SYSFS_AF,
	UVDM_CHARGE_SYSFS_END,
};

enum uvdm_charge_switch_type {
	UVDM_CHARGE_MODE_KEEP,
	UVDM_CHARGE_BUCK5WTOSC,
	UVDM_CHARGE_BUCK10WTOSC,
	UVDM_CHARGE_SCTOBUCK5W,
	UVDM_CHARGE_SCTOBUCK10W,
};

enum uvdm_charge_power_type {
	UVDM_CAHRGE_TYPE_5W,
	UVDM_CAHRGE_TYPE_10W,
	UVDM_CHARGE_TYPE_20W,
};

enum uvdm_charge_stage_type {
	UVDM_STAGE_BEGIN = 0,
	UVDM_STAGE_DEFAULT = UVDM_STAGE_BEGIN,
	UVDM_STAGE_ADAPTER_CHECK,
	UVDM_STAGE_SWITCH_PATH,
	UVDM_STAGE_CHARGE_INIT,
	UVDM_STAGE_SECURITY_CHECK,
	UVDM_STAGE_SUCCESS,
	UVDM_STAGE_CHARGING,
	UVDM_STAGE_CHARGE_DONE,
	UVDM_STAGE_END,
};

enum uvdm_charge_otg_state {
	UVDM_OTG_PLUG_IN = 0,
	UVDM_OTG_PLUG_OUT = 2,
};

enum uvdm_charge_error_code {
	UVDM_SUCC,
	UVDM_ERROR_CHARGE_DISABLED,
	UVDM_ERROR_ADAPTER_DETECT,
	UVDM_ERROR_BAT_TEMP,
	UVDM_ERROR_BAT_VOL,
	UVDM_ERROR_SWITCH_PATH,
	UVDM_ERROR_INIT,
	UVDM_ERROR_USB_PORT_LEAKAGE_CURRENT,
	UVDM_ERROR_ADAPTER_VOLTAGE_ACCURACY,
	UVDM_ERROR_OPEN_CHARGE_PATH,
	UVDM_ERROR_FULL_REISISTANCE,
	UVDM_ERROR_ADAPTER_ANTI_FAKE,
};

enum uvdm_charge_disable_type {
	UVDM_DISABLE_BEGIN,
	UVDM_DISABLE_SYS_NODE = UVDM_DISABLE_BEGIN,
	UVDM_DISABLE_FATAL_ISC_TYPE,
	UVDM_DISABLE_WIRELESS_TX,
	UVDM_DISABLE_BATT_CERTIFICATION_TYPE,
	UVDM_DISABLE_END,
};

enum uvdm_charge_charging_path_type {
	UVDM_PATH_BEGIN,
	UVDM_PATH_BUCK = UVDM_PATH_BEGIN,
	UVDM_PATH_SC,
	UVDM_PATH_END,
};

enum uvdm_charge_fault_type {
	UVDM_FAULT_NON = 0,
	UVDM_FAULT_VBUS_OVP = 1,
	UVDM_FAULT_REVERSE_OCP = 2,
	UVDM_FAULT_TDIE_OTP = 6,
	UVDM_FAULT_VDROP_OVP = 8,
	UVDM_FAULT_VBAT_OVP = 10,
	UVDM_FAULT_IBAT_OCP = 11,
	UVDM_FAULT_IBUS_OCP = 12,
	UVDM_FAULT_OTG = 100,
	UVDM_FAULT_COVER_ABNORMAL = 101,
	UVDM_FAULT_TOTAL,
};

enum uvdm_charge_cover_abnormal_type {
	UVDM_ABNORMAL_TYPE_OVP,
	UVDM_ABNORMAL_TYPE_OTP_TO10W,
	UVDM_ABNORMAL_TYPE_OTP_SHUTDOWN,
	UVDM_ABNORMAL_TYPE_VBUS_FALL,
	UVDM_ABNORMAL_TYPE_RX_OFFSET,
	UVDM_ABNORMAL_TYPE_VOLTAGE_RISE_FAIL,
	UVDM_ABNORMAL_TYPE_CP_ABNORMAL,
	UVDM_ABNORMAL_TYPE_OTP_TO5W,
	UVDM_ABNORMAL_TYPE_TEMP_RECOVERY,
};

struct uvdm_switchcap_ops {
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

struct uvdm_batinfo_ops {
	int (*init)(void);
	int (*exit)(void);
	int (*get_bat_btb_voltage)(void);
	int (*get_bat_package_voltage)(void);
	int (*get_vbus_voltage)(int *vol);
	int (*get_bat_current)(int *cur);
	int (*get_ls_ibus)(int *ibus);
	int (*get_ls_temp)(int *temp);
};

#ifdef CONFIG_UVDM_CHARGER
int uvdm_sc_batinfo_ops_register(struct uvdm_batinfo_ops *ops);
int uvdm_sc_ops_register(struct uvdm_switchcap_ops *ops);
bool uvdm_get_sc_enable(void);
void uvdm_pre_check(void);
void uvdm_check(int type);
int uvdm_charge_in_charging_stage(void);
int uvdm_charge_get_super_charging_flag(void);
int uvdm_charge_cutoff_reset_param(void);
void uvdm_charge_fault_event_notify(unsigned long event, void *data);
void uvdm_schedule_work(void);
void uvdm_handle_cover_report_event(void);
#else
static inline int uvdm_sc_batinfo_ops_register(struct uvdm_batinfo_ops *ops)
{
	return -EINVAL;
}

static inline int uvdm_sc_ops_register(struct uvdm_switchcap_ops *ops)
{
	return -EINVAL;
}

static inline bool uvdm_get_sc_enable(void)
{
	return false;
}

static inline void uvdm_pre_check(void)
{
}

static inline void uvdm_check(int type)
{
}

static inline int uvdm_charge_in_charging_stage(void)
{
	return -1;
}

static inline int uvdm_charge_get_super_charging_flag(void)
{
	return -1;
}

static inline int uvdm_charge_cutoff_reset_param(void)
{
	return -1;
}

static inline void uvdm_charge_fault_event_notify(unsigned long event,
	void *data)
{
}

static inline void uvdm_schedule_work(void)
{
}

static inline void uvdm_handle_cover_report_event(void)
{
}
#endif /* CONFIG_UVDM_CHARGER */

#endif /* _UVDM_CHARGER_H_ */
