#ifndef _LINUX_HISI_CMD_DEBUG_H
#define _LINUX_HISI_CMD_DEBUG_H

#ifdef CONFIG_HISI_CMA_RECORD_DEBUG
void record_cma_alloc_info(struct cma *cma, unsigned long pfn, size_t count);
void record_cma_release_info(struct cma *cma, unsigned long pfn, size_t count);
void dump_cma_mem_info(void);
#else
static inline void record_cma_alloc_info(struct cma *cma, unsigned long pfn, size_t count) {}
static inline void record_cma_release_info(struct cma *cma, unsigned long pfn, size_t count) {}
static inline void dump_cma_mem_info(void) {}
#endif

#ifdef CONFIG_HISI_CMA_DEBUG
unsigned int cma_bitmap_used(struct cma *cma);
unsigned long int cma_bitmap_maxchunk(struct cma *cma);
void dump_cma_page_flags(unsigned long pfn);
void dump_cma_page(struct cma *cma, size_t count,
			unsigned long mask, unsigned long offset,
			unsigned long bitmap_maxno, unsigned long bitmap_count);
#else
static inline unsigned int cma_bitmap_used(struct cma *cma) {}
static inline unsigned long int cma_bitmap_maxchunk(struct cma *cma) {}
static inline void dump_cma_page_flags(unsigned long pfn){}
static void dump_cma_page(struct cma *cma, size_t count,
			unsigned long mask, unsigned long offset,
			unsigned long bitmap_maxno, unsigned long bitmap_count) {}
#endif

#endif
