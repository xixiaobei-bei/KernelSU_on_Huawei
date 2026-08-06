/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold implement
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author:	He Biao <hebiao6@huawei.com>
 *		Wang Cheng Ke <wangchengke2@huawei.com>
 *		Wang Fa <fa.wang@huawei.com>
 *
 * Create: 2020-4-16
 *
 */

#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/bit_spinlock.h>
#include <linux/zsmalloc.h>

#include "hyperhold_list.h"
#include "hyperhold_internal.h"

static struct hh_list_head *get_node_default(int idx, void *private)
{
	struct hh_list_head *table = private;

	return &table[idx];
}

struct hh_list_table *alloc_table(struct hh_list_head *(*get_node)(int, void *),
				  void *private, gfp_t gfp)
{
	struct hh_list_table *table =
				kmalloc(sizeof(struct hh_list_table), gfp);

	if (!table)
		return NULL;
	table->get_node = get_node ? get_node : get_node_default;
	table->private = private;

	return table;
}

void hh_lock_list(int idx, struct hh_list_table *table)
{
	struct hh_list_head *node = idx_node(idx, table);

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK\n", idx, table);
		return;
	}
	bit_spin_lock(HH_LIST_LOCK_BIT, (unsigned long *)node);
}

void hh_unlock_list(int idx, struct hh_list_table *table)
{
	struct hh_list_head *node = idx_node(idx, table);

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK\n", idx, table);
		return;
	}
	bit_spin_unlock(HH_LIST_LOCK_BIT, (unsigned long *)node);
}

bool hh_list_empty(int hidx, struct hh_list_table *table)
{
	bool ret = false;

	hh_lock_list(hidx, table);
	ret = (prev_idx(hidx, table) == hidx) && (next_idx(hidx, table) == hidx);
	hh_unlock_list(hidx, table);

	return ret;
}

void hh_list_init(int idx, struct hh_list_table *table)
{
	struct hh_list_head *node = idx_node(idx, table);

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK\n", idx, table);
		return;
	}
	memset(node, 0, sizeof(struct hh_list_head));
	node->prev = idx;
	node->next = idx;
}


void hh_list_add_nolock(int idx, int hidx, struct hh_list_table *table)
{
	struct hh_list_head *node = NULL;
	struct hh_list_head *head = NULL;
	struct hh_list_head *next = NULL;
	int nidx;

	node = idx_node(idx, table);
	if (!node) {
		hh_print(HHLOG_ERR,
			 "NULL node, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}
	head = idx_node(hidx, table);
	if (!head) {
		hh_print(HHLOG_ERR,
			 "NULL head, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}
	next = idx_node(head->next, table);
	if (!next) {
		hh_print(HHLOG_ERR,
			 "NULL next, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}

	nidx = head->next;
	if (idx != hidx)
		hh_lock_list(idx, table);
	node->prev = hidx;
	node->next = nidx;
	if (idx != hidx)
		hh_unlock_list(idx, table);
	head->next = idx;
	if (nidx != hidx)
		hh_lock_list(nidx, table);
	next->prev = idx;
	if (nidx != hidx)
		hh_unlock_list(nidx, table);
}

void hh_list_add_tail_nolock(int idx, int hidx, struct hh_list_table *table)
{
	struct hh_list_head *node = NULL;
	struct hh_list_head *head = NULL;
	struct hh_list_head *tail = NULL;
	int tidx;

	node = idx_node(idx, table);
	if (!node) {
		hh_print(HHLOG_ERR,
			 "NULL node, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}
	head = idx_node(hidx, table);
	if (!head) {
		hh_print(HHLOG_ERR,
			 "NULL head, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}
	tail = idx_node(head->prev, table);
	if (!tail) {
		hh_print(HHLOG_ERR,
			 "NULL tail, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}

	tidx = head->prev;
	if (idx != hidx)
		hh_lock_list(idx, table);
	node->prev = tidx;
	node->next = hidx;
	if (idx != hidx)
		hh_unlock_list(idx, table);
	head->prev = idx;
	if (tidx != hidx)
		hh_lock_list(tidx, table);
	tail->next = idx;
	if (tidx != hidx)
		hh_unlock_list(tidx, table);
}

void hh_list_del_nolock(int idx, int hidx, struct hh_list_table *table)
{
	struct hh_list_head *node = NULL;
	struct hh_list_head *prev = NULL;
	struct hh_list_head *next = NULL;
	int pidx, nidx;

	node = idx_node(idx, table);
	if (!node) {
		hh_print(HHLOG_ERR,
			 "NULL node, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}
	prev = idx_node(node->prev, table);
	if (!prev) {
		hh_print(HHLOG_ERR,
			 "NULL prev, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}
	next = idx_node(node->next, table);
	if (!next) {
		hh_print(HHLOG_ERR,
			 "NULL next, idx = %d, hidx = %d, table = %pK\n",
			 idx, hidx, table);
		return;
	}

	if (idx != hidx)
		hh_lock_list(idx, table);
	pidx = node->prev;
	nidx = node->next;
	node->prev = idx;
	node->next = idx;
	if (idx != hidx)
		hh_unlock_list(idx, table);
	if (pidx != hidx)
		hh_lock_list(pidx, table);
	prev->next = nidx;
	if (pidx != hidx)
		hh_unlock_list(pidx, table);
	if (nidx != hidx)
		hh_lock_list(nidx, table);
	next->prev = pidx;
	if (nidx != hidx)
		hh_unlock_list(nidx, table);
}

void hh_list_add(int idx, int hidx, struct hh_list_table *table)
{
	hh_lock_list(hidx, table);
	hh_list_add_nolock(idx, hidx, table);
	hh_unlock_list(hidx, table);
}

void hh_list_add_tail(int idx, int hidx, struct hh_list_table *table)
{
	hh_lock_list(hidx, table);
	hh_list_add_tail_nolock(idx, hidx, table);
	hh_unlock_list(hidx, table);
}

void hh_list_del(int idx, int hidx, struct hh_list_table *table)
{
	hh_lock_list(hidx, table);
	hh_list_del_nolock(idx, hidx, table);
	hh_unlock_list(hidx, table);
}

unsigned short hh_list_get_mcgid(int idx, struct hh_list_table *table)
{
	struct hh_list_head *node = idx_node(idx, table);
	int mcg_id;

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK\n", idx, table);
		return 0;
	}

	hh_lock_list(idx, table);
	mcg_id = (node->mcg_hi << HH_LIST_MCG_SHIFT_HALF) | node->mcg_lo;
	hh_unlock_list(idx, table);

	return mcg_id;
}

void hh_list_set_mcgid(int idx, struct hh_list_table *table, int mcg_id)
{
	struct hh_list_head *node = idx_node(idx, table);

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK, mcg = %d\n",
			 idx, table, mcg_id);
		return;
	}

	hh_lock_list(idx, table);
	node->mcg_hi = (u32)mcg_id >> HH_LIST_MCG_SHIFT_HALF;
	node->mcg_lo = (u32)mcg_id & ((1 << HH_LIST_MCG_SHIFT_HALF) - 1);
	hh_unlock_list(idx, table);
}

bool hh_list_set_priv(int idx, struct hh_list_table *table)
{
	struct hh_list_head *node = idx_node(idx, table);
	bool ret = false;

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK\n", idx, table);
		return false;
	}
	hh_lock_list(idx, table);
	ret = !test_and_set_bit(HH_LIST_PRIV_BIT, (unsigned long *)node);
	hh_unlock_list(idx, table);

	return ret;
}

bool hh_list_clear_priv(int idx, struct hh_list_table *table)
{
	struct hh_list_head *node = idx_node(idx, table);
	bool ret = false;

	if (!node) {
		hh_print(HHLOG_ERR, "idx = %d, table = %pK\n", idx, table);
		return false;
	}

	hh_lock_list(idx, table);
	ret = test_and_clear_bit(HH_LIST_PRIV_BIT, (unsigned long *)node);
	hh_unlock_list(idx, table);

	return ret;
}
