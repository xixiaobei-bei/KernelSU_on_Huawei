/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: sysctl interface implementation for xreclaimer
 * Author: Miao Xie <miaoxie@huawei.com>
 * Create: 2020-06-29
 */
#include <linux/mm.h>
#include <linux/xreclaimer.h>
#include <linux/mmu_notifier.h>
#include <linux/sched/mm.h>
#include <linux/freezer.h>
#include <linux/oom.h>
#include <asm/tlb.h>

#include "../internal.h"

unsigned int sysctl_xreclaimer_enable = 1;

static struct task_struct	*xreclaimer;
static DECLARE_WAIT_QUEUE_HEAD(xreclaimer_wait);
static struct mm_struct		*xreclaimer_mmlist;
static DEFINE_SPINLOCK(xreclaimer_mmlist_lock);

#define XRMMF_SELF_REAPING	0
#define XRMMF_QUEUED		1
#define XRMMF_ASYNC_REAPED	2

static void xreclaimer_reap_task_mm(struct mm_struct *mm)
{
	struct vm_area_struct *vma = NULL;
	struct mmu_gather tlb;

	down_read(&mm->mmap_sem);
	if (test_bit(MMF_OOM_SKIP, &mm->flags)) {
		up_read(&mm->mmap_sem);
		return;
	}

	/* See comment of __oom_reap_task_mm() */
	set_bit(MMF_UNSTABLE, &mm->flags);

	for (vma = mm->mmap ; vma; vma = vma->vm_next) {
		if (!can_madv_dontneed_vma(vma))
			continue;

		if (vma_is_anonymous(vma) || !(vma->vm_flags & VM_SHARED)) {
			tlb_gather_mmu(&tlb, mm, vma->vm_start, vma->vm_end);
			unmap_page_range(&tlb, vma, vma->vm_start, vma->vm_end,
					 NULL);
			tlb_finish_mmu(&tlb, vma->vm_start, vma->vm_end);
		}

		if (test_bit(MMF_OOM_SKIP, &mm->flags))
			break;
	}
	up_read(&mm->mmap_sem);

	set_bit(MMF_OOM_SKIP, &mm->flags);
}

static bool xreclaimer_can_quick_reclaimed(struct task_struct *tsk)
{
	struct mm_struct *mm = NULL;

	mm = tsk->mm;
	if (!mm)
		return false;

	if (test_bit(MMF_OOM_SKIP, &mm->flags) ||
	    test_bit(MMF_OOM_VICTIM, &mm->flags))
		return false;

	if (!tsk->signal)
		return false;

	/*
	 * This is safe without lock for this simple check because we just
	 * want to make sure there is no key service that share mm with it.
	 *
	 * If nr_threads == mm-shared-nr-tasks, it means there is no other
	 * task that share mm with it, and it's sure there is no such service.
	 *
	 * Don't worry about the child tasks that are forked after the check,
	 * it is the same as oom killer that those children will abort.
	 */
	if (tsk->signal->nr_threads != mm->mm_xreclaimer.xrmm_ntasks)
		return false;

	return true;
}

void xreclaimer_exit_mmap(struct mm_struct *mm)
{
	struct xreclaimer_mm *xrmm = &mm->mm_xreclaimer;

	if(unlikely(mm_is_oom_victim(mm)))
		return;

	if (!test_bit(XRMMF_QUEUED, &xrmm->xrmm_flags))
		return;

	if (test_bit(XRMMF_ASYNC_REAPED, &xrmm->xrmm_flags))
		return;

	if (!test_bit(MMF_OOM_SKIP, &mm->flags))
		set_bit(MMF_OOM_SKIP, &mm->flags);

	down_write(&mm->mmap_sem);
	up_write(&mm->mmap_sem);
}

void xreclaimer_cond_self_reclaim(int exit_code)
{
	struct xreclaimer_mm *xrmm = NULL;

	if (!sysctl_xreclaimer_enable)
		return;

	if (exit_code != SIGKILL)
		return;

	/*
	 * Self status check, lock is unnecessary because if is safe
	 * to access mm struct and signal struct.
	 */
	if (!xreclaimer_can_quick_reclaimed(current))
		return;

	xrmm = &current->mm->mm_xreclaimer;
	if (test_and_set_bit(XRMMF_SELF_REAPING, &xrmm->xrmm_flags))
		return;

	pr_debug("xreclaimer: Reap memory early by myself.\n");
	xreclaimer_reap_task_mm(current->mm);
}

void xreclaimer_queue_task(struct task_struct *tsk)
{
	struct xreclaimer_mm *xrmm = &tsk->mm->mm_xreclaimer;

	if (test_and_set_bit(XRMMF_QUEUED, &xrmm->xrmm_flags))
		return;

	/* Refer to oom reaper implementation, maybe it is unnecessary. */
	get_task_struct(tsk);
	mmgrab(tsk->mm);
	xrmm->xrmm_owner = tsk;

	spin_lock(&xreclaimer_mmlist_lock);
	xrmm->xrmm_next_mm = xreclaimer_mmlist;
	xreclaimer_mmlist = tsk->mm;
	spin_unlock(&xreclaimer_mmlist_lock);
	wake_up(&xreclaimer_wait);
}

void xreclaimer_quick_reclaim(struct task_struct *tsk)
{
	if (!xreclaimer)
		return;

	task_lock(tsk);
	if (xreclaimer_can_quick_reclaimed(tsk)) {
		pr_debug("xreclaimer: About to reap pid:%d (%s)\n",
			 task_pid_nr(tsk), tsk->comm);
		xreclaimer_queue_task(tsk);
	}
	task_unlock(tsk);
}

static int xreclaimer_executor(void *unused)
{
	struct mm_struct *mm = NULL;

	set_freezable();

	while (!kthread_should_stop()) {
		/*lint -e578*/
		wait_event_freezable(xreclaimer_wait,
				     xreclaimer_mmlist != NULL);
		/*lint -e578*/
		spin_lock(&xreclaimer_mmlist_lock);
		if (xreclaimer_mmlist == NULL) {
			spin_unlock(&xreclaimer_mmlist_lock);
			continue;
		}

		mm = xreclaimer_mmlist;
		xreclaimer_mmlist = mm->mm_xreclaimer.xrmm_next_mm;
		spin_unlock(&xreclaimer_mmlist_lock);

		xreclaimer_reap_task_mm(mm);

		smp_mb__before_atomic();
		set_bit(XRMMF_ASYNC_REAPED, &mm->mm_xreclaimer.xrmm_flags);

		put_task_struct(mm->mm_xreclaimer.xrmm_owner);
		mmdrop_async(mm);
	}

	return 0;
}

static int __init xreclaimer_init(void)
{
	int ret;

	xreclaimer = kthread_run(xreclaimer_executor, NULL, "xreclaimer");
	if (IS_ERR(xreclaimer)) {
		pr_err("Unable to create xreclaimer.\n");
		ret = PTR_ERR(xreclaimer);
		xreclaimer = NULL;
		return ret;
	}

	ret = xreclaimer_init_sysctl();
	if (ret) {
		kthread_stop(xreclaimer);
		xreclaimer = NULL;
		return ret;
	}

	return 0;
}
subsys_initcall(xreclaimer_init)
