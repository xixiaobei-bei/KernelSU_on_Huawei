/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _MM_ION_PRIV_H
#define _MM_ION_PRIV_H

#include "ion.h"

#ifndef CONFIG_HISI_ION_CACHE_FLUSH_THRESHOLD
#define CONFIG_HISI_ION_CACHE_FLUSH_THRESHOLD (18)
#endif

#define MM_ION_FLUSH_ALL_CPUS_CACHES  \
	(CONFIG_HISI_ION_CACHE_FLUSH_THRESHOLD * SZ_1M)

#ifdef CONFIG_ION_SYSTEM_HEAP
struct ion_heap *ion_system_heap_create(struct ion_platform_heap *heap_data);
#else
static inline struct ion_heap *ion_system_heap_create(struct ion_platform_heap *heap_data)
{
	return NULL;
}
#endif

#ifdef CONFIG_ION_CARVEOUT_HEAP
struct ion_heap *ion_carveout_heap_create(struct ion_platform_heap *);
void ion_carveout_heap_destroy(struct ion_heap *);
#else
static inline struct ion_heap *ion_carveout_heap_create(
		struct ion_platform_heap *iph)
{
	return NULL;
}
static inline void ion_carveout_heap_destroy(struct ion_heap *ih){ }
#endif

#ifdef CONFIG_ION_HISI_SECCM
struct ion_heap *ion_seccm_heap_create(struct ion_platform_heap *);
void ion_seccm_heap_destroy(struct ion_heap *);
int ion_seccm_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len);
#else
static inline struct ion_heap *ion_seccm_heap_create(
				struct ion_platform_heap *iph)
{
	return NULL;
}

static inline void ion_seccm_heap_destroy(struct ion_heap *ih){ }

static inline int ion_seccm_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len)
{
	pr_err("%s: not sec mem!\n", __func__);
	return -EINVAL;
}
#endif

#ifdef CONFIG_ION_HISI_SUPPORT_HEAP_MERGE
int ion_dma_pool_heap_phys(struct ion_heap *heap,
			   struct ion_buffer *buffer,
			   phys_addr_t *addr, size_t *len);
#else
static inline int ion_dma_pool_heap_phys(struct ion_heap *heap,
					 struct ion_buffer *buffer,
					 phys_addr_t *addr, size_t *len)
{
	pr_err("%s: not sec mem!\n", __func__);
	return -EINVAL;
}
#endif

#ifdef CONFIG_ION_HISI_SECSG
struct ion_heap *ion_seccg_heap_create(struct ion_platform_heap *);
struct ion_heap *ion_secsg_heap_create(struct ion_platform_heap *);
void ion_secsg_heap_destroy(struct ion_heap *);
void ion_seccg_heap_destroy(struct ion_heap *);
int ion_seccg_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len);
int ion_secsg_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len);
#else
static inline struct ion_heap *ion_seccg_heap_create(
				struct ion_platform_heap *iph)
{
	return NULL;
}

static inline void ion_seccg_heap_destroy(struct ion_heap *ih){ }

static inline struct ion_heap *ion_secsg_heap_create(
				struct ion_platform_heap *iph)
{
	return NULL;
}

static inline void ion_secsg_heap_destroy(struct ion_heap *ih){ }

static inline int ion_seccg_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len)
{
	pr_err("%s: not sec mem!\n", __func__);
	return -EINVAL;
}

static inline int ion_secsg_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len)
{
	return -EINVAL;
}
#endif

#ifdef CONFIG_ION_HISI_CPA
struct ion_heap *ion_cpa_heap_create(struct ion_platform_heap *);

int ion_cpa_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len);
#else
static inline struct ion_heap *ion_cpa_heap_create(
				struct ion_platform_heap *iph)
{
	return NULL;
}

static inline int ion_cpa_heap_phys(struct ion_heap *heap,
		struct ion_buffer *buffer,
		phys_addr_t *addr, size_t *len)
{
	pr_err("%s: not cpa mem!\n", __func__);
	return -EINVAL;
}
#endif

#ifdef CONFIG_ION_HISI_CMA_HEAP
struct ion_heap *ion_mm_cma_heap_create(struct ion_platform_heap *);
#else
static inline struct ion_heap *ion_mm_cma_heap_create(
				struct ion_platform_heap *iph)
{
	return NULL;
}
#endif

void ion_flush_all_cpus_caches(void);
#ifdef CONFIG_HISI_ION_FLUSH_CACHE_ALL
void ion_flush_all_cpus_caches_raw(void);
#endif

#ifdef CONFIG_ION_HISI_DMA_POOL
struct ion_heap *ion_dma_pool_heap_create(struct ion_platform_heap *);
void ion_dma_pool_heap_destroy(struct ion_heap *);
void ion_register_dma_camera_cma(void *cma);
void ion_clean_dma_camera_cma(void);

#else
static inline struct ion_heap *ion_dma_pool_heap_create(
				struct ion_platform_heap *iph)
{
	return NULL;
}
static inline void ion_dma_pool_heap_destroy(struct ion_heap *ih){ }
static inline void ion_register_dma_camera_cma(void *cma){ }
static inline void ion_clean_dma_camera_cma(void){ }
#endif

#ifdef CONFIG_ION_HISI
void mm_svc_secmem_info(void);
int mm_ion_mutex_lock_recursive(struct mutex *lock);
void mm_ion_mutex_unlock_recursive(struct mutex *lock, int is_lock_recursive);
#else
static inline void mm_svc_secmem_info(void){}
static inline int mm_ion_mutex_lock_recursive(struct mutex *lock)
{
	mutex_lock(lock);
	return 0;
}
static inline void mm_ion_mutex_unlock_recursive(struct mutex *lock,
				int is_lock_recursive)
{
	mutex_unlock(lock);
}
#endif

#endif /* _MM_ION_PRIV_H */
