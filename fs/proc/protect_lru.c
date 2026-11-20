/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Protect lru of task support. It's between normal lru and mlock,
 * that means we will reclaim protect lru pages as late as possible.
 */
#include <linux/protect_lru.h>

#if defined(CONFIG_TASK_PROTECT_LRU) || defined(CONFIG_MEMCG_PROTECT_LRU)
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/xattr.h>
#include <linux/swap.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/mm_inline.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#include "internal.h"
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
#include "../internal.h"
#endif
#include <securec.h>

#define MAX_BUFFER_LEN          256
#define MAX_LEN                 (10 + MAX_BUFFER_LEN)
#define DECIMAL_BASE			10
#define PAGES_PER_MBYTES_SHIFT	(20 - PAGE_SHIFT)
#define	PERCENT_DENOMINATOR		100

enum Protect_Data_Index {
	PROTECT_FD,
	PROTECT_LEVEL,
	PROTECT_MAX,
};

static int sysctl_zero;
static int sysctl_one = 1;
static int sysctl_one_hundred = 100;
static struct mutex handler_mutex;
static unsigned long protect_max_mbytes[PROTECT_LEVELS_MAX];
static unsigned long protect_cur_mbytes[PROTECT_LEVELS_MAX];

unsigned long protect_lru_enable __read_mostly = 1;
unsigned long protect_reclaim_ratio __read_mostly = 50;
#endif

#ifdef CONFIG_TASK_PROTECT_LRU
static inline bool check_protect_page(struct page *page)
{
	/* Skip non-lru page */
	if (!PageLRU(page))
		return false;

	return check_file_page(page);
}

static bool is_protect_lru_overlimit(struct lruvec *lruvec)
{
	int i;
	unsigned long cur;
	unsigned long max;

	/* Skip the last head, because it is non-prot head */
	for (i = 0; i < PROTECT_HEAD_END; i++) {
		cur = lruvec->heads[i].pages;
		max = lruvec->heads[i].max_pages;
		if (cur && cur > max)
			return true;
	}

	return false;
}

static bool is_protect_lru_overratio(struct pglist_data *pgdat)
{
	int i;
	unsigned long prot_file = 0;
	unsigned long all_file = 0;

	/* Skip the last head, because it is non-prot head */
	for (i = 0; i < MAX_NR_ZONES; i++) {
		struct zone *zone = pgdat->node_zones + i;

		prot_file += zone_page_state(zone, NR_PROTECT_ACTIVE_FILE) +
			zone_page_state(zone, NR_PROTECT_INACTIVE_FILE);
		all_file += zone_page_state(zone, NR_ZONE_ACTIVE_FILE) +
			zone_page_state(zone, NR_ZONE_INACTIVE_FILE);
	}

	if (protect_reclaim_ratio &&
		(prot_file * PERCENT_DENOMINATOR >
		protect_reclaim_ratio * all_file))
		return true;
	else
		return false;
}

static int protect_switch_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;

	if (!mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	mutex_unlock(&handler_mutex);
	return ret;
}

static int protect_reclaim_ratio_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;
	unsigned long enable;
	unsigned long flags;
	struct pglist_data *pgdat = NULL;

	if (!mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (ret || !write)
		goto out;

	enable = protect_lru_enable;
	protect_lru_enable = 0;
	for_each_online_pgdat(pgdat) {
		while (is_protect_lru_overratio(pgdat)) {
			spin_lock_irqsave(&pgdat->lru_lock, flags);
			shrink_protect_file(node_lruvec(pgdat), true);
			spin_unlock_irqrestore(&pgdat->lru_lock, flags);
			cond_resched();
		}
	}
	protect_lru_enable = enable;

out:
	mutex_unlock(&handler_mutex);
	return ret;
}

static void set_protect_max_mbytes(void)
{
	int i;
	enum zone_type j;
	unsigned long node_pages;
	unsigned long prot_pages;
	struct pglist_data *pgdat = NULL;

	/* Skip the last head, because it is non-prot head */
	for (i = 0; i < PROTECT_HEAD_END; i++) {
		prot_pages = protect_max_mbytes[i] <<
			PAGES_PER_MBYTES_SHIFT;
		for_each_online_pgdat(pgdat) {
			node_pages = 0;
			for (j = 0; j < MAX_NR_ZONES; j++)
				node_pages +=
					pgdat->node_zones[j].managed_pages;
			node_lruvec(pgdat)->heads[i].max_pages = node_pages *
				prot_pages / totalram_pages;
		}
	}
}

static int protect_max_mbytes_handler(struct ctl_table *table,
	int write, void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;
	unsigned long enable;
	unsigned long flags;
	struct pglist_data *pgdat = NULL;

	if (!mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	ret = proc_doulongvec_minmax(table, write, buffer, length, ppos);
	if (ret || !write)
		goto out;

	enable = protect_lru_enable;
	protect_lru_enable = 0;
	set_protect_max_mbytes();
	for_each_online_pgdat(pgdat) {
		while (is_protect_lru_overlimit(node_lruvec(pgdat))) {
			spin_lock_irqsave(&pgdat->lru_lock, flags);
			shrink_protect_file(node_lruvec(pgdat), false);
			spin_unlock_irqrestore(&pgdat->lru_lock, flags);
			cond_resched();
		}
	}
	protect_lru_enable = enable;

out:
	mutex_unlock(&handler_mutex);
	return ret;
}

static int protect_cur_mbytes_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone = NULL;
	struct lruvec *lruvec = NULL;
	int i;

	if (!mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	/* Skip the last head, because it is non-prot head */
	for (i = 0; i < PROTECT_HEAD_END; i++) {
		protect_cur_mbytes[i] = 0;
		for_each_populated_zone(zone) {
			lruvec = &zone->zone_pgdat->lruvec;
			protect_cur_mbytes[i] = lruvec->heads[i].pages;
		}
		protect_cur_mbytes[i] >>= PAGES_PER_MBYTES_SHIFT;
	}
	mutex_unlock(&handler_mutex);

	return proc_doulongvec_minmax(table, write, buffer, length, ppos);
}

void protect_lru_set_from_process(struct page *page)
{
	struct mm_struct *mm = current->mm;

	if (protect_lru_enable && mm && page && check_file_page(page)) {
		struct zone *zone = page_zone(page);
		struct lruvec *lruvec = mem_cgroup_page_lruvec(page,
			zone->zone_pgdat);
		int num = mm->protect;

		if (is_valid_protect_level(num) &&
			lruvec->heads[num - 1].max_pages &&
			!PageProtect(page)) {
			SetPageActive(page);
			SetPageProtect(page);
			set_page_num(page, num);
		}
	}
}

void protect_lru_set_from_file(struct page *page)
{
	int num;

	if (!protect_lru_enable || !page)
		return;

	if (page->mapping && page->mapping->host)
		num = page->mapping->host->i_protect;
	else
		num = 0;
	/*
	 * Add the protect flag by file.
	 * Its priority is higher than process.
	 * We cannot change the prot level dynamically.
	 */
	if (unlikely(is_valid_protect_level(num) && check_file_page(page))) {
		struct zone *zone = page_zone(page);
		struct lruvec *lruvec = mem_cgroup_page_lruvec(page,
			zone->zone_pgdat);

		if (lruvec->heads[num - 1].max_pages) {
			SetPageActive(page);
			SetPageProtect(page);
			set_page_num(page, num);
		}
	}
}

void add_page_to_protect_lru_list(
	struct page *page, struct lruvec *lruvec, bool lru_head)
{
	enum lru_list lru;
	struct protect_head *head = NULL;

	if (!page || !lruvec || !check_protect_page(page))
		return;

	lru = page_lru(page);
	if (unlikely(PageProtect(page))) {
		int num = get_page_num(page);

		if (is_valid_protect_level(num)) {
			int nr_pages = hpage_nr_pages(page);

			if (lru_head) {
				head = lruvec->heads + num - 1;
				list_move(&page->lru,
					&head->protect_page[lru].lru);
			} else {
				head = lruvec->heads + num;
				list_move_tail(&page->lru,
					&head->protect_page[lru].lru);
			}
			lruvec->heads[num - 1].pages += nr_pages;
			__mod_zone_page_state(page_zone(page),
				NR_PROTECT_LRU_BASE + lru, nr_pages);
			return;
		}

		pr_emerg("The level of protected page is %d\n", num);
		ClearPageProtect(page);
		set_page_num(page, 0);
	}

	if (lru_head) {
		/* Move to the head for both normal and prot page */
		head = lruvec->heads + PROTECT_HEAD_END;
		list_move(&page->lru, &head->protect_page[lru].lru);
	} else {
		/* Move to the tail for normal page */
		list_move_tail(&page->lru, &lruvec->lists[lru]);
	}
}

void del_page_from_protect_lru_list(struct page *page, struct lruvec *lruvec)
{
	int num;

	if (!page || !lruvec || !check_protect_page(page))
		return;

	num = get_page_num(page);
	if (unlikely(PageProtect(page) && is_valid_protect_level(num))) {
		enum lru_list lru = page_lru(page);
		int nr_pages = hpage_nr_pages(page);

		lruvec->heads[num - 1].pages -= nr_pages;
		__mod_zone_page_state(page_zone(page),
			NR_PROTECT_LRU_BASE + lru, -nr_pages);
	}
}

struct page *shrink_protect_lru_by_overlimit(struct page *page)
{
	struct zone *zone = NULL;
	struct lruvec *lruvec = NULL;
	unsigned long flags;

	if (!protect_lru_enable || !page)
		return page;

	zone = page_zone(page);
	lruvec = mem_cgroup_page_lruvec(page, zone->zone_pgdat);
	if (is_protect_lru_overlimit(lruvec)) {
		spin_lock_irqsave(zone_lru_lock(zone), flags);
		shrink_protect_file(lruvec, false);
		spin_unlock_irqrestore(zone_lru_lock(zone), flags);
	}

	if (PageProtect(page))
		mark_page_accessed(page);

	return page;
}

void shrink_protect_lru_by_overratio(struct pglist_data *pgdat)
{
	unsigned long flags;

	if (!pgdat)
		return;

	/* If normal file is less than ratio, shrink protect file */
	if (is_protect_lru_overratio(pgdat)) {
		/*lint -save -e550 -e747*/
		spin_lock_irqsave(&pgdat->lru_lock, flags);
		shrink_protect_file(node_lruvec(pgdat), true);
		/*lint -restore*/
		spin_unlock_irqrestore(&pgdat->lru_lock, flags);
	}
}

void shrink_protect_file(struct lruvec *lruvec, bool force)
{
	int i;
	int count;
	unsigned long cur;
	unsigned long max;
	struct protect_head *prot = NULL;
	struct list_head *head = NULL;
	struct page *tail = NULL;

	if (!lruvec)
		return;

	/* Skip the last head, because it is non-prot head */
	for (i = 0; i < PROTECT_HEAD_END; i++) {
		cur = lruvec->heads[i].pages;
		max = lruvec->heads[i].max_pages;
		if (!cur)
			continue;
		if (!force && cur <= max)
			continue;

		/* Shrink the inactive list first */
		prot = lruvec->heads + i + 1;
		head = &prot->protect_page[LRU_INACTIVE_FILE].lru;
		tail = list_entry(head->prev, struct page, lru);
		if (PageReserved(tail))
			head = &prot->protect_page[LRU_ACTIVE_FILE].lru;

		/* Move 32 tail pages every time */
		count = SWAP_CLUSTER_MAX;
		while (count--) {
			tail = list_entry(head->prev, struct page, lru);
			if (PageReserved(tail))
				break;
			del_page_from_protect_lru_list(tail, lruvec);
			ClearPageProtect(tail);
			set_page_num(tail, 0);
			add_page_to_protect_lru_list(tail, lruvec, true);
		}
	}
}

void protect_lruvec_init(struct lruvec *lruvec)
{
	enum lru_list lru;
	int i;

	if (!lruvec)
		return;

	for (i = 0; i < PROTECT_HEAD_MAX; i++) {
		for_each_evictable_lru(lru) {
			struct page *page =
				&lruvec->heads[i].protect_page[lru];

			/*
			 * The address is not from direct mapping,
			 * so can't use __va()/__pa(), page_to_pfn()...
			 */
			init_page_count(page);
			page_mapcount_reset(page);
			SetPageReserved(page);
			SetPageLRU(page);
			INIT_LIST_HEAD(&page->lru);
			/* Only protect file list */
			if (lru >= LRU_INACTIVE_FILE)
				list_add_tail(&page->lru, &lruvec->lists[lru]);
		}
		lruvec->heads[i].max_pages = 0;
		lruvec->heads[i].pages = 0;
	}
}

#elif defined(CONFIG_MEMCG_PROTECT_LRU)
struct mem_cgroup *prot_mem_cgroup[PROTECT_LEVELS_MAX] = {0, 0, 0};

static struct cgroup_subsys_state *mem_cgroup_css(struct mem_cgroup *memcg)
{
	return &memcg->css;
}

/*lint -e548*/
static bool is_prot_memcg_overlimit(struct mem_cgroup *memcg)
{
	struct page_counter *counter = &memcg->memory;
	unsigned long val;
	unsigned long limit;

	val = page_counter_read(counter);

#if	(KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
	limit = counter->max;
#else
	limit = counter->limit;
#endif

	if (!limit)
		return false;

	if (val + SWAP_CLUSTER_MAX > limit)
		return true;

	return false;
}

static bool is_prot_memcg_overratio(void)
{
	unsigned long prot_file = 0;
	unsigned long all_file = 0;
	struct mem_cgroup *memcg = NULL;
	struct zone *zone = NULL;
	int i;

	for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
		if (!prot_mem_cgroup[i])
			continue;
		memcg = prot_mem_cgroup[i];
		prot_file += memcg_page_state(memcg, MEMCG_CACHE);
	}

	if (!prot_file)
		return false;

	for_each_populated_zone(zone)
		all_file += zone_page_state(zone, NR_ZONE_ACTIVE_FILE) +
			zone_page_state(zone, NR_ZONE_INACTIVE_FILE);

	if (!protect_reclaim_ratio ||
		(prot_file * PERCENT_DENOMINATOR >
		protect_reclaim_ratio * all_file))
		return true;

	return false;
}

static int move_prot_memcg_page(
	struct mem_cgroup *from, struct mem_cgroup *to, struct page *page)
{
	struct pglist_data *pnode = NULL;
	struct lruvec *lruvec = NULL;
	int lru;
	int ret = -EBUSY;

	/* Skip if it is a non lru page. */
	if (!PageLRU(page))
		return ret;

	/* Skip if it is a free page. */
	if (!get_page_unless_zero(page))
		return ret;

	/* Isolate form lru list. */
	pnode = page_pgdat(page);
	lruvec = mem_cgroup_page_lruvec(page, pnode);
	lru = page_lru(page);

	ClearPageLRU(page);
	del_page_from_lru_list(page, lruvec, lru);

	/* Move to a new memcg. */
	if (!protect_memcg_move_account(page, 0, from, to)) {
		ret = 0;
		WARN_ON(page->mem_cgroup != root_mem_cgroup);
		ClearPageProtect(page);
	}

	if (PageProtect(page))
		WARN_ON(!is_prot_memcg(page->mem_cgroup, false));

	lruvec = mem_cgroup_page_lruvec(page, pnode);
	add_page_to_lru_list(page, lruvec, lru);
	SetPageLRU(page);
	put_page(page);

	return ret;
}

static int protect_switch_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;

	if (mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	mutex_unlock(&handler_mutex);
	return ret;
}

static int protect_reclaim_ratio_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;
	int i;
	unsigned long enable;

	if (mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (ret || !write)
		goto out;

	enable = protect_lru_enable;
	protect_lru_enable = 0;
	/* Shrink until reach the protect file ratio. */
	while (is_prot_memcg_overratio()) {
		lru_add_drain_all();
		for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
			if (prot_mem_cgroup[i])
				shrink_prot_memcg(prot_mem_cgroup[i]);
		}
		cond_resched();
	}
	protect_lru_enable = enable;
out:
	mutex_unlock(&handler_mutex);
	return ret;
}

static int protect_max_mbytes_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;
	int i;
	unsigned long enable;

	if (mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	ret = proc_doulongvec_minmax(table, write, buffer, length, ppos);

	if (ret || !write)
		goto out;

	enable = protect_lru_enable;
	protect_lru_enable = 0;
	for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
		if (!prot_mem_cgroup[i])
			continue;

		protect_memcg_drain_all_stock(prot_mem_cgroup[i]);
		lru_add_drain_all();
		ret = protect_memcg_resize_limit(prot_mem_cgroup[i],
			protect_max_mbytes[i] << PAGES_PER_MBYTES_SHIFT);

		if (ret)
			break;
	}

	for (; i < PROTECT_LEVELS_MAX; i++)
#if	(KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
		protect_max_mbytes[i] = prot_mem_cgroup[i]->memory.max >>
			PAGES_PER_MBYTES_SHIFT;
#else
		protect_max_mbytes[i] = prot_mem_cgroup[i]->memory.limit >>
			PAGES_PER_MBYTES_SHIFT;
#endif

	protect_lru_enable = enable;
out:
	mutex_unlock(&handler_mutex);
	return ret;
}

static int protect_cur_mbytes_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int i;

	if (mem_cgroup_disabled())
		return -EINVAL;

	mutex_lock(&handler_mutex);
	for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
		if (prot_mem_cgroup[i]) {
			protect_cur_mbytes[i] = protect_memcg_usage(
				prot_mem_cgroup[i], false);
			protect_cur_mbytes[i] >>= PAGES_PER_MBYTES_SHIFT;
		} else {
			protect_cur_mbytes[i] = 0;
		}
	}
	mutex_unlock(&handler_mutex);

	return proc_doulongvec_minmax(table, write, buffer, length, ppos);
}

/*
 * Return 0 for non protect memcg, return num for protect memcg.
 */
int is_prot_memcg(struct mem_cgroup *memcg, bool boot)
{
	char memcg_name[NAME_MAX + 1];
	char prot_memcg_name[NAME_MAX + 1];
	int i;

	if (!memcg)
		return 0;

	if (mem_cgroup_disabled())
		return 0;

	if (boot) {
		struct cgroup_subsys_state *css = mem_cgroup_css(memcg);

		cgroup_name(css->cgroup, memcg_name, sizeof(memcg_name));
		for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
			if (sprintf_s(prot_memcg_name, NAME_MAX,
				"protect_lru_%d", i + 1) == -1)
				return 0;
			if (!strcmp(memcg_name, prot_memcg_name))
				return i + 1;
		}
	} else {
		for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
			if (prot_mem_cgroup[i] && memcg == prot_mem_cgroup[i])
				return i + 1;
		}
	}

	return 0;
}

int shrink_prot_memcg(struct mem_cgroup *memcg)
{
	struct lruvec *lruvec = NULL;
	struct list_head *head = NULL;
	struct page *page = NULL;
	unsigned long scan;
	unsigned long flags;
	int nid;
	int moved = 0;

	if (!memcg)
		return moved;
	/*
	 * A memcg includes at least one lru list, so we may shrink the lru
	 * which have added pages just now.
	 */

	for_each_node_state(nid, N_MEMORY) {
		spin_lock_irqsave(&NODE_DATA(nid)->lru_lock, flags);
		lruvec = mem_cgroup_lruvec(NODE_DATA(nid), memcg);
		head = &lruvec->lists[LRU_INACTIVE_FILE];
		/* Shrink inactive list first. */
		if (list_empty(head))
			head = &lruvec->lists[LRU_ACTIVE_FILE];

		if (list_empty(head))
			head = &lruvec->lists[LRU_UNEVICTABLE];

		/*
		 * As move to the root memcg, so there is
		 * no need to call precharge.
		 */
		for (scan = 0; scan < SWAP_CLUSTER_MAX &&
					!list_empty(head); scan++) {
			page = list_entry(head->prev, struct page, lru);
			if (!move_prot_memcg_page(memcg, root_mem_cgroup,
				page))
				moved++;
		}

		/*
		 * We can not sched here, because it
		 * may be called from atomic opt.
		 */
		spin_unlock_irqrestore(&NODE_DATA(nid)->lru_lock, flags);
	}

	protect_memcg_cancel_charge(memcg, moved);

	return moved;
}

void shrink_prot_memcg_by_overlimit(struct mem_cgroup *memcg)
{
	if (mem_cgroup_disabled())
		return;

	if (!is_prot_memcg(memcg, false))
		return;

	while (is_prot_memcg_overlimit(memcg))
		shrink_prot_memcg(memcg);
}

void shrink_prot_memcg_by_overratio(void)
{
	struct mem_cgroup *memcg = NULL;
	int i;

	if (mem_cgroup_disabled())
		return;

	while (is_prot_memcg_overratio()) {
		for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
			if (!prot_mem_cgroup[i])
				continue;
			memcg = prot_mem_cgroup[i];
			shrink_prot_memcg(memcg);
		}
	}
}

int protect_memcg_css_online(struct cgroup_subsys_state *css,
	struct mem_cgroup *memcg)
{
	int num;
	struct mem_cgroup *parent = NULL;

	if (!css || !memcg)
		return -EINVAL;

	parent = mem_cgroup_from_css(css->parent);
	/* Do not create a child memcg under protect memcg. */
	if (is_prot_memcg(parent, true))
		return -EINVAL;

	/* Protect memcg could only be created under the root memcg. */
	num = is_prot_memcg(memcg, true);
	if (!is_valid_protect_level(num))
		return 0;

	if (parent != root_mem_cgroup)
		return -EINVAL;

	/* Init protect memcg ptr. */
	WARN_ON(prot_mem_cgroup[num - 1]);
	prot_mem_cgroup[num - 1] = memcg; /*lint !e676*/
	pr_info("protect_lru: online memcg,num=%d\n", num);
	return 0;
}

void protect_memcg_css_offline(struct mem_cgroup *memcg)
{
	int num;
	unsigned long prot_file;

	if (!memcg)
		return;

	num = is_prot_memcg(memcg, false);
	if (is_valid_protect_level(num)) {
		/* Empty the protect memcg */
		do {
			lru_add_drain_all();
			shrink_prot_memcg(memcg);
			prot_file = memcg_page_state(memcg, MEMCG_CACHE);
			cond_resched();
		} while (prot_file);

		WARN_ON(!prot_mem_cgroup[num - 1]);
		prot_mem_cgroup[num - 1] = NULL; /*lint !e676*/
		pr_info("protect_lru: offline memcg,num=%d\n", num);
	}
}

struct mem_cgroup *get_protect_memcg(
	struct page *page, struct mem_cgroup **memcgp)
{
	struct mem_cgroup *memcg = NULL;

	if (!page || !memcgp || !PageProtect(page))
		return memcg;

	WARN_ON(PageSwapCache(page));
	WARN_ON(page->mem_cgroup);

	rcu_read_lock();
	memcg = *memcgp;
	if (memcg && !css_tryget_online(&memcg->css)) {
		ClearPageProtect(page);
		memcg = NULL;
	}

#if	(KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
	if (is_prot_memcg(memcg, false) && !memcg->memory.max) {
#else
	if (is_prot_memcg(memcg, false) && !memcg->memory.limit) {
#endif
		ClearPageProtect(page);
		memcg = NULL;
	}
	rcu_read_unlock();

	return memcg;
}

#ifdef CONFIG_MEMCG_FINE_GRANULARITY_PROTECT_LRU
static int in_protect_region(struct inode *inode, struct page *page)
{
	unsigned long start;
	unsigned long end;
	struct list_head *pos;
	struct protect_region *region;
	int ret = 0;

	if (!inode || !page)
		return 0;

	mutex_lock(&handler_mutex);
	if (list_empty(&inode->i_protect_head)) {
		/* compatible with MEMCG_PROTECT_LRU */
		ret = 1;
		goto out;
	}

	if (inode->last_region) {
		start = inode->last_region->offset;
		end = start + inode->last_region->length;
		if (page->index >= start && page->index < end) {
			ret = 1;
			goto out;
		}
	}

	list_for_each(pos, &inode->i_protect_head) {
		region = list_entry(pos, struct protect_region, list);
		start = region->offset;
		end = start + region->length;
		if (page->index >= start && page->index < end) {
			inode->last_region = region;
			ret = 1;
			break;
		}

		if (page->index < start) {
			inode->last_region = NULL;
			break;
		}
	}
out:
	mutex_unlock(&handler_mutex);
	return ret;
}
#endif

struct mem_cgroup *get_protect_file_memcg(
	struct page *page, struct address_space *mapping)
{
	struct mem_cgroup *memcg = NULL;
	struct inode *inode = NULL;
	int num;

	if (!page || !mapping || !protect_lru_enable)
		return memcg;

	inode = mapping->host;
	if (inode)
		num = inode->i_protect;
	else
		num = 0;

	if (unlikely(is_valid_protect_level(num)) &&
		prot_mem_cgroup[num - 1]) {
		WARN_ON(page->mem_cgroup);
#ifdef CONFIG_MEMCG_FINE_GRANULARITY_PROTECT_LRU
		if (in_protect_region(inode, page)) {
			SetPageProtect(page);
			memcg = prot_mem_cgroup[num - 1];
		}
#else
		SetPageProtect(page);
		memcg = prot_mem_cgroup[num - 1];
#endif
	}

	return memcg;
}

unsigned long get_protected_pages(void)
{
	unsigned long nr_pages = 0;
	int i;

	if (mem_cgroup_disabled())
		return nr_pages;

	for (i = 0; i < PROTECT_LEVELS_MAX; i++) {
		if (prot_mem_cgroup[i])
			nr_pages += protect_memcg_usage(
				prot_mem_cgroup[i], false);
	}

	return nr_pages;
}
/*lint +e548*/
#endif

#if defined(CONFIG_TASK_PROTECT_LRU) || defined(CONFIG_MEMCG_PROTECT_LRU)
/*lint -e545*/
struct ctl_table protect_lru_table[] = {
	{
		.procname	= "protect_max_mbytes",
		.data		= &protect_max_mbytes,
		.maxlen		= sizeof(protect_max_mbytes),
		.mode		= 0640,
		.proc_handler	= protect_max_mbytes_handler,
		.extra1         = &sysctl_zero,
	},
	{
		.procname	= "protect_cur_mbytes",
		.data		= &protect_cur_mbytes,
		.maxlen		= sizeof(protect_cur_mbytes),
		.mode		= 0440,
		.proc_handler	= protect_cur_mbytes_handler,
	},
	{
		.procname	= "protect_lru_enable",
		.data		= &protect_lru_enable,
		.maxlen		= sizeof(int),
		.mode		= 0640,
		.proc_handler	= protect_switch_handler,
		.extra1		= &sysctl_zero,
		.extra2		= &sysctl_one,
	},
	{
		.procname	= "protect_reclaim_ratio",
		.data		= &protect_reclaim_ratio,
		.maxlen		= sizeof(int),
		.mode		= 0640,
		.proc_handler	= protect_reclaim_ratio_handler,
		.extra1		= &sysctl_one,
		.extra2		= &sysctl_one_hundred,
	},
	{ }
};
/*lint +e545*/

static int protect_lru_set(struct file *filp, int level)
{
	struct inode *inode = file_inode(filp);
#if defined(CONFIG_OVERLAY_FS) && (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
	struct file *real_file = NULL;
#endif

	if (!inode)
		return -EINVAL;

	inode->i_protect = level;
#if defined(CONFIG_OVERLAY_FS) && (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
	real_file = get_real_file(filp);
	inode = file_inode(real_file);
	if (inode)
		inode->i_protect = level;
#endif
	pr_info("prot_lru: %s, %d\n", filp->f_path.dentry->d_name.name, level);

	return 0;
}

static int parse_protect_data(char *start, int *fd, int *level)
{
	int index = 0;
	char *p = start;
	char *str_level = NULL;

	while ((*p) != '\0') {
		if (*p != ' ') {
			p++;
			continue;
		}

		if (index == PROTECT_FD) {
			index++;
			*p = '\0';
			p++;
			while (*p == ' ')
				p++;
			str_level = p;
		} else if (index == PROTECT_LEVEL) {
			index++;
			*p = '\0';
			p++;
			while (*p == ' ')
				p++;
			break;
		}
	}

	if ((index != PROTECT_MAX) || kstrtoint(start, DECIMAL_BASE, fd) ||
		kstrtoint(str_level, DECIMAL_BASE, level))
		return -EINVAL;
	else
		return 0;
}

#ifdef CONFIG_MEMCG_FINE_GRANULARITY_PROTECT_LRU
enum PROTECT_REGION {
	PROTECT_REGION_FD,
	PROTECT_REGION_OFFSET,
	PROTECT_REGION_LENGTH,
	PROTECT_REGION_LEVEL,
};

static int parse_protect_region(char *start, int *fd,
				unsigned long *offset,
				unsigned long *len,
				int *level)
{
	int index = 0;
	char *p = start;
	char *str_fd = start;
	char *str_offset = NULL;
	char *str_len = NULL;
	char *str_level = NULL;

	while ((*p) != '\0') {
		if (*p != ' ') {
			p++;
			continue;
		}

		if (index == PROTECT_REGION_FD) {
			index++;
			*p = '\0';
			p++;
			while (*p == ' ')
				p++;
			str_offset = p;
		} else if (index == PROTECT_REGION_OFFSET) {
			index++;
			*p = '\0';
			p++;
			while (*p == ' ')
				p++;
			str_len = p;
		} else if (index == PROTECT_REGION_LENGTH) {
			index++;
			*p = '\0';
			p++;
			while(*p == ' ')
				p++;
			str_level = p;
		} else if (index == PROTECT_REGION_LEVEL) {
			index++;
			*p = '\0';
			break;
		}
	}

	if (!str_fd || !str_offset || !str_len || !str_level)
		return -EINVAL;

	if (kstrtoint(str_fd, DECIMAL_BASE, fd) ||
			kstrtoul(str_offset, DECIMAL_BASE, offset) ||
			kstrtoul(str_len, DECIMAL_BASE, len) ||
			kstrtoint(str_level, DECIMAL_BASE, level))
		return -EINVAL;
	else
		return 0;
}
#endif

/*
 * Format: 'fd level path'
 */
static ssize_t protect_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *ppos)
{
	char proctectData[MAX_LEN] = {0};
	char *start = NULL;
	int ret;
	int fd = -1;
	int level = 0;
	struct fd f;

	if ((count > sizeof(proctectData) - 1) || (count <= 0))
		return -EINVAL;

	if (copy_from_user(proctectData, buffer, count))
		return -EFAULT;

	proctectData[count] = '\0';
	start = strstrip(proctectData);
	if (strlen(start) <= 0)
		return -EINVAL;

	ret = parse_protect_data(start, &fd, &level);
	if (ret)
		return ret;

	f = fdget(fd);
	if (!f.file)
		return -EBADF;

	if (level >= 0 && level <= PROTECT_LEVELS_MAX) {
		ret = protect_lru_set(f.file, level);
	} else {
		pr_err("set protect lru: level is not right\n");
		ret = -EINVAL;
	}
	fdput(f);

	return ret ? ret : count;
}

#ifdef CONFIG_MEMCG_FINE_GRANULARITY_PROTECT_LRU
static void __protect_region_add(struct inode *inode,
		struct protect_region *region)
{
	struct list_head *entry = &inode->i_protect_head;
	struct protect_region *tmp = NULL;

	mutex_lock(&handler_mutex);
	list_for_each_entry(tmp, &inode->i_protect_head, list) {
		if (tmp->offset > region->offset)
			break;

		entry = &tmp->list;
	}

	list_add(&region->list, entry);
	mutex_unlock(&handler_mutex);
}

static int protect_region_add(struct file *filp, unsigned long offset,
		unsigned long length, int level)
{
	struct protect_region *i_protect_region = NULL;
	struct inode *inode = NULL;
	pgoff_t max_off;

	inode = file_inode(filp);
	if (unlikely(!inode))
		return  -EINVAL;

	max_off = DIV_ROUND_UP(i_size_read(inode), PAGE_SIZE);
	if (unlikely(offset >= max_off))
		return -EINVAL;

	i_protect_region = kvzalloc(sizeof(struct protect_region), GFP_KERNEL);
	if (unlikely(!i_protect_region))
		return -ENOMEM;

	inode->i_protect = level;
	i_protect_region->offset = offset;
	i_protect_region->length = length;
	__protect_region_add(inode, i_protect_region);

	pr_info("[protect_lru] set region : %s, %d, %d\n",
			filp->f_path.dentry->d_name.name, offset, length);
	return 0;
}

static ssize_t proc_add_protect_region(struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	char proctectData[MAX_LEN] = {0};
	char *start = NULL;
	int ret;
	int fd;
	unsigned long offset;
	unsigned long length;
	int level;
	struct fd f;

	if ((count > sizeof(proctectData) - 1) || (count <= 0))
		return -EINVAL;

	if (copy_from_user(proctectData, buffer, count))
		return -EFAULT;

	proctectData[count] = '\0';
	start = strstrip(proctectData);

	if (strlen(start) <= 0)
		return -EINVAL;

	ret = parse_protect_region(start, &fd, &offset, &length, &level);
	if (ret)
		return ret;

	f = fdget(fd);
	if (!f.file)
		return -EBADF;

	if (level <= 0 || level > PROTECT_LEVELS_MAX)
		return -EINVAL;

	ret = protect_region_add(f.file, offset, length, level);

	return ret ? ret : count;
}

static int parse_clear_data(char *start, int *fd)
{
	int index = 0;
	char *p = start;

	if (!start)
		return -EINVAL;

	while ((*p) != '\0') {
		if (*p != ' ') {
			p++;
			continue;
		}

		if (index == PROTECT_FD) {
			index++;
			*p = '\0';
			break;
		}
	}

	if (kstrtoint(start, DECIMAL_BASE, fd))
		return -EINVAL;
	else
		return 0;
}


void shrink_protect_lru_by_inode(struct inode *inode)
{
	struct lruvec *lruvec = NULL;
	struct list_head *head = NULL;
	struct page *page = NULL;
	struct page *next = NULL;
	int nid;
	unsigned long flags;
	struct mem_cgroup *memcg = NULL;
	int lru;
	int moved = 0;
	int ret;

	lru_add_drain_all();
	memcg = prot_mem_cgroup[inode->i_protect - 1];

	for_each_node_state(nid, N_MEMORY) {
		spin_lock_irqsave(&NODE_DATA(nid)->lru_lock, flags);
		lruvec = mem_cgroup_lruvec(NODE_DATA(nid), memcg);
		for (lru = LRU_INACTIVE_FILE; lru < NR_LRU_LISTS; lru++) {
			head = &lruvec->lists[lru];
			if (list_empty(head))
				continue;

			list_for_each_entry_safe(page, next, head, lru) {
				if (!page->mapping)
					continue;

				if (page->mapping->host != inode)
					continue;

				ret = move_prot_memcg_page(memcg,
							   root_mem_cgroup,
							   page);
				if (!ret)
					moved++;
			}
		}
		spin_unlock_irqrestore(&NODE_DATA(nid)->lru_lock, flags);
	}

	if (!moved)
		return;

	protect_memcg_cancel_charge(memcg, moved);
}

static int protect_region_clear(struct file *filp)
{
	struct inode *inode = NULL;
	struct protect_region *tmp = NULL;
	struct protect_region *next = NULL;
	unsigned long enable;

	inode = file_inode(filp);
	if (unlikely(!inode))
		return -EINVAL;

	if (unlikely(inode->i_protect <= 0 ||
				inode->i_protect > PROTECT_LEVELS_MAX))
		return -EINVAL;

	mutex_lock(&handler_mutex);

	inode->last_region = NULL;
	list_for_each_entry_safe(tmp, next, &inode->i_protect_head, list) {
		list_del(&tmp->list);
		kvfree(tmp);
	}

	enable = protect_lru_enable;
	protect_lru_enable = 0;

	if (!prot_mem_cgroup[inode->i_protect - 1])
		goto out;

	shrink_protect_lru_by_inode(inode);
out:
	inode->i_protect = 0;
	protect_lru_enable = enable;
	mutex_unlock(&handler_mutex);

	return 0;
}

static ssize_t proc_clear_protect_region(struct file *file,
		const char __user *buffer, size_t count, loff_t *pos)
{
	char proctectData[MAX_LEN] = {0};
	char *start = NULL;
	int ret;
	int fd;
	struct fd f;

	if ((count > sizeof(proctectData) - 1) || (count <= 0))
		return -EINVAL;

	if (copy_from_user(proctectData, buffer, count))
		return -EFAULT;

	proctectData[count] = '\0';
	start = strstrip(proctectData);

	if (strlen(start) <= 0)
		return -EINVAL;

	ret = parse_clear_data(start, &fd);
	if (ret)
		return ret;

	f = fdget(fd);
	if (!f.file)
		return -EBADF;

	ret = protect_region_clear(f.file);

	return ret ? ret : count;
}
#endif

static const struct file_operations protect_proc_fops = {
	.write  = protect_write,
	.llseek = noop_llseek,
};

#ifdef CONFIG_MEMCG_FINE_GRANULARITY_PROTECT_LRU
static const struct file_operations add_protect_region_fops = {
	.write  = proc_add_protect_region,
	.llseek = noop_llseek,
};

static const struct file_operations clear_protect_region_fops = {
	.write  = proc_clear_protect_region,
	.llseek = noop_llseek,
};
#endif

static int __init proc_protect_init(void)
{
	mutex_init(&handler_mutex);
	proc_create("protect_lru", 0220, NULL, &protect_proc_fops);
#ifdef CONFIG_MEMCG_FINE_GRANULARITY_PROTECT_LRU
	proc_create("add_protect_lru_region", 0220,
			NULL, &add_protect_region_fops);

	proc_create("clear_protect_lru_region", 0220,
			NULL, &clear_protect_region_fops);
#endif
	return 0;
}

fs_initcall(proc_protect_init);
#endif
