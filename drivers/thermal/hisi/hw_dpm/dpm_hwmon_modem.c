
/*
 * dpm_hwmon_modem.c
 *
 * dpm interface for modem
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include <linux/dpm_hwmon_modem.h>
#include <linux/kernel.h>
#include <linux/errno.h>

unsigned int (*get_dpm_modem_num)(void) = NULL;
int (*get_dpm_modem_data)(struct mdm_transmit_t *, unsigned int) = NULL;

int dpm_modem_register(unsigned int (*pf1)(void),
		       int (*pf2)(struct mdm_transmit_t *, unsigned int))
{
#ifdef CONFIG_MODEM_DPM
	if (pf1 == NULL || pf2 == NULL)
		return -EINVAL;
	get_dpm_modem_num = pf1;
	get_dpm_modem_data = pf2;
#endif
	return 0;
}
EXPORT_SYMBOL(dpm_modem_register);
