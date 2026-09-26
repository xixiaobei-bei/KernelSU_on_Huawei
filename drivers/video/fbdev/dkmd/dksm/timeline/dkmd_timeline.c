/** @file
 * Copyright (c) 2020-2020, Dkmdlicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/module.h>
#include "dkmd_timeline.h"
#include "dkmd_utils.h"

static int _timeline_step_pt_value(struct dkmd_timeline *timeline)
{
	spin_lock(&timeline->value_lock);

	dpu_pr_info("timeline pt_value=%u, inc_step=%u", timeline->pt_value, timeline->inc_step);
	if (timeline->inc_step == 0) {
		spin_unlock(&timeline->value_lock);
		return -1;
	}

	timeline->pt_value += timeline->inc_step;
	timeline->inc_step = 0;

	spin_unlock(&timeline->value_lock);
	return 0;
}

static int _timeline_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_timeline *tl = (struct dkmd_timeline *)data;
	struct dkmd_timeline_listener *listener_node = NULL;
	struct dkmd_timeline_listener *_node_ = NULL;
	bool is_signaled = false;

	if (action != tl->listening_isr_bit) {
		dpu_pr_info("action 0x%x is not equal to isr_unmask_bit 0x%x", action, tl->isr_unmask_bit);
		return 0;
	}

	/* pt_value doesn't need step */
	if (_timeline_step_pt_value(tl) < 0)
		return 0;

	list_for_each_entry_safe(listener_node, _node_, &tl->listener_list, list_node) {
		if (!listener_node->ops || !listener_node->ops->is_signaled)
			continue;

		is_signaled = listener_node->ops->is_signaled(listener_node, tl->pt_value);
		if (!is_signaled)
			continue;

		if (listener_node->ops->handle_signal)
			listener_node->ops->handle_signal(listener_node);

		if (listener_node->ops->release)
			listener_node->ops->release(listener_node);

		dkmd_timeline_del_listener(tl, listener_node);
		kfree(listener_node);
	}

	return 0;
}

static struct notifier_block timeline_isr_notifier = {
	.notifier_call = _timeline_isr_notify,
};

void dkmd_timline_init(struct dkmd_timeline *tl, const char *name, void *parent, uint32_t listening_isr_bit)
{
	dpu_pr_info(" ++++ ");

	snprintf(tl->name, sizeof(tl->name), "%s", name);
	kref_init(&tl->kref);
	spin_lock_init(&tl->value_lock);
	spin_lock_init(&tl->list_lock);
	INIT_LIST_HEAD(&tl->listener_list);

	tl->pt_value = 0;
	tl->next_value = 0;
	tl->parent = parent;
	tl->listening_isr_bit = listening_isr_bit;
	tl->notifier = &timeline_isr_notifier;

	dpu_pr_info("init call dkmd_isr_register_listener, listening_isr_bit = %d", tl->listening_isr_bit);
}
EXPORT_SYMBOL(dkmd_timline_init);

uint64_t dkmd_timeline_get_pt_value(struct dkmd_timeline *timeline)
{
	uint64_t value;
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	value = timeline->pt_value;
	spin_unlock_irqrestore(&timeline->value_lock, flags);

	return value;
}

struct dkmd_timeline_listener *dkmd_timeline_alloc_listener(struct dkmd_timeline_listener_ops *listener_ops, void *listener_data, uint64_t value)
{
	struct dkmd_timeline_listener *listener = NULL;

	listener = kzalloc(sizeof(*listener), GFP_KERNEL);
	if (!listener)
		return NULL;

	listener->listener_data = listener_data;
	listener->ops = listener_ops;
	listener->pt_value = value;

	return listener;
}

MODULE_LICENSE("GPL");