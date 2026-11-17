/*
 * direct_charge_ic_manager.h
 *
 * direct charge ic management interface
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGE_IC_MANAGER_H_
#define _DIRECT_CHARGE_IC_MANAGER_H_

#include <linux/init.h>
#include <linux/bitops.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

#define DC_IC_ENABLE            1
#define DC_IC_DISABLE           0

#ifdef CONFIG_DIRECT_CHARGER
int dcm_init_ic(int mode, unsigned int path);
int dcm_exit_ic(int mode, unsigned int path);
int dcm_enable_ic(int mode, unsigned int path, int enable);
int dcm_enable_ic_adc(int mode, unsigned int path, int enable);
int dcm_discharge_ic(int mode, unsigned int path, int enable);
int dcm_is_ic_close(int mode, unsigned int path);
bool dcm_is_ic_support_prepare(int mode, unsigned int path);
int dcm_prepare_enable_ic(int mode, unsigned int path);
int dcm_config_ic_watchdog(int mode, unsigned int path, int time);
int dcm_kick_ic_watchdog(int mode, unsigned int path);
int dcm_get_ic_id(int mode, unsigned int path);
int dcm_get_ic_status(int mode, unsigned int path);
int dcm_set_ic_buck_enable(int mode, unsigned int path, int enable);
int dcm_reset_and_init_ic_reg(int mode, unsigned int path);
int dcm_get_ic_freq(int mode, unsigned int path);
int dcm_set_ic_freq(int mode, unsigned int path, int freq);
const char *dcm_get_ic_name(int mode, unsigned int path);
int dcm_init_batinfo(int mode, unsigned int path);
int dcm_exit_batinfo(int mode, unsigned int path);
int dcm_get_ic_vbtb(int mode, unsigned int path);
int dcm_get_ic_vpack(int mode, unsigned int path);
int dcm_get_ic_vbtb_with_comp(int mode, unsigned int path, const int *vbat_comp);
int dcm_get_ic_max_vbtb_with_comp(int mode, const int *vbat_comp);
int dcm_get_ic_vbus(int mode, unsigned int path, int *vbus);
int dcm_get_ic_ibat(int mode, unsigned int path, int *ibat);
int dcm_get_total_ibat(int mode, unsigned int path, int *ibat);
int dcm_get_ic_ibus(int mode, unsigned int path, int *ibus);
int dcm_get_ic_temp(int mode, unsigned int path, int *temp);
int dcm_get_ic_vusb(int mode, unsigned int path, int *vusb);
int dcm_get_ic_vout(int mode, unsigned int path, int *vout);
#else
static inline int dcm_init_ic(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_exit_ic(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_enable_ic(int mode, unsigned int path, int enable)
{
	return -1;
}

static inline int dcm_enable_ic_adc(int mode, unsigned int path, int enable)
{
	return -1;
}

static inline int dcm_discharge_ic(int mode, unsigned int path, int enable)
{
	return -1;
}

static inline int dcm_is_ic_close(int mode, unsigned int path)
{
	return -1;
}

static inline bool dcm_is_ic_support_prepare(int mode, unsigned int path)
{
	return false;
}

static inline int dcm_prepare_enable_ic(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_config_ic_watchdog(int mode, unsigned int path, int time)
{
	return -1;
}

static inline int dcm_kick_ic_watchdog(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_get_ic_id(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_get_ic_status(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_set_ic_buck_enable(int mode, unsigned int path, int enable)
{
	return -1;
}

static inline int dcm_reset_and_init_ic_reg(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_get_ic_freq(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_set_ic_freq(int mode, unsigned int path, int freq)
{
	return -1;
}

static inline const char *dcm_get_ic_name(int mode, unsigned int path)
{
	return "invalid ic";
}

static inline int dcm_init_batinfo(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_exit_batinfo(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_get_ic_vbtb(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_get_ic_vpack(int mode, unsigned int path)
{
	return -1;
}

static inline int dcm_get_ic_vbtb_with_comp(int mode, unsigned int path, const int *vbat_comp)
{
	return -1;
}

static inline int dcm_get_ic_max_vbtb_with_comp(int mode, const int *vbat_comp)
{
	return -1;
}

static inline int dcm_get_ic_vbus(int mode, unsigned int path, int *vbus)
{
	return -1;
}

static inline int dcm_get_ic_ibat(int mode, unsigned int path, int *ibat)
{
	return -1;
}

static inline int dcm_get_total_ibat(int mode, unsigned int path, int *ibat)
{
	return -1;
}

static inline int dcm_get_ic_ibus(int mode, unsigned int path, int *ibus)
{
	return -1;
}

static inline int dcm_get_ic_temp(int mode, unsigned int path, int *temp)
{
	return -1;
}

static inline int dcm_get_ic_vusb(int mode, unsigned int path, int *vusb)
{
	return -1;
}

static inline int dcm_get_ic_vout(int mode, unsigned int path, int *vout)
{
	return -1;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_IC_MANAGER_H_ */
