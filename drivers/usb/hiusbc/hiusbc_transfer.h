/*
 * hiusbc_transfer.h -- Trasnfer Header File for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HIUSBC_TRANSFER_H
#define __HIUSBC_TRANSFER_H

#include <linux/usb/ch9.h>
#include "hiusbc_core.h"

struct hiusbc_xfer_ring *hiusbc_xfer_ring_alloc(struct hiusbc *hiusbc,
	unsigned int num_segs, unsigned int pcs, gfp_t gfp_flags);
void hiusbc_xfer_ring_free(struct hiusbc *hiusbc,
		struct hiusbc_xfer_ring *ring);
void xfer_ring_inc_deq(struct hiusbc_xfer_ring *ring);
dma_addr_t hiusbc_xfer_trb_vrt_to_dma(
		const struct hiusbc_xfer_seg *seg,
		const union hiusbc_trb *trb);
void hiusbc_finish_req(struct hiusbc_req *hreq, int status);
void hiusbc_reclaim_invalid_trb_in_ctrl(
		struct hiusbc_xfer_ring *ring);
void hiusbc_cleanup_exist_req_on_ep(struct hiusbc_ep *hep,
		int status);
int hiusbc_queue_req_bulk_int(struct hiusbc_ep *hep,
		gfp_t gfp_flags);
int hiusbc_queue_req_isoc(struct hiusbc_ep *hep, gfp_t gfp_flags);
int hiusbc_queue_req_ctrl_status(struct hiusbc *hiusbc,
		bool is_stall, gfp_t gfp_flags);
int hiusbc_ep_queue(struct usb_ep *ep, struct usb_request *req);
int hiusbc_ep_dequeue(struct usb_ep *ep, struct usb_request *req);

#endif /* __HIUSBC_TRANSFER_H */
