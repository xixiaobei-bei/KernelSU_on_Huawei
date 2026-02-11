/*
 * hwqos_fork.h
 *
 * Qos schedule declaration
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

#ifndef HWQOS_FORK_H
#define HWQOS_FORK_H

struct task_struct;

void init_task_qos_info(struct task_struct *p);
void release_task_qos_info(struct task_struct *p);
#endif
