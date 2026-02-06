/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_core_freq.c
 *
 * eis core freq driver header
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

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <stdbool.h>
#include <securec.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/coul/coul_event.h>
#include <linux/power/hisi/bci_battery.h>
#include "hisi_eis_core_sysfs.h"
#include "hisi_eis_data_archive.h"
#include "pmic/hisi_pmic_eis.h"
#ifdef CONFIG_BATT_SOH
#include "../soh/hisi_soh_core.h"
#endif
#include "hisi_eis_core_freq.h"

static unsigned int g_samp_start_prd;
/* global variable for adb shell read */
static unsigned int g_is_screen_on = SCREEN_IS_ON;
static struct hisi_eis_device *g_eis_di;
static struct eis_freq_device_ops *g_eis_freq_core_ops;
static struct eis_device_ops *g_eis_core_ops;
/* unit:mS, do not change the array order */
static unsigned int t_freq[] = {
	1, 2, 4, 5, 10, 16, 20, 50,
	100, 200, 500, 1000, 2000, 5000, 10000, 20000
};
/*
 * freq method real coefficient when vol gain is 1, if vol gain is N, then
 * each term in this list would miltiply 1 / N, magnified 100000 TIMES
 */
static long long k_real[] = {
	-8365, -23820, -35653, -42053, -42053,
	-35653, -23820, -8365, 8365, 23820,
	35653, 42053, 42053, 35653, 23820, 8365
};
/*
 * freq method imag coefficient when vol gain is 1, if vol gain is N, then
 * each term in this list would miltiply 1 / N, magnified 100000 TIMES
 */
static long long k_imag[] = {
	-42053, -35653, -23820, -8365, 8365,
	23820, 35653, 42053, 42053, 35653,
	23820, 8365, -8365, -23820, -35653, -42053
};

struct hisi_eis_device *get_g_eis_di(void)
{
	return g_eis_di;
}

int get_freq_work_flag(void)
{
	if (g_eis_di != NULL)
		return g_eis_di->eis_freq_dev.freq_work_flag;
	return 0;
}

static void set_freq_work_flag(int flag)
{
#ifdef CONFIG_BATT_SOH
	if (get_soh_core_device() == NULL) {
		g_eis_di->eis_freq_dev.freq_work_flag = flag;
		hisi_eis_inf("soh core device is null, flag = %d!\n", flag);
		return;
	}
	mutex_lock(&get_soh_core_device()->soh_mutex);
	g_eis_di->eis_freq_dev.freq_work_flag = flag;
	hisi_eis_inf("eis_freq is set to %d!\n", flag);
	mutex_unlock(&get_soh_core_device()->soh_mutex);
#else
	g_eis_di->eis_freq_dev.freq_work_flag = flag;
#endif
}

static void eis_wake_lock(struct hisi_eis_device *di)
{
	if (di == NULL)
		return;
	if (di->eis_wake_lock.active != 1) {
		__pm_stay_awake(&di->eis_wake_lock);
		hisi_eis_inf("eis core wake lock!\n");
	}
}

static void eis_wake_unlock(struct hisi_eis_device *di)
{
	if (di == NULL)
		return;
	if (di->eis_wake_lock.active == 1) {
		__pm_relax(&di->eis_wake_lock);
		hisi_eis_inf("eis core wake unlock!\n");
	}
}

static void set_g_is_screen_on(unsigned int val)
{
	mutex_lock(&g_eis_di->screen_update_lock);
	g_is_screen_on = val;
	mutex_unlock(&g_eis_di->screen_update_lock);
}

int eis_core_drv_ops_register(
	void *ops, enum eis_drv_ops_type ops_type)
{
	if (ops == NULL) {
		hisi_eis_err("ops is null!\n");
		return _ERROR;
	}
	switch (ops_type) {
	case FREQ_DRV_OPS:
		g_eis_freq_core_ops = ops;
		break;
	case EIS_OPS:
		g_eis_core_ops = ops;
		break;
	default:
		break;
	}
	hisi_eis_inf("ops register success!\n");
	return _SUCC;
}

static int eis_drv_freq_ops_check(struct hisi_eis_device *di)
{
	if (di->eis_freq_dev.freq_support == 0) {
		hisi_eis_err("not support eis freq!\n");
		return _ERROR;
	}

	di->eis_freq_dev.freq_ops = g_eis_freq_core_ops;

	if (di->eis_freq_dev.freq_ops == NULL) {
		hisi_eis_err("g_eis_freq_core_ops is null!\n");
		return _ERROR;
	}

	check_eis_freq_ops(di, locate_in_ary);
	check_eis_freq_ops(di, get_v_bat_gain);
	check_eis_freq_ops(di, get_v_digit_data);
	check_eis_freq_ops(di, chk_adjust_freq_i_gain);
	check_eis_freq_ops(di, chk_adjust_freq_v_gain);
	check_eis_freq_ops(di, set_cur_thres);
	check_eis_freq_ops(di, eis_sampling_init);
	check_eis_freq_ops(di, set_v_gain);
	check_eis_freq_ops(di, set_i_gain);
	return _SUCC;
}

static int eis_core_ops_check(struct hisi_eis_device *di)
{
	if (g_eis_core_ops == NULL) {
		hisi_eis_err("core ops not registered!\n");
		return _ERROR;
	}
	di->eis_ops = g_eis_core_ops;

	check_eis_ops(di, get_fifo_period_now);
	check_eis_ops(di, get_fifo_order_now);
	check_eis_ops(di, get_v_fifo_data);
	check_eis_ops(di, get_i_fifo_data);
	check_eis_ops(di, read_and_clear_irq);
	check_eis_ops(di, irq_seq_clear);
	check_eis_ops(di, irq_seq_read);
	check_eis_ops(di, eis_detect_flag);
	check_eis_ops(di, set_t_n_m);
	check_eis_ops(di, get_t_n_m);
	check_eis_ops(di, eis_enable);
	check_eis_ops(di, cnt_overflow_num);
	check_eis_ops(di, cur_offset);

	return _SUCC;
}


int eis_drv_ops_check(
	struct hisi_eis_device *di, enum eis_drv_ops_type type)
{
	int ret = _ERROR;

	if (di == NULL) {
		hisi_eis_err("di is null!\n");
		return ret;
	}
	switch (type) {
	case FREQ_DRV_OPS:
		ret = eis_drv_freq_ops_check(di);
		break;
	case EIS_OPS:
		ret = eis_core_ops_check(di);
		break;
	default:
		hisi_eis_err("ops register failed!\n");
		break;
	}
	return ret;
}

static int parse_eis_freq_dts(struct hisi_eis_device *di)
{
	struct device_node *np = NULL;

	if (di == NULL) {
		hisi_eis_err("di is null\n");
		return _ERROR;
	}

	np = di->dev->of_node;
	if (np == NULL) {
		hisi_eis_err("np is null\n");
		return _ERROR;
	}

	if (of_property_read_u32(np, "freq_support",
		(u32 *)&di->eis_freq_dev.freq_support)) {
		/* 0: not support */
		di->eis_freq_dev.freq_support = 0;
		hisi_eis_err("get freq support fail\n");
	}
	hisi_eis_inf("eis freq support = %d\n",
			 di->eis_freq_dev.freq_support);
	return _SUCC;
}

static int hisi_eis_is_screen_on_callback(
	struct notifier_block *nb, unsigned long event, void *data)
{
	switch (event) {
	case FB_EVENT_BLANK:
		hisi_eis_inf("screen is off\n");
		set_g_is_screen_on(SCREEN_IS_OFF);
		break;
	case FB_EARLY_EVENT_BLANK:
		hisi_eis_inf("screen is on\n");
		set_g_is_screen_on(SCREEN_IS_ON);
		break;
	default:
		hisi_eis_inf("event = 0x%lx!\n", event);
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

static void hisi_eis_register_fb_chain_list(struct hisi_eis_device *di)
{
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null\n");
		return;
	}
	di->screen_status_nb.notifier_call = hisi_eis_is_screen_on_callback;
	ret = fb_register_client(&di->screen_status_nb);
	if (ret) {
		hisi_eis_err("ret of fb_register = %d, failed!\n", ret);
		return;
	}
	hisi_eis_inf("succeed!\n", ret);
}

static void hisi_eis_unregister_fb_chain_list(struct hisi_eis_device *di)
{
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null\n");
		return;
	}
	ret = fb_unregister_client(&di->screen_status_nb);
	hisi_eis_inf("ret of fb_unregister = %d!\n", ret);
}

static bool hisi_eis_cur_temp_soc_check(void)
{
	int bat_temp = coul_drv_battery_temperature();
	int i_bat = coul_drv_battery_current();
	int soc = coul_drv_battery_capacity();
#ifdef CONFIG_HISI_DEBUG_FS
	int temper_test;
#endif

	if (abs(i_bat) > EIS_FREQ_ENTER_CUR) {
		hisi_eis_err("i_bat = %d, over |%u| mA\n",
			i_bat, EIS_FREQ_ENTER_CUR);
		return false;
	}
#ifdef CONFIG_HISI_DEBUG_FS
	temper_test = get_test_bat_temp();
	if (temper_test < EIS_FREQ_TEMP_LOW || temper_test > EIS_FREQ_TEMP_HIGH) {
		hisi_eis_err("bat_temp_test = %d, over [%u,%u]\n",
			temper_test, EIS_FREQ_TEMP_LOW, EIS_FREQ_TEMP_HIGH);
		return false;
	}
#endif
	if (bat_temp < EIS_FREQ_TEMP_LOW || bat_temp > EIS_FREQ_TEMP_HIGH) {
		hisi_eis_err("bat_temp = %d, over [%u,%u]\n",
			bat_temp, EIS_FREQ_TEMP_LOW, EIS_FREQ_TEMP_HIGH);
		return false;
	}

	if (soc < EIS_FREQ_SOC) {
		hisi_eis_err("bat_soc = %d, less than %u\n", soc, EIS_FREQ_SOC);
		return false;
	}

	return true;
}

bool hisi_eis_is_sampling_allowed(enum eis_mode mode)
{
	int bat_cycle_now = coul_drv_battery_cycle_count();
	int latest_history_cycle;
	int full_freq_rolling, freq_rolling_now;
	struct eis_freq_info freq_his_inf[LEN_T_FREQ] = {};
	int last_rolling;
	bool ret = false;

	full_freq_rolling = eis_get_g_full_freq_rolling();
	freq_rolling_now = eis_get_g_freq_rolling_now();
	if (full_freq_rolling > 0 || freq_rolling_now > 0) {
		last_rolling = freq_rolling_now - 1;
		if (last_rolling < 0)
			last_rolling = EIS_FREQ_ROLLING - 1;
	} else {
		last_rolling = freq_rolling_now;
	}

	switch (mode) {
	case EIS_FREQ:
		if (hisi_eis_cur_temp_soc_check() == false)
			return false;
		/* 0: newly run eis freq */
		if (full_freq_rolling == 0 && freq_rolling_now == 0) {
			latest_history_cycle = 0;
			hisi_eis_inf("cycle_now = %d, cycle_history = %d, first run eis freq\n",
				 bat_cycle_now, latest_history_cycle);
			ret = true;
		} else {
			hisi_eis_freq_read_single_info_from_flash(freq_his_inf,
				last_rolling);
			/* 15: the last record */
			latest_history_cycle = freq_his_inf[LEN_T_FREQ - 1].cycle;
			hisi_eis_inf("latest_history_cycle = %d\n",
				latest_history_cycle);
			if (bat_cycle_now == latest_history_cycle) {
				hisi_eis_err("cycle_now = %d, cycle_history = %d, in the same cycle\n",
					bat_cycle_now, latest_history_cycle);
				ret = false;
			} else {
				hisi_eis_inf("cycle_now = %d, cycle_history = %d, not in the same cycle\n",
					bat_cycle_now, latest_history_cycle);
				ret = true;
			}
		}
		return ret;
	default:
		hisi_eis_err("mode = %u, input invalid!\n", mode);
		return false;
	}
}

static void collect_para_prepare(struct hisi_eis_device *di,
	struct collect_para *para, enum eis_mode mode, int idx)
{
	long long timeout_ms;
	errno_t err;

	switch (mode) {
	case EIS_FREQ:
		para->ptr_v = di->eis_freq_dev.freq_sample_info[idx].v_bat;
		para->ptr_i = di->eis_freq_dev.freq_sample_info[idx].i_bat;
		para->key_prds = -1; /* not use this flag */
		timeout_ms = (long long)t_freq[idx];
		break;
	default:
		hisi_eis_err("invalid mode = %u, sampling stopped\n", mode);
		return;
	}
	para->timeout_ms = timeout_ms;

	if (idx < T_W_COLLECT_FLAG || idx > F_COLLECT_FLAG) {
		err = memset_s(para->ptr_v, SIZEOF_VBAT_SAMPLE, 0,
			SIZEOF_VBAT_SAMPLE);
		if (err != EOK) {
			hisi_eis_err("v_bat memset_s failed\n");
			return;
		}
		err = memset_s(para->ptr_i, SIZEOF_IBAT_SAMPLE, 0,
			SIZEOF_IBAT_SAMPLE);
		if (err != EOK) {
			hisi_eis_err("i_bat memset_s failed\n");
			return;
		}
	}
}

static void hisi_eis_traverse_cur_fifo(struct hisi_eis_device *di, int irq_idx,
	unsigned int (*ptr_i)[I_BAT_FIFO_DEPTH], unsigned int p_now, int shift)
{
	int q, r;

	/*
	 * when i equals to 3, 5; r in the range of [0, 1], [2, 3]
	 * q: intermediate variable
	 * 1: intermediate variable
	 * 2: half of I_BAT_FIFO_DEPTH
	 * r: cur fifo order, r + 1: func param starts from 1
	 */
	q = irq_idx - 1;
	r = q - 2;
	hisi_eis_debug("g_samp_start_prd = %u!\n", g_samp_start_prd);
	while (r < q) {
		ptr_i[p_now - g_samp_start_prd + shift][r] =
			di->eis_ops->get_i_fifo_data(r + 1);
		r++;
	}
}

static void hisi_eis_traverse_vol_fifo(struct hisi_eis_device *di, int irq_idx,
	unsigned int (*ptr_v)[V_BAT_FIFO_DEPTH], unsigned int p_now, int shift)
{
	int q, r;

	/*
	 * when i equals to 3, 5; r in the range of [0, 7], [8, 15]
	 * q: intermediate variable
	 * 4: intermediate variable
	 * 8: half of V_BAT_FIFO_DEPTH
	 * r: vol fifo order, r + 1: func param starts from 1
	 */
	q = 4 * (irq_idx - 1);
	r = q - 8;
	hisi_eis_debug("g_samp_start_prd = %u!\n", g_samp_start_prd);
	while (r < q) {
		ptr_v[p_now - g_samp_start_prd + shift][r] =
			di->eis_ops->get_v_fifo_data(r + 1);
		r++;
	}
}

static void clear_all_irq_before_collect_and_enable(
	struct hisi_eis_device *di, int idx, unsigned int n, long long prd_ms)
{
	int eis_irq;
	struct timeval tv;
	long long time_b, time_a, time_span;

	/* idx > 4 for freq >=10mS in freq mode, idx < 0 for t-w mode */
	if (idx > F_COLLECT_FLAG || idx < T_W_COLLECT_FLAG) {
		di->eis_ops->irq_seq_clear(EIS_IRQ_START, EIS_FIFO_IRQ_16);
		di->eis_ops->eis_enable(true);
		eis_irq = di->eis_ops->irq_seq_read(
			EIS_IRQ_START, EIS_FIFO_IRQ_16);
		/* wait for null periods to be depleted if n isn't zero */
		do_gettimeofday(&tv);
		time_b = (long long)(tv.tv_sec * S_TO_MS) +
			tv.tv_usec / MS_TO_US;
		if (n == 0) {
			di->eis_ops->irq_seq_clear(EIS_FIFO_IRQ_4, EIS_FIFO_IRQ_16);
			return;
		}

		while (eis_irq != EIS_IRQ_NULL_DEPLETED) {
			udelay(1); /* 1: 1uS to avoid busy */
			eis_irq = di->eis_ops->irq_seq_read(
				EIS_IRQ_START, EIS_FIFO_IRQ_16);
			do_gettimeofday(&tv);
			time_a = (long long)(tv.tv_sec * S_TO_MS) +
				tv.tv_usec / MS_TO_US;
			time_span = time_a - time_b;
			if (time_span > prd_ms * n) {
				hisi_eis_err("time span = %lld, time out = %lld, abort!\n",
					time_span, prd_ms * n);
				return;
			}
		}
		/* before collecting data, clear irq4~irq16 irq status */
		di->eis_ops->irq_seq_clear(EIS_FIFO_IRQ_4, EIS_FIFO_IRQ_16);
	}
}

static int check_if_stop_in_advance(
	struct hisi_eis_device *di, int idx, int p)
{
	int eis_end_irq;

	eis_end_irq = di->eis_ops->read_and_clear_irq(EIS_IRQ_END, _READ);
	/* 1: eis work ended, 4: idx of freq > 10mS, idx < 0, t-w mode */
	if (eis_end_irq == 1 && (idx > F_COLLECT_FLAG || idx < T_W_COLLECT_FLAG)) {
		hisi_eis_err("period_now=%d, EIS_IRQ=:%x, idx = %d end irq caused sample_lock stop\n",
			 p, eis_end_irq, idx);
		/* stopped in advance */
		return _ERROR;
	}
	/* didn't stop in advance */
	return _SUCC;
}

static int wait_until_fifo_is_filled_up(
	struct hisi_eis_device *di, int irq_num, int idx, long long timeout)
{
	unsigned int eis_irq_tmp;
	long long t_b, t_a;
	struct timeval tv_1;

	eis_irq_tmp = di->eis_ops->read_and_clear_irq(irq_num, _READ);
	hisi_eis_debug("eis_irq_tmp = %u!\n", eis_irq_tmp);
	/* idx = 4, freq mode when freq > 10mS, idx < 0, t-w mode */
	if (idx > F_COLLECT_FLAG || idx < T_W_COLLECT_FLAG) {
		hisi_eis_inf("idx = %d!\n", idx);
		do_gettimeofday(&tv_1);
		t_b = (long long)(tv_1.tv_sec * S_TO_MS) +
			tv_1.tv_usec / MS_TO_US;
		hisi_eis_debug("idx = %d, t_b = %ld!\n", idx, t_b);
		while (eis_irq_tmp != EIS_FIFO_DATA_FILLED) {
			eis_irq_tmp = di->eis_ops->read_and_clear_irq(
				irq_num, _READ);
			do_gettimeofday(&tv_1);
			t_a = (long long)(tv_1.tv_sec * S_TO_MS) +
				tv_1.tv_usec / MS_TO_US;
			if (t_a - t_b > timeout) {
				hisi_eis_err("time span = %lld, time out = %lld, abort!\n",
					t_a - t_b, timeout);
				/* _ERROR: time out */
				return _ERROR;
			}
		}
	}
	/* _SUCC: not time out */
	return _SUCC;
}

static int if_collect_time_out(
	int idx, long long time_before, long long tm_out)
{
	struct timeval tv;
	long long time_val_a1, time_span;

	do_gettimeofday(&tv);
	time_val_a1 = (long long)(tv.tv_sec * S_TO_MS) + tv.tv_usec / MS_TO_US;
	time_span = time_val_a1 - time_before;
	if (time_span > tm_out) {
		hisi_eis_err("idx = %d, time_span = %lld, timeout = %lld, time out!\n",
			idx, time_span, tm_out);
		/* _ERROR: time out */
		return _ERROR;
	}
	/* _SUCC: not time out */
	return _SUCC;
}
/* idx is only valid for freq mode, it is assigned to minus num for t-w mode */
static void hisi_eis_sampling_data_collect(
	enum eis_mode mode, unsigned int periods,
	struct hisi_eis_device *di, int idx, int shift)
{
	unsigned int p_now;
	int i, ret;
	unsigned int t_n_m[T_N_M_PARA_LEN] = {0};
	struct timeval tv_0;
	long long t_ms[TIMEOUT_PARA_NUM] = {0};
	struct collect_para para = {};

	collect_para_prepare(di, &para, mode, idx);
	t_ms[1] = para.timeout_ms;
	/* 0, 1, 2: the 1st, 2nd, 3rd term of array */
	di->eis_ops->get_t_n_m(&t_n_m[0], &t_n_m[1], &t_n_m[2]);

	/* clear all irq status */
	clear_all_irq_before_collect_and_enable(di, idx, t_n_m[1], t_ms[1]);
	p_now = di->eis_ops->get_fifo_period_now();
	g_samp_start_prd = p_now;
	t_ms[2] = t_ms[1] * (periods + 1);
	do_gettimeofday(&tv_0);
	t_ms[0] = (long long)(tv_0.tv_sec * S_TO_MS) + tv_0.tv_usec / MS_TO_US;
	while (p_now - g_samp_start_prd < periods) {
		ret = check_if_stop_in_advance(di, idx, p_now);
		if (ret != _SUCC)
			return;
		for (i = EIS_FIFO_IRQ_8; i <= EIS_FIFO_IRQ_16; i++) {
			if (i == EIS_FIFO_IRQ_12)
				continue;
			ret = wait_until_fifo_is_filled_up(di, i, idx, t_ms[1]);
			if (ret != _SUCC)
				break;
			hisi_eis_traverse_cur_fifo(di, i, para.ptr_i,
				p_now, shift);
			hisi_eis_traverse_vol_fifo(di, i, para.ptr_v,
				p_now, shift);
			/* clear irq4, irq8; irq12, irq16 correspondingly */
			di->eis_ops->read_and_clear_irq(i - 1, _CLEAR);
			di->eis_ops->read_and_clear_irq(i, _CLEAR);
		}
		/* update p_now */
		p_now = (idx > 4 || idx < 0) ?
			di->eis_ops->get_fifo_period_now() : (p_now + 1);
		ret = if_collect_time_out(idx, t_ms[0], t_ms[2]);
		if (ret != _SUCC)
			return;
	}
}
/* idx is only valid for freq mode, it is redundant for t-w mode */
static void hisi_eis_sampling_data_print(enum eis_mode mode,
	struct hisi_eis_device *di, int idx)
{
	unsigned int (*ptr_v)[V_BAT_FIFO_DEPTH];
	unsigned int (*ptr_i)[I_BAT_FIFO_DEPTH];
	unsigned int freq;
	int i, periods;
	int cur[I_BAT_FIFO_DEPTH], vol[V_BAT_FIFO_DEPTH];

	switch (mode) {
	case EIS_FREQ:
		ptr_v = di->eis_freq_dev.freq_sample_info[idx].v_bat;
		ptr_i = di->eis_freq_dev.freq_sample_info[idx].i_bat;
		freq = t_freq[idx];
		periods = di->eis_freq_dev.freq_sample_info[idx].sample_prds;
		break;
	default:
		return;
	}
	for (i = 0; i < periods; i++) {
		cur[0] = ptr_i[i][0];
		cur[1] = ptr_i[i][1];
		cur[2] = ptr_i[i][2];
		cur[3] = ptr_i[i][3];
		hisi_eis_inf("%s:sample_freq = 1000/%u, %dth cur = %d, %d, %d, %d\n",
			"EIS_FREQ", freq, i, cur[0], cur[1], cur[2], cur[3]);
		vol[0] = ptr_v[i][0];
		vol[1] = ptr_v[i][1];
		vol[2] = ptr_v[i][2];
		vol[3] = ptr_v[i][3];
		vol[4] = ptr_v[i][4];
		vol[5] = ptr_v[i][5];
		vol[6] = ptr_v[i][6];
		vol[7] = ptr_v[i][7];
		vol[8] = ptr_v[i][8];
		vol[9] = ptr_v[i][9];
		vol[10] = ptr_v[i][10];
		vol[11] = ptr_v[i][11];
		vol[12] = ptr_v[i][12];
		vol[13] = ptr_v[i][13];
		vol[14] = ptr_v[i][14];
		vol[15] = ptr_v[i][15];
		hisi_eis_inf("%s:sample_freq = 1000/%u, %dth vol = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			"EIS_FREQ", freq, i, vol[0], vol[1], vol[2], vol[3],
			vol[4], vol[5], vol[6], vol[7], vol[8], vol[9], vol[10],
			vol[11], vol[12], vol[13], vol[14], vol[15]);
	}
}

static int wait_eis_working_stop(
	struct hisi_eis_device *di, struct timeval tv, long timeout_ms)
{
	int eis_detect_flag;
	struct timeval tv_tmp;
	long long time_span, time_b, time_a;

	if (di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return _ERROR;
	}
	time_b = (long long)(tv.tv_sec * S_TO_MS) + tv.tv_usec / MS_TO_US;
	eis_detect_flag = di->eis_ops->eis_detect_flag();
	while (eis_detect_flag == EIS_IS_WORKING) {
		eis_detect_flag = di->eis_ops->eis_detect_flag();
		do_gettimeofday(&tv_tmp);
		time_a = (long long)(tv_tmp.tv_sec * S_TO_MS) +
			tv_tmp.tv_usec / MS_TO_US;
		time_span = time_a - time_b;
		if (time_span > timeout_ms) {
			hisi_eis_err("time span = %lld, timeout!\n", time_span);
			return _ERROR;
		}
	}
	return _SUCC;
}

static void eis_freq_info_temp_cur_soc_update(
	struct eis_freq_device *di, int idx)
{
	int bat_temp, i_bat, soc;

	bat_temp = coul_drv_battery_temperature();
	i_bat = coul_drv_battery_current();
	soc = coul_drv_battery_capacity();
	di->freq_info[idx].temp = bat_temp;
	di->freq_info[idx].cur = i_bat;
	di->freq_info[idx].soc = soc;
}

static void sampling_for_freq_less_than_ten_ms(
	struct eis_freq_device *di, int freq, int idx)
{
	struct hisi_eis_device *eis_di;
	int p, eis_irq;
	struct timeval tv;
	int cur_offset;

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return;
	}
	eis_freq_info_temp_cur_soc_update(di, idx);
	for (p = 0; p < FREQ_SAMP_PRDS_10; p++) {
		/* clear all irq status */
		eis_di->eis_ops->irq_seq_clear(EIS_IRQ_START,
			EIS_FIFO_IRQ_16);
		/*
		 * for the first sampling, null period is set to 4,
		 * sampling period is set to 2; for the rest periods,
		 * null period is set to 0, sampling period is set to 2
		 */
		do_gettimeofday(&tv);
		if (wait_eis_working_stop(eis_di, tv, WAIT_EIS_END) == _ERROR) {
			hisi_eis_err("wait working stop time out!\n");
			return;
		}

		if (p == 0)
			/* 4: null periods, 1: sampling periods */
			eis_di->eis_ops->set_t_n_m(freq, 4, 1);
		else
			/* 1: null periods, 1: sampling periods */
			eis_di->eis_ops->set_t_n_m(freq, 1, 1);
		eis_di->eis_ops->eis_enable(true);
		eis_irq = eis_di->eis_ops->irq_seq_read(EIS_IRQ_START,
			EIS_FIFO_IRQ_16);
		hisi_eis_debug("p = %d, eis_irq = %d!\n", p, eis_irq);
		do_gettimeofday(&tv);
		if (wait_eis_working_stop(eis_di, tv, WAIT_EIS_END) == _ERROR) {
			hisi_eis_err("wait working stop time out!\n");
			return;
		}
		hisi_eis_sampling_data_collect(EIS_FREQ, 1, eis_di, idx, p);
		eis_di->eis_ops->eis_enable(false);
		cur_offset = eis_di->eis_ops->cur_offset();
		hisi_eis_inf("idx = %d, p = %d, cur_offset = %d!\n", idx, p, cur_offset);
	}
}

static void sampling_for_freq_less_than_thousand_ms(
	struct eis_freq_device *di, int freq, int idx)
{
	struct hisi_eis_device *eis_di = NULL;
	int cur_offset;

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return;
	}

	eis_freq_info_temp_cur_soc_update(di, idx);
	/* 15: set one more for the depletion of intermediate process */
	eis_di->eis_ops->set_t_n_m(freq, 1, 15); /* F_SAMP_NULL_PRDS */
	/* 0: shift set to zero for cases when freq greater than 10mS */
	hisi_eis_sampling_data_collect(
		EIS_FREQ, FREQ_SAMP_PRDS_10, eis_di, idx, 0);
	cur_offset = eis_di->eis_ops->cur_offset();
	hisi_eis_inf("idx = %d, cur_offset = %d!\n", idx, cur_offset);
	hisi_eis_debug("freq < 1000!\n");
	di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_10;
	eis_di->eis_ops->eis_enable(false);
}

static void sampling_for_freq_greater_than_thousand_ms(
	struct eis_freq_device *di, int freq, int idx)
{
	struct hisi_eis_device *eis_di = NULL;
	int cur_offset;

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	if (eis_di == NULL) {
		hisi_eis_err("null eis_di!\n");
		return;
	}

	eis_freq_info_temp_cur_soc_update(di, idx);
	/* 0: null periods, 3: extra 3 periods for preparing */
	eis_di->eis_ops->set_t_n_m(freq, 0, FREQ_SAMP_PRDS_5 + 3);
	hisi_eis_debug("freq = %d!\n", freq);
	/* 0: shift set to zero for cases when freq greater than 10mS */
	hisi_eis_sampling_data_collect(
		EIS_FREQ, FREQ_SAMP_PRDS_5, eis_di, idx, 0);
	cur_offset = eis_di->eis_ops->cur_offset();
	hisi_eis_inf("idx = %d, cur_offset = %d!\n", idx, cur_offset);
	hisi_eis_debug("freq < 20000!\n");
	di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_5;
	eis_di->eis_ops->eis_enable(false);
}

static void hisi_eis_freq_start_sampling_func(
	struct eis_freq_device *di, int idx)
{
	int freq;
	struct timeval tv;
	struct hisi_eis_device *eis_di = NULL;

	if (di == NULL || idx < 0 || idx > LEN_T_FREQ - 1) {
		hisi_eis_err("input invalid!\n");
		return;
	}
	/* 0: just a intermediate variable */
	freq = t_freq[idx];
	hisi_eis_debug("freq = %d, idx = %d!\n", freq, idx);
	di->freq_info[idx].freq = freq;
	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	di->freq_sample_info[idx].sample_freq = freq;
	eis_di->eis_freq_dev.freq_ops->eis_sampling_init();
	/* if eis is working, wait until it's not working */
	do_gettimeofday(&tv);
	if (wait_eis_working_stop(eis_di, tv, WAIT_EIS_END) == _ERROR) {
		hisi_eis_err("wait working stop time out!\n");
		return;
	}
	/* 10: freq eauals 10mS */
	if (freq <= 10) {
		hisi_eis_debug("freq = %d!\n", freq);
		sampling_for_freq_less_than_ten_ms(di, freq, idx);
		hisi_eis_debug("freq < 10!\n");
		di->freq_sample_info[idx].sample_prds = FREQ_SAMP_PRDS_10;
	/* 1000: freq eauals 1000mS */
	} else if (freq <= 1000) {
		sampling_for_freq_less_than_thousand_ms(di, freq, idx);
	} else {
		sampling_for_freq_greater_than_thousand_ms(di, freq, idx);
	}
	hisi_eis_debug("completed!\n");
}

void hisi_eis_set_cur_threshold(struct hisi_eis_device *di, enum eis_mode mode)
{
	int eis_detect_flag;
	int curr_offset;
	struct timeval tv;

	/* get offset current and set current threshold */
	di->eis_freq_dev.freq_ops->eis_sampling_init();
	di->eis_ops->irq_seq_clear(EIS_IRQ_START, EIS_FIFO_IRQ_16);
	/* 4, 0,1: period, null periods, sampling periods */
	di->eis_ops->set_t_n_m(4, 0, 1);
	eis_detect_flag = di->eis_ops->eis_detect_flag();
	hisi_eis_debug("eis_detect_flag = %d!\n", eis_detect_flag);
	do_gettimeofday(&tv);
	if (wait_eis_working_stop(di, tv, WAIT_EIS_END) == _ERROR) {
		hisi_eis_err("wait working stop time out!\n");
		return;
	}

	di->eis_ops->eis_enable(true);
	hisi_eis_debug("eis_enabled!\n");

	do_gettimeofday(&tv);
	if (wait_eis_working_stop(di, tv, WAIT_EIS_END) == _ERROR) {
		hisi_eis_err("wait working stop time out!\n");
		return;
	}
	hisi_eis_debug("eis_detect_flag = %d!\n", eis_detect_flag);
	di->eis_ops->eis_enable(false);
	curr_offset = di->eis_ops->cur_offset();
	hisi_eis_inf("cur_offset = %d!\n", curr_offset);
	di->eis_freq_dev.freq_ops->set_cur_thres(curr_offset);
	hisi_eis_inf("mode = %u, set threshold finished!\n", mode);
}

static int check_and_adjust_v_i_gain_for_freq(
	struct eis_freq_device *di, int idx)
{
	int ret_v, ret_i;

	ret_v = di->freq_ops->chk_adjust_freq_v_gain(
		&di->freq_sample_info[idx].v_gain,
		di->freq_sample_info[idx].sample_prds,
		(int (*)[V_BAT_FIFO_DEPTH])di->freq_sample_info[idx].v_bat);
	/* if i_gain gear is improper, resample */
	ret_i = di->freq_ops->chk_adjust_freq_i_gain(
		&di->freq_sample_info[idx].i_gain,
		di->freq_sample_info[idx].sample_prds,
		(int (*)[I_BAT_FIFO_DEPTH])di->freq_sample_info[idx].i_bat);
	if (ret_v == V_GAIN_ADJUSTED || ret_i == V_GAIN_ADJUSTED)
		return V_I_GAIN_ADJUSTED;
	return V_I_GAIN_UNADJUSTED;
}

static int hisi_eis_freq_sampling_work_exe(struct eis_freq_device *di)
{
	int err_cnt = 0;
	int i, ret, err_idx, bat_cycle_now;
	int gain_err_freq[LEN_T_FREQ] = {0};
	struct hisi_eis_device *eis_di = NULL;

	if (di == NULL) {
		hisi_eis_err("di is null!\n");
		return _ERROR;
	}
	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);

	for (i = 0; i < LEN_T_FREQ; i++) {
		hisi_eis_set_cur_threshold(eis_di, EIS_FREQ);
		/* 0: just an intermediate variable, could be [0, 15] */
		bat_cycle_now = coul_drv_battery_cycle_count();
		di->freq_info[i].cycle = bat_cycle_now;
		hisi_eis_freq_start_sampling_func(di, i);

		/* if v_gain gear is improper, resample */
		hisi_eis_debug("%dth sample_prds = %d!\n", i,
			di->freq_sample_info[i].sample_prds);
		ret = check_and_adjust_v_i_gain_for_freq(di, i);
		if (ret == V_I_GAIN_ADJUSTED) {
			gain_err_freq[err_cnt] = t_freq[i];
			err_cnt++;
		}
		msleep(50); /* 50: delay 50mS for next sampling */
		hisi_eis_inf("%dth, freq = %d,slept 50mS!\n", i, t_freq[i]);
	}

	/* for those improper gain gear freq cases, re-samplig later */
	for (i = 0; i < err_cnt; i++) {
		hisi_eis_err("gain_gear_err happened!\n");
		err_idx = di->freq_ops->locate_in_ary(
			t_freq, LEN_T_FREQ, gain_err_freq[i]);
		di->freq_ops->set_i_gain(di->freq_sample_info[err_idx].i_gain);
		di->freq_ops->set_v_gain(di->freq_sample_info[err_idx].v_gain);
		hisi_eis_freq_start_sampling_func(di, err_idx);
		msleep(50); /* 50: delay 50mS for next sampling */
		hisi_eis_err("%dth, freq = %d resample ended,slept 50mS!\n",
			i, gain_err_freq[i]);
	}
	return _SUCC;
}

static int is_all_element_zero_in_array(const int *array, int len)
{
	int i;
	int cnt = 0;

	if (array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null array, invalid\n", len);
		return _ERROR;
	}
	for (i = 0; i < len; i++) {
		if (array[i] == 0)
			cnt += 1;
	}
	return cnt;
}

static int hisi_eis_is_freq_sampling_valid(
	int frq, struct eis_freq_device *di)
{
	unsigned int i, j;
	int ret = _ERROR;
	unsigned int invalid_cnt = 0;
	int cur_over_flag, idx, act_v;

	idx = di->freq_ops->locate_in_ary(t_freq, LEN_T_FREQ, frq);
	if (idx == -1) {
		hisi_eis_err("freq = %d,invalid input\n", frq);
		return ret;
	}

	for (i = 0; i < di->freq_sample_info[idx].sample_prds; i++) {
		if (is_all_element_zero_in_array(
			di->freq_sample_info[idx].v_bat[i],
			V_BAT_FIFO_DEPTH) == V_BAT_FIFO_DEPTH) {
			invalid_cnt += 1;
			di->freq_sample_info[idx].valid_prd_idx[i] = false;
			continue;
		}
		for (j = 0; j < V_BAT_FIFO_DEPTH; j++) {
			/* 0, 8: no need to judge D1 and D9 */
			if (j == 0 || j == 8)
				continue;
			/*
			 * 12: the <12> bit
			 * 0,11: <11:0> bit
			 */
			cur_over_flag = eis_reg_getbits(
				di->freq_sample_info[idx].v_bat[i][j], 12, 12);
			act_v = eis_reg_getbits(
				di->freq_sample_info[idx].v_bat[i][j], 0, 11);
			if (cur_over_flag == CUR_OVERFLOW_FLAG || act_v == 0) {
				invalid_cnt += 1;
				di->freq_sample_info[idx].valid_prd_idx[i] = false;
				hisi_eis_inf("%dth period is invalid\n", i);
				break;
			}
		}
		if (j == V_BAT_FIFO_DEPTH) {
			di->freq_sample_info[idx].valid_sample_cnt += 1;
			di->freq_sample_info[idx].valid_prd_idx[i] = true;
		}
	}

	/* 2: invalid counts less than half of total periods is valid */
	if (2 * invalid_cnt <= di->freq_sample_info[idx].sample_prds) {
		di->freq_info[idx].sample_valid = true;
		hisi_eis_inf("freq = %d, EIS sample valid\n", t_freq[idx]);
		ret = _SUCC;
	} else {
		di->freq_info[idx].sample_valid = false;
		hisi_eis_err("freq = %d,EIS sample invalid\n", t_freq[idx]);
		ret = _ERROR;
	}
	return ret;
}

static int _compare(const void *_a, const void *_b)
{
	long long *a = (long long *)_a;
	long long *b = (long long *)_b;

	if (*a > *b)
		return 1;
	if (*a < *b)
		return -1;
	return 0;
}

static bool is_all_element_non_negative(
	const long long *sorted_array, int len)
{
	int i;

	if (sorted_array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null ptr, invalid\n", len);
		return false;
	}

	for (i = 0; i < len; i++) {
		if (sorted_array[i] < 0)
			return false;
	}

	return true;
}

/*
 * for valid eis impedance over 5 cases, sort them and get the avg of
 * intermediate 5, otherwise, get the avged eis directly
 */
static long long get_averaged_freq_eis_real(
	const long long *sorted_array, int len)
{
	int i, start, end;
	int len_non_zero = 0;
	long long ret = 0;
	long long mean_thres;
	long long non_zero_ary[FREQ_SAMP_PRDS_10];

	if (sorted_array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null ptr, invalid\n", len);
		return _ERROR;
	}

	for (i = 0; i < len; i++) {
		if (sorted_array[i] != 0) {
			non_zero_ary[len_non_zero] = sorted_array[i];
			len_non_zero++;
		}
	}

	if (len_non_zero == 0) {
		hisi_eis_inf("all real tmp is 0, retutn 0\n");
		return 0;
	}

	if (len_non_zero <= FREQ_EIS_AVGED_LEN) {
		for (i = 0; i < len_non_zero; i++)
			ret += non_zero_ary[i];
		ret /= len_non_zero;
		mean_thres = ret / 10; /* 10% */
		for (i = 0; i < len_non_zero; i++) {
			if (abs(non_zero_ary[i] - ret) > mean_thres) {
				hisi_eis_err("mean diff over 10%, eis_real set -1\n");
				ret = -1;
				break;
			}
		}
	} else {
		start = (len_non_zero - FREQ_EIS_AVGED_LEN) / 2;
		end = start + FREQ_EIS_AVGED_LEN;
		for (i = start; i < end; i++)
			ret += non_zero_ary[i];
		ret /= FREQ_EIS_AVGED_LEN;
		mean_thres = ret / 10; /* 10% */
		for (i = start; i < end; i++) {
			if (abs(non_zero_ary[i] - ret) > mean_thres) {
				hisi_eis_err("mean diff over 10%, eis_real set -1\n");
				ret = -1;
				break;
			}
		}
	}

	return ret;
}

static long long get_averaged_freq_eis_imag(
	const long long *sorted_array, int len)
{
	int i;
	int len_non_zero = 0;
	int start;
	int end;
	long long ret = 0;
	long long non_zero_ary[FREQ_SAMP_PRDS_10];

	if (sorted_array == NULL || len < 1) {
		hisi_eis_err("len = %d, or null ptr, invalid\n", len);
		return _ERROR;
	}
	for (i = 0; i < len; i++) {
		if (sorted_array[i] != 0) {
			non_zero_ary[len_non_zero] = sorted_array[i];
			len_non_zero++;
		}
	}

	if (len_non_zero == 0) {
		hisi_eis_inf("all imag tmp is 0, retutn 0\n");
		return 0;
	}

	if (len_non_zero <= FREQ_EIS_AVGED_LEN) {
		for (i = 0; i < len_non_zero; i++)
			ret += non_zero_ary[i];
		ret /= len_non_zero;
		hisi_eis_debug("ret = %lld\n", ret);
	} else {
		start = (len_non_zero - FREQ_EIS_AVGED_LEN) / 2;
		end = start + FREQ_EIS_AVGED_LEN;
		for (i = start; i < end; i++)
			ret += non_zero_ary[i];
		ret /= FREQ_EIS_AVGED_LEN;
		hisi_eis_debug("ret = %lld\n", ret);
	}

	return ret;
}

static int hisi_eis_init_freq_raw_to_zero(
	struct eis_freq_device *di, int idx)
{
	int ret;

	ret = memset_s(di->freq_sample_info[idx].raw_eis_r, SIZE_OF_RAW_EIS, 0,
		SIZE_OF_RAW_EIS);
	if (ret != EOK) {
		hisi_eis_err("memset_s failed, ret = %d!\n", ret);
		return _ERROR;
	}
	ret = memset_s(di->freq_sample_info[idx].raw_eis_i, SIZE_OF_RAW_EIS, 0,
		SIZE_OF_RAW_EIS);
	if (ret != EOK) {
		hisi_eis_err("memset_s failed, ret = %d!\n", ret);
		return _ERROR;
	}
	return _SUCC;
}

static bool hisi_eis_cal_freq_intermediate_data_check(
	struct eis_freq_device *di, int idx)
{
	bool data_judge = false;

	data_judge = is_all_element_non_negative(
		di->freq_sample_info[idx].raw_eis_r, FREQ_SAMP_PRDS_10);
	if (data_judge == false) {
		di->freq_info[idx].real = CAL_ERR;
		hisi_eis_err("freq = 1000/%d, negative err, eis_real set to -1!\n",
			t_freq[idx]);
		return false;
	}
	return true;
}

static void hisi_eis_cal_freq_avg_val(struct eis_freq_device *di, int idx)
{
	sort(di->freq_sample_info[idx].raw_eis_r, FREQ_SAMP_PRDS_10,
		sizeof(long long), &_compare, NULL);
	sort(di->freq_sample_info[idx].raw_eis_i, FREQ_SAMP_PRDS_10,
		sizeof(long long), &_compare, NULL);
	/* for over 5 cases, sort them and get the avg of intermediate 5 */
	di->freq_info[idx].real = get_averaged_freq_eis_real(
		di->freq_sample_info[idx].raw_eis_r, FREQ_SAMP_PRDS_10);
	hisi_eis_inf("freq = 1000/%d, eis_real = %lld!\n",
		 t_freq[idx], di->freq_info[idx].real);
	di->freq_info[idx].imag = get_averaged_freq_eis_imag(
		di->freq_sample_info[idx].raw_eis_i, FREQ_SAMP_PRDS_10);
	hisi_eis_inf("freq = 1000/%d, eis_imag = %lld!\n",
		 t_freq[idx], di->freq_info[idx].imag);
}

static void hisi_eis_calculate_freq_func(struct eis_freq_device *di, int idx)
{
	unsigned int i, j;
	int v_gain;
	int ret;
	long long r_t, i_t;
	int v_tmp[V_BAT_FIFO_DEPTH];

	v_gain = di->freq_sample_info[idx].v_gain;
	if (v_gain <= 0) {
		hisi_eis_err("v_gain = %d, invalid!\n", v_gain);
		return;
	}
	hisi_eis_inf("v_gain = %d!\n", v_gain);

	ret = hisi_eis_init_freq_raw_to_zero(di, idx);
	if (ret == _ERROR) {
		hisi_eis_err("init freq raw to zero failed!\n");
		return;
	}
	for (i = 0; i < di->freq_sample_info[idx].sample_prds; i++) {
		if (di->freq_sample_info[idx].valid_prd_idx[i] == true) {
			(void)di->freq_ops->get_v_digit_data(
				di->freq_sample_info[idx].v_bat[i],
				V_BAT_FIFO_DEPTH, v_tmp, V_BAT_FIFO_DEPTH);
			r_t = 0;
			i_t = 0;
			for (j = 0; j < V_BAT_FIFO_DEPTH; j++) {
				r_t += k_real[j] / v_gain * v_tmp[j];
				i_t += k_imag[j] / v_gain * v_tmp[j];
			}
			di->freq_sample_info[idx].raw_eis_r[i] = r_t;
			hisi_eis_debug("idx%d %dth r_t = %lld\n", idx, i, r_t);
			di->freq_sample_info[idx].raw_eis_i[i] = i_t;
			hisi_eis_debug("idx%d %dth i_t = %lld\n", idx, i, i_t);
		} else {
			hisi_eis_err("idx%d invalid, set to 0\n", idx);
			di->freq_sample_info[idx].raw_eis_r[i] = 0;
			di->freq_sample_info[idx].raw_eis_i[i] = 0;
		}
	}
	if (hisi_eis_cal_freq_intermediate_data_check(di, idx) == false) {
		di->freq_info[idx].imag = CAL_ERR;
		hisi_eis_err("freq = 1000/%d, negative in eis_real, eis_imag set to -1!\n",
			t_freq[idx]);
		return;
	}
	hisi_eis_cal_freq_avg_val(di, idx);
}

static int eis_check_freq_valid(struct hisi_eis_device *eis_di, struct eis_freq_device *di)
{
	int i, ret;
	int fail_cnt = 0;

	for (i = 0; i < LEN_T_FREQ; i++) {
		hisi_eis_sampling_data_print(EIS_FREQ, eis_di, i);
		ret = hisi_eis_is_freq_sampling_valid(t_freq[i], di);
		if (ret == _SUCC) {
			hisi_eis_calculate_freq_func(di, i);
		} else {
			hisi_eis_err("freq = %d, invalid sampling, eis_imag and eis_real set to -1!\n",
				 t_freq[i]);
			di->freq_info[i].imag = CAL_ERR;
			di->freq_info[i].real = CAL_ERR;
			fail_cnt++;
		}
	}
	if (fail_cnt >= LEN_T_FREQ) {
		hisi_eis_err("all cal failed, not notify and save to flash!\n");
		eis_wake_unlock(g_eis_di);
		set_freq_work_flag(0);
		return _ERROR;
	}
	return _SUCC;
}

static void hisi_eis_freq_cal_impedance_work(struct work_struct *work)
{
	int ret;
	errno_t err;
	struct eis_freq_infos infos;
	struct hisi_eis_device *eis_di = NULL;
	struct eis_freq_device *di =
		container_of(work, struct eis_freq_device, freq_cal_work.work);

	eis_di = container_of(di, struct hisi_eis_device, eis_freq_dev);
	/* prevent system sleep when calculating eis_freq */
	eis_wake_lock(g_eis_di);
	if (eis_get_g_freq_rolling_now() == 0 && eis_get_g_full_freq_rolling() == 0)
		hisi_eis_freq_read_index_from_flash();
	if (hisi_eis_is_sampling_allowed(EIS_FREQ) == false) {
		hisi_eis_err("freq sampling condition not satisfied!\n");
		eis_wake_unlock(g_eis_di);
		set_freq_work_flag(0);
		return;
	}
	ret = hisi_eis_freq_sampling_work_exe(di); /* start sampling */
	if (ret != _SUCC) {
		hisi_eis_err("freq sampling execution failed!\n");
		eis_wake_unlock(g_eis_di);
		set_freq_work_flag(0);
		return;
	}
	if (eis_check_freq_valid(eis_di, di) == _ERROR)
		return;
	sysfs_notify(&g_eis_di->dev->kobj, NULL, "freq_raw_data");
	infos.num_of_info = LEN_T_FREQ;
	err = memcpy_s(infos.freq_infos, sizeof(struct eis_freq_info) * infos.num_of_info,
		di->freq_info, sizeof(di->freq_info));
	if (err != EOK)
		hisi_eis_err("eis freq memcpy_s failed!\n");
	hisi_call_eis_blocking_notifiers(EVT_EIS_FREQ_UPDATE, (void *)&infos);
	hisi_eis_inf("write to freq flash!\n");
	hisi_eis_freq_write_info_to_flash(di->freq_info);
	eis_wake_unlock(g_eis_di);
	set_freq_work_flag(0);
}

static int eis_freq_calculate_notifier_callback(
	struct notifier_block *freq_nb, unsigned long event, void *data)
{
	struct eis_freq_device *di = container_of(freq_nb,
		struct eis_freq_device, eis_freq_notify);
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null, invalid!\n");
		return NOTIFY_DONE;
	}
	hisi_eis_inf("event = %lu!\n", event);
	if (event == BATT_EIS_FREQ && g_eis_di != NULL) {
#ifdef CONFIG_BATT_SOH
		if (get_soh_core_device() != NULL) {
			mutex_lock(&get_soh_core_device()->soh_mutex);
			if (get_soh_core_device()->soh_acr_dev.acr_work_flag == 1 ||
				get_soh_core_device()->soh_dcr_dev.dcr_work_flag == 1) {
				mutex_unlock(&get_soh_core_device()->soh_mutex);
				hisi_eis_inf("acr or dcr is working!\n");
				return NOTIFY_DONE;
			}
			g_eis_di->eis_freq_dev.freq_work_flag = 1;
			mutex_unlock(&get_soh_core_device()->soh_mutex);
		} else {
			g_eis_di->eis_freq_dev.freq_work_flag = 1;
		}
#else
		g_eis_di->eis_freq_dev.freq_work_flag = 1;
#endif
		ret = queue_delayed_work(system_power_efficient_wq,
			&di->freq_cal_work, msecs_to_jiffies(0));
		hisi_eis_inf("ret of freq cal work = %d!\n", ret);
	}
	return NOTIFY_OK;
}

static int eis_freq_init(struct hisi_eis_device *di)
{
	int ret;

	if (di == NULL) {
		hisi_eis_err("di is null!\n");
		return _ERROR;
	}
	ret = parse_eis_freq_dts(di);
	if (ret == _ERROR) {
		hisi_eis_err("parse freq dts fail!\n");
		return _ERROR;
	}
	/* already parsed, but not support */
	if (di->eis_freq_dev.freq_support == 0) {
		hisi_eis_err("not support eis freq!\n");
		return _ERROR;
	}

	ret = eis_drv_ops_check(di, FREQ_DRV_OPS);
	if (ret != _SUCC) {
		hisi_eis_err("freq_drv_ops check fail!\n");
		return _ERROR;
	}
	ret = eis_drv_ops_check(di, EIS_OPS);
	if (ret != _SUCC) {
		hisi_eis_err("eis_core_ops check fail!\n");
		return _ERROR;
	}
	di->eis_freq_dev.eis_freq_notify.notifier_call =
		eis_freq_calculate_notifier_callback;
	ret = coul_register_blocking_notifier(
		&di->eis_freq_dev.eis_freq_notify);
	if (ret < 0) {
		hisi_eis_err("eis_freq_register_notifier failed!\n");
		return _ERROR;
	}
	/* init interruptable notifier work */
	INIT_DELAYED_WORK(&di->eis_freq_dev.freq_cal_work,
		hisi_eis_freq_cal_impedance_work);
	hisi_eis_inf("eis_freq init success\n");
	return _SUCC;
}

static void eis_freq_uninit(struct hisi_eis_device *di)
{
	(void)coul_unregister_blocking_notifier(
		&di->eis_freq_dev.eis_freq_notify);
	hisi_eis_inf("eis_freq uninit succ\n");
}

static int eis_probe(struct platform_device *pdev)
{
	int ret;
	struct hisi_eis_device *di = NULL;

	di = (struct hisi_eis_device *)devm_kzalloc(&pdev->dev, sizeof(*di),
		GFP_KERNEL);
	if (di == NULL)
		return -ENOMEM;

	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	mutex_init(&di->eis_mutex);
	mutex_init(&di->screen_update_lock);
	mutex_init(&di->eis_freq_dev.eis_freq_mutex);
	spin_lock_init(&di->sample_lock);
	/* init wakeup lock */
	wakeup_source_init(&di->eis_wake_lock, "eis_wakelock");
	hisi_eis_register_fb_chain_list(di);
	ret = eis_freq_init(di);
	if (ret == _ERROR) {
		hisi_eis_err("eis freq init fail!\n");
		goto eis_freq_fail;
	}

	ret = eis_create_sysfs_file(di);
	if (ret != 0) {
		hisi_eis_err("create sysfs fail!\n");
		goto sysfs_fail;
	}
	g_eis_di = di;
	hisi_eis_err("probe ok!\n");
	return 0;

sysfs_fail:
	eis_freq_uninit(di);
eis_freq_fail:
	hisi_eis_unregister_fb_chain_list(di);
	wakeup_source_trash(&di->eis_wake_lock);
	mutex_destroy(&di->eis_mutex);
	mutex_destroy(&di->screen_update_lock);
	mutex_destroy(&di->eis_freq_dev.eis_freq_mutex);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int eis_remove(struct platform_device *pdev)
{
	struct hisi_eis_device *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		hisi_eis_err(" null ptr di!\n");
		return -ENODEV;
	}

	eis_freq_uninit(di);
	hisi_eis_unregister_fb_chain_list(di);
	wakeup_source_trash(&di->eis_wake_lock);
	mutex_destroy(&di->eis_mutex);
	mutex_destroy(&di->screen_update_lock);
	mutex_destroy(&di->eis_freq_dev.eis_freq_mutex);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_eis_di = NULL;
	return 0;
}

static const struct of_device_id hisi_eis_core_match_table[] = {
	{
		.compatible = "hisi,eis_core",
	},
	{
	},
};

static void eis_shutdown(struct platform_device *pdev)
{
	struct hisi_eis_device *di = platform_get_drvdata(pdev);

	if (di == NULL)
		return;
	hisi_eis_err("ok!\n");
}

static struct platform_driver hisi_eis_core_driver = {
	.probe          = eis_probe,
	.remove         = eis_remove,
	.shutdown       = eis_shutdown,
	.driver         = {
		.name           = "hisi_eis_core",
		.owner          = THIS_MODULE,
		.of_match_table = hisi_eis_core_match_table,
	},
};

int __init eis_core_init(void)
{
	return platform_driver_register(&hisi_eis_core_driver);
}

void __exit eis_core_exit(void)
{
	platform_driver_unregister(&hisi_eis_core_driver);
}

late_initcall(eis_core_init);
module_exit(eis_core_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("eis core driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
