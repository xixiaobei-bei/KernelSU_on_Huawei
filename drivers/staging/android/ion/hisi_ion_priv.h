/*
 * drivers/staging/android/ion/ion_priv.h
 *
 * Copyright (C) 2011 Google, Inc.
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
 */

#ifndef _ION_PRIV_H
#define _ION_PRIV_H

#include "ion.h"

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
static inline struct ion_heap *ion_seccm_heap_create(struct ion_platform_heap
		*iph)
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
static inline struct ion_heap *ion_seccg_heap_create(struct ion_platform_heap
		*iph)
{
	return NULL;
}

static inline void ion_seccg_heap_destroy(struct ion_heap *ih){ }

static inline struct ion_heap *ion_secsg_heap_create(struct ion_platform_heap
		*iph)
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
static inline struct ion_heap *ion_cpa_heap_create(struct ion_platform_heap
			*iph)
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

void ion_flush_all_cpus_caches(void);

#ifdef CONFIG_ION_HISI_DMA_POOL
struct ion_heap *ion_dma_pool_heap_create(struct ion_platform_heap *);
void ion_dma_pool_heap_destroy(struct ion_heap *);
void ion_register_dma_camera_cma(void *cma);
void ion_clean_dma_camera_cma(void);

#else
static inline struct ion_heap *ion_dma_pool_heap_create(struct ion_platform_heap
		*iph)
{
	return NULL;
}
static inline void ion_dma_pool_heap_destroy(struct ion_heap *ih){ }
static inline void ion_register_dma_camera_cma(void *cma){ }
static inline void ion_clean_dma_camera_cma(void){ }
#endif

#ifdef CONFIG_ION_HISI
void hisi_svc_secmem_info(void);
#else
static inline void hisi_svc_secmem_info(void){}
#endif

#endif /* _ION_PRIV_H */
