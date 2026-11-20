// SPDX-License-Identifier: GPL-2.0
/*
 * D2D TX buffer implementation
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/net.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include "buffer.h"
#include "dynamic_list.h"
#include "tx_buffer.h"
#include "wrappers.h"

struct tx_buffer *tx_buf_alloc(size_t bufsize, size_t elem_size)
{
	struct tx_buffer *txb = NULL;

	txb = vzalloc(sizeof(*txb));
	if (!txb)
		return NULL;

	txb->dlist = dlist_alloc(bufsize, elem_size);
	if (!txb->dlist)
		goto out_txb;

	return txb;

out_txb:
	vfree(txb);
	return NULL;
}

void tx_buf_free(struct tx_buffer *txb)
{
	if (!txb)
		return;

	dlist_free(txb->dlist);
	vfree(txb);
}

static bool tx_buf_node_writeable(const struct dlist_node *node)
{
	return node->state <= D2D_NODE_STATE_UNSENT && node->len < node->maxlen;
}

static bool tx_buf_node_peeked(const struct dlist_node *node)
{
	return node->state == D2D_NODE_STATE_SENDING ||
		node->state == D2D_NODE_STATE_ACKNOWLEDGED;
}

bool tx_buf_has_free_space(const struct tx_buffer *txb)
{
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;

	/* check if the last node contains some free space */
	if (!list_empty(&dlist->list)) {
		node = list_last_entry(&dlist->list, struct dlist_node, list);
		if (node && tx_buf_node_writeable(node))
			return true;
	}

	/* the last node is full so let's check the free space in the list */
	return dlist_has_space(dlist);
}

static size_t tx_buf_node_get_free_bytes(const struct dlist_node *node)
{
	return node->maxlen - node->len;
}

size_t tx_buf_get_free_bytes(const struct tx_buffer *txb)
{
	size_t bytes = 0;
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;

	/* check if the last node contains some free space */
	if (!list_empty(&dlist->list)) {
		node = list_last_entry(&dlist->list, struct dlist_node, list);
		if (node && tx_buf_node_writeable(node))
			bytes += tx_buf_node_get_free_bytes(node);
	}

	/* calculate amount of free bytes in the rest nodes */
	bytes += (dlist->n_nodes - dlist->n_filled) * dlist->elem_size;

	return bytes;
}

static size_t tx_buf_node_try_append(struct dlist_node *node,
				     const void *data, size_t len)
{
	if (tx_buf_node_writeable(node)) {
		size_t free = node->maxlen - node->len;
		size_t to_copy = min(len, free);

		memcpy(node->data + node->len, data, to_copy);
		node->len += to_copy;
		return to_copy;
	}

	return 0;
}

static size_t __tx_buf_fill_nodes(struct tx_buffer *txb, const struct kvec *iov,
				  size_t iovlen, size_t to_fill,
				  struct dlist_node *last,
				  struct dlist_node *prealloc)
{
	size_t i     = 0;
	size_t total = 0;
	struct dlist_node *node = NULL;

	for (i = 0; i < iovlen; i++) {
		size_t bytes = 0;
		size_t len = min(iov[i].iov_len, to_fill);
		void *data = iov[i].iov_base;

		to_fill -= len;

		/* first try to append to last node in buffer */
		if (last && tx_buf_node_writeable(last)) {
			bytes = tx_buf_node_try_append(last, data, len);
			total += bytes;

			len -= bytes;
			data += bytes;
		}

		/* if there are still data to add, fill pre-allocated nodes */
		while (len) {
			if (!node) {
				node = prealloc;
				node->state = D2D_NODE_STATE_UNSENT;
				node->seq_id = txb->tx_seq_id + total;
			}

			if (!tx_buf_node_writeable(node)) {
				node = list_next_entry(node, list);
				node->state = D2D_NODE_STATE_UNSENT;
				node->seq_id = txb->tx_seq_id + total;
			}

			if (!txb->first_to_send)
				txb->first_to_send = node;

			bytes = tx_buf_node_try_append(node, data, len);
			total += bytes;

			len -= bytes;
			data += bytes;
		}
	}

	txb->tx_seq_id += total;

	return total;
}

static bool __tx_buf_prealloc_nodes(size_t to_alloc,
				    struct tx_buffer *txb,
				    struct dlist_node **first_prealloc,
				    struct list_head *preallocated_list)
{
	size_t i            = 0;
	size_t nodes        = 0;
	size_t n_free_nodes = 0;
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;

	/* calculate amount of nodes to allocate */
	nodes = DIV_ROUND_UP(to_alloc, dlist->elem_size);

	n_free_nodes = dlist_get_free_nodes(dlist);
	if (n_free_nodes < nodes)
		return false;

	/* allocate nodes and store them in temporary list */
	for (i = 0; i < nodes; i++) {
		node = dlist_node_alloc(dlist);
		if (!node)
			return false;

		list_add_tail(&node->list, preallocated_list);
	}

	*first_prealloc = list_first_entry(preallocated_list,
					   struct dlist_node, list);

	/* add allocated nodes to the end of tx buffer */
	list_splice_tail(preallocated_list, &dlist->list);

	return true;
}

size_t tx_buf_append_iov(struct tx_buffer *txb, const struct kvec *iov,
			 size_t iovlen, size_t to_append)
{
	LIST_HEAD(preallocated_list);
	bool ret            = false;
	size_t to_alloc     = 0;
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;
	struct dlist_node *next = NULL;
	struct dlist_node *last = NULL;

	if (!to_append)
		return 0;

	to_alloc = to_append;

	/* account available space in the last node if writeable */
	if (!list_empty(&dlist->list)) {
		node = list_last_entry(&dlist->list, struct dlist_node, list);
		if (tx_buf_node_writeable(node)) {
			size_t free_bytes = tx_buf_node_get_free_bytes(node);

			last = node;
			if (free_bytes < to_alloc)
				to_alloc -= free_bytes;
			else
				to_alloc = 0;
		}
	}

	if (to_alloc) {
		ret = __tx_buf_prealloc_nodes(to_alloc, txb, &node,
					      &preallocated_list);
		if (!ret)
			goto fail_free_preallocated;

		return __tx_buf_fill_nodes(txb, iov, iovlen,
					   to_append, last, node);
	} else {
		return __tx_buf_fill_nodes(txb, iov, iovlen,
					   to_append, last, NULL);
	}

fail_free_preallocated:
	list_for_each_entry_safe(node, next, &preallocated_list, list) {
		list_del(&node->list);
		dlist_node_free(dlist, node);
	}
	return 0;
}

bool tx_buf_get_last_seq_id(const struct tx_buffer *txb, wrap_t *res)
{
	struct dlist *dlist     = NULL;
	struct dlist_node *node = NULL;

	dlist = txb->dlist;
	if (list_empty(&dlist->list))
		return false;

	node = list_last_entry(&dlist->list, struct dlist_node, list);
	*res = node->seq_id + node->len;
	return true;
}

bool tx_buf_has_packets_to_send(const struct tx_buffer *txb)
{
	struct dlist_node *first_to_send = txb->first_to_send;

	if (!txb->flowcontrol_mode)
		return first_to_send;
	/*
	 * If flowcontrol mode was enabled suspend sending new data,
	 * only resend the old one.
	 */
	return first_to_send &&
		(first_to_send->state == D2D_NODE_STATE_NEED_RESEND);
}

static inline void tx_buf_reset_first_to_send(struct tx_buffer *txb)
{
	txb->first_to_send = list_first_entry_or_null(&txb->dlist->list,
						      struct dlist_node,
						      list);
}

static void tx_buf_switch_next_to_send(struct tx_buffer *txb)
{
	struct dlist_node *node = txb->first_to_send;

	if (!node)
		return;

	list_for_each_entry_continue(node, &txb->dlist->list, list) {
		if (node->state == D2D_NODE_STATE_UNSENT ||
		    node->state == D2D_NODE_STATE_NEED_RESEND) {
			txb->first_to_send = node;
			return;
		}
	}
	txb->first_to_send = NULL;
}

void tx_buf_peek_packet(struct tx_buffer *txb, struct d2d_data_packet *packet)
{
	struct dlist_node *node = NULL;

	node = txb->first_to_send;
	if (!node)
		return;

	packet->len    = node->len;
	packet->seq_id = node->seq_id;
	packet->data   = node->data;
	packet->resend = node->state == D2D_NODE_STATE_NEED_RESEND;

	/* This packet is not first anymore */
	tx_buf_switch_next_to_send(txb);

	/*
	 * This is very important - we protect this node from being changed
	 * by setting SENDIND state. The node then cannot be damaged while
	 * caller sends this packet without holding the lock. The node is
	 * returned back via `peek_return` paired function.
	 */
	node->state = D2D_NODE_STATE_SENDING;
}

int tx_buf_peek_return(struct tx_buffer *txb, struct d2d_data_packet *packet)
{
	struct dlist_node *node = container_of(packet->data,
					       struct dlist_node,
					       data);

	if (node->state == D2D_NODE_STATE_SENDING) {
		/*
		 * The node has not been disturbed while sending. Just mark as
		 * sent and process it in ordinary way.
		 */
		node->state = D2D_NODE_STATE_SENT;
	} else if (node->state == D2D_NODE_STATE_ACKNOWLEDGED) {
		/*
		 * The node has been acknowledged while being sent. No need to
		 * re-insert it, we can safely delete this node but not forget
		 * to reset `first_to_send` to not point to poisoned node.
		 */
		bool need_reset = node == txb->first_to_send;

		list_del(&node->list);
		dlist_node_free(txb->dlist, node);
		if (need_reset)
			tx_buf_reset_first_to_send(txb);
	} else {
		/*
		 * The node state has been incorrectly changed while sending.
		 * This should never happen, return false to trigger protocol
		 * closing.
		 */
		WARN(true, "bad TX node state: %d\n", node->state);
		return -EINVAL;
	}

	return 0;
}

void tx_buf_mark_as_need_resent(struct tx_buffer *txb)
{
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;

	list_for_each_entry(node, &dlist->list, list) {
		/* Invariant: the list tail consists of UNSENT nodes */
		if (node->state == D2D_NODE_STATE_UNSENT)
			break;

		/*
		 * Please do not touch the sending packets, let them being
		 * re-sent at some time in the future. If we mark them as
		 * `NEED_RESEND` we will get data race immediately because
		 * sending nodes can be deleted by acknowledgement packet.
		 */
		if (tx_buf_node_peeked(node))
			continue;

		node->state = D2D_NODE_STATE_NEED_RESEND;
	}

	tx_buf_reset_first_to_send(txb);
}

static size_t __tx_buf_free_ack(struct tx_buffer *txb, wrap_t ack_id)
{
	size_t freed = 0;
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;
	struct dlist_node *next = NULL;
	bool need_reset = false;

	list_for_each_entry_safe(node, next, &dlist->list, list) {
		/*
		 * Internal list buffer keeps packets in the sending order so
		 * when condition is true for the first time, it is true for the
		 * rest of the list.
		 */
		if (d2d_wrap_ge(node->seq_id, ack_id))
			break;

		/*
		 * Do not damage packets being sent right now and let `return`
		 * function delete them when needed.
		 */
		if (tx_buf_node_peeked(node)) {
			freed += (node->state == D2D_NODE_STATE_SENDING)
					? node->len : 0;
			node->state = D2D_NODE_STATE_ACKNOWLEDGED;
			continue;
		}

		/*
		 * Check that we are deleting node `first_to_send` points to.
		 */
		if (node == txb->first_to_send) {
			txb->first_to_send = NULL;
			need_reset = true;
		}

		freed += node->len;
		list_del(&node->list);
		dlist_node_free(dlist, node);
	}

	/*
	 * We need to flush `first_to_send` pointer because the node
	 * it points to has been deleted and contains poisoned links.
	 */
	if (need_reset)
		tx_buf_reset_first_to_send(txb);

	return freed;
}

static size_t __tx_buf_free_sack(struct tx_buffer *txb,
				 const struct sack_pairs *sack)
{
	struct dlist *dlist = txb->dlist;
	struct dlist_node *node = NULL;
	struct dlist_node *next = NULL;
	size_t pair_num = 0;
	size_t freed    = 0;

	/*
	 * We use the fact that SACKs and buffer nodes are ordered
	 */
	list_for_each_entry_safe(node, next, &dlist->list, list) {
		wrap_t l_sack = 0;
		wrap_t r_sack = 0;

		if (pair_num >= sack->len)
			break;

		l_sack = sack->pairs[pair_num].l;
		r_sack = sack->pairs[pair_num].r;

		/*
		 * The node does not fit to the SACK. Just go to the next node
		 * without switching to the next SACK pair.
		 */
		if (d2d_wrap_lt(node->seq_id, l_sack)) {
			/* Need re-send only if not sending */
			if (!tx_buf_node_peeked(node))
				node->state = D2D_NODE_STATE_NEED_RESEND;

			continue;
		}

		/*
		 * The node is located in [l_sack, r_sack] range, it is
		 * acknowledged so we safely delete it from the buffer.
		 */
		if (d2d_wrap_le(node->seq_id, r_sack)) {
			/*
			 * This packet is the last in this SACK pair, switch to
			 * the next one on the next iteration.
			 */
			if (d2d_wrap_gt(node->seq_id + node->len, r_sack))
				pair_num++;

			/* Do not disturb sending packets */
			if (tx_buf_node_peeked(node)) {
				freed += (node->state == D2D_NODE_STATE_SENDING)
					? node->len : 0;
				node->state = D2D_NODE_STATE_ACKNOWLEDGED;
				continue;
			}

			freed += node->len;
			list_del(&node->list);
			dlist_node_free(dlist, node);
		}
	}

	/*
	 * Rewind the `first_unsent` pointer to the packets not
	 * acknowledged by this SACK.
	 */
	tx_buf_reset_first_to_send(txb);

	return freed;
}

size_t tx_buf_process_ack(struct tx_buffer *txb,
			  wrap_t ack_id,
			  const struct sack_pairs *sack)
{
	size_t freed = 0;

	freed += __tx_buf_free_ack(txb, ack_id);
	if (sack && sack->len)
		freed += __tx_buf_free_sack(txb, sack);

	return freed;
}

bool tx_buf_is_empty(struct tx_buffer *txb)
{
	return list_empty(&txb->dlist->list);
}

void tx_buf_set_flowcontrol_mode(struct tx_buffer *txb, bool mode)
{
	txb->flowcontrol_mode = mode;
}

size_t tx_buf_get_size(struct tx_buffer *txb)
{
	return dlist_get_size(txb->dlist);
}
