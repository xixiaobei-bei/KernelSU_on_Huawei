/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/interrupt.h>

#include "dpu_isr.h"
#include "dpu_utils.h"

static uint32_t _get_bit(uint32_t value)
{
	uint32_t i = 0;

	while (value > 1) {
		value >>= 1;
		i++;
	}

	return i;
}

static void dpu_isr_init_listener_list(struct dpu_isr *isr_ctrl)
{
	int i;

	for (i = 0; i < ISR_LISTENER_LIST_COUNT; ++i)
		INIT_LIST_HEAD(&isr_ctrl->isr_listener_list[i]);
}

static struct list_head* dpu_isr_get_listener_list(struct dpu_isr *isr_ctrl, uint32_t listen_bit)
{
	return &isr_ctrl->isr_listener_list[_get_bit(listen_bit)];
}

void dpu_isr_setup(struct dpu_isr *isr_ctrl, handle_irq_func irq_handle)
{
	int ret;

	ret = request_irq(isr_ctrl->irq_no, irq_handle, 0, isr_ctrl->irq_name, isr_ctrl->parent);
	if (ret) {
		dpu_pr_err("setup irq fail %s, isr_ctrl->irq_no %d", isr_ctrl->irq_name, isr_ctrl->irq_no);
		return;
	}
	disable_irq(isr_ctrl->irq_no);

	dpu_isr_init_listener_list(isr_ctrl);
}

int dpu_isr_register_listener(struct dpu_isr *isr_ctrl, struct notifier_block *nb, uint32_t listen_bit, void *listener_data)
{
	struct dpu_isr_listener_node *listener_node = NULL;
	struct list_head *listener_list = NULL;

	listener_list = dpu_isr_get_listener_list(isr_ctrl, listen_bit);
	if (!listener_list) {
		dpu_pr_err("get listener_list fail, listen_bit=0x%x", listen_bit);
		return -1;
	}

	listener_node = kzalloc(sizeof(*listener_node), GFP_KERNEL);
	if (!listener_node) {
		dpu_pr_err("alloc  listener node fail, listen_bit=0x%x", listen_bit);
		return -1;
	}

	listener_node->listen_bit = listen_bit;
	listener_node->data = listener_data;

	return raw_notifier_chain_register(&listener_node->irq_nofitier, nb);
}
EXPORT_SYMBOL(dpu_isr_register_listener);

int dpu_isr_unregister_listener(struct dpu_isr *isr_ctrl, struct notifier_block *nb, uint32_t listen_bit)
{
	struct list_head *listener_list = NULL;
	struct dpu_isr_listener_node *listener_node = NULL;
	struct dpu_isr_listener_node *_node_ = NULL;

	listener_list = dpu_isr_get_listener_list(isr_ctrl, listen_bit);

	list_for_each_entry_safe(listener_node, _node_, listener_list, list_node) {
		if (listener_node->listen_bit != listen_bit)
			continue;

		raw_notifier_chain_unregister(&listener_node->irq_nofitier, nb);

		list_del(&listener_node->list_node);
		kfree(listener_node);
	}

	return 0;
}

int dpu_isr_notify_listener(struct dpu_isr *isr_ctrl, uint32_t listen_bit)
{
	struct list_head *listener_list = NULL;
	struct dpu_isr_listener_node *listener_node = NULL;
	struct dpu_isr_listener_node *_node_ = NULL;

	listener_list = dpu_isr_get_listener_list(isr_ctrl, listen_bit);

	list_for_each_entry_safe(listener_node, _node_, listener_list, list_node) {
		if (listener_node->listen_bit != listen_bit)
			continue;

		raw_notifier_call_chain(&listener_node->irq_nofitier, listen_bit, isr_ctrl->parent);

		dpu_pr_info("notify listener, listen_bit = 0x%x", listen_bit);
	}

	return 0;
}
