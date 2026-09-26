/*
 * direct_charge_para_parse.c
 *
 * parameter parse interface for direct charge
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
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_para
HWLOG_REGIST();

/* for scharger_v300 cv limit */
#define DC_HI6523_CV_CUT         150

static void dc_parse_gain_current_para(struct device_node *np,
	struct direct_charge_device *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gain_curr_10v2a", &di->gain_curr_10v2a, 0);
}

static void dc_parse_adp_cur_para(struct device_node *np,
	struct direct_charge_adp_cur_para *data, const char *name)
{
	int row, col, len;
	int idata[DC_ADP_CUR_LEVEL * DC_ADP_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, DC_ADP_CUR_LEVEL, DC_ADP_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_ADP_TOTAL; row++) {
		col = row * DC_ADP_TOTAL + DC_ADP_VOL_MIN;
		data[row].vol_min = idata[col];
		col = row * DC_ADP_TOTAL + DC_ADP_VOL_MAX;
		data[row].vol_max = idata[col];
		col = row * DC_ADP_TOTAL + DC_ADP_CUR_TH;
		data[row].cur_th = idata[col];
	}
}

static void dc_parse_adapter_antifake_para(struct device_node *np,
	struct direct_charge_device *di)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_check_enable",
		&di->adp_antifake_enable, 0))
		return;

	if (di->adp_antifake_enable != 1) {
		di->adp_antifake_enable = 0;
		return;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_execute_enable",
		&di->adp_antifake_execute_enable, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_key_index",
		&di->adp_antifake_key_index, 1);

	/* set public key as default key in factory mode */
	if (power_cmdline_is_factory_mode())
		di->adp_antifake_key_index = 1;

	hwlog_info("adp_antifake_key_index=%d\n",
		di->adp_antifake_key_index);
}

static int dc_parse_volt_para(struct device_node *np,
	struct direct_charge_device *di, int group)
{
	int i, row, col, len;
	int idata[DC_VOLT_LEVEL * DC_PARA_VOLT_TOTAL] = { 0 };
	int j = group;
	char *volt_para = NULL;

	volt_para = di->orig_volt_para_p[j].bat_info.volt_para_index;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		volt_para, idata, DC_VOLT_LEVEL, DC_PARA_VOLT_TOTAL);
	if (len < 0)
		return -EINVAL;

	di->orig_volt_para_p[j].stage_size = len / DC_PARA_VOLT_TOTAL;
	hwlog_info("%s stage_size=%d\n", volt_para,
		di->orig_volt_para_p[j].stage_size);

	for (row = 0; row < len / DC_PARA_VOLT_TOTAL; row++) {
		col = row * DC_PARA_VOLT_TOTAL + DC_PARA_VOL_TH;
#ifdef CONFIG_SCHARGER_V300
		if (is_hi6523_cv_limit())
			idata[col] -= DC_HI6523_CV_CUT;
#endif /* CONFIG_SCHARGER_V300 */
		di->orig_volt_para_p[j].volt_info[row].vol_th = idata[col];
		col = row * DC_PARA_VOLT_TOTAL + DC_PARA_CUR_TH_HIGH;
		di->orig_volt_para_p[j].volt_info[row].cur_th_high = idata[col];
		col = row * DC_PARA_VOLT_TOTAL + DC_PARA_CUR_TH_LOW;
		di->orig_volt_para_p[j].volt_info[row].cur_th_low = idata[col];
	}

	di->orig_volt_para_p[j].bat_info.parse_ok = 1;

	for (i = 0; i < di->orig_volt_para_p[j].stage_size; i++)
		hwlog_info("%s[%d]=%d %d %d\n", volt_para, i,
			di->orig_volt_para_p[j].volt_info[i].vol_th,
			di->orig_volt_para_p[j].volt_info[i].cur_th_high,
			di->orig_volt_para_p[j].volt_info[i].cur_th_low);

	return 0;
}

static void dc_parse_group_volt_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;

	di->stage_size = 0;

	for (i = 0; i < di->stage_group_size; i++) {
		if (dc_parse_volt_para(np, di, i))
			return;
	}

	di->stage_size = di->orig_volt_para_p[0].stage_size;
}

static void dc_parse_bat_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	di->stage_group_cur = 0;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"bat_para", DC_VOLT_GROUP_MAX, DC_PARA_BAT_TOTAL);
	if (array_len < 0) {
		di->stage_group_size = 1;
		 /* default temp_high is 45 centigrade */
		di->orig_volt_para_p[0].bat_info.temp_high = 45;
		 /* default temp_low is 10 centigrade */
		di->orig_volt_para_p[0].bat_info.temp_low = 10;
		strncpy(di->orig_volt_para_p[0].bat_info.batid,
			"default", DC_BAT_BRAND_LEN_MAX - 1);
		strncpy(di->orig_volt_para_p[0].bat_info.volt_para_index,
			"volt_para", DC_VOLT_NODE_LEN_MAX - 1);
		return;
	}

	di->stage_group_size = array_len / DC_PARA_BAT_TOTAL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "bat_para", i, &tmp_string))
			return;

		row = i / DC_PARA_BAT_TOTAL;
		col = i % DC_PARA_BAT_TOTAL;

		switch (col) {
		case DC_PARA_BAT_ID:
			strncpy(di->orig_volt_para_p[row].bat_info.batid,
				tmp_string, DC_BAT_BRAND_LEN_MAX - 1);
			break;
		case DC_PARA_TEMP_LOW:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return;

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("invalid temp_low=%d\n", idata);
				return;
			}

			di->orig_volt_para_p[row].bat_info.temp_low = idata;
			break;
		case DC_PARA_TEMP_HIGH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return;

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("invalid temp_high=%d\n", idata);
				return;
			}

			di->orig_volt_para_p[row].bat_info.temp_high = idata;
			break;
		case DC_PARA_INDEX:
			strncpy(di->orig_volt_para_p[row].bat_info.volt_para_index,
				tmp_string, DC_VOLT_NODE_LEN_MAX - 1);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < di->stage_group_size; i++)
		hwlog_info("bat_para[%d]=%d %d %s\n", i,
			di->orig_volt_para_p[i].bat_info.temp_low,
			di->orig_volt_para_p[i].bat_info.temp_high,
			di->orig_volt_para_p[i].bat_info.volt_para_index);
}

static void dc_parse_resist_para(struct device_node *np,
	struct direct_charge_resist_para *data, const char *name)
{
	int row, col, len;
	int idata[DC_RESIST_LEVEL * DC_RESIST_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, DC_RESIST_LEVEL, DC_RESIST_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_RESIST_TOTAL; row++) {
		col = row * DC_RESIST_TOTAL + DC_RESIST_MIN;
		data[row].resist_min = idata[col];
		col = row * DC_RESIST_TOTAL + DC_RESIST_MAX;
		data[row].resist_max = idata[col];
		col = row * DC_RESIST_TOTAL + DC_RESIST_CUR_MAX;
		data[row].resist_cur_max = idata[col];
	}
}

static void dc_parse_temp_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int row, col, len;
	int idata[DC_TEMP_LEVEL * DC_TEMP_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"temp_para", idata, DC_TEMP_LEVEL, DC_TEMP_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_TEMP_TOTAL; row++) {
		col = row * DC_TEMP_TOTAL + DC_TEMP_MIN;
		di->temp_para[row].temp_min = idata[col];
		col = row * DC_TEMP_TOTAL + DC_TEMP_MAX;
		di->temp_para[row].temp_max = idata[col];
		col = row * DC_TEMP_TOTAL + DC_TEMP_CUR_MAX;
		di->temp_para[row].temp_cur_max = idata[col];
		if (idata[col] > di->iin_thermal_default)
			di->iin_thermal_default = idata[col];
	}

	hwlog_info("iin_thermal_default=%d\n", di->iin_thermal_default);
}

static int dc_parse_stage_jump_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i, array_len, idata;
	const char *tmp_string = NULL;

	/* 2: cc and cv stage */
	for (i = 0; i < 2 * DC_VOLT_LEVEL; ++i)
		di->stage_need_to_jump[i] = -1;

	/* 2: cc and cv stage */
	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"stage_need_to_jump", 2 * DC_VOLT_LEVEL, 1);
	if (array_len < 0)
		return -EINVAL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"stage_need_to_jump", i, &tmp_string))
			return -EINVAL;

		if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
			return -EINVAL;

		di->stage_need_to_jump[i] = idata;

		hwlog_info("stage_need_to_jump[%d]=%d\n", i, idata);
	}

	return 0;
}

static void dc_parse_rt_test_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int row, col, len;
	int idata[DC_MODE_TOTAL * DC_RT_TEST_INFO_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"rt_test_para", idata, DC_MODE_TOTAL, DC_RT_TEST_INFO_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_RT_TEST_INFO_TOTAL; row++) {
		col = row * DC_RT_TEST_INFO_TOTAL + DC_RT_CURR_TH;
		di->rt_test_para[row].rt_curr_th = idata[col];
		col = row * DC_RT_TEST_INFO_TOTAL + DC_RT_TEST_TIME;
		di->rt_test_para[row].rt_test_time = idata[col];
	}
}

static void dc_parse_basic_para(struct device_node *np,
	struct direct_charge_device *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_series_bat_mode", &di->support_series_bat_mode, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bat_cell_num", &di->bat_cell_num, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adapter_watchdog_time", &di->adapter_watchdog_time, 3); /* default is 3s */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adapter_iwatt_iset", &di->adapter_iwatt_iset, 400); /* default is 400ma */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"need_wired_sw_off", &di->need_wired_sw_off, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_detect_by_voltage", &di->adaptor_detect_by_voltage, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dc_volt_ratio", (u32 *)&di->dc_volt_ratio, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"init_adapter_vset", (u32 *)&di->init_adapter_vset,
		4400); /* default is 4400mv */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"init_delt_vset", (u32 *)&di->init_delt_vset,
		300); /* default is 300mv */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"init_delt_iset", (u32 *)&di->init_delt_iset,
		0); /* default is 0ma */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ui_max_pwr", (u32 *)&di->ui_max_pwr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"product_max_pwr", (u32 *)&di->product_max_pwr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"second_resist_check_en", (u32 *)&di->second_resist_check_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"scp_work_on_charger", &di->scp_work_on_charger, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"standard_cable_full_path_res_max",
		(u32 *)&di->std_cable_full_path_res_max, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"full_path_res_max",
		(u32 *)&di->nonstd_cable_full_path_res_max, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ctc_cable_full_path_res_max",
		(u32 *)&di->ctc_cable_full_path_res_max,
		320); /* default is 320mohm */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_current_for_none_standard_cable",
		(u32 *)&di->max_current_for_nonstd_cable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_current_for_ctc_cable",
		(u32 *)&di->max_current_for_ctc_cable,
		di->max_current_for_nonstd_cable);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"second_path_res_report_th",
		(u32 *)&di->second_path_res_report_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"super_ico_current", (u32 *)&di->super_ico_current,
		4000); /* default is 4000ma */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_show_ico_first", &di->is_show_ico_first, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_send_cable_type", &di->is_send_cable_type, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"use_5A", &di->use_5a, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"use_8A", &di->use_8a, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tadapt", (u32 *)&di->max_tadapt, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tls", (u32 *)&di->max_tls, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibat_abnormal_th", (u32 *)&di->ibat_abnormal_th, 0);
	if (power_cmdline_is_factory_mode())
		di->ibat_abnormal_th = 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"first_cc_stage_timer_in_min",
		&di->first_cc_stage_timer_in_min, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_err_th", (u32 *)&di->vol_err_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_leakage_current_th",
		(u32 *)&di->adaptor_leakage_current_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"compensate_r", (u32 *)&di->compensate_r, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_protect", &di->cc_protect, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_dc_bat_vol", (u32 *)&di->max_dc_bat_vol, 0);
#ifdef CONFIG_SCHARGER_V300
	if (is_hi6523_cv_limit())
		di->max_dc_bat_vol -= DC_HI6523_CV_CUT;
#endif /* CONFIG_SCHARGER_V300 */

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"min_dc_bat_vol", (u32 *)&di->min_dc_bat_vol, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_adaptor_vset", (u32 *)&di->max_adapter_vset, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"charge_control_interval", &di->charge_control_interval, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"threshold_caculation_interval",
		&di->threshold_caculation_interval, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vstep", (u32 *)&di->vstep, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err", (u32 *)&di->delta_err, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err_10v2p25a", (u32 *)&di->delta_err_10v2p25a,
		(u32)di->delta_err);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err_10v4a", (u32 *)&di->delta_err_10v4a,
		(u32)di->delta_err);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_cable_detect_enable", &di->cc_cable_detect_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"reset_adap_volt_enabled", &di->reset_adap_volt_enabled, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"low_temp_hysteresis", &di->orig_low_temp_hysteresis, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"high_temp_hysteresis", &di->orig_high_temp_hysteresis, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"startup_iin_limit", &di->startup_iin_limit, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hota_iin_limit", &di->hota_iin_limit, di->startup_iin_limit);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cur_inversion", (u32 *)&di->cur_inversion, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_unsafe_sc_enable", (u32 *)&di->cc_unsafe_sc_enable, 0);
}

static void dc_parse_multi_ic_para(struct device_node *np,
	struct direct_charge_device *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_multi_ic", (u32 *)&di->multi_ic_mode_para.support_multi_ic, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"multi_ic_ibat_th", (u32 *)&di->multi_ic_ibat_th, DC_MULTI_IC_IBAT_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"single_ic_ibat_th", (u32 *)&di->single_ic_ibat_th,
		DC_SINGLEIC_CURRENT_LIMIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"curr_offset", (u32 *)&di->curr_offset, DC_CURRENT_OFFSET);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"multi2single_ibat_delta", (u32 *)&di->multi2single_ibat_delta, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"use_higher_vbat", (u32 *)&di->use_higher_vbat, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_select_temp", &di->multi_ic_mode_para.support_select_temp, 0);

	multi_ic_parse_check_para(np, &di->multi_ic_check_info);
}

static void dc_parse_time_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int row, col, len;
	int idata[DC_TIME_PARA_LEVEL * DC_TIME_INFO_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"time_para", idata, DC_TIME_PARA_LEVEL, DC_TIME_INFO_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_TIME_INFO_MAX; row++) {
		col = row * DC_TIME_INFO_MAX + DC_TIME_INFO_TIME_TH;
		di->time_para[row].time_th = idata[col];
		col = row * DC_TIME_INFO_MAX + DC_TIME_INFO_IBAT_MAX;
		di->time_para[row].ibat_max = idata[col];
	}

	di->time_para_parse_ok = 1;
}

static void dc_parse_vstep_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int row, col, len;
	int idata[DC_VSTEP_PARA_LEVEL * DC_VSTEP_INFO_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"vstep_para", idata, DC_VSTEP_PARA_LEVEL, DC_VSTEP_INFO_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_VSTEP_INFO_MAX; row++) {
		col = row * DC_VSTEP_INFO_MAX + DC_VSTEP_INFO_CURR_GAP;
		di->vstep_para[row].curr_gap = idata[col];
		col = row * DC_VSTEP_INFO_MAX + DC_VSTEP_INFO_VSTEP;
		di->vstep_para[row].vstep = idata[col];
	}
}

static void dc_adapter_max_power_time_para(
	struct device_node *np, struct direct_charge_device *di)
{
	int len, row, col;
	int idata[DC_MAX_POWER_TIME_PARA_LEVEL * DC_MAX_POWER_PARA_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"adapter_max_power_time_para", idata, DC_MAX_POWER_TIME_PARA_LEVEL,
		DC_MAX_POWER_PARA_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_MAX_POWER_PARA_MAX; row++) {
		col = row * DC_MAX_POWER_PARA_MAX + DC_ADAPTER_TYPE;
		di->max_power_time[row].adatper_type = idata[col];
		col = row * DC_MAX_POWER_PARA_MAX + DC_MAX_POWER_TIME;
		di->max_power_time[row].max_power_time = idata[col];
		col = row * DC_MAX_POWER_PARA_MAX + DC_MAX_POWER_LIMIT_CURRENT;
		di->max_power_time[row].limit_current = idata[col];
	}
}

static void dc_parse_avoid_dts(struct device_node *np,
	struct direct_charge_device *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"use_force_sleep", &di->use_force_sleep, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cam_flash_stop", &di->cam_flash_stop, 1);
}

int dc_parse_dts(struct device_node *np, void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (!np || !di) {
		hwlog_err("np or di is null\n");
		return -EINVAL;
	}

	hwlog_info("parse_dts\n");

	if (dc_parse_stage_jump_para(np, di))
		return -EINVAL;

	dc_parse_basic_para(np, di);
	dc_parse_temp_para(np, di);
	dc_parse_resist_para(np, di->std_resist_para, "std_resist_para");
	dc_parse_resist_para(np, di->second_resist_para, "second_resist_para");
	dc_parse_resist_para(np, di->ctc_second_resist_para, "ctc_second_resist_para");
	dc_parse_resist_para(np, di->nonstd_resist_para, "resist_para");
	dc_parse_resist_para(np, di->ctc_resist_para, "ctc_resist_para");
	dc_parse_bat_para(np, di);
	dc_parse_group_volt_para(np, di);
	dc_parse_adapter_antifake_para(np, di);
	dc_parse_gain_current_para(np, di);
	dc_parse_adp_cur_para(np, di->adp_10v2p25a, "10v2p25a_cur_para");
	dc_parse_adp_cur_para(np, di->adp_10v2p25a_car, "10v2p25a_car_cur_para");
	dc_parse_adp_cur_para(np, di->adp_qtr_a_10v2p25a, "adp_qtr_a_10v2p25a_cur_para");
	dc_parse_adp_cur_para(np, di->adp_qtr_c_20v3a, "adp_qtr_c_20v3a_cur_para");
	dc_parse_adp_cur_para(np, di->adp_10v4a, "10v4a_cur_para");
	dc_parse_multi_ic_para(np, di);
	dc_parse_time_para(np, di);
	dc_parse_vstep_para(np, di);
	dc_adapter_max_power_time_para(np, di);
	dc_parse_rt_test_para(np, di);
	dc_comp_parse_dts(np, &di->comp_para);
	dc_parse_avoid_dts(np, di);
	di->vterm_dec = BASP_VTERM_DEC_DEFAULT;
	di->ichg_ratio = BASP_ICHG_RATIO_DEFAULT;
	return 0;
}
