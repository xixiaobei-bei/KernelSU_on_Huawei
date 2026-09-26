/*
 * direct_charge_path_switch.c
 *
 * path switch for direct charge
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

#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_path
HWLOG_REGIST();

#define MULTI_IC_CHECK_COUNT     3

static const char * const g_dc_charging_path[PATH_END] = {
	[PATH_NORMAL] = "path_normal",
	[PATH_LVC] = "path_lvc",
	[PATH_SC] = "path_sc",
};

static const char *dc_get_charging_path_string(unsigned int path)
{
	if ((path >= PATH_BEGIN) && (path < PATH_END))
		return g_dc_charging_path[path];

	return "illegal charging_path";
}

static int dc_enable_multi_ic(struct direct_charge_device *di, int enable)
{
	int mode;

	if (di->cur_mode == CHARGE_IC_AUX)
		mode = CHARGE_IC_MAIN;
	else
		mode = CHARGE_IC_AUX;

	if (enable && dcm_init_ic(di->working_mode, mode)) {
		di->multi_ic_error_cnt++;
		return -1;
	}
	if (enable && dcm_init_batinfo(di->working_mode, mode)) {
		di->multi_ic_error_cnt++;
		return -1;
	}

	if (dcm_enable_ic(di->working_mode, mode, enable)) {
		di->multi_ic_error_cnt++;
		return -1;
	}

	return 0;
}

static int dc_check_multi_ic_current(struct direct_charge_device *di)
{
	int main_ibus = 0;
	int aux_ibus = 0;
	int count = 10; /* 10 : retry times */

	while (count) {
		if (dcm_get_ic_ibus(di->working_mode, CHARGE_IC_MAIN, &main_ibus))
			return -1;
		if (dcm_get_ic_ibus(di->working_mode, CHARGE_IC_AUX, &aux_ibus))
			return -1;
		hwlog_info("check multi curr, main ibus=%d, aux_ibus=%d\n",
			main_ibus, aux_ibus);

		if ((main_ibus > MIN_CURRENT_FOR_MULTI_IC) &&
			(aux_ibus > MIN_CURRENT_FOR_MULTI_IC))
			return 0;
		msleep(DT_MSLEEP_30MS); /* adc cycle 20+ms */
		count--;
	}

	return -1;
}

/* rt charge mode test select single charge path */
static int dc_rt_select_charge_path(struct direct_charge_device *di)
{
	if (power_cmdline_is_factory_mode() &&
		(di->sysfs_mainsc_enable_charger ^ di->sysfs_auxsc_enable_charger)) {
		hwlog_info("RT single sc charge mode test\n");
		return -1;
	}

	return 0;
}

static void dc_change2singlepath(struct direct_charge_device *di)
{
	int count = MULTI_IC_CHECK_COUNT;
	int ibus = 0;

	if (di->cur_mode != CHARGE_MULTI_IC)
		return;

	while (count) {
		if (di->force_single_path_flag)
			break;
		if (dcm_get_ic_ibus(di->working_mode, di->cur_mode, &ibus))
			return;
		hwlog_info("charge2single check ibat is %d\n", ibus);
		if (ibus > (di->multi_ic_ibat_th / di->dc_volt_ratio))
			return;
		msleep(DT_MSLEEP_30MS); /* adc cycle 20+ms */
		count--;
	}

	if (dcm_enable_ic(di->working_mode, CHARGE_IC_AUX, DC_IC_DISABLE)) {
		di->stop_charging_flag_error = 1; /* set sc or lvc error flag */
		return;
	}

	dc_close_aux_wired_channel();
	di->cur_mode = CHARGE_IC_MAIN;
}

static void dc_change2multipath(struct direct_charge_device *di)
{
	int ibus = 0;
	int count = MULTI_IC_CHECK_COUNT;

	if (di->cur_mode == CHARGE_MULTI_IC)
		return;

	if (di->force_single_path_flag)
		return;

	if (multi_ic_check_ic_status(&di->multi_ic_mode_para)) {
		hwlog_err("ic status error, can not enter multi ic charge\n");
		return;
	}

	while (count) {
		if (dcm_get_ic_ibus(di->working_mode, di->cur_mode, &ibus))
			return;
		hwlog_info("change2multi check ibus is %d\n", ibus);
		if (ibus < (di->multi_ic_ibat_th / di->dc_volt_ratio))
			return;
		msleep(DT_MSLEEP_30MS); /* adc cycle 20+ms */
		count--;
	}

	if (dc_enable_multi_ic(di, DC_IC_ENABLE)) {
		di->multi_ic_error_cnt++;
		return;
	}

	di->cur_mode = CHARGE_MULTI_IC;
	msleep(DT_MSLEEP_250MS); /* need 200+ms to wait for sc open */

	if (dc_check_multi_ic_current(di)) {
		dc_enable_multi_ic(di, DC_IC_DISABLE);
		multi_ic_check_set_ic_error_flag(CHARGE_IC_AUX, &di->multi_ic_mode_para);
		di->cur_mode = CHARGE_IC_MAIN;
		di->multi_ic_error_cnt++;
		return;
	}

	di->multi_ic_check_info.multi_ic_start_time = current_kernel_time().tv_sec;
}

int dc_open_wired_channel(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (l_di->need_wired_sw_off)
		return wired_chsw_set_wired_channel(WIRED_CHANNEL_MAIN, WIRED_CHANNEL_RESTORE);

	return 0;
}

int dc_close_wired_channel(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (l_di->need_wired_sw_off)
		return wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);

	return 0;
}

void dc_open_aux_wired_channel(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	if (l_di->multi_ic_mode_para.support_multi_ic &&
		(l_di->multi_ic_error_cnt < MULTI_IC_CHECK_ERR_CNT_MAX)) {
		wired_chsw_set_wired_channel(WIRED_CHANNEL_AUX, WIRED_CHANNEL_RESTORE);
		power_usleep(DT_USLEEP_5MS); /* delay 5ms, gennerally 0.1ms */
	}
}

void dc_close_aux_wired_channel(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	if (l_di->multi_ic_mode_para.support_multi_ic)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_AUX, WIRED_CHANNEL_CUTOFF);
}

void dc_select_charge_path(void)
{
	int ibus = 0;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !l_di->multi_ic_mode_para.support_multi_ic ||
		(l_di->multi_ic_error_cnt > MULTI_IC_CHECK_ERR_CNT_MAX))
		return;

	if (dc_rt_select_charge_path(l_di))
		return;

	if (l_di->force_single_path_flag) {
		dc_change2singlepath(l_di);
		return;
	}

	if (dcm_get_ic_ibus(l_di->working_mode, l_di->cur_mode, &ibus))
		return;

	if (ibus >= ((l_di->multi_ic_ibat_th + l_di->curr_offset) / l_di->dc_volt_ratio))
		dc_change2multipath(l_di);
	else if (ibus <= ((l_di->multi_ic_ibat_th - l_di->curr_offset) / l_di->dc_volt_ratio))
		dc_change2singlepath(l_di);
}

int dc_switch_charging_path(unsigned int path)
{
	hwlog_info("switch to %d,%s charging path\n",
		path, dc_get_charging_path_string(path));

	switch (path) {
	case PATH_NORMAL:
		return direct_charge_switch_path_to_normal_charging();
	case PATH_LVC:
	case PATH_SC:
		return direct_charge_switch_path_to_dc_charging();
	default:
		return -1;
	}
}
