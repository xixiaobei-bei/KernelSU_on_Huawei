/*
 * hiusbc_transfer.c -- Transfer Management for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/dmapool.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/usb/gadget.h>
#include "hiusbc_core.h"
#include "hiusbc_debug.h"
#include "hiusbc_gadget.h"
#include "hiusbc_setup.h"
#include "hiusbc_transfer.h"

static struct hiusbc_xfer_seg *hiusbc_xfer_seg_alloc(
					struct hiusbc *hiusbc,
					unsigned int pcs, gfp_t gfp_flags)
{
	struct hiusbc_xfer_seg *seg = NULL;
	int i;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	seg = kzalloc(sizeof(struct hiusbc_xfer_seg), gfp_flags);
	if (!seg)
		return NULL;

	seg->trbs = dma_pool_zalloc(hiusbc->xfer_seg_pool,
			gfp_flags, &seg->dma);
	if (!seg->trbs) {
		kfree(seg);
		return NULL;
	}

	if (pcs == 0) {
		for (i = 0; i < HIUSBC_TRBS_PER_XFER_SEG; i++)
			seg->trbs[i].transfer.ctrl |= HIUSBC_TRB_CYCLE;
	}

	/* The CH bit in LINK TRB is always 1. */
	seg->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.ctrl |= HIUSBC_TRB_CH;
	seg->next = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");

	return seg;
}

static void hiusbc_xfer_seg_free(struct hiusbc *hiusbc,
					struct hiusbc_xfer_seg *seg)
{
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	if (seg->trbs) {
		dma_pool_free(hiusbc->xfer_seg_pool, seg->trbs, seg->dma);
		seg->trbs = NULL;
	}
	kfree(seg);
}

static void hiusbc_link_xfer_segs(
	struct hiusbc_xfer_seg *prev, struct hiusbc_xfer_seg *next)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	if (!prev || !next)
		return;

	prev->next = next;
	prev->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.seg_ptrl =
					lower_32_bits(next->dma);
	prev->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.seg_ptrh =
					upper_32_bits(next->dma);
	reg = prev->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.ctrl;
	reg &= ~HIUSBC_TRB_TYPE_MASK;
	reg |= hiusbc_trb_type(HIUSBC_TRB_TYPE_LINK);
	prev->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.ctrl = reg;
}

struct hiusbc_xfer_ring *hiusbc_xfer_ring_alloc(struct hiusbc *hiusbc,
	unsigned int num_segs, unsigned int pcs, gfp_t gfp_flags)
{
	struct hiusbc_xfer_ring *ring = NULL;
	struct hiusbc_xfer_seg *prev = NULL;
	struct hiusbc_xfer_seg *next = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	ring = kzalloc(sizeof(struct hiusbc_xfer_ring), gfp_flags);
	if (!ring)
		return NULL;

	ring->num_segs = num_segs;

	if (num_segs == 0)
		return ring;

	ring->first_seg = hiusbc_xfer_seg_alloc(hiusbc, pcs, gfp_flags);
	if (!ring->first_seg) {
		kfree(ring);
		return NULL;
	}
	num_segs--;
	prev = ring->first_seg;
	while (num_segs > 0) {
		next = hiusbc_xfer_seg_alloc(hiusbc, pcs, gfp_flags);
		if (!next) {
			prev = ring->first_seg;
			while (prev) {
				next = prev->next;
				hiusbc_xfer_seg_free(hiusbc, prev);
				prev = next;
			}
			kfree(ring);
			return NULL;
		}
		hiusbc_link_xfer_segs(prev, next);
		prev = next;
		num_segs--;
	}
	ring->last_seg = prev;
	hiusbc_link_xfer_segs(ring->last_seg, ring->first_seg);

	ring->last_seg->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.ctrl |=
							HIUSBC_TRB_TC;

	ring->enqueue = ring->first_seg->trbs;
	ring->enqueue_seg = ring->first_seg;
	ring->dequeue = ring->enqueue;
	ring->dequeue_seg = ring->enqueue_seg;
	ring->pcs = pcs;
	ring->num_trbs_free =
			ring->num_segs * (HIUSBC_TRBS_PER_XFER_SEG - 1) - 1;

	INIT_LIST_HEAD(&ring->queue);

	return ring;
}

void hiusbc_xfer_ring_free(struct hiusbc *hiusbc,
				struct hiusbc_xfer_ring *ring)
{
	struct hiusbc_xfer_seg *cur_seg = NULL;
	struct hiusbc_xfer_seg *temp_seg = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	if (!ring)
		return;

	cur_seg = ring->first_seg;
	if (!cur_seg) {
		kfree(ring);
		return;
	}

	cur_seg = cur_seg->next;
	while (cur_seg != ring->first_seg) {
		temp_seg = cur_seg->next;
		hiusbc_xfer_seg_free(hiusbc, cur_seg);
		cur_seg = temp_seg;
	}
	hiusbc_xfer_seg_free(hiusbc, cur_seg);
	kfree(ring);

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");
}

/*
 * Cannot call this funciton
 * when (dequeue_seg == enqueue_seg) && (dequeue > enqueue).
 * and, we check this in prepare_ring().
 */
static int hiusbc_enlarge_xfer_ring(struct hiusbc *hiusbc,
					struct hiusbc_xfer_ring *ring,
					unsigned int num_trbs, gfp_t gfp_flags)
{
	struct hiusbc_xfer_ring *ring_new = NULL;
	struct hiusbc_xfer_seg *temp_seg = NULL;
	unsigned int num_segs;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	if (!num_trbs)
		return 0;

	num_segs = (num_trbs + (HIUSBC_TRBS_PER_XFER_SEG - 1) - 1) /
				(HIUSBC_TRBS_PER_XFER_SEG - 1);
	ring_new = hiusbc_xfer_ring_alloc(
			hiusbc, num_segs, ring->pcs, gfp_flags);
	if (!ring_new)
		return -ENOMEM;

	temp_seg = ring->enqueue_seg->next;
	hiusbc_link_xfer_segs(ring->enqueue_seg, ring_new->first_seg);
	hiusbc_link_xfer_segs(ring_new->last_seg, temp_seg);
	ring->num_segs += num_segs;
	ring->num_trbs_free += (HIUSBC_TRBS_PER_XFER_SEG - 1) * num_segs;

	if (ring->enqueue_seg == ring->last_seg) {
		ring->last_seg->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.ctrl
				&= ~HIUSBC_TRB_TC;
		ring->last_seg = ring_new->last_seg;
	} else {
		temp_seg = ring_new->last_seg;
		temp_seg->trbs[HIUSBC_TRBS_PER_XFER_SEG - 1].link.ctrl
			&= ~HIUSBC_TRB_TC;
	}

	kfree(ring_new);

	return 0;
}

static bool is_link_trb(const union hiusbc_trb *trb)
{
	return ((trb->link.ctrl & HIUSBC_TRB_TYPE_MASK) ==
				hiusbc_trb_type(HIUSBC_TRB_TYPE_LINK));
}

void xfer_ring_inc_deq(struct hiusbc_xfer_ring *ring)
{
	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"ring = 0x%p\n from dequeue = 0x%p, num_trbs_free = %u\n",
		ring, ring->dequeue, ring->num_trbs_free);

	if (!is_link_trb(ring->dequeue)) {
		ring->dequeue++;
		ring->num_trbs_free++;
	}

	while (is_link_trb(ring->dequeue)) {
		hiusbc_dbg(HIUSBC_DEBUG_XFER, "a link TRB\n");
		ring->dequeue_seg = ring->dequeue_seg->next;
		ring->dequeue = ring->dequeue_seg->trbs;
	}

	hiusbc_dbg(HIUSBC_DEBUG_XFER,
			"to dequeue = 0x%p, num_trbs_free = %u\n",
			ring->dequeue, ring->num_trbs_free);
}

static void xfer_ring_inc_enq(struct hiusbc_xfer_ring *ring)
{
	u32 chain = ring->enqueue->transfer.ctrl & HIUSBC_TRB_CH;
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "for ring = 0x%p\n", ring);

	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"From enqueue = 0x%p, num_trbs_free = %u\n",
			ring->enqueue, ring->num_trbs_free);

	if (!is_link_trb(ring->enqueue)) {
		ring->enqueue++;
		ring->num_trbs_free--;
	}

	if (!chain) {
		hiusbc_dbg(HIUSBC_DEBUG_XFER,
			"To enqueue = 0x%p, num_trbs_free = %u\n",
			ring->enqueue, ring->num_trbs_free);
		return;
	}

	while (is_link_trb(ring->enqueue)) {
		/*
		 * Note: Link TRB cannot be the last trb on a TD.
		 * We can't give the LINK TRB to HW if chain == 0.
		 * then, enqueue may point to a LINK TRB here,
		 * So, we must handle LINK TRB situation
		 * before call this enq function. (call prepare_ring function)
		 */
		reg = ring->enqueue->link.ctrl;
		reg &= ~HIUSBC_TRB_CYCLE;
		reg |= ring->pcs & HIUSBC_TRB_CYCLE;
		ring->enqueue->link.ctrl = reg;
		if (reg & HIUSBC_TRB_TC)
			ring->pcs ^= 1;

		ring->enqueue_seg = ring->enqueue_seg->next;
		ring->enqueue = ring->enqueue_seg->trbs;
	}
	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"To enqueue = 0x%p, num_trbs_free = %u\n",
		ring->enqueue, ring->num_trbs_free);
}

static unsigned int calc_trbs(u64 length)
{
	return ((length + HIUSBC_DATA_BUF_SIZE_MAX - 1) /
		HIUSBC_DATA_BUF_SIZE_MAX);
}

static unsigned int calc_trbs_needed(struct hiusbc_req *hreq)
{
	struct hiusbc_ep *hep = hreq->hep;
	unsigned int req_length_remain = hreq->req.length;
	unsigned int sg_length;
	unsigned int i;
	unsigned int trbs_needed = 0;
	struct scatterlist *sg = NULL;

	if (!hreq->req.num_sgs) {
		trbs_needed = calc_trbs(req_length_remain);
	} else {
		for_each_sg(hreq->req.sg, sg, hreq->req.num_mapped_sgs, i) {
			sg_length = sg_dma_len(sg);
			trbs_needed += calc_trbs(sg_length);
			if (sg_length < req_length_remain)
				req_length_remain -= sg_length;
			else
				break;
		}
	}

	if (hreq->req.zero
		&& (hreq->req.length % hep->ep.maxpacket == 0)
		&& (hreq->req.length > 0)) {
		/* Need to queue anoter 0-length TRB. */
		hreq->zlp = true;
		trbs_needed += 1;
	}

	if (usb_endpoint_xfer_control(hep->ep.desc))
		trbs_needed += 1;

	if (trbs_needed > 8)
		pr_err("!!!more than 8 TRBs in the TD!!!\n");

	return trbs_needed;
}

static bool is_trbs_on_ring_enough(struct hiusbc_xfer_ring *ring,
					unsigned int trbs_needed)
{
	unsigned int remaind = ring->dequeue - ring->dequeue_seg->trbs;
	/* make sure enqueue never goes into dequeue_seg. */
	return (ring->num_trbs_free >= trbs_needed + remaind);
}

static void skip_link_trb(struct hiusbc_xfer_ring *ring)
{
	u32 ctrl;

	while (is_link_trb(ring->enqueue)) {
		ctrl = ring->enqueue->link.ctrl;
		ctrl &= ~HIUSBC_TRB_CYCLE;
		ctrl |= ring->pcs & HIUSBC_TRB_CYCLE;
		ring->enqueue->link.ctrl = ctrl;

		if (ctrl & HIUSBC_TRB_TC)
			ring->pcs ^= 1;

		ring->enqueue_seg = ring->enqueue_seg->next;
		ring->enqueue = ring->enqueue_seg->trbs;
	}
}
static int hiusbc_prepare_xfer_ring(struct hiusbc *hiusbc,
					struct hiusbc_xfer_ring *ring,
					unsigned int trbs_needed,
					gfp_t gfp_flags)
{
	unsigned int num_trbs = trbs_needed;


	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	while (!is_trbs_on_ring_enough(ring, trbs_needed)) {
		num_trbs += ((ring->dequeue - ring->dequeue_seg->trbs) -
							ring->num_trbs_free);

		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Try to Enlarge Xfer Ring = %p, "
			"TRBs free = %u, TRBs needed = %u.\n",
			ring, ring->num_trbs_free, num_trbs);

		if (hiusbc_enlarge_xfer_ring(
				hiusbc, ring, num_trbs, gfp_flags))
			return -ENOMEM;

		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Enlarge Success, Xfer Ring = %p, TRBs free = %u.\n",
				ring, ring->num_trbs_free);
	}

	/*
	 * Enqueue may stop on link trb when prepare last TD,
	 * we handle it before preparing next TD.
	 */
	skip_link_trb(ring);

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");

	return 0;
}

static void hiusbc_queue_one_trb(struct hiusbc_xfer_ring *ring,
				u32 word0, u32 word1, u32 word2, u32 word3)
{
	struct hiusbc_generic_trb *trb = NULL;

	trb = &ring->enqueue->generic;

	trb->word[0] = word0;
	trb->word[1] = word1;
	trb->word[2] = word2;
	trb->word[3] = word3;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "Queue a TRB to Ring = 0x%p:\n", ring);
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "TRB addr = 0x%p\n", trb);
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "word0 = 0x%x\n", trb->word[0]);
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "word1 = 0x%x\n", trb->word[1]);
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "word2 = 0x%x\n", trb->word[2]);
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "word3 = 0x%x\n", trb->word[3]);

	xfer_ring_inc_enq(ring);
}

dma_addr_t hiusbc_xfer_trb_vrt_to_dma(
			const struct hiusbc_xfer_seg *seg,
			const union hiusbc_trb *trb)
{
	u32 offset;

	if (!seg || !trb || trb < seg->trbs ||
		trb - seg->trbs >= HIUSBC_TRBS_PER_XFER_SEG)
		return 0;

	offset = trb - seg->trbs;

	return seg->dma + (offset * sizeof(*trb));
}

static void hiusbc_move_request_to_ring(struct hiusbc_req *hreq)
{
	struct hiusbc_ep *hep = hreq->hep;
	struct hiusbc_xfer_ring *ring = hep->ring;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");
	/*
	 * For control xfer,
	 * we need to queue a invalid TRB at the head of every request.
	 * Both Data and Status Stage need this.
	 * So, keep the cycle bit != PCS, and only inc enqueue here.
	 * NOTE: Software needs to reclaim this TRB
	 * every time the Stage finished.
	 */
	if (usb_endpoint_xfer_control(hep->ep.desc)) {
		/* tell xfer_ring_inc_enq() there is more TRB coming. */
		ring->enqueue->transfer.ctrl |= HIUSBC_TRB_CH;
		xfer_ring_inc_enq(ring);
	}

	ring->num_queued_req++;

	hreq->queued_to_ring = true;
	hreq->start_seg = ring->enqueue_seg;
	hreq->first_trb = ring->enqueue;
	hreq->trb_dma =
		hiusbc_xfer_trb_vrt_to_dma(ring->enqueue_seg, ring->enqueue);
	hreq->dcs = ring->pcs;
	list_move_tail(&hreq->queue, &ring->queue);
}

static void hiusbc_remove_request_from_ring(
						struct hiusbc_req *hreq,
						int status)
{
	struct hiusbc_ep *hep = hreq->hep;
	struct hiusbc_xfer_ring *ring = hep->ring;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	hreq->zlp = false;

	list_del(&hreq->queue);

	if (hreq->req.status == -EINPROGRESS)
		hreq->req.status = status;

	if (!hreq->queued_to_ring)
		return;

	ring->num_queued_req--;

	hreq->queued_to_ring = false;
	hreq->start_seg = NULL;
	hreq->first_trb = NULL;
	hreq->trb_dma = 0;
	hreq->dcs = 0;

	hreq->last_trb = NULL;
	hreq->last_seg = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");
}

static bool hiusbc_map_req_buf(struct hiusbc *hiusbc,
			struct hiusbc_req *hreq, int is_in)
{
	int ret;

	if ((hreq->req.dma == DMA_ADDR_INVALID) || (hreq->epnum <= 1)) {
		ret = usb_gadget_map_request_by_dev(hiusbc->sysdev,
				&hreq->req, is_in);
		if (ret) {
			hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"failed to map request 0x%p\n", &hreq->req);
			return false;
		}

		hreq->dma_mapped = true;
	} else {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"request 0x%p: dma is not DMA_ADDR_INVALID\n",
			&hreq->req);
		hreq->req.dma_mapped = 0;
	}
	return true;
}

static void hiusbc_unmap_req_buf(struct hiusbc *hiusbc,
			struct hiusbc_req *hreq, int is_in)
{
	if (hreq->dma_mapped) {
		usb_gadget_unmap_request_by_dev(hiusbc->sysdev,
				&hreq->req, is_in);
		hreq->dma_mapped = false;
	}

	if ((hreq->req.dma_mapped) && (hreq->epnum > 1))
		hreq->req.dma = DMA_ADDR_INVALID;
}

static bool hiusbc_prepare_trb_for_req(
			struct hiusbc *hiusbc, struct hiusbc_ep *hep,
			struct hiusbc_req *hreq, gfp_t gfp_flags)
{
	int ret;
	unsigned int num_trbs_needed;

	if (!hiusbc_map_req_buf(hiusbc, hreq, hep->is_in))
		return false;

	num_trbs_needed = calc_trbs_needed(hreq);

	ret = hiusbc_prepare_xfer_ring(hiusbc, hep->ring,
					num_trbs_needed, gfp_flags);
	if (ret) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"faied to prepare xfer ring of ep%u\n", hep->epnum);
		hiusbc_unmap_req_buf(hiusbc, hreq, hep->is_in);
		return false;
	}

	hiusbc_move_request_to_ring(hreq);
	return true;
}

static void hiusbc_queue_trbs(struct hiusbc *hiusbc,
				struct hiusbc_ep *hep, dma_addr_t buf,
				unsigned int length, unsigned int total,
				bool zlp, gfp_t gfp_flags,
				union hiusbc_trb **last,
				struct hiusbc_xfer_seg **last_seg)
{
	unsigned int queued_length, trb_length, sent_length;
	bool first_trb = true;
	dma_addr_t buf_dma;
	u32 ctrl;

	buf_dma = buf;
	trb_length = length;

	for (queued_length = 0; first_trb || queued_length < total;
					queued_length += sent_length) {

		ctrl = hiusbc_trb_type(HIUSBC_TRB_TYPE_CTRL_DATA);
		if (hep->epnum > 1)
			ctrl = hiusbc_trb_type(HIUSBC_TRB_TYPE_NORMAL);

		sent_length = trb_length;
		if (sent_length > HIUSBC_DATA_BUF_SIZE_MAX)
			sent_length = HIUSBC_DATA_BUF_SIZE_MAX;

		if (queued_length + sent_length > total)
			sent_length = total - queued_length;

		if (first_trb) {
			first_trb = false;
			/* Change the cycle bit of the first TRB at last. */
			if (hep->ring->pcs == 0)
				ctrl |= HIUSBC_TRB_CYCLE;
		} else {
			ctrl |= hep->ring->pcs & HIUSBC_TRB_CYCLE;
		}

		if (queued_length + sent_length < total) {
			ctrl |= HIUSBC_TRB_CH;
		} else {
			if (!zlp) {
				ctrl |= HIUSBC_TRB_IOC;
				*last = hep->ring->enqueue;
				*last_seg = hep->ring->enqueue_seg;
			}
		}
		hiusbc_queue_one_trb(hep->ring,
			lower_32_bits(buf_dma),
			upper_32_bits((buf_dma)),
			hiusbc_trb_xfer_length(sent_length),
			ctrl);

		buf_dma += sent_length;
		trb_length -= sent_length;
	}

	if (zlp) {
		*last = hep->ring->enqueue;
		*last_seg = hep->ring->enqueue_seg;
		hiusbc_queue_one_trb(hep->ring,
			0, 0, 0, ctrl | HIUSBC_TRB_IOC);
		/*
		 * last TD may stopped at a link TRB,
		 * we need to jump over it
		 */
		skip_link_trb(hep->ring);
	}
}

static void hiusbc_queue_normal_trbs_for_req
				(struct hiusbc *hiusbc,
				struct hiusbc_ep *hep,
				struct hiusbc_req *hreq, gfp_t gfp_flags)
{
	union hiusbc_trb *last = NULL;
	struct hiusbc_xfer_seg *last_seg = NULL;

	hiusbc_queue_trbs(hiusbc, hep, hreq->req.dma, hreq->req.length,
				hreq->req.length, !!hreq->zlp, gfp_flags,
				&last, &last_seg);

	if (!last || !last_seg) {
		pr_err("queue noraml trbs fail!\n");
		return;
	}

	/* Change the Cycle bit of the first trb at last. */
	hreq->first_trb->transfer.ctrl &= ~HIUSBC_TRB_CYCLE;
	hreq->first_trb->transfer.ctrl |= hreq->dcs & HIUSBC_TRB_CYCLE;

	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"Queued a TD on EP%u for usb request = 0x%p, "
		"buf_dma = 0x%llx, length = %u, first TRB = 0x%p (0x%llx dma),"
		" last TRB = 0x%p (0x%llx dma)\n",
		hep->epnum,
		&hreq->req, hreq->req.dma, hreq->req.length,
		hreq->first_trb, hreq->trb_dma,
		hreq->last_trb,
		hiusbc_xfer_trb_vrt_to_dma(hreq->last_seg, hreq->last_trb));
}

static void hiusbc_queue_sg_trbs_for_req(struct hiusbc *hiusbc,
				struct hiusbc_ep *hep,
				struct hiusbc_req *hreq, gfp_t gfp_flags)
{
	union hiusbc_trb *last = NULL;
	struct hiusbc_xfer_seg *last_seg = NULL;
	unsigned int num_sgs, i;
	struct scatterlist *sg = NULL;

	num_sgs = hreq->req.num_mapped_sgs;
	sg = hreq->req.sg;
	for (i = 0; i < num_sgs; i++) {
		hiusbc_queue_trbs(hiusbc,
				hep, sg_dma_address(sg), sg_dma_len(sg),
				hreq->req.length, !!hreq->zlp, gfp_flags,
				&last, &last_seg);
		sg = sg_next(sg);

		if (last || last_seg) {
			pr_err("queue sg trbs, maybe something wrong\n");
			return;
		}
	}

	if (!last || !last_seg) {
		pr_err("queue sg trbs fail!\n");
		return;
	}

	/* Change the Cycle bit of the first trb at last. */
	hreq->first_trb->transfer.ctrl &= ~HIUSBC_TRB_CYCLE;
	hreq->first_trb->transfer.ctrl |= hreq->dcs & HIUSBC_TRB_CYCLE;

	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"Queued a TD on EP%u for usb request = 0x%p, "
		"buf_dma = 0x%llx, length = %u, first TRB = 0x%p (0x%llx dma),"
		" last TRB = 0x%p (0x%llx dma)\n",
		hep->epnum,
		&hreq->req, hreq->req.dma, hreq->req.length,
		hreq->first_trb, hreq->trb_dma,
		hreq->last_trb,
		hiusbc_xfer_trb_vrt_to_dma(hreq->last_seg, hreq->last_trb));
}

static void hiusbc_queue_trbs_for_req(struct hiusbc *hiusbc,
				struct hiusbc_ep *hep,
				struct hiusbc_req *hreq, gfp_t gfp_flags)
{
	if (hreq->req.num_sgs)
		hiusbc_queue_sg_trbs_for_req(hiusbc, hep, hreq, gfp_flags);
	else
		hiusbc_queue_normal_trbs_for_req(hiusbc, hep, hreq, gfp_flags);
}

/* Convert requests pending on ep to TDs on xfer ring. */
static bool hiusbc_request_to_trbs(struct hiusbc_ep *hep,
					gfp_t gfp_flags)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_req *n = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	if (list_empty(&hep->queue)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"hep->queue for EP%u is empty!\n", hep->epnum);
		return false;
	}

	/* We may delete request in progress, so we use _safe here. */
	list_for_each_entry_safe(hreq, n, &hep->queue, queue) {
		if (hreq->queued_to_ring)
			continue;

		if (!hiusbc_prepare_trb_for_req(hiusbc, hep, hreq, gfp_flags))
			return false;

		hiusbc_queue_trbs_for_req(hiusbc, hep, hreq, gfp_flags);
	}

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");
	return true;
}

void hiusbc_finish_req(struct hiusbc_req *hreq, int status)
{
	struct hiusbc_ep *hep = hreq->hep;
	struct hiusbc *hiusbc = hep->hiusbc;
	u8 dma_mapped;
	u8 *temp_p_buf = NULL;
	u32 temp_actual;

	hiusbc_remove_request_from_ring(hreq, status);

	dma_mapped = hreq->req.dma_mapped;

	if (hreq->dma_mapped == true) {
		usb_gadget_unmap_request_by_dev(
			hiusbc->sysdev, &hreq->req, hep->is_in);
		hreq->dma_mapped = false;
	}

	if (dma_mapped && (hreq->epnum > 1))
		hreq->req.dma = DMA_ADDR_INVALID;

	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"Giveback request 0x%p on EP%u, actual = %u, status = %d\n",
		&hreq->req, hreq->epnum, hreq->req.actual, hreq->req.status);

	temp_actual = hreq->req.actual;
	if (!status && (temp_actual > 0)) {
		hiusbc_dbg(HIUSBC_DEBUG_XFER, "Confirm buffer content:\n");
		temp_p_buf = (u8 *)hreq->req.buf;
		hiusbc_dbg(HIUSBC_DEBUG_XFER,
			"first = 0x%x, middle = 0x%x, last = 0x%x\n",
			temp_p_buf[0], temp_p_buf[(temp_actual - 1) / 2],
			temp_p_buf[temp_actual - 1]);
	}

	if (hep->epnum == 1) {
		/*
		 * usb core always queue ctrl request to EP0.
		 * but, for data stage of ctrl IN,
		 * we controller driver had moved this request from EP0 to EP1.
		 * so we need to move back to ep0
		 * before we giveback this request to usb core.
		 */
		hep = hiusbc->eps[0];
		hreq->hep = hep;
		hreq->epnum = hep->epnum;
	}

	spin_unlock(&hiusbc->lock);

	usb_gadget_giveback_request(&hep->ep, &hreq->req);

	spin_lock(&hiusbc->lock);

	if (hep->epnum > 1)
		pm_runtime_put(hiusbc->dev);
}

/*
 * For control xfer, we have queued a invalid TRB at the head of every request.
 * So, software needs to reclaim that TRB here.
 */
void hiusbc_reclaim_invalid_trb_in_ctrl(
				struct hiusbc_xfer_ring *ring)
{
	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "+\n");

	if (ring->dequeue->transfer.ctrl & HIUSBC_TRB_CYCLE)
		ring->dequeue->transfer.ctrl &= ~HIUSBC_TRB_CYCLE;
	else
		ring->dequeue->transfer.ctrl |= HIUSBC_TRB_CYCLE;

	xfer_ring_inc_deq(ring);
}

void hiusbc_cleanup_exist_req_on_ep(
			struct hiusbc_ep *hep, int status)
{
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_req *n = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");
	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "EP = %u\n", hep->epnum);

	if (!list_empty(&hep->queue)) {
		list_for_each_entry_safe(hreq, n, &hep->queue, queue) {
			hiusbc_finish_req(hreq, status);
		}
	}

	if (!list_empty(&ring->queue)) {
		list_for_each_entry_safe(hreq, n, &ring->queue, queue) {
			if (usb_endpoint_xfer_control(hep->ep.desc))
				hiusbc_reclaim_invalid_trb_in_ctrl(ring);

			while (ring->dequeue != hreq->last_trb)
				xfer_ring_inc_deq(ring);

			xfer_ring_inc_deq(ring);

			hiusbc_finish_req(hreq, status);
		}
	}

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");
}

static void hiusbc_xfer_trb_info_print(
			const struct hiusbc_xfer_trb *trb, u32 level)
{
	hiusbc_dbg(level, "Confirm TRB addr = 0x%p:\n", trb);
	hiusbc_dbg(level, "word0 = 0x%x\n", trb->buf_ptrl);
	hiusbc_dbg(level, "word1 = 0x%x\n", trb->buf_ptrh);
	hiusbc_dbg(level, "word2 = 0x%x\n", trb->length);
	hiusbc_dbg(level, "word3 = 0x%x\n", trb->ctrl);
}

int hiusbc_queue_req_bulk_int(
			struct hiusbc_ep *hep, gfp_t gfp_flags)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc_req *hreq = NULL;
	u8 init;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_BULK, "+\n");

	if (hep->stalled || hep->set_halt_pending) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"EP is stalled, can't queue trbs to transfe ring.\n");
		return 0;
	}

	if (hep->xfer_state == HIUSBC_XFER_END) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"EndXfer cmd pending, can't queue trbs\n");
		return 0;
	}

	init = (hep->xfer_state == HIUSBC_XFER_IDLE);

	if (!hiusbc_request_to_trbs(hep, gfp_flags)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Bulk_Int EP%u ran out of request\n", hep->epnum);
		/*
		 * didn't queue any new TRB, send StartXfer or not?
		 * borrow db_on_received_event flag.
		 */
		if (!hiusbc->db_on_received_event)
			return 0;
	}

	hreq = list_first_entry_or_null(&ring->queue,
			struct hiusbc_req, queue);
	if (!hreq)
		return 0;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP,
		"Ring is not empty, send StartXfer cmd. init = %u\n", init);

	hiusbc_xfer_trb_info_print(&hreq->first_trb->transfer,
					HIUSBC_DEBUG_BULK);
	hiusbc_xfer_trb_info_print(&hreq->last_trb->transfer,
					HIUSBC_DEBUG_BULK);

	ret = hiusbc_send_cmd_start_transfer(hep, 0,
			hreq->trb_dma, hreq->dcs, init, 0);

	if (ret < 0) {
		pr_err("Failed to send StartXfer on bulk_int EP%u! ret = %d\n",
			hep->epnum, ret);
		hiusbc_cleanup_exist_req_on_ep(hep, -ESHUTDOWN);
		return ret;
	}

	hep->xfer_state = HIUSBC_XFER_INPROGRESS;

	hiusbc_dbg(HIUSBC_DEBUG_BULK, "-\n");

	return 0;
}

int hiusbc_queue_req_isoc(struct hiusbc_ep *hep, gfp_t gfp_flags)
{
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc_req *hreq = NULL;
	u8 init;
	int cur_mf;
	u32 start_mf;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_ISOC,
		"xfer_state = %d, nrdy_received = %d\n",
		hep->xfer_state, hep->nrdy_received);

	if (hep->stalled == true) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Miss Service error, can't queue trbs\n");
		return 0;
	}

	if (hep->xfer_state == HIUSBC_XFER_END) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Can't queue, EndXfer cmd pending for EP%u.\n",
			hep->epnum);
		return 0;
	}

	if (hep->xfer_state == HIUSBC_XFER_IDLE && !hep->nrdy_received) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "Can't queue, ep state is idle\n",
			hep->epnum);
		return 0;
	}

	init = (hep->xfer_state == HIUSBC_XFER_IDLE);

	if (!hiusbc_request_to_trbs(hep, gfp_flags)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "Isoc ran out of request.\n");
		return 0;
	}

	hreq = list_first_entry_or_null(&ring->queue,
		struct hiusbc_req, queue);
	if (!hreq)
		return 0;

	cur_mf = hiusbc_get_frame_id(hep->hiusbc);
	start_mf = cur_mf + HIUSBC_DEV_IST;
	start_mf = (start_mf & ~(hep->si - 1)) + hep->si;

	hiusbc_dbg(HIUSBC_DEBUG_ISOC,
		"cur_mf = %d, start_mf = %u\n", cur_mf, start_mf);

	hiusbc_xfer_trb_info_print(&hreq->first_trb->transfer,
					HIUSBC_DEBUG_ISOC);

	hiusbc_xfer_trb_info_print(&hreq->last_trb->transfer,
					HIUSBC_DEBUG_ISOC);

	ret = hiusbc_send_cmd_start_transfer(hep,
		start_mf, hreq->trb_dma, hreq->dcs, init, 0);

	if (ret < 0) {
		pr_err("Failed to send StartXfer cmd on isoc EP%u! ret = %d\n",
				hep->epnum, ret);
		hiusbc_cleanup_exist_req_on_ep(hep, -ESHUTDOWN);
		return 0;
	}

	hep->nrdy_received = false;
	hep->nrdy_mf = 0;

	hep->xfer_state = HIUSBC_XFER_INPROGRESS;

	hiusbc_dbg(HIUSBC_DEBUG_ISOC, "-\n");

	return 0;
}

static void hiusbc_ctrl_status_stall_cmplt(
			struct usb_ep *ep, struct usb_request *req)
{
	struct hiusbc_ep *hep = to_hiusbc_ep(ep);
	struct hiusbc *hiusbc = hep->hiusbc;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "+\n");

	hiusbc->ep0_stall_pending = false;

	if (!req->status) {
		hiusbc->eps[0]->stalled = true;
		hiusbc->eps[1]->stalled = true;
		hiusbc->next_stage = HIUSBC_SETUP_STAGE;
	}

	hiusbc_gadget_ep_free_request(ep, req);
}

static void hiusbc_ctrl_status_ack_cmplt(
			struct usb_ep *ep, struct usb_request *req)
{
	struct hiusbc_ep *hep = to_hiusbc_ep(ep);
	struct hiusbc *hiusbc = hep->hiusbc;
	struct usb_ctrlrequest *setup = &hiusbc->setup_packet;
	u8 addr;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "req->status = %d\n", req->status);

	if (!req->status) {
		hiusbc->next_stage = HIUSBC_SETUP_STAGE;

		if ((setup->bRequest == USB_REQ_SET_ADDRESS) &&
			((setup->bRequestType & USB_RECIP_MASK) ==
						USB_RECIP_DEVICE)) {
			addr = (u8)(setup->wValue & 0x7f);

			if (addr) {
				usb_gadget_set_state(&hiusbc->gadget,
							USB_STATE_ADDRESS);
			} else {
				usb_gadget_set_state(&hiusbc->gadget,
							USB_STATE_DEFAULT);
			}

			hiusbc_dbg(HIUSBC_DEBUG_SYS,
				"Success to set address = 0x%x\n", addr);
		} else if ((setup->bRequest == USB_REQ_SET_FEATURE) &&
			((setup->bRequestType & USB_RECIP_MASK) ==
							USB_RECIP_DEVICE) &&
			(setup->wValue == USB_DEVICE_TEST_MODE)) {
			if (hiusbc_req_set_test_mode(hiusbc,
							hiusbc->test_mode))
				hiusbc_dbg(HIUSBC_DEBUG_ERR,
					"Failed to set Test Mode!");
		}
	} else {
		if ((setup->bRequest == USB_REQ_SET_ADDRESS) &&
			((setup->bRequestType & USB_RECIP_MASK) ==
					USB_RECIP_DEVICE)) {
			addr = (u8)(setup->wValue & 0x7f);

			hiusbc_dbg(HIUSBC_DEBUG_SYS,
				"Failed to set address = 0x%x\n", addr);
		}
	}

	hiusbc_gadget_ep_free_request(ep, req);
}

int hiusbc_queue_req_ctrl_status(struct hiusbc *hiusbc,
					bool is_stall, gfp_t gfp_flags)
{
	struct hiusbc_ep *hep = NULL;
	struct hiusbc_xfer_ring *ring = NULL;
	struct hiusbc_req *hreq = NULL;
	struct usb_ep *ep = NULL;
	struct usb_request *status_req = NULL;
	u32 ctrl;
	int ret;

	struct usb_ctrlrequest *setup = &hiusbc->setup_packet;
	u8 addr;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "+\n");

	if (hiusbc->gadget.speed >= USB_SPEED_SUPER)
		hep = hiusbc->eps[1];
	else
		hep = hiusbc->eps[!!hiusbc->status_stage_dir];

	ep = &hep->ep;
	ring = hep->ring;

	status_req = hiusbc_gadget_ep_alloc_request(ep, gfp_flags);

	if (is_stall)
		status_req->complete = hiusbc_ctrl_status_stall_cmplt;
	else
		status_req->complete = hiusbc_ctrl_status_ack_cmplt;

	hreq = to_hiusbc_req(status_req);

	hreq->req.status = -EINPROGRESS;
	hreq->epnum = hep->epnum;

	list_add_tail(&hreq->queue, &hep->queue);

	ret = hiusbc_prepare_xfer_ring(hiusbc, ring, 1, gfp_flags);
	if (ret) {
		hiusbc_cleanup_exist_req_on_ep(hep, -ECONNRESET);
		return ret;
	}

	hiusbc_move_request_to_ring(hreq);

	ctrl = hiusbc_trb_type(HIUSBC_TRB_TYPE_CTRL_STATUS) |
		hiusbc_trb_response(is_stall) | HIUSBC_TRB_IOC |
		(ring->pcs & HIUSBC_TRB_CYCLE);

	if ((setup->bRequest == USB_REQ_SET_ADDRESS) &&
		((setup->bRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE) &&
		(!is_stall)) {
		addr = (u8)(setup->wValue & 0x7f);
		pr_info("Set addr to 0x%x by ACK TRB\n", addr);
		ctrl |= HIUSBC_TRB_DEV_ADDR_EN | hiusbc_trb_dev_addr(addr);
	}

	hreq->last_trb = ring->enqueue;
	hreq->last_seg = ring->enqueue_seg;

	hiusbc_queue_one_trb(ring, 0, 0, 0, ctrl);

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"Queued a %s TD on EP%u for request = %p, "
		"first TRB = %p (0x%llx dma), last TRB = %p (0x%llx dma).\n",
		is_stall ? "STALL" : "ACK",
		hep->epnum,
		&hreq->req,
		hreq->first_trb, hreq->trb_dma,
		hreq->last_trb,
		hiusbc_xfer_trb_vrt_to_dma(hreq->last_seg, hreq->last_trb));

	/* For ctrl xfer, SW should always send StartXfer with init = 1. */
	ret = hiusbc_send_cmd_start_transfer(hep, 0,
		hreq->trb_dma, hreq->dcs, 1, hiusbc->ctrl_num);
	if (ret < 0) {
		pr_err("Failed to send StartXfer on ctrl EP%u "
			"for status stage! ret = %d\n", hep->epnum, ret);
		hiusbc_cleanup_exist_req_on_ep(hep, -ESHUTDOWN);
		return ret;
	}

	if (is_stall)
		hiusbc->ep0_stall_pending = true;

	hiusbc->cur_stage = HIUSBC_STATUS_STAGE;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "-\n");

	return ret;
}

static int hiusbc_queue_req_ctrl_data(
			struct hiusbc_ep *hep, gfp_t gfp_flags)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc_req *hreq = NULL;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "+\n");

	if (!hiusbc_request_to_trbs(hep, gfp_flags))
		return 0;

	hreq = list_first_entry_or_null(&ring->queue,
		struct hiusbc_req, queue);
	if (!hreq)
		return 0;

	hiusbc_xfer_trb_info_print(&hreq->first_trb->transfer,
					HIUSBC_DEBUG_CTRL);
	hiusbc_xfer_trb_info_print(&hreq->last_trb->transfer,
					HIUSBC_DEBUG_CTRL);

	ret = hiusbc_send_cmd_start_transfer(hep, 0,
			hreq->trb_dma, hreq->dcs, 1, hiusbc->ctrl_num);

	if (ret < 0) {
		pr_err("Failed to send StartXfer cmd on ctrl EP%u "
			"for data stage! ret = %d\n", hep->epnum, ret);

		/*
		 * Anyway, we can't return non-zero for now,
		 * because we have queued this TD to Xfer Ring.
		 * we must cleanup this TD,
		 * if we want to return a non-zero error code.
		 */
		goto exit;
	}

	hiusbc->cur_stage = HIUSBC_DATA_STAGE;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "-\n");
	return 0;

exit:
	hiusbc_cleanup_exist_req_on_ep(hep, 0);
	return ret;
}

static bool hiusbc_check_req_busy(struct hiusbc_ep *_hep,
				struct list_head *hreq)
{
	struct hiusbc_ep *hep = _hep;
	struct hiusbc_xfer_ring *ring = hep->ring;
	struct hiusbc *hiusbc = hep->hiusbc;
	struct list_head *n = NULL;

	list_for_each(n, &hep->queue) {
		if (n == hreq)
			return true;
	}

	list_for_each(n, &ring->queue) {
		if (n == hreq)
			return true;
	}

	if (hep->epnum == 0) {
		hep = hiusbc->eps[1];
		ring = hep->ring;

		list_for_each(n, &hep->queue) {
			if (n == hreq)
				return true;
		}

		list_for_each(n, &ring->queue) {
			if (n == hreq)
				return true;
		}
	}

	return false;
}

int hiusbc_ep_queue(struct usb_ep *ep, struct usb_request *req)
{
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_ep *hep = NULL;
	struct hiusbc *hiusbc = NULL;
	unsigned int trbs_needed;
	int ret;
	int temp_ep_type;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	temp_ep_type = usb_endpoint_type(ep->desc);
	hiusbc_dbg(HIUSBC_DEBUG_XFER, "ep type = %d\n", temp_ep_type);

	if (!ep || !req) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Target ep or req not allocated!\n");
		return -EINVAL;
	}

	hep = to_hiusbc_ep(ep);
	hiusbc = hep->hiusbc;
	hreq = to_hiusbc_req(req);

	if (!hep->ep.desc) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"EP%u not configured, can't queue request!\n",
			hep->epnum);
		return -ESHUTDOWN;
	}

	if (hiusbc_check_req_busy(hep, &hreq->queue)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Request busy! request = 0x%p\n", req);
		return -EBUSY;
	}

	hreq->req.actual = 0;
	hreq->req.status = -EINPROGRESS;
	hreq->hep = hep;
	hreq->epnum = hep->epnum;

	trbs_needed = calc_trbs_needed(hreq);

	if (usb_endpoint_xfer_isoc(ep->desc) &&
			(trbs_needed > HIUSBC_TRB_NUM_IN_ISOC_TD_MAX)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"A TD can't exceed %d TRBs for Isoc EP%u.\n",
			HIUSBC_TRB_NUM_IN_ISOC_TD_MAX, hep->epnum);
		ret = -E2BIG;
		goto out;
	}

	if ((req->length < ep->maxpacket) &&
			(trbs_needed > HIUSBC_TRB_NUM_FOR_ONE_PACKET_MAX)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"A Packet can't exceed %d TRBs for EP%u.\n",
			HIUSBC_TRB_NUM_FOR_ONE_PACKET_MAX, hep->epnum);
		ret = -E2BIG;
		goto out;
	}

	if (usb_endpoint_xfer_control(ep->desc)) {
		if (!hiusbc->need_data_stage ||
			hiusbc->next_stage != HIUSBC_DATA_STAGE) {
			ret = 0;
			goto out;
		}

		if (hiusbc->data_stage_dir) {
			hiusbc_dbg(HIUSBC_DEBUG_CTRL,
				"Move request = 0x%p from EP0 to EP1",
				&hreq->req);
			hep = hiusbc->eps[1];
			hreq->hep = hep;
			hreq->epnum = hep->epnum;
		}

		if (!list_empty(&hep->queue) ||
				!list_empty(&hep->ring->queue)) {
			hiusbc_dbg(HIUSBC_DEBUG_ERR, "Ctrl EP%u is busy!\n",
				hep->epnum);
			ret = -EBUSY;
			goto out;
		}
	}

	list_add_tail(&hreq->queue, &hep->queue);

	hiusbc_dbg(HIUSBC_DEBUG_XFER,
		"Queued a request = 0x%p for EP%u, length = %u, zero = %u, "
		"no_int = %u, short_not_ok = %u.\n",
		req, hep->epnum, req->length, req->zero,
		req->no_interrupt, req->short_not_ok);

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "trbs_needed = %u.\n", trbs_needed);

	if (usb_endpoint_xfer_control(ep->desc)) {
		hiusbc->delayed_status = false;
		ret = hiusbc_queue_req_ctrl_data(hep, GFP_ATOMIC);
	} else if (usb_endpoint_xfer_isoc(ep->desc)) {
		ret = hiusbc_queue_req_isoc(hep, GFP_ATOMIC);
	} else {
		ret = hiusbc_queue_req_bulk_int(hep, GFP_ATOMIC);
	}

	pm_runtime_get(hiusbc->dev);

out:
	return ret;
}

int hiusbc_ep_dequeue(struct usb_ep *ep, struct usb_request *req)
{
	struct hiusbc_req *hreq = to_hiusbc_req(req);
	struct hiusbc_ep *hep = NULL;
	struct hiusbc_xfer_ring *ring = NULL;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	/*
	 * For data stage of ctrl IN,
	 * controller driver had moved the request from EP0 to EP1
	 * so we can't use to_hiusbc_ep(ep) here.
	 */
	hep = hreq->hep;
	ring = hep->ring;

	list_for_each_entry(hreq, &hep->queue, queue) {
		if (&hreq->req == req) {
			hiusbc_finish_req(hreq, -ECONNRESET);
			return 0;
		}
	}

	hreq = list_first_entry_or_null(&ring->queue,
		struct hiusbc_req, queue);
	if (&hreq->req != req)
		return -EINVAL;

	if (hep->xfer_state != HIUSBC_XFER_END) {
		ret = hiusbc_send_cmd_end_transfer(hep, true);
		if ((ret < 0) && (!hep->end_xfer_pending))
			return ret;
	}

	pr_err("Success endxfer cmplt\n");

	if (hreq->queued_to_ring) {
		pr_err("Start giveback request\n");
		/*
		 * Double check whether this request is still on the ring.
		 * because it may had been giveback in endxfer cmd cmplt event.
		 */
		if (usb_endpoint_xfer_control(hep->ep.desc))
			hiusbc_reclaim_invalid_trb_in_ctrl(ring);

		while (ring->dequeue != hreq->last_trb)
			xfer_ring_inc_deq(ring);

		xfer_ring_inc_deq(ring);

		hiusbc_finish_req(hreq, -ECONNRESET);
	}

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "-\n");
	return 0;
}
