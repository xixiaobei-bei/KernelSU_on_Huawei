/*
 * proc_state.h
 *
 * proc state manager header
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

#ifndef PROC_STATE_H
#define PROC_STATE_H

#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#define MAX_DATA_LEN 256
#define DECIMAL 10

enum proc_state {
	STATE_MIN = 0,
	FRAME_BUFFER_0,
	FRAME_BUFFER_1,
	FRAME_BUFFER_2,
	FRAME_END_STATE,
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
	FRAME_ERR_PID = 100,
	FRAME_ERR_TID,
	ACT_ERR_UID,
	ACT_ERR_QOS,
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

struct rtg_enable_data {
	struct rtg_data_head head;
	int enable;
	int len;
	char *data;
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

int init_proc_state(int *config);
void deinit_proc_state(void);
int parse_config(const struct rtg_str_data *rs_data);
int parse_rtg_thread(const struct rtg_str_data *rs_data);
int update_frame_state(const struct rtg_data_head *rtg_head,
			int frame_type, bool in_frame);
int update_act_state(const struct rtg_data_head *rtg_head, bool in_activity);
int is_cur_frame(void);

#endif
