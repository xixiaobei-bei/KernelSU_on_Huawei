/*
 * memcheck_memstat.c
 *
 * provide memstat information to user space to determine if leak happens
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include "memcheck_common.h"
#include "memcheck_interface.h"
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/pagemap.h>
#include <linux/version.h>
#include <linux/signal.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#endif
#include <chipset_common/hwmemcheck/memcheck.h>

#define PSS_SHIFT	12
#define IDX_JAVA	0
#define IDX_NATIVE	1
#define ADDR_NUM	2

#define JAVA_TAG	"dalvik-"
#define JAVA_TAG_LEN	7
#define JAVA_TAG2	"maple_alloc_ros"
#define JAVA_TAG2_LEN	15

#define NATIVE_TAG2	"libc_malloc"
#define NATIVE_TAG2_LEN	11

struct memsize_stats {
	unsigned long resident;
	unsigned long swap;
	u64 pss;
	u64 native_rss;
	u64 java_rss;
	u64 vss;
};

enum heap_type {
	HEAP_OTHER,
	HEAP_JAVA,
	HEAP_NATIVE,
};

static char top_slub_name[SLUB_NAME_LEN];

static const char *const java_tag[] = {
	"dalvik-alloc space",
	"dalvik-main space",
	"dalvik-large object space",
	"dalvik-free list large object space",
	"dalvik-non moving space",
	"dalvik-zygote space",
};

static u64 addr_array[][ADDR_NUM] = {
	/* MEMCMD_NONE */
	{ 0, 0 },
	/* MEMCMD_ENABLE */
	{ ADDR_JAVA_ENABLE, ADDR_NATIVE_ENABLE },
	/* MEMCMD_DISABLE */
	{ ADDR_JAVA_DISABLE, ADDR_NATIVE_DISABLE },
	/* MEMCMD_SAVE_LOG */
	{ ADDR_JAVA_SAVE, ADDR_NATIVE_SAVE },
	/* MEMCMD_CLEAR_LOG */
	{ ADDR_JAVA_CLEAR, ADDR_NATIVE_CLEAR },
};

static bool is_java_heap(const char *tag)
{
	int i;
	char *tmp = NULL;

	if (strncmp(tag, JAVA_TAG2, JAVA_TAG2_LEN) == 0)
		return true;

	for (i = 0; i < ARRAY_SIZE(java_tag); i++) {
		tmp = strstr(tag, java_tag[i]);
		if (tmp == tag)
			return true;
	}

	return false;
}

static bool is_native_heap(const char *tag)
{
	if (strncmp(tag, NATIVE_TAG2, NATIVE_TAG2_LEN) == 0)
		return true;
	return false;
}

static enum heap_type memcheck_get_heap_type(const char *name)
{
	enum heap_type type = HEAP_OTHER;

	if (!name)
		return type;

	if (is_native_heap(name))
		type = HEAP_NATIVE;
	else if (is_java_heap(name))
		type = HEAP_JAVA;
	return type;
}

static struct page **alloc_page_pointers(size_t num)
{
	struct page **page = NULL;
	size_t page_len = sizeof(**page) * num;

	page = kzalloc(page_len, GFP_KERNEL);
	if (!page)
		return ERR_PTR(-ENOMEM);

	return page;
}

static size_t do_strncpy_from_remote_string(char *dst, long page_offset,
					    struct page **page, long num_pin,
					    long count)
{
	long i;
	size_t sz;
	size_t strsz;
	size_t copy_sum = 0;
	long page_left = min((long)PAGE_SIZE - page_offset, count);
	const char *p = NULL;
	const char *kaddr = NULL;

	count = min(count, num_pin * (long)PAGE_SIZE - page_offset);

	for (i = 0; i < num_pin; i++) {
		kaddr = (const char *)kmap(page[i]);
		if (!kaddr)
			break;

		if (i == 0) {
			p = kaddr + page_offset;
			sz = page_left;
		} else {
			p = kaddr;
			sz = min((long)PAGE_SIZE, count - page_left -
				 (i - 1) * (long)PAGE_SIZE);
		}

		strsz = strnlen(p, sz);
		memcpy(dst, p, strsz);

		kunmap(page[i]);

		dst += strsz;
		copy_sum += strsz;

		if (strsz != sz)
			break;
	}

	for (i = 0; i < num_pin; i++)
		put_page(page[i]);

	return copy_sum;
}

static long strncpy_from_remote_user(char *dst, struct mm_struct *remote_mm,
				     const char __user *src, long count)
{
	long num_pin;
	size_t copy_sum;
	struct page **page = NULL;

	uintptr_t src_page_start = (uintptr_t)src & PAGE_MASK;
	uintptr_t src_page_offset = (uintptr_t)(src - src_page_start);
	size_t num_pages = DIV_ROUND_UP(src_page_offset + count,
					(long)PAGE_SIZE);

	page = alloc_page_pointers(num_pages);
	if (IS_ERR_OR_NULL(page))
		return PTR_ERR(page);

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	num_pin = get_user_pages_remote(current, remote_mm,
					src_page_start, num_pages, 0,
					page, NULL, NULL);
#else
	num_pin = get_user_pages_remote(current, remote_mm,
					src_page_start, num_pages, 0,
					page, NULL);
#endif
	if (num_pin < 1) {
		kfree(page);
		return 0;
	}

	copy_sum = do_strncpy_from_remote_string(dst, src_page_offset, page,
						 num_pin, count);
	kfree(page);

	return copy_sum;
}

enum heap_type memcheck_anon_vma_name(struct vm_area_struct *vma)
{
	const char __user *name_user = vma_get_anon_name(vma);
	unsigned long max_len = min((unsigned long)NAME_MAX + 1,
				    (unsigned long)PAGE_SIZE);
	char *out_name = NULL;
	enum heap_type type = HEAP_OTHER;
	long retcpy;

	out_name = kzalloc(max_len, GFP_KERNEL);
	if (!out_name)
		return type;

	retcpy = strncpy_from_remote_user(out_name, vma->vm_mm,
					  name_user, max_len);
	if (retcpy <= 0)
		goto free_name;

	type = memcheck_get_heap_type(out_name);

free_name:
	kfree(out_name);

	return type;
}

enum heap_type memcheck_get_type(struct vm_area_struct *vma)
{
	char *name = NULL;
	struct mm_struct *mm = vma->vm_mm;
	enum heap_type type = HEAP_OTHER;

	/* file map is never heap in Android Q */
	if (vma->vm_file)
		return type;

	/* get rid of stack */
	if ((vma->vm_start <= vma->vm_mm->start_stack) &&
	    (vma->vm_end >= vma->vm_mm->start_stack))
		return type;

	if ((vma->vm_ops) && (vma->vm_ops->name)) {
		name = (char *)vma->vm_ops->name(vma);
		if (name)
			goto got_name;
	}

	name = (char *)arch_vma_name(vma);
	if (name)
		goto got_name;

	/* get rid of vdso */
	if (!mm)
		return type;

	/* main thread native heap */
	if ((vma->vm_start <= mm->brk) && (vma->vm_end >= mm->start_brk))
		return HEAP_NATIVE;

	if (vma_get_anon_name(vma))
		return memcheck_anon_vma_name(vma);

got_name:
	return memcheck_get_heap_type(name);
}

static void memcheck_pte_entry(pte_t *pte, unsigned long addr,
			       struct mm_walk *walk)
{
	struct memsize_stats *mss = walk->private;
	struct vm_area_struct *vma = walk->vma;
	struct page *page = NULL;

	if (pte_present(*pte)) {
		page = vm_normal_page(vma, addr, *pte);
	} else if (is_swap_pte(*pte)) {
		swp_entry_t swpent = pte_to_swp_entry(*pte);
		if (!non_swap_entry(swpent))
			mss->swap += PAGE_SIZE;
		else if (is_migration_entry(swpent))
			page = migration_entry_to_page(swpent);
	}
	if (page)
		mss->resident += PAGE_SIZE;
}

static int memcheck_pte_range(pmd_t *pmd, unsigned long addr,
			      unsigned long end, struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	pte_t *pte = NULL;
	spinlock_t *ptl = NULL;

	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE)
		memcheck_pte_entry(pte, addr, walk);
	pte_unmap_unlock(pte - 1, ptl);
	cond_resched();

	return 0;
}

static int memcheck_get_mss(pid_t pid, struct memsize_stats *mss_total)
{
	struct task_struct *tsk = NULL;
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma = NULL;
	enum heap_type type;
	struct memsize_stats mss;
	struct mm_walk memstat_walk = {
		.pmd_entry = memcheck_pte_range,
		.private = &mss,
	};

	rcu_read_lock();
	tsk = find_task_by_vpid(pid);
	if (tsk)
		get_task_struct(tsk);
	rcu_read_unlock();
	if (!tsk)
		return -EINVAL;

	mm = get_task_mm(tsk);
	if (!mm) {
		put_task_struct(tsk);
		return -EINVAL;
	}

	memset(mss_total, 0, sizeof(*mss_total));

	down_read(&mm->mmap_sem);
	vma = mm->mmap;
	while (vma) {
		memset(&mss, 0, sizeof(mss));
		memstat_walk.mm = vma->vm_mm;
		if ((vma->vm_mm) && (!is_vm_hugetlb_page(vma))) {
			walk_page_range(vma->vm_start, vma->vm_end,
					&memstat_walk);
			/* simply add swap to resident to get a total rss */
			mss.resident += mss.swap;
			mss_total->resident += mss.resident;
			mss_total->vss += vma->vm_end - vma->vm_start;

			type = memcheck_get_type(vma);
			if (type == HEAP_JAVA)
				mss_total->java_rss += mss.resident;
			else if (type == HEAP_NATIVE)
				mss_total->native_rss += mss.resident;
		}
		vma = vma->vm_next;
	}
	up_read(&mm->mmap_sem);
	mmput(mm);
	put_task_struct(tsk);
	return 0;
}

unsigned short memcheck_get_memstat(struct memstat_all *p)
{
	int ret;
	unsigned int i;
	struct memsize_stats mss_total;
	unsigned short result = 0;

	/* get userspace memstat */
	if ((p->type & MTYPE_USER_PSS) || (p->type & MTYPE_USER_VSS)) {
		memset(&mss_total, 0, sizeof(mss_total));

		/* read the smaps */
		ret = memcheck_get_mss(p->id, &mss_total);
		if (!ret) {
			if (p->type & MTYPE_USER_PSS_JAVA) {
				p->pss.java_pss = mss_total.java_rss;
				result = result | MTYPE_USER_PSS_JAVA;
			}
			if (p->type & MTYPE_USER_PSS_NATIVE) {
				p->pss.native_pss = mss_total.native_rss;
				result = result | MTYPE_USER_PSS_NATIVE;
			}
			if (p->type & MTYPE_USER_PSS)
				p->total_pss = mss_total.resident;
			if (p->type & MTYPE_USER_VSS) {
				p->vss = mss_total.vss;
				result = result | MTYPE_USER_VSS;
			}
		}
	}
	if (p->type & MTYPE_USER_ION) {
		p->ion_pid = get_ion_by_pid(p->id);
		if (p->ion_pid)
			result = result | MTYPE_USER_ION;
	}

	if (!(p->type & MTYPE_KERNEL))
		return result;

	/* get kernel memstat */
	for (i = 0; i < NUM_KERN_MAX; i++) {
		if (TEST_BIT(p->type, i + IDX_KERN_START)) {
			p->memory = get_mem_total(i);
			if (p->memory) {
				result |= (1 << (i + IDX_KERN_START));
				break;
			}
		}
	}

	return result;
}

static bool process_disappear(u64 t, const struct track_cmd *cmd)
{
	if (cmd->cmd == MEMCMD_ENABLE)
		return false;
	if (cmd->timestamp != nsec_to_clock_t(t))
		return true;

	return false;
}

void memcheck_save_top_slub(const char *name)
{
	memcpy(top_slub_name, name, sizeof(top_slub_name));
}

int memcheck_do_kernel_command(const struct track_cmd *cmd)
{
	int ret = 0;

	switch (cmd->cmd) {
	case MEMCMD_ENABLE:
		if (cmd->type == MTYPE_KERN_SLUB) {
			memcheck_info("top1 slub is %s\n", top_slub_name);
			ret = page_trace_on(SLUB_TRACK, top_slub_name);
		} else if (cmd->type == MTYPE_KERN_BUDDY) {
			ret = page_trace_on(BUDDY_TRACK, "buddy");
		} else if (cmd->type == MTYPE_KERN_LSLUB) {
			ret = page_trace_on(LSLUB_TRACK, "lsub");
		}
		if (ret)
			memcheck_err("trace on failed, memtype=%d\n",
				     cmd->type);
		else
			memcheck_info("trace on success, memtype=%d\n",
				      cmd->type);
		break;
	case MEMCMD_DISABLE:
		if (cmd->type == MTYPE_KERN_SLUB)
			ret = page_trace_off(SLUB_TRACK, top_slub_name);
		else if (cmd->type == MTYPE_KERN_BUDDY)
			ret = page_trace_off(BUDDY_TRACK, "buddy");
		else if (cmd->type == MTYPE_KERN_LSLUB)
			ret = page_trace_off(LSLUB_TRACK, "lsub");
		if (ret)
			memcheck_err("trace off failed, memtype=%d\n",
				     cmd->type);
		else
			memcheck_info("trace off success, memtype=%d\n",
				      cmd->type);
		break;
	case MEMCMD_SAVE_LOG:
		break;
	case MEMCMD_CLEAR_LOG:
		break;
	default:
		break;
	}

	return ret;
}

int memcheck_do_command(const struct track_cmd *cmd)
{
	int ret = 0;
	struct task_struct *p = NULL;
	struct siginfo info = { 0 };
	u64 addr = 0;
	bool is_java = (cmd->type & MTYPE_USER_PSS_JAVA) ? true : false;
	bool is_native = (cmd->type & MTYPE_USER_PSS_NATIVE) ? true : false;

	if (cmd->type & MTYPE_KERNEL)
		return memcheck_do_kernel_command(cmd);

	if (is_java == is_native) {
		memcheck_err("invalid type=%d\n", cmd->type);
		return -EFAULT;
	}
	memset(&info, 0, sizeof(info));
	info.si_signo = SIGNO_MEMCHECK;
	info.si_errno = 0;
	info.si_code = SI_TKILL;
	info.si_pid = task_tgid_vnr(current);
	info.si_uid = from_kuid_munged(current_user_ns(), current_uid());

	rcu_read_lock();
	p = find_task_by_vpid(cmd->id);
	if (p)
		get_task_struct(p);
	rcu_read_unlock();

	if (p && (task_tgid_vnr(p) == cmd->id)) {
		if (process_disappear(p->real_start_time, cmd)) {
			memcheck_err("pid %d disappear\n", cmd->id);
			ret = MEMCHECK_PID_INVALID;
			goto err_pid_disappear;
		}

		if (is_java)
			addr = addr_array[cmd->cmd][IDX_JAVA];
		if (is_native)
			addr |= addr_array[cmd->cmd][IDX_NATIVE];
		info.si_addr = (void *)addr;
		if (is_java || is_native)
			ret = do_send_sig_info(SIGNO_MEMCHECK, &info, p, false);
	}

err_pid_disappear:
	if (p)
		put_task_struct(p);
	if ((!ret) && (cmd->cmd == MEMCMD_SAVE_LOG))
		memcheck_wait_stack_ready(cmd->type);
	else if ((!ret) && (cmd->cmd == MEMCMD_CLEAR_LOG))
		memcheck_stack_clear();

	return ret;
}
