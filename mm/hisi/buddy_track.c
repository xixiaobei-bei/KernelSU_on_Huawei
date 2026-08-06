/*
 * mm/hisi/buddy_track.c
 *
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

#include <linux/stddef.h>
#include <linux/hisi/mem_trace.h>
#include <linux/memory.h>
#include <linux/mmzone.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

/*
 * the first 4 bits used to record order,
 * the rest bits used to record caller.
 */
struct funcmap {
	unsigned long caller;
};

struct buddy_node {
	struct rb_node node;
	unsigned long caller;
	atomic_t ref;
	int type;
};

static struct funcmap *func_map;
static int buddy_track_on;
static int buddy_track_open;
static struct mutex buddy_mutex;
static struct rb_root buddy_rb;
static DEFINE_SPINLOCK(buddy_lock);
static int buddy_global_init;

#define ORDER_SHIFT          60UL
#define CALLER_MASK          (0xFUL << ORDER_SHIFT)
#define page_size_up(size) (((size) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define fmap_size(size) (page_size_up(size) * sizeof(struct funcmap))

#define page_exc(p) (!PageLslub(p) && !PageSKB(p) && !PageION(p) \
			 && !PageVmalloc(p) && !PageZspage(p)   \
			&& !PageReserved(p) && !PageLRU(p))    \

#define page_track(page) (PageDrv(page) && page_exc(page))

#define buddy_pfn_track(pfn)  page_track(pfn_to_page(pfn))
#define lslub_pfn_track(pfn)  PageLslub(pfn_to_page(pfn))
#define invalid_buddy_type(type) (type != BUDDY_TRACK  \
				&& type != LSLUB_TRACK)

static inline struct funcmap *page_to_funcmap(struct page *page)
{
	return (func_map + page_to_pfn(page));/*lint !e704*/
}

void set_buddy_track(struct page *page,
				unsigned int order, unsigned long caller)
{
	unsigned long flags;
	unsigned long __caller;

	if (!buddy_track_on || !buddy_global_init || !page)
		return;

	__caller = caller & ~CALLER_MASK;
	__caller |= (unsigned long)order << ORDER_SHIFT;
	spin_lock_irqsave(&buddy_lock, flags);
	if (likely(buddy_track_on))
		page_to_funcmap(page)->caller = __caller;
	spin_unlock_irqrestore(&buddy_lock, flags);
	SetPageDrv(page);
}

void set_lslub_track(struct page *page,
				unsigned int order, unsigned long caller)
{
	unsigned long flags;
	unsigned long __caller;

	if (!buddy_track_on  || !buddy_global_init || !page)
		return;

	__caller = caller & ~CALLER_MASK;
	__caller |= (unsigned long)order << ORDER_SHIFT;
	spin_lock_irqsave(&buddy_lock, flags);
	if (likely(buddy_track_on))
		page_to_funcmap(page)->caller = __caller;
	spin_unlock_irqrestore(&buddy_lock, flags);
	SetPageLslub(page);
}

static unsigned long get_buddy_caller(unsigned long pfn)
{
	return page_to_funcmap(pfn_to_page(pfn))->caller;/*lint !e704*/
}

static void buddy_track_info_dump(unsigned long pfn)
{
	unsigned long caller;

	if (buddy_pfn_track(pfn) || lslub_pfn_track(pfn)) {/*lint !e704*/
		caller = get_buddy_caller(pfn);
		if (caller)
			pr_err("pfn:0x%lx,caller:%pS,order:%lu\n",
			pfn, (void *)(caller | CALLER_MASK),
			caller >> ORDER_SHIFT);
	}
}

void buddy_track_show(void)
{
	unsigned long pfn;
	struct memblock_region *reg = NULL;
	unsigned long start, end;

	for_each_memblock(memory, reg) {
		start = __phys_to_pfn(reg->base);
		end = __phys_to_pfn(reg->base + reg->size);
		for (pfn = start; pfn < end && pfn_valid(pfn); pfn++)
			buddy_track_info_dump(pfn);
	}
}

int buddy_track_map(int nid)
{
	phys_addr_t size;
	unsigned long flags;
	void *base = NULL;
#ifndef CONFIG_HISI_MEM_OFFLINE
	size = fmap_size(memblock_end_of_DRAM() - memblock_start_of_DRAM());
#else
	size = fmap_size(bootloader_memory_limit - memblock_start_of_DRAM());
#endif
	base = memblock_virt_alloc_node_nopanic(size, nid);
	if (!base)
		return -ENOMEM;
	func_map = base;

	spin_lock_irqsave(&buddy_lock, flags);
	buddy_track_on = 1;
	spin_unlock_irqrestore(&buddy_lock, flags);

	return 0;
}

int buddy_track_unmap(void)
{
	phys_addr_t i, size;
	unsigned long flags;
	struct page *p = NULL;

	if (!func_map)
		return -EINVAL;

	spin_lock_irqsave(&buddy_lock, flags);
	if (!buddy_track_on) {
		spin_unlock_irqrestore(&buddy_lock, flags);
		return 0;
	}

	buddy_track_on = 0;
	spin_unlock_irqrestore(&buddy_lock, flags);
	p = virt_to_page(func_map);
#ifndef CONFIG_HISI_MEM_OFFLINE
	size = fmap_size(memblock_end_of_DRAM() - memblock_start_of_DRAM());
#else
	size = fmap_size(bootloader_memory_limit - memblock_start_of_DRAM());
#endif
	for (i = 0; (i < (size >> PAGE_SHIFT)) && p; i++, p++) {
		__ClearPageReserved(p);
		__free_pages(p, 0);
	}
	return 0;
}

static inline void buddy_buid_rbtree(void)
{
	buddy_rb = RB_ROOT;
}

static void buddy_del_node(struct buddy_node *rbnode)
{
	rb_erase(&rbnode->node, &buddy_rb);
	kfree(rbnode);
}

static void buddy_add_node(unsigned long caller, int type)
{
	struct rb_node **p;
	struct rb_node *parent = NULL;
	struct buddy_node *entry = NULL;
	struct buddy_node *rbnode = NULL;

	p = &buddy_rb.rb_node;
	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct buddy_node, node);
		if (caller < entry->caller) {
			p = &(*p)->rb_left;
		} else if (caller > entry->caller) {
			p = &(*p)->rb_right;
		} else {
			atomic_inc(&entry->ref);
			return;
		}
	}
	rbnode = kzalloc(sizeof(*rbnode), GFP_KERNEL);
	if (rbnode) {
		atomic_inc(&rbnode->ref);
		rbnode->caller = caller;
		rbnode->type = type;
		rb_link_node(&rbnode->node, parent, p);
		rb_insert_color(&rbnode->node, &buddy_rb);
	}
}

static void buddy_add_caller(int type, unsigned long pfn)
{
	int i;
	unsigned long caller;

	caller = get_buddy_caller(pfn);
	if (!caller)
		return;
	for (i = 0; i < (1 << (caller >> ORDER_SHIFT)); i++)
		buddy_add_node(caller | CALLER_MASK, type);
}

static void buddy_fetch_stack(int type)
{
	unsigned long pfn;
	struct memblock_region *reg = NULL;
	unsigned long start, end;

	if (invalid_buddy_type(type))
		return;

	for_each_memblock(memory, reg) {
		start = __phys_to_pfn(reg->base);
		end = __phys_to_pfn(reg->base + reg->size);
		for (pfn = start; pfn < end && pfn_valid(pfn); pfn++) {
			if (buddy_pfn_track(pfn) || lslub_pfn_track(pfn)) /*lint !e704*/
				buddy_add_caller(type, pfn);
		}
	}
}

static size_t buddy_read_node(
	struct mm_stack_info *buf, size_t len, int type)
{
	struct rb_node *n = NULL;
	struct buddy_node *vnode = NULL;
	size_t length = len;
	size_t cnt = 0;

	if (!buf)
		return 0;

	n = rb_first(&buddy_rb);
	while (n && length) {
		vnode = rb_entry(n, struct buddy_node, node);
		if (type != vnode->type) {
			n = rb_next(n);
			continue;
		}
		(buf + cnt)->caller = vnode->caller;
		(buf + cnt)->ref.counter = vnode->ref.counter;
		n = rb_next(n);
		cnt++;
		length--;
		buddy_del_node(vnode);
	}
	return cnt;
}

int hisi_buddy_stack_open(int type)
{
	pr_err("into %s, type:%d\n", __func__, type);

	if (invalid_buddy_type(type))
		return -EINVAL;

	if (!buddy_track_on)
		return -EINVAL;

	mutex_lock(&buddy_mutex);
	if (buddy_track_open) {
		mutex_unlock(&buddy_mutex);
		return 0;
	}
	buddy_fetch_stack(type);
	buddy_track_open = 1;
	mutex_unlock(&buddy_mutex);

	return 0;
}

int hisi_buddy_stack_close(void)
{
	mutex_lock(&buddy_mutex);
	buddy_track_open = 0;
	mutex_unlock(&buddy_mutex);
	return 0;
}

size_t hisi_buddy_stack_read(
	struct mm_stack_info *buf, size_t len, int type)
{
	size_t cnt;

	if (invalid_buddy_type(type))
		return 0;
	mutex_lock(&buddy_mutex);
	if (!buddy_track_open) {
		mutex_unlock(&buddy_mutex);
		return 0;
	}
	cnt = buddy_read_node(buf, len, type);
	mutex_unlock(&buddy_mutex);
	return cnt;
}

static int __init buddy_track_init(void)
{
	mutex_init(&buddy_mutex);
	buddy_buid_rbtree();

	/* notice:buddy work before late_initcall */
	buddy_global_init = 1;

	return 0;
}

late_initcall(buddy_track_init);
