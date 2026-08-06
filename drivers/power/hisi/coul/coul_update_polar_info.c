/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: polar operation interface for coul module
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

#include <securec.h>
#ifdef CONFIG_HUAWEI_DSM
#include <chipset_common/hwpower/common_module/power_dsm.h>
#endif
#include "coul_interface.h"
#include "coul_polar.h"
#include "coul_private_interface.h"
#include "polar_table.h"
#include "coul_update_polar_info.h"

static DEFINE_SEMAPHORE(polar_sample_sem);

#define RPROC_LPM3_MBX  HISI_LPM3_ACPU_MBX_1

#define A_COE_MUL       1000
#define UOHM_PER_MOHM   1000
#define POLAR_OCV_TEMP_LIMIT    100
#define POLAR_ECO_IBAT_LIMIT    50
#define POLAR_OCV_TSAMPLE_LIMIT 5
#define POLAR_SR_VOL0_LIMIT     3500
#define POLAR_SR_VOL1_LIMIT     7000
#define SIXTY_MINUTES           60
#define MIN_UPDATE_POLAR_TEMP   30
#define MIN_SAMPLE_POLAR_DALTA_TIME 5
#define MIN_CHG_DONE_TIME       20
struct ocv_eco_data    {
	int cc;
	int vbat;
	int ibat;
	int sleep_cc;
};

static int g_always_eco_sample;
static int g_enable_ocv_calc;
static struct polar_device_ops *g_polar_dev_ops;

int polar_ops_register(struct polar_device_ops *ops)
{
	if (ops == NULL) {
		polar_err("polar ops register fail\n");
		return -EPERM;
	}

	g_polar_dev_ops = ops;
	polar_err("polar ops register success\n");

	return 0;
}

struct polar_device_ops *get_polar_dev_ops(void)
{
	return g_polar_dev_ops;
}

void update_polar_ishort_info_cc(int cc_temp)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;

	di->ishort_info.cc_all += cc_temp;
}

void update_polar_ishort_info_time(unsigned int time_now)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;

	if (di->ishort_info.time0 && di->ishort_info.last_time) {
		di->ishort_info.time0 -= time_now;
		di->ishort_info.last_time -= time_now;
	}
}

void update_polar_info_chgdone(int status)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;
	di->ishort_info.has_chg_done = status;
}

int get_coul_polar_avg(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return 0;
	return di->polar.curr_5s;
}

int get_coul_polar_peak(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return 0;
	return di->polar.curr_peak;
}

void get_polar_sysfs_info(struct polar_sysfs_info *info)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;

	info->ocv_old = di->polar.ocv_old;
	info->ori_vol = di->polar.ori_vol;
	info->ori_cur = di->polar.ori_cur;
	info->err_a = di->polar.err_a;
	info->vol = di->polar.vol;
	info->curr_5s = di->polar.curr_5s;
	info->curr_peak = di->polar.curr_peak;
	info->curr_ua = di->ishort_info.curr_ua;
}

static int coul_convert_regval2uah(u64 reg_val)
{
	struct hisi_polar_device *di = get_polar_dev();

	if ((di == NULL) || (di->polar_dev_ops == NULL) ||
		(di->polar_dev_ops->convert_regval2uah == NULL))
		return -1;
	return di->polar_dev_ops->convert_regval2uah(reg_val);
}

static bool could_update_polar_ocv(struct hisi_polar_device *di,
	 int time_now, int eco_ibat, unsigned int suspend_time)
{
	if (di == NULL)
		return FALSE;

	polar_info("[%s]ibat:%d, last_sample_time:%d,suspend_time:%u,time_now:%d\n",
		__func__, eco_ibat, di->eco_info.last_sample_time, suspend_time, time_now);
	if (eco_ibat > POLAR_ECO_IBAT_LIMIT)
		return FALSE;
	if (di->eco_info.last_sample_time - suspend_time <
		POLAR_OCV_TSAMPLE_LIMIT)
		return FALSE;
	if ((di->polar.sr_polar_vol0 > POLAR_SR_VOL0_LIMIT) ||
		(di->polar.sr_polar_vol0 < -POLAR_SR_VOL0_LIMIT))
		return FALSE;
	if ((di->polar.sr_polar_vol1 > POLAR_SR_VOL1_LIMIT) ||
		(di->polar.sr_polar_vol1 < -POLAR_SR_VOL1_LIMIT))
		return FALSE;
	return TRUE;
}

static void save_ishort_first_para(int ocv, s64 cc, int time)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;
	if ((di->ishort_info.ocv0 != 0) && (di->ishort_info.time0 != 0))
		return;
	polar_info("%s:ocv:%d,cc:%ld,time:%d\n", __func__, ocv, cc, time);
	di->ishort_info.ocv0 = ocv;
	di->ishort_info.time0 = time;
	di->ishort_info.cc0 = cc;
	di->ishort_info.last_time = time;
}

void clear_ishort_first_para(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;
	polar_info("%s\n", __func__);
	di->ishort_info.ocv0 = 0;
	di->ishort_info.time0 = 0;
	di->ishort_info.cc0 = 0;
	di->ishort_info.last_time = 0;
}

static int calc_ishort(
	struct hisi_polar_device *di, int ocv, s64 cc, int time)
{
	s64 qmax;
	int pc0, pc1, delta_pc, delta_cc, delta_time_s;
	int ishort = 0;

	qmax = update_coul_dev_qmax();
	pc0 = coul_drv_interpolate_pc(di->ishort_info.ocv0 / UVOLT_PER_MVOLT);
	pc1 = coul_drv_interpolate_pc(ocv / UVOLT_PER_MVOLT);
	delta_pc = pc0 - pc1;
	if (delta_pc <= 0) {
		polar_info("delta pc abnormal:pc0:%d,pc1:%d\n", pc0, pc1);
		return 0;
	}
	delta_cc = (int)(qmax * delta_pc / SOC_FULL);
	polar_info("qmax = %lduAh, delta_pc = %d, delta_cc = %duAh\n",
		qmax * PERMILLAGE, delta_pc, delta_cc);
	delta_time_s = time - di->ishort_info.time0;
	if (delta_time_s > 0)
		ishort = (s64)(delta_cc - (cc - di->ishort_info.cc0)) *
			 coul_minutes(SIXTY_MINUTES) / delta_time_s;
	return ishort;
}

static void update_polar_ishort_dmd(
	struct hisi_polar_device *di, char *buff, int buff_len)
{
	int ret = -1;

	if ((buff_len >= DSM_BUFF_SIZE_MAX) || (buff == NULL))
		return;

	if (di->ishort_info.reported == 0) {
		if ((di->ishort_info.curr_ua / UA_PER_MA) >=
			di->ishort_info.report_curr_ma)
			polar_info("%s\n", buff);
		ret = power_dsm_report_dmd(
			POWER_DSM_BATTERY, ERROR_BATTERY_POLAR_ISHORT, buff);
	} else if (di->ishort_info.report_urgent) {
		polar_info("%s\n", buff);
		ret = power_dsm_report_dmd(
			POWER_DSM_BATTERY, ERROR_BATTERY_POLAR_ISHORT, buff);
	}
	/* if reported, change the flag */
	if (ret == 0) {
		di->ishort_info.reported = 1;
		di->ishort_info.report_urgent = 0;
	}
}

static void update_polar_ishort(struct hisi_polar_device *di, int charging_state,
	int charging_stop_soc, int batt_soc_real, int qmax)
{
	int current_sec, buff_len, cycle_now;
	s64 current_cc;
	char buff[DSM_BUFF_SIZE_MAX] = {0};
	char *batt_brand = coul_drv_battery_brand();

	if (di == NULL)
		return;
	if (di->ishort_info.enable == 0)
		return;
	cycle_now = coul_drv_battery_cycle_count();
	current_sec = di->polar_dev_ops->get_coul_time();
	current_cc = di->polar_dev_ops->calculate_cc_uah();
	current_cc += di->ishort_info.cc_all;
	/* after charge done more than 20min */
	if (di->ishort_info.has_chg_done &&
		(current_sec > coul_minutes(MIN_CHG_DONE_TIME))) {
		save_ishort_first_para(
			di->polar.polar_ocv, current_cc, current_sec);
		di->ishort_info.has_chg_done = 0;
	} else if ((charging_state == CHARGING_STATE_CHARGE_STOP) &&
		(charging_stop_soc -batt_soc_real >
			di->ishort_info.dischg_interval * TENTH)) {
		save_ishort_first_para(
			di->polar.polar_ocv, current_cc, current_sec);
	}
	/* time limit to calc ishort */
	if (di->ishort_info.time0 &&
		(current_sec - di->ishort_info.time0 >
			di->ishort_info.time_interval)) {
		di->ishort_info.curr_ua = calc_ishort(di, di->polar.polar_ocv,
			current_cc, current_sec);
		/* judege dmd report policy */
		if ((di->ishort_info.curr_ua - di->ishort_info.last_curr_ua) >=
			di->ishort_info.report_curr_ma)
			di->ishort_info.report_urgent = 1;
		if (cycle_now > di->ishort_info.last_chg_cycle)
			di->ishort_info.reported = 0;
		buff_len = snprintf_s(buff, DSM_BUFF_SIZE_MAX,
			DSM_BUFF_SIZE_MAX - 1,
			"ishort:%d, last_ishort:%d, ocv0:%d, ocv1:%d, cc0:%ld, cc1:%ld, time0:%d, time1:%d, qmax:%d, last_cycle:%d, cycle:%d, brand:%s\n",
			di->ishort_info.curr_ua, di->ishort_info.last_curr_ua,
			di->ishort_info.ocv0, di->polar.polar_ocv,
			di->ishort_info.cc0, current_cc,
			di->ishort_info.time0, current_sec,
			qmax, di->ishort_info.last_chg_cycle,
			cycle_now, batt_brand);
		if (buff_len >= 0)
			update_polar_ishort_dmd(di, buff, buff_len);
		/* update last info */
		di->ishort_info.last_time = current_sec;
		di->ishort_info.last_curr_ua = di->ishort_info.curr_ua;
		di->ishort_info.last_chg_cycle = cycle_now;
	}
}

static bool judge_eco_info_valid(struct hisi_polar_device *di,
	int current_sec)
{
	u8 eco_sample_flag = 0;

	di->polar_dev_ops->get_eco_sample_flag(&eco_sample_flag);
	polar_info("[%s]vbat:0x%x, ibat:0x%x\n",
		__func__, di->eco_info.eco_vbat_reg, di->eco_info.eco_ibat_reg);
	if ((di->eco_info.eco_vbat_reg == 0) || (eco_sample_flag == 0) ||
		(current_sec == 0))
		return FALSE;
	return TRUE;
}

static void deal_with_invalid_eco_info(int temp, int soc,
	int sr_sleep_time, int sleep_cc)
{
	unsigned long sample_time_rtc;
	int sample_time, curr_ma;
	struct polar_com_para para = {0};

	sample_time_rtc = hisi_getcurtime();
	sample_time = (int)(sample_time_rtc / NSEC_PER_MSEC);
	if (sr_sleep_time) {
		curr_ma = -cc_uas2ma(sleep_cc, sr_sleep_time);
		para.temp = temp;
		para.soc = soc;
		para.cur = curr_ma;
		get_resume_polar_info(&para, 0,
			(sr_sleep_time * MSEC_PER_SEC), sample_time);
	}
}

static void calc_polar_ocv(struct hisi_polar_device *di,
	struct ocv_eco_data *eco_data, int r_pcb)
{
	if (di->polar.sr_polar_err_a > 0)
		di->polar.polar_ocv = eco_data->vbat -
			(eco_data->ibat / UA_PER_MA) * (r_pcb / UOHM_PER_MOHM) -
			di->polar.sr_polar_vol0 * di->polar.sr_polar_err_a / A_COE_MUL;
	else
		di->polar.polar_ocv = eco_data->vbat -
			(eco_data->ibat / UA_PER_MA) * (r_pcb / UOHM_PER_MOHM) -
			di->polar.sr_polar_vol0;
}

static int calc_current_by_cc(struct hisi_polar_device *di, struct ocv_eco_data *eco_data,
	int duration)
{
	int cc_now, cur;

	cc_now = di->polar_dev_ops->calculate_cc_uah();
	cc_now = cc_now - eco_data->cc;
	cur = -cc_uas2ma(cc_now, (duration / MSEC_PER_SEC));
	return cur;
}

void update_polar_ocv(struct info_to_update_polar_info *coul_info)
{
	int sample_time, duration, current_sec;
	unsigned long sample_time_rtc;
	struct ocv_eco_data eco_data;
	struct polar_com_para para = {0};
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;

	para.temp = coul_info->temp;
	para.soc = coul_info->soc;
	current_sec = di->polar_dev_ops->get_coul_time();
	if (judge_eco_info_valid(di, current_sec) == FALSE)
		return deal_with_invalid_eco_info(coul_info->temp, coul_info->soc,
			coul_info->sr_sleep_time, coul_info->sleep_cc);

	eco_data.vbat = coul_drv_convert_regval2uv(di->eco_info.eco_vbat_reg);
	eco_data.ibat = -coul_drv_convert_regval2ua(di->eco_info.eco_ibat_reg);
	eco_data.cc = coul_convert_regval2uah(di->eco_info.eco_cc_reg);
	eco_data.sleep_cc = eco_data.cc - coul_info->suspend_cc;
	polar_info("[%s]vbat:%d, ibat:%d\n",
		__func__, eco_data.vbat, eco_data.ibat);

	sample_time_rtc = hisi_getcurtime();
	sample_time = (int)(sample_time_rtc / NSEC_PER_MSEC);
	sample_time -= ((current_sec - di->eco_info.now_sample_time) * MSEC_PER_SEC);
	duration = di->eco_info.now_sample_time - coul_info->suspend_time;
	if (duration) {
		para.cur = -cc_uas2ma(eco_data.sleep_cc, duration);
		get_resume_polar_info(&para, eco_data.ibat,
			(duration * MSEC_PER_SEC), sample_time);
	}
	if (polar_ocv_params_calc(&di->polar, coul_info->soc, coul_info->temp,
		eco_data.ibat / UA_PER_MA) == -1)
		return;
	if ((g_enable_ocv_calc == 0) &&
		(could_update_polar_ocv(di, current_sec,
			eco_data.ibat / UA_PER_MA, coul_info->suspend_time) == FALSE))
		return;
	calc_polar_ocv(di, &eco_data, coul_info->r_pcb);
	di->polar.polar_ocv_time = current_sec;

	duration = (int)(sample_time_rtc / NSEC_PER_MSEC) - sample_time;
	sample_time = (int)(sample_time_rtc / NSEC_PER_MSEC);
	if (duration) {
		para.cur = calc_current_by_cc(di, &eco_data, duration);
		get_resume_polar_info(&para, 0, duration, sample_time);
	}
	polar_info("[%s]polar_ocv:%d, polar_ocv_time:%d\n",
		__func__, di->polar.polar_ocv, di->polar.polar_ocv_time);
	if (!is_in_capacity_dense_area(di->polar.polar_ocv))
		update_polar_ishort(di, coul_info->charging_state,
			coul_info->charging_stop_soc, coul_info->batt_soc_real, coul_info->qmax);
}

/* get bat ocv data from lpm3 by ipc, 0:suc;other fail */
static int bat_lpm3_ocv_msg_handler(struct notifier_block *nb,
	unsigned long action, void *msg)
{
	struct ipc_msg *p_ipcmsg = NULL;
	int mins;
	errno_t ret_s;
	struct hisi_polar_device *di = container_of(nb,
		struct hisi_polar_device, bat_lpm3_ipc_block);

	if (msg == NULL) {
		polar_err("%s:msg is NULL\n", __func__);
		return 0;
	}
	p_ipcmsg = (struct ipc_msg *)msg;
	if (p_ipcmsg->data[0] == IPC_BAT_OCVINFO) {
		mins = min(sizeof(di->eco_info),
			   (MAX_MAIL_SIZE - 1) * sizeof(int));
		ret_s = memcpy_s((void *)&di->eco_info, sizeof(di->eco_info),
			(void *)&p_ipcmsg->data[1], mins);
		if (ret_s) {
			polar_err("%s:memcpy fail\n", __func__);
			return 0;
		}
		polar_info("%s:vbat:0x%x,ibat:0x%x\n", __func__,
			di->eco_info.eco_vbat_reg, di->eco_info.eco_ibat_reg);
	}
	return 0;
}

void polar_ipc_init(struct hisi_polar_device *di)
{
	int ret;

	if (di == NULL) {
		polar_err("[%s]di is null\n", __func__);
		return;
	}
	/* initialization mailbox */
	di->bat_lpm3_ipc_block.next = NULL;
	di->bat_lpm3_ipc_block.notifier_call = bat_lpm3_ocv_msg_handler;
	ret = RPROC_MONITOR_REGISTER(RPROC_LPM3_MBX, &di->bat_lpm3_ipc_block);
	if (ret)
		polar_err("[%s]ipc register fail\n", __func__);
	di->polar_dev_ops->set_eco_sample_flag(0);
	di->polar_dev_ops->clr_eco_data(0);
}

void update_polar_params(int ocv_soc_mv, int curr_now, int vol_now, bool update_flag)
{
	static unsigned long last_calc_time;
	unsigned long curr_time;
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;
	if (down_interruptible(&polar_sample_sem)) {
		polar_err("%s:down failed\n", __func__);
		return;
	}
	if (last_calc_time == 0) {
		last_calc_time = hisi_getcurtime() / NSEC_PER_MSEC;
	} else {
		curr_time = hisi_getcurtime() / NSEC_PER_MSEC;
		if (time_after(last_calc_time + POLAR_CALC_INTERVAL, curr_time)) {
			polar_err("%s:update too soon\n", __func__);
			up(&polar_sample_sem);
			return;
		}
		last_calc_time = curr_time;
	}

	sync_sample_info();
	polar_params_calculate(&di->polar, ocv_soc_mv, vol_now / UVOLT_PER_MVOLT,
		-curr_now / UA_PER_MA, update_flag);
	polar_info("vol:%d,curr:%d,p_vol:%ld,curr_5s:%d,curr_peak:%d,p_ocv:%d\n",
		vol_now / UVOLT_PER_MVOLT, -curr_now / UA_PER_MA,
		di->polar.vol, di->polar.curr_5s, di->polar.curr_peak,
		di->polar.ocv);
	up(&polar_sample_sem);
}

/* only be called in coul resume */
void resume_polar_calc(int temp, int ocv_soc_mv, int curr_now, int vol_now)
{
	int delta_temp;
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;

	delta_temp = abs(temp - di->polar.last_calc_temp);
	/* if resume delta temp is over 3 deg */
	if (delta_temp >= MIN_UPDATE_POLAR_TEMP)
		update_polar_params(ocv_soc_mv, curr_now, vol_now, FALSE);
}

bool could_sample_polar_ocv(int time_now, int charging_stop_time,
	int batt_temp, unsigned int polar_ocv_enable)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return FALSE;
	if (g_always_eco_sample == 1)
		return FALSE;
	polar_info("[%s]time_now:%d, chg_stop_time:%d,batt_temp:%d\n",
		__func__, time_now, charging_stop_time, batt_temp);
	if (polar_ocv_enable == 0)
		goto no_sample;
	if (time_now - charging_stop_time <
		coul_minutes(MIN_SAMPLE_POLAR_DALTA_TIME))
		goto no_sample;
	if (batt_temp < POLAR_OCV_TEMP_LIMIT)
		goto no_sample;
	if (is_polar_list_ready() == FALSE)
		goto no_sample;
	di->polar_dev_ops->set_eco_sample_flag(1);
	di->polar_dev_ops->clr_eco_data(1);
	return TRUE;
no_sample:
	di->polar_dev_ops->set_eco_sample_flag(0);
	polar_info("%s:not update polar ocv\n", __func__);
	return FALSE;
}

#ifdef CONFIG_HISI_DEBUG_FS
void test_enable_sample(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;
	g_always_eco_sample = 1;
	di->polar_dev_ops->set_eco_sample_flag(1);
	di->polar_dev_ops->clr_eco_data(1);
}

void test_disable_sample(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL)
		return;
	g_always_eco_sample = 0;
	di->polar_dev_ops->set_eco_sample_flag(0);
}

void test_enable_ocv_calc(void)
{
	g_enable_ocv_calc = 1;
}

int test_polar_ocv_tbl_lookup(int soc, int batt_temp_degc)
{
	int ocv;
	struct polar_ocv_tbl *test_polar_ocv_lut = get_polar_ocv_lut();

	ocv = interpolate_polar_ocv(test_polar_ocv_lut, batt_temp_degc, soc);
	return ocv;
}

int test_res_tbl_lookup(int soc, int batt_temp_degc, int curr)
{
	int res;
	struct polar_res_tbl *test_polar_res_lut = get_polar_res_lut();

	res = interpolate_two_dimension(test_polar_res_lut,
		batt_temp_degc, soc, curr);
	return res;
}

int test_vector_res_tbl_lookup(int soc, int batt_temp_degc, int curr)
{
	int res_vector;
	struct polar_res_tbl *test_polar_res_lut = get_polar_res_lut();

	res_vector = get_polar_vector_res(test_polar_res_lut,
		batt_temp_degc, soc, curr);
	return res_vector;
}

int test_vector_value_tbl_lookup(int soc, int batt_temp_degc, int curr)
{
	int polar_vector;
	struct polar_x_y_z_tbl *test_polar_vector_lut = get_polar_vector_lut();

	polar_vector = get_polar_vector_value(test_polar_vector_lut,
		batt_temp_degc, soc, curr, 0);
	return polar_vector;
}

int test_get_trained_a(int temp, int soc)
{
	struct polar_learn_tbl *test_polar_learn_lut = get_polar_learn_lut();
	return get_trained_a(test_polar_learn_lut, temp, soc);
}

short test_get_trained_a_vol(int temp, int soc)
{
	struct polar_learn_tbl *test_polar_learn_lut = get_polar_learn_lut();
	return get_trained_polar_vol(test_polar_learn_lut, temp, soc);
}

int test_store_trained_a(int temp, int soc, long a, long polar_vol_uv)
{
	struct polar_learn_tbl *test_polar_learn_lut = get_polar_learn_lut();
	store_trained_a(test_polar_learn_lut, temp, soc, a, polar_vol_uv);
	return 0;
}

int test_store_neg_trained_a(int temp, int soc, long a, long polar_vol_uv)
{
	struct polar_learn_tbl *test_polar_learn_lut = get_polar_learn_lut();
	store_trained_a(test_polar_learn_lut, temp, soc, a, -polar_vol_uv);
	return 0;
}

int get_last_5s_curr(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (!di)
		return -1;
	return di->last_avgcurr_5s;
}

int get_last_25s_curr(void)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (!di)
		return -1;
	return di->last_avgcurr_25s;
}

int test_could_vbat_learn_a(int ocv_soc_mv, int vol_now_mv, int cur,
	long polar_vol_uv, int temp, int soc)

{
	struct hisi_polar_device *di = get_polar_dev();
	struct polar_com_para para = {0};

	if (!di)
		return -1;
	polar_info("ocv:%d,vbat:%d,curr:%d,polar_vol:%ld,temp:%d,soc:%d",
		   ocv_soc_mv, vol_now_mv, cur, polar_vol_uv, temp, soc);
	para.ocv_soc_mv = ocv_soc_mv;
	para.cur = cur;
	para.temp = temp;
	para.soc = soc;
	return could_vbat_learn_a(di, &para, vol_now_mv, polar_vol_uv);
}

int test_set_ishort_dischg(int num)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL) {
		polar_err("NULL point in %s\n", __func__);
		return -1;
	}

	di->ishort_info.dischg_interval = num;

	return di->ishort_info.dischg_interval;
}

int test_set_ishort_gap(int time)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL) {
		polar_err("NULL point in %s\n", __func__);
		return -1;
	}

	di->ishort_info.time_interval = time;

	return di->ishort_info.time_interval;
}

int test_set_ishort_limit(int curr)
{
	struct hisi_polar_device *di = get_polar_dev();

	if (di == NULL) {
		polar_err("NULL point in %s\n", __func__);
		return -1;
	}

	di->ishort_info.report_curr_ma = curr;

	return di->ishort_info.report_curr_ma;
}

#endif
#endif

