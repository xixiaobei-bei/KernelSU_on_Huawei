/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ocv operation functions for coul module
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

#ifdef CONFIG_COUL_POLAR
#include "coul_polar.h"
#include "coul_update_polar_info.h"
#endif
#include "coul_private_interface.h"
#include "coul_ocv_ops.h"

#define DELTA_SLEEP_TIME_OFFSET 30 /* sleep time offset, in s */
#define DELTA_SLEEP_TIME        (10 * 60) /* sleep time bigger could update ocv, in s */
#define DELTA_SLEEP_CURRENT     50 /* sleep current less could updat eocv, in mA */
#define VOL_FIFO_MAX            10
#define VOL_MAX_DIFF_UV         5000
#define CURR2UPDATE_OCV_TIME    (10 * 60)
#define SR_ARR_LEN              100
#define SR_MAX_RESUME_TIME      90         /* 90 s */
#define SR_DELTA_SLEEP_TIME     (4 * 60)   /* 4 min */
#define SR_DELTA_WAKEUP_TIME    30         /* 30 s */
#define SR_TOTAL_TIME           (30 * 60)  /* 30 min */
#define SR_DUTY_RATIO           95
#define DEFAULT_BATTERY_OHMIC_RESISTANCE        100 /* mohm */
#define FLAG_USE_CLAC_OCV       0xABCD
#define CAPACITY_INVALID_AREA_4500      4500000
#define CAPACITY_INVALID_AREA_2500      2500000
#define INDEX_MAX               3
#define CALI_RBATT_CC_MAX       20 /* mAh */
#define CALI_RBATT_CURR_MIN     500 /* mA */
#define PC_UPPER_LIMIT          1000
#define MIN_VALID_VOL_DATA_NUM  3
#define WAKEUP_AVG_CUR_500MA    500
#define WAKEUP_AVG_CUR_250MA    250
#define SEC_PER_HOUR_DIV_200    18
#define PERMILLAGE_DIV_200      5
#define OCV_UPDATE_DELTA_TIME   30
#define SEC_PER_MIN             60
#define SEC_PER_HOUR            3600

struct coul_ocv_cali_info {
	char cali_timestamp[TIMESTAMP_STR_SIZE];
	int cali_ocv_uv;
	int cali_ocv_temp; /* tenth degree */
	s64 cali_cc_uah;
	int cali_rbatt;
	unsigned char cali_ocv_level;
};

struct ocv_level_param {
	int duty_ratio_limit;
	int sleep_current_limit;
	int sleep_time_limit;
	int wake_time_limit;
	int max_avg_curr_limit;
	int temp_limit;
	int delta_soc_limit;
	int ocv_gap_time_limit;
	bool ocv_update_anyway;
	bool allow_fcc_update;
	bool is_enabled;
};

struct ocv_judge_data {
	int sleep_time;
	int wake_time;
	int sleep_current;
	int big_current_10min;
	int temp;
	int duty_ratio;
};

struct avg_curr2update_ocv {
	int current_ma;
	int time;
};

static struct avg_curr2update_ocv g_curr2update_ocv[AVG_CURR_MAX] = {
	{ 0, 0 },
	{ 0, 0 }
};

static struct ocv_level_param g_ocv_level_para[OCV_LEVEL_MAX] = {
	{ 0, 50, 570, 9999, 9999, 0, 0, 0, 0, 1, 1 },
	{ 95, 50, 240, 30, 9999, 0, 0, 0, 0, 1, 1 },
	{ 95, 50, 120, 15, 500, 0, 0, 0, 0, 1, 1 },
	{ 98, 50, 60, 4, 250, 0, 0, 0, 0, 1, 1 },
	{ 95, 50, 60, 4, 250, 0, 3, 3600, 0, 0, 1 },
	{ 95, 50, 30, 4, 250, 0, 6, 7200, 0, 0, 1 },
	{ 95, 50, 30, 4, 500, 100, 8, 10800, 0, 0, 1 },
	{ 90, 50, 3, 4, 250, 100, 10, 18000, 0, 0, 1 }
};

static int g_sr_time_sleep[SR_ARR_LEN];
static int g_sr_time_wakeup[SR_ARR_LEN];
static int g_sr_index_sleep;
static int g_sr_index_wakeup;
static u8 g_delta_soc_renew_ocv;
static u8 g_test_delta_soc_renew_ocv;
static struct coul_ocv_cali_info g_coul_ocv_cali_info[INDEX_MAX];
static int g_ocv_cali_index;
static int g_ocv_cali_rbatt_valid_flag;
static int g_eco_leak_uah;

void judge_eco_leak_uah(int soc)
{
	if (g_eco_leak_uah == 0) {
		coul_core_info("NOT EXIT FROM ECO,SOC_NEW = %d\n", soc);
	} else {
		coul_core_info("EXIT FROM ECO,SOC_NEW = %d\n", soc);
		g_eco_leak_uah = 0;
	}
}

void set_ocv_cali_level(unsigned char last_ocv_level)
{
	g_coul_ocv_cali_info[g_ocv_cali_index].cali_ocv_level = last_ocv_level;
}

void update_ocv_cali_rbatt(struct smartstar_coul_device *di, int avg_c,
	int rbatt_calc)
{
	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;

	if ((!g_ocv_cali_rbatt_valid_flag) &&
		(di->coul_dev_ops->calculate_cc_uah() / UA_PER_MA <
			CALI_RBATT_CC_MAX) &&
		(avg_c > CALI_RBATT_CURR_MIN)) {
		g_coul_ocv_cali_info[g_ocv_cali_index].cali_rbatt = rbatt_calc;
		g_ocv_cali_rbatt_valid_flag = 1;
	}
}

/* clear sleep and wakeup global variable */
static void coul_clear_sr_time_array(void)
{
	memset(g_sr_time_sleep, 0, sizeof(g_sr_time_sleep));
	memset(g_sr_time_wakeup, 0, sizeof(g_sr_time_wakeup));
	g_sr_index_sleep = 0;
	g_sr_index_wakeup = 0;
}

void record_ocv_cali_info(struct smartstar_coul_device *di)
{
	static int index;
	char timestamp[TIMESTAMP_STR_SIZE] = {0};

	if (di == NULL)
		return;

	/* clean buff */
	memset(&g_coul_ocv_cali_info[index], 0,
		sizeof(g_coul_ocv_cali_info[index]));

	get_timestamp(timestamp, TIMESTAMP_STR_SIZE);
	snprintf(g_coul_ocv_cali_info[index].cali_timestamp, TIMESTAMP_STR_SIZE,
		 "%s", timestamp);

	/* save log: ocv, ocv_temp, cc, level */
	g_coul_ocv_cali_info[index].cali_ocv_uv = di->batt_ocv;
	g_coul_ocv_cali_info[index].cali_ocv_temp = di->batt_ocv_temp;
	g_coul_ocv_cali_info[index].cali_cc_uah =
		di->coul_dev_ops->calculate_cc_uah();
	g_coul_ocv_cali_info[index].cali_ocv_level = di->last_ocv_level;

	/* Save the currently used buff number to the global variable */
	g_ocv_cali_index = index;
	g_ocv_cali_rbatt_valid_flag = 0;

	/* switch buffer number */
	index++;
	index = index % INDEX_MAX;
}

int coul_dsm_report_ocv_cali_info(struct smartstar_coul_device *di,
	int err_num, const char *buff)
{
#ifdef CONFIG_HUAWEI_DSM
	char timestamp[TIMESTAMP_STR_SIZE] = {0};
	char dsm_buf[DSM_BUFF_SIZE_MAX] = {0};
	int i;
	int tmp_len = 0;

	if ((di == NULL) || (buff == NULL))
		return -1;

	get_timestamp(timestamp, TIMESTAMP_STR_SIZE);
	tmp_len += snprintf(dsm_buf, DSM_BUFF_SIZE_MAX, "%s\n", timestamp);

	/* common info: brand, cycles */
	tmp_len += snprintf(dsm_buf + tmp_len, DSM_BUFF_SIZE_MAX - tmp_len,
		"batteryName:%s, chargeCycles:%d\n",
		di->batt_data->batt_brand,
		di->batt_chargecycles / PERCENT);

	/* key info */
	tmp_len += snprintf(dsm_buf + tmp_len, DSM_BUFF_SIZE_MAX - tmp_len,
		"%s\n", buff);

	/* OCV history calibration information */
	for (i = 0; i < INDEX_MAX; i++) {
		tmp_len += snprintf(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "[OCV calibration]%s ",
			g_coul_ocv_cali_info[i].cali_timestamp);
		tmp_len += snprintf(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "OCV:%duV ",
			g_coul_ocv_cali_info[i].cali_ocv_uv);
		tmp_len += snprintf(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "temp:%d ",
			g_coul_ocv_cali_info[i].cali_ocv_temp);
		tmp_len += snprintf(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "CC:%duAh ",
			(int)g_coul_ocv_cali_info[i].cali_cc_uah);
		tmp_len += snprintf(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "rbatt:%d ",
			g_coul_ocv_cali_info[i].cali_rbatt);
		tmp_len += snprintf(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "ocv_cali_level:%u\n",
			g_coul_ocv_cali_info[i].cali_ocv_level);
	}

	/* report */
	return power_dsm_report_dmd(POWER_DSM_BATTERY, err_num, dsm_buf);
#else
	return 0;
#endif
}


#ifdef CONFIG_PM
#ifdef CONFIG_HISI_DEBUG_FS
int control_ocv_level(int level, int val)
{
	if ((level >= OCV_LEVEL_MAX) || (level < OCV_LEVEL_0))
		return -1;
	if (val == 1)
		g_ocv_level_para[level].is_enabled = 1;
	else if (val == 0)
		g_ocv_level_para[level].is_enabled = 0;
	else
		return -1;
	coul_core_info("ocv level[%d]is set[%d]\n",
		level, g_ocv_level_para[level].is_enabled);
	return 0;
}

int print_multi_ocv_threshold(void)
{
	int i;

	coul_core_info("%s++++", __func__);
	coul_core_info("duty_ratio_limit|sleep_time_limit|wake_time_limit|max_avg_curr_limit|temp_limit|ocv_gap_time_limit|delta_soc_limit|ocv_update_anyway|allow_fcc_update|is_enabled|\n");
	for (i = 0; i < OCV_LEVEL_MAX; i++)
		coul_core_info("%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|\n",
			g_ocv_level_para[i].duty_ratio_limit,
			g_ocv_level_para[i].sleep_time_limit,
			g_ocv_level_para[i].wake_time_limit,
			g_ocv_level_para[i].max_avg_curr_limit,
			g_ocv_level_para[i].temp_limit,
			g_ocv_level_para[i].ocv_gap_time_limit,
			g_ocv_level_para[i].delta_soc_limit,
			g_ocv_level_para[i].ocv_update_anyway,
			g_ocv_level_para[i].allow_fcc_update,
			g_ocv_level_para[i].is_enabled);
	coul_core_info("%s----", __func__);
	return 0;
}

u8 get_delta_soc(void)
{
	if (!g_test_delta_soc_renew_ocv)
		return g_delta_soc_renew_ocv;
	return g_test_delta_soc_renew_ocv;
}

u8 set_delta_soc(u8 delta_soc)
{
	g_test_delta_soc_renew_ocv = delta_soc;
	coul_core_info("delta_soc is set[%d]\n", delta_soc);
	return g_test_delta_soc_renew_ocv;
}
#endif
#endif

void coul_clear_cc_register(void)
{
	int cc_temp;
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;
	cc_temp = di->coul_dev_ops->calculate_cc_uah();
	di->iscd_full_update_cc += cc_temp;
#ifdef CONFIG_COUL_POLAR
	update_polar_ishort_info_cc(cc_temp);
#endif
	di->coul_dev_ops->clear_cc_register();
}

/* clear sleep /wakeup /cl_in /cl_out time */
void coul_clear_coul_time(void)
{
	unsigned int time_now;
	int sr_cur_state;
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_err("[SR]%s %d: di is NULL\n", __func__, __LINE__);
		return;
	}

	sr_cur_state = get_sr_cur_state();

	time_now = di->coul_dev_ops->get_coul_time();
	if (sr_cur_state == SR_DEVICE_WAKEUP) {
		di->sr_resume_time -= time_now;
		if (di->sr_resume_time > 0) {
			coul_core_err("[SR]%s %d: di->sr_resume_time = %d\n",
				__func__, __LINE__, di->sr_resume_time);
			di->sr_resume_time = 0;
		}
		di->sr_suspend_time = 0;
	} else {
		di->sr_resume_time = 0;
		di->sr_suspend_time = 0;
		coul_core_err("[SR]%s %d: sr_cur_state = %d\n",
			      __func__, __LINE__, sr_cur_state);
	}
	di->charging_stop_time -= time_now;
#ifdef CONFIG_COUL_POLAR
	update_polar_ishort_info_time(time_now);
#endif
	di->coul_dev_ops->clear_coul_time();
}

/* get ocv by soft way when shutdown time less 20min */
static int get_calc_ocv(struct smartstar_coul_device *di)
{
	int ocv, batt_temp, chargecycles, soc_rbatt;
	int rbatt, vbatt_uv, ibatt_ua;

	batt_temp = di->batt_temp;
	chargecycles = di->batt_chargecycles / PERCENT;
	vbatt_uv = di->coul_dev_ops->convert_ocv_regval2uv(
		di->nv_info.calc_ocv_reg_v);
	ibatt_ua = di->coul_dev_ops->convert_ocv_regval2ua(
		di->nv_info.calc_ocv_reg_c);
	soc_rbatt = calculate_pc(di, vbatt_uv, batt_temp, chargecycles);
	soc_rbatt = soc_rbatt / TENTH;
	rbatt = get_rbatt(di, soc_rbatt, batt_temp);
	ocv = vbatt_uv + ibatt_ua * rbatt / PERMILLAGE;

	coul_core_info("calc ocv, v_uv=%d, i_ua=%d, soc_rbatt=%d, rbatt=%d, ocv=%d\n",
		vbatt_uv, ibatt_ua, soc_rbatt, rbatt, ocv);
	return ocv;
}

/* get first ocv from register, hardware record it during system reset */
void coul_get_initial_ocv(struct smartstar_coul_device *di)
{
	unsigned short ocvreg;
	int ocv_uv;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return;
	}

	ocvreg = di->coul_dev_ops->get_ocv();
	coul_core_info("[%s]ocvreg = 0x%x\n", __func__, ocvreg);
	di->batt_ocv_valid_to_refresh_fcc = 1;

	if (ocvreg == (unsigned short)FLAG_USE_CLAC_OCV) {
		coul_core_info("using calc ocv\n");
		ocv_uv = get_calc_ocv(di);
		di->batt_ocv_valid_to_refresh_fcc = 0;
		/* ocv temp saves in fastboot */
		di->coul_dev_ops->save_ocv(ocv_uv, NOT_UPDATE_FCC);
		di->is_nv_need_save = 0;
		di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
		di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
	} else if (di->coul_dev_ops->get_use_saved_ocv_flag()) {
		if (di->coul_dev_ops->get_fcc_invalid_up_flag())
			di->batt_ocv_valid_to_refresh_fcc = 0;
		di->is_nv_need_save = 0;
		ocv_uv = di->coul_dev_ops->convert_ocv_regval2uv(ocvreg);
		coul_core_info("using save ocv\n");
	}  else {
		if (di->coul_dev_ops->get_fcc_invalid_up_flag())
			di->batt_ocv_valid_to_refresh_fcc = 0;
		ocv_uv = di->coul_dev_ops->convert_ocv_regval2uv(ocvreg);
		di->is_nv_need_save = 0;
		coul_core_info("using pmu ocv from fastboot\n");
	}

	di->batt_ocv_temp = di->coul_dev_ops->get_ocv_temp();
	di->batt_ocv = ocv_uv;
	di->coul_dev_ops->get_ocv_level(&(di->last_ocv_level));
#ifdef CONFIG_HISI_DEBUG_FS
	print_multi_ocv_threshold();
#endif
	coul_core_info("initial OCV = %d, OCV_temp=%d, fcc_flag= %d, ocv_level: %d\n",
		di->batt_ocv, di->batt_ocv_temp,
		di->batt_ocv_valid_to_refresh_fcc, di->last_ocv_level);
}

/* interpolate ocv value by full charge capacity when charging done */
void get_ocv_by_fcc(struct smartstar_coul_device *di)
{
	unsigned int new_ocv;
	int batt_temp_degc;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return;
	}
	batt_temp_degc = di->batt_temp / TENTH;
	/* looking for ocv value in the OCV-FCC table */
	new_ocv = interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
		batt_temp_degc, PC_UPPER_LIMIT);
	if (new_ocv > (CAPACITY_INVALID_AREA_4500 / UVOLT_PER_MVOLT))
		new_ocv = CAPACITY_INVALID_AREA_4500;
	else
		new_ocv = new_ocv * UVOLT_PER_MVOLT;

	if ((new_ocv - di->batt_ocv) > 0) {
		dbg_cnt_inc(dbg_ocv_cng_1);
		coul_core_info("full charged, and OCV change, "
			"new_ocv = %u, old_ocv = %d\n", new_ocv, di->batt_ocv);
		di->batt_ocv = new_ocv;
		di->batt_ocv_temp = di->batt_temp;
		di->coul_dev_ops->save_ocv_temp((short)di->batt_ocv_temp);
		di->batt_ocv_valid_to_refresh_fcc = 1;
		di->coul_dev_ops->save_ocv(new_ocv, IS_UPDATE_FCC);
		di->last_ocv_level = OCV_LEVEL_0;
		di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
		coul_clear_cc_register();
		coul_clear_coul_time();
	} else {
		dbg_cnt_inc(dbg_ocv_fc_failed);
		coul_core_err("full charged, but OCV don't change, new_ocv = %u, old_ocv = %d\n",
			new_ocv, di->batt_ocv);
	}
}

/* get array max value and min value */
static void max_min_value(const int array[], u32 size, int *min, int *max)
{
	u32 i;
	int max_value, min_value;

	if ((!size) || (max == NULL) || (min == NULL))
		return;
	max_value = array[0];
	min_value = array[0];

	for (i = 1; i < size; i++) {
		if (array[i] > max_value)
			max_value = array[i];

		if (array[i] < min_value)
			min_value = array[i];
	}

	*max = max_value;
	*min = min_value;
}

/*
 * check vol data from fifo valid,  Return: -1:data invalid, 0:data valid.
 * Remark: 1 all data is same ,invalid. 2 error exceeding 5mv, invalid
 */
static int check_ocv_vol_data_valid(struct smartstar_coul_device *di,
	const int vol_data[], u32 data_cnt)
{
	int max = 0;
	int min = 0;

	if (data_cnt > VOL_FIFO_MAX)
		data_cnt = VOL_FIFO_MAX;

	if (!di->check_ocv_data_enable) {
		coul_core_err("[%s] not check\n", __func__);
		return 0;
	}
	max_min_value(vol_data, data_cnt, &min, &max);

	/* all data is same, invalid */
	if (max == min) {
		coul_core_err("[%s] all vol data is same, invalid\n", __func__);
		return -1;
	}
	/* Error exceeding 5mv, invalid */
	if (abs(max - min) > VOL_MAX_DIFF_UV) {
		coul_core_err("[%s] fifo vol difference is more than 5 millivolts, invalid\n",
			__func__);
		return -1;
	}
	coul_core_err("[%s] ocv data valid\n", __func__);
	return 0;
}

int get_ocv_vol_from_fifo(struct smartstar_coul_device *di)
{
	short i;
	int current_ua, voltage_uv, totalvol, totalcur;
	int used = 0;
	int vol_fifo[VOL_FIFO_MAX] = {0};

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("[%s]di is null\n", __func__);
		return 0;
	}

	totalvol = 0;
	totalcur = 0;
	for (i = 0; i < di->coul_dev_ops->get_fifo_depth(); i++) {
		current_ua = di->coul_dev_ops->get_battery_cur_ua_from_fifo(i);
		voltage_uv = di->coul_dev_ops->get_battery_vol_uv_from_fifo(i);
		if ((current_ua >= CURRENT_LIMIT) ||
			(current_ua < CHARGING_CURRENT_OFFSET)) {
			dbg_cnt_inc(dbg_invalid_vol);
			coul_core_info("invalid current = %d ua\n", current_ua);
			continue;
		}
		if ((voltage_uv >= CAPACITY_INVALID_AREA_4500) ||
			(voltage_uv <= CAPACITY_INVALID_AREA_2500)) {
			dbg_cnt_inc(dbg_invalid_vol);
			coul_core_info("invalid voltage = %d uv\n", voltage_uv);
			continue;
		}
		dbg_cnt_inc(dbg_valid_vol);
		coul_core_info("valid current = %d ua, vol = %d uv!\n",
			current_ua, voltage_uv);
		totalvol += voltage_uv;
		totalcur += current_ua;
		vol_fifo[used % VOL_FIFO_MAX] = voltage_uv;
		used++;
	}

	coul_core_info("used = %d\n", used);
	if (used > MIN_VALID_VOL_DATA_NUM) {
		/* check vol inalid */
		if (check_ocv_vol_data_valid(di, vol_fifo, used) != 0)
			return 0;
		voltage_uv = totalvol / used;
		current_ua = totalcur / used;
		voltage_uv += current_ua / PERMILLAGE *
			(di->r_pcb / PERMILLAGE +
				DEFAULT_BATTERY_OHMIC_RESISTANCE);
		return voltage_uv;
	}
	return 0;
}

#ifdef CONFIG_PM
static int calc_wakeup_avg_current(struct smartstar_coul_device *di,
	int last_cc, int last_time, int curr_cc, int curr_time)
{
	static int first_in = 1;
	int iavg_ma, delta_cc, delta_time;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("[%s]di is null\n", __func__);
		return 0;
	}

	if (first_in) {
		iavg_ma = di->coul_dev_ops->get_battery_current_ua() / UA_PER_MA;
		first_in = 0;
	} else {
		delta_cc = curr_cc - last_cc;
		delta_time = curr_time - last_time;
		if (delta_time > 0)
			iavg_ma = (div_s64((s64)delta_cc * SEC_PER_HOUR,
				delta_time)) / UA_PER_MA;
		else
			iavg_ma = di->coul_dev_ops->get_battery_current_ua() /
				UA_PER_MA;

		coul_core_info("wake_up delta_time=%ds, iavg_ma=%d\n",
			delta_time, iavg_ma);
	}

	if (abs(iavg_ma) >= WAKEUP_AVG_CUR_500MA) {
		g_curr2update_ocv[AVG_CURR_500MA].current_ma = abs(iavg_ma);
		g_curr2update_ocv[AVG_CURR_500MA].time = curr_time;
	} else if (abs(iavg_ma) >= WAKEUP_AVG_CUR_250MA) {
		g_curr2update_ocv[AVG_CURR_250MA].current_ma = abs(iavg_ma);
		g_curr2update_ocv[AVG_CURR_250MA].time = curr_time;
	}
	return iavg_ma;
}

void record_wakeup_info(struct smartstar_coul_device *di, int wakeup_time)
{
	int wakeup_avg_current_ma = 0;

	if (di == NULL)
		return;

	if (wakeup_time > SR_MAX_RESUME_TIME) {
		coul_clear_sr_time_array();
		coul_core_info("[SR]%s %d: wakeup_time %d > SR_MAX_RESUME_TIME %d\n",
			__func__, __LINE__, wakeup_time, SR_MAX_RESUME_TIME);
		wakeup_avg_current_ma = calc_wakeup_avg_current(
			di, di->resume_cc, di->sr_resume_time,
			di->suspend_cc, di->sr_suspend_time);
	} else if (wakeup_time >= 0) {
		g_sr_time_wakeup[g_sr_index_wakeup] = wakeup_time;
		g_sr_index_wakeup++;
		g_sr_index_wakeup = g_sr_index_wakeup % SR_ARR_LEN;
		wakeup_avg_current_ma = calc_wakeup_avg_current(di,
			di->resume_cc, di->sr_resume_time,
			di->suspend_cc, di->sr_suspend_time);
	} else {
		coul_core_err("[SR]%s %d: wakeup_time=%d, di->sr_suspend_time=%d, di->sr_resume_time=%d\n",
			__func__, __LINE__, wakeup_time,
			di->sr_suspend_time, di->sr_resume_time);
	}
	coul_core_info("SUSPEND! cc=%d, time=%d, wakeup_avg_current:%d\n",
		di->suspend_cc, di->suspend_time, wakeup_avg_current_ma);
}

void record_sleep_info(int sr_sleep_time)
{
	if (sr_sleep_time >= 0) {
		g_sr_time_sleep[g_sr_index_sleep] = sr_sleep_time;
		g_sr_index_sleep++;
		g_sr_index_sleep = g_sr_index_sleep % SR_ARR_LEN;
	} else {
		coul_core_err("[SR]%s %d: sr_sleep_time = %d\n",
			__func__, __LINE__, sr_sleep_time);
	}
}

/* calculate last SR_TOTAL_TIME seconds duty ratio */
static int sr_get_duty_ratio(void)
{
	int total_sleep_time = 0;
	int total_wakeup_time = 0;
	int last_sleep_idx = ((g_sr_index_sleep - 1) < 0) ?
		SR_ARR_LEN - 1 : g_sr_index_sleep - 1;
	int last_wakeup_idx = ((g_sr_index_wakeup - 1) < 0) ?
		SR_ARR_LEN - 1 : g_sr_index_wakeup - 1;
	int cnt = 0;
	int duty_ratio = 0;

	do {
		total_sleep_time += g_sr_time_sleep[last_sleep_idx];
		total_wakeup_time += g_sr_time_wakeup[last_wakeup_idx];

		last_sleep_idx = ((last_sleep_idx - 1) < 0) ?
			SR_ARR_LEN - 1 : last_sleep_idx - 1;
		last_wakeup_idx = ((last_wakeup_idx - 1) < 0) ?
			SR_ARR_LEN - 1 : last_wakeup_idx - 1;

		cnt++;
		if (cnt >= SR_ARR_LEN)
			break;
	} while (total_sleep_time + total_wakeup_time < SR_TOTAL_TIME);

	/* calculate duty ratio */
	if (total_sleep_time + total_wakeup_time >= SR_TOTAL_TIME) {
		duty_ratio = total_sleep_time * PERCENT /
			(total_sleep_time + total_wakeup_time);
		coul_core_info("[SR]%s %d: total_wakeup=%ds, total_sleep=%ds, duty_ratio=%d\n",
			__func__, __LINE__, total_wakeup_time,
			total_sleep_time, duty_ratio);
	}
	return duty_ratio;
}

static int get_big_current_10min(int time_now)
{
	u8 i;

	for (i = 0; i < AVG_CURR_MAX; i++) {
		if (time_now - g_curr2update_ocv[i].time < CURR2UPDATE_OCV_TIME)
			return g_curr2update_ocv[i].current_ma;
	}
	return -1;
}

static void clear_big_current_10min(void)
{
	u8 i;

	for (i = 0; i < AVG_CURR_MAX; i++) {
		g_curr2update_ocv[i].current_ma = 0;
		g_curr2update_ocv[i].time = 0;
	}
}

static int save_multi_ocv_and_level(struct smartstar_coul_device *di)
{
	int cc;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("[%s]di is null\n", __func__);
		return 0;
	}
	g_eco_leak_uah = di->coul_dev_ops->calculate_eco_leak_uah();
	di->coul_dev_ops->save_ocv_temp((short)di->batt_ocv_temp);
	if (g_ocv_level_para[di->last_ocv_level].allow_fcc_update) {
		di->batt_ocv_valid_to_refresh_fcc = 1;
		di->coul_dev_ops->save_ocv(di->batt_ocv, IS_UPDATE_FCC);
	} else {
		di->batt_ocv_valid_to_refresh_fcc = 0;
		di->coul_dev_ops->save_ocv(di->batt_ocv, NOT_UPDATE_FCC);
	}
	clear_big_current_10min();
	coul_clear_cc_register();
	coul_clear_coul_time();
	cc = di->coul_dev_ops->calculate_cc_uah();
	cc = cc + g_eco_leak_uah;
	di->coul_dev_ops->save_cc_uah(cc);
	di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
	batt_coul_notify(di, BATT_OCV_UPDATE);
	coul_core_info("awake from deep sleep, new OCV = %d, ocv level = %u, fcc_flag = %d\n",
		       di->batt_ocv, di->last_ocv_level,
		       di->batt_ocv_valid_to_refresh_fcc);
	return 0;
}

static int is_ocv_reach_renew_threshold(
	struct smartstar_coul_device *di, u8 current_ocv_level, int ocv_now,
	int time_now, int batt_temp)
{
	int pc_now;
	u8 last_ocv_level;

	last_ocv_level = di->last_ocv_level;
	if (current_ocv_level <= last_ocv_level)
		return 1;
	pc_now = calculate_pc(di, ocv_now, batt_temp,
		di->batt_chargecycles / PERCENT);
	if (g_test_delta_soc_renew_ocv == 0)
		g_delta_soc_renew_ocv = abs(pc_now - di->batt_soc_real) / TENTH;
	else
		g_delta_soc_renew_ocv = g_test_delta_soc_renew_ocv;
	coul_core_info("[SR]%s(delta_soc:%d, time_now:%d)\n",
		__func__, g_delta_soc_renew_ocv, time_now);
	if ((time_now >=
			g_ocv_level_para[current_ocv_level].ocv_gap_time_limit -
			g_ocv_level_para[last_ocv_level].ocv_gap_time_limit) &&
		(g_delta_soc_renew_ocv >=
			g_ocv_level_para[current_ocv_level].delta_soc_limit -
			g_ocv_level_para[last_ocv_level].delta_soc_limit))
		return 1;

	return 0;
}

static u8 judge_ocv_threshold(struct ocv_judge_data *data)
{
	u8 i;

	for (i = 0; i < OCV_LEVEL_MAX; i++) {
		if ((g_ocv_level_para[i].is_enabled == TRUE) &&
			(data->duty_ratio >=
				g_ocv_level_para[i].duty_ratio_limit) &&
			(data->sleep_time >=
				g_ocv_level_para[i].sleep_time_limit) &&
			(data->wake_time <=
				g_ocv_level_para[i].wake_time_limit) &&
			(data->sleep_current <=
				g_ocv_level_para[i].sleep_current_limit) &&
			(data->temp >= g_ocv_level_para[i].temp_limit) &&
			(data->big_current_10min <=
				g_ocv_level_para[i].max_avg_curr_limit)) {
			coul_core_info("[SR]%s(LEVEL:%d): sleep_time=%ds, wake_time=%ds, sleep_current=%dma, duty_ratio=%d, temp=%d, big_current=%d\n",
				__func__, i, data->sleep_time,
				data->wake_time, data->sleep_current,
				data->duty_ratio, data->temp,
				data->big_current_10min);
			break;
		}
	}
	return i;
}

static int calc_ocv_level(struct smartstar_coul_device *di, int time_now,
	int sleep_time, int wake_time, int sleep_current)
{
	/* judge if need update ocv */
	u8 cur_level;
	int ocv_now, big_current_10min, temp, duty_ratio;
	struct ocv_judge_data data = {0};

	temp = di->batt_temp;
	/* get big wakeup current in 10 min */
	big_current_10min = get_big_current_10min(time_now);
	/* get last SR_TOTAL_TIME seconds duty ratio */
	duty_ratio = sr_get_duty_ratio();

	data.sleep_time = sleep_time;
	data.wake_time = wake_time;
	data.sleep_current = sleep_current;
	data.big_current_10min = big_current_10min;
	data.temp = temp;
	data.duty_ratio = duty_ratio;

	cur_level = judge_ocv_threshold(&data);
	ocv_now = get_ocv_vol_from_fifo(di);
	if (ocv_now == 0)
		return 0;
	if (cur_level < LOW_PRECISION_OCV_LEVEL) {
		if ((!g_ocv_level_para[cur_level].ocv_update_anyway) &&
			(is_in_capacity_dense_area(ocv_now) == TRUE)) {
			coul_core_info("%s:do not update OCV %d\n",
				__func__, ocv_now);
			return 0;
		}
		di->last_ocv_level = cur_level;
		di->batt_ocv = ocv_now;
		di->batt_ocv_temp = temp;
		return 1;
	} else if ((cur_level < OCV_LEVEL_MAX) &&
		(is_ocv_reach_renew_threshold(di, cur_level, ocv_now,
			time_now, temp))) {
		/* calculate low precision ocv */
		if ((!g_ocv_level_para[cur_level].ocv_update_anyway) &&
			(is_in_capacity_dense_area(ocv_now) == TRUE)) {
			coul_core_info("%s:do not update OCV %d\n",
				__func__, ocv_now);
			return 0;
		}
		di->last_ocv_level = cur_level;
		di->batt_ocv = ocv_now;
		di->batt_ocv_temp = temp;
		return 1;
	}
	return 0;
}

/* check whether MULTI OCV can update, Return: 1: can update, 0: can not */
static int multi_ocv_could_update(struct smartstar_coul_device *di)
{
	int sleep_cc, sleep_time, time_now;
	int sleep_current = 0;
	int last_wakeup_time = 0;
	int last_sleep_time = 0;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return 0;

	sleep_cc = di->coul_dev_ops->calculate_cc_uah();
	/* sleep uah */
	sleep_cc = sleep_cc - di->suspend_cc;
	time_now = di->coul_dev_ops->get_coul_time();
	sleep_time = time_now - di->suspend_time;

	/* get sleep current */
	/* ma = ua/1000 = uas/s/1000 = uah*3600/s/1000 = uah*18/(s*5) */
	if (sleep_time > 0) {
		sleep_current = (sleep_cc * SEC_PER_HOUR_DIV_200) /
			(sleep_time * PERMILLAGE_DIV_200);
	} else {
		coul_core_err("[SR]%s %d: sleep_time = %d\n",
			__func__, __LINE__, sleep_time);
		return 0;
	}
	/* get last wakeup time */
	if ((g_sr_index_wakeup >= 0) && (g_sr_index_wakeup < SR_ARR_LEN)) {
		if (g_sr_index_wakeup - 1 >= 0)
			last_wakeup_time = g_sr_time_wakeup[g_sr_index_wakeup - 1];
		else
			last_wakeup_time = g_sr_time_wakeup[SR_ARR_LEN - 1];
	}
	/* get last sleep time */
	if ((g_sr_index_sleep >= 0) && (g_sr_index_sleep < SR_ARR_LEN)) {
		if (g_sr_index_sleep - 1 >= 0)
			last_sleep_time = g_sr_time_sleep[g_sr_index_sleep - 1];
		else
			last_sleep_time = g_sr_time_sleep[SR_ARR_LEN - 1];
	}
	coul_core_info("[SR]going to update ocv, sleep_time=%ds, sleep_current=%d ma\n",
		       sleep_time, sleep_current);
#ifdef CONFIG_HUAWEI_DUBAI
	HWDUBAI_LOGE("DUBAI_TAG_BATTERY_SLEEP_CURRENT", "current=%d", sleep_current);
#endif
	return calc_ocv_level(di, time_now, last_sleep_time, last_wakeup_time, sleep_current);
}

static int sr_need_update_ocv(struct smartstar_coul_device *di)
{
	int last_wakeup_time = 0;
	int last_sleep_time = 0;
	int duty_ratio;

	/* get last wakeup time */
	if ((g_sr_index_wakeup >= 0) && (g_sr_index_wakeup < SR_ARR_LEN)) {
		if (g_sr_index_wakeup - 1 >= 0)
			last_wakeup_time = g_sr_time_wakeup[g_sr_index_wakeup - 1];
		else
			last_wakeup_time = g_sr_time_wakeup[SR_ARR_LEN - 1];
	}
	/* get last sleep time */
	if ((g_sr_index_sleep >= 0) && (g_sr_index_sleep < SR_ARR_LEN)) {
		if (g_sr_index_sleep - 1 >= 0)
			last_sleep_time = g_sr_time_sleep[g_sr_index_sleep - 1];
		else
			last_sleep_time = g_sr_time_sleep[SR_ARR_LEN - 1];
	}
	/* get last SR_TOTAL_TIME seconds duty ratio */
	duty_ratio = sr_get_duty_ratio();
	/* judge if need update ocv */
	if ((last_sleep_time > SR_DELTA_SLEEP_TIME) &&
		(last_wakeup_time < SR_DELTA_WAKEUP_TIME) &&
		(duty_ratio > SR_DUTY_RATIO)) {
		coul_core_info("[SR]%s %d: need_update, last_sleep=%ds, last_wakeup=%ds, duty_ratio=%d\n",
			__func__, __LINE__, last_sleep_time,
			last_wakeup_time, duty_ratio);
		return 1;
	}
	coul_core_info("[SR]%s %d: no_need_update, last_sleep=%ds, last_wakeup=%ds, duty_ratio=%d\n",
		__func__, __LINE__, last_sleep_time,
		last_wakeup_time, duty_ratio);
	return 0;
}

/*
 * check whether OCV can update 1: can update, 0: can not.
 * Remark:  update condition----sleep_time > 10min && sleep_current < 50mA
 */
static int ocv_could_update(struct smartstar_coul_device *di)
{
	int sleep_cc, sleep_time, time_now;
	int sleep_current = 0;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return 0;

	sleep_cc = di->coul_dev_ops->calculate_cc_uah();
	sleep_cc = sleep_cc - di->suspend_cc;  /* sleep uah */
	time_now = di->coul_dev_ops->get_coul_time();
	sleep_time = time_now - di->suspend_time; /* sleep time */

	if ((sleep_time < (DELTA_SLEEP_TIME - DELTA_SLEEP_TIME_OFFSET)) &&
		(!sr_need_update_ocv(di))) {
		coul_core_info("[SR]Can't update ocv, sleep_time=%ds\n",
			sleep_time);
		return 0;
	}

	/* ma = ua/1000 = uas/s/1000 = uah*3600/s/1000 = uah*18/(s*5) */
	if (sleep_time > 0) {
		sleep_current = (sleep_cc * SEC_PER_HOUR_DIV_200) /
			(sleep_time * PERMILLAGE_DIV_200);

		if (sleep_current > DELTA_SLEEP_CURRENT) {
			coul_core_info("[SR]Can't update ocv, sleep_current=%d ma, sleep_time=%ds\n",
				sleep_current, sleep_time);
			return 0;
		}
	} else {
		coul_core_err("[SR]%s %d: sleep_time = %d\n",
			__func__, __LINE__, sleep_time);
		return 0;
	}

	coul_core_info("[SR]going to update ocv, sleep_time=%ds, sleep_current=%d ma\n",
		       sleep_time, sleep_current);
	return 1;
}

/*
 * get ocv after resuming Return: 1: can update, 0: can not
 * Remark: update condition----sleep_time > 10min && sleep_current < 50mA
 */
static void get_ocv_resume(struct smartstar_coul_device *di)
{
	int cc;

	g_eco_leak_uah = di->coul_dev_ops->calculate_eco_leak_uah();
	get_ocv_by_vol(di);
	cc = di->coul_dev_ops->calculate_cc_uah();
	cc = cc + g_eco_leak_uah;
	di->coul_dev_ops->save_cc_uah(cc);
}

void update_ocv_after_resume(struct smartstar_coul_device *di)
{
	int old_ocv;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;
	if ((di->sr_resume_time - di->charging_stop_time >
			OCV_UPDATE_DELTA_TIME * SEC_PER_MIN) &&
		di->multi_ocv_open_flag && (multi_ocv_could_update(di) != 0)) {
		record_ocv_cali_info(di);
		save_multi_ocv_and_level(di);
#ifdef CONFIG_COUL_POLAR
		clear_polar_err_b();
#endif
	} else if ((di->sr_resume_time - di->charging_stop_time >
			OCV_UPDATE_DELTA_TIME * SEC_PER_MIN) &&
		!di->multi_ocv_open_flag && (ocv_could_update(di) != 0)) {
		get_ocv_resume(di);
		di->last_ocv_level = OCV_LEVEL_0;
		di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
		set_ocv_cali_level(di->last_ocv_level);
	} else if ((di->batt_delta_rc >
			(int)di->batt_data->fcc * ABNORMAL_DELTA_SOC * TENTH) &&
			(di->charging_state != CHARGING_STATE_CHARGE_START) &&
			(di->sr_resume_time - di->charging_stop_time >
				OCV_UPDATE_DELTA_TIME * SEC_PER_MIN)) {
		old_ocv = di->batt_ocv;
		coul_core_info("Update ocv for delta_rc %d\n",
			       di->batt_delta_rc);

		get_ocv_resume(di);
		di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
		di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
		set_ocv_cali_level(di->last_ocv_level);
		if (old_ocv != di->batt_ocv) {
			/* for set NOT_UPDATE_fCC Flag */
			di->coul_dev_ops->save_ocv(di->batt_ocv, NOT_UPDATE_FCC);
			di->batt_ocv_valid_to_refresh_fcc = 0;
		}
	}
}

#endif

