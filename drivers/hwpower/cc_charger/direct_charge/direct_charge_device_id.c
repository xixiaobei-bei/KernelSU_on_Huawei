/*
 * direct_charge_device_id.c
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

#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_devid
HWLOG_REGIST();

static const char * const g_dc_device_name[] = {
	[LOADSWITCH_RT9748] = "lvc_rt9748",
	[LOADSWITCH_BQ25870] = "lvc_bq25870",
	[LOADSWITCH_FAN54161] = "lvc_fan54161",
	[LOADSWITCH_PCA9498] = "lvc_pca9498",
	[LOADSWITCH_SCHARGERV600] = "lvc_hi6526",
	[LOADSWITCH_FPF2283] = "lvc_fpf2283",
	[SWITCHCAP_TI_BQ25970] = "sc_bq25970",
	[SWITCHCAP_SCHARGERV600] = "sc_hi6526",
	[SWITCHCAP_LTC7820] = "sc_ltc7820",
	[SWITCHCAP_MULTI_SC] = "sc_multi_hl1506",
	[SWITCHCAP_RT9759] = "sc_rt9759",
	[SWITCHCAP_SM_SM5450] = "sc_sm5450",
	[SWITCHCAP_SC8551] = "sc_sc8551",
	[SWITCHCAP_HL1530] = "sc_hl1530",
	[SWITCHCAP_SYH69637] = "sc_syh69637",
	[SWITCHCAP_SC8545] = "sc_sc8545",
	[SWITCHCAP_NU2105] = "sc_nu2105",
};

const char *dc_get_device_name(int id)
{
	if ((id >= DC_DEVICE_ID_BEGIN) && (id < DC_DEVICE_ID_END))
		return g_dc_device_name[id];

	hwlog_err("invalid id=%d\n", id);
	return "invalid";
}
