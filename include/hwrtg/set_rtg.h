/*
 * set_rtg.h
 *
 * set rtg sched header
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

#ifndef SET_RTG_H
#define SET_RTG_H

#include <linux/sched.h>
#include <linux/types.h>

#define STATIC_RTG_DEPTH (-1)
#define INVALID_VALUE 0xffff

int set_rtg_sched(struct task_struct *task, bool is_rtg);

#endif
