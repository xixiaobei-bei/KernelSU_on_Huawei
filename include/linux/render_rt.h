/*
 * render_rt.h
 *
 * Copyright (c) 2019, Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __RENDER_RT_H
#define __RENDER_RT_H

#define MAX_TID_COUNT 512

struct related_tid_info {
	/* pid of query task */
	pid_t pid;
	/* Obtain tid count */
	int rel_count;
	/* Obtain tid array */
	pid_t rel_tid[MAX_TID_COUNT];
};

/*
 * pid == 0 indicates invalid entry.
 * util should be in range of 0 to 1024
 */
struct thread_util {
	pid_t pid;
	unsigned long util;
	unsigned int woken_count;
};

#define MAX_THREAD_NUM 21
struct render_rt {
	pid_t render_pid;
	int num;
	struct thread_util utils[MAX_THREAD_NUM];
};

struct render_ht {
	pid_t render_pid;
	struct thread_util utils[MAX_THREAD_NUM];
};

struct render_init_paras {
	pid_t render_pid;
	bool force_init;
};

struct render_stop {
	pid_t render_pid;
	int stopped;
};

#ifdef CONFIG_RENDER_RT
void add_render_rthread(struct task_struct *task);
void remove_render_rthread(struct task_struct *task);
void add_waker_to_render_rthread(struct task_struct *task);
bool render_rt_inited(void);

/* perf ctrl call interfaces */
int init_render_rthread(void __user *uarg);
int destroy_render_rthread(void __user *uarg);
int stop_render_rthread(void __user *uarg);
int get_render_rthread(void __user *uarg);
int get_render_hrthread(void __user *uarg);
#else
static inline void add_waker_to_render_rthread(struct task_struct *task)
{
	return;
}
static inline void add_render_rthread(struct task_struct *task)
{
	return;
}
static inline void remove_render_rthread(struct task_struct *task)
{
	return;
}
static inline bool render_rt_inited(void)
{
	return false;
}

/* perf ctrl call interfaces */
static inline int init_render_rthread(void __user *uarg)
{
	return -ENODEV;
}
static inline int destroy_render_rthread(void __user *uarg)
{
	return -ENODEV;
}
static inline int stop_render_rthread(void __user *uarg)
{
	return -ENODEV;
}
static inline int get_render_rthread(void __user *uarg)
{
	return -ENODEV;
}
static inline int get_render_hrthread(void __user *uarg)
{
	return -ENODEV;
}
#endif

#endif /* __RENDER_RT_H  */
