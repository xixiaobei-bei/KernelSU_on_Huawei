/*
 * battery_basp.h
 *
 * driver adapter for basp.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/battery/battery_soh.h>

#define MAX_RECORDS_CNT          5
#define BASP_DEFAULT_ICHG_MAX    8000
#define DEFAULT_NDC_VSTEP        16

struct basp_policy {
	unsigned int level;
	unsigned int learned_fcc;
	unsigned int nondc_volt_dec;
	unsigned int dmd_no;
};

#endif /* _BATTERY_BASP_H_ */
