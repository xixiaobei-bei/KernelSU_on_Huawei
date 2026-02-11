#ifndef _MEM_TRACE_H
#define _MEM_TRACE_H

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/page-flags.h>

#define SLUB_NAME_LEN  64

enum {
	START_TRACK,
	ION_TRACK = START_TRACK,
	SLUB_TRACK,
	LSLUB_TRACK,
	VMALLOC_TRACK,
	CMA_TRACK,
	ZSPAGE_TRACK,
	BUDDY_TRACK,
	SKB_TRACK,
	NR_TRACK,
};

enum {
	SLUB_ALLOC,
	SLUB_FREE,
	NR_SLUB_ID,
};

struct hisi_slub_detail_info {
	char name[SLUB_NAME_LEN];
	unsigned long active_objs;
	unsigned long num_objs;
	unsigned long active_slabs;
	unsigned long num_slabs;
	unsigned long size;//total size
	unsigned int objects_per_slab;
	unsigned int objsize;
};

struct hisi_ion_detail_info {
	pid_t pid;
	size_t size;
};

struct hisi_vmalloc_detail_info {
	int type;
	size_t size;
};

struct hisi_stack_info {
	unsigned long caller;
	atomic_t ref;
};

#ifdef CONFIG_HISI_PAGE_TRACE
size_t hisi_get_mem_total(int type);
size_t hisi_get_mem_detail(int type, void *buf, size_t len);
int hisi_page_trace_on(int type, char *name);
int hisi_page_trace_off(int type, char *name);
int hisi_page_trace_open(int type, int subtype);
int hisi_page_trace_close(int type, int subtype);
size_t hisi_page_trace_read(int type,
	struct hisi_stack_info *info, size_t len, int subtype);
size_t hisi_get_ion_by_pid(pid_t pid);
void set_buddy_track(struct page *page,
	unsigned int order, unsigned long caller);
void set_lslub_track(struct page *page,
	unsigned int order, unsigned long caller);
void alloc_node_func_map(struct pglist_data *pgdat);
void set_alloc_track(unsigned long caller);
void set_free_track(unsigned long caller);
int buddy_track_map(int nid);
int buddy_track_unmap(void);
void hisi_vmalloc_detail_show(void);
void hisi_mem_stats_show(void);
#else
static inline size_t hisi_get_mem_total(int type)
{
	return 0;
}

static inline size_t hisi_get_mem_detail(int type, void *buf, size_t len)
{
	return 0;
}

static inline size_t hisi_get_ion_by_pid(pid_t pid)
{
	return 0;
}

static inline int hisi_page_trace_on(int type, char *name)
{
	return 0;
}

static inline int hisi_page_trace_off(int type, char *name)
{
	return 0;
}

static inline int hisi_page_trace_open(int type, int subtype)
{
	return 0;
}

static inline int hisi_page_trace_close(int type, int subtype)
{
	return 0;
}

static inline size_t hisi_page_trace_read(int type,
	struct hisi_stack_info *info, size_t len, int subtype)
{
	return 0;
}
static inline void set_buddy_track(struct page *page,
	unsigned int order, unsigned long caller){}
static inline void set_lslub_track(struct page *page,
	unsigned int order, unsigned long caller){}
static inline void alloc_node_func_map(struct pglist_data *pgdat){}
static inline void set_alloc_track(unsigned long caller){}
static inline void set_free_track(unsigned long caller){}
static inline int buddy_track_map(int nid)
{
	return 0;
}
static inline int buddy_track_unmap(void)
{
	return 0;
}
static inline void hisi_vmalloc_detail_show(void){}
static inline void hisi_mem_stats_show(void){}

#endif
#endif
