/*
 * Watch TP Stability Recovery
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
#ifndef __WATCH_TP_RECOVERY_H_
#define __WATCH_TP_RECOVERY_H_

#include "huawei_ts_kit.h"

struct ts_recovery_data {
	struct ts_cmd_queue queue;
	struct work_struct recovery_work;
	struct timer_list recovery_timer;
};

void release_memory(void);
int get_one_detect_cmd(struct ts_cmd_node *cmd);
int put_one_detect_cmd(struct ts_cmd_node *cmd);
void ts_recovery_init(void);
void ts_stop_recovery_timer(void);

#endif

