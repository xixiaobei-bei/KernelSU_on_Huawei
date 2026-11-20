/*
 * iolimit_cgroup.h
 *
 * Provide external call interfaces of iolimit
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _CGROUP_IOLIMIT_H
#define _CGROUP_IOLIMIT_H

#include <linux/cgroup.h>
#include <linux/atomic.h>
#include <linux/timer.h>

#ifdef CONFIG_CGROUP_IOLIMIT

struct iolimit_cgroup {
	struct cgroup_subsys_state      css;
	atomic64_t                      switching;
	atomic64_t                      write_limit;
	s64                             write_part_nbyte;
	s64                             write_already_used;
	struct timer_list               write_timer;
	spinlock_t                      write_lock;
	wait_queue_head_t               write_wait;

	atomic64_t                      read_limit;
	s64                             read_part_nbyte;
	s64                             read_already_used;
	struct timer_list               read_timer;
	spinlock_t                      read_lock;
	wait_queue_head_t               read_wait;
};

static inline struct iolimit_cgroup *css_iolimit(
	struct cgroup_subsys_state *css)
{
	return css ? container_of(css, struct iolimit_cgroup, css) : NULL;
}

static inline struct iolimit_cgroup *task_iolimitcg(struct task_struct *tsk)
{
	return css_iolimit(task_css(tsk, iolimit_cgrp_id));
}

void do_io_write_bandwidth_control(size_t count);

void do_io_read_bandwidth_control(size_t count);

static inline void io_read_bandwidth_control(size_t count)
{
	if (unlikely(!task_css_is_root(current, iolimit_cgrp_id)))
		do_io_read_bandwidth_control(count);
}

static inline void io_write_bandwidth_control(size_t count)
{
	if (unlikely(!task_css_is_root(current, iolimit_cgrp_id)))
		do_io_write_bandwidth_control(count);
}

static inline void pagefault_io_read_bandwidth_control(size_t count)
{
	task_set_in_pagefault(current);
	do_io_read_bandwidth_control(count);
	task_clear_in_pagefault(current);
}

static inline void io_generic_read_bandwidth_control(size_t count)
{
	if (unlikely(!task_css_is_root(current, iolimit_cgrp_id)))
		pagefault_io_read_bandwidth_control(count);
}
#else  /* !CONFIG_CGROUP_IOLIMIT */

static inline void io_write_bandwidth_control(size_t count)
{
}

static inline void io_read_bandwidth_control(size_t count)
{
}

static inline void io_generic_read_bandwidth_control(size_t count)
{
}
#endif /* !CONFIG_CGROUP_IOLIMIT */

#endif /* _CGROUP_IOLIMIT_H */

