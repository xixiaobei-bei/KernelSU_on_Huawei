#ifndef _LINUX_HISI_CMD_DEBUG_H
#define _LINUX_HISI_CMD_DEBUG_H

#define HISI_CMA_TRACE_FRAME_MAX 8
#define HISI_CMA_TRACE_MASTER_MAX 40

struct cma_debug_task {
	pid_t page_pid;
	atomic_t count;
	char comm[TASK_COMM_LEN];
	void *func[HISI_CMA_TRACE_FRAME_MAX];
};

struct cma_debug_page {
	atomic64_t cur_obj;
	struct cma_debug_task entries[HISI_CMA_TRACE_MASTER_MAX];
};

struct cma_debug_traced_cma {
	struct list_head cma_debug_node;
	unsigned long cma_spfn;
	unsigned long cma_epfn;
	struct cma_debug_page *cma_traced_pages;
};

#ifdef CONFIG_HISI_CMA_RECORD_DEBUG
void record_cma_alloc_info(struct cma *cma, unsigned long pfn, size_t count);
void record_cma_release_info(struct cma *cma, unsigned long pfn, size_t count);
void dump_cma_mem_info(void);
#else
static inline void record_cma_alloc_info(struct cma *cma, unsigned long pfn, size_t count) {}
static inline void record_cma_release_info(struct cma *cma, unsigned long pfn, size_t count) {}
static inline void dump_cma_mem_info(void) {}
#endif

void dump_cma_page(struct cma *cma, size_t count,
			unsigned long mask, unsigned long offset,
			unsigned long bitmap_maxno, unsigned long bitmap_count);
void cma_page_count_stack_saved(struct page *page, int nr, int set_flag);
void cma_set_flag(struct cma *cma, unsigned long node);
void dump_cma_debug_task(struct cma *cma);
void cma_add_debug_list(struct cma *cma);
void set_himntn_cma_trace_flag(void);
int get_himntn_cma_trace_flag(void);

#endif
