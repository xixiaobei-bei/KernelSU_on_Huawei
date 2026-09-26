/*
 * cgroup_workingset.c
 *
 * control group workingset subsystem
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd
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

#include <securec.h>

#include <asm/tlbflush.h>
#include <asm/pgtable.h>

#include <linux/delayacct.h>
#include <linux/hugetlb_inline.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/mm_inline.h>
#include <linux/module.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <linux/version.h>

static bool g_module_initialized;
static struct s_ws_collector *g_collector;
static const char *moniter_states[E_MONITOR_STATE_MAX] = {
	"OUTOFWORK",
	"INWORKING",
	"PAUSED",
	"STOP",
	"ABORT",
	"PREREAD",
	"BACKUP",
	"CLEARYOUNG"
};

/* Dynamic debug informatioins controller */
bool ws_debug_enable __read_mostly;
module_param_named(debug_enable, ws_debug_enable, bool, S_IRWUSR);

/*
 * Shrinker of workingset code permmit shrinking the page_cache array
 * memory of workingset only if the workingset is outofwork or aborted.
 */
static unsigned long workingset_shrinker_scan(
	struct shrinker *shrinker, struct shrink_control *sc)
{
	int idx;
	int stop_idx;
	int max_count;
	unsigned long pages_freed;
	size_t offset = offsetof(struct s_workingset, cache_pages);
	struct s_workingset *ws =
	    container_of(shrinker, struct s_workingset, shrinker);

	max_count = (PAGE_SIZE - offset) / sizeof(struct s_pagecache_info **);
	if (!mutex_trylock(&ws->mutex))
		return SHRINK_STOP;

	/* Reclaim the page array when the workingset is out of work */
	if ((ws->state == E_CGROUP_STATE_MONITOR_OUTOFWORK) ||
	    (ws->state == E_CGROUP_STATE_MONITOR_ABORT)) {
		if (!ws->alloc_index)
			stop_idx = -1;
		else
			stop_idx = ws->alloc_index / PAGECACHEINFO_PER_PAGE;

		pages_freed = 0;
		for (idx = max_count - 1; idx > stop_idx; idx--) {
			if (ws->cache_pages[idx]) {
				free_page((unsigned long)ws->cache_pages[idx]);
				ws->cache_pages[idx] = NULL;
				pages_freed++;
			}
		}
	} else {
		pages_freed = 0;
	}

	mutex_unlock(&ws->mutex);  /*lint !e455*/
	ws_dbg("%s: reclaimed %lu pages\n", __func__, pages_freed);

	return pages_freed ? pages_freed : SHRINK_STOP;
}

static unsigned long workingset_shrinker_count(
	struct shrinker *shrinker, struct shrink_control *sc)
{
	unsigned int idx;
	unsigned int max_count;
	unsigned long pages_to_free = 0;
	struct s_workingset *ws =
	    container_of(shrinker, struct s_workingset, shrinker);

	max_count = (PAGE_SIZE - offsetof(struct s_workingset, cache_pages)) /
	    sizeof(struct s_pagecache_info **);
	if (!mutex_trylock(&ws->mutex))
		return 0;

	/* Reclaim the page array when the workingset is out of work */
	if ((ws->state == E_CGROUP_STATE_MONITOR_OUTOFWORK) ||
	    (ws->state == E_CGROUP_STATE_MONITOR_ABORT)) {
		for (idx = 0; idx < max_count; idx++) {
			if (ws->cache_pages[idx])
				pages_to_free++;
			else
				break;
		}
		if (pages_to_free >
		    1 + ws->alloc_index / PAGECACHEINFO_PER_PAGE)
			pages_to_free -=
			    1 + ws->alloc_index / PAGECACHEINFO_PER_PAGE;
		else
			pages_to_free = 0;
	} else {
		pages_to_free = 0;
	}
	mutex_unlock(&ws->mutex);  /*lint !e455*/

	return pages_to_free;
}

static void workingset_unregister_shrinker(struct s_workingset *ws)
{
	if (ws->shrinker_enabled) {
		unregister_shrinker(&ws->shrinker);
		ws->shrinker_enabled = false;
	}
}

static int workingset_register_shrinker(struct s_workingset *ws)
{
	ws->shrinker.scan_objects = workingset_shrinker_scan;
	ws->shrinker.count_objects = workingset_shrinker_count;
	ws->shrinker.batch = 0;
	ws->shrinker.seeks = DEFAULT_SEEKS;

	return register_shrinker(&ws->shrinker);
}

static inline struct s_workingset *css_workingset(
	struct cgroup_subsys_state *css)
{
	return container_of(css, struct s_workingset, css);
}

const char *workingset_state_strs(unsigned int state)
{
	unsigned int monitor_state;

	switch (state) {
	case E_CGROUP_STATE_MONITOR_INWORKING:
		monitor_state = E_MONITOR_STATE_INWORKING;
		break;
	case E_CGROUP_STATE_MONITOR_PAUSED:
		monitor_state = E_MONITOR_STATE_PAUSED;
		break;
	case E_CGROUP_STATE_MONITOR_PREREAD:
		monitor_state = E_MONITOR_STATE_PREREAD;
		break;
	case E_CGROUP_STATE_MONITOR_BACKUP:
		monitor_state = E_MONITOR_STATE_BACKUP;
		break;
	case E_CGROUP_STATE_MONITOR_CLEARYOUNG:
		monitor_state = E_MONITOR_STATE_CLEARYOUNG;
		break;
	case E_CGROUP_STATE_MONITOR_STOP:
		monitor_state = E_MONITOR_STATE_STOP;
		break;
	case E_CGROUP_STATE_MONITOR_ABORT:
		monitor_state = E_MONITOR_STATE_ABORT;
		break;
	default:
		monitor_state = E_MONITOR_STATE_OUTOFWORK;
		break;
	}

	return moniter_states[monitor_state];
};

static struct cgroup_subsys_state *workingset_css_alloc(
	struct cgroup_subsys_state *parent_css)
{
	struct s_workingset *ws;

	/*
	 * We alloc a page for saving struct s_workingset,
	 * because it need save pointer of pages
	 * that caching page offset range information.
	 */
	ws = (struct s_workingset *)get_zeroed_page(GFP_KERNEL);
	if (!ws)
		return ERR_PTR(-ENOMEM);

	mutex_init(&ws->mutex);
	return &ws->css;
}

/*
 * workingset_css_online - Commit creation of a workingset css
 * @css: css being created
 */
static int workingset_css_online(struct cgroup_subsys_state *css)
{
	struct s_workingset *ws = NULL;

	if (!css)
		return -EINVAL;
	ws = css_workingset(css);
	mutex_lock(&ws->mutex);
	ws->state = E_CGROUP_STATE_ONLINE;
	ws->file_count = 0;
	ws->pageseq_count = 0;
	ws->repeated_count = 0;
	ws->page_sum = 0;
	ws->stage_num = 0;
	ws->leader_blkio_cnt = 0;
	ws->leader_blkio_base = 0;
	ws->alloc_index = 0;
	ws->clear_young = false;
	INIT_LIST_HEAD(&ws->file_list);

	if (!workingset_register_shrinker(ws))
		ws->shrinker_enabled = true;
	else
		ws->shrinker_enabled = false;

	mutex_unlock(&ws->mutex);
	return 0;
}

/*
 * workingset_css_offline - Initiate destruction of a workingset css
 * @css: css being destroyed
 */
static void workingset_css_offline(struct cgroup_subsys_state *css)
{
	struct s_workingset *ws = NULL;

	if (css) {
		ws = css_workingset(css);
		mutex_lock(&ws->mutex);

		ws->state = E_CGROUP_STATE_OFFLINE;
		workingset_destroy_data(ws, true);

		workingset_unregister_shrinker(ws);

		mutex_unlock(&ws->mutex);
	}
}

static void workingset_css_free(struct cgroup_subsys_state *css)
{
	if (css)
		free_page((unsigned long)css_workingset(css));
}

static int workingset_can_attach(struct cgroup_taskset *tset)
{
	return g_module_initialized ? 0 : -ENODEV;
}

/*
 * Tasks can be migrated into a different workingset anytime regardless of its
 * current state.  workingset_attach() is responsible for making new tasks
 * conform to the current state.
 */
/*lint -e454*/
/*lint -e455*/
/*lint -e456*/
static void workingset_attach(struct cgroup_taskset *tset)
{
	struct task_struct *task = NULL;
	struct cgroup_subsys_state *new_css = NULL;
	struct s_workingset *ws = NULL;

	rcu_read_lock();

	cgroup_taskset_for_each(task, new_css, tset) {
		if (new_css == NULL)
			continue;
		ws = css_workingset(new_css);
		if ((ws->state & E_CGROUP_STATE_MONITOR_INWORKING) ==
		    E_CGROUP_STATE_MONITOR_INWORKING) {
			task->ext_flags |= PF_EXT_WSCG_MONITOR;
		} else {
			task->ext_flags &= ~PF_EXT_WSCG_MONITOR;
		}
	}

	rcu_read_unlock();
}
/*lint +e454*/
/*lint +e455*/
/*lint +e456*/

static int clear_pte_young_range(
	pmd_t *pmd, unsigned long addr,
	unsigned long end, struct mm_walk *walk)
{
	struct s_clear_param *cp = walk->private;
	struct vm_area_struct *vma = cp->vma;
	pte_t *pte = NULL;
	pte_t ptent;
	spinlock_t *ptl = NULL;
	struct page *page = NULL;

	split_huge_pmd(vma, pmd, addr);
	if (pmd_trans_unstable(pmd))
		return 0;

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;
		if (!pte_present(ptent))
			continue;
		if (!pte_young(ptent))
			continue;
		page = vm_normal_page(vma, addr, ptent);
		if (!page)
			continue;

		if (PageSwapBacked(page))
			continue;

		cp->nr_cleared++;
		ptep_test_and_clear_young(vma, addr, pte);
	}

	pte_unmap_unlock(pte - 1, ptl);

	return 0;
}

static int workingset_clear_pte_young_of_process(int pid)
{
	int ret = 0;
	struct task_struct *task = NULL;
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma = NULL;
	struct mm_walk clear_young_walk = {};
	struct s_clear_param cp;

	if (pid <= 0)
		return -EINVAL;

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task) {
		rcu_read_unlock();
		return -ESRCH;
	}
	get_task_struct(task);
	rcu_read_unlock();

	mm = get_task_mm(task);
	if (!mm)
		goto out;
	clear_young_walk.mm = mm;
	clear_young_walk.pmd_entry = clear_pte_young_range;

	down_read(&mm->mmap_sem);
	cp.nr_cleared = 0;
	clear_young_walk.private = &cp;
	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		if (is_vm_hugetlb_page(vma))
			continue;

		if (!vma->vm_file)
			continue;

		cp.vma = vma;
		walk_page_range(
			vma->vm_start, vma->vm_end, &clear_young_walk);
	}
	/*
	 * Entries with the Access flag set to 0 are never held in the TLB,
	 * meaning software does not have to flush the entry from the TLB
	 * after setting the flag.
	 */
	up_read(&mm->mmap_sem);
	mmput(mm);
out:
	put_task_struct(task);
	return ret;
}

static void workingset_blkio_monitor_wslocked(
	struct s_workingset *ws, unsigned int monitor_state)
{
	if (monitor_state == E_CGROUP_STATE_MONITOR_INWORKING ||
	    monitor_state == E_CGROUP_STATE_MONITOR_PAUSED ||
	    monitor_state == E_CGROUP_STATE_MONITOR_STOP) {
		struct task_struct *tsk = NULL;

		rcu_read_lock();
		tsk = find_task_by_vpid(ws->owner.pid);
		if (!tsk) {
			rcu_read_unlock();
			return;
		}

		if (tsk->delays == NULL) {
			rcu_read_unlock();
			pr_warn("%s, delayacct of task[pid:%d] is NULL!\n",
				__func__, ws->owner.pid);
			return;
		}

		get_task_struct(tsk);
		rcu_read_unlock();

		if (monitor_state == E_CGROUP_STATE_MONITOR_INWORKING)
			ws->leader_blkio_base = tsk->delays->blkio_count;
		else if (tsk->delays->blkio_count > ws->leader_blkio_base)
			ws->leader_blkio_cnt +=
			    (unsigned short)(tsk->delays->blkio_count
			    - ws->leader_blkio_base);
		ws_dbg("%s: state=%s, nr_blkio=%u, delta=%u\n", __func__,
			workingset_state_strs(monitor_state),
			tsk->delays->blkio_count, ws->leader_blkio_cnt);
		put_task_struct(tsk);
	}
}

/*
 * workingset_apply_state - Apply state change to a single cgroup_workingset
 * @ws: workingset to apply state change to
 * @monitor_state: the state of monitor of workingset.
 *
 * Set @state on @ws according to @monitor_state, and perform
 * inworking or outwork as necessary.
 */
static void workingset_apply_state(
	struct s_workingset *ws, unsigned int monitor_state)
{
	mutex_lock(&ws->mutex);
	if (ws->state & E_CGROUP_STATE_ONLINE) {
		if ((monitor_state & E_CGROUP_STATE_MONITORING) &&
		    !(ws->state & E_CGROUP_STATE_MONITORING)) {
			workingset_blkio_monitor_wslocked(ws, monitor_state);
		} else if ((ws->state & E_CGROUP_STATE_MONITORING) &&
		    !(monitor_state & E_CGROUP_STATE_MONITORING)) {
			workingset_blkio_monitor_wslocked(ws, monitor_state);
		}

		if ((ws->stage_num < PAGE_STAGE_NUM_MASK) && (ws->stage_num ||
		    (ws->state == E_CGROUP_STATE_MONITOR_INWORKING)))
			ws->stage_num++;
		ws->state &= ~E_CGROUP_STATE_MONITOR_BITMASK;
		ws->state |= monitor_state;
	}
	mutex_unlock(&ws->mutex);
}

void workingset_collector_reset(const struct s_workingset *ws)
{
	spin_lock(&g_collector->lock);
	workingset_set_preread_state(E_SELFREAD_NONE);
	if (g_collector->monitor == ws) {
		g_collector->monitor = NULL;
		g_collector->read_pos = 0;
		g_collector->write_pos = 0;
		g_collector->discard_count = 0;
	}
	spin_unlock(&g_collector->lock);
}

/*
 * workingset_change_state -
 * Change the enter or exit state of a cgroup_workingset
 * @ws: workingset of interest
 * @monitor_state: the state of monitor of workingset.
 *
 * The operations are recursive -
 * all descendants of @workingset will be affected.
 */
static void workingset_change_state(
	struct s_workingset *ws, unsigned int monitor_state)
{
	struct cgroup_subsys_state *pos = NULL;

	rcu_read_lock();
	css_for_each_descendant_pre(pos, &ws->css) {
		struct s_workingset *pos_f = css_workingset(pos);

		if (!css_tryget_online(pos))
			continue;
		rcu_read_unlock();

		workingset_apply_state(pos_f, monitor_state);

		rcu_read_lock();
		css_put(pos);
	}
	rcu_read_unlock();
}

static ssize_t workingset_get_target_state(
	struct s_workingset *ws, char *buf,
	size_t nbytes, unsigned int *target_state)
{
	ssize_t nr_write = 0;

	buf = strstrip(buf);
	if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_INWORKING)) == 0) {
		*target_state = E_CGROUP_STATE_MONITOR_INWORKING;
	} else if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_PAUSED)) == 0) {
		*target_state = E_CGROUP_STATE_MONITOR_PAUSED;
	} else if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_STOP)) == 0) {
		*target_state = E_CGROUP_STATE_MONITOR_STOP;
	} else if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_ABORT)) == 0) {
		*target_state = E_CGROUP_STATE_MONITOR_ABORT;
	} else if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_PREREAD)) == 0) {
		*target_state = E_CGROUP_STATE_MONITOR_PREREAD;
	} else if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_BACKUP)) == 0) {
		workingset_writeback_all_records();
		nr_write = nbytes;
	} else if (strcmp(buf, workingset_state_strs(
	    E_CGROUP_STATE_MONITOR_CLEARYOUNG)) == 0) {
		ws->clear_young = true;
		workingset_clear_pte_young_of_process(ws->owner.pid);
		nr_write = nbytes;
	} else {
		nr_write = -EINVAL;
	}

	return nr_write;
}

static void workingset_mark_target_process(int pid, bool clear)
{
	struct task_struct *task = NULL;
	struct task_struct *leader = NULL;

	if (pid <= 0)
		return;

	if (!clear) {
		spin_lock(&g_collector->lock);
		if (workingset_get_preread_state() != E_SELFREAD_INIT) {
			spin_unlock(&g_collector->lock);
			return;
		}
		workingset_set_preread_state(E_SELFREAD_WAIT);
		spin_unlock(&g_collector->lock);
	}
	if (workingset_get_preread_state() == E_SELFREAD_DOING)
		return;

	rcu_read_lock();
	leader = find_task_by_vpid(pid);
	if (!leader) {
		rcu_read_unlock();
		return;
	}

	task = leader;
	// cppcheck-suppress *
	do {
		if (clear)
			task->ext_flags &= ~PF_EXT_WSCG_PREREAD;
		else
			task->ext_flags |= PF_EXT_WSCG_PREREAD;
	} while_each_thread(leader, task);
	rcu_read_unlock();
}

static void workingset_permit_selfread(int pid)
{
	spin_lock(&g_collector->lock);
	if (workingset_get_preread_state() != E_SELFREAD_NONE) {
		spin_unlock(&g_collector->lock);
		return;
	}
	workingset_set_preread_state(E_SELFREAD_INIT);
	spin_unlock(&g_collector->lock);
	workingset_mark_target_process(pid, false);
}

static void workingset_prereader_handler(
	struct s_workingset *ws, unsigned int target_state)
{
	if (target_state == E_CGROUP_STATE_MONITOR_PREREAD ||
	    target_state == E_CGROUP_STATE_MONITOR_STOP ||
	    target_state == E_CGROUP_STATE_MONITOR_ABORT) {
		struct s_ws_record *record = NULL;

		mutex_lock(&ws->mutex);
		record = workingset_get_existed_record_wslocked(&ws->owner,
		    false);
		mutex_unlock(&ws->mutex);

		if (record)  {
			if ((target_state == E_CGROUP_STATE_MONITOR_ABORT) ||
			    (target_state == E_CGROUP_STATE_MONITOR_STOP))
				workingset_preread_force_stop();

			mutex_lock(&record->mutex);
			if ((target_state == E_CGROUP_STATE_MONITOR_PREREAD) &&
			    !(record->state & E_RECORD_STATE_PREREADING)) {
				record->state |= E_RECORD_STATE_PREREADING;
				workingset_do_preread_work_rcrdlocked(record);
			} else if (target_state ==
			    E_CGROUP_STATE_MONITOR_STOP) {
				record->state &= ~E_RECORD_STATE_PREREADING;
				workingset_preread_permmit();
			} else if (target_state ==
			    E_CGROUP_STATE_MONITOR_ABORT) {
				record->state &= ~(E_RECORD_STATE_PREREADING |
				    E_RECORD_STATE_UPDATE_BASE_BLKIO);
				workingset_preread_permmit();
			}
			mutex_unlock(&record->mutex);

			if (target_state == E_CGROUP_STATE_MONITOR_PREREAD)
				workingset_permit_selfread(ws->owner.pid);
		}
	}
}

static ssize_t workingset_collector_start_handler(
	struct s_workingset *ws, unsigned int target_state)
{
	spin_lock(&g_collector->lock);
	if (g_collector->monitor && g_collector->monitor != ws) {
		spin_unlock(&g_collector->lock);
		return -EBUSY;
	}
	g_collector->monitor = ws;
	spin_unlock(&g_collector->lock);

	return 0;
}

static void workingset_collector_stop_handler(
	struct cgroup_subsys_state *css,
	struct s_workingset *ws, unsigned int target_state)
{
	if (target_state == E_CGROUP_STATE_MONITOR_STOP) {
		workingset_mark_target_process(ws->owner.pid, true);
		spin_lock(&g_collector->lock);
		workingset_set_preread_state(E_SELFREAD_NONE);
		g_collector->wait_flag = F_RECORD_PENDING;
		/* Notify the collect thread monitor is stoped */
		if (waitqueue_active(&g_collector->collect_wait))
			wake_up_interruptible_all(
				&g_collector->collect_wait);
		spin_unlock(&g_collector->lock);
	} else if (target_state == E_CGROUP_STATE_MONITOR_ABORT) {
		workingset_mark_target_process(ws->owner.pid, true);
		workingset_destroy_data(ws, false);
		workingset_collector_reset(ws);
	}
}

static ssize_t workingset_state_write(
	struct kernfs_open_file *of,
	char *buf, size_t nbytes, loff_t off)
{
	ssize_t nr_write;
	unsigned int target_state = E_CGROUP_STATE_MAX;
	struct cgroup_subsys_state *css = NULL;
	struct s_workingset *ws = NULL;

	if (!g_module_initialized)
		return -ENODEV;

	if (!of || !buf)
		return -EINVAL;
	css = of_css(of);
	if (!css)
		return -EINVAL;
	ws = css_workingset(css);
	nr_write = workingset_get_target_state(ws, buf, nbytes, &target_state);
	if (nr_write)
		return nr_write;

	if (target_state == E_CGROUP_STATE_MONITOR_INWORKING) {
		nr_write = workingset_collector_start_handler(ws, target_state);
		if (nr_write)
			return nr_write;
	}

	if (target_state != E_CGROUP_STATE_MONITOR_PREREAD)
		workingset_change_state(ws, target_state);

	ws_dbg("%s: uid=%u, name=%s, state=%s\n", __func__, ws->owner.uid,
		ws->owner.name, workingset_state_strs(ws->state));
	workingset_prereader_handler(ws, target_state);

	workingset_collector_stop_handler(css, ws, target_state);

	/* Writeback a dirty record when we preread completely */
	if (target_state == E_CGROUP_STATE_MONITOR_PREREAD)
		workingset_writeback_last_record_if_need();

	return nbytes;
}

static int workingset_state_read(struct seq_file *m, void *v)
{
	struct cgroup_subsys_state *css = NULL;

	if (!g_module_initialized)
		return -ENODEV;

	if (!m)
		return -EINVAL;
	css = seq_css(m);
	if (!css)
		return -EINVAL;

	seq_puts(m, workingset_state_strs(css_workingset(css)->state));
	seq_putc(m, '\n');
	return 0;
}

static void init_owner(
	struct s_workingset *ws, unsigned int uid,
	int pid, char *owner_name, char *record_path)
{
	mutex_lock(&ws->mutex);
	ws->owner.uid = uid; /*lint !e530*/
	ws->owner.pid = pid; /*lint !e530*/

	kfree(ws->owner.name);
	ws->owner.name = owner_name;

	kfree(ws->owner.record_path);
	ws->owner.record_path = record_path;
	mutex_unlock(&ws->mutex);
}

/*
 * workingset_data_parse_owner -
 * Parse information of the owner of workingset from the comming string.
 * @ws workingset the owner working on.
 * @owner_string the comming string.
 */
static int workingset_data_parse_owner(struct s_workingset *ws, char *str)
{
	int ret = 0;
	int pid;
	unsigned int uid;
	unsigned int len;
	char *token = NULL;
	char *owner_name = NULL;
	char *record_path = NULL;

	/* The 1th: uid */
	token = strsep(&str, " ");
	if (token == NULL || str == NULL || kstrtouint(token, 0, &uid))
		return -EINVAL;

	/* The 2th: pid */
	token = strsep(&str, " ");
	if (token == NULL || str == NULL || kstrtouint(token, 0, &pid))
		return -EINVAL;

	/* The 3th: name of owner */
	token = strsep(&str, " ");
	if (token == NULL || str == NULL)
		return -EINVAL;

	len = strlen(token);	/*lint !e668*/
	if (len <= 0 || len >= OWNER_MAX_CHAR)
		return -EINVAL;

	owner_name = kzalloc(++len, GFP_NOFS);
	if (!owner_name)
		return -ENOMEM;

	if (strncpy_s(owner_name, len, token, len)) {
		ret = -EINVAL;
		goto parse_path_failed;
	}

	/* The 4th: the path of record */
	len = strlen(str);	/*lint !e668*/
	if (len <= 0 || len >= PATH_MAX_CHAR) {
		ret = -EINVAL;
		goto parse_path_failed;
	}

	record_path = kzalloc(++len, GFP_NOFS);
	if (!record_path) {
		ret = -ENOMEM;
		goto parse_path_failed;
	}

	if (strncpy_s(record_path, len, str, len)) {
		ret = -EINVAL;
		goto copy_path_failed;
	}

	init_owner(ws, uid, pid, owner_name, record_path); /*lint !e530*/
	return 0;

copy_path_failed:
	kfree(record_path);
parse_path_failed:
	kfree(owner_name);
	return ret;
}

static ssize_t workingset_data_write(
	struct kernfs_open_file *of, char *buf, size_t nbytes, loff_t off)
{
	int ret;
	struct cgroup_subsys_state *css = NULL;
	struct s_workingset *ws = NULL;

	if (!g_module_initialized)
		return -ENODEV;

	if (!of || !buf)
		return -EINVAL;
	css = of_css(of);
	if (!css)
		return -EINVAL;

	ws = css_workingset(css);
	buf = strstrip(buf);
	ret = workingset_data_parse_owner(ws, buf);
	if (ret)
		return ret;

	workingset_mark_target_process(ws->owner.pid, false);
	return nbytes;
}

static int workingset_data_read(struct seq_file *m, void *v)
{
	struct cgroup_subsys_state *css = NULL;
	struct s_workingset *ws = NULL;
	struct s_ws_record *record = NULL;

	if (!g_module_initialized)
		return -ENODEV;

	if (!m)
		return -EINVAL;
	css = seq_css(m);
	if (!css)
		return -EINVAL;

	ws = css_workingset(css);
	mutex_lock(&ws->mutex);
	seq_printf(m, "Uid: %u\n", ws->owner.uid);
	seq_printf(m, "Pid: %d\n", ws->owner.pid);
	seq_printf(m, "Name: %s\n",
		ws->owner.name ? ws->owner.name : "Unknow");
#ifdef CONFIG_HW_CGROUP_WORKINGSET_DEBUG
	seq_printf(m, "RecordPath: %s\n",
		ws->owner.record_path ? ws->owner.record_path : "Unknow");
#endif
	record = workingset_get_existed_record_wslocked(&ws->owner, false);
	seq_printf(m, "RecordState:%s\n",
	!record ? "none" : (record->need_update ? "older" : "uptodate"));
	mutex_unlock(&ws->mutex);

	return 0;
}

static ssize_t workingset_clear_record_write(
	struct kernfs_open_file *of, char *buf, size_t nbytes, loff_t off)
{
	int ret;

	if (!g_module_initialized)
		return -ENODEV;

	if (!buf)
		return -EINVAL;

	buf = strstrip(buf);
	ret = workingset_clear_record(buf);
	if (ret)
		return ret;
	else
		return nbytes;
}

static struct cftype files[] = {
	{
		.name = "state",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = workingset_state_read,
		.write = workingset_state_write,
	},
	{
		.name = "data",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = workingset_data_read,
		.write = workingset_data_write,
	},
	{
		.name = "clearRecord",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.write = workingset_clear_record_write,
	},
	{ } /* Terminate */
};

struct cgroup_subsys workingset_cgrp_subsys = {
	.css_alloc	= workingset_css_alloc,
	.css_online	= workingset_css_online,
	.css_offline	= workingset_css_offline,
	.css_free	= workingset_css_free,
	.can_attach = workingset_can_attach,
	.attach = workingset_attach,
	.legacy_cftypes	= files,
};

static int workingset_cachepage_info_init(
	struct file *file, pgoff_t offset, unsigned int count,
	bool is_major, struct s_cachepage_info *info)
{
	if (unlikely(!file) ||
	    unlikely(offset > MAX_TOUCHED_FILE_OFFSET))
		return -EINVAL;

	if (unlikely(offset + count - 1 > MAX_TOUCHED_FILE_OFFSET))
		count = MAX_TOUCHED_FILE_OFFSET - offset + 1;

	info->filp = file;
	info->offset = (unsigned int)offset;
	info->count = count;
	info->is_major = is_major;
	return 0;
}

void workingset_pagecache_record(
	struct file *file, pgoff_t offset,
	unsigned int count, bool is_pagefault)
{
	struct s_cachepage_info info;
	unsigned int remain_space;
	bool mmap_only = false;
#if defined(CONFIG_HW_VIP_THREAD)
	bool is_major = current->static_vip;
#else
	bool is_major = (current->pid == current->tgid);
#endif
	if (workingset_cachepage_info_init(file, offset,
	    count, is_major, &info))
		return;

	spin_lock(&g_collector->lock);
	if (!g_collector->monitor)
		goto abort;

	mmap_only = !is_major &&
	    (g_collector->monitor->state == E_CGROUP_STATE_MONITOR_PAUSED);
	if (!is_pagefault && mmap_only)
		goto abort;

	info.stage = g_collector->monitor->stage_num;
	if (g_collector->read_pos <= g_collector->write_pos)
		remain_space = COLLECTOR_CACHE_SIZE - g_collector->write_pos +
		    g_collector->read_pos;
	else
		remain_space = g_collector->read_pos - g_collector->write_pos;

	/*
	 * When the circle buffer is almost full, we collect touched
	 * file page of main thread only.
	 */
	if (remain_space < COLLECTOR_REMAIN_CACHE_LOW_WATER) {
		if (!is_major || (remain_space <= sizeof(info))) {
			g_collector->discard_count++;
			goto abort;
		}
	}

	*(struct s_cachepage_info *)(g_collector->circle_buffer +
	    g_collector->write_pos) = info;
	if (g_collector->write_pos + sizeof(info) == COLLECTOR_CACHE_SIZE)
		g_collector->write_pos = 0;
	else
		g_collector->write_pos +=  sizeof(info);
	atomic_long_inc(&file->f_count);
	spin_unlock(&g_collector->lock);

	/* Notify the collect thread pageinfos comming */
	if (waitqueue_active(&g_collector->collect_wait)) {
		g_collector->wait_flag = F_COLLECT_PENDING;
		wake_up_interruptible_all(&g_collector->collect_wait);
	}
	return;

abort:
	spin_unlock(&g_collector->lock);
}

void workingset_preread_by_self(void)
{
	struct s_workingset *ws = NULL;
	struct s_ws_record *record = NULL;
	const struct cred *cred = NULL;
	kuid_t ws_uid;

	spin_lock(&g_collector->lock);
	if (workingset_get_preread_state() != E_SELFREAD_WAIT)
		goto out;

	ws = g_collector->monitor;
	if ((ws == NULL) || !mutex_trylock(&ws->mutex))
		goto out;

	cred = current_cred();	/*lint !e666*/
	ws_uid = KUIDT_INIT(ws->owner.uid);
	if (!uid_eq(cred->uid, ws_uid))
		goto uid_miss;

	workingset_set_preread_state(E_SELFREAD_DOING);
	spin_unlock(&g_collector->lock);
	record = workingset_get_existed_record_wslocked(&ws->owner, true);
	mutex_unlock(&ws->mutex);
	workingset_preread_by_self_internal(record);
	return;

uid_miss:
	mutex_unlock(&ws->mutex);
out:
	spin_unlock(&g_collector->lock);
}

static int alloc_circle_buffer(void)
{
	int ret = 0;
#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
	struct page *page = alloc_pages_node(NUMA_NO_NODE, GFP_KERNEL,
	    COLLECTOR_CACHE_SIZE_ORDER);
#else
	struct page *page = alloc_kmem_pages_node(NUMA_NO_NODE, GFP_KERNEL,
	    COLLECTOR_CACHE_SIZE_ORDER);
#endif

	if (!page)
		ret = -ENOMEM;
	else
		g_collector->circle_buffer = page_address(page);

	return ret;
}

static void free_circle_buffer(void)
{
	if (g_collector->circle_buffer) {
#if KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
		__free_pages(virt_to_page(g_collector->circle_buffer),
						COLLECTOR_CACHE_SIZE_ORDER);
#else
		free_kmem_pages((unsigned long)g_collector->circle_buffer,
						COLLECTOR_CACHE_SIZE_ORDER);
#endif
		g_collector->circle_buffer = NULL;
	}
}

static int __init cgroup_workingset_init(void)
{
	int ret = 0;

	if (COLLECTOR_CACHE_SIZE % sizeof(struct s_cachepage_info)) {
		pr_err("%s, size of cache is not aligned with %lu\n",
			__func__, sizeof(struct s_cachepage_info));
		ret = -EINVAL;
		goto out;
	}

	if (workingset_crc32_init()) {
		pr_err("%s, init crc32c cypto failed\n", __func__);
		goto out;
	}

	g_collector = kzalloc(sizeof(struct s_ws_collector), GFP_KERNEL);
	if (!g_collector)
		goto create_collector_fail;

	if (alloc_circle_buffer()) {
		pr_err("%s, collector cache alloc failed!\n", __func__);
		goto create_collector_cache_fail;
	}

	spin_lock_init(&g_collector->lock);
	init_waitqueue_head(&g_collector->collect_wait);

	g_collector->collector_thread = kthread_run(
	    workingset_collect_kworkthread,
	    g_collector, "workingset:collector");
	if (IS_ERR(g_collector->collector_thread)) {
		ret = PTR_ERR(g_collector->collector_thread);
		pr_err("%s: create the collector thread failed!\n", __func__);
		goto create_collector_thread_fail;
	}

	workingset_record_list_init();
	g_module_initialized = true;
	return 0;

create_collector_thread_fail:
	free_circle_buffer();
create_collector_cache_fail:
	kfree(g_collector);
	g_collector = NULL;
create_collector_fail:
	workingset_crc32_deinit();
out:
	return ret;
}

static void __exit cgroup_workingset_exit(void)
{
	kthread_stop(g_collector->collector_thread);
	g_collector->collector_thread = NULL;
	free_circle_buffer();
	kfree(g_collector);
	workingset_crc32_deinit();
	g_collector = NULL;
}

late_initcall(cgroup_workingset_init);
module_exit(cgroup_workingset_exit);
