/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * proc state manager
 */

#include "include/proc_state.h"

#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/cred.h>
#include <linux/sched/task.h>
#include <linux/string.h>
#include <linux/idr.h>
#include <trace/events/sched.h>
#include <securec.h>

#include "include/set_rtg.h"
#include "include/frame_timer.h"
#include "include/aux.h"
#include "include/iaware_rtg.h"

#ifdef CONFIG_HW_RTG_SCHED
#include <linux/hisi_rtg.h>
#include <linux/sched/frame.h>
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
#include <mtkrtg/frame.h>
#include "mtkrtg/mtk_rtg_sched.h"
#endif
#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#define FRAME_START (1 << 0)
#define FRAME_END (1 << 1)
#define FRAME_USE_MARGIN_IMME (1 << 4)
#define FRAME_TIMESTAMP_SKIP_START (1 << 5)
#define FRAME_TIMESTAMP_SKIP_END (1 << 6)

#define MIN_RTG_QOS 2

#define INIT_VALUE (-1)
#define UPDATE_RTG_FRAME (1 << 0)
#define ADD_RTG_FRAME (1 << 1)
#define CLEAR_RTG_FRAME (1 << 2)
#define DEFAULT_MAX_RT_FRAME 3
#define MAX_RT_THREAD (MAX_TID_NUM + 2)

DEFINE_IDR(rate_margin_idr);
static int g_default_rate = 0;
static rwlock_t g_state_margin_lock;

static atomic_t g_boost_thread_pid = ATOMIC_INIT(INIT_VALUE);
static atomic_t g_boost_thread_tid = ATOMIC_INIT(INIT_VALUE);
static atomic_t g_boost_thread_min_util = ATOMIC_INIT(0);

static int g_frame_max_util = DEFAULT_MAX_UTIL;
static int g_act_max_util = DEFAULT_MAX_UTIL;
static int g_max_rt_frames = DEFAULT_MAX_RT_FRAME;
#ifdef CONFIG_HW_RTG_MULTI_FRAME
static atomic_t g_rt_frame_num = ATOMIC_INIT(0);
#endif

static inline bool is_valid_type(int type)
{
	return (type >= VIP && type < RTG_TYPE_MAX);
}

static inline bool is_rt_type(int type)
{
	return (type >= VIP && type < NORMAL_TASK);
}

static inline bool is_clear_frame_thread(const struct rtg_data_head *head)
{
	return (head->uid == -1 || head->pid == -1 || head->tid == -1);
}

/*lint -save -e446 -e666 -e734*/
static void set_frame(struct frame_info *frame_info, int margin, int frame_type)
{
	if (!frame_info)
		return;

	atomic_set(&frame_info->frame_state, frame_type);
	if (set_frame_margin(frame_info, margin) == SUCC)
		set_frame_timestamp(frame_info, FRAME_START);
}

static void reset_frame(struct frame_info *frame_info)
{
	if (!frame_info)
		return;

	if (atomic_read(&frame_info->frame_state) == FRAME_END_STATE) {
		pr_debug("[AWARE_RTG]: Frame state is already reset\n");
		return;
	}

	atomic_set(&frame_info->frame_state, FRAME_END_STATE);
	set_frame_timestamp(frame_info, FRAME_END);
}

static void set_activity(struct frame_info *frame_info, int margin)
{
	if (!frame_info)
		return;

	pr_debug("[AWARE_RTG]: %s id=%d, margin=%d\n", __func__, frame_info->rtg->id, margin);
	atomic_set(&frame_info->act_state, ACTIVITY_BEGIN);
	if (set_frame_margin(frame_info, margin) == SUCC) {
		set_frame_max_util(frame_info, g_act_max_util);
		set_frame_timestamp(frame_info, FRAME_START | FRAME_USE_MARGIN_IMME |
				    FRAME_TIMESTAMP_SKIP_START);
	}
}

static void reset_activity(struct frame_info *frame_info)
{
	if (!frame_info)
		return;

	if (atomic_read(&frame_info->act_state) == ACTIVITY_END) {
		pr_debug("[AWARE_RTG]: Activity state is already reset\n");
		return;
	}
	pr_debug("[AWARE_RTG]: %s id=%d\n", __func__, frame_info->rtg->id);
	atomic_set(&frame_info->act_state, ACTIVITY_END);
	set_frame_max_util(frame_info, g_frame_max_util);
	set_frame_timestamp(frame_info, FRAME_END | FRAME_TIMESTAMP_SKIP_END);
}

static int get_proc_state(const char *state_str)
{
	int state;

	pr_debug("[AWARE_RTG,str]: %s\n", state_str);
	if (!strcmp("activity", state_str))
		state = ACTIVITY_BEGIN;
	else if (!strcmp("frame0", state_str))
		state = FRAME_BUFFER_0;
	else if (!strcmp("frame1", state_str))
		state = FRAME_BUFFER_1;
	else if (!strcmp("frame2", state_str))
		state = FRAME_BUFFER_2;
	else if (!strcmp("click", state_str))
		state = FRAME_CLICK;
	else
		state = STATE_MAX;
	return state;
}

/*lint -e429*/
static int put_state_margin(const char *state_str, const char *margin_str,
			    struct idr *margin_idr)
{
	int *temp = NULL;
	int state;
	int margin;
	int ret;

	if ((state_str == NULL) || (margin_str == NULL) || margin_idr == NULL)
		return -1;

	if (kstrtoint((const char *)margin_str, DECIMAL, &margin))
		return -1;

	state = get_proc_state(state_str);
	if ((state <= STATE_MIN) || (state >= STATE_MAX))
		return -1;

	pr_debug("[AWARE_RTG]: %s state=%d, margin=%d", __func__, state, margin);

	write_lock(&g_state_margin_lock);
	temp = idr_find(margin_idr, state);
	if (!temp) {
		temp = kzalloc(sizeof(int), GFP_ATOMIC);
		if (temp == NULL) {
			write_unlock(&g_state_margin_lock);
			pr_err("[AWARE_RTG] %s kzalloc failed\n", __func__);
			return -1;
		}
		*temp = margin;

		ret = idr_alloc(margin_idr, temp, state, state + 1, GFP_ATOMIC);
		if (ret < 0) {
			kfree(temp);
			write_unlock(&g_state_margin_lock);
			pr_err("[AWARE_RTG] %s idr_alloc failed, ret = %d\n", __func__, ret);
			return -1;
		}
	}

	write_unlock(&g_state_margin_lock);

	return 0;
}
/*lint +e429*/

static int get_state_margin_by_rate(int rate, int state)
{
	struct idr *margin_idr = NULL;
	int *temp = NULL;
	int margin = INVALID_VALUE;

	read_lock(&g_state_margin_lock);
	margin_idr = idr_find(&rate_margin_idr, rate);
	/* if margin_idr is null, try to use default rate */
	if (margin_idr == NULL)
		margin_idr = idr_find(&rate_margin_idr, g_default_rate);

	if (margin_idr != NULL) {
		temp = idr_find(margin_idr, state);
		if (temp != NULL)
			margin = *temp;
	}

	/* state not found from activity or framesched or click, then it's rme state */
	if (margin == INVALID_VALUE)
		margin = state;

	read_unlock(&g_state_margin_lock);

	return margin;
}

#ifdef CONFIG_HW_QOS_THREAD
static int get_qos(pid_t tid)
{
	int qos = -1;
	struct task_struct *tsk = NULL;

	rcu_read_lock();
	if (tid > 0) {
		tsk = find_task_by_vpid(tid);
		if (tsk == NULL) {
			rcu_read_unlock();
			return qos;
		}
		qos = get_task_qos(tsk);
	}
	rcu_read_unlock();
	pr_debug("[AWARE_RTG]: %s tid=%d, qos=%d", __func__, tid, qos);
	return qos;
}
#elif defined CONFIG_HW_VIP_THREAD
static int get_vip(pid_t tid)
{
	int vip = 0;
	struct task_struct *tsk = NULL;

	rcu_read_lock();
	if (tid > 0) {
		tsk = find_task_by_vpid(tid);
		if (tsk == NULL) {
			rcu_read_unlock();
			return vip;
		}

		get_task_struct(tsk);
		vip = tsk->static_vip;
		put_task_struct(tsk);
	}
	rcu_read_unlock();
	pr_debug("[AWARE_RTG]: %s tid=%d, vip=%d", __func__, tid, vip);
	return vip;
}
#endif

#ifdef CONFIG_HW_RTG_SCHED
static bool all_cfs_tids_invalid(const int *tids, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (tids[i] > 0)
			return false;
	}
	return true;
}

static void init_cfs_tids(struct frame_info *frame_info)
{
	int i;

	for (i = 0; i < MAX_FRAME_CFS_THREADS; i++)
		update_frame_cfs_thread(frame_info, -1, i);
}

static int update_rtg_frame_cfs_thread(struct frame_info *frame_info, const struct rtg_cfs_data_head *proc_info)
{
	int pid = proc_info->pid;
	int cfs_tid;
	int type = proc_info->type;
	int index = -1;
	int i;

	if (all_cfs_tids_invalid(proc_info->cfs_tid, MAX_FRAME_CFS_THREADS)) {
		init_cfs_tids(frame_info);
		return SUCC;
	}

	if (type) {
		init_cfs_tids(frame_info);
		index = 0;
	} else {
		for (i = 0; i < MAX_FRAME_CFS_THREADS; i++) {
			if (frame_info->cfs_task[i] == NULL) {
				index = i;
				break;
			}
		}
	}

	if (index < 0)
		return SUCC;

	for (i = 0; i < MAX_FRAME_CFS_THREADS && index < MAX_FRAME_CFS_THREADS; i++) {
		cfs_tid = proc_info->cfs_tid[i];
		if (cfs_tid > 0) {
			update_frame_cfs_thread(frame_info, cfs_tid, index);
			index++;
		}
	}

	pr_debug("[AWARE_RTG] %s uiTid=%d, type=%d cfs_tid=%d, %d, %d\n",
		__func__, pid, type, proc_info->cfs_tid[0], proc_info->cfs_tid[1], proc_info->cfs_tid[2]);
	return SUCC;
}
#endif

#ifdef CONFIG_HW_RTG_MULTI_FRAME
static int do_update_rt_frame_num(struct frame_info *frame_info, int new_type)
{
	int rtgid = frame_info->rtg->id;
	int old_type;
	int ret = SUCC;

	read_lock(&frame_info->lock);
	old_type = frame_info->prio - DEFAULT_RT_PRIO;
	if (rtgid == DEFAULT_RT_FRAME_ID
	    || (is_rt_type(new_type) == is_rt_type(old_type)))
		goto out;

	if (is_rt_type(old_type)) {
		if (atomic_read(&g_rt_frame_num) > 0)
			atomic_dec(&g_rt_frame_num);
	} else if (is_rt_type(new_type)) {
		if (atomic_read(&g_rt_frame_num) < g_max_rt_frames) {
			atomic_inc(&g_rt_frame_num);
		} else {
			pr_err("[AWARE_RTG]: %s g_max_rt_frames is %d", __func__, g_max_rt_frames);
			ret = -INVALID_ARG;
		}
	}
out:
	read_unlock(&frame_info->lock);
	return ret;
}

static int update_rt_frame_num(struct frame_info *frame_info, int new_type, int cmd)
{
	int ret = SUCC;

	switch (cmd) {
	case UPDATE_RTG_FRAME:
		ret = do_update_rt_frame_num(frame_info, new_type);
		break;
	case ADD_RTG_FRAME:
		if (is_rt_type(new_type)) {
			if (atomic_read(&g_rt_frame_num) >= g_max_rt_frames) {
				pr_err("[AWARE_RTG] g_max_rt_frames is %d!\n", g_max_rt_frames);
				ret = -INVALID_ARG;
			} else {
				atomic_inc(&g_rt_frame_num);
			}
		}
		break;
	case CLEAR_RTG_FRAME:
		if ((atomic_read(&g_rt_frame_num) > 0) && is_rt_type(new_type))
			atomic_dec(&g_rt_frame_num);
		break;
	default:
		return -INVALID_ARG;
	}
	trace_rtg_frame_sched(frame_info->rtg->id, "g_rt_frame_num", atomic_read(&g_rt_frame_num));
	trace_rtg_frame_sched(frame_info->rtg->id, "g_max_rt_frames", g_max_rt_frames);
	return ret;
}
#endif

static void clear_rtg_frame_thread(struct frame_info *frame_info)
{
	struct frame_thread_info frame_thread_info;
	int i;

	frame_thread_info.prio = NOT_RT_PRIO;
	frame_thread_info.pid = -1;
	frame_thread_info.tid = -1;
	for (i = 0; i < MAX_TID_NUM; i++)
		frame_thread_info.thread[i] = -1;
	frame_thread_info.thread_num = MAX_TID_NUM;

	update_frame_thread(frame_info, &frame_thread_info);
#ifdef CONFIG_HW_RTG_SCHED
	init_cfs_tids(frame_info);
#endif
	atomic_set(&frame_info->max_rt_thread_num, DEFAULT_MAX_RT_THREAD);
	atomic_set(&frame_info->frame_sched_state, 0);
#ifdef CONFIG_FRAME_RTG
	trace_rtg_frame_sched(frame_info->rtg->id, "FRAME_SCHED_ENABLE", 0);
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
	trace_rtg_frame_sched("FRAME_SCHED_ENABLE", 0);
#endif
}

static void init_frame_thread_info(struct frame_thread_info *frame_thread_info,
				   const struct rtg_proc_data *proc_info)
{
	int i;
	int type = proc_info->type;

	frame_thread_info->prio = (type == NORMAL_TASK ? NOT_RT_PRIO : (type + DEFAULT_RT_PRIO));
	frame_thread_info->pid = proc_info->head.pid;
	frame_thread_info->tid = proc_info->head.tid;
	for (i = 0; i < MAX_TID_NUM; i++)
		frame_thread_info->thread[i] = proc_info->thread[i];
	frame_thread_info->thread_num = MAX_TID_NUM;
}

static int update_rtg_frame_thread(const struct rtg_proc_data *proc_info, int cmd)
{
	struct frame_thread_info frame_thread_info;
	int id = proc_info->rtgid;
	int type = proc_info->type;
	int uid = proc_info->head.uid;
	struct frame_info *frame_info = rtg_frame_info(id);

	if (!frame_info)
		return -INVALID_RTG_ID;

	atomic_set(&frame_info->max_rt_thread_num, proc_info->rtcnt);
#ifdef CONFIG_FRAME_RTG
	trace_rtg_frame_sched(frame_info->rtg->id, "curr_rt_thread_num", atomic_read(&frame_info->curr_rt_thread_num));
	trace_rtg_frame_sched(frame_info->rtg->id, "max_rt_thread_num", atomic_read(&frame_info->max_rt_thread_num));
#endif
	if (is_clear_frame_thread(&proc_info->head)) {
		atomic_set(&frame_info->uid, -1);
		type = frame_info->prio - DEFAULT_RT_PRIO;
		clear_rtg_frame_thread(frame_info);
#ifdef CONFIG_HW_RTG_MULTI_FRAME
		if (id != DEFAULT_RT_FRAME_ID) {
			release_multi_frame_info(id);
			update_rt_frame_num(frame_info, type, CLEAR_RTG_FRAME);
		}
#endif
		pr_debug("[AWARE_RTG] %s clear frame(id=%d)\n", __func__, id);
		return SUCC;
	}

#ifdef CONFIG_HW_RTG_MULTI_FRAME
	if (update_rt_frame_num(frame_info, type, cmd)) {
		if (cmd == ADD_RTG_FRAME && id != DEFAULT_RT_FRAME_ID)
			release_multi_frame_info(id);
		return -NO_RT_FRAME;
	}
#endif
	init_frame_thread_info(&frame_thread_info, proc_info);
	atomic_set(&frame_info->uid, uid);
	atomic_set(&frame_info->frame_sched_state, 1);
#ifdef CONFIG_FRAME_RTG
	trace_rtg_frame_sched(id, "FRAME_SCHED_ENABLE", 1);
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
	trace_rtg_frame_sched("FRAME_SCHED_ENABLE", 1);
#endif
#ifdef CONFIG_HW_RTG_SCHED
	if (!frame_info->pid_task || (frame_info->pid_task && (frame_info->pid_task->pid != frame_thread_info.pid)))
		init_cfs_tids(frame_info);
#endif
	update_frame_thread(frame_info, &frame_thread_info);
	pr_debug("[AWARE_RTG] %s rtgid=%d uid=%d, pid=%d, tid=%d, type=%d, prio=%d\n",
		 __func__, id, uid, frame_thread_info.pid, frame_thread_info.tid, type, frame_thread_info.prio);

	return SUCC;
}

static void free_margin_list(void)
{
	int rate;
	int state;
	int *margin = NULL;
	struct idr *margin_idr = NULL;

	write_lock(&g_state_margin_lock);
	idr_for_each_entry(&rate_margin_idr, margin_idr, rate) {
		idr_for_each_entry(margin_idr, margin, state) {
			idr_remove(margin_idr, state);
			kfree(margin);
		}
		idr_remove(&rate_margin_idr, rate);
		kfree(margin_idr);
	}
	g_default_rate = 0;
	write_unlock(&g_state_margin_lock);
}

static void start_frame_freq(struct frame_info *frame_info)
{
	if (!frame_info)
		return;

	if (atomic_read(&frame_info->start_frame_freq) == 0) {
		atomic_set(&frame_info->start_frame_freq, 1);
		set_frame_sched_state(frame_info, true);
	}
}

static void reset_frame_info(struct frame_info *frame_info)
{
	clear_rtg_frame_thread(frame_info);
	atomic_set(&frame_info->uid, -1);
	atomic_set(&frame_info->act_state, -1);
	atomic_set(&frame_info->frame_state, -1);
	atomic_set(&frame_info->curr_rt_thread_num, 0);
	atomic_set(&frame_info->max_rt_thread_num, DEFAULT_MAX_RT_THREAD);
}

static int do_init_proc_state(int rtgid, const int *config, int len)
{
	struct related_thread_group *grp = NULL;
	struct rtg_load_mode mode;
	struct frame_info *frame_info = NULL;

	grp = lookup_related_thread_group(rtgid);
	if (unlikely(!grp))
		return -EINVAL;

	frame_info = (struct frame_info *)grp->private_data;
	if (!frame_info)
		return -EINVAL;

	reset_frame_info(frame_info);

	if (config[RTG_LOAD_FREQ] != INVALID_VALUE) {
		mode.grp_id = rtgid;
		mode.freq_enabled = !config[RTG_LOAD_FREQ];
		mode.util_enabled = 1;
		sched_set_group_load_mode(&mode);
	}

	if ((config[RTG_FREQ_CYCLE] >= MIN_FREQ_CYCLE) &&
		(config[RTG_FREQ_CYCLE] <= MAX_FREQ_CYCLE))
		sched_set_group_freq_update_interval(rtgid,
				(unsigned int)config[RTG_FREQ_CYCLE]);
	else
		sched_set_group_freq_update_interval(rtgid,
				DEFAULT_FREQ_CYCLE);

	if (config[RTG_INVALID_INTERVAL] != INVALID_VALUE)
		sched_set_group_util_invalid_interval(rtgid,
				config[RTG_INVALID_INTERVAL]);
	else
		sched_set_group_util_invalid_interval(rtgid,
				DEFAULT_INVALID_INTERVAL);

	set_frame_max_util(frame_info, g_frame_max_util);

	return SUCC;
}

int init_proc_state(const int *config, int len)
{
	int ret;
	int id = DEFAULT_RT_FRAME_ID;

	if ((config == NULL) || (len != RTG_CONFIG_NUM))
		return -INVALID_ARG;

	rwlock_init(&g_state_margin_lock);
	if ((config[RTG_FRAME_MAX_UTIL] > 0) &&
		(config[RTG_FRAME_MAX_UTIL] < DEFAULT_MAX_UTIL))
		g_frame_max_util = config[RTG_FRAME_MAX_UTIL];
	if ((config[RTG_ACT_MAX_UTIL] > 0) &&
		(config[RTG_ACT_MAX_UTIL] < DEFAULT_MAX_UTIL))
		g_act_max_util = config[RTG_ACT_MAX_UTIL];

	ret = do_init_proc_state(id, config, len);
	if (ret) {
		pr_err("[AWARE_RTG] init proc state for DEFAULT_RT_FRAME_ID failed, ret=%d\n", ret);
		return ret;
	}
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	for (id = MULTI_FRAME_ID; id < (MULTI_FRAME_ID + MULTI_FRAME_NUM); id++) {
		ret = do_init_proc_state(id, config, len);
		if (ret) {
			pr_err("[AWARE_RTG] init proc state for FRAME_ID=%d failed, ret=%d\n", id, ret);
			return ret;
		}
	}
	atomic_set(&g_rt_frame_num, 0);
#endif
	return SUCC;
}

void deinit_proc_state(void)
{
	int id = DEFAULT_RT_FRAME_ID;
	struct frame_info *frame_info = rtg_frame_info(id);
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	struct related_thread_group *grp = NULL;
#endif

	free_margin_list();
	if (frame_info)
		reset_frame_info(frame_info);
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	for (id = MULTI_FRAME_ID; id < (MULTI_FRAME_ID + MULTI_FRAME_NUM); id++) {
		grp = lookup_related_thread_group(id);
		if (unlikely(!grp))
			return;

		frame_info = (struct frame_info *)grp->private_data;
		if (frame_info)
			reset_frame_info(frame_info);
	}
	clear_multi_frame_info();
	atomic_set(&g_rt_frame_num, 0);
#endif
}

static struct idr *alloc_rtg_margin_idr(int rate)
{
	struct idr *margin_idr = NULL;
	int ret;

	if ((rate < MIN_FRAME_RATE) || (rate > MAX_FRAME_RATE)) {
		pr_err("[AWARE_RTG] %s invalid rate : %d\n", __func__, rate);
		return ERR_PTR(-EINVAL);
	}

	write_lock(&g_state_margin_lock);
	margin_idr = idr_find(&rate_margin_idr, rate);
	if (!margin_idr) {
		margin_idr = kzalloc(sizeof(*margin_idr), GFP_ATOMIC);
		if (!margin_idr) {
			write_unlock(&g_state_margin_lock);
			pr_err("[AWARE_RTG] %s kzalloc failed\n", __func__);
			return ERR_PTR(-ENOMEM);
		}
		idr_init(margin_idr);

		ret = idr_alloc(&rate_margin_idr, margin_idr, rate, rate + 1, GFP_ATOMIC);
		if (ret < 0) {
			kfree(margin_idr);
			write_unlock(&g_state_margin_lock);
			pr_err("[AWARE_RTG] %s idr_alloc failed, ret = %d\n", __func__, ret);
			return ERR_PTR(ret);
		}
	}
	write_unlock(&g_state_margin_lock);

	return margin_idr;
}

int parse_config(const struct rtg_str_data *rs_data)
{
	int len;
	char *p = NULL;
	char *tmp = NULL;
	char *data = NULL;
	struct idr *margin_idr = NULL;
	int value;

	if (rs_data == NULL)
		return -INVALID_ARG;
	data = rs_data->data;
	len = rs_data->len;
	if ((data == NULL) || (strlen(data) != len)) //lint !e737
		return -INVALID_ARG;
	g_default_rate = 0;
	/*
	 * DEFAULT_FRAME_RATE(60):
	 *     eg: rtframe:4;activity:-16;frame0:8;frame1:10;frame2:20;click:-8;
	 * Multi rate:
	 *     eg: rtframe:2;rate:60;activity:-16;frame0:8;frame1:10;frame2:20;click:-8;
	 *         rate:90;activity:-11;frame0:8;frame1:10;frame2:20;click:-8;
	 *         rate:120;activity:-6;frame0:8;frame1:10;frame2:20;click:-8;
	 */
	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if ((tmp == NULL) || (p == NULL))
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &value))
			return -INVALID_ARG;
		if (!strcmp(tmp, "rtframe")) {
			if (value > 0 && value <= MULTI_FRAME_NUM) {
				g_max_rt_frames = value;
			} else {
				pr_err("[AWARE_RTG] %s invalid max_rt_frame: %d, MULTI_FRAME_NUM=%d\n",
				       __func__, value, MULTI_FRAME_NUM);
				return -INVALID_ARG;
			}
		} else if (!strcmp(tmp, "rate")) {
			margin_idr = alloc_rtg_margin_idr(value);
			if (IS_ERR(margin_idr))
				return PTR_ERR(margin_idr);
			if (g_default_rate == 0)
				g_default_rate = value;
		} else {
			if (margin_idr == NULL && g_default_rate == 0) {
				margin_idr = alloc_rtg_margin_idr(DEFAULT_FRAME_RATE);
				if (IS_ERR(margin_idr))
					return PTR_ERR(margin_idr);
				g_default_rate = DEFAULT_FRAME_RATE;
			}
			// eg: margin0:-1;margin1:0;margin2:1;margin3:2;margin4:3
			if (put_state_margin(tmp, p, margin_idr)) {
				pr_err("[AWARE_RTG] parse config failed\n");
				return -INVALID_ARG;
			}
		}
	}

	return SUCC;
}

#ifdef CONFIG_HW_RTG_MULTI_FRAME
int parse_rtg_attr(const struct rtg_str_data *rs_data)
{
	char *p = NULL;
	char *tmp = NULL;
	char *data = NULL;
	int value;
	struct frame_info *frame_info = NULL;
	int rate = -1;
	int type = -1;

	if (rs_data == NULL)
		return -INVALID_ARG;
	data = rs_data->data;
	if ((data == NULL) || (rs_data->len <= 0) ||
		(rs_data->len > MAX_DATA_LEN))
		return -INVALID_ARG;

	// eg: rtgId:xx;rate:xx;type:xx;
	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if ((tmp == NULL) || (p == NULL))
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &value))
			return -INVALID_ARG;
		if (!strcmp(tmp, "rtgId")) {
			frame_info = rtg_frame_info(value);
			if (!frame_info)
				return -INVALID_RTG_ID;
		} else if (!strcmp(tmp, "rate")) {
			rate = value;
		} else if (!strcmp(tmp, "type")) {
			if (is_valid_type(value)) {
				type = value;
			} else {
				pr_err("[AWARE_RTG] invalid type : %d\n", value);
				return -INVALID_ARG;
			}
		} else {
			pr_err("[AWARE_RTG] parse rtg attr failed!\n");
			return -INVALID_ARG;
		}
	}

	if (rate > 0)
		set_frame_rate(frame_info, rate);

	if (is_valid_type(type)) {
		if (update_rt_frame_num(frame_info, type, UPDATE_RTG_FRAME))
			return -INVALID_ARG;

		set_frame_prio(frame_info, (type == NORMAL_TASK ?
			       NOT_RT_PRIO : (type + DEFAULT_RT_PRIO)));
	}

	return SUCC;
}
#endif

int parse_aux_thread(const struct rtg_str_data *rs_data)
{
	char *p = NULL;
	char *tmp = NULL;
	char *data = NULL;
	int id;
	int tid = 0;
	int enable = 0;
	struct aux_info info;

	if (rs_data == NULL)
		return -INVALID_ARG;
	data = rs_data->data;
	if ((data == NULL) || (rs_data->len <= 0) ||
		(rs_data->len > MAX_DATA_LEN))
		return -INVALID_ARG;

	// eg: auxTid:10000;enable:1;minUtil:100;rtgPrio:1
	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if ((tmp == NULL) || (p == NULL))
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &id))
			return -INVALID_ARG;
		if (!strcmp(tmp, "auxTid")) {
			tid = id;
		} else if (!strcmp(tmp, "enable")) {
			enable = id;
		} else if (!strcmp(tmp, "minUtil")) {
			info.min_util = id;
		} else if (!strcmp(tmp, "rtgPrio")) {
			info.prio = id;
		} else {
			pr_err("[AWARE_RTG] parse config failed!\n");
			return -INVALID_ARG;
		}
	}

	return sched_rtg_aux(tid, enable, &info);
}

#define AUX_KEY_COMM_MAX 12
struct st_aux_comms {
	int count;
	char comms[AUX_KEY_COMM_MAX][TASK_COMM_LEN];
};

static struct st_aux_comms s_aux_comms = {
	.count = 0,
	.comms = {{0}}
};

int parse_aux_comm_config(const struct rtg_str_data *rs_data)
{
	char *tmp = NULL;
	char *p = NULL;
	int len;

	if (rs_data == NULL)
		return -INVALID_ARG;

	p = rs_data->data;
	len = rs_data->len;
	if ((p == NULL) || (strlen(p) != len)) //lint !e737
		return -INVALID_ARG;

	s_aux_comms.count = 0;

	// eg: keyThreads:Chrome_InProcRe,CrRendererMain
	tmp = strsep(&p, ":");
	while (p != NULL) {
		if (s_aux_comms.count >= AUX_KEY_COMM_MAX)
			break;

		tmp = strsep(&p, ",");
		if (tmp && *tmp != '\0') {
			strncpy(s_aux_comms.comms[s_aux_comms.count],
				tmp, TASK_COMM_LEN - 1);
			s_aux_comms.count++;
		}
	}

	return SUCC;
}

bool is_key_aux_comm(const struct task_struct *task, const char *comm)
{
	int i;

	if (!comm || !task || !is_rtg_enable() ||
		(s_aux_comms.count == 0))
		return false;

	for (i = 0; i < s_aux_comms.count; ++i) {
		if (strstr(comm, s_aux_comms.comms[i]))
			return true;
	}

	return false;
}

static void set_min_util_detail(int tid, int min_util)
{
	struct task_struct *task = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(tid);
	if (!task) {
		rcu_read_unlock();
		return;
	}
	get_task_struct(task);
	rcu_read_unlock();

	(void)set_task_min_util(task, min_util);
	put_task_struct(task);
}

void set_boost_thread_min_util(int min_util)
{
	pid_t pid;
	pid_t tid;

	if ((min_util > DEFAULT_MAX_UTIL) || (min_util < 0))
		return;

	pid = atomic_read(&g_boost_thread_pid);
	tid = atomic_read(&g_boost_thread_tid);
	if ((pid == INIT_VALUE) || (tid == INIT_VALUE))
		return;
#ifdef CONFIG_FRAME_RTG
	trace_rtg_frame_sched(0, "boost_util", min_util);
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
	trace_rtg_frame_sched("boost_util", min_util);
#endif
	atomic_set(&g_boost_thread_min_util, min_util);

	set_min_util_detail(pid, min_util);
	set_min_util_detail(tid, min_util);
}

static void update_boost_thread(int pid, int tid)
{
	int min_util = 0;

	if ((pid == INIT_VALUE) || (tid == INIT_VALUE)) {
		set_boost_thread_min_util(0);
		atomic_set(&g_boost_thread_pid, INIT_VALUE);
		atomic_set(&g_boost_thread_tid, INIT_VALUE);
		return;
	}

	if ((pid != atomic_read(&g_boost_thread_pid)) ||
		(tid != atomic_read(&g_boost_thread_tid))) {
		min_util = atomic_read(&g_boost_thread_min_util);
		set_boost_thread_min_util(0);
	}

	atomic_set(&g_boost_thread_pid, pid);
	atomic_set(&g_boost_thread_tid, tid);
	if (min_util != 0)
		set_boost_thread_min_util(min_util);
}

int parse_boost_thread(const struct rtg_str_data *rs_data)
{
	char *p = NULL;
	char *tmp = NULL;
	char *data = NULL;
	pid_t pid = INIT_VALUE;
	pid_t tid = INIT_VALUE;
	int id;

	if (rs_data == NULL)
		return -INVALID_ARG;
	data = rs_data->data;
	if ((data == NULL) || (rs_data->len <= 0) || (rs_data->len > MAX_DATA_LEN))
		return -INVALID_ARG;

	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if (tmp == NULL || p == NULL)
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &id))
			return -INVALID_ARG;

		if (!strcmp(tmp, "boostUid")) {
			continue;
		} else if (!strcmp(tmp, "utid")) {
			pid = id;
		} else if (!strcmp(tmp, "rtid")) {
			tid = id;
		} else {
			pr_err("[AWARE_RTG] parse thread boost config failed!\n");
			return -INVALID_ARG;
		}
	}

	update_boost_thread(pid, tid);
	return SUCC;
}

static int do_parse_frame_thread(const struct rtg_str_data *rs_data,
				 struct rtg_proc_data *proc_info)
{
	char *p = NULL;
	char *tmp = NULL;
	char *data = rs_data->data;
	int id;
	int tid_num = 0;

	(void)memset_s(proc_info, sizeof(struct rtg_proc_data), 0, sizeof(struct rtg_proc_data));
	proc_info->rtgid = DEFAULT_RT_FRAME_ID;
	proc_info->type = VIP;
	proc_info->rtcnt = DEFAULT_MAX_RT_THREAD;
	/*
	 * Update the frame's threads by rtgId:
	 *   eg: fgUid:10000;uiTid:10000;renderTid:10000;Tid:1000;type:1;rtgId:1
	 * Add threads to new frame:
	 *   eg: fgUid:10000;uiTid:10000;renderTid:10000;Tid:1000;type:0;rtcnt:3;rtgId:-1
	 * Release the frame by rtgId:
	 *   eg: fgUid:-1;uiTid:-1;renderTid:-1;rtgId:1
	 */
	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if (tmp == NULL || p == NULL)
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &id))
			return -INVALID_ARG;

		if (!strcmp(tmp, "fgUid")) {
			proc_info->head.uid = id;
		} else if (!strcmp(tmp, "uiTid")) {
			proc_info->head.pid = id;
		} else if (!strcmp(tmp, "renderTid")) {
			proc_info->head.tid = id;
		} else if (!strcmp(tmp, "Tid") && tid_num < MAX_TID_NUM) {
			proc_info->thread[tid_num++] = id;
		} else if (!strcmp(tmp, "rtcnt")) {
			if (id < 0 || id > MAX_RT_THREAD) {
				pr_err("[AWARE_RTG] %s invalid rtcnt: %d, MAX_RT_THREAD=%d\n", __func__, id, MAX_RT_THREAD);
				return -INVALID_ARG;
			}
			proc_info->rtcnt = id;
#ifdef CONFIG_HW_RTG_MULTI_FRAME
		} else if (!strcmp(tmp, "type")) {
			proc_info->type = id;
		} else if (!strcmp(tmp, "rtgId")) {
			proc_info->rtgid = id;
#endif
		} else {
			pr_err("[AWARE_RTG] parse config failed!\n");
			return -INVALID_ARG;
		}
	}

	pr_debug("[AWARE_RTG] %s uid:%d, pid:%d, tid:%d, type:%d, rtcnt=%d, rtgid:%d\n",
		__func__, proc_info->head.uid, proc_info->head.pid, proc_info->head.tid,
		proc_info->type, proc_info->rtcnt, proc_info->rtgid);

	return SUCC;
}

#ifdef CONFIG_HW_RTG_SCHED
static struct frame_info *lookup_frame_info_by_rtgid(const int rtgid)
{
	struct frame_info *frame_info = NULL;

	if (rtgid) {
		frame_info = rtg_frame_info(rtgid);
		if (frame_info && atomic_read(&frame_info->uid) != current_uid().val) {
			pr_err("[AWARE_RTG] %s uid=%d is not current uid %d\n", __func__,
			       atomic_read(&frame_info->uid), current_uid().val);
			frame_info = NULL;
		}
	} else {
		frame_info = lookup_frame_info_by_task(current);
	}

	return frame_info;
}

static int do_parse_frame_cfs_thread(const struct rtg_str_data *rs_data, int *rtgid,
				     struct rtg_cfs_data_head *proc_info)
{
	int id;
	int index;
	char *data = rs_data->data;
	char *tmp = NULL;
	char *p = NULL;

	proc_info->pid = 0;
	proc_info->type = 0;
	for (index = 0; index < MAX_FRAME_CFS_THREADS; index++)
		proc_info->cfs_tid[index] = 0;

	index = 0;
	// eg: uiTid:10000;Tid:10001;Tid:10002;type:1:rtgId:xxx;
	// eg: uiTid:10000;Tid:10003;type:0;rtgId:xxx;
	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if (tmp == NULL || p == NULL)
			continue;
		if (kstrtoint((const char *)p, DECIMAL, &id))
			return -INVALID_ARG;

		if (!strcmp(tmp, "uiTid")) {
			proc_info->pid = id;
		} else if (!strcmp(tmp, "Tid")) {
			if (index < MAX_FRAME_CFS_THREADS)
				proc_info->cfs_tid[index++] = id;
		} else if (!strcmp(tmp, "type")) {
			proc_info->type = id;
		} else if (!strcmp(tmp, "rtgId")) {
			*rtgid = id;
		} else {
			pr_err("[AWARE_RTG] parse config failed!\n");
			return -INVALID_ARG;
		}
	}

	return SUCC;
}


int parse_frame_cfs_thread(const struct rtg_str_data *rs_data)
{
	struct rtg_cfs_data_head proc_info;
	int rtgid = 0;
	int ret;
	struct frame_info *frame_info = NULL;

	if (rs_data == NULL)
		return -INVALID_ARG;
	if ((rs_data->data == NULL) || (rs_data->len <= 0) || (rs_data->len > MAX_DATA_LEN))
		return -INVALID_ARG;

	ret = do_parse_frame_cfs_thread(rs_data, &rtgid, &proc_info);
	if (ret)
		return ret;

	if (proc_info.pid != current->pid) {
		pr_err("[AWARE_RTG] %s pid=%d is not current pid\n", __func__, proc_info.pid);
		return -NO_PERMISSION;
	}

	frame_info = lookup_frame_info_by_rtgid(rtgid);
	if (!frame_info || !frame_info->rtg) {
		pr_err("[AWARE_RTG] %s pid=%d is not in frame rtg\n", __func__, proc_info.pid);
		return -FRAME_ERR_PID;
	}

	ret = update_rtg_frame_cfs_thread(frame_info, &proc_info);
	if (ret == 0)
		reset_activity(frame_info);
	return ret;
}
#endif

int parse_frame_thread(const struct rtg_str_data *rs_data)
{
	struct rtg_proc_data proc_info;
	int ret;
	int cmd;
	int err;

	if (rs_data == NULL)
		return -INVALID_ARG;

	if ((rs_data->data == NULL) || (rs_data->len <= 0) ||
		(rs_data->len > MAX_DATA_LEN))
		return -INVALID_ARG;

	ret = do_parse_frame_thread(rs_data, &proc_info);
	if (ret)
		return ret;

	if (is_valid_type(proc_info.type) && !is_clear_frame_thread(&proc_info.head)) {
		if (proc_info.rtgid < 0) {
			proc_info.rtgid = alloc_multi_frame_info();
			ret = proc_info.rtgid;
			cmd = ADD_RTG_FRAME;
			pr_debug("[AWARE_RTG] alloc frame, rtgid=%d, type=%d\n",
				 proc_info.rtgid, proc_info.type);
		} else if (is_frame_rtg(proc_info.rtgid)) {
			pr_debug("[AWARE_RTG] update frame, rtgid=%d, type=%d\n",
				 proc_info.rtgid, proc_info.type);
			ret = proc_info.rtgid;
			cmd = UPDATE_RTG_FRAME;
		} else {
			pr_err("[AWARE_RTG] param is invailed!\n");
			return -INVALID_ARG;
		}
	} else if (is_clear_frame_thread(&proc_info.head)
		   && is_frame_rtg(proc_info.rtgid)) {
		pr_debug("[AWARE_RTG] clear frame, rtgid=%d, type=%d\n",
			 proc_info.rtgid, proc_info.type);
		ret = 0;
	} else {
		pr_err("[AWARE_RTG] param is invailed!\n");
		return -INVALID_ARG;
	}

	if (is_frame_rtg(proc_info.rtgid)) {
		err = update_rtg_frame_thread(&proc_info, cmd);
		if (err)
			return err;

		reset_activity(rtg_frame_info(proc_info.rtgid));
	}

	return ret;
}

int update_frame_state(const struct rtg_data_head *rtg_head,
	int frame_type, bool in_frame)
{
	int margin;
	struct frame_info *frame_info = NULL;
	int rate;
	int id;

	if ((rtg_head == NULL) || (rtg_head->tid != current->pid))
		return -FRAME_ERR_PID;

	frame_info = lookup_frame_info_by_task(current);
	if (!frame_info || !frame_info->rtg)
		return -FRAME_ERR_PID;

	if (atomic_read(&frame_info->act_state) == ACTIVITY_BEGIN)
		return -FRAME_IS_ACT;

	id = frame_info->rtg->id;
	if (in_frame) {
		start_frame_freq(frame_info);
		rate = get_frame_rate(frame_info);
		margin = get_state_margin_by_rate(rate, frame_type);
		if (margin != INVALID_VALUE)
			set_frame(frame_info, margin, frame_type);
#ifdef CONFIG_FRAME_RTG
		trace_rtg_frame_sched(id, "rate", rate);
		trace_rtg_frame_sched(id, "margin", margin);
		trace_rtg_frame_sched(id, "frame_type", frame_type);
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
		trace_rtg_frame_sched("rate", rate);
		trace_rtg_frame_sched("margin", margin);
		trace_rtg_frame_sched("frame_type", frame_type);
#endif
	} else {
		reset_frame(frame_info);
	}
	return SUCC;
}

#ifdef CONFIG_HW_RTG_SCHED
int set_min_util(const struct rtg_data_head *rtg_head, int min_util)
{
	struct frame_info *frame_info = NULL;

	if ((rtg_head == NULL) || (rtg_head->tid != current->pid))
		return -FRAME_ERR_PID;

	frame_info = lookup_frame_info_by_task(current);
	if (!frame_info)
		return -FRAME_ERR_PID;

	if (atomic_read(&frame_info->act_state) == ACTIVITY_BEGIN)
		return -FRAME_IS_ACT;

	set_frame_min_util(frame_info, min_util, false);
	return SUCC;
}

int set_margin(const struct rtg_data_head *rtg_head, int margin)
{
	struct frame_info *frame_info = NULL;

	if ((rtg_head == NULL) || (rtg_head->tid != current->pid))
		return -FRAME_ERR_PID;

	frame_info = lookup_frame_info_by_task(current);
	if (!frame_info)
		return -FRAME_ERR_PID;

	if (atomic_read(&frame_info->act_state) == ACTIVITY_BEGIN)
		return -FRAME_IS_ACT;

	set_frame_margin(frame_info, margin);
	return SUCC;
}

int set_min_util_and_margin(const struct rtg_data_head *rtg_head,
			int min_util, int margin)
{
	struct frame_info *frame_info = NULL;

	if ((rtg_head == NULL) || (rtg_head->tid != current->pid))
		return -FRAME_ERR_PID;

	frame_info = lookup_frame_info_by_task(current);
	if (!frame_info)
		return -FRAME_ERR_PID;

	if (atomic_read(&frame_info->act_state) == ACTIVITY_BEGIN)
		return -FRAME_IS_ACT;

	set_frame_min_util_and_margin(frame_info, min_util, margin);
	return SUCC;
}
#endif

int update_act_state(const struct rtg_data_head *rtg_head, bool in_activity)
{
#if ((!defined(CONFIG_HW_QOS_THREAD)) && (!defined(CONFIG_HW_VIP_THREAD)))
	return -ACT_ERR_QOS;
#endif
	int margin;
	int id;
	int rate;
	struct frame_info *frame_info = NULL;

	if (!rtg_head)
		return -INVALID_ARG;

	frame_info = lookup_frame_info_by_pid(rtg_head->tid);
	if (!frame_info || !frame_info->rtg)
		return -FRAME_ERR_PID;

	if ((rtg_head == NULL) || (rtg_head->uid != current_uid().val) ||
	    (rtg_head->uid != atomic_read(&frame_info->uid)))
		return -ACT_ERR_UID;
#ifdef CONFIG_HW_QOS_THREAD
	if (get_qos(rtg_head->tid) < MIN_RTG_QOS)
		return -ACT_ERR_QOS;
#elif defined CONFIG_HW_VIP_THREAD
	if (get_vip(rtg_head->tid) <= 0)
		return -ACT_ERR_QOS;
#endif
	if (in_activity) {
		start_frame_freq(frame_info);
		rate = get_frame_rate(frame_info);
		margin = get_state_margin_by_rate(rate, ACTIVITY_BEGIN);
		if (margin != INVALID_VALUE)
			set_activity(frame_info, margin);
		id = frame_info->rtg->id;
#ifdef CONFIG_FRAME_RTG
		trace_rtg_frame_sched(id, "rate", rate);
		trace_rtg_frame_sched(id, "margin", margin);
		trace_rtg_frame_sched(id, "frame_type", ACTIVITY_BEGIN);
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
		trace_rtg_frame_sched("rate", rate);
		trace_rtg_frame_sched("margin", margin);
		trace_rtg_frame_sched("frame_type", ACTIVITY_BEGIN);
#endif
	} else {
		reset_activity(frame_info);
	}
	return SUCC;
}

int is_cur_frame(void)
{
	struct frame_info *frame_info = lookup_frame_info_by_task(current);

	if (frame_info && atomic_read(&frame_info->uid) == current_uid().val)
		return 1;
	return 0;
}

int stop_frame_freq(const struct rtg_data_head *rtg_head)
{
	struct frame_info *frame_info = NULL;

	if (rtg_head == NULL)
		return -INVALID_ARG;

	frame_info = lookup_frame_info_by_pid(rtg_head->tid);
	if (!frame_info)
		return -FRAME_ERR_PID;

	atomic_set(&frame_info->start_frame_freq, 0);
	set_frame_sched_state(frame_info, false);
	return 0;
}

void start_rtg_boost(void)
{
	int id = DEFAULT_RT_FRAME_ID;
	struct frame_info *frame_info = NULL;

	frame_info = rtg_frame_info(id);
	if (frame_info)
		set_frame_sched_state(frame_info, true);
#ifdef CONFIG_HW_RTG_MULTI_FRAME
	for (id = MULTI_FRAME_ID; id < (MULTI_FRAME_ID + MULTI_FRAME_NUM); id++) {
		frame_info = rtg_frame_info(id);
		if (frame_info)
			set_frame_sched_state(frame_info, true);
	}
#endif
}

bool is_rtg_sched_enable(int id)
{
	struct frame_info *frame_info = rtg_frame_info(id);

	if (!frame_info)
		return false;

	return atomic_read(&frame_info->start_frame_freq) == 1;
}

bool is_frame_start(void)
{
	struct frame_info *frame_info = NULL;

	frame_info = lookup_frame_info_by_task(current);
	if (!frame_info)
		return false;

	if ((atomic_read(&frame_info->frame_state) == FRAME_END_STATE) &&
	    (atomic_read(&frame_info->act_state) == ACTIVITY_END))
		return false;
	return true;
}

/*lint -restore*/
