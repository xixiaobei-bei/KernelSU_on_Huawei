/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: get dts info for coul core method
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/of.h>
#include <securec.h>
#include "coul_nv.h"
#include "coul_temp.h"
#include "coul_dts.h"

#define ZERO_V_MAX      3200
#define ZERO_V_MIN      2800
#define DEFAULT_HKADC_BATT_TEMP         10
#define DEFAULT_HKADC_BATT_ID           11
#define DEFAULT_SOC_MONITOR_LIMIT       100
#define TEMP_TOO_HOT            60
#define TEMP_TOO_COLD           (-20)
#define LOW_INT_VOL_COUNT       3
#define BASE_OF_SIMPLE_STRTOL   10
#define DEFAULT_DISCHG_OCV_SOC  5
#define DEFAULT_SOC_AT_TERM     100
#define WAKELOCK_LOW_BATT_SOC   3

/* get dts info */
static void check_low_temp_opt(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "low_temp_opt_enable",
		(unsigned int *)&di->low_temp_opt_flag);
	if (ret || (di->low_temp_opt_flag != LOW_TEMP_OPT_OPEN))
		di->low_temp_opt_flag = LOW_TEMP_OPT_CLOSE;
	coul_core_info("low_temp_opt: low temp soc show optimize is %d\n",
		di->low_temp_opt_flag);
}

static void get_multi_ocv_open_flag(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "multi_ocv_open",
		(unsigned int *)&di->multi_ocv_open_flag);
	if (ret)
		di->multi_ocv_open_flag = 0;
	coul_core_info("multi_ocv_open: flag is %d\n", di->multi_ocv_open_flag);
}

static void get_fcc_update_limit_flag(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "fcc_update_limit",
		(unsigned int *)&di->fcc_update_limit_flag);
	if (ret)
		di->fcc_update_limit_flag = 0;
	coul_core_info("fcc_update_limit: flag is %d\n",
		       di->fcc_update_limit_flag);
}

static void get_current_full_enable(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;
	unsigned int enable_current_full = 0;

	ret = of_property_read_u32(np, "current_full_enable",
		&enable_current_full);
	if (ret)
		coul_core_err("dts: can not get current_full_enable, use default : %u\n",
			enable_current_full);
	di->enable_current_full = enable_current_full;
	coul_core_info("dts:get enable_current_full = %u!\n",
		       di->enable_current_full);
}

static void get_cutoff_vol_mv(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "normal_cutoff_vol_mv", &di->v_cutoff);
	if (ret)
		di->v_cutoff = BATTERY_NORMAL_CUTOFF_VOL;
	ret = of_property_read_u32(np, "sleep_cutoff_vol_mv",
		&di->v_cutoff_sleep);
	if (ret)
		di->v_cutoff_sleep = BATTERY_VOL_2_PERCENT;
	ret = of_property_read_u32(np, "low_temp_cutoff_vol_mv",
		(unsigned int *)&di->v_cutoff_low_temp);
	if (ret)
		di->v_cutoff_low_temp = BATTERY_NORMAL_CUTOFF_VOL;
	coul_core_err("get_cutoff_vol_mv: cutoff = %u, cutoff_sleep = %u, low_temp_vol = %d\n",
		di->v_cutoff, di->v_cutoff_sleep, di->v_cutoff_low_temp);
}

static void get_ocv_enable_flag(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "dischg_ocv_enable",
		(unsigned int *)&di->dischg_ocv_enable);
	if (ret)
		di->dischg_ocv_enable = 0;
	coul_core_info("dischg_ocv_limit: flag is %u\n", di->dischg_ocv_enable);
}

static void get_dischg_ocv_soc(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "dischg_ocv_soc",
		(unsigned int *)&di->dischg_ocv_soc);
	if (ret)
		/* if not configured, default value is 5 percent */
		di->dischg_ocv_soc = DEFAULT_DISCHG_OCV_SOC;
	coul_core_info("dischg_ocv_soc: flag is %d\n", di->dischg_ocv_soc);
}

static void get_ocv_data_check_flag(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "check_ocv_data_enable",
		(unsigned int *)&di->check_ocv_data_enable);
	if (ret) {
		di->check_ocv_data_enable = 0;
		coul_core_err("dts:get ocv data check flag fail\n");
	}
	coul_core_info("ocv_data_check_flag is %u\n", di->check_ocv_data_enable);
}

static void get_polar_supply_dts(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;
	unsigned int i, len;
	unsigned int stru_size = sizeof(di->supply_info) / sizeof(int);
	unsigned long max_size = (unsigned long)stru_size * SUPPLY_PARA_LEVEL;
	unsigned int temp_info[max_size];

	ret = of_property_read_u32(np, "polar_ocv_enable",
		(unsigned int *)&di->polar_ocv_enable);
	if (ret) {
		di->polar_ocv_enable = 0;
		coul_core_err("can't get polar_ocv enable dts\n");
		return;
	}
	ret = of_property_read_u32(np, "supply_enable",
		(unsigned int *)&di->supply_enable);
	if (ret) {
		di->supply_enable = 0;
		coul_core_err("can't get supply enable dts\n");
		return;
	}
	len = (unsigned int)of_property_count_u32_elems(np, "supply_table");
	if (len > max_size) {
		di->supply_enable = 0;
		coul_core_err("supply dismatch:%u,size:%lu\n", len, max_size);
		return;
	}

	memset_s(temp_info, max_size, 0, max_size);
	ret = of_property_read_u32_array(np, "supply_table", temp_info, max_size);
	if (ret) {
		di->supply_enable = 0;
		coul_core_err("can't get polar supply dts:%d\n", ret);
		return;
	}
	coul_core_info("%s success: flag is %u\n", __func__, di->supply_enable);
	ret = memcpy_s(di->supply_info, sizeof(di->supply_info),
		       temp_info, sizeof(temp_info));
	if (ret) {
		coul_core_err("%s:memcpy fail\n", __func__);
		return;
	}
	for (i = 0; i < SUPPLY_PARA_LEVEL; i++)
		/* get data of 5 elements with offset from 0 ~ 4 */
		coul_core_info("supply[%d]:%d~%d:%d\n", i,
			di->supply_info[i].soc_min, di->supply_info[i].soc_max,
			di->supply_info[i].temp_back);
}

static void get_uuc_vol_dts(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "uuc_zero_vol", (u32 *)&di->uuc_zero_vol);
	if (ret)
		di->uuc_zero_vol = ZERO_V_MAX;

	if (di->uuc_zero_vol < ZERO_V_MIN)
		di->uuc_zero_vol = ZERO_V_MIN;

	coul_core_info("uuc_zero_vol:%d\n", di->uuc_zero_vol);
}

static void get_bci_dts_info(struct smartstar_coul_device *di)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,bci_battery");
	if (np == NULL) {
		di->is_board_type = BAT_BOARD_SFT;
		di->battery_is_removable = 0;
		return;
	}
	ret = of_property_read_u32(np, "battery_board_type", &di->is_board_type);
	if (ret) {
		di->is_board_type = BAT_BOARD_SFT;
		coul_core_err("dts:get board type fail\n");
	}
	ret = of_property_read_u32(np, "battery_is_removable",
		&di->battery_is_removable);
	if (ret) {
		di->battery_is_removable = 0;
		coul_core_err("dts:get battery_is_removable fail\n");
	}
	coul_core_err("dts:get board type is %u, battery removable flag is %u\n",
		      di->is_board_type, di->battery_is_removable);
}

static void get_r_pcb_dts(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;
	unsigned int r_pcb = DEFAULT_RPCB;

	ret = of_property_read_u32(np, "r_pcb", &r_pcb);
	if (ret)
		coul_core_err("error:get r_pcb value failed\n");
	di->r_pcb = r_pcb;
	coul_core_info("dts:get r_pcb = %u\n", r_pcb);
}

static void get_dec_enable_status(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	/* dec_enable control function is enable or disable */
	ret = of_property_read_u32(np, "dec_enable", &di->dec_state);
	if (ret) {
		di->dec_state = 0;
		coul_core_err("dts error:get dec_enable value failed\n");
	}
	coul_core_info("dts:get dec_enable_status = %u\n", di->dec_state);
}

static void get_adc_info_dts(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "adc_batt_id", &di->adc_batt_id);
	if (ret) {
		di->adc_batt_id = DEFAULT_HKADC_BATT_ID;
		coul_core_err("dts:can not get batt id adc channel,use default channel: %u\n",
			di->adc_batt_id);
	}
	coul_core_info("dts:get batt id adc channel = %u\n", di->adc_batt_id);

	ret = of_property_read_u32(np, "adc_batt_temp", &di->adc_batt_temp);
	if (ret) {
		di->adc_batt_temp = DEFAULT_HKADC_BATT_TEMP;
		coul_core_err("dts:can not get batt temp adc channel,use default channel: %u\n",
			di->adc_batt_temp);
	}
	coul_core_info("dts:get batt temperature adc channel = %u\n",
		       di->adc_batt_temp);
}

static void get_soc_dts_info(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;
	unsigned int last_soc_enable = 0;
	unsigned int startup_delta_soc = 0;
	unsigned int soc_at_term = DEFAULT_SOC_AT_TERM;
	unsigned int soc_monitor_limit = DEFAULT_SOC_MONITOR_LIMIT;

	ret = of_property_read_u32(np, "last_soc_enable", &last_soc_enable);
	if (ret)
		coul_core_err("dts:can not get last_soc_enable, use default : %u\n",
			last_soc_enable);
	di->last_soc_enable = last_soc_enable;
	coul_core_info("dts:get last_soc_enable = %u\n", last_soc_enable);

	ret = of_property_read_u32(np, "startup_delta_soc", &startup_delta_soc);
	if (ret)
		coul_core_err("dts:can not get delta_soc,use default: %u!\n",
			startup_delta_soc);
	di->startup_delta_soc = startup_delta_soc;
	coul_core_info("dts:get delta_soc = %u\n", startup_delta_soc);

	ret = of_property_read_u32(np, "soc_at_term", &soc_at_term);
	if (ret)
		coul_core_err("dts:can not get soc_at_term,use default : %u\n",
			soc_at_term);
	di->soc_at_term = soc_at_term;
	coul_core_info("dts:get soc_at_term = %u\n", soc_at_term);

	ret = of_property_read_u32(np, "soc_monitor_limit", &soc_monitor_limit);
	if (ret)
		coul_core_err("dts:get soc_monitor_limit fail, use default limit value!\n");
	di->soc_monitor_limit = soc_monitor_limit;
	coul_core_info("soc_monitor_limit = %d\n", di->soc_monitor_limit);
}

static void get_ntc_paras(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int i, ret, array_len, idata;
	const char *compensation_data_string = NULL;

	/* reset to 0 */
	memset(di->ntc_temp_compensation_para, 0,
		sizeof(di->ntc_temp_compensation_para));
	array_len = of_property_count_strings(np, "ntc_temp_compensation_para");
	if ((array_len <= 0) || (array_len % NTC_COMPENSATION_PARA_TOTAL != 0)) {
		coul_core_err("ntc_temp_compensation_para is invaild, please check ntc_temp_compensation_para number\n");
		return;
	}
	if (array_len > COMPENSATION_PARA_LEVEL * NTC_COMPENSATION_PARA_TOTAL) {
		array_len =
			COMPENSATION_PARA_LEVEL * NTC_COMPENSATION_PARA_TOTAL;
		coul_core_err("ntc_temp_compensation_para is too long, use only front %d paras\n",
			array_len);
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np, "ntc_temp_compensation_para",
			i, &compensation_data_string);
		if (ret) {
			coul_core_err("get ntc_temp_compensation_para failed\n");
			return;
		}

		idata = 0;
		ret = kstrtoint(compensation_data_string,
			BASE_OF_SIMPLE_STRTOL, &idata);
		if (ret < 0)
			coul_core_err("compensation_data_string failed\n");
		switch (i % NTC_COMPENSATION_PARA_TOTAL) {
		case NTC_COMPENSATION_PARA_ICHG:
			di->ntc_temp_compensation_para[
				i / NTC_COMPENSATION_PARA_TOTAL].
					ntc_compensation_ichg = idata;
			break;
		case NTC_COMPENSATION_PARA_VALUE:
			di->ntc_temp_compensation_para[
				i / NTC_COMPENSATION_PARA_TOTAL].
					ntc_compensation_value = idata;
			break;
		default:
			coul_core_err("ntc_temp_compensation_para get failed\n");
		}
		coul_core_info("di->ntc_temp_compensation_para[%d][%d] = %d\n",
			i / (NTC_COMPENSATION_PARA_TOTAL),
			i % (NTC_COMPENSATION_PARA_TOTAL), idata);
	}
}

static void get_ntc_temp_compensation_para(
	struct smartstar_coul_device *di, const struct device_node *np)
{
	unsigned int ntc_compensation_is = 0;
	int ret;

	ret = of_property_read_u32(np, "ntc_compensation_is",
		&ntc_compensation_is);
	if (ret) {
		di->ntc_compensation_is = 0;
		coul_core_info("get ntc_compensation_is failed\n");
	} else {
		di->ntc_compensation_is = ntc_compensation_is;
		coul_core_info("ntc_compensation_is = %d\n",
			di->ntc_compensation_is);
		get_ntc_paras(di, np);
	}
}

static void get_batt_temp_range(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret, err;
	const char *batt_temp_too_hot_string = NULL;
	const char *batt_temp_too_cold_string = NULL;

	ret = of_property_read_string(np, "batt_temp_too_hot",
		&batt_temp_too_hot_string);
	if (ret) {
		di->batt_temp_too_hot = TEMP_TOO_HOT;
		coul_core_err("error:get batt_temp_too_hot value failed\n");
	} else {
		err = kstrtoint(batt_temp_too_hot_string,
			BASE_OF_SIMPLE_STRTOL, &di->batt_temp_too_hot);
		if (err < 0) {
			di->batt_temp_too_hot = TEMP_TOO_HOT;
			coul_core_err("batt_temp_too_hot failed\n");
		}
	}
	coul_core_info("dts:get batt_temp_too_hot = %d\n", di->batt_temp_too_hot);

	ret = of_property_read_string(np, "batt_temp_too_cold",
		&batt_temp_too_cold_string);
	if (ret) {
		di->batt_temp_too_cold = TEMP_TOO_COLD;
		coul_core_err("error:get batt_temp_too_cold value failed!\n");
	} else {
		err = kstrtoint(batt_temp_too_cold_string,
			BASE_OF_SIMPLE_STRTOL, &di->batt_temp_too_cold);
		if (err < 0) {
			di->batt_temp_too_cold = TEMP_TOO_COLD;
			coul_core_err("batt_temp_too_cold failed\n");
		}
	}
	coul_core_info("dts:get temp_too_cold = %d\n", di->batt_temp_too_cold);
}

static void get_low_vol_filter_cnt(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;
	unsigned int low_vol_filter_cnt = LOW_INT_VOL_COUNT;

	ret = of_property_read_u32(np, "low_vol_filter_cnt",
		&low_vol_filter_cnt);
	if (ret)
		coul_core_err("dts:get low_vol_filter_cnt fail, use default limit value\n");
	di->low_vol_filter_cnt = low_vol_filter_cnt;
	coul_core_info("low_vol_filter_cnt = %u\n", di->low_vol_filter_cnt);
}

static void get_dts_c_offset_a(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "current_offset_a",
		(u32 *)&di->dts_c_offset_a);
	if (ret) {
		di->dts_c_offset_a = DEFAULT_C_OFF_A;
		coul_core_err("error:get current_offset_a value failed\n");
	}
	coul_core_info("dts:get dts_c=%d\n", di->dts_c_offset_a);
}

static void get_dts_nv_flag(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "batt_backup_nv_flag",
		(unsigned int *)&di->batt_backup_nv_flag);
	if (ret) {
		di->batt_backup_nv_flag = 0;
		coul_core_err("dts: get batt_backup_nv_flag fail, use default value\n");
	}
	coul_core_info("batt_backup_nv_flag = %d\n", di->batt_backup_nv_flag);

	ret = of_property_read_u32(np, "need_restore_cycle_flag",
		(unsigned int *)&di->need_restore_cycle_flag);
	if (ret) {
		di->need_restore_cycle_flag = 0;
		coul_core_err("dts: get need_restore_cycle_flag fail, use default value\n");
	}
	coul_core_info("need_restore_cycle_flag = %d\n",
		di->need_restore_cycle_flag);
}

static void get_wakelock_low_batt_soc(struct smartstar_coul_device *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "wakelock_low_batt_soc",
		&di->wakelock_low_batt_soc);
	if (ret) {
		di->wakelock_low_batt_soc = WAKELOCK_LOW_BATT_SOC;
		coul_core_err("dts: use default wakelock_low_batt_soc value\n");
	}

	if (di->wakelock_low_batt_soc > WAKELOCK_LOW_BATT_SOC)
		di->wakelock_low_batt_soc = WAKELOCK_LOW_BATT_SOC;

	coul_core_info("wakelock_low_batt_soc = %u\n", di->wakelock_low_batt_soc);
}

void coul_core_get_dts(struct smartstar_coul_device *di)
{
	struct device_node *np = NULL;

	if (di == NULL) {
		coul_core_err("%s di is null\n", __func__);
		return;
	}
	if (di->dev == NULL) {
		coul_core_err("%s di->dev is null\n", __func__);
		return;
	}
	np = di->dev->of_node;
	if (np == NULL) {
		coul_core_err("%s np is null\n", __func__);
		return;
	}
	/* check if open low temp soc show optimize function */
	check_low_temp_opt(di, np);
	get_multi_ocv_open_flag(di, np);
	get_fcc_update_limit_flag(di, np);
	get_cutoff_vol_mv(di, np);
	get_current_full_enable(di, np);
	get_ocv_enable_flag(di, np);
	get_dischg_ocv_soc(di, np);
	get_ocv_data_check_flag(di, np);
	get_polar_supply_dts(di, np);
	get_uuc_vol_dts(di, np);
	get_dts_c_offset_a(di, np);
	get_bci_dts_info(di);
	get_r_pcb_dts(di, np);
	get_dec_enable_status(di, np);
	get_adc_info_dts(di, np);
	get_soc_dts_info(di, np);
	get_ntc_temp_compensation_para(di, np);
	get_low_vol_filter_cnt(di, np);
	get_dts_nv_flag(di, np);
	get_batt_temp_range(di, np);
	get_temp_dts(np);
	get_wakelock_low_batt_soc(di, np);
}

