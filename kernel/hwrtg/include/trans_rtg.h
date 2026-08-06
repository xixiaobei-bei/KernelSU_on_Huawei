/*
 * trans_rtg.h
 *
 * trans rtg thread header
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

#ifndef TRANS_RTG_H
#define TRANS_RTG_H

#include "set_rtg.h"
#include <linux/sched.h>
#include <linux/types.h>

#define DEFAULT_TRANS_DEPTH (-100)
#define DEFAULT_MAX_THREADS (-100)

void set_trans_config(int depth, int max_threads);
void add_trans_thread(struct task_struct *target, struct task_struct *from);
void remove_trans_thread(struct task_struct *target);

#endif
