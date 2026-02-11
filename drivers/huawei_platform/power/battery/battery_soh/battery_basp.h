/*
 * battery_basp.h
 *
 * Driver adapter for basp.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_BASP_H_
#define _BATTERY_BASP_H_

#include <huawei_platform/power/battery_soh.h>

#define MAX_RECORDS_CNT          5
#define MAX_TIMESTAMP_STR_SIZE   32
#define MAX_BATTERY_BRAND_LEN    32
#define MAX_TEMP_BUF_LEN         128
#define BASP_BASIC_INFO_MAX_LEN  32
#define HISI_CHARGER_CYCLE_SCALE 100

struct basp_policy {
	unsigned int level;
	unsigned int learned_fcc;
	unsigned int nondc_volt_dec;
	unsigned int dmd_no;
};

struct basp_data {
	unsigned int ratio_fcc;
	unsigned int charge_cycles;
	unsigned int record_fcc_num;
	unsigned int record_fcc[MAX_RECORDS_CNT];
};

struct basp_imonitor_data {
	unsigned int bat_cap; /* battery capacity in mAh */
	unsigned long bat_cyc; /* Battery charging cycles */
	unsigned int q_max; /* battery QMAX */
	char bat_man[BASP_BASIC_INFO_MAX_LEN]; /* battery manufactor id */
	char bat_par_f[BASP_BASIC_INFO_MAX_LEN]; /* batt Flag bit of para */
};

void basp_event_notify(struct bsoh_device *di, unsigned int event);
void basp_dmd_content_prepare(char *buff, unsigned int size);
int basp_sysfs_create_group(struct bsoh_device *di);
void basp_sysfs_remove_group(struct bsoh_device *di);

#endif /* _BATTERY_BASP_H_ */
