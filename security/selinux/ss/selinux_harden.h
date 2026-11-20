/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Use prmem to enhance CFI protection.
 * Create: 2020/10/19
 */

#ifndef __HKIP_SELINUX_HARDEN__
#define __HKIP_SELINUX_HARDEN__

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/hisi/prmem.h>
#include "ebitmap.h"
#include "avtab.h"

extern struct prmem_pool selinux_pool;

struct avtab_node*
avtab_insert_node(struct avtab *h, int hvalue,
		  struct avtab_node *prev, struct avtab_node *cur,
		  struct avtab_key *key, struct avtab_datum *datum);

void avtab_destroy(struct avtab *h);

#ifdef CONFIG_HKIP_SELINUX_PROT
static inline struct ebitmap_node *try_alloc(bool protectable, gfp_t gfp)
{
	if(protectable == true)
		return pzalloc(&selinux_pool, sizeof(struct ebitmap_node),
			       PRMEM_NO_FLAGS);
	else
		return kzalloc(sizeof(struct ebitmap_node), gfp);
}

static inline void try_free(bool protectable, struct ebitmap_node *n)
{
	if (protectable == false)
		kfree(n);
}
#else
static inline struct ebitmap_node *try_alloc(bool protectable, gfp_t gfp)
{
	(void)protectable;
	return kzalloc(sizeof(struct ebitmap_node), gfp);
}

static inline void try_free(bool protectable, struct ebitmap_node *n)
{
	(void)protectable;
	kfree(n);
}
#endif

#endif
