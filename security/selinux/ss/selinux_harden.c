/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Use prmem to enhance CFI protection.
 * Create: 2020/10/19
 */

#include <linux/errno.h>
#include <linux/hisi/prmem.h>
#include "selinux_harden.h"
#include "policydb.h"

struct avtab_node*
avtab_insert_node(struct avtab *h, int hvalue,
		  struct avtab_node *prev, struct avtab_node *cur,
		  struct avtab_key *key, struct avtab_datum *datum)
{
	struct avtab_node *newnode = NULL;
	struct avtab_extended_perms *xperms = NULL;
	newnode = pzalloc(&selinux_pool, sizeof(struct avtab_node),
			  PRMEM_NO_FLAGS);
	if (newnode == NULL)
		return NULL;
	newnode->key = *key;

	if (key->specified & AVTAB_XPERMS) {
		xperms = pzalloc(&selinux_pool,
				 sizeof(struct avtab_extended_perms),
				 PRMEM_NO_FLAGS);
		if (xperms == NULL) {
			pfree(newnode);
			return NULL;
		}
		*xperms = *(datum->u.xperms);
		newnode->datum.u.xperms = xperms;
	} else {
		newnode->datum.u.data = datum->u.data;
	}

	if (prev) {
		newnode->next = prev->next;
		prev->next = newnode;
	} else {
		newnode->next = h->htable[hvalue];
		h->htable[hvalue] = newnode;
	}

	h->nel++;
	return newnode;
}

void avtab_destroy(struct avtab *h)
{
	unsigned int i;
	struct avtab_node *cur = NULL;
	struct avtab_node *temp = NULL;

	if (!h || !h->htable)
		return;

	for (i = 0; i < h->nslot; i++) {
		cur = h->htable[i];
		while (cur) {
			temp = cur;
			cur = cur->next;
			if (temp->key.specified & AVTAB_XPERMS)
				pfree(temp->datum.u.xperms);
			pfree(temp);
		}
		h->htable[i] = NULL;
	}
	pfree(h->htable);
	h->htable = NULL;
	h->nslot = 0;
	h->mask = 0;
}
