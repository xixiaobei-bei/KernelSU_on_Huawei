/*
 * series_batt_speaker_charger_check.c
 *
 * series_batt_speaker_charger_check driver
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
#include <huawei_platform/power/speaker_charger/series_batt_speaker_charger_check.h>
#include <huawei_platform/power/speaker_charger/series_batt_speaker_charge_adapter.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/hardware_monitor/uscp.h>
#include <chipset_common/hwpower/battery/battery_temp.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG series_batt_speaker_charge_check
HWLOG_REGIST();

#define SPEAKER_CHARGER_CHECK_RETRY_TIMES              3

static void series_batt_speaker_charge_check_set_succ_flag(int mode)
{
	switch (mode) {
	case LVC_MODE:
		dc_mmi_set_succ_flag(mode, DC_SUCC);
		return;
	case SC_MODE:
		dc_mmi_set_succ_flag(mode, DC_ERROR_START_CHARGE);
		return;
	default:
		return;
	}
}

static int series_batt_speaker_charge_check_battery_temp(
	struct direct_charge_device *di)
{
	int bat_temp = 0;
	int bat_temp_cur_max;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &bat_temp);
	di->bat_temp_before_charging = bat_temp;
	if ((bat_temp < DC_LOW_TEMP_MAX + di->low_temp_hysteresis) ||
		(bat_temp >= DC_HIGH_TEMP_MAX - di->high_temp_hysteresis)) {
		hwlog_info("can not do scp charging, abnormal battery temp=%d\n",
			bat_temp);
		return -1;
	}

	bat_temp_cur_max = direct_charge_battery_temp_handler(bat_temp);
	if (bat_temp_cur_max == 0) {
		hwlog_info("can not do scp charging, battery temp=%d\n", bat_temp);
		return -1;
	}

	return 0;
}

static int series_batt_speaker_charge_check_battery_voltage(
	struct direct_charge_device *di)
{
	int volt_max, volt_min;

	volt_max = hw_battery_voltage(BAT_ID_MAX);
	volt_min = hw_battery_voltage(BAT_ID_MIN);

	if ((volt_min < di->min_dc_bat_vol) || (volt_max > di->max_dc_bat_vol)) {
		hwlog_info("can not do scp charging, v_min=%d v_max=%d\n",
			volt_min, volt_max);
		return -1;
	}

	return 0;
}

static bool series_batt_speaker_charge_check_enable_status(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return false;

	if ((l_di->sysfs_enable_charger == 0) &&
		!(l_di->sysfs_mainsc_enable_charger ^ l_di->sysfs_auxsc_enable_charger)) {
		dc_mmi_set_succ_flag(l_di->working_mode, DC_ERROR_CHARGE_DISABLED);
		hwlog_info("%d is disabled\n", l_di->working_mode);
		return false;
	}
	return true;
}

static int series_batt_speaker_charge_check_adapter_voltage_accuracy(
	struct direct_charge_device *di)
{
	int vadapt = 0;
	int ibat = 0;
	int vol_err;
	int i;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int error_flag = 0;

	di->adaptor_vset = di->init_adapter_vset;
	dc_set_adapter_voltage(di->adaptor_vset);

	/* delay 500ms */
	usleep_range(500000, 501000);

	/* keep communication with the adaptor within 1 second */
	direct_charge_get_bat_current(&ibat);

	for (i = 0; i < SPEAKER_CHARGER_CHECK_RETRY_TIMES; ++i) {
		if (dc_get_adapter_voltage(&vadapt))
			return -1;

		vol_err = vadapt - di->adaptor_vset;
		if (vol_err < 0)
			vol_err = -vol_err;

		hwlog_info("[%d]: verr=%d, verr_th=%d, vset=%d, vread=%d, vbus=%d\n",
			i, vol_err, di->vol_err_th,
			di->adaptor_vset, vadapt, get_charger_vbus_vol());

		if (vol_err > di->vol_err_th) {
			error_flag = 1;
			break;
		}
	}

	if (error_flag == 0) {
		hwlog_err("adapter voltage accuracy check succ\n");
		return 0;
	}

	/* process error report */
	snprintf(tmp_buf, sizeof(tmp_buf),
		"[%d]: verr=%d > verr_th=%d, vset=%d, vread=%d, vbus=%d\n",
		i, vol_err, di->vol_err_th,
		di->adaptor_vset, vadapt, get_charger_vbus_vol());

	dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_APT_VOLTAGE_ACCURACY, tmp_buf);

	power_dsm_report_dmd(POWER_DSM_BATTERY,
		DSM_DIRECT_CHARGE_VOL_ACCURACY, tmp_buf);

	hwlog_err("adapter voltage accuracy check fail\n");
	return -1;
}

static int series_batt_speaker_charge_security_check(void)
{
	int ret;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	dc_mmi_set_succ_flag(l_di->working_mode, DC_ERROR_ADAPTER_VOLTAGE_ACCURACY);
	ret = series_batt_speaker_charge_check_adapter_voltage_accuracy(l_di);
	if (ret)
		return -1;

	return 0;
}

void series_batt_speaker_charge_mode_check(struct direct_charge_device *di)
{
	unsigned int stage;
	unsigned int local_mode;
	unsigned int path;

	if (!di)
		return;

	if (di->working_mode == LVC_MODE) {
		path = PATH_LVC;
		hwlog_info("lvc mode check begin\n");
	} else if (di->working_mode == SC_MODE) {
		path = PATH_SC;
		hwlog_info("sc mode check begin\n");
	} else {
		hwlog_info("illegal mode\n");
		return;
	}
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_CHECK_START, NULL);

	/* step-1: check local mode */
	local_mode = direct_charge_get_local_mode();
	if (!(local_mode & di->working_mode)) {
		hwlog_err("local mode not support %d\n", di->working_mode);
		return;
	}

	/* step-2: check dc error and report dmd */
	if (di->error_cnt >= DC_ERR_CNT_MAX) {
		hwlog_info("error exceed %d times, dc is disabled\n",
			DC_ERR_CNT_MAX);

		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_CHARGE_DISABLED);
		dc_send_normal_charging_uevent();

		if ((di->dc_err_report_flag == FALSE) &&
			(di->dc_open_retry_cnt <= DC_OPEN_RETRY_CNT_MAX)) {
			dc_show_eh_buf(di->dsm_buff);

			if (di->cc_cable_detect_ok)
				dc_report_eh_buf(di->dsm_buff,
					DSM_DIRECT_CHARGE_ERR_WITH_STD_CABLE);
			else
				dc_report_eh_buf(di->dsm_buff,
					DSM_DIRECT_CHARGE_ERR_WITH_NONSTD_CABLE);

			dc_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
			di->dc_err_report_flag = TRUE;
		}

		return;
	}

	/* step-3: check enable status */
	if (!series_batt_speaker_charge_check_enable_status())
		return;

	/* step-4: check scp protocol status */
	stage = direct_charge_get_stage_status();
	if (stage == DC_STAGE_DEFAULT) {
		if (dc_get_protocol_register_state())
			return;
	}

	/* step-5: check uscp status */
	if (uscp_is_in_protect_mode()) {
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_CHARGE_DISABLED);
		hwlog_err("%d is disabled by uscp\n", di->working_mode);
		return;
	}

	/* step-6: detect cable */
	if (direct_charge_get_stage_status() == DC_STAGE_ADAPTER_DETECT) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_ADAPTER_DETECT);
		dc_detect_cable();
	}

	if (!series_batt_speaker_charge_check_enable_status())
		return;

	/* step-7: update adapter info */
	dc_update_adapter_info();

	/* step-8: check battery temp */
	if (series_batt_speaker_charge_check_battery_temp(di)) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_BAT_TEMP);
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->bat_temp_err_flag = true;
		hwlog_err("temp out of range, try next loop\n");
		return;
	} else {
		di->bat_temp_err_flag = false;
	}

	/* step-9: check battery voltage */
	if (series_batt_speaker_charge_check_battery_voltage(di)) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_BAT_VOL);
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		hwlog_err("volt out of range, try next loop\n");
		return;
	}

	/* step-10: check adapter antifake */
	if (series_batt_speaker_charge_check_adapter_antifake()) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_ADAPTER_ANTI_FAKE);
		direct_charge_set_stage_status(DC_STAGE_DEFAULT);
		di->error_cnt += 1;
		di->adp_antifake_failed_cnt += 1;
		hwlog_err("adapter antifake check failed\n");
		return;
	}
	if (di->adp_antifake_failed_cnt < ADP_ANTIFAKE_CHECK_THLD)
		di->adp_antifake_failed_cnt = 0;

	direct_charge_set_stage_status(DC_STAGE_CHARGE_INIT);

	/* step-11: init adapter and device */
	if (direct_charge_get_stage_status() == DC_STAGE_CHARGE_INIT) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_INIT);
		if (!series_batt_speaker_charge_check_enable_status())
			goto err_out;
		if (series_batt_speaker_charge_init_adapter_and_device() == 0) {
			direct_charge_set_stage_status(DC_STAGE_SECURITY_CHECK);
		} else {
			hwlog_err("charge init failed\n");
			multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
			goto err_out;
		}
	}

	/* step-12: security check */
	if (direct_charge_get_stage_status() == DC_STAGE_SECURITY_CHECK) {
		if (!series_batt_speaker_charge_check_enable_status())
			goto err_out;
		if (series_batt_speaker_charge_security_check() == 0) {
			direct_charge_set_stage_status(DC_STAGE_SUCCESS);
		} else {
			hwlog_err("security check failed\n");
			multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
			goto err_out;
		}
	}

	/* step-13: start charging */
	if (direct_charge_get_stage_status() == DC_STAGE_SUCCESS) {
		series_batt_speaker_charge_check_set_succ_flag(di->working_mode);
		di->dc_succ_flag = DC_SUCCESS;
		if (!series_batt_speaker_charge_check_enable_status())
			goto err_out;
		if (series_batt_speaker_charge_start_charging())
			goto err_out;
	}

	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_CHECK_SUCC, NULL);
	goto end;

err_out:
	direct_charge_set_stop_charging_flag(1);
	series_batt_speaker_charge_stop_charging();
end:
	stage = direct_charge_get_stage_status();
	hwlog_info("direct_charge stage=%d,%s\n", stage,
		direct_charge_get_stage_status_string(stage));
}
