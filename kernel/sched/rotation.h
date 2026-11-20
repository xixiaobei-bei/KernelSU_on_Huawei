/*
 * rotation.h
 *
 * sched running task rotation
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
#ifndef __ROTATION_H
#define __ROTATION_H

struct rq;

#ifdef CONFIG_SCHED_RUNNING_TASK_ROTATION
extern unsigned int rotation_enabled;

void rotate_work_init(int cpu);
void check_for_rotation(struct rq *rq);
void rotation_checkpoint(bool sched_avg_updated);
#else
static inline void check_for_rotation(struct rq *rq) {}
#define rotation_enabled (0);
#endif

#endif
