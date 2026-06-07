/*
 * kernel/sched/hwstatus.c
 *
 * Copyright(C) 2018, Huawei, Inc., hwfatboy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include <linux/utsname.h>
#include <linux/mempolicy.h>
#include <linux/version.h>
#include <linux/delayacct.h>
#include <log/log_usertype.h>

#include <asm/barrier.h>
#include <asm/stacktrace.h>
#include <asm/irq.h>
#include "securec.h"

#include "sched.h"

#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

/*
 * Ease the printing of nsec fields:
 */
static long long nsec_high(unsigned long long nsec)
{
	if ((long long)nsec < 0) {
		nsec = -nsec;
		do_div(nsec, 1000000);
		return -nsec;
	}
	do_div(nsec, 1000000);

	return nsec;
}

static unsigned long nsec_low(unsigned long long nsec)
{
	if ((long long)nsec < 0)
		nsec = -nsec;

	return do_div(nsec, 1000000);
}

#define SEQ_printf(m, x...)			\
do {						\
	if (m)					\
		seq_printf(m, x);		\
	else					\
		printk(x);			\
} while (0)

#define PN(F) \
	SEQ_printf(m, "  .%-30s: %lld.%06ld\n", #F,		\
	nsec_high((long long)F), nsec_low((long long)F))

#define FGTASK_MAX 4
#define M_IODELAY_VALUE  (300*1000*1000)
#define CALLER_NAME_LEN  256
#define M_DSDELAY_VALUE  (200*1000*1000)
#define STACK_DUMP_SIZE  1024

enum{
	FG_UI = 0,
	FG_RENDER,
	PREV_UI,
	PREV_RENDER
};

enum{
	VERSION_V1 = 1,
	VERSION_V2 = 2,
	VERSION_V3,
};

enum {
	HW_SCHED_MAGIC_DEFAULT = 0X48570000, /* HW00 */
	HW_SCHED_MAGIC_IOCOLLECTOR = 0X48570001, /* HW01 */
	HW_SCHED_MAGIC_SCHED_BUFFER = 0X48570002 /* HW02 */
};

struct sched_hwstatus_rsthead {
	u64 ktime_last;
	u64 ktime_now;
};

struct hwstatus_mem {
	u64 allocuser_delay;
	u64 allocuser_count;
	u64 allocuser_delay_max;
	u64 allocuser_delay_max_order;
};

struct hwstatus_caller {
	u64  ktime_iodelay;
	char caller_iodelay[CALLER_NAME_LEN];
};

struct hwstatus_stack {
	char	stack[STACK_DUMP_SIZE];
};

struct sched_hwstatus_rstbody {
	pid_t pid;
	pid_t tgid;
	sched_hwstatus hwstatus;
};

struct sched_hwstatus_rst_V1 {
	struct sched_hwstatus_rsthead head;
	struct sched_hwstatus_rstbody body[FGTASK_MAX];
};

struct sched_hwstatus_rst_V2 {
	struct sched_hwstatus_rsthead head;
	struct sched_hwstatus_rstbody body[FGTASK_MAX];
	struct hwstatus_mem           mem[FGTASK_MAX];
	struct hwstatus_caller        caller;
};

struct sched_hwstatus_rst_V3 {
	struct sched_hwstatus_rsthead head;
	struct sched_hwstatus_rstbody body[FGTASK_MAX];
	struct hwstatus_mem           mem[FGTASK_MAX];
	struct hwstatus_caller        caller;
	struct hwstatus_stack         stack;
};

struct sched_hwstatus_rst_iocollector {
	struct sched_hwstatus_rsthead head;
	struct sched_hwstatus_rstbody body;
};

struct sched_task_info {
	char comm[TASK_COMM_LEN];
	pid_t pid;
	pid_t tgid;
	int target_cpu;
	int static_vip;
	u64 dynamic_vip;
	u64 enqueue_time;
	int vip_depth;
};

struct sched_qos {
	int dynamic_qos;
	int trans_qos;
	int trans_type;
	pid_t trans_from;
};

struct sched_data {
	u64 ktime;
	u64 rtime;
	struct sched_task_info task_info;
	struct sched_qos qos;
	struct sched_hwstatus hwstatus;
	struct hwstatus_mem mem;
};

struct sched_buffer {
	u64 write_index;
	u64 data_count;
	struct sched_data vip_data[FGTASK_MAX];
	void *data[0];
};

static const u64 SCHED_BUFFER_DATA_COUNT = 4096;
static const int HW_SCHED_NOT_BETA_USER;
static const int HW_SCHED_INVALID_COUNT = -1;
static const int HW_SCHED_INVALID_TASK_STRUCT = -2;
static const int HW_SCHED_COPY_FROM_USER_FAILED = -3;
static const int HW_SCHED_BUFFER_DISABLE = -4;
static const int HW_SCHED_BUFFER_CREATE_FAILED = -5;
static const int HW_SCHED_NOT_TIME_YET = -6;

static struct sched_buffer *g_buffer;
static u64 g_buffer_size;
static spinlock_t g_buffer_lock;
static atomic_t g_buffer_read_ref;
static bool g_is_buffer_init_success;
static bool g_is_buffer_already_init;

static pid_t fgtasks[FGTASK_MAX] = {0};
static u64 ktime_last;
static struct hwstatus_caller caller = {0};
static struct hwstatus_stack stack_dump = { {0} };
static u64 delta_max;

static inline void record_hwstatus(struct sched_hwstatus *hwstatus,
	struct sched_statistics *ssp)
{
	hwstatus->last_jiffies  = jiffies - ssp->hwstatus.last_jiffies;
	hwstatus->sum_exec_runtime_big = ssp->hwstatus.sum_exec_runtime_big;
	hwstatus->sum_exec_runtime_mid = ssp->hwstatus.sum_exec_runtime_mid;
	hwstatus->sum_exec_runtime_ltt = ssp->hwstatus.sum_exec_runtime_ltt;

	hwstatus->wait_sum = ssp->wait_sum - ssp->hwstatus.wait_sum;
	hwstatus->sum_sleep_runtime = ssp->sum_sleep_runtime -
		ssp->hwstatus.sum_sleep_runtime;

	hwstatus->iowait_max = ssp->hwstatus.iowait_max;
	hwstatus->iowait_sum = ssp->iowait_sum - ssp->hwstatus.iowait_sum;

	hwstatus->dstate_block_max = ssp->hwstatus.dstate_block_max;
	hwstatus->dstate_block_sum = ssp->hwstatus.dstate_block_sum;

	hwstatus->wait_count = ssp->wait_count - ssp->hwstatus.wait_count;
	hwstatus->sleep_count = ssp->hwstatus.sleep_count;
	hwstatus->iowait_count = ssp->iowait_count - ssp->hwstatus.iowait_count;
	hwstatus->dstate_block_count = ssp->hwstatus.dstate_block_count;
}

static inline void record_mem(struct hwstatus_mem *mem,
	struct task_struct *taskp)
{
#ifdef CONFIG_HW_MEMORY_MONITOR
	if (taskp->delays) {
		mem->allocuser_delay = taskp->delays->allocuser_delay;
		mem->allocuser_count = taskp->delays->allocuser_count;
		mem->allocuser_delay_max = taskp->delays->allocuser_delay_max;
		mem->allocuser_delay_max_order =
			taskp->delays->allocuser_delay_max_order;
	} else {
		mem->allocuser_delay = 0;
		mem->allocuser_count = 0;
		mem->allocuser_delay_max = 0;
		mem->allocuser_delay_max_order = 0;
	}
#else
	mem->allocuser_delay = 0;
	mem->allocuser_count = 0;
	mem->allocuser_delay_max = 0;
	mem->allocuser_delay_max_order = 0;
#endif
}

static inline void record_caller(struct hwstatus_caller *hw_caller,
	struct hwstatus_stack *hw_stack, struct task_struct *taskp, int version)
{
	if (caller.ktime_iodelay < ktime_last) {
		hw_caller->ktime_iodelay  = 0;
		hw_caller->caller_iodelay[0] = '\0';
		return;
	}

	hw_caller->ktime_iodelay = caller.ktime_iodelay;
	strncpy_s(hw_caller->caller_iodelay, CALLER_NAME_LEN,
		caller.caller_iodelay, CALLER_NAME_LEN - 1);

	if (version > VERSION_V2)
		strncpy_s(hw_stack->stack, STACK_DUMP_SIZE,
			stack_dump.stack, STACK_DUMP_SIZE - 1);
}

static inline void record_task_info(struct sched_data *data,
	struct task_struct *taskp)
{
	memcpy_s(data->task_info.comm, sizeof(data->task_info.comm),
		taskp->comm, TASK_COMM_LEN);
	data->task_info.pid = taskp->pid;
	data->task_info.tgid = taskp->tgid;
	data->task_info.target_cpu = (int) task_cpu(taskp);

#if ((defined(CONFIG_HW_VIP_THREAD)) || (defined(CONFIG_HW_QOS_THREAD)))
	data->task_info.static_vip = taskp->static_vip;
#endif

#ifdef CONFIG_HW_VIP_THREAD
	data->task_info.dynamic_vip = atomic64_read(&taskp->dynamic_vip);
	data->task_info.enqueue_time = taskp->enqueue_time;
	data->task_info.vip_depth = taskp->vip_depth;
#endif
}

static void record_qos(struct sched_data *data, struct task_struct *taskp,
	int qos_type)
{
#ifdef CONFIG_HW_QOS_THREAD
	struct transact_qos *tq_iter = NULL;
	struct transact_qos *tq_highest = NULL;
	int qos_iter = -1;
	int qos_highest = -1;
	int i;

	data->qos.dynamic_qos = get_task_set_qos(taskp);
	if (qos_type >= 0 && qos_type < DYNAMIC_QOS_TYPE_MAX) {
		tq_highest = &(taskp->trans_qos[qos_type]);
	} else {
		for (i = DYNAMIC_QOS_BINDER; i < DYNAMIC_QOS_TYPE_MAX; i++) {
			tq_iter = &(taskp->trans_qos[i]);
			if (tq_iter->trans_from == NULL)
				continue;

			qos_iter = get_task_trans_qos(tq_iter);
			if (qos_iter > qos_highest) {
				qos_highest = qos_iter;
				tq_highest = tq_iter;
			}
		}
	}

	if (tq_highest != NULL) {
		data->qos.trans_qos = get_task_trans_qos(tq_highest);
		data->qos.trans_type = tq_highest->trans_type;
		data->qos.trans_from = tq_highest->trans_pid;
	}
#endif
}

static void sched_data_record(struct sched_data *data,
	struct task_struct *taskp, int qos_type)
{
	struct sched_statistics *ssp  = &(taskp->se.statistics);

	data->ktime = ktime_get_ns();
	data->rtime = ktime_get_real_ns();

	record_task_info(data, taskp);
	record_qos(data, taskp, qos_type);
	record_hwstatus(&(data->hwstatus), ssp);
	record_mem(&(data->mem), taskp);
}

static bool sched_buffer_create(void)
{
	printk("create sched_buffer begin");
	g_buffer_size = SCHED_BUFFER_DATA_COUNT * sizeof(struct sched_data) +
		sizeof(struct sched_buffer);
	g_buffer = kmalloc(g_buffer_size, GFP_ATOMIC);
	if (g_buffer == NULL) {
		printk("create sched_buffer failed");
		return false;
	}

	g_buffer->write_index = 0;
	g_buffer->data_count = 0;
	printk("create sched_buffer success");

	return true;
}

static void sched_buffer_put(struct sched_data *data)
{
	struct sched_data *write_ptr;

	if (g_buffer == NULL)
		return;

	if (atomic_read(&g_buffer_read_ref) > 0)
		return;

	spin_lock(&g_buffer_lock);

	write_ptr = (struct sched_data *)g_buffer->data;
	write_ptr += g_buffer->write_index;
	memcpy_s(write_ptr, sizeof(struct sched_data), data,
		sizeof(struct sched_data));
	g_buffer->write_index++;
	if (g_buffer->write_index >= SCHED_BUFFER_DATA_COUNT)
		g_buffer->write_index = 0;

	if (g_buffer->data_count < SCHED_BUFFER_DATA_COUNT)
		g_buffer->data_count++;

	spin_unlock(&g_buffer_lock);
}

static void sched_buffer_put_vip_data(void)
{
	struct sched_data data;
	struct task_struct *taskp;
	int i;

	for (i = 0; i < FGTASK_MAX; i++) {
		memset_s(&data, sizeof(struct sched_data), 0,
			sizeof(struct sched_data));

		taskp = find_task_by_vpid(fgtasks[i]);
		if (taskp != NULL) {
			get_task_struct(taskp);
			sched_data_record(&data, taskp, -1);
			put_task_struct(taskp);
		}

		memcpy_s(&(g_buffer->vip_data[i]), sizeof(struct sched_data),
			&data, sizeof(struct sched_data));
	}
}

static ssize_t sched_buffer_read(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	ssize_t ret;

	if (count < g_buffer_size)
		return HW_SCHED_INVALID_COUNT;

	sched_buffer_put_vip_data();

	atomic_inc(&g_buffer_read_ref);
	spin_lock(&g_buffer_lock);
	ret = simple_read_from_buffer(buf, count, ppos, g_buffer,
		g_buffer_size);
	spin_unlock(&g_buffer_lock);
	atomic_dec(&g_buffer_read_ref);

	return ret;
}

static inline bool is_beta_user(void)
{
	static unsigned int user_type;

	if (user_type == 0)
		user_type = get_logusertype_flag();

	return user_type == BETA_USER;
}

static inline bool is_buffer_init(void)
{
	if (g_is_buffer_init_success)
		return true;

	// Already init but failed
	if (g_is_buffer_already_init)
		return false;

	// Init
	spin_lock(&g_buffer_lock);
	if (!g_is_buffer_already_init) {
		g_is_buffer_already_init = true;
		g_is_buffer_init_success = sched_buffer_create();
	}
	spin_unlock(&g_buffer_lock);

	return g_is_buffer_init_success;
}

static void simple_record_stack(struct task_struct *tsk, char *buf, int max)
{
	struct stackframe frame;
	int pos = 0;
	int len = 0;

	if (!tsk)
		tsk = current;

	if (!try_get_task_stack(tsk))
		return;

	if (tsk == current) {
		frame.fp = (unsigned long)__builtin_frame_address(0);
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		frame.sp = current_stack_pointer;
#endif
		frame.pc = (unsigned long)simple_record_stack;
	} else {
		frame.fp = thread_saved_fp(tsk);
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		frame.sp = thread_saved_sp(tsk);
#endif
		frame.pc = thread_saved_pc(tsk);
	}

	while (1) {
		unsigned long where = frame.pc;
		int ret;

		if (max <= pos)
			break;
		len = snprintf_s(buf + pos, max - pos, max - pos - 1, "%pS\n",
			(void *) where);
		if ((len <= 0) || (len >= (max-pos)))
			break;
		pos += len;
		ret = unwind_frame(tsk, &frame);
		if (ret < 0)
			break;
	}

	put_task_stack(tsk);

	barrier();
}

static inline void clear_hwstatus(struct task_struct *taskp)
{
	struct sched_statistics *ssp;

	ssp = &(taskp->se.statistics);
	ssp->hwstatus.last_jiffies = jiffies;

	ssp->hwstatus.sum_exec_runtime_big = 0;
	ssp->hwstatus.sum_exec_runtime_mid = 0;
	ssp->hwstatus.sum_exec_runtime_ltt = 0;

	ssp->hwstatus.wait_sum = ssp->wait_sum;
	ssp->hwstatus.sum_sleep_runtime = ssp->sum_sleep_runtime;

	ssp->hwstatus.iowait_sum = ssp->iowait_sum;
	ssp->hwstatus.iowait_max = 0;

	ssp->hwstatus.dstate_block_sum = 0;
	ssp->hwstatus.dstate_block_max = 0;

	ssp->hwstatus.wait_count = ssp->wait_count;
	ssp->hwstatus.sleep_count = 0;
	ssp->hwstatus.iowait_count = ssp->iowait_count;
	ssp->hwstatus.dstate_block_count = 0;
}

static inline void clear_delays(struct task_struct *taskp)
{
#ifdef CONFIG_HW_MEMORY_MONITOR
	unsigned long flags;

	if (taskp->delays) {
		raw_spin_lock_irqsave(&taskp->delays->allocpages_lock, flags);
		taskp->delays->allocuser_delay = 0;
		taskp->delays->allocuser_count = 0;
		taskp->delays->allocuser_delay_max = 0;
		taskp->delays->allocuser_delay_max_order = 0;
		raw_spin_unlock_irqrestore(&taskp->delays->allocpages_lock,
			flags);
	}
#endif
}

static void clear_task(struct task_struct *taskp)
{
	get_task_struct(taskp);

	clear_hwstatus(taskp);
	clear_delays(taskp);

	put_task_struct(taskp);
}

static void clear_pid(pid_t pid)
{
	struct task_struct *taskp;

	taskp = find_task_by_vpid(pid);
	if (!taskp)
		return;

	clear_task(taskp);
}

static void sched_hwstatus_clear(pid_t pid)
{
	if (pid == 0) {
		clear_pid(fgtasks[0]);
		clear_pid(fgtasks[1]);
	} else {
		clear_pid(pid);
	}
}

void sched_hwstatus_iodelay_caller(struct task_struct *tsk, u64 delta)
{
	void *__caller;
	u64 delaymax;

	if (!is_beta_user())
		return;

	if (tsk->in_iowait)
		delaymax = M_IODELAY_VALUE;
	else
		delaymax = M_DSDELAY_VALUE;

	if (delta < delaymax)
		return;

	if ((tsk) && (tsk->static_vip)) {
		if (tsk->pid != tsk->tgid) {
			if ((tsk->comm[0] != 'R') || (tsk->comm[1] != 'e'))
				return;
		}

		if (delta_max < delta)
			delta_max = delta;
		else
			return;

		__caller = (void *)get_wchan(tsk);
		caller.ktime_iodelay = ktime_get_ns();
		snprintf_s(caller.caller_iodelay, CALLER_NAME_LEN,
			CALLER_NAME_LEN - 1,
			"[%d,%d]%s W:%d P:[%d,%d] N:%s T:%llu %pS\n",
			current->pid, current->tgid, current->comm,
			tsk->in_iowait, tsk->pid, tsk->tgid, tsk->comm,
			delta_max, __caller);
		simple_record_stack(tsk, stack_dump.stack, STACK_DUMP_SIZE);
	}
}

void sched_hwstatus_updatefg(pid_t pid, pid_t tgid)
{
	if (!is_beta_user())
		return;

	if (tgid != fgtasks[FG_UI]) {
		sched_hwstatus_clear(pid);
		if (pid != tgid) {
			fgtasks[PREV_UI]     = fgtasks[FG_UI];
			fgtasks[PREV_RENDER] = fgtasks[FG_RENDER];

			fgtasks[FG_UI]     = tgid;
			fgtasks[FG_RENDER] = pid;
		}
	}
}

void sched_hwstatus_qos_enqueue(struct task_struct *task, int qos_type)
{
	if (task == NULL)
		return;

	if (!is_beta_user())
		return;

	if (!is_buffer_init())
		return;

	clear_task(task);
}

void sched_hwstatus_qos_dequeue(struct task_struct *task, int qos_type)
{
	struct sched_data data;

	if (task == NULL)
		return;

	if (!is_beta_user())
		return;

	if (!is_buffer_init())
		return;

	sched_data_record(&data, task, qos_type);
	sched_buffer_put(&data);
}

static ssize_t sched_hwstatus_read_sched_buffer(struct file *file,
			char __user *buf, size_t count, loff_t *ppos)
{
	ssize_t ret;

	if (!g_is_buffer_already_init)
		return HW_SCHED_BUFFER_DISABLE;

	if (!g_is_buffer_init_success)
		return HW_SCHED_BUFFER_CREATE_FAILED;

	ret = sched_buffer_read(file, buf, count, ppos);

	return ret;
}

static int sched_hwstatus_show(struct seq_file *m, void *v)
{
	int i;

	if (!is_beta_user())
		return 0;

	for (i = 0; i < FGTASK_MAX; i++) {
		struct task_struct *taskp;
		sched_hwstatus *statusp;
		u64 wait_sum;
		u64 sum_sleep_runtime;
		u64 iowait_sum;
		u64 wait_count;
		u64 iowait_count;

		taskp = find_task_by_vpid(fgtasks[i]);
		if (!taskp) {
			SEQ_printf(m, "### %s find_task_by_vpid err!\n",
				__func__);
			return 0;
		}

		get_task_struct(taskp);
		statusp = &(taskp->se.statistics.hwstatus);
		SEQ_printf(m, "pid:%d,jiffies:%llu\n", fgtasks[i], jiffies -
			statusp->last_jiffies);
		wait_sum = taskp->se.statistics.wait_sum - statusp->wait_sum;
		sum_sleep_runtime = taskp->se.statistics.sum_sleep_runtime -
			statusp->sum_sleep_runtime;
		iowait_sum = taskp->se.statistics.iowait_sum -
			statusp->iowait_sum;
		PN(statusp->sum_exec_runtime_big);
		PN(statusp->sum_exec_runtime_mid);
		PN(statusp->sum_exec_runtime_ltt);
		PN(wait_sum);
		PN(sum_sleep_runtime);
		PN(iowait_sum);
		PN(statusp->iowait_max);

		PN(statusp->dstate_block_max);
		PN(statusp->dstate_block_sum);

		PN(statusp->dstate_block_count);
		wait_count = taskp->se.statistics.wait_count -
			statusp->wait_count;
		iowait_count = taskp->se.statistics.iowait_count -
			statusp->iowait_count;
		PN(wait_count);
		PN(iowait_count);
		PN(statusp->sleep_count);
#ifdef CONFIG_HW_MEMORY_MONITOR
		if (taskp->delays) {
			PN(taskp->delays->allocuser_delay);
			PN(taskp->delays->allocuser_count);
			PN(taskp->delays->allocuser_delay_max);
			PN(taskp->delays->allocuser_delay_max_order);
		}
#endif
		put_task_struct(taskp);
	}

	if (caller.ktime_iodelay >= ktime_last)
		SEQ_printf(m, "### blockinfo:%s\n", caller.caller_iodelay);
	return 0;
}

static ssize_t sched_hwstatus_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	int i;
	static unsigned long last_jiffies;
	struct sched_hwstatus_rst_V3 hwstatus_rst;
	int version;

	switch (count) {
	case sizeof(struct sched_hwstatus_rst_V3):
		version = VERSION_V3;
		break;
	case sizeof(struct sched_hwstatus_rst_V2):
		version = VERSION_V2;
		break;
	case sizeof(struct sched_hwstatus_rst_V1):
		version = VERSION_V1;
		break;
	default:
		return HW_SCHED_INVALID_COUNT;
	}

	if ((jiffies - last_jiffies) < HZ)
		return HW_SCHED_NOT_TIME_YET;

	last_jiffies = jiffies;

	for (i = 0; i < FGTASK_MAX; i++) {
		struct task_struct *taskp;
		struct sched_statistics *ssp;
		struct sched_hwstatus_rstbody *rstp;

		taskp = find_task_by_vpid(fgtasks[i]);
		if (!taskp)
			return HW_SCHED_INVALID_TASK_STRUCT;

		get_task_struct(taskp);
		ssp = &(taskp->se.statistics);
		rstp = &hwstatus_rst.body[i];
		rstp->pid = fgtasks[i];
		rstp->tgid = fgtasks[(i >> 1) << 1];
		record_hwstatus(&(rstp->hwstatus), ssp);

		if (version > VERSION_V1) {
			record_mem(&(hwstatus_rst.mem[i]), taskp);
			record_caller(&(hwstatus_rst.caller),
				&(hwstatus_rst.stack), taskp, version);
		}

		put_task_struct(taskp);
	}

	hwstatus_rst.head.ktime_last = ktime_last;
	hwstatus_rst.head.ktime_now = ktime_get_ns();

	return simple_read_from_buffer(buf, count, ppos, &hwstatus_rst, count);
}

static ssize_t sched_hwstatus_read_for_iocollector(struct file *file,
	char __user *buf, size_t count, loff_t *ppos)
{
	int i = 0;
	struct sched_hwstatus_rst_iocollector hwstatus_rst;
	struct task_struct *taskp = NULL;
	struct sched_statistics *ssp = NULL;
	struct sched_hwstatus_rstbody *rstp = NULL;

	if (count < sizeof(struct sched_hwstatus_rst_iocollector))
		return HW_SCHED_INVALID_COUNT;

	taskp = find_task_by_vpid(fgtasks[i]);
	if (!taskp)
		return HW_SCHED_INVALID_TASK_STRUCT;

	get_task_struct(taskp);
	ssp = &(taskp->se.statistics);
	rstp = &hwstatus_rst.body;
	rstp->pid = fgtasks[i];
	rstp->tgid = fgtasks[(i >> 1) << 1];
	record_hwstatus(&(rstp->hwstatus), ssp);

	put_task_struct(taskp);

	hwstatus_rst.head.ktime_last = ktime_last;
	hwstatus_rst.head.ktime_now = ktime_get_ns();

	return simple_read_from_buffer(buf, count, ppos, &hwstatus_rst, count);
}

static ssize_t sched_hwstatus_read_comm(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	unsigned int magic = 0;
	ssize_t ret = -1;

	if (!is_beta_user())
		return HW_SCHED_NOT_BETA_USER;

	if (count <= sizeof(magic))
		return HW_SCHED_INVALID_COUNT;

	if (copy_from_user(&magic, buf, sizeof(magic)))
		return HW_SCHED_COPY_FROM_USER_FAILED;

	switch (magic) {
	case HW_SCHED_MAGIC_SCHED_BUFFER:
		ret = sched_hwstatus_read_sched_buffer(file, buf, count, ppos);
		break;
	case HW_SCHED_MAGIC_IOCOLLECTOR:
		ret = sched_hwstatus_read_for_iocollector(file, buf, count,
			ppos);
		break;
	case HW_SCHED_MAGIC_DEFAULT:
		ret = sched_hwstatus_read(file, buf, count, ppos);
		break;
	default:
		ret = sched_hwstatus_read(file, buf, count, ppos);
		break;
	}

	return ret;
}

static ssize_t sched_hwstatus_write(struct file *file, const char __user *buf,
			size_t count, loff_t *ppos)
{
	pid_t pid = 0;
	int rv;
	static unsigned long last_jiffies;

	if (!is_beta_user())
		return HW_SCHED_NOT_BETA_USER;

	if (count > 10)
		return HW_SCHED_INVALID_COUNT;

	if ((jiffies - last_jiffies) < HZ)
		return HW_SCHED_NOT_TIME_YET;

	last_jiffies = jiffies;
	rv = kstrtos32_from_user(buf, count, 10, &pid);
	if (rv < 0)
		return rv;

	if (pid == 0) {
		sched_hwstatus_show(NULL, NULL);
		return count;
	}

	sched_hwstatus_clear(0);
	ktime_last = ktime_get_ns();
	delta_max = 0;

	return count;
}

static const struct file_operations sched_hwstatus_fops = {
	.read		= sched_hwstatus_read_comm,
	.write		= sched_hwstatus_write,
};

static int __init init_sched_hwstatus_procfs(void)
{
	struct proc_dir_entry *pe;

	pe = proc_create("sched_hw", 0660, NULL, &sched_hwstatus_fops);
	if (!pe)
		return -ENOMEM;

	spin_lock_init(&g_buffer_lock);
	atomic_set(&g_buffer_read_ref, 0);
	return 0;
}

device_initcall(init_sched_hwstatus_procfs);
