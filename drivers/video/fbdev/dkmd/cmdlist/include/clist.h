/**
 * @file clist.h
 * @brief Interface for cmdlist list function
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

#ifndef __CLIST_H__
#define __CLIST_H__

#include <stdbool.h>
#include <stddef.h>

struct cnode {
	struct cnode *next, *prev;
};

/**
 * cnode_entry - get the struct for this entry
 * @node:	the &struct cnode pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the cnode within the struct.
 */
#define cnode_entry(node, type, member) \
	((type*)((char*)(node) - offsetof(type, member)))

struct clist_head {
	struct cnode base;
};

static inline void init_clist_head(struct clist_head *list)
{
	list->base.next = &list->base;
	list->base.prev = &list->base;
}

static inline bool clist_is_empty(const struct clist_head *list)
{
	return list->base.next == &list->base;
}

static inline bool clist_is_head(const struct clist_head *list, const struct cnode *node)
{
	return list->base.next == node;
}

static inline bool clist_is_tail(const struct clist_head *list, const struct cnode *node)
{
	return list->base.prev == node;
}

static inline void clist_insert(struct cnode *pos, struct cnode *node)
{
	node->prev = pos->prev;
	node->next = pos;
	node->prev->next = node;
	node->next->prev = node;
}

static inline void clist_add_tail(struct clist_head *list, struct cnode *node)
{
	clist_insert(&list->base, node);
}

static inline void clist_add_head(struct clist_head *list, struct cnode *node)
{
	clist_insert(list->base.next, node);
}

static inline void clist_remove(struct cnode *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline void clist_remove_tail(struct clist_head *list)
{
	clist_remove(list->base.prev);
}

static inline void clist_remove_head(struct clist_head *list)
{
	clist_remove(list->base.next);
}

static inline void clist_replace(struct cnode *old, struct cnode *node)
{
	node->next = old->next;
	node->next->prev = node;
	node->prev = old->prev;
	node->prev->next = node;
}

#define clist_for_each(node, list) \
	for (node = (list)->base.next; \
		 node != &((list)->base); \
		 node = (node)->next)

#define clist_for_each_saft(node, tmp, list) \
	for (node = (list)->base.next, tmp = (node)->next; \
		 node != &(list)->base; \
		 node = tmp, tmp = (node)->next)

/**
 * clist_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the clist_head within the struct.
 */
#define clist_first_entry(ptr, type, member) \
	(clist_is_empty(ptr) ? NULL : cnode_entry((ptr)->base.next, type, member))

/**
 * clist_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the clist_head within the struct.
 */
#define clist_last_entry(ptr, type, member) \
	(clist_is_empty(ptr) ? NULL : cnode_entry((ptr)->base.prev, type, member))

/**
 * clist_next_entry - get the next element of @entry from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the clist_head within the struct.
 */
#define clist_next_entry(entry, ptr, type, member) \
	(clist_is_tail(ptr, &(entry)->member) ? \
		NULL : \
		cnode_entry((entry)->member.next, type, member))

/**
 * clist_prev_entry - get the prev element of @entry from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the clist_head within the struct.
 */
#define clist_prev_entry(entry, ptr, type, member) \
	(clist_is_head(ptr, &(entry)->member) ? \
		NULL : \
		cnode_entry((entry)->member.prev, type, member))

/**
 * clist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @tmp:	another type * to use as temporary storage
 * @list:	the list head to take the element from.
 * @type:	the type for your list.
 * @member:	the name of the clist_head within the struct.
 */
#define clist_for_each_entry_safe(pos, tmp, list, type, member)	\
	for (pos = cnode_entry((list)->base.next, type, member), \
		 tmp = cnode_entry((pos)->member.next, type, member); \
		 &(pos)->member != &(list)->base; \
		 pos = tmp, tmp = cnode_entry((pos)->member.next, type, member))

/**
 * clist_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @tmp:	another type * to use as temporary storage
 * @list:	the list head to take the element from.
 * @type:	the type for your list.
 * @member:	the name of the clist_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define clist_for_each_entry_safe_reverse(pos, tmp, list, type, member)	\
	for (pos = cnode_entry((list)->base.prev, type, member), \
		tmp = cnode_entry((pos)->member.prev, type, member); \
		&(pos)->member != &(list)->base; \
		pos = tmp, tmp = cnode_entry((pos)->member.prev, type, member))

#endif