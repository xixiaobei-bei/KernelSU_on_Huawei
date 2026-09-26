/*
 * hw_xcollie_list.h
 *
 * This file is the header file for hwxcollie
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HW_XCOLLIE_LIST_H
#define HW_XCOLLIE_LIST_H

#include <linux/stddef.h>

struct hwlistnode {
	struct hwlistnode *next;
	struct hwlistnode *prev;
};

#define hw_node_to_item(node, container, member) \
	((container *) (((char *) (node)) - offsetof(container, member)))

#define hw_list_for_each(node, list) \
	for ((node) = (list)->next; (node) != (list); (node) = (node)->next)

#define hw_list_for_each_reverse(node, list) \
	for ((node) = (list)->prev; (node) != (list); (node) = (node)->prev)

#define hw_list_for_each_safe(node, n, list) \
	for ((node) = (list)->next, (n) = (node)->next; \
	(node) != (list); \
	(node) = (n), (n) = (node)->next)

#define hw_list_empty(list) ((list) == (list)->next)
#define hw_list_head(list) ((list)->next)

#endif /* HW_XCOLLIE_LIST_H */
