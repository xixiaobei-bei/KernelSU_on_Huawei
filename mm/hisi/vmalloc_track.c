/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: vmalloc_track.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 * vmalloc_track.c is included by vmalloc.c,
 * so it is the extension of vmalloc.c in fact.
 * This file records the vmalloc detail stats and stacktrace in rbtree.
 */

#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/hisi/mem_trace.h>

struct vm_node {
	struct rb_node node;
	unsigned long caller;
	void *addr;
	atomic_t ref;
};

struct vm_track {
	struct mutex v_mutex;
	struct rb_root v_rb;
};

#define VM_VALID_FLAG (VM_ALLOC | VM_IOREMAP | VM_MAP | VM_USERMAP)

static struct vm_track vm_track;
static int hisi_vmalloc_open;
static int vm_type[] = {VM_IOREMAP, VM_ALLOC, VM_MAP, VM_USERMAP};

static inline void vm_buid_rbtree(void)
{
	vm_track.v_rb = RB_ROOT;
}

static void vm_del_node(struct vm_node *rbnode)
{
	rb_erase(&rbnode->node, &vm_track.v_rb);
	kfree(rbnode);
}

static void vm_set_track(unsigned long caller)
{
	struct rb_node **p = NULL;
	struct rb_node *parent = NULL;
	struct vm_node *entry = NULL;
	struct vm_node *rbnode = NULL;

	p = &vm_track.v_rb.rb_node;
	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct vm_node, node);
		if (caller < entry->caller)
			p = &(*p)->rb_left;
		else if (caller > entry->caller)
			p = &(*p)->rb_right;
		else {
			atomic_inc(&entry->ref);
			return;
		}
	}
	rbnode = kzalloc(sizeof(*rbnode), GFP_KERNEL);
	if (rbnode) {
		atomic_inc(&rbnode->ref);
		rbnode->caller = caller;
		rb_link_node(&rbnode->node, parent, p);
		rb_insert_color(&rbnode->node, &vm_track.v_rb);
	}
}

size_t vm_type_detail_get(int subtype)
{
	struct vmap_area *va = NULL;
	struct vm_struct *vm = NULL;
	size_t len = 0;
	unsigned long type = (unsigned long)(unsigned int)subtype;

	if (!(type & VM_VALID_FLAG))
		return 0;

	spin_lock(&vmap_area_lock);
	list_for_each_entry(va, &vmap_area_list, list) {/*lint !e666*/
		if (!(va->flags & VM_VM_AREA))
			continue;
		vm = va->vm;
		if (!(vm->flags & type))
			continue;
		len += vm->size;
	}
	spin_unlock(&vmap_area_lock);
	return len;
}

static size_t vm_read_node(struct hisi_stack_info *buf, const size_t len)
{
	struct rb_node *n = NULL;
	struct vm_node *vnode = NULL;
	size_t length = len;
	size_t cnt = 0;

	if (!buf)
		return 0;

	n = rb_first(&vm_track.v_rb);
	while (n && (length--)) {
		vnode = rb_entry(n, struct vm_node, node);
		(buf + cnt)->caller = vnode->caller;
		(buf + cnt)->ref.counter = vnode->ref.counter;
		n = rb_next(n);
		cnt++;
		vm_del_node(vnode);
	}
	return cnt;
}

void vm_show_node(void)
{
	struct rb_node *n = NULL;
	struct vm_node *vnode = NULL;

	mutex_lock(&vm_track.v_mutex);
	for (n = rb_first(&vm_track.v_rb); n; n = rb_next(n)) {
		vnode = rb_entry(n, struct vm_node, node);
		if (vnode)
			pr_err("caller:%pS, ref:0x%x\n",
				(void *)vnode->caller, atomic_read(&vnode->ref));
	}
	mutex_unlock(&vm_track.v_mutex);
}

int hisi_vmalloc_stack_open(int subtype)
{
	struct vmap_area *va = NULL;
	struct vm_struct *vm = NULL;
	unsigned long type = (unsigned long)(unsigned int)subtype;

	if (!(type & VM_VALID_FLAG))
		return -EINVAL;

	mutex_lock(&vm_track.v_mutex);
	if (hisi_vmalloc_open) {
		mutex_unlock(&vm_track.v_mutex);
		return -EINVAL;
	}
	spin_lock(&vmap_area_lock);
	list_for_each_entry(va, &vmap_area_list, list) {/*lint !e666*/
		if (!(va->flags & VM_VM_AREA))
			continue;
		vm = va->vm;
		if (!(vm->flags & type))
			continue;
		vm_set_track((unsigned long)vm->caller);
	}
	spin_unlock(&vmap_area_lock);
	hisi_vmalloc_open = 1;
	mutex_unlock(&vm_track.v_mutex);
	return 0;
}

int hisi_vmalloc_stack_close(void)
{
	mutex_lock(&vm_track.v_mutex);
	hisi_vmalloc_open = 0;
	mutex_unlock(&vm_track.v_mutex);
	return 0;
}

size_t hisi_vmalloc_stack_read(
	struct hisi_stack_info *buf, size_t len, int type)
{
	size_t cnt;

	mutex_lock(&vm_track.v_mutex);
	if (!hisi_vmalloc_open) {
		mutex_unlock(&vm_track.v_mutex);
		return 0;
	}
	cnt = vm_read_node(buf, len);
	mutex_unlock(&vm_track.v_mutex);
	return cnt;
}

size_t hisi_get_vmalloc_detail(void *buf, size_t len)
{
	size_t i;
	size_t num = len;
	size_t size;
	struct hisi_vmalloc_detail_info *info =
		(struct hisi_vmalloc_detail_info *)buf;

	for (i = 0; i < ARRAY_SIZE(vm_type) && num--; i++) {/*lint !e514*/
		size = vm_type_detail_get(vm_type[i]);
		(info + i)->type = vm_type[i];
		(info + i)->size = size;
	}
	return i;
}

static int __init vm_track_init(void)
{
	mutex_init(&vm_track.v_mutex);
	vm_buid_rbtree();
	return 0;
}

late_initcall(vm_track_init);
