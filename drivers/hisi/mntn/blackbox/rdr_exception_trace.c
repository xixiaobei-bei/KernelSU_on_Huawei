/*
 * rdr_exception_trace.c
 *
 * blackbox. (kernel run data recorder.)
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/thread_info.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/kmsg_dump.h>
#include <linux/io.h>
#include <linux/kallsyms.h>
#include <linux/blkdev.h>
#include <linux/hisi/mntn_record_sp.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <linux/hisi/mntn_dump.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <securec.h>
#include <mntn_subtype_exception.h>

#include "../bl31/hisi_bl31_exception.h"
#include "platform_ap/rdr_hisi_ap_adapter.h"
#include "rdr_inner.h"
#include "rdr_print.h"
#include "rdr_field.h"


#define TICK_PER_SECOND        32768
#define MIN_EXCEPTION_TIME_GAP (5 * 60 * TICK_PER_SECOND) /* 5minute, 32768 32k time tick per second */

struct exception_core_s {
	u32 offset;
	u32 size;
};

static struct rdr_register_module_result g_current_info;
static struct exception_core_s           g_exception_core[EXCEPTION_TRACE_ENUM];
static u8                                *g_exception_trace_addr = NULL;
static                                   DEFINE_SPINLOCK(rdr_exception_trace_lock);

/*
 * in the case of reboot reason error reported, we must correct it to the real
 * reboot reason.
 * The way is to traverse each recorded exception trace, select the most early
 * exception.
 * Now only support the ap watchdog reset case, afterword extend to the other
 * popular reset exception case.
 *
 * func args:
 * @etime: the exception break out time
 * @addr: the start address of reserved memory to record the exception trace
 * @len: the length of reserved memory to record the exception trace
 * @exception: to store the corrected real exception
 *
 * return value
 * 0 success
 * otherwise failure
 *
 */
int rdr_exception_analysis_ap(u64 etime, u8 *addr, u32 len,
				struct rdr_exception_info_s *exception)
{
	struct hisiap_ringbuffer_s *q = NULL;
	rdr_exception_trace_t *trace = NULL;
	rdr_exception_trace_t *min_trace = NULL;
	u64 min_etime = etime;
	u32 start, end, i;

	q = (struct hisiap_ringbuffer_s *)addr;
	if (unlikely((q == NULL) ||
		is_ringbuffer_invalid(sizeof(rdr_exception_trace_t), len, q) ||
		(exception == NULL))) {
		BB_PRINT_ERR("%s() fail:check_ringbuffer_invalid, exception 0x%pK\n",
			__func__, exception);
		return -1;
	}

	/* ring buffer is empty, return directly */
	if (unlikely(is_ringbuffer_empty(q))) {
		BB_PRINT_ERR("%s():ring buffer is empty\n", __func__);
		return -1;
	}

	get_ringbuffer_start_end(q, &start, &end);

	min_trace = NULL;
	for (i = start; i <= end; i++) {
		trace = (rdr_exception_trace_t *)&q->data[(i % q->max_num) * q->field_count];

		if ((trace->e_exce_type == AP_S_AWDT) ||
			((rdr_get_reboot_type() == trace->e_exce_type) &&
			(rdr_get_exec_subtype_value() == trace->e_exce_subtype)))
			continue;

		if ((trace->e_32k_time < min_etime) &&
			(trace->e_32k_time + MIN_EXCEPTION_TIME_GAP >= etime)) {
			/* shall be exception which trigger the whose system reset */
			if (trace->e_reset_core_mask & RDR_AP) {
				min_trace = trace;
				min_etime = trace->e_32k_time;
			}
		}
	}

	if (unlikely(min_trace == NULL)) {
		BB_PRINT_PN("%s(): seach minimum exception trace fail\n", __func__);
		return -1;
	}

	exception->e_reset_core_mask = min_trace->e_reset_core_mask;
	exception->e_from_core       = min_trace->e_from_core;
	exception->e_exce_type       = min_trace->e_exce_type;
	exception->e_exce_subtype    = min_trace->e_exce_subtype;

	return 0;
}

static pfn_exception_analysis_ops g_exception_analysis_fn[EXCEPTION_TRACE_ENUM] = {
	rdr_exception_analysis_ap,
	rdr_exception_analysis_bl31,
};


/*
 * in the case of reboot reason error reported, we must correct it to the real
 * reboot reason.
 * Now only support the ap watchdog reset case, afterword extend to the other
 * popular reset exception case.
 * The ap watchdog breakout recording in follow ways:
 * AP_S_AWDT,
 * AP_S_PANIC:HI_APPANIC_SOFTLOCKUP,AP_S_PANIC:HI_APPANIC_OHARDLOCKUP,
 * AP_S_PANIC:HI_APPANIC_HARDLOCKUP
 *
 * return value
 * 0 return the correct ap watchdog
 * otherwise incorrect
 *
 */
static int ap_awdt_check(void)
{
	u32 reboot_reason = rdr_get_reboot_type();
	u32 subtype;

	if (reboot_reason == AP_S_AWDT)
		return 0;

	if (reboot_reason == AP_S_PANIC) {
		subtype = rdr_get_exec_subtype_value();
		if ((subtype == HI_APPANIC_SOFTLOCKUP) ||
			(subtype == HI_APPANIC_OHARDLOCKUP) ||
			(subtype == HI_APPANIC_HARDLOCKUP)) {
			return 0;
		}
	}

	return -1;
}

/*
 * get the exception breaks out real time
 *
 * func args:
 * @p_etime: to record the exception breaks out real time
 *
 * return value
 * 0 success
 * otherwise failure
 *
 */
static int ap_awdt_analysis_get_etime(u64 *p_etime)
{
	struct rdr_register_module_result info;
	struct ap_eh_root *ap_root = NULL;
	u64 offset, etime;
	u32 i;

	if (unlikely(rdr_get_areainfo(RDR_AREA_AP, &info))) {
		BB_PRINT_ERR("[%s], rdr_get_areainfo fail!\n", __func__);
		return -1;
	}

	offset = info.log_addr - rdr_reserved_phymem_addr();
	if (unlikely((offset + info.log_len > rdr_reserved_phymem_size()) ||
		(offset + sizeof(*ap_root) + PMU_RESET_RECORD_DDR_AREA_SIZE >
		rdr_reserved_phymem_size()))) {
		BB_PRINT_ERR("[%s], offset %llu log_len %u sizeof(*ap_root) %u\n"
			"PMU_RESET_RECORD_DDR_AREA_SIZE %u rdr_reserved_phymem_size %llu!\n",
			__func__, offset, info.log_len, (u32)sizeof(*ap_root),
			(u32)PMU_RESET_RECORD_DDR_AREA_SIZE, rdr_reserved_phymem_size());
		return -1;
	}

	ap_root = (struct ap_eh_root *)((u8 *)rdr_get_tmppbb() + offset + PMU_RESET_RECORD_DDR_AREA_SIZE);

	etime = 0;
	if (rdr_get_reboot_type() == AP_S_AWDT) {
		for (i = 0; i < WDT_KICK_SLICE_TIMES; i++) {
			if (ap_root->wdt_kick_slice[i] > etime)
				etime = ap_root->wdt_kick_slice[i];
		}
	} else {
		etime = ap_root->slice;
	}

	if (unlikely(etime == 0)) {
		BB_PRINT_ERR("[%s], etime invalid must not be zero!\n", __func__);
		return -1;
	}

	*p_etime = etime;
	return 0;
}

/*
 * in the case of reboot reason error reported, we must correct it to the real
 * reboot reason.
 * The way is to traverse each recorded exception trace, select the most early
 * exception.
 * Now only support the ap watchdog reset case, afterword extend to the other
 * popular reset exception case.
 *
 * func args:
 * @exception: to store the corrected real exception
 *
 * return value
 * 0 success
 * otherwise failure
 *
 */
int ap_awdt_analysis(struct rdr_exception_info_s *exception)
{
	pfn_exception_analysis_ops        ops_fn = NULL;
	u64                               offset, etime;
	u32                               i, num, size[EXCEPTION_TRACE_ENUM];
	u8                                *exception_addr = NULL;

	if (unlikely(IS_ERR_OR_NULL(exception)))
		return -1;

	if (ap_awdt_check())
		return -1;

	if (ap_awdt_analysis_get_etime(&etime))
		return -1;

	if (get_every_core_exception_info(&num, size, EXCEPTION_TRACE_ENUM)) {
		BB_PRINT_ERR("[%s], bbox_get_every_core_area_info fail!\n", __func__);
		return -1;
	}

	offset = g_current_info.log_addr - rdr_reserved_phymem_addr();
	if (unlikely(offset + g_current_info.log_len > rdr_reserved_phymem_size())) {
		BB_PRINT_ERR("[%s], offset %llu log_len %u rdr_reserved_phymem_size %llu!\n",
			__func__, offset, g_current_info.log_len, rdr_reserved_phymem_size());
		return -1;
	}

	exception_addr = (u8 *)rdr_get_tmppbb() + offset;
	offset = 0;
	for (i = 0; i < EXCEPTION_TRACE_ENUM; i++) {
		if (unlikely(offset + size[i] > g_current_info.log_len)) {
			BB_PRINT_ERR("[%s], offset %llu overflow! core %u size %u log_len %u\n",
					 __func__, offset, i, size[i], g_current_info.log_len);
			return -1;
		}

		ops_fn = g_exception_analysis_fn[i];
		if (!ops_fn) {
			offset += size[i];
			continue;
		}

		/* the ap wdt timeout 12s */
		if (ops_fn(etime + 12 * TICK_PER_SECOND, exception_addr + offset, (u32)size[i], exception)) {
			BB_PRINT_PN("[%s], pfn_exception_analysis_ops 0x%pK fail! core %u size %u\n",
				__func__, ops_fn, i, size[i]);
		} else {
			return 0;
		}

		offset += size[i];
	}

	return -1;
}

/*
 * when the exception break out, it's necessary to record it
 *
 * func args:
 * @e_reset_core_mask: notify which core need to be reset, when include
 *  the ap core to be reset that will reboot the whole system
 * @e_from_core: exception triggered from which core
 * @e_exce_type: exception type
 * @e_exce_subtype: exception subtype
 *
 * return value
 * 0 success
 * otherwise failure
 *
 */
int rdr_exception_trace_record(u64 e_reset_core_mask, u64 e_from_core,
				u32 e_exce_type, u32 e_exce_subtype)
{
	rdr_exception_trace_t trace;
	unsigned long         lock_flag;

	BB_PRINT_START();

	if (!rdr_init_done()) {
		BB_PRINT_ERR("rdr init faild!\n");
		BB_PRINT_END();
		return -1;
	}

	if (g_arch_timer_func_ptr)
		trace.e_32k_time = (*g_arch_timer_func_ptr) ();
	else
		trace.e_32k_time = jiffies_64;

	trace.e_reset_core_mask = e_reset_core_mask;
	trace.e_from_core = e_from_core;
	trace.e_exce_type = e_exce_type;
	trace.e_exce_subtype = e_exce_subtype;

	spin_lock_irqsave(&rdr_exception_trace_lock, lock_flag);

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
				(g_exception_trace_addr + g_exception_core[EXCEPTION_TRACE_AP].offset),
				(u8 *)&trace);

	spin_unlock_irqrestore(&rdr_exception_trace_lock, lock_flag);

	BB_PRINT_END();

	return 0;
}

/*
 * Get the info about the reserved debug memroy area from
 * the dtsi file.
 *
 * func args:
 * @num: the number of reserved debug memory area
 * @size: the size of each reserved debug memory area
 *
 * return value
 * 0 success
 * -1 failed
 *
 */
int get_every_core_exception_info(u32 *num, u32 *size, u32 sizelen)
{
	struct device_node *np = NULL;
	int                ret;

	if ((num == NULL) || (size == NULL)) {
		BB_PRINT_ERR("invalid  parameter num or size\n");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,exceptiontrace");
	if (unlikely(!np)) {
		BB_PRINT_ERR("[%s], find rdr_memory node fail!\n", __func__);
		return -1;
	}

	ret = of_property_read_u32(np, "area_num", num);
	if (unlikely(ret)) {
		BB_PRINT_ERR("[%s], cannot find area_num in dts!\n", __func__);
		return -1;
	}

	BB_PRINT_DBG("[%s], get area_num %u in dts!\n", __func__, *num);

	if (unlikely(*num != sizelen)) {
		BB_PRINT_ERR("[%s], invaild core num in dts!\n", __func__);
		return -1;
	}

	ret = of_property_read_u32_array(np, "area_sizes", &size[0], (unsigned long)(*num));
	if (unlikely(ret)) {
		BB_PRINT_ERR("[%s], cannot find area_sizes in dts!\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * to initialize the ring buffer head of reserved memory for core AP exception trace
 *
 * func args:
 * @addr: the virtual start address of the reserved memory for core AP exception trace
 * @size: the length of the reserved memory for core AP exception trace
 *
 * return value
 * 0 success
 * otherwise failure
 *
 */
int exception_trace_buffer_init(u8 *addr, unsigned int size)
{
	u32 min_size = sizeof(struct hisiap_ringbuffer_s) + sizeof(rdr_exception_trace_t);

	if (unlikely(IS_ERR_OR_NULL(addr)))
		return -1;

	if (unlikely(size < min_size))
		return -1;

	return hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)(addr), size,
		sizeof(rdr_exception_trace_t), NULL);
}

/*
 * to initialize the reserved memory of core AP exception trace
 *
 * func args:
 * @phy_addr: the physical start address of the reserved memory for core AP exception trace
 * @virt_addr: the virtual start address of the reserved memory for core AP exception trace
 * @log_len: the length of the reserved memory for core AP exception trace
 *
 * return value
 * 0 success
 * otherwise failure
 *
 */
static int rdr_exception_trace_ap_init(u8 *phy_addr, u8 *virt_addr, u32 log_len)
{
	(void)memset_s(virt_addr, log_len, 0, log_len);

	if (unlikely(exception_trace_buffer_init(virt_addr, log_len)))
		return -1;

	return 0;
}

static pfn_exception_init_ops g_exception_init_fn[EXCEPTION_TRACE_ENUM] = {
	rdr_exception_trace_ap_init,
	rdr_exception_trace_bl31_init,
};

/*
 * The initialize of exception trace module
 *
 * return value
 * 0 success
 * -1 failed
 *
 */
int rdr_exception_trace_init(void)
{
	pfn_exception_init_ops ops_fn = NULL;
	static bool            init = false;
	u32                    i;
	u32                    num;
	u32                    size[EXCEPTION_TRACE_ENUM];
	u32                    offset;

	if (init)
		return 0;

	BB_PRINT_START();

	if (unlikely(rdr_get_areainfo(RDR_AREA_EXCEPTION_TRACE, &g_current_info))) {
		BB_PRINT_ERR("[%s], rdr_get_areainfo fail!\n", __func__);
		goto error;
	}

	if (g_exception_trace_addr == NULL) {
		g_exception_trace_addr = (u8 *)hisi_bbox_map(g_current_info.log_addr, g_current_info.log_len);
		if (unlikely(!g_exception_trace_addr)) {
			BB_PRINT_ERR("[%s], hisi_bbox_map fail!\n", __func__);
			goto error;
		}
	}

	if (unlikely(get_every_core_exception_info(&num, size, EXCEPTION_TRACE_ENUM))) {
		BB_PRINT_ERR("[%s], bbox_get_every_core_area_info fail!\n", __func__);
		goto error;
	}

	offset = 0;
	for (i = 0; i < EXCEPTION_TRACE_ENUM; i++) {
		g_exception_core[i].offset = offset;
		g_exception_core[i].size = size[i];

		BB_PRINT_PN("[%s]core %u offset %u size %u addr 0x%pK\n",
			__func__, i, offset, size[i], g_exception_trace_addr + offset);

		offset += size[i];

		if (unlikely(offset > g_current_info.log_len)) {
			BB_PRINT_ERR("[%s], offset %u overflow! core %u size %u log_len %u\n",
					 __func__, offset, i, size[i], g_current_info.log_len);
			goto error;
		}

		ops_fn = g_exception_init_fn[i];
		if (unlikely(ops_fn &&
			ops_fn((u8 *)(uintptr_t)g_current_info.log_addr + g_exception_core[i].offset,
			g_exception_trace_addr + g_exception_core[i].offset, size[i]))) {
			BB_PRINT_ERR("[%s], exception init fail: core %u size %u ops_fn 0x%pK\n",
				__func__, i, size[i], ops_fn);
			goto error;
		}
	}

	init = true;
	BB_PRINT_END();
	return 0;

error:
	BB_PRINT_END();
	return -1;
}

