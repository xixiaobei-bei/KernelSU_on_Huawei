/*
 * wireless_dc_ic.c
 *
 * direct charge ic interface for wireless direct charge
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>

#define HWLOG_TAG wireless_dc_ic
HWLOG_REGIST();

static int wldc_ic_enable_multi_ic(struct wldc_dev_info *di, int type, int enable)
{
	if (enable && dcm_init_ic(SC_MODE, type)) {
		di->ic_data.multi_ic_err_cnt++;
		return -EINVAL;
	}
	if (enable && dcm_init_batinfo(SC_MODE, type)) {
		di->ic_data.multi_ic_err_cnt++;
		return -EINVAL;
	}
	if (dcm_enable_ic(SC_MODE, type, enable)) {
		di->ic_data.multi_ic_err_cnt++;
		return -EINVAL;
	}
	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return -EINVAL;

	return 0;
}

static int wldc_ic_check_multi_ic_current(struct wldc_dev_info *di)
{
	int main_ibus = 0;
	int aux_ibus = 0;
	int count = WLDC_IC_IBUS_CHECK_CNT;

	while (count) {
		if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
			return -EINVAL;
		if (dcm_get_ic_ibus(SC_MODE, CHARGE_IC_MAIN, &main_ibus))
			return -EINVAL;
		if (dcm_get_ic_ibus(SC_MODE, CHARGE_IC_AUX, &aux_ibus))
			return -EINVAL;
		hwlog_info("[check_multi_ic_current] main ibus=%d,aux_ibus=%d\n",
			main_ibus, aux_ibus);
		if ((main_ibus > WLDC_OPEN_PATH_IOUT_MIN) &&
			(aux_ibus > WLDC_OPEN_PATH_IOUT_MIN))
			return 0;
		if (!power_msleep(DT_MSLEEP_500MS, DT_MSLEEP_25MS, wldc_msleep_exit))
			return -EINVAL;
		count--;
	}

	return -EINVAL;
}

static void wldc_ic_switch_to_multipath(struct wldc_dev_info *di)
{
	int type;

	wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW2, true);
	power_usleep(DT_USLEEP_10MS);

	di->ic_data.ibus_para.sw_cnt = 0;

	if (di->ic_data.cur_type == CHARGE_IC_AUX)
		type = CHARGE_IC_MAIN;
	else
		type = CHARGE_IC_AUX;

	if (wldc_ic_enable_multi_ic(di, type, DC_IC_ENABLE)) {
		wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW2, false);
		di->ic_data.multi_ic_err_cnt++;
		return;
	}

	 /* at least need 200ms to wait for sc open */
	if (!power_msleep(DT_MSLEEP_250MS, DT_MSLEEP_25MS, wldc_msleep_exit)) {
		wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW2, false);
		return;
	}
	if (wldc_ic_check_multi_ic_current(di)) {
		wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW2, false);
		wldc_ic_enable_multi_ic(di, type, DC_IC_DISABLE);
		multi_ic_check_set_ic_error_flag(type, &di->ic_mode_para);
		return;
	}

	di->ic_data.cur_type = CHARGE_MULTI_IC;
	di->ic_check_info.multi_ic_start_time = current_kernel_time().tv_sec;
	hwlog_info("[switch_to_multipath] succ\n");
}

static bool wldc_ic_is_need_multipath(struct wldc_dev_info *di)
{
	int ibus = 0;
	int count;

	if (di->ic_data.cur_type == CHARGE_MULTI_IC)
		return false;

	if (dcm_get_ic_ibus(SC_MODE, CHARGE_IC_MAIN, &ibus))
		return false;

	if (ibus < di->ic_data.ibus_para.hth) {
		di->ic_data.ibus_para.hth_cnt = 0;
		return false;
	}

	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return false;

	di->ic_data.ibus_para.hth_cnt++;
	if (!di->ctrl_interval)
		return false;
	count = di->ic_data.ibus_para.h_time / di->ctrl_interval;
	if (di->ic_data.ibus_para.hth_cnt < count)
		return false;

	if (multi_ic_check_ic_status(&di->ic_mode_para))
		return false;

	return true;
}

static void wldc_ic_switch_to_singlepath(struct wldc_dev_info *di)
{
	if (dcm_enable_ic(SC_MODE, CHARGE_IC_AUX, DC_IC_DISABLE)) {
		di->stop_flag_error = true;
		return;
	}
	di->ic_data.cur_type = CHARGE_IC_MAIN;
	di->ic_data.ibus_para.sw_cnt = WLDC_IC_SC_SW2_CNT;
	hwlog_info("[switch_to_singlepath] succ\n");
}

static bool wldc_ic_is_need_singlepath(struct wldc_dev_info *di)
{
	int count;
	int ibus = 0;

	if (di->ic_data.cur_type != CHARGE_MULTI_IC)
		return false;

	if (dcm_get_ic_ibus(SC_MODE, CHARGE_MULTI_IC, &ibus))
		return false;

	if (ibus > di->ic_data.ibus_para.lth) {
		di->ic_data.ibus_para.lth_cnt = 0;
		return false;
	}

	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return false;

	di->ic_data.ibus_para.lth_cnt++;
	if (!di->ctrl_interval)
		return false;
	count = di->ic_data.ibus_para.l_time / di->ctrl_interval;
	if (di->ic_data.ibus_para.lth_cnt < count)
		return false;

	return true;
}

void wldc_ic_gpio_sc_sw2_check(struct wldc_dev_info *di)
{
	if (di->ic_data.ibus_para.sw_cnt) {
		di->ic_data.ibus_para.sw_cnt--;
		if (di->ic_data.ibus_para.sw_cnt)
			return;
		wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW2, false);
	}
}

void wldc_ic_select_charge_path(struct wldc_dev_info *di)
{
	if (!di || !di->ic_mode_para.support_multi_ic ||
		(di->ic_data.multi_ic_err_cnt > WLDC_IC_ERR_CNT))
		return;

	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return;

	if (wldc_ic_is_need_multipath(di))
		wldc_ic_switch_to_multipath(di);

	if (wldc_ic_is_need_singlepath(di))
		wldc_ic_switch_to_singlepath(di);

	wldc_ic_gpio_sc_sw2_check(di);
}

void wldc_ic_para_reset(struct wldc_dev_info *di)
{
	if (!di || !di->ic_mode_para.support_multi_ic)
		return;

	di->ic_data.multi_ic_err_cnt = 0;
	di->ic_data.ibus_para.sw_cnt = 0;
	di->ic_data.ibus_para.hth_cnt = 0;
	di->ic_data.ibus_para.lth_cnt = 0;
	di->ic_data.cur_type = CHARGE_IC_MAIN;
	di->ic_check_info.limit_current = WLDC_DFT_MAX_CP_IOUT;
	memset(di->ic_check_info.report_info, 0,
		sizeof(di->ic_check_info.report_info));
	memset(di->ic_check_info.ibus_error_num, 0,
		sizeof(di->ic_check_info.ibus_error_num));
	memset(di->ic_check_info.vbat_error_num, 0,
		sizeof(di->ic_check_info.vbat_error_num));
	memset(di->ic_check_info.tbat_error_num, 0,
		sizeof(di->ic_check_info.tbat_error_num));
	memset(di->ic_mode_para.ic_error_cnt, 0,
		sizeof(di->ic_mode_para.ic_error_cnt));
}
