/*
 * Copyright(C) 2019-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
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

#include <linux/dma-buf.h>
#include <linux/fdtable.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi/mem_trace.h>
#include <linux/sched/signal.h>
#ifdef CONFIG_HISI_CMA_DEBUG
#include <linux/hisi/hisi_cma_debug.h>
#endif

#include "mm_ion_priv.h"
#include "ion.h"

unsigned long mm_ion_total(void)
{
	return get_ion_total_size();
}

/* this func must be in ion.c */
static inline struct ion_buffer *get_ion_buf(struct dma_buf *dbuf)
{
	return dbuf->priv;
}

static int mm_ion_debug_process_cb(const void *data,
			      struct file *f, unsigned int fd)
{
	const struct task_struct *tsk = data;
	struct dma_buf *dbuf = NULL;
	struct ion_buffer *ibuf = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dbuf = file_to_dma_buf(f);
	if (!dbuf)
		return 0;

	if (!is_ion_dma_buf(dbuf))
		return 0;

	ibuf = get_ion_buf(dbuf);
	if (!ibuf)
		return 0;

	pr_err("%-16.s%3s%-16d%-16u%-16zu%-16u%-16llu\n",
	tsk->comm, "", tsk->pid, fd, dbuf->size,
	ibuf->heap->id, ibuf->magic);
	return 0;
}

int mm_ion_proecss_info(void)
{
	struct task_struct *tsk = NULL;

	pr_err("Detail process info is below:\n");
	pr_err("%-16.s%3s%-16s%-16s%-16s%-16s%-16s\n",
	"taskname", "", "pid", "fd", "sz", "heapid", "magic");

	rcu_read_lock();
	for_each_process(tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;

		task_lock(tsk);
		(void)iterate_fd(tsk->files, 0,
			mm_ion_debug_process_cb, (void *)tsk);
		task_unlock(tsk);
	}
	rcu_read_unlock();

	return 0;
}

static size_t mm_ion_detail_cb(const void *data,
			      struct file *f, unsigned int fd)
{
	struct dma_buf *dbuf = NULL;
	struct ion_buffer *ibuf = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dbuf = file_to_dma_buf(f);
	if (!dbuf)
		return 0;

	if (!is_ion_dma_buf(dbuf))
		return 0;

	ibuf = get_ion_buf(dbuf);
	if (!ibuf)
		return 0;

	return dbuf->size;
}

static size_t ion_iterate_fd(struct files_struct *files, unsigned int n,
		size_t (*f)(const void *, struct file *, unsigned),
		const void *p)
{
	struct fdtable *fdt = NULL;
	size_t res = 0;
	struct file *file = NULL;

	if (!files)
		return 0;
	spin_lock(&files->file_lock);
	for (fdt = files_fdtable(files); n < fdt->max_fds; n++) {
		file = rcu_dereference_check_fdtable(files, fdt->fd[n]);
		if (!file)
			continue;
		res += f(p, file, n);
	}
	spin_unlock(&files->file_lock);
	return res;
}

size_t mm_get_ion_detail(void *buf, size_t len)
{
	size_t cnt = 0;
	size_t size;
	struct task_struct *tsk = NULL;
	struct mm_ion_detail_info *info =
		(struct mm_ion_detail_info *)buf;

	if (!buf)
		return cnt;
	rcu_read_lock();
	for_each_process(tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;
		if (cnt >= len) {
			rcu_read_unlock();
			return len;
		}
		task_lock(tsk);
		size = ion_iterate_fd(tsk->files, 0,
			mm_ion_detail_cb, (void *)tsk);
		task_unlock(tsk);
		if (size) { /* ion fd */
			(info + cnt)->size = size;
			(info + cnt)->pid = tsk->pid;
			cnt++;
		}
	}
	rcu_read_unlock();
	return cnt;
}

size_t mm_get_ion_size_by_pid(pid_t pid)
{
	size_t size;
	struct task_struct *tsk = NULL;

	rcu_read_lock();
	tsk = find_task_by_vpid(pid);
	if (!tsk) {
		rcu_read_unlock();
		return 0;
	}
	if (tsk->flags & PF_KTHREAD) {
		rcu_read_unlock();
		return 0;
	}
	task_lock(tsk);
	size = ion_iterate_fd(tsk->files, 0,
		mm_ion_detail_cb, (void *)tsk);
	task_unlock(tsk);

	rcu_read_unlock();
	return size;
}

void mm_ion_process_summary_info(void)
{
	struct task_struct *tsk = NULL;
	size_t tsksize;

	pr_err("Summary process info is below:\n");
	pr_err("%-16.s%3s%-16s%-16s\n", "taskname", "",
	"pid", "totalsize");

	rcu_read_lock();
	for_each_process(tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;

		task_lock(tsk);
		tsksize = ion_iterate_fd(tsk->files, 0,
				mm_ion_detail_cb, (void *)tsk);
		if (tsksize)
			pr_err("%-16.s%3s%-16d%-16zu\n",
			tsk->comm, "", tsk->pid, tsksize);
		task_unlock(tsk);
	}
	rcu_read_unlock();
}

int mm_ion_memory_info(bool verbose)
{
	struct rb_node *n = NULL;
	struct ion_device *dev = get_ion_device();

	if (!dev)
		return -EINVAL;

	pr_info("ion total size:%lu\n", get_ion_total_size());
	if (!verbose)
		return 0;

	pr_info("orphaned allocations (info is from last known client):\n");
	mutex_lock(&dev->buffer_lock);
	for (n = rb_first(&dev->buffers); n; n = rb_next(n)) {
		struct ion_buffer *buffer = rb_entry(n, struct ion_buffer,
				node);

		if ((buffer->heap->type != ION_HEAP_TYPE_CARVEOUT) &&
			buffer->heap->name)
			pr_info("%16.s %16u %16zu %16s\n", buffer->task_comm,
				buffer->pid, buffer->size, buffer->heap->name);
	}
	mutex_unlock(&dev->buffer_lock);

	mm_ion_process_summary_info();

	mm_ion_proecss_info();
#ifdef CONFIG_HISI_CMA_DEBUG
	dump_cma_mem_info();
#endif

	mm_svc_secmem_info();

	return 0;
}
