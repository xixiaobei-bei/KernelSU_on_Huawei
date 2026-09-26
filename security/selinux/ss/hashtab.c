// SPDX-License-Identifier: GPL-2.0
/*
 * Implementation of the hash table type.
 *
 * Author : Stephen Smalley, <sds@tycho.nsa.gov>
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/sched.h>
#ifdef CONFIG_HKIP_SELINUX_PROT
#include <linux/hisi/prmem.h>
#endif
#include "hashtab.h"

#ifdef CONFIG_HKIP_SELINUX_PROT
extern struct prmem_pool selinux_pool;
#endif

struct hashtab *hashtab_create(u32 (*hash_value)(struct hashtab *h, const void *key),
			       int (*keycmp)(struct hashtab *h, const void *key1, const void *key2),
			       u32 size)
{
	struct hashtab *p;
	u32 i;

#ifdef CONFIG_HKIP_SELINUX_PROT
	p = pzalloc(&selinux_pool, sizeof(*p), PRMEM_NO_FLAGS);
#else
	p = kzalloc(sizeof(*p), GFP_KERNEL);
#endif
	if (!p)
		return p;

	p->size = size;
	p->nel = 0;
	p->hash_value = hash_value;
	p->keycmp = keycmp;
#ifdef CONFIG_HKIP_SELINUX_PROT
	p->htable = pmalloc(&selinux_pool, sizeof(*(p->htable)) * size, PRMEM_NO_FLAGS);
	if (!p->htable) {
		pfree(p);
		return NULL;
	}
#else
	p->htable = kmalloc_array(size, sizeof(*p->htable), GFP_KERNEL);
	if (!p->htable) {
		kfree(p);
		return NULL;
	}
#endif

	for (i = 0; i < size; i++)
		p->htable[i] = NULL;

	return p;
}

int hashtab_insert(struct hashtab *h, void *key, void *datum)
{
	u32 hvalue;
	struct hashtab_node *prev, *cur, *newnode;

	cond_resched();

	if (!h || h->nel == HASHTAB_MAX_NODES)
		return -EINVAL;

	hvalue = h->hash_value(h, key);
	prev = NULL;
	cur = h->htable[hvalue];
	while (cur && h->keycmp(h, key, cur->key) > 0) {
		prev = cur;
		cur = cur->next;
	}

	if (cur && (h->keycmp(h, key, cur->key) == 0))
		return -EEXIST;

#ifdef CONFIG_HKIP_SELINUX_PROT
	newnode = pzalloc(&selinux_pool, sizeof(*newnode), PRMEM_NO_FLAGS);
#else
	newnode = kzalloc(sizeof(*newnode), GFP_KERNEL);
#endif
	if (!newnode)
		return -ENOMEM;
	newnode->key = key;
	newnode->datum = datum;
	if (prev) {
		newnode->next = prev->next;
		prev->next = newnode;
	} else {
		newnode->next = h->htable[hvalue];
		h->htable[hvalue] = newnode;
	}

	h->nel++;
	return 0;
}

void *hashtab_search(struct hashtab *h, const void *key)
{
	u32 hvalue;
	struct hashtab_node *cur;

	if (!h)
		return NULL;

	hvalue = h->hash_value(h, key);
	cur = h->htable[hvalue];
	while (cur && h->keycmp(h, key, cur->key) > 0)
		cur = cur->next;

	if (!cur || (h->keycmp(h, key, cur->key) != 0))
		return NULL;

	return cur->datum;
}

void hashtab_destroy(struct hashtab *h)
{
	u32 i;
	struct hashtab_node *cur, *temp;

	if (!h)
		return;

	for (i = 0; i < h->size; i++) {
		cur = h->htable[i];
		while (cur) {
			temp = cur;
			cur = cur->next;
#ifdef CONFIG_HKIP_SELINUX_PROT
			pfree(temp);
#else
			kfree(temp);
#endif
		}
		h->htable[i] = NULL;
	}

#ifdef CONFIG_HKIP_SELINUX_PROT
	pfree(h->htable);
	h->htable = NULL;
	pfree(h);
#else
	kfree(h->htable);
	h->htable = NULL;
	kfree(h);
#endif
}

int hashtab_map(struct hashtab *h,
		int (*apply)(void *k, void *d, void *args),
		void *args)
{
	u32 i;
	int ret;
	struct hashtab_node *cur;

	if (!h)
		return 0;

	for (i = 0; i < h->size; i++) {
		cur = h->htable[i];
		while (cur) {
			ret = apply(cur->key, cur->datum, args);
			if (ret)
				return ret;
			cur = cur->next;
		}
	}
	return 0;
}


void hashtab_stat(struct hashtab *h, struct hashtab_info *info)
{
	u32 i, chain_len, slots_used, max_chain_len;
	struct hashtab_node *cur;

	slots_used = 0;
	max_chain_len = 0;
	for (slots_used = max_chain_len = i = 0; i < h->size; i++) {
		cur = h->htable[i];
		if (cur) {
			slots_used++;
			chain_len = 0;
			while (cur) {
				chain_len++;
				cur = cur->next;
			}

			if (chain_len > max_chain_len)
				max_chain_len = chain_len;
		}
	}

	info->slots_used = slots_used;
	info->max_chain_len = max_chain_len;
}
