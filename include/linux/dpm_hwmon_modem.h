/*
 * dpm_hwmon_modem.h
 *
 * dpm interface for modem
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
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

#ifndef __DPM_HWMON_MODEM_H__
#define __DPM_HWMON_MODEM_H__

#define MDM_NAME_SIZE		64
#define MDM_MODULE_MAX		20

#pragma pack(1)
struct mdm_energy {
	char name[MDM_NAME_SIZE];
	unsigned long long energy;
};

struct mdm_transmit_t {
	unsigned int length;
	char data[0];
};
#pragma pack()

extern unsigned int (*get_dpm_modem_num)(void);
extern int (*get_dpm_modem_data)(struct mdm_transmit_t *, unsigned int);
int dpm_modem_register(unsigned int (*pf1)(void),
		       int (*pf2)(struct mdm_transmit_t *, unsigned int));
#endif
