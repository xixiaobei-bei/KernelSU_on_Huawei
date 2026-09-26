/*
 * series_batt_speaker_charge_adapter.h
 *
 * adapter operate for series batt speaker charge
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

#ifndef _SERIES_BATT_SPEAKER_CHARGE_ADAPTER_H_
#define _SERIES_BATT_SPEAKER_CHARGE_ADAPTER_H_

#ifdef CONFIG_HUAWEI_SPEAKER_CHARGER
int series_batt_speaker_charge_init_adapter(void);
int series_batt_speaker_charge_check_adapter_antifake(void);
#else
static inline int series_batt_speaker_charge_init_adapter(void)
{
	return -1;
}

static inline int series_batt_speaker_charge_check_adapter_antifake(void)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_SPEAKER_CHARGER */

#endif /* _SERIES_BATT_SPEAKER_CHARGE_ADAPTER_H_ */
