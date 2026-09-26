/*
 * hisi_drm_heaps_debugger.c
 *
 * Offer debug feature for hisi drm, if CONFIG_HISI_DEBUGFS is not open, the
 * feature also close.
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#include "hisi_drm_heaps_tracer_inner.h"

#include <linux/kernel.h>
#include <linux/mm_types.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/sched/signal.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <linux/hisi/hisi_drm_heaps_tracer.h>

#include "hisi_drm_gem.h"
#include "hisi_gem_dmabuf.h"
#include "hisi_drm_heaps_defs.h"

struct hisi_gem_tracer_root {
	struct rb_root gems;

	spinlock_t rb_lock;

#ifdef CONFIG_HISI_DEBUG_FS
	struct dentry *debugfs_root;

	struct dentry *heaps_dir;
#endif
};

static struct hisi_gem_tracer_root *hisi_gem_tracer;

/**
 * dump nessary trace info into gem
 */
static void hisi_drm_heaps_dump_info(struct hisi_drm_heap_context *ctx)
{
	struct task_struct *task = NULL;
	struct hisi_drm_gem *hisi_gem = ctx->priv;

	get_task_struct(current->group_leader);
	task_lock(current->group_leader);

	if (current->group_leader->flags & PF_KTHREAD) {
		task = NULL;
		hisi_gem->pid = 0;
	} else {
		task = current->group_leader;
		hisi_gem->pid = task_pid_nr(task);
	}

	task_unlock(current->group_leader);
	put_task_struct(current->group_leader);

	if (!task)
		strncpy(hisi_gem->task_comm, "invalid task", TASK_COMM_LEN);
	else
		get_task_comm(hisi_gem->task_comm, task);
}

/**
 * when gem is destroy, cancel trace
 */
void hisi_drm_heaps_remove_trace(struct hisi_drm_heap_context *ctx)
{
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!ctx || !ctx->priv) {
		pr_heaps_err("invalid args for remove trace\n");
		return;
	}

	hisi_gem = ctx->priv;
	spin_lock(&hisi_gem_tracer->rb_lock);
	rb_erase(&hisi_gem->node, &hisi_gem_tracer->gems);
	spin_unlock(&hisi_gem_tracer->rb_lock);
}

/**
 * link gem into rb_tree, so we can trace it
 */
void hisi_drm_heaps_gem_trace(struct hisi_drm_heap_context *ctx)
{
	struct rb_node **p = &hisi_gem_tracer->gems.rb_node;
	struct rb_node *parent = NULL;
	struct hisi_drm_gem *buffer_gem = NULL;
	struct hisi_drm_gem *entry_gem = NULL;

	if (!ctx || !ctx->priv) {
		pr_heaps_err("invalid args for trace\n");
		return;
	}

	buffer_gem = ctx->priv;
	hisi_drm_heaps_dump_info(ctx);

	spin_lock(&hisi_gem_tracer->rb_lock);
	while (*p) {
		parent = *p;
		entry_gem = rb_entry(parent, struct hisi_drm_gem, node);

		if (buffer_gem < entry_gem) {
			p = &(*p)->rb_left;
		} else if (buffer_gem > entry_gem) {
			p = &(*p)->rb_right;
		} else {
			pr_heaps_err("hisi gem is already exist");
			BUG();
		}
	}

	rb_link_node(&buffer_gem->node, parent, p);
	rb_insert_color(&buffer_gem->node, &hisi_gem_tracer->gems);
	spin_unlock(&hisi_gem_tracer->rb_lock);
}

#ifdef CONFIG_HISI_DEBUG_FS

#define DEBUG_FS_BUF_SIZE 256
#define DEBUG_FS_BUF_NAME_SIZE 64

struct debug_process_heap_args {
	struct seq_file *seq;
	struct task_struct *tsk;
	size_t *total_heap_size;
};

static int debug_shrink_set(void *data, u64 val)
{
	struct hisi_drm_heap *heap = data;
	struct shrink_control sc;
	int objs;

	sc.gfp_mask = GFP_HIGHUSER;
	sc.nr_to_scan = val;

	if (!val) {
		objs = (int)heap->shrinker.count_objects(&heap->shrinker, &sc);
		sc.nr_to_scan = objs;
	}

	heap->shrinker.scan_objects(&heap->shrinker, &sc);
	return 0;
}

static int debug_shrink_get(void *data, u64 *val)
{
	struct hisi_drm_heap *heap = data;
	struct shrink_control sc;
	int objs;

	sc.gfp_mask = GFP_HIGHUSER;
	sc.nr_to_scan = 0;

	objs = (int)heap->shrinker.count_objects(&heap->shrinker, &sc);
	*val = objs;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debug_shrink_fops, debug_shrink_get, debug_shrink_set,
			"%llu\n");

static int debug_wt_mark_get(void *data, u64 *val)
{
	struct hisi_drm_heap *heap = data;

	*val = heap->ops->get_water_mark(heap);

	return 0;
}

static int debug_wt_mark_set(void *data, u64 val)
{
	struct hisi_drm_heap *heap = data;

	heap->ops->set_water_mark(heap, val);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debug_wt_mark_fops, debug_wt_mark_get,
			debug_wt_mark_set, "%llu\n");

/**
 * Search trace node, if node's head id is equals with request,
 * output trace info by seq_printf.
 */
static void hisi_drm_heaps_get_heap_info(struct seq_file *s, void *d,
					 unsigned int heap_id)
{
	struct rb_node *pos = NULL;
	struct hisi_drm_gem *hisi_gem = NULL;
	size_t heap_size = 0;

	seq_printf(s, "%16s %10s %16s %16s\n", "Proc", "PID", "Size",
		   "Kmap_cnt");
	seq_puts(
		s,
		"------------------------------------------------------------------------\n");
	spin_lock(&hisi_gem_tracer->rb_lock);
	for (pos = rb_first(&hisi_gem_tracer->gems); pos; pos = rb_next(pos)) {
		hisi_gem = rb_entry(pos, struct hisi_drm_gem, node);

		if (hisi_gem->heap_id != heap_id)
			continue;

		heap_size += hisi_gem->size;

		seq_printf(s, "%16s %10u %#16lx %16u\n", hisi_gem->task_comm,
			   hisi_gem->pid, hisi_gem->size, hisi_gem->kmap_cnt);
	}
	spin_unlock(&hisi_gem_tracer->rb_lock);
	seq_puts(
		s,
		"------------------------------------------------------------------------\n");
	seq_printf(s, "Heap ID %u, total size = %#16lx\n", heap_id, heap_size);
	seq_puts(
		s,
		"------------------------------------------------------------------------\n");
}

static int hisi_drm_heap_debug_show(struct seq_file *s, void *d)
{
	struct hisi_drm_heap *heap = s->private;

	if (!heap)
		return 0;

	hisi_drm_heaps_get_heap_info(s, d, heap->heap_id);

	if (heap->ops->debug_show)
		heap->ops->debug_show(heap, s, d);

	return 0;
}

static int hisi_drm_heap_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, hisi_drm_heap_debug_show, inode->i_private);
}

static const struct file_operations hisi_drm_heap_debug_fops = {
	.open = hisi_drm_heap_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __drm_debug_process_heap(const void *data, struct file *f,
				     unsigned int fd)
{
	const struct debug_process_heap_args *args = data;
	struct task_struct *tsk = args->tsk;
	struct dma_buf *dma_buf = NULL;
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dma_buf = file_to_dma_buf(f);
	if (!dma_buf || !is_hisi_drm_dmabuf(dma_buf))
		return 0;

	hisi_gem = to_hisi_gem(dma_buf->priv);
	if (!hisi_gem)
		return 0;

	*args->total_heap_size += hisi_gem->size;

	seq_printf(args->seq, "%16s %16u %16u %#16lx %16u %16s %16u\n",
		   tsk->comm, tsk->pid, fd, dma_buf->size, hisi_gem->pid,
		   hisi_gem->task_comm, hisi_gem->kmap_cnt);
	return 0;
}

static int hisi_drm_process_heap_debug_show(struct seq_file *s, void *d)
{
	struct task_struct *tsk = NULL;
	size_t task_total_heap_size;
	struct debug_process_heap_args args;

	seq_puts(s, "Process HISI DRM Heap info:\n");
	seq_puts(s, "----------------------------------------------------\n");
	seq_printf(s, "%16s %16s %16s %16s %16s %16s %16s %16s\n", "Process name",
		   "Process ID", "FD", "Size", "PID", "Task Name", "Kmap_cnt", "Heap ID");

	args.seq = s;
	rcu_read_lock();
	for_each_process (tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;

		task_total_heap_size = 0;
		args.total_heap_size = &task_total_heap_size;
		args.tsk = tsk;

		task_lock(tsk);
		iterate_fd(tsk->files, 0, __drm_debug_process_heap,
			   (void *)&args);
		if (task_total_heap_size > 0)
			seq_printf(s, "%16s %-16s %#16lx\n", "Total size of ",
				   tsk->comm, task_total_heap_size);

		task_unlock(tsk);
	}
	rcu_read_unlock();

	seq_puts(s, "----------------------------------------------------\n");

	return 0;
}

static int hisi_drm_process_heap_debug_open(struct inode *inode,
					    struct file *file)
{
	return single_open(file, hisi_drm_process_heap_debug_show,
			   inode->i_private);
}

static const struct file_operations hisi_drm_process_debug_fops = {
	.open = hisi_drm_process_heap_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/**
 * Init trace node by debugfs, we will create debugfs dir, and debugfs file.
 * First, create debugfs root dir in debugfs root dir name as "hisi_drm_heap".
 * and create "heaps" dir to save the debugfs which offer debug feature only
 * about heap.
 * Then, create "process_heap_info" debugfs, which will search all dma_buf,
 * and if it's drm dma_buf, show trace info about it
 */
static int hisi_drm_heaps_init_debugfs(void)
{
	struct dentry *d = NULL;

	hisi_gem_tracer->debugfs_root =
		debugfs_create_dir("hisi_drm_heap", NULL);
	if (IS_ERR_OR_NULL(hisi_gem_tracer->debugfs_root))
		return -ENOMEM;

	hisi_gem_tracer->heaps_dir =
		debugfs_create_dir("heaps", hisi_gem_tracer->debugfs_root);
	if (IS_ERR_OR_NULL(hisi_gem_tracer->heaps_dir))
		return -ENOMEM;

	d = debugfs_create_file("process_heap_info", 0440,
				hisi_gem_tracer->debugfs_root, NULL,
				&hisi_drm_process_debug_fops);
	if (IS_ERR_OR_NULL(d)) {
		pr_heaps_err("failed to create process debug file\n");
		debugfs_remove_recursive(hisi_gem_tracer->debugfs_root);
		debugfs_remove_recursive(hisi_gem_tracer->heaps_dir);
		hisi_gem_tracer->debugfs_root = NULL;
		hisi_gem_tracer->heaps_dir = NULL;
		return -ENOMEM;
	}

	return 0;
}


/**
 * Register heap's debugfs into heaps dir, we can cat this heap's debugfs
 * to trace all task who alloc memory by this heap.
 *
 * If this heap register shrink feature, also will create shrink debugfs for it.
 * When cat this debugfs, it will show the all page's count it can shrink. If
 * want to shrink memory, can use echo.
 *
 * If this heap implement set/get watermark function, also will
 * create debugfs for it. When "cat" this debugfs, it will show the heap's
 * water_mark by use the get_water_mark func. If echo, it will save the
 * water_mark by use the set_water_mark func.
 *
 * @param heap the heap which want to register debugfs node.
 */
void hisi_drm_heaps_init_heap_debugfs(struct hisi_drm_heap *heap)
{
	struct dentry *debug_file = NULL;
	char buf[DEBUG_FS_BUF_SIZE];
	char debug_name[DEBUG_FS_BUF_NAME_SIZE];
	char *path = NULL;

	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err("invalid input heap\n");
		return;
	}

	debug_file = debugfs_create_file(heap->name, 0440,
					 hisi_gem_tracer->heaps_dir, heap,
					 &hisi_drm_heap_debug_fops);
	if (IS_ERR_OR_NULL(debug_file)) {
		path = dentry_path(hisi_gem_tracer->heaps_dir, buf,
				   DEBUG_FS_BUF_SIZE);
		pr_heaps_info("Failed to create heap debugfs at %s/%s\n", path,
			      heap->name);
	}

	if (heap->shrinker.count_objects && heap->shrinker.scan_objects) {
		snprintf(debug_name, DEBUG_FS_BUF_NAME_SIZE, "%s_shrink",
			 heap->name);
		debug_file = debugfs_create_file(debug_name, 0640,
						 hisi_gem_tracer->heaps_dir,
						 heap, &debug_shrink_fops);
		if (IS_ERR_OR_NULL(debug_file)) {
			path = dentry_path(hisi_gem_tracer->heaps_dir, buf,
					   DEBUG_FS_BUF_SIZE);
			pr_heaps_info(
				"Failed to create heap shrinker debugfs at %s/%s\n",
				path, debug_name);
		}
	}

	if (heap->ops->set_water_mark && heap->ops->get_water_mark) {
		snprintf(debug_name, DEBUG_FS_BUF_NAME_SIZE, "%s_wt_mark", heap->name);
		debug_file = debugfs_create_file(debug_name, 0640,
						 hisi_gem_tracer->heaps_dir,
						 heap, &debug_wt_mark_fops);
		if (IS_ERR_OR_NULL(debug_file)) {
			path = dentry_path(hisi_gem_tracer->heaps_dir, buf,
					   DEBUG_FS_BUF_SIZE);
			pr_heaps_info(
				"Failed to create heap shrinker debugfs at %s/%s\n",
				path, debug_name);
		}
	}
}
#else
static inline int hisi_drm_heaps_init_debugfs(void)
{
	return 0;
}
#endif

/**
 * show gem dump info which file is dma_buf_file and dma_buf is drm's dma_buf
 * @data task_struct
 * @f task's open file
 * @fd file's fd
 */
static int hisi_drm_debug_process_cb(const void *data, struct file *f,
				     unsigned int fd)
{
	const struct task_struct *tsk = data;
	struct dma_buf *dma_buf = NULL;
	struct hisi_drm_gem *hisi_gem = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dma_buf = file_to_dma_buf(f);
	if (!dma_buf || !is_hisi_drm_dmabuf(dma_buf))
		return 0;

	hisi_gem = to_hisi_gem(dma_buf->priv);
	if (!hisi_gem)
		return 0;

	pr_heaps_err("%16s %16u %16u %#16lx %16u %16s %16u\n", tsk->comm,
		     tsk->pid, fd, dma_buf->size, hisi_gem->pid,
		     hisi_gem->task_comm, hisi_gem->heap_id);
	return 0;
}

/**
 * when alloc limit the time(maybe 500ms), show all task info which contains DRM
 * dma_buf
 */
void hisi_drm_heaps_process_show(void)
{
	struct task_struct *tsk = NULL;

	pr_heaps_err("Process HISI DRM Heap info:\n");
	pr_heaps_err("%16s %16s %16s %16s %16s %16s %16s\n", "Process name",
		     "Process ID", "FD", "Size(0x)", "PID", "Task Name", "Heap ID");
	rcu_read_lock();
	for_each_process (tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;

		task_lock(tsk);
		iterate_fd(tsk->files, 0, hisi_drm_debug_process_cb,
			   (void *)tsk);
		task_unlock(tsk);
	}
	rcu_read_unlock();
}

void hisi_drm_gem_info_show(struct hisi_drm_gem *hisi_gem)
{
	if (unlikely(!hisi_gem)) {
		pr_heaps_err("invalid gem input\n");
		return;
	}

	pr_heaps_err("%16s %16s %16s %16s %16s\n", "Process name", "Process ID",
		     "Size(0x)", "Kmap_cnt", "Heap Id");
	pr_heaps_err("%16s %16u %#16lx %16u %16u\n", hisi_gem->task_comm,
		     hisi_gem->pid, hisi_gem->size, hisi_gem->kmap_cnt,
		     hisi_gem->heap_id);
}

int hisi_drm_heaps_tracer_init(void)
{
	int ret = 0;

	hisi_gem_tracer = kzalloc(sizeof(*hisi_gem_tracer), GFP_KERNEL);
	if (!hisi_gem_tracer)
		return -ENOMEM;

	hisi_gem_tracer->gems = RB_ROOT;

	spin_lock_init(&hisi_gem_tracer->rb_lock);

	ret = hisi_drm_heaps_init_debugfs();
	if (ret)
		return ret;

	return ret;
}
