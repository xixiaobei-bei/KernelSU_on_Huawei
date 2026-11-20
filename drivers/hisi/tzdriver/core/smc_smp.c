/*
 * smc_smp.c
 *
 * function for sending smc cmd.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "smc_smp.h"
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/cpu.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/string.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/version.h>
#include <linux/cpumask.h>
#include <linux/err.h>

#ifdef CONFIG_HISI_SECS_CTRL
#include <linux/hisi/secs_power_ctrl.h>
#endif

#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#include <securec.h>
#include <asm/compiler.h>
#include <asm/cacheflush.h>

#ifdef CONFIG_TEE_AUDIT
#include <chipset_common/security/hw_kernel_stp_interface.h>
#endif

#ifdef CONFIG_TEE_LOG_EXCEPTION
#include <huawei_platform/log/imonitor.h>
#define IMONITOR_TA_CRASH_EVENT_ID           901002003
#endif

#include "tc_ns_log.h"
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "agent.h"
#include "teek_ns_client.h"
#include "mailbox_mempool.h"
#include "cmdmonitor.h"
#include "tui.h"
#include "security_auth_enhance.h"
#include "tlogger.h"
#include "ko_adapt.h"
#include "log_cfg_api.h"

#ifndef CONFIG_ARM64
#error "TEE SMP has ARM64 support only"
#endif

#define SECS_SUSPEND_STATUS      0xA5A5
#define PREEMPT_COUNT            10000
#define HZ_COUNT                 10
#define IDLED_COUNT              100

#define CPU_ZERO    0
#define CPU_ONE     1
#define CPU_FOUR    4
#define CPU_FIVE    5
#define CPU_SIX     6
#define CPU_SEVEN   7
#define LOW_BYTE    0xF

#define PENDING2_RETRY      (-1)

#define RETRY_WITH_PM     1
#define CLEAN_WITHOUT_PM  2

#define MAX_CHAR 0xff

/* Current state of the system */
static uint8_t g_sys_crash;

struct smc_in_params {
	u64 x0;
	u64 x1;
	u64 x2;
	u64 x3;
	u64 x4;
};

struct smc_out_params {
	u64 ret;
	u64 exit_reason;
	u64 ta;
	u64 target;
};

struct shadow_work {
	struct kthread_work kthwork;
	struct work_struct work;
	uint64_t target;
};

unsigned long g_shadow_thread_id = 0;
static struct task_struct *g_siq_thread;
static struct task_struct *g_smc_svc_thread;
static struct task_struct *g_ipi_helper_thread;
static DEFINE_KTHREAD_WORKER(g_ipi_helper_worker);

enum cmd_reuse {
	CLEAR,      /* clear this cmd index */
	RESEND,     /* use this cmd index resend */
};

#if CONFIG_CPU_AFF_NR
static struct cpumask g_cpu_mask;
static int g_mask_flag = 0;
#endif

#ifdef CONFIG_DRM_ADAPT
static struct cpumask g_drm_cpu_mask;
static int g_drm_mask_flag = 0;
#endif

struct tc_ns_smc_queue *g_cmd_data;
phys_addr_t g_cmd_phys;

static struct list_head g_pending_head;
static spinlock_t g_pend_lock;

static DECLARE_WAIT_QUEUE_HEAD(siq_th_wait);
static DECLARE_WAIT_QUEUE_HEAD(ipi_th_wait);
static atomic_t g_siq_th_run;

enum {
	TYPE_CRASH_TA  = 1,
	TYPE_CRASH_TEE = 2,
};

enum smc_ops_exit {
	SMC_OPS_NORMAL   = 0x0,
	SMC_OPS_SCHEDTO  = 0x1,
	SMC_OPS_START_SHADOW    = 0x2,
	SMC_OPS_START_FIQSHD    = 0x3,
	SMC_OPS_PROBE_ALIVE     = 0x4,
	SMC_OPS_ABORT_TASK      = 0x5,
	SMC_EXIT_NORMAL         = 0x0,
	SMC_EXIT_PREEMPTED      = 0x1,
	SMC_EXIT_SHADOW         = 0x2,
	SMC_EXIT_ABORT          = 0x3,
	SMC_EXIT_MAX            = 0x4,
};

#define SHADOW_EXIT_RUN             0x1234dead
#define SMC_EXIT_TARGET_SHADOW_EXIT 0x1

#define SYM_NAME_LEN_MAX 16
#define SYM_NAME_LEN_1 7
#define SYM_NAME_LEN_2 4
#define CRASH_REG_NUM  3
#define LOW_FOUR_BITE  4

union crash_inf {
	uint64_t crash_reg[CRASH_REG_NUM];
	struct {
		uint8_t halt_reason : LOW_FOUR_BITE;
		uint8_t app : LOW_FOUR_BITE;
		char sym_name[SYM_NAME_LEN_1];
		uint16_t off;
		uint16_t size;
		uint32_t far;
		uint32_t fault;
		union {
			char sym_name_append[SYM_NAME_LEN_2];
			uint32_t elr;
		};
	} crash_msg;
};

static const char *g_hungtask_monitor_list[] = {
	"system_server", "fingerprintd", "atcmdserver", "keystore",
	"gatekeeperd", "volisnotd", "secure_storage", "secure_storage_s",
	"mediaserver", "vold", "IFAAPluginThrea",
};

#define compile_time_assert(cond, msg) typedef char ASSERT_##msg[(cond) ? 1 : -1]

compile_time_assert(sizeof(struct tc_ns_smc_queue) <= PAGE_SIZE,
	size_of_tc_ns_smc_queue_too_large);

struct tc_ns_smc_queue *get_cmd_data_buffer(void)
{
	return g_cmd_data;
}

static void acquire_smc_buf_lock(smc_buf_lock_t *lock)
{
	int ret;

	preempt_disable();
	do
		ret = cmpxchg(lock, 0, 1);
	while (ret);
}

static inline void release_smc_buf_lock(smc_buf_lock_t *lock)
{
	(void)cmpxchg(lock, 1, 0);
	preempt_enable();
}

static int occupy_free_smc_in_entry(const struct tc_ns_smc_cmd *cmd)
{
	int idx = -1;
	int i;

	if (!cmd) {
		tloge("bad parameters! cmd is NULL\n");
		return -1;
	}
	/*
	 * Note:
	 * acquire_smc_buf_lock will disable preempt and kernel will forbid
	 * call mutex_lock in preempt disabled scenes.
	 * To avoid such case(update_timestamp and update_chksum will call
	 * mutex_lock), only cmd copy is done when preempt is disable,
	 * then do update_timestamp and update_chksum.
	 * As soon as this idx of in_bitmap is set, gtask will see this
	 * cmd_in, but the cmd_in is not ready that lack of update_xxx,
	 * so we make a tricky here, set doing_bitmap and in_bitmap both
	 * at first, after update_xxx is done, clear doing_bitmap.
	 */
	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	for (i = 0; i < MAX_SMC_CMD; i++) {
		if (test_bit(i, g_cmd_data->in_bitmap))
			continue;
		if (memcpy_s(&g_cmd_data->in[i], sizeof(g_cmd_data->in[i]),
			cmd, sizeof(*cmd)) != EOK) {
			tloge("memcpy failed,%s line:%d", __func__, __LINE__);
			break;
		}
		g_cmd_data->in[i].event_nr = i;
		isb();
		wmb();
		set_bit(i, g_cmd_data->in_bitmap);
		set_bit(i, g_cmd_data->doing_bitmap);
		idx = i;
		break;
	}
	release_smc_buf_lock(&g_cmd_data->smc_lock);
	if (idx == -1) {
		tloge("can't get any free smc entry\n");
		return -1;
	}

	if (update_timestamp(&g_cmd_data->in[idx])) {
		tloge("update timestamp failed!\n");
		goto clean;
	}
	if (update_chksum(&g_cmd_data->in[idx])) {
		tloge("update chksum failed\n");
		goto clean;
	}

	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	isb();
	wmb();
	clear_bit(idx, g_cmd_data->doing_bitmap);
	release_smc_buf_lock(&g_cmd_data->smc_lock);
	return idx;

clean:
	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	clear_bit(i, g_cmd_data->in_bitmap);
	clear_bit(i, g_cmd_data->doing_bitmap);
	release_smc_buf_lock(&g_cmd_data->smc_lock);

	return -1;
}

static int reuse_smc_in_entry(uint32_t idx)
{
	int rc = 0;

	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	if (!(test_bit(idx, g_cmd_data->in_bitmap) &&
		test_bit(idx, g_cmd_data->doing_bitmap))) {
		tloge("invalid cmd to reuse\n");
		rc = -1;
		goto out;
	}
	if (memcpy_s(&g_cmd_data->in[idx], sizeof(g_cmd_data->in[idx]),
		&g_cmd_data->out[idx], sizeof(g_cmd_data->out[idx]))) {
		tloge("memcpy failed,%s line:%d", __func__, __LINE__);
		rc = -1;
		goto out;
	}
	release_smc_buf_lock(&g_cmd_data->smc_lock);
	if (update_timestamp(&g_cmd_data->in[idx])) {
		tloge("update timestamp failed!\n");
		return -1;
	}
	if (update_chksum(&g_cmd_data->in[idx])) {
		tloge("update chksum failed\n");
		return -1;
	}

	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	isb();
	wmb();
	clear_bit(idx, g_cmd_data->doing_bitmap);
out:
	release_smc_buf_lock(&g_cmd_data->smc_lock);
	return rc;
}

static int copy_smc_out_entry(uint32_t idx, struct tc_ns_smc_cmd *copy,
	enum cmd_reuse *usage)
{
	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	if (!test_bit(idx, g_cmd_data->out_bitmap)) {
		tloge("cmd out %u is not ready\n", idx);
		release_smc_buf_lock(&g_cmd_data->smc_lock);
		show_cmd_bitmap();
		return -ENOENT;
	}
	if (memcpy_s(copy, sizeof(*copy), &g_cmd_data->out[idx],
		sizeof(g_cmd_data->out[idx]))) {
		tloge("copy smc out failed\n");
		release_smc_buf_lock(&g_cmd_data->smc_lock);
		return -EFAULT;
	}

	isb();
	wmb();
	if (g_cmd_data->out[idx].ret_val == TEEC_PENDING2 ||
		g_cmd_data->out[idx].ret_val == TEEC_PENDING) {
		*usage = RESEND;
	} else {
		clear_bit(idx, g_cmd_data->in_bitmap);
		clear_bit(idx, g_cmd_data->doing_bitmap);
		*usage = CLEAR;
	}
	clear_bit(idx, g_cmd_data->out_bitmap);
	release_smc_buf_lock(&g_cmd_data->smc_lock);

	return 0;
}

static inline void clear_smc_in_entry(uint32_t idx)
{
	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	clear_bit(idx, g_cmd_data->in_bitmap);
	release_smc_buf_lock(&g_cmd_data->smc_lock);
}

static void release_smc_entry(uint32_t idx)
{
	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	clear_bit(idx, g_cmd_data->in_bitmap);
	clear_bit(idx, g_cmd_data->doing_bitmap);
	clear_bit(idx, g_cmd_data->out_bitmap);
	release_smc_buf_lock(&g_cmd_data->smc_lock);
}

static bool is_cmd_working_done(uint32_t idx)
{
	bool ret = false;

	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	if (test_bit(idx, g_cmd_data->out_bitmap))
		ret = true;
	release_smc_buf_lock(&g_cmd_data->smc_lock);
	return ret;
}

static void show_in_bitmap(int *cmd_in, uint32_t len)
{
	uint32_t idx;
	uint32_t in = 0;
	char bitmap[MAX_SMC_CMD + 1];

	if (len != MAX_SMC_CMD || !g_cmd_data)
		return;

	for (idx = 0; idx < MAX_SMC_CMD; idx++) {
		if (test_bit(idx, g_cmd_data->in_bitmap)) {
			bitmap[idx] = '1';
			cmd_in[in++] = idx;
		} else {
			bitmap[idx] = '0';
		}
	}
	bitmap[MAX_SMC_CMD] = '\0';
	tloge("in bitmap: %s\n", bitmap);
}

static void show_out_bitmap(int *cmd_out, uint32_t len)
{
	uint32_t idx;
	uint32_t out = 0;
	char bitmap[MAX_SMC_CMD + 1];

	if (len != MAX_SMC_CMD || !g_cmd_data)
		return;

	for (idx = 0; idx < MAX_SMC_CMD; idx++) {
		if (test_bit(idx, g_cmd_data->out_bitmap)) {
			bitmap[idx] = '1';
			cmd_out[out++] = idx;
		} else {
			bitmap[idx] = '0';
		}
	}
	bitmap[MAX_SMC_CMD] = '\0';
	tloge("out bitmap: %s\n", bitmap);
}

static void show_doing_bitmap(void)
{
	uint32_t idx;
	char bitmap[MAX_SMC_CMD + 1];

	if (!g_cmd_data)
		return;
	for (idx = 0; idx < MAX_SMC_CMD; idx++) {
		if (test_bit(idx, g_cmd_data->doing_bitmap))
			bitmap[idx] = '1';
		else
			bitmap[idx] = '0';
	}
	bitmap[MAX_SMC_CMD] = '\0';
	tloge("doing bitmap: %s\n", bitmap);
}

void show_cmd_bitmap(void)
{
	uint32_t idx;
	int *cmd_in = NULL;
	int *cmd_out = NULL;

	cmd_in = kzalloc(sizeof(int) * MAX_SMC_CMD, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)cmd_in)) {
		tloge("out of mem! cannot show in bitmap\n");
		return;
	}

	cmd_out = kzalloc(sizeof(int) * MAX_SMC_CMD, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)cmd_out)) {
		kfree(cmd_in);
		tloge("out of mem! cannot show out bitmap\n");
		return;
	}

	if (memset_s(cmd_in, sizeof(int) * MAX_SMC_CMD, MAX_CHAR, sizeof(int) * MAX_SMC_CMD) ||
		memset_s(cmd_out, sizeof(int) * MAX_SMC_CMD, MAX_CHAR, sizeof(int) * MAX_SMC_CMD)) {
		tloge("memset failed\n");
		goto error;
	}

	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	show_in_bitmap(cmd_in, MAX_SMC_CMD);
	show_doing_bitmap();
	show_out_bitmap(cmd_out, MAX_SMC_CMD);

	tloge("cmd in value:\n");
	for (idx = 0; idx < MAX_SMC_CMD; idx++) {
		if (cmd_in[idx] == -1)
			break;
		tloge("cmd[%d]: cmd_id=%u, ca_pid=%u, dev_id = 0x%x, event_nr=%u, ret_val=0x%x\n",
			cmd_in[idx], g_cmd_data->in[cmd_in[idx]].cmd_id,
			g_cmd_data->in[cmd_in[idx]].ca_pid, g_cmd_data->in[cmd_in[idx]].dev_file_id,
			g_cmd_data->in[cmd_in[idx]].event_nr, g_cmd_data->in[cmd_in[idx]].ret_val);
	}

	tloge("cmd_out value:\n");
	for (idx = 0; idx < MAX_SMC_CMD; idx++) {
		if (cmd_out[idx] == -1)
			break;
		tloge("cmd[%d]: cmd_id=%u, ca_pid=%u, dev_id = 0x%x, event_nr=%u, ret_val=0x%x\n",
			cmd_out[idx], g_cmd_data->out[cmd_out[idx]].cmd_id,
			g_cmd_data->out[cmd_out[idx]].ca_pid, g_cmd_data->out[cmd_out[idx]].dev_file_id,
			g_cmd_data->out[cmd_out[idx]].event_nr, g_cmd_data->out[cmd_out[idx]].ret_val);
	}
	release_smc_buf_lock(&g_cmd_data->smc_lock);

error:
	kfree(cmd_in);
	kfree(cmd_out);
}

static struct pending_entry *init_pending_entry(void)
{
	struct pending_entry *pe = NULL;

	pe = kzalloc(sizeof(*pe), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)pe)) {
		tloge("alloc pe failed\n");
		return NULL;
	}

	atomic_set(&pe->users, 1);
	get_task_struct(current);
	pe->task = current;

#ifdef CONFIG_TA_AFFINITY
	cpumask_copy(&pe->ca_mask, &current->cpus_allowed);
	cpumask_copy(&pe->ta_mask, &current->cpus_allowed);
#endif

	init_waitqueue_head(&pe->wq);
	atomic_set(&pe->run, 0);
	INIT_LIST_HEAD(&pe->list);
	spin_lock(&g_pend_lock);
	list_add_tail(&pe->list, &g_pending_head);
	spin_unlock(&g_pend_lock);

	return pe;
}

struct pending_entry *find_pending_entry(pid_t pid)
{
	struct pending_entry *pe = NULL;

	spin_lock(&g_pend_lock);
	list_for_each_entry(pe, &g_pending_head, list) {
		if (pe->task->pid == pid) {
			atomic_inc(&pe->users);
			spin_unlock(&g_pend_lock);
			return pe;
		}
	}
	spin_unlock(&g_pend_lock);
	return NULL;
}

void foreach_pending_entry(void (*func)(struct pending_entry *))
{
	struct pending_entry *pe = NULL;

	if (!func)
		return;

	spin_lock(&g_pend_lock);
	list_for_each_entry(pe, &g_pending_head, list) {
		func(pe);
	}
	spin_unlock(&g_pend_lock);
}

void put_pending_entry(struct pending_entry *pe)
{
	if (!pe)
		return;

	if (!atomic_dec_and_test(&pe->users))
		return;

	put_task_struct(pe->task);
	kfree(pe);
}

#ifdef CONFIG_TA_AFFINITY
static void restore_cpu_mask(struct pending_entry *pe)
{
	if (cpumask_equal(&pe->ca_mask, &pe->ta_mask))
		return;

	set_cpus_allowed_ptr(current, &pe->ca_mask);
}
#endif

static void release_pending_entry(struct pending_entry *pe)
{
#ifdef CONFIG_TA_AFFINITY
	restore_cpu_mask(pe);
#endif
	spin_lock(&g_pend_lock);
	list_del(&pe->list);
	spin_unlock(&g_pend_lock);
	put_pending_entry(pe);
}

static inline bool is_shadow_exit(uint64_t target)
{
	return target & SMC_EXIT_TARGET_SHADOW_EXIT;
}

/*
 * check ca and ta's affinity is match in 2 scene:
 * 1. when TA is blocked to REE
 * 2. when CA is wakeup by SPI wakeup
 * match_ta_affinity return true if affinity is changed
 */
#ifdef CONFIG_TA_AFFINITY
static bool match_ta_affinity(struct pending_entry *pe)
{
	if (!cpumask_equal(&current->cpus_allowed, &pe->ta_mask)) {
		if (set_cpus_allowed_ptr(current, &pe->ta_mask)) {
			tlogw("set %s affinity failed\n", current->comm);
			return false;
		}
		return true;
	}

	return false;
}
#else
static inline bool match_ta_affinity(struct pending_entry *pe)
{
	return false;
}
#endif

struct smc_cmd_ret {
	u64 exit;
	u64 ta;
	u64 target;
};

bool sigkill_pending(struct task_struct *tsk)
{
	bool flag = false;

	if (!tsk) {
		tloge("tsk is null!\n");
		return false;
	}

	flag = sigismember(&tsk->pending.signal, SIGKILL) ||
		sigismember(&tsk->pending.signal, SIGUSR1);

	if (tsk->signal)
		return flag || sigismember(&tsk->signal->shared_pending.signal,
			SIGKILL);
	return flag;
}

#if CONFIG_CPU_AFF_NR
static void set_cpu_strategy(struct cpumask *old_mask)
{
	unsigned int i;

	if (!g_mask_flag) {
		cpumask_clear(&g_cpu_mask);
		for (i = 0; i < CONFIG_CPU_AFF_NR; i++)
			cpumask_set_cpu(i, &g_cpu_mask);
		g_mask_flag = 1;
	}
	cpumask_copy(old_mask, &current->cpus_allowed);
	set_cpus_allowed_ptr(current, &g_cpu_mask);
}
#endif

#if CONFIG_CPU_AFF_NR
static void restore_cpu(struct cpumask *old_mask)
{
	/* current equal old means no set cpu affinity, no need to restore */
	if (cpumask_equal(&current->cpus_allowed, old_mask))
		return;

	set_cpus_allowed_ptr(current, old_mask);
	schedule();
}
#endif

static bool is_ready_to_kill(bool need_kill)
{
	return (need_kill && sigkill_pending(current) &&
			is_thread_reported(current->pid));
}

static void set_smc_send_arg(struct smc_in_params *in_param,
	const struct smc_cmd_ret *secret, u64 ops)
{
	if (secret->exit == SMC_EXIT_PREEMPTED) {
		in_param->x1 = SMC_OPS_SCHEDTO;
		in_param->x3 = secret->ta;
		in_param->x4 = secret->target;
	}

	if (ops == SMC_OPS_SCHEDTO || ops == SMC_OPS_START_FIQSHD)
		in_param->x4 = secret->target;

	tlogd("[cpu %d]begin send x0=%llx x1=%llx x2=%llx x3=%llx x4=%llx\n",
		raw_smp_processor_id(), in_param->x0, in_param->x1,
		in_param->x2, in_param->x3, in_param->x4);
}

static void send_asm_smc_cmd(struct smc_in_params *in_param,
	struct smc_out_params *out_param)
{
	do {
		asm volatile(
			"mov x0, %[fid]\n"
			"mov x1, %[a1]\n"
			"mov x2, %[a2]\n"
			"mov x3, %[a3]\n"
			"mov x4, %[a4]\n"
			"smc #0\n"
			"str x0, [%[re0]]\n"
			"str x1, [%[re1]]\n"
			"str x2, [%[re2]]\n"
			"str x3, [%[re3]]\n" :
			[fid] "+r"(in_param->x0),
			[a1] "+r"(in_param->x1),
			[a2] "+r"(in_param->x2),
			[a3] "+r"(in_param->x3),
			[a4] "+r"(in_param->x4) :
			[re0] "r"(&out_param->ret),
			[re1] "r"(&out_param->exit_reason),
			[re2] "r"(&out_param->ta),
			[re3] "r"(&out_param->target) :
			"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
			"x8", "x9", "x10", "x11", "x12", "x13",
			"x14", "x15", "x16", "x17");
	} while (0);
}

static noinline int smp_smc_send(uint32_t cmd, u64 ops, u64 ca,
	struct smc_cmd_ret *secret, bool need_kill)
{
	struct smc_in_params in_param = { cmd, ops, ca, 0, 0 };
	struct smc_out_params out_param = {0};
#if CONFIG_CPU_AFF_NR
	struct cpumask old_mask;
#endif

#if CONFIG_CPU_AFF_NR
	set_cpu_strategy(&old_mask);
#endif
retry:
	set_smc_send_arg(&in_param, secret, ops);
	isb();
	wmb();
	send_asm_smc_cmd(&in_param, &out_param);
	isb();
	wmb();
	tlogd("[cpu %d] return val %llx exit_reason %llx ta %llx targ %llx\n",
		raw_smp_processor_id(), out_param.ret, out_param.exit_reason,
		out_param.ta, out_param.target);

	secret->exit = out_param.exit_reason;
	secret->ta = out_param.ta;
	secret->target = out_param.target;

	if (out_param.exit_reason == SMC_EXIT_PREEMPTED) {
		/*
		 * There's 2 ways to send a terminate cmd to kill a running TA,
		 * in current context or another. If send terminate in another
		 * context, may encounter concurrency problem, as terminate cmd
		 * is send but not process, the original cmd has finished.
		 * So we send the terminate cmd in current context.
		 */
		if (is_ready_to_kill(need_kill)) {
			secret->exit = SMC_EXIT_ABORT;
			tloge("receive kill signal\n");
		} else {
#ifndef CONFIG_PREEMPT
			/* yield cpu to avoid soft lockup */
			cond_resched();
#endif
			goto retry;
		}
	}
#if CONFIG_CPU_AFF_NR
	restore_cpu(&old_mask);
#endif
	return out_param.ret;
}

static uint64_t send_smc_cmd(uint32_t cmd, phys_addr_t cmd_addr,
	uint32_t cmd_type, uint8_t wait)
{
	register u64 x0 asm("x0") = cmd;
	register u64 x1 asm("x1") = cmd_addr;
	register u64 x2 asm("x2") = cmd_type;
	register u64 x3 asm("x3") = cmd_addr >> ADDR_TRANS_NUM;

	do {
		asm volatile(
			__asmeq("%0", "x0")
			__asmeq("%1", "x0")
			__asmeq("%2", "x1")
			__asmeq("%3", "x2")
			__asmeq("%4", "x3")
			"smc #0\n" :
			"+r"(x0) :
			"r"(x0), "r"(x1), "r"(x2), "r"(x3));
	} while (x0 == TSP_REQUEST && wait);

	return x0;
}

int raw_smc_send(uint32_t cmd, phys_addr_t cmd_addr,
	uint32_t cmd_type, uint8_t wait)
{
	uint64_t x0;

#if (CONFIG_CPU_AFF_NR != 0)
	struct cpumask old_mask;
	set_cpu_strategy(&old_mask);
#endif
	x0 = send_smc_cmd(cmd, cmd_addr, cmd_type, wait);
#if (CONFIG_CPU_AFF_NR != 0)
	restore_cpu(&old_mask);
#endif
	return x0;
}

void siq_dump(phys_addr_t mode)
{
	raw_smc_send(TSP_REE_SIQ, mode, 0, false);
	tz_log_write();
	do_cmd_need_archivelog();
}

static int siq_thread_fn(void *arg)
{
	int ret;

	while (1) {
		ret = wait_event_interruptible(siq_th_wait,
			atomic_read(&g_siq_th_run));
		if (ret) {
			tloge("wait event interruptible failed!\n");
			return -EINTR;
		}
		atomic_set(&g_siq_th_run, 0);
		siq_dump((phys_addr_t)(1));
	}
}

#ifdef CONFIG_TEE_AUDIT
#define MAX_UPLOAD_INFO_LEN      4
#define INFO_HIGH_OFFSET         24U
#define INFO_MID_OFFSET          16U
#define INFO_LOW_OFFSET          8U

static void upload_audit_event(unsigned int eventindex)
{
#ifdef CONFIG_HW_KERNEL_STP
	struct stp_item item;
	int ret;
	char att_info[MAX_UPLOAD_INFO_LEN + 1] = {0};

	att_info[0] = (unsigned char)(eventindex >> INFO_HIGH_OFFSET);
	att_info[1] = (unsigned char)(eventindex >> INFO_MID_OFFSET);
	att_info[2] = (unsigned char)(eventindex >> INFO_LOW_OFFSET);
	att_info[3] = (unsigned char)eventindex;
	att_info[MAX_UPLOAD_INFO_LEN] = '\0';
	item.id = item_info[ITRUSTEE].id; /* 0x00000185 */
	item.status = STP_RISK;
	item.credible = STP_REFERENCE;
	item.version = 0;
	ret = strcpy_s(item.name, STP_ITEM_NAME_LEN, STP_NAME_ITRUSTEE);
	if (ret) {
		tloge("strncpy failed %x\n", ret);
		return;
	}
	tlogd("stp get size %lx succ\n", sizeof(item_info[ITRUSTEE].name));
	ret = kernel_stp_upload(item, att_info);
	if (ret)
		tloge("stp %x event upload failed\n", eventindex);
	else
		tloge("stp %x event upload succ\n", eventindex);
#else
	(void)eventindex;
#endif
}
#endif

static void cmd_result_check(struct tc_ns_smc_cmd *cmd)
{
	if (cmd->ret_val == TEEC_SUCCESS && verify_chksum(cmd)) {
		cmd->ret_val = TEEC_ERROR_GENERIC;
		tloge("verify chksum failed\n");
	}

	if (cmd->ret_val == TEEC_PENDING || cmd->ret_val == TEEC_PENDING2)
		tlogd("wakeup command %u\n", cmd->event_nr);

	if (cmd->ret_val == TEE_ERROR_TAGET_DEAD) {
		tloge("error smc call: ret = %x and cmd.err_origin=%x\n",
			cmd->ret_val, cmd->err_origin);
		cmd_monitor_ta_crash(TYPE_CRASH_TA);
		ta_crash_report_log();
	} else if (cmd->ret_val == TEEC_ERROR_TUI_NOT_AVAILABLE) {
		do_ns_tui_release();
	} else if (cmd->ret_val == TEE_ERROR_AUDIT_FAIL) {
		tloge("error smc call: ret = %x and err-origin=%x\n",
			cmd->ret_val, cmd->err_origin);
#ifdef CONFIG_TEE_AUDIT
		tloge("error smc call: status = %x and err-origin=%x\n",
			cmd->eventindex, cmd->err_origin);
		upload_audit_event(cmd->eventindex);
#endif
	}
}

static void set_shadow_smc_param(struct smc_in_params *in_params,
	const struct smc_out_params *out_params, int *n_idled)
{
	if (out_params->exit_reason == SMC_EXIT_PREEMPTED) {
		in_params->x0 = TSP_REQUEST;
		in_params->x1 = SMC_OPS_SCHEDTO;
		in_params->x2 = current->pid;
		in_params->x3 = out_params->ta;
		in_params->x4 = out_params->target;
	} else if (out_params->exit_reason == SMC_EXIT_NORMAL) {
		in_params->x0 = TSP_REQUEST;
		in_params->x1 = SMC_OPS_SCHEDTO;
		in_params->x2 = current->pid;
		in_params->x3 = 0;
		in_params->x4 = 0;
		if (*n_idled > IDLED_COUNT) {
			*n_idled = 0;
			in_params->x1 = SMC_OPS_PROBE_ALIVE;
		}
	}
}

static void shadow_wo_pm(const void *arg, struct smc_out_params *out_params,
	int *n_idled)
{
	struct smc_in_params in_params = {
		TSP_REQUEST, SMC_OPS_START_SHADOW, current->pid, 0, *(u64 *)arg
	};

	set_shadow_smc_param(&in_params, out_params, n_idled);
	isb();
	wmb();
	tlogd("%s: [cpu %d] x0=%llx x1=%llx x2=%llx x3=%llx x4=%llx\n",
		__func__, raw_smp_processor_id(), in_params.x0, in_params.x1,
		in_params.x2, in_params.x3, in_params.x4);
	do {
		asm volatile(
			"mov x0, %[fid]\n"
			"mov x1, %[a1]\n"
			"mov x2, %[a2]\n"
			"mov x3, %[a3]\n"
			"mov x4, %[a4]\n"
			"smc #0\n"
			"str x0, [%[re0]]\n"
			"str x1, [%[re1]]\n"
			"str x2, [%[re2]]\n"
			"str x3, [%[re3]]\n" :
			[fid] "+r"(in_params.x0), [a1] "+r"(in_params.x1),
			[a2] "+r"(in_params.x2), [a3] "+r"(in_params.x3),
			[a4] "+r"(in_params.x4) :
			[re0] "r"(&out_params->ret),
			[re1] "r"(&out_params->exit_reason),
			[re2] "r"(&out_params->ta),
			[re3] "r"(&out_params->target) :
			"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
			"x8", "x9", "x10", "x11", "x12", "x13",
			"x14", "x15", "x16", "x17");
	} while (0);

	isb();
	wmb();
}

#ifdef CONFIG_HISI_SECS_CTRL

static int power_on_cc(void)
{
	return hisi_secs_power_on();
}

static int power_down_cc(void)
{
	return hisi_secs_power_down();
}

#else

static int power_on_cc(void)
{
	return 0;
}

static int power_down_cc(void)
{
	return 0;
}

#endif

static void set_preempted_counter(int *n_preempted, int *n_idled,
	struct pending_entry *pe)
{
	*n_idled = 0;
	(*n_preempted)++;

	if (*n_preempted > PREEMPT_COUNT) {
		tlogi("%s: retry 10K times on CPU%d\n", __func__, smp_processor_id());
		*n_preempted = 0;
	}
#ifndef CONFIG_PREEMPT
	/* yield cpu to avoid soft lockup */
	cond_resched();
#endif
	if (match_ta_affinity(pe))
		tloge("set shadow pid %d affinity after preempted\n",
			pe->task->pid);
}

static int proc_shadow_thread_normal_exit(struct pending_entry *pe,
	int *n_preempted, int *n_idled, int *ret_val)
{
	long long timeout;
	int rc;

	if (power_down_cc()) {
		tloge("power down cc failed\n");
		*ret_val = -1;
		return CLEAN_WITHOUT_PM;
	}
	*n_preempted = 0;

	timeout = HZ * (long)(HZ_COUNT + ((uint8_t)current->pid & LOW_BYTE));
	rc = wait_event_freezable_timeout(pe->wq,
		atomic_read(&pe->run), (long)timeout);
	if (!rc)
		(*n_idled)++;
	if (atomic_read(&pe->run) == SHADOW_EXIT_RUN) {
		tlogd("shadow thread work quit, be killed\n");
		return CLEAN_WITHOUT_PM;
	} else {
		atomic_set(&pe->run, 0);
		return RETRY_WITH_PM;
	}

	return 0;
}

static bool check_shadow_crash(uint64_t crash_reason, int *ret_val)
{
	if (crash_reason != TSP_CRASH)
		return false;

	tloge("TEEOS shadow has crashed!\n");
	if (power_down_cc())
		tloge("power down cc failed\n");

	g_sys_crash = 1;
	cmd_monitor_ta_crash(TYPE_CRASH_TEE);
	report_log_system_error();
	*ret_val = -1;
	return true;
}

static int shadow_thread_fn(void *arg)
{
	int n_preempted = 0;
	int ret = 0;
	struct pending_entry *pe = NULL;
	struct smc_out_params params = { 0, SMC_EXIT_MAX, 0, 0 };
	int n_idled = 0;

	set_freezable();
	pe = init_pending_entry();
	if (!pe) {
		tloge("init pending entry failed\n");
		kfree(arg);
		return -ENOMEM;
	}
	isb();
	wmb();

retry:
	if (power_on_cc()) {
		tloge("power on cc failed\n");
		ret = -EINVAL;
		goto clean_wo_pm;
	}

retry_wo_pm:
	shadow_wo_pm(arg, &params, &n_idled);
	if (check_shadow_crash(params.ret, &ret))
		goto clean_wo_pm;

	if (params.exit_reason == SMC_EXIT_PREEMPTED) {
		set_preempted_counter(&n_preempted, &n_idled, pe);
		goto retry_wo_pm;
	} else if (params.exit_reason == SMC_EXIT_NORMAL) {
		ret = proc_shadow_thread_normal_exit(pe, &n_preempted,
			&n_idled, &ret);
		if (ret == CLEAN_WITHOUT_PM) {
			goto clean_wo_pm;
		} else if (ret == RETRY_WITH_PM) {
			if (match_ta_affinity(pe))
				tlogi("set shadow pid %d\n", pe->task->pid);
			goto retry;
		}
	} else if (params.exit_reason == SMC_EXIT_SHADOW) {
		tlogd("probe shadow thread non exit, just quit\n");
	} else {
		tlogd("exit on unknown code %ld\n", (long)params.exit_reason);
	}

	if (power_down_cc()) {
		tloge("power down cc failed\n");
		ret = -1;
	}
clean_wo_pm:
	kfree(arg);
	release_pending_entry(pe);
	return ret;
}

static void shadow_work_func(struct kthread_work *work)
{
	struct task_struct *shadow_thread = NULL;
	struct shadow_work *s_work =
		container_of(work, struct shadow_work, kthwork);
	uint64_t *target_arg = kzalloc(sizeof(uint64_t), GFP_KERNEL);

	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)target_arg)) {
		tloge("%s: kmalloc failed\n", __func__);
		return;
	}

	*target_arg = s_work->target;
	shadow_thread = kthread_create(shadow_thread_fn,
		(void *)(uintptr_t)target_arg, "shadow th/%lu",
		g_shadow_thread_id++);
	if (IS_ERR_OR_NULL(shadow_thread)) {
		kfree(target_arg);
		tloge("couldn't create shadow_thread %ld\n",
			PTR_ERR(shadow_thread));
		return;
	}
	tlogd("%s: create shadow thread %lu for target %llx\n",
		__func__, g_shadow_thread_id, *target_arg);
	wake_up_process(shadow_thread);
}

static int proc_smc_wakeup_ca(pid_t ca, int which)
{
	if (ca <= 0) {
		tlogw("wakeup for ca <= 0\n");
	} else {
		struct pending_entry *pe = find_pending_entry(ca);

		if (!pe) {
			tlogd("invalid ca pid=%d for pending entry\n",
				(int)ca);
			return -1;
		}
		atomic_set(&pe->run, which);
		wake_up(&pe->wq);
		tlogd("wakeup pending thread %ld\n", (long)ca);
		put_pending_entry(pe);
	}
	return 0;
}

void wakeup_pe(struct pending_entry *pe)
{
	if (!pe)
		return;

	atomic_set(&pe->run, 1);
	wake_up(&pe->wq);
}

int smc_wakeup_broadcast(void)
{
	foreach_pending_entry(wakeup_pe);
	return 0;
}

int smc_wakeup_ca(pid_t ca)
{
	return proc_smc_wakeup_ca(ca, 1);
}

int smc_shadow_exit(pid_t ca)
{
	return proc_smc_wakeup_ca(ca, SHADOW_EXIT_RUN);
}

void fiq_shadow_work_func(uint64_t target)
{
	struct smc_cmd_ret secret = { SMC_EXIT_MAX, 0, target };

#ifdef CONFIG_HISI_SECS_CTRL
	if (get_secs_suspend_status() == SECS_SUSPEND_STATUS)
		tloge("WARNING irq during suspend! No = %lld\n", target);
#endif
	if (power_on_cc()) {
		tloge("power on cc failed\n");
		return;
	}
	smp_smc_send(TSP_REQUEST, SMC_OPS_START_FIQSHD, current->pid,
		&secret, false);

	if (power_down_cc())
		tloge("power down cc failed\n");

	return;
}

int smc_queue_shadow_worker(uint64_t target)
{
	struct shadow_work work = {
		KTHREAD_WORK_INIT(work.kthwork, shadow_work_func),
		.target = target,
	};
	if (!kthread_queue_work(&g_ipi_helper_worker, &work.kthwork)) {
		tloge("ipi helper work fail queue, was already pending\n");
		return -1;
	}

	kthread_flush_work(&work.kthwork);
	return 0;
}

#ifdef CONFIG_DRM_ADAPT
#define DRM_USR_PRIOR (-5)
static void set_drm_strategy(void)
{
	if (!g_drm_mask_flag) {
		cpumask_clear(&g_drm_cpu_mask);
		cpumask_set_cpu(CPU_FOUR, &g_drm_cpu_mask);
		cpumask_set_cpu(CPU_FIVE, &g_drm_cpu_mask);
		cpumask_set_cpu(CPU_SIX, &g_drm_cpu_mask);
		cpumask_set_cpu(CPU_SEVEN, &g_drm_cpu_mask);
		g_drm_mask_flag = 1;
	}

	if (current->group_leader &&
		strstr(current->group_leader->comm, "drm@1.")) {
		set_cpus_allowed_ptr(current, &g_drm_cpu_mask);
		set_user_nice(current, DRM_USR_PRIOR);
	}
}
#endif

static int smc_ops_normal(enum cmd_reuse *cmd_usage, int *cmd_index,
	int *last_index, const struct tc_ns_smc_cmd *cmd, u64 ops)
{
	if (ops != SMC_OPS_NORMAL)
		return 0;

	if (*cmd_usage == RESEND) {
		if (reuse_smc_in_entry(*cmd_index)) {
			tloge("reuse smc entry failed\n");
			release_smc_entry(*cmd_index);
			return -ENOMEM;
		}
	} else {
		*cmd_index = occupy_free_smc_in_entry(cmd);
		if (*cmd_index == -1) {
			tloge("there's no more smc entry\n");
			return -ENOMEM;
		}
	}

	if (*cmd_usage != CLEAR) {
		*cmd_index = *last_index;
		*cmd_usage = CLEAR;
	} else {
		*last_index = *cmd_index;
	}

	tlogd("submit new cmd: cmd.ca=%u cmd-id=%x ev-nr=%u "
		"cmd-index=%u last-index=%d\n",
		cmd->ca_pid, cmd->cmd_id,
		g_cmd_data->in[*cmd_index].event_nr, *cmd_index,
		*last_index);
	return 0;
}

static int smp_smc_send_cmd_done(int cmd_index, struct tc_ns_smc_cmd *cmd,
	struct tc_ns_smc_cmd *in)
{
	cmd_result_check(cmd);
	switch (cmd->ret_val) {
	case TEEC_PENDING2: {
		unsigned int agent_id = cmd->agent_id;
		/* If the agent does not exist post
		 * the answer right back to the TEE
		 */
		if (agent_process_work(cmd, agent_id))
			tloge("agent process work failed\n");
		return PENDING2_RETRY;
	}
	case TEE_ERROR_TAGET_DEAD:
	case TEEC_PENDING:
	/* just copy out, and let out to proceed */
	default:
		if (memcpy_s(in, sizeof(*in), cmd, sizeof(*cmd))) {
			tloge("memcpy failed,%s line:%d", __func__, __LINE__);
			cmd->ret_val = -1;
		}

		break;
	}

	return 0;
}

static void print_crash_msg(union crash_inf *crash_info)
{
	static const char *tee_critical_app[] = {
		"gtask",
		"teesmcmgr",
		"hmsysmgr",
		"hmfilemgr",
		"platdrv",
		"kernel",
		"vltmm_service"
	};
	int app_num = sizeof(tee_critical_app) / sizeof(tee_critical_app[0]);
	const char *crash_app_name = "NULL";
	uint16_t off = crash_info->crash_msg.off;
	int app_index = crash_info->crash_msg.app & LOW_BYTE;
	int halt_reason = crash_info->crash_msg.halt_reason;

	crash_info->crash_msg.off = 0;

	if (app_index >= 0 && app_index < app_num)
		crash_app_name = tee_critical_app[app_index];
	else
		tloge("index error: %x\n", crash_info->crash_msg.app);

	if (app_index == (app_num - 1)) {
		tloge("====crash app:%s user sym:%s kernel crash off/size: "
			"<0x%x/0x%x>\n", crash_app_name,
			crash_info->crash_msg.sym_name,
			off, crash_info->crash_msg.size);
		tloge("====crash halt reason: 0x%x far:0x%x fault:0x%x "
			"elr:0x%x (ret_ip: 0x%llx)\n",
			halt_reason, crash_info->crash_msg.far,
			crash_info->crash_msg.fault, crash_info->crash_msg.elr,
			crash_info->crash_reg[2]);
	} else {
		char syms[SYM_NAME_LEN_MAX] = {0};

		if (memcpy_s(syms, SYM_NAME_LEN_MAX,
			crash_info->crash_msg.sym_name, SYM_NAME_LEN_1))
			tloge("memcpy sym name failed!\n");

		if (memcpy_s(syms + SYM_NAME_LEN_1,
			SYM_NAME_LEN_MAX - SYM_NAME_LEN_1,
			crash_info->crash_msg.sym_name_append, SYM_NAME_LEN_2))
			tloge("memcpy sym_name_append failed!\n");
		tloge("====crash app:%s user_sym:%s + <0x%x/0x%x>\n",
		      crash_app_name, syms, off, crash_info->crash_msg.size);
		tloge("====crash far:0x%x fault:%x\n",
		      crash_info->crash_msg.far, crash_info->crash_msg.fault);
	}
}

static int smp_smc_send_process(struct tc_ns_smc_cmd *cmd, u64 ops,
	struct smc_cmd_ret *cmd_ret, int cmd_index)
{
	int ret;

	tlogd("smc send start cmd_id = %u, ca = %u\n",
		cmd->cmd_id, cmd->ca_pid);

	if (power_on_cc()) {
		tloge("power on cc failed\n");
		cmd->ret_val = -1;
		return -1;
	}

	ret = smp_smc_send(TSP_REQUEST, ops, current->pid, cmd_ret,
		ops != SMC_OPS_ABORT_TASK);

	if (power_down_cc()) {
		tloge("power down cc failed\n");
		cmd->ret_val = -1;
		return -1;
	}

	tlogd("smc send ret = %x, cmd ret.exit=%ld, cmd index=%d\n",
		ret, (long)cmd_ret->exit, cmd_index);
	isb();
	wmb();
	if (ret == (int)TSP_CRASH) {
		union crash_inf crash_info;
		crash_info.crash_reg[0] = cmd_ret->exit;
		crash_info.crash_reg[1] = cmd_ret->ta;
		crash_info.crash_reg[2] = cmd_ret->target;

		tloge("TEEOS has crashed!\n");
		print_crash_msg(&crash_info);

		g_sys_crash = 1;
		cmd_monitor_ta_crash(TYPE_CRASH_TEE);

		report_log_system_error();

		cmd->ret_val = -1;
		return -1;
	}

	return 0;
}

static int init_for_smc_send(struct tc_ns_smc_cmd *in,
	struct pending_entry **pe, struct tc_ns_smc_cmd *cmd,
	bool reuse)
{
#ifdef CONFIG_DRM_ADAPT
	set_drm_strategy();
#endif
	*pe = init_pending_entry();
	if (!(*pe)) {
		tloge("init pending entry failed\n");
		return -ENOMEM;
	}

	in->ca_pid = current->pid;
	if (reuse)
		return 0;

	if (memcpy_s(cmd, sizeof(*cmd), in, sizeof(*in))) {
		tloge("memcpy in cmd failed\n");
		release_pending_entry(*pe);
		return -EFAULT;
	}

	return 0;
}

static bool is_ca_killed(int cmd_index)
{
	/* if CA has not been killed */
	if (sigkill_pending(current)) {
		/* signal pending, send abort cmd */
		tloge("wait event timeout and find pending signal\n");
		return true;
	}
	return false;
}

static void clean_smc_resrc(enum cmd_reuse cmd_usage,
	const struct tc_ns_smc_cmd *cmd,
	struct pending_entry *pe, int cmd_index)
{
	if (cmd_usage != CLEAR && cmd->ret_val != TEEC_PENDING)
		release_smc_entry(cmd_index);

	release_pending_entry(pe);
}

static int set_abort_cmd(int index)
{
	acquire_smc_buf_lock(&g_cmd_data->smc_lock);
	if (!test_bit(index, g_cmd_data->doing_bitmap)) {
		release_smc_buf_lock(&g_cmd_data->smc_lock);
		tloge("can't abort an unprocess cmd\n");
		return -1;
	}

	if (g_cmd_data->in[index].global_cmd) {
		release_smc_buf_lock(&g_cmd_data->smc_lock);
		tloge("can't abort a global cmd\n");
		return -1;
	}

	g_cmd_data->in[index].cmd_id = GLOBAL_CMD_ID_KILL_TASK;
	g_cmd_data->in[index].global_cmd = true;
	/* these phy addrs are not necessary, clear them to avoid gtask check err */
	g_cmd_data->in[index].operation_phys = 0;
	g_cmd_data->in[index].operation_h_phys = 0;
	g_cmd_data->in[index].login_data_phy = 0;
	g_cmd_data->in[index].login_data_h_addr = 0;
	g_cmd_data->in[index].token_phys = 0;
	g_cmd_data->in[index].token_h_phys = 0;
	g_cmd_data->in[index].params_phys = 0;
	g_cmd_data->in[index].params_h_phys = 0;

	clear_bit(index, g_cmd_data->doing_bitmap);
	release_smc_buf_lock(&g_cmd_data->smc_lock);
	tloge("set abort cmd success\n");

	return 0;
}

static enum smc_ops_exit process_abort_cmd(int index, struct pending_entry *pe)
{
	if (!set_abort_cmd(index))
		return SMC_OPS_ABORT_TASK;

	return SMC_OPS_SCHEDTO;
}

#define TO_STEP_SIZE 5
#define INVALID_STEP_SIZE 0xFFFFFFFFU

struct timeout_step_t {
	unsigned long steps[TO_STEP_SIZE];
	uint32_t size;
	uint32_t cur;
	bool timeout_reset;
};

static void init_timeout_step(uint32_t timeout, struct timeout_step_t *step)
{
	uint32_t i = 0;

	if (timeout == 0) {
		step->steps[0] = RESLEEP_TIMEOUT * HZ;
		step->size = 1;
	} else {
		uint32_t timeout_in_jiffies;

		if (timeout > RESLEEP_TIMEOUT * MSEC_PER_SEC)
			timeout = RESLEEP_TIMEOUT * MSEC_PER_SEC;
		timeout_in_jiffies = msecs_to_jiffies(timeout);
		/*
		 * [timeout_in_jiffies-1, timeout_in_jiffies+2] jiffies
		 * As REE and TEE tick have deviation, to make sure last REE timeout
		 * is after TEE timeout, we set a timeout step from
		 * 'timeout_in_jiffies -1' to 'timeout_in_jiffies + 2'
		 */
		if (timeout_in_jiffies > 1) {
			step->steps[i++] = timeout_in_jiffies - 1;
			step->steps[i++] = 1;
		} else {
			step->steps[i++] = timeout_in_jiffies;
		}
		step->steps[i++] = 1;
		step->steps[i++] = 1;

		if (RESLEEP_TIMEOUT * HZ > (timeout_in_jiffies + 2))
			step->steps[i++] = RESLEEP_TIMEOUT * HZ - 2 - timeout_in_jiffies;
		step->size = i;
	}
	step->cur = 0;
}

enum pending_t {
	PD_WAKEUP,
	PD_TIMEOUT,
	PD_DONE,
	PD_RETRY,
};

enum smc_status_t {
	ST_DONE,
	ST_RETRY,
};

static enum pending_t proc_ta_pending(struct pending_entry *pe,
	struct timeout_step_t *step, uint64_t pending_args, uint32_t cmd_index,
	u64 *ops)
{
	bool kernel_call = false;
	bool woke_up = false;

	/*
	 * if ->mm is NULL, it's a kernel thread and a kthread will never
	 * receive a signal.
	 */
	if (!current->mm) {
		kernel_call = true;
		if (wait_event_interruptible(pe->wq, atomic_read(&pe->run)))
			tloge("kernel CA is interrupted\n");
	} else {
		uint32_t timeout = (uint32_t)pending_args;
		bool timer_no_irq = (pending_args >> 32) == 0 ? false : true;
		uint32_t cur_timeout;

		tlogd("%s timeout %u\n", __func__, timeout);
		if (step->cur == INVALID_STEP_SIZE)
			init_timeout_step(timeout, step);
resleep:
		cur_timeout = jiffies_to_msecs(step->steps[step->cur]);

		tlogd("%s start wait %lu\n", __func__, step->steps[step->cur]);
		if (!wait_event_timeout(pe->wq, atomic_read(&pe->run),
				step->steps[step->cur])) {
			tlogd("%s after wait %lu, %u/%u\n",
				__func__, step->steps[step->cur], step->cur, step->size);
			if (step->cur < (step->size - 1)) {
				step->cur++;
				/*
				 * As there may no timer irq in TEE, we need a chance to
				 * run timer's irq handler initiatively by SMC_OPS_SCHEDTO.
				 */
				if (timer_no_irq) {
					*ops = SMC_OPS_SCHEDTO;
					return PD_TIMEOUT;
				} else {
					goto resleep;
				}
			}

			if (is_ca_killed(cmd_index)) {
				*ops = (u64)process_abort_cmd(cmd_index, pe);
				return PD_WAKEUP;
			}
		} else {
			woke_up = true;
			tlogd("%s woke up\n", __func__);
		}
	}

	atomic_set(&pe->run, 0);
	if (!is_cmd_working_done(cmd_index)) {
		*ops = SMC_OPS_SCHEDTO;
		return PD_WAKEUP;
	} else if (!kernel_call && !woke_up) {
		tloge("cmd done, may miss a spi!\n");
		show_cmd_bitmap();
	}
	tlogd("cmd is done\n");

	return PD_DONE;
}

static void set_timeout_step(struct timeout_step_t *timeout_step)
{
	if (!timeout_step->timeout_reset)
		return;

	timeout_step->cur = INVALID_STEP_SIZE;
	timeout_step->timeout_reset = false;
}

static enum smc_status_t proc_normal_exit(struct pending_entry *pe, u64 *ops,
	struct timeout_step_t *timeout_step, struct smc_cmd_ret *cmd_ret,
	int cmd_index)
{
	enum pending_t pd_ret;

	/* notify and set affinity came first, goto retry directly */
	if (match_ta_affinity(pe)) {
		*ops = SMC_OPS_SCHEDTO;
		return ST_RETRY;
	}

	pd_ret = proc_ta_pending(pe, timeout_step,
		cmd_ret->ta, cmd_index, ops);
	if (pd_ret == PD_DONE)
		return ST_DONE;

	if (pd_ret == PD_WAKEUP)
		timeout_step->timeout_reset = true;
	return ST_RETRY;
}

static enum smc_status_t handle_cmd_working_done(
	struct tc_ns_smc_cmd *cmd, u64 *ops, struct tc_ns_smc_cmd *in,
	enum cmd_reuse *cmd_usage, int cmd_index)
{
	if (copy_smc_out_entry(cmd_index, cmd, cmd_usage)) {
		cmd->ret_val = TEEC_ERROR_GENERIC;
		return ST_DONE;
	}

	if (smp_smc_send_cmd_done(cmd_index, cmd, in)) {
		*ops = SMC_OPS_NORMAL; /* cmd will be reused */
		return ST_RETRY;
	}

	return ST_DONE;
}

static int smp_smc_send_func(struct tc_ns_smc_cmd *in, bool reuse)
{
	int cmd_index = 0;
	int last_index = 0;
	struct smc_cmd_ret cmd_ret = {0};
	struct tc_ns_smc_cmd cmd = { {0}, 0 };
	struct pending_entry *pe = NULL;
	u64 ops;
	enum cmd_reuse cmd_usage = CLEAR;
	struct timeout_step_t timeout_step =
		{ {0, 0, 0, 0}, TO_STEP_SIZE, -1, false };

	if (init_for_smc_send(in, &pe, &cmd, reuse))
		return TEEC_ERROR_GENERIC;

	if (reuse) {
		last_index = in->event_nr;
		cmd_index = in->event_nr;
		cmd_usage = RESEND;
	}
	ops = SMC_OPS_NORMAL;

retry:
	set_timeout_step(&timeout_step);

	if (smc_ops_normal(&cmd_usage, &cmd_index, &last_index, &cmd, ops)) {
		release_pending_entry(pe);
		return TEEC_ERROR_GENERIC;
	}

	if (smp_smc_send_process(&cmd, ops, &cmd_ret, cmd_index) == -1)
		goto clean;

	if (!is_cmd_working_done(cmd_index)) {
		if (cmd_ret.exit == SMC_EXIT_NORMAL) {
			if (proc_normal_exit(pe, &ops, &timeout_step, &cmd_ret,
				cmd_index) == ST_RETRY)
				goto retry;
		} else if (cmd_ret.exit == SMC_EXIT_ABORT) {
			ops = (u64)process_abort_cmd(cmd_index, pe);
			goto retry;
		} else {
			tloge("invalid cmd work state\n");
			cmd.ret_val = TEEC_ERROR_GENERIC;
			goto clean;
		}
	}

	if (handle_cmd_working_done(&cmd, &ops, in, &cmd_usage,
		cmd_index) == ST_RETRY)
		goto retry;
clean:
	clean_smc_resrc(cmd_usage, &cmd, pe, cmd_index);
	return cmd.ret_val;
}

static int smc_svc_thread_fn(void *arg)
{
	while (!kthread_should_stop()) {
		struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
		int ret;

		smc_cmd.global_cmd = true;
		smc_cmd.cmd_id = GLOBAL_CMD_ID_SET_SERVE_CMD;
		ret = smp_smc_send_func(&smc_cmd, false);
		tlogd("smc svc return 0x%x\n", ret);
	}
	tloge("smc svc thread stop\n");
	return 0;
}

bool is_tee_hungtask(struct task_struct *task)
{
	uint32_t i;
	uint32_t hungtask_num = sizeof(g_hungtask_monitor_list) /
		sizeof(g_hungtask_monitor_list[0]);

	if (!task)
		return false;
	for (i = 0; i < hungtask_num; i++) {
		if (!strcmp(task->comm, g_hungtask_monitor_list[i])) {
			tloge("tee hungtask detected:the hungtask is %s\n",
				task->comm);
			return true;
		}
	}

	return false;
}

void wakeup_tc_siq(void)
{
	atomic_set(&g_siq_th_run, 1);
	wake_up_interruptible(&siq_th_wait);
}

/*
 * This function first power on crypto cell, then send smc cmd to trustedcore.
 * After finished, power off crypto cell.
 */
static int proc_tc_ns_smc(struct tc_ns_smc_cmd *cmd, bool reuse)
{
	int ret;
	struct cmd_monitor *item = NULL;

	if (g_sys_crash) {
		tloge("ERROR: sys crash happened!!!\n");
		return TEEC_ERROR_GENERIC;
	}
	if (!cmd) {
		tloge("invalid cmd\n");
		return TEEC_ERROR_GENERIC;
	}
	tlogd(KERN_INFO "***smc call start on cpu %d ***\n",
		raw_smp_processor_id());

	item = cmd_monitor_log(cmd);
	ret = smp_smc_send_func(cmd, reuse);
	cmd_monitor_logend(item);

	return ret;
}

int tc_ns_smc(struct tc_ns_smc_cmd *cmd)
{
	return proc_tc_ns_smc(cmd, false);
}

int tc_ns_smc_with_no_nr(struct tc_ns_smc_cmd *cmd)
{
	return proc_tc_ns_smc(cmd, true);
}

static void smc_work_no_wait(uint32_t type)
{
	raw_smc_send(TSP_REQUEST, g_cmd_phys, type, true);
}

static void smc_work_set_cmd_buffer(struct work_struct *work)
{
	(void)work;
	smc_work_no_wait(TC_NS_CMD_TYPE_SECURE_CONFIG);
}

static void smc_set_cmd_buffer(void)
{
	struct work_struct work;

	INIT_WORK_ONSTACK(&work, smc_work_set_cmd_buffer);
	/* Run work on CPU 0 */
	schedule_work_on(0, &work);
	flush_work(&work);
	tlogd("smc set cmd buffer done\n");
}

static int alloc_cmd_buffer(void)
{
	g_cmd_data = (struct tc_ns_smc_queue *)(uintptr_t)__get_free_page(
		GFP_KERNEL | __GFP_ZERO);
	if (!g_cmd_data)
		return -ENOMEM;

	g_cmd_phys = virt_to_phys(g_cmd_data);

	return 0;
}

static int init_smc_related_rsrc(const struct device *class_dev)
{
	struct cpumask new_mask;
	int ret;

	/*
	 * TEE Dump will disable IRQ/FIQ for about 500 ms, it's not
	 * a good choice to ask CPU0/CPU1 to do the dump.
	 * So, bind this kernel thread to other CPUs rather than CPU0/CPU1.
	 */
	cpumask_setall(&new_mask);
	cpumask_clear_cpu(CPU_ZERO, &new_mask);
	cpumask_clear_cpu(CPU_ONE, &new_mask);
	koadpt_kthread_bind_mask(g_siq_thread, &new_mask);
	g_ipi_helper_thread = kthread_create(kthread_worker_fn,
		&g_ipi_helper_worker, "ipihelper");
	if (IS_ERR_OR_NULL(g_ipi_helper_thread)) {
		dev_err(class_dev, "couldn't create ipi helper threads %ld\n",
			PTR_ERR(g_ipi_helper_thread));
		ret = (int)PTR_ERR(g_ipi_helper_thread);
		return ret;
	}

	wake_up_process(g_ipi_helper_thread);
	wake_up_process(g_siq_thread);
	init_cmd_monitor();
	INIT_LIST_HEAD(&g_pending_head);
	spin_lock_init(&g_pend_lock);

	return 0;
}

int smc_context_init(const struct device *class_dev)
{
	int ret;

	if (!class_dev || IS_ERR_OR_NULL(class_dev))
		return -ENOMEM;

	ret = alloc_cmd_buffer();
	if (ret)
		return ret;

	/* Send the allocated buffer to TrustedCore for init */
	smc_set_cmd_buffer();

	if (get_session_root_key()) {
		ret = -EFAULT;
		goto free_mem;
	}

	g_siq_thread = kthread_create(siq_thread_fn, NULL, "siqthread/%d", 0);
	if (unlikely(IS_ERR_OR_NULL(g_siq_thread))) {
		dev_err(class_dev, "couldn't create siqthread %ld\n",
			PTR_ERR(g_siq_thread));
		ret = (int)PTR_ERR(g_siq_thread);
		goto free_mem;
	}

	ret = init_smc_related_rsrc(class_dev);
	if (ret)
		goto free_siq_worker;

	return 0;

free_siq_worker:
	kthread_stop(g_siq_thread);
	g_siq_thread = NULL;
free_mem:
	free_page((unsigned long)(uintptr_t)g_cmd_data);
	g_cmd_data = NULL;
	free_root_key();
	return ret;
}

int init_smc_svc_thread(void)
{
	g_smc_svc_thread = kthread_create(smc_svc_thread_fn, NULL,
		"smc_svc_thread");
	if (unlikely(IS_ERR_OR_NULL(g_smc_svc_thread))) {
		tloge("couldn't create smc_svc_thread %ld\n",
			PTR_ERR(g_smc_svc_thread));
		return PTR_ERR(g_smc_svc_thread);
	}
	wake_up_process(g_smc_svc_thread);
	return 0;
}

int teeos_log_exception_archive(unsigned int eventid,
	const char *exceptioninfo)
{
#ifdef CONFIG_TEE_LOG_EXCEPTION
	int ret;
	struct imonitor_eventobj *teeos_obj = NULL;

	teeos_obj = imonitor_create_eventobj(eventid);
	if (exceptioninfo) {
		ret = imonitor_set_param(teeos_obj, 0,
			(long)(uintptr_t)exceptioninfo);
	} else {
		ret = imonitor_set_param(teeos_obj, 0,
			(long)(uintptr_t)"teeos something crash");
	}
	if (ret) {
		tloge("imonitor_set_param failed\n");
		imonitor_destroy_eventobj(teeos_obj);
		return ret;
	}
	ret = imonitor_add_dynamic_path(teeos_obj,
		"/data/vendor/log/hisi_logs/tee");
	if (ret) {
		tloge("add path failed\n");
		imonitor_destroy_eventobj(teeos_obj);
		return ret;
	}
	ret = imonitor_add_dynamic_path(teeos_obj, "/data/log/tee");
	if (ret) {
		tloge("add path failed\n");
		imonitor_destroy_eventobj(teeos_obj);
		return ret;
	}
	ret = imonitor_send_event(teeos_obj);
	imonitor_destroy_eventobj(teeos_obj);
	return ret;
#else
	return 0;
#endif
}

void smc_free_data(void)
{
	free_page((unsigned long)(uintptr_t)g_cmd_data);
	if (!IS_ERR_OR_NULL(g_smc_svc_thread)) {
		kthread_stop(g_smc_svc_thread);
		g_smc_svc_thread = NULL;
	}

	free_root_key();
}
