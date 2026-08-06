/*
 * proc_state.c
 *
 * proc state manager
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "include/proc_state.h"
#include "include/set_rtg.h"
#include <linux/atomic.h>
#include <linux/hisi_rtg.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/sched/frame.h>
#include <linux/spinlock.h>

#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#define FRAME_START			(1 << 0)
#define FRAME_END			(1 << 1)
#define FRAME_USE_MARGIN_IMME		(1 << 4)
#define FRAME_TIMESTAMP_SKIP_START	(1 << 5)
#define FRAME_TIMESTAMP_SKIP_END	(1 << 6)

#define MIN_RTG_QOS 2
#define DEFAULT_FREQ_CYCLE 4
#define DEFAULT_MAX_UTIL 1024
#define MIN_FREQ_CYCLE 4
#define MAX_FREQ_CYCLE 16
#define DEFAULT_INVALID_INTERVAL 50

static struct state_margin_list *g_margin_list;
static rwlock_t g_state_margin_lock;

static atomic_t g_rtg_uid;
static atomic_t g_rtg_pid;
static atomic_t g_rtg_tid;

static atomic_t g_act_state;
static atomic_t g_frame_state;

static int g_frame_max_util = DEFAULT_MAX_UTIL;
static int g_act_max_util = DEFAULT_MAX_UTIL;

static void set_rtg_thread(pid_t tid, bool is_rtg)
{
	struct task_struct *task;

	if (tid <= 0)
		return;
	rcu_read_lock();
	task = find_task_by_vpid(tid);
	if (task == NULL) {
		pr_err("[AWARE_RTG] %s tid:%d task is null\n",
				__func__, tid);
		rcu_read_unlock();
		return;
	}
	get_task_struct(task);
	rcu_read_unlock();

	if (set_rtg_sched(task, is_rtg) < 0) {
		put_task_struct(task);
		return;
	}
	if (is_rtg)
		task->rtg_depth = STATIC_RTG_DEPTH;
	put_task_struct(task);
}

/*lint -save -e446 -e666 -e734*/
static void set_frame(int margin, int frame_type)
{
	atomic_set(&g_frame_state, frame_type);
	if (set_frame_margin(margin) == SUCC)
		set_frame_timestamp(FRAME_START);
}

static void reset_frame(void)
{
	if (atomic_read(&g_frame_state) == FRAME_END_STATE) {
		pr_debug("[AWARE_RTG]: Frame state is already reset\n");
		return;
	}
	atomic_set(&g_frame_state, FRAME_END_STATE);
	set_frame_timestamp(FRAME_END);
}

static void set_activity(int margin)
{
	pr_debug("[AWARE_RTG]: %s margin=%d\n", __func__, margin);
	atomic_set(&g_act_state, ACTIVITY_BEGIN);
	if (set_frame_margin(margin) == SUCC) {
		set_frame_max_util(g_act_max_util);
		set_frame_timestamp(FRAME_START | FRAME_USE_MARGIN_IMME |
			FRAME_TIMESTAMP_SKIP_START);
	}
}

static void reset_activity(void)
{
	if (atomic_read(&g_act_state) == ACTIVITY_END) {
		pr_debug("[AWARE_RTG]: Activity state is already reset\n");
		return;
	}
	pr_debug("[AWARE_RTG]: %s\n", __func__);
	atomic_set(&g_act_state, ACTIVITY_END);
	set_frame_max_util(g_frame_max_util);
	set_frame_timestamp(FRAME_END | FRAME_TIMESTAMP_SKIP_END);
}

/*lint -e429*/
static int put_state_margin(const char *state_str, const char *margin_str)
{
	struct state_margin_list *temp;
	int state = STATE_MAX;
	int margin;
	bool minus = false;

	if (state_str == NULL || margin_str == NULL)
		return -1;

	if (*margin_str == '-') {
		minus = true;
		if (strlen(margin_str) <= 1)
			return -1;
		margin_str++;
	}
	if (kstrtouint((const char *)margin_str, DECIMAL, &margin))
		return -1;
	if (minus)
		margin = 0 - margin;

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

	if (state <= STATE_MIN || state >= STATE_MAX)
		return -1;

	temp = kmalloc(sizeof(*temp), GFP_ATOMIC);
	if (temp == NULL)
		return -1;

	temp->state = state;
	temp->margin = margin;
	pr_info("[AWARE_RTG]: %s state=%d, margin=%d", __func__, state, margin);

	write_lock(&g_state_margin_lock);
	list_add_tail(&(temp->list), &(g_margin_list->list));
	write_unlock(&g_state_margin_lock);
	return 0;
}
/*lint +e429*/

static int get_state_margin(int state)
{
	struct state_margin_list *temp;
	struct list_head *pos;
	struct list_head *n;
	int margin = INVALID_VALUE;

	read_lock(&g_state_margin_lock);
	if (g_margin_list == NULL) {
		read_unlock(&g_state_margin_lock);
		return margin;
	}
	list_for_each_safe(pos, n, &(g_margin_list->list)) {
		temp = list_entry(pos, struct state_margin_list, list);
		if (temp->state == state) {
			margin = temp->margin;
			break;
		}
	}
	read_unlock(&g_state_margin_lock);
	return margin;
}

#ifdef CONFIG_HW_QOS_THREAD
static int get_qos(pid_t tid)
{
	int qos = -1;
	struct task_struct *tsk;

	if (atomic_read(&g_rtg_uid) != current_uid().val)
		return qos;

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
	struct task_struct *tsk;

	if (atomic_read(&g_rtg_uid) != current_uid().val)
		return vip;

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

static void update_rtg_thread(const struct rtg_data_head *proc_info)
{
	int uid = proc_info->uid;
	int pid = proc_info->pid;
	int tid = proc_info->tid;
	if (uid == -1 || pid == -1 || tid == -1) {
		atomic_set(&g_rtg_uid, -1);
		set_rtg_thread(atomic_read(&g_rtg_pid), false);
		atomic_set(&g_rtg_pid, -1);
		set_rtg_thread(atomic_read(&g_rtg_tid), false);
		atomic_set(&g_rtg_tid, -1);
		return;
	}

	if (uid != atomic_read(&g_rtg_uid))
		atomic_set(&g_rtg_uid, uid);

	if (pid != atomic_read(&g_rtg_pid)) {
		set_rtg_thread(atomic_read(&g_rtg_pid), false);
		atomic_set(&g_rtg_pid, pid);
		set_rtg_thread(atomic_read(&g_rtg_pid), true);
	}
	if (tid != atomic_read(&g_rtg_tid)) {
		set_rtg_thread(atomic_read(&g_rtg_tid), false);
		atomic_set(&g_rtg_tid, tid);
		set_rtg_thread(atomic_read(&g_rtg_tid), true);
	}
	pr_debug("[AWARE_RTG] %s uid=%d, pid=%d, tid=%d\n",
			__func__, atomic_read(&g_rtg_uid),
			atomic_read(&g_rtg_pid), atomic_read(&g_rtg_tid));
}

static void free_margin_list(void)
{
	struct state_margin_list *temp;
	struct list_head *pos;
	struct list_head *n;

	write_lock(&g_state_margin_lock);
	if (g_margin_list == NULL) {
		write_unlock(&g_state_margin_lock);
		return;
	}
	list_for_each_safe(pos, n, &(g_margin_list->list)) {
		temp = list_entry(pos, struct state_margin_list, list);
		list_del(&temp->list);
		kfree(temp);
	}
	kfree(g_margin_list);
	g_margin_list = NULL;
	write_unlock(&g_state_margin_lock);
}

int init_proc_state(int *config)
{
	if (config == NULL)
		return -INVALID_ARG;
	rwlock_init(&g_state_margin_lock);

	write_lock(&g_state_margin_lock);
	g_margin_list = kmalloc(sizeof(*g_margin_list), GFP_ATOMIC);
	if (g_margin_list == NULL) {
		write_unlock(&g_state_margin_lock);
		return -RTG_DISABLED;
	}
	INIT_LIST_HEAD(&(g_margin_list->list));
	g_margin_list->state = STATE_MIN;
	g_margin_list->margin = INVALID_VALUE;
	write_unlock(&g_state_margin_lock);

	atomic_set(&g_rtg_uid, -1);
	atomic_set(&g_rtg_pid, -1);
	atomic_set(&g_rtg_tid, -1);
	atomic_set(&g_act_state, -1);
	atomic_set(&g_frame_state, -1);

	if (config[RTG_LOAD_FREQ] != INVALID_VALUE)
		sched_set_group_load_mode(DEFAULT_RT_FRAME_ID,
				!config[RTG_LOAD_FREQ], 1);

	if (config[RTG_FREQ_CYCLE] >= MIN_FREQ_CYCLE &&
			config[RTG_FREQ_CYCLE] <= MAX_FREQ_CYCLE)
		sched_set_group_freq_update_interval(DEFAULT_RT_FRAME_ID,
				(unsigned int)config[RTG_FREQ_CYCLE]);
	else
		sched_set_group_freq_update_interval(DEFAULT_RT_FRAME_ID,
				DEFAULT_FREQ_CYCLE);

	if (config[RTG_INVALID_INTERVAL] != INVALID_VALUE)
		sched_set_group_util_invalid_interval(DEFAULT_RT_FRAME_ID,
				config[RTG_INVALID_INTERVAL]);
	else
		sched_set_group_util_invalid_interval(DEFAULT_RT_FRAME_ID,
				DEFAULT_INVALID_INTERVAL);

	if (config[RTG_FRAME_MAX_UTIL] > 0 &&
			config[RTG_FRAME_MAX_UTIL] < DEFAULT_MAX_UTIL)
		g_frame_max_util = config[RTG_FRAME_MAX_UTIL];
	if (config[RTG_ACT_MAX_UTIL] > 0 &&
			config[RTG_ACT_MAX_UTIL] < DEFAULT_MAX_UTIL)
		g_act_max_util = config[RTG_ACT_MAX_UTIL];

	set_frame_max_util(g_frame_max_util);

	return SUCC;
}

void deinit_proc_state(void)
{
	free_margin_list();
	atomic_set(&g_rtg_uid, -1);
	atomic_set(&g_rtg_pid, -1);
	atomic_set(&g_rtg_tid, -1);
	atomic_set(&g_act_state, -1);
	atomic_set(&g_frame_state, -1);
}

int parse_config(const struct rtg_str_data *rs_data)
{
	int len;
	char *p;
	char *tmp;
	char *data;

	if (rs_data == NULL)
		return -INVALID_ARG;
	data = rs_data->data;
	len = rs_data->len;
	if (data == NULL || strlen(data) != len) //lint !e737
		return -INVALID_ARG;

	// eg: margin0:-1;margin1:0;margin2:1;margin3:2;margin4:3
	p = strsep(&data, ";");
	while (p != NULL) {
		tmp = strsep(&p, ":");
		if (put_state_margin(tmp, p)) {
			pr_err("[AWARE_RTG] parse config failed\n");
			return -INVALID_ARG;
		}
		p = strsep(&data, ";");
	}
	return SUCC;
}

int parse_rtg_thread(const struct rtg_str_data *rs_data)
{
	char *p;
	char *tmp;
	char *data;
	int id;
	struct rtg_data_head proc_info;
	bool minus;

	if (rs_data == NULL)
		return -INVALID_ARG;
	data = rs_data->data;
	if (data == NULL || rs_data->len <= 0 || rs_data->len > MAX_DATA_LEN)
		return -INVALID_ARG;

	proc_info.uid = 0;
	proc_info.pid = 0;
	proc_info.tid = 0;
	// eg: fgUid:10000;uiTid:10000;renderTid:10000
	for (p = strsep(&data, ";"); p != NULL; p = strsep(&data, ";")) {
		tmp = strsep(&p, ":");
		if (tmp == NULL || p == NULL)
			continue;
		minus = false;
		if (*p == '-') {
			minus = true;
			if (strlen(p) <= 1)
				continue;
			p++;
		}
		if (kstrtouint((const char *)p, DECIMAL, &id))
			return -INVALID_ARG;
		if (minus)
			id = 0 - id;
		if (!strcmp(tmp, "fgUid")) {
			proc_info.uid = id;
		} else if (!strcmp(tmp, "uiTid")) {
			proc_info.pid = id;
		} else if (!strcmp(tmp, "renderTid")) {
			proc_info.tid = id;
		} else {
			pr_err("[AWARE_RTG] parse config failed!\n");
			return -INVALID_ARG;
		}
	}

	update_rtg_thread(&proc_info);
	reset_activity();
	return SUCC;
}

int update_frame_state(const struct rtg_data_head *rtg_head,
						int frame_type, bool in_frame)
{
	int margin;

	if (rtg_head == NULL || rtg_head->tid != current->pid)
		return -FRAME_ERR_PID;

	if (atomic_read(&g_act_state) == ACTIVITY_BEGIN)
		return -FRAME_IS_ACT;

	if (rtg_head->tid != atomic_read(&g_rtg_pid) &&
		rtg_head->tid != atomic_read(&g_rtg_tid))
		return -FRAME_ERR_TID;

	if (in_frame) {
		margin = get_state_margin(frame_type);
		if (margin != INVALID_VALUE)
			set_frame(margin, frame_type);
	} else {
		reset_frame();
	}
	return SUCC;
}

int update_act_state(const struct rtg_data_head *rtg_head, bool in_activity)
{
#if ((!defined(CONFIG_HW_QOS_THREAD)) && (!defined(CONFIG_HW_VIP_THREAD)))
	return -ACT_ERR_QOS;
#endif
	int margin;

	if (rtg_head == NULL || rtg_head->uid != current_uid().val ||
		rtg_head->uid != atomic_read(&g_rtg_uid))
		return -ACT_ERR_UID;

#ifdef CONFIG_HW_QOS_THREAD
	if (get_qos(rtg_head->tid) < MIN_RTG_QOS)
		return -ACT_ERR_QOS;
#elif defined CONFIG_HW_VIP_THREAD
	if (get_vip(rtg_head->tid) <= 0)
		return -ACT_ERR_QOS;
#endif

	if (in_activity) {
		margin = get_state_margin(ACTIVITY_BEGIN);
		if (margin != INVALID_VALUE)
			set_activity(margin);
	} else {
		reset_activity();
	}
	return SUCC;
}

int is_cur_frame(void)
{
	if (atomic_read(&g_rtg_uid) != current_uid().val)
		return 0;
	if (is_frame_task(current))
		return 1;
	return 0;
}
/*lint -restore*/
