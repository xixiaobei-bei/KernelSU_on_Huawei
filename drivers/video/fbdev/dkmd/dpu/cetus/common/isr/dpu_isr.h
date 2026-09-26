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
#ifndef DPU_ISR_H
#define DPU_ISR_H

#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/irqreturn.h>
#include <linux/list.h>

#define ISR_LISTENER_LIST_COUNT   32

typedef irqreturn_t (*handle_irq_func)(int irq, void *ptr);

enum {
	DSI_INT_FRM_START = BIT(0),
	DSI_INT_FRM_END = BIT(1),
	DSI_INT_UNDER_FLOW = BIT(2),
	DSI_INT_VFP_END = BIT(3),
	DSI_INT_VSYNC = BIT(4),
	DSI_INT_VBP = BIT(5),
	DSI_INT_VFP = BIT(6),
	DSI_INT_VACT0_START = BIT(7),
	DSI_INT_VACT0_END = BIT(8),
	DSI_INT_VACT1_START = BIT(9),
	DSI_INT_VACT1_END = BIT(10),
	DSI_INT_LCD_TE1 = BIT(11),
	DSI_INT_LCD_TE0 = BIT(12),

	/* TODO: other irq status bits */
};

struct dpu_isr_listener_node {
	struct list_head list_node;

	void *data;
	uint32_t listen_bit;
	struct raw_notifier_head irq_nofitier;
};

struct dpu_isr {
	uint32_t irq_no;
	const char *irq_name;
	void *parent;
	uint32_t unmask;
	struct list_head isr_listener_list[ISR_LISTENER_LIST_COUNT]; /* struct dpu_isr_state_node, 32 is uint32_t's bits length */
};

void dpu_isr_setup(struct dpu_isr *isr_ctrl, handle_irq_func irq_handle);
int dpu_isr_notify_listener(struct dpu_isr *isr_ctrl, uint32_t listen_bit);
int dpu_isr_unregister_listener(struct dpu_isr *isr_ctrl, struct notifier_block *nb, uint32_t listen_bit);
int dpu_isr_register_listener(struct dpu_isr *isr_ctrl, struct notifier_block *nb, uint32_t listen_bit, void *listener_data);

#endif