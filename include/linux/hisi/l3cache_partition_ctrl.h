/*
 * l3cache_partition_ctrl.h
 *
 * header of l3cache_partition_ctrl
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

#ifndef __L3CACHE_PARTITION_CTRL_H__
#define __L3CACHE_PARTITION_CTRL_H__
#include <linux/types.h>

#ifdef CONFIG_L3CACHE_PARTITION_CTRL
void set_partition_control(unsigned int val);
int perf_ctrl_set_task_l3c_part(void __user *uarg);
void set_task_partition_control(unsigned int val, unsigned int cluster);
#else
static inline void set_partition_control(unsigned int val) {}

static inline int perf_ctrl_set_task_l3c_part(void __user *uarg)
{
	return -EFAULT;
}

static inline void set_task_partition_control(unsigned int val, unsigned int cluster) {}
#endif

#endif /* __L3CACHE_PARTITION_CTRL_H__ */
