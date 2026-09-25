/*
 * direct_charger_para_parse.c
 *
 * parameter parse interface for direct charger
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include <huawei_platform/log/hw_log.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charge_para
HWLOG_REGIST();

/* for scharger_v300 cv limit */
#define DC_HI6523_CV_CUT         150

static void direct_charge_parse_gain_current_para(struct device_node *np,
	struct direct_charge_device *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gain_curr_10v2a", &di->gain_curr_10v2a, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gain_curr_10v2p25a", &di->gain_curr_10v2p25a, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gain_curr_10v2p25a_car", &di->gain_curr_10v2p25a_car, 0);
}

static void direct_charge_parse_adp_cur_para(
	struct device_node *np, struct direct_charge_device *di,
	struct direct_charge_adp_cur_para *data, const char *name)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		name, DC_ADP_CUR_LEVEL, DC_ADP_TOTAL);
	if ((array_len < 0) || (array_len > DC_ADP_CUR_LEVEL * DC_ADP_TOTAL))
		return;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			name, i, &tmp_string))
			return;

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return;

		row = i / DC_ADP_TOTAL;
		col = i % DC_ADP_TOTAL;

		switch (col) {
		case DC_ADP_VOL_MIN:
			data[row].vol_min = idata;
			break;
		case DC_ADP_VOL_MAX:
			data[row].vol_max = idata;
			break;
		case DC_ADP_CUR_TH:
			data[row].cur_th = idata;
			break;
		}

		hwlog_info("%s[%d][%d]=%d\n", name, row, col, idata);
	}
}

static void direct_charge_parse_adapter_antifake_para(struct device_node *np,
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
		"adaptor_antifake_key_index",
		&di->adp_antifake_key_index, 1);

	/* set public key as default key in factory mode */
	if (power_cmdline_is_factory_mode())
		di->adp_antifake_key_index = 1;

	hwlog_info("adp_antifake_key_index=%d\n",
		di->adp_antifake_key_index);
}

static int direct_charge_parse_volt_para(struct device_node *np,
	struct direct_charge_device *di, int group)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;
	int j = group;
	char *volt_para = NULL;

	volt_para = di->orig_volt_para_p[j].bat_info.volt_para_index;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		volt_para, DC_VOLT_LEVEL, DC_PARA_VOLT_TOTAL);
	if (array_len < 0)
		return -EINVAL;

	di->orig_volt_para_p[j].stage_size = array_len / DC_PARA_VOLT_TOTAL;
	hwlog_info("%s stage_size=%d\n", volt_para,
		di->orig_volt_para_p[j].stage_size);

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, volt_para, i, &tmp_string))
			return -EINVAL;

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return -EINVAL;

		row = i / DC_PARA_VOLT_TOTAL;
		col = i % DC_PARA_VOLT_TOTAL;

		switch (col) {
		case DC_PARA_VOL_TH:
#ifdef CONFIG_SCHARGER_V300
			if (is_hi6523_cv_limit())
				idata -= DC_HI6523_CV_CUT;
#endif /* CONFIG_SCHARGER_V300 */
			di->orig_volt_para_p[j].volt_info[row].vol_th = idata;
			break;
		case DC_PARA_CUR_TH_HIGH:
			di->orig_volt_para_p[j].volt_info[row].cur_th_high = idata;
			break;
		case DC_PARA_CUR_TH_LOW:
			di->orig_volt_para_p[j].volt_info[row].cur_th_low = idata;
			break;
		}
	}

	di->orig_volt_para_p[j].bat_info.parse_ok = 1;

	for (i = 0; i < di->orig_volt_para_p[j].stage_size; i++)
		hwlog_info("%s[%d]=%d %d %d\n", volt_para, i,
			di->orig_volt_para_p[j].volt_info[i].vol_th,
			di->orig_volt_para_p[j].volt_info[i].cur_th_high,
			di->orig_volt_para_p[j].volt_info[i].cur_th_low);

	return 0;
}

static void direct_charge_parse_group_volt_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i;

	di->stage_size = 0;

	for (i = 0; i < di->stage_group_size; i++) {
		if (direct_charge_parse_volt_para(np, di, i))
			return;
	}

	di->stage_size = di->orig_volt_para_p[0].stage_size;
}

static void direct_charge_parse_bat_para(struct device_node *np,
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
			/* 10: decimal base */
			if (kstrtoint(tmp_string, 10, &idata))
				return;

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("invalid temp_low=%d\n", idata);
				return;
			}

			di->orig_volt_para_p[row].bat_info.temp_low = idata;
			break;
		case DC_PARA_TEMP_HIGH:
			/* 10: decimal base */
			if (kstrtoint(tmp_string, 10, &idata))
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
		}
	}

	for (i = 0; i < di->stage_group_size; i++)
		hwlog_info("bat_para[%d]=%d %d %s\n", i,
			di->orig_volt_para_p[i].bat_info.temp_low,
			di->orig_volt_para_p[i].bat_info.temp_high,
			di->orig_volt_para_p[i].bat_info.volt_para_index);
}

static void direct_charge_parse_std_resist_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"std_resist_para", DC_RESIST_LEVEL, DC_RESIST_TOTAL);
	if (array_len < 0)
		return;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"std_resist_para", i, &tmp_string))
			return;

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return;

		row = i / DC_RESIST_TOTAL;
		col = i % DC_RESIST_TOTAL;

		switch (col) {
		case DC_RESIST_MIN:
			di->std_resist_para[row].resist_min = idata;
			break;
		case DC_RESIST_MAX:
			di->std_resist_para[row].resist_max = idata;
			break;
		case DC_RESIST_CUR_MAX:
			di->std_resist_para[row].resist_cur_max = idata;
			break;
		}

		hwlog_info("std_resist_para[%d][%d]=%d\n", row, col, idata);
	}
}

static void direct_charge_parse_nonstd_resist_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"resist_para", DC_RESIST_LEVEL, DC_RESIST_TOTAL);
	if (array_len < 0)
		return;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"resist_para", i, &tmp_string))
			return;

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return;

		row = i / DC_RESIST_TOTAL;
		col = i % DC_RESIST_TOTAL;

		switch (col) {
		case DC_RESIST_MIN:
			di->nonstd_resist_para[row].resist_min = idata;
			break;
		case DC_RESIST_MAX:
			di->nonstd_resist_para[row].resist_max = idata;
			break;
		case DC_RESIST_CUR_MAX:
			di->nonstd_resist_para[row].resist_cur_max = idata;
			break;
		}

		hwlog_info("nonstd_resist_para[%d][%d]=%d\n", row, col, idata);
	}
}

static void direct_charge_parse_ctc_resist_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"ctc_resist_para", DC_RESIST_LEVEL, DC_RESIST_TOTAL);
	if (array_len < 0)
		return;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"ctc_resist_para", i, &tmp_string))
			return;

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return;

		row = i / DC_RESIST_TOTAL;
		col = i % DC_RESIST_TOTAL;

		switch (col) {
		case DC_RESIST_MIN:
			di->ctc_resist_para[row].resist_min = idata;
			break;
		case DC_RESIST_MAX:
			di->ctc_resist_para[row].resist_max = idata;
			break;
		case DC_RESIST_CUR_MAX:
			di->ctc_resist_para[row].resist_cur_max = idata;
			break;
		}

		hwlog_info("ctc_resist_para[%d][%d]=%d\n", row, col, idata);
	}
}

static void direct_charge_parse_temp_para(struct device_node *np,
	struct direct_charge_device *di)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"temp_para", DC_TEMP_LEVEL, DC_TEMP_TOTAL);
	if (array_len < 0)
		return;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"temp_para", i, &tmp_string))
			return;

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return;

		row = i / DC_TEMP_TOTAL;
		col = i % DC_TEMP_TOTAL;

		switch (col) {
		case DC_TEMP_MIN:
			di->temp_para[row].temp_min = idata;
			break;
		case DC_TEMP_MAX:
			di->temp_para[row].temp_max = idata;
			break;
		case DC_TEMP_CUR_MAX:
			di->temp_para[row].temp_cur_max = idata;

			if (idata > di->iin_thermal_default)
				di->iin_thermal_default = idata;
			break;
		}

		hwlog_info("temp_para[%d][%d]=%d\n", row, col, idata);
	}

	hwlog_info("iin_thermal_default=%d\n", di->iin_thermal_default);
}

static int direct_charge_parse_stage_jump_para(struct device_node *np,
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

		/* 10: decimal base */
		if (kstrtoint(tmp_string, 10, &idata))
			return -EINVAL;

		di->stage_need_to_jump[i] = idata;

		hwlog_info("stage_need_to_jump[%d]=%d\n", i, idata);
	}

	return 0;
}

static int direct_charge_parse_basic_para(struct device_node *np,
	struct direct_charge_device *di)
{
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
		"ui_max_pwr", (u32 *)&di->ui_max_pwr, 0);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"scp_work_on_charger", &di->scp_work_on_charger, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"standard_cable_full_path_res_max",
		(u32 *)&di->std_cable_full_path_res_max, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"full_path_res_max",
		(u32 *)&di->nonstd_cable_full_path_res_max, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ctc_cable_full_path_res_max",
		(u32 *)&di->ctc_cable_full_path_res_max,
		320); /* default is 320mohm */

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_current_for_none_standard_cable",
		(u32 *)&di->max_current_for_nonstd_cable, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_current_for_ctc_cable",
		(u32 *)&di->max_current_for_ctc_cable,
		di->max_current_for_nonstd_cable);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"super_ico_current", (u32 *)&di->super_ico_current,
		4000); /* default is 4000ma */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_show_ico_first", &di->is_show_ico_first, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"use_5A", &di->use_5a, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"use_8A", &di->use_8a, 0);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tadapt", (u32 *)&di->max_tadapt, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tls", (u32 *)&di->max_tls, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibat_abnormal_th", (u32 *)&di->ibat_abnormal_th, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"first_cc_stage_timer_in_min",
		&di->first_cc_stage_timer_in_min, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_err_th", (u32 *)&di->vol_err_th, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_leakage_current_th",
		(u32 *)&di->adaptor_leakage_current_th, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"compensate_r", (u32 *)&di->compensate_r, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_protect", &di->cc_protect, 0);

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_dc_bat_vol", (u32 *)&di->max_dc_bat_vol, 0))
		return -EINVAL;

#ifdef CONFIG_SCHARGER_V300
	if (is_hi6523_cv_limit())
		di->max_dc_bat_vol -= DC_HI6523_CV_CUT;
#endif /* CONFIG_SCHARGER_V300 */

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"min_dc_bat_vol", (u32 *)&di->min_dc_bat_vol, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_adaptor_vset", (u32 *)&di->max_adapter_vset, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"charge_control_interval", &di->charge_control_interval, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"threshold_caculation_interval",
		&di->threshold_caculation_interval, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vstep", (u32 *)&di->vstep, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err", (u32 *)&di->delta_err, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err_10v2p25a", (u32 *)&di->delta_err_10v2p25a,
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
		"rt_curr_th", &di->rt_curr_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt_test_time", &di->rt_test_time, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"startup_iin_limit", &di->startup_iin_limit, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hota_iin_limit", &di->hota_iin_limit, di->startup_iin_limit);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cur_inversion", (u32 *)&di->cur_inversion, 0);

	return 0;
}

int direct_charge_parse_dts(struct device_node *np, void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (!np || !di) {
		hwlog_err("np or di is null\n");
		return -EINVAL;
	}

	hwlog_info("parse_dts\n");

	if (direct_charge_parse_basic_para(np, di))
		return -EINVAL;

	if (direct_charge_parse_stage_jump_para(np, di))
		return -EINVAL;

	direct_charge_parse_temp_para(np, di);
	direct_charge_parse_std_resist_para(np, di);
	direct_charge_parse_nonstd_resist_para(np, di);
	direct_charge_parse_ctc_resist_para(np, di);
	direct_charge_parse_bat_para(np, di);
	direct_charge_parse_group_volt_para(np, di);
	direct_charge_parse_adapter_antifake_para(np, di);
	direct_charge_parse_gain_current_para(np, di);
	direct_charge_parse_adp_cur_para(np, di, di->adp_10v2p25a, "10v2p25a_cur_para");
	direct_charge_parse_adp_cur_para(np, di, di->adp_10v2p25a_car, "10v2p25a_car_cur_para");

	return 0;
}
