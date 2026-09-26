/*
 * zsmalloc memory allocator
 *
 * Copyright (C) 2011  Nitin Gupta
 * Copyright (C) 2012, 2013 Minchan Kim
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the license that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 */

#ifndef _ZS_MALLOC_H_
#define _ZS_MALLOC_H_

#include <linux/types.h>

/*
 * zsmalloc mapping modes
 *
 * NOTE: These only make a difference when a mapped object spans pages.
 * They also have no effect when PGTABLE_MAPPING is selected.
 */
enum zs_mapmode {
	ZS_MM_RW, /* normal read-write mapping */
	ZS_MM_RO, /* read-only (no copy-out at unmap time) */
	ZS_MM_WO /* write-only (no copy-in at map time) */
	/*
	 * NOTE: ZS_MM_WO should only be used for initializing new
	 * (uninitialized) allocations.  Partial writes to already
	 * initialized allocations should use ZS_MM_RW to preserve the
	 * existing data.
	 */
};

struct zs_pool_stats {
	/* How many pages were migrated (freed) */
	unsigned long pages_compacted;
};

struct zs_pool;

#ifdef CONFIG_ZS_MALLOC_EXT

typedef size_t (ext_size_parse_fn) (void *);
typedef struct page *(ext_zsmalloc_fn) (void *, gfp_t);

bool is_ext_pool(struct zs_pool *pool);
extern void zs_pool_enable_ext(struct zs_pool *pool, bool enable,
				ext_size_parse_fn *parse_fn);

extern void zs_pool_ext_malloc_register(struct zs_pool *pool,
						ext_zsmalloc_fn *fn);

#else

typedef size_t (ext_size_parse_fn) (void *);
typedef struct page *(ext_zsmalloc_fn) (void *, gfp_t);

static inline bool is_ext_pool(struct zs_pool *pool)
{
	return false;
}
static inline void zs_pool_enable_ext(struct zs_pool *pool,
			bool enable, ext_size_parse_fn parse_fn) {}

static inline void zs_pool_ext_malloc_register(
		struct zs_pool *pool, ext_zsmalloc_fn fn) {}
#endif

struct zs_pool *zs_create_pool(const char *name);
void zs_destroy_pool(struct zs_pool *pool);

unsigned long zs_malloc(struct zs_pool *pool, size_t size, gfp_t flags);
void zs_free(struct zs_pool *pool, unsigned long obj);

size_t zs_huge_class_size(struct zs_pool *pool);

void *zs_map_object(struct zs_pool *pool, unsigned long handle,
			enum zs_mapmode mm);
void zs_unmap_object(struct zs_pool *pool, unsigned long handle);

unsigned long zs_get_total_pages(struct zs_pool *pool);
unsigned long zs_compact(struct zs_pool *pool);

void zs_pool_stats(struct zs_pool *pool, struct zs_pool_stats *stats);
#endif
