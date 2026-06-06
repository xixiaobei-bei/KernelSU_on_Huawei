/*
 *  Frame-based load tracking for  rt_frame and hisi_rtg
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; version 2
 *  of the License.
 */

#include <linux/sched.h>
#include <linux/cpufreq.h>

#ifdef CONFIG_TRACE_FRAME_SYSTRACE
#include <trace/events/power.h>
#endif

#include <../kernel/sched/sched.h>
#include <linux/hisi_rtg.h>

#include "frame_info.h"

/* QOS value : [1, 120] */
#define DEFAULT_FRAME_QOS		60
#define MIN_FRAME_QOS			1
#define MAX_FRAME_QOS			120

/* MARGIN value : [-100, 100] */
#define DEFAULT_VLOAD_MARGIN		16
#define MIN_VLOAD_MARGIN		-100
#define MAX_VLOAD_MARGIN		0xffff


#define FRAME_MAX_VLOAD			SCHED_CAPACITY_SCALE
#define FRAME_MAX_LOAD			SCHED_CAPACITY_SCALE
#define FRAME_UTIL_INVALID_FACTOR	4

#define FRAME_DEFAULT_MAX_UTIL		SCHED_CAPACITY_SCALE

/*lint -save -e508 -e712 -e732 -e571 -e737 -e563*/
bool is_frame_task(struct task_struct *task)
{
	struct related_thread_group *grp = NULL;

	if (task == NULL)
		return false;

	rcu_read_lock();
	grp = task_related_thread_group(task);
	rcu_read_unlock();

	return (grp && grp->id == DEFAULT_RT_FRAME_ID);
}
EXPORT_SYMBOL_GPL(is_frame_task);

static inline struct related_thread_group *frame_rtg(void)
{
	return lookup_related_thread_group(DEFAULT_RT_FRAME_ID);
}

static inline struct frame_info *
__rtg_frame_info(struct related_thread_group *grp)
{
	return (struct frame_info *) grp->private_data;
}

struct frame_info *rtg_frame_info(void)
{
	struct frame_info *frame_info = NULL;
	struct related_thread_group *grp = frame_rtg();

	if (!grp)
		return NULL;

	frame_info = __rtg_frame_info(grp);

	return frame_info;
}

int set_frame_rate(int qos)
{
	struct frame_info *frame_info =	NULL;

	if (qos < MIN_FRAME_QOS || qos > MAX_FRAME_QOS) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid QOS(rate) value", __func__);
		return -EINVAL;
	}

	frame_info = rtg_frame_info();

	if (!frame_info)
		return -EIO;

	frame_info->qos_frame = qos;
	frame_info->qos_frame_time = NSEC_PER_SEC / qos;
	frame_info->max_vload_time = frame_info->qos_frame_time / NSEC_PER_MSEC
		+ frame_info->vload_margin;
	FRAME_SYSTRACE("FRAME_QOS", qos, smp_processor_id());
	FRAME_SYSTRACE("FRAME_MAX_TIME", frame_info->max_vload_time, smp_processor_id());

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_rate);

int set_frame_margin(int margin)
{
	struct frame_info *frame_info = NULL;

	if (margin < MIN_VLOAD_MARGIN || margin > MAX_VLOAD_MARGIN) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid MARGIN value", __func__);
		return -EINVAL;
	}

	frame_info = rtg_frame_info();

	if (!frame_info)
		return -EIO;

	frame_info->vload_margin = margin;
	frame_info->max_vload_time = frame_info->qos_frame_time / NSEC_PER_MSEC
		+ frame_info->vload_margin;
	FRAME_SYSTRACE("FRAME_MARGIN", margin, smp_processor_id());
	FRAME_SYSTRACE("FRAME_MAX_TIME", frame_info->max_vload_time, smp_processor_id());

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_margin);


int set_frame_max_util(int max_util)
{
	struct frame_info *frame_info = NULL;

	if (max_util < 0 || max_util > SCHED_CAPACITY_SCALE) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid max_util value", __func__);
		return -EINVAL;
	}

	frame_info = rtg_frame_info();

	if (!frame_info)
		return -EIO;

	frame_info->frame_max_util = max_util;
	FRAME_SYSTRACE("FRAME_MAX_TIME", frame_info->frame_max_util, smp_processor_id());

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_max_util);

int set_frame_timestamp(unsigned long timestamp)
{
	int ret = 0;

	ret = sched_set_group_window_rollover(DEFAULT_RT_FRAME_ID);

	if (!ret)
		ret = set_frame_status(timestamp);

	return ret;
}
EXPORT_SYMBOL_GPL(set_frame_timestamp);

/*
 * frame_vload [0~1024]
 * vtime = now - timestamp
 * max_time = frame_info->qos_frame_time + vload_margin
 * load = F(vtime)
 *	= vtime ^ 2 - vtime * max_time + FRAME_MAX_VLOAD * vtime / max_time;
 *	= vtime * (vtime + FRAME_MAX_VLOAD / max_time - max_time);
 * [0, 0] -=> [max_time, FRAME_MAX_VLOAD]
 *
 */
u64 calc_frame_vload(struct frame_info *frame_info, u64 timeline)
{
	u64 vload;
	int vtime = timeline / NSEC_PER_MSEC;
	int max_time = frame_info->max_vload_time;
	int factor = 0;

	if (max_time <= 0 || vtime > max_time)
		return FRAME_MAX_VLOAD;

	factor = vtime + FRAME_MAX_VLOAD / max_time;
	/* margin maybe negative */
	if (vtime <= 0 || factor <= max_time)
		return 0;

	vload = (u64)vtime * (u64)(factor - max_time);

	return vload;
}

/*
 * frame_load : vload for FRMAE_END and FRAME_INVALID
 */
static inline u64 calc_frame_load(struct frame_info *frame_info)
{
	return (frame_info_rtg_load(frame_info)->curr_window_load
		<< SCHED_CAPACITY_SHIFT) / frame_info->qos_frame_time;
}

/*
 * real_util = max(last_util, virtual_util, boost_util, phase_util)
 */
static inline u64 calc_frame_util(struct frame_info *frame_info, bool fake)
{
	unsigned long load_util;

	if (fake)
		load_util = frame_info->prev_fake_load_util;
	else
		load_util = frame_info->prev_frame_load_util;

	return min_t(unsigned long, max_t(unsigned long, load_util,
			frame_info->frame_vload),
			READ_ONCE(frame_info->frame_max_util));
}

static inline u64 calc_prev_frame_load_util(struct frame_info *frame_info)
{
	u64 prev_frame_load = frame_info->prev_frame_load;
	u64 frame_util = 0;

	if (prev_frame_load >= frame_info->qos_frame_time)
		frame_util = FRAME_MAX_LOAD;
	else
		frame_util = (prev_frame_load << SCHED_CAPACITY_SHIFT) /
			frame_info->qos_frame_time;
	return frame_util;
}

/* last frame load tracking */
static inline void update_frame_prev_load(
		struct frame_info *frame_info,
		bool fake)
{
	unsigned long load_util;

	/* last frame load tracking */
	frame_info->prev_frame_exec = frame_info_rtg_load(frame_info)->prev_window_exec;
	frame_info->prev_frame_time = frame_info_rtg(frame_info)->prev_window_time;
	frame_info->prev_frame_load = frame_info_rtg_load(frame_info)->prev_window_load;

	load_util = calc_prev_frame_load_util(frame_info);

	if (fake)
		frame_info->prev_fake_load_util = load_util;
	else
		frame_info->prev_frame_load_util = load_util;

}

static inline bool
check_frame_util_invalid(struct frame_info *frame_info, u64 timeline)
{
	return (frame_info_rtg(frame_info)->util_invalid_interval <= timeline &&
		frame_info_rtg_load(frame_info)->curr_window_exec * FRAME_UTIL_INVALID_FACTOR
		<= timeline);
}

static inline void set_frame_start(struct frame_info *frame_info)
{
	if (likely(frame_info->status == FRAME_START)) {
		/*
		 * START -=> START -=> ......
		 * FRMAE_START is
		 *	the end of last frame
		 *	the start of the current frame
		 */
		update_frame_prev_load(frame_info, false);
	} else if (frame_info->status == FRAME_END ||
		frame_info->status ==  FRAME_INVALID) {
		/* START -=> END -=> [START]
		 *  FRAME_START is
		 *	only the start of current frame
		 * we shoudn't tracking the last rtg-window
		 * [FRAME_END, FRAME_START]
		 * it's not an available frame window
		 */
		update_frame_prev_load(frame_info, true);
		frame_info->status = FRAME_START;
	}

	FRAME_SYSTRACE("FRAME_STATUS", frame_info->status, smp_processor_id());

	FRAME_SYSTRACE("frame_last_task_time", frame_info->prev_frame_exec, smp_processor_id());
	FRAME_SYSTRACE("frame_last_time", frame_info->prev_frame_time, smp_processor_id());
	FRAME_SYSTRACE("frame_last_load", frame_info->prev_frame_load, smp_processor_id());
	FRAME_SYSTRACE("frame_last_load_util", frame_info->prev_frame_load_util, smp_processor_id());

	/* new_frame_start */
	frame_info->frame_vload = 0;
	if (!frame_info->margin_imme) {
		frame_info->frame_util = min_t(unsigned long,
			frame_info->frame_max_util,
			frame_info->prev_frame_load_util);
	} else {
		frame_info->frame_vload = calc_frame_vload(frame_info, 0);
		frame_info->frame_util = calc_frame_util(frame_info, false);
	}

	FRAME_SYSTRACE("frame_util", frame_info->frame_util, smp_processor_id());
	FRAME_SYSTRACE("frame_vload", frame_info->frame_vload, smp_processor_id());
}

static inline void set_frame_end(struct frame_info *frame_info, bool fake)
{
	unsigned long prev_util = 0;

	frame_info->status = FRAME_END;
	FRAME_SYSTRACE("FRAME_STATUS", frame_info->status, smp_processor_id());

	/* last frame load tracking */
	update_frame_prev_load(frame_info, fake);

	/* reset frame_info */
	frame_info->frame_vload = 0;

	if (fake)
		prev_util = frame_info->prev_fake_load_util;
	else
		prev_util = frame_info->prev_frame_load_util;
	frame_info->frame_util = min_t(unsigned long,
		frame_info->frame_max_util,
		prev_util);

	FRAME_SYSTRACE("frame_last_task_time", frame_info->prev_frame_exec, smp_processor_id());
	FRAME_SYSTRACE("frame_last_time", frame_info->prev_frame_time, smp_processor_id());
	FRAME_SYSTRACE("frame_last_load", frame_info->prev_frame_load, smp_processor_id());

	FRAME_SYSTRACE("frame_last_load_util", frame_info->prev_frame_load_util, smp_processor_id());

	FRAME_SYSTRACE("frame_util", frame_info->frame_util, smp_processor_id());
	FRAME_SYSTRACE("frame_vload", frame_info->frame_vload, smp_processor_id());
}

int set_frame_status(unsigned long status)
{
	u64 timestamp;
	struct related_thread_group *grp = NULL;
	struct frame_info *frame_info = NULL;

	if (!(status & FRAME_SETTIME) ||
		status == (unsigned long)FRAME_SETTIME_PARAM) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid timetsamp(status)\n", __func__);
		return -EINVAL;
	}

	grp = frame_rtg();

	if (!grp)
		return -EIO;

	frame_info = __rtg_frame_info(grp);

	if (!frame_info)
		return -EIO;

	if (status & FRAME_TIMESTAMP_SKIP_START) {
		frame_info->timestamp_skipped = true;
		status &= ~FRAME_TIMESTAMP_SKIP_START;
	} else if (status & FRAME_TIMESTAMP_SKIP_END) {
		frame_info->timestamp_skipped = false;
		status &= ~FRAME_TIMESTAMP_SKIP_END;
	} else if (frame_info->timestamp_skipped) {
		/*
		 * skip the following timestamp until
		 * FRAME_TIMESTAMP_SKIPPED reset
		 */
		return 0;
	}
	FRAME_SYSTRACE("FRAME_TIMESTAMP_SKIPPED", frame_info->timestamp_skipped, smp_processor_id());
	FRAME_SYSTRACE("FRAME_MAX_UTIL", frame_info->frame_max_util, smp_processor_id());

	if (status & FRAME_USE_MARGIN_IMME) {
		frame_info->margin_imme = true;
		status &= ~FRAME_USE_MARGIN_IMME;
	} else {
		frame_info->margin_imme = false;
	}
	FRAME_SYSTRACE("FRAME_MARGIN_IMME", frame_info->margin_imme, smp_processor_id());

	timestamp = ktime_get_ns();
	FRAME_SYSTRACE("FRAME_TIMESTAMP", status, smp_processor_id());

	/* SCHED_FRAME timestamp */
	switch (status) {
	case FRAME_START:
		/* collect frame_info when frame_end timestamp comming */
		set_frame_start(frame_info);

		/* update cpufreq force when frame_start */
		sched_set_group_normalized_util(DEFAULT_RT_FRAME_ID,
			frame_info->frame_util, FRAME_FORCE_UPDATE);
		break;
	case FRAME_END:
		/* FRAME_END should only set and update freq once */
		if (unlikely(frame_info->status == FRAME_END))
			return 0;

		set_frame_end(frame_info, false);

		/* update cpufreq force when frame_stop */
		sched_set_group_normalized_util(DEFAULT_RT_FRAME_ID,
			frame_info->frame_util, FRAME_FORCE_UPDATE);
		break;
	default:
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid timestamp(status)\n", __func__);
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_status);

struct frame_info global_frame_info;
static int __init init_frame_info(void)
{
	struct related_thread_group *grp = NULL;
	struct frame_info *frame_info = NULL;
	unsigned long flags;

	frame_info = &global_frame_info;
	memset(frame_info, 0, sizeof(struct frame_info));

	frame_info->qos_frame = DEFAULT_FRAME_QOS;
	frame_info->qos_frame_time = NSEC_PER_SEC / frame_info->qos_frame;
	frame_info->vload_margin = DEFAULT_VLOAD_MARGIN;
	frame_info->max_vload_time = frame_info->qos_frame_time / NSEC_PER_MSEC
		+ frame_info->vload_margin;

	frame_info->frame_max_util = FRAME_DEFAULT_MAX_UTIL;

	frame_info->margin_imme = false;
	frame_info->timestamp_skipped = false;

	frame_info->status = FRAME_END;

	grp = frame_rtg();

	raw_spin_lock_irqsave(&grp->lock, flags);
	grp->private_data = frame_info;
	raw_spin_unlock_irqrestore(&grp->lock, flags);

	frame_info->rtg = grp;

	return 0;
}
late_initcall(init_frame_info);

/*
 * update CPUFREQ and PLACEMENT when frame task running (in tick) and migration
 */
void update_frame_info_tick(struct related_thread_group *grp)
{
	u64 window_start = 0, wallclock = 0, timeline = 0;
	struct frame_info *frame_info = NULL;

	rcu_read_lock();
	if (grp->id != DEFAULT_RT_FRAME_ID) {
		rcu_read_unlock();
		return;
	}

	frame_info = __rtg_frame_info(grp);
	window_start = grp->window_start;
	rcu_read_unlock();

	if (!frame_info)
		return;

	FRAME_SYSTRACE("frame_status", frame_info->status, smp_processor_id());

	wallclock = ktime_get_ns();
	timeline = wallclock - window_start;

	FRAME_SYSTRACE("update_curr_pid", current->pid, smp_processor_id());
	FRAME_SYSTRACE("frame_timeline", timeline / NSEC_PER_MSEC, smp_processor_id());

	switch (frame_info->status) {
	case FRAME_INVALID:
	case FRAME_END:
		if (timeline >= frame_info->qos_frame_time) {
			/*
			 * fake FRAME_END here to rollover frame_window.
			 * set_frame_timestamp(FRAME_END);
			 */
			sched_set_group_window_rollover(DEFAULT_RT_FRAME_ID);
			set_frame_end(frame_info, true);
		} else {
			frame_info->frame_vload = calc_frame_load(frame_info);
			frame_info->frame_util = calc_frame_util(frame_info, true);
		}
		break;
	case FRAME_START:
		/* check frame_util invalid */
		if (!check_frame_util_invalid(frame_info, timeline)) {
			/* frame_vload statistic */
			frame_info->frame_vload =
				calc_frame_vload(frame_info, timeline);

			/* frame_util statistic */
			frame_info->frame_util = calc_frame_util(frame_info, false);
		} else {
			frame_info->status = FRAME_INVALID;
			FRAME_SYSTRACE("FRAME_STATUS", frame_info->status, smp_processor_id());
			FRAME_SYSTRACE("frame_status", frame_info->status, smp_processor_id());

			/*
			 * trigger FRAME_END to rollover frame_window,
			 * we treat FRAME_INVALID as FRAME_END.
			 */
			sched_set_group_window_rollover(DEFAULT_RT_FRAME_ID);
			set_frame_end(frame_info, false);
		}
		break;
	default:
		return;
	}

	FRAME_SYSTRACE("frame_vload", frame_info->frame_vload, smp_processor_id());
	FRAME_SYSTRACE("frame_util", frame_info->frame_util, smp_processor_id());
	sched_set_group_normalized_util(DEFAULT_RT_FRAME_ID,
		frame_info->frame_util, FRAME_NORMAL_UPDATE);
}

int update_frame_isolation(void)
{
	struct related_thread_group *grp = frame_rtg();

	if (!grp || !grp->preferred_cluster)
		return -1;

	return !(grp->preferred_cluster == max_cap_cluster());
}
/*lint -restore*/
