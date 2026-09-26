/*
 * series_batt_speaker_charger.h
 *
 * series batt speaker charger driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _SERIES_BATT_SPEAKER_CHARGER_H_
#define _SERIES_BATT_SPEAKER_CHARGER_H_

#define DC_ADAPTER_MAX_POWER_20W        20000000
#define DC_ADAPTER_MAX_POWER_22P5W      22500000
#define DC_ADAPTER_MAX_POWER_40W        40000000
#define DC_ADAPTER_CURRENT_4A           4000
#define DC_ADAPTER_CURRENT_4P5A         4500
#define DC_ADAPTER_CURRENT_2P25A        2250
#define DC_ADAPTER_CURRENT_2A           2000
#define DC_ADAPTER_VOLTAGE_5V           5000
#define DC_ADAPTER_VOLTAGE_10V          10000

#ifdef CONFIG_HUAWEI_SPEAKER_CHARGER
int series_batt_speaker_charge_start_charging(void);
void series_batt_speaker_charge_stop_charging(void);
bool series_batt_in_speaker_charging_mode(void);
int series_batt_speaker_charge_init_adapter_and_device(void);
int series_batt_speaker_charge_get_iin_power(int *iin_val, int *max_power);
int series_batt_speaker_charge_get_iin_limit(void);
#else
static inline int series_batt_speaker_charge_start_charging(void)
{
	return -1;
}

static inline void series_batt_speaker_charge_stop_charging(void)
{
}

static inline bool series_batt_in_speaker_charging_mode(void)
{
	return false;
}

static inline int series_batt_speaker_charge_init_adapter_and_device(void)
{
	return -1;
}

static inline int series_batt_speaker_charge_get_iin_power(int *iin_val, int *max_power)
{
	return -1;
}

static inline int series_batt_speaker_charge_get_iin_limit(void)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_SPEAKER_CHARGER */

#endif /* _SERIES_BATT_SPEAKER_CHARGER_H_ */
