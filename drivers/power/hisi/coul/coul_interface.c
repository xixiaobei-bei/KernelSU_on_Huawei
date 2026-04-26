/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: interface for coul module
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

#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#endif
#ifdef CONFIG_COUL_POLAR
#include "coul_update_polar_info.h"
#endif
#include "coul_nv.h"
#include "coul_private_interface.h"
#include "coul_temp.h"
#include "coul_interface.h"

#define TEMP_TENTH_DEGREE               10
#define DEFAULT_SOC_MONITOR_TEMP_MIN    10
#define DEFAULT_SOC_MONITOR_TEMP_MAX    45
#define MIN_PER_HOUR                    60
#define CAPACITY_MIN                    0
#define CAPACITY_CRITICAL               5
#define CAPACITY_LOW                    15
#define CAPACITY_HIGH                   95
#define CAPACITY_FULL                   100
#define SOC_VARY_LOW                    10
#define SOC_VARY_HIGH                   90
#define CHARGING_BEGIN_SOC_UPPER        80
#define CURRENT_FULL_SOC_UPPER          90
#define FCC_DEBOUNCE_LOWER              85
#define FCC_DEBOUNCE_UPPER              115

static int g_supply_temp_points[SUPPLY_PARA_LEVEL] = { 5, 0, -5, -10, -15 };

static int coul_get_low_temp_opt(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return LOW_TEMP_OPT_CLOSE;
	return di->low_temp_opt_flag;
}

/* Return: 0: Unknown, 1:Good, 2:Overheat, 3:Dead, 4:Over voltage, 5:Unspecified failure, 6:Cold */
static int coul_get_battery_health(void)
{
	int status, temp;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	if (!coul_is_battery_exist())
		return 0;

	status = POWER_SUPPLY_HEALTH_GOOD;
	temp = di->batt_temp / TENTH;
	if (temp < di->batt_temp_too_cold)
		status = POWER_SUPPLY_HEALTH_COLD;
	else if (temp > di->batt_temp_too_hot)
		status = POWER_SUPPLY_HEALTH_OVERHEAT;
	else if (di->batt_under_voltage_flag)
		status = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;

	return status;
}

/*
 * get voltage on ID pin by HKADC
 * should be modified to static after del battery_plug_in
 */
void get_battery_id_voltage_real(struct smartstar_coul_device *di)
{
	int volt;

	if (di == NULL)
		return;

#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
	bat_type_apply_mode(BAT_ID_VOLTAGE);
#endif
	volt = hisi_adc_get_adc(di->adc_batt_id);
#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
	bat_type_release_mode(false);
#endif
	/* volt negative means get adc fail */
	if (volt < 0) {
		coul_core_err("HKADC get battery id fail\n");
		volt = 0;
	}
	di->batt_id_vol = (unsigned int)volt;
	coul_core_info("get battery id voltage is %u mv\n", di->batt_id_vol);
}

static int coul_get_battery_id_vol(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;

	get_battery_id_voltage_real(di);
	return di->batt_id_vol;
}

 /* check wheather coul is ready 1: OK 0:not ready */
static int coul_is_ready(void)
{
	struct platform_device *pdev = get_coul_pdev();

	if (pdev != NULL)
		return 1;
	return 0;
}

/* check whether battery uah reach threshold 0:not, 4: lower than warning_lev, 8: lower than Low_Lev */
static int coul_is_battery_reach_threshold(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return -1;
	}

	if (!coul_is_battery_exist())
		return 0;

	if (di->batt_soc > BATTERY_CC_WARNING_LEV)
		return 0;
	if (di->batt_soc > (int)di->wakelock_low_batt_soc)
		return BQ_FLAG_SOC1;
	return BQ_FLAG_LOCK;
}

static char *coul_get_battery_brand(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return "default";

	if (di->batt_data->id_status == BAT_ID_VALID)
		return di->batt_data->batt_brand;

	return "default";
}

/* avg current_ma in 200 seconds */
static int coul_get_battery_current_avg_ma(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->last_iavg_ma;
}

int coul_get_battery_voltage_uv(void)
{
	struct smartstar_coul_device *di = get_coul_dev();
	int ibat_ua = 0;
	int vbat_uv = 0;

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return -1;
	}

	coul_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);
	return vbat_uv;
}

/* tte: time to empty value in min or -1----charging */
static int coul_get_battery_tte(void)
{
	int cc, cur;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;

	if (!coul_is_battery_exist())
		return 0;

	cur = di->coul_dev_ops->get_battery_current_ua();
	/* charging now */
	if (cur <= 0)
		return -1;

	cc = di->batt_ruc - di->batt_uuc;
	if (cc <= 0)
		return -1;

	return cc * MIN_PER_HOUR / cur;
}

/* time to full value in min or -1----discharging */
static int coul_get_battery_ttf(void)
{
	int cc, cur;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;

	if (!coul_is_battery_exist())
		return 0;

	cur = di->coul_dev_ops->get_battery_current_ua();
	/* discharging now */
	if (cur >= 0)
		return -1;

	cc = di->batt_fcc - di->batt_ruc;
	/* cur is < 0 */
	return cc * MIN_PER_HOUR / (-cur);
}

/* battery health level 0: Unknown, 1:CRITICAL, 2:LOW, 3:NORMAL, 4:HIGH, 5:FULL */
static int coul_get_battery_capacity_level(void)
{
	struct smartstar_coul_device *di = get_coul_dev();
	int data_capacity, status;

	if (di == NULL)
		return 0;

	if (!coul_is_battery_exist())
		return 0;
	data_capacity = di->batt_soc;
	if ((data_capacity > CAPACITY_FULL) || (data_capacity < CAPACITY_MIN))
		return POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	else if ((data_capacity >= CAPACITY_MIN) &&
		(data_capacity <= CAPACITY_CRITICAL))
		status = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if ((data_capacity > CAPACITY_CRITICAL) &&
		(data_capacity <= CAPACITY_LOW))
		status = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if ((data_capacity >= CAPACITY_HIGH) &&
		(data_capacity < CAPACITY_FULL))
		status = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else if (data_capacity == CAPACITY_FULL)
		status = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	else
		status = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	return status;
}

static int coul_get_battery_technology(void)
{
	/* Default technology is "Li-poly" */
	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

/* 0: invalid battery, 1: successed */
static struct chrg_para_lut *coul_get_battery_charge_params(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return NULL;
	return di->batt_data->chrg_para;
}

static int coul_get_battery_ifull(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_data->ifull;
}

/* battery vbat max vol */
static int coul_get_battery_vbat_max(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_data->vbatt_max;
}

static int coul_get_battery_limit_fcc(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_limit_fcc / PERMILLAGE;
}

static int coul_battery_cycle_count(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_chargecycles / PERCENT;
}

static int coul_battery_fcc_design(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_data->fcc;
}

/* check whether coul is ok 0: success  1: fail */
static int coul_device_check(void)
{
	return COUL_IC_GOOD;
}

static bool check_soc_vary(struct smartstar_coul_device *di,
	int soc_changed_abs, int last_record_soc, bool temp_stablity)
{
	if ((soc_changed_abs >= di->soc_monitor_limit) &&
		((last_record_soc > SOC_VARY_LOW) &&
		(last_record_soc < SOC_VARY_HIGH)) &&
		(temp_stablity == TRUE))
		return TRUE;
	return FALSE;
}

static int judge_soc_vary_valid(struct smartstar_coul_device *di,
	int monitor_flag, int *deta_soc)
{
	int ret, soc_changed_abs, current_record_soc, soc_changed;
	bool temp_stablity = FALSE;
	static int last_record_soc;

	/* Start up or resume, refresh record soc and return invalid data */
	if (di->soc_monitor_flag != STATUS_RUNNING) {
		last_record_soc = di->soc_unlimited;
		di->soc_monitor_flag = STATUS_RUNNING;
		return -1;
	}

	if ((di->batt_temp > DEFAULT_SOC_MONITOR_TEMP_MIN) &&
		(di->batt_temp < DEFAULT_SOC_MONITOR_TEMP_MAX))
		temp_stablity = TRUE;
	else
		temp_stablity = FALSE;

	if (monitor_flag == 1) {
		current_record_soc = di->soc_unlimited;
		soc_changed = current_record_soc - last_record_soc;
		if (soc_changed < 0)
			soc_changed_abs = -soc_changed;
		else
			soc_changed_abs = soc_changed;
		last_record_soc = current_record_soc;
		/* if needed, report soc error */
		if (check_soc_vary(di, soc_changed_abs,
			last_record_soc, temp_stablity)) {
			*deta_soc = soc_changed;
			coul_core_err("soc vary fast! soc_changed is %d\n",
				soc_changed);
			ret = 0;
		} else {
			ret = -1;
		}
	} else {
		if (temp_stablity == TRUE)
			ret = 0;
		else
			ret = -1;
	}
	return ret;
}

/*
 * during wake-up, monitor the soc variety Input: monitor flag
 * 0:monitoring in one period 1:one period done
 * Return: data valid:0: data is valid( soc err happened) others: data is invalid
 */
static int coul_get_soc_vary_flag(int monitor_flag, int *deta_soc)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return -1;
	}

	if (!coul_is_battery_exist()) {
		coul_core_info("battery not exist!\n");
		return -1;
	}

	return judge_soc_vary_valid(di, monitor_flag, deta_soc);
}

/* battery temperature in centigrade for charger */
static int coul_get_battery_temperature_for_charger(void)
{
	struct smartstar_coul_device *di = get_coul_dev();
	int temp;

	if (di == NULL) {
		coul_core_err("error, di is NULL, return default temp for charger\n");
		return DEFAULT_TEMP;
	}
	temp = get_temperature_stably(di, USER_CHARGER);
	return (temp / TENTH);
}

static int coul_get_battery_fifo_curr(unsigned int index)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_battery_cur_ua_from_fifo == NULL))
		return 0;
	return di->coul_dev_ops->get_battery_cur_ua_from_fifo((short)index);
}

static int coul_get_battery_fifo_vol(unsigned int index)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_battery_vol_uv_from_fifo == NULL))
		return 0;
	return di->coul_dev_ops->get_battery_vol_uv_from_fifo((short)index);
}

static int coul_get_battery_fifo_depth(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_fifo_depth == NULL))
		return 0;
	return di->coul_dev_ops->get_fifo_depth();
}

static int coul_get_battery_ufcapacity_tenth(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_soc_real;
}

static int coul_convert_regval2ua(unsigned int reg_val)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_regval2ua == NULL))
		return -1;
	return di->coul_dev_ops->convert_regval2ua(reg_val);
}

static int coul_convert_regval2uv(unsigned int reg_val)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_regval2uv == NULL))
		return -1;
	return di->coul_dev_ops->convert_regval2uv(reg_val);
}

static int coul_convert_regval2temp(unsigned int reg_val)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_regval2temp == NULL))
		return -1;
	return di->coul_dev_ops->convert_regval2temp(reg_val);
}

static int coul_convert_mv2regval(int vol_mv)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_uv2regval == NULL))
		return -1;
	return (int)di->coul_dev_ops->convert_uv2regval(vol_mv * PERMILLAGE);
}

static int coul_get_calibration_status(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_coul_calibration_status == NULL))
		return 0;
	return di->coul_dev_ops->get_coul_calibration_status();
}

static int get_batt_reset_flag(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_err("g_smartstar_coul_dev is null found in %s\n",
			      __func__);
		return -1;
	}
	return di->batt_reset_flag;
}

/* new battery, clear record fcc */
static void clear_record_fcc(struct smartstar_coul_device *di)
{
	int index;
	struct ss_coul_nv_info *pinfo = NULL;

	if (di == NULL) {
		coul_core_err(BASP_TAG"%s, input param NULL\n", __func__);
		return;
	}
	pinfo = &di->nv_info;
	/* clear learn fcc index check su */
	for (index = 0; index < MAX_RECORDS_CNT; index++)
		pinfo->real_fcc_record[index] = 0;
	pinfo->latest_record_index = 0;
	pinfo->fcc_check_sum_ext = 0;
	/* set save nv flag, clear nv */
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
}

static bool basp_check_sum(void)
{
	int i;
	int records_sum = 0;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return FALSE;
	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		records_sum += di->nv_info.real_fcc_record[i];
		coul_core_info(BASP_TAG"check fcc records, %d:%dmAh\n",
			       i, di->nv_info.real_fcc_record[i]);
	}
	if (records_sum != di->nv_info.fcc_check_sum_ext) {
		coul_core_info(BASP_TAG"check learn fcc valid, records_sum=[%d],check_sum=%d\n",
			       records_sum, di->nv_info.fcc_check_sum_ext);
		return FALSE;
	}
	return TRUE;
}

static int coul_get_record_fcc(unsigned int size, unsigned int *records)
{
	int i;
	int records_num = 0;
	int index;
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (records == NULL) || (size != MAX_RECORDS_CNT))
		return 0;
	index = di->nv_info.latest_record_index;
	if ((index >= MAX_RECORDS_CNT) || (index < 0))
		return 0;

	if (!basp_check_sum()) {
		clear_record_fcc(di);
		return 0;
	}

	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		if (di->nv_info.real_fcc_record[index] == 0)
			continue;
		records[i] = di->nv_info.real_fcc_record[index];
		records_num++;
		index++;
		if (index >= MAX_RECORDS_CNT)
			index = 0;
	}
	return records_num;
}

static int coul_get_last_powerdown_soc(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return -EPERM;
	return di->last_powerdown_soc;
}

static int coul_get_battery_temperature_raw(void)
{
	return coul_battery_temperature_tenth_degree(USER_COUL) /
	       TEMP_TENTH_DEGREE;
}

static int coul_battery_temperature_permille(void)
{
	return coul_retry_temp_permille(USER_COUL);
}

static int coul_get_battery_rpcb(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return 0;
	}
	return di->r_pcb;
}

/* be called when charge report current full */
static void coul_process_current_full(struct smartstar_coul_device *di)
{
	coul_core_info("[%s]++\n", __func__);
	if (di == NULL)
		return;
	if ((di->charging_state != CHARGING_STATE_CHARGE_START) ||
		(di->charging_begin_soc / TENTH >=  CHARGING_BEGIN_SOC_UPPER) ||
		(di->batt_soc_real / TENTH <= CURRENT_FULL_SOC_UPPER)) {
		coul_core_info("[%s]charging_state = %d,batt_soc = %d,charging_begin_soc=%d,do not update current_fcc_real!\n",
			__func__, di->charging_state,
			di->batt_soc_real, di->charging_begin_soc);
		return;
	}
	di->batt_report_full_fcc_real = di->batt_ruc;
	coul_core_info("[%s] batt_report_full_fcc_real %d, batt_report_full_fcc_cal %d battrm %d, battruc %d\n",
		__func__, di->batt_report_full_fcc_real,
		di->batt_report_full_fcc_cal, di->batt_rm, di->batt_ruc);
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	coul_core_info("[%s]--\n", __func__);
}

static void charger_event_process(struct smartstar_coul_device *di,
	unsigned int event)
{
	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		coul_core_info("receive charge start event = 0x%x\n", (int)event);
		/* record soc and cc value */
		di_lock();
		coul_charging_begin(di);
		di_unlock();
		break;

	case VCHRG_STOP_CHARGING_EVENT:
		coul_core_info("receive charge stop event = 0x%x\n", (int)event);
		di_lock();
		coul_charging_stop(di);
		di_unlock();
		break;

	case VCHRG_CURRENT_FULL_EVENT:
		coul_core_info("receive current full event = 0x%x\n", (int)event);
		di_lock();
		coul_process_current_full(di);
		di_unlock();
		break;

	case VCHRG_CHARGE_DONE_EVENT:
		coul_core_info("receive charge done event = 0x%x\n", (int)event);
		di_lock();
		coul_charging_done(di);
		di_unlock();
		break;

	case VCHRG_NOT_CHARGING_EVENT:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("charging is stop by fault\n");
		break;

	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("charging is stop by overvoltage\n");
		break;

	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("charging is stop by weaksource\n");
		break;

	default:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("unknow event %d\n", (int)event);
		break;
	}
}

/* package charger_event_process, and be registered in scharger Model to get charge event */
static int coul_battery_charger_event_rcv(unsigned int evt)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return -1;
	if (!di->batt_exist)
		return 0;

	if (!coul_is_battery_exist())
		return 0;

	charger_event_process(di, evt);
	return 0;
}

/* check whether fcc is debounce 0: no  1: is debounce */
static int coul_is_fcc_debounce(void)
{
	struct smartstar_coul_device *di = get_coul_dev();
	int batt_fcc;
	int fcc;

	if (di == NULL)
		return FALSE;

	if (!coul_is_battery_exist())
		return FALSE;

	batt_fcc = coul_get_battery_fcc();
	fcc = interpolate_fcc(di, di->batt_temp);
	if ((batt_fcc < fcc * FCC_DEBOUNCE_LOWER / PERCENT) ||
		(batt_fcc > fcc * FCC_DEBOUNCE_UPPER / PERCENT))
		coul_core_err("%s, fcc_from_temp=%d, batt_fcc=%d, ret=%d\n",
			__func__, fcc, batt_fcc, TRUE);

	return TRUE;
}

static int coul_get_ocv_by_soc(int temp, int soc)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->batt_data == NULL))
		return -EPERM;

	return interpolate_ocv(di->batt_data->pc_temp_ocv_lut, temp, soc);
}

static int coul_get_soc_by_ocv(int temp, int ocv)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->batt_data == NULL))
		return -EPERM;

	return interpolate_pc(di->batt_data->pc_temp_ocv_lut, temp, ocv);
}

/* get supply current with soc */
static int coul_judge_soc_supply_level(const struct smartstar_coul_device *di,
	int soc, int temp_idx)
{
	temp_idx = clamp_val(temp_idx, 0, SUPPLY_PARA_LEVEL - 1);
	coul_core_info("%s: soc:%d, temp_idx:%d", __func__, soc, temp_idx);
	if (soc > di->supply_info[temp_idx].soc_max)
		return CURR_AVG_DEFAULT;
	if (soc > di->supply_info[temp_idx].soc_min)
		return di->supply_info[temp_idx].supply_curr2;
	return di->supply_info[temp_idx].supply_curr1;
}

static bool coul_judge_update_temp_level(
	const struct smartstar_coul_device *di, int first_run, int temp,
	int last_temp, int temp_idx)
{
	temp_idx = clamp_val(temp_idx, 0, SUPPLY_PARA_LEVEL - 1);
	coul_core_info("%s: first_run:%d,temp:%d,last_temp:%d,last_i:%d",
		       __func__, first_run, temp, last_temp, temp_idx);
	if (first_run || (temp <= last_temp) ||
		(temp - g_supply_temp_points[temp_idx] >
			 di->supply_info[temp_idx].temp_back))
		return TRUE;
	return FALSE;
}

/* report supply current with thermal table */
/* need modify to static if del test_polar_supply_table */
int coul_get_polar_table_val(int temp, int soc)
{
	int i, ret_val, temp_max;
	static int last_i = SUPPLY_PARA_LEVEL - 1;
	static int last_temp = -20;
	static int first_run = 1;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	if (di->supply_enable == 0)
		return CURR_AVG_DEFAULT;

	for (i = 0; i < SUPPLY_PARA_LEVEL; i++) {
		if (temp > g_supply_temp_points[i])
			break;
	}

	temp_max = g_supply_temp_points[0] + di->supply_info[0].temp_back;
	/* in case polar is disabled or temp > 5 when first run */
	if ((first_run && i == 0) || temp > temp_max)
		return CURR_AVG_DEFAULT;

	/* update temp level when temp drop or temp resume back to threshold */
	if (coul_judge_update_temp_level(di, first_run, temp,
		last_temp, last_i - 1)) {
		ret_val = coul_judge_soc_supply_level(di, soc, i - 1);
		last_i = i;
	} else {
		ret_val = coul_judge_soc_supply_level(di, soc, last_i - 1);
	}
	first_run = 0;
	last_temp = temp;
	return ret_val;
}

/* report avg current as thermal node; use update_polar_params if need calculate */
static int coul_get_polar_avg(void)
{
	struct smartstar_coul_device *di = get_coul_dev();
	int temp = 0;

	if (di == NULL)
		return CURR_AVG_DEFAULT;
#ifdef CONFIG_COUL_POLAR
	if (di->batt_temp >= (TEMP_POLAR_REPORT * TENTH))
		temp = CURR_AVG_DEFAULT;
	else
		temp = get_coul_polar_avg();
#endif
	/* we use lut value instead, if polar supply is not calculated */
	if (temp == 0)
		temp = coul_get_polar_table_val(di->batt_soc,
			di->batt_temp / TENTH);
	temp = clamp_val(temp, 0, CURR_AVG_DEFAULT);
	return temp;
}

/* report peak current as thermal node; use update_polar_params if need calculate */
static int coul_get_polar_peak(void)
{
	struct smartstar_coul_device *di = get_coul_dev();
	int temp = 0;

	if (di == NULL)
		return CURR_AVG_DEFAULT;
#ifdef CONFIG_COUL_POLAR
	if (di->batt_temp >= (TEMP_POLAR_REPORT * TENTH))
		temp = CURR_AVG_DEFAULT;
	else
		temp = get_coul_polar_peak();
#endif
	if (temp == 0)
		temp = coul_get_polar_table_val(
			di->batt_soc, di->batt_temp / TENTH);
	temp = clamp_val(temp, 0, CURR_AVG_DEFAULT);
	return temp;
}

static int coul_update_basp_policy(unsigned int level,
	unsigned int nondc_volt_dec)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_err(BASP_TAG"[%s]input param NULL\n", __func__);
		return -1;
	}

	di->basp_level = level;
	di->nondc_volt_dec = nondc_volt_dec;
	di->qmax = coul_get_qmax(di);
	coul_core_info("bsoh update policy level %u, nondc_volt_dec %u\n",
		level, nondc_volt_dec);

	if (battery_para_changed(di) < 0)
		coul_core_err("battery charge para fail\n");
	return 0;
}

static int coul_cal_uah_by_ocv(int ocv_uv, int *ocv_soc_uah)
{
	int pc;
	s64 qmax;
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (!ocv_soc_uah)) {
		coul_core_err("[%s] para null\n", __func__);
		return ERROR;
	}
	qmax = coul_get_qmax(di);
	pc = interpolate_pc_high_precision(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp, ocv_uv / UVOLT_PER_MVOLT);

	coul_core_info("qmax = %llduAh, pc = %d/100000, ocv_soc = %llduAh\n",
		qmax, pc, qmax * pc / (SOC_FULL * PERMILLAGE));
	*ocv_soc_uah = (int)(qmax * pc / (SOC_FULL * PERMILLAGE));

	return ((*ocv_soc_uah > 0) ? SUCCESS : ERROR);
}

static void get_cur_calibration(int *cur_offset_a, int *cur_offset_b)
{
	int c_offset_a = DEFAULT_C_OFF_A;
	int c_offset_b = DEFAULT_C_OFF_B;

	if (cur_offset_a == NULL || cur_offset_b == NULL) {
		coul_core_err("%s input para is null\n", __func__);
		return;
	}

	get_coul_cali_params(NULL, NULL, &c_offset_a, &c_offset_b);
	*cur_offset_a = c_offset_a;
	*cur_offset_b = c_offset_b;
}

static void iscd_force_ocv_update(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return;

	get_ocv_by_vol(di);
	coul_core_info("iscd force ocv update: ocv %d, ocv_temp %d\n",
		       di->batt_ocv, di->batt_ocv_temp);
}

static int coul_interpolate_pc(int ocv)
{
	int pc;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	pc = interpolate_pc_high_precision(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp, ocv);
	return pc;
}

struct coulometer_ops g_coulometer_ops = {
	.is_coul_ready = coul_is_ready,
	.is_battery_exist = coul_is_battery_exist,
	.is_battery_reach_threshold = coul_is_battery_reach_threshold,
	.battery_brand = coul_get_battery_brand,
	.battery_id_voltage = coul_get_battery_id_vol,
	.battery_voltage = coul_get_battery_voltage_mv,
	.battery_voltage_uv = coul_get_battery_voltage_uv,
	.battery_current = coul_get_battery_current_ma,
	.battery_resistance = coul_get_battery_resistance,
	.fifo_avg_current = coul_get_fifo_avg_current_ma,
	.battery_current_avg = coul_get_battery_current_avg_ma,
	.battery_unfiltered_capacity = coul_battery_unfiltered_capacity,
	.battery_capacity = coul_get_battery_capacity,
	.battery_temperature = coul_get_battery_temperature,
	.chip_temperature = coul_get_chip_temp,
	.battery_temperature_for_charger =
		coul_get_battery_temperature_for_charger,
	.battery_rm = coul_get_battery_rm,
	.battery_fcc = coul_get_battery_fcc,
	.battery_fcc_design = coul_battery_fcc_design,
	.battery_tte = coul_get_battery_tte,
	.battery_ttf = coul_get_battery_ttf,
	.battery_health = coul_get_battery_health,
	.battery_capacity_level = coul_get_battery_capacity_level,
	.battery_technology = coul_get_battery_technology,
	.battery_charge_params = coul_get_battery_charge_params,
	.battery_vbat_max = coul_get_battery_vbat_max,
	.battery_ifull = coul_get_battery_ifull,
	.charger_event_rcv = coul_battery_charger_event_rcv,
	.battery_cycle_count = coul_battery_cycle_count,
	.get_battery_limit_fcc = coul_get_battery_limit_fcc,
	.coul_is_fcc_debounce = coul_is_fcc_debounce,
	.set_hltherm_flag = NULL,
	.get_hltherm_flag = NULL,
	.dev_check = coul_device_check,
	.get_soc_vary_flag = coul_get_soc_vary_flag,
	.coul_low_temp_opt = coul_get_low_temp_opt,
	.battery_cc = coul_get_battery_cc,
	.battery_fifo_curr = coul_get_battery_fifo_curr,
	.battery_fifo_vol = coul_get_battery_fifo_vol,
	.battery_rpcb = coul_get_battery_rpcb,
	.battery_fifo_depth = coul_get_battery_fifo_depth,
	.battery_ufcapacity_tenth = coul_get_battery_ufcapacity_tenth,
	.convert_regval2ua = coul_convert_regval2ua,
	.convert_regval2uv = coul_convert_regval2uv,
	.convert_regval2temp = coul_convert_regval2temp,
	.convert_mv2regval = coul_convert_mv2regval,
	.cal_uah_by_ocv = coul_cal_uah_by_ocv,
	.convert_temp_to_adc = coul_convert_temp_to_adc,
	.get_coul_calibration_status = coul_get_calibration_status,
	.battery_removed_before_boot = get_batt_reset_flag,
	.get_qmax = coul_get_battery_qmax,
	.get_ocv_by_soc = coul_get_ocv_by_soc,
	.get_soc_by_ocv = coul_get_soc_by_ocv,
	.get_ocv = coul_get_battery_ocv,
	.get_polar_avg = coul_get_polar_avg,
	.get_polar_peak = coul_get_polar_peak,
	.update_basp_policy = coul_update_basp_policy,
	.get_record_fcc = coul_get_record_fcc,
	.get_last_powerdown_soc = coul_get_last_powerdown_soc,
	.get_battery_temperature_raw = coul_get_battery_temperature_raw,
	.get_battery_temp_permille = coul_battery_temperature_permille,
	.get_c_calibration = get_cur_calibration,
	.force_ocv = iscd_force_ocv_update,
	.coul_interpolate_pc = coul_interpolate_pc,
};

struct coulometer_ops *get_coul_ops(void)
{
	return &g_coulometer_ops;
}

bool check_coul_dev(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_err("coul dev not exist in %s", __func__);
		return false;
	}
	return true;
}

unsigned int get_nondc_vol_dec(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->nondc_volt_dec;
}

int get_coul_dev_batt_temp(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_temp;
}

int update_coul_dev_qmax(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	di->qmax = coul_get_qmax(di);
	return di->qmax / PERMILLAGE;
}

int get_coul_dev_batt_rm(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_rm;
}

int get_coul_dev_batt_fcc(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_fcc;
}

int get_coul_dev_qmax(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->qmax;
}

int get_coul_dev_batt_chargecycles(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_chargecycles;
}

int get_coul_dev_batt_ruc(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->batt_ruc;
}

s64 get_iscd_full_update_cc(void)
{
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return 0;
	return di->iscd_full_update_cc;
}

