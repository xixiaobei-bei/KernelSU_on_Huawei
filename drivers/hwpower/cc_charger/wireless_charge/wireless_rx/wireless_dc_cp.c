/*
 * wireless_dc_cp.c
 *
 * charge pump operations for wireless direct charge
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

#include <linux/delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/wireless_charge/wireless_dc_cp.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>

#define HWLOG_TAG wireless_dc_cp
HWLOG_REGIST();

void wldc_parse_multi_cp_para(struct device_node *np, struct wldc_dev_info *di)
{
	u32 tmp_para[WIRELESS_DC_CP_TOTAL] = { 0 };

	if (!di || !np)
		return;

	di->cp_data.cur_type = CP_TYPE_MAIN;
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "support_multi_cp",
		(u32 *)&di->cp_data.support_multi_cp, 0))
		return;

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "single_cp_iout_th",
		(u32 *)&di->cp_data.single_cp_iout_th, 0);

	if (!di->cp_data.support_multi_cp)
		return;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"multi_cp_para", tmp_para, WIRELESS_DC_CP_TOTAL)) {
		di->cp_data.support_multi_cp = 0;
		return;
	}

	di->cp_data.ibus_para.hth = (int)tmp_para[WIRELESS_DC_CP_IBUS_HTH];
	di->cp_data.ibus_para.h_time = (int)tmp_para[WIRELESS_DC_CP_HTIME];
	di->cp_data.ibus_para.lth = (int)tmp_para[WIRELESS_DC_CP_IBUS_LTH];
	di->cp_data.ibus_para.l_time = (int)tmp_para[WIRELESS_DC_CP_LTIME];
	hwlog_info("cp_para ibus_hth:%dmA h_time:%d ibus_lth:%dmA l_time:%d\n",
		di->cp_data.ibus_para.hth, di->cp_data.ibus_para.h_time,
		di->cp_data.ibus_para.lth, di->cp_data.ibus_para.l_time);
}

static bool wldc_cp_aux_status_check(struct wldc_dev_info *di)
{
	int rx_ratio;
	int rx_vout = 0;
	int cp_vout;

	if (di->cp_data.cur_type != CP_TYPE_MULTI)
		return true;

	if (di->cp_data.aux_check_cnt <= 0)
		return true;

	di->cp_data.aux_check_cnt--;
	if (!charge_pump_is_cp_open(CP_TYPE_AUX))
		return false;

	rx_ratio = charge_pump_get_cp_ratio(CP_TYPE_AUX);
	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &rx_vout);
	cp_vout = charge_pump_get_cp_vout(CP_TYPE_AUX);
	cp_vout = (cp_vout > 0) ? cp_vout : wldc_get_ls_vbus();

	hwlog_info("[aux_cp_status_check] [rx] vout:%d [cp] ratio:%d vout:%d\n",
		rx_vout, rx_ratio, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - WLDC_CP_VOUT_ERR_TH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + WLDC_CP_VOUT_ERR_TH))
		return false;

	return true;
}

static void wldc_cp_switch_to_multipath(struct wldc_dev_info *di)
{
	charge_pump_chip_init(CP_TYPE_AUX);
	charge_pump_chip_enable(CP_TYPE_AUX, true);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW_AUX, true);
	di->cp_data.aux_check_cnt = WLDC_CP_AUX_CHECK_CNT;
	di->cp_data.cur_type = CP_TYPE_MULTI;
	hwlog_info("[switch_to_multipath] aux_cp enabled\n");
}

static bool wldc_cp_need_multipath(struct wldc_dev_info *di)
{
	int ibus = 0;
	int count;

	if (di->cp_data.cur_type == CP_TYPE_MULTI)
		return false;

	if (dcm_get_ic_ibus(SC_MODE, CHARGE_IC_MAIN, &ibus))
		return false;

	if ((ibus < di->cp_data.ibus_para.hth) ||
		(di->cp_data.multi_cp_err_cnt >= WLDC_CP_ERR_CNT)) {
		di->cp_data.ibus_para.hth_cnt = 0;
		return false;
	}

	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return false;

	di->cp_data.ibus_para.hth_cnt++;
	if (!di->ctrl_interval)
		return false;
	count = di->cp_data.ibus_para.h_time / di->ctrl_interval;
	if (di->cp_data.ibus_para.hth_cnt < count)
		return false;

	return true;
}

static void wldc_cp_switch_to_singlepath(struct wldc_dev_info *di)
{
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW_AUX, false);
	charge_pump_chip_enable(CP_TYPE_AUX, false);
	di->cp_data.aux_check_cnt = 0;
	di->cp_data.cur_type = CP_TYPE_MAIN;
	hwlog_info("switch to single path succ\n");
}

static bool wldc_cp_need_singlepath(struct wldc_dev_info *di)
{
	int count;
	int ibus = 0;

	if (di->cp_data.cur_type != CP_TYPE_MULTI)
		return false;

	if (dcm_get_ic_ibus(SC_MODE, CHARGE_IC_MAIN, &ibus))
		return false;

	if (ibus > di->cp_data.ibus_para.lth) {
		di->cp_data.ibus_para.lth_cnt = 0;
		return false;
	}

	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return false;

	di->cp_data.ibus_para.lth_cnt++;
	if (!di->ctrl_interval)
		return false;
	count = di->cp_data.ibus_para.l_time / di->ctrl_interval;
	if (di->cp_data.ibus_para.lth_cnt < count)
		return false;

	return true;
}

static void wldc_cp_current_status_check(struct wldc_dev_info *di)
{
	if (!wldc_cp_aux_status_check(di)) {
		di->cp_data.multi_cp_err_cnt++;
		wldc_cp_switch_to_singlepath(di);
		hwlog_err("current_status_check: aux cp status abnormal\n");
	}
}

void wldc_cp_select_charge_path(struct wldc_dev_info *di)
{
	if (!di || !di->cp_data.support_multi_cp ||
		(di->cp_data.multi_cp_err_cnt >= WLDC_CP_ERR_CNT))
		return;

	if (wldc_get_charge_stage() == WLDC_STAGE_STOP_CHARGING)
		return;

	wldc_cp_current_status_check(di);

	if (wldc_cp_need_multipath(di))
		wldc_cp_switch_to_multipath(di);

	if (wldc_cp_need_singlepath(di))
		wldc_cp_switch_to_singlepath(di);
}

void wldc_cp_reset(struct wldc_dev_info *di)
{
	if (!di || !di->cp_data.support_multi_cp)
		return;

	di->cp_data.aux_check_cnt = 0;
	di->cp_data.multi_cp_err_cnt = 0;
	di->cp_data.cur_type = CP_TYPE_MAIN;
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW_AUX, false);
	charge_pump_chip_enable(CP_TYPE_AUX, false);
}
