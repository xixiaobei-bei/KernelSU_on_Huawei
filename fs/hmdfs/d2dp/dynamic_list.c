// SPDX-License-Identifier: GPL-2.0
/*
 * D2D dynamic list implementation
 */

#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "dynamic_list.h"

/*
 * We need atomic allocations to create nodes under spinlocks but as D2DP
 * buffers are quite big, we don't want to increase memory pressure so forbid
 * the usage of kernel emergency pools.
 */
#define DLIST_GFP_FLAGS (GFP_ATOMIC | __GFP_NOMEMALLOC | __GFP_NOWARN)

struct dlist *dlist_alloc(size_t bufsize, size_t elem_size)
{
	struct dlist *dlist = NULL;
	unsigned int kmem_obj_size = sizeof(struct dlist_node) + elem_size;

	if (!bufsize || !elem_size)
		return NULL;

	dlist = vzalloc(sizeof(*dlist));
	if (!dlist)
		return NULL;

	dlist->n_filled = 0;
	dlist->elem_size = elem_size;
	dlist->n_nodes = DIV_ROUND_UP(bufsize, elem_size);
	dlist->cache = kmem_cache_create("d2dp_kmem_cache", kmem_obj_size,
					 0, 0, NULL);
	if (!dlist->cache)
		goto out_alloc;

	INIT_LIST_HEAD(&dlist->list);

	return dlist;

out_alloc:
	vfree(dlist);
	return NULL;
}

struct dlist_node *dlist_node_alloc(struct dlist *dlist)
{
	struct dlist_node *node = NULL;
	size_t elem_size = dlist->elem_size;

	/* check the buffer is full */
	if (dlist->n_filled == dlist->n_nodes)
		return NULL;

	node = kmem_cache_alloc(dlist->cache, DLIST_GFP_FLAGS);
	if (!node)
		return NULL;

	node->len = 0;
	node->maxlen = elem_size;
	node->seq_id = 0;
	node->state = 0;
	INIT_LIST_HEAD(&node->list);

	dlist->n_filled++;

	return node;
}

void dlist_node_free(struct dlist *dlist, struct dlist_node *node)
{
	node->state = -1;  /* enforce error detection */
	kmem_cache_free(dlist->cache, node);
	dlist->n_filled--;
}

bool dlist_has_space(const struct dlist *dlist)
{
	return dlist->n_filled < dlist->n_nodes;
}

size_t dlist_get_free_nodes(const struct dlist *dlist)
{
	return dlist->n_nodes - dlist->n_filled;
}

bool dlist_is_empty(const struct dlist *dlist)
{
	return !dlist->n_filled;
}

void dlist_free(struct dlist *dlist)
{
	if (!dlist)
		return;

	if (!list_empty(&dlist->list)) {
		struct dlist_node *node = NULL;
		struct dlist_node *next = NULL;

		list_for_each_entry_safe(node, next, &dlist->list, list) {
			list_del(&node->list);
			dlist_node_free(dlist, node);
		}
	}

	kmem_cache_destroy(dlist->cache);
	vfree(dlist);
}

size_t dlist_get_size(struct dlist *dlist)
{
	return dlist->n_nodes * dlist->elem_size;
}
