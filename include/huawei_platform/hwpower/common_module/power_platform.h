/*
 * power_platform.h
 *
 * differentiated interface related to chip platform
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

#ifndef _POWER_PLATFORM_H_
#define _POWER_PLATFORM_H_

#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/wireless/wireless_tx_pwr_ctrl.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power/hisi/soh/hisi_soh_interface.h>
#include <linux/power/hisi/battery_data.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_tcpm.h>
#include <linux/mfd/hisi_pmic.h>
#include <media/huawei/camera/pmic/hw_pmic.h>
#ifdef CONFIG_HISI_POWERKEY_SPMI
#include <linux/hisi/powerkey_event.h>
#endif

#define POWER_PLATFOR_SOC_UI_OFFSET    0

static inline int power_platform_get_filter_soc(int base)
{
	return bci_capacity_get_filter_sum(base);
}

static inline void power_platform_sync_filter_soc(int rep_soc,
	int round_soc, int base)
{
	bci_capacity_sync_filter(rep_soc, round_soc, base);
}

static inline void power_platform_cancle_capacity_work(void)
{
}

static inline void power_platform_restart_capacity_work(void)
{
}

static inline int power_platform_get_adc_sample(int adc_channel)
{
	return hisi_adc_get_adc(adc_channel);
}

static inline int power_platform_get_adc_voltage(int adc_channel)
{
	return hisi_adc_get_value(adc_channel);
}

static inline int power_platform_get_battery_id_voltage(void)
{
	return coul_drv_battery_id_voltage();
}

static inline int power_platform_get_battery_capacity(void)
{
	return coul_drv_battery_capacity();
}

static inline int power_platform_get_battery_ui_capacity(void)
{
	return coul_drv_battery_capacity();
}

static inline int power_platform_get_battery_temperature(void)
{
	return coul_drv_battery_temperature();
}

static inline int power_platform_get_rt_battery_temperature(void)
{
	return coul_drv_battery_temperature_for_charger();
}

static inline char *power_platform_get_battery_brand(void)
{
	return coul_drv_battery_brand();
}

static inline int power_platform_get_battery_voltage(void)
{
	return coul_drv_battery_voltage();
}

static inline int power_platform_get_battery_current(void)
{
	return coul_drv_battery_current();
}

static inline int power_platform_get_battery_current_avg(void)
{
	return coul_drv_battery_current_avg();
}

static inline int power_platform_is_battery_removed(void)
{
	return coul_drv_battery_removed_before_boot();
}

static inline int power_platform_is_battery_exit(void)
{
	return coul_drv_is_battery_exist();
}

static inline unsigned int power_platform_get_charger_type(void)
{
	return charge_get_charger_type();
}

static inline int power_platform_get_vbus_status(void)
{
	return pmic_get_vbus_status();
}

static inline int power_platform_pmic_enable_boost(int value)
{
	return pmic_enable_boost(value);
}

static inline int power_platform_get_vusb_status(int *value)
{
	return charge_get_vusb(value);
}

static inline bool power_platform_usb_state_is_host(void)
{
	return chip_usb_state_is_host();
}

static inline bool power_platform_pogopin_is_support(void)
{
	return pogopin_is_support();
}

static inline bool power_platform_pogopin_otg_from_buckboost(void)
{
	return pogopin_otg_from_buckboost();
}

#ifdef CONFIG_HISI_POWERKEY_SPMI
static inline int power_platform_powerkey_register_notifier(struct notifier_block *nb)
{
	return powerkey_register_notifier(nb);
}

static inline int power_platform_powerkey_unregister_notifier(struct notifier_block *nb)
{
	return powerkey_unregister_notifier(nb);
}

static inline bool power_platform_is_powerkey_up(unsigned long event)
{
	return event == PRESS_KEY_UP;
}
#else
static inline int power_platform_powerkey_register_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int power_platform_powerkey_unregister_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline bool power_platform_is_powerkey_up(unsigned long event)
{
	return false;
}
#endif /* CONFIG_HISI_POWERKEY_SPMI */

extern bool get_cancel_work_flag(void);
static inline bool power_platform_get_cancel_work_flag(void)
{
	return get_cancel_work_flag();
}

extern bool get_sysfs_wdt_disable_flag(void);
static inline bool power_platform_get_sysfs_wdt_disable_flag(void)
{
	return get_sysfs_wdt_disable_flag();
}

#ifdef CONFIG_CHARGER_SYS_WDG
extern void charge_stop_sys_wdt(void);
static inline void power_platform_charge_stop_sys_wdt(void)
{
	return charge_stop_sys_wdt();
}
#else
static inline void power_platform_charge_stop_sys_wdt(void)
{
}
#endif /* CONFIG_CHARGER_SYS_WDG */

#ifdef CONFIG_CHARGER_SYS_WDG
extern void charge_feed_sys_wdt(unsigned int timeout);
static inline void power_platform_charge_feed_sys_wdt(unsigned int timeout)
{
	charge_feed_sys_wdt(timeout);
}
#else
static inline void power_platform_charge_feed_sys_wdt(unsigned int timeout)
{
}
#endif /* CONFIG_CHARGER_SYS_WDG */

static inline int power_platform_set_max_input_current(void)
{
	return charge_set_input_current_max();
}

static inline void power_platform_start_acr_calc(void)
{
	soh_acr_low_precision_cal_start();
}

static inline int power_platform_get_acr_resistance(int *acr_value)
{
	struct acr_info acr_data;
	int ret;

	if (!acr_value)
		return -1;

	memset(&acr_data, 0, sizeof(acr_data));
	ret = soh_get_acr_resistance(&acr_data, ACR_L_PRECISION);
	*acr_value = acr_data.batt_acr;
	return ret;
}

#ifdef CONFIG_DIRECT_CHARGER
static inline bool power_platform_in_dc_charging_stage(void)
{
	return direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE;
}
#else
static inline bool power_platform_in_dc_charging_stage(void)
{
	return false;
}
#endif /* CONFIG_DIRECT_CHARGER */

static inline void power_platform_set_charge_batfet(int val)
{
	charge_set_batfet_disable(val);
}

static inline void power_platform_set_charge_hiz(int enable)
{
	charge_set_hiz_enable(enable);
}

#endif /* _POWER_PLATFORM_H_ */
