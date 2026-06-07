#include <linux/err.h>
#include <linux/mmzone.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include "slab.h"
#include "mem_stack.h"
#ifdef CONFIG_CMA
#include <linux/cma.h>
#endif
#ifdef CONFIG_ION
#include <linux/hisi/hisi_ion.h>
#endif

#define PAGES_TO_BYTE(pages) ((pages) << PAGE_SHIFT)

static size_t get_stats_cma(void);
static size_t get_stats_ion(void);
static size_t get_stats_slub(void);
static size_t get_stats_lslub(void);
static size_t get_stats_vmalloc(void);
static size_t get_stats_skb(void);
static size_t get_stats_zspage(void);
static size_t get_stats_buddy(void);
static size_t hisi_get_slub_detail(void *buf, size_t len);


struct page_stack_trace {
	int (*page_stack_on)(char *);
	int (*page_stack_off)(char *);
	int (*page_stack_open)(int);
	int (*page_stack_close)(void);
	size_t (*page_stack_read)(struct hisi_stack_info *, size_t, int);
};

struct mem_trace {
	int type;
	size_t (*get_mem_stats)(void);
	size_t (*get_mem_detail)(void *, size_t);
	struct page_stack_trace *stack_trace;
};

struct page_stack_trace vmalloc_page_stack = {
	.page_stack_on = hisi_vmalloc_track_on,
	.page_stack_off = hisi_vmalloc_track_off,
	.page_stack_open = hisi_vmalloc_stack_open,
	.page_stack_close = hisi_vmalloc_stack_close,
	.page_stack_read = hisi_vmalloc_stack_read,
};

struct page_stack_trace slub_page_stack = {
	.page_stack_on = hisi_slub_track_on,
	.page_stack_off = hisi_slub_track_off,
	.page_stack_open = hisi_slub_stack_open,
	.page_stack_close = hisi_slub_stack_close,
	.page_stack_read = hisi_slub_stack_read,
};

struct page_stack_trace buddy_page_stack = {
	.page_stack_on = hisi_buddy_track_on,
	.page_stack_off = hisi_buddy_track_off,
	.page_stack_open = hisi_buddy_stack_open,
	.page_stack_close = hisi_buddy_stack_close,
	.page_stack_read = hisi_buddy_stack_read,
};

struct mem_trace memtrace[] = {
	{VMALLOC_TRACK,           get_stats_vmalloc,
	 hisi_get_vmalloc_detail, &vmalloc_page_stack},

	{BUDDY_TRACK,             get_stats_buddy,
	 NULL,                    &buddy_page_stack},

	{SLUB_TRACK,              get_stats_slub,
	 hisi_get_slub_detail,    &slub_page_stack},

	{LSLUB_TRACK,             get_stats_lslub,
	 NULL,                    &buddy_page_stack},/*the same with buddy*/

	{SKB_TRACK,               get_stats_skb,
	 NULL,                    NULL},

	{ZSPAGE_TRACK,            get_stats_zspage,
	 NULL,                    NULL},

	{ION_TRACK,               get_stats_ion,
	 hisi_get_ion_detail,     NULL},

	{CMA_TRACK,               get_stats_cma,
	 NULL,                    NULL},
};

static const char * const track_text[] = {
	"ION_TRACK",
	"SLUB_TRACK",
	"LSLUB_TRACK",
	"VMALLOC_TRACK",
	"CMA_TRACK",
	"ZSPAGE_TRACK",
	"BUDDY_TRACK",
	"SKB_TRACK",
};

static int vmalloc_type[] = {VM_IOREMAP, VM_ALLOC, VM_MAP, VM_USERMAP};
static const char * const vmalloc_text[] = {
	"VM_IOREMAP",
	"VM_ALLOC",
	"VM_MAP",
	"VM_USERMAP",
};

static size_t get_stats_cma(void)
{
	return PAGES_TO_BYTE(totalcma_pages);
}

static size_t get_stats_ion(void)
{
	return hisi_ion_total();
}

static size_t get_stats_slub(void)
{
	return PAGES_TO_BYTE(global_node_page_state(NR_SLAB_UNRECLAIMABLE));
}

static size_t get_stats_lslub(void)
{
	return PAGES_TO_BYTE(global_zone_page_state(NR_LSLAB_PAGES));
}

static size_t get_stats_vmalloc(void)
{
	unsigned int i;
	size_t size = 0;

	for (i = 0; i < ARRAY_SIZE(vmalloc_type); i++)/*lint !e514*/
		size += vm_type_detail_get(vmalloc_type[i]);

	return size;
}

static size_t get_stats_zspage(void)
{
	return PAGES_TO_BYTE(global_zone_page_state(NR_ZSPAGES));
}

static size_t get_stats_skb(void)
{
	return PAGES_TO_BYTE(global_zone_page_state(NR_SKB_PAGES));
}

static size_t get_stats_buddy(void)
{
	struct sysinfo i;

	si_meminfo(&i);
	return PAGES_TO_BYTE(i.totalram - i.freeram);
}

static size_t hisi_get_slub_detail(void *buf, size_t len)
{
	size_t cnt = 0;
	unsigned long size;
	struct kmem_cache *s = NULL;
	struct slabinfo sinfo;
	struct hisi_slub_detail_info *info =
		(struct hisi_slub_detail_info *)buf;

	mutex_lock(&slab_mutex);
	list_for_each_entry(s, &slab_caches, list) {/*lint !e666*/
		if (cnt >= len) {
			mutex_unlock(&slab_mutex);
			return len;
		}
		memset(&sinfo, 0, sizeof(sinfo));
		get_slabinfo(s, &sinfo);
		size = (unsigned long)(unsigned int)s->size;
		(info + cnt)->active_objs = sinfo.active_objs;
		(info + cnt)->num_objs = sinfo.num_objs;
		(info + cnt)->objsize = (unsigned int)s->size;
		(info + cnt)->size = sinfo.num_objs * size;
		strncpy((info + cnt)->name, s->name, SLUB_NAME_LEN);
		cnt++;
	}
	mutex_unlock(&slab_mutex);

	return cnt;
}

size_t hisi_get_ion_by_pid(pid_t pid)
{
	return hisi_get_ion_size_by_pid(pid);
}

size_t hisi_get_mem_total(int type)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type)
			return memtrace[i].get_mem_stats();

	return 0;
}

size_t hisi_get_mem_detail(int type, void *buf, size_t len)
{
	unsigned int i;

	if (!buf)
		return 0;
	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type && memtrace[i].get_mem_detail)
			return memtrace[i].get_mem_detail(buf, len);

	return 0;
}

int hisi_page_trace_on(int type, char *name)
{
	unsigned int i;

	if (!name)
		return -EINVAL;
	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type &&
			memtrace[i].stack_trace)
			return memtrace[i].stack_trace->page_stack_on(name);
	return -EINVAL;
}

int hisi_page_trace_off(int type, char *name)
{
	unsigned int i;

	if (!name)
		return -EINVAL;
	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type &&
			memtrace[i].stack_trace)
			return memtrace[i].stack_trace->page_stack_off(name);

	return -EINVAL;
}

int hisi_page_trace_open(int type, int subtype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type &&
			memtrace[i].stack_trace)
			return memtrace[i].stack_trace->page_stack_open(subtype);

	return -EINVAL;
}

int hisi_page_trace_close(int type, int subtype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type &&
			memtrace[i].stack_trace)
			return memtrace[i].stack_trace->page_stack_close();

	return -EINVAL;
}

size_t hisi_page_trace_read(int type,
	struct hisi_stack_info *info, size_t len, int subtype)
{
	unsigned int i;

	if (!info)
		return 0;
	for (i = 0; i < ARRAY_SIZE(memtrace); i++)/*lint !e514*/
		if (type == memtrace[i].type &&
			memtrace[i].stack_trace)
			return memtrace[i].stack_trace->page_stack_read(
				info, len, subtype);

	return 0;
}

void hisi_mem_stats_show(void)
{
	size_t size;
	int i;

	pr_err("========mem stat start==========\n");
	for (i = START_TRACK; i < NR_TRACK; i++) {
		size = hisi_get_mem_total(i);
		pr_err("%s used: %ld kB\n", track_text[i], size/1024UL);
	}
	pr_err("=========mem stat end==========\n");
}

void hisi_vmalloc_detail_show(void)
{
	unsigned int i;
	size_t size;

	pr_err("========get vmalloc info start==========\n");

	for (i = 0; i < ARRAY_SIZE(vmalloc_type); i++) {/*lint !e514*/
		size = vm_type_detail_get(vmalloc_type[i]);
		pr_err("vmalloc type:%s, size:%zu kB\n",
			vmalloc_text[i], size/1024UL);
	}

	pr_err("========get vmalloc info end==========\n");
}

#ifdef CONFIG_HISI_PAGE_TRACE_TEST
#include "mem_trace_test.c"
#endif
