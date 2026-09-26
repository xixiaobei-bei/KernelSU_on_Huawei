/*
 * series_batt_speaker_charger.c
 *
 * series_batt_speaker_charger driver
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

#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/speaker_charger/series_batt_speaker_charger.h>
#include <huawei_platform/power/speaker_charger/series_batt_speaker_charge_adapter.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG series_batt_speaker_charger

HWLOG_REGIST();

bool series_batt_in_speaker_charging_mode(void)
{
	struct direct_charge_device *di = direct_charge_get_di();

	if (!di)
		return false;

	if ((direct_charge_get_stage_status() == DC_STAGE_CHARGING) &&
		di->support_series_bat_mode)
		return true;

	return false;
}

int series_batt_speaker_charge_init_adapter_and_device(void)
{
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	pd_dpm_notify_direct_charge_status(true);

	ret = series_batt_speaker_charge_init_adapter();
	if (ret) {
		snprintf(tmp_buf, sizeof(tmp_buf), "adapter data init fail\n");
		goto fail_init;
	}

	if (dcm_init_ic(l_di->working_mode, l_di->cur_mode)) {
		snprintf(tmp_buf, sizeof(tmp_buf), "device ic init fail\n");
		goto fail_init;
	}

	if (dcm_init_batinfo(l_di->working_mode, l_di->cur_mode)) {
		snprintf(tmp_buf, sizeof(tmp_buf), "device batinfo init fail\n");
		goto fail_init;
	}

	hwlog_info("init adapter and device succ\n");
	return 0;

fail_init:
	pd_dpm_notify_direct_charge_status(false);

	dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff),
		DC_EH_INIT_DIRECT_CHARGE, tmp_buf);

	return -1;
}

int series_batt_speaker_charge_get_iin_limit(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();
	int idx;

	if (!l_di)
		return -1;

	idx = (l_di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	return l_di->sysfs_iin_thermal_array[idx];
}

int series_batt_speaker_charge_get_iin_power(int *iin_val, int *max_power)
{
	int adapter_type;

	if (!iin_val || !max_power)
		return -1;

	adapter_type = dc_get_adapter_type();
	switch (adapter_type) {
	case ADAPTER_TYPE_5V4P5A:
		*iin_val = DC_ADAPTER_CURRENT_4P5A;
		*max_power = DC_ADAPTER_MAX_POWER_22P5W;
		break;
	case ADAPTER_TYPE_10V2A:
		*iin_val = DC_ADAPTER_CURRENT_2A;
		*max_power = DC_ADAPTER_MAX_POWER_20W;
		break;
	case ADAPTER_TYPE_10V2P25A:
	case ADAPTER_TYPE_10V2P25A_CAR:
	case ADAPTER_TYPE_QTR_A_10V2P25A:
		*iin_val = DC_ADAPTER_CURRENT_2P25A;
		*max_power = DC_ADAPTER_MAX_POWER_22P5W;
		break;
	case ADAPTER_TYPE_10V4A:
	case ADAPTER_TYPE_10V4A_BANK:
	case ADAPTER_TYPE_10V4A_CAR:
	case ADAPTER_TYPE_QTR_C_10V4A:
	case ADAPTER_TYPE_QTR_A_10V4A:
		*iin_val = DC_ADAPTER_CURRENT_2P25A;
		*max_power = DC_ADAPTER_MAX_POWER_22P5W;
		break;
	default:
		*iin_val = DC_ADAPTER_CURRENT_2A;
		*max_power = DC_ADAPTER_MAX_POWER_20W;
		break;
	}

	return 0;
}

static int series_batt_speaker_charge_config_adapter(int type)
{
	int vset;
	int iset;
	int ret;

	switch (type) {
	case ADAPTER_TYPE_5V4P5A:
		vset = DC_ADAPTER_VOLTAGE_5V;
		iset = DC_ADAPTER_CURRENT_4P5A;
		break;
	case ADAPTER_TYPE_10V2A:
		vset = DC_ADAPTER_VOLTAGE_10V;
		iset = DC_ADAPTER_CURRENT_2A;
		break;
	case ADAPTER_TYPE_10V2P25A:
	case ADAPTER_TYPE_10V2P25A_CAR:
	case ADAPTER_TYPE_QTR_A_10V2P25A:
		vset = DC_ADAPTER_VOLTAGE_10V;
		iset = DC_ADAPTER_CURRENT_2P25A;
		break;
	case ADAPTER_TYPE_10V4A:
	case ADAPTER_TYPE_10V4A_BANK:
	case ADAPTER_TYPE_10V4A_CAR:
	case ADAPTER_TYPE_QTR_C_10V4A:
	case ADAPTER_TYPE_QTR_A_10V4A:
		vset = DC_ADAPTER_VOLTAGE_10V;
		iset = DC_ADAPTER_CURRENT_4A;
		break;
	case ADAPTER_TYPE_UNKNOWN:
		vset = DC_ADAPTER_VOLTAGE_5V;
		iset = DC_ADAPTER_CURRENT_2A;
		break;
	default:
		vset = DC_ADAPTER_VOLTAGE_10V;
		iset = DC_ADAPTER_CURRENT_2A;
		break;
	}

	ret = dc_set_adapter_voltage(vset);
	if (ret < 0)
		return ret;
	power_usleep(DT_USLEEP_5MS);
	ret = dc_set_adapter_current(iset);
	msleep(DT_MSLEEP_500MS);

	return ret;
}

static bool series_batt_speaker_charge_is_dc_support(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);
	if (!lvc_di && !sc_di) {
		hwlog_info("unsupport direct charge");
		return false;
	}

	return true;
}

static void series_batt_speaker_charge_reset_para_in_stop(struct direct_charge_device *di)
{
	di->error_cnt = 0;
	di->otp_cnt = 0;
	di->adp_otp_cnt = 0;
	di->reverse_ocp_cnt = 0;
	di->dc_open_retry_cnt = 0;
	di->full_path_resistance = DC_ERROR_RESISTANCE;
	di->second_path_resistance = DC_ERROR_RESISTANCE;
	di->second_resist_check_ok = false;
	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_ADAPTER_DETECT);
	dc_mmi_set_test_flag(false);
	di->dc_succ_flag = DC_ERROR;
	di->quick_charge_flag = 0;
	di->super_charge_flag = 0;
	di->cc_cable_detect_ok = 0;
	di->dc_err_report_flag = FALSE;
	di->sc_conv_ocp_count = 0;
	di->low_temp_hysteresis = 0;
	di->high_temp_hysteresis = 0;
	di->cable_type = DC_UNKNOWN_CABLE;
	di->orig_cable_type = DC_UNKNOWN_CABLE;
	di->max_pwr = 0;
	di->pri_inversion = false;
	direct_charge_set_abnormal_adp_flag(false);
	dc_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
}

static void series_batt_speaker_charge_check_vbus_state(void)
{
	int vbus = 0;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);

	/* judging whether the adapter is disconnect */
	direct_charge_get_device_vbus(&vbus);
	if (vbus < VBUS_ON_THRESHOLD) {
		hwlog_info("adapter disconnect\n");

		if (!power_cmdline_is_factory_mode())
			direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
				DC_DISABLE_SYS_NODE);

		direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
			DC_DISABLE_WIRELESS_TX);
		if (lvc_di)
			series_batt_speaker_charge_reset_para_in_stop(lvc_di);

		if (sc_di)
			series_batt_speaker_charge_reset_para_in_stop(sc_di);

		wired_disconnect_send_icon_uevent();
	}
}

static void series_batt_speaker_charge_ic_reset(struct direct_charge_device *di)
{
	if (dcm_exit_ic(di->working_mode, CHARGE_MULTI_IC))
		hwlog_err("ic exit fail\n");

	if (dcm_exit_batinfo(di->working_mode, di->cur_mode))
		hwlog_err("batinfo exit fail\n");

	if (dc_reset_operate(DC_RESET_MASTER))
		hwlog_err("soft reset master fail\n");
}

static void series_batt_speaker_charge_clear_resource(struct direct_charge_device *di)
{
	int i;

	/* reset parameter and release resource */
	direct_charge_set_stop_charging_flag(0);
	dc_mmi_set_test_flag(false);
	di->scp_stop_charging_flag_info = 0;
	di->cur_stage = 0;
	di->pre_stage = 0;
	di->vbat = 0;
	di->ibat = 0;
	di->vadapt = 0;
	di->iadapt = 0;
	di->ls_vbus = 0;
	di->ls_ibus = 0;
	di->compensate_v = 0;
	di->ibat_abnormal_cnt = 0;
	di->max_adaptor_cur = 0;
	di->force_single_path_flag = false;
	memset(&di->limit_max_pwr, 0, sizeof(di->limit_max_pwr));
	di->scp_stop_charging_complete_flag = 1;
	di->adaptor_test_result_type = AT_TYPE_OTHER;
	di->working_mode = UNDEFINED_MODE;
	di->cur_mode = CHARGE_IC_MAIN;
	if (di->multi_ic_check_info.limit_current < 0)
		di->multi_ic_check_info.limit_current = di->iin_thermal_default;

	for (i = 0; i < DC_MODE_TOTAL; i++)
		di->rt_test_para[i].rt_test_result = false;
}

int series_batt_speaker_charge_start_charging(void)
{
	int adapter_type;
	struct direct_charge_device *di = direct_charge_get_di();

	if (!di)
		return -1;

	charge_send_uevent(VCHRG_START_AC_CHARGING_EVENT);
	di->can_stop_kick_wdt = 0;
	adapter_test_set_result(di->adaptor_test_result_type,
		AT_PROTOCOL_FINISH_SUCC);

	/* config adapter vset and iset_bound by adapter type */
	adapter_type = dc_get_adapter_type();
	if (series_batt_speaker_charge_config_adapter(adapter_type)) {
		hwlog_err("start charging fail\n");
		return -1;
	}

	direct_charge_set_stage_status(DC_STAGE_CHARGING);
	di->pri_inversion = false;

	hwlog_info("start charging success\n");
	return 0;
}

void series_batt_speaker_charge_stop_charging(void)
{
	bool stage_default = false;
	struct direct_charge_device *di = direct_charge_get_di();

	if (!di)
		return;

	if (!series_batt_speaker_charge_is_dc_support())
		return;

	if (di->stop_charging_flag_error)
		di->error_cnt += 1;

	if (di->stop_charging_flag_error ||
		di->scp_stop_charging_flag_info ||
		!di->sysfs_enable_charger) {
		if (!power_cmdline_is_factory_mode())
			direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		else
			stage_default = true;
	}

	if (dcm_enable_ic(di->working_mode, di->cur_mode, DC_IC_DISABLE))
		hwlog_err("ic enable fail\n");

	if (dc_set_adapter_default())
		hwlog_err("set adapter default state fail\n");

	pd_dpm_notify_direct_charge_status(false);
	if (di->scp_work_on_charger) {
		dc_adapter_protocol_power_supply(DISABLE);
		charge_set_hiz_enable_by_direct_charge(HIZ_MODE_DISABLE);
	}

	msleep(DT_MSLEEP_20MS);
	series_batt_speaker_charge_check_vbus_state();
	series_batt_speaker_charge_ic_reset(di);
	if (stage_default)
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);

	series_batt_speaker_charge_clear_resource(di);
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_STOP_CHARGE, NULL);
	if (di->pri_inversion) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		charge_request_charge_monitor();
	}
}
