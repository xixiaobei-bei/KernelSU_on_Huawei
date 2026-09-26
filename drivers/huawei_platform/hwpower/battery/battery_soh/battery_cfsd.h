/*
 * battery_cfsd.h
 *
 * driver adapter for cfsd.
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

#ifndef _BATTERY_CFSD_H_
#define _BATTERY_CFSD_H_

#include <chipset_common/hwpower/battery/battery_soh.h>

struct cfsd_ocv_info {
	time_t time;
	int ocv_uv; /* uV */
	int ocv_temp;
	s64 cc_value; /* uAh */
	int ocv_soc;
	int batt_chargecycles;
	int ocv_level;
};

enum CFSD_EVENT_TYPE {
	CFSD_EVENT_NONE,
	CFSD_OCV_UPDATE_EVENT,
	CFSD_BATTERY_REMOVED,
	CFSD_EVENT_CNT,
};

#endif /* _BATTERY_CFSD_H_ */
