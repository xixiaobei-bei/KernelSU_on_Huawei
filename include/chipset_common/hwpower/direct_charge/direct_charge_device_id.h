/*
 * direct_charge_device_id.h
 *
 * device id for direct charge
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

#ifndef _DIRECT_CHARGE_DEVICE_ID_H_
#define _DIRECT_CHARGE_DEVICE_ID_H_

enum dc_device_id {
	DC_DEVICE_ID_BEGIN,
	LOADSWITCH_RT9748 = DC_DEVICE_ID_BEGIN,
	LOADSWITCH_BQ25870,
	LOADSWITCH_FAN54161,
	LOADSWITCH_PCA9498,
	LOADSWITCH_SCHARGERV600,
	LOADSWITCH_FPF2283,
	SWITCHCAP_TI_BQ25970,
	SWITCHCAP_SCHARGERV600,
	SWITCHCAP_LTC7820,
	SWITCHCAP_MULTI_SC,
	SWITCHCAP_RT9759,
	SWITCHCAP_SM_SM5450,
	SWITCHCAP_SC8551,
	SWITCHCAP_HL1530,
	SWITCHCAP_SYH69637,
	SWITCHCAP_SC8545,
	SWITCHCAP_NU2105,
	SWITCHCAP_AW32280,
	DC_DEVICE_ID_END,
};

#ifdef CONFIG_DIRECT_CHARGER
const char *dc_get_device_name(int id);
#else
static inline const char *dc_get_device_name(int id)
{
	return "invalid";
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_DEVICE_ID_H_ */
