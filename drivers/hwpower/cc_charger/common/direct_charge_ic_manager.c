/*
 * direct_charge_ic_manager.c
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

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/math64.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_interface.h>

#define HWLOG_TAG direct_charge_ic_manager
HWLOG_REGIST();

int dcm_init_ic(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	hwlog_info("path[%u] num[%d]:init ic\n", path, num);

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_init_ic(mode, index[i]))
			return -1;
	}

	return 0;
}

int dcm_exit_ic(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	hwlog_info("path[%u] num[%d]:exit ic\n", path, num);

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_exit_ic(mode, index[i]))
			return -1;
	}

	return 0;
}

int dcm_enable_ic(int mode, unsigned int path, int enable)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	hwlog_info("path[%u] num[%d]:enable ic, enable %d\n", path, num, enable);

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_enable_ic(mode, index[i], enable))
			return -1;
	}

	return 0;
}

int dcm_enable_ic_adc(int mode, unsigned int path, int enable)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int ret = 0;

	hwlog_info("path[%u] num[%d]:enable adc, enable %d\n", path, num, enable);

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++)
		ret += dc_enable_ic_adc(mode, index[i], enable);

	return ret;
}

int dcm_discharge_ic(int mode, unsigned int path, int enable)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_discharge_ic(mode, index[i], enable))
			return -1;
	}

	return 0;
}

int dcm_is_ic_close(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_is_ic_close(mode, index[i]))
			return -1;
	}

	return 0;
}

bool dcm_is_ic_support_prepare(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return false;

	for (i = 0; i < num; i++) {
		if (!dc_is_ic_support_prepare(mode, index[i]))
			return false;
	}

	return true;
}

int dcm_prepare_enable_ic(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_prepare_enable_ic(mode, index[i]))
			return -1;
	}

	return 0;
}

int dcm_config_ic_watchdog(int mode, unsigned int path, int time)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_config_ic_watchdog(mode, index[i], time))
			return -1;
	}

	return 0;
}

int dcm_kick_ic_watchdog(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int ret = 0;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++)
		ret += dc_kick_ic_watchdog(mode, index[i]);

	return ret;
}

int dcm_get_ic_id(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (num <= 0)
		return -1;

	return dc_get_ic_id(mode, index[0]);
}

int dcm_get_ic_status(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int ret = 0;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++)
		ret += dc_get_ic_status(mode, index[i]);

	return ret;
}

int dcm_set_ic_buck_enable(int mode, unsigned int path, int enable)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int ret = 0;

	hwlog_info("path[%u]:set ic buck enable, enable %d\n", path, enable);

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++)
		ret += dc_set_ic_buck_enable(mode, index[i], enable);

	return ret;
}

int dcm_reset_and_init_ic_reg(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int ret = 0;

	hwlog_info("path[%u]:reg and init\n", path);

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++)
		ret += dc_reset_and_init_ic_reg(mode, index[i]);

	return ret;
}

int dcm_get_ic_freq(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (num <= 0)
		return -1;

	return dc_get_ic_freq(mode, index[0]);
}

int dcm_set_ic_freq(int mode, unsigned int path, int freq)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (num <= 0)
		return -1;

	return dc_set_ic_freq(mode, index[0], freq);
}

const char *dcm_get_ic_name(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (num <= 0)
		return "invalid ic";

	return dc_get_ic_name(mode, index[0]);
}

int dcm_init_batinfo(int mode, unsigned int path)
{
	int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_init_batinfo(mode, index[i]))
			return -1;
	}

	return 0;
}

int dcm_exit_batinfo(int mode, unsigned int path)
{
	int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;

	if (num <= 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_exit_batinfo(mode, index[i]))
			return -1;
	}

	return 0;
}

int dcm_get_ic_vbtb(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int vbat = 0;

	if (dc_ic_get_vbat_from_coul(&vbat))
		return vbat;

	if (num <= 0)
		return -1;

	return dc_get_ic_vbtb(mode, index[0]);
}

int dcm_get_ic_vbtb_with_comp(int mode, unsigned int path, const int *vbat_comp)
{
	int vbat;
	int comp;

	if (!vbat_comp)
		return -1;

	vbat = dcm_get_ic_vbtb(mode, path);
	if (vbat < 0)
		return -1;
	else if (vbat == 0)
		return vbat;

	/* vbat_comp[0] : main ic comp, vbat_comp[1] : aux ic comp */
	switch (path) {
	case CHARGE_MULTI_IC:
	case CHARGE_IC_MAIN:
		comp = vbat_comp[CHARGE_IC_TYPE_MAIN];
		break;
	case CHARGE_IC_AUX:
		comp = vbat_comp[CHARGE_IC_TYPE_AUX];
		break;
	default:
		comp = 0;
		break;
	}

	return vbat + comp;
}

int dcm_get_ic_max_vbtb_with_comp(int mode, const int *vbat_comp)
{
	int vbat_main;
	int vbat_aux;

	if (!vbat_comp)
		return -1;

	vbat_main = dcm_get_ic_vbtb_with_comp(mode, CHARGE_IC_MAIN, vbat_comp);
	vbat_aux = dcm_get_ic_vbtb_with_comp(mode, CHARGE_IC_AUX, vbat_comp);

	return (vbat_main > vbat_aux) ? vbat_main : vbat_aux;
}

int dcm_get_ic_vpack(int mode, unsigned int path)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (num <= 0)
		return -1;

	return dc_get_ic_vpack(mode, index[0]);
}

int dcm_get_ic_ibat(int mode, unsigned int path, int *ibat)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (!ibat || (num <= 0))
		return -1;

	if (dc_get_ic_ibat(mode, index[0], ibat))
		return -1;

	return 0;
}

int dcm_get_total_ibat(int mode, unsigned int path, int *ibat)
{
	int i;
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index_for_ibat(mode, path, index, CHARGE_IC_MAX_NUM);
	int tmp_ibat = 0;
	int total_ibat = 0;

	if (!ibat || (num <= 0))
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_get_ic_ibat(mode, index[i], &tmp_ibat))
			return -1;
		hwlog_info("ic[%u] ibat is %d\n", index[i], tmp_ibat);
		total_ibat += tmp_ibat;
	}

	*ibat = total_ibat;
	hwlog_info("total ibat is %d\n", *ibat);
	return 0;
}

int dcm_get_ic_vbus(int mode, unsigned int path, int *vbus)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (!vbus || (num <= 0))
		return -1;

	return dc_get_ic_vbus(mode, index[0], vbus);
}

int dcm_get_ic_ibus(int mode, unsigned int path, int *ibus)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int tmp_ibus = 0;
	int total_ibus = 0;

	if (!ibus || (num <= 0))
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_get_ic_ibus(mode, index[i], &tmp_ibus) < 0)
			return -1;
		hwlog_info("ic[%u] ibus is %d\n", index[i], tmp_ibus);
		total_ibus += tmp_ibus;
	}

	*ibus = total_ibus;
	return 0;
}

int dcm_get_ic_temp(int mode, unsigned int path, int *temp)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);
	int i;
	int tmp_temp = 0;
	int max_temp = 0;

	if (!temp || (num <= 0))
		return -1;

	for (i = 0; i < num; i++) {
		if (dc_get_ic_temp(mode, index[i], &tmp_temp) < 0)
			return -1;
		max_temp = max_temp > tmp_temp ? max_temp : tmp_temp;
	}

	*temp = max_temp;
	return 0;
}

int dcm_get_ic_vusb(int mode, unsigned int path, int *vusb)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (!vusb || (num <= 0))
		return -1;

	return dc_get_ic_vusb(mode, index[0], vusb);
}

int dcm_get_ic_vout(int mode, unsigned int path, int *vout)
{
	unsigned int index[CHARGE_IC_MAX_NUM] = { 0 };
	int num = dc_ic_get_ic_index(mode, path, index, CHARGE_IC_MAX_NUM);

	if (!vout || (num <= 0))
		return -1;

	return dc_get_ic_vout(mode, index[0], vout);
}
