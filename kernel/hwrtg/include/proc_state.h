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
 * proc state manager header
 */

#ifndef PROC_STATE_H
#define PROC_STATE_H

#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#ifdef CONFIG_HW_RTG_SCHED
#include <linux/sched/frame_info.h>
#endif
#ifdef CONFIG_HW_MTK_RTG_SCHED
#include <mtkrtg/frame_info.h>
#endif

#define MAX_DATA_LEN 256
#define DECIMAL 10
#define DEFAULT_MAX_UTIL 1024

#define RTG_ID_INVALID (-1)
#define DEFAULT_FREQ_CYCLE 4
#define MIN_FREQ_CYCLE 4
#define MAX_FREQ_CYCLE 16
#define DEFAULT_INVALID_INTERVAL 50
#define DEFAULT_MAX_RT_THREAD 2

enum proc_state {
	STATE_MIN = 0,
	FRAME_BUFFER_0,
	FRAME_BUFFER_1,
	FRAME_BUFFER_2,
	/* From 4 to 19 used by rme, do not use these type */
	FRAME_INPUT = 4,
	FRAME_ANIMATION,
	FRAME_TRAVERSAL,
	FRAME_RME_MAX = 19,
	/* rme end */
	FRAME_END_STATE = FRAME_RME_MAX + 1,
	ACTIVITY_BEGIN,
	ACTIVITY_END,
	FRAME_CLICK = 100,
	STATE_MAX,
};

enum rtg_config {
	RTG_LOAD_FREQ = 0,
	RTG_FREQ_CYCLE,
	RTG_TRANS_DEPTH,
	RTG_MAX_THREADS,
	RTG_FRAME_MAX_UTIL,
	RTG_ACT_MAX_UTIL,
	RTG_INVALID_INTERVAL,
	RTG_CONFIG_NUM,
};

enum rtg_err_no {
	SUCC = 0,
	RTG_DISABLED = 1,
	INVALID_ARG,
	INVALID_MAGIC,
	INVALID_CMD,
	NOT_SYSTEM_UID,
	FRAME_IS_ACT,
	NO_PERMISSION,
	OPEN_ERR_UID,
	OPEN_ERR_TID,
	ERR_RTG_BOOST_ARG,
	FRAME_ERR_PID = 100,
	FRAME_ERR_TID,
	ACT_ERR_UID,
	ACT_ERR_QOS,
	NO_FREE_MULTI_FRAME,
	NOT_MULTI_FRAME,
	INVALID_RTG_ID,
	NO_RT_FRAME,
	ERR_SET_AUX_RTG = 200,
	ERR_SCHED_AUX_RTG,
};

enum rme_buffer {
	PING = 0,
	PONG,
	MAX_BUF_SIZE
};

enum fps_state {
	INIT_STATE = 0,
	WRITING,
	READING
};

struct state_margin_list {
	struct list_head list;
	int state;
	int margin;
};

struct rtg_data_head {
	pid_t uid;
	pid_t pid;
	pid_t tid;
};

struct rtg_cfs_data_head {
	pid_t pid;
	pid_t cfs_tid[MAX_FRAME_CFS_THREADS];
	int type;
};

struct rtg_enable_data {
	struct rtg_data_head head;
	int enable;
	int len;
	char *data;
};

struct rtg_proc_data {
	struct rtg_data_head head;
	int rtgid;
	int type;
	int thread[MAX_TID_NUM];
	int rtcnt;
};

struct rtg_str_data {
	struct rtg_data_head head;
	int len;
	char *data;
};

struct proc_state_data {
	struct rtg_data_head head;
	int frame_freq_type;
};

struct rtg_boost_data {
	struct rtg_data_head head;
	int duration;
	int min_util;
};

struct min_util_margin_data {
	struct rtg_data_head head;
	int min_util;
	int margin;
};

struct rme_fps_data {
	int deadline_margin;
	int input_margin;
	int animation_margin;
	int traversal_margin;
	int input_period;
	int animation_period;
	int traversal_period;
	int default_util;
	int frame_rate;
	/* util clamp, not used */
	int min_prev_util;
	int max_prev_util;
};

int init_proc_state(const int *config, int len);
void deinit_proc_state(void);
int parse_config(const struct rtg_str_data *rs_data);
#ifdef CONFIG_HW_RTG_MULTI_FRAME
int parse_rtg_attr(const struct rtg_str_data *rs_data);
#else
static inline int parse_rtg_attr(const struct rtg_str_data *rs_data)
{
	return -EFAULT;
}
#endif
void set_boost_thread_min_util(int min_util);
int parse_boost_thread(const struct rtg_str_data *rs_data);
int parse_frame_thread(const struct rtg_str_data *rs_data);
int parse_aux_thread(const struct rtg_str_data *rs_data);
int update_frame_state(const struct rtg_data_head *rtg_head,
	int frame_type, bool in_frame);
int update_act_state(const struct rtg_data_head *rtg_head, bool in_activity);
int is_cur_frame(void);
int stop_frame_freq(const struct rtg_data_head *rtg_head);
void start_rtg_boost(void);
bool is_rtg_sched_enable(int id);
bool is_frame_start(void);
int parse_aux_comm_config(const struct rtg_str_data *rs_data);
bool is_key_aux_comm(const struct task_struct *task, const char *comm);

#ifdef CONFIG_HW_RTG_SCHED
int set_min_util(const struct rtg_data_head *rtg_head, int min_util);
int set_margin(const struct rtg_data_head *rtg_head, int margin);
int set_min_util_and_margin(const struct rtg_data_head *rtg_head,
	int min_util, int margin);
int parse_frame_cfs_thread(const struct rtg_str_data *rs_data);
#else
static inline int set_min_util(const struct rtg_data_head *rtg_head, int min_util) {return 0;}
static inline int set_margin(const struct rtg_data_head *rtg_head, int margin) {return 0;}
static inline int set_min_util_and_margin(const struct rtg_data_head *rtg_head,
	int min_util, int margin) {return 0;}
static inline int parse_frame_cfs_thread(const struct rtg_str_data *rs_data)
{
	return 0;
}
#endif

#endif
