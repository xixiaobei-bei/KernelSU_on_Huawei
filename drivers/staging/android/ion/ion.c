/*
 *
 * drivers/staging/android/ion/ion.c
 *
 * Copyright (C) 2011 Google, Inc.
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

#define pr_fmt(fmt) "[ION: ]" fmt

#include <linux/device.h>
#include <linux/err.h>
#include <linux/file.h>
#include <linux/freezer.h>
#include <linux/fs.h>
#include <linux/anon_inodes.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/memblock.h>
#include <linux/miscdevice.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/dma-buf.h>
#include <linux/idr.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/platform_device.h>
#include <linux/sched/task.h>
#include <linux/time.h>

#ifdef CONFIG_HISI_LB
#include <linux/hisi/hisi_lb.h>
#endif
#include <linux/fdtable.h>
#include <linux/sched/signal.h>

#include "ion.h"
#include "hisi_ion_priv.h"

#define HISI_ION_FLUSH_ALL_CPUS_CACHES	0x800000 /*8MB*/
#define HISI_ION_ALLOC_MAX_LATENCY_US	(500 * 1000) /* 500ms */

static struct ion_device *internal_dev;
static atomic_long_t ion_total_size;
static atomic_long_t ion_magic;

/* this function should only be called while dev->lock is held */
static void ion_buffer_add(struct ion_device *dev,
			   struct ion_buffer *buffer)
{
	struct rb_node **p = &dev->buffers.rb_node;
	struct rb_node *parent = NULL;
	struct ion_buffer *entry;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct ion_buffer, node);

		if (buffer < entry) {
			p = &(*p)->rb_left;
		} else if (buffer > entry) {
			p = &(*p)->rb_right;
		} else {
			pr_err("%s: buffer already found.", __func__);
			BUG();/*lint !e146*/
		}
	}

	rb_link_node(&buffer->node, parent, p);
	rb_insert_color(&buffer->node, &dev->buffers);
}

void  init_dump(struct ion_buffer *buffer)
{
	struct task_struct *task = NULL;
	pid_t pid;

	get_task_struct(current->group_leader);
	task_lock(current->group_leader);
	pid = task_pid_nr(current->group_leader);
	/*
	 * don't bother to store task struct for kernel threads,
	 * they can't be killed anyway
	 */
	if (current->group_leader->flags & PF_KTHREAD) {
		task = NULL;
	} else {
		task = current->group_leader;
	}
	task_unlock(current->group_leader);
	put_task_struct(current->group_leader);

	if (!task) {
		/*
		 * record ion_alloc from kernel with
		 * name "invalid task" and pid 0;
		 */
		strncpy(buffer->task_comm, "invalid task",
		 sizeof(buffer->task_comm));
		buffer->pid = 0;
	} else {
		get_task_comm(buffer->task_comm, task);
		buffer->pid = task_pid_nr(task);
	}
}

/* this function should only be called while dev->lock is held */
/*lint -e578 -e574*/
static struct ion_buffer *ion_buffer_create(struct ion_heap *heap,
					    struct ion_device *dev,
					    unsigned long len,
					    unsigned long flags)
{
	struct ion_buffer *buffer;
	struct sg_table *table;
	struct scatterlist *sg;
	int ret;
	int i;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	buffer->magic = atomic64_inc_return(&ion_magic);
	buffer->heap = heap;
	buffer->flags = flags;

	ret = heap->ops->allocate(heap, buffer, len, flags);

	if (ret) {
		if (!(heap->flags & ION_HEAP_FLAG_DEFER_FREE))
			goto err2;

		ion_heap_freelist_drain(heap, 0);
		ret = heap->ops->allocate(heap, buffer, len, flags);
		if (ret)
			goto err2;
	}

	if (!buffer->sg_table) {
		WARN_ONCE(1, "This heap needs to set the sgtable");
		ret = -EINVAL;
		goto err2;
	}

	table = buffer->sg_table;
	buffer->dev = dev;
	buffer->size = len;

	buffer->dev = dev;
	buffer->size = len;
	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);

	/*
	 * this will set up dma addresses for the sglist -- it is not
	 * technically correct as per the dma api -- a specific
	 * device isn't really taking ownership here.  However, in practice on
	 * our systems the only dma_address space is physical addresses.
	 * Additionally, we can't afford the overhead of invalidating every
	 * allocation via dma_map_sg. The implicit contract here is that
	 * memory coming from the heaps is ready for dma, ie if it has a
	 * cached mapping that mapping has been invalidated
	 */
	for_each_sg(buffer->sg_table->sgl, sg, buffer->sg_table->nents, i) {
		sg_dma_address(sg) = sg_phys(sg);
		sg_dma_len(sg) = sg->length;
#ifdef CONFIG_HISI_PAGE_TRACE
		SetPageION(sg_page(sg));
#endif
	}

	if (buffer->heap->type != ION_HEAP_TYPE_CARVEOUT)
		atomic_long_add(buffer->size, &ion_total_size);

#ifdef CONFIG_HISI_LB
	if (flags & ION_FLAG_HISI_LB_MASK) {
		buffer->plc_id = ION_FLAG_2_PLC_ID(flags);
		pr_info("HISI ION LB %lx\n", flags);
		/*
		 * will inv cache with normal va,
		 * and need after set zero
		 */
		if (lb_sg_attach(buffer->plc_id, buffer->sg_table->sgl,
				 buffer->sg_table->nents))
			goto err1;
	}
#endif
	init_dump(buffer);
	mutex_lock(&dev->buffer_lock);
	ion_buffer_add(dev, buffer);
	mutex_unlock(&dev->buffer_lock);

	return buffer;

#ifdef CONFIG_HISI_LB
err1:
	heap->ops->free(buffer);
#endif
err2:
	kfree(buffer);
	return ERR_PTR(ret);
}
/*lint +e578 +e574*/
void ion_buffer_destroy(struct ion_buffer *buffer)
{
	if (buffer->heap->type != ION_HEAP_TYPE_CARVEOUT)
		atomic_long_sub(buffer->size, &ion_total_size);

	if (buffer->kmap_cnt > 0) {
		pr_warn_once("%s: buffer still mapped in the kernel\n",
			     __func__);
		buffer->heap->ops->unmap_kernel(buffer->heap, buffer);
	}

#ifdef CONFIG_HISI_LB
	/*
	 * will inv cache with gid va,
	 * and need before free
	 */
	if (buffer->plc_id)
		(void)lb_sg_detach(buffer->plc_id, buffer->sg_table->sgl,
			buffer->sg_table->nents);
#endif
	buffer->heap->ops->free(buffer);
	kfree(buffer);
}

static void _ion_buffer_destroy(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;
	struct ion_device *dev = buffer->dev;

	mutex_lock(&dev->buffer_lock);
	rb_erase(&buffer->node, &dev->buffers);
	mutex_unlock(&dev->buffer_lock);

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE)
		ion_heap_freelist_add(heap, buffer);
	else
		ion_buffer_destroy(buffer);
}

static void *ion_buffer_kmap_get(struct ion_buffer *buffer)
{
	void *vaddr;

	if (buffer->kmap_cnt) {
		buffer->kmap_cnt++;
		return buffer->vaddr;
	}
	vaddr = buffer->heap->ops->map_kernel(buffer->heap, buffer);
	if (WARN_ONCE(!vaddr,
		      "heap->ops->map_kernel should return ERR_PTR on error"))
		return ERR_PTR(-EINVAL);
	if (IS_ERR(vaddr))
		return vaddr;
	buffer->vaddr = vaddr;
	buffer->kmap_cnt++;
	return vaddr;
}

static void ion_buffer_kmap_put(struct ion_buffer *buffer)
{
	buffer->kmap_cnt--;
	if (!buffer->kmap_cnt) {
		buffer->heap->ops->unmap_kernel(buffer->heap, buffer);
		buffer->vaddr = NULL;
	}
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table;
	int ret, i;
	struct scatterlist *sg, *new_sg;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sg(table->sgl, sg, table->nents, i) {/*lint !e574*/
		memcpy(new_sg, sg, sizeof(*sg));
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static void free_duped_table(struct sg_table *table)
{
	sg_free_table(table);
	kfree(table);
}

struct ion_dma_buf_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
};

static int ion_dma_buf_attach(struct dma_buf *dmabuf, struct device *dev,
			      struct dma_buf_attachment *attachment)
{
	struct ion_dma_buf_attachment *a;
	struct sg_table *table;
	struct ion_buffer *buffer = dmabuf->priv;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	table = dup_sg_table(buffer->sg_table);
	if (IS_ERR(table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->table = table;
	a->dev = dev;
	INIT_LIST_HEAD(&a->list);

	attachment->priv = a;

	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	return 0;
}

static void ion_dma_buf_detatch(struct dma_buf *dmabuf,
				struct dma_buf_attachment *attachment)
{
	struct ion_dma_buf_attachment *a = attachment->priv;
	struct ion_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);
	free_duped_table(a->table);

	kfree(a);
}

static struct sg_table *ion_map_dma_buf(struct dma_buf_attachment *attachment,
					enum dma_data_direction direction)
{
	struct ion_dma_buf_attachment *a = attachment->priv;
	struct sg_table *table;

	table = a->table;

	return table;
}

static void ion_unmap_dma_buf(struct dma_buf_attachment *attachment,
			      struct sg_table *table,
			      enum dma_data_direction direction)
{
}

static int ion_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct ion_buffer *buffer = dmabuf->priv;
	int ret = 0;

	if (!buffer->heap->ops->map_user) {
		pr_err("%s: this heap does not define a method for mapping to userspace\n",
		       __func__);
		return -EINVAL;
	}

	if (!(buffer->flags & ION_FLAG_CACHED))
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	mutex_lock(&buffer->lock);
	/* now map it to userspace */
	ret = buffer->heap->ops->map_user(buffer->heap, buffer, vma);
	mutex_unlock(&buffer->lock);

	if (ret)
		pr_err("%s: failure mapping buffer to userspace\n",
		       __func__);

	return ret;
}

static void ion_dma_buf_release(struct dma_buf *dmabuf)
{
	struct ion_buffer *buffer = dmabuf->priv;

	_ion_buffer_destroy(buffer);
}

static void *ion_dma_buf_kmap(struct dma_buf *dmabuf, unsigned long offset)
{
	struct ion_buffer *buffer = dmabuf->priv;
	void *vaddr = NULL;

	if (buffer->heap->ops->map_kernel) {
		mutex_lock(&buffer->lock);
		vaddr = ion_buffer_kmap_get(buffer);
		mutex_unlock(&buffer->lock);
		if (IS_ERR(vaddr))
			return NULL;

		return vaddr + offset * PAGE_SIZE;
	}

	return NULL;
}

static void ion_dma_buf_kunmap(struct dma_buf *dmabuf, unsigned long offset,
			       void *ptr)
{
	struct ion_buffer *buffer = dmabuf->priv;

	if (buffer->heap->ops->map_kernel) {
		mutex_lock(&buffer->lock);
		ion_buffer_kmap_put(buffer);
		mutex_unlock(&buffer->lock);
	}
}

static int ion_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction direction)
{
	struct ion_buffer *buffer = dmabuf->priv;
	struct platform_device *hisi_ion_dev = get_hisi_ion_platform_device();
	struct sg_table *table = buffer->sg_table;

	if (dmabuf->size >= HISI_ION_FLUSH_ALL_CPUS_CACHES) {
		ion_flush_all_cpus_caches();
	} else {
		mutex_lock(&buffer->lock);
		dma_sync_sg_for_cpu(&hisi_ion_dev->dev, table->sgl, table->nents,
				    direction);
		mutex_unlock(&buffer->lock);
	}

	return 0;
}

static int ion_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
				      enum dma_data_direction direction)
{
	struct ion_buffer *buffer = dmabuf->priv;
	struct platform_device *hisi_ion_dev = get_hisi_ion_platform_device();
	struct sg_table *table = buffer->sg_table;

	if (dmabuf->size >= HISI_ION_FLUSH_ALL_CPUS_CACHES) {
		ion_flush_all_cpus_caches();
	} else {
		mutex_lock(&buffer->lock);
		dma_sync_sg_for_device(&hisi_ion_dev->dev, table->sgl, table->nents,
				       direction);
		mutex_unlock(&buffer->lock);
	}

	return 0;
}

static const struct dma_buf_ops dma_buf_ops = {
	.map_dma_buf = ion_map_dma_buf,
	.unmap_dma_buf = ion_unmap_dma_buf,
	.mmap = ion_mmap,
	.release = ion_dma_buf_release,
	.attach = ion_dma_buf_attach,
	.detach = ion_dma_buf_detatch,
	.begin_cpu_access = ion_dma_buf_begin_cpu_access,
	.end_cpu_access = ion_dma_buf_end_cpu_access,
	.map_atomic = ion_dma_buf_kmap,
	.unmap_atomic = ion_dma_buf_kunmap,
	.map = ion_dma_buf_kmap,
	.unmap = ion_dma_buf_kunmap,
};

int ion_alloc(size_t len, unsigned int heap_id_mask, unsigned int flags)
{
	ktime_t _stime, _etime, _htime;
	s64 _timedelta;
	s64 _htimedelta;
	struct ion_device *dev = internal_dev;
	struct ion_buffer *buffer = NULL;
	struct ion_heap *heap;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	int fd;
	struct dma_buf *dmabuf;
	struct platform_device *hisi_ion_dev = get_hisi_ion_platform_device();
	long msleep_count_s;
	long msleep_count_e;
	unsigned long kernel_stack;
	unsigned long free_cma;
	unsigned long free_page;
	unsigned long nr_inactive_file;
	unsigned long nr_isolated_anon;
	unsigned long nr_isolated_file;

	pr_debug("%s: len %zu heap_id_mask %u flags %x\n", __func__,
		 len, heap_id_mask, flags);

	msleep_count_s = atomic64_read(&shrink_msleep_count);
	kernel_stack = global_zone_page_state(NR_KERNEL_STACK_KB);
	free_cma = global_zone_page_state(NR_FREE_CMA_PAGES);
	free_page = global_zone_page_state(NR_FREE_PAGES);
	nr_inactive_file = global_node_page_state(NR_INACTIVE_FILE);
	nr_isolated_anon = global_node_page_state(NR_ISOLATED_ANON);
	nr_isolated_file = global_node_page_state(NR_ISOLATED_FILE);

	/*
	 * traverse the list of heaps available in this system in priority
	 * order.  If the heap type is supported by the client, and matches the
	 * request of the caller allocate from it.  Repeat until allocate has
	 * succeeded or all heaps have been tried
	 */
	len = PAGE_ALIGN(len);

	if (!len)
		return -EINVAL;

	if (len > SZ_1G) {
		pr_err("%s: size more than 1G(0x%lx), pid: %d, process name: %s\n",
			__func__, len, current->pid, current->comm);
		return -ENOMEM;
	}

	if (len > (SZ_2M * 100))
		pr_err("%s: len > 200MB, inactive_file:%lu, isolated_file:%lu, isolated_anon:%lu, free pages:%lu, kernel_stack:%luKB, free_cma:%lu\n",
			__func__, nr_inactive_file, nr_isolated_file, nr_isolated_anon, free_page, kernel_stack, free_cma);

	_stime = ktime_get();
	down_read(&dev->lock);

	_htime = ktime_get();
	_htimedelta = ktime_us_delta(_htime, _stime);

	if (heap_id_mask == ION_ALL_HEAP)
		heap_id_mask = 0x1U << ION_SYSTEM_HEAP_ID;

	if (heap_id_mask & 0x1U << ION_IRIS_HEAP_ID)
		heap_id_mask |= 0x1U << ION_IRIS_DAEMON_HEAP_ID;

	if ((heap_id_mask & 0x1U << ION_CAMERA_HEAP_ID)) {
		heap_id_mask |= 0x1U << ION_CAMERA_DAEMON_HEAP_ID;
		flags |= (ION_FLAG_NO_SHRINK_BUFFER |
			ION_FLAG_ALLOC_NOWARN_BUFFER);
#ifdef CONFIG_ION_HISI_SUPPORT_SUBBIT
		heap_id_mask |= 0x1U << ION_IRIS_HEAP_ID;
		flags |= ION_FLAG_SECURE_BUFFER;
#endif
	}

#ifdef CONFIG_ION_HISI_CPA
	if (heap_id_mask & 0x1U << ION_DRM_HEAP_ID)
		heap_id_mask |= 0x1U << ION_DRM_CPA_HEAP_ID;
#endif

	plist_for_each_entry(heap, &dev->heaps, node) {
		/* if the caller didn't specify this heap id */
		if (!((1 << heap->id) & heap_id_mask))
			continue;
		buffer = ion_buffer_create(heap, dev, len, flags);
		if (!IS_ERR(buffer))
			break;
	}
	up_read(&dev->lock);

	_etime = ktime_get();
	_timedelta = ktime_us_delta(_etime, _stime);
	if (_timedelta > HISI_ION_ALLOC_MAX_LATENCY_US) {
		msleep_count_e = atomic64_read(&shrink_msleep_count);
		pr_err("%s: alloc size=%zu, heap_id_mask=0x%x, flags=0x%x, htime cost=%lld, time cost=%lld\n",
			__func__, len, heap_id_mask, flags, _htimedelta, _timedelta);
		pr_err("%s: alloc timeout, inactive_file:%lu, isolated_file:%lu, isolated_anon:%lu, free pages:%lu, kernel_stack:%luKB, free_cma:%lu, msleep_count_s:%ld, msleep_count_e:%ld\n",
			__func__, nr_inactive_file, nr_isolated_file, nr_isolated_anon, free_page, kernel_stack, free_cma, msleep_count_s, msleep_count_e);
		hisi_ion_proecss_info();
		show_mem(0, NULL);
	}

	if (!buffer)
		return -ENODEV;

	if (IS_ERR(buffer))
		return PTR_ERR(buffer);

	if (!(flags & ION_FLAG_CACHED) && !(flags & ION_FLAG_SECURE_BUFFER)) {
		if (len >= HISI_ION_FLUSH_ALL_CPUS_CACHES)
			ion_flush_all_cpus_caches();
		else
			dma_sync_sg_for_cpu(&hisi_ion_dev->dev,
					    buffer->sg_table->sgl,
					    buffer->sg_table->nents,
					    DMA_FROM_DEVICE);
	}

	exp_info.ops = &dma_buf_ops;
	exp_info.size = buffer->size;
	exp_info.flags = O_RDWR;
	exp_info.priv = buffer;

	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		_ion_buffer_destroy(buffer);
		return PTR_ERR(dmabuf);
	}

	fd = dma_buf_fd(dmabuf, O_CLOEXEC);
	if (fd < 0) {
		pr_err("can't get valid fd(%d) to install\n", fd);
		dma_buf_put(dmabuf);
	}

	return fd;
}

struct ion_device *get_ion_device(void)
{
	return internal_dev;
}

unsigned long get_ion_total_size(void)
{
	return (unsigned long)atomic_long_read(&ion_total_size);
}

/**
 * is_ion_dma_buf - check if the dmabuf is coherent with a ION buffer
 * @dmabuf:		dma_buf structure
 * Returns
 *   1- when dma_buf is coherent with a ION buffer
 *   0- when dma_buf is not coherent with a ION buffer
 */

bool is_ion_dma_buf(struct dma_buf *dmabuf)
{
	if ((dmabuf != NULL) && (dmabuf->ops != NULL)
			&& (dmabuf->ops == &dma_buf_ops))
		return true;
	return false;
}

int ion_query_heaps(struct ion_heap_query *query)
{
	struct ion_device *dev = internal_dev;
	struct ion_heap_data __user *buffer = u64_to_user_ptr(query->heaps);
	int ret = -EINVAL, cnt = 0, max_cnt;
	struct ion_heap *heap;
	struct ion_heap_data hdata;

	memset(&hdata, 0, sizeof(hdata));

	down_read(&dev->lock);
	if (!buffer) {
		query->cnt = dev->heap_cnt;
		ret = 0;
		goto out;
	}

	if (query->cnt <= 0)
		goto out;

	max_cnt = query->cnt;

	plist_for_each_entry(heap, &dev->heaps, node) {
		strncpy(hdata.name, heap->name, MAX_HEAP_NAME);
		hdata.name[sizeof(hdata.name) - 1] = '\0';
		hdata.type = heap->type;
		hdata.heap_id = heap->id;

		if (copy_to_user(&buffer[cnt], &hdata, sizeof(hdata))) {
			ret = -EFAULT;
			goto out;
		}

		cnt++;
		if (cnt >= max_cnt)
			break;
	}

	query->cnt = cnt;
	ret = 0;
out:
	up_read(&dev->lock);
	return ret;
}

static const struct file_operations ion_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = ion_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= ion_ioctl,
#endif
};

struct ion_debug_process_heap_args {
	struct seq_file *seq;
	struct task_struct *tsk;
	size_t *total_ion_size;
};

static int ion_debug_process_heap_cb(const void *data,
				struct file *f, unsigned int fd)
{
	const struct ion_debug_process_heap_args *args = data;
	struct task_struct *tsk = args->tsk;
	struct dma_buf    *dbuf;
	struct ion_buffer *ibuf;

	if (!is_dma_buf_file(f))
		return 0;

	dbuf = file_to_dma_buf(f);
	if (!dbuf)
		return 0;

	if (dbuf->owner != THIS_MODULE)
		return 0;

	ibuf = dbuf->priv;
	if (!ibuf)
		return 0;

	*args->total_ion_size += dbuf->size;
	seq_printf(args->seq,
		"%16s %16u %16u %16zu %16llu %16u %16s\n",
		tsk->comm, tsk->pid,
		fd, dbuf->size, ibuf->magic,
		ibuf->pid, ibuf->task_comm);

	return 0;
}

static int ion_debug_process_heap_show(struct seq_file *s, void *d)
{
	struct task_struct *tsk = NULL;
	size_t task_total_ion_size;
	struct ion_debug_process_heap_args cb_args;

	seq_puts(s, "Process ION heap info:\n");
	seq_puts(s, "----------------------------------------------------\n");
	seq_printf(s, "%16s %16s %16s %16s %16s %16s %16s\n",
			"Process name", "Process ID",
			"fd", "size",
			"magic", "buf->pid",
			"buf->task_comm");

	rcu_read_lock();
	for_each_process(tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;

		task_total_ion_size = 0;
		cb_args.seq = s;
		cb_args.tsk = tsk;
		cb_args.total_ion_size = &task_total_ion_size;

		task_lock(tsk);
		iterate_fd(tsk->files, 0,
			   ion_debug_process_heap_cb, (void *)&cb_args);
		if (task_total_ion_size > 0) {
			seq_printf(s, "%16s %-16s %16zu\n",
				"Total Ion size of ",
				tsk->comm, task_total_ion_size);
		}
		task_unlock(tsk);
	}
	rcu_read_unlock();

	seq_puts(s, "----------------------------------------------------\n");
	return 0;
}

static int ion_debug_process_heap_open(struct inode *inode, struct file *file)
{
	return single_open(file, ion_debug_process_heap_show, inode->i_private);
}

static const struct file_operations debug_process_heap_fops = {
	.open = ion_debug_process_heap_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/*lint -e578*/
static int ion_debug_heap_show(struct seq_file *s, void *d)
{
	struct ion_heap *heap = s->private;
	struct ion_device *dev = heap->dev;
	struct rb_node *n = NULL;
	size_t total_size = 0;

	seq_printf(s, "%16s %10s %16s  %4s       %8s\n",
		"proc", "pid", "size", "map_cnt", "magic");
	seq_puts(s, "------------------------------------------------------------------------\n");
	mutex_lock(&dev->buffer_lock);
	for (n = rb_first(&dev->buffers); n; n = rb_next(n)) {
		struct ion_buffer *buffer = rb_entry(n, struct ion_buffer,
						     node);
		if (buffer->heap->id != heap->id)
			continue;
		total_size += buffer->size;

		seq_printf(s, "%16s %10u %16zu %4d %16llu\n",
			   buffer->task_comm, buffer->pid,
			   buffer->size, buffer->kmap_cnt, buffer->magic);
	}
	mutex_unlock(&dev->buffer_lock);
	seq_puts(s, "------------------------------------------------------------------------\n");
	seq_printf(s, "%16s %16zu\n", "total ", total_size);
	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE)
		seq_printf(s, "%16s %16zu\n", "deferred free",
			   heap->free_list_size);
	seq_puts(s, "------------------------------------------------------------------------\n");

	if (heap->debug_show)
		heap->debug_show(heap, s, d);

	return 0;
}
/*lint +e578*/
static int ion_debug_heap_open(struct inode *inode, struct file *file)
{
	return single_open(file, ion_debug_heap_show, inode->i_private);
}

static const struct file_operations debug_heap_fops = {
	.open = ion_debug_heap_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int debug_shrink_set(void *data, u64 val)
{
	struct ion_heap *heap = data;
	struct shrink_control sc;
	int objs;

	sc.gfp_mask = GFP_HIGHUSER;
	sc.nr_to_scan = val;

	if (!val) {
		objs = heap->shrinker.count_objects(&heap->shrinker, &sc);
		sc.nr_to_scan = objs;
	}

	heap->shrinker.scan_objects(&heap->shrinker, &sc);
	return 0;
}

static int debug_shrink_get(void *data, u64 *val)
{
	struct ion_heap *heap = data;
	struct shrink_control sc;
	int objs;

	sc.gfp_mask = GFP_HIGHUSER;
	sc.nr_to_scan = 0;

	objs = heap->shrinker.count_objects(&heap->shrinker, &sc);
	*val = objs;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debug_shrink_fops, debug_shrink_get,
			debug_shrink_set, "%llu\n");
/*lint -e501 */
void ion_device_add_heap(struct ion_heap *heap)
{
	struct dentry *debug_file;
	struct ion_device *dev = internal_dev;

	if (!heap->ops->allocate || !heap->ops->free)
		pr_err("%s: can not add heap with invalid ops struct.\n",
		       __func__);

	spin_lock_init(&heap->free_lock);
	heap->free_list_size = 0;

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE)
		ion_heap_init_deferred_free(heap);

	if ((heap->flags & ION_HEAP_FLAG_DEFER_FREE) || heap->ops->shrink)
		ion_heap_init_shrinker(heap);

	heap->dev = dev;
	down_write(&dev->lock);
	/*
	 * use negative heap->id to reverse the priority -- when traversing
	 * the list later attempt higher id numbers first
	 */
	plist_node_init(&heap->node, -heap->id);
	plist_add(&heap->node, &dev->heaps);
	debug_file = debugfs_create_file(heap->name, 0660,
					 dev->heaps_debug_root, heap,
					 &debug_heap_fops);

	if (!debug_file) {
		char buf[256], *path;

		path = dentry_path(dev->heaps_debug_root, buf, 256);
		pr_err("Failed to create heap debugfs at %s/%s\n",
		       path, heap->name);
	}
	if (heap->shrinker.count_objects && heap->shrinker.scan_objects) {
		char debug_name[64];

		snprintf(debug_name, 64, "%s_shrink", heap->name);
		debug_file = debugfs_create_file(
			debug_name, 0640, dev->debug_root, heap,
			&debug_shrink_fops);
		if (!debug_file) {
			char buf[256], *path;

			path = dentry_path(dev->debug_root, buf, 256);
			pr_err("Failed to create heap shrinker debugfs at %s/%s\n",
			       path, debug_name);
		}
	}

	dev->heap_cnt++;
	up_write(&dev->lock);
}
EXPORT_SYMBOL(ion_device_add_heap);

static int ion_memtrack_show(struct seq_file *s, void *d)
{
	pid_t pid;
	char task_comm[TASK_COMM_LEN];
	struct rb_node *n;
	int flag = 0;
	size_t size = 0;
	struct ion_device *dev = s->private;

	mutex_lock(&dev->buffer_lock);
	for (n = rb_first(&dev->buffers); n; n = rb_next(n)) {
		struct ion_buffer *buffer = rb_entry(n, struct ion_buffer,
				node);
		if (buffer->heap->type == ION_HEAP_TYPE_CARVEOUT)
			continue;

		if (buffer->heap->flags & ION_FLAG_SMMUV3_BUFFER) {
			if (!flag) {
				strncpy(task_comm, buffer->task_comm, TASK_COMM_LEN);
				pid = buffer->pid;
				flag = 1;
			}

			size += buffer->size;
			continue;
		}

		seq_printf(s, "%16.s %16u %16zu\n", buffer->task_comm,
						buffer->pid, buffer->size);

	}
	mutex_unlock(&dev->buffer_lock);

	if (size)
		seq_printf(s, "%16.s %16u %16zu\n", task_comm, pid, size);

	return 0;
}

static int ion_memtrack_open(struct inode *inode, struct file *file)
{
	return single_open(file,
			ion_memtrack_show, PDE_DATA(file_inode(file)));
}

static const struct file_operations memtrack_fops = {
	.open = ion_memtrack_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
/*lint +e501 */
static int ion_device_create(void)
{
	struct proc_dir_entry *entry;
	struct ion_device *idev;
	int ret;
	struct dentry *debug_file;

	idev = kzalloc(sizeof(*idev), GFP_KERNEL);
	if (!idev)
		return -ENOMEM;

	idev->dev.minor = MISC_DYNAMIC_MINOR;
	idev->dev.name = "ion";
	idev->dev.fops = &ion_fops;
	idev->dev.parent = NULL;
	ret = misc_register(&idev->dev);
	if (ret) {
		pr_err("ion: failed to register misc device.\n");
		kfree(idev);
		return ret;
	}

	idev->debug_root = debugfs_create_dir("ion", NULL);
	if (!idev->debug_root) {
		pr_err("ion: failed to create debugfs root directory.\n");
		goto debugfs_done;
	}
	idev->heaps_debug_root = debugfs_create_dir("heaps", idev->debug_root);
	if (!idev->heaps_debug_root) {
		pr_err("ion: failed to create debugfs heaps directory.\n");
		goto debugfs_done;
	}

	debug_file = debugfs_create_file(
		"process_heap_info", 0660, idev->heaps_debug_root, NULL,
		&debug_process_heap_fops);
	if (!debug_file) {
		char buf[256], *path;

		path = dentry_path(idev->heaps_debug_root, buf, 256);
		pr_err("Failed to create process heap debugfs at %s/%s\n",
			path, "process_hep_info");
	}

	entry = proc_create_data("memtrack", 0664,
				 NULL, &memtrack_fops, idev);
	if (!entry)
		pr_err("Failed to create heap debug memtrack\n");

debugfs_done:
	idev->buffers = RB_ROOT;
	mutex_init(&idev->buffer_lock);
	init_rwsem(&idev->lock);
	plist_head_init(&idev->heaps);
	internal_dev = idev;
	return 0;
}
subsys_initcall(ion_device_create);
