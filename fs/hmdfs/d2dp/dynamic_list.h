/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D dynamic list declarations
 */

#ifndef D2D_DYNAMIC_LIST_H
#define D2D_DYNAMIC_LIST_H

#include <linux/types.h>
#include <linux/slab.h>

#include "params.h"

/**
 * struct dlist_node - the single data unit for D2D dynamic list.
 *
 * This structure is used to keep single packet data for D2D TX and RX buffers.
 * The elements are organized in a list and contain the actual application data.
 *
 * @list: the list head used to organize packet stream
 * @len: the actual data length contained in this node
 * @maxlen: the maximum amount of data that can be contained in the node
 * @seq_id: D2D protocol's sequence ID of the corresponding data packet
 * @state: a label which can be used by TX/RX buffers to synchronize access
 * @data: the actual array of data in this node
 */
struct dlist_node {
	struct list_head list;
	unsigned int len;
	unsigned int maxlen;
	u32 seq_id;
	u32 state;
	char data[0];
};

/**
 * struct dlist - Dynamic LIST structure.
 *
 * Dynamic list is a simple data structure used to allocate nodes for a list
 * from kmem_cache object. Dynamic allocation allows to decrease memory
 * consuption since it is allocated only if necessary.
 *
 * @list: beginning of list structure
 * @cache: cache object used for nodes allocation
 * @n_nodes: amount of nodes total
 * @n_filled: amount of already allocated nodes
 * @elem_size: node's data size
 */
struct dlist {
	struct list_head  list;
	struct kmem_cache *cache;
	size_t            n_nodes;
	size_t            n_filled;
	size_t            elem_size;
};

struct dlist *dlist_alloc(size_t bufsize, size_t elem_size);
void dlist_free(struct dlist *dlist);

bool dlist_has_space(const struct dlist *dlist);
bool dlist_is_empty(const struct dlist *dlist);

struct dlist_node *dlist_node_alloc(struct dlist *dlist);
void dlist_node_free(struct dlist *dlist, struct dlist_node *node);
size_t dlist_get_size(struct dlist *dlist);
size_t dlist_get_free_nodes(const struct dlist *dlist);

#endif /* D2D_DYNAMIC_LIST_H */
