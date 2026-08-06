/*
 * hiusbc_event.c -- Event Management for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/slab.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include "hiusbc_core.h"
#include "hiusbc_debug.h"
#include "hiusbc_gadget.h"
#include "hiusbc_transfer.h"
#include "hiusbc_setup.h"
#include "hiusbc_event.h"

struct hiusbc_evt_ring *hiusbc_event_ring_alloc(struct hiusbc *hiusbc,
	unsigned int size, unsigned int index)
{
	struct hiusbc_evt_ring *ring = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	ring = kzalloc(sizeof(*ring), GFP_KERNEL);
	if (!ring)
		return NULL;

	ring->trbs = dma_alloc_coherent(hiusbc->sysdev,
				size * sizeof(*ring->trbs),
		&ring->dma, GFP_KERNEL);
	if (!ring->trbs) {
		dev_err(hiusbc->dev, "failed to alloc event ring!\n");
		kfree(ring);
		return NULL;
	}

	ring->size = size;
	ring->dequeue = ring->trbs;
	ring->ccs = 1;

	ring->hiusbc = hiusbc;
	ring->int_idx = index;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "-\n");

	return ring;
}

void hiusbc_event_ring_free(struct hiusbc_evt_ring *ring)
{
	struct hiusbc *hiusbc = ring->hiusbc;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	dma_free_coherent(hiusbc->sysdev, ring->size * sizeof(*ring->trbs),
		ring->trbs, ring->dma);

	kfree(ring);
}

static bool last_trb_on_evt_ring(struct hiusbc_evt_ring *event_ring,
		union hiusbc_trb *trb)
{
	return trb == &event_ring->trbs[event_ring->size - 1];
}

static void evt_ring_inc_deq(struct hiusbc_evt_ring *event_ring)
{
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	if (!last_trb_on_evt_ring(event_ring, event_ring->dequeue)) {
		event_ring->dequeue++;
		return;
	}

	event_ring->ccs ^= 1;
	event_ring->dequeue = event_ring->trbs;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "-\n");
}

static bool is_trb_on_ring(struct hiusbc_xfer_seg *start_seg,
		union hiusbc_trb *start_trb, union hiusbc_trb *end_trb,
		dma_addr_t trb_addr)
{
	struct hiusbc_xfer_seg *cur_seg = start_seg;
	dma_addr_t seg_start_addr =
			hiusbc_xfer_trb_vrt_to_dma(start_seg, start_trb);
	dma_addr_t seg_end_addr;
	dma_addr_t end_trb_addr;

	do {
		if (seg_start_addr == 0)
			return false;

		seg_end_addr = hiusbc_xfer_trb_vrt_to_dma(cur_seg,
				&cur_seg->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1]);

		end_trb_addr = hiusbc_xfer_trb_vrt_to_dma(cur_seg, end_trb);

		/*
		 * end_trb_addr > 0 means end_trb is in the cur_seg,
		 * that is cur_seg is the last seg
		 */
		if (end_trb_addr > 0) {
			if (seg_start_addr <= end_trb_addr) {
				if (trb_addr >= seg_start_addr &&
					trb_addr <= end_trb_addr)
					return true;
			}
			/*
			 * Enqueue never goes into dequeue_seg,
			 * so we don't consider the situation that
			 * a TD wrapped around to the top.
			 * just give warning here and return false.
			 */

			return false;
		}

		if (trb_addr >= seg_start_addr && trb_addr <= seg_end_addr)
			return true;

		cur_seg = cur_seg->next;
		seg_start_addr =
			hiusbc_xfer_trb_vrt_to_dma(cur_seg, cur_seg->trbs);
	} while (cur_seg != start_seg);
	return false;
}

static bool is_trb_in_req_list(struct list_head *req_list,
		dma_addr_t req_last_trb_addr)
{
	struct hiusbc_req *hreq = NULL;
	dma_addr_t hreq_last_trb_addr;

	list_for_each_entry(hreq, req_list, queue) {
		hreq_last_trb_addr = hiusbc_xfer_trb_vrt_to_dma(hreq->last_seg,
							hreq->last_trb);
		if (hreq_last_trb_addr == req_last_trb_addr)
			return true;
	}
	return false;
}

static void hiusbc_reqs_complete_on_last_trb(
			struct hiusbc_ep *hep,
			const struct hiusbc_evt_trb *event, int status)
{
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc_xfer_trb *xfer_trb = NULL;
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_req *n = NULL;
	dma_addr_t success_trb_addr = ((dma_addr_t)event->trb_ptrh << 32) |
				event->trb_ptrl;
	u32 remain_length = hiusbc_get_trb_xfer_length(event->status);

	list_for_each_entry_safe(hreq, n, &ring->queue, queue) {
		if (usb_endpoint_xfer_control(hep->ep.desc))
			hiusbc_reclaim_invalid_trb_in_ctrl(ring);

		while (ring->dequeue != hreq->last_trb) {
			xfer_trb = &ring->dequeue->transfer;
			hreq->req.actual +=
				hiusbc_get_trb_xfer_length(xfer_trb->length);
			xfer_ring_inc_deq(ring);
		};

		xfer_trb = &ring->dequeue->transfer;
		hreq->req.actual +=
				hiusbc_get_trb_xfer_length(xfer_trb->length);
		xfer_ring_inc_deq(ring);

		if (hiusbc_xfer_trb_vrt_to_dma(
			hreq->last_seg, hreq->last_trb) == success_trb_addr) {
			hreq->req.actual -= remain_length;
			hiusbc_finish_req(hreq, status);
			break;
		} else {
			/* list del and ummap, and give back to core */
			hiusbc_finish_req(hreq, status);
		}
	}
}

static void hiusbc_prepare_next_xfer_on_last_trb(
				struct hiusbc_ep *hep,
				const struct hiusbc_xfer_ring *ring, u32 code)
{
	struct hiusbc *hiusbc = hep->hiusbc;

	if (!hep->enabled || !hep->ep.desc)
		return;

	if (usb_endpoint_xfer_control(hep->ep.desc)) {
		if (code == HIUSBC_CMPLT_CODE_SUCCESS) {
			if (hiusbc->cur_stage == HIUSBC_DATA_STAGE) {
				hiusbc->next_stage = HIUSBC_STATUS_STAGE;
				hiusbc_queue_req_ctrl_status(
					hiusbc, 0, GFP_ATOMIC);
			}
		} else if (code == HIUSBC_CMPLT_CODE_BABBLE_ERR) {
			if (hiusbc->cur_stage == HIUSBC_DATA_STAGE) {
				/*
				 * reply Stall for TD Babble during data stage.
				 */
				hiusbc->next_stage = HIUSBC_STATUS_STAGE;
				hiusbc_queue_req_ctrl_status(
					hiusbc, 1, GFP_ATOMIC);
			} else if (hiusbc->cur_stage == HIUSBC_STATUS_STAGE) {
				dev_err(hiusbc->dev,
					"Host still sending data "
					"after data stage finished!\n");

				/*
				 * HW sends a STALL autonomously
				 * when babble error occur.
				 */
				hiusbc->eps[0]->stalled = true;
				hiusbc->eps[1]->stalled = true;
				hiusbc->next_stage = HIUSBC_SETUP_STAGE;
			}
		}
		return;
	}

	if (list_empty(&ring->queue))
		/* We don't maintain the xfer_state for Ctrl EP in SW. */
		if (hep->xfer_state == HIUSBC_XFER_INPROGRESS)
			hep->xfer_state = HIUSBC_XFER_WAITING;

	if (!usb_endpoint_xfer_isoc(hep->ep.desc))
		if ((!list_empty(&hep->queue)) || hiusbc->db_on_received_event)
			hiusbc_queue_req_bulk_int(hep, GFP_ATOMIC);
}
/*
 * The IOC flag must set in the last TRB of a TD.
 * If we will set IOC in any other TRB,
 * we must modify this function in advance.
 */
static void hiusbc_xfer_event_on_last_trb(
					struct hiusbc_ep *hep,
					const struct hiusbc_evt_trb *event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_xfer_ring *ring = hep->ring;
	u32 cmplt_code = hiusbc_get_trb_cmplt_code(event->status);
	dma_addr_t success_trb_addr =
		((dma_addr_t)event->trb_ptrh << 32) | event->trb_ptrl;
	int status = 0;

	if (list_empty(&ring->queue) ||
		!is_trb_in_req_list(&ring->queue, success_trb_addr)) {
		dev_err(hiusbc->dev,
			"Can't find TRB on the ring for Xfer Success Event, "
			"EP = %u, TRB dma = 0x%llx.\n",
			hep->epnum, success_trb_addr);
		return;
	}

	switch (cmplt_code) {
	case HIUSBC_CMPLT_CODE_SUCCESS:
		hep->succ_num++;
		hiusbc_dbg(HIUSBC_DEBUG_XFER,
			"%uth Xfer SUCCESS Event, EP = %u, "
			"TRB dma = 0x%llx.\n",
			hep->succ_num, hep->epnum, success_trb_addr);

		/* for isoc ep, clear miss service flag */
		hep->stalled = false;
		break;

	case HIUSBC_CMPLT_CODE_ISO_OVERRUN:
		hep->isoc_overrun_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Xfer ISOC OVERRUN Event, "
			"EP = %u, TRB dma = 0x%llx.\n",
			hep->isoc_overrun_num, hep->epnum, success_trb_addr);
		/* Only isoc eps have Isoc Overrun event. */
		if (!usb_endpoint_xfer_isoc(hep->ep.desc)) {
			dev_err(hiusbc->dev,
				"Isoc OVERRUN Event happened "
				"on a non-isoc ep!\n");
			return;
		}
		status = -EOVERFLOW;
		break;

	case HIUSBC_CMPLT_CODE_BABBLE_ERR:
		hep->babble_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Xfer BABBLE Event, EP = %u, TRB dma = 0x%llx.\n",
			hep->babble_num, hep->epnum, success_trb_addr);
		/* Isoc eps don't have Babble event. */
		if (usb_endpoint_xfer_isoc(hep->ep.desc)) {
			dev_err(hiusbc->dev,
				"BABBLE Event happened on an isoc ep!\n");
			return;
		}
		status = -EOVERFLOW;
		break;

	default:
		break;
	}

	hiusbc_reqs_complete_on_last_trb(hep, event, status);
	hiusbc_prepare_next_xfer_on_last_trb(hep, ring, cmplt_code);
}

static bool state_check_on_random_trb(struct hiusbc_ep *hep,
					const struct hiusbc_evt_trb *event,
					int *status)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_xfer_ring *ring = hep->ring;
	u32 cmplt_code = hiusbc_get_trb_cmplt_code(event->status);
	dma_addr_t short_trb_addr =
		((dma_addr_t)event->trb_ptrh << 32) | event->trb_ptrl;

	/*
	 * We also judge if the trb which generated this event is on the ring.
	 * We use enqueue as the end_trb,
	 * but actually enqueue is not on the ring for now,
	 * so we need rule out this situation.
	 * Just for safe, maybe this situation will never happen.
	 */
	if (list_empty(&ring->queue) ||
			!is_trb_on_ring(
				ring->dequeue_seg, ring->dequeue,
				ring->enqueue, short_trb_addr) ||
			hiusbc_xfer_trb_vrt_to_dma(
				ring->enqueue_seg, ring->enqueue) ==
					short_trb_addr) {
		dev_err(hiusbc->dev, "Can't find TRB on the ring! "
			"cmplt_code = %u, EP = %u, TRB dma = 0x%llx.\n",
			cmplt_code, hep->epnum, short_trb_addr);
		return false;
	}

	switch (cmplt_code) {
	case HIUSBC_CMPLT_CODE_FIFO_ERR:
		hep->fifo_err_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Data Buffer Error Event, "
			"EP = %u, TRB dma = 0x%llx.\n",
			hep->fifo_err_num, hep->epnum, short_trb_addr);
		/* We only Data Buffer Error for isoc eps. */
		if (!usb_endpoint_xfer_isoc(hep->ep.desc)) {
			dev_err(hiusbc->dev,
				"We shouldn't handle Data Buffer Error Event "
				"for non-isoc ep!\n");
			return false;
		}
		*status = -ENOSR;
		break;

	case HIUSBC_CMPLT_CODE_TRANS_ERR:
		hep->trans_err_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Transaction Error Event, "
			"EP = %u, TRB dma = 0x%llx.\n",
			hep->trans_err_num, hep->epnum, short_trb_addr);
		*status = -EPROTO;
		if (!usb_endpoint_xfer_isoc(hep->ep.desc))
			hep->stalled = true;

		break;

	case HIUSBC_CMPLT_CODE_MISSED:
		hep->miss_service_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Miss Service Error Event, "
			"EP = %u, TRB dma = 0x%llx.\n",
			hep->miss_service_num, hep->epnum, short_trb_addr);
		/* Only isoc eps have Miss Service event. */
		if (!usb_endpoint_xfer_isoc(hep->ep.desc)) {
			dev_err(hiusbc->dev, "Miss Service Error Event "
				"happened on a non-isoc ep!\n");
			return false;
		}
		hep->stalled = true;
		break;

	case HIUSBC_CMPLT_CODE_SHORT:
		hep->short_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Xfer SHORT Event, EP = %u, TRB dma = 0x%llx.\n",
			hep->short_num, hep->epnum, short_trb_addr);
		if (usb_endpoint_xfer_control(hep->ep.desc) &&
			(!(hiusbc->cur_stage == HIUSBC_DATA_STAGE &&
			hiusbc->next_stage == HIUSBC_DATA_STAGE))) {
			/*
			 * We only handle short event from sending StartXfer
			 * for data stage to receiving the corresponding
			 * XferSuccess event.
			 */
			dev_err(hiusbc->dev,
				"Short Event happened "
				"not during the data satge!\n");
			return false;
		}

		hep->stalled = false;
		break;

	case HIUSBC_CMPLT_CODE_CTRL_SHORT:
		hep->ctrl_short_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Xfer Ctrl SHORT Event, "
			"EP = %u, TRB dma = 0x%llx.\n",
			hep->ctrl_short_num, hep->epnum, short_trb_addr);
		if (!usb_endpoint_xfer_control(hep->ep.desc)) {
			dev_err(hiusbc->dev,
				"Ctrl Short Event happened "
				"on a non-ctrl ep!\n");
			return false;
		}
		if (!(hiusbc->cur_stage == HIUSBC_DATA_STAGE &&
			hiusbc->next_stage == HIUSBC_DATA_STAGE)) {
			dev_err(hiusbc->dev,
				"Ctrl Short Event happened "
				"not during the data satge!\n");
			return false;
		}

		hep->stalled = false;
		break;

	default:
		break;
	}
	return true;
}

static void hiusbc_reqs_complete_on_random_trb(
			struct hiusbc_ep *hep,
			const struct hiusbc_evt_trb *event, int status)
{
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc_xfer_trb *xfer_trb = NULL;
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_req *n = NULL;
	bool skip_to_next_req = false;
	dma_addr_t short_trb_addr =
		((dma_addr_t)event->trb_ptrh << 32) | event->trb_ptrl;

	list_for_each_entry_safe(hreq, n, &ring->queue, queue) {
		if (usb_endpoint_xfer_control(hep->ep.desc))
			hiusbc_reclaim_invalid_trb_in_ctrl(ring);

		skip_to_next_req = false;

		do {
			if (!skip_to_next_req) {
				xfer_trb = &ring->dequeue->transfer;
				hreq->req.actual +=
					hiusbc_get_trb_xfer_length(
						xfer_trb->length);
				if (hiusbc_xfer_trb_vrt_to_dma(
					ring->dequeue_seg, ring->dequeue) ==
						short_trb_addr) {
					hreq->req.actual -=
						hiusbc_get_trb_xfer_length(
							event->status);
					skip_to_next_req = true;
				}
			}

			/*
			 * Move to next TD.
			 * must judge dequeue before inc dequeue.
			 */
			if (ring->dequeue == hreq->last_trb) {
				xfer_ring_inc_deq(ring);
				break;
			}

			xfer_ring_inc_deq(ring);
		} while (true);

		if (hiusbc_get_trb_cmplt_code(event->status) ==
					HIUSBC_CMPLT_CODE_MISSED)
			hreq->req.actual = 0;

		/* list del and ummap, and give back to core */
		hiusbc_finish_req(hreq, status);

		if (skip_to_next_req)
			break;
		/* Continue to process next request. */
		pr_err("ERROR: this request lost cmplt event!\n");
	}
}

static void hiusbc_prepare_next_xfer_on_random_trb(
			struct hiusbc *hiusbc, struct hiusbc_ep *hep,
			const struct hiusbc_xfer_ring *ring, u32 code)
{
	if (!hep->enabled || !hep->ep.desc) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"ep has been disabled just return!\n");
		return;
	}

	if (usb_endpoint_xfer_control(hep->ep.desc)) {
		if ((code == HIUSBC_CMPLT_CODE_SHORT) ||
			(code == HIUSBC_CMPLT_CODE_CTRL_SHORT)) {
			/* it must be data stage now. */
			hiusbc->next_stage = HIUSBC_STATUS_STAGE;
			hiusbc_queue_req_ctrl_status(hiusbc, 0, GFP_ATOMIC);
		} else if (code == HIUSBC_CMPLT_CODE_TRANS_ERR) {
			/*
			 * HW sends a STALL autonomously
			 * when Transaction error occur.
			 */
			hiusbc->eps[0]->stalled = true;
			hiusbc->eps[1]->stalled = true;
			hiusbc->next_stage = HIUSBC_SETUP_STAGE;
		}
		return;
	}

	if (list_empty(&ring->queue))
		/* We needn't maintain the xfer_state for Ctrl EP in SW. */
		if (hep->xfer_state == HIUSBC_XFER_INPROGRESS)
			hep->xfer_state = HIUSBC_XFER_WAITING;

	if (!usb_endpoint_xfer_isoc(hep->ep.desc))
		if ((!list_empty(&hep->queue)) || hiusbc->db_on_received_event)
			hiusbc_queue_req_bulk_int(hep, GFP_ATOMIC);
}

static void hiusbc_xfer_event_on_random_trb(
					struct hiusbc_ep *hep,
					const struct hiusbc_evt_trb *event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_xfer_ring *ring = hep->ring;
	u32 cmplt_code = hiusbc_get_trb_cmplt_code(event->status);
	int status = 0;

	if (!state_check_on_random_trb(hep, event, &status))
		return;

	hiusbc_reqs_complete_on_random_trb(hep, event, status);
	hiusbc_prepare_next_xfer_on_random_trb(hiusbc, hep, ring, cmplt_code);
}

static void hiusbc_xfer_event_nrdy(struct hiusbc_ep *hep,
			const struct hiusbc_evt_trb *event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	int temp_mf;

	hep->nrdy_num++;
	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"%uth Xfer NOT READY Event, EP = %u.\n",
		hep->nrdy_num, hep->epnum);

	if (usb_endpoint_xfer_control(hep->ep.desc))
		 /* controller doesn't generate nrdy event during ctrl xfer. */
		return;

	if (usb_endpoint_xfer_isoc(hep->ep.desc)) {
		if (hep->xfer_state == HIUSBC_XFER_END)
			return;

		hep->stalled = false;

		hep->nrdy_received = true;
		hep->nrdy_mf = hiusbc_get_trb_mf(event->info);

		temp_mf = hiusbc_get_frame_id(hiusbc);
		hiusbc_dbg(HIUSBC_DEBUG_ISOC,
			"NRDY happened in mf = %u handled in mf = %d\n",
			hep->nrdy_mf, temp_mf);

		if (!list_empty(&hep->queue))
			hiusbc_queue_req_isoc(hep, GFP_ATOMIC);
	} else {
		if ((!list_empty(&hep->queue)) || hiusbc->db_on_received_event)
			hiusbc_queue_req_bulk_int(hep, GFP_ATOMIC);
	}
}

static void hiusbc_get_setup_packet(struct hiusbc *hiusbc,
			const struct hiusbc_evt_trb *event)
{
	const struct usb_ctrlrequest *setup_ptr =
			(const struct usb_ctrlrequest *)event;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"Setup Packet Lo = 0x%x\n", event->trb_ptrl);
	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"Setup Packet Hi = 0x%x\n", event->trb_ptrh);

	hiusbc->setup_packet.bRequestType = setup_ptr->bRequestType;
	hiusbc->setup_packet.bRequest = setup_ptr->bRequest;
	hiusbc->setup_packet.wValue = le16_to_cpu(setup_ptr->wValue);
	hiusbc->setup_packet.wIndex = le16_to_cpu(setup_ptr->wIndex);
	hiusbc->setup_packet.wLength = le16_to_cpu(setup_ptr->wLength);

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"bRequestType = 0x%x\n", hiusbc->setup_packet.bRequestType);
	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"bRequest = 0x%x\n", hiusbc->setup_packet.bRequest);
	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"wValue = 0x%x\n", hiusbc->setup_packet.wValue);
	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"wIndex = 0x%x\n", hiusbc->setup_packet.wIndex);
	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"wLength = 0x%x\n", hiusbc->setup_packet.wLength);
}

static void state_check_ctrl_xfer(struct hiusbc *hiusbc)
{
	int ret;

	if ((hiusbc->next_stage != HIUSBC_SETUP_STAGE) ||
		(hiusbc->eps[0]->stalled) || (hiusbc->eps[1]->stalled)) {
		/*
		 * if a new SETUP comes before last ctrl xfer ended,
		 * send endxfer before handle the new SETUP.
		 */
		if (hiusbc->eps[0]->xfer_state != HIUSBC_XFER_END) {
			ret = hiusbc_send_cmd_end_transfer(
				hiusbc->eps[0], false);
			if (ret)
				pr_err("Failed to send EndXfer to EP0\n");
		}

		if (hiusbc->eps[1]->xfer_state != HIUSBC_XFER_END) {
			ret = hiusbc_send_cmd_end_transfer(
				hiusbc->eps[1], false);
			if (ret)
				pr_err("Failed to send EndXfer to EP1\n");
		}

		hiusbc_cleanup_exist_req_on_ep(hiusbc->eps[0], -ECONNRESET);
		hiusbc_cleanup_exist_req_on_ep(hiusbc->eps[1], -ECONNRESET);
	}
}

static void ctrl_xfer_three_stage(struct hiusbc *hiusbc,
		struct hiusbc_ep *hep, const struct hiusbc_evt_trb *event)
{
	struct usb_ctrlrequest *setup = &hiusbc->setup_packet;

	hiusbc->cur_stage = HIUSBC_SETUP_STAGE;
	hiusbc->eps[0]->stalled = false;
	hiusbc->eps[1]->stalled = false;
	hiusbc->delayed_status = false;
	hiusbc->ctrl_num = hiusbc_get_trb_ctrl_num(event->info);
	hep->nrdy_received = false;

	if (setup->wLength == 0) {
		hiusbc->next_stage = HIUSBC_STATUS_STAGE;
		hiusbc->need_data_stage = 0;
		hiusbc->data_stage_dir = 0;
		hiusbc->status_stage_dir = 1;
	} else {
		hiusbc->next_stage = HIUSBC_DATA_STAGE;
		hiusbc->need_data_stage = 1;
		hiusbc->data_stage_dir = !!(setup->bRequestType & USB_DIR_IN);
		hiusbc->status_stage_dir = !hiusbc->data_stage_dir;
	}
}

static void hiusbc_xfer_event_setup(struct hiusbc_ep *hep,
			const struct hiusbc_evt_trb *event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct usb_ctrlrequest *setup = &hiusbc->setup_packet;
	int ret;

	state_check_ctrl_xfer(hiusbc);

	ctrl_xfer_three_stage(hiusbc, hep, event);

	if ((setup->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		ret = hiusbc_handle_std_req(hiusbc, setup);
	} else {
		hiusbc_dbg(HIUSBC_DEBUG_CTRL,
			"Delegate non-std req: %u to usbcore.\n",
			setup->bRequest);
		ret = hiusbc_gadget_delegate_req(hiusbc, setup);
	}

	if (ret == USB_GADGET_DELAYED_STATUS) {
		hiusbc->delayed_status = true;
		return;
	}

	if (ret < 0) {
		/* Start Ctrl Status Stall */
		if (hiusbc->need_data_stage)
			hiusbc->status_stage_dir = 1;

		hiusbc_queue_req_ctrl_status(hiusbc, 1, GFP_ATOMIC);
	} else if (!hiusbc->need_data_stage) {
		/* Start Ctrl Status ACK */
		hiusbc_queue_req_ctrl_status(hiusbc, 0, GFP_ATOMIC);
	}
}

static void hiusbc_xfer_event_ctrl_dir_err(struct hiusbc_ep *hep)
{
	struct hiusbc *hiusbc = hep->hiusbc;

	hiusbc_dbg(HIUSBC_DEBUG_ERR,
		"Xfer CTRL Direction Error Event, EP = %u.\n", hep->epnum);

	if (usb_endpoint_xfer_control(hep->ep.desc)) {
		hiusbc_cleanup_exist_req_on_ep(hiusbc->eps[0], -ECONNRESET);
		hiusbc_cleanup_exist_req_on_ep(hiusbc->eps[1], -ECONNRESET);

		/* HW sends a STALL autonomously when ctrl dir error occur. */
		hiusbc->eps[0]->stalled = true;
		hiusbc->eps[1]->stalled = true;
		hiusbc->next_stage = HIUSBC_SETUP_STAGE;
	} else {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Unexpected Xfer CTRL Direction Error Event "
			"for a non-ctrl EP, EP = %u.\n",
			hep->epnum);
	}
}

static void hiusbc_handle_xfer_event(struct hiusbc_ep *hep,
			const struct hiusbc_evt_trb *event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	u32 cmplt_code = hiusbc_get_trb_cmplt_code(event->status);

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	hep->flow_control = false;

	switch (cmplt_code) {
	/* Transfer Complete */
	case HIUSBC_CMPLT_CODE_ISO_OVERRUN:
	case HIUSBC_CMPLT_CODE_BABBLE_ERR:
	case HIUSBC_CMPLT_CODE_SUCCESS:
		hiusbc_xfer_event_on_last_trb(hep, event);
		break;

	/* Short Packet */
	case HIUSBC_CMPLT_CODE_FIFO_ERR:
		/*
		 * for isoc eps, fallthrough as a Transaction Error.
		 * for non-isoc eps, do nothing and return early.
		 * if retry more than Cerr times,
		 * a Transaction Error will be reported.
		 */
		if (!usb_endpoint_xfer_isoc(hep->ep.desc)) {
			hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"Data Buffer Error Event "
				"on a non-isoc ep, EP = %u.\n",
				hep->epnum);
			return;
		}
	/* RTL fix cerr to 0, so never receive trans err event. */
	case HIUSBC_CMPLT_CODE_TRANS_ERR:
	case HIUSBC_CMPLT_CODE_MISSED:
	case HIUSBC_CMPLT_CODE_CTRL_SHORT:
	case HIUSBC_CMPLT_CODE_SHORT:
		hiusbc_xfer_event_on_random_trb(hep, event);
		break;

	/* Transfer Not Ready */
	case HIUSBC_CMPLT_CODE_NRDY:
		hep->flow_control = true;
		hiusbc_xfer_event_nrdy(hep, event);
		break;

	/* Setup Packet */
	case HIUSBC_CMPLT_CODE_SETUP:
		hiusbc_dbg(HIUSBC_DEBUG_XFER,
			"Xfer SETUP Event, EP = %u.\n", hep->epnum);
		hiusbc_get_setup_packet(hiusbc, event);
		hiusbc_xfer_event_setup(hep, event);
		break;

	/* Control Direction Error */
	case HIUSBC_CMPLT_CODE_CTRL_DIR_ERR:
		hiusbc_xfer_event_ctrl_dir_err(hep);
		break;

	/* Wrong Completion Code */
	case HIUSBC_CMPLT_CODE_TRB_ERR:
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"TRB ERR Event, EP = %u.\n", hep->epnum);
		break;

	case HIUSBC_CMPLT_CODE_EVT_LOST:
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"Event Lost Event, EP = %u.\n", hep->epnum);
		break;

	default:
		dev_err(hiusbc->dev,
			"Unknown Xfer Event, EP = %u, cmplt_code = %u.\n",
			hep->epnum, cmplt_code);
		break;
	}
}

static void handle_end_xfer_success_event(
			struct hiusbc *hiusbc, struct hiusbc_ep *hep)
{
	struct hiusbc_xfer_ring *ring = NULL;
	int cur_mf;
	u32 start_mf;
	int ret;

	ring = hep->ring;
	if (list_empty(&ring->queue) &&
		usb_endpoint_xfer_isoc(hep->ep.desc)) {
		cur_mf = hiusbc_get_frame_id(hiusbc);
		start_mf = cur_mf + HIUSBC_DEV_IST;
		start_mf = (start_mf & ~(hep->si - 1)) + hep->si;

		/* Cycle bit of enqueue is always invalid. */
		ret = hiusbc_send_cmd_start_transfer(hep, start_mf,
				hiusbc_xfer_trb_vrt_to_dma(
					ring->enqueue_seg, ring->enqueue),
				hep->ring->pcs, 1, 0);
		if (ret < 0)
			pr_err("Failed to send StartXfer cmd "
				"after EndXfer cmd on isoc EP%u! ret = %d\n",
				hep->epnum, ret);
	}

	if (!hep->end_xfer_pending)
		return;

	hep->end_xfer_pending = false;
	hep->xfer_state = HIUSBC_XFER_IDLE;
	hep->nrdy_received = false;
	hep->nrdy_mf = 0;

	wake_up(&hep->wait_end_xfer_cmd);
}

static void handle_disable_ep_success_event(
				struct hiusbc_ep *hep)
{
	if (!hep->disable_pending)
		return;

	hep->disable_pending = false;

	if (!list_empty(&hep->ring->queue))
		hiusbc_cleanup_exist_req_on_ep(hep, -ESHUTDOWN);

	hep->enabled = false;
	hep->wedged = false;
	hep->stalled = false;
	hep->xfer_state = HIUSBC_XFER_IDLE;
	hep->nrdy_received = false;
	hep->flow_control = false;
	hep->nrdy_mf = 0;
	hep->si = 0;

	if (hep->epnum > 1) {
		hep->ep.desc = NULL;
		hep->ep.comp_desc = NULL;
	}

	wake_up(&hep->wait_disable_ep_cmd);
}

static void handle_cmd_success_event(struct hiusbc *hiusbc,
				struct hiusbc_ep *hep, u32 cmd_type)
{
	switch (cmd_type) {
	case HIUSBC_CMD_TYPE_END_TRANSFER:
		hiusbc_dbg(HIUSBC_DEBUG_TEMP, "Cmd Type: END_TRANSFER.\n");
		handle_end_xfer_success_event(hiusbc, hep);

		break;
	case HIUSBC_CMD_TYPE_DISABLE_EP:
		hiusbc_dbg(HIUSBC_DEBUG_TEMP, "Cmd Type: DISABLE_EP.\n");

		handle_disable_ep_success_event(hep);
		break;

	case HIUSBC_CMD_TYPE_SET_ADDR:
	case HIUSBC_CMD_TYPE_ENABLE_EP:
	case HIUSBC_CMD_TYPE_START_TRANSFER:
	case HIUSBC_CMD_TYPE_CLEAR_HALT:
	case HIUSBC_CMD_TYPE_DEV_NOTE:
		/* fall-through */
		break;
	case HIUSBC_CMD_TYPE_SET_HALT:
		hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Cmd Type: SET_HALT.\n");
		if (!hep->set_halt_pending)
			break;

		hep->set_halt_pending = false;
		hep->stalled = true;
		/* need to send init=1 after set and clear halt. */
		hep->xfer_state = HIUSBC_XFER_IDLE;
		break;
	default:
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Cmd Type: Unknown: %u.\n", cmd_type);
		break;
	}
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Done Info: SUCCESS.\n");
}

static void hiusbc_handle_cmd_event(struct hiusbc_ep *hep,
		const struct hiusbc_evt_trb *event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	u32 cmplt_code = hiusbc_get_trb_cmplt_code(event->status);
	u32 cmd_type = hiusbc_get_trb_cmd_type(event->status);

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Cmd Event, EP = %u:\n", hep->epnum);

	switch (cmplt_code) {
	case HIUSBC_CMPLT_CODE_SUCCESS:
		handle_cmd_success_event(hiusbc, hep, cmd_type);
		break;

	case HIUSBC_CMPLT_CODE_PARM_ERR:
	case HIUSBC_CMPLT_CODE_EP_STATE_ERR:
	case HIUSBC_CMPLT_CODE_CMD_ABORT_ERR:
		break;

	default:
		dev_err(hiusbc->dev,
			"Unknown Done info for Cmd Event, "
			"EP = %u, cmd_type = %u.\n", hep->epnum, cmd_type);
		break;
	}
}

static void hiusbc_dev_event_disconnect(struct hiusbc *hiusbc)
{
	int reg;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	hiusbc->u1_sel = 0;
	hiusbc->u1_pel = 0;
	hiusbc->u2_sel = 0;
	hiusbc->u2_pel = 0;
	hiusbc_set_ux_exit(hiusbc);

	hiusbc->u1_enable = false;
	hiusbc->u2_enable = false;

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
	reg &= ~(MPI_APP_DEVICE_REG_ENABLE_U1_INITIATE_MASK |
		MPI_APP_DEVICE_REG_ENABLE_U2_INITIATE_MASK);
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_LPM_ENABLE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);

	hiusbc_disable_ctrl_eps(hiusbc);

	hiusbc_gadget_disconnect(hiusbc);

	hiusbc->connected = false;
	hiusbc->gadget.speed = USB_SPEED_UNKNOWN;
	usb_gadget_set_state(&hiusbc->gadget, USB_STATE_NOTATTACHED);
}

static void hiusbc_clear_stall_all_ep(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	u8 epnum;

	hiusbc_dbg(HIUSBC_DEBUG_EP, "+\n");

	for (epnum = 1; epnum < HIUSBC_EP_NUMS; epnum++) {
		hep = hiusbc->eps[epnum];
		if (!hep || !(hep->stalled))
			continue;

		hiusbc_send_cmd_clear_halt(hep);
		hep->stalled = false;
	}
}

static void hiusbc_dev_event_reset(struct hiusbc *hiusbc)
{
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	usb_gadget_set_state(&hiusbc->gadget, USB_STATE_DEFAULT);
	hiusbc->connected = true;
	hiusbc->delayed_status = false;
	hiusbc_gadget_reset(hiusbc);
	hiusbc->test_mode = 0;
	hiusbc_req_set_test_mode(hiusbc, 0);
	hiusbc_clear_stall_all_ep(hiusbc);
	hiusbc_send_cmd_set_addr(hiusbc, 0);
}

static void fill_ep0_attr(struct hiusbc *hiusbc, enum usb_device_speed speed)
{
	switch (speed) {
	case USB_SPEED_SUPER_PLUS:
	case USB_SPEED_SUPER:
		hiusbc->ep0_desc->wMaxPacketSize = cpu_to_le16(512);
		hiusbc->gadget.ep0->maxpacket = 512;
		hiusbc->gadget.speed = speed;
		break;
	case USB_SPEED_HIGH:
	case USB_SPEED_FULL:
		hiusbc->ep0_desc->wMaxPacketSize = cpu_to_le16(64);
		hiusbc->gadget.ep0->maxpacket = 64;
		hiusbc->gadget.speed = speed;
		break;
	case USB_SPEED_LOW:
		hiusbc->ep0_desc->wMaxPacketSize = cpu_to_le16(8);
		hiusbc->gadget.ep0->maxpacket = 8;
		hiusbc->gadget.speed = speed;
		break;
	default:
		break;
	}
	hiusbc->eps[1]->ep.maxpacket = hiusbc->gadget.ep0->maxpacket;
}

static void config_lpm(struct hiusbc *hiusbc, enum usb_device_speed speed)
{
	u32 reg;

	if ((speed == USB_SPEED_HIGH) || (speed == USB_SPEED_FULL)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "enable USB2 LPM!\n");
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
		reg &= ~MPI_APP_DEVICE_REG_BESL_THRESHOLD_SUSPEND_MASK;
		reg |= hiusbc_ctl_besl_suspend(hiusbc->hird_suspend);
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);

		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
		reg |= MPI_APP_DEVICE_REG_LPM_ENABLE_MASK;
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
	} else {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "disable USB2 LPM!\n");
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
		reg &= ~MPI_APP_DEVICE_REG_LPM_ENABLE_MASK;
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
	}
}

static void hiusbc_dev_event_connect(struct hiusbc *hiusbc)
{
	enum usb_device_speed speed;
	enum usb_device_speed old_speed;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	hiusbc_clear_dev_event_counter(hiusbc);

	speed = hiusbc_get_link_speed(hiusbc);

	if (speed == USB_SPEED_UNKNOWN) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Got a invalid speed from portsc!\n");
		return;
	}

	old_speed = hiusbc->gadget.speed;
	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "old_speed = %d\n", old_speed);

	if ((old_speed == speed) &&
		(hiusbc->ep0_changed_by_gadget_api == false) &&
		(hiusbc->eps[0]->enabled == true) &&
		(hiusbc->eps[1]->enabled == true)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Early return! Got a same speed as old speed!\n");
		return;
	}

	if (old_speed != USB_SPEED_UNKNOWN) {
		/*
		 * Controller may generate 2 connect events Redundantly.
		 * Software only look at the last one.
		 */
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"two connect events come together "
			"with different speed.\n");
		hiusbc_dev_event_reset(hiusbc);
	}

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "Connected at speed = %d\n", speed);

	fill_ep0_attr(hiusbc, speed);

	config_lpm(hiusbc, speed);

	hiusbc_disable_ctrl_eps(hiusbc);

	if (hiusbc_enable_ctrl_eps(hiusbc)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Early return! Failed to enable ctrl eps!\n");
		return;
	}
	hiusbc->ep0_changed_by_gadget_api = false;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "-\n");
}

static void hiusbc_dev_event_plc(struct hiusbc *hiusbc, u32 evt_info)
{
	enum hiusbc_link_state state = (int)(evt_info & 0x0f);

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "evt_info = 0x%x\n", evt_info);
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "State = %d\n", state);

	if (hiusbc->gadget.speed == USB_SPEED_SUPER_PLUS ||
		hiusbc->gadget.speed == USB_SPEED_SUPER) {
		switch (state) {
		case HIUSBC_LINK_STATE_U1:
		case HIUSBC_LINK_STATE_U2:
		case HIUSBC_LINK_STATE_U3:

			break;
		default:
			break;
		}
	} else {
		switch (state) {
		case HIUSBC_LINK_STATE_L1:
		case HIUSBC_LINK_STATE_L2:

			break;
		case HIUSBC_LINK_STATE_L1_RESUME:
		case HIUSBC_LINK_STATE_L2_RESUME:

			break;
		default:
			break;
		}
	}
}

/* resend startxfer cmd to prevent startxfer and lpm request conflict. */
int hiusbc_wakeup_active_bulk_eps(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	u8 epnum;
	int ret = 0;

	for (epnum = 2; epnum < HIUSBC_EP_NUMS; epnum++) {
		hep = hiusbc->eps[epnum];
		if (hep->enabled && hep->flow_control &&
			!list_empty(&hep->ring->queue) &&
			usb_endpoint_xfer_bulk(hep->ep.desc)) {
			hiusbc_dbg(HIUSBC_DEBUG_TEMP,
				"StartXfer and LPM request conflict on EP%u\n",
				epnum);
			hep->flow_control = false;

			ret = hiusbc_send_cmd_start_transfer(
					hep, 0, 0, 0, 0, 0);

			if (ret < 0) {
				pr_err("Failed to send StartXfer cmd "
					"on flow control EP%u! ret = %d\n",
					hep->epnum, ret);
				hiusbc_cleanup_exist_req_on_ep(
							hep, -ESHUTDOWN);
				return ret;
			}
		}
	}
	return ret;
}

static void hiusbc_dev_event_suspend(struct hiusbc *hiusbc)
{
	if (hiusbc->gadget.state >= USB_STATE_CONFIGURED)
		hiusbc_gadget_suspend(hiusbc);
}

static void hiusbc_dev_event_resume(struct hiusbc *hiusbc)
{
	hiusbc_gadget_resume(hiusbc);
}

static void hiusbc_dev_event_l1_suspend(struct hiusbc *hiusbc)
{
	if (hiusbc->gadget.state >= USB_STATE_CONFIGURED)
		hiusbc_wakeup_active_bulk_eps(hiusbc);
}

static void hiusbc_dev_event_sof(struct hiusbc *hiusbc)
{
	int temp;

	temp = hiusbc_get_frame_id(hiusbc);
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Frame ID = %d", temp);
}

static void hiusbc_handle_dev_event(struct hiusbc *hiusbc,
				struct hiusbc_evt_trb *event)
{
	u32 status = event->status;
	u32 info = event->info;
	u32 cmplt_code = hiusbc_get_trb_cmplt_code(status);
	u32 evt_type = hiusbc_get_trb_devt_type(info);
	u32 reg;
	/* FPGA_Debug */
	u8 event_count = 0;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+ cmplt_code = 0x%x\n", cmplt_code);

	hiusbc->stat.dev_event_total_num++;

	if (cmplt_code == HIUSBC_CMPLT_CODE_EB_FULL) {
		hiusbc->stat.eb_full_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "%uth Dev EB_FULL Event.\n",
			hiusbc->stat.eb_full_num);
		return;
	}

	if (cmplt_code != HIUSBC_CMPLT_CODE_SUCCESS) {
		hiusbc->stat.unsuccess_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Cmplt Code for Dev Event is not SUCCESS.\n",
			hiusbc->stat.unsuccess_num);
		return;
	}

	if (evt_type & HIUSBC_DEVT_TYPE_RST) {
		event_count++;
		hiusbc->stat.rst_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev RST Event.\n", hiusbc->stat.rst_num);

		reg = hiusbc_readl(hiusbc->com_regs,
			MPI_APP_COM_REG_TRPU_COM_RSV_OFFSET);
		reg |= HIUSBC_NON_XFER_STATE_MASK;
		hiusbc_writel(reg, hiusbc->com_regs,
			MPI_APP_COM_REG_TRPU_COM_RSV_OFFSET);

		hiusbc_dev_event_reset(hiusbc);
	}

	if (hiusbc->connected == false) {
		hiusbc->stat.event_after_discon_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"only handle RST event after "
			"Disconnect event! event type = 0x%x\n", evt_type);
		return;
	}

	if (evt_type & HIUSBC_DEVT_TYPE_CONNECT) {
		event_count++;
		hiusbc->stat.connect_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev CONNECT Event.\n", hiusbc->stat.connect_num);

		reg = hiusbc_readl(hiusbc->com_regs,
			MPI_APP_COM_REG_TRPU_COM_RSV_OFFSET);
		reg &= ~HIUSBC_NON_XFER_STATE_MASK;
		hiusbc_writel(reg, hiusbc->com_regs,
			MPI_APP_COM_REG_TRPU_COM_RSV_OFFSET);

		hiusbc_dev_event_connect(hiusbc);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_SUSPEND) {
		event_count++;
		hiusbc->stat.suspend_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev SUSPEND Event.\n", hiusbc->stat.suspend_num);
		hiusbc_dev_event_suspend(hiusbc);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_RESUME) {
		event_count++;
		hiusbc->stat.resume_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev RESUME Event.\n", hiusbc->stat.resume_num);
		hiusbc_dev_event_resume(hiusbc);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_L1_SUSPEND) {
		event_count++;
		hiusbc->stat.l1_suspend_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev L1 SUSPEND Event.\n",
			hiusbc->stat.l1_suspend_num);
		hiusbc_dev_event_l1_suspend(hiusbc);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_L1_RESUME) {
		event_count++;
		hiusbc->stat.l1_resume_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev L1 RESUME Event.\n",
			hiusbc->stat.l1_resume_num);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_SOF) {
		event_count++;
		hiusbc->stat.sof_num++;
		hiusbc_dbg(HIUSBC_DEBUG_EVENT,
			"%uth Dev SOF Event.\n", hiusbc->stat.sof_num);
		hiusbc_dev_event_sof(hiusbc);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_PLC) {
		event_count++;
		hiusbc->stat.plc_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev PLC Event.\n", hiusbc->stat.plc_num);
		hiusbc_dev_event_plc(hiusbc, hiusbc_get_trb_devt_info(info));
	}

	if (evt_type & HIUSBC_DEVT_TYPE_DISCONNECT) {
		event_count++;
		hiusbc->stat.disconnect_num++;
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"%uth Dev DISCONNECT Event.\n",
			hiusbc->stat.disconnect_num);

		reg = hiusbc_readl(hiusbc->com_regs,
			MPI_APP_COM_REG_TRPU_COM_RSV_OFFSET);
		reg |= HIUSBC_NON_XFER_STATE_MASK;
		hiusbc_writel(reg, hiusbc->com_regs,
			MPI_APP_COM_REG_TRPU_COM_RSV_OFFSET);

		hiusbc_dev_event_disconnect(hiusbc);
	}

	if (evt_type & HIUSBC_DEVT_TYPE_PHY_ERR) {
		event_count++;
		hiusbc->stat.phy_err_num++;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth Dev PHY_ERR Event.\n", hiusbc->stat.phy_err_num);
	}

	if (event_count > 1) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"%uth multi-bits Dev Event. evt_type = %u\n",
			hiusbc->stat.combo_num, evt_type);
		hiusbc->stat.combo_num++;
	}

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "-\n");
}

static int hiusbc_handle_event(struct hiusbc_evt_ring *event_ring)
{
	struct hiusbc *hiusbc = event_ring->hiusbc;
	struct hiusbc_ep *hep = NULL;
	struct hiusbc_evt_trb *event = NULL;
	u32 cmplt_code;
	u32 event_info;
	int epnum;

	event = &event_ring->dequeue->event;
	event_info = event->info;

	/*
	 * Check if we have handled to the enq pointer,
	 * which means event buffer is empty now.
	 */
	if ((event_info & HIUSBC_TRB_CYCLE) != event_ring->ccs)
		return 0;

	cmplt_code = hiusbc_get_trb_cmplt_code(event->status);
	epnum = hiusbc_get_trb_epnum(event_info);
	hep = hiusbc->eps[epnum];

	hiusbc_dbg(HIUSBC_DEBUG_EVENT,
		"Handle event(TRB) = 0x%p, EP = %u, cmplt_code = %u.\n",
		event, epnum, cmplt_code);

	if (cmplt_code == HIUSBC_CMPLT_CODE_INVALID)
		goto out;

	switch (hiusbc_get_trb_type(event_info)) {
	case HIUSBC_TRB_TYPE_XFER_EVENT:
		hiusbc_handle_xfer_event(hep, event);
		break;

	case HIUSBC_TRB_TYPE_CMD_EVENT:
		hiusbc_handle_cmd_event(hep, event);
		break;

	case HIUSBC_TRB_TYPE_DEV_EVENT:
		hiusbc_handle_dev_event(hiusbc, event);
		break;

	case HIUSBC_TRB_TYPE_MF_WRAP_EVENT:
		if (cmplt_code != HIUSBC_CMPLT_CODE_SUCCESS) {
			dev_err(hiusbc->dev,
				"Cmplt Code for MF_WRAP is not SUCCESS.\n");
			goto out;
		}
		hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Dev MF_WRAP Event.\n");
		break;

	case HIUSBC_TRB_TYPE_VENDOR_TEST_EVENT:
		if (cmplt_code != HIUSBC_CMPLT_CODE_SUCCESS) {
			dev_err(hiusbc->dev,
				"Cmplt Code for VENDOR_TEST "
				"is not SUCCESS.\n");
			goto out;
		}
		hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Dev VENDOR_TEST Event.\n");
		break;

	default:
		dev_err(hiusbc->dev, "Unknown Dev Event type.\n");
		break;
	}

out:
	evt_ring_inc_deq(event_ring);

	return 1;
}

dma_addr_t hiusbc_evt_trb_vrt_to_dma(
		const struct hiusbc_evt_ring *event_ring,
		const union hiusbc_trb *trb)
{
	u32 offset;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	if (!event_ring || !trb || trb < event_ring->trbs ||
		trb - event_ring->trbs >= event_ring->size)
		return 0;

	offset = trb - event_ring->trbs;

	return event_ring->dma + (offset * sizeof(*trb));
}

static void hiusbc_intr_handle_evt_ring(struct hiusbc *hiusbc,
				struct hiusbc_evt_ring *event_ring)
{
	union hiusbc_trb *dequeue_backup = NULL;
	dma_addr_t dequeue;
	u32 reg;

	dequeue_backup = event_ring->dequeue;

	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"Start to handle interrupt at Deq = %p (0x%llx dma).\n",
			event_ring->dequeue,
			hiusbc_evt_trb_vrt_to_dma(
				event_ring, event_ring->dequeue));

	while (hiusbc_handle_event(event_ring) > 0)
		;

	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"End to handle interrupt at New_Deq = %p (0x%llx dma).\n",
			event_ring->dequeue,
			hiusbc_evt_trb_vrt_to_dma(
				event_ring, event_ring->dequeue));

	if (dequeue_backup == event_ring->dequeue)
		pr_err("Interrupt comes with empty event ring!\n");

	dequeue = hiusbc_evt_trb_vrt_to_dma(event_ring, event_ring->dequeue);
	if (!dequeue)
		pr_err("Bad sw event ring dequeue ptr!\n");

	dequeue &= MPI_APP_DEVICE_REG_DEV_ER_DEQ_PTR_LO_MASK;
	dequeue |= MPI_APP_DEVICE_REG_DEV_EVENT_HANDLER_BUSY_MASK;
	reg = lower_32_bits(dequeue);
	hiusbc_writel(reg, hiusbc->dev_regs,
		hiusbc_dev_ebdp0_offset(event_ring->int_idx));
	reg = upper_32_bits(dequeue);
	hiusbc_writel(reg, hiusbc->dev_regs,
		hiusbc_dev_ebdp1_offset(event_ring->int_idx));
}

irqreturn_t hiusbc_interrupt(int irq, void *_hiusbc)
{
	struct hiusbc *hiusbc = _hiusbc;
	struct hiusbc_evt_ring *event_ring = NULL;
	u32 reg;
	unsigned long flags;
	irqreturn_t ret = IRQ_NONE;

	spin_lock_irqsave(&hiusbc->lock, flags);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+ %p\n", hiusbc);
	if (!hiusbc) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "+ hiusbc null\n");
		goto error;
	}

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_STATUS_OFFSET);
	if (!(reg & MPI_APP_DEVICE_REG_DEV_EVENT_INTERRUPT_MASK)) {
		dev_err(hiusbc->dev,
			"Interrupt comes but dev_status_IP is 0, "
			"dev_status = 0x%x!\n", reg);
		goto error;
	}
	reg |= MPI_APP_DEVICE_REG_DEV_EVENT_INTERRUPT_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_STATUS_OFFSET);

	event_ring = hiusbc->event_ring;
	if (!event_ring || !event_ring->dequeue) {
		dev_err(hiusbc->dev,
			"Interrupt came before Event Ring was allocated!\n");
		goto error;
	}

	reg = hiusbc_readl(hiusbc->dev_regs,
		hiusbc_dev_iman_offset(event_ring->int_idx));
	if (!(reg & MPI_APP_DEVICE_REG_DEV_INT_PENDING_MASK)) {
		dev_err(hiusbc->dev,
			"Interrupt comes but IMAN_IP is 0, IMAN = 0x%x!\n",
			reg);
		goto error;
	}
	reg |= MPI_APP_DEVICE_REG_DEV_INT_PENDING_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		hiusbc_dev_iman_offset(event_ring->int_idx));

	hiusbc_intr_handle_evt_ring(hiusbc, event_ring);
	ret = IRQ_HANDLED;

error:
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "-\n");

	return ret;
}
