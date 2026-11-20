// SPDX-License-Identifier: GPL-2.0
/*
 * D2D RX buffer implementation
 */

#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "buffer.h"
#include "rx_buffer.h"
#include "dynamic_list.h"
#include "wrappers.h"

static void rx_buf_panic_dump(const struct rx_buffer *rxb)
{
	size_t i = 0;
	const struct dlist *dlist = rxb->dlist;
	const struct dlist_node *node = NULL;

	pr_err("RXB panic dump:\n");
	pr_err("rpos=%u offset=%zu,\n", rxb->reader_pos, rxb->node_read_offset);
	list_for_each_entry (node, &dlist->list, list) {
		wrap_t l_id = node->seq_id;
		wrap_t r_id = l_id + node->len;

		pr_err("[%3zu]: [%9u, %9u) %4uB %s\n", i, l_id, r_id, node->len,
		       (node == rxb->last_sequential_node) ? "<-- LSEQ" : "");
		++i;
	}
}

struct rx_buffer *rx_buf_alloc(size_t bufsize, size_t elem_size)
{
	struct rx_buffer *rxb = NULL;

	rxb = vzalloc(sizeof(*rxb));
	if (!rxb)
		goto out;

	rxb->dlist = dlist_alloc(bufsize, elem_size);
	if (!rxb->dlist)
		goto out_rxb;

	rxb->reader_pos = 0;
	rxb->node_read_offset = 0;
	rxb->last_sequential_node = NULL;
	rxb->available_bytes = 0;
	return rxb;

out_rxb:
	vfree(rxb);
out:
	return NULL;
}

void rx_buf_free(struct rx_buffer *rxb)
{
	if (!rxb)
		return;

	dlist_free(rxb->dlist);
	vfree(rxb);
}

bool rx_buf_has_data(const struct rx_buffer *rxb)
{
	struct dlist_node *node = NULL;
	struct dlist *dlist     = rxb->dlist;
	u32 reader_pos          = rxb->reader_pos;

	/* Check if there are some data left in the first node */
	if (rxb->node_read_offset)
		return true;

	/* Then check the node */
	node = list_first_entry_or_null(&dlist->list, struct dlist_node, list);
	if (!node)
		return false;

	return reader_pos == node->seq_id;
}

size_t rx_buf_get_available_bytes(const struct rx_buffer *rxb)
{
	return rxb->available_bytes;
}

uint32_t rx_buf_get_ack(const struct rx_buffer *rxb)
{
	const struct dlist_node *last_seq = rxb->last_sequential_node;

	if (!last_seq)
		return rxb->reader_pos;

	return last_seq->seq_id + last_seq->len;
}

size_t rx_buf_get_size(const struct rx_buffer *rxb)
{
	return dlist_get_size(rxb->dlist);
}

static void __append_sack_pair(struct sack_pairs *s_pairs, u32 start, u32 end)
{
	size_t s_len = s_pairs->len;

	s_pairs->pairs[s_len].l = start;
	s_pairs->pairs[s_len].r = end;
	s_pairs->len += 1;
}

static bool __check_sack_pair_len_limits(const struct sack_pairs *s_pairs,
					 size_t len)
{
	return s_pairs->len + 1 <= len;
}

void rx_buf_generate_sack_pairs(const struct rx_buffer *rxb,
				struct sack_pairs *s_pairs,
				size_t len)
{
	struct dlist *dlist = rxb->dlist;
	const struct dlist_node *iter = NULL;
	u32 start_seq = 0;
	u32 end_seq   = 0;

	if (list_empty(&dlist->list)) {
		s_pairs->len = 0;
		return;
	}

	/* get last node from contiguous area */
	iter = rxb->last_sequential_node;
	if (!iter) {
		/* there is hole in the beginning */
		iter = list_first_entry(&dlist->list, struct dlist_node, list);
	} else {
		/* check whether next node exists */
		if (list_is_last(&iter->list, &dlist->list)) {
			s_pairs->len = 0;
			return;
		}
		iter = list_next_entry(iter, list);
	}

	/* set initial seq ids */
	start_seq = iter->seq_id;
	end_seq = start_seq + iter->len;

	/* continue after last acked node */
	list_for_each_entry_continue(iter, &dlist->list, list) {
		if (d2d_wrap_eq(iter->seq_id, end_seq)) {
			end_seq = iter->seq_id + iter->len;
			continue;
		}

		/*
		 * sack pair end found, check if there is enough space to
		 * store it
		 */
		if (!__check_sack_pair_len_limits(s_pairs, len))
			return;

		__append_sack_pair(s_pairs, start_seq, end_seq - 1);

		/* new sack pair started */
		start_seq = iter->seq_id;
		end_seq = start_seq + iter->len;
	}

	/* append last pair found if there is enough space */
	if (__check_sack_pair_len_limits(s_pairs, len))
		__append_sack_pair(s_pairs, start_seq, end_seq - 1);
}

size_t rx_buf_get(struct rx_buffer *rxb, void *data, size_t len)
{
	struct dlist_node *node = NULL;
	struct dlist_node *next = NULL;
	ssize_t total_copied = 0;

	list_for_each_entry_safe(node, next, &rxb->dlist->list, list) {
		size_t offset  = 0;
		size_t to_copy = 0;

		if (d2d_wrap_gt(node->seq_id, rxb->reader_pos + total_copied))
			break;

		/*
		 * The result of subtraction is non-negative
		 * (total_copied <= len, node_read_offset <= len)
		 */
		offset = rxb->node_read_offset;
		to_copy = min(len - total_copied, node->len - offset);
		memcpy(data + total_copied, node->data + offset, to_copy);
		rxb->node_read_offset += to_copy;
		total_copied += to_copy;

		/* This node has no data anymore */
		if (rxb->node_read_offset == node->len) {
			rxb->node_read_offset = 0;

			if (rxb->last_sequential_node == node)
				rxb->last_sequential_node = NULL;

			list_del(&node->list);
			dlist_node_free(rxb->dlist, node);
		}

		if (total_copied == len)
			break;
	}

	rxb->reader_pos += total_copied;
	rxb->available_bytes -= total_copied;

	return total_copied;
}

static void rx_buf_update_last_sequential_node(struct rx_buffer *rxb)
{
	struct dlist *dlist = rxb->dlist;
	struct dlist_node *node = NULL;
	u32 ack_id = 0;

	node = rxb->last_sequential_node;
	if (!node) {
		/*
		 * If last_sequential_node is NULL check whether the first node
		 * in the list is sequential.
		 */
		node = list_first_entry(&dlist->list, struct dlist_node, list);

		/* if first node is not sequential no need to check others */
		if (!d2d_wrap_eq(rxb->reader_pos, node->seq_id))
			return;

		rxb->last_sequential_node = node;
		rxb->available_bytes = node->len;
	}

	/* Starting from last_sequential_node try to move this pointer */
	ack_id = node->seq_id + node->len;
	list_for_each_entry_continue(node, &dlist->list, list) {
		if (node->seq_id != ack_id)
			break;

		ack_id = node->seq_id + node->len;
		rxb->last_sequential_node = node;
		rxb->available_bytes += node->len;
	}
}

static int rx_buf_insert_node_traverse(struct rx_buffer *rxb,
				       struct dlist_node *node)
{
	struct dlist *dlist = rxb->dlist;
	struct dlist_node *iter = NULL;
	wrap_t node_l = node->seq_id;
	wrap_t node_r = node->seq_id + node->len;

	/* select the starting point for traversing the list */
	iter = rxb->last_sequential_node;
	if (iter) {
		/* there are some already-ordered packets in the beginning */
		if (d2d_wrap_le(node->seq_id, iter->seq_id)) {
			/*
			 * Check fast path first. We don't compare against all
			 * the packets because it damages the performance so
			 * just return DUPLICATE here.
			 */
			dlist_node_free(dlist, node);
			return RX_BUF_PUT_DUPLICATE;
		}
		/* fast path failed - traverse the list from last seq node */
	} else {
		/* otherwise traverse the list from the beginning */
		iter = list_first_entry(&dlist->list, struct dlist_node, list);
	}

	/* find place to insert new node */
	list_for_each_entry_from(iter, &dlist->list, list) {
		wrap_t iter_l = iter->seq_id;
		wrap_t iter_r = iter->seq_id + iter->len;

		if (d2d_wrap_gt(node_l, iter_l)) {
			if (d2d_wrap_ge(node_l, iter_r))
				continue;
			else
				goto bad_packet;
		} else if (d2d_wrap_lt(node_l, iter_l)) {
			if (d2d_wrap_le(node_r, iter_l))  {
				list_add_tail(&node->list, &iter->list);
				rx_buf_update_last_sequential_node(rxb);
				return RX_BUF_PUT_OK;
			} else {
				goto bad_packet;
			}
		} else {
			/* node_l == iter_l */
			if (d2d_wrap_eq(node_r, iter_r)) {
				dlist_node_free(dlist, node);
				return RX_BUF_PUT_DUPLICATE;
			} else {
				goto bad_packet;
			}
		}
	}

bad_packet:
	WARN(true, "packet does not fit: [%u, %u)\n", node_l, node_r);
	rx_buf_panic_dump(rxb);
	dlist_node_free(dlist, node);
	return RX_BUF_PUT_BAD_PACKET;
}

static int rx_buf_insert_node(struct rx_buffer *rxb, struct dlist_node *node)
{
	struct dlist *dlist = rxb->dlist;
	struct dlist_node *last  = NULL;
	struct dlist_node *first = NULL;

	/* simplest case: the list is empty - just add a node */
	if (list_empty(&dlist->list)) {
		list_add(&node->list, &dlist->list);
		rx_buf_update_last_sequential_node(rxb);
		return RX_BUF_PUT_OK;
	}

	first = list_first_entry(&dlist->list, struct dlist_node, list);
	last = list_last_entry(&dlist->list, struct dlist_node, list);

	/*
	 * Border case: the buffer is almost full, but there is discontiguous
	 * region (a 'hole') in the beginning of the data. In this scenario we
	 * try to keep the last free space for the packet which exactly fits
	 * this 'hole', and drop other ones.
	 */
	if (d2d_wrap_gt(first->seq_id, rxb->reader_pos) &&
	    !dlist_has_space(dlist) &&
	    !d2d_wrap_eq(node->seq_id, rxb->reader_pos)) {
		dlist_node_free(dlist, node);
		return RX_BUF_PUT_OVERFLOW;
	}

	/* Easy case: the node should be the last - add new node to the end */
	if (d2d_wrap_ge(node->seq_id, last->seq_id + last->len)) {
		list_add_tail(&node->list, &dlist->list);
		rx_buf_update_last_sequential_node(rxb);
		return RX_BUF_PUT_OK;
	}

	/*
	 * Filling hole case: new packet fills the hole in the beginning.
	 * We should check this condition here as subsequent traverse
	 * relies on it.
	 */
	if (d2d_wrap_lt(node->seq_id, rxb->reader_pos) &&
	    d2d_wrap_gt(node->seq_id + node->len, rxb->reader_pos) &&
	    d2d_wrap_lt(node->seq_id + node->len, first->seq_id)) {
		dlist_node_free(dlist, node);
		return RX_BUF_PUT_BAD_PACKET;
	}

	/* hard case: fast path failed - need to traverse the list */
	return rx_buf_insert_node_traverse(rxb, node);
}

int rx_buf_put(struct rx_buffer *rxb, const struct d2d_data_packet *packet)
{
	struct dlist *dlist     = rxb->dlist;
	struct dlist_node *node = NULL;

	/* protect ourselves from buffer overflows */
	if (WARN(dlist->elem_size < packet->len,
		 "elem = %zu, pkt = %zu\n", dlist->elem_size, packet->len))
		return RX_BUF_PUT_BAD_PACKET;

	/* do not accept data that is already read by application */
	if (d2d_wrap_le(packet->seq_id + packet->len, rxb->reader_pos))
		return RX_BUF_PUT_CONSUMED;

	/* try to allocate a new node for the data */
	node = dlist_node_alloc(dlist);
	if (!node)
		return RX_BUF_PUT_OVERFLOW;

	/* copy all data from packet to the new node */
	memcpy(node->data, packet->data, packet->len);
	node->seq_id = packet->seq_id;
	node->len    = packet->len;

	return rx_buf_insert_node(rxb, node);
}

size_t rx_buf_get_fill_percentage(const struct rx_buffer *rxb)
{
	const struct dlist *dlist = rxb->dlist;

	/* Calculate current RX buffer fill percentage */
	return DIV_ROUND_UP(dlist->n_filled * 100, dlist->n_nodes);
}
