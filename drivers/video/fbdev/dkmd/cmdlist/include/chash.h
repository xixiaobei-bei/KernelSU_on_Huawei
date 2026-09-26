/**
 * @file chash.h
 * @brief Interface for cmdlist hash function
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __CHASH_H__
#define __CHASH_H__

#include <linux/slab.h>

#include "clist.h"

typedef bool (*chash_equal_func)(const struct cnode *a, const struct cnode *b);
typedef size_t (*get_bkt_idx_func)(const struct cnode *node, size_t bkt_size);

struct chash_table {
	size_t bkt_size;
	chash_equal_func equal;
	get_bkt_idx_func get_bkt_idx;
	struct clist_head *bkts;
};

static inline int chash_init(struct chash_table *ht, size_t bkt_size,
	chash_equal_func equal, get_bkt_idx_func key)
{
	size_t i;

	ht->bkt_size = bkt_size;
	ht->equal = equal;
	ht->get_bkt_idx = key;
	ht->bkts = (struct clist_head *)kzalloc(sizeof(struct clist_head) * bkt_size, GFP_KERNEL);
	if (!ht->bkts)
		return -1;

	for (i = 0; i < bkt_size; i++)
		init_clist_head(&ht->bkts[i]);

	return 0;
}

static inline void chash_add(struct chash_table *ht, struct cnode *node)
{
	size_t k = ht->get_bkt_idx(node, ht->bkt_size);
	struct clist_head *list = &ht->bkts[k];

	clist_add_tail(list, node);
}

static inline void chash_remove(struct cnode *node)
{
	clist_remove(node);
}

static inline struct cnode *chash_find(const struct chash_table *ht,
	const struct cnode *cmp_node)
{
	struct cnode *node;
	size_t k = ht->get_bkt_idx(cmp_node, ht->bkt_size);
	struct clist_head *list = &ht->bkts[k];

	clist_for_each(node, list) {
		if ((node != NULL) && ht->equal(cmp_node, node))
			return node;
	}
	return NULL;
}

#endif