/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _OSL_LIST_H
#define _OSL_LIST_H

#ifdef __KERNEL__
#include <linux/list.h>

#elif defined(__OS_VXWORKS__)||defined(__CMSIS_RTOS)||defined(__OS_RTOSCK__)||defined(__OS_RTOSCK_SMP__)||defined(__FASTBOOT__)||defined(__NR_LL1C_)

#include "osl_common.h"
#ifdef __FASTBOOT__
#include <types.h>
#endif

/********** include/linux/list.h **********/

#define OSL_INLINE __inline__
#ifndef typeof
#define typeof __typeof__
#endif

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next;
	struct hlist_node **pprev;
};


#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static OSL_INLINE void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static OSL_INLINE void list_add_between(struct list_head *new_list, struct list_head *prev, struct list_head *next)
{
	next->prev = new_list;
	new_list->next = next;
	new_list->prev = prev;
	prev->next = new_list;
}

static OSL_INLINE void list_add(struct list_head *new_list, struct list_head *head)
{
	list_add_between(new_list, head, head->next);/* [false alarm]:Îó±¨ */
}

static OSL_INLINE void list_delete_between(struct list_head * prev_list, struct list_head * next_list)
{
	next_list->prev = prev_list;
	prev_list->next = next_list;
}


static OSL_INLINE void list_add_tail(struct list_head *new_list, struct list_head *head)
{
	list_add_between(new_list, head->prev, head);/* [false alarm]:Îó±¨ */
}

static OSL_INLINE void _inline_list_del_cur_entry(struct list_head *entry)
{
	list_delete_between(entry->prev, entry->next);
}


#define LIST_POISON_NEXT  ((void *) 0x00100100 + 0)
#define LIST_POISON_PREV  ((void *) 0x00200200 + 0)

static OSL_INLINE void list_del(struct list_head *entry)
{
	list_delete_between(entry->prev, entry->next);
	entry->next = LIST_POISON_NEXT;
	entry->prev = LIST_POISON_PREV;
}


static OSL_INLINE void list_replace(struct list_head *old,
				struct list_head *new_list)
{
	new_list->next = old->next;
	new_list->next->prev = new_list;
	new_list->prev = old->prev;
	new_list->prev->next = new_list;
}

static OSL_INLINE void list_replace_init(struct list_head *old,
					struct list_head *new_list)
{
	list_replace(old, new_list);
	INIT_LIST_HEAD(old);
}

static OSL_INLINE void list_del_init(struct list_head *entry)
{
	_inline_list_del_cur_entry(entry);
	INIT_LIST_HEAD(entry);
}

static OSL_INLINE void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	_inline_list_del_cur_entry(list);
	list_add_tail(list, head);
}

static OSL_INLINE int list_empty(const struct list_head *the_head)
{
	return the_head->next == the_head;
}

static OSL_INLINE int list_is_last(const struct list_head *cur_list, const struct list_head *the_head)
{
	return cur_list->next == the_head;
}


static OSL_INLINE int list_empty_careful(const struct list_head *head)
{
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}


static OSL_INLINE void _inline_list_splice(const struct list_head *list, struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static OSL_INLINE void list_splice_init(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list)) {
		_inline_list_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

#define list_for_each(position, head) \
	for (position = (head)->next; position != (head); position = position->next)

#define list_for_each_prev(position, head) \
	for (position = (head)->prev; position != (head); position = position->prev)

#define list_for_each_prev_safe(position, n, head) \
            for (position = (head)->prev, n = position->prev; \
                 position != (head); \
                 position = n, n = position->prev)

#define list_for_each_safe(position, n, head) \
	for (position = (head)->next, n = position->next; position != (head); \
		position = n, n = position->next)

#define list_entry(ptr, type, member) 	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) 	list_entry((ptr)->next, type, member)

/*lint -esym(718,__typeof__)*/
/*lint +rw( __typeof__ ) */
#define list_for_each_entry(position, head, member)				\
	for (/*lint -epn -epp*/position = list_entry((head)->next, typeof(*position)/*lint +rw( typeof )*/, member);	\
	     &position->member != (head); 	\
	     position = list_entry(position->member.next, typeof(*position), member))

#define list_for_each_entry_safe(position, n, head, member)			\
        for (position = list_entry(/*lint -epn -epp*/(head)->next, typeof(*position), member),  \
            n = list_entry(position->member.next, typeof(*position), member);   \
             &position->member != (head);                   \
             position = n, n = list_entry(n->member.next, typeof(*n), member))
    
#define list_prepare_entry(position, head, member) \
            ((position) ? : list_entry(head, typeof(*position), member))
        
#define list_for_each_entry_reverse(position, head, member)			\
	for (position = list_entry((head)->prev, typeof(*position), member);	\
	     &position->member != (head); 	                            \
	     position = list_entry(position->member.prev, typeof(*position), member))

#define list_for_each_entry_continue(position, head, member) 		\
	for (position = list_entry(position->member.next, typeof(*position), member);	\
	     &position->member != (head);	\
	     position = list_entry(position->member.next, typeof(*position), member))

#define list_for_each_entry_continue_reverse(position, head, member)		\
	for (position = list_entry(position->member.prev, typeof(*position), member);	\
	     &position->member != (head);	\
	     position = list_entry(position->member.prev, typeof(*position), member))

#define list_for_each_entry_from(position, head, member) 			\
	for (; &position->member != (head);	\
	     position = list_entry(position->member.next, typeof(*position), member))

#define list_for_each_entry_safe_continue(position, n, head, member) 		\
	for (position = list_entry(position->member.next, typeof(*position), member), 		\
		n = list_entry(position->member.next, typeof(*position), member);		\
	     &position->member != (head);						\
	     position = n, n = list_entry(n->member.next, typeof(*n), member))

#define list_for_each_entry_safe_reverse(position, n, head, member)		\
        for (position = list_entry((head)->prev, typeof(*position), member),    \
            n = list_entry(position->member.prev, typeof(*position), member);   \
             &position->member != (head);                   \
             position = n, n = list_entry(n->member.prev, typeof(*n), member))
    
#define list_for_each_entry_safe_from(position, n, head, member) 			\
	for (n = list_entry(position->member.next, typeof(*position), member);		\
	     &position->member != (head);						\
	     position = n, n = list_entry(n->member.next, typeof(*n), member))

#define list_safe_reset_next(position, n, member)				\
	n = list_entry(position->member.next, typeof(*position), member)

#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }

static OSL_INLINE void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static OSL_INLINE int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static OSL_INLINE int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

static OSL_INLINE void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static OSL_INLINE void hlist_add_head(struct hlist_node *h_node, struct hlist_head *hlist)
{
	struct hlist_node *first = hlist->first;
	h_node->next = first;
	if (first)
		first->pprev = &h_node->next;
	hlist->first = h_node;
	h_node->pprev = &hlist->first;
}

static OSL_INLINE void hlist_del(struct hlist_node *hlist)
{
	__hlist_del(hlist);
	hlist->next = LIST_POISON_NEXT;
	hlist->pprev = LIST_POISON_PREV;
}

static OSL_INLINE void hlist_del_init(struct hlist_node *hlist)
{
	if (!hlist_unhashed(hlist)) {
		__hlist_del(hlist);
		INIT_HLIST_NODE(hlist);
	}
}

#define hlist_for_each(position, head) \
	for (position = (head)->first; position ; position = position->next)

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each_entry(position, head, member)				\
	for (position = /*lint -save -e62 */hlist_entry_safe((head)->first, typeof(*(position)), member);\
	     position;							\
	     position = hlist_entry_safe((position)->member.next, typeof(*(position)), member)/*lint -restore +e62*/)

#define hlist_for_each_safe(position, h_node, head) \
	for (position = (head)->first; position && ({ h_node = position->next; 1; }); \
	     position = h_node)

#define hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
	})


#define hlist_for_each_entry_continue(position, member)			\
	for (position = hlist_entry_safe((position)->member.next, typeof(*(position)), member);\
	     position;							\
	     position = hlist_entry_safe((position)->member.next, typeof(*(position)), member))

#define hlist_for_each_entry_safe(position, h_node, head, member) 		\
        for (position = hlist_entry_safe(/*lint -save -e62*/(head)->first, typeof(*position), member);\
             position && ({ h_node = position->member.next; 1; });          \
             position = hlist_entry_safe(h_node, typeof(*position), member)/*lint -restore +e62*/)
    
#define hlist_for_each_entry_from(position, member)				\
	for (; position;							\
	     position = hlist_entry_safe((position)->member.next, typeof(*(position)), member))

#else

struct list_head {
	struct list_head *next;
    struct list_head *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next;
    struct hlist_node **pprev;
};


#endif /* __KERNEL__ */

#endif
