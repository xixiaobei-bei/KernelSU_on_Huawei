/*
 * menu.c - the menu idle governor
 *
 * Copyright (C) 2006-2007 Adam Belay <abelay@novell.com>
 * Copyright (C) 2009 Intel Corporation
 * Author:
 *        Arjan van de Ven <arjan@linux.intel.com>
 *
 * This code is licenced under the GPL version 2 as described
 * in the COPYING file that acompanies the Linux Kernel.
 */

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/cpuidle.h>
#include <linux/pm_qos.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/sched/stat.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
#define CREATE_TRACE_POINTS
#include <trace/events/cpuidle.h>
#endif

#define PREDICT_THRESHOLD   5000000 //in us
/*
 * Please note when changing the tuning values:
 * If (MAX_INTERESTING-1) * RESOLUTION > UINT_MAX, the result of
 * a scaling operation multiplication may overflow on 32 bit platforms.
 * In that case, #define RESOLUTION as ULL to get 64 bit result:
 * #define RESOLUTION 1024ULL
 *
 * The default values do not overflow.
 */
#define BUCKETS 12
#define INTERVAL_SHIFT 3
#define INTERVALS (1UL << INTERVAL_SHIFT)
#define RESOLUTION 64
#define DECAY 128
#define MAX_INTERESTING 50000

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
/*
 * CPU incorrectly goto deep C-state multi times in a short time.
 * In order to fix the problem, record the cpu idle history info.
 * the max num of history info is HIST_SIZE.
 * if more than HIST_FAILED_COUNT history record have less idle time
 * than the pre-select C-stat, reselect again.
 * The valid window width of history record is HIST_DATA_INV_GAP_US.
 * Earlier record not be used.
 */
#define HIST_SIZE 8
#define HIST_FAILED_COUNT 2
#define HIST_DATA_INV_GAP_US 20000UL

#define HIST_TASK_RUNTIME_US 1000UL
#define HIST_MIN_RESIDENCY_US 4000UL
#define HIST_RESIDENCY_US 500UL

#define HIST_INV_PREDICT_CN 2
#define HIST_INV_REPEAT_CN 2

#define MENU_HRTIMER_DELTA_TIME_US	500UL
#define MENU_HRTIMER_MIN_TIME_US	4000UL
#define MENU_HRTIMER_MAX_TIME_US	20000UL

#ifdef CONFIG_ARCH_HAS_CPU_RELAX
#define CPUIDLE_DRIVER_STATE_START	1
#else
#define CPUIDLE_DRIVER_STATE_START	0
#endif
#endif

/*
 * Concepts and ideas behind the menu governor
 *
 * For the menu governor, there are 3 decision factors for picking a C
 * state:
 * 1) Energy break even point
 * 2) Performance impact
 * 3) Latency tolerance (from pmqos infrastructure)
 * These these three factors are treated independently.
 *
 * Energy break even point
 * -----------------------
 * C state entry and exit have an energy cost, and a certain amount of time in
 * the  C state is required to actually break even on this cost. CPUIDLE
 * provides us this duration in the "target_residency" field. So all that we
 * need is a good prediction of how long we'll be idle. Like the traditional
 * menu governor, we start with the actual known "next timer event" time.
 *
 * Since there are other source of wakeups (interrupts for example) than
 * the next timer event, this estimation is rather optimistic. To get a
 * more realistic estimate, a correction factor is applied to the estimate,
 * that is based on historic behavior. For example, if in the past the actual
 * duration always was 50% of the next timer tick, the correction factor will
 * be 0.5.
 *
 * menu uses a running average for this correction factor, however it uses a
 * set of factors, not just a single factor. This stems from the realization
 * that the ratio is dependent on the order of magnitude of the expected
 * duration; if we expect 500 milliseconds of idle time the likelihood of
 * getting an interrupt very early is much higher than if we expect 50 micro
 * seconds of idle time. A second independent factor that has big impact on
 * the actual factor is if there is (disk) IO outstanding or not.
 * (as a special twist, we consider every sleep longer than 50 milliseconds
 * as perfect; there are no power gains for sleeping longer than this)
 *
 * For these two reasons we keep an array of 12 independent factors, that gets
 * indexed based on the magnitude of the expected duration as well as the
 * "is IO outstanding" property.
 *
 * Repeatable-interval-detector
 * ----------------------------
 * There are some cases where "next timer" is a completely unusable predictor:
 * Those cases where the interval is fixed, for example due to hardware
 * interrupt mitigation, but also due to fixed transfer rate devices such as
 * mice.
 * For this, we use a different predictor: We track the duration of the last 8
 * intervals and if the stand deviation of these 8 intervals is below a
 * threshold value, we use the average of these intervals as prediction.
 *
 * Limiting Performance Impact
 * ---------------------------
 * C states, especially those with large exit latencies, can have a real
 * noticeable impact on workloads, which is not acceptable for most sysadmins,
 * and in addition, less performance has a power price of its own.
 *
 * As a general rule of thumb, menu assumes that the following heuristic
 * holds:
 *     The busier the system, the less impact of C states is acceptable
 *
 * This rule-of-thumb is implemented using a performance-multiplier:
 * If the exit latency times the performance multiplier is longer than
 * the predicted duration, the C state is not considered a candidate
 * for selection due to a too high performance impact. So the higher
 * this multiplier is, the longer we need to be idle to pick a deep C
 * state, and thus the less likely a busy CPU will hit such a deep
 * C state.
 *
 * Two factors are used in determing this multiplier:
 * a value of 10 is added for each point of "per cpu load average" we have.
 * a value of 5 points is added for each process that is waiting for
 * IO on this CPU.
 * (these values are experimentally determined)
 *
 * The load average factor gives a longer term (few seconds) input to the
 * decision, while the iowait value gives a cpu local instantanious input.
 * The iowait factor may look low, but realize that this is also already
 * represented in the system load average.
 *
 */
struct menu_device {
	int		last_state_idx;
	int             needs_update;
	int             tick_wakeup;

	unsigned int	next_timer_us;
	unsigned int	predicted_us;
	unsigned int	bucket;
	unsigned int	correction_factor[BUCKETS];
	unsigned int	intervals[INTERVALS];
	int		interval_ptr;

	unsigned int	repeat;

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	int		org_state_idx;
	int		hist_inv_flag;
	int		hist_inv_repeat;
	int		hist_inv_repeat_old;
	int		hist_inv_predict;
	int		hist_ptr;
	struct menu_hist_cstate_data hist_data[HIST_SIZE];

	unsigned int	hrtime_us;
	unsigned int	hrtime_out;
	unsigned int	hrtime_addtime;
#endif

#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
	unsigned int	deep_state_num;
	unsigned int	last_max_interval_us;
	unsigned int	resel_deep_state;
#endif
};

#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
#define SKIP_DEEP_CSTATE_THD_NUM (8)
#define SKIP_DEEP_CSTATE_TOP_NUM (8 + 4)
#define SKIP_DEEP_CSTATE_MAX_INTERVAL_US (2000)
#endif

#define BIT(nr) (1UL << (nr))

/* 60 * 60 > STDDEV_THRESH * INTERVALS = 400 * 8 */
#define MAX_DEVIATION 60
static DEFINE_PER_CPU(int, hrtimer_status); //lint !e129
/*lint -e528 -esym(528,*)*/
static DEFINE_PER_CPU(struct hrtimer, menu_hrtimer);
/*lint -e528 +esym(528,*)*/
static unsigned int menu_switch_profile __read_mostly;
static struct cpumask menu_cpumask;

/* menu hrtimer mode */
enum {
	MENU_HRTIMER_STOP,
	MENU_HRTIMER_REPEAT,
	MENU_HRTIMER_GENERAL,
#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	MENU_HRTIMER_INV,
#endif
};

static unsigned int perfect_cstate_ms __read_mostly = 30;
module_param(perfect_cstate_ms, uint, 0000);
static unsigned int menu_hrtimer_enable __read_mostly;

/*add cpufreq notify block for dvfs target profile */
/*lint -e715*/
static int menu_cpufreq_callback(struct notifier_block *nb,
				       unsigned long event, void *data)
{
	struct cpufreq_freqs *freq = data;

	if (!cpumask_test_cpu(freq->cpu, &menu_cpumask))
		return 0;
	if (event != CPUFREQ_POSTCHANGE)
		return 0;
	if (menu_switch_profile <= freq->new)
		menu_hrtimer_enable = 1;
	else
		menu_hrtimer_enable = 0;
	return 0;
}
/*lint +e715*/
/*lint -e785*/
static struct notifier_block menu_cpufreq_notifier = {
	.notifier_call  = menu_cpufreq_callback,
};
/*lint +e785*/
static int __init register_menu_cpufreq_notifier(void)
{
	int ret;

	ret = cpufreq_register_notifier(&menu_cpufreq_notifier,
		CPUFREQ_TRANSITION_NOTIFIER);
	return ret;
}
/*lint -e64 -e507 -e530 */
/* Cancel the hrtimer if it is not triggered yet */
void menu_hrtimer_cancel(void)
{
	unsigned int cpu = smp_processor_id();
	struct hrtimer *hrtmr = &per_cpu(menu_hrtimer, cpu);

	/* The timer is still not time out*/
	if (per_cpu(hrtimer_status, cpu)) {
		hrtimer_cancel(hrtmr);
		per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;
	}
}
EXPORT_SYMBOL_GPL(menu_hrtimer_cancel);

static DEFINE_PER_CPU(struct menu_device, menu_devices);

/* Call back for hrtimer is triggered */
static enum hrtimer_restart menu_hrtimer_notify(struct hrtimer *phrtimer)
{
	unsigned int cpu = smp_processor_id();
	struct menu_device *data = &per_cpu(menu_devices, cpu);
#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	int wakeup_status;
#endif

	if (!phrtimer)
		return HRTIMER_NORESTART;

	/* In general case, the expected residency is much larger than
	 *  deepest C-state target residency, but prediction logic still
	 *  predicts a small predicted residency, so the prediction
	 *  history is totally broken if the timer is triggered.
	 *  So reset the correction factor.
	 */

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	wakeup_status = per_cpu(hrtimer_status, cpu);
	per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;

	if (wakeup_status == MENU_HRTIMER_GENERAL)
		data->correction_factor[data->bucket] = RESOLUTION * DECAY;
	else if (wakeup_status == MENU_HRTIMER_INV)
		data->hist_inv_flag = true;
	else if (wakeup_status == MENU_HRTIMER_REPEAT)
		data->hist_inv_repeat++;
	else
		return HRTIMER_NORESTART;

	data->hrtime_out = true;

	trace_menu_hrtimerout(cpu, wakeup_status, data->hrtime_us);
#else
	if (per_cpu(hrtimer_status, cpu) == MENU_HRTIMER_GENERAL)
		data->correction_factor[data->bucket] = RESOLUTION * DECAY;
	per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;
#endif

	return HRTIMER_NORESTART;
}
/*lint +e64 +e507 +e530 */
static inline int get_loadavg(unsigned long load)
{
	return LOAD_INT(load) * 10 + LOAD_FRAC(load) / 10;
}

static inline int which_bucket(unsigned int duration, unsigned long nr_iowaiters)
{
	int bucket = 0;

	/*
	 * We keep two groups of stats; one with no
	 * IO pending, one without.
	 * This allows us to calculate
	 * E(duration)|iowait
	 */
	if (nr_iowaiters)
		bucket = BUCKETS/2;

	if (duration < 10)
		return bucket;
	if (duration < 100)
		return bucket + 1;
	if (duration < 1000)
		return bucket + 2;
	if (duration < 10000)
		return bucket + 3;
	if (duration < 100000)
		return bucket + 4;
	return bucket + 5;
}
/*
 * Return a multiplier for the exit latency that is intended
 * to take performance requirements into account.
 * The more performance critical we estimate the system
 * to be, the higher this multiplier, and thus the higher
 * the barrier to go to an expensive C state.
 */
static inline int performance_multiplier(unsigned long nr_iowaiters, unsigned long load)
{
	int mult = 1;

	/* for higher loadavg, we are more reluctant */

	/*
	 * this doesn't work as intended - it is almost always 0, but can
	 * sometimes, depending on workload, spike very high into the hundreds
	 * even when the average cpu load is under 10%.
	 */
	/* mult += 2 * get_loadavg(); */

	/* for IO wait tasks (per cpu!) we add 5x each */
	mult += 10 * nr_iowaiters;

	return mult;
} /*lint !e715*/

static DEFINE_PER_CPU(struct menu_device, menu_devices);

static void menu_update(struct cpuidle_driver *drv, struct cpuidle_device *dev);

/*
 * Try detecting repeating patterns by keeping track of the last 8
 * intervals, and checking if the standard deviation of that set
 * of points is below a threshold. If it is... then use the
 * average of these 8 points as the estimated value.
 */
static unsigned int get_typical_interval(struct menu_device *data)
{
	int i, divisor;
	unsigned int max, thresh, avg;
	uint64_t sum, variance;

	thresh = UINT_MAX; /* Discard outliers above this value */

again:

	/* First calculate the average of past intervals */
	max = 0;
	sum = 0;
	divisor = 0;
	for (i = 0; i < INTERVALS; i++) {
		unsigned int value = data->intervals[i];
		if (value <= thresh) {
			sum += value;
			divisor++;
			if (value > max)
				max = value;
		}
	}
	if (divisor == INTERVALS)
		avg = sum >> INTERVAL_SHIFT;
	else
		avg = div_u64(sum, divisor);

#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
	if (thresh == UINT_MAX)
		data->last_max_interval_us = max;
#endif

	/* Then try to determine variance */
	variance = 0;
	for (i = 0; i < INTERVALS; i++) {
		unsigned int value = data->intervals[i];
		if (value <= thresh) {
			int64_t diff = (int64_t)value - avg;
			variance += diff * diff;
		}
	}
	if (divisor == INTERVALS)
		variance >>= INTERVAL_SHIFT;
	else
		do_div(variance, divisor);

	/*
	 * The typical interval is obtained when standard deviation is
	 * small (stddev <= 20 us, variance <= 400 us^2) or standard
	 * deviation is small compared to the average interval (avg >
	 * 6*stddev, avg^2 > 36*variance). The average is smaller than
	 * UINT_MAX aka U32_MAX, so computing its square does not
	 * overflow a u64. We simply reject this candidate average if
	 * the standard deviation is greater than 715 s (which is
	 * rather unlikely).
	 *
	 * Use this result only if there is no timer to wake us up sooner.
	 */
	if (likely(variance <= U64_MAX / 36)) {
		if ((((u64)avg * avg > variance * 36) && (divisor * 4 >= INTERVALS * 3))
							|| variance <= 400) {
			/* if the avg is beyond the known next tick, it's worthless */
			if (avg > data->next_timer_us)
				data->repeat = 0;
			else
				data->repeat = 1;

			return avg;
		}
	}

	/*
	 * If we have outliers to the upside in our distribution, discard
	 * those by setting the threshold to exclude these outliers, then
	 * calculate the average and standard deviation again. Once we get
	 * down to the bottom 3/4 of our samples, stop excluding samples.
	 *
	 * This can deal with workloads that have long pauses interspersed
	 * with sporadic activity with a bunch of short pauses.
	 */
	if ((divisor * 4) <= INTERVALS * 3) {
		data->repeat = 0;
		return UINT_MAX;
	}

	thresh = max - 1;
	goto again;
}

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
/**
 * menu_hist_calc_stat_info - statistics the history idle info for
 *    reselect idle stat.including:
 *    a) number and time length of same pre-select idle state
 *       with less idle time than residency_thresh.
 *    b) busy status info, as total running time/ idle time, last run time.
 * @residency_thresh: residency threshold for select excessive deep idle state.
 * @stat_info: the result of history idle state info
 */
static void menu_hist_calc_stat_info(unsigned int residency_thresh,
			struct menu_hist_state_info *stat_info)
{
	int i;
	int hist_ptr;
	unsigned int min_residency_us = HIST_DATA_INV_GAP_US;
	s64 stime;
	s64 enter_time;
	s64 first_enter_time;
	s64 last_exit_time = 0;
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	struct menu_hist_cstate_data *hist_data;

	stime = ktime_to_us(ktime_get());
	first_enter_time = stime;
	hist_ptr = data->hist_ptr;

	for (i = 0 ; i < HIST_SIZE ; i++) {
		hist_ptr--;
		if (hist_ptr < 0)
			hist_ptr += HIST_SIZE;
		hist_data = &data->hist_data[hist_ptr];

		if (hist_data->residency_us == 0)
			continue;

		/* calc last exit time for last run time. */
		if (last_exit_time < hist_data->exit_time)
			last_exit_time = hist_data->exit_time;
		if (min_residency_us > hist_data->residency_us)
			min_residency_us = hist_data->residency_us;

		enter_time = hist_data->exit_time - hist_data->residency_us;
		if (enter_time < 0)
			continue;
		if (enter_time + HIST_DATA_INV_GAP_US < stime)
			continue;

		stat_info->total_count++;
		stat_info->total_residency_us += hist_data->residency_us;

		if (first_enter_time > enter_time)
			first_enter_time = enter_time;

		if (hist_data->residency_us < residency_thresh) {
			if (hist_data->org_state_idx == data->last_state_idx) {
				stat_info->same_state_failed_count++;
				stat_info->same_state_failed_us +=
					hist_data->residency_us;
			}
		}

		trace_menu_hist_data(hist_ptr, hist_data);
	}

	stat_info->total_us =
		min_t(u64, HIST_DATA_INV_GAP_US, stime - first_enter_time);
	stat_info->total_residency_us =
		min_t(u64, HIST_DATA_INV_GAP_US, stat_info->total_residency_us);

	stat_info->last_run_us = stime - last_exit_time;
	stat_info->min_residency_us = min_residency_us;

	trace_menu_hist_info(stat_info, residency_thresh, stime);
}

static int menu_hist_sel_state_idx(struct cpuidle_driver *drv,
			struct cpuidle_device *dev,
			unsigned int target_residency, int exit_latency)
{
	int i;
	int last_state_idx = CPUIDLE_DRIVER_STATE_START;

	/* Use history data to choice new idle state*/
	for (i = CPUIDLE_DRIVER_STATE_START + 1; i < drv->state_count; i++) {
		struct cpuidle_state_usage *lu = &dev->states_usage[i];
		struct cpuidle_state *l = &drv->states[i];

		if (l->disabled || lu->disable)
			continue;

		if (l->target_residency > target_residency)
			continue;

		if (l->exit_latency > exit_latency)/*lint !e574*/
			continue;

		last_state_idx = i;
	}

	return last_state_idx;
}

/**
 * menu_hist_get_busy_flag - get cpu busy status
 * @stat_info: the statistics info of history idle
 */
static int menu_hist_get_busy_flag(struct menu_hist_state_info *stat_info)
{
	/*
	 * the last task has a long running time. and
	 * the min idle time is less than threshold.
	 */
	if (stat_info->last_run_us > HIST_TASK_RUNTIME_US &&
	    stat_info->min_residency_us < HIST_MIN_RESIDENCY_US)
		return 1;

	/*
	 * there are multi idle history records which has short idle time.
	 */
	if (stat_info->total_count > HIST_SIZE / 2 &&
	    stat_info->total_residency_us <
	    stat_info->total_count * HIST_RESIDENCY_US)
		return 1;

	/*
	 * task is busy, do not goto deep sleep.
	 */
	if (stat_info->total_residency_us + HIST_DATA_INV_GAP_US / 2 <
	    stat_info->total_us ||
	    stat_info->total_residency_us * 3 < stat_info->total_us)
		return 1;

	return 0;
}

/**
 * menu_hist_busy_resel - reslect idle state by busy status.
 * @stat_info: the statistics info of history idle
 */
static inline void menu_hist_busy_resel(struct menu_device *data,
			struct menu_hist_state_info *stat_info)
{
	int hist_busy_flag;

	hist_busy_flag = menu_hist_get_busy_flag(stat_info);
	if (!hist_busy_flag)
		return;

	if (data->last_state_idx < 1)
		return;
	data->last_state_idx--;

	if (stat_info->total_count) {
		do_div(stat_info->total_residency_us, stat_info->total_count);
		data->predicted_us = min_t(unsigned int, data->predicted_us,
			stat_info->total_residency_us);
	} else {
		data->predicted_us = min_t(unsigned int, data->predicted_us,
			MENU_HRTIMER_MIN_TIME_US);
	}
}

/**
 * menu_hist_check_state_idx - check state idx enable or disable
 * @drv: cpuidle driver containing state data
 * @dev: cpuidle_device
 * @state_idx: cpu idle state idx
 * return: 1: enable; 0: disabled
 */
static int menu_hist_check_state_idx(struct cpuidle_driver *drv,
			struct cpuidle_device *dev, int state_idx)
{
	if (state_idx >= drv->state_count)
		return 0;

	if (drv->states[state_idx].disabled ||
	    dev->states_usage[state_idx].disable)
		return 0;

	return 1;
}

/**
 * menu_hist_reselect - reselects the next idle state to enter
 *    with history idle info
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 * @latency_req: latency qos request
 */
static void menu_hist_reselect(struct cpuidle_driver *drv,
			struct cpuidle_device *dev, int latency_req)
{
	unsigned int failed_cnt_th = HIST_FAILED_COUNT;
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	data->org_state_idx = data->last_state_idx;

	if (data->last_state_idx > 0 &&
	    cpumask_test_cpu((int)dev->cpu, &menu_cpumask) &&
	    data->hist_inv_repeat  < HIST_INV_REPEAT_CN &&
	    data->hist_inv_predict < HIST_INV_PREDICT_CN) {
		struct menu_hist_state_info stat_info = {0};
		struct cpuidle_state *s = &drv->states[data->last_state_idx];

		menu_hist_calc_stat_info(
			(s->target_residency + s->exit_latency) / 2, &stat_info);

		if (data->last_state_idx + 1 == drv->state_count)
			failed_cnt_th++;

		if (stat_info.same_state_failed_count >= failed_cnt_th) {
			do_div(stat_info.same_state_failed_us,
				stat_info.same_state_failed_count);

			data->predicted_us = min_t(unsigned int, data->predicted_us,
				stat_info.same_state_failed_us);

			data->last_state_idx = menu_hist_sel_state_idx(drv, dev,
				data->predicted_us, latency_req);
		} else if (data->last_state_idx + 1 == drv->state_count) {
			menu_hist_busy_resel(data, &stat_info);
		}
	}

	if (data->hist_inv_predict == 1 &&
	    data->last_state_idx < (drv->state_count - 1)) {
		data->last_state_idx++;

		if (data->last_state_idx < data->org_state_idx)
			data->last_state_idx++;
	}

	if (data->org_state_idx != data->last_state_idx) {
		if (!menu_hist_check_state_idx(drv, dev, data->last_state_idx))
			data->last_state_idx = data->org_state_idx;
	}

	if (data->hist_inv_predict >= HIST_INV_PREDICT_CN)
		data->hist_inv_predict = 0;

	if (data->hist_inv_repeat >= HIST_INV_REPEAT_CN)
		data->hist_inv_repeat = 0;
}


/**
 * menu_sel_hrtimer_algo - calc the hrtime length and type.
 * @dev: the CPU
 * @hrtimer_type: latency qos request
 */
static unsigned int menu_sel_hrtimer_algo(struct menu_device *data,
				struct cpuidle_driver *drv, int *hrtimer_type)
{
	unsigned int hrtime_us = 0;
	unsigned int last_time_us;
	unsigned int next_time_us;

	struct cpuidle_state *s_next = NULL;
	struct cpuidle_state *s_last = NULL;

	if (data->last_state_idx + 1 >= drv->state_count)
		return 0;

	s_last = &drv->states[data->last_state_idx]; /*lint !e679*/
	s_next = &drv->states[data->last_state_idx + 1]; /*lint !e679*/

	last_time_us = s_last->target_residency + s_last->exit_latency;
	next_time_us = s_next->target_residency + s_next->exit_latency;

	/* Avoid new predictions result cause shallow sleep */
	if (data->last_state_idx != data->org_state_idx &&
	    data->next_timer_us > last_time_us + next_time_us) {
		hrtime_us = data->predicted_us + MENU_HRTIMER_DELTA_TIME_US;
		hrtime_us = max(hrtime_us, s_next->target_residency / 2);
		hrtime_us = min(hrtime_us, s_next->target_residency);
		hrtime_us = max(hrtime_us, s_last->target_residency);

		hrtime_us = max_t(unsigned int, hrtime_us, MENU_HRTIMER_MIN_TIME_US);
		hrtime_us = min_t(unsigned int, hrtime_us, MENU_HRTIMER_MAX_TIME_US);

		*hrtimer_type = MENU_HRTIMER_INV;
		return hrtime_us;
	}

	/* Avoid error prediction cause shallow sleep */
	if (data->next_timer_us > perfect_cstate_ms * USEC_PER_MSEC ||
	    data->next_timer_us > data->predicted_us + next_time_us) {
		hrtime_us = data->predicted_us + MENU_HRTIMER_DELTA_TIME_US;
		hrtime_us = max(hrtime_us, s_next->target_residency / 2);
		hrtime_us = min(hrtime_us, s_next->target_residency);
		hrtime_us = max(hrtime_us, s_last->target_residency);

		hrtime_us = max_t(unsigned int, hrtime_us, MENU_HRTIMER_MIN_TIME_US);
		hrtime_us = min_t(unsigned int, hrtime_us, MENU_HRTIMER_MAX_TIME_US);

		if (data->repeat)
			*hrtimer_type = MENU_HRTIMER_REPEAT;
		else
			*hrtimer_type = MENU_HRTIMER_GENERAL;

		return hrtime_us;
	}

	return 0;
}

static void menu_sel_hrtimer_start(struct cpuidle_driver *drv,
			struct cpuidle_device *dev)
{
	unsigned int cpu = dev->cpu;
	struct hrtimer *ek_hrtimer = NULL;
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	int hrtimer_type = MENU_HRTIMER_STOP;

	data->hrtime_us = 0;
	if (menu_hrtimer_enable &&
	    cpumask_test_cpu((int)cpu, &menu_cpumask) &&
	    data->last_state_idx + 1 < drv->state_count) {
		data->hrtime_us = menu_sel_hrtimer_algo(data,
						drv, &hrtimer_type);
		per_cpu(hrtimer_status, cpu) = hrtimer_type;

		if (data->hrtime_us) {
			ek_hrtimer = &per_cpu(menu_hrtimer, cpu);
			hrtimer_start(ek_hrtimer,
				ns_to_ktime(data->hrtime_us * NSEC_PER_USEC),
				HRTIMER_MODE_REL_PINNED);
		}
	}
}

static inline void menu_hist_update_inv_predict(struct menu_device *data)
{
	if (data->hist_inv_flag) {
		if (data->hist_inv_predict < HIST_INV_PREDICT_CN)
			data->hist_inv_predict++;
	} else {
		data->hist_inv_predict = 0;
	}
}

static inline void menu_hist_update_interval_ptr(struct menu_device *data)
{
	data->interval_ptr++;
	if (data->interval_ptr >= INTERVALS)
		data->interval_ptr = 0;

}

static inline void menu_hist_update_hist_ptr(struct menu_device *data)
{
	data->hist_ptr++;
	if (data->hist_ptr >= HIST_SIZE)
		data->hist_ptr = 0;
}

static inline void menu_hist_update_inv_repeat(struct menu_device *data)
{
	if (data->hist_inv_repeat > 0 &&
	    data->hist_inv_repeat_old == data->hist_inv_repeat)
		data->hist_inv_repeat--;

	data->hist_inv_repeat_old = data->hist_inv_repeat;
}

/**
 * menu_hist_update - update history info
 * @measured_us: last idle time length
 */
static void menu_hist_update(struct menu_device *data, unsigned int measured_us)
{
	if (data->hrtime_addtime) {
		data->intervals[data->interval_ptr] += measured_us;
		data->hist_data[data->hist_ptr].residency_us +=
			measured_us;
	} else {
		data->intervals[data->interval_ptr] = measured_us;
		data->hist_data[data->hist_ptr].residency_us =
			measured_us;
	}

	/* Update history data */
	menu_hist_update_inv_predict(data);

	trace_menu_update(data->hist_data[data->hist_ptr].residency_us,
		data->hist_data[data->hist_ptr].idle_state_idx,
		data->hist_data[data->hist_ptr].exit_time,
		data->hist_inv_flag,
		data->hist_inv_repeat,
		data->hist_inv_predict,
		data->hrtime_addtime);

	data->hist_inv_flag = false;
	data->hrtime_addtime = false;

	if (data->hrtime_out) {
		/*
		 * menu gov's hrtime call up core, set flag to add the next idle time.
		 * the sum of last idle and next idle is real idle time.
		 */
		data->hrtime_out = false;
		data->hrtime_addtime = true;
	} else {
		menu_hist_update_interval_ptr(data);

		menu_hist_update_hist_ptr(data);
	}

	menu_hist_update_inv_repeat(data);
}
#endif /* CONFIG_HISI_CPUIDLE_MENU_GOV_HIST */

/**
 * menu_select - selects the next idle state to enter
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 * @stop_tick: indication on whether or not to stop the tick
 */
static int menu_select(struct cpuidle_driver *drv, struct cpuidle_device *dev,
		       bool *stop_tick)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	struct device *device = get_cpu_device(dev->cpu);
	int latency_req = pm_qos_request(PM_QOS_CPU_DMA_LATENCY);
	int i;
	int first_idx;
	int idx;
#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
	int pre_idx = -1;
#endif
	unsigned int interactivity_req;
	unsigned int expected_interval;
	unsigned long nr_iowaiters, cpu_load;
	int low_predicted = 0;
	unsigned int cpu = dev->cpu;
#ifndef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	unsigned int timer_us = 0;
	unsigned int perfect_us = 0;
	struct hrtimer *hrtmr = &per_cpu(menu_hrtimer, cpu);
#endif
	int resume_latency = dev_pm_qos_raw_read_value(device);
	ktime_t delta_next;

	if (data->needs_update) {
		menu_update(drv, dev);
		data->needs_update = 0;
	}

	/* resume_latency is 0 means no restriction */
	if (resume_latency && resume_latency < latency_req)
		latency_req = resume_latency;

	/* Special case when user has set very strict latency requirement */
	if (unlikely(latency_req == 0)) {
		*stop_tick = false;
#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
		data->org_state_idx  = 0;
		data->last_state_idx = 0;
		data->hist_inv_predict = 0;
		data->hist_inv_repeat = 0;
#endif
		return 0;
	}

	/* determine the expected residency time, round up */
	data->next_timer_us = ktime_to_us(tick_nohz_get_sleep_length(&delta_next));

	get_iowait_load(&nr_iowaiters, &cpu_load);
	data->bucket = which_bucket(data->next_timer_us, nr_iowaiters);

	/*
	 * Force the result of multiplication to be 64 bits even if both
	 * operands are 32 bits.
	 * Make sure to round up for half microseconds.
	 */
	data->predicted_us = DIV_ROUND_CLOSEST_ULL((uint64_t)data->next_timer_us *
					 data->correction_factor[data->bucket],
					 RESOLUTION * DECAY);

	data->repeat = 0;
	expected_interval = get_typical_interval(data);
	expected_interval = min(expected_interval, data->next_timer_us);

	first_idx = 0;
	if (drv->states[0].flags & CPUIDLE_FLAG_POLLING) {
		struct cpuidle_state *s = &drv->states[1];
		unsigned int polling_threshold;

		/*
		 * We want to default to C1 (hlt), not to busy polling
		 * unless the timer is happening really really soon, or
		 * C1's exit latency exceeds the user configured limit.
		 */
		polling_threshold = max_t(unsigned int, 20, s->target_residency);
		if (data->next_timer_us > polling_threshold &&
		    latency_req > s->exit_latency && !s->disabled &&
		    !dev->states_usage[1].disable)
			first_idx = 1;
	}

	/*
	 * Use the lowest expected idle interval to pick the idle state.
	 */
	data->predicted_us = min(data->predicted_us, expected_interval);

	/*
	 * We disable the predict when the next timer is too long,
	 * so that it'll not stay in a light C state for a long time after
	 * a wrong predict.
	 */
	if (data->next_timer_us > PREDICT_THRESHOLD)
		data->predicted_us = data->next_timer_us;

	if (tick_nohz_tick_stopped()) {
		/*
		 * If the tick is already stopped, the cost of possible short
		 * idle duration misprediction is much higher, because the CPU
		 * may be stuck in a shallow idle state for a long time as a
		 * result of it.  In that case say we might mispredict and try
		 * to force the CPU into a state for which we would have stopped
		 * the tick, unless a timer is going to expire really soon
		 * anyway.
		 */
		if (data->predicted_us < TICK_USEC)
			data->predicted_us = min_t(unsigned int, TICK_USEC,
						   ktime_to_us(delta_next));
	} else {
		/*
		 * Use the performance multiplier and the user-configurable
		 * latency_req to determine the maximum exit latency.
		 */
		interactivity_req = data->predicted_us / performance_multiplier(nr_iowaiters, cpu_load);
		if (latency_req > interactivity_req)
			latency_req = interactivity_req;
	}

	expected_interval = data->predicted_us;
	/*
	 * Find the idle state with the lowest power while satisfying
	 * our constraints.
	 */
	idx = -1;
	for (i = first_idx; i < drv->state_count; i++) {
		struct cpuidle_state *s = &drv->states[i];
		struct cpuidle_state_usage *su = &dev->states_usage[i];

		if (s->disabled || su->disable)
			continue;
#ifdef CONFIG_HISI_CPU_ISOLATION
		if (cpu_isolated(cpu)) {
#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
			pre_idx = idx;
#endif
			idx = i;
			continue;
		}
#endif
		if (idx == -1)
			idx = i; /* first enabled state */
		if (s->target_residency > data->predicted_us) {
			low_predicted = 1;
			break;
		}
		if (s->exit_latency > latency_req) {
			/*
			 * If we break out of the loop for latency reasons, use
			 * the target residency of the selected state as the
			 * expected idle duration so that the tick is retained
			 * as long as that target residency is low enough.
			 */
			expected_interval = drv->states[idx].target_residency;
			break;
		}
#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
		pre_idx = idx;
#endif
		idx = i;
	}

#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
	if (idx + 1 == drv->state_count &&
		!hisi_test_slow_cpu(cpu) &&
		data->deep_state_num > SKIP_DEEP_CSTATE_THD_NUM &&
		data->last_max_interval_us < SKIP_DEEP_CSTATE_MAX_INTERVAL_US) {
		data->resel_deep_state = 1;
		idx = pre_idx;
	} else {
		data->resel_deep_state = 0;
	}
#endif

	if (idx == -1)
		idx = 0; /* No states enabled. Must use 0. */

#ifdef CONFIG_HISI_CPU_ISOLATION
	if (cpu_isolated(cpu)) {
		data->last_state_idx = idx;
		return data->last_state_idx;
	}
#endif

	/*
	 * Don't stop the tick if the selected state is a polling one or if the
	 * expected idle duration is shorter than the tick period length.
	 */
	if ((drv->states[idx].flags & CPUIDLE_FLAG_POLLING) ||
	    expected_interval < TICK_USEC) {
		unsigned int delta_next_us = ktime_to_us(delta_next);

		*stop_tick = false;

		if (!tick_nohz_tick_stopped() && idx > 0 &&
		    drv->states[idx].target_residency > delta_next_us) {
			/*
			 * The tick is not going to be stopped and the target
			 * residency of the state to be returned is not within
			 * the time until the next timer event including the
			 * tick, so try to correct that.
			 */
			for (i = idx - 1; i >= 0; i--) {
				if (drv->states[i].disabled ||
				    dev->states_usage[i].disable)
					continue;

				idx = i;
				if (drv->states[i].target_residency <= delta_next_us)
					break;
			}
		}
	}

	data->last_state_idx = idx;

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	menu_hist_reselect(drv, dev, latency_req);

	menu_sel_hrtimer_start(drv, dev);

	trace_menu_select(dev->cpu,
		data->last_state_idx,
		data->predicted_us,
		latency_req,
		per_cpu(hrtimer_status, cpu),
		data->org_state_idx,
		data->next_timer_us,
		data->hrtime_us,
		ktime_to_us(delta_next),
		expected_interval);
#else
	if ((menu_hrtimer_enable) && (low_predicted) && (cpumask_test_cpu((int)cpu, &menu_cpumask))) {
		/*
		 * Set a timer to detect whether this sleep is much
		 * longer than repeat mode predicted.  If the timer
		 * triggers, the code will evaluate whether to put
		 * the CPU into a deeper C-state.
		 * The timer is cancelled on CPU wakeup.
		 */
		timer_us = 5 * (data->predicted_us + MAX_DEVIATION);
		perfect_us = perfect_cstate_ms * 1000;

		if (data->repeat && (4 * timer_us < data->next_timer_us)) {
			hrtimer_start(hrtmr, ns_to_ktime(1000 * (unsigned long)timer_us),
				HRTIMER_MODE_REL_PINNED);
			/* In repeat case, menu hrtimer is started */
			per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_REPEAT;
		} else if (perfect_us < data->next_timer_us) {
			/*
			 * The next timer is long. This could be because
			 * we did not make a useful prediction.
			 * In that case, it makes sense to re-enter
			 * into a deeper C-state after some time.
			 */
			hrtimer_start(hrtmr, ns_to_ktime(1000 * (unsigned long)timer_us),
				HRTIMER_MODE_REL_PINNED);
			/* In general case, menu hrtimer is started */
			per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_GENERAL;
		}
	}
#endif
	return data->last_state_idx;
}

/**
 * menu_reflect - records that data structures need update
 * @dev: the CPU
 * @index: the index of actual entered state
 *
 * NOTE: it's important to be fast here because this operation will add to
 *       the overall exit latency.
 */
static void menu_reflect(struct cpuidle_device *dev, int index)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	data->last_state_idx = index;
	data->needs_update = 1;
	data->tick_wakeup = tick_nohz_idle_got_tick();
#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	data->hist_data[data->hist_ptr].idle_state_idx = index;
	data->hist_data[data->hist_ptr].org_state_idx = data->org_state_idx;
	data->hist_data[data->hist_ptr].exit_time = ktime_to_us(ktime_get());
#endif
}

/**
 * menu_update - attempts to guess what happened after entry
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 */
static void menu_update(struct cpuidle_driver *drv, struct cpuidle_device *dev)
{
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	int last_idx = data->last_state_idx;
	struct cpuidle_state *target = &drv->states[last_idx];
	unsigned int measured_us;
	unsigned int new_factor;

	/*
	 * Try to figure out how much time passed between entry to low
	 * power state and occurrence of the wakeup event.
	 *
	 * If the entered idle state didn't support residency measurements,
	 * we use them anyway if they are short, and if long,
	 * truncate to the whole expected time.
	 *
	 * Any measured amount of time will include the exit latency.
	 * Since we are interested in when the wakeup begun, not when it
	 * was completed, we must subtract the exit latency. However, if
	 * the measured amount of time is less than the exit latency,
	 * assume the state was never reached and the exit latency is 0.
	 */
	/* measured value */
	measured_us = cpuidle_get_last_residency(dev);

	/* Deduct exit latency */
	if (measured_us > 2 * target->exit_latency)
		measured_us -= target->exit_latency;
	else
		measured_us /= 2;

	/* Make sure our coefficients do not exceed unity */
	if (measured_us > data->next_timer_us)
		measured_us = data->next_timer_us;

	/* Update our correction ratio */
	new_factor = data->correction_factor[data->bucket];
	new_factor -= new_factor / DECAY;

	if (data->next_timer_us > 0 && measured_us < MAX_INTERESTING)
		new_factor += RESOLUTION * measured_us / data->next_timer_us;
	else
		/*
		 * we were idle so long that we count it as a perfect
		 * prediction
		 */
		new_factor += RESOLUTION;

	/*
	 * We don't want 0 as factor; we always want at least
	 * a tiny bit of estimated time. Fortunately, due to rounding,
	 * new_factor will stay nonzero regardless of measured_us values
	 * and the compiler can eliminate this test as long as DECAY > 1.
	 */
	if (DECAY == 1 && unlikely(new_factor == 0))
		new_factor = 1;

	data->correction_factor[data->bucket] = new_factor;

#ifdef CONFIG_HISI_CPUIDLE_MENU_GOV_HIST
	if (cpumask_test_cpu((int)dev->cpu, &menu_cpumask)) {
		menu_hist_update(data, measured_us);
		return;
	}
#endif
	/* update the repeating-pattern data */
	data->intervals[data->interval_ptr++] = measured_us;
	if (data->interval_ptr >= INTERVALS)
		data->interval_ptr = 0;

#ifdef CONFIG_HISI_CPUIDLE_SKIP_DEEP_CSTATE
	if (data->deep_state_num > SKIP_DEEP_CSTATE_TOP_NUM)
		data->deep_state_num = 0;
	else if (last_idx + 1 == drv->state_count)
		data->deep_state_num++;
	else if (data->resel_deep_state == 1)
		data->deep_state_num++;
	else
		data->deep_state_num = 0;
#endif
}

/**
 * menu_enable_device - scans a CPU's states and does setup
 * @drv: cpuidle driver
 * @dev: the CPU
 */
static int menu_enable_device(struct cpuidle_driver *drv,
				struct cpuidle_device *dev)
{
	struct menu_device *data = &per_cpu(menu_devices, dev->cpu);
	int i;
	struct hrtimer *t = &per_cpu(menu_hrtimer, dev->cpu);

	hrtimer_init(t, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	t->function = menu_hrtimer_notify;
	memset(data, 0, sizeof(struct menu_device));

	/*
	 * if the correction factor is 0 (eg first time init or cpu hotplug
	 * etc), we actually want to start out with a unity factor.
	 */
	for (i = 0; i < BUCKETS; i++)
		data->correction_factor[i] = RESOLUTION * DECAY;

	return 0;
} /*lint !e715*/

static int get_menu_switch_profile(void)
{
	struct device_node *np;
	int ret, cpu;
	unsigned int  cpu_mask;

	menu_hrtimer_enable = 0;
	menu_switch_profile = 0;

	np = of_find_compatible_node(NULL, NULL, "hisi,menu-switch");
	if (!np)
		return -ENODEV;

	ret = of_property_read_u32(np, "cpu-mask", (u32 *)&cpu_mask);
	if (ret) {
		pr_err("get menu cpumask error!\n");
		return -EFAULT;
	}

	cpumask_clear(&menu_cpumask);
	for_each_online_cpu(cpu) {
		if (BIT(cpu) & cpu_mask)
			cpumask_set_cpu(cpu, &menu_cpumask);
	}

	ret = of_property_read_u32(np, "switch-profile", &menu_switch_profile);
	if (ret)
		return -EFAULT;
	return 0;
}

static struct cpuidle_governor menu_governor = {
	.name =		"menu",
	.rating =	20,
	.enable =	menu_enable_device,
	.select =	menu_select,
	.reflect =	menu_reflect,
};

/**
 * init_menu - initializes the governor
 */
static int __init init_menu(void)
{
	int ret;

	ret = get_menu_switch_profile();
	if (!ret)
		register_menu_cpufreq_notifier();
	return cpuidle_register_governor(&menu_governor);
}

postcore_initcall(init_menu);
