/*
 * include/linux/hisi/hisi_cma_debug.h
 *
 * Copyright(C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_HISI_CMD_DEBUG_H
#define _LINUX_HISI_CMD_DEBUG_H

#include <linux/dma-contiguous.h>
#include <linux/cpumask.h>
#define HISI_CMA_TRACE_FRAME_MAX 8
#define HISI_CMA_TRACE_MASTER_MAX 40

struct cma_debug_task {
	pid_t page_pid;
	atomic_t count;
	char comm[TASK_COMM_LEN];
	void *func[HISI_CMA_TRACE_FRAME_MAX];
};

enum LOG_TRACE_TAG {
	UNDO_MIGRATE_ISOLATE_PATH,
	MIGRATE_RANGE_PATH,
	ISOLATE_MIGRATE_RANGE_LOOP,
	RECLEAM_CLEAN_PAGES_FROM_LIST_LOOP,
	MIGRATE_PAGES_LOOP,
	TAG_NUM,
};

enum LOOP_TRACE_POS {
	LOOP_TRACE_ISOLATE_MIGRATE_RANGE_POS = 0,
	LOOP_TRACE_RECLEAM_CLEAN_PAGES_POS = 1,
	LOOP_TRACE_MIGRATE_PAGES_POS = 2,
	LOOP_TRACE_MIGRARE_RANGE_NUM = 3,
};

struct cma_debug_page {
	atomic64_t cur_obj;
	struct cma_debug_task entries[HISI_CMA_TRACE_MASTER_MAX];
};

struct cma_debug_traced_cma {
	struct list_head cma_debug_node;
	unsigned long cma_spfn;
	unsigned long cma_epfn;
	struct cma_debug_page *cma_traced_pages;
};

void record_cma_alloc_info(struct cma *cma, unsigned long pfn, size_t count);
void record_cma_release_info(struct cma *cma, unsigned long pfn, size_t count);
void dump_cma_mem_info(void);

void cma_record_alloc_stime(ktime_t *time);
void cma_init_record_alloc_time(ktime_t *time);
void cma_end_alloc_time_record(ktime_t stime, const void *func);
/* record the trace end time, info from the function name */
void cma_record_alloc_etime_with_func(ktime_t stime, const void *func);

/*
 * print the all record info, always show in eng version.
 * Or, in beta version or alloc total time above the
 * MM_CMA_ALLOC_MAX_LATENCY_US, show record info
 */
void show_record_alloc_time_info(ktime_t _etime, ktime_t _stime);

/* Used to record the loop body cost time */
void cma_record_alloc_etime_with_log(ktime_t stime, enum LOG_TRACE_TAG flag);

/*
 * Used to init the loop path cost time trace
 * @Param loop_trace_count, the max trace count,
 * plz not above the MAX_LOOP_TRACE_SIZE
 */
void cma_record_alloc_stime_with_log_loop(ktime_t *stime,
					  enum LOOP_TRACE_POS loop_trace_count);
void cma_record_alloc_etime_with_log_loop(ktime_t stime,
					  enum LOG_TRACE_TAG flag);

/* Used to record the path in loop total cost time */
void cma_record_alloc_stime_trace_loop(ktime_t *loop_stime);
void cma_record_alloc_etime_trace_loop(ktime_t loop_stime,
				       enum LOOP_TRACE_POS pos,
				       enum LOG_TRACE_TAG flag);

void cma_work_record_start(int cpu);
void cma_work_record_exec_time(cpumask_t *cpus);

#ifdef CONFIG_HISI_CMA_RECORD_DEBUG
void record_cma_caller_info(void);
static inline void record_dma_cma_caller_info(struct device *dev,
						   gfp_t flags)
{
	if (!dev_get_cma_area(dev) || !gfpflags_allow_blocking(flags))
		return;

	record_cma_caller_info();
}
#ifdef CONFIG_CMA_DEBUGFS
void hisi_cma_debugfs_init(void);
#endif
#endif

void dump_cma_multi_pfn(unsigned long pfn_start,
			unsigned long pfn_end);
void dump_cma_page(struct cma *cma, size_t count,
		   unsigned long mask, unsigned long offset,
		   unsigned long bitmap_maxno, unsigned long bitmap_count);
void cma_page_count_stack_saved(struct page *page, int nr, int set_flag);
void cma_set_flag(struct cma *cma, unsigned long node);
void dump_cma_debug_task(struct cma *cma);
void cma_add_debug_list(struct cma *cma);
void set_himntn_cma_trace_flag(void);
int get_himntn_cma_trace_flag(void);

void cma_mutex_lock_with_record(void);
#endif
