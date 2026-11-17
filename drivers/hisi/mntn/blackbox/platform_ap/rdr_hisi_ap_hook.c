/*
 *
 * AP side track hook function code
 *
 * Copyright (c) 2001-2019 Huawei Technologies Co., Ltd.
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
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/atomic.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/ctype.h>
#include <linux/smp.h>
#include <linux/preempt.h>
#include <linux/delay.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/cpu_pm.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/time64.h>
#include <linux/percpu.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <asm/compiler.h>
#include "../rdr_print.h"
#include <pr_log.h>
#include <securec.h>
#include "../rdr_inner.h"

#define PR_LOG_TAG BLACKBOX_TAG
#define HISI_AP_HOOK_CPU_NUMBERS 8
#define PERCPU_TOTAL_RATIO 16


static unsigned char *g_hook_buffer_addr[HK_MAX] = { 0 };
static struct percpu_buffer_info *g_hook_percpu_buffer[HK_PERCPU_TAG] = { NULL };
#ifdef CONFIG_PM
static struct percpu_buffer_info g_hook_percpu_buffer_bak[HK_PERCPU_TAG];
#endif

static const char *g_hook_trace_pattern[HK_MAX] = {
	"irq_switch::ktime,slice,vec,dir", /* IRQ */
	"task_switch::ktime,stack,pid,comm", /* TASK */
	"cpuidle::ktime,dir", /* CPUIDLE */
	"worker::ktime,action,dir,cpu,resv,pid", /* WORKER */
	"mem_alloc::ktime,pid/vec,comm/irq_name,caller,operation,vaddr,paddr,size", /* MEM ALLOCATOR */
	"ion_alloc::ktime,pid/vec,comm/irq_name,operation,vaddr,paddr,size", /* ION ALLOCATOR */
	"time::ktime,action,dir", /* TIME */
	"net_tx::ktime,action,dir", /* NET_TX*/
	"net_rx::ktime,action,dir", /* NET_RX*/
	"block::ktime,action,dir", /* BLOCK_SOFTIRQ*/
	"sched::ktime,action,dir",/* SCHED_SOFTIRQ*/
	"rcu::ktime,action,dir",/* RCU_SOFTIRQ*/
	"cpu_onoff::ktime,cpu,on", /* CPU_ONOFF */
	"syscall::ktime,syscall,cpu,dir", /* SYSCALL */
	"hung_task::ktime,timeout,pid,comm", /* HUNG_TASK */
	"tasklet::ktime,action,cpu,dir", /* TASKLET */
	"diaginfo::Time,Module,Error_ID,Log_Level,Fault_Type,Data", /* diaginfo */
	"timeout::ktime,action,dir" /*TIMEOUT_SOFTIRQ*/
};

/* default opened hook id for each case */
static enum hook_type g_hisi_defopen_hook_id[] = { HK_CPUIDLE, HK_CPU_ONOFF };

static atomic_t g_hisi_ap_hook_on[HK_MAX] = { ATOMIC_INIT(0) };
static unsigned int g_softirq_timeout_period[NR_SOFTIRQS] =
	{0xFF,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static DEFINE_PER_CPU(u64[NR_SOFTIRQS], g_softirq_in_time);
static u32 g_hisi_last_irqs[NR_CPUS] = { 0 };
struct task_struct *g_last_task_ptr[NR_CPUS] = { NULL };

struct mutex hook_switch_mutex;
arch_timer_func_ptr g_arch_timer_func_ptr;

struct task_struct **get_last_task_ptr(void)
{
	return (struct task_struct **)g_last_task_ptr;
}

int register_arch_timer_func_ptr(arch_timer_func_ptr func)
{
	if (IS_ERR_OR_NULL(func)) {
		BB_PRINT_ERR("[%s], arch_timer_func_ptr [0x%pK] is invalid!\n", __func__, func);
		return -EINVAL;
	}
	g_arch_timer_func_ptr = func;
	return 0;
}

static int single_buffer_init(unsigned char **addr, unsigned int size, enum hook_type hk,
				unsigned int fieldcnt)
{
	unsigned int min_size = sizeof(struct hisiap_ringbuffer_s) + fieldcnt;

	if ((!addr) || !(*addr)) {
		BB_PRINT_ERR("[%s], argument addr is invalid\n", __func__);
		return -EINVAL;
	}
	if (hk >= HK_MAX) {
		BB_PRINT_ERR("[%s], argument hook_type [%d] is invalid!\n", __func__, hk);
		return -EINVAL;
	}
	if (size < min_size) {
		g_hook_buffer_addr[hk] = 0;
		*addr = 0;
		BB_PRINT_ERR("[%s], argument size [0x%x] is invalid!\n",
		       __func__, size);
		return 0;
	}
	g_hook_buffer_addr[hk] = (*addr);

	BB_PRINT_DBG(
	       "hisiap_ringbuffer_init: g_hook_trace_pattern[%d]: %s\n", hk,
	       g_hook_trace_pattern[hk]);
	return hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)(*addr), size,
				      fieldcnt, g_hook_trace_pattern[hk]);
}

int percpu_buffer_init(struct percpu_buffer_info *buffer_info, u32 ratio[][HISI_AP_HOOK_CPU_NUMBERS], /* HERE:8 */
				u32 cpu_num, u32 fieldcnt, const char *keys, u32 gap)
{
	unsigned int i;
	int ret;
	struct percpu_buffer_info *buffer = buffer_info;

	if (!keys) {
		BB_PRINT_ERR("[%s], argument keys is NULL\n", __func__);
		return -EINVAL;
	}

	if (!buffer) {
		BB_PRINT_ERR("[%s], buffer info is null\n", __func__);
		return -EINVAL;
	}

	if (!buffer->buffer_addr) {
		BB_PRINT_ERR("[%s], buffer_addr is NULL\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < cpu_num; i++) {
		BB_PRINT_DBG("[%s], ratio[%u][%u] = [%u]\n", __func__,
		       (cpu_num - 1), i, ratio[cpu_num - 1][i]);
		buffer->percpu_length[i] =
			(buffer->buffer_size - cpu_num * gap) / PERCPU_TOTAL_RATIO *
			ratio[cpu_num - 1][i];

		if (i == 0) {
			buffer->percpu_addr[0] = buffer->buffer_addr;
		} else {
			buffer->percpu_addr[i] =
			    buffer->percpu_addr[i - 1] +
			    buffer->percpu_length[i - 1] + gap;
		}

		BB_PRINT_DBG(
		       "[%s], [%u]: percpu_addr [0x%pK], percpu_length [0x%x], fieldcnt [%u]\n",
		       __func__, i, buffer->percpu_addr[i],
		       buffer->percpu_length[i], fieldcnt);

		ret =
		    hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)
					   buffer->percpu_addr[i],
					   buffer->percpu_length[i], fieldcnt,
					   keys);
		if (ret) {
			BB_PRINT_ERR(
			       "[%s], cpu [%u] ringbuffer init failed!\n",
			       __func__, i);
			return ret;
		}
	}
	return 0;
}

int hook_percpu_buffer_init(struct percpu_buffer_info *buffer_info,
				unsigned char *addr, u32 size, u32 fieldcnt,
				enum hook_type hk, u32 ratio[][HISI_AP_HOOK_CPU_NUMBERS])
{
	u64 min_size;
	u32 cpu_num = num_possible_cpus();

	pr_info("[%s], num_online_cpus [%u] !\n", __func__, num_online_cpus());

	if (IS_ERR_OR_NULL(addr) || IS_ERR_OR_NULL(buffer_info)) {
		BB_PRINT_ERR(
		       "[%s], buffer_info [0x%pK] buffer_addr [0x%pK], buffer_size [0x%x]\n",
		       __func__, buffer_info, addr, size);
		return -EINVAL;
	}

	min_size = cpu_num * (sizeof(struct hisiap_ringbuffer_s) + PERCPU_TOTAL_RATIO * (u64)(unsigned int)fieldcnt);
	if (size < (u32)min_size) {
		g_hook_buffer_addr[hk] = 0;
		g_hook_percpu_buffer[hk] = buffer_info;
		g_hook_percpu_buffer[hk]->buffer_addr = 0;
		g_hook_percpu_buffer[hk]->buffer_size = 0;
		BB_PRINT_ERR(
		       "[%s], buffer_info [0x%pK] buffer_addr [0x%pK], buffer_size [0x%x]\n",
		       __func__, buffer_info, addr, size);
		return 0;
	}

	if (hk >= HK_PERCPU_TAG) {
		BB_PRINT_ERR("[%s], hook_type [%d] is invalid!\n", __func__, hk);
		return -EINVAL;
	}

	pr_info("[%s], buffer_addr [0x%pK], buffer_size [0x%x]\n",
	       __func__, addr, size);

	g_hook_buffer_addr[hk] = addr;
	g_hook_percpu_buffer[hk] = buffer_info;
	g_hook_percpu_buffer[hk]->buffer_addr = addr;
	g_hook_percpu_buffer[hk]->buffer_size = size;

	return percpu_buffer_init(buffer_info, ratio, cpu_num,
				  fieldcnt, g_hook_trace_pattern[hk], 0);
}

#ifdef CONFIG_PM
void hook_percpu_buffer_backup(void)
{
	int i;

	for (i = 0; i < HK_PERCPU_TAG; i++)
		if (g_hook_percpu_buffer[i])
			(void)memcpy_s(&g_hook_percpu_buffer_bak[i], sizeof(struct percpu_buffer_info),
					g_hook_percpu_buffer[i], sizeof(struct percpu_buffer_info));
}

void hook_percpu_buffer_restore(void)
{
	int i;

	for (i = 0; i < HK_PERCPU_TAG; i++)
		if (g_hook_percpu_buffer[i])
			(void)memcpy_s(g_hook_percpu_buffer[i], sizeof(struct percpu_buffer_info),
					&g_hook_percpu_buffer_bak[i], sizeof(struct percpu_buffer_info));
}
#endif

int irq_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int irq_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 8, 4, 2, 1, 1, 0, 0, 0 },
	{ 8, 4, 1, 1, 1, 1, 0, 0 },
	{ 6, 4, 2, 1, 1, 1, 1, 0 },
	{ 6, 4, 1, 1, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct hook_irq_info), HK_IRQ,
				       irq_record_ratio);
}

int task_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int task_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 2, 2, 2, 2, 2, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct task_info), HK_TASK,
				       task_record_ratio);
}

int cpuidle_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int cpuidle_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct cpuidle_info), HK_CPUIDLE,
				       cpuidle_record_ratio);
}

int worker_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int worker_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct worker_info), HK_WORKER,
				       worker_record_ratio);
}
int mem_alloc_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int mem_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	pr_info("[%s], memstart_addr [0x%pK] sizeof(struct mem_allocator_info)[%d]!\n",
			__func__, (void *)memstart_addr, (int)sizeof(struct mem_allocator_info));

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       (unsigned int)sizeof(struct mem_allocator_info), HK_MEM_ALLOCATOR,
				       mem_record_ratio);
}

int ion_alloc_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int ion_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	pr_info("[%s], memstart_addr [0x%pK] sizeof(struct ion_allocator_info)[%d]!\n",
			__func__, (void *)memstart_addr, (int)sizeof(struct ion_allocator_info));

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       (unsigned int)sizeof(struct ion_allocator_info), HK_ION_ALLOCATOR,
				       ion_record_ratio);
}
int time_buffer_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int time_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct time_info), HK_TIME, time_record_ratio);
}
int net_tx_softirq_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int net_tx_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct softirq_info), HK_NET_TX_SOFTIRQ, net_tx_record_ratio);
}
int net_rx_softirq_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int net_rx_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct softirq_info), HK_NET_RX_SOFTIRQ, net_rx_record_ratio);
}

int block_softirq_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int block_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct softirq_info), HK_BLOCK_SOFTIRQ, block_record_ratio);
}

int sched_softirq_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int sched_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct softirq_info), HK_SCHED_SOFTIRQ, sched_record_ratio);
}

int rcu_softirq_init(struct percpu_buffer_info *buffer_info, unsigned char *addr, unsigned int size)
{
	unsigned int rcu_record_ratio[HISI_AP_HOOK_CPU_NUMBERS][HISI_AP_HOOK_CPU_NUMBERS] = {
	{ 16, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, 0, 0, 0 },
	{ 8, 4, 4, 0, 0, 0, 0, 0 },
	{ 8, 4, 2, 2, 0, 0, 0, 0 },
	{ 4, 4, 4, 2, 2, 0, 0, 0 },
	{ 4, 4, 2, 2, 2, 2, 0, 0 },
	{ 4, 4, 2, 2, 2, 1, 1, 0 },
	{ 4, 3, 3, 2, 1, 1, 1, 1 }
	};

	return hook_percpu_buffer_init(buffer_info, addr, size,
				       sizeof(struct softirq_info), HK_RCU_SOFTIRQ, rcu_record_ratio);
}

int cpu_onoff_buffer_init(unsigned char **addr, unsigned int size)
{
	return single_buffer_init(addr, size, HK_CPU_ONOFF, sizeof(struct cpu_onoff_info));
}

int syscall_buffer_init(unsigned char **addr, unsigned int size)
{
	return single_buffer_init(addr, size, HK_SYSCALL, sizeof(struct syscall_info));
}

int hung_task_buffer_init(unsigned char **addr, unsigned int size)
{
	return single_buffer_init(addr, size, HK_HUNGTASK, sizeof(struct hung_task_info));
}

int tasklet_buffer_init(unsigned char **addr, unsigned int size)
{
	return single_buffer_init(addr, size, HK_TASKLET, sizeof(struct tasklet_info));
}

int diaginfo_buffer_init(unsigned char **addr, unsigned int size)
{
	return single_buffer_init(addr, size, HK_DIAGINFO, DIAGINFO_STRING_MAX_LEN);
}

int timeout_softirq_buffer_init(unsigned char **addr, unsigned int size)
{
	int i;
	for (i = 0; i < NR_SOFTIRQS; i++)
		per_cpu(g_softirq_in_time, smp_processor_id())[i] = 0;

	return single_buffer_init(addr, size, HK_TIMEOUT_SOFTIRQ, sizeof(struct softirq_info));
}

/*
 * Description: Interrupt track record
 * Input:       dir: 0 interrupt entry, 1 interrupt exit, new_vec: current interrupt
 */
void irq_trace_hook(unsigned int dir, unsigned int old_vec, unsigned int new_vec)
{
	/* Record time stamp, cpu_id, interrupt number, interrupt in and out direction */
	struct hook_irq_info info;
	u8 cpu;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_IRQ]))
		return;


	info.clock = hisi_getcurtime();
	if (g_arch_timer_func_ptr)
		info.jiff = (*g_arch_timer_func_ptr)();
	else
		info.jiff = jiffies_64;

	cpu = (u8)smp_processor_id();
	info.dir = (u8)dir;
	info.irq = (u32)new_vec;
	g_hisi_last_irqs[cpu] = new_vec;

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_percpu_buffer[HK_IRQ]->percpu_addr[cpu],
				(u8 *)&info);
}
EXPORT_SYMBOL(irq_trace_hook);

/*
 * Description: Record kernel task traces
 * Input:       Pre_task: current task task structure pointer, next_task: next task task structure pointer
 * Other:       added to kernel/sched/core.c
 */
void task_switch_hook(const void *pre_task, void *next_task)
{
	/* Record the timestamp, cpu_id, next_task, task name, and the loop buffer corresponding to the cpu */
	struct task_struct *task = next_task;
	struct task_info info;
	u8 cpu;

	if (!pre_task || !next_task) {
		BB_PRINT_ERR("%s() error:pre_task or next_task is NULL\n", __func__);
		return;
	}

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_TASK]))
		return;


	info.clock = hisi_getcurtime();
	cpu = (u8)smp_processor_id();
	info.pid = (u32)task->pid;
	(void)strncpy(info.comm, task->comm, sizeof(task->comm) - 1);
	info.comm[TASK_COMM_LEN - 1] = '\0';
	info.stack = (uintptr_t)task->stack;

	g_last_task_ptr[cpu] = task;
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_percpu_buffer[HK_TASK]->percpu_addr[cpu],
				(u8 *)&info);
}
EXPORT_SYMBOL(task_switch_hook);

/*
 * Description: Record the cpu into the idle power off, exit the idle power-on track
 * Input:       dir: 0 enters idle or 1 exits idle
 */
void cpuidle_stat_hook(u32 dir)
{
	/* Record timestamp, cpu_id, enter idle or exit idle to the corresponding loop buffer */
	struct cpuidle_info info;
	u8 cpu;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_CPUIDLE]))
		return;


	info.clock = hisi_getcurtime();
	cpu = (u8)smp_processor_id();
	info.dir = (u8)dir;

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_percpu_buffer[HK_CPUIDLE]->percpu_addr[cpu],
				(u8 *)&info);
}
EXPORT_SYMBOL(cpuidle_stat_hook);

/*
 * Description: The CPU inserts and deletes the core record, which is consistent with the scenario of the SR process
 * Input:       cpu:cpu number, on:1 plus core, 0 minus core
 * Other:       added to drivers/cpufreq/cpufreq.c
 */
void cpu_on_off_hook(u32 cpu, u32 on)
{
	/* Record the time stamp, cpu_id, cpu is on or off, to the corresponding loop buffer */
	struct cpu_onoff_info info;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_CPU_ONOFF]))
		return;

	info.clock = hisi_getcurtime();
	info.cpu = (u8)cpu;
	info.on = (u8)on;

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_buffer_addr[HK_CPU_ONOFF],
				(u8 *)&info);
}
EXPORT_SYMBOL(cpu_on_off_hook);

/*
 * Description: Record system call trace
 * Input:       syscall_num: system call number, dir: call in and out direction, 0: enter, 1: exit
 * Other:       added to arch/arm64/kernel/entry.S
 */
asmlinkage void syscall_hook(u32 syscall_num, u32 dir)
{
	/* Record the time stamp, system call number, to the corresponding loop buffer */
	struct syscall_info info;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_SYSCALL]))
		return;

	info.clock = hisi_getcurtime();
	info.syscall = (u32)syscall_num;
	preempt_disable();
	info.cpu = (u8)smp_processor_id();
	preempt_enable_no_resched();
	info.dir = (u8)dir;

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_buffer_addr[HK_SYSCALL], (u8 *)&info);
}
EXPORT_SYMBOL(syscall_hook);

/*
 * Description: Record the task information of the hung task
 * Input:       tsk: task struct body pointer, timeout: timeout time
 */
void hung_task_hook(void *tsk, u32 timeout)
{
	/* Record time stamp, task pid, timeout time, to the corresponding loop buffer */
	struct task_struct *task = tsk;
	struct hung_task_info info;

	if (!tsk) {
		BB_PRINT_ERR("%s() error:tsk is NULL\n", __func__);
		return;
	}

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_HUNGTASK]))
		return;

	info.clock = hisi_getcurtime();
	info.timeout = (u32)timeout;
	info.pid = (u32)task->pid;
	(void)strncpy(info.comm, task->comm, sizeof(task->comm) - 1);
	info.comm[TASK_COMM_LEN - 1] = '\0';

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_buffer_addr[HK_HUNGTASK], (u8 *)&info);
}
EXPORT_SYMBOL(hung_task_hook);

/*
 * Description: Record tasklet execution track
 * Input:       address:For the tasklet to execute the function address,
 *              dir:    call the inbound and outbound direction, 0: enter, 1: exit
 * Other:       added to arch/arm64/kernel/entry.S
 */
asmlinkage void tasklet_hook(u64 address, u32 dir)
{
	/* Record the timestamp, function address, CPU number, to the corresponding loop buffer */
	struct tasklet_info info;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_TASKLET]))
		return;

	info.clock = hisi_getcurtime();
	info.action = (u64)address;
	info.cpu = (u8)smp_processor_id();
	info.dir = (u8)dir;

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_buffer_addr[HK_TASKLET], (u8 *)&info);
}
EXPORT_SYMBOL(tasklet_hook);

/*
 * Description: Record worker execution track
 * Input:       address:for the worker to execute the function address,
 *              dir:    call the inbound and outbound direction, 0: enter, 1: exit
 * Other:       added to arch/arm64/kernel/entry.S
 */
asmlinkage void worker_hook(u64 address, u32 dir)
{
	/* Record the timestamp, function address, CPU number, to the corresponding loop buffer */
	struct worker_info info;
	u8 cpu;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_WORKER]))
		return;


	info.clock = hisi_getcurtime();
	info.action = (u64)address;
	info.pid = (u32)(current->pid);
	info.dir = (u8)dir;

	preempt_disable();
	cpu = (u8)smp_processor_id();
	info.cpu = cpu;
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_percpu_buffer[HK_WORKER]->percpu_addr[cpu],
				(u8 *)&info);
	preempt_enable();
}
EXPORT_SYMBOL(worker_hook);

/*
 * Description: Find worker execution track in delta time
 * Input:       cpu: cpu ID
 *              basetime: start time of print worker execution track
 *              delta_time: duration time of print worker execution track
 */
void worker_recent_search(u8 cpu, u64 basetime, u64 delta_time)
{
	struct worker_info *worker = NULL;
	struct worker_info *cur_worker_info = NULL;
	struct hisiap_ringbuffer_s *q = NULL;
	u32 start, end, i;
	u32 begin_worker = 0;
	u32 last_worker = 0;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_WORKER]))
		return;

	if (cpu >= CONFIG_NR_CPUS)
		return;


	atomic_set(&g_hisi_ap_hook_on[HK_WORKER], 0);

	q = (struct hisiap_ringbuffer_s *)(uintptr_t)g_hook_percpu_buffer[HK_WORKER]->percpu_addr[cpu];
	get_ringbuffer_start_end(q, &start, &end);

	for (i = start; i <= end; i++) {
		worker = (struct worker_info *)&q->data[(i % q->max_num) * q->field_count];
		if (worker->clock > basetime) {
			last_worker = i;
			break;
		}
	}

	if ((!last_worker) && (i > end))
		last_worker = end;

	if (unlikely(worker->clock < delta_time))
		delta_time = worker->clock;

	for (i = start; i <= last_worker; i++) {
		cur_worker_info = (struct worker_info *)&q->data[(i % q->max_num) * q->field_count];
		if (cur_worker_info->clock > worker->clock - delta_time) {
			begin_worker = i;
			break;
		}
	}

	if ((!begin_worker) && (i > last_worker))
		begin_worker = start;

	BB_PRINT_PN("[%s], worker info: begin_worker:%u, last_worker:%u, s_ktime:%llu, e_ktime:%llu!\n",
		__func__, begin_worker, last_worker, worker->clock - delta_time, worker->clock);

	if (begin_worker > start + AHEAD_BEGIN_WORKER) {
		begin_worker -= AHEAD_BEGIN_WORKER;
	} else {
		begin_worker = start;
	}

	for (i = begin_worker; i <= last_worker; i++) {
		cur_worker_info = (struct worker_info *)&q->data[(i % q->max_num) * q->field_count];
			BB_PRINT_PN("worker cpu:%d, pid:%u, ktime:%llu, action:%pf, dir:%u\n",
				cur_worker_info->cpu, cur_worker_info->pid, cur_worker_info->clock,
					(void *)(uintptr_t)cur_worker_info->action, cur_worker_info->dir ? 1 : 0);
	}

	atomic_set(&g_hisi_ap_hook_on[HK_WORKER], 1);
}



static int is_softirq_timeout(u32 softirq_type, u64 in, u64 out)
{
	if (in == 0 || out < in)
		return 0;
	if (((out - in) > (u64)(g_softirq_timeout_period[softirq_type] * NSEC_PER_MSEC))
			&& (g_softirq_timeout_period[softirq_type] != 0xff))
		return 1;
	else
		return 0;
}

static unsigned int hook_type_to_softirq(unsigned int hook_type)
{
	unsigned int softirq;
	switch(hook_type) {
	case HK_NET_TX_SOFTIRQ:
		softirq = NET_TX_SOFTIRQ;
		break;
	case HK_NET_RX_SOFTIRQ:
		softirq = NET_RX_SOFTIRQ;
		break;
	case HK_BLOCK_SOFTIRQ:
		softirq = BLOCK_SOFTIRQ;
		break;
	case HK_SCHED_SOFTIRQ:
		softirq = SCHED_SOFTIRQ;
		break;
	case HK_RCU_SOFTIRQ:
		softirq = RCU_SOFTIRQ;
		break;
	default:
		softirq = NR_SOFTIRQS; /*unused softirq type*/
		break;
	}
	return softirq;
}

static void record_softirq_in_ringbuffer(unsigned int hook_type, u64 address, u32 dir, struct softirq_info *info)
{
	unsigned int softirq_type = hook_type_to_softirq(hook_type);
	u8 cpu;

	if (softirq_type == NR_SOFTIRQS)
		return;

	preempt_disable();
	cpu = (u8)smp_processor_id();
	preempt_enable();

	if(dir) {
		if(is_softirq_timeout(softirq_type, per_cpu(g_softirq_in_time, cpu)[softirq_type], info->clock))
			softirq_timeout_hook(address,dir);

		hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
					g_hook_percpu_buffer[hook_type]->percpu_addr[cpu],
					(u8 *)info);
		per_cpu(g_softirq_in_time, cpu)[softirq_type] = 0;
	} else {
		per_cpu(g_softirq_in_time, cpu)[softirq_type] = info->clock;
		hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
					g_hook_percpu_buffer[hook_type]->percpu_addr[cpu],
					(u8 *)info);
	}
}


/*
 * Description: Record softirq track
 * Input:       address: function address where softirq executes
 *              dir:     direction, 0: enter, 1: exit
 */
asmlinkage void softirq_hook(unsigned int hook_type, u64 address, u32 dir)
{
	/* Record the timestamp, function address, CPU number, to the corresponding loop buffer */
	struct softirq_info info;

	if (hook_type >= HK_MAX)
		return;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[hook_type]))
		return;

	info.clock = hisi_getcurtime();
	info.action = (u64)address;
	info.dir = (u8)dir;

	record_softirq_in_ringbuffer(hook_type, address, dir, &info);
}
EXPORT_SYMBOL(softirq_hook);

/*
 * Description: Record softirq track if it executes too long time
 * Input:       address: function address where softirq executes
 *              dir:     direction, 0: enter, 1: exit
 */
asmlinkage void softirq_timeout_hook(u64 address, u32 dir)
{
	/* Record the timestamp, function address to the corresponding loop buffer */
	struct softirq_info info;

	/* hook is not installed */
	if (!atomic_read(&g_hisi_ap_hook_on[HK_TIMEOUT_SOFTIRQ]))
		return;

	info.clock = hisi_getcurtime();
	info.action = (u64)address;
	info.dir = (u8)dir;

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				g_hook_buffer_addr[HK_TIMEOUT_SOFTIRQ],
				(u8 *)&info);
}
EXPORT_SYMBOL(softirq_timeout_hook);

static int get_timeout_period_from_dts(void)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr_ap_adapter");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find rdr_ap_adapter node in dts!\n", __func__);
		return -ENODEV;
	}

	np = of_find_compatible_node(NULL, NULL, "softirq_timeout_period");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find softirq_timeout_period node in dts!\n", __func__);
		return -ENODEV;
	}

	if (of_property_read_u32_array(np, "timeout_period",
				(u32 *)g_softirq_timeout_period, NR_SOFTIRQS) != 0) {
		BB_PRINT_ERR("[%s] Get timeout_period error in dts\n", __func__);
		return -ENODEV;
	}

	return 0;
}

/*
 * Description: default oepned hook install
 */
void hisi_ap_defopen_hook_install(void)
{
	enum hook_type hk;
	u32 i, size;

	size = ARRAY_SIZE(g_hisi_defopen_hook_id);
	for (i = 0; i < size; i++) {
		hk = g_hisi_defopen_hook_id[i];
		if (g_hook_buffer_addr[hk]) {
			atomic_set(&g_hisi_ap_hook_on[hk], 1);
			BB_PRINT_DBG("[%s], hook [%d] is installed!\n", __func__, hk);
		}
	}
}

/*
 * Description: Install hooks
 * Input:       hk: hook type
 * Return:      0: The installation was successful, <0: The installation failed
 */
int hisi_ap_hook_install(enum hook_type hk)
{
	if (hk >= HK_MAX) {
		BB_PRINT_ERR("[%s], hook type [%d] is invalid!\n", __func__, hk);
		return -EINVAL;
	}

	if (g_hook_buffer_addr[hk]) {
		atomic_set(&g_hisi_ap_hook_on[hk], 1);
		BB_PRINT_DBG("[%s], hook [%d] is installed!\n", __func__, hk);
	}

	return 0;
}

/*
 * Description: Uninstall the hook
 * Input:       hk: hook type
 * Return:      0: Uninstall succeeded, <0: Uninstall failed
 */
int hisi_ap_hook_uninstall(enum hook_type hk)
{
	if (hk >= HK_MAX) {
		BB_PRINT_ERR("[%s], hook type [%d] is invalid!\n", __func__, hk);
		return -EINVAL;
	}

	atomic_set(&g_hisi_ap_hook_on[hk], 0);
	BB_PRINT_DBG("[%s], hook [%d] is uninstalled!\n", __func__, hk);

	return 0;
}

static int cpuidle_notifier(struct notifier_block *self, unsigned long cmd, void *v)
{
	if (!self) {
		BB_PRINT_ERR("[%s], self is NULL\n", __func__);
		return NOTIFY_BAD;
	}

	switch (cmd) {
	case CPU_PM_ENTER:
		cpuidle_stat_hook(0);
		break;
	case CPU_PM_EXIT:
		cpuidle_stat_hook(1);
		break;
	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

static struct notifier_block cpuidle_notifier_block = {
	.notifier_call = cpuidle_notifier,
};

#if (KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE)
static int cpu_online_hook(unsigned int cpu)
{
	cpu_on_off_hook(cpu, 1);
	return 0;
}

static int cpu_offline_hook(unsigned int cpu)
{
	cpu_on_off_hook(cpu, 0);
	return 0;
}
#else
/*
 * Description: when cpu on/off, the func will be exec. And record cpu on/off hook to memory.
 */
#if (KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE)
static int hot_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
#else
static int __cpuinit hot_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
#endif
{
	unsigned int cpu;

	if (!hcpu) {
		BB_PRINT_ERR("[%s], hcpu is NULL\n", __func__);
		return NOTIFY_BAD;
	}

	cpu = (uintptr_t)hcpu;
	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		cpu_on_off_hook(cpu, 1); /* recorded as online */
		break;
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		cpu_on_off_hook(cpu, 0); /* recorded as down */
		break;
	case CPU_DOWN_FAILED:
	case CPU_DOWN_FAILED_FROZEN:
		cpu_on_off_hook(cpu, 1); /* recorded as down failed */
		break;
	default:
		return NOTIFY_DONE;
	}

	return NOTIFY_OK;
}

static struct notifier_block __refdata hot_cpu_notifier = {
	.notifier_call = hot_cpu_callback,
};
#endif


static int __init hisi_ap_hook_init(void)
{
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	int ret;
#endif

	mutex_init(&hook_switch_mutex);

	cpu_pm_register_notifier(&cpuidle_notifier_block);

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "cpuonoff:online", cpu_online_hook, cpu_offline_hook);
	if (ret < 0)
		BB_PRINT_ERR("cpu_on_off cpuhp_setup_state_nocalls failed\n");

#else
	register_hotcpu_notifier(&hot_cpu_notifier);
#endif

	ret = get_timeout_period_from_dts();
	if (ret)
		BB_PRINT_ERR("[%s], get_timeout_period_from_dts failed!\n", __func__);

	/* wait for kernel_kobj node ready: */
	while (!kernel_kobj)
		msleep(500);

	return 0;
}

postcore_initcall(hisi_ap_hook_init);
