/*
 * cgroup_workingset_preread.c
 *
 * The preread part of control group workingset subsystem
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
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
 */
#include "cgroup_workingset_internal.h"

#include <linux/blkdev.h>
#include <linux/memcontrol.h>
#include <linux/mm.h>
#include <linux/mm_inline.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/syscalls.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
long __sys_setresuid(uid_t ruid, uid_t euid, uid_t suid);

#define GFP_ALLOC_PAGES		(__GFP_RECLAIM | __GFP_IO)
#define GFP_ADD_PAGE_CACHE	(GFP_NOFS)
#else
#define GFP_ALLOC_PAGES		(__GFP_RECLAIM | __GFP_IO | __GFP_COLD)
#define GFP_ADD_PAGE_CACHE	(__GFP_COLD | GFP_NOFS)
#endif
/* Use to interrupt prereading process. */
static atomic_t g_preread_abort = ATOMIC_INIT(0);
static enum ws_self_read_state g_selfread_state = E_SELFREAD_NONE;

static const char * const g_selfread_dir[] = {
	"/data/data/",
	"/data/user_de/",
	NULL
};

void workingset_preread_qos_wsrcrdlocked(
	struct s_workingset *ws, struct s_ws_record *record)
{
	bool need_dirty = false;

	if (!record->leader_blkio_cnt &&
	    (record->state & E_RECORD_STATE_UPDATE_BASE_BLKIO) &&
	    ws->leader_blkio_cnt) {
		record->leader_blkio_cnt = ws->leader_blkio_cnt;
		need_dirty = true;
		ws_dbg("%s, preread first blkio count = %u\n",
			__func__, ws->leader_blkio_cnt);
	} else if (record->leader_blkio_cnt &&
	    (ws->leader_blkio_cnt >= CARE_BLKIO_MIN_THRESHOLD) &&
	    (record->leader_blkio_cnt * BLKIO_MULTIPLE_FOR_UPDATE <
	    ws->leader_blkio_cnt)) {
		record->need_update = 1;
		need_dirty = true;
		ws_dbg("%s, base blkio=%u,current blkio=%u\n", __func__,
			record->leader_blkio_cnt, ws->leader_blkio_cnt);
	} else if (ws->leader_blkio_cnt > (record->leader_blkio_cnt +
	    (record->data.pageseq_cnt *
	    BLKIO_PERCENTAGE_THRESHOLD_FOR_UPDATE / ONE_HUNDRED))) {
		if (!(record->state & E_RECORD_STATE_UPDATE_BASE_BLKIO) &&
		    !record->leader_blkio_cnt) {
			record->leader_blkio_cnt = ws->leader_blkio_cnt;
			ws_dbg("%s, preread base blkio count = %u\n",
				__func__, ws->leader_blkio_cnt);
		} else {
			record->need_update = 1;
			ws_dbg("%s, blkio=%u, pages = %u\n",
				__func__, ws->leader_blkio_cnt,
				record->data.pageseq_cnt);
		}
		need_dirty = true;
	}

	if (need_dirty && !(record->state & E_RECORD_STATE_DIRTY))
		record->state |= E_RECORD_STATE_DIRTY |
		    E_RECORD_STATE_UPDATE_HEADER_ONLY;
	ws->leader_blkio_cnt = 0;
	ws->leader_blkio_base = 0;
}

/*
 * workingset_page_cache_read -
 * Adds requested page to the page cache if not already there.
 * @file:	file to read
 * @offset:	page index
 *
 * This adds the requested page to the page cache if it isn't already there,
 * and schedules an I/O to read in its contents from disk.
 */
static int workingset_page_cache_read(struct s_readpages_control *rpc)
{
	struct address_space *mapping = rpc->mapping;
	loff_t isize = i_size_read(mapping->host);
	struct page *page = NULL;
	int ret;

	if (!isize || (rpc->offset > ((isize - 1) >> PAGE_SHIFT)) ||
	    !mapping->a_ops->readpage)
		return -EINVAL;

	do {
		page = alloc_pages((mapping_gfp_mask(mapping) & ~__GFP_FS) |
		    GFP_ALLOC_PAGES, 0);
		if (!page) {
			pr_err("%s: out of memory!\n", __func__);
			return -ENOMEM;
		}

		ret = add_to_page_cache_lru(page, mapping, rpc->offset,
		    GFP_NOFS);
		if (ret == 0)
			ret = mapping->a_ops->readpage(rpc->filp, page);
		else if (ret == -EEXIST)
			ret = 0; /* Losing race to add is OK */

#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
		put_page(page);
#else
		page_cache_release(page);
#endif
	} while (ret == AOP_TRUNCATED_PAGE);

	return ret;
}

#ifdef CONFIG_TASK_PROTECT_LRU
static inline struct list_head *get_protect_head_lru(
	struct lruvec *lruvec, struct page *page)
{
	enum lru_list lru = page_lru(page);

	return &lruvec->heads[PROTECT_HEAD_END].protect_page[lru].lru;
}
#endif

/*
 * Move inactive page to head of the lru list.
 * @page:	page to move
 */
static bool workingset_adjust_page_lru(struct page *page)
{
	bool adjusted = false;

	if (!PageUnevictable(page) &&
#ifdef CONFIG_TASK_PROTECT_LRU
	    !PageProtect(page) &&
#endif
	    !PageActive(page)) {
		if (PageLRU(page)) {
			struct lruvec *lruvec = NULL;
			struct zone *zone = page_zone(page);

#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
			spin_lock_irq(zone_lru_lock(zone));
			lruvec = mem_cgroup_page_lruvec(page,
			    zone->zone_pgdat);
#else
			spin_lock_irq(&zone->lru_lock);
			lruvec = mem_cgroup_page_lruvec(page, zone);
#endif
#ifdef CONFIG_TASK_PROTECT_LRU
			if (PageLRU(page) && !PageProtect(page) &&
			    !PageSwapBacked(page) &&
			    !PageUnevictable(page)) {
				struct list_head *head;

				head = get_protect_head_lru(lruvec, page);
				list_move(&page->lru, head);
				adjusted = true;
			}
#else
			if (PageLRU(page) && !PageSwapBacked(page) &&
			    !PageUnevictable(page)) {
				list_move(&page->lru,
					&lruvec->lists[page_lru(page)]);
				adjusted = true;
			}
#endif
#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
			spin_unlock_irq(zone_lru_lock(zone));
#else
			spin_unlock_irq(&zone->lru_lock);
#endif
		} else {
			mark_page_accessed(page);
			adjusted = true;
		}
	}
	return adjusted;
}

/*
 * Read contiguous filepage from disk.
 */
static int workingset_read_pages(
	struct address_space *mapping, struct file *filp,
	struct list_head *pages, unsigned int nr_pages)
{
	struct blk_plug plug;
	unsigned int page_idx;
	int ret;

	blk_start_plug(&plug);

	if (mapping->a_ops->readpages) {
		ret = mapping->a_ops->readpages(filp, mapping,
		    pages, nr_pages);
		/* Clean up the remaining pages */
		put_pages_list(pages);
		goto out;
	}

	for (page_idx = 0; page_idx < nr_pages; page_idx++) {
		struct page *page = list_entry((pages)->prev,
		    struct page, lru);
		list_del(&page->lru);
		if (mapping->a_ops->readpage &&
		    !add_to_page_cache_lru(page, mapping, page->index,
		    (mapping_gfp_mask(mapping) & ~__GFP_FS) |
		    GFP_ADD_PAGE_CACHE))
			mapping->a_ops->readpage(filp, page);
#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
		put_page(page);
#else
		page_cache_release(page);
#endif
	}
	ret = 0;

out:
	blk_finish_plug(&plug);

	return ret;
}

/*
 * Read contiguous filepage.
 */
/*lint -e548*/
static int workingset_page_cache_range_read(struct s_readpages_control *rpc)
{
	LIST_HEAD(page_pool);
	unsigned int page_idx;
	int ret = 0;
	struct address_space *mapping = rpc->mapping;
	struct page *page = NULL;
	unsigned long end_index; /* The last page we want to read */
	loff_t isize = i_size_read(mapping->host);

	rpc->nr_adjusted = 0;
	if (isize == 0)
		goto out;

#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
	end_index = ((isize - 1) >> PAGE_SHIFT);
#else
	end_index = ((isize - 1) >> PAGE_CACHE_SHIFT);
#endif

	/*
	 * Preallocate as many pages as we will need.
	 */
	for (page_idx = 0; page_idx < rpc->nr_to_read; page_idx++) {
		pgoff_t pageoffset = rpc->offset + page_idx;

		if (pageoffset > end_index)
			break;

		page = find_get_page(mapping, pageoffset);
		if (page) {
			if (workingset_adjust_page_lru(page))
				rpc->nr_adjusted++;
			put_page(page);
			continue;
		}

		page = alloc_pages((mapping_gfp_mask(mapping) & ~__GFP_FS) |
		    GFP_ALLOC_PAGES, 0);
		if (!page) {
			pr_err("%s: out of memory!\n", __func__);
			break;
		}
		page->index = pageoffset;
		list_add(&page->lru, &page_pool);
		ret++;
	}

	/*
	 * Now start the IO.  We ignore I/O errors - if the page is not
	 * uptodate then the caller will launch readpage again, and
	 * will then handle the error.
	 */
	if (ret)
		workingset_read_pages(mapping, rpc->filp, &page_pool, ret);

	WARN_ON(!list_empty(&page_pool));

out:
	return ret;
}
/*lint +e548*/

static bool path_is_under_dirs(const char *path, const char * const dir[])
{
	int i = 0;

	while (dir[i] != NULL) {
		if (strncmp(path, dir[i], strlen(dir[i])) == 0)
			return true;
		i++;
	}
	return false;
}

static bool workingset_prereader_can_open(const char *path)
{
	bool is_myself_dir = path_is_under_dirs(path, g_selfread_dir);
	if ((g_selfread_state != E_SELFREAD_DOING) && is_myself_dir)
		return false;
	else if ((g_selfread_state == E_SELFREAD_DOING) && !is_myself_dir)
		return false;
	else
		return true;
}

static void workingset_prereader_setresuid(uid_t euid)
{
#ifdef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	__sys_setresuid((gid_t)-1, euid, (gid_t)-1);
#else
	sys_setresuid((gid_t)-1, euid, (gid_t)-1);
#endif
}

static void workingset_prereader_open_all_files_rcrdlocked(
	struct s_ws_record *record, struct file **filpp)
{
	int flags = O_RDONLY;
	unsigned int idx;
	unsigned int next_loop;
	struct s_ws_data *data = &record->data;
	const struct cred *mycred = current_cred(); /*lint !e666*/
	uid_t myself_euid;
	uid_t target_euid;

	if (force_o_largefile())
		flags |= O_LARGEFILE;

	next_loop = 0;
	myself_euid = from_kuid_munged(mycred->user_ns, mycred->euid);
	while (next_loop < data->file_cnt) {
		target_euid = data->file_array[next_loop].owner_uid;
		if (target_euid != myself_euid) {
			if (g_selfread_state == E_SELFREAD_DOING) {
				next_loop++;
				continue;
			}
			workingset_prereader_setresuid(target_euid);
		}

		for (idx = next_loop, next_loop = data->file_cnt;
			idx < data->file_cnt; idx++) {
			if (filpp[idx] || !data->file_array[idx].path ||
			    !workingset_prereader_can_open(
			    data->file_array[idx].path))
				continue;

			if (target_euid != data->file_array[idx].owner_uid) {
				next_loop = (next_loop == data->file_cnt) ?
				    idx : next_loop;
				continue;
			}

			filpp[idx] = filp_open(data->file_array[idx].path,
			    flags, 0);

			if (!IS_ERR_OR_NULL(filpp[idx]))
				continue;

			if (record->state & E_RECORD_STATE_EXTERNAL_FILEPATH)
				kfree(data->file_array[idx].path);

			data->file_array[idx].path = NULL;
			filpp[idx] = NULL;
		}

		if (target_euid != myself_euid)
			workingset_prereader_setresuid(myself_euid);
	}
}

static struct file **workingset_prereader_alloc_filepp_rcrdlocked(
	struct s_ws_record *record)
{
	unsigned int idx;
	struct file **filpp = NULL;
	struct page *page = NULL;
	struct s_ws_data *data = &record->data;
	unsigned int need_pages;

	if (data->file_cnt <= FILPS_PER_PAGE)
		return (struct file **)get_zeroed_page(GFP_NOFS);

	need_pages = (data->file_cnt + FILPS_PER_PAGE - 1) / FILPS_PER_PAGE;
	for (idx = 0; idx < need_pages; idx++) {
		page = alloc_page(GFP_NOFS | __GFP_ZERO);
		if (!page) {
			pr_err("%s: OOM, alloc %u pages failed!\n",
				__func__, need_pages);
			break;
		}
		record->filp_pages[idx] = page;
	}

	if (idx >= need_pages)
		filpp = vmap(record->filp_pages, need_pages, VM_MAP,
		    PAGE_KERNEL); /*lint !e446*/
	else
		filpp = NULL;
	if (!filpp) {
		for (idx = 0; idx < need_pages; idx++) {
			if (record->filp_pages[idx]) {
				__free_page(record->filp_pages[idx]);
				record->filp_pages[idx] = NULL;
			}
		}
	}
	return filpp;
}

static bool workingset_filepage_is_need_skip_read(
	struct s_read_control *rc, unsigned int idx,
	unsigned int stage, bool read_major, unsigned int *pfile_idx)
{
	bool ret = false;
	bool is_major;
	unsigned int file_idx;
	unsigned int stage_num;
	struct s_ws_data *data = rc->data;
	struct file **filpp = rc->filpp;

	if (read_major) {
		stage_num = (data->cacheseq[idx] >> PAGE_STAGE_NUM_SHIFT) &
		    PAGE_STAGE_NUM_MASK;
		if (stage < stage_num) {
			rc->stage_end = idx;
			ret = true;
			goto out;
		}
	}

	file_idx = (data->cacheseq[idx] >> FILE_OFFSET_BITS) &
	    MAX_TOUCHED_FILES_COUNT;
	if ((file_idx >= data->file_cnt) || !filpp[file_idx]) {
		ret = true;
		goto out;
	}

	is_major = data->cacheseq[idx] & (PAGE_MAJOR_MASK << PAGE_MAJOR_SHIFT);
	if ((read_major && !is_major) || (!read_major && is_major)) {
		ret = true;
		goto out;
	}
	*pfile_idx = file_idx;
out:
	return ret;
}

static void workingset_fill_rpt(struct s_readpages_control *rpc,
	struct file *filp)
{
#if defined(CONFIG_OVERLAY_FS) && (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
	struct file *real_file = get_real_file(filp);
	rpc->filp = real_file;
	rpc->mapping = real_file->f_mapping;
#else
	rpc->filp = filp;
	rpc->mapping = filp->f_mapping;
#endif
}

static int workingset_read_filepage_looper(
	struct s_read_control *rc, unsigned int stage_begin,
	unsigned int stage, bool read_major)
{
	struct s_readpages_control rpc;
	struct s_ws_data *data = rc->data;
	struct file **filpp = rc->filpp;
	struct page *page = NULL;
	unsigned int idx;
	unsigned int file_idx;
	unsigned int read_count;
	/*
	 * In some case, io request is congested, so we must be ensure
	 * read file page of main thread touched first.
	 */
	for (idx = stage_begin; idx < rc->stage_end; idx++) {
		if (!(idx % ONE_HUNDRED) && atomic_read(&g_preread_abort))
			return -EINTR;
		if (workingset_filepage_is_need_skip_read(
		    rc, idx, stage, read_major, &file_idx))
			continue;

		/* Find file page in page cache. */
		workingset_fill_rpt(&rpc, filpp[file_idx]);
		rpc.offset = data->cacheseq[idx] & MAX_TOUCHED_FILE_OFFSET;
		if ((data->cacheseq[idx] >> PAGE_RANGE_HEAD_SHIFT) &
		    PAGE_RANGE_HEAD_MASK) {
			/* In the case, prereading multi file pages */
			rpc.nr_to_read = (data->cacheseq[++idx] &
			    MAX_TOUCHED_FILE_OFFSET) - rpc.offset;
			read_count = workingset_page_cache_range_read(&rpc);
			rc->present_pages_cnt += rpc.nr_to_read - read_count;
			rc->read_pages_cnt += read_count;
			rc->move_lru_cnt += rpc.nr_adjusted;
			continue;
		}

		/* In the case, prereading single file page */
		page = find_get_page(rpc.mapping, rpc.offset);
		if (page) {
			if (workingset_adjust_page_lru(page))
				rc->move_lru_cnt += 1;
			put_page(page);
			rc->present_pages_cnt += 1;
		} else if (!workingset_page_cache_read(&rpc)) {
			rc->read_pages_cnt += 1;
		}
	}
	return 0;
}

static void check_cachemiss_threshold_rcrdlocked(
	struct s_ws_record *record, unsigned int present,
	unsigned int absent)
{
	if ((((ONE_HUNDRED - CACHE_MISSED_THRESHOLD_FOR_BLKIO) *
	    (present + absent)) / ONE_HUNDRED) > present)
		record->state |= E_RECORD_STATE_UPDATE_BASE_BLKIO;
}

static void workingset_read_filepages_rcrdlocked(
	struct s_ws_record *record, struct file **filpp)
{
	struct s_ws_data *data = &record->data;
	unsigned int stage = 0;
	unsigned int stage_begin = 0;
	bool read_major = true;
	struct s_read_control rc = {
		.data = data,
		.filpp = filpp,
		.stage_end = data->pageseq_cnt,
		.present_pages_cnt = 0,
		.read_pages_cnt = 0,
		.move_lru_cnt = 0};

	/*
	 * In some case, io request is congested, so we must be ensure
	 * read file page of main thread touched first.
	 */
	while (1) {
		if (!workingset_read_filepage_looper(
		    &rc, stage_begin, stage, read_major)) {
			if (read_major) {
				read_major = false;
				continue;
			} else if (rc.stage_end < data->pageseq_cnt) {
				ws_dbg("%s %s, stage %u prsnt %u,mv %u,rd %u\n",
				__func__, record->owner.name, stage,
				rc.present_pages_cnt,
				rc.move_lru_cnt, rc.read_pages_cnt);
				read_major = true;
				stage_begin = rc.stage_end;
				rc.stage_end = data->pageseq_cnt;
				stage++;
				continue;
			}
		} else {
			break;
		}
		/*
		 * When many file pages are not present, the blkio count of main
		 * thread can be consider as the base blkio of prereading.
		 */
		if (g_selfread_state != E_SELFREAD_DOING)
			check_cachemiss_threshold_rcrdlocked(record,
				rc.present_pages_cnt, rc.read_pages_cnt);
		break;
	}

	ws_dbg("%s %s,%u fls,%u sqs,%u pgs,prsnt %u,mv %u,rd %u\n",
		__func__, record->owner.name, data->file_cnt,
		data->pageseq_cnt, data->page_sum, rc.present_pages_cnt,
		rc.move_lru_cnt, rc.read_pages_cnt);
}

static void workingset_preread_post_work_rcrdlocked(
	struct s_ws_record *record, struct file **filpp)
{
	unsigned int file_idx;
	unsigned int idx;
	struct s_ws_data *data = &record->data;

	if (filpp) {
		for (file_idx = 0; file_idx < data->file_cnt; file_idx++) {
			if (filpp[file_idx])
				filp_close(filpp[file_idx], NULL);
		}
	}

	for (idx = 0; idx < FILP_PAGES_COUNT; idx++) {
		if (record->filp_pages[idx]) {
			if (filpp) {
				vunmap(filpp);
				filpp = NULL;
			}
			__free_page(record->filp_pages[idx]);
			record->filp_pages[idx] = NULL;
		}
	}

	if (filpp) {
		free_page((unsigned long)filpp);
		filpp = NULL;
	}
}

void workingset_do_preread_work_rcrdlocked(struct s_ws_record *record)
{
	struct file **filpp = NULL;
	struct s_ws_data *data = &record->data;

	if (!data->file_cnt || !data->pageseq_cnt ||
	    !data->file_array || !data->cacheseq)
		return;

	if (atomic_read(&g_preread_abort))
		return;

	/* Alloc pages for save opened struct files */
	filpp = workingset_prereader_alloc_filepp_rcrdlocked(record);
	if (!filpp)
		return;

	workingset_prereader_open_all_files_rcrdlocked(record, filpp);
	workingset_read_filepages_rcrdlocked(record, filpp);
	workingset_preread_post_work_rcrdlocked(record, filpp);
}

void workingset_preread_by_self_internal(struct s_ws_record *record)
{
	struct task_struct *tsk = NULL;
	struct task_struct *leader = NULL;

	if (record && mutex_trylock(&record->mutex))  {
		record->state |= E_RECORD_STATE_PREREADING;
		workingset_do_preread_work_rcrdlocked(record);
		record->state &= ~E_RECORD_STATE_PREREADING;
		mutex_unlock(&record->mutex);
	}

	rcu_read_lock();
	leader = current->group_leader;
	tsk = leader;
	// cppcheck-suppress *
	do {
		tsk->ext_flags &= ~PF_EXT_WSCG_PREREAD;
	} while_each_thread(leader, tsk);
	rcu_read_unlock();
}

void workingset_preread_force_stop(void)
{
	atomic_set(&g_preread_abort, 1);
}

void workingset_preread_permmit(void)
{
	atomic_set(&g_preread_abort, 0);
}

enum ws_self_read_state workingset_get_preread_state(void)
{
	return g_selfread_state;
}

void workingset_set_preread_state(enum ws_self_read_state state)
{
	g_selfread_state = state;
}
