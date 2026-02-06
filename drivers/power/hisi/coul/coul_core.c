/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: coul_core.c
 *
 * smartstar coulometer functions
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

#include <securec.h>
#include <linux/power/hisi/coul/coul_event.h>
#include "coul_dts.h"
#include "coul_fault_evt_ops.h"
#include "coul_interface.h"
#include "coul_nv.h"
#include "coul_ocv_ops.h"
#include "coul_private_interface.h"
#include "coul_sysfs.h"
#include "coul_temp.h"
#ifdef CONFIG_COUL_POLAR
#include "coul_polar.h"
#include "coul_update_polar_info.h"
#endif
#include "coul_core.h"

static int g_last_charge_cycles;
static struct smartstar_coul_device *g_smartstar_coul_dev;
static struct platform_device *g_pdev;
static int g_sr_cur_state; /* 1:wakeup  2:sleep */
static int g_sr_suspend_temp;
static enum basp_fcc_learn_state g_basp_fcc_ls = LS_UNKNOWN;
static int coul_get_rm(struct smartstar_coul_device *di);
static int coul_pm_notify(struct notifier_block *nb,
	unsigned long mode, void *unused);
static int calculate_real_fcc_uah(struct smartstar_coul_device *di,
	int *ret_fcc_uah);

static ATOMIC_NOTIFIER_HEAD(g_coul_fault_notifier_list);
static int register_coul_fault_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&g_coul_fault_notifier_list, nb);
}

static int unregister_coul_fault_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&g_coul_fault_notifier_list, nb);
}

int coul_notify_fault(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(&g_coul_fault_notifier_list, val, v);
}

struct smartstar_coul_device *get_coul_dev(void)
{
	return g_smartstar_coul_dev;
}

struct platform_device *get_coul_pdev(void)
{
	return g_pdev;
}

int get_sr_cur_state(void)
{
	return g_sr_cur_state;
}

static void basp_fcc_learn_evt_handler(struct smartstar_coul_device *di,
	enum basp_fcc_learn_event evt)
{
	static enum basp_fcc_learn_state prev_state = LS_UNKNOWN;

	switch (evt) {
	case EVT_START:
		g_basp_fcc_ls = LS_INIT;
		break;
	case EVT_PER_CHECK:
		if ((g_basp_fcc_ls == LS_INIT) || (g_basp_fcc_ls == LS_GOOD)) {
			if ((di->batt_temp > BASP_FCC_LERAN_TEMP_MIN) &&
				(di->batt_temp < BASP_FCC_LERAN_TEMP_MAX))
				g_basp_fcc_ls = LS_GOOD;
			else
				g_basp_fcc_ls = LS_BAD;
		}
		break;
	case EVT_DONE:
		g_basp_fcc_ls = LS_UNKNOWN;
		break;
	default:
		break;
	}

	if (g_basp_fcc_ls != prev_state) {
		coul_core_info(BASP_TAG"prev_state:%d, new_state:%d, batt_temp:%d\n",
			prev_state, g_basp_fcc_ls, di->batt_temp);
		prev_state = g_basp_fcc_ls;
	}
}

static int calc_rm_chg_uah(struct smartstar_coul_device *di, int fcc_uah)
{
	int pc, remaining_charge_uah;

	pc = calculate_pc(di, di->batt_ocv, di->batt_ocv_temp,
		di->batt_chargecycles / PERCENT);
	remaining_charge_uah = (fcc_uah / PERMILLAGE) * pc;

	return remaining_charge_uah;
}

static int calc_uuc(struct smartstar_coul_device *di, int zero_voltage,
	int ratio, int i_ma)
{
	int i, unusable_uv, uuc_rbatt_uv, ocv_uv, rbatt_mohm;
	int prev_delta_uv = 0;
	int prev_rbatt_mohm = 0;

	for (i = 0; i <= SOC_FULL; i++) {
		ocv_uv = interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
			di->batt_temp / TENTH, i * TENTH) * PERMILLAGE;
		rbatt_mohm = get_rbatt(di, i, di->batt_temp);
		rbatt_mohm = rbatt_mohm * ratio / PERCENT;
		unusable_uv = (rbatt_mohm * i_ma) + (zero_voltage * PERMILLAGE);

		if (ocv_uv - unusable_uv > 0)
			break;

		prev_delta_uv = ocv_uv - unusable_uv;
		prev_rbatt_mohm = rbatt_mohm;
	}
	uuc_rbatt_uv = linear_interpolate(rbatt_mohm, ocv_uv - unusable_uv,
		prev_rbatt_mohm, prev_delta_uv, 0);

	unusable_uv = (uuc_rbatt_uv * i_ma) + (zero_voltage * PERMILLAGE);
	return calculate_pc(di, unusable_uv, di->batt_temp,
		di->batt_chargecycles / PERCENT);
}

static int modify_ratio(int batt_temp_degc, int ratio)
{
	if ((ratio > 0) && (ratio < RARIO_MIN) &&
		(batt_temp_degc < LOW_TEMP_UPPER))
		return RARIO_MIN;

	return ratio;
}

static int modify_curent(struct smartstar_coul_device *di, int i_ma)
{
	int new_i_ma = i_ma;
	int batt_temp_degc = di->batt_temp / PERCENT;
	const int low_temp_min_i_ma = UUC_MIN_CURRENT_MA +
		(batt_temp_degc - LOW_TEMP_MINUS_TWENTY_DEG) * TEMP_UUC_STEP;

	if (di->soc_limit_flag != STATUS_RUNNING) {
		if (batt_temp_degc < LOW_TEMP_UPPER)
			new_i_ma = (i_ma > low_temp_min_i_ma) ?
				i_ma : low_temp_min_i_ma;
		else
			new_i_ma = (i_ma > UUC_MIN_CURRENT_MA) ?
				i_ma : UUC_MIN_CURRENT_MA;
	}

	return new_i_ma;
}

/* Return: unuse uah without adjust */
static int calculate_termination_uuc_pc(
	struct smartstar_coul_device *di, int cur_ma)
{
	int zero_voltage, pc_unusable;
	int ratio = RARIO_MIN;
	int i_ma = cur_ma;

#if RBATT_ADJ
	if (di->rbatt_ratio) {
		ratio = di->rbatt_ratio;
		i_ma = di->last_fifo_iavg_ma;
		if (di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) {
			ratio = modify_ratio(di->batt_temp / PERCENT, ratio);
			i_ma = modify_curent(di, i_ma);
			coul_core_info("low_temp_opt:low_temp_opt old_ratio =%d, low_ratio =%d,old_i_ma = %d,low_i_ma = %d\n",
				di->rbatt_ratio, ratio,
				di->last_fifo_iavg_ma, i_ma);
		}
	}
#endif
	zero_voltage = get_zero_cap_vol(di);
	pc_unusable = calc_uuc(di, zero_voltage, ratio, i_ma);

	return pc_unusable;
}

/* adjust unuse uah, changes no more than 2% */
static int adjust_uuc(struct smartstar_coul_device *di, int fcc_uah,
	int new_pc_unusable, int new_uuc)
{
	int uuc_pc_step_add = UUC_PC_STEP_ADD;
	int uuc_pc_step_sub = UUC_PC_STEP_SUB;
	int uuc_pc_max_diff = UUC_PC_MAX_DIFF;

	/* start or wake allow jump */
	if ((di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) &&
		(di->soc_limit_flag != STATUS_RUNNING))
		uuc_pc_max_diff = LOW_TEMP_UUC_PC_MAX_DIFF;
	if ((di->prev_pc_unusable == -EINVAL) ||
		(abs(di->prev_pc_unusable - new_pc_unusable) <= uuc_pc_max_diff)) {
		di->prev_pc_unusable = new_pc_unusable;
		return new_uuc;
	}
	if (di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) {
		uuc_pc_step_add = LOW_TEMP_UUC_PC_STEP_ADD;
		uuc_pc_step_sub = LOW_TEMP_UUC_PC_STEP_SUB;
	}

	/* The UUC change in each cycle is reduced by half in low-temperature scenarios for soc smooth */
	if ((di->batt_soc > NORMAL_SOC_LIMIT) &&
		(di->batt_temp / TENTH < ZERO_V_ADJ_END_T))
		uuc_pc_step_add = uuc_pc_step_add / HALF;

	/* the uuc is trying to change more than 2% restrict it */
	if (new_pc_unusable > di->prev_pc_unusable)
		di->prev_pc_unusable += uuc_pc_step_add;
	else
		di->prev_pc_unusable -= uuc_pc_step_sub;

	new_uuc = (fcc_uah / PERMILLAGE) * di->prev_pc_unusable;

	return new_uuc;
}

/* Return: unuse uah without adjust and changes less than 2% */
static int calculate_unusable_charge_uah(
	struct smartstar_coul_device *di, int fcc_uah, int iavg_ua)
{
	int i;
	int pc_unusable;
	int uuc_uah_iavg;
	int iavg_ma = iavg_ua / PERMILLAGE;
	static int iavg_samples[IAVG_SAMPLES];
	static int iavg_index;
	static int iavg_num_samples;

	/* use a nominal avg current to keep a reasonable UUC while charging */
	iavg_ma = (iavg_ma < 0) ? CHARGING_IAVG_MA : iavg_ma;
	iavg_samples[iavg_index] = iavg_ma;
	iavg_index = (iavg_index + 1) % IAVG_SAMPLES;
	iavg_num_samples++;
	if (iavg_num_samples >= IAVG_SAMPLES)
		iavg_num_samples = IAVG_SAMPLES;

	/* now that this sample is added calcualte the average */
	iavg_ma = 0;
	if (iavg_num_samples != 0) {
		for (i = 0; i < iavg_num_samples; i++)
			iavg_ma += iavg_samples[i];
		iavg_ma = DIV_ROUND_CLOSEST(iavg_ma, iavg_num_samples);
	}

	pc_unusable = calculate_termination_uuc_pc(di, iavg_ma);
	uuc_uah_iavg = pc_unusable * (fcc_uah / PERMILLAGE);
	coul_core_info("RBATT_ADJ:UUC= %d uAh, pc= %d.%d\n",
		uuc_uah_iavg, pc_unusable / TENTH, pc_unusable % TENTH);

	di->rbatt_ratio = 0;
	/* restrict the uuc such that it can increase only by one percent */
	uuc_uah_iavg = adjust_uuc(di, fcc_uah, pc_unusable, uuc_uah_iavg);
	uuc_uah_iavg += fcc_uah / PERCENT;
	di->batt_uuc = uuc_uah_iavg;

	return uuc_uah_iavg;
}

/* recalculate the chargecycle after charging done */
static unsigned int recalc_chargecycles(struct smartstar_coul_device *di)
{
	int cc_end, real_fcc, fcc, pc, new_chargecycles;
	unsigned int retval = 0;

	if ((di->batt_soc == SOC_FULL) &&
		(di->charging_begin_soc / TENTH < MIN_BEGIN_PERCENT_FOR_LEARNING)) {
		cc_end = di->coul_dev_ops->calculate_cc_uah();
		real_fcc = (cc_end - di->charging_begin_cc) * PERMILLAGE /
			(SOC_FULL_TENTH - di->charging_begin_soc);
		fcc = interpolate_fcc(di, di->batt_temp);
		if (fcc == 0) {
			coul_core_err("%s Divisor fcc equal zero\n", __func__);
			pc = 0;
		} else {
			pc = real_fcc * PERCENT / fcc;
		}
		new_chargecycles =
			interpolate_single_y_lut(di->batt_data->fcc_sf_lut, pc);
		new_chargecycles -= MODIFY_NEW_BATT_CYCLE * PERCENT;
		retval = (unsigned int)((new_chargecycles > 0) ?
			new_chargecycles : 0);
		coul_core_info("trigger battery charge cycle reclac, val = %d\n",
			new_chargecycles);
	}

	return retval;
}

/* set the new battery charge cycles and notify all who care about */
static void set_charge_cycles(struct smartstar_coul_device *di,
	 const unsigned int cycles)
{
	di->batt_chargecycles = cycles;
	call_coul_blocking_notifiers(BATT_EEPROM_CYC, &di->batt_chargecycles);
}

/* update charge/discharge times */
static void update_chargecycles(struct smartstar_coul_device *di)
{
	int batt_soc_real, charging_begin_soc;

	batt_soc_real = di->batt_soc_real / TENTH;
	charging_begin_soc = di->charging_begin_soc / TENTH;
	if (batt_soc_real - charging_begin_soc > 0) {
		set_charge_cycles(di, di->batt_chargecycles +
			batt_soc_real - charging_begin_soc);
		coul_core_info("new chargecycle=%u, added=%d\n",
			di->batt_chargecycles, batt_soc_real - charging_begin_soc);
		/* update battery backup nv chargecycles */
		hw_coul_update_chargecycles(di);
	} else {
		coul_core_info("chargecycle not updated, soc_begin=%d, soc_current=%d, batt_soc=%d\n",
			charging_begin_soc, batt_soc_real, di->batt_soc);
	}
	if (battery_para_changed(di) < 0)
		coul_core_err("battery charge para fail\n");
	di->charging_begin_soc = SOC_FULL_TENTH;
}

/* Determine if high precision qmax can update, Remark: Depending on the FCC's update condition */
static void is_high_precision_qmax_ready_to_refresh(
	struct smartstar_coul_device *di)
{
	di->qmax_start_pc = interpolate_pc(di->batt_data->pc_temp_ocv_lut0,
		di->batt_ocv_temp, di->batt_ocv / PERMILLAGE) / TENTH;
	di->qmax_cc = di->coul_dev_ops->calculate_cc_uah() / UA_PER_MA;

	/* start pc is lower than 20% */
	if (di->qmax_start_pc < MIN_BEGIN_PERCENT_FOR_QMAX)
		di->qmax_refresh_flag = 1;
	coul_core_info("[%s] start_ocv = %d, start_pc = %d, cc = %d\n",
		__func__, di->batt_ocv, di->qmax_start_pc, di->qmax_cc);
}

/* For Repair Network NFF tool detection only Dependencies on OCV Updates */
static int get_high_pre_qmax(struct smartstar_coul_device *di)
{
	int tmp_qmax;
	int design_fcc_mah;
	int delta_cv_pc;

	if (di->qmax_refresh_flag) {
		di->qmax_end_pc = interpolate_pc(di->batt_data->pc_temp_ocv_lut0,
			di->batt_ocv_temp, di->batt_ocv / PERMILLAGE) / TENTH;
		design_fcc_mah = interpolate_fcc(di, di->batt_ocv_temp);
		/* get the percent of power after the CV is lowered */
		delta_cv_pc = basp_full_pc_by_voltage(di) / TENTH;
		if (!delta_cv_pc)
			delta_cv_pc = PERCENT;
		/* calculate qmax */
		if (di->qmax_end_pc - di->qmax_start_pc != 0) {
			tmp_qmax = (-di->qmax_cc) * PERCENT * di->qmax_end_pc /
				(di->qmax_end_pc - di->qmax_start_pc) /
				delta_cv_pc;
		} else {
			coul_core_err("[%s] qmax_end_pc = %d, start_pc = %d, delta_cv_pc = %d\n",
				__func__, di->qmax_end_pc,
				di->qmax_start_pc, delta_cv_pc);
			return -1;
		}

		/* limit qmax max */
		if (tmp_qmax >
			design_fcc_mah * FCC_UPPER_LIMIT_PERCENT / PERCENT) {
			coul_core_info("[%s] qmax = %d, over design\n",
				__func__, tmp_qmax);
			tmp_qmax = design_fcc_mah *
				FCC_UPPER_LIMIT_PERCENT / PERCENT;
		}
		/* clear qmax refresh flag, prevent continuous calculation */
		di->qmax_refresh_flag = 0;
		di->high_pre_qmax = tmp_qmax;

		coul_core_info("[%s] qmax =%d, start_pc =%d, end_pc =%d, delta_cv_pc =%d\n",
			__func__, di->high_pre_qmax, di->qmax_start_pc,
			di->qmax_end_pc, delta_cv_pc);
		return 0;
	}

	coul_core_info("[%s] not update\n", __func__);
	return -1;
}

/* calculate ocv by 10 history data when AP exist from deep sleep */
void get_ocv_by_vol(struct smartstar_coul_device *di)
{
	int voltage_uv, rm;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;

	voltage_uv = get_ocv_vol_from_fifo(di);
	if (voltage_uv == 0)
		return;

	if (is_in_capacity_dense_area(voltage_uv)) {
		coul_core_info("do not update OCV %d\n", voltage_uv);
		return;
	}
	coul_core_info("awake from deep sleep, old OCV = %d\n", di->batt_ocv);
	di->batt_ocv = voltage_uv;
	di->batt_ocv_temp = di->batt_temp;
	di->coul_dev_ops->save_ocv_temp((short)di->batt_ocv_temp);
	di->batt_ocv_valid_to_refresh_fcc = 1;
	record_ocv_cali_info(di);
	coul_clear_cc_register();
	coul_clear_coul_time();
#ifdef CONFIG_COUL_POLAR
	clear_polar_err_b();
#endif
	di->coul_dev_ops->save_ocv(voltage_uv, IS_UPDATE_FCC);
	coul_core_info("awake from deep sleep, new OCV = %d,fcc_flag=%d\n",
		di->batt_ocv, di->batt_ocv_valid_to_refresh_fcc);
	dbg_cnt_inc(dbg_ocv_cng_0);
	if (di->charging_state == CHARGING_STATE_CHARGE_DONE) {
		rm = coul_get_rm(di);
		if (rm < di->batt_limit_fcc) {
			di->batt_limit_fcc = rm * PERCENT / LIMIT_FCC_CAL_RATIO;
			di->is_nv_need_save = 1;
			di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
		}
		/* update qmax */
		if (!get_high_pre_qmax(di)) {
			di->is_nv_need_save = 1;
			di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
		}
	}
	batt_coul_notify(di, BATT_OCV_UPDATE);
}

/* get delta_rc if ratio < 0 */
static int get_delta_rc(struct smartstar_coul_device *di, int ocv,
	int rbatt_tbl, int pc_new, int avg_c)
{
	int pc_new_100;
	int delta_ocv_100 = 0;
	int delta_pc_100 = 0;
	int delta_rc_uah_100 = 0;
	int delta_pc, delta_rc_uah;
	int delta_rc_final = 0;

	delta_pc = pc_new - di->batt_soc_real;
	delta_rc_uah = di->batt_fcc / PERMILLAGE * delta_pc;
	if (di->rbatt_ratio <= 0) {
		delta_ocv_100 = -rbatt_tbl * avg_c / PERMILLAGE;
		pc_new_100 = interpolate_pc(di->batt_data->pc_temp_ocv_lut,
			di->batt_temp, ocv - delta_ocv_100);
		delta_pc_100 = pc_new_100 - di->batt_soc_real;
		delta_rc_uah_100 = di->batt_fcc / PERMILLAGE * delta_pc_100;
		delta_rc_final = delta_rc_uah - delta_rc_uah_100;
	}
	coul_core_info("RBATT_ADJ: delta_pc=%d.%d delta_rc_uah=%d "
	       "delta_ocv_100=%d delta_pc_100=%d.%d delta_rc_uah_100=%d "
	       "delta_rc_final=%d\n",
	       delta_pc / TENTH, (int)abs(delta_pc % TENTH), delta_rc_uah,
	       delta_ocv_100, delta_pc_100 / TENTH,
	       (int)abs(delta_pc_100 % TENTH), delta_rc_uah_100, delta_rc_final);

	return delta_rc_final;
}

static void calc_ratio(struct smartstar_coul_device *di, int rbatt_tbl,
	int rbatt_calc)
{
	int ratio;

	if (rbatt_tbl == 0) {
		coul_core_err("%s Divisor rbatt_tbl equal zero\n", __func__);
		ratio = 0;
	} else {
		ratio = rbatt_calc * PERCENT / rbatt_tbl;
		coul_core_info("low_temp_opt: old ratio =%d\n", ratio);
	}
	if ((di->soc_limit_flag == STATUS_WAKEUP) &&
		(di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) &&
		(di->batt_temp / TENTH < LOW_TEMP_UPPER)) {
		ratio = (ratio > RARIO_MIN) ? ratio : RARIO_MIN;
		coul_core_info("low_temp_opt: new ratio = %d\n", ratio);
	}

	di->rbatt_ratio = ratio;
	coul_core_info("RBATT_ADJ: ratio = %d\n", ratio);
}

static int cal_pc_new(struct smartstar_coul_device *di, int rbatt_tbl,
	int rbatt_calc, int ocv, int avg_c)
{
	int pc_new, delta_ocv, ocv_new;

	delta_ocv = (rbatt_calc - rbatt_tbl) * avg_c / PERMILLAGE;
	ocv_new = ocv - delta_ocv;
	pc_new = interpolate_pc(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp, ocv_new);
	coul_core_info("RBATT_ADJ: delta_ocv=%d\n", delta_ocv);

	return pc_new;
}

static int cal_delta_rc_final(struct smartstar_coul_device *di,
	int rbatt_tbl, int fcc_uah, struct vcdata *vc, int *soc_new)
{
	int ocv, rbatt_calc, pc_new, delta_rc_final, rc_new_uah;

	ocv = interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp / TENTH, di->batt_soc_real);

	rbatt_calc = (ocv - vc->avg_v) * PERMILLAGE / vc->avg_c;

	update_ocv_cali_rbatt(di, vc->avg_c, rbatt_calc);

	calc_ratio(di, rbatt_tbl, rbatt_calc);

	pc_new = cal_pc_new(di, rbatt_tbl, rbatt_calc, ocv, vc->avg_c);

	delta_rc_final = get_delta_rc(di, ocv, rbatt_tbl, pc_new, vc->avg_c);

	rc_new_uah = di->batt_fcc / PERMILLAGE * pc_new;
	if (fcc_uah == 0) {
		coul_core_err("%s Divisor fcc_uah equal zero\n", __func__);
		*soc_new = 0;
	} else {
		*soc_new = rc_new_uah * PERCENT / fcc_uah;
	}
	*soc_new = bound_soc(*soc_new);
	coul_core_info("RBATT_ADJ: soc_new=%d rbat_calc=%d rbat_tbl=%d last_ocv=%d "
		"ocv_temp=%d soc=%d.%d, ocv=%d delta_rc_final=%d\n",
		*soc_new, rbatt_calc, rbatt_tbl, di->batt_ocv, di->batt_ocv_temp,
		di->batt_soc_real / TENTH, di->batt_soc_real % TENTH, ocv, delta_rc_final);

	return delta_rc_final;
}

static int calculate_delta_rc(struct smartstar_coul_device *di,
	int rbatt_tbl, int fcc_uah)
{
	int ibat_ua = 0;
	int vbat_uv = 0;
	int delta_rc_final = 0;
	int soc_new = -EINVAL;
	struct vcdata vc = {0};

	coul_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);

	di->coul_dev_ops->get_fifo_avg_data(&vc);
	vc.avg_v += (di->r_pcb / PERMILLAGE) * (vc.avg_c) / PERMILLAGE;

	/* exit sleep, cal uuc */
	if ((di->soc_limit_flag == STATUS_WAKEUP) &&
		(di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN)) {
		/* sensorhub charging or charge done */
		if (ibat_ua / PERMILLAGE < CHARGING_CUR_MA)
			goto out;
		coul_core_info("low_temp_opt: old_vc.avg_c=%d, old vc.avg_v=%d\n",
			vc.avg_c, vc.avg_v);
		vc.avg_c = (ibat_ua / PERMILLAGE > UUC_MIN_CURRENT_MA) ?
			(ibat_ua / PERMILLAGE) : UUC_MIN_CURRENT_MA;
		vc.avg_v = vbat_uv / PERMILLAGE;
		coul_core_info("low_temp_opt: new_vc.avg_c=%d, new vc.avg_v=%d\n",
			vc.avg_c, vc.avg_v);
	} else {
		if (vc.avg_c < FIFO_AVG_CUR)
			goto out;
	}

	if (di->coul_dev_ops->get_delta_rc_ignore_flag()) {
		if (!((di->batt_temp / TENTH < LOW_TEMP_FIVE_DEG) &&
			(di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN))) {
			coul_core_info("first ignore delta_rc\n");
			goto out;
		}
	}

	di->last_fifo_iavg_ma = vc.avg_c;

	delta_rc_final = cal_delta_rc_final(di, rbatt_tbl, fcc_uah, &vc, &soc_new);

out:
	coul_core_info("RBATT_ADJ: c=%d u=%d cmin=%d cmax=%d cavg=%d vavg=%d, soc_new=%d\n",
		ibat_ua, vbat_uv, vc.min_c, vc.max_c, vc.avg_c, vc.avg_v, soc_new);
	di->batt_soc_est = soc_new;

	return delta_rc_final;
}

static int adjust_soc(struct smartstar_coul_device *di, int soc)
{
	int ibat_ua = 0;
	int vbat_uv = 0;
	int delta_soc = 0;
	int zero_vol;
	int n = 0;
	int soc_new = soc;
	int soc_est_avg = 0;
	static int soc_ests[SOC_EST_NUM] = { SOC_FULL, SOC_FULL, SOC_FULL };
	static int i;

	coul_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);

	if ((ibat_ua < -CHARGING_CURRENT_OFFSET) || (di->batt_soc_est < 0))
		goto out;

	soc_ests[i % SOC_EST_NUM] = di->batt_soc_est;
	i++;

	soc_est_avg = DIV_ROUND_CLOSEST((soc_ests[0] + soc_ests[1] + soc_ests[2]),
		SOC_EST_NUM);
	delta_soc = soc - soc_est_avg;
	if ((soc_est_avg > SOC_TWO) || (soc <= soc_est_avg))
		goto out;

	zero_vol = get_zero_cap_vol(di);
	if ((vbat_uv / UA_PER_MA > zero_vol + ZERO_V_COPM_MV) &&
		(di->batt_temp / TENTH < ZERO_V_ADJ_START_T))
		goto out;

	n = SOC_THREE - soc_est_avg;
	soc_new = soc - delta_soc * n / SOC_THREE;

	if ((get_temperature_stably(di, USER_COUL) > TEMP_OCV_ALLOW_CLEAR *
		TENTH) && (delta_soc > ABNORMAL_DELTA_SOC)) {
		coul_core_info("delta_soc = %d, mark save ocv is invalid\n",
			delta_soc);
		di->coul_dev_ops->clear_ocv();
		di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
		di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
		di->batt_ocv_valid_to_refresh_fcc = 0;
	}
out:
	coul_core_info("soc_est_avg=%d delta_soc=%d n=%d\n",
		soc_est_avg, delta_soc, n);
	soc_new = bound_soc(soc_new);

	return soc_new;
}

static int limit_soc_during_running(struct smartstar_coul_device *di,
	int current_ua, int input_soc, int last_soc)
{
	int output_soc;
	/* soc can not increase during discharging */
	if (current_ua >= CHARGING_CURRENT_OFFSET) {
		if (last_soc - input_soc >= 1) {
			if ((di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) &&
				(last_soc - input_soc >= SOC_VARY_MAX) &&
				(di->batt_temp < LOW_TEMP_MINUS_FIVE_TENTH))
				output_soc = last_soc - SOC_VARY_MAX;
			else
				output_soc = last_soc - 1;
		} else {
			output_soc = last_soc;
		}
	} else {
		if (input_soc - last_soc >= 1)
			output_soc = last_soc + 1;
		else
			output_soc = last_soc;
	}
	return output_soc;
}

static int limit_soc_after_resume(struct smartstar_coul_device *di,
	int current_ua, int input_soc, int last_soc)
{
	int output_soc = input_soc;

	coul_core_info("current_ua: %d, last_soc: %d, input_soc: %d",
		current_ua, last_soc, input_soc);
	if ((current_ua >= CHARGING_CURRENT_OFFSET) ||
		(di->charging_state == CHARGING_STATE_CHARGE_STOP)) {
		if (last_soc < input_soc)
			output_soc = last_soc;
	} else {
		if (last_soc > input_soc)
			output_soc = last_soc;
	}
	return output_soc;
}

static int limit_soc(struct smartstar_coul_device *di, int input_soc)
{
	int output_soc = input_soc;
	int last_soc;
	static int power_on_cnt;
	int current_ua = 0;
	int voltage_uv = 0;

	last_soc = di->batt_soc;
	coul_get_battery_voltage_and_current(di, &current_ua, &voltage_uv);
	/* change <=1% */
	if (di->soc_limit_flag == STATUS_RUNNING)
		output_soc = limit_soc_during_running(di, current_ua, input_soc,
			last_soc);
	/* exist from sleep */
	else if (di->soc_limit_flag == STATUS_WAKEUP)
		output_soc = limit_soc_after_resume(di, current_ua, input_soc,
			last_soc);

	/* charge_done, then soc 100% */
	if (di->charging_state == CHARGING_STATE_CHARGE_DONE) {
		coul_core_info("pre_chargedone output_soc = %d\n", output_soc);
		output_soc = SOC_FULL;
	}
	if ((di->charging_state == CHARGING_STATE_CHARGE_START) &&
		(voltage_uv / PERMILLAGE > BATTERY_SWITCH_ON_VOLTAGE) &&
		(output_soc == 0) &&
		((current_ua < -CHARGING_CURRENT_OFFSET) ||
		(power_on_cnt < POWER_ON_CNT_MAX)))
		output_soc = SOC_ONE;

	if ((di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) &&
		(di->soc_limit_flag != STATUS_RUNNING)) {
		if ((output_soc < SOC_TWO) &&
			(coul_get_battery_voltage_mv() > THREE_PERCENT_SOC_VOL)) {
			output_soc = SOC_THREE;
			coul_core_err("low_temp_opt: soc < 2, vol > 3500, soc = 3\n");
		}
	}

	power_on_cnt++;
	return output_soc;
}

/* limit delta_rc 1% change */
static int adjust_delta_rc(struct smartstar_coul_device *di,
	int delta_rc, int fcc_uah)
{
	int max_changeable_delta_rc = fcc_uah * MAX_DELTA_RC_PC / PERCENT;

	if (abs(di->batt_pre_delta_rc - delta_rc) <= max_changeable_delta_rc) {
		di->batt_pre_delta_rc = delta_rc;
		return delta_rc;
	}
	coul_core_info("delta_rc change exceed 1 percents, pre = %d, current = %d\n",
			di->batt_pre_delta_rc, delta_rc);
	if (di->batt_pre_delta_rc > delta_rc)
		di->batt_pre_delta_rc -= max_changeable_delta_rc;
	else
		di->batt_pre_delta_rc += max_changeable_delta_rc;
	return di->batt_pre_delta_rc;
}

static void calculate_soc_params(struct smartstar_coul_device *di,
	struct soc_param_data *soc_params)
{
	int soc_rbatt, delt_rc, delta_cc, delta_time, iavg_ua;
	static int first_in = 1;

	/* calc fcc by cc and soc change */
	soc_params->fcc_uah = calculate_fcc_uah(di);
	di->batt_fcc = soc_params->fcc_uah;

	/* calculate remainging charge */
	soc_params->remaining_charge_uah = calc_rm_chg_uah(di, soc_params->fcc_uah);
	di->batt_rm = soc_params->remaining_charge_uah;

	/* calculate cc micro_volt_hour */
	di->cc_end_value = di->coul_dev_ops->calculate_cc_uah();
	soc_params->cc_uah = di->cc_end_value;

	di->batt_ruc = soc_params->remaining_charge_uah - soc_params->cc_uah;
	di->get_cc_end_time = di->coul_dev_ops->get_coul_time();
	if (di->batt_fcc == 0)
		di->batt_soc_real = 0;
	else
		di->batt_soc_real = DIV_ROUND_CLOSEST(di->batt_ruc, di->batt_fcc / PERMILLAGE);
	coul_core_info("SOC real = %d\n", di->batt_soc_real);

	soc_rbatt = di->batt_soc_real / TENTH;
	soc_rbatt = (soc_rbatt > 0) ? soc_rbatt : 0;
	soc_params->rbatt = get_rbatt(di, soc_rbatt, di->batt_temp);

#if RBATT_ADJ
	delt_rc = calculate_delta_rc(di, soc_params->rbatt, soc_params->fcc_uah);
	soc_params->delta_rc_uah = adjust_delta_rc(di, delt_rc, di->batt_fcc);
	di->batt_delta_rc = soc_params->delta_rc_uah;
#endif

	if (first_in) {
		iavg_ua = di->coul_dev_ops->get_battery_current_ua();
		first_in = 0;
	} else {
		delta_cc = di->cc_end_value - di->last_cc;
		delta_time = di->get_cc_end_time - di->last_time;

		if (delta_time > 0)
			iavg_ua = div_s64((s64)delta_cc * SEC_PER_HOUR, delta_time);
		else
			iavg_ua = di->coul_dev_ops->get_battery_current_ua();

		coul_core_info("delta_time=%d, i_ua=%d\n", delta_time, iavg_ua);
	}
	di->last_cc = di->cc_end_value;
	di->last_time = di->get_cc_end_time;

	calculate_iavg_ma(di, iavg_ua);

	soc_params->unusable_charge_uah =
		calculate_unusable_charge_uah(di, soc_params->fcc_uah, iavg_ua);
	coul_core_info("FCC=%duAh, UUC=%duAh, RC=%duAh, CC=%duAh, delta_RC=%duAh, Rbatt=%dmOhm\n",
		soc_params->fcc_uah, soc_params->unusable_charge_uah,
		soc_params->remaining_charge_uah, soc_params->cc_uah,
		soc_params->delta_rc_uah, soc_params->rbatt);
}

static int current_full_adjust_limit_fcc(struct smartstar_coul_device *di)
{
	if (!di->batt_report_full_fcc_cal) {
		if (!di->batt_report_full_fcc_real)
			di->batt_report_full_fcc_cal =
				di->batt_fcc * di->soc_at_term / SOC_FULL;
		else
			di->batt_report_full_fcc_cal =
				min(di->batt_report_full_fcc_real,
				di->batt_fcc * di->soc_at_term / SOC_FULL);
	}

	if (!di->batt_limit_fcc)
		di->batt_limit_fcc = di->batt_report_full_fcc_cal;

	if (di->batt_limit_fcc_begin &&
		(di->charging_state == CHARGING_STATE_CHARGE_START) &&
			(di->charging_begin_soc / TENTH <=
				CUR_FULL_CHG_BEGIN_SOC_UPPER) &&
			(di->batt_soc_real / TENTH <= CURRENT_FULL_TERM_SOC))
		di->batt_limit_fcc = di->batt_limit_fcc_begin +
			(di->batt_soc_real - di->charging_begin_soc) *
			(di->batt_report_full_fcc_cal - di->batt_limit_fcc_begin) /
			(CURRENT_FULL_TERM_SOC * TENTH - di->charging_begin_soc);

	coul_core_info("[%s] limit_fcc %d, full_fcc_cal %d, limit_fcc_begin %d, soc_real %d, begin_soc %d\n",
		       __func__, di->batt_limit_fcc,
		       di->batt_report_full_fcc_cal, di->batt_limit_fcc_begin,
		       di->batt_soc_real, di->charging_begin_soc);

	return di->batt_limit_fcc;
}

static bool adjust_fcc_uah(struct smartstar_coul_device *di,
	struct soc_param_data *soc_params)
{
	bool soc_at_term_flag = true;

	if (di->enable_current_full) {
		soc_params->fcc_uah = current_full_adjust_limit_fcc(di);
		soc_at_term_flag = false;
	} else  {
		if (di->batt_limit_fcc &&
			(di->batt_limit_fcc <
				soc_params->fcc_uah * di->soc_at_term / SOC_FULL)) {
			soc_at_term_flag = false;
			coul_core_info("FCC = %duAh term flag= %d\n",
				soc_params->fcc_uah, soc_at_term_flag);
		}

		if (di->batt_limit_fcc &&
			(di->batt_limit_fcc < soc_params->fcc_uah)) {
			soc_params->fcc_uah = di->batt_limit_fcc;
			coul_core_info("use limit_FCC %duAh\n",
				soc_params->fcc_uah);
		}
	}
	/* 100 is to fit the percentage */
	if (soc_params->fcc_uah < INVALID_FCC_UAH)
		soc_params->fcc_uah = di->batt_fcc * di->soc_at_term / SOC_FULL;

	return soc_at_term_flag;
}

static int calculate_state_of_charge(struct smartstar_coul_device *di)
{
	int rm_usable_chg_uah, soc, soc_no_uuc, soc_before_adjust;
	bool soc_at_term_flag = true;
	struct soc_param_data soc_params = {0};

	coul_core_info("coul_board_type: board_type = %u, batt_exist = %d\n",
		       di->is_board_type, di->batt_exist);
	if (!di->batt_exist)
		return 0;

	check_chg_done_max_avg_cur_flag(di);

	calculate_soc_params(di, &soc_params);

	di->rbatt = soc_params.rbatt;

	soc_at_term_flag = adjust_fcc_uah(di, &soc_params);
	if (soc_params.fcc_uah / PERCENT == 0)
		soc = 0;
	else
		soc = DIV_ROUND_CLOSEST(
			(soc_params.remaining_charge_uah - soc_params.cc_uah),
			(soc_params.fcc_uah / PERCENT));

	soc_no_uuc = soc;

	rm_usable_chg_uah = soc_params.remaining_charge_uah -
		soc_params.cc_uah - soc_params.unusable_charge_uah +
		soc_params.delta_rc_uah;

	if (soc_params.fcc_uah - soc_params.unusable_charge_uah <= 0) {
		soc = 0;
	} else {
		if ((di->soc_at_term == SOC_FULL) || !soc_at_term_flag)
			soc = DIV_ROUND_CLOSEST((rm_usable_chg_uah),
				((soc_params.fcc_uah - soc_params.unusable_charge_uah) / PERCENT));
		else
			soc = DIV_ROUND_CLOSEST((rm_usable_chg_uah), ((soc_params.fcc_uah -
				soc_params.unusable_charge_uah) *
				(di->soc_at_term) / SOC_FULL / PERCENT));
	}
	soc = min(soc, SOC_FULL);
	soc_before_adjust = soc;
	soc = adjust_soc(di, soc);
	di->soc_unlimited = soc;
	/* not exiting from ECO Mode capacity can not change more than 1% */
	soc = limit_soc(di, soc);
	judge_eco_leak_uah(soc);
	coul_core_info("SOC without UUC = %d, SOC before adjust = %d, SOC before limit = %d, SOC after limit = %d\n",
		soc_no_uuc, soc_before_adjust, di->soc_unlimited, soc);

	/* default is no battery in sft and udp, so here soc is fixed 20 to prevent low power reset */
	if (di->is_board_type != BAT_BOARD_ASIC) {
		soc = max(soc, DEFAULT_SOC);
		coul_core_info("SFT and udp board: adjust Battery Capacity to %d Percents\n", soc);
	}
	di->batt_soc = soc;

	return soc;
}

/* get remain capacity */
static int coul_get_rm(struct smartstar_coul_device *di)
{
	int rm;
	struct soc_param_data soc_params = {0};

	calculate_soc_params(di, &soc_params);
	rm = soc_params.remaining_charge_uah - soc_params.cc_uah;

	return rm;
}

static void calc_initial_ocv(struct smartstar_coul_device *di)
{
#if (defined(CONFIG_HUAWEI_CHARGER_AP))
	int old_charge_state;

	old_charge_state = charge_set_charge_state(0);
#endif
	coul_cali_adc(di);
	/* 2.2s for calibration, 0.11s for sampling, and 0.19s for pad */
	mdelay(CALI_ADC_DELAY_MS);
	di->batt_ocv_temp = di->batt_temp;
	di->coul_dev_ops->save_ocv_temp((short)di->batt_ocv_temp);
	di->batt_ocv = coul_get_battery_voltage_mv() * PERMILLAGE;
	di->coul_dev_ops->save_ocv(di->batt_ocv, NOT_UPDATE_FCC);
#if (defined(CONFIG_HUAWEI_CHARGER_AP))
	charge_set_charge_state(old_charge_state);
#endif
	coul_clear_cc_register();
	coul_clear_coul_time();

	coul_core_info("OCV = %d\n", di->batt_ocv);
}

static void battery_plug_in(struct smartstar_coul_device *di)
{
	struct blocking_notifier_head *notifier_list = NULL;

	coul_core_info("%s: Enter\n", __func__);
	get_notifier_list(&notifier_list);

	di->batt_exist = 1;
	/* set battery data */
	get_battery_id_voltage_real(di);
	di->batt_data = get_battery_data(di->batt_id_vol);
	if (di->batt_data != NULL) {
		coul_core_info("%s: batt ID is %u\n",
			__func__, di->batt_id_vol);
	} else {
		coul_core_err("%s: %d di->batt_data is NULL\n",
			__func__, __LINE__);
		return;
	}
	update_battery_temperature(di, TEMPERATURE_INIT_STATUS);
	/* calculate first soc */
	calc_initial_ocv(di);

	di->charging_stop_time = di->coul_dev_ops->get_coul_time();
	di->last_iavg_ma = IMPOSSIBLE_IAVG;
	di->prev_pc_unusable = -EINVAL;
	di->sr_resume_time = di->coul_dev_ops->get_coul_time();
	g_sr_cur_state = SR_DEVICE_WAKEUP;

	set_charge_cycles(di, 0);
	di->batt_changed_flag = 1;
	di->batt_limit_fcc = 0;
	di->adjusted_fcc_temp_lut = NULL;
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	coul_core_info("new battery plug in, reset chargecycles\n");
	di->nv_info.report_full_fcc_real = 0;

	/* get the first soc value */
	di_lock();
	di->soc_limit_flag = STATUS_START;
	di->batt_soc = calculate_state_of_charge(di);
	di->soc_limit_flag = STATUS_RUNNING;
	di_unlock();

	coul_set_low_vol_int(di, LOW_INT_STATE_RUNNING);

	/* schedule calculate_soc_work */
	queue_delayed_work(system_power_efficient_wq,
		&di->calculate_soc_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(di->soc_work_interval)));

	/* save battery plug in magic number */
	di->coul_dev_ops->set_battery_moved_magic_num(BATTERY_PLUG_IN);
	blocking_notifier_call_chain(notifier_list, BATTERY_MOVE, NULL);

	coul_core_info("%s: Exit\n", __func__);
}

static void battery_plug_out(struct smartstar_coul_device *di)
{
	struct blocking_notifier_head *notifier_list = NULL;

	get_notifier_list(&notifier_list);

	coul_core_info("%s: Enter\n", __func__);

	di->batt_exist = 0;
	blocking_notifier_call_chain(notifier_list, BATTERY_MOVE, NULL);

	cancel_delayed_work(&di->calculate_soc_delayed_work);

	/* save battery move magic number */
	di->coul_dev_ops->set_battery_moved_magic_num(BATTERY_PLUG_OUT);

	/* clear saved ocv */
	di->coul_dev_ops->clear_ocv();
	di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
	di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
	/* clear saved last soc */
	di->coul_dev_ops->clear_last_soc_flag();

	coul_core_info("%s: Exit\n", __func__);
}

static void battery_check_work(struct work_struct *work)
{
	int batt_exist;
	struct smartstar_coul_device *di = container_of(work,
		struct smartstar_coul_device, battery_check_delayed_work.work);

	batt_exist = coul_is_battery_exist();
	if (batt_exist != di->batt_exist) {
		if (batt_exist)
			battery_plug_in(di);
		else
			battery_plug_out(di);
	}

	queue_delayed_work(system_power_efficient_wq,
		&di->battery_check_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(BATTERY_CHECK_TIME_MS)));
}

static void basp_record_fcc(struct smartstar_coul_device *di)
{
	int index, i;
	int sum = 0;
	struct ss_coul_nv_info *pinfo = NULL;
#ifdef CONFIG_HUAWEI_DSM
	char buff[DSM_BUFF_SIZE_MAX] = {0};
#endif

	pinfo = &di->nv_info;
	index = pinfo->latest_record_index % MAX_RECORDS_CNT;
	pinfo->real_fcc_record[index] = di->fcc_real_mah;
	pinfo->latest_record_index = index + 1;
	coul_core_info(BASP_TAG"[%s], learn times = %d, index = %d\n",
		__func__, pinfo->latest_record_index, index);
	for (i = 0; i < MAX_RECORDS_CNT; i++)
		sum = sum + pinfo->real_fcc_record[i];
	pinfo->fcc_check_sum_ext = sum;

#ifdef CONFIG_HUAWEI_DSM
	snprintf(buff, (size_t)DSM_BUFF_SIZE_MAX, "fcc_real_mah:%d, batt_brand:%s, batt_fcc:%d, charging_begin_soc:%d, "
		 "batt_chargecycles:%u, batt_ocv:%d, basp_level:%u\n",
		 di->fcc_real_mah, di->batt_data->batt_brand,
		 di->batt_fcc / PERMILLAGE, di->charging_begin_soc,
		 di->batt_chargecycles / PERCENT, di->batt_ocv, di->basp_level);
	coul_dsm_report_ocv_cali_info(di, ERROR_SAFE_PLOICY_LEARN, buff);
#endif
}

static bool check_ocv_valid(struct smartstar_coul_device *di)
{
	if (((di->batt_ocv > OCV_3200_UV) && (di->batt_ocv < OCV_3670_UV)) ||
		((di->batt_ocv > OCV_3690_UV) && (di->batt_ocv < OCV_3730_UV)) ||
		((di->batt_ocv > OCV_3800_UV) && (di->batt_ocv < OCV_3900_UV)))
		return true;
	return false;
}

static void basp_refresh_fcc(struct smartstar_coul_device *di)
{
	int fcc_uah, new_fcc_uah, delta_fcc_uah, max_delta_fcc_uah;

	if ((g_basp_fcc_ls == LS_GOOD) &&
		((di->batt_temp > BASP_FCC_LERAN_TEMP_MIN) &&
		(di->batt_temp < BASP_FCC_LERAN_TEMP_MAX)) &&
		(di->charging_begin_soc / TENTH < MIN_BEGIN_PERCENT_FOR_SAFE) &&
		di->batt_ocv_valid_to_refresh_fcc && check_ocv_valid(di)) {
		new_fcc_uah = calculate_real_fcc_uah(di, &fcc_uah);
		max_delta_fcc_uah = interpolate_fcc(di, di->batt_temp) *
			DELTA_SAFE_FCC_PERCENT * TENTH;
		delta_fcc_uah = new_fcc_uah - fcc_uah;
		if (delta_fcc_uah < 0)
			delta_fcc_uah = -delta_fcc_uah;
		if (delta_fcc_uah > max_delta_fcc_uah) {
			/* new_fcc_uah is outside the scope limit it */
			if (new_fcc_uah > fcc_uah)
				new_fcc_uah = (fcc_uah + max_delta_fcc_uah);
			else
				new_fcc_uah = (fcc_uah - max_delta_fcc_uah);
			coul_core_info(BASP_TAG"delta_fcc=%d > %d percent of fcc = %d restring it to %d\n",
				       delta_fcc_uah, DELTA_SAFE_FCC_PERCENT,
				       fcc_uah, new_fcc_uah);
		}
		di->fcc_real_mah = new_fcc_uah / PERMILLAGE;
		coul_core_info(BASP_TAG"refresh_fcc, start soc=%d, new fcc=%d\n",
			       di->charging_begin_soc, di->fcc_real_mah);
		/* record fcc */
		basp_record_fcc(di);
	} else {
		coul_core_err(BASP_TAG"[%s], basp_fcc_ls:%d, batt_temp:%d, charging_begin_soc:%d, "
			"ocv_valid:%d, batt_ocv:%d\n",
			 __func__, g_basp_fcc_ls, di->batt_temp,
			di->charging_begin_soc,
			di->batt_ocv_valid_to_refresh_fcc, di->batt_ocv);
	}
	basp_fcc_learn_evt_handler(di, EVT_DONE);
}

static void coul_check_drained_battery_flag(struct smartstar_coul_device *di)
{
	char buff[DSM_BUFF_SIZE_MAX] = {0};
	int drained_battery_flag = false;
	int ibat_ua = 0;
	int vbat_uv = 0;
	int ret;

	if (!strstr(saved_command_line, "androidboot.mode=normal") ||
		!strstr(saved_command_line, "androidboot.swtype=normal")) {
		coul_core_err("%s not a normal version, do nothing\n", __func__);
		return;
	}
	if (di->coul_dev_ops->get_drained_battery_flag &&
		di->coul_dev_ops->get_drained_battery_flag())
		drained_battery_flag = true;

	coul_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);
	ret = snprintf_s(buff, sizeof(buff), sizeof(buff) - 1,
		"last_cycles:%d,cur_cylces:%u,vol=%d,cur=%d\n",
		g_last_charge_cycles, di->batt_chargecycles, vbat_uv, ibat_ua);
	if (ret < 0) {
		coul_core_err("%s: snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}
	coul_core_info("changed_flag=%d, drained_flag=%d,vol=%d,cur=%d\n",
		di->batt_changed_flag, drained_battery_flag, vbat_uv, ibat_ua);

#ifdef CONFIG_HUAWEI_DSM
	if (drained_battery_flag)
		coul_dsm_report_ocv_cali_info(di, DSM_BATTERY_DRAINED_NO, buff);
	else if (di->batt_changed_flag)
		coul_dsm_report_ocv_cali_info(di, DSM_BATTERY_CHANGED_NO, buff);
#endif

	if (di->coul_dev_ops->clear_drained_battery_flag)
		di->coul_dev_ops->clear_drained_battery_flag();
}

static void judge_update_ocv_after_charge_done(
	struct smartstar_coul_device *di, int sleep_cc, int sleep_time)
{
	int sleep_current;

	if (sleep_time <= 0) {
		coul_core_info("sleep time < 0\n");
	} else {
		/* uah/s = (mah/1000)/(s/3600) */
		sleep_current = (sleep_cc * SEC_PER_HOUR_DIV_200) /
			(sleep_time * PERMILLAGE_DIV_200);

		if (sleep_current < 0)
			sleep_current = -sleep_current;
		coul_core_info("sleep_current = %d\n", sleep_current);

		if (sleep_current < CHG_DONE_SLEEP_CUR_UPPER) {
			di->last_ocv_level = OCV_LEVEL_0;
			di->coul_dev_ops->save_ocv_level(di->last_ocv_level);
			get_ocv_by_vol(di);
		}
	}
}

static void cal_soc(struct smartstar_coul_device *di)
{
	static struct static_soc_data data = {0};
	int ocv_time_inc, sleep_cc, sleep_time, time_now;

	di_lock();
	/* calc soc */
	di->batt_soc = calculate_state_of_charge(di);
	/* Here coul must calibrate! when first */
	if (data.cali_cnt % (CALIBRATE_INTERVAL / di->soc_work_interval) == 0) {
		if (get_pl_calibration_en() == FALSE)
			coul_cali_adc(di);
		else
			coul_core_info("pl_calibration_en == TRUE, do not calibrate coul\n");
	} else if (data.cali_cnt % (CALIBRATE_INTERVAL /
		di->soc_work_interval) == 1) {
		di->coul_dev_ops->show_key_reg();
	}
	data.cali_cnt++;
	if (di->charging_state == CHARGING_STATE_CHARGE_DONE) {
		if (data.charged_cnt == 0) {
			data.last_cc = di->coul_dev_ops->calculate_cc_uah();
			data.last_time = di->coul_dev_ops->get_coul_time();
			data.charging_done_ocv_enter_time =
				hisi_getcurtime() / NSEC_PER_SEC;
			/* charge done need sleep by CEC, limit SR OCV update time */
			di->charging_stop_time =
				di->coul_dev_ops->get_coul_time();
		}

		ocv_time_inc = hisi_getcurtime() / NSEC_PER_SEC -
			data.charging_done_ocv_enter_time;
		data.charged_cnt++;

		if (ocv_time_inc >= CHARGED_OCV_UPDATE_INTERVAL_S) {
			data.charging_done_ocv_enter_time =
				hisi_getcurtime() / NSEC_PER_SEC;
			sleep_cc = di->coul_dev_ops->calculate_cc_uah();
			sleep_cc = sleep_cc - data.last_cc;
			time_now = di->coul_dev_ops->get_coul_time();
			sleep_time = time_now - data.last_time;
			coul_core_info("sleep_cc=%d, sleep_time=%d\n",
				sleep_cc, sleep_time);

			judge_update_ocv_after_charge_done(di, sleep_cc,
				sleep_time);

			data.last_cc = di->coul_dev_ops->calculate_cc_uah();
			data.last_time = di->coul_dev_ops->get_coul_time();
		}
		/* acr check condition and notify */
		coul_start_soh_check();
	} else {
		data.charged_cnt = 0;
	}
	di_unlock();
}

/* calculate soc every(schedule workqueue) CALCULATE_SOC_MS */
static void calculate_soc_work(struct work_struct *work)
{
	struct smartstar_coul_device *di = container_of(work,
		struct smartstar_coul_device, calculate_soc_delayed_work.work);
	int ret;
	short offset_cur_modify_val;
	static int sum_time;
	struct blocking_notifier_head *notifier_list = NULL;

	/* get battery backup nv info */
	hw_coul_get_nv(di);
	if (di->is_nv_need_save) {
		ret = save_nv_info(di);
		if (!ret)
			di->is_nv_need_save = 0;
	}
	if (!di->batt_exist) {
		coul_core_info("battery not exist, do not calc soc any more\n");
		return;
	}

	if (sum_time >= 0) {
		sum_time += di->soc_work_interval;
		if (sum_time >= DELAY_REPORT_DMD_TIME) {
			coul_check_drained_battery_flag(di);
			sum_time = -1;
		}
	}

	coul_set_low_vol_int(di, LOW_INT_STATE_RUNNING);
	basp_fcc_learn_evt_handler(di, EVT_PER_CHECK);

	offset_cur_modify_val = di->coul_dev_ops->get_offset_current_mod();
	coul_core_info("offset_cur_modify_val:0x%x\n", offset_cur_modify_val);
	if (offset_cur_modify_val != 0)
		coul_core_err("curexception, offset_cur_modify_val:0x%x\n",
			offset_cur_modify_val);
	offset_cur_modify_val = di->coul_dev_ops->get_offset_vol_mod();
	if (offset_cur_modify_val != 0) {
		di->coul_dev_ops->set_offset_vol_mod();
		coul_core_err("curexception, offset_vol_modify_val:0x%x\n",
		offset_cur_modify_val);
	}

	cal_soc(di);
	coul_set_work_interval(di);
	/* work faster when capacity <= 3% */
	if (di->batt_soc <= BATTERY_CC_LOW_LEV) {
		coul_core_info("SMARTSTAR SHUTDOWN SOC LEVEL\n");
		get_notifier_list(&notifier_list);
		blocking_notifier_call_chain(notifier_list,
			BATTERY_LOW_SHUTDOWN, NULL);
	}
	queue_delayed_work(system_power_efficient_wq,
		&di->calculate_soc_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(di->soc_work_interval)));
}

static void read_temperature_work(struct work_struct *work)
{
#ifdef CONFIG_COUL_POLAR
	int ocv_soc_mv;
	int curr_now = 0;
	int vol_now = 0;
#endif

	struct smartstar_coul_device *di =
		container_of(work, struct smartstar_coul_device,
			read_temperature_delayed_work.work);

	update_battery_temperature(di, TEMPERATURE_UPDATE_STATUS);
#ifdef CONFIG_COUL_POLAR
	ocv_soc_mv = interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp / TENTH, di->batt_soc_real);
	coul_get_battery_voltage_and_current(di, &curr_now, &vol_now);
	update_polar_params(ocv_soc_mv, curr_now, vol_now, TRUE);
#endif
	queue_delayed_work(system_power_efficient_wq,
		&di->read_temperature_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));
}

/*
 * be called when charge begin, update batt_ocv_valid_to_refresh_fcc flag by ocv update long,
 * because cc err that affects fcc accuracy is larger in long interval of ocv update.
 * ocv time limit(T):  current [200ma,]: NA current [50ma, 100ma]: T<4H
 * current [100ma, 200ma]: T<8H current [,50ma]: NA;
 */
static void fcc_update_limit_by_ocv(struct smartstar_coul_device *di)
{
	int ocv_update_time, delta_cc_uah;
	int iavg_ma = 0;

	if (di->coul_dev_ops == NULL) {
		coul_core_err("%s, di is null\n", __func__);
		return;
	}
	if (!di->batt_ocv_valid_to_refresh_fcc)
		return;
	ocv_update_time = di->coul_dev_ops->get_coul_time();
	delta_cc_uah = di->coul_dev_ops->calculate_cc_uah();
	/* ocv update begin */
	if (ocv_update_time > 0)
		iavg_ma = (div_s64((s64)delta_cc_uah * SEC_PER_HOUR,
			ocv_update_time)) / UA_PER_MA;

	if (iavg_ma > IAVG_MA_200) {
		coul_core_info("%s:current [200ma,]=%dma\n", __func__, iavg_ma);
	} else if (iavg_ma > IAVG_MA_100) {
		if (ocv_update_time > (FOUR_HOUR * SEC_PER_HOUR))
			di->batt_ocv_valid_to_refresh_fcc = 0;
		coul_core_info("%s: current [100ma,200]= %d,t=%d\n",
				__func__, iavg_ma, ocv_update_time);
	} else if (iavg_ma > IAVG_MA_50) {
		if (ocv_update_time > (EIGHT_HOUR * SEC_PER_HOUR))
			di->batt_ocv_valid_to_refresh_fcc = 0;
		coul_core_info("%s:current [50ma,100]= %d,t=%d\n",
			__func__, iavg_ma, ocv_update_time);
	} else {
		coul_core_info("%s:current[,50ma]=%dma,NA\n", __func__, iavg_ma);
	}
	coul_core_info("[%s]:fcc_flag = %d\n",
		__func__, di->batt_ocv_valid_to_refresh_fcc);
}

/* be called when charge begin, update charge status, calc soc, begin cc, can't be called in atomic context */
void coul_charging_begin(struct smartstar_coul_device *di)
{
	coul_core_info("%s +\n", __func__);
	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return;
	}
	coul_core_info("pre charging state is %d\n", di->charging_state);

	if (di->charging_state == CHARGING_STATE_CHARGE_START)
		return;

	di->charging_state = CHARGING_STATE_CHARGE_START;

	if (di->fcc_update_limit_flag)
		fcc_update_limit_by_ocv(di);

	/* calculate soc again */
	di->batt_soc = calculate_state_of_charge(di);

	/* record soc of charging begin */
	di->charging_begin_soc = di->batt_soc_real;
	di->batt_limit_fcc_begin = di->batt_limit_fcc;
	di->batt_report_full_fcc_cal =
		min(di->batt_fcc * di->soc_at_term / SOC_FULL,
			di->batt_report_full_fcc_real);
	basp_fcc_learn_evt_handler(di, EVT_START);

	/* record cc value */
	di->charging_begin_cc = di->coul_dev_ops->calculate_cc_uah();
	di->charging_begin_time = di->coul_dev_ops->get_coul_time();

#ifdef CONFIG_COUL_POLAR
	clear_ishort_first_para();
	update_polar_info_chgdone(0);
#endif

	coul_core_info("%s -\n", __func__);
	coul_core_info("batt_soc=%d, charging_begin_soc=%d, charging_begin_cc=%d,batt_limit_fcc_begin =%d\n",
		di->batt_soc, di->charging_begin_soc,
		di->charging_begin_cc, di->batt_limit_fcc_begin);
}

/*
 * be called when charge stop, update charge status, update chargecycles
 * calc soc, cc, rm and set low vol reg. can't be called in atomic context
 */
void coul_charging_stop(struct smartstar_coul_device *di)
{
	int rm, cc, fcc_101;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_err("NULL point in %s\n", __func__);
		return;
	}
	fcc_101 = di->batt_fcc * LIMIT_FCC_CAL_RATIO / SOC_FULL;
	if (di->charging_state == CHARGING_STATE_CHARGE_UNKNOW)
		return;

	di->coul_dev_ops->irq_enable();
	di->batt_soc = calculate_state_of_charge(di);

	if (di->charging_state == CHARGING_STATE_CHARGE_START) {
		update_chargecycles(di);
		di->is_nv_need_save = 1;
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	}
	if (di->fcc_update_limit_flag)
		di->batt_ocv_valid_to_refresh_fcc = 0;
	cc = di->coul_dev_ops->calculate_cc_uah();

	rm = di->batt_rm - cc;
	/* adjust rm */
	if (rm > fcc_101) {
		cc = cc + (rm - fcc_101);
		di->coul_dev_ops->save_cc_uah(cc);
		di->batt_limit_fcc = 0;
		di->is_nv_need_save = 1;
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	} else if ((di->batt_soc == SOC_FULL) &&
		(di->batt_soc_real > SOC_TO_CAL_LIMIT_FCC)) {
		di->batt_limit_fcc = rm * SOC_FULL / LIMIT_FCC_CAL_RATIO;
		di->is_nv_need_save = 1;
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	}

	if (di->is_nv_need_save)
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	di->charging_state = CHARGING_STATE_CHARGE_STOP;
	di->charging_stop_soc = di->batt_soc_real;
	di->charging_stop_time = di->coul_dev_ops->get_coul_time();
	coul_set_low_vol_int(di, LOW_INT_STATE_RUNNING);
}

/* calc fcc by cc_change /soc_change */
static int calculate_real_fcc_uah(struct smartstar_coul_device *di,
	int *ret_fcc_uah)
{
	int real_fcc_uah, terminate_soc_real;
	struct soc_param_data soc_params = {0};

	terminate_soc_real = di->batt_soc_real;
	calculate_soc_params(di, &soc_params);
	*ret_fcc_uah = soc_params.fcc_uah;
	if (terminate_soc_real == di->charging_begin_soc)
		real_fcc_uah = *ret_fcc_uah;
	else
		real_fcc_uah = (-(soc_params.cc_uah - di->charging_begin_cc)) /
			(terminate_soc_real - di->charging_begin_soc) *
			terminate_soc_real;

	coul_core_info("real_fcc=%d, RC=%d CC=%d fcc=%d charging_begin_soc=%d.%d\n",
		real_fcc_uah, soc_params.remaining_charge_uah,
		soc_params.cc_uah, soc_params.fcc_uah,
		di->charging_begin_soc / TENTH, di->charging_begin_soc);

	return real_fcc_uah;
}

/* fcc self_study, establish a temp_fcc lookup table */
static void readjust_fcc_table(struct smartstar_coul_device *di)
{
	struct single_row_lut *temp = NULL;
	struct single_row_lut *now = NULL;
	int i, fcc, ratio;

	if (!di->batt_data->fcc_temp_lut) {
		coul_core_err("%s The static fcc lut table is NULL\n", __func__);
		return;
	}
	if (di->adjusted_fcc_temp_lut == NULL) {
		temp = &di->adjusted_fcc_temp_lut_tbl1;
		now = di->batt_data->fcc_temp_lut;
	} else if (di->adjusted_fcc_temp_lut == &di->adjusted_fcc_temp_lut_tbl1) {
		temp = &di->adjusted_fcc_temp_lut_tbl2;
		now = di->batt_data->fcc_temp_lut;
	} else {
		temp = &di->adjusted_fcc_temp_lut_tbl1;
		now = di->batt_data->fcc_temp_lut;
	}

	fcc = interpolate_fcc(di, di->batt_temp);
	temp->cols = now->cols;
	for (i = 0; i < now->cols; i++) {
		temp->x[i] = now->x[i];
		ratio = div_u64(((u64)(now->y[i]) * PERMILLAGE), fcc);
		temp->y[i] = ratio * di->fcc_real_mah;
		temp->y[i] /= PERMILLAGE;
		coul_core_info("temp=%d, staticfcc=%d, adjfcc=%d, ratio=%d\n",
			temp->x[i], now->y[i], temp->y[i], ratio);
	}
	di->adjusted_fcc_temp_lut = temp;
}

static bool is_fcc_ready_to_refresh(struct smartstar_coul_device *di,
	int charging_iavg_ma)
{
	if ((di->charging_begin_soc / TENTH < MIN_BEGIN_PERCENT_FOR_LEARNING) &&
		((di->batt_temp > FCC_UPDATE_TEMP_MIN) &&
		(di->batt_temp < FCC_UPDATE_TEMP_MAX)) &&
		(di->batt_ocv_temp > FCC_UPDATE_TEMP_MIN) &&
		(charging_iavg_ma < -FCC_UPDATE_CHARGING_CURR_MIN_MA) &&
		check_ocv_valid(di))
		return TRUE;

	return FALSE;
}

static int init_lut_by_backup_nv(struct single_row_lut *fcc_lut,
	const struct hw_coul_nv_info *nv_info)
{
	int i;

	for (i = 0; i < MAX_TEMPS; i++) {
		if (nv_info->real_fcc[i] == 0)
			break;

		if (nv_info->real_fcc[i] < INVALID_FCC_UAH) {
			coul_core_info("%s:real fcc in back nv is not currect\n",
				__func__);
			return 0;
		}

		fcc_lut->x[i] = nv_info->temp[i];
		fcc_lut->y[i] = nv_info->real_fcc[i];
	}

	if (i == 0) {
		coul_core_info("%s:no real fcc data in back nv\n", __func__);
		return 0;
	}

	fcc_lut->cols = i;
	return 1;
}

static void adjust_cyclestostore(struct smartstar_coul_device *di,
	int avg_fcc, int design_fcc_mah)
{
	int cylces2restore, old_fcc, delta_avg_old;
	struct single_row_lut *back_fcc_lut = NULL;
	struct hw_coul_nv_info batt_backup_nv_info;

	batt_backup_nv_info = get_batt_backup_nv_info();
	back_fcc_lut = &di->adjusted_fcc_temp_lut_tbl1;
	/* If the new FCC greater than design_fcc_mah*97/100, the battery is new */
	if (avg_fcc >= design_fcc_mah * NEW_BATT_DIFF_RADIO / PERCENT) {
		cylces2restore = di->batt_chargecycles;
		coul_core_err("%s:new battery, charge_cycle = %d\n",
			__func__, cylces2restore);
		goto valid_restore;
	}

	if (di->batt_backup_nv_flag &&
		init_lut_by_backup_nv(back_fcc_lut, &batt_backup_nv_info)) {
		old_fcc = PERMILLAGE * interpolate_single_lut(back_fcc_lut,
			di->batt_temp / TENTH);
		delta_avg_old = (avg_fcc >= old_fcc) ?
			avg_fcc - old_fcc : old_fcc - avg_fcc;
		coul_core_info("%s:read backup nv, old_fcc = %d, old_cycle = %d\n",
			__func__, old_fcc, batt_backup_nv_info.charge_cycles);
		if (delta_avg_old <= design_fcc_mah * OLD_BATT_DIFF_RADIO / PERCENT) {
			cylces2restore = batt_backup_nv_info.charge_cycles;
			coul_core_err("%s:old battery, charge_cycle = %d\n",
				__func__, cylces2restore);
			goto valid_restore;
		}
	}

	cylces2restore = PERCENT * interpolate_single_y_lut(
		di->batt_data->fcc_sf_lut, avg_fcc * PERCENT / design_fcc_mah);
	coul_core_err("%s:change other old battery, charge_cycle = %d\n",
		__func__, cylces2restore);

valid_restore:
	di->nv_info.change_battery_learn_flag = 0;
	di->batt_chargecycles = (unsigned int)cylces2restore;
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
}

static void check_restore_cycles(struct smartstar_coul_device *di,
	int new_fcc, int last_fcc, int avg_fcc)
{
	int delta_last_new, design_fcc_mah;

	if ((new_fcc <= 0) || (last_fcc <= 0)) {
		coul_core_err("%s: invalid input\n", __func__);
		return;
	}

	design_fcc_mah = PERMILLAGE * di->batt_data->fcc;
	/* After two FCC self-learning, restore chargecycle */
	if (!di->nv_info.change_battery_learn_flag)
		return;
	if (di->nv_info.change_battery_learn_flag == CHANGE_BATTERY_MOVE) {
		coul_core_info("%s:first learning, do not restore, flag = %d\n",
			__func__, di->nv_info.change_battery_learn_flag);
		di->nv_info.change_battery_learn_flag = CHANGE_BATTERY_NEED_RESTORE;
		return;
	}
	if (di->batt_chargecycles >= RESTORE_CYCLE_MAX) {
		di->nv_info.change_battery_learn_flag = 0;
		coul_core_info("%s:charge cycle is %u more then max_cycle, not restore\n",
			__func__, di->batt_chargecycles);
		return;
	}

	delta_last_new = (new_fcc >= last_fcc) ?
		new_fcc - last_fcc : last_fcc - new_fcc;
	coul_core_info("%s:design_fcc_mah = %d, avg_fcc = %d\n",
		__func__, design_fcc_mah, avg_fcc);

	/* If the FCC difference of the two learning is greater than design_fcc_mah*5/100, invalid and return */
	if (delta_last_new >= design_fcc_mah * DELTA_FCC_INVALID_RADIO / PERCENT) {
		coul_core_info("%s:delta_last_new = %d, new FCC is invalid, once again\n",
			__func__, delta_last_new);
		return;
	}

	adjust_cyclestostore(di, avg_fcc, design_fcc_mah);
}

static int get_fcc_after_cv_adjust(struct smartstar_coul_device *di, int fcc_mah)
{
	int delta_cv_pc;
	int real_fcc_mah = fcc_mah;

	if (di->nondc_volt_dec > BASP_FATAL_VDEC_TH) {
		delta_cv_pc = basp_full_pc_by_voltage(di) / TENTH;
		if (!delta_cv_pc)
			delta_cv_pc = PERCENT;
		real_fcc_mah = fcc_mah * PERCENT / delta_cv_pc;
		coul_core_info("basp fatal dec adjust fcc %d, delta_cv_pc %d\n",
			fcc_mah, delta_cv_pc);
	}
	return real_fcc_mah;
}

static int calc_charging_iavg_ma(struct smartstar_coul_device *di)
{
	int delta_cc_uah, charging_time, charging_iavg_ma;

	charging_time =
		di->coul_dev_ops->get_coul_time() - di->charging_begin_time;
	delta_cc_uah =
		di->coul_dev_ops->calculate_cc_uah() - di->charging_begin_cc;
	if (charging_time > 0)
		charging_iavg_ma =
			(div_s64((s64)delta_cc_uah * SEC_PER_HOUR,
				charging_time)) / UA_PER_MA;
	else
		charging_iavg_ma = 0;
	coul_core_info("[%s]:charging_time = %ds,delta_cc_uah = %duah,charging_iavg_ma = %dma\n",
		__func__, charging_time, delta_cc_uah, charging_iavg_ma);

	return charging_iavg_ma;
}

static void limit_delta_fcc_uah(struct smartstar_coul_device *di,
	int new_fcc_uah, int delta_fcc_uah, int design_fcc_mah, int fcc_uah)
{
	int max_delta_fcc_uah;
#ifdef CONFIG_HUAWEI_DSM
	char buff[DSM_BUFF_SIZE_MAX] = {0};
#endif

	if (delta_fcc_uah < 0) {
		delta_fcc_uah = -delta_fcc_uah;
		max_delta_fcc_uah = design_fcc_mah * DELTA_MAX_DECR_FCC_PERCENT * TENTH;
	} else {
		max_delta_fcc_uah = design_fcc_mah * DELTA_MAX_INCR_FCC_PERCENT * TENTH;
	}
	if (delta_fcc_uah > max_delta_fcc_uah) {
		/* new_fcc_uah is outside the scope limit it */
		if (new_fcc_uah > fcc_uah)
			new_fcc_uah = (fcc_uah + max_delta_fcc_uah);
		else
			new_fcc_uah = (fcc_uah - max_delta_fcc_uah);
		coul_core_info("delta_fcc=%d > %d percent of fcc=%d restring it to %d\n",
			delta_fcc_uah, max_delta_fcc_uah, fcc_uah, new_fcc_uah);
#ifdef CONFIG_HUAWEI_DSM
		/*
		 * dmd report: current information --
		 * old_fcc,new_fcc, delta_fcc, charging_begin_soc,
		 * charging_begin_cc, charing_end_cc, temp, basplevel
		 */
		snprintf(buff, (size_t)DSM_BUFF_SIZE_MAX, " [refresh fcc warning]old_fcc:%dmAh, "
			"new_fcc:%dmAh, delta_fcc:%dmAh, charging_beging_soc:%d, "
			"charging_begin_cc:%dmAh, charging_end_cc:%dmAh, temp:%d, basplevel:%u",
			di->fcc_real_mah, new_fcc_uah / UA_PER_MA,
			delta_fcc_uah / UA_PER_MA,
			di->charging_begin_soc,
			di->charging_begin_cc / UA_PER_MA,
			di->coul_dev_ops->calculate_cc_uah() / UA_PER_MA,
			di->batt_temp, di->basp_level);
		coul_dsm_report_ocv_cali_info(di,
			ERROR_REFRESH_FCC_OUTSIDE, buff);
#endif
	}
	di->fcc_real_mah = new_fcc_uah / PERMILLAGE;
}

/*
 * fcc self_study, check learning condition when charge done and
 * call readjust_fcc_table to establish a temp_fcc lookup table
 */
static void refresh_fcc(struct smartstar_coul_device *di)
{
	int charging_iavg_ma;
	int fcc_uah = 0;
	int new_fcc_uah, delta_fcc_uah, design_fcc_mah;

	charging_iavg_ma = calc_charging_iavg_ma(di);
	if (di->batt_ocv_valid_to_refresh_fcc &&
		(is_fcc_ready_to_refresh(di, charging_iavg_ma))) {
		new_fcc_uah = calculate_real_fcc_uah(di, &fcc_uah);
		design_fcc_mah = interpolate_fcc(di, di->batt_temp);
		if (di->need_restore_cycle_flag)
			check_restore_cycles(di, new_fcc_uah, fcc_uah,
				(new_fcc_uah + fcc_uah) / HALF);
		delta_fcc_uah = new_fcc_uah - fcc_uah;
		limit_delta_fcc_uah(di, new_fcc_uah, delta_fcc_uah, design_fcc_mah, fcc_uah);
		di->fcc_real_mah = get_fcc_after_cv_adjust(di, di->fcc_real_mah);
		/* limit max fcc, consider boardd 1.05 * fcc gain */
		if (di->fcc_real_mah >
			design_fcc_mah * FCC_UPPER_LIMIT_PERCENT / SOC_FULL)
			di->fcc_real_mah = design_fcc_mah *
				FCC_UPPER_LIMIT_PERCENT / SOC_FULL;
		coul_core_info("refresh_fcc, start soc=%d, new fcc=%d\n",
			di->charging_begin_soc, di->fcc_real_mah);
		/* update the temp_fcc lookup table */
		readjust_fcc_table(di);

		/* high precision qmax refresh check */
		is_high_precision_qmax_ready_to_refresh(di);
	}
}

/*
 * be called when charge finish, update charge status, chargecycles  calc soc(100%), OCV.
 * can't be called in atomic context, refresh_fcc if can be
 */
void coul_charging_done(struct smartstar_coul_device *di)
{
	int rm, ocv_update_hour;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;
	if (di->charging_state != CHARGING_STATE_CHARGE_START) {
		coul_core_info("charging_done, pre charge state is %d\n",
			di->charging_state);
		return;
	}
	/* limt fcc refresh by ocv update time */
	ocv_update_hour = di->coul_dev_ops->get_coul_time() / SEC_PER_HOUR;
	if (ocv_update_hour >= FCC_UPDATE_MAX_OCV_INTERVAL)
		di->batt_ocv_valid_to_refresh_fcc = 0;
	coul_core_info("done fcc_flag = %d,ocv_time = %d hour\n",
		di->batt_ocv_valid_to_refresh_fcc, ocv_update_hour);
	/* enable coul irq */
	di->coul_dev_ops->irq_enable();
	basp_refresh_fcc(di);
	refresh_fcc(di);
	rm = coul_get_rm(di);
	di->batt_limit_fcc = rm * SOC_FULL / LIMIT_FCC_CAL_RATIO;
	coul_core_info("coul_charging_done, adjust soc from %d to 100\n",
		di->batt_soc);

	di->batt_soc = SOC_FULL;

	if (di->batt_changed_flag) {
		/* recalculate charge cycles */
		recalc_chargecycles(di);
		di->batt_changed_flag = 0;
	}
	get_ocv_by_fcc(di);

	di->batt_fcc = calculate_fcc_uah(di);
	update_chargecycles(di);
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);

	/* NV save in shutdown charging */
	if (coul_get_pd_charge_flag() == 1) {
		if (di->is_nv_need_save) {
			if (!save_nv_info(di))
				di->is_nv_need_save = 0;
		}
	}
	di->charging_state = CHARGING_STATE_CHARGE_DONE;

	if (di->nondc_volt_dec > BASP_FATAL_VDEC_TH) {
		/* wait for ibat 1.5s */
		msleep(SLEEP_1500_MS);
		force_ocv_update();
	}

	coul_core_info("new charging cycles = %u%%\n", di->batt_chargecycles);
#ifdef CONFIG_COUL_POLAR
	update_polar_info_chgdone(1);
#endif
}

static void coul_get_last_soc(struct smartstar_coul_device *di)
{
	bool flag_soc_valid = false;
	short soc_temp = 0;

	di->coul_dev_ops->get_last_soc_flag(&flag_soc_valid);
	di->coul_dev_ops->get_last_soc(&soc_temp);
	coul_core_info("%s: flag=%d,di->batt_soc=%d,soc_temp=%d\n",
		__func__, flag_soc_valid, di->batt_soc, soc_temp);

	if (flag_soc_valid &&
		abs(di->batt_soc - soc_temp) < di->startup_delta_soc) {
		di->last_powerdown_soc = soc_temp;
		coul_core_info("last_powerdown_soc=%d,flag=%d\n",
			soc_temp, flag_soc_valid);
	} else {
		di->last_powerdown_soc = -1;
		coul_core_info("last_powerdown_soc invalid\n");
	}

	di->coul_dev_ops->clear_last_soc_flag();
}

/* smooth first soc to avoid soc jump in startup step */
static void coul_smooth_startup_soc(struct smartstar_coul_device *di)
{
	coul_get_last_soc(di);

	if ((!di->last_soc_enable) || (di->last_powerdown_soc < 0))
		return;

	di->batt_soc = di->last_powerdown_soc;
	coul_core_info("battery last soc=%d\n", di->last_powerdown_soc);
}

/*
 * respond the fault events from coul driver
 * Parameters: fault_nb:fault notifier_block, event:fault event name, data:unused
 * return value: NOTIFY_OK-success or others
 */
static int coul_fault_notifier_call(struct notifier_block *fault_nb,
	unsigned long event, void *data)
{
	struct smartstar_coul_device *di =
		container_of(fault_nb, struct smartstar_coul_device, fault_nb);

	di->coul_fault = (enum coul_fault_type)event;
	queue_work(system_power_efficient_wq, &di->fault_work);

	return NOTIFY_OK;
}

static int coul_shutdown_prepare(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct smartstar_coul_device *di =
		container_of(nb, struct smartstar_coul_device, reboot_nb);

	switch (event) {
	case SYS_DOWN:
	case SYS_HALT:
	case SYS_POWER_OFF:
		coul_core_info("coul prepare to shutdown, event = %lu\n", event);
		cancel_delayed_work_sync(&di->calculate_soc_delayed_work);
		break;
	default:
		coul_core_err("error event, coul ignore, event = %lu\n", event);
		break;
	}
	return 0;
}

static void di_initial(struct smartstar_coul_device *di)
{
	/* get dts data */
	coul_core_get_dts(di);
	contexthub_thermal_init();
	mutex_init(&di->soc_mutex);
	coul_set_low_vol_int(di, LOW_INT_STATE_RUNNING);

	/* set di element with default data */
	di->prev_pc_unusable = -EINVAL;
	di->batt_pre_delta_rc = 0;
	di->soc_work_interval = CALCULATE_SOC_MS;
	di->last_iavg_ma = IMPOSSIBLE_IAVG;
	di->batt_under_voltage_flag = 0;

	/* read nv info */
	get_initial_params(di);
	g_last_charge_cycles = di->batt_chargecycles;
	di->is_nv_read = di->coul_dev_ops->get_nv_read_flag();
	di->is_nv_need_save = 0;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_SUCCESS);
	di->sr_resume_time = di->coul_dev_ops->get_coul_time();
	di->resume_cc = di->coul_dev_ops->calculate_cc_uah();
	g_sr_cur_state = SR_DEVICE_WAKEUP;
	di->batt_temp = coul_battery_temperature_tenth_degree(USER_COUL);
	get_battery_id_voltage(di);
}

static int coul_work_register(struct smartstar_coul_device *di)
{
	int retval;
	/* init battery remove check work */
	if (di->battery_is_removable)
		INIT_DELAYED_WORK(&di->battery_check_delayed_work,
			battery_check_work);
	coul_lock_init();
	/* Init soc calc work */
	INIT_DELAYED_WORK(&di->calculate_soc_delayed_work, calculate_soc_work);
	INIT_WORK(&di->fault_work, coul_fault_work);
	INIT_DELAYED_WORK(&di->read_temperature_delayed_work,
		read_temperature_work);
	di->work_init_flag = 1;
	di->fault_nb.notifier_call = coul_fault_notifier_call;
	retval = register_coul_fault_notifier(&di->fault_nb);
	di->reboot_nb.notifier_call = coul_shutdown_prepare;
	register_reboot_notifier(&(di->reboot_nb));

	return retval;
}

static void get_initial_soc(struct smartstar_coul_device *di)
{
	/* get the first soc value */
	di_lock();
	di->soc_limit_flag = STATUS_START;
	di->soc_monitor_flag = STATUS_START;
	di->batt_soc = calculate_state_of_charge(di);
	coul_smooth_startup_soc(di);
	di->soc_limit_flag = STATUS_RUNNING;
	di->charging_stop_soc = di->batt_soc_real;
	di->charging_stop_time = di->coul_dev_ops->get_coul_time();
	di_unlock();
}

static int coul_probe(struct platform_device *pdev)
{
	struct coulometer_ops *coul_ops = NULL;
	struct smartstar_coul_device *di = NULL;
	int retval;

	di = (struct smartstar_coul_device *)devm_kzalloc(
		&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL) {
		coul_core_err("%s failed to alloc di struct\n", __func__);
		return -ENOMEM;
	}
	di->work_init_flag = 0;
	di->low_vol_filter_cnt = LOW_INT_VOL_COUNT;
	di->dev = &pdev->dev;
	retval = probe_check_coul_dev_ops(di);
	if (retval)
		goto coul_failed;

	g_smartstar_coul_dev = di;
	platform_set_drvdata(pdev, di);

	di_initial(di);

	retval = check_batt_data(di);
	if (retval == -EINVAL)
		goto coul_no_battery;
	if (retval == -1)
		goto coul_unset_pdev;

	retval = coul_work_register(di);
	if (retval < 0) {
		coul_core_err("coul_fault_register_notifier failed\n");
		goto coul_unregister_nb;
	}

#ifdef CONFIG_PM
	di->pm_notify.notifier_call = coul_pm_notify;
	register_pm_notifier(&di->pm_notify);
#endif
	coul_core_info("battery temperature is %d.%d\n",
		di->batt_temp / TENTH, di->batt_temp % TENTH);

	/* calculate init soc */
	coul_get_initial_ocv(di);

	/* battery moved, clear battery data, then update basp level */
	clear_moved_battery_data(di);
	di->qmax = coul_get_qmax(di);
	coul_core_info("%s qmax is %dmAh\n", __func__, di->qmax / UA_PER_MA);

	get_initial_soc(di);

	/* schedule calculate_soc_work */
	queue_delayed_work(system_power_efficient_wq,
		&di->calculate_soc_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(di->soc_work_interval)));
	queue_delayed_work(system_power_efficient_wq,
		&di->read_temperature_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));

coul_no_battery:
	coul_ops = get_coul_ops();
	di->ops = coul_ops;
	retval = coul_drv_coul_ops_register(coul_ops, KCOUL);
	if (retval) {
		coul_core_err("failed to register coul ops\n");
		goto coul_cancel_work;
	}
	/* create sysfs */
	retval = coul_create_sysfs();
	if (retval) {
		coul_core_err("%s failed to create sysfs\n", __func__);
		goto coul_cancel_work;
	}
	coul_cali_adc(di);
	g_pdev = pdev;
	/* on boot multi steps will update charge cycles, so notify at end of function */
	call_coul_blocking_notifiers(BATT_EEPROM_CYC,
		 &di->batt_chargecycles);
	coul_core_info("coul core probe ok\n");
	return 0;

coul_cancel_work:
	di->ops = NULL;
	if (di->work_init_flag == 1) {
		cancel_delayed_work(&di->calculate_soc_delayed_work);
		cancel_delayed_work(&di->read_temperature_delayed_work);
	}
coul_unregister_nb:
	unregister_coul_fault_notifier(&di->fault_nb);
	if (di->battery_is_removable && (di->work_init_flag == 1))
		cancel_delayed_work(&di->battery_check_delayed_work);
coul_unset_pdev:
	platform_set_drvdata(pdev, NULL);
	g_smartstar_coul_dev = NULL;
coul_failed:
	coul_core_err("coul core probe failed\n");
	return retval;
}

static int coul_remove(struct platform_device *pdev)
{
	struct smartstar_coul_device *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		coul_core_info("%s: di is null\n", __func__);
	} else {
		mutex_destroy(&di->soc_mutex);
		coul_lock_trash();
		di = NULL;
	}
	return 0;
}

#ifdef CONFIG_PM
static int coul_pm_notify(struct notifier_block *nb,
	unsigned long mode, void *priv_unused)
{
	struct smartstar_coul_device *di =
		container_of(nb, struct smartstar_coul_device, pm_notify);

	switch (mode) {
	case PM_SUSPEND_PREPARE:
		coul_core_info("%s:-n", __func__);
		g_sr_suspend_temp =
			coul_battery_temperature_tenth_degree(USER_COUL);
		cancel_delayed_work_sync(&di->calculate_soc_delayed_work);
		break;
	case PM_POST_SUSPEND:
		coul_core_info("%s:+n", __func__);
		di->batt_soc = calculate_state_of_charge(di);
		queue_delayed_work(system_power_efficient_wq,
			&di->calculate_soc_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(
				(unsigned int)di->soc_work_interval)));
		break;
	case PM_HIBERNATION_PREPARE:
	case PM_POST_HIBERNATION:
	default:
		break;
	}
	return 0;
}

/* suspend function, called when coul enter sleep, v9 no sleep */
int coul_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct smartstar_coul_device *di = platform_get_drvdata(pdev);

	int current_sec, wakeup_time, sr_delta_temp;
	int last_cali_time = 0;
	int last_cali_temp = 0;

	if (di == NULL) {
		coul_core_info("%s: di is null\n", __func__);
		return 0;
	}

	coul_core_info("%s: +\n", __func__);
	current_sec = di->coul_dev_ops->get_coul_time();
	di_lock();
	di->suspend_cc = di->coul_dev_ops->calculate_cc_uah();
	di->suspend_time = (unsigned int)current_sec;
	di->sr_suspend_time = current_sec;

	wakeup_time = current_sec - di->sr_resume_time;
	record_wakeup_info(di, wakeup_time);
	g_sr_cur_state = SR_DEVICE_SLEEP;
	di_unlock();
	if (di->work_init_flag == 1)
		cancel_delayed_work(&di->read_temperature_delayed_work);
	if (di->batt_exist && (di->work_init_flag == 1))
		cancel_delayed_work(&di->calculate_soc_delayed_work);
	get_last_cali_info(&last_cali_temp, &last_cali_time);
	sr_delta_temp = g_sr_suspend_temp - last_cali_temp;
	if (sr_delta_temp < 0)
		sr_delta_temp = -sr_delta_temp;
	if ((sr_delta_temp >= TEMP_THRESHOLD_CALI * TENTH) ||
		(current_sec - last_cali_time > CALIBRATE_INTERVAL / PERMILLAGE) ||
		(current_sec - last_cali_time < 0)) {
		/* this is to reduce calibrate times in frequently SR */
		di->coul_dev_ops->cali_adc();
		last_cali_time = current_sec;
		last_cali_temp = g_sr_suspend_temp;
	}

	if (di->battery_is_removable && (di->work_init_flag == 1))
		cancel_delayed_work(&di->battery_check_delayed_work);

#ifdef CONFIG_COUL_POLAR
	sync_sample_info();
	stop_polar_sample();
	could_sample_polar_ocv(current_sec, di->charging_stop_time,
		di->batt_temp, di->polar_ocv_enable);
#endif
	coul_set_low_vol_int(di, LOW_INT_STATE_SLEEP);
	di->coul_dev_ops->enter_eco();
	coul_core_info("%s: -\n", __func__);

	return 0;
}

#ifdef CONFIG_HUAWEI_DUBAI
static void report_battery_adjust(int delta_ocv, int delta_soc,
	int delta_uah, int sleep_cc)
{
	if (delta_ocv != 0)
		HWDUBAI_LOGE("DUBAI_TAG_BATTERY_ADJUST", "delta_soc=%d delta_uah=%d sleep_uah=%d",
			delta_soc, delta_uah, sleep_cc);
}
#else
static inline void report_battery_adjust(int delta_ocv, int delta_soc,
	int delta_uah, int sleep_cc)
{
}
#endif

static void queue_work_after_coul_resume(struct smartstar_coul_device *di)
{
	if (di->batt_exist && (di->work_init_flag == 1)) {
		queue_delayed_work(system_power_efficient_wq,
			&di->read_temperature_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));
		queue_delayed_work(system_power_efficient_wq,
			&di->calculate_soc_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(
				(unsigned int)di->soc_work_interval)));
		/* start soh check if charging done */
		coul_start_soh_check();
	}

	if (di->battery_is_removable && (di->work_init_flag == 1))
		queue_delayed_work(system_power_efficient_wq,
			&di->battery_check_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(BATTERY_CHECK_TIME_MS)));
}

#ifdef CONFIG_COUL_POLAR
static void coul_resume_polar_calc(struct smartstar_coul_device *di)
{
	int ocv_soc_mv;
	int curr_now = 0;
	int vol_now = 0;

	ocv_soc_mv = interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp / TENTH, di->batt_soc_real);
	coul_get_battery_voltage_and_current(di, &curr_now, &vol_now);
	resume_polar_calc(di->batt_temp, ocv_soc_mv, curr_now, vol_now);
}
#endif

#ifdef CONFIG_HUAWEI_DSM
static void resume_soc_jump_report(struct smartstar_coul_device *di, int old_soc)
{
	char buff[DSM_BUFF_SIZE_MAX] = {0};

	/* dmd report */
	if (abs(old_soc - di->batt_soc) >= SOC_JUMP_MAX) {
		/* current information --- fcc, temp, old_soc, new_soc */
		snprintf(buff, (size_t)DSM_BUFF_SIZE_MAX, "[resume jump]fcc:%d, temp:%d, old_soc:%d, new_soc:%d",
			 di->batt_fcc, di->batt_temp, old_soc, di->batt_soc);
		coul_dsm_report_ocv_cali_info(di, ERROR_RESUME_SOC_JUMP, buff);
	}
}
#endif

/* called when coul wakeup from deep sleep */
int coul_resume(struct platform_device *pdev)
{
	struct smartstar_coul_device *di = platform_get_drvdata(pdev);
	int sr_sleep_time, old_soc, sleep_cc;
	int pre_ocv, delta_soc;
#ifdef CONFIG_COUL_POLAR
	struct info_to_update_polar_info coul_info = {0};
#endif

	if (di == NULL) {
		coul_core_info("%s: di is null\n", __func__);
		return 0;
	}
	coul_core_info("%s: +\n", __func__);
	pre_ocv = di->batt_ocv;
	di_lock();
	di->sr_resume_time = di->coul_dev_ops->get_coul_time();
	update_battery_temperature(di, TEMPERATURE_INIT_STATUS);
	sr_sleep_time = di->sr_resume_time - di->sr_suspend_time;
	sleep_cc = di->coul_dev_ops->calculate_cc_uah() - di->suspend_cc; /* sleep uah */

	coul_set_low_vol_int(di, LOW_INT_STATE_RUNNING);

	g_sr_cur_state = SR_DEVICE_WAKEUP;
	record_sleep_info(sr_sleep_time);
	update_ocv_after_resume(di);

#ifdef CONFIG_COUL_POLAR
	set_info_to_update_polar_info(di, &coul_info, sr_sleep_time, sleep_cc);
	update_polar_ocv(&coul_info);
	coul_resume_polar_calc(di);
	start_polar_sample();
#endif
	di->coul_dev_ops->exit_eco();
	di->soc_limit_flag = STATUS_WAKEUP;
	di->soc_monitor_flag = STATUS_WAKEUP;
	old_soc = di->batt_soc;
	di->batt_soc = calculate_state_of_charge(di);

#ifdef CONFIG_HUAWEI_DSM
	resume_soc_jump_report(di, old_soc);
#endif
	di->soc_limit_flag = STATUS_RUNNING;
	di->resume_cc = di->coul_dev_ops->calculate_cc_uah();
	delta_soc = old_soc - di->batt_soc;
	report_battery_adjust(pre_ocv - di->batt_ocv, delta_soc,
		delta_soc * di->batt_fcc / PERCENT, sleep_cc);
	if ((di->low_temp_opt_flag == LOW_TEMP_OPT_OPEN) &&
		(di->batt_temp < LOW_TEMP_FIVE_TENTH))
		di->soc_work_interval = CALCULATE_SOC_MS / QUARTER;
	else
		di->soc_work_interval = CALCULATE_SOC_MS / HALF;

	di_unlock();

	queue_work_after_coul_resume(di);

	coul_core_info("%s: -\n", __func__);
	return 0;
}
#endif

static const struct of_device_id coul_core_match_table[] = {
	{
		.compatible = "hisilicon,coul_core",
	},
	{
		/* end */
	},
};

static void coul_shutdown(struct platform_device *pdev)
{
	struct smartstar_coul_device *di = platform_get_drvdata(pdev);
	int last_soc = bci_show_capacity();

	coul_core_err("coul_shutdown start\n");
	if (di == NULL) {
		coul_core_err("[coul_shutdown]:di is NULL\n");
		return;
	}
	if (last_soc >= 0)
		di->coul_dev_ops->save_last_soc(last_soc);
	if ((di->coul_dev_ops->set_bootocv_sample != NULL) &&
		di->dischg_ocv_enable) {
		if (could_cc_update_ocv() == TRUE)
			di->coul_dev_ops->set_bootocv_sample(1);
		else
			di->coul_dev_ops->set_bootocv_sample(0);
	}
	if (di->work_init_flag == 1) {
		cancel_delayed_work(&di->calculate_soc_delayed_work);
		cancel_delayed_work(&di->read_temperature_delayed_work);
		if (di->battery_is_removable)
			cancel_delayed_work(&di->battery_check_delayed_work);
	}
	coul_core_err("coul_shutdown end\n");
}

static struct platform_driver coul_core_driver = {
	.probe = coul_probe,
	.remove = coul_remove,
	.shutdown = coul_shutdown,
#ifdef CONFIG_PM
	.suspend = coul_suspend,
	.resume = coul_resume,
#endif
	.driver = {
		.name = "coul_core",
		.owner = THIS_MODULE,
		.of_match_table = coul_core_match_table,
	},
};

static int __init coul_core_init(void)
{
	return platform_driver_register(&coul_core_driver);
}

static void __exit coul_core_exit(void)
{
	platform_driver_unregister(&coul_core_driver);
}

fs_initcall(coul_core_init);
module_exit(coul_core_exit);

MODULE_DESCRIPTION("coul core driver");
MODULE_LICENSE("GPL");
