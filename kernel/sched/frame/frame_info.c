/*
 * frame_info.c
 *
 * Frame-based load tracking for rt_frame and hisi_rtg
 *
 * Copyright (c) 2019-2021 Huawei Technologies Co., Ltd.
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

#include "frame_info.h"

#include <linux/cpufreq.h>
#include <linux/hisi_rtg.h>
#include <linux/sched.h>
#include <linux/sched/types.h>
#include <trace/events/sched.h>
#include <../kernel/sched/sched.h>
#include <hwrtg/proc_state.h>
#include <securec.h>
#include "hwrtg/set_rtg.h"

static struct frame_info g_frame_info;

/*lint -save -e508 -e712 -e732 -e571 -e737 -e563 -e580*/
bool is_frame_task(struct task_struct *task)
{
	struct related_thread_group *grp = NULL;

	if (unlikely(!task))
		return false;

	rcu_read_lock();
	grp = task_related_thread_group(task);
	rcu_read_unlock();

	return (grp && is_frame_rtg(grp->id));
}
EXPORT_SYMBOL_GPL(is_frame_task);

bool is_frame_rtg(int id)
{
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	return ((id == DEFAULT_RT_FRAME_ID) || ((id >= MULTI_FRAME_ID) &&
		(id < MULTI_FRAME_ID + MULTI_FRAME_NUM)));
#else
	return (id == DEFAULT_RT_FRAME_ID);
#endif
}
EXPORT_SYMBOL_GPL(is_frame_rtg);

static struct related_thread_group *frame_rtg(int id)
{
	if (!is_frame_rtg(id)) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid frame id : %d\n",
		       __func__, id);
		return NULL;
	}
	return lookup_related_thread_group(id);
}

static inline struct frame_info *rtg_frame_info_inner(
	const struct related_thread_group *grp)
{
	return (struct frame_info *)grp->private_data;
}

static void do_update_frame_task_prio(struct frame_info *frame_info,
				      struct task_struct *task, int prio)
{
	int policy = SCHED_NORMAL;
	struct sched_param sp = {0};
	bool is_rt_task = (prio != NOT_RT_PRIO);
	bool need_dec_flag = false;
	bool need_inc_flag = false;
	int err;

	trace_rtg_frame_sched(frame_info->rtg->id, "rtg_rt_thread_num", read_rtg_rt_thread_num());
	/* change policy to RT */
	if (is_rt_task && (atomic_read(&frame_info->curr_rt_thread_num) <
			   atomic_read(&frame_info->max_rt_thread_num))) {
		/* change policy from CFS to RT */
		if (!is_rtg_rt_task(task)) {
			if (test_and_read_rtg_rt_thread_num() >= RTG_MAX_RT_THREAD_NUM)
				goto out;
			need_inc_flag = true;
		}
		/* change RT priority */
		policy = SCHED_FIFO | SCHED_RESET_ON_FORK;
		sp.sched_priority = MAX_USER_RT_PRIO - 1 - prio;
		atomic_inc(&frame_info->curr_rt_thread_num);
	} else {
		/* change policy from RT to CFS */
		if (!is_rt_task && is_rtg_rt_task(task))
			need_dec_flag = true;
	}
out:
	trace_rtg_frame_sched(frame_info->rtg->id, "rtg_rt_thread_num", read_rtg_rt_thread_num());
	trace_rtg_frame_sched(frame_info->rtg->id, "curr_rt_thread_num", atomic_read(&frame_info->curr_rt_thread_num));
	err = sched_setscheduler_nocheck(task, policy, &sp);
	if (err == 0) {
		if (need_dec_flag)
			dec_rtg_rt_thread_num();
		else if (need_inc_flag)
			inc_rtg_rt_thread_num();
	}
}

static void update_frame_task_prio(struct frame_info *frame_info, int prio)
{
	int i;
	struct task_struct *thread = NULL;

	/* reset curr_rt_thread_num */
	atomic_set(&frame_info->curr_rt_thread_num, 0);
	thread = frame_info->pid_task;
	if (thread)
		do_update_frame_task_prio(frame_info, thread, prio);

	thread = frame_info->tid_task;
	if (thread)
		do_update_frame_task_prio(frame_info, thread, prio);

	for (i = 0; i < MAX_TID_NUM; i++) {
		thread = frame_info->thread[i];
		if (thread)
			do_update_frame_task_prio(frame_info, thread, prio);
	}
}

struct frame_info *rtg_frame_info(int id)
{
	struct frame_info *frame_info = NULL;

	if (!is_frame_rtg(id)) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid frame id : %d\n", __func__, id);
		return NULL;
	}
	if (id == DEFAULT_RT_FRAME_ID)
		frame_info = &g_frame_info;
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	else
		frame_info = rtg_active_multi_frame_info(id);
#endif

	return frame_info;
}

int set_frame_rate(struct frame_info *frame_info, int rate)
{
	int id;

	if ((rate < MIN_FRAME_RATE) || (rate > MAX_FRAME_RATE)) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid QOS(rate) value",
			__func__);
		return -EINVAL;
	}

	if (!frame_info || !frame_info->rtg)
		return -EINVAL;

	frame_info->qos_frame = rate;
	frame_info->qos_frame_time = NSEC_PER_SEC / rate;
	frame_info->max_vload_time =
		frame_info->qos_frame_time / NSEC_PER_MSEC +
		frame_info->vload_margin;
	id = frame_info->rtg->id;
	trace_rtg_frame_sched(id, "FRAME_QOS", rate);
	trace_rtg_frame_sched(id, "FRAME_MAX_TIME", frame_info->max_vload_time);

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_rate);

int get_frame_rate(struct frame_info *frame_info)
{
	int rate;

	if (!frame_info)
		return -EINVAL;

	rate = frame_info->qos_frame;

	return rate;
}
EXPORT_SYMBOL_GPL(get_frame_rate);

int set_frame_margin(struct frame_info *frame_info, int margin)
{
	int id;

	if ((margin < MIN_VLOAD_MARGIN) || (margin > MAX_VLOAD_MARGIN)) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid MARGIN value",
			__func__);
		return -EINVAL;
	}

	if (!frame_info || !frame_info->rtg)
		return -EINVAL;

	frame_info->vload_margin = margin;
	frame_info->max_vload_time =
		frame_info->qos_frame_time / NSEC_PER_MSEC +
		frame_info->vload_margin;
	id = frame_info->rtg->id;
	trace_rtg_frame_sched(id, "FRAME_MARGIN", margin);
	trace_rtg_frame_sched(id, "FRAME_MAX_TIME", frame_info->max_vload_time);

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_margin);

int set_frame_max_util(struct frame_info *frame_info, int max_util)
{
	int id;

	if ((max_util < 0) || (max_util > SCHED_CAPACITY_SCALE)) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid max_util value",
			__func__);
		return -EINVAL;
	}

	if (!frame_info || !frame_info->rtg)
		return -EINVAL;

	frame_info->frame_max_util = max_util;
	id = frame_info->rtg->id;
	trace_rtg_frame_sched(id, "FRAME_MAX_UTIL", frame_info->frame_max_util);

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_max_util);

static void do_set_frame_sched_state(struct frame_info *frame_info,
				     struct task_struct *task,
				     bool enable, int prio)
{
	int new_prio = prio;
	bool is_rt_task = (prio != NOT_RT_PRIO);

	if (enable && is_rt_task) {
		if (atomic_read(&frame_info->curr_rt_thread_num) < atomic_read(&frame_info->max_rt_thread_num))
			atomic_inc(&frame_info->curr_rt_thread_num);
		else
			new_prio = NOT_RT_PRIO;
	}
	trace_rtg_frame_sched(frame_info->rtg->id, "curr_rt_thread_num", atomic_read(&frame_info->curr_rt_thread_num));
	trace_rtg_frame_sched(frame_info->rtg->id, "rtg_rt_thread_num", read_rtg_rt_thread_num());
	set_frame_rtg_thread(frame_info->rtg->id, task, enable, new_prio);
}

void set_frame_sched_state(struct frame_info *frame_info, bool enable)
{
	atomic_t *frame_sched_state = NULL;
	int prio;
	int i;
	int id;

	if (!frame_info || !frame_info->rtg)
		return;

	frame_sched_state = &(frame_info->frame_sched_state);
	id = frame_info->rtg->id;
	if (enable) {
		if (atomic_read(frame_sched_state) == 1)
			return;
		atomic_set(frame_sched_state, 1);
		trace_rtg_frame_sched(id, "FRAME_SCHED_ENABLE", 1);

		frame_info->prev_fake_load_util = 0;
		frame_info->prev_frame_load_util = 0;
		frame_info->frame_vload = 0;
		frame_info_rtg_load(frame_info)->curr_window_load = 0;
	} else {
		if (atomic_read(frame_sched_state) == 0)
			return;
		atomic_set(frame_sched_state, 0);
		trace_rtg_frame_sched(id, "FRAME_SCHED_ENABLE", 0);

		(void)sched_set_group_normalized_util(id, 0, FRAME_NORMAL_UPDATE);
		frame_info->status = FRAME_END;
		trace_rtg_frame_sched(id, "preferred_cluster",
			INVALID_PREFERRED_CLUSTER);
	}

	/* reset curr_rt_thread_num */
	atomic_set(&frame_info->curr_rt_thread_num, 0);
	write_lock(&frame_info->lock);
	prio = frame_info->prio;
	if (frame_info->pid_task)
		do_set_frame_sched_state(frame_info, frame_info->pid_task, enable, prio);
	if (frame_info->tid_task)
		do_set_frame_sched_state(frame_info, frame_info->tid_task, enable, prio);
	for (i = 0; i < MAX_TID_NUM; i++) {
		if (frame_info->thread[i])
			do_set_frame_sched_state(frame_info, frame_info->thread[i],
						 enable, prio);
	}
	for (i = 0; i < MAX_FRAME_CFS_THREADS; i++) {
		if (frame_info->cfs_task[i])
			do_set_frame_sched_state(frame_info, frame_info->cfs_task[i],
						 enable, NOT_RT_PRIO);
	}
	write_unlock(&frame_info->lock);

	trace_rtg_frame_sched(id, "FRAME_STATUS", frame_info->status);
	trace_rtg_frame_sched(id, "frame_status", frame_info->status);
}
EXPORT_SYMBOL_GPL(set_frame_sched_state);

static struct task_struct *do_update_thread(struct frame_info *frame_info, int old_prio, int prio, int pid,
					    struct task_struct *old_task)
{
	struct task_struct *task = NULL;
	bool is_rt_task = (prio != NOT_RT_PRIO);
	int new_prio = prio;

	if (pid > 0) {
		if (old_task && (pid == old_task->pid) && (old_prio == new_prio)) {
			if (is_rt_task && atomic_read(&frame_info->curr_rt_thread_num) <
			    atomic_read(&frame_info->max_rt_thread_num) &&
			    (atomic_read(&frame_info->frame_sched_state) == 1))
				atomic_inc(&frame_info->curr_rt_thread_num);
			trace_rtg_frame_sched(frame_info->rtg->id, "curr_rt_thread_num", atomic_read(&frame_info->curr_rt_thread_num));
			return old_task;
		}
		rcu_read_lock();
		task = find_task_by_vpid(pid);
		if (task)
			get_task_struct(task);
		rcu_read_unlock();
	}

	trace_rtg_frame_sched(frame_info->rtg->id, "FRAME_SCHED_ENABLE",
			      atomic_read(&frame_info->frame_sched_state));
	if (atomic_read(&frame_info->frame_sched_state) == 1) {
		if (task && is_rt_task) {
			if (atomic_read(&frame_info->curr_rt_thread_num) < atomic_read(&frame_info->max_rt_thread_num))
				atomic_inc(&frame_info->curr_rt_thread_num);
			else
				new_prio = NOT_RT_PRIO;
		}
		trace_rtg_frame_sched(frame_info->rtg->id, "curr_rt_thread_num", atomic_read(&frame_info->curr_rt_thread_num));
		trace_rtg_frame_sched(frame_info->rtg->id, "rtg_rt_thread_num", read_rtg_rt_thread_num());
		set_frame_rtg_thread(frame_info->rtg->id, old_task, false, NOT_RT_PRIO);
		trace_rtg_frame_sched(frame_info->rtg->id, "rtg_rt_thread_num", read_rtg_rt_thread_num());
		set_frame_rtg_thread(frame_info->rtg->id, task, true, new_prio);
	}

	if (old_task)
		put_task_struct(old_task);

	return task;
}

void update_frame_thread(struct frame_info *frame_info, struct frame_thread_info *frame_thread_info)
{
	int i;
	int old_prio;
	int pid;
	int tid;
	int prio;
	int thread_num;

	if (!frame_info || !frame_thread_info
	    || frame_thread_info->thread_num < 0)
		return;

	pid = frame_thread_info->pid;
	tid = frame_thread_info->tid;
	prio = frame_thread_info->prio;
	thread_num = frame_thread_info->thread_num;
	if (thread_num > MAX_TID_NUM)
		thread_num = MAX_TID_NUM;

	/* reset curr_rt_thread_num */
	atomic_set(&frame_info->curr_rt_thread_num, 0);
	write_lock(&frame_info->lock);
	old_prio = frame_info->prio;
	frame_info->pid_task = do_update_thread(frame_info, old_prio, prio, pid, frame_info->pid_task);
	frame_info->tid_task = do_update_thread(frame_info, old_prio, prio, tid, frame_info->tid_task);
	for (i = 0; i < thread_num; i++)
		frame_info->thread[i] = do_update_thread(frame_info, old_prio, prio, frame_thread_info->thread[i],
							 frame_info->thread[i]);
	frame_info->prio = prio;
	write_unlock(&frame_info->lock);
}
EXPORT_SYMBOL_GPL(update_frame_thread);

void update_frame_cfs_thread(struct frame_info *frame_info, int tid, int index)
{
	if (!frame_info)
		return;
	if (index >= 0 && index < MAX_FRAME_CFS_THREADS)
		frame_info->cfs_task[index] = do_update_thread(frame_info, 0, NOT_RT_PRIO, tid, frame_info->cfs_task[index]);
}
EXPORT_SYMBOL_GPL(update_frame_cfs_thread);

int set_frame_timestamp(struct frame_info *frame_info, unsigned long timestamp)
{
	int ret;

	if (!frame_info || !frame_info->rtg)
		return -EINVAL;

	if (atomic_read(&frame_info->frame_sched_state) == 0)
		return -EINVAL;

	ret = sched_set_group_window_rollover(frame_info->rtg->id);
	if (!ret)
		ret = set_frame_status(frame_info, timestamp);

	return ret;
}
EXPORT_SYMBOL_GPL(set_frame_timestamp);

/*
 * frame_vload [0~1024]
 * vtime: now - timestamp
 * max_time: frame_info->qos_frame_time + vload_margin
 * load = F(vtime)
 *      = vtime ^ 2 - vtime * max_time + FRAME_MAX_VLOAD * vtime / max_time;
 *      = vtime * (vtime + FRAME_MAX_VLOAD / max_time - max_time);
 * [0, 0] -=> [max_time, FRAME_MAX_VLOAD]
 *
 */
static u64 calc_frame_vload(const struct frame_info *frame_info, u64 timeline)
{
	u64 vload;
	int vtime = timeline / NSEC_PER_MSEC;
	int max_time = frame_info->max_vload_time;
	int factor;

	if ((max_time <= 0) || (vtime > max_time))
		return FRAME_MAX_VLOAD;

	factor = vtime + FRAME_MAX_VLOAD / max_time;
	/* margin maybe negative */
	if ((vtime <= 0) || (factor <= max_time))
		return 0;

	vload = (u64)vtime * (u64)(factor - max_time);
	return vload;
}

static inline void frame_boost(struct frame_info *frame_info)
{
	if (frame_info->frame_util < frame_info->frame_boost_min_util)
		frame_info->frame_util = frame_info->frame_boost_min_util;
}

/*
 * frame_load : caculate frame load using exec util
 */
static inline u64 calc_frame_exec(const struct frame_info *frame_info)
{
	if (frame_info->qos_frame_time > 0)
		return (frame_info_rtg_load(frame_info)->curr_window_exec <<
			SCHED_CAPACITY_SHIFT) / frame_info->qos_frame_time;
	else
		return 0;
}

/*
 * frame_load: vload for FRMAE_END and FRAME_INVALID
 */
static inline u64 calc_frame_load(const struct frame_info *frame_info)
{
	return (frame_info_rtg_load(frame_info)->curr_window_load <<
		SCHED_CAPACITY_SHIFT) / frame_info->qos_frame_time;
}

/*
 * real_util:
 * max(last_util, virtual_util, boost_util, phase_util, frame_min_util)
 */
static u64 calc_frame_util(const struct frame_info *frame_info, bool fake)
{
	unsigned long load_util;

	if (fake)
		load_util = frame_info->prev_fake_load_util;
	else
		load_util = frame_info->prev_frame_load_util;
	load_util = max_t(unsigned long, load_util, frame_info->frame_min_util);
	return min_t(unsigned long, max_t(unsigned long, load_util,
			frame_info->frame_vload),
			READ_ONCE(frame_info->frame_max_util));
}

static u64 calc_prev_frame_load_util(const struct frame_info *frame_info)
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

static u64 calc_prev_fake_load_util(const struct frame_info *frame_info)
{
	u64 prev_frame_load = frame_info->prev_frame_load;
	u64 prev_frame_time = max_t(unsigned long, frame_info->prev_frame_time,
		frame_info->qos_frame_time);
	u64 frame_util = 0;

	if (prev_frame_time > 0)
		frame_util = (prev_frame_load << SCHED_CAPACITY_SHIFT) /
			prev_frame_time;

	if (frame_util > FRAME_MAX_LOAD)
		frame_util = FRAME_MAX_LOAD;
	return frame_util;
}

/* last frame load tracking */
static void update_frame_prev_load(struct frame_info *frame_info, bool fake)
{
	/* last frame load tracking */
	frame_info->prev_frame_exec =
		frame_info_rtg_load(frame_info)->prev_window_exec;
	frame_info->prev_frame_time =
		frame_info_rtg(frame_info)->prev_window_time;
	frame_info->prev_frame_load =
		frame_info_rtg_load(frame_info)->prev_window_load;

	if (fake)
		frame_info->prev_fake_load_util =
			calc_prev_fake_load_util(frame_info);
	else
		frame_info->prev_frame_load_util =
			calc_prev_frame_load_util(frame_info);
}

static inline bool check_frame_util_invalid(const struct frame_info *frame_info,
	u64 timeline)
{
	return ((frame_info_rtg(frame_info)->util_invalid_interval <= timeline) &&
		(frame_info_rtg_load(frame_info)->curr_window_exec *
		FRAME_UTIL_INVALID_FACTOR <= timeline));
}

static void set_frame_start(struct frame_info *frame_info)
{
	int id = frame_info->rtg->id;
	if (likely(frame_info->status == FRAME_START)) {
		/*
		 * START -=> START -=> ......
		 * FRMAE_START is
		 *	the end of last frame
		 *	the start of the current frame
		 */
		update_frame_prev_load(frame_info, false);
	} else if ((frame_info->status == FRAME_END) ||
		(frame_info->status == FRAME_INVALID)) {
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
	trace_rtg_frame_sched(id, "FRAME_STATUS", frame_info->status);
	trace_rtg_frame_sched(id, "frame_last_task_time",
		frame_info->prev_frame_exec);
	trace_rtg_frame_sched(id, "frame_last_time", frame_info->prev_frame_time);
	trace_rtg_frame_sched(id, "frame_last_load", frame_info->prev_frame_load);
	trace_rtg_frame_sched(id, "frame_last_load_util",
		frame_info->prev_frame_load_util);

	/* new_frame_start */
	frame_info->frame_vload = 0;
	if (!frame_info->margin_imme) {
		unsigned long util = max_t(unsigned long,
			frame_info->prev_frame_load_util,
			frame_info->frame_min_util);
		frame_info->frame_util = min_t(unsigned long,
			frame_info->frame_max_util, util);
	} else {
		frame_info->frame_vload = calc_frame_vload(frame_info, 0);
		frame_info->frame_util = calc_frame_util(frame_info, false);
	}

	trace_rtg_frame_sched(id, "frame_vload", frame_info->frame_vload);
}

static void do_frame_end(struct frame_info *frame_info, bool fake)
{
	unsigned long prev_util;
	int id = frame_info->rtg->id;

	frame_info->status = FRAME_END;
	trace_rtg_frame_sched(id, "frame_status", frame_info->status);

	/* last frame load tracking */
	update_frame_prev_load(frame_info, fake);

	/* reset frame_info */
	frame_info->frame_vload = 0;

	if (fake)
		prev_util = frame_info->prev_fake_load_util;
	else
		prev_util = frame_info->prev_frame_load_util;
	frame_info->frame_min_util = 0;
	frame_info->frame_util = min_t(unsigned long,
		frame_info->frame_max_util,
		prev_util);

	trace_rtg_frame_sched(id, "frame_last_task_time",
		frame_info->prev_frame_exec);
	trace_rtg_frame_sched(id, "frame_last_time", frame_info->prev_frame_time);
	trace_rtg_frame_sched(id, "frame_last_load", frame_info->prev_frame_load);
	trace_rtg_frame_sched(id, "frame_last_load_util",
		frame_info->prev_frame_load_util);
	trace_rtg_frame_sched(id, "frame_last_fake_load_util",
		frame_info->prev_fake_load_util);
	trace_rtg_frame_sched(id, "frame_util", frame_info->frame_util);
	trace_rtg_frame_sched(id, "frame_vload", frame_info->frame_vload);
}

static void set_frame_end(struct frame_info *frame_info)
{
	trace_rtg_frame_sched(frame_info->rtg->id, "FRAME_STATUS", FRAME_END);
	do_frame_end(frame_info, false);
}

static int update_frame_timestamp(unsigned long status,
	struct frame_info *frame_info, const struct related_thread_group *grp)
{
	int id = frame_info->rtg->id;

	/* SCHED_FRAME timestamp */
	switch (status) {
	case FRAME_START:
		/* collect frame_info when frame_end timestamp comming */
		set_frame_start(frame_info);
		break;
	case FRAME_END:
		/* FRAME_END should only set and update freq once */
		if (unlikely(frame_info->status == FRAME_END))
			return 0;
		set_frame_end(frame_info);
		break;
	default:
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid timestamp(status)\n",
			__func__);
		return -EINVAL;
	}

	frame_boost(frame_info);
	trace_rtg_frame_sched(id, "frame_util", frame_info->frame_util);
	/* update cpufreq force when frame_stop */
	sched_set_group_normalized_util(grp->id,
		frame_info->frame_util, FRAME_FORCE_UPDATE);

	if (grp->preferred_cluster)
		trace_rtg_frame_sched(id, "preferred_cluster",
			grp->preferred_cluster->id);

	return 0;
}

int set_frame_status(struct frame_info *frame_info, unsigned long status)
{
	struct related_thread_group *grp = NULL;
	int id;

	if (!frame_info)
		return -EINVAL;

	grp = frame_info->rtg;
	if (unlikely(!grp))
		return -EINVAL;

	if (atomic_read(&frame_info->frame_sched_state) == 0)
		return -EINVAL;

	if (!(status & FRAME_SETTIME) ||
		(status == (unsigned long)FRAME_SETTIME_PARAM)) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid timetsamp(status)\n",
			__func__);
		return -EINVAL;
	}

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
	id = grp->id;
	trace_rtg_frame_sched(id, "FRAME_TIMESTAMP_SKIPPED",
		frame_info->timestamp_skipped);
	trace_rtg_frame_sched(id, "FRAME_MAX_UTIL", frame_info->frame_max_util);

	if (status & FRAME_USE_MARGIN_IMME) {
		frame_info->margin_imme = true;
		status &= ~FRAME_USE_MARGIN_IMME;
	} else {
		frame_info->margin_imme = false;
	}
	trace_rtg_frame_sched(id, "FRAME_MARGIN_IMME", frame_info->margin_imme);
	trace_rtg_frame_sched(id, "FRAME_TIMESTAMP", status);

	return update_frame_timestamp(status, frame_info, grp);
}
EXPORT_SYMBOL_GPL(set_frame_status);

static int update_frame_info_tick_inner(int id, struct frame_info *frame_info,
	u64 timeline)
{
	switch (frame_info->status) {
	case FRAME_INVALID:
	case FRAME_END:
		if (timeline >= frame_info->qos_frame_time) {
			/*
			 * fake FRAME_END here to rollover frame_window.
			 */
			sched_set_group_window_rollover(id);
			do_frame_end(frame_info, true);
		} else {
			frame_info->frame_vload = calc_frame_exec(frame_info);
			frame_info->frame_util =
				calc_frame_util(frame_info, true);
		}

		/* when not in boost, start tick timer */
		break;
	case FRAME_START:
		/* check frame_util invalid */
		if (!check_frame_util_invalid(frame_info, timeline)) {
			/* frame_vload statistic */
			frame_info->frame_vload = calc_frame_vload(frame_info, timeline);
			/* frame_util statistic */
			frame_info->frame_util =
				calc_frame_util(frame_info, false);
		} else {
			frame_info->status = FRAME_INVALID;
			trace_rtg_frame_sched(id, "frame_status",
				frame_info->status);
			/*
			 * trigger FRAME_END to rollover frame_window,
			 * we treat FRAME_INVALID as FRAME_END.
			 */
			sched_set_group_window_rollover(id);
			do_frame_end(frame_info, false);
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * update CPUFREQ and PLACEMENT when frame task running (in tick) and migration
 */
static void update_frame_info_tick(struct related_thread_group *grp)
{
	u64 window_start;
	u64 wallclock;
	u64 timeline;
	struct frame_info *frame_info = NULL;
	int id = grp->id;

	rcu_read_lock();
	frame_info = rtg_frame_info_inner(grp);
	window_start = grp->window_start;
	rcu_read_unlock();
	if (unlikely(!frame_info))
		return;

	if (atomic_read(&frame_info->frame_sched_state) == 0)
		return;
	trace_rtg_frame_sched(id, "frame_status", frame_info->status);

	wallclock = ktime_get_ns();
	timeline = wallclock - window_start;

	trace_rtg_frame_sched(id, "update_curr_pid", current->pid);
	trace_rtg_frame_sched(id, "frame_timeline", timeline / NSEC_PER_MSEC);

	if (update_frame_info_tick_inner(grp->id, frame_info, timeline) == -EINVAL)
		return;

	frame_boost(frame_info);
	trace_rtg_frame_sched(id, "frame_vload", frame_info->frame_vload);
	trace_rtg_frame_sched(id, "frame_util", frame_info->frame_util);
	sched_set_group_normalized_util(grp->id, frame_info->frame_util,
					FRAME_NORMAL_UPDATE);
	if (grp->preferred_cluster)
		trace_rtg_frame_sched(id, "preferred_cluster",
			grp->preferred_cluster->id);
}

int update_frame_isolation(void)
{
	struct related_thread_group *grp = current->grp;

	if (unlikely(!grp || !grp->preferred_cluster))
		return -EINVAL;

	if (unlikely(!is_frame_rtg(grp->id) &&
		(grp->id != DEFAULT_AUX_ID)))
		return -EINVAL;

	return !(grp->preferred_cluster == max_cap_cluster());
}

int set_frame_min_util(struct frame_info *frame_info, int min_util, bool is_boost)
{
	int id;
	if (unlikely((min_util < 0) || (min_util > SCHED_CAPACITY_SCALE))) {
		pr_err("[%s] [IPROVISION-FRAME_INFO] invalid min_util value",
			__func__);
		return -EINVAL;
	}

	if (!frame_info || !frame_info->rtg)
		return -EINVAL;

	id = frame_info->rtg->id;
	if (is_boost) {
		frame_info->frame_boost_min_util = min_util;
		trace_rtg_frame_sched(id, "FRAME_BOOST_MIN_UTIL", min_util);
	} else {
		frame_info->frame_min_util = min_util;

		frame_info->frame_util = calc_frame_util(frame_info, false);
		trace_rtg_frame_sched(id, "frame_util", frame_info->frame_util);
		sched_set_group_normalized_util(id, frame_info->frame_util,
						FRAME_FORCE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(set_frame_min_util);

int set_frame_min_util_and_margin(struct frame_info *frame_info,
				  int min_util, int margin)
{
	int ret;
	if (!frame_info)
		return -EINVAL;

	ret = set_frame_margin(frame_info, margin);
	if (ret != 0)
		return ret;

	ret = set_frame_min_util(frame_info, min_util, false);
	return ret;
}
EXPORT_SYMBOL_GPL(set_frame_min_util_and_margin);

static bool is_task_in_frame(const struct task_struct *task,
			     const struct frame_info *frame_info)
{
	int i;

	if (unlikely(!frame_info))
		return false;

	if (task == frame_info->pid_task || task == frame_info->tid_task)
		return true;

	for (i = 0; i < MAX_TID_NUM; i++)
		if (task == frame_info->thread[i])
			return true;

	for (i = 0; i < MAX_FRAME_CFS_THREADS; i++)
		if (task == frame_info->cfs_task[i])
			return true;

	return false;
}

struct frame_info *lookup_frame_info_by_task(struct task_struct *task)
{
	struct related_thread_group *grp = NULL;
	int id = DEFAULT_RT_FRAME_ID;
	struct frame_info *frame_info = NULL;

	if (unlikely(!task))
		return NULL;

	rcu_read_lock();
	grp = task_related_thread_group(task);
	rcu_read_unlock();

	if (grp && is_frame_rtg(grp->id))
		return rtg_frame_info_inner(grp);

	frame_info = rtg_frame_info(id);
	if (frame_info && is_task_in_frame(task, frame_info))
		return frame_info;
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	for (id = MULTI_FRAME_ID; id < (MULTI_FRAME_ID + MULTI_FRAME_NUM); id++) {
		frame_info = rtg_frame_info(id);
		if (frame_info && is_task_in_frame(task, frame_info))
			return frame_info;
	}
#endif
	return NULL;
}

struct frame_info *lookup_frame_info_by_pid(int pid)
{
	struct task_struct *task = NULL;
	struct frame_info *frame_info = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task) {
		rcu_read_unlock();
		return frame_info;
	}
	get_task_struct(task);
	rcu_read_unlock();
	frame_info = lookup_frame_info_by_task(task);
	put_task_struct(task);

	return frame_info;
}

int get_frame_prio_by_id(int rtgid)
{
	struct frame_info *frame_info = NULL;
	int prio;

	frame_info = rtg_frame_info(rtgid);
	if (unlikely(!frame_info))
		return -EINVAL;

	read_lock(&frame_info->lock);
	prio = frame_info->prio;
	read_unlock(&frame_info->lock);

	return prio;
}

void set_frame_prio(struct frame_info *frame_info, int prio)
{
	if (!frame_info)
		return;

	write_lock(&frame_info->lock);
	if (frame_info->prio == prio)
		goto out;

	update_frame_task_prio(frame_info, prio);
	frame_info->prio = prio;
out:
	write_unlock(&frame_info->lock);
}

const struct rtg_class frame_rtg_class = {
	.sched_update_rtg_tick = update_frame_info_tick,
};

static int _init_frame_info(struct frame_info *frame_info, int id)
{
	struct related_thread_group *grp = NULL;
	unsigned long flags;

	(void)memset_s(frame_info, sizeof(struct frame_info), 0, sizeof(struct frame_info));
	rwlock_init(&frame_info->lock);

	write_lock(&frame_info->lock);
	atomic_set(&(frame_info->frame_sched_state), 0);
	atomic_set(&(frame_info->curr_rt_thread_num), 0);
	frame_info->qos_frame = DEFAULT_FRAME_RATE;
	frame_info->qos_frame_time = NSEC_PER_SEC / frame_info->qos_frame;
	frame_info->vload_margin = DEFAULT_VLOAD_MARGIN;
	frame_info->max_vload_time =
		frame_info->qos_frame_time / NSEC_PER_MSEC +
		frame_info->vload_margin;
	frame_info->frame_max_util = FRAME_DEFAULT_MAX_UTIL;
	frame_info->frame_min_util = 0;
	frame_info->margin_imme = false;
	frame_info->timestamp_skipped = false;
	frame_info->status = FRAME_END;
	frame_info->prio = NOT_RT_PRIO;

	grp = frame_rtg(id);
	if (unlikely(!grp)) {
		write_unlock(&frame_info->lock);
		return -EINVAL;
	}

	raw_spin_lock_irqsave(&grp->lock, flags);
	grp->private_data = frame_info;
	grp->rtg_class = &frame_rtg_class;
	raw_spin_unlock_irqrestore(&grp->lock, flags);

	frame_info->rtg = grp;
	write_unlock(&frame_info->lock);

	return 0;
}

static int __init init_frame_info(void)
{
	int ret = 0;
	int id = DEFAULT_RT_FRAME_ID;

	ret = _init_frame_info(&g_frame_info, id);

#ifdef CONFIG_HW_RTG_MULTI_FRAME
	for (id = MULTI_FRAME_ID; id < (MULTI_FRAME_ID + MULTI_FRAME_NUM); id++) {
		if (ret != 0)
			break;
		ret = _init_frame_info(rtg_multi_frame_info(id), id);
	}
#endif

	return ret;
}
late_initcall(init_frame_info);
/*lint -restore*/
