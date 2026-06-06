/*
 * /ion/ion_cpa_heap.c
 *
 * Copyright (C) 2018 HUAWEI HISILICON.
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
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/of_address.h>
#include <linux/oom.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/sysctl.h>
#include <linux/workqueue.h>
#include <linux/compaction.h>
#include <linux/hisi/hisi_ion.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include <teek_client_api.h>
#include <teek_client_id.h>
#include <teek_client_constants.h>

#include "ion.h"
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#include "ion_priv.h"
#endif
#include "hisi/ion_sec_priv.h"

#define CPA_SIZE_320M_PAGE		(81920)
#define CPA_MAX_KILL_NUM		(20)
#define CPA_WATER_MARK_RATIO		(50)
#define CPA_ZRAM_RATIO			(75)
#define CPA_RK_RATIO			(5)
#define CPA_RK_RATIO_BY_ZRAM		(5)
#define CPA_ERR_LVL			(0)
#define CPA_DBG_LVL			(1)
#define CPA_KP_FREEMEM_MODEL		(0)
#define CPA_KP_ZRAM_MODEL		(1)

#define TOTAL_RAM_PAGES_1G	(1 << 18)

static unsigned int orders[] = {9, 0};

#define NUM_ORDERS    ARRAY_SIZE(orders)

struct cpa_page_info {
	struct list_head pages;
	unsigned long num;
};

struct ion_cpa_heap {
	struct ion_heap heap;
	size_t heap_size;
	atomic64_t alloc_size;
	struct ion_page_pool *pools[NUM_ORDERS];
	struct device *dev;
	/* heap mutex */
	struct mutex mutex;
	TEEC_Context *context;
	TEEC_Session *session;
	phys_addr_t pgtable_phys;
	u32 pgtable_size;
	int TA_init;
};

struct cpa_work_data {
	struct work_struct work;
	struct ion_cpa_heap *cpa_heap;
};

#define cpa_debug(level, x...)			\
	do {					\
		if (cpa_debug_level >= (level))	\
			pr_info(x);		\
	} while (0)

#define to_cpa_heap(x) container_of(x, struct ion_cpa_heap, heap)

static short cpa_process_adj = 500;
static int cpa_debug_level;
static gfp_t high_order_gfp_flags = (GFP_KERNEL | __GFP_ZERO | __GFP_NOWARN |
				     __GFP_NORETRY) & ~__GFP_DIRECT_RECLAIM;
static gfp_t low_order_gfp_flags = GFP_KERNEL | __GFP_NOWARN;
static struct cpa_work_data *cpa_mem_work_data;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
static const char state_to_char[] = TASK_STATE_TO_CHAR_STR;
static int task_state_char(unsigned long state)
{
	int bit = state ? __ffs(state) + 1 : 0;

	return bit < sizeof(state_to_char) - 1 ? state_to_char[bit] : '?'; /*lint !e574 */
}
#endif

static void cpa_dump_tasks(void)
{
	struct task_struct *p;
	struct task_struct *task;
	short tsk_oom_adj = 0;
	unsigned long tsk_nr_ptes = 0;
	int task_state = 0;
	char frozen_mark = ' ';

	pr_info("[ pid ]   uid  tgid total_vm    rss nptes  swap   adj s name\n");

	rcu_read_lock();
	for_each_process(p) {
		task = find_lock_task_mm(p);
		if (!task)
			continue;

		tsk_oom_adj = task->signal->oom_score_adj;
		tsk_nr_ptes = (unsigned long)atomic_long_read(&task->mm->nr_ptes);

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		task_state = task_state_char(task->state);
#else
		task_state = (int)__get_task_state(task);
#endif
		frozen_mark = frozen(task) ? '*' : ' ';

		pr_info("[%5d] %5d %5d %8lu %6lu %5lu %5lu %5hd %c %s%c\n",
			task->pid, from_kuid(&init_user_ns, task_uid(task)),
			task->tgid, task->mm->total_vm, get_mm_rss(task->mm),
			tsk_nr_ptes,
			get_mm_counter(task->mm, MM_SWAPENTS),
			tsk_oom_adj,
			task_state,
			task->comm,
			frozen_mark); /*lint !e1058*/
		task_unlock(task);
	}
	rcu_read_unlock();
}

static void cpa_show_mem(int debug_lev, char *show_msg)
{
	cpa_debug(debug_lev, "CPA show mem: %s\n", show_msg);
	if (debug_lev <= cpa_debug_level)
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		show_mem(0);
#else
		show_mem(0, NULL);
#endif
}

static unsigned long cpa_free_mem_page(void)
{
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	unsigned long free_pages = global_page_state(NR_FREE_PAGES);/*lint !e578*/
#else
	unsigned long free_pages = global_zone_page_state(NR_FREE_PAGES);/*lint !e578*/
#endif
	if (free_pages > totalreserve_pages)
		free_pages -= totalreserve_pages;

	return free_pages;
}

static void cpa_direct_dropcache(void)
{
	cpa_drop_pagecache();
	drop_slab();
}

/* Compact all nodes in the system */
static void cpa_direct_compact(int model)
{
	cpa_compact_nodes(model);
}

static unsigned long cpa_reclaim_anon2zram(unsigned long nr_to_reclaim)
{
	return cpa_shrink_all_memory(nr_to_reclaim);
}

static unsigned long cpa_direct_kill(int model, unsigned long reclaim_by_kill)
{
	struct task_struct *p = NULL;
	struct task_struct *tsk = NULL;
	struct task_struct *selected = NULL;
	short oom_score_adj = 0;
	unsigned long selected_task_size = 0;
	int nr_to_kill = CPA_MAX_KILL_NUM;
	unsigned long task_size = 0;
	unsigned long rem = 0;

	rcu_read_lock();
	do {
		for_each_process(tsk) {
			if (tsk->flags & PF_KTHREAD)
				continue;

			p = find_lock_task_mm(tsk);
			if (!p)
				continue;

			if (selected == p) {
				task_unlock(p);
				continue;
			}

			if (test_tsk_thread_flag(p, TIF_MEMDIE)) {
				task_unlock(p);
				continue;
			}

			oom_score_adj = p->signal->oom_score_adj;
			if (oom_score_adj < cpa_process_adj) {
				task_unlock(p);
				continue;
			}

			if (!model)
				task_size = get_mm_rss(p->mm);
			else
				task_size = get_mm_counter(p->mm, MM_SWAPENTS);

			task_unlock(p);

			if (task_size <= 0)
				continue;

			if (selected) {
				if (task_size < selected_task_size)
					continue;
			}
			oom_score_adj = 0;
			selected = p;
			selected_task_size = task_size;
		}

		if (selected) {
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
			unsigned long other_free =
					global_page_state(NR_FREE_PAGES) -
					global_page_state(NR_FREE_CMA_PAGES) -
					totalreserve_pages;
#else
			unsigned long other_free =
					global_zone_page_state(NR_FREE_PAGES) -
					global_zone_page_state(NR_FREE_PAGES) -
					totalreserve_pages;
#endif
			unsigned long other_file =
					global_node_page_state(NR_FILE_PAGES) -
					global_node_page_state(NR_SHMEM) -
					total_swapcache_pages();
			unsigned long other_anon =
					global_node_page_state(NR_ACTIVE_ANON) +
					global_node_page_state(NR_INACTIVE_ANON);

			task_lock(selected);
			send_sig(SIGKILL, selected, 0);
			task_unlock(selected);

			cpa_debug(CPA_DBG_LVL, "%s : Killing '%s' (%d), tgid=%d, adj %hd\n"
				  "   to free %ldkB on behalf of '%s' (%d) when\n"
				  "   free %lukB  file %lukB  anon %lukB\n",
				  __func__,
				  selected->comm, selected->pid, selected->tgid,
				  selected->signal->oom_score_adj,
				  selected_task_size * (long)(PAGE_SIZE / 1024),
				  current->comm, current->pid,
				  other_free * (PAGE_SIZE / 1024),
				  other_file * (PAGE_SIZE / 1024),
				  other_anon * (PAGE_SIZE / 1024));

			rem += selected_task_size;
		}

		p = NULL;
		tsk = NULL;
		selected_task_size = 0;
	} while ((rem < reclaim_by_kill) && (--nr_to_kill));

	rcu_read_unlock();

	return rem;
}

static inline unsigned int order_to_size(int order)
{
	return (PAGE_SIZE << order);
}

static int order_to_index(unsigned int order)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++)
		if (order == orders[i])
			return i;

	return -1;
}

static struct page *alloc_buffer_page(struct ion_cpa_heap *cpa_heap,
				      unsigned long order,
				      unsigned int is_nofail)
{
	struct ion_page_pool *pool;
	struct page *page;

	pool = cpa_heap->pools[order_to_index(order)];
	if (is_nofail && (order <= orders[1]))
		pool->gfp_mask = (pool->gfp_mask | __GFP_NOFAIL) &
							(~__GFP_NORETRY);

	/*
	 * if need to use page pool;
	 * now using the alloc func as below:
	 * page = ion_page_pool_alloc(pool);
	 */
	page = ion_page_pool_alloc(pool);

	return page;
}

static void free_buffer_page(struct ion_cpa_heap *cpa_heap,
			     struct ion_buffer *buffer, struct page *page)
{
	struct ion_page_pool *pool;
	unsigned int order = compound_order(page);

	pool = cpa_heap->pools[order_to_index(order)];

	/*
	 * if need to use page pool;
	 * now using the alloc func as below:
	 * ion_page_pool_free(pool, page);
	 */
	__free_pages(page, pool->order);
}

static struct page *alloc_largest_available(struct ion_cpa_heap *cpa_heap,
					    unsigned long ac_sz,
					    unsigned int max_order,
					    unsigned int is_nofail)
{
	unsigned int i;
	struct page *page;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (ac_sz < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		page = alloc_buffer_page(cpa_heap, orders[i], is_nofail);
		if (!page)
			continue;

		return page;
	}

	return NULL;
}

static unsigned long cpa_alloc_large(struct ion_cpa_heap *cpa_heap,
				     struct cpa_page_info *info,
				     unsigned long size,
				     unsigned int is_nofail)
{
	struct page *page;
	unsigned int max_order = orders[0];
	unsigned long size_remaining = size;

	while (size_remaining > 0) {
		page = alloc_largest_available(cpa_heap,
					       size_remaining,
					       max_order,
					       is_nofail);
		if (!page)
			break;
		max_order = compound_order(page);
		list_add_tail(&page->lru, &info->pages);
		info->num++;
		size_remaining -= (PAGE_SIZE << max_order);
	}

	return (size - size_remaining);
}

static int __cpa_heap_input_check(struct ion_cpa_heap *cpa_heap,
				  unsigned long size)
{
	long int alloc_size;

	alloc_size = atomic64_read(&cpa_heap->alloc_size);
	if ((alloc_size + size) > cpa_heap->heap_size) {
		pr_err("alloc size = 0x%lx, size = 0x%lx, heap size = 0x%lx\n",
		       alloc_size, size,
		       cpa_heap->heap_size);
		return -EINVAL;
	}

	if (size > SZ_64M) {
		pr_err("size too large! size 0x%lx, per_alloc_sz 0x4000000(64M)\n",
		       size);
		return -EINVAL;
	}

	return 0;
}

static int change_cpa_prop(struct ion_cpa_heap *cpa_heap,
			   struct ion_buffer *buffer, u32 cmd)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct tz_pageinfo *pageinfo = NULL;
	struct mem_chunk_list mcl;
	unsigned int nents = table->nents;
	unsigned int i = 0;
	int ret = 0;

	if (!cpa_heap->TA_init) {
		pr_err("[%s] TA not inited.\n", __func__);
		return -EINVAL;
	}

	if (cmd == ION_SEC_CMD_ALLOC) {
		pageinfo = kcalloc(nents, sizeof(*pageinfo), GFP_KERNEL);
		if (!pageinfo)
			return -ENOMEM;

		for_each_sg(table->sgl, sg, table->nents, i) {
			page = sg_page(sg);
			pageinfo[i].addr = page_to_phys(page);
			pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
		}

		mcl.phys_addr = (void *)pageinfo;
		mcl.nents = nents;
		mcl.protect_id = SEC_TASK_DRM;
	} else if (cmd == ION_SEC_CMD_FREE) {
		mcl.protect_id = SEC_TASK_DRM;
		mcl.buff_id = buffer->id;
		mcl.phys_addr = NULL;
	} else {
		pr_err("[%s] Error cmd\n", __func__);
		return -EINVAL;
	}

	ret = secmem_tee_exec_cmd(cpa_heap->session, &mcl, cmd);
	if (ret) {
		pr_err("[%s] Exec cmd[%d] fail\n", __func__, cmd);
		ret = -EINVAL;
	} else {
		if (cmd == ION_SEC_CMD_ALLOC)
			buffer->id = mcl.buff_id;
	}

	if (pageinfo)
		kfree(pageinfo);

	return ret;
}

static int cpa_propare_session(struct ion_cpa_heap *cpa_heap)
{
	int ret = 0;

	if (!cpa_heap->TA_init) {
		ret = secmem_tee_init(cpa_heap->context,
				      cpa_heap->session);
		if (ret) {
			pr_err("[%s] TA init failed\n", __func__);
			goto err;
		}

		if (!ion_sgtable_init && cpa_heap->pgtable_phys) {
			ret = ion_sg_pgtable_init(cpa_heap->session,
						  cpa_heap->pgtable_phys,
						  cpa_heap->pgtable_size);
			if (ret) {
				pr_err("[%s] TA init failed\n", __func__);
				goto err;
			}
		}

		cpa_heap->TA_init = 1;
	}

err:
	return ret;
}

static inline unsigned long cpa_water_mark_page(struct ion_cpa_heap *cpa_heap)
{
	unsigned long alloc_size = (unsigned long)atomic64_read(&cpa_heap->alloc_size);

	return (((cpa_heap->heap_size - alloc_size) * 100 / CPA_WATER_MARK_RATIO) >> PAGE_SHIFT);
}

static inline unsigned long get_file_page(void)
{
	return global_node_page_state(NR_FILE_PAGES) -
		    global_node_page_state(NR_SHMEM) -
		    global_node_page_state(NR_UNEVICTABLE);
}

static inline unsigned long get_free_page(void)
{
	return cpa_free_mem_page();
}

static inline unsigned long get_zram_page(void)
{
	return (unsigned long)(total_swap_pages - get_nr_swap_pages());
}

static void cpa_prepare_memory(struct ion_cpa_heap *cpa_heap,
				int compact_model,
				unsigned long water_mark_page,
				unsigned long tokill_rem_page,
				unsigned long toreclaim_page,
				unsigned long *kill_rem_page,
				unsigned long *reclaimed_page)
{
	unsigned long free_page;
	unsigned long file_page;
	unsigned long file_water_mark_page;
	unsigned long zram_page;
	unsigned long zram_water_mark_page;

	cpa_show_mem(CPA_DBG_LVL, "start cpa prepare memory\n");

	/**
	 * 1.dropcache
	 */
	file_water_mark_page = CPA_SIZE_320M_PAGE;
	free_page = get_free_page();
	file_page = get_file_page();

	cpa_debug(CPA_DBG_LVL, "[%s] step dropcache before, free_page: %lu, water_mark_page: %lu; file_page: %lu, file_page_water_mark: %lu\n",
		  __func__, free_page, water_mark_page,
		  file_page, file_water_mark_page);

	if ((free_page < water_mark_page) &&
	    (file_page > file_water_mark_page)) {

		cpa_direct_dropcache();

		cpa_show_mem(CPA_DBG_LVL, "after dropcache\n");
	}

	/**
	 * 2.kill process
	 */
	zram_page = get_zram_page();
	zram_water_mark_page = (total_swap_pages * CPA_ZRAM_RATIO / 100);

	cpa_debug(CPA_DBG_LVL, "[%s] step kill proc before, zram_page: %lu, zram_water_mark_page: %lu\n",
		  __func__, zram_page, zram_water_mark_page);

	if (zram_page > zram_water_mark_page) {

		*kill_rem_page = cpa_direct_kill(CPA_KP_ZRAM_MODEL,
						 tokill_rem_page);
		if (!*kill_rem_page) {
			cpa_show_mem(CPA_ERR_LVL,
				     "prepare memory kill process fail\n");
			cpa_dump_tasks();
		}

		cpa_show_mem(CPA_DBG_LVL, "after kill process\n");
	}

	/**
	 * 3.reclaim memory
	 */
	free_page = get_free_page();
	cpa_debug(CPA_DBG_LVL, "[%s] step reclaim before, free_page: %lu, water_mark_page: %lu\n",
		  __func__, free_page, water_mark_page);

	if (free_page < water_mark_page) {
		*reclaimed_page = cpa_reclaim_anon2zram(toreclaim_page);
		if (!*reclaimed_page)
			cpa_show_mem(CPA_ERR_LVL,
				     "prepare memory reclaim memory fail\n");

		if (!compact_model)
			cpa_direct_compact(compact_model);

		cpa_show_mem(CPA_DBG_LVL, "after reclaim\n");
	}

	if (compact_model)
		cpa_direct_compact(compact_model);

	cpa_show_mem(CPA_DBG_LVL, "finish cpa prepare memory\n");
}

static void cpa_prepare_memory_for_work(struct work_struct *work)
{
	unsigned long water_mark_page = 0;
	unsigned long free_page = 0;
	unsigned long tokill_rem_page = 0;
	unsigned long toreclaim_page = 0;
	unsigned long kill_rem_page = 0;
	unsigned long reclaimed_page = 0;
	struct cpa_work_data *work_data = container_of(work, struct cpa_work_data, work);

	water_mark_page = cpa_water_mark_page(work_data->cpa_heap);
	free_page = get_free_page();
	tokill_rem_page = CPA_RK_RATIO_BY_ZRAM * (water_mark_page - free_page);
	toreclaim_page = water_mark_page - free_page;

	cpa_prepare_memory(work_data->cpa_heap, 1, water_mark_page,
			    tokill_rem_page, toreclaim_page,
			   &kill_rem_page, &reclaimed_page);

	cpa_debug(CPA_DBG_LVL, "[%s] kill process free mem: %lu, reclaim mem reclaimed: %lu\n",
		  __func__, kill_rem_page, reclaimed_page);
}

static void cpa_prepare_memory_for_alloc(struct ion_cpa_heap *cpa_heap,
					unsigned long size,
					unsigned long *kill_rem_page,
					unsigned long *reclaimed_page)
{
	unsigned long water_mark_page;
	unsigned long tokill_rem_page;
	unsigned long toreclaim_page;

	water_mark_page = cpa_water_mark_page(cpa_heap);
	tokill_rem_page = (size / PAGE_SIZE);
	toreclaim_page = tokill_rem_page * 100 /CPA_WATER_MARK_RATIO;

	cpa_prepare_memory(cpa_heap, 0, water_mark_page,
			   tokill_rem_page, toreclaim_page,
			   kill_rem_page, reclaimed_page);

	cpa_debug(CPA_DBG_LVL,"[%s] kill process free mem:%lu, reclaim mem reclaimed:%lu\n",
			__func__, *kill_rem_page, *reclaimed_page);
}

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
static int ion_cpa_allocate(struct ion_heap *heap,
			    struct ion_buffer *buffer,
			    unsigned long size, unsigned long align,
			    unsigned long flags)
#else
static int ion_cpa_allocate(struct ion_heap *heap,
			    struct ion_buffer *buffer,
			    unsigned long size, unsigned long flags)
#endif
{
	struct page *page = NULL;
	struct page *tmp_page = NULL;
	struct scatterlist *sg = NULL;
	struct sg_table *table = NULL;
	struct ion_cpa_heap *cpa_heap = to_cpa_heap(heap);
	struct cpa_page_info info;
	unsigned long size_remaining = ALIGN(size, order_to_size(orders[1]));
	unsigned long kill_rem_page = 0;
	unsigned long reclaimed_page = 0;
	unsigned long retry_to_rk_page = 0;
	unsigned long retry_kill_rem_page = 0;
	unsigned long retry_reclaimed_page = 0;
	unsigned int i = 0;
	int ret = 0;
	bool flag = false;

	cpa_debug(CPA_DBG_LVL, "cpa alloc enter [%s] -- alloc size: 0x%lx, heap_id: %u, cpa heap all alloc_size: 0x%lx\n",
		  __func__, size, heap->id,
		  atomic64_read(&cpa_heap->alloc_size));

	mutex_lock(&cpa_heap->mutex);

	if (__cpa_heap_input_check(cpa_heap, size)) {
		pr_err("cpa alloc input check: params err!\n");
		ret = -EINVAL;
		goto out;
	}

	info.num = 0;
	INIT_LIST_HEAD(&info.pages);

	schedule_work(&(cpa_mem_work_data->work));

	/**
	 * 1.cpa prepare session for the first time
	 */
	if (cpa_propare_session(cpa_heap))
		goto out;

	/**
	 * 2.cpa prepare memory for remain size alloc
	 */

	cpa_prepare_memory_for_alloc(cpa_heap, size_remaining,
				     &kill_rem_page, &reclaimed_page);

	size_remaining -= cpa_alloc_large(cpa_heap,
					  &info, size_remaining, flag);
	if (size_remaining) {

		retry_to_rk_page = size / PAGE_SIZE;

		retry_kill_rem_page = cpa_direct_kill(CPA_KP_FREEMEM_MODEL,
						      retry_to_rk_page);
		if (!retry_kill_rem_page)
			cpa_debug(CPA_ERR_LVL, "[%s]retry kill process but no free\n",
				  __func__);

		retry_reclaimed_page = cpa_reclaim_anon2zram(retry_to_rk_page);
		if (!retry_reclaimed_page)
			cpa_debug(CPA_ERR_LVL, "[%s]retry alloc to reclaim page: no mem page reclaim\n",
				  __func__);

		cpa_direct_compact(1);

		size_remaining -= cpa_alloc_large(cpa_heap, &info,
						  size_remaining, flag);
	}

	if (size_remaining) {
		pr_err("[%s] No enough memory to alloc! size_remaining: %lu\n",
		       __func__, size_remaining);

		pr_err("[%s] prepare memory argument show kill freed: %lu, mem reclaimed: %lu\n",
		       __func__, kill_rem_page, reclaimed_page);

		pr_err("[%s] retry alloc -- need free mempage:%lu; kill proc freed: %lu, reclaim mem reclaimed: %lu\n",
		       __func__, retry_to_rk_page,
		       retry_kill_rem_page, retry_reclaimed_page);

		cpa_show_mem(CPA_ERR_LVL, "cpa alloc fail\n");

		retry_kill_rem_page = cpa_direct_kill(CPA_KP_FREEMEM_MODEL,
						      CPA_RK_RATIO * (size / PAGE_SIZE));
		cpa_debug(CPA_ERR_LVL, "[%s]No enough mem kill process tofree mem page: %lu, free mem page: %lu\n",
			  __func__, (CPA_RK_RATIO * (size / PAGE_SIZE)),
			  retry_kill_rem_page);

		schedule_work(&(cpa_mem_work_data->work));

		ret = -1;
		goto free_pages;
	}

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		ret = -1;
		goto free_pages;
	}

	ret = sg_alloc_table(table, info.num, GFP_KERNEL);
	if (ret) {
		ret = -1;
		goto free_table;
	}

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &info.pages, lru) {
		change_secpage_range(page_to_phys(page),
				     (unsigned long)page_address(page),
				     PAGE_SIZE << compound_order(page),
				     __pgprot(PROT_DEVICE_nGnRE));
		sg_set_page(sg, page, PAGE_SIZE << compound_order(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	flush_tlb_all();
	ion_flush_all_cpus_caches();
	buffer->sg_table = table;

	ret = change_cpa_prop(cpa_heap, buffer, ION_SEC_CMD_ALLOC);
	if (ret) {
		ret = -1;
		goto free_sg_table;
	}

	atomic64_add(size, &cpa_heap->alloc_size);

	mutex_unlock(&cpa_heap->mutex);

	cpa_debug(CPA_DBG_LVL, "cpa alloc succ -- alloc size: 0x%lx , have already alloc size: 0x%lx\n",
		  size, atomic64_read(&cpa_heap->alloc_size));

	return 0;

free_sg_table:
	for_each_sg(table->sgl, sg, table->nents, i) {
		change_secpage_range(page_to_phys(sg_page(sg)),
				     (unsigned long)page_address(sg_page(sg)),
				     sg->length, PAGE_KERNEL);
		flush_tlb_all();
	}
	flush_tlb_all();
	sg_free_table(table);

free_table:
	kfree(table);

free_pages:
	list_for_each_entry_safe(page, tmp_page, &info.pages, lru)
		free_buffer_page(cpa_heap, buffer, page);

out:
	mutex_unlock(&cpa_heap->mutex);

	cpa_debug(CPA_DBG_LVL, "CPA alloc failed!");

	return ret;
}

static void cpa_free(struct ion_buffer *buffer)
{
	struct ion_cpa_heap *cpa_heap = to_cpa_heap(buffer->heap);
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg;
	unsigned long size = 0;
	unsigned int i = 0;

	/* add to cpa pool */
	for_each_sg(table->sgl, sg, table->nents, i) {
		change_secpage_range(page_to_phys(sg_page(sg)),
				     (unsigned long)page_address(sg_page(sg)),
				     sg->length, PAGE_KERNEL);
		flush_tlb_all();
		free_buffer_page(cpa_heap, buffer, sg_page(sg));
	}

	/* shrink to buddy */
	size = buffer->size;
	atomic64_sub(size, &cpa_heap->alloc_size);
	if (!atomic64_read(&cpa_heap->alloc_size)) {
		for (i = 0; i < NUM_ORDERS; i++)
			(void)ion_page_pool_shrink(cpa_heap->pools[i],
				0, (int)(cpa_heap->heap_size >> PAGE_SHIFT));
	}
}

static void ion_cpa_free(struct ion_buffer *buffer)
{
	struct ion_cpa_heap *cpa_heap = to_cpa_heap(buffer->heap);
	struct sg_table *table = buffer->sg_table;
	unsigned long size = buffer->size;
	int ret = 0;

	mutex_lock(&cpa_heap->mutex);

	ret = change_cpa_prop(cpa_heap, buffer, ION_SEC_CMD_FREE);
	if (ret) {
		mutex_unlock(&cpa_heap->mutex);
		pr_err("cpa release MPU protect fail! Need check DRM runtime\n");
		return;
	}

	cpa_free(buffer);
	flush_tlb_all();
	sg_free_table(table);
	kfree(table);

	mutex_unlock(&cpa_heap->mutex);

	cpa_debug(CPA_DBG_LVL, "cpa free succ -- free size: 0x%lx, cpa heap size: 0x%lx, have already alloc size: 0x%lx\n",
		  size, cpa_heap->heap_size,
		  atomic64_read(&cpa_heap->alloc_size));
}

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
static int cpa_map_iommu(struct ion_cpa_heap *cpa_heap,
			 struct ion_buffer *buffer,
			 struct ion_iommu_map *map_data)
{
	struct mem_chunk_list mcl;
	int ret;

	if (!cpa_heap->TA_init) {
		pr_err("[%s] TA not inited.\n", __func__);
		return -EINVAL;
	}

	mcl.protect_id = SEC_TASK_DRM;
	mcl.buff_id = buffer->id;
	mcl.phys_addr = NULL;
	mcl.size = (u32)buffer->size;
	if (mcl.size != buffer->size) {
		pr_err("[%s] size(0x%zx) too large\n", __func__, buffer->size);
		return -EINVAL;
	}

	mutex_lock(&cpa_heap->mutex);
	ret = secmem_tee_exec_cmd(cpa_heap->session,
				  &mcl, ION_SEC_CMD_MAP_IOMMU);
	mutex_unlock(&cpa_heap->mutex);
	if (ret) {
		pr_err("[%s] exec map iommu cmd fail\n", __func__);
		return -EINVAL;
	}

	map_data->format.iova_start = mcl.va;
	map_data->format.iova_size = buffer->size;

	return 0;
}

static int ion_cpa_heap_map_iommu(struct ion_buffer *buffer,
				  struct ion_iommu_map *map_data)
{
	struct ion_cpa_heap *cpa_heap = to_cpa_heap(buffer->heap);
	int ret = 0;

	ret = cpa_map_iommu(cpa_heap, buffer, map_data);
	if (ret)
		pr_err("[%s] cpa heap map iommu fail\n", __func__);
	else
		cpa_debug(CPA_DBG_LVL, "[%s] cpa map iommu succ\n",
			  __func__);

	return ret;
}

static int cpa_unmap_iommu(struct ion_cpa_heap *cpa_heap,
			   struct ion_buffer *buffer,
			   struct ion_iommu_map *map_data)
{
	struct mem_chunk_list mcl;
	int ret = 0;

	if (!cpa_heap->TA_init) {
		pr_err("[%s] TA not inited.\n", __func__);
		return -EINVAL;
	}

	mcl.protect_id = SEC_TASK_DRM;
	mcl.buff_id = buffer->id;
	mcl.phys_addr = NULL;

	mutex_lock(&cpa_heap->mutex);
	ret = secmem_tee_exec_cmd(cpa_heap->session,
				  &mcl, ION_SEC_CMD_UNMAP_IOMMU);
	mutex_unlock(&cpa_heap->mutex);
	if (ret)
		pr_err("[%s] exec unmap iommu cmd fail\n", __func__);

	return ret;
}

static void ion_cpa_heap_unmap_iommu(struct ion_iommu_map *map_data)
{
	struct ion_buffer *buffer = map_data->buffer;
	struct ion_cpa_heap *cpa_heap = to_cpa_heap(buffer->heap);
	int ret = 0;

	ret = cpa_unmap_iommu(cpa_heap, buffer, map_data);
	if (ret)
		pr_err("[%s] cpa heap unmap iommu fail\n", __func__);
	else
		cpa_debug(CPA_DBG_LVL, "[%s] cpa unmap iommu succ\n",
			  __func__);
}
#endif

int ion_cpa_heap_phys(struct ion_heap *heap, struct ion_buffer *buffer,
		      phys_addr_t *addr, size_t *len)
{
	if (heap->type != ION_HEAP_TYPE_CPA)
		return -EINVAL;

	*addr = buffer->id;
	*len = buffer->size;

	return 0;
}

/*
 * if need to use page pool;
 * now using the alloc func as below:
 *
 * FUNC is for pool shrink
 * static int ion_cpa_heap_shrink(struct ion_heap *heap, gfp_t gfp_mask,
 * int nr_to_scan)
 * {
 * struct ion_page_pool *pool;
 * struct ion_cpa_heap *cpa_heap = to_cpa_heap(heap);
 * int nr_total = 0;
 * int i, nr_freed;
 * int only_scan = 0;
 *
 * if (!nr_to_scan)
 * only_scan = 1;
 *
 * for (i = 0; i < NUM_ORDERS; i++) {
 * pool = cpa_heap->pools[i];
 *
 * if (only_scan) {
 * nr_total += ion_page_pool_shrink(pool,
 * gfp_mask,
 * nr_to_scan);
 * } else {
 * nr_freed = ion_page_pool_shrink(pool,
 * gfp_mask,
 * nr_to_scan);
 * nr_to_scan -= nr_freed;
 * nr_total += nr_freed;
 * if (nr_to_scan <= 0)
 * break;
 * }
 * }
 *
 * return nr_total;
 * }
 */

static struct ion_heap_ops ion_cpa_ops = {
	.allocate = ion_cpa_allocate,
	.free = ion_cpa_free,
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	.map_iommu = ion_cpa_heap_map_iommu,
	.unmap_iommu = ion_cpa_heap_unmap_iommu,
#endif
	/*
	 * if need to use page pool;
	 * now using the alloc func as below:
	 *
	 * .shrink = ion_cpa_heap_shrink,
	 */
};

static int cpa_parse_dt(struct device *dev, struct ion_platform_heap *data,
			struct ion_cpa_heap *cpa_heap)
{
	struct device_node *nd;
	struct device_node *mem_region;
	struct resource res;
	int ret = 0;

	nd = of_get_child_by_name(dev->of_node, data->name);
	if (!nd) {
		pr_err("cpa can't of_get_child_by_name %s\n", data->name);
		ret = -EINVAL;
		goto out;
	}

	mem_region = of_parse_phandle(nd, "memory-region", 0);
	if (mem_region) {
		ret = of_address_to_resource(mem_region, 0, &res);
		of_node_put(mem_region);
		if (ret) {
			pr_err("failed to parse memory-region: %d\n", ret);
			goto out;
		}
		cpa_heap->pgtable_phys = res.start;
		cpa_heap->pgtable_size = resource_size(&res);
	} else {
		cpa_heap->pgtable_phys = 0;
		cpa_heap->pgtable_size = 0;
		pr_err("cpa no memory-region phandle\n");
	}

out:
	return ret;
}

static void ion_cpa_heap_destroy_pools(struct ion_page_pool **pools)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++)
		if (pools[i])
			ion_page_pool_destroy(pools[i]);
}

static int ion_cpa_heap_create_pools(struct ion_page_pool **pools)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		struct ion_page_pool *pool;
		gfp_t gfp_flags = low_order_gfp_flags;

		if (orders[i] >= 8)
			gfp_flags = high_order_gfp_flags & ~__GFP_RECLAIMABLE;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		pool = ion_page_pool_create(gfp_flags, orders[i],
					    false, false);
#else
		pool = ion_page_pool_create(gfp_flags, orders[i]);
#endif
		if (!pool)
			goto err_create_pool;
		pools[i] = pool;
	}

	return 0;

err_create_pool:
	ion_cpa_heap_destroy_pools(pools);

	return -ENOMEM;
}

struct ion_heap *ion_cpa_heap_create(struct ion_platform_heap *data)
{
	struct ion_cpa_heap *cpa_heap;
	struct cpa_work_data *cpa_work_data;
	struct device *dev;
	int ret;

	cpa_heap = kzalloc(sizeof(*cpa_heap), GFP_KERNEL);

	if (!cpa_heap)
		return ERR_PTR(-ENOMEM);

	cpa_work_data = kzalloc(sizeof(*cpa_work_data), GFP_KERNEL);
	if (!cpa_work_data)
		return ERR_PTR(-ENOMEM);

	INIT_WORK(&(cpa_work_data->work), cpa_prepare_memory_for_work);

	mutex_init(&cpa_heap->mutex);

	cpa_heap->heap.ops = &ion_cpa_ops;
	cpa_heap->dev = data->priv;
	cpa_heap->heap.type = ION_HEAP_TYPE_CPA;
	cpa_heap->heap_size = data->size;
	dev = data->priv;

	ret = cpa_parse_dt(dev, data, cpa_heap);
	if (ret)
		goto free_heap;

	if (totalram_pages > 8 * TOTAL_RAM_PAGES_1G)
		orders[1] = 6;
	else if (totalram_pages > 4 * TOTAL_RAM_PAGES_1G)
		orders[1] = 5;
	else
		orders[1] = 4;

	if (ion_cpa_heap_create_pools(cpa_heap->pools))
		goto free_heap;

	cpa_heap->context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
	if (!cpa_heap->context)
		goto destroy_pools;
	cpa_heap->session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
	if (!cpa_heap->session)
		goto free_context;

	pr_err("cpa heap info %s:\n"
		  "\t\t\t\t heap id : %u\n"
		  "\t\t\t\t heap size : %lu MB\n",
		  data->name,
		  data->id,
		  cpa_heap->heap_size / SZ_1M);

	cpa_work_data->cpa_heap = cpa_heap;
	cpa_mem_work_data = cpa_work_data;

	return &cpa_heap->heap;

free_context:
	if (cpa_heap->context)
		kfree(cpa_heap->context);
destroy_pools:
	ion_cpa_heap_destroy_pools(cpa_heap->pools);
free_heap:
	mutex_destroy(&cpa_heap->mutex);
	kfree(cpa_work_data);
	kfree(cpa_heap);

	return ERR_PTR(-ENOMEM);/*lint !e747*/
}

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
void ion_cpa_heap_destroy(struct ion_heap *heap)
{
	struct ion_cpa_heap *cpa_heap = to_cpa_heap(heap);

	if (cpa_heap->TA_init) {
		secmem_tee_destroy(cpa_heap->context, cpa_heap->session);
		cpa_heap->TA_init = 0;
	}

	if (cpa_heap->context)
		kfree(cpa_heap->context);
	if (cpa_heap->session)
		kfree(cpa_heap->session);

	mutex_destroy(&cpa_heap->mutex);
	kfree(cpa_heap);
}
#endif

/**
 * /sys/module/ion_cpa_heap/parameters/cpa_debug_level
 */
module_param_named(cpa_debug_level, cpa_debug_level, int, 0640);
