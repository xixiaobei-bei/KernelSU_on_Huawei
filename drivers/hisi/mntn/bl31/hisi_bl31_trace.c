/*
 * bl31_trace.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2019. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <global_ddr_map.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <soc_sctrl_interface.h>
#include <soc_acpu_baseaddr_interface.h>
#include <bl31_platform_memory_def.h>
#include <linux/compiler.h>
#include <asm/compiler.h>
#include <linux/debugfs.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <securec.h>
#include <linux/kthread.h>
#include "hisi_bl31_exception.h"
#include "../blackbox/rdr_inner.h"
#include "../blackbox/rdr_print.h"

#define align8(size) (((size) / 8) * 8)

#define HISI_L3SHARE_BB_CPUID_FID_VALUE     0xc500f005u
#define HISI_L3SHARE_BB_BL31INIT_FID_VALUE  0xc500f006u
#define MAX_CPU_NUM   8
#define BL31_TRACE_T_NUM    16
#define BL31_SHARE_MEM_NUM    2

static cpumask_t kernel_bl31_cpuid_notify_mask;
static struct semaphore kernel_notify_bl31_sem;

void rdr_init_sucess_notify_bl31(void)
{
	up(&kernel_notify_bl31_sem);
}

static noinline u64 __kernel_cpuid_notify_bl31(u64 cpu)
{
	register u64 x0 asm("x0") = HISI_L3SHARE_BB_CPUID_FID_VALUE;
	register u64 x1 asm("x1") = cpu;

	asm volatile (
		      __asmeq("%0", "x0")
		      __asmeq("%1", "x1")
		      "smc    #0\n"
		      : "+r"(x0)
		      : "r"(x1));

	return x0;
}

/*
 * As soon as the blackbox in kernel finish it's initialization, blakcbox will notify bl31 to
 * start bl31 mntn initialization. bl31 can start it's mntn record after the initialization.
 * How the blackbox in kernel notify bl31? Of course in smc call. It introduce two kinds of
 * smc call, one for cpu id synchronization between blackbox in kernel and bl31, the other one
 * is for the successful initilization of blackbox in kernel.
 */
static void kernel_cpuid_notify_bl31(void *info)
{
	u64 ret;
	int cpu = (int)((uintptr_t)info);

	if (unlikely(cpumask_test_cpu(cpu, &kernel_bl31_cpuid_notify_mask) == 0)) {
		ret = __kernel_cpuid_notify_bl31((u64) cpu);
		if (ret) {
			BB_PRINT_ERR("[%s], cpu %d fail ret %llu\n", __func__, cpu, ret);
		} else {
			BB_PRINT_PN("[%s], cpu %d success\n", __func__, cpu);
			cpumask_set_cpu(cpu, &kernel_bl31_cpuid_notify_mask);
		}
	}
}

/*
 * As soon as the blackbox in kernel finish it's initialization, blakcbox will notify bl31 to
 * start bl31 mntn initialization.
 */
static noinline u64 kernel_init_notify_bl31(void)
{
	register u64 x0 asm("x0") = HISI_L3SHARE_BB_BL31INIT_FID_VALUE;

	asm volatile (
		    __asmeq("%0", "x0")
		    "smc    #0\n"
		    : "+r"(x0));

	return x0;
}

/*
 * As soon as the blackbox in kernel finish it's initialization, blakcbox will notify bl31 to
 * start bl31 mntn initialization. bl31 can start it's mntn record after the initialization.
 * How the blackbox in kernel notify bl31? Of course in smc call. It introduce two kinds of
 * smc call, one for cpu id synchronization between blackbox in kernel and bl31, the other one
 * is for the successful initilization of blackbox in kernel.
 */
static int kernel_notify_bl31(void *arg)
{
	u64 u64_ret;
	int ret, cpu;

	down(&kernel_notify_bl31_sem);

	for_each_possible_cpu(cpu) {
		ret = smp_call_function_single(cpu, kernel_cpuid_notify_bl31, (void *)((uintptr_t)cpu), 0);
		if (unlikely(ret))
			BB_PRINT_ERR("[%s] fail, cpu %d\n", __func__, cpu);
	}

	/* if smc success, print sucessful, else print failure */
	u64_ret = kernel_init_notify_bl31();
	if (u64_ret)
		BB_PRINT_ERR("%s():smc call kernel_init_notify_bl31 fail ret %llu\n", __func__, u64_ret);
	else
		BB_PRINT_PN("%s():smc call kernel_init_notify_bl31 success\n", __func__);

	return 0;
}

static u32 bl31_trace_size[BL31_TRACE_ENUM] = {
	BL31_TRACE_EXCEPTION_SIZE,
	BL31_TRACE_IRQ_SMC_SIZE,
	BL31_TRACE_DMSS_SIZE,
};

/*
 * to initialize the reserved memory of bl31 exception trace
 *
 * func args:
 * @phy_addr: the physical start address of the reserved memory for bl31 exception trace
 * @virt_addr: the virtual start address of the reserved memory for bl31 exception trace
 * @log_len: the length of the reserved memory for bl31 exception trace
 */
static int bl31_trace_exception_init(u8 *phy_addr, u8 *virt_addr, u32 log_len)
{
	if (unlikely(exception_trace_buffer_init(virt_addr, log_len)))
		return -1;

	return 0;
}

/*
 * to initialize the reserved memory of bl31 irq&smc trace
 *
 * func args:
 * @phy_addr: the physical start address of the reserved memory for bl31 irq&smc trace
 * @virt_addr: the virtual start address of the reserved memory for bl31 irq&smc trace
 * @log_len: the length of the reserved memory for bl31 exception trace
 */
static int bl31_trace_irq_smc_init(u8 *phy_addr, u8 *virt_addr, u32 log_len)
{
	bl31_trace_irq_smc_head_t *head = NULL;
	u64 i, head_len, min_size, offset, size;
	u64 cpu_num = num_possible_cpus();
	int ret;
	u32 ratio[MAX_CPU_NUM][MAX_CPU_NUM] = {
		{ 16, 0, 0, 0, 0, 0, 0, 0 }, /* HERE:[8][8] */
		{ 8, 8, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 4, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 2, 0, 0, 0, 0 },
		{ 8, 4, 2, 1, 1, 0, 0, 0 },
		{ 8, 4, 1, 1, 1, 1, 0, 0 },
		{ 6, 4, 2, 1, 1, 1, 1, 0 },
		{ 6, 4, 1, 1, 1, 1, 1, 1 }
	};

	if (unlikely(cpu_num > MAX_CPU_NUM)) {
		BB_PRINT_ERR("[%s] fail, invalid cpu num %llu\n", __func__, cpu_num);
		return -1;
	}

	head_len = sizeof(bl31_trace_irq_smc_head_t) + sizeof(u64) * cpu_num;
	BB_PRINT_DBG("[%s], num_online_cpus [%llu] head_len %llu field_count %u!\n",
		__func__, cpu_num, head_len, (u32) sizeof(bl31_trace_t));

	min_size = head_len + cpu_num * (sizeof(struct hisiap_ringbuffer_s) +
		BL31_TRACE_T_NUM * sizeof(bl31_trace_t));
	if (unlikely(log_len < min_size)) {
		BB_PRINT_ERR("[%s] fail, log_len %u < min_size %llu\n", __func__, log_len, min_size);
		return -1;
	}

	head = (bl31_trace_irq_smc_head_t *)virt_addr;
	head->cpu_num = cpu_num;
	offset = head_len;

	for (i = 0; i < cpu_num; i++) {
		BB_PRINT_DBG("[%s], ratio[%llu][%llu] = [%d], offset [%llu]\n",
			 __func__, (cpu_num - 1), i, ratio[cpu_num - 1][i], offset);

		size = ((log_len - head_len) / BL31_TRACE_T_NUM) * ratio[cpu_num - 1][i];
		size = align8(size);

		head->offset[i] = offset;

		ret = hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)(virt_addr + offset),
			size, sizeof(bl31_trace_t), NULL);
		if (ret) {
			BB_PRINT_ERR("[%s], cpu [%llu] ringbuffer init failed!\n", __func__, i);
			return -1;
		}

		offset += size;
	}

	return 0;
}

/*
 * to initialize the ring buffer head of reserved memory for dmss exception trace
 *
 * func args:
 * @addr: the virtual start address of the reserved memory for dmss exception trace
 * @size: the length of the reserved memory for dmss exception trace
 */
static int dmss_trace_buffer_init(u8 *addr, unsigned int size)
{
	u32 min_size = sizeof(struct hisiap_ringbuffer_s) + sizeof(dmss_exception_trace_t);

	if (unlikely(IS_ERR_OR_NULL(addr)))
		return -1;

	if (unlikely(size < min_size))
		return -1;

	return hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)(addr), size,
		sizeof(dmss_exception_trace_t), "dmss-exception");
}

/*
 * to initialize the reserved memory of dmss exception trace
 *
 * func args:
 * @phy_addr: the physical start address of the reserved memory for dmss exception trace
 * @virt_addr: the virtual start address of the reserved memory for dmss exception trace
 * @log_len: the length of the reserved memory for dmss exception trace
 */
static int bl31_trace_dmss_init(u8 *phy_addr, u8 *virt_addr, u32 log_len)
{
	/* for no dmss trace support, return success directly */
	if (log_len == 0)
		return 0;

	if (unlikely(dmss_trace_buffer_init(virt_addr, log_len)))
		return -1;

	return 0;
}

static pfn_exception_init_ops g_bl31_trace_init_fn[BL31_TRACE_ENUM] = {
	bl31_trace_exception_init,
	bl31_trace_irq_smc_init,
	bl31_trace_dmss_init,
};

/*
 * the physical start address and length of the reserved memory for bl31 exception trace
 * should notify the bl31, so bl31 can record it's trace.
 *
 * func args:
 * @addr: the physical start address of the reserved memory for bl31 exception trace
 * @len: the length of the reserved memory for bl31 exception trace
 */
static int bl31_trace_addr_len_set(u64 addr, u64 len)
{
	struct device_node *np = NULL;
	uint32_t data[BL31_SHARE_MEM_NUM];
	int ret;
	u8 *bl31_ctrl_addr = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon, bl31_mntn");
	if (unlikely(!np)) {
		BB_PRINT_ERR("%s fail: no compatible node found\n", __func__);
		return -1;
	}

	ret = of_property_read_u32_array(np, "hisi,bl31-share-mem", &data[0], 2UL);
	if (unlikely(ret)) {
		BB_PRINT_ERR("%s fail: get val mem compatible node err\n", __func__);
		return -1;
	}

	bl31_ctrl_addr = (u8 *) ioremap(HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE + data[0], (u64) data[1]);
	if (unlikely(bl31_ctrl_addr == NULL)) {
		BB_PRINT_ERR("%s fail: allocate memory for bl31_ctrl_addr failed\n", __func__);
		return -1;
	}

	writeq(addr, (void *)(bl31_ctrl_addr + sizeof(u64)));
	writeq(len, (void *)(bl31_ctrl_addr + sizeof(u64) + sizeof(u64)));

	iounmap(bl31_ctrl_addr);
	return 0;
}

/*
 * to initialize the reserved memory of bl31 exception trace
 *
 * func args:
 * @phy_addr: the physical start address of the reserved memory for bl31 exception trace
 * @virt_addr: the virtual start address of the reserved memory for bl31 exception trace
 * @log_len: the length of the reserved memory for bl31 exception trace
 */
int rdr_exception_trace_bl31_init(u8 *phy_addr, u8 *virt_addr, u32 log_len)
{
	pfn_exception_init_ops ops_fn = NULL;
	struct task_struct *cpuid_notify_thread = NULL;
	static bool init = false;
	u32 i, offset;

	if (init)
		return 0;

	if (unlikely(IS_ERR_OR_NULL(phy_addr) || IS_ERR_OR_NULL(virt_addr)))
		return 0;

	if (memset_s(virt_addr, log_len, 0, log_len) != EOK)
		BB_PRINT_ERR("%s():%d:memset_s fail!\n", __func__, __LINE__);

	offset = 0;
	for (i = 0; i < BL31_TRACE_ENUM; i++) {
		if (unlikely(offset + bl31_trace_size[i] > log_len)) {
			BB_PRINT_ERR
			    ("[%s] fail, offset %u overflow! index %u size %u log_len %u\n",
			     __func__, offset, i, bl31_trace_size[i], log_len);
			return -1;
		}

		ops_fn = g_bl31_trace_init_fn[i];
		if (unlikely(ops_fn && ops_fn(phy_addr + offset, virt_addr + offset, bl31_trace_size[i]))) {
			BB_PRINT_ERR
			    ("[%s] fail, exception bl31 init fail: index %u size %u ops_fn 0x%pK\n",
			     __func__, i, bl31_trace_size[i], ops_fn);
			return -1;
		}

		offset += bl31_trace_size[i];
	}

	if (unlikely(bl31_trace_addr_len_set((uintptr_t) phy_addr, (u64) log_len))) {
		BB_PRINT_ERR("[%s] fail, bl31_trace_addr_len_set faild\n", __func__);
		return -1;
	}

	sema_init(&kernel_notify_bl31_sem, 0);

	cpuid_notify_thread = kthread_run(kernel_notify_bl31, NULL, "kernel_notify_bl31");
	if (unlikely(!cpuid_notify_thread)) {
		BB_PRINT_ERR("[%s] fail, create thread cpuid_notify_thread faild\n", __func__);
		return -1;
	}
	init = true;

	return 0;
}

/*
 * in the case of reboot reason error reported, we must correct it to the real
 * real reboot reason.
 * The way is to traverse each recorded exception trace, select the most early
 * exception.
 *
 * func args:
 * @etime: the exception break out time
 * @addr: the start address of reserved memory to record the exception trace
 * @len: the length of reserved memory to record the exception trace
 * @exception: to store the corrected real exception
 */
int rdr_exception_analysis_bl31(u64 etime, u8 *addr, u32 len, struct rdr_exception_info_s *exception)
{
	u32 i, offset;

	if (unlikely(IS_ERR_OR_NULL(addr) || IS_ERR_OR_NULL(exception))) {
		BB_PRINT_ERR("[%s]invalid parameter\n", __func__);
		return -1;
	}

	offset = 0;
	for (i = 0; i < BL31_TRACE_ENUM; i++) {
		if (unlikely(offset + bl31_trace_size[i] > len)) {
			BB_PRINT_ERR
			    ("[%s], offset %u overflow! core %u size %u log_len %u\n",
			     __func__, offset, i, bl31_trace_size[i], len);
			return -1;
		}

		if (i == BL31_TRACE_EXCEPTION)
			return rdr_exception_analysis_ap(etime, addr + offset, bl31_trace_size[i], exception);

		offset += bl31_trace_size[i];
	}

	return -1;
}

/*
 * The clear text printing for the reserved debug memory of bl31 exceptiontrace
 *
 * func args:
 * @dir_path: the file directory of saved clear text
 * @log_addr: the start address of reserved memory for specified core
 * @log_len: the length of reserved memory for specified core
 */
static int bl31_cleartext_exception_print(const char *dir_path, u64 log_addr, u32 log_len)
{
	struct hisiap_ringbuffer_s *q = NULL;
	struct hisiap_ringbuffer_s temp;
	rdr_exception_trace_t *trace = NULL;
	struct file *fp = NULL;
	bool error = false;
	u32 start, end, i;

	q = (struct hisiap_ringbuffer_s *)(uintptr_t) log_addr;
	if (memcpy_s(&temp, sizeof(temp), q, sizeof(*q)) != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return -1;
	}

	if (unlikely(is_ringbuffer_invalid(sizeof(rdr_exception_trace_t), log_len, &temp))) {
		BB_PRINT_ERR("%s() fail:check_ringbuffer_invalid\n", __func__);
		return -1;
	}

	/* ring buffer is empty, return directly */
	if (is_ringbuffer_empty(&temp)) {
		pr_info("%s():ring buffer is empty\n", __func__);
		return 0;
	}

	/* get the file descriptor from the specified directory path */
	fp = bbox_cleartext_get_filep(dir_path, "exception_trace_bl31.txt");
	if (unlikely(IS_ERR_OR_NULL(fp))) {
		BB_PRINT_ERR("%s() fail:fp 0x%pK\n", __func__, fp);
		return -1;
	}

	rdr_cleartext_print(fp, &error,
			    "slice          reset_core_mask   from_core      "
			    "exception_type           exception_subtype\n");

	get_ringbuffer_start_end(&temp, &start, &end);
	for (i = start; i <= end; i++) {
		trace = (rdr_exception_trace_t *)&(q->data[(i % q->max_num) * q->field_count]);
		rdr_cleartext_print(fp, &error,
				    "%-15llu0x%-16llx%-15s%-25s%-25s\n",
				    trace->e_32k_time, trace->e_reset_core_mask,
				    rdr_get_exception_core(trace->e_from_core),
				    rdr_get_exception_type(trace->e_exce_type),
				    rdr_get_subtype_name(trace->e_exce_type,
							 trace->e_exce_subtype)
		    );
	}

	/* the cleaning of specified file descriptor */
	bbox_cleartext_end_filep(fp, dir_path, "exception_trace_bl31.txt");

	if (unlikely(error == true))
		return -1;
	return 0;
}

/*
 * The clear text printing for the reserved debug memory of per cpu bl31 exceptiontrace
 *
 * func args:
 * @dir_path: the file directory of saved clear text
 * @log_addr: the start address of reserved memory for specified core
 * @log_len: the length of reserved memory for specified core
 */
static int bl31_cleartext_irq_smc_print_on_cpu(struct file *fp,
				u64 log_addr,
				u32 log_len,
				u8 *addr, u32 len, u32 cpu)
{
	struct hisiap_ringbuffer_s *q = NULL;
	struct hisiap_ringbuffer_s temp;
	bl31_trace_t *irq = NULL;
	bool error = false;
	u32 start, end, i;

	if (unlikely((addr < (u8 *) (uintptr_t) log_addr) || (addr + len > (u8 *) (uintptr_t) (log_addr + log_len)))) {
		BB_PRINT_ERR("%s() fail: addr 0x%pK len %u min_addr 0x%pK max_addr 0x%pK\n",
		     __func__, addr, len, (u8 *) (uintptr_t) log_addr, (u8 *) (uintptr_t) (log_addr + log_len));
		return -1;
	}

	q = (struct hisiap_ringbuffer_s *)addr;
	if (memcpy_s(&temp, sizeof(temp), q, sizeof(*q)) != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return -1;
	}

	if (unlikely(is_ringbuffer_invalid(sizeof(bl31_trace_t), len, &temp))) {
		BB_PRINT_ERR("%s() fail:check_ringbuffer_invalid\n", __func__);
		return -1;
	}

	rdr_cleartext_print(fp, &error, "cpu[%u]\n", cpu);
	rdr_cleartext_print(fp, &error, "slice          type dir ns id\n");

	/* ring buffer is empty, return directly */
	if (is_ringbuffer_empty(&temp))
		goto exit;

	get_ringbuffer_start_end(&temp, &start, &end);

	if (q->max_num == 0)
		return -1;

	for (i = start; i <= end; i++) {
		irq = (bl31_trace_t *)&(q->data[(i % q->max_num) * q->field_count]);
		rdr_cleartext_print(fp, &error, "%-15llu%-5s%-4s%-3s",
			irq->bl31_32k_time,
			(irq->type == BL31_TRACE_TYPE_SMC) ? "smc" :
			((irq->type == BL31_TRACE_TYPE_INTERRUPT) ? "irq" : "out"),
				(irq->dir == BL31_TRACE_IN) ? "in" : "out",
				(irq->ns == 0) ? "s" : ((irq->ns == 1) ? "ns" : "un"));

		if (irq->type == BL31_TRACE_TYPE_SMC)
			rdr_cleartext_print(fp, &error, "0x%llx\n", irq->smc.func_id);
		else if (irq->type == BL31_TRACE_TYPE_INTERRUPT)
			rdr_cleartext_print(fp, &error, "%-3u\n", irq->interrupt.id);
		else
			rdr_cleartext_print(fp, &error, "%s\n", "###");
	}

exit:
	if (unlikely(error == true))
		return -1;
	return 0;
}

/*
 * The clear text printing for the reserved debug memory of bl31 irq&smc trace
 *
 * func args:
 * @dir_path: the file directory of saved clear text
 * @log_addr: the start address of reserved memory for specified core
 * @log_len: the length of reserved memory for specified core
 */
static int bl31_cleartext_irq_smc_print(const char *dir_path, u64 log_addr, u32 log_len)
{
	bl31_trace_irq_smc_head_t *head = NULL;
	struct file *fp = NULL;
	u64 i, head_len, cpu_num, len, next_offset;
	u8 *addr = NULL;
	int ret = 0;

	/* get the file descriptor from the specified directory path */
	fp = bbox_cleartext_get_filep(dir_path, "bl31_trace.txt");
	if (unlikely(IS_ERR_OR_NULL(fp))) {
		BB_PRINT_ERR("%s() fail:fp 0x%pK\n", __func__, fp);
		return -1;
	}

	head = (bl31_trace_irq_smc_head_t *) (uintptr_t) log_addr;
	if (unlikely(log_len < sizeof(bl31_trace_irq_smc_head_t))) {
		BB_PRINT_ERR
		    ("%s() fail:log_len %u < sizeof(bl31_trace_irq_smc_head_t) %u\n",
		     __func__, log_len, (u32) sizeof(bl31_trace_irq_smc_head_t));
		ret = -1;
		goto exit;
	}

	cpu_num = head->cpu_num;
	head_len = sizeof(bl31_trace_irq_smc_head_t) + sizeof(u64) * cpu_num;
	if (unlikely(log_len < head_len)) {
		BB_PRINT_ERR("%s() fail:log_len %u < head_len %llu\n", __func__, log_len, head_len);
		ret = -1;
		goto exit;
	}

	for (i = 0; i < cpu_num; i++) {
		if (i >= cpu_num - 1)
			next_offset = log_len;
		else
			next_offset = head->offset[i + 1];

		if (unlikely((next_offset <= head->offset[i]) ||
			(next_offset > log_len) || (head->offset[i] > log_len))) {
			BB_PRINT_ERR
			    ("%s() fail:next_offset %llu <= head->offset[%llu] %llu\n",
			     __func__, next_offset, i, head->offset[i]);
			ret = -1;
			goto exit;
		}

		addr = (u8 *)(uintptr_t) log_addr + head->offset[i];
		len = next_offset - head->offset[i];

		if (unlikely(bl31_cleartext_irq_smc_print_on_cpu
			(fp, log_addr + head_len, log_len - head_len, addr, len, i))) {
			ret = -1;
			goto exit;
		}
	}

exit:
	/* the cleaning of specified file descriptor */
	bbox_cleartext_end_filep(fp, dir_path, "bl31_trace.txt");
	return ret;
}

/*
 * The clear text printing for the reserved debug memory of dmss exceptiontrace
 *
 * func args:
 * @dir_path: the file directory of saved clear text
 * @log_addr: the start address of reserved memory for specified core
 * @log_len: the length of reserved memory for specified core
 */
static int bl31_cleartext_dmss_print(const char *dir_path, u64 log_addr, u32 log_len)
{
	struct hisiap_ringbuffer_s *q = NULL;
	struct hisiap_ringbuffer_s temp;
	dmss_exception_trace_t *trace = NULL;
	struct file *fp = NULL;
	bool error = false;
	u32 start, end, i;

	/* for no dmss trace support, return success directly */
	if (log_len == 0)
		return 0;

	q = (struct hisiap_ringbuffer_s *)(uintptr_t) log_addr;
	if (memcpy_s(&temp, sizeof(temp), q, sizeof(*q)) != EOK)
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);

	if (unlikely(is_ringbuffer_invalid(sizeof(dmss_exception_trace_t), log_len, &temp))) {
		BB_PRINT_ERR("%s() fail:check_ringbuffer_invalid\n", __func__);
		return -1;
	}

	/* ring buffer is empty, return directly */
	if (is_ringbuffer_empty(&temp)) {
		pr_info("%s():ring buffer is empty\n", __func__);
		return 0;
	}

	/* get the file descriptor from the specified directory path */
	fp = bbox_cleartext_get_filep(dir_path, "exception_trace_dmss.txt");
	if (unlikely(IS_ERR_OR_NULL(fp))) {
		BB_PRINT_ERR("%s() fail:fp 0x%pK\n", __func__, fp);
		return -1;
	}

	rdr_cleartext_print(fp, &error, "slice          offset\n");

	get_ringbuffer_start_end(&temp, &start, &end);

	if (q->max_num == 0)
		return -1;

	for (i = start; i <= end; i++) {
		trace = (dmss_exception_trace_t *)&(q->data[(i % q->max_num) * q->field_count]);
		rdr_cleartext_print(fp, &error, "%-15llu0x%llx\n", trace->e_32k_time, (uintptr_t) trace - log_addr);
	}

	/* the cleaning of specified file descriptor */
	bbox_cleartext_end_filep(fp, dir_path, "exception_trace_dmss.txt");

	if (unlikely(error == true))
		return -1;
	return 0;
}

static pfn_cleartext_ops g_bl31_cleartext_fn[BL31_TRACE_ENUM] = {
	bl31_cleartext_exception_print,
	bl31_cleartext_irq_smc_print,
	bl31_cleartext_dmss_print,
};

/*
 * The clear text printing for the reserved debug memory of bl31 exceptiontrace
 *
 * func args:
 * @dir_path: the file directory of saved clear text
 * @log_addr: the start address of reserved memory for specified core
 * @log_len: the length of reserved memory for specified core
 */
int rdr_exception_trace_bl31_cleartext_print(const char *dir_path, u64 log_addr, u32 log_len)
{
	pfn_cleartext_ops ops_fn = NULL;
	u32 i, offset;

	if (unlikely(IS_ERR_OR_NULL(dir_path) || IS_ERR_OR_NULL((void *)(uintptr_t) log_addr))) {
		BB_PRINT_ERR("%s() error:dir_path 0x%pK log_addr 0x%pK\n",
			     __func__, dir_path, (void *)(uintptr_t) log_addr);
		return -1;
	}

	offset = 0;
	for (i = 0; i < BL31_TRACE_ENUM; i++) {
		if (unlikely(offset + bl31_trace_size[i] > log_len)) {
			BB_PRINT_ERR
			    ("[%s], offset %u overflow! core %u size %u log_len %u\n",
			     __func__, offset, i, bl31_trace_size[i], log_len);
			return -1;
		}

		ops_fn = g_bl31_cleartext_fn[i];
		if (unlikely(ops_fn && ops_fn(dir_path, log_addr + offset, bl31_trace_size[i]))) {
			BB_PRINT_ERR
			    ("[%s], pfn_cleartext_ops 0x%pK fail! core %u size %u\n",
			     __func__, ops_fn, i, bl31_trace_size[i]);
			return -1;
		}

		offset += bl31_trace_size[i];
	}

	return 0;
}
