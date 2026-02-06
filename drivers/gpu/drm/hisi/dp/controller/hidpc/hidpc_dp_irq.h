/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __HIDPC_DP_IRQ_H__
#define __HIDPC_DP_IRQ_H__

#include <linux/kernel.h>
#include <linux/interrupt.h>

struct dp_ctrl;

irqreturn_t dptx_irq(int irq, void *dev);
irqreturn_t dptx_threaded_irq(int irq, void *dev);
void dptx_hpd_handler(struct dp_ctrl *dptx, bool plugin, uint8_t dp_lanes);
void dptx_hpd_irq_handler(struct dp_ctrl *dptx);
int handle_test_link_phy_pattern(struct dp_ctrl *dptx);


#endif // HIDPC_DP_IRQ
