/*
 * series_batt_speaker_charger_check.h
 *
 * check for series batt speaker charge module
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

#ifndef _SERIES_BATT_SPEAKER_CHARGER_CHECK_H_
#define _SERIES_BATT_SPEAKER_CHARGER_CHECK_H_

#ifdef CONFIG_HUAWEI_SPEAKER_CHARGER
void series_batt_speaker_charge_mode_check(struct direct_charge_device *di);
#else
static inline void series_batt_speaker_charge_mode_check(struct direct_charge_device *di)
{
}
#endif /* CONFIG_HUAWEI_SPEAKER_CHARGER */

#endif /* _SERIES_BATT_SPEAKER_CHARGER_CHECK_H_ */
