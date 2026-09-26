/*
 * direct_charge_work.c
 *
 * direct charge work module
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
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_work
HWLOG_REGIST();

void dc_control_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, control_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error || di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		hwlog_info("direct charge stop\n");
		direct_charge_stop_charging();
		return;
	}

	/* check if sc switch to lvc or lvc switch to sc */
	if (direct_charge_check_priority_inversion()) {
		direct_charge_stop_charging();
		return;
	}

	if (mulit_ic_check(di->working_mode, di->cur_mode,
		&di->multi_ic_check_info, di->dc_volt_ratio) < 0) {
		di->multi_ic_error_cnt++;
		direct_charge_set_stop_charging_flag(1);
		direct_charge_stop_charging();
		return;
	}

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("cur_stage=%d, vbat=%d, ibat=%d\n",
			di->cur_stage, di->vbat, di->ibat);
		hwlog_info("direct charge done\n");
		direct_charge_set_stage_status(DC_STAGE_CHARGE_DONE);
		direct_charge_stop_charging();
		return;
	}

	direct_charge_get_ffc_para();
	dc_select_charge_path();
	direct_charge_regulation();
	direct_charge_update_charge_info();

	t = di->charge_control_interval;
	hrtimer_start(&di->control_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void dc_calc_thld_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, calc_thld_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error || di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		hwlog_info("direct charge stop, stop calc threshold\n");
		return;
	}

	if (di->pri_inversion)
		return;

	direct_charge_soh_policy();
	direct_charge_select_charging_stage();
	direct_charge_select_charging_param();

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("direct charge done, stop calc threshold\n");
		return;
	}

	t = di->threshold_caculation_interval;
	hrtimer_start(&di->calc_thld_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void dc_kick_wtd_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;
	int ibat = 0;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, kick_wtd_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->can_stop_kick_wdt) {
		hwlog_info("stop kick watchdog\n");
		return;
	}

	if (dcm_kick_ic_watchdog(di->working_mode, di->cur_mode))
		direct_charge_get_bat_current(&ibat);

	t = KICK_WATCHDOG_TIME;
	hrtimer_start(&di->kick_wtd_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

enum hrtimer_restart dc_calc_thld_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, calc_thld_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->calc_thld_work);

	return HRTIMER_NORESTART;
}

enum hrtimer_restart dc_control_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, control_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->control_work);

	return HRTIMER_NORESTART;
}

enum hrtimer_restart dc_kick_wtd_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, kick_wtd_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->kick_wtd_wq, &di->kick_wtd_work);

	return HRTIMER_NORESTART;
}
