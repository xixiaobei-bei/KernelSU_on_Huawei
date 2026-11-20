/*
 * hiusbc_event.h -- Event Header File for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HIUSBC_EVENT_H
#define __HIUSBC_EVENT_H

#include <linux/irqreturn.h>
#include "hiusbc_core.h"

struct hiusbc_evt_ring *hiusbc_event_ring_alloc(struct hiusbc *hiusbc,
	unsigned int size, unsigned int index);
void hiusbc_event_ring_free(struct hiusbc_evt_ring *ring);
dma_addr_t hiusbc_evt_trb_vrt_to_dma(
			const struct hiusbc_evt_ring *event_ring,
			const union hiusbc_trb *trb);
irqreturn_t hiusbc_interrupt(int irq, void *_hiusbc);

#endif /* __HIUSBC_EVENT_H */
