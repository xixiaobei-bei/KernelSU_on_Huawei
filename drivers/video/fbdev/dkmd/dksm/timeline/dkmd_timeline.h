/** @file
 * Copyright (c) 2020-2021, Dkmdlicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef DKMD_TIMELINE_H
#define DKMD_TIMELINE_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kref.h>

#include "dkmd_timeline_listener.h"

#define DKMD_SYNC_NAME_SIZE 64

struct dkmd_timeline {
	struct kref kref;
	char name[DKMD_SYNC_NAME_SIZE];
	uint32_t listening_isr_bit;
	uint32_t isr_unmask_bit;
	void *parent;

	spinlock_t value_lock;
	uint32_t next_value;
	uint64_t pt_value;
	uint32_t inc_step;

	struct notifier_block *notifier; // isr will notify timeline to hanle event

	spinlock_t list_lock;
	struct list_head listener_list; /* struct dkmd_timeline_listener */
};

static inline void dkmd_timeline_add_listener(struct dkmd_timeline *timeline, struct dkmd_timeline_listener *node)
{
	spin_lock(&timeline->list_lock);
	list_add_tail(&node->list_node, &timeline->listener_list);
	spin_unlock(&timeline->list_lock);
}

static inline void dkmd_timeline_del_listener(struct dkmd_timeline *timeline, struct dkmd_timeline_listener *node)
{
	spin_lock(&timeline->list_lock);
	list_del(&node->list_node);
	spin_unlock(&timeline->list_lock);
}

static inline void dkmd_timeline_inc_step(struct dkmd_timeline *timeline)
{
	spin_lock(&timeline->value_lock);
	++timeline->inc_step;
	spin_unlock(&timeline->value_lock);
}

uint64_t dkmd_timeline_get_pt_value(struct dkmd_timeline *timeline);
struct dkmd_timeline_listener *dkmd_timeline_alloc_listener(struct dkmd_timeline_listener_ops *listener_ops, void *listener_data, uint64_t value);
void dkmd_timline_init(struct dkmd_timeline *timeline, const char *name, void *parent, uint32_t listening_isr_bit);


#endif /* DKMD_TIMELINE_H */
