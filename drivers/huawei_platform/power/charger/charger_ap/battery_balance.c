/*
 * battery_balance.c
 *
 * battery balance driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/raid/pq.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/wakelock.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/direct_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/series_batt_charger.h>
#include <huawei_platform/power/battery_balance.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif

#define HWLOG_TAG battery_balance
HWLOG_REGIST();

#define BAL_CHANNEL_OPEN            1
#define BAL_CHANNEL_CLOSE           0
#define BAL_DELAY_TIME              10000
#define ICHG_30MA                   30
#define WORK_DELAY_30S              30000
#define VOL_BUFF_SIZE               5
#define READ_TIMES                  5
#define INIT_VAL_VOL                4200
#define CHG_CURR_MIN                (-100)
#define CHG_CURR_MAX                200
#define DEFAULT_BAL_TIME            5
#define DEFAULT_RALAX_TIME          1
#define DEFAULT_VOL_DIFF_BAL_START  10
#define DEFAULT_VOL_DIFF_BAL_STOP   5
#define DEFAULT_TERM_CURR_PULSE     50
#define DEFAULT_VOL_THRESHOLD       4100
#define SEC_TO_HOUR                 3600
#define R_MOS_MOHM                  66
#define BAL_CHANNEL_OPEN_TIME       (10 * 60 * 1000) /* 10min */

enum bal_sysfs_type {
	BATT_BAL_SYSFS_BAL_CHANNEL_0,
	BATT_BAL_SYSFS_BAL_CHANNEL_1,
	BATT_BAL_SYSFS_BAL_MODE,
	BATT_BAL_SYSFS_GPIO_VAL_BAL,
	BATT_BAL_SYSFS_FACTORY_BALANCE_EN,
};

struct batt_vol_info {
	int vol_buff[VOL_BUFF_SIZE];
	int vol_avg;
	int vol_pointer;
};

struct sysfs_data {
	int bat0_bal_en;
	int bat1_bal_en;
	int fac_bal_en;
};

struct batt_bal_info {
	struct device *dev;
	struct workqueue_struct *batt_bal_wq;
	struct delayed_work factory_bal_work;
	struct delayed_work charge_bal_work;
	struct delayed_work bal_ch0_work;
	struct delayed_work bal_ch1_work;
	struct work_struct bal_fsm_work;
	struct notifier_block usb_nb;
	struct notifier_block chg_state_nb;
	struct blocking_notifier_head bal_state_nh;
	struct hrtimer timer;
	struct wake_lock wakelock;
	struct batt_vol_info batt0_vol_info;
	struct batt_vol_info batt1_vol_info;
	struct sysfs_data sysfs_data;
	enum bal_state bal_state;
	enum bal_mode bal_mode;
	enum bal_event_type event;
	int bal_request_bat;
	int vol_diff_bal_start;
	int vol_diff_bal_stop;
	int gpio_bat0_bal;
	int gpio_bat1_bal;
	int bal_time;
	int relax_time;
	int term_curr_pulse;
	int chgen_in_pulse;
	int vol_threshold;
	int vol_max;
	int r_mos_mohm;
	bool in_hiz_mode;
	bool chg_en;
	bool chg_done;
	s64 signal_gauge;
	s64 sum_gauge;
	int signal_time;
	int sum_time;
};

static struct batt_bal_info *g_balance_di;
static int batt_bal_state_machine(enum bal_state bal_state);
static void bal_state_handle(struct batt_bal_info *di);
static void relax_state_handle(struct batt_bal_info *di);
static void idle_state_handle(struct batt_bal_info *di);

static void batt_bal_wake_lock(void)
{
	if (!g_balance_di)
		return;

	if (!wake_lock_active(&g_balance_di->wakelock)) {
		wake_lock(&g_balance_di->wakelock);
		hwlog_info("batt_bal wake lock\n");
	}
}

static void batt_bal_wake_unlock(void)
{
	if (!g_balance_di)
		return;

	if (wake_lock_active(&g_balance_di->wakelock)) {
		wake_unlock(&g_balance_di->wakelock);
		hwlog_info("batt_bal wake unlock\n");
	}
}

int batt_bal_state_notifier_register(struct notifier_block *nb)
{
	if (!g_balance_di || !nb)
		return -EINVAL;

	return blocking_notifier_chain_register(
		&g_balance_di->bal_state_nh, nb);
}

int batt_bal_state_notifier_unregister(struct notifier_block *nb)
{
	if (!g_balance_di || !nb)
		return -EINVAL;

	return blocking_notifier_chain_unregister(
		&g_balance_di->bal_state_nh, nb);
}

int batt_bal_get_info(struct bal_info *info)
{
	if (!g_balance_di || !info) {
		hwlog_err("g_balance_di or info is null\n");
		return -EINVAL;
	}

	info->bal_mode = g_balance_di->bal_mode;
	info->bal_state = g_balance_di->bal_state;
	info->signal_time = g_balance_di->signal_time;
	info->sum_time = g_balance_di->sum_time;
	info->signal_gauge = g_balance_di->signal_gauge / SEC_TO_HOUR;
	info->sum_gauge = g_balance_di->sum_gauge / SEC_TO_HOUR;
	info->bat_id = g_balance_di->bal_request_bat;
	hwlog_info("time:%ds,%ds,gauge:%lldmah,%lldmah\n", info->signal_time,
		info->sum_time, info->signal_gauge, info->sum_gauge);
	return 0;
}

void batt_bal_reset_info(void)
{
	if (!g_balance_di) {
		hwlog_err("g_balance_di is null\n");
		return;
	}

	g_balance_di->sum_gauge = 0;
	g_balance_di->sum_time = 0;
	g_balance_di->signal_gauge = 0;
	g_balance_di->signal_time = 0;
	hwlog_info("batt_bal_reset_info\n");
}

static void batt_bal_reset_signal_gauge(void)
{
	if (!g_balance_di) {
		hwlog_err("g_balance_di is null\n");
		return;
	}

	g_balance_di->signal_gauge = 0;
	g_balance_di->signal_time = 0;
}

static int batt_bal_channel_ctrl(enum bal_batt_id bal_batt_id, int value)
{
	int gpio_num;
	int val;

	if (!g_balance_di) {
		hwlog_err("g_balance_di is null\n");
		return -1;
	}

	hwlog_info("batt_bal_channel_ctrl: bal_batt_id=[%d], value=[%d]\n",
		bal_batt_id, value);

	if (bal_batt_id == BATT_ID_0) {
		gpio_num = g_balance_di->gpio_bat0_bal;
	} else if (bal_batt_id == BATT_ID_1) {
		gpio_num = g_balance_di->gpio_bat1_bal;
	} else {
		hwlog_err("bal_batt_id is error, = [%d]\n", bal_batt_id);
		return -1;
	}

	val = value ? BAL_CHANNEL_OPEN : BAL_CHANNEL_CLOSE;
	gpio_set_value(gpio_num, val);
	return 0;
}

static void batt_bal_channel_open(struct batt_bal_info *di)
{
	int batt_vol_diff;

	batt_vol_diff = di->batt0_vol_info.vol_avg -
		di->batt1_vol_info.vol_avg;
	di->bal_request_bat = batt_vol_diff > 0 ? BATT_ID_0 : BATT_ID_1;
	batt_bal_channel_ctrl(di->bal_request_bat, BAL_CHANNEL_OPEN);
}

static void init_vol_buff(struct batt_bal_info *di)
{
	int i;

	for (i = 0; i < VOL_BUFF_SIZE; i++) {
		di->batt0_vol_info.vol_buff[i] = INIT_VAL_VOL;
		di->batt1_vol_info.vol_buff[i] = INIT_VAL_VOL;
	}

	di->batt0_vol_info.vol_pointer = 0;
	di->batt1_vol_info.vol_pointer = 0;
	di->batt0_vol_info.vol_avg = INIT_VAL_VOL;
	di->batt1_vol_info.vol_avg = INIT_VAL_VOL;
}

static void update_vol_buff(struct batt_bal_info *di)
{
	int num = READ_TIMES;
	int sum_vbat0 = 0;
	int sum_vbat1 = 0;
	int sum_vbat_max = 0;
	int vbat0;
	int vbat1;
	int i;

	for (i = 0; i < num; i++) {
		vbat0 = hw_battery_voltage(BAT_ID_0);
		sum_vbat0 += vbat0;
		vbat1 = hw_battery_voltage(BAT_ID_1);
		sum_vbat1 += vbat1;
		sum_vbat_max += hw_battery_voltage(BAT_ID_MAX);
	}

	vbat0 = sum_vbat0 / num;
	vbat1 = sum_vbat1 / num;
	di->vol_max = sum_vbat_max / num;

	di->batt0_vol_info.vol_pointer =
		di->batt0_vol_info.vol_pointer % VOL_BUFF_SIZE;
	di->batt1_vol_info.vol_pointer =
		di->batt1_vol_info.vol_pointer % VOL_BUFF_SIZE;
	hwlog_info("batt0_cnt:%d,batt1_cnt:%d\n",
		di->batt0_vol_info.vol_pointer, di->batt1_vol_info.vol_pointer);

	di->batt0_vol_info.vol_buff[di->batt0_vol_info.vol_pointer++] = vbat0;
	di->batt1_vol_info.vol_buff[di->batt1_vol_info.vol_pointer++] = vbat1;
	hwlog_info("vbat0:%d,vbat1:%d\n", vbat0, vbat1);

	sum_vbat0 = 0;
	sum_vbat1 = 0;

	for (i = 0; i < VOL_BUFF_SIZE; i++) {
		sum_vbat0 += di->batt0_vol_info.vol_buff[i];
		sum_vbat1 += di->batt1_vol_info.vol_buff[i];
	}

	di->batt0_vol_info.vol_avg = sum_vbat0 / VOL_BUFF_SIZE;
	di->batt1_vol_info.vol_avg = sum_vbat1 / VOL_BUFF_SIZE;

	hwlog_info("batt0_vol_avg:%d,batt1_vol_avg:%d\n",
		di->batt0_vol_info.vol_avg,
		di->batt1_vol_info.vol_avg);
}

static void factory_bal_work(struct work_struct *work)
{
	int batt_vol_diff;
	struct batt_bal_info *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct batt_bal_info, factory_bal_work.work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info("factory_bal_work++\n");

	if (!is_hisi_battery_exist() ||
		!di->sysfs_data.fac_bal_en) {
		hwlog_info("fac_bal_en is 0 or battery not exit\n");
		batt_bal_state_machine(BAL_STATE_IDLE);
		return;
	}

	if (di->in_hiz_mode ||
		di->sysfs_data.bat0_bal_en ||
		di->sysfs_data.bat1_bal_en) {
		hwlog_info("in hiz mode or in or vol cali\n");
		goto fac_bal_work;
	}

	if (di->bal_state == BAL_STATE_IDLE) {
		update_vol_buff(di);
		batt_vol_diff = di->batt0_vol_info.vol_avg -
			di->batt1_vol_info.vol_avg;

		if (abs(batt_vol_diff) < di->vol_diff_bal_start) {
			hwlog_info("batt_vol_diff < %d, no balance\n",
				di->vol_diff_bal_start);
			goto fac_bal_work;
		}

		if (!di->chg_en) {
			hwlog_info("factory_bal_work: entry batt_bal_state_machine\n");
			batt_bal_wake_lock();
			di->bal_mode = BAL_MODE_DISCHG;
			batt_bal_reset_signal_gauge();
			di->event = BAL_START_FAC;
			blocking_notifier_call_chain(&di->bal_state_nh,
				(unsigned long)di->event, NULL);
			batt_bal_state_machine(BAL_STATE_BALANCE);
		}
	}

fac_bal_work:
	schedule_delayed_work(&di->factory_bal_work,
		msecs_to_jiffies(BAL_DELAY_TIME));
	hwlog_info("factory_bal_work--\n");
}

static bool is_charge_bal_en(void)
{
	struct batt_bal_info *di = g_balance_di;
	int ichg = -hisi_battery_current();
	int ichg_avg = hisi_battery_current_avg();
	int chg_state = NOT_IN_SCP_CHARGING_STAGE;

	if (!di) {
		hwlog_err("di is null\n");
		return FALSE;
	}

#ifdef CONFIG_DIRECT_CHARGER
	chg_state = is_in_scp_charging_stage();
#endif /* CONFIG_DIRECT_CHARGER */
	if (chg_state == NOT_IN_SCP_CHARGING_STAGE &&
		di->vol_max > di->vol_threshold &&
		ichg_avg > ICHG_30MA &&
		ichg > ICHG_30MA)
		return TRUE;

	hwlog_info("ichg:%d,ichg_avg:%d,is_in_scp_chg:%d\n",
		ichg, ichg_avg, chg_state);
	return FALSE;
}

static void charge_bal_work(struct work_struct *work)
{
	int batt_vol_diff;
	struct batt_bal_info *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct batt_bal_info, charge_bal_work.work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info("charge_bal_work++\n");

	if (!is_hisi_battery_exist() ||
		di->bal_mode == BAL_MODE_DISCHG) {
		hwlog_err("factory bal or battery not exist\n");
		return;
	}

	if (is_in_scp_charging_stage() == IN_SCP_CHARGING_STAGE ||
		di->in_hiz_mode ||
		di->sysfs_data.bat0_bal_en ||
		di->sysfs_data.bat1_bal_en) {
		hwlog_info("scp chg or hiz mode or vol cali\n");
		goto chg_bal_work;
	}

	if (di->bal_state == BAL_STATE_IDLE) {
		update_vol_buff(di);
		batt_vol_diff = di->batt0_vol_info.vol_avg -
			di->batt1_vol_info.vol_avg;

		if (abs(batt_vol_diff) < di->vol_diff_bal_start) {
			hwlog_info("batt_vol_diff < %d\n",
				di->vol_diff_bal_start);
			goto chg_bal_work;
		}

		if (!di->chg_done) {
			if (is_charge_bal_en()) {
				batt_bal_wake_lock();
				di->bal_mode = BAL_MODE_CHG;
				hwlog_info("charge_bal_work, entry BAL_MODE_CHG\n");
				batt_bal_reset_signal_gauge();
				di->event = BAL_START_CHG;
				blocking_notifier_call_chain(&di->bal_state_nh,
					(unsigned long)di->event, NULL);
				batt_bal_state_machine(BAL_STATE_BALANCE);
			}
		} else {
			di->bal_mode = BAL_MODE_PULSE;
			hwlog_info("entry pulse bal mode\n");
			batt_bal_wake_lock();
			batt_bal_reset_signal_gauge();
			di->event = BAL_START_PULSE;
			blocking_notifier_call_chain(&di->bal_state_nh,
				(unsigned long)di->event, NULL);
			batt_bal_state_machine(BAL_STATE_BALANCE);
			di->chg_done = FALSE;
		}
	}

	hwlog_info("bal_mode:%d,bal_state:%d\n", di->bal_mode, di->bal_state);
chg_bal_work:
	schedule_delayed_work(&di->charge_bal_work,
		msecs_to_jiffies(BAL_DELAY_TIME));

	hwlog_info("charge_bal_work--\n");
}

static void bal_fsm_work(struct work_struct *work)
{
	struct batt_bal_info *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct batt_bal_info, bal_fsm_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->bal_state == BAL_STATE_BALANCE)
		batt_bal_state_machine(BAL_STATE_RELAX);
	else if (di->bal_state == BAL_STATE_RELAX)
		batt_bal_state_machine(BAL_STATE_BALANCE);
}

static void batt_bal_update_gauge(struct batt_bal_info *di)
{
	int batt_vol;

	if (di->bal_state == BAL_STATE_BALANCE) {
		if (di->bal_request_bat == BAT_ID_0) {
			batt_vol = di->batt0_vol_info.vol_avg;
			di->signal_gauge += batt_vol / di->r_mos_mohm *
				di->bal_time;
			di->sum_gauge += batt_vol / di->r_mos_mohm *
				di->bal_time;
		} else {
			batt_vol = di->batt1_vol_info.vol_avg;
			di->signal_gauge -= batt_vol / di->r_mos_mohm *
				di->bal_time;
			di->sum_gauge -= batt_vol / di->r_mos_mohm *
				di->bal_time;
		}

		di->signal_time += di->bal_time;
		di->sum_time += di->bal_time;
	}
}

static enum hrtimer_restart batt_bal_timer_func(struct hrtimer *timer)
{
	struct batt_bal_info *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct batt_bal_info, timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	hwlog_info("batt_bal_timer_func,di->bal_state=%d\n",
		di->bal_state);

	batt_bal_update_gauge(di);
	queue_work(di->batt_bal_wq, &di->bal_fsm_work);

	return HRTIMER_NORESTART;
}

static void charge_type_handler(struct batt_bal_info *di,
	enum hisi_charger_type type)
{
	hwlog_info("charge_type_handler,type = %d\n", type);

	if (type == CHARGER_TYPE_NONE) {
		hwlog_info("charge type is none, cancle time,work\n");
		hrtimer_cancel(&di->timer);
		cancel_work_sync(&di->bal_fsm_work);
		cancel_delayed_work_sync(&di->charge_bal_work);
		cancel_delayed_work_sync(&di->factory_bal_work);
		init_vol_buff(di);
		batt_bal_state_machine(BAL_STATE_IDLE);
		series_batt_set_term_curr(DEFAULT_VALUE);
		di->event = BAL_STOP;
		blocking_notifier_call_chain(&di->bal_state_nh,
			(unsigned long)di->event, NULL);
		di->chg_done = FALSE;
	} else {
		if (di->sysfs_data.fac_bal_en)
			schedule_delayed_work(&di->factory_bal_work,
				msecs_to_jiffies(WORK_DELAY_30S));
		else
			schedule_delayed_work(&di->charge_bal_work,
				msecs_to_jiffies(WORK_DELAY_30S));
	}
}

static int charge_usb_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct batt_bal_info *di = NULL;
	enum hisi_charger_type type = (enum hisi_charger_type)event;

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct batt_bal_info, usb_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	charge_type_handler(di, type);

	return NOTIFY_OK;
}

static int charge_state_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct batt_bal_info *di = NULL;

	if (!nb) {
		hwlog_err("nb is null\n");
		return 0;
	}

	di = container_of(nb, struct batt_bal_info, chg_state_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	hwlog_info("charge_state_notifier_call: event=%ld\n", event);

	switch (event) {
	case SERIES_BATT_CHG_EN:
		series_batt_set_term_curr(DEFAULT_VALUE);
		di->chg_en = TRUE;
		break;

	case SERIES_BATT_CHG_DISABLE:
		di->chg_en = FALSE;
		break;

	case SERIES_BATT_HIZ_EN:
		di->in_hiz_mode = TRUE;
		break;

	case SERIES_BATT_HIZ_DISABLE:
		di->in_hiz_mode = FALSE;
		break;

	case SERIES_BATT_CHG_DONE:
		if (series_batt_get_term_curr() != di->term_curr_pulse) {
			hwlog_info("reset term curr\n");
			series_batt_set_term_curr(di->term_curr_pulse);
			series_batt_set_charge_en(CHARGE_DISABLE);
			series_batt_set_charge_en(CHARGE_ENABLE);
		}

		di->chg_done = TRUE;
		break;

	default:
		break;
	}

	return 0;
}

static bool is_batt_bal_done(struct batt_bal_info *di)
{
	int batt_vol_diff;
	int ichg_avg;
	int ichg;

	ichg = -hisi_battery_current();
	ichg_avg = hisi_battery_current_avg();
	hwlog_info("ichg=%d, ichg_avg=%d\n", ichg, ichg_avg);

	batt_vol_diff = di->batt0_vol_info.vol_avg -
		di->batt1_vol_info.vol_avg;

	if (abs(batt_vol_diff) < di->vol_diff_bal_stop)
		return TRUE;

	return FALSE;
}

static void batt_bal_done_handle(struct batt_bal_info *di)
{
	if (di->bal_mode == BAL_MODE_DISCHG) {
		di->event = BAL_DONE_FAC;
	} else if (di->bal_mode == BAL_MODE_CHG) {
		di->event = BAL_DONE_CHG;
	} else if (di->bal_mode == BAL_MODE_PULSE) {
		di->event = BAL_DONE_PULSE;
	} else {
		hwlog_err("bal_mode error\n");
		return;
	}

	batt_bal_state_machine(BAL_STATE_IDLE);
	blocking_notifier_call_chain(&di->bal_state_nh,
		(unsigned long)di->event, NULL);
	hwlog_info("sum_time:%ds,sum_cap:%lldmah\n", di->sum_time,
		di->sum_gauge / SEC_TO_HOUR);
	init_vol_buff(di);
}

static void bal_state_handle(struct batt_bal_info *di)
{
	int ichg = -hisi_battery_current();

	hwlog_info("bal_state_handle\n");

	if (di->in_hiz_mode ||
		!is_hisi_battery_exist() ||
		di->sysfs_data.bat0_bal_en ||
		di->sysfs_data.bat1_bal_en) {
		hwlog_err("battery not exist or in hiz mode or vol cali\n");
		goto BAL_STATE_IDLE_HANDLER;
	}

	update_vol_buff(di);
	if (is_batt_bal_done(di)) {
		hwlog_info("batt bal done\n");
		batt_bal_done_handle(di);
		return;
	}

	di->bal_state = BAL_STATE_BALANCE;

	if (di->bal_mode == BAL_MODE_DISCHG) {
		if (di->chg_en || abs(ichg) > ICHG_30MA ||
			!di->sysfs_data.fac_bal_en) {
			hwlog_err("dischg or ichg >30ma or fac_bal_en is 0\n");
			goto BAL_STATE_IDLE_HANDLER;
		}
	}

	if (di->bal_mode == BAL_MODE_CHG) {
		if (!is_charge_bal_en())
			goto BAL_STATE_IDLE_HANDLER;
	}

	if (di->bal_mode == BAL_MODE_PULSE) {
		if (!di->chg_en || ichg < CHG_CURR_MIN || ichg > CHG_CURR_MAX) {
			hwlog_err("not chg done or chg disable:%dma\n", ichg);
			goto BAL_STATE_IDLE_HANDLER;
		}

		di->chgen_in_pulse = CHARGE_ENABLE;
		series_batt_set_charge_en(di->chgen_in_pulse);
	}

	batt_bal_channel_open(di);
	hrtimer_start(&di->timer, ktime_set(di->bal_time, 0), HRTIMER_MODE_REL);
	return;

BAL_STATE_IDLE_HANDLER:
	batt_bal_state_machine(BAL_STATE_IDLE);
}

static void relax_state_handle(struct batt_bal_info *di)
{
	hwlog_info("relax_state_handle\n");
	di->bal_state = BAL_STATE_RELAX;

	if (di->bal_mode == BAL_MODE_PULSE)
		series_batt_set_charge_en(CHARGE_DISABLE);

	batt_bal_channel_ctrl(di->bal_request_bat, BAL_CHANNEL_CLOSE);
	hwlog_info("di->relax_time = %d\n", di->relax_time);

	hrtimer_start(&di->timer,
		ktime_set(di->relax_time, 0), HRTIMER_MODE_REL);
}

static void idle_state_handle(struct batt_bal_info *di)
{
	hwlog_info("idle_state_handle\n");
	di->bal_state = BAL_STATE_IDLE;

	if (gpio_get_value(di->gpio_bat0_bal))
		batt_bal_channel_ctrl(BATT_ID_0, BAL_CHANNEL_CLOSE);

	if (gpio_get_value(di->gpio_bat1_bal))
		batt_bal_channel_ctrl(BATT_ID_1, BAL_CHANNEL_CLOSE);

	if (hrtimer_active(&di->timer))
		hrtimer_cancel(&di->timer);

	if (di->bal_mode == BAL_MODE_PULSE)
		series_batt_set_charge_en(CHARGE_ENABLE);

	di->bal_mode = BAL_MODE_MAX;
	batt_bal_wake_unlock();
}

static int batt_bal_state_machine(enum bal_state bal_state)
{
	struct batt_bal_info *di = g_balance_di;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	hwlog_info("batt_bal_state_machine:bal_mod=%d\n", di->bal_mode);

	switch (bal_state) {
	case BAL_STATE_BALANCE:
		bal_state_handle(di);
		break;

	case BAL_STATE_RELAX:
		relax_state_handle(di);
		break;

	case BAL_STATE_IDLE:
		idle_state_handle(di);
		break;

	default:
		hwlog_err("bal_state error:bal_state=%d\n", bal_state);
		break;
	}

	return 0;
}

static void bal_channel0_work(struct work_struct *work)
{
	struct batt_bal_info *di = NULL;

	di = container_of(work, struct batt_bal_info, bal_ch0_work.work);
	if (di->sysfs_data.bat0_bal_en) {
		hwlog_info("bal channel 0 is open for 10 minutes");
		di->sysfs_data.bat0_bal_en = 0;
		batt_bal_channel_ctrl(BATT_ID_0, BAL_CHANNEL_CLOSE);
	}
}

static void bal_channel1_work(struct work_struct *work)
{
	struct batt_bal_info *di = NULL;

	di = container_of(work, struct batt_bal_info, bal_ch1_work.work);
	if (di->sysfs_data.bat1_bal_en) {
		hwlog_info("bal channel 1 is open for 10 minutes");
		di->sysfs_data.bat1_bal_en = 0;
		batt_bal_channel_ctrl(BATT_ID_1, BAL_CHANNEL_CLOSE);
	}
}

#ifdef CONFIG_SYSFS
#define BATTERY_BALANCE_SYSFS_FIELD(_name, n, m, store)            \
{                                                                  \
	.attr = __ATTR(_name, m, balance_sysfs_show, store),           \
	.name = BATT_BAL_SYSFS_##n,                                    \
}

#define BATTERY_BALANCE_SYSFS_FIELD_RW(_name, n) \
	BATTERY_BALANCE_SYSFS_FIELD(_name, n, 0644, balance_sysfs_store)

#define BATTERY_BALANCE_SYSFS_FIELD_RO(_name, n)                   \
	BATTERY_BALANCE_SYSFS_FIELD(_name, n, 0444, NULL)

struct balance_sysfs_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t balance_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static ssize_t balance_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct balance_sysfs_info balance_sysfs_tbl[] = {
	BATTERY_BALANCE_SYSFS_FIELD_RW(balance_channel_0,  BAL_CHANNEL_0),
	BATTERY_BALANCE_SYSFS_FIELD_RW(balance_channel_1,  BAL_CHANNEL_1),
	BATTERY_BALANCE_SYSFS_FIELD_RW(factory_balance_en, FACTORY_BALANCE_EN),
	BATTERY_BALANCE_SYSFS_FIELD_RO(bal_mode,           BAL_MODE),
	BATTERY_BALANCE_SYSFS_FIELD_RO(gpio_val_bal,       GPIO_VAL_BAL),
};

static struct attribute
	*balance_sysfs_attrs[ARRAY_SIZE(balance_sysfs_tbl) + 1];

static const struct attribute_group balance_sysfs_attr_group = {
	.attrs = balance_sysfs_attrs,
};

static void balance_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(balance_sysfs_tbl);

	for (i = 0; i < limit; i++)
		balance_sysfs_attrs[i] = &balance_sysfs_tbl[i].attr.attr;

	balance_sysfs_attrs[limit] = NULL;
}

static struct balance_sysfs_info *balance_sysfs_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(balance_sysfs_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, balance_sysfs_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &balance_sysfs_tbl[i];
}

static int balance_sysfs_create_group(struct batt_bal_info *di)
{
	balance_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &balance_sysfs_attr_group);
}

static void balance_sysfs_remove_group(struct batt_bal_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &balance_sysfs_attr_group);
}

#else

static int balance_sysfs_create_group(struct batt_bal_info *di)
{
	return 0;
}

static void balance_sysfs_remove_group(struct batt_bal_info *di)
{
}
#endif /* CONFIG_SYSFS */

static ssize_t balance_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct balance_sysfs_info *info = NULL;
	struct batt_bal_info *di = dev_get_drvdata(dev);
	int len = -1;
	int fac_bal_en;

	info = balance_sysfs_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case BATT_BAL_SYSFS_BAL_CHANNEL_0:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.bat0_bal_en);
		break;

	case BATT_BAL_SYSFS_BAL_CHANNEL_1:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->sysfs_data.bat1_bal_en);
		break;

	case BATT_BAL_SYSFS_FACTORY_BALANCE_EN:
		fac_bal_en = di->bal_mode == BAL_MODE_DISCHG ? 1 : 0;
		len = snprintf(buf, PAGE_SIZE, "%d\n", fac_bal_en);
		break;

	case BATT_BAL_SYSFS_BAL_MODE:
		if (di->bal_mode == BAL_MODE_DISCHG)
			len = snprintf(buf, PAGE_SIZE, "factory bal mode\n");
		else if (di->bal_mode == BAL_MODE_CHG)
			len = snprintf(buf, PAGE_SIZE, "charge bal mode\n");
		else
			len = snprintf(buf, PAGE_SIZE, "no bal mode\n");
		break;

	case BATT_BAL_SYSFS_GPIO_VAL_BAL:
		len = snprintf(buf, PAGE_SIZE, "val0 = %d,val1 = %d\n",
			gpio_get_value(di->gpio_bat0_bal),
			gpio_get_value(di->gpio_bat1_bal));
		break;

	default:
		hwlog_err("HAVE NO THIS NODE:(%d)\n", info->name);
		break;
	}

	return len;
}

static ssize_t balance_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct balance_sysfs_info *info = NULL;
	struct batt_bal_info *di = dev_get_drvdata(dev);
	long val;

	info = balance_sysfs_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case BATT_BAL_SYSFS_BAL_CHANNEL_0:
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		di->sysfs_data.bat0_bal_en = val;
		if (di->sysfs_data.bat0_bal_en)
			schedule_delayed_work(&di->bal_ch0_work,
				msecs_to_jiffies(BAL_CHANNEL_OPEN_TIME));

		batt_bal_channel_ctrl(BATT_ID_0, val);
		hwlog_info("di->bat0_bal_en = [%ld]\n", val);
		break;

	case BATT_BAL_SYSFS_BAL_CHANNEL_1:
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		di->sysfs_data.bat1_bal_en = val;
		if (di->sysfs_data.bat1_bal_en)
			schedule_delayed_work(&di->bal_ch1_work,
				msecs_to_jiffies(BAL_CHANNEL_OPEN_TIME));

		batt_bal_channel_ctrl(BATT_ID_1, val);
		hwlog_info("di->bat1_bal_en = [%ld]\n", val);
		break;

	case BATT_BAL_SYSFS_FACTORY_BALANCE_EN:
		if ((kstrtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		hwlog_info("fac_bal_en = [%ld]\n", val);
		di->sysfs_data.fac_bal_en = val;

		if ((!di->sysfs_data.fac_bal_en) &&
			(di->bal_mode != BAL_MODE_DISCHG))
			break;

		if (di->sysfs_data.fac_bal_en) {
			hrtimer_cancel(&di->timer);
			cancel_work_sync(&di->bal_fsm_work);
			cancel_delayed_work_sync(&di->charge_bal_work);
			batt_bal_state_machine(BAL_STATE_IDLE);
		}

		schedule_delayed_work(&di->factory_bal_work,
			msecs_to_jiffies(0));
		break;

	default:
		hwlog_err("HAVE NO THIS NODE:(%u)\n", info->name);
		break;
	}

	return count;
}

static int battery_balance_gpio_init(struct batt_bal_info *di,
	struct device_node *np)
{
	int ret;

	di->gpio_bat0_bal = of_get_named_gpio(np,
		"gpio_bat0_balance_en", 0);
	hwlog_info("gpio_bat0_balance_en=%d\n", di->gpio_bat0_bal);

	if (!gpio_is_valid(di->gpio_bat0_bal)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		return ret;
	}

	ret = gpio_request(di->gpio_bat0_bal, "gpio_bat0_balance_en");
	if (ret) {
		hwlog_err("gpio request fail\n");
		return ret;
	}

	ret = gpio_direction_output(di->gpio_bat0_bal, 0);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		return ret;
	}

	di->gpio_bat1_bal = of_get_named_gpio(np,
		"gpio_bat1_balance_en", 0);
	hwlog_info("gpio_bat1_balance_en=%d\n", di->gpio_bat1_bal);

	if (!gpio_is_valid(di->gpio_bat1_bal)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto gpio_init_err;
	}

	ret = gpio_request(di->gpio_bat1_bal, "gpio_bat1_balance_en");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto gpio_init_err;
	}

	ret = gpio_direction_output(di->gpio_bat1_bal, 0);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto gpio_init_err;
	}

	return 0;

gpio_init_err:
	gpio_free(di->gpio_bat0_bal);
	return ret;
}

static void battery_balance_parse_dts(struct device_node *np,
	struct batt_bal_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "bal_time", &di->bal_time);
	if (ret) {
		hwlog_err("bal_time dts read failed\n");
		di->bal_time = DEFAULT_BAL_TIME;
	}
	hwlog_info("bal_time=%d\n", di->relax_time);

	ret = of_property_read_u32(np, "relax_time", &di->relax_time);
	if (ret) {
		hwlog_err("relax_time dts read failed\n");
		di->relax_time = DEFAULT_RALAX_TIME;
	}
	hwlog_info("relax_time=%d\n", di->relax_time);

	ret = of_property_read_u32(np, "vol_diff_bal_start",
		&di->vol_diff_bal_start);
	if (ret) {
		hwlog_err("vol_diff_bal_start dts read failed\n");
		di->vol_diff_bal_start = DEFAULT_VOL_DIFF_BAL_START;
	}
	hwlog_info("vol_diff_bal_start=%d\n", di->vol_diff_bal_start);

	ret = of_property_read_u32(np, "vol_diff_bal_stop",
		&di->vol_diff_bal_stop);
	if (ret) {
		hwlog_err("vol_diff_bal_stop dts read failed\n");
		di->vol_diff_bal_stop = DEFAULT_VOL_DIFF_BAL_STOP;
	}
	hwlog_info("vol_diff_bal_stop=%d\n", di->vol_diff_bal_stop);

	ret = of_property_read_u32(np, "term_curr_pulse", &di->term_curr_pulse);
	if (ret) {
		hwlog_err("term_curr_pulse dts read failed\n");
		di->term_curr_pulse = DEFAULT_TERM_CURR_PULSE;
	}
	hwlog_info("term_curr_pulse=%d\n", di->term_curr_pulse);

	ret = of_property_read_u32(np, "vol_threshold", &di->vol_threshold);
	if (ret) {
		hwlog_err("vol_threshold dts read failed\n");
		di->vol_threshold = DEFAULT_VOL_THRESHOLD;
	}
	hwlog_info("vol_threshold=%d\n", di->vol_threshold);

	ret = of_property_read_u32(np, "r_mos_mohm", &di->r_mos_mohm);
	if (ret || !di->r_mos_mohm) {
		hwlog_err("r_mos_mohm dts read failed\n");
		di->r_mos_mohm = R_MOS_MOHM;
	}
	hwlog_info("r_mos_mohm=%d\n", di->r_mos_mohm);
}

static int battery_balance_probe(struct platform_device *pdev)
{
	struct batt_bal_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;
	int ret;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_balance_di = di;
	platform_set_drvdata(pdev, di);
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;

	battery_balance_parse_dts(np, di);

	ret = battery_balance_gpio_init(di, np);
	if (ret)
		goto battery_balance_fail_0;

	ret = balance_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto battery_balance_fail_1;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charge");

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"battery_balance");
		if (ret) {
			hwlog_err("create link fail\n");
			goto battery_balance_fail_2;
		}
	}

	wake_lock_init(&di->wakelock, WAKE_LOCK_SUSPEND,
		"batt_bal_wakelock");
	di->batt_bal_wq = create_singlethread_workqueue("battery_balance_wq");
	INIT_WORK(&di->bal_fsm_work, bal_fsm_work);
	hrtimer_init(&di->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->timer.function = batt_bal_timer_func;
	INIT_DELAYED_WORK(&di->charge_bal_work, charge_bal_work);
	INIT_DELAYED_WORK(&di->factory_bal_work, factory_bal_work);
	INIT_DELAYED_WORK(&di->bal_ch0_work, bal_channel0_work);
	INIT_DELAYED_WORK(&di->bal_ch1_work, bal_channel1_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&di->bal_state_nh);

	di->chg_state_nb.notifier_call = charge_state_notifier_call;
	ret = series_batt_chg_state_notifier_register(&di->chg_state_nb);
	if (ret < 0) {
		hwlog_err("series_batt_chg_state_notifier failed\n");
		goto battery_balance_fail_3;
	}

	di->usb_nb.notifier_call = charge_usb_notifier_call;
	ret = hisi_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0) {
		hwlog_err("charger_type_notifier register failed\n");
		goto battery_balance_fail_4;
	} else {
		hwlog_info("charger_type_notifier register ok\n");
	}

	init_vol_buff(di);
	batt_bal_reset_info();
	di->sysfs_data.fac_bal_en = 0;
	di->bal_mode = BAL_MODE_MAX;
	di->bal_state = BAL_STATE_IDLE;
	di->chgen_in_pulse = CHARGE_ENABLE;
	di->in_hiz_mode = FALSE;
	di->sysfs_data.bat0_bal_en = 0;
	di->sysfs_data.bat1_bal_en = 0;
	di->chg_done = FALSE;

	hwlog_info("probe end\n");
	return 0;

battery_balance_fail_4:
	series_batt_chg_state_notifier_unregister(&di->chg_state_nb);
battery_balance_fail_3:
	wake_lock_destroy(&di->wakelock);
battery_balance_fail_2:
	balance_sysfs_remove_group(di);
battery_balance_fail_1:
	gpio_free(di->gpio_bat0_bal);
	gpio_free(di->gpio_bat1_bal);
battery_balance_fail_0:
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	g_balance_di = NULL;

	return ret;
}

#ifdef CONFIG_PM
static int battery_balance_resume(struct platform_device *pdev)
{
	enum hisi_charger_type type = hisi_get_charger_type();
	struct batt_bal_info *di = platform_get_drvdata(pdev);

	hwlog_info("resume begin\n");

	if (!di)
		return 0;

	if (type == CHARGER_TYPE_NONE) {
		hwlog_info("charger_type=%d\n", type);
		return 0;
	}

	if (di->sysfs_data.fac_bal_en)
		schedule_delayed_work(&di->factory_bal_work,
			msecs_to_jiffies(0));
	else
		schedule_delayed_work(&di->charge_bal_work,
			msecs_to_jiffies(0));

	hwlog_info("resume end\n");
	return 0;
}

static int battery_balance_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct batt_bal_info *di = platform_get_drvdata(pdev);

	hwlog_info("suspend begin\n");

	if (!di)
		return 0;

	if (di->sysfs_data.fac_bal_en)
		cancel_delayed_work(&di->factory_bal_work);

	cancel_delayed_work(&di->charge_bal_work);
	cancel_work(&di->bal_fsm_work);
	hrtimer_cancel(&di->timer);

	hwlog_info("suspend end\n");
	return 0;
}
#endif /* CONFIG_PM */

static int battery_balance_remove(struct platform_device *pdev)
{
	struct batt_bal_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	hisi_charger_type_notifier_unregister(&di->usb_nb);
	wake_lock_destroy(&di->wakelock);
	platform_set_drvdata(pdev, NULL);

	if (di->gpio_bat0_bal)
		gpio_free(di->gpio_bat0_bal);

	if (di->gpio_bat1_bal)
		gpio_free(di->gpio_bat1_bal);

	kfree(di);
	g_balance_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id battery_balance_match_table[] = {
	{
		.compatible = "huawei,battery_balance",
		.data = NULL,
	},
	{},
};

static struct platform_driver battery_balance_driver = {
#ifdef CONFIG_PM
	.resume = battery_balance_resume,
	.suspend = battery_balance_suspend,
#endif /* CONFIG_PM */
	.probe = battery_balance_probe,
	.remove = battery_balance_remove,
	.driver = {
		.name = "huawei,battery_balance",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(battery_balance_match_table),
	},
};

int __init battery_balance_init(void)
{
	return platform_driver_register(&battery_balance_driver);
}

void __exit battery_balance_exit(void)
{
	platform_driver_unregister(&battery_balance_driver);
}

module_init(battery_balance_init);
module_exit(battery_balance_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery balance module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
