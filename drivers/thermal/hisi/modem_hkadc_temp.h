/*
 * modem_hkadc_temp.h
 *
 * modem hkadc head file for thermal
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifndef __MODEM_HKADC_TEMP_H
#define __MODEM_HKADC_TEMP_H
#include <linux/thermal.h>
#include <linux/hisi/hisi_adc.h>

struct hisi_mdm_adc_t {
	struct thermal_zone_device *tz_dev;
	const char *name;
	const char *ntc_name;
	unsigned int channel;
	unsigned int mode;
};

struct hisi_mdm_adc_chip {
	int tsens_num_sensor;
	struct hisi_mdm_adc_t hisi_mdm_sensor[0];
};

extern  int mdm_ntc_2_temp(struct hisi_mdm_adc_t *mdm_sensor, int *temp,
				int ntc);

#endif
