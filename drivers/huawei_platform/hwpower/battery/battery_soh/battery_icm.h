/*
 * battery_icm.h
 *
 * driver adapter for icm.
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

#ifndef _BATTERY_ICM_H_
#define _BATTERY_ICM_H_

#include <linux/bitops.h>
#include <linux/power/hisi/hisi_eis.h>
#include <linux/power/hisi/soh/hisi_soh_interface.h>
#include <chipset_common/hwpower/battery/battery_soh.h>

struct eis_freq_array {
	int freq[LEN_T_FREQ];
	int temp[LEN_T_FREQ];
	int cycle[LEN_T_FREQ];
	int real[LEN_T_FREQ];
	int imag[LEN_T_FREQ];
};

struct icm_device_info {
	struct bsoh_device *b_di;
	struct acr_info acr_info;
	struct eis_freq_infos eis_freq_infos;
	struct eis_freq_array eis_freq_a;
	struct notifier_block soh_nb;
	struct notifier_block eis_nb;
	unsigned int eis_evt_to_send;
	struct delayed_work eis_uevent_work;
	unsigned int soh_evt_to_send;
	struct delayed_work soh_uevent_work;
};

#endif /* _BATTERY_ICM_H_ */
