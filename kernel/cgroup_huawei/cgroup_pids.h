/*
 * cgroup_pids.h
 *
 * Process number limiting controller header file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef CGROUP_PIDS_H_INCLUDED
#define CGROUP_PIDS_H_INCLUDED

#ifdef CONFIG_HW_CGROUP_PIDS

int cgroup_pids_can_fork(void);
void cgroup_pids_cancel_fork(void);

#else /* CONFIG_HW_CGROUP_PIDS */

static inline int cgroup_pids_can_fork(void)
{
	return 0;
}

static inline void cgroup_pids_cancel_fork(void)
{
}

#endif /* CONFIG_HW_CGROUP_PIDS */

#endif /* CGROUP_PIDS_H_INCLUDED */
