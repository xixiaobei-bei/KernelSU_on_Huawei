/*
 * hw_xcollie_misc.h
 *
 * This file is the header file for hwxcollie
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#ifndef HW_XCOLLIE_MISC_H
#define HW_XCOLLIE_MISC_H

#include <chipset_common/hwxcollie/hw_xcollie.h>
#include <linux/unistd.h>
#include <linux/stddef.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/wait.h>


#define MAX_HW_XCOLLIE_NODES            HW_MAX_XCOLLIE_NUM
#define MAX_HW_TIMERRING_SIZE           HW_ALIGN_TO(HW_MAX_TIMEOUT_VAL, 16)
#define MAX_HW_DELAY_COUNT              3
#define HW_SLEEP_THREASHOLD_NANOSECOND  (100 * 1000 * 1000)
#define HW_TIMERRING_CHECK_INTVAL       1

#define HW_XCOLLIE_CALLBACK_HISTORY_MAX 5
#define HW_XCOLLIE_CALLBACK_TIMEWIN_MAX 60
#define MAX_STACK_TRACE_DEPTH 64


#undef HW_XCOLLIE_FAULT_INJECT

#define HW_DEFAULT_SEQ_VALUE     (-1)
#define THOUSAND                 1000
#define MILLION                  (THOUSAND * THOUSAND)
#define BILLION                  (MILLION * THOUSAND)
#define TEN_SECOND               10
#define hw_atomic_read(x)        __sync_fetch_and_add(&(x), 0)
#define hw_atomic_inc(x)         __sync_fetch_and_add(&(x), 1)
#define HW_ALIGN_TO(len, align) (((((len) - 1) / (align)) + 1) * (align))
#define HW_COOKIE_SHIFT         (HW_MAX_XCOLLIE_SHIFT + 2)
#define HW_NODE_MAGIC(node)     ((node)->flag ^ (node)->timeout ^ \
				(node)->tid ^ \
				hw_atomic_read(hwtimectrl.count_start))
#define HW_WRAP_ID(pos, node)   ((HW_NODE_MAGIC(node) << HW_COOKIE_SHIFT) | \
				(pos))
#define HW_UNWRAP_ID(id)        ((id) & ((1 << HW_COOKIE_SHIFT) - 1))

#define HW_FILLUP_NODE(node, name, timeout, func, arg, flag, task) do { \
	(node)->id = HW_WRAP_ID((node)->seq, node); \
	(node)->name = name; \
	(node)->start_time = get_seconds(); \
	(node)->timeout = timeout; \
	(node)->tid = (task)->pid;\
	(node)->arg = arg; \
	(node)->flag = flag; \
	(node)->callback = func; \
} while (0)

#define HW_ID_INVALID(x) (unlikely(HW_UNWRAP_ID(x) >= MAX_HW_XCOLLIE_NODES))

#ifdef HW_XCOLLIE_FAULT_INJECT
#define hw_timespec_cmp(a, b, op) \
	((a)->tv_sec == (b)->tv_sec ? \
	(a)->tv_nsec op(b)->tv_nsec : \
	(a)->tv_sec op(b)->tv_sec)
#endif

struct hw_xcollie_timerring_sleepctrl {
	int monitor_thread_in_sleep;
	unsigned int count_start;
	unsigned int count_end;
	unsigned int delay_count;
	unsigned int last_count_start;
	struct mutex sleep_mutex;
	wait_queue_head_t sleep_wq;
};

struct hw_xcollie_timer_ring {
	struct hwlistnode head[MAX_HW_TIMERRING_SIZE];
	int pos;
};

struct hw_xcollie_node {
	int id;
	int seq;
	const char *name;
	time_t start_time;
	int timeout;
	pid_t tid;
	void *arg;
	int flag;
	int (*callback)(void *args);
	struct hwlistnode list;
};

struct hw_callback_struct {
	int id;
	int (*callback)(void *args);
	pid_t tid;
	const char *name;
	time_t start_time;
	void *arg;
	int flag;
};

struct hw_xcollie_callback {
	struct hw_callback_struct cb[MAX_HW_XCOLLIE_NODES];
	int count;
};


#endif /* HW_XCOLLIE_MISC_H */

