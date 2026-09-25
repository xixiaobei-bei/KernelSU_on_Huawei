/*
 * hw_xcollie.c
 *
 * This file is the implementation for hwxcollie
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

#include <chipset_common/hwxcollie/hw_xcollie.h>
#include "hw_xcollie_list.h"
#include "hw_xcollie_misc.h"
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/stacktrace.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#endif


/* mark the xcollie initialize progress end */
static int hw_xcollie_initialized;

/* list for the free nodes */
struct hwlistnode hw_xcollie_free_list = {
	.next = &(hw_xcollie_free_list),
	.prev = &(hw_xcollie_free_list),
};

/* to store the time */
static struct hw_xcollie_timer_ring hw_xcollie_tring;
/* save all the monitor info */
static struct hw_xcollie_node hw_xcollie_nodes[MAX_HW_XCOLLIE_NODES];

#define HW_CALC_TIMERRING_POS(x) ((hw_xcollie_tring.pos + \
		((((x) - 1) / HW_TIMERRING_CHECK_INTVAL) + 1)) % \
		MAX_HW_TIMERRING_SIZE)

/* initializing mutex */
static DEFINE_MUTEX(hw_xcollie_init_mutex);
/* free list and node mutex */
static DEFINE_MUTEX(hw_xcollie_list_mutex);

/* call back time stamp */
static time_t hw_xcollie_time_callback;
/* call back count */
static unsigned int hw_xcollie_nr_callback;

/* data struct for monitor thread sleep/wake control */
static struct hw_xcollie_timerring_sleepctrl hwtimectrl = {
	.monitor_thread_in_sleep = 0,
	.count_start = 0,
	.count_end = 0,
	.delay_count = 0,
	.last_count_start = 0,
	.sleep_mutex = __MUTEX_INITIALIZER(hwtimectrl.sleep_mutex),
};


static struct task_struct *hw_xcollie_task;

#ifdef HW_XCOLLIE_FAULT_INJECT
static struct task_struct *hw_xcollie_inject_task;
#endif

static inline void hw_list_init(struct hwlistnode *node)
{
	node->next = node;
	node->prev = node;
}

static inline void hw_list_add_tail(struct hwlistnode *head,
	struct hwlistnode *item)
{
	item->next = head;
	item->prev = head->prev;
	head->prev->next = item;
	head->prev = item;
}

static inline void hw_list_add_head(struct hwlistnode *head,
	struct hwlistnode *item)
{
	item->next = head->next;
	item->prev = head;
	head->next->prev = item;
	head->next = item;
}

static inline void hw_list_remove(struct hwlistnode *item)
{
	item->next->prev = item->prev;
	item->prev->next = item->next;
}

/*
 * copy a node information to the callback array
 *
 * @param cbs one callback struct
 * @param node monitor node information
 */
static void hw_copy_node_to_cbs(struct hw_xcollie_callback *cbs,
	struct hw_xcollie_node *node)
{
	int cnt = cbs->count;

	cbs->cb[cnt].id       = node->id;
	cbs->cb[cnt].callback = node->callback;
	cbs->cb[cnt].tid      = node->tid;
	cbs->cb[cnt].arg      = node->arg;
	cbs->cb[cnt].name     = node->name;
	cbs->cb[cnt].flag     = node->flag;
	cbs->cb[cnt].start_time = node->start_time;

	cbs->count++;
}

/*
 * initialize a monitor node
 *
 * @param node monitor node struct
 * @param seq pre-alloced id
 */
static void hw_xcollie_manager_init_node(struct hw_xcollie_node *node,
	int len, int seq)
{
	int saved_seq = node->seq;

	memset(node, 0, len);
	node->id = HW_INVALID_ID;

	if ((seq >= 0) && (seq < MAX_HW_XCOLLIE_NODES))
		node->seq = seq;
	else
		node->seq = saved_seq;

	hw_list_init(&(node->list));
}

int hw_xcollie_callback_should_limit(int flag)
{
	int ret = 0;
	unsigned long now;

	if (flag == HW_XCOLLIE_FLAG_NOOP)
		return 0;

	now = get_seconds();
	if ((hw_xcollie_time_callback + HW_XCOLLIE_CALLBACK_TIMEWIN_MAX) < now)
		hw_xcollie_time_callback = now;
	else if (++hw_xcollie_nr_callback > HW_XCOLLIE_CALLBACK_HISTORY_MAX)
		ret = 1;

	return ret;
}

int hw_dump_pid_stack(pid_t pid)
{
	struct stack_trace trace;
	unsigned long *entries = NULL;
	int err;
	int i;
	struct task_struct *task = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		return -EINVAL;

	entries = kmalloc(MAX_STACK_TRACE_DEPTH * sizeof(*entries), GFP_KERNEL);
	if (!entries) {
		put_task_struct(task);
		return -ENOMEM;
	}

	trace.nr_entries        = 0;
	trace.max_entries       = MAX_STACK_TRACE_DEPTH;
	trace.entries           = entries;
	trace.skip              = 0;

	err = mutex_lock_killable(&task->signal->cred_guard_mutex);
	if (!err) {
		save_stack_trace_tsk(task, &trace);

		for (i = 0; i < trace.nr_entries; i++) {
			printk(KERN_ERR "hwxcollie: #%d [<%pK>] %pB\n", i,
			(void *)entries[i], (void *)entries[i]);
		}
		mutex_unlock(&task->signal->cred_guard_mutex);
	}

	kfree(entries);
	if (task)
		put_task_struct(task);

	return err;
}

/* trigger callback and caller-defined callback */
static void hw_xcollie_do_callback(struct hw_callback_struct *cb)
{
	if (hw_xcollie_callback_should_limit(cb->flag)) {
		pr_err("hwxcollie: Too many callback triggerd\n");
		return;
	}

	if (cb->callback)
		(void)cb->callback(cb->arg);

	(void)hw_dump_pid_stack(cb->tid);
#ifdef HW_XCOLLIE_FAULT_INJECT
	pr_info("hwxcollie callback done\n");
#endif
}

/* check the current timer ring position and run all the callbacks */
static void hw_xcollie_timerring_do_timeout(void)
{
	int i;
	struct hwlistnode *head = NULL;
	struct hwlistnode *ptr = NULL;
	struct hwlistnode *ptr_safe = NULL;
	struct hw_xcollie_node *node = NULL;
	struct hw_xcollie_callback xcollie_cbs;

	xcollie_cbs.count = 0;
	mutex_lock(&hw_xcollie_list_mutex);
	/* point to the current list timeout in timer ring */
	head = &(hw_xcollie_tring.head[hw_xcollie_tring.pos]);

	/* get all the timeout nodes from here */
	hw_list_for_each_safe(ptr, ptr_safe, head) {
		node = hw_node_to_item(ptr, struct hw_xcollie_node, list);

		hw_list_remove(&(node->list));
		hw_copy_node_to_cbs(&xcollie_cbs, node);
		hw_xcollie_manager_init_node(node,
					sizeof(*node),
					HW_DEFAULT_SEQ_VALUE);
		hw_atomic_inc(hwtimectrl.count_end);
		hw_list_add_tail(&hw_xcollie_free_list, &(node->list));
	}

	/* point to the next timeout node */
	hw_xcollie_tring.pos = HW_CALC_TIMERRING_POS(HW_TIMERRING_CHECK_INTVAL);
	mutex_unlock(&hw_xcollie_list_mutex);

	/* run timeout callback */
	for (i = 0; i < xcollie_cbs.count; i++) {
#ifdef HW_XCOLLIE_FAULT_INJECT
		pr_info("hwxcollie: Trigger 0x%x Callback Function\n",
			xcollie_cbs.cb[i].id);
#endif
		hw_xcollie_do_callback(&xcollie_cbs.cb[i]);
	}
}

/* find a free node @return free node pointer if success, NULL if failed */
static struct hw_xcollie_node *hw_xcollie_manager_get_free_node(void)
{
	struct hw_xcollie_node *node = NULL;

	if (hw_list_empty(&hw_xcollie_free_list))
		return NULL;

	node = hw_node_to_item(hw_list_head(&hw_xcollie_free_list),
		struct hw_xcollie_node, list);
	hw_list_remove(&(node->list));
	hw_list_init(&(node->list));

	return node;
}

/*
 * fill the xcollie node, and put it into the timer ring
 *
 * @param name watchpoint name
 * @param timeout timeout configuration
 * @param func call-defined callback
 * @param arg  caller-defined args for callback
 * @param flag flag is not used now
 *
 * @return id if success, -1 if failed
 */
static int hw_xcollie_manager_start(const char *name, int timeout,
	int (*func)(void *), void *arg, int flag)
{
	struct hw_xcollie_node *node = NULL;
	int pos;

	node = hw_xcollie_manager_get_free_node();
	if (!node) {
		pr_err("hwxcollie: No free xcollie node\n");
		return HW_INVALID_ID;
	}

	/* fill node info */
	HW_FILLUP_NODE(node, name, timeout, func, arg, flag, current);

	/* add to timer ring */
	pos = HW_CALC_TIMERRING_POS(timeout);
	hw_list_add_tail(&(hw_xcollie_tring.head[pos]), &(node->list));

	return node->id;
}

static int hw_xcollie_manager_update(struct hw_xcollie_node *node,
	int timeout)
{
	int pos;

	/* remove from timer ring */
	hw_list_remove(&(node->list));
	pos = HW_CALC_TIMERRING_POS(timeout);

	/* add to timer ring */
	hw_list_add_tail(&(hw_xcollie_tring.head[pos]), &(node->list));
	return node->id;
}

/*
 * Export Function: locked start extended function
 *
 * @param name watchpoint name
 * @param timeout timeout configuration
 * @param func call-defined callback
 * @param arg  caller-defined args for callback
 * @param flag flag is not used now
 *
 * @return id if success, -1 if failed
 */
int __hw_xcollie_start_ex(const char *name, int timeout,
	int (*func)(void *), void *arg, int flag)
{
	int id;

	if (!hw_xcollie_initialized)
		return HW_INVALID_ID;

	if (!timeout || timeout > HW_MAX_TIMEOUT_VAL)
		return HW_INVALID_ID;

	mutex_lock(&hw_xcollie_list_mutex);
	id = hw_xcollie_manager_start(name, timeout, func, arg, flag);
	mutex_unlock(&hw_xcollie_list_mutex);
	if (id == HW_INVALID_ID)
		return id;

	/* notify if delay block */
	hw_atomic_inc(hwtimectrl.count_start);

	return id;
}

int hw_xcollie_update(int id, int timeout)
{
	struct hw_xcollie_node *node = NULL;
	int pos;

	if (!hw_xcollie_initialized)
		return HW_INVALID_ID;

	if (HW_ID_INVALID(id))
		return HW_INVALID_ID;

	mutex_lock(&hw_xcollie_list_mutex);

	pos = HW_UNWRAP_ID(id);
	if (hw_xcollie_nodes[pos].id != id) {
		mutex_unlock(&hw_xcollie_list_mutex);
		return HW_INVALID_ID;
	}

	node = &(hw_xcollie_nodes[pos]);
	hw_xcollie_manager_update(node, timeout);

	mutex_unlock(&hw_xcollie_list_mutex);

	return id;
}

/* Export Function: locked end function @param id id to destroy */
void hw_xcollie_end(int id)
{
	struct hw_xcollie_node *node = NULL;
	int pos;

	if (!hw_xcollie_initialized)
		return;

	if (HW_ID_INVALID(id)) {
		pr_err("hwxcollie: %s: not valid id 0x%x\n", __func__, id);
		return;
	}

	pos = HW_UNWRAP_ID(id);

	/* get node from timer ring, add to free list */
	mutex_lock(&hw_xcollie_list_mutex);

	if (hw_xcollie_nodes[pos].id != id) {
		mutex_unlock(&hw_xcollie_list_mutex);
		pr_err("hwxcollie: %s: alrealy release id 0x%x ?\n",
			__func__, id);
		return;
	}
	node = &(hw_xcollie_nodes[pos]);

	/* remove from timer ring */
	hw_list_remove(&(node->list));

	hw_xcollie_manager_init_node(node, sizeof(*node), HW_DEFAULT_SEQ_VALUE);

	/* add to free list */
	hw_list_add_tail(&hw_xcollie_free_list, &(node->list));

	mutex_unlock(&hw_xcollie_list_mutex);

	hw_atomic_inc(hwtimectrl.count_end);
}

#ifdef HW_XCOLLIE_FAULT_INJECT
static void hw_sub_timespec(struct timespec *minuend,
	struct timespec *subtrahend, struct timespec *x)
{
	x->tv_sec = minuend->tv_sec - subtrahend->tv_sec;
	if (minuend->tv_nsec >= subtrahend->tv_nsec) {
		x->tv_nsec = minuend->tv_nsec - subtrahend->tv_nsec;
	} else {
		x->tv_nsec = BILLION - subtrahend->tv_nsec + minuend->tv_nsec;
		x->tv_sec--;
	}
}

static int hw_xcollie_inject_callback(void *arg)
{
	pr_info("hwxcollie: inject callback %s\n", (char *)arg);

	return 0;
}

static int hw_xcollie_clock_target_check(struct timespec *ts,
	struct timespec *cur,
	struct timespec *delta)
{
	struct timespec min = {0, HW_SLEEP_THREASHOLD_NANOSECOND};

	*cur = CURRENT_TIME;
	if (hw_timespec_cmp(cur, ts, >))
		return -1;

	hw_sub_timespec(ts, cur, delta);
	if (hw_timespec_cmp(delta, &min, <))
		return 1;
	return 0;
}

static void hw_xcollie_sleep(unsigned long second)
{
	struct timespec to;
	struct timespec cur;
	struct timespec delta;
	unsigned int msecs;

	to = CURRENT_TIME;
	to.tv_sec += second;

	while (1) {
		if (hw_xcollie_clock_target_check(&to, &cur, &delta))
			break;

		msecs = (delta.tv_sec % TEN_SECOND) * THOUSAND +
			delta.tv_nsec / MILLION;

		if (msecs > (TEN_SECOND*THOUSAND)) {
			pr_err("hwxcollie: xcollie_sleep ovt %u\n", msecs);
			break;
		}
		msleep(msecs);
	}
}

static int hw_xcollie_inject_thread_handle(void *arg)
{
	int round = 0;
	int id = 0;
	char hahaha[] = "haahhahaa";

	pr_info("hwxcollie: inject thread start run\n");

	while (1) {
		msleep(HW_TIMERRING_CHECK_INTVAL * THOUSAND);

		round++;

		if ((round % TEN_SECOND) != 0)
			continue;

		pr_info("hwxcollie: inject start\n");
		/* Set overtime Value 5S */
		id = hw_xcollie_start_ex(5 * HW_TIMERRING_CHECK_INTVAL,
					hw_xcollie_inject_callback,
					(void *)hahaha,
					HW_XCOLLIE_FLAG_DEFAULT);

		/* Set Sleep time Value 6S */
		hw_xcollie_sleep(6 * HW_TIMERRING_CHECK_INTVAL);

		hw_xcollie_end(id);

		pr_info("hwxcollie: inject end\n");
	}

	return 0;
}
#endif

/* monitor thread main loop @return not care about */
static int hw_xcollie_thread_handle(void *arg)
{
#ifdef HW_XCOLLIE_FAULT_INJECT
	int round = 0;
#endif

	pr_info("hwxcollie: thread start run\n");

	while (1) {
		msleep(HW_TIMERRING_CHECK_INTVAL * THOUSAND);

#ifdef HW_XCOLLIE_FAULT_INJECT
		struct timespec t1, t2, t3;

		t1 = CURRENT_TIME;
#endif

		/* timeout process */
		/* step 1: get timeout info and run callback */
		hw_xcollie_timerring_do_timeout();

		/* step 2: goto sleep if needed */
		/* if wake up from sleep state, time should be re-calc */

#ifdef HW_XCOLLIE_FAULT_INJECT
	t2 = CURRENT_TIME;
	hw_sub_timespec(&t2, &t1, &t3);
	pr_info("hwxcollie: Round %d, %ld.%09ld\n",
		round++, t3.tv_sec, t3.tv_nsec);
#endif
	}

	return 0;
}

/* initializing the data struct */
static int hw_xcollie_manager_init(void)
{
	int i;

	hw_xcollie_initialized = 0;
	hw_xcollie_time_callback = 0;
	hw_xcollie_nr_callback = 0;

	hw_list_init(&hw_xcollie_free_list);
	init_waitqueue_head(&hwtimectrl.sleep_wq);

	for (i = 0; i < MAX_HW_XCOLLIE_NODES; i++) {
		hw_xcollie_manager_init_node(&hw_xcollie_nodes[i],
			sizeof(hw_xcollie_nodes[i]), i);
		/* add to free list */
		hw_list_add_tail(&hw_xcollie_free_list,
			&(hw_xcollie_nodes[i].list));
	}

	hw_xcollie_tring.pos = 0;
	for (i = 0; i < MAX_HW_TIMERRING_SIZE; i++)
		hw_list_init(&(hw_xcollie_tring.head[i]));

	hw_xcollie_task = kthread_run(hw_xcollie_thread_handle,
				NULL, "hwxcollie");

#ifdef HW_XCOLLIE_FAULT_INJECT
	hw_xcollie_inject_task = kthread_run(hw_xcollie_inject_thread_handle,
					NULL, "hwxcollieinject");
#endif

	hw_xcollie_initialized = 1;

	return 0;
}

/* Export Function: locked preinit function */
static int __init init_hw_xcollie_init(void)
{
	int ret;

	mutex_lock(&hw_xcollie_init_mutex);
	ret = hw_xcollie_manager_init();
	if (ret)
		pr_err("hwxcollie: hw_xcollie_manager_init fail\n");

	mutex_unlock(&hw_xcollie_init_mutex);
	pr_info("hwxcollie: init_hw_xcollie success\n");
	return 0;
}

subsys_initcall(init_hw_xcollie_init);


