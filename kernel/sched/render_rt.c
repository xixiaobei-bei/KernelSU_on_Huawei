/*
 * render_rt.c
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "render_rt: " fmt

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/render_rt.h>
#include "sched.h"
#include "walt.h"
#include <securec.h>

#define CREATE_TRACE_POINTS
#include <trace/events/render_rt.h>

/*
 * g_rt_rwlock --the global lock for struct render_related_thread
 *
 * write_lock: while modifying the fileds of struct render_related_thread,
 * read_lock: while referencing the files of struct render_related_thread.
 * But there is a exception for performance, when modifying the
 * fields of structure child_thread, we only hold the read_lock, As holding
 * child_thread.lock to make sure concurrent aceessing the child_thread safely.
 *
 * A rule must be observed when holding the lock:
 * Do not call any function that causes wakeup, such as, kmalloc/printk etc
 *
 * If you violate the rule, there may be a deadlock.
 */
static DEFINE_RWLOCK(g_rt_rwlock);

/* each child_thread could have MAX_WAKER_COUNT waker at most */
#define MAX_WAKER_COUNT	8

/*
 * child_thread: used to save the wakeup relationship
 * @pid: the pid of the child_thread
 * @task: pointer to task_struct of the child_thread
 * @lock: protect the fields in the structure
 * @visited: flag to use in BFS procedure
 * @exited:  flag to mark the task is destroyed
 * @frame_demand: the normalized util for last frame
 * @last_walt_ravg_sum: the normalized load of the last frame
 * @waker_bitmap: bitmap used the waker's index
 * @waker_count: total count of the waker tasks
 * @waker_array: save the waker's index of ta_child_threads array
 * @woken_count: times of being woken by a related thread
 */
struct child_thread {
	pid_t pid;
	struct task_struct *task;

	raw_spinlock_t lock;
	/* fields update every frame */
	bool visted;
	bool exited;
	u32 frame_demand;
	u64 last_walt_ravg_sum;
	DECLARE_BITMAP(waker_bitmap, MAX_TID_COUNT);
	int waker_count;
	int waker_array[MAX_WAKER_COUNT];
	unsigned int woken_count;
};

/*
 * render_related_thread: used to save the wakeup relationship
 * @pid: the pid of the render
 * @ta_grp_task: pointer to task_struct of the render
 * @render_index: index of ta_child_threads array
 * @window_start: frame start timestamp
 * @total_count: the number of the child threads
 * @exited_thread_nr: the exited thread number in the last frame
 * @stopped: flag to represent related thread funciton is stopped
 * @ta_child_threads: save all the tasks in the task group of render
 */
struct render_related_thread {
	/* render thread info */
	pid_t pid;
	struct task_struct *ta_grp_task;
	int render_index;

	u64 window_start;
	int total_count;
	u16 exited_thread_nr;
	bool stopped;
	struct child_thread ta_child_threads[MAX_TID_COUNT];
};

/* g_ta_render_ptr: pointer to global data of the module */
static struct render_related_thread *g_ta_render_ptr __read_mostly;

/*
 * g_ta_render: global variable to hold the data
 *
 * Using global variable instead of dynamic allocating to enache the performance
 */
static struct render_related_thread g_ta_render;

static inline bool inited(void)
{
	if (g_ta_render_ptr == NULL || g_ta_render_ptr->ta_grp_task == NULL)
		return false;

	return true;
}

bool render_rt_inited(void)
{
	unsigned long flags;
	bool initialized = false;

	read_lock_irqsave(&g_rt_rwlock, flags);
	initialized = inited();
	read_unlock_irqrestore(&g_rt_rwlock, flags);

	return initialized;
}

/*
 * both_render_rthread() - test if the two tasks are render_rthread
 * @a: pointer to one of the two tasks
 * @b: pointer to one of the two tasks
 */
static inline bool both_render_rthread(struct task_struct *a,
				       struct task_struct *b)
{
	return same_thread_group(a, g_ta_render_ptr->ta_grp_task) &&
	       same_thread_group(b, g_ta_render_ptr->ta_grp_task);
}

/*
 * get_child_rthread_index() - binary search the index of the task
 * @task: task to search in ta_child_threads array
 *
 * Returns MAX_TID_COUNT if the @task cannot found
 */
static inline int get_child_rthread_index(struct task_struct *task)
{
	struct child_thread *threads = g_ta_render_ptr->ta_child_threads;
	int head, tail, pivot;
	pid_t target_pid = task->pid;

	head = 0;
	tail = g_ta_render_ptr->total_count - 1;

	while (tail >= head) {
		pivot = (head + tail) / 2;
		if (threads[pivot].pid == target_pid)
			return pivot;
		else if (threads[pivot].pid < target_pid)
			head = pivot + 1;
		else if (threads[pivot].pid > target_pid)
			tail = pivot - 1;
	}

	return MAX_TID_COUNT;
}

/*
 * bsearch_insert_index() - binary search the insert pos index of the task
 * @task: task to search in ta_child_threads array
 *
 * Returns MAX_TID_COUNT if the @task found in the array
 * Returns INT_MAX if cannot found the insert pos
 */
static int bsearch_insert_index(struct task_struct *task)
{
	struct child_thread *threads = g_ta_render_ptr->ta_child_threads;
	int head, tail, pivot;
	pid_t target_pid = task->pid;

	head = 0;
	tail = g_ta_render_ptr->total_count - 1;

	while (tail >= head) {
		pivot = (head + tail) / 2;

		if (threads[pivot].pid == target_pid)
			return MAX_TID_COUNT;
		else if (pivot > 0 &&
			 (target_pid > threads[pivot - 1].pid &&
			  target_pid < threads[pivot].pid))
			return pivot;
		else if (threads[pivot].pid < target_pid)
			head = pivot + 1;
		else if (threads[pivot].pid > target_pid)
			tail = pivot - 1;
	}

	return INT_MAX;
}

static inline void fix_child_rthread_waker(int threshold)
{
	int total_count = g_ta_render_ptr->total_count;
	struct child_thread *thread = NULL;
	int index, waker_index, waker_bit_index;

	for (index = 0; index < total_count; index++) {
		thread = &g_ta_render_ptr->ta_child_threads[index];

		if (thread == NULL || thread->waker_count == 0)
			continue;

		for (waker_index = 0; waker_index < thread->waker_count; waker_index++) {
			waker_bit_index = thread->waker_array[waker_index];
			if (waker_bit_index < threshold)
				continue;

			__clear_bit(waker_bit_index, thread->waker_bitmap);

			waker_bit_index++;
			__set_bit(waker_index, thread->waker_bitmap);
			thread->waker_array[waker_index] = waker_bit_index;
		}
	}
}

static inline void init_child_rthread(int index, struct task_struct *p);

/*
 * add_render_rthread() - add task to render task group ta_child_threads
 *
 * Add the task to sorted ta_child_threads array
 */
void add_render_rthread(struct task_struct *task)
{
	struct child_thread *rthreads = NULL;
	int i, j, last_index;
	unsigned long flags;
	int index = -1;
	int reason = 0;

	if (oops_in_progress || task == NULL)
		return;

	write_lock_irqsave(&g_rt_rwlock, flags);

	if (!inited())
		goto err;

	if (!same_thread_group(task, g_ta_render_ptr->ta_grp_task))
		goto err;

	if (g_ta_render_ptr->total_count == MAX_TID_COUNT) {
		reason = 1;
		goto err;
	}

	index = get_child_rthread_index(task);
	if (index != MAX_TID_COUNT) {
		/* it has already been added */
		rthreads = g_ta_render_ptr->ta_child_threads;
		if (rthreads[index].exited) {
			init_child_rthread(index, task);
			goto unlock;
		} else {
			reason = 2;
			goto err;
		}
	}

	/* add task to g_ta_render_ptr->ta_child_threads array */
	rthreads = g_ta_render_ptr->ta_child_threads;
	last_index = g_ta_render_ptr->total_count - 1;
	if (last_index < 0) {
		reason = 3;
		goto err;
	}

	if (rthreads[last_index].pid < task->pid) {
		init_child_rthread(g_ta_render_ptr->total_count, task);
	} else if (rthreads[0].pid > task->pid) {
		for (j = last_index; j >= 0; j--) {
			rthreads[j + 1] = rthreads[j];
			if (rthreads[j].pid == g_ta_render_ptr->pid)
				g_ta_render_ptr->render_index++;
		}

		init_child_rthread(0, task);
		fix_child_rthread_waker(0);
	} else {
		i = bsearch_insert_index(task);
		if (i == MAX_TID_COUNT || i == INT_MAX) {
			reason = 4;
			goto err;
		}

		/* insert pos is i, move the i..last_index afterwards */
		for (j = last_index; j >= i; j--) {
			rthreads[j + 1] = rthreads[j];
			if (rthreads[j].pid == g_ta_render_ptr->pid)
				g_ta_render_ptr->render_index++;
		}

		init_child_rthread(i, task);
		fix_child_rthread_waker(i);
	}

	g_ta_render_ptr->total_count++;

unlock:
	trace_add_render_rthread(task);

	write_unlock_irqrestore(&g_rt_rwlock, flags);

	return;

err:
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (reason)
		printk_ratelimited(KERN_WARNING "Error [%s %d] reason=%d index=%d\n",
				   __func__, __LINE__, reason, index);
}

static inline void update_rthread_util(struct child_thread *thread,
				       u64 window_size);

/*
 * remove_render_rthread()
 * mark the task is exited and update the util of the task
 *
 * just set the flag to indicates the task is exited,
 * the actual remove operation to be done in remove_exited_render_rthread function.
 */
void remove_render_rthread(struct task_struct *task)
{
	struct child_thread *exited_thread = NULL;
	u64 curr_ktime, window_size;
	unsigned long flags;
	int index;
	int reason = 0;

	if (oops_in_progress || task == NULL)
		return;

	write_lock_irqsave(&g_rt_rwlock, flags);

	if (!inited())
		goto err;

	/*
	 * This module can't work as expected when the leader thread or
	 * the render thread exit. Uninitialize itself in such cases.
	 */
	if (task == g_ta_render_ptr->ta_grp_task ||
	    task->pid == g_ta_render_ptr->pid) {
		g_ta_render_ptr = NULL;
		reason = 1;
		goto err;
	}

	if (!same_thread_group(task, g_ta_render_ptr->ta_grp_task))
		goto err;

	index = get_child_rthread_index(task);
	if (index == MAX_TID_COUNT) {
		reason = 2;
		goto err;
	}

	exited_thread = &g_ta_render_ptr->ta_child_threads[index];
	if (unlikely(exited_thread == NULL)) {
		reason = 3;
		goto err;
	}

	if (exited_thread->exited) {
		reason = 4;
		goto err;
	}

	curr_ktime = walt_ktime_clock();
	window_size = curr_ktime - g_ta_render_ptr->window_start;
	update_rthread_util(exited_thread, window_size);

	exited_thread->exited = true;
	trace_remove_render_rthread(task);

	write_unlock_irqrestore(&g_rt_rwlock, flags);

	return;
err:
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (reason)
		printk_ratelimited(KERN_WARNING "[%s] fail to find the task"
				   "reason=%d comm=%s pid=%d\n",
				   __func__, reason, task->comm, task->pid);
}

/*
 * add_waker_to_render_rthread() - add the realted task to child_thread
 *
 * The function will add waker to wakee's waker_array to save
 * the wakeup relationship
 */
void add_waker_to_render_rthread(struct task_struct *task)
{
	int waker_index, wakee_index;
	struct child_thread *thread = NULL;
	unsigned long flags, rw_lock_flag;
	int fail_reason = 0;

	if (oops_in_progress || task == NULL)
		return;

	read_lock_irqsave(&g_rt_rwlock, rw_lock_flag);
	if (!inited())
		goto err;

	/* ignore wakeup event if waker or wakee is not a ta thread */
	if (!both_render_rthread(current, task))
		goto err;

	wakee_index = get_child_rthread_index(task);
	if (unlikely(wakee_index == MAX_TID_COUNT)) {
		fail_reason = 1;
		goto err;
	}

	waker_index = get_child_rthread_index(current);
	if (unlikely(waker_index == MAX_TID_COUNT)) {
		fail_reason = 2;
		goto err;
	}

	/* if waker already added then return, else set the waker_bitmap */
	thread = &g_ta_render_ptr->ta_child_threads[wakee_index];
	if (unlikely(thread == NULL)) {
		fail_reason = 3;
		goto err;
	}

	raw_spin_lock_irqsave(&thread->lock, flags);

	if (!test_bit(waker_index, thread->waker_bitmap)) {
		if (thread->waker_count < MAX_WAKER_COUNT) {
			thread->waker_array[thread->waker_count++] = waker_index;
			__set_bit(waker_index, thread->waker_bitmap);

			trace_add_waker_to_render_rthread(current, waker_index,
							  task, wakee_index,
							  thread->waker_count);
		}
	}

	thread->woken_count++;

	raw_spin_unlock_irqrestore(&thread->lock, flags);
	read_unlock_irqrestore(&g_rt_rwlock, rw_lock_flag);

	return;

err:
	read_unlock_irqrestore(&g_rt_rwlock, rw_lock_flag);

	if (fail_reason > 1)
		printk_ratelimited(KERN_WARNING "[%s %d] reason=%d\n",
				   __func__, __LINE__, fail_reason);
}

static inline void init_child_rthread(int index, struct task_struct *p)
{
	struct child_thread *thread = &g_ta_render_ptr->ta_child_threads[index];

	if (unlikely(thread == NULL))
		return;

	raw_spin_lock_init(&thread->lock);
	thread->task = p;
	thread->pid = p->pid;
	thread->visted = false;
	thread->exited = false;
	thread->last_walt_ravg_sum = p->ravg.ravg_sum;
	thread->frame_demand = 0;
	thread->waker_count = 0;
	thread->woken_count = 0;
	(void)memset_s(thread->waker_bitmap,
		       BITS_TO_LONGS(MAX_TID_COUNT) * sizeof(unsigned long),
		       0, BITS_TO_LONGS(MAX_TID_COUNT) * sizeof(unsigned long));
}

static inline void update_rthread_util(struct child_thread *thread,
				       u64 window_size)
{
	u64 last_walt_ravg_sum, curr_walt_ravg_sum, frame_load, frame_util;
	struct task_struct *task = thread->task;

	curr_walt_ravg_sum = task->ravg.ravg_sum;
	last_walt_ravg_sum = thread->last_walt_ravg_sum;

	/*
	 * if the task hasn't run in this window,
	 * then set fraeme_demand to 0
	 */
	if (curr_walt_ravg_sum == last_walt_ravg_sum || window_size == 0) {
		thread->frame_demand = 0;
		return;
	}

	thread->last_walt_ravg_sum = curr_walt_ravg_sum;

	/* handle overflow case */
	if (unlikely(curr_walt_ravg_sum < last_walt_ravg_sum))
		frame_load = (U64_MAX - last_walt_ravg_sum + 1) + curr_walt_ravg_sum;
	else
		frame_load = curr_walt_ravg_sum - last_walt_ravg_sum;

	/* normalized the util */
	frame_load = frame_load << SCHED_CAPACITY_SHIFT;
	frame_util = frame_load / window_size;

	trace_update_rthread_util(thread->task, frame_load, frame_util);

	/* in case of the resolution is too big */
	frame_util = clamp(frame_util, 1ULL, 1024ULL); /*lint !e666*/

	thread->frame_demand = frame_util;
}

/*
 * update_all_rthread_util()
 * update all child threads util, window_start and exited_thrad_nr
 */
static inline void update_all_rthread_util(void)
{
	struct child_thread *thread = NULL;
	u64 curr_ktime, window_size;
	int index;
	int total_count = g_ta_render_ptr->total_count;
	u16 exited_cnt = 0;

	/* update the window_start */
	curr_ktime = walt_ktime_clock();
	window_size = curr_ktime - g_ta_render_ptr->window_start;
	g_ta_render_ptr->window_start = curr_ktime;

	for (index = 0; index < total_count; index++) {
		thread = &g_ta_render_ptr->ta_child_threads[index];
		if (unlikely(thread == NULL))
			continue;

		/* The exited thread load have already updated */
		if (thread->exited) {
			exited_cnt++;
			continue;
		}

		if (!thread->task) {
			exited_cnt++;
			thread->exited = true;
			continue;
		}

		update_rthread_util(thread, window_size);
	}
	g_ta_render_ptr->exited_thread_nr = exited_cnt;
}

/*
 * Descending order by frame_demand
 *
 * Ascending Return: 0 if both are equal, -1 if the first is smaller, else 1
 * Descending Return: 0 if both are equal, 1 if the first is smaller, else -1
 */
static int cmp_waker(const void *ax, const void *bx)
{
	struct child_thread *prev_wakeup = NULL;
	struct child_thread *next_wakeup = NULL;
	int prev, next;

	prev = *((int *)ax);
	next = *((int *)bx);

	prev_wakeup = &g_ta_render_ptr->ta_child_threads[prev];
	next_wakeup = &g_ta_render_ptr->ta_child_threads[next];
	if (unlikely(prev_wakeup == NULL || next_wakeup == NULL))
		return 0;

	if (prev_wakeup->frame_demand > next_wakeup->frame_demand)
		return -1;
	else if (prev_wakeup->frame_demand < next_wakeup->frame_demand)
		return 1;
	else
		return 0;
}

/*
 * Descending order by util
 * Ascending Return: 0 if both are equal, -1 if the first is smaller, else 1
 * Descending Return:  0 if both are equal, 1 if the first is smaller, else -1
 */
static int cmp_rthread(const void *ax, const void *bx)
{
	struct thread_util *prev_rthread = NULL;
	struct thread_util *next_rthread = NULL;

	prev_rthread = (struct thread_util *)ax;
	next_rthread = (struct thread_util *)bx;
	if (unlikely(prev_rthread == NULL || next_rthread == NULL))
		return 0;

	if (prev_rthread->util > next_rthread->util)
		return -1;
	else if (prev_rthread->util < next_rthread->util)
		return 1;
	else
		return 0;
}

#ifdef CONFIG_RENDER_RT_DEBUG
unsigned int sysctl_sched_enable_render_rt_trace = 0;

/*lint -e24 -e31 -e563*/
static inline void dbg_print_waker(int index)
{
	int i, waker_index;
	struct child_thread *waker_thread = NULL;
	struct child_thread *thread = &g_ta_render_ptr->ta_child_threads[index];
	int waker_cnt;

	if (unlikely(thread == NULL))
		return;

	waker_cnt = thread->waker_count;

	if (thread->exited || !thread->task)
		trace_printk("index=%d count=%d exited=1\n", index, waker_cnt);
	else
		trace_printk("index=%d count=%d comm=%s pid =%d\n",
			     index, waker_cnt, thread->task->comm,
			     thread->task->pid);

	for (i = 0; i < waker_cnt; i++) {
		waker_index = thread->waker_array[i];
		waker_thread = &g_ta_render_ptr->ta_child_threads[waker_index];
		if (unlikely(waker_thread == NULL))
			continue;

		if (waker_thread->exited || !waker_thread->task)
			trace_printk("index=%d\n", waker_index);
		else
			trace_printk("\t<waker_comm=%s pid=%d index=%d util=%u\n",
				     waker_thread->task->comm,
				     waker_thread->task->pid, waker_index,
				     waker_thread->frame_demand);
	}
}

static inline void dbg_dump_ta_threads(void)
{
	int total_count = g_ta_render_ptr->total_count;
	struct child_thread *thread = NULL;
	int index;

	trace_printk("dump_all_threads tc=%u\n", total_count);
	for (index = 0; index < total_count; index++) {
		thread = &g_ta_render_ptr->ta_child_threads[index];
		if (thread && thread->waker_count)
			trace_printk("[idx=%d] pid=%d comm=%s wakers_cnt=%d exited=%d woken_cnt=%u\n",
				     index, thread->pid, thread->task->comm,
				     thread->waker_count, thread->exited ? 1 : 0,
				     thread->woken_count);
	}
}

static inline void dbg_show_rthread(int *rthread_results)
{
	int i;

	trace_printk("bfs_results:\n");
	trace_printk("render:[%d]=%d\n", 0, g_ta_render_ptr->render_index);
	dbg_print_waker(g_ta_render_ptr->render_index);
	for (i = 0; i < MAX_THREAD_NUM; i++) {
		if (rthread_results[i] != -1) {
			trace_printk("[%d]=%d\n", i, rthread_results[i]);
			dbg_print_waker(rthread_results[i]);
		}
	}
}

static inline void dbg_show_hrthread(struct thread_util *rthread_results,
				     int total_cnt)
{
	int i;

	for (i = 0; i < total_cnt; i++)
		trace_printk("[%d] pid=%d util=%lu woken=%u\n",
			     i, rthread_results[i].pid, rthread_results[i].util,
			     rthread_results[i].woken_count);
}
/*lint +e24 +e31 +e563*/
#endif

/*
 * As the kernel stack is small, used in the global variable
 * in function get_render_hrthread to keep the results.
 */
struct render_ht g_ht_result;

/*
 * In order to reduce the swap opeartion cost in the sort algorithm,
 * the smaller sized "thread_util" instead of "child_thread" is perferred.
 */
struct thread_util g_sorted_child_threads[MAX_TID_COUNT];

/*
 * As copy_to/form_user may sleep, need to hold the mutex lock while accessing
 * g_ht_reuslt and g_sorted_child_threads.
 */
static DEFINE_MUTEX(g_hrthread_lock);

/*
 * get_render_hrthread() - get top 21 threads which have bigger util
 *
 * To impove performance:
 * 1. used the smaller size elements to sort
 * 2. only sorting the runned threads in last frame
 */
int get_render_hrthread(void __user *uarg)
{
	struct render_ht *ht_result = NULL;
	struct thread_util *rthread_utils = NULL;
	struct thread_util *ta_threads = NULL;
	unsigned long flags;
	int i, thread_nr, ret;

	mutex_lock(&g_hrthread_lock);
	if (copy_from_user(&g_ht_result, uarg, sizeof(struct render_ht))) {
		pr_err("%s copy_from_user fail.\n", __func__);
		ret = -EFAULT;
		goto m_unlock;
	}

	write_lock_irqsave(&g_rt_rwlock, flags);
	if (!inited()) {
		ret = -ENODEV;
		goto unlock;
	}

	if (g_ht_result.render_pid != g_ta_render_ptr->pid) {
		ret = -ENODEV;
		goto unlock;
	}

	ht_result = &g_ht_result;
	ta_threads = g_sorted_child_threads;

	for (i = 0, thread_nr = 0; i < g_ta_render_ptr->total_count; i++) {
		if (g_ta_render_ptr->ta_child_threads[i].frame_demand == 0)
			continue;

		ta_threads[thread_nr].pid =
			g_ta_render_ptr->ta_child_threads[i].pid;
		ta_threads[thread_nr].util =
			g_ta_render_ptr->ta_child_threads[i].frame_demand;
		ta_threads[thread_nr].woken_count =
			g_ta_render_ptr->ta_child_threads[i].woken_count;
		thread_nr++;
	}

	sort(ta_threads, thread_nr,
	     sizeof(struct thread_util), &cmp_rthread, NULL);

	/* set the result to ht_result */
	rthread_utils = ht_result->utils;

	for (i = 0; i < MAX_THREAD_NUM && i < thread_nr; i++) {
		rthread_utils[i].pid = ta_threads[i].pid;
		rthread_utils[i].util = ta_threads[i].util;
		rthread_utils[i].woken_count = ta_threads[i].woken_count;
	}

	/* set the remaining elements to invlid ones */
	while (i < MAX_THREAD_NUM)
		rthread_utils[i++].pid = 0;

#ifdef CONFIG_RENDER_RT_DEBUG
	if (sysctl_sched_enable_render_rt_trace)
		dbg_show_hrthread(rthread_utils, MAX_THREAD_NUM);
#endif

	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (copy_to_user(uarg, ht_result, sizeof(struct render_ht))) {
		ret = -EFAULT;
		goto m_unlock;
	}

	mutex_unlock(&g_hrthread_lock);

	return 0;

unlock:
	write_unlock_irqrestore(&g_rt_rwlock, flags);
m_unlock:
	mutex_unlock(&g_hrthread_lock);
	printk_ratelimited(KERN_ERR "[%s %d] ret=%d\n", __func__, __LINE__, ret);
	return ret;
}

static inline void reset_child_rthread(void)
{
	int total_count = g_ta_render_ptr->total_count;
	struct child_thread *thread = NULL;
	int index;

	/*
	 * frame_demand can not be reset, as the field may be used.
	 * in get_render_hrthread function. So the filed need to update
	 * every frame even the thread is not run in the frame.
	 */
	for (index = 0; index < total_count; index++) {
		thread = &g_ta_render_ptr->ta_child_threads[index];
		if (unlikely(thread == NULL))
			continue;
		thread->waker_count = 0;
		(void)memset_s(thread->waker_array, MAX_WAKER_COUNT * sizeof(int),
			       0, MAX_WAKER_COUNT * sizeof(int));
		(void)memset_s(thread->waker_bitmap,
			       BITS_TO_LONGS(MAX_TID_COUNT) * sizeof(unsigned long),
			       0, BITS_TO_LONGS(MAX_TID_COUNT) * sizeof(unsigned long));
		thread->visted = false;
		thread->exited = false;
		thread->woken_count = 0;
	}
}

/*
 * remove_exited_render_rthread() - remove the exited task
 *
 * This function will remove the task with exited set to true,
 * the exited flag is set in remove_render_rthread function.
 */
static inline void remove_exited_render_rthread(void)
{
	int total_count = g_ta_render_ptr->total_count;
	struct child_thread *thread = NULL;
	struct child_thread *curr_rthread = NULL;
	struct child_thread *next_rthread = NULL;
	int i, index, last_index;
	u16 exited_cnt = 0;

	/* exited_thread_nr updated in update_all_rthread_util func */
	if (g_ta_render_ptr->exited_thread_nr == 0)
		return;

	for (index = total_count - 1; index >= 0; index--) {
		thread = &g_ta_render_ptr->ta_child_threads[index];

		if (thread && thread->exited) {
			last_index = total_count - exited_cnt - 1;

			/* need to reset the value */
			thread->exited = false;
			for (i = index; i < last_index; i++) {
				curr_rthread =
					&g_ta_render_ptr->ta_child_threads[i];
				next_rthread =
					&g_ta_render_ptr->ta_child_threads[i + 1]; /*lint !e679*/
				if (unlikely(curr_rthread == NULL ||
					     next_rthread == NULL))
					continue;

				curr_rthread->pid = next_rthread->pid;
				curr_rthread->task = next_rthread->task;
				curr_rthread->last_walt_ravg_sum =
					next_rthread->last_walt_ravg_sum;

				/* update the render_index */
				if (next_rthread->pid == g_ta_render_ptr->pid)
					g_ta_render_ptr->render_index = i;
			}
			exited_cnt++;

			/* If all exited thread have been removed, stop the loop */
			if (exited_cnt == g_ta_render_ptr->exited_thread_nr) {
				g_ta_render_ptr->exited_thread_nr = 0;
				break;
			}
		}
	}

	if (total_count > exited_cnt)
		g_ta_render_ptr->total_count = total_count - exited_cnt;
	else
		g_ta_render_ptr->total_count = 0;
}

#define INVALID_THREAD_IDX	(-1)
#define MAX_BREADTH		4

/*
 * If MAX_BREADTH is m, then N level search:
 * g_bfs_thread_cnt = m^0+m^1+...+m^(n-1);
 *
 * If the If MAX_BREADTH=4 and N=2,
 * the number of nodes to search is 5.
 */
const int g_bfs_thread_nr = 5;

/* g_bfs_related_thread_nr = (g_bfs_thread_nr * MAX_BREADTH + 1) */
const int g_bfs_related_thread_nr = 21;

static inline void reset_rthread_results(int *rthread_results)
{
	int i;

	for (i = 0; i < g_bfs_related_thread_nr; i++)
		rthread_results[i] = INVALID_THREAD_IDX;
}

static inline void populate_rthread(int *rthread_results,
				    int result_index, int index)
{
	int i = 0;
	int waker_index = 0;
	int curr_waker_index;
	struct child_thread *curr_waker_thread = NULL;
	struct child_thread *thread = &g_ta_render_ptr->ta_child_threads[index];

	if (unlikely(thread == NULL))
		return;

	/*
	 * need to sort the wakers only if
	 * the number is bigger than MAX_BREADTH
	 */
	if (thread->waker_count > MAX_BREADTH)
		sort(thread->waker_array, thread->waker_count,
		     sizeof(int), &cmp_waker, NULL);

	do {
		if (waker_index < thread->waker_count) {
			curr_waker_index = thread->waker_array[waker_index];
			if (unlikely(curr_waker_index >= MAX_TID_COUNT)) {
				waker_index++;
				rthread_results[result_index++] = INVALID_THREAD_IDX;
				continue;
			}

			curr_waker_thread =
				&g_ta_render_ptr->ta_child_threads[curr_waker_index];
			if (unlikely(curr_waker_thread == NULL)) {
				waker_index++;
				rthread_results[result_index++] = INVALID_THREAD_IDX;
				continue;
			}

			if (curr_waker_thread->visted) {
				waker_index++;
				continue;
			} else {
				rthread_results[result_index++] = curr_waker_index;
				curr_waker_thread->visted = true;
				waker_index++;
			}
		} else {
			rthread_results[result_index++] = INVALID_THREAD_IDX;
		}

		i++;
	} while (i < MAX_BREADTH);
}

static inline void hierarchical_bfs_traverse(int *rthread_results)
{
	int i = 0;
	int rthread_index;
	int result_start_index;
	struct child_thread *first_thread = NULL;

	reset_rthread_results(rthread_results);

	rthread_index = g_ta_render_ptr->render_index;

	/* visit the first thread */
	first_thread = &g_ta_render_ptr->ta_child_threads[rthread_index];
	if (unlikely(first_thread == NULL))
		return;

	first_thread->visted = true;
	rthread_results[0] = rthread_index;

	do {
		rthread_index = rthread_results[i];
		if (rthread_index != INVALID_THREAD_IDX) {
			result_start_index = i * MAX_BREADTH + 1;
			populate_rthread(rthread_results, result_start_index,
					 rthread_index);
		}
		i++;
	} while (i < g_bfs_thread_nr);
}

static inline void populate_results(struct render_rt *result,
				    const int *rthread_results)
{
	struct thread_util *rthread_utils = NULL;
	struct child_thread *thread = NULL;
	int thread_index, i;
	int thread_nr = 0;

	rthread_utils = result->utils;
	for (i = 0; i < MAX_THREAD_NUM; i++) {
		thread_index = rthread_results[i];
		if (thread_index == -1) {
			rthread_utils[i].pid = 0;
		} else {
			thread = &g_ta_render_ptr->ta_child_threads[thread_index];
			if (unlikely(thread == NULL)) {
				rthread_utils[i].pid = 0;
				continue;
			}

			rthread_utils[i].pid = thread->pid;
			rthread_utils[i].util = thread->frame_demand;
			rthread_utils[i].woken_count = thread->woken_count;
			thread_nr++;
		}
	}

	result->num = thread_nr;
}

/*
 * get_render_rthread() - get related thread
 *
 * This function will do the following work:
 * 1. update the util of the threads which have run in last frmae
 * 2. do bfs search form render thread, and save the results
 */
int get_render_rthread(void __user *uarg)
{
	struct render_rt result;
	int rthread_results[MAX_THREAD_NUM];
	unsigned long flags;
	int ret = 0;
	bool is_stopped = false;

	if (copy_from_user(&result, uarg, sizeof(struct render_rt))) {
		ret = -EFAULT;
		printk_ratelimited(KERN_ERR "[%s %d] ret=%d\n",
				   __func__, __LINE__, ret);
		return ret;
	}

	write_lock_irqsave(&g_rt_rwlock, flags);

	if (!inited()) {
		ret = -ENOENT;
		goto err;
	}

	if (result.render_pid != g_ta_render_ptr->pid) {
		ret = -ENODEV;
		goto err;
	}

	update_all_rthread_util();

	is_stopped = g_ta_render_ptr->stopped;
	if (is_stopped)
		goto skip_traverse;

	hierarchical_bfs_traverse(rthread_results);

	populate_results(&result, rthread_results);

#ifdef CONFIG_RENDER_RT_DEBUG
	if (sysctl_sched_enable_render_rt_trace) {
		dbg_dump_ta_threads();
		dbg_show_rthread(rthread_results);
	}
#endif

skip_traverse:
	remove_exited_render_rthread();

	reset_child_rthread();
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	if (!is_stopped) {
		if (copy_to_user(uarg, &result, sizeof(struct render_rt))) {
			ret = -EIO;
			goto err_copy;
		}
	}

	return 0;

err:
	write_unlock_irqrestore(&g_rt_rwlock, flags);
err_copy:
	printk_ratelimited(KERN_ERR "[%s %d] ret=%d\n", __func__, __LINE__, ret);

	return ret;
}

static inline struct task_struct *get_leader_task(pid_t pid)
{
	struct task_struct *cur = NULL;
	struct task_struct *leader = NULL;

	cur = find_task_by_vpid(pid);
	if (!cur)
		return NULL;

	leader = cur->group_leader;
	if (!leader)
		return NULL;

	return leader;
}

struct render_thread_task {
	int pid;
	struct task_struct *task;
};

/*
 *  Ascending order by pid
 *
 * Ascending Return: 0 if both are equal, -1 if the first is smaller, else 1
 * Descending Return:  0 if both are equal, 1 if the first is smaller, else -1
 */
static int cmp_render_task(const void *ax, const void *bx)
{
	struct render_thread_task *prev_rthread = NULL;
	struct render_thread_task *next_rthread = NULL;

	prev_rthread = (struct render_thread_task *)ax;
	next_rthread = (struct render_thread_task *)bx;
	if (unlikely(prev_rthread == NULL || next_rthread == NULL))
		return 0;

	if (prev_rthread->pid > next_rthread->pid)
		return 1;
	else if (prev_rthread->pid < next_rthread->pid)
		return -1;
	else
		return 0;
}

/*
 * init_render_rthread() - initialize the global data
 *
 * If inited params with rfps->force_init set,
 * the function will reinit the modle any way.
 */
int init_render_rthread(void __user *uarg)
{
	int i;
	int ret;
	int count = 0;
	int render_index = -1;
	struct task_struct *leader = NULL;
	struct task_struct *pos = NULL;
	struct render_init_paras rfps;
	unsigned long flags;
	struct render_thread_task *child_tasks = NULL;

	if (copy_from_user(&rfps, uarg, sizeof(struct render_init_paras)))
		return -EFAULT;

	child_tasks = kmalloc(sizeof(struct render_thread_task) * MAX_TID_COUNT,
			      GFP_ATOMIC);
	if (!child_tasks)
		return -ENOMEM;

	write_lock_irqsave(&g_rt_rwlock, flags);

	/* if rps.force_init is true, ignore the error */
	if (!rfps.force_init && inited()) {
		ret = -EBUSY;
		goto err;
	}

	/* clear all the members */
	(void)memset_s(&g_ta_render, sizeof(struct render_related_thread),
		       0, sizeof(struct render_related_thread));
	g_ta_render_ptr = &g_ta_render;
	g_ta_render_ptr->pid = rfps.render_pid;

	/* mark the start timestamp */
	g_ta_render_ptr->window_start = walt_ktime_clock();

	rcu_read_lock();
	leader = get_leader_task(g_ta_render_ptr->pid);
	pos = leader;
	if (pos == NULL) {
		rcu_read_unlock();
		ret = -ENODEV;
		goto err;
	}

	// cppcheck-suppress *
	do {
		// cppcheck-suppress *
		if (count >= MAX_TID_COUNT)
			break;

		if (pos->flags & PF_EXITING)
			continue;

		child_tasks[count].pid = pos->pid;
		child_tasks[count].task = pos;

		count++;
	} while_each_thread(leader, pos);
	rcu_read_unlock();

	sort(child_tasks, count, sizeof(struct render_thread_task),
	     &cmp_render_task, NULL);

	for (i = 0; i < count; i++) {
		/* save the render_thread index */
		if (child_tasks[i].pid == g_ta_render_ptr->pid)
			render_index = i;
		/* init each entry for wakeupinfo */
		init_child_rthread(i, child_tasks[i].task);
	}

	if (render_index == -1) {
		ret = -EINVAL;
		goto err;
	}

	g_ta_render_ptr->render_index = render_index;
	g_ta_render_ptr->total_count = count;
	g_ta_render_ptr->ta_grp_task = leader;
	write_unlock_irqrestore(&g_rt_rwlock, flags);
	pr_info("init success\n");

	kfree(child_tasks);
	return 0;

err:
	g_ta_render_ptr = NULL;
	write_unlock_irqrestore(&g_rt_rwlock, flags);
	pr_err("[%s %d] init failed ret=%d\n", __func__, __LINE__, ret);

	kfree(child_tasks);
	return ret;
}

/*
 * destroy_render_rthread() - destroy the module
 */
int destroy_render_rthread(void __user *uarg)
{
	pid_t destroy_pid;
	unsigned long flags;
	int ret = 0;

	if (copy_from_user(&destroy_pid, uarg, sizeof(pid_t))) {
		pr_err("related_tid copy_from_user fail.\n");
		ret = -EFAULT;
		goto err;
	}

	write_lock_irqsave(&g_rt_rwlock, flags);
	if (!inited())
		goto unlock;

	if (destroy_pid != g_ta_render_ptr->pid) {
		ret = -ENODEV;
		goto unlock;
	}

	g_ta_render_ptr = NULL;
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	return 0;

unlock:
	write_unlock_irqrestore(&g_rt_rwlock, flags);
err:
	pr_err("[%s %d] ret=%d\n", __func__, __LINE__, ret);
	return ret;
}

/*
 * stop_render_rthread() - stop the module getting bfs results
 */
int stop_render_rthread(void __user *uarg)
{
	struct render_stop rs;
	unsigned long flags;
	int ret = 0;

	if (copy_from_user(&rs, uarg, sizeof(struct render_stop))) {
		pr_err("related_stop copy_from_user fail.\n");
		ret = -EFAULT;
		goto err;
	}

	write_lock_irqsave(&g_rt_rwlock, flags);
	if (!inited()) {
		ret = -ENODEV;
		goto unlock;
	}
	if (rs.render_pid != g_ta_render_ptr->pid) {
		ret = -ENODEV;
		goto unlock;
	}

	g_ta_render_ptr->stopped = rs.stopped;
	write_unlock_irqrestore(&g_rt_rwlock, flags);

	return 0;
unlock:
	write_unlock_irqrestore(&g_rt_rwlock, flags);
err:
	pr_err("[%s %d] ret=%d\n", __func__, __LINE__, ret);
	return ret;
}
