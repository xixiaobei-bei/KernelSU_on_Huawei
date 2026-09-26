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
 * Provide external call interfaces of protect_lru
 */
#ifndef PROTECT_LRU_H
#define PROTECT_LRU_H

#ifdef CONFIG_TASK_PROTECT_LRU
#include <linux/sysctl.h>
#include <linux/mmzone.h>
#include <linux/page-flags.h>

extern struct ctl_table protect_lru_table[];
extern unsigned long protect_lru_enable __read_mostly;
extern unsigned long protect_reclaim_ratio __read_mostly;

void add_page_to_protect_lru_list(
	struct page *page, struct lruvec *lruvec, bool lru_head);
void del_page_from_protect_lru_list(
	struct page *page, struct lruvec *lruvec);
void protect_lru_set_from_process(struct page *page);
void protect_lru_set_from_file(struct page *page);
void protect_lruvec_init(struct lruvec *lruvec);
void shrink_protect_file(struct lruvec *lruvec, bool force);
struct page *shrink_protect_lru_by_overlimit(struct page *page);
void shrink_protect_lru_by_overratio(struct pglist_data *pgdat);

static inline bool is_valid_protect_level(int num)
{
	return (num > 0) && (num <= PROTECT_LEVELS_MAX);
}

static inline bool check_file_page(struct page *page)
{
	/*
	 * When do page migration, the mapping of file page maybe
	 * set to null, however it still need to be charged.
	 */

	/* Skip anon page and mlock page */
	if (PageSwapBacked(page) || PageUnevictable(page))
		return false;

	return true;
}
#elif defined(CONFIG_MEMCG_PROTECT_LRU)
#include <linux/sysctl.h>
#include <linux/memcontrol.h>
#include <linux/fs.h>

extern struct ctl_table protect_lru_table[];
extern unsigned long protect_lru_enable __read_mostly;

struct mem_cgroup *get_protect_memcg(
	struct page *page, struct mem_cgroup **memcgp);
struct mem_cgroup *get_protect_file_memcg(
	struct page *page, struct address_space *mapping);
unsigned long get_protected_pages(void);
int protect_memcg_css_online(
	struct cgroup_subsys_state *css, struct mem_cgroup *memcg);
void protect_memcg_css_offline(struct mem_cgroup *memcg);
void shrink_prot_memcg_by_overlimit(struct mem_cgroup *memcg);
void shrink_prot_memcg_by_overratio(void);
int shrink_prot_memcg(struct mem_cgroup *memcg);
int is_prot_memcg(struct mem_cgroup *memcg, bool boot);

static inline bool is_valid_protect_level(int num)
{
	return (num > 0) && (num <= PROTECT_LEVELS_MAX);
}

static inline void protect_add_page_cache_rollback(struct page *page)
{
	if (PageProtect(page)) {
		ClearPageProtect(page);
		page->mem_cgroup = NULL;
	}
}
#endif

#endif
