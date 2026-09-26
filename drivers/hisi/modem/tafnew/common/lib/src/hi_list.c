/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "hi_list.h"


/*
 * Insert a new entry between two known consecutive entries.
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
MODULE_EXPORTED void __list_add_msp(struct msp_list_header *newnew, struct msp_list_header *prev,
                                    struct msp_list_header *next)
{
    next->prev   = newnew;
    newnew->next = next;
    newnew->prev = prev;
    prev->next   = newnew;
}

/*
 * msp_list_add - add a new entry
 * @newnew: new entry to be added
 * @head: list head to add it after
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
MODULE_EXPORTED void msp_list_add(struct msp_list_header *newnew, struct msp_list_header *head)
{
    __list_add_msp(newnew, head, head->next);
}

/*
 * msp_list_add_tail - add a new entry
 * @newnew: new entry to be added
 * @head: list head to add it before
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
MODULE_EXPORTED void msp_list_add_tail(struct msp_list_header *newnew, struct msp_list_header *head)
{
    __list_add_msp(newnew, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
MODULE_EXPORTED void __msp_list_del(struct msp_list_header *prev, struct msp_list_header *next)
{
    next->prev = prev;
    prev->next = next;
}

/*
 * msp_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: msp_list_empty on entry does not return true after this, the entry is in an undefined state.
 */
MODULE_EXPORTED void msp_list_del(struct msp_list_header *entry)
{
    __msp_list_del(entry->prev, entry->next);
}

/*
 * msp_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
MODULE_EXPORTED int msp_list_empty(const struct msp_list_header *head)
{
    return head->next == head;
}

void __msp_list_splice(const struct msp_list_header *list,
                       struct msp_list_header *prev,
                       struct msp_list_header *next)
{
    struct msp_list_header *first = list->next;
    struct msp_list_header *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/*
 * msp_list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
MODULE_EXPORTED void msp_list_splice(const struct msp_list_header *list,
                                     struct msp_list_header *head)
{
    if (!msp_list_empty(list)) {
        __msp_list_splice(list, head, head->next);
    }
}

/*
 * msp_list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
MODULE_EXPORTED void msp_list_splice_tail(const struct msp_list_header *list,
                                          struct msp_list_header *head)
{
    if (!msp_list_empty(list)) {
        __msp_list_splice(list, head->prev, head);
    }
}

/*
 * msp_list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
MODULE_EXPORTED void msp_list_splice_init(struct msp_list_header *list,
                                          struct msp_list_header *head)
{
    if (!msp_list_empty(list)) {
        __msp_list_splice(list, head, head->next);
        HI_INIT_LIST_HEAD(list);
    }
}

/**
 * msp_list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
MODULE_EXPORTED void msp_list_splice_tail_init(struct msp_list_header *list,
                                               struct msp_list_header *head)
{
    if (!msp_list_empty(list)) {
        __msp_list_splice(list, head->prev, head);
        HI_INIT_LIST_HEAD(list);
    }
}

