/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Memory protection library - core functions
 * Date: 2020/04/15
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/hisi/hkip.h>
#include <asm/cacheflush.h>
#include <asm/barrier.h>
#include <asm/tlbflush.h>
#include <linux/hisi/prmem.h>

#ifdef pr_fmt
#undef pr_fmt
#endif

#define pr_fmt(fmt) "prmem : " fmt

/* ------------------------ physical pages dbs ------------------------ */

static unsigned int pmalloc_phys_db_size_mb = PRMEM_PHYS_MEMORY_SIZE_MB;

PRMEM_PHYS_DB(prmem_pgtable_pages, DO_NOT_PREMAP);

PRMEM_PHYS_DB(prmem_pmalloc, DO_PREMAP);

/* ------------------------- prmem locking ---------------------------- */

static PRMEM_LOCK(master_lock);

static inline void get_prmem_lock(struct prmem_lock *lock)
{
	spin_lock_irqsave(&lock->spinlock, lock->irq_flags);
}

static inline void put_prmem_lock(struct prmem_lock *lock)
{
	spin_unlock_irqrestore(&lock->spinlock, lock->irq_flags);
}

#define get_master_lock(void)                                           \
	get_prmem_lock(&master_lock)
#define put_master_lock(void)                                           \
	put_prmem_lock(&master_lock)

#define get_prmem_obj_lock(obj_ptr)                                     \
	get_prmem_lock(obj_ptr->lock)
#define put_prmem_obj_lock(obj_ptr)                                     \
	put_prmem_lock(obj_ptr->lock)

#define demote_master_lock(obj_ptr)                                     \
do {                                                                    \
	spin_lock(&obj_ptr->lock->spinlock);                            \
	obj_ptr->lock->irq_flags = master_lock.irq_flags;               \
	spin_unlock(&master_lock.spinlock);                             \
} while (0)

/* --------------------- prmem error handling ------------------------- */

/* enum prmem_errno - error states that can be returned within prmem */
enum prmem_errno {
	PRMEM_OK = 0,
	PRMEM_VA_NOT_VALID,
	PRMEM_VA_NOT_FOUND,
	PRMEM_VA_NOT_RECLAIMABLE,
	PRMEM_NODE_NOT_FOUND,
	PRMEM_NODES_MAXED,
	PRMEM_NODES_BATCH_FAIL,
	PRMEM_POOL_UNKNOWN_TYPE,
	PRMEM_NO_SPACE,
	PRMEM_INVALID_INPUT_PARAMETERS,
	PRMEM_DB_INIT_NO_CHUNK,
	PRMEM_NO_CHUNK,
	PRMEM_NO_NODE,
	PRMEM_NO_RANGE,
	PRMEM_NO_PHYS,
	PRMEM_NO_PREP,
	PRMEM_NO_VA,
	PRMEM_NO_GROW,
	PRMEM_NO_METADATA,
	PRMEM_STRING_TOO_LONG,
	PRMEM_INVALID_ADDRESS,
	PRMEM_POOL_IS_START_WR,
	PRMEM_POOL_NOT_RECLAIMABLE,
	PRMEM_INVALID_RANGE,
	PRMEM_OVERLAP_RANGE,
	PRMEM_LEAK_PHYS,
	PRMEM_PREPARE_VA_FAIL,
	PRMEM_ERR_NUMBERS,
};

static const char *error_messages[PRMEM_ERR_NUMBERS] = {
	[PRMEM_VA_NOT_VALID] = "Not a valid vmap_area",
	[PRMEM_VA_NOT_FOUND] = "Not found vmap_area",
	[PRMEM_VA_NOT_RECLAIMABLE] = "Not reclaimable vmap_area",
	[PRMEM_NODE_NOT_FOUND] = "Not found prmem node",
	[PRMEM_NODES_MAXED] = "Reached maximum number of prmem_nodes",
	[PRMEM_NODES_BATCH_FAIL] = "Failed allocating prmem_nodes batch",
	[PRMEM_NO_SPACE] = "Could not allocate space",
	[PRMEM_POOL_UNKNOWN_TYPE] = "Pool has unknown type",
	[PRMEM_INVALID_INPUT_PARAMETERS] = "Prmem invalid input parameters",
	[PRMEM_DB_INIT_NO_CHUNK] = "Failed to init phys db pages pool",
	[PRMEM_NO_CHUNK] = "Could not allocate chunk of contiguous physical pages",
	[PRMEM_NO_NODE] = "Could not allocate node",
	[PRMEM_NO_RANGE] = "Failed to reserve address range",
	[PRMEM_NO_PHYS] = "Failed to reserve physical pages",
	[PRMEM_NO_PREP] = "Failed to prepare the physical db",
	[PRMEM_NO_VA] = "Failed to allocate new vmap area",
	[PRMEM_NO_GROW] = "Failed to grow the memory pool",
	[PRMEM_NO_METADATA] = "Could not allocate metadata",
	[PRMEM_STRING_TOO_LONG] = "String doesn't fit in a cache object",
	[PRMEM_INVALID_ADDRESS] = "Not prmem memory",
	[PRMEM_POOL_IS_START_WR] = "Trying to protect a start_wr pool",
	[PRMEM_POOL_NOT_RECLAIMABLE] = "Pool is not reclaimable",
	[PRMEM_INVALID_RANGE] = "Usercopy range invalid",
	[PRMEM_OVERLAP_RANGE] = "Usercopy range overlaps with prmem",
	[PRMEM_LEAK_PHYS] =  "Failed to return physical pages to the db",
	[PRMEM_PREPARE_VA_FAIL] = "Failed to prepare vma area",
};

static inline void __prmem_err(enum prmem_errno errno,
			       const char *filename, const char *func,
			       unsigned int line_nr)
{
	if (likely((errno >= PRMEM_ERR_NUMBERS) || (!filename) ||
		   (!func)))
		return;
	pr_err("[%s:%u %s] %s, free pages:%lu", filename, line_nr, func,
	       error_messages[errno],
	       prmem_pmalloc_phys_db.total_free_pages_nr);
}

#define prmem_err(errno)						\
	__prmem_err(errno, __FILE__, __func__, __LINE__)

#define prmem_err_ret(errno)						\
{									\
	prmem_err(errno);						\
	return -(errno);						\
}

/* --------------------- vmap area protection ------------------------ */

static LIST_HEAD(prmem_vmap_area_list);
static struct rb_root vmap_area_root = RB_ROOT;

static inline bool is_vmap_area_valid(struct vmap_area *va);
static struct vmap_area *__prmem_find_vmap_area(unsigned long addr)
{
	struct rb_node *n = vmap_area_root.rb_node;

	while (n) {
		struct vmap_area *va = NULL;

		va = rb_entry(n, struct vmap_area, rb_node);
		if (addr < va->va_start)
			n = n->rb_left;
		else if (addr >= va->va_end)
			n = n->rb_right;
		else if (likely(is_vmap_area_valid(va)))
			return va;
		else
			return NULL;
	}
	return NULL;
}

int __change_prmem_common(unsigned long start, unsigned long size,
			  pgprot_t set_mask, pgprot_t clear_mask);

static int change_prmem_common(unsigned long addr, int numpages,
			       pgprot_t set_mask, pgprot_t clear_mask)
{
	unsigned long start = addr;
	unsigned long size = PAGE_SIZE * numpages;
	unsigned long end = start + size;

	if (numpages == 0)
		return 0;

	if (!PAGE_ALIGNED(addr)) {
		start &= PAGE_MASK;
		end = start + size;
		WARN_ON_ONCE(1);
	}

	return __change_prmem_common(start, size, set_mask, clear_mask);
}

static int set_prmem_ro(unsigned long addr, int numpages)
{
	if (unlikely(!prmem_active()))
		return numpages;
	return change_prmem_common(addr, numpages, __pgprot(PTE_RDONLY),
				   __pgprot(PTE_WRITE));
}

static int set_prmem_rw(unsigned long addr, int numpages)
{
	if (unlikely(!prmem_active()))
		return numpages;
	return change_prmem_common(addr, numpages, __pgprot(PTE_WRITE),
				   __pgprot(PTE_RDONLY));
}


/*--------------------- Physical pages allocator ----------------------*/

#define PMALLOC_CHUNKS_NR						\
	DIV_ROUND_UP(MB_UL(pmalloc_phys_db_size_mb), PMALLOC_CHUNK_SIZE)

#define PMALLOC_PHYS_DB_PAGES_NR					\
	DIV_ROUND_UP(MB_UL(pmalloc_phys_db_size_mb), PAGE_SIZE)

/*
 * The number of pages required to support the pmalloc entries in the
 * pagetables is rounded up to double the number of ptes.
 */
#define PGTABLE_PHYS_DB_PAGES_NR					\
	DIV_ROUND_UP(PMALLOC_PHYS_DB_PAGES_NR * 2, (PAGE_SIZE / sizeof(pte_t)))

#define PGTABLE_PAGES_PER_CHUNK						\
	(PGTABLE_PHYS_DB_PAGES_NR * 3)

#define PGTABLE_CHUNKS_NR 1

static struct vmap_area aux_va;
static struct page **aux_pages;

int prmem_vmap_page_range(unsigned long start, unsigned long end,
			  pgprot_t prot, struct page **pages);
void prmem_vunmap_page_range(unsigned long addr, unsigned long end);
/*
 * Temporarily invoke the hypervisor protection on the whole new chunk, to
 * force the allocation of pages in the page table of the underlying
 * protection mechanism.
 */
static __init int prepare_mappings(struct prmem_phys_chunk *chunk,
				   unsigned long pfn)
{
	unsigned long i;
	int ret;

	for (i = 0; i < chunk->pages_per_chunk; i++)
		aux_pages[i] = pfn_to_page(pfn + i);
	ret = prmem_vmap_page_range(aux_va.va_start, aux_va.va_end, PAGE_KERNEL,
				    aux_pages);
	if (ret != chunk->pages_per_chunk) {
		prmem_err(PRMEM_NO_PHYS);
		return -PRMEM_NO_PHYS;
	}

	set_prmem_ro(aux_va.va_start, chunk->pages_per_chunk);
	hkip_register_rowm_mod((void *)(uintptr_t)aux_va.va_start,
			       chunk->pages_per_chunk * PAGE_SIZE);
	hkip_unregister_rowm_mod((void *)(uintptr_t)aux_va.va_start,
				chunk->pages_per_chunk * PAGE_SIZE);
	set_prmem_rw(aux_va.va_start, chunk->pages_per_chunk);
	prmem_vunmap_page_range(aux_va.va_start, aux_va.va_end);
	return 0;
}

static __init bool phys_db_add_chunk(struct prmem_phys_chunk *chunk,
				     bool premap)
{
	void *addr = NULL;

	/* Use "exact" to split the chunk into order 0 pages */
	addr = alloc_pages_exact(chunk->pages_per_chunk * PAGE_SIZE, GFP_KERNEL);
	if (unlikely(addr == NULL)) {
		prmem_err(PRMEM_NO_CHUNK);
		return false;
	}
	if (premap && prepare_mappings(chunk, virt_to_pfn(addr)) != 0) {
		prmem_err(PRMEM_NO_PREP);
		return false;
	}
	chunk->base_pfn = virt_to_pfn((uintptr_t)addr);
	chunk->free_pages_nr =  chunk->pages_per_chunk;
	/* 0xFF set every bits of chunk bitmap to 1 */
	memset(chunk->map, 0xFF, sizeof(unsigned long) * PRMEM_MAP_SIZE);
	return true;
}

static unsigned long get_chunk_pages(struct prmem_phys_chunk *c,
				     struct page **p,
				     unsigned long pages_nr)
{
	unsigned long n = 0;
	unsigned int word;
	unsigned int bit;
	unsigned long pfn;

	for (word = 0; word < PRMEM_MAP_SIZE; word++)
		for (bit = 0; bit < PRMEM_MAP_ROW_BITS; bit++)
			if ((c->free_pages_nr == 0) || (n == pages_nr)) {
				goto out;
			} else if ((c->map[word]) & (1UL << bit)) {
				c->map[word] &= ~(1UL << bit);
				pfn = c->base_pfn;
				pfn += word * PRMEM_MAP_ROW_BITS + bit;
				p[n] = pfn_to_page(pfn);
				n++;
				c->free_pages_nr--;
			}
out:
	return n;
}

static inline void get_phys_db_pages(struct prmem_phys_db *db,
				     struct page **p,
				     unsigned long pages_nr)
{
	unsigned int i;
	unsigned long n;

	for (i = 0, n = 0; (n < pages_nr) && (i < db->chunks_nr); i++)
		n += get_chunk_pages(db->chunks + i, p + n, pages_nr - n);
	db->total_free_pages_nr -= n;
	db->total_used_pages_nr += n;
	if (db->total_used_pages_nr > db->peak_total_used_pages_nr)
		db->peak_total_used_pages_nr = db->total_used_pages_nr;
}

static bool get_phys_pages(struct prmem_phys_db *db, struct page **p,
			   unsigned long pages_nr)
{
	struct prmem_phys_db tmp_db;

	if (db->total_free_pages_nr < pages_nr)
		return false;
	memcpy(&tmp_db, db, sizeof(struct prmem_phys_db));
	get_phys_db_pages(&tmp_db, p, pages_nr);
	if (memcmp(&tmp_db, db, sizeof(struct prmem_phys_db)))
		wr_assign(*db, tmp_db);
	return true;
}

static struct page *get_phys_page(struct prmem_phys_db *db)
{
	struct page *p = NULL;

	if (unlikely(!get_phys_pages(db, &p, 1)))
		return NULL;
	return p;
}

static bool page_in_chunk(struct page *page,
			  struct prmem_phys_chunk *chunk)
{
	unsigned long pfn = page_to_pfn(page);

	return ((chunk->base_pfn <= pfn) &&
		(pfn < (chunk->base_pfn + chunk->pages_per_chunk)));
}

static struct prmem_phys_chunk *get_page_chunk(struct prmem_phys_db *db,
	struct page *page)
{
	unsigned int c;

	for (c = 0; c < db->chunks_nr; c++)
		if (unlikely(page_in_chunk(page, db->chunks + c)))
			return db->chunks + c;
	return NULL;
}

static void put_chunk_page(struct page *page,
			   struct prmem_phys_chunk *chunk)
{
	unsigned long offset = page_to_pfn(page) - chunk->base_pfn;
	unsigned long word = offset / PRMEM_MAP_ROW_BITS;
	unsigned long bit = offset % PRMEM_MAP_ROW_BITS;

	chunk->map[word] |= (1UL << bit);
	chunk->free_pages_nr++;
}

static int put_phys_db_page(struct prmem_phys_db *db,
			    struct page *page)
{
	struct prmem_phys_chunk *chunk = NULL;

	chunk = get_page_chunk(db, page);
	if (unlikely(chunk == NULL)) {
		prmem_err(PRMEM_LEAK_PHYS);
		return -PRMEM_LEAK_PHYS;
	}
	put_chunk_page(page, chunk);
	db->total_free_pages_nr++;
	db->total_used_pages_nr--;
	return 0;
}

static int put_phys_pages(struct prmem_phys_db *db, struct prmem_node *node)
{
	struct prmem_phys_db tmp_db;
	unsigned long base = node->start;
	unsigned long offset;

	memcpy(&tmp_db, db, sizeof(struct prmem_phys_db));
	for (offset = 0; offset < node->size; offset += PAGE_SIZE) {
		struct page *p = NULL;

		p = vmalloc_to_page((void *)(uintptr_t)(base + offset));
		if (unlikely(!p)) {
			prmem_err(PRMEM_LEAK_PHYS);
			goto out;
		}
		if (unlikely(put_phys_db_page(&tmp_db, p)))
			goto out;
	}
out:
	if (memcmp(&tmp_db, db, sizeof(struct prmem_phys_db)))
		wr_assign(*db, tmp_db);
	return (node->size - offset) / PAGE_SIZE;
}

static int put_phys_page(struct prmem_phys_db *db, struct page *page)
{
	struct prmem_phys_db tmp_db;
	bool retval = false;

	memcpy(&tmp_db, db, sizeof(struct prmem_phys_db));
	retval = put_phys_db_page(&tmp_db, page);
	if (memcmp(&tmp_db, db, sizeof(struct prmem_phys_db)))
		wr_assign(*db, tmp_db);
	return retval;
}

static bool is_phys_page(struct prmem_phys_db *db, struct page *page)
{
	return !!get_page_chunk(db, page);
}
/*--------------- Original vmalloc code fitted to prmem ----------------*/

/**
 * declare_prmem_range() - instantiates a specific pmalloc range
 * @name: the name of the range
 *
 * To avoid the tedious - and error-prone - work of populating the data
 * with the description of each pmalloc range, it is more appropriate to
 * have a macro pulling all the related information together.
 * WARNING: this *MUST* be kept in sync with "enum prmem_type"
 */
#define declare_prmem_range(name)					\
	[PRMEM_##name] = {						\
		.start = PRMEM_##name##_START,				\
		.end = PRMEM_##name##_END,				\
		.range_name = __stringify(name),			\
	}

const struct prmem_range g_prmem_ranges[PRMEM_INDEX_NUM] = {
	declare_prmem_range(RO_NO_RECL),
	declare_prmem_range(WR_NO_RECL),
	declare_prmem_range(START_WR_NO_RECL),
	declare_prmem_range(START_WR_RECL),
	declare_prmem_range(WR_RECL),
	declare_prmem_range(RO_RECL),
	declare_prmem_range(RW_RECL),
	declare_prmem_range(NO_RECL),
	declare_prmem_range(RECL),
	declare_prmem_range(WR),
	declare_prmem_range(START_WR),
	declare_prmem_range(FULL),
};
EXPORT_SYMBOL(g_prmem_ranges); /*lint !e545*/

static unsigned long get_paddr(unsigned long size, unsigned long align,
			       unsigned long vstart, unsigned long vend)
{
	unsigned long addr = ALIGN(vstart, align);
	struct rb_node *n = vmap_area_root.rb_node;
	struct vmap_area *first = NULL;
	struct vmap_area *tmp = NULL;

	if (unlikely((vend - align) < size)) {
		pr_err("[%s:%s %u] address space smaller than %lu",
		       __FILE__, __func__, __LINE__, size);
		return (unsigned long)NULL;
	}

	while (n) {
		tmp = rb_entry(n, struct vmap_area, rb_node);
		if (tmp->va_end >= addr) {
			first = tmp;
			if (tmp->va_start <= addr)
				break;
			n = n->rb_left;
		} else {
			n = n->rb_right;
		}
	}

	if (!first)
		goto found;
	/* walk areas until a suitable hole is found */
	while (addr + size > first->va_start && addr + size <= vend) {
		addr = ALIGN(first->va_end, align);
		if (unlikely(addr + size < addr)) {
			pr_err("[%s:%s %u] No address holes as large as %lu",
			       __FILE__, __func__, __LINE__, size);
			return (unsigned long)NULL;
		}
		if (list_is_last(&first->list, &prmem_vmap_area_list))
			goto found;
		first = list_next_entry(first, list);
	}
found:
	if (unlikely(addr + size > vend || addr < vstart)) {
		pr_err("[%s:%s %u] No address holes as large as aligned %lu",
		       __FILE__, __func__, __LINE__, size);
		return (unsigned long)NULL;
	}
	return addr;
}

static bool prepare_vmap_area(struct prmem_phys_db *db,
			      struct vmap_area *va, unsigned long size,
			      unsigned long vstart, unsigned long vend)
{
	size = PAGE_ALIGN(size);
	/* Try to locate a suitable address range */
	va->va_start = get_paddr(size, PAGE_SIZE, vstart, vend);
	if (unlikely(!va->va_start)) {
		prmem_err(PRMEM_NO_RANGE);
		return false;
	}
	va->va_end = va->va_start + size;

	/* Try to reserve enough physical pages */
	if (unlikely(!get_phys_pages(db, aux_pages, size >> PAGE_SHIFT))) {
		prmem_err(PRMEM_NO_PHYS);
		return false;
	}
	return true;
}

static void insert_vmap_area(struct vmap_area *va)
{
	struct rb_node **p = &vmap_area_root.rb_node;
	struct rb_node *parent = NULL;
	struct rb_node *tmp = NULL;

	while (*p) {
		struct vmap_area *tmp_va = NULL;

		parent = *p;
		tmp_va = rb_entry(parent, struct vmap_area, rb_node);
		if (va->va_start < tmp_va->va_end)
			p = &(*p)->rb_left;
		else if (va->va_end > tmp_va->va_start)
			p = &(*p)->rb_right;
		else
			BUG();
	}

	rb_link_node(&va->rb_node, parent, p);
	rb_insert_color(&va->rb_node, &vmap_area_root);

	/* address-sort this list */
	tmp = rb_prev(&va->rb_node);
	if (tmp) {
		struct vmap_area *prev = NULL;

		prev = rb_entry(tmp, struct vmap_area, rb_node);
		list_add_rcu(&va->list, &prev->list);
	} else {
		list_add_rcu(&va->list, &prmem_vmap_area_list);
	}
}

static void remove_vmap_area(struct vmap_area *va)
{
	BUG_ON(RB_EMPTY_NODE(&va->rb_node));

	rb_erase(&va->rb_node, &vmap_area_root);
	RB_CLEAR_NODE(&va->rb_node);
	list_del_rcu(&va->list);
}

static unsigned long vmap_areas_block __wr;

static struct vmap_area *vmap_areas_cache __wr;

static struct vmap_area *get_vmap_area(struct prmem_phys_db *db,
				       unsigned long size, enum prmem_type type)
{
	struct vmap_area *va = NULL;
	struct vmap_area **pva = NULL;
	struct vmap_area *next = NULL;
	unsigned long vstart;
	unsigned long vend;

	va = vmap_areas_cache;
	if (unlikely(!va)) {
		prmem_err(PRMEM_NO_METADATA);
		return NULL;
	}
	pva = (struct vmap_area **)vmap_areas_cache;
	next = *pva;
	wr_assign(vmap_areas_cache, next);
	memset(va, 0, sizeof(struct vmap_area));
	vstart = g_prmem_ranges[type].start;
	vend = g_prmem_ranges[type].end;

	if (unlikely(!prepare_vmap_area(db, va, size, vstart, vend))) {
		prmem_err(PRMEM_PREPARE_VA_FAIL);
		pva = (struct vmap_area **)va;
		*pva = vmap_areas_cache;
		next = *pva;
		wr_assign(vmap_areas_cache, next);
		return NULL;
	}
	prmem_vmap_page_range(va->va_start, va->va_end, PAGE_KERNEL,
			      aux_pages);
	insert_vmap_area(va);
	return va;
}

static void prmem_put_vmap_area(struct prmem_phys_db *db, struct prmem_node *n)
{
	unsigned long end = n->start + n->size;
	struct vmap_area **pva = NULL;

	flush_cache_vunmap(n->start, end);
	flush_tlb_kernel_range(n->start, end);
	put_phys_pages(db, n);
	prmem_vunmap_page_range(n->start, end);
	/* re-instate the correct values, just in case */
	n->va->va_start = n->start;
	n->va->va_end = end;
	remove_vmap_area(n->va);
	pva = (struct vmap_area **)(n->va);
	*pva = vmap_areas_cache;
	wr_assign(vmap_areas_cache, n->va);
}

/* -------------------------- phys db init ------------------------------- */

/* Reserve a permanet vmap_area for premapping chunks. */
static void __init prepare_aux_va(unsigned long pages_nr)
{
	memset(&aux_va, 0, sizeof(aux_va));
	aux_pages = vmalloc(pages_nr * sizeof(struct page *));
	if (unlikely(!aux_pages))
		BUG();
	aux_va.va_start = get_paddr(PMALLOC_CHUNK_SIZE, PAGE_SIZE,
				    PRMEM_WR_RECL_START,
				    PRMEM_WR_RECL_END);
	if (unlikely(!aux_va.va_start))
		BUG();
	aux_va.va_end = aux_va.va_start + PMALLOC_CHUNK_SIZE;
	insert_vmap_area(&aux_va);
}

static void __init prmem_phys_db_init(struct prmem_phys_db *db)
{
	unsigned int i;

	for (i = 0; i < db->chunks_nr; i++) {
		struct prmem_phys_chunk *chunk;

		chunk = db->chunks + i;
		chunk->pages_per_chunk = db->db_pages_per_chunk;
		if (likely(phys_db_add_chunk(chunk, db->premap))) {
			db->total_free_pages_nr += db->db_pages_per_chunk;
		} else {
			prmem_err(PRMEM_DB_INIT_NO_CHUNK);
			BUG();
		}
	}
}

/*
 * Note: prmem_phys_dbs_init() is invoked *before* __wr data becomes
 * actually wr, so it performs direct memory writes to targets which later
 * on will become write protected.
 *
 * Note: the order of iniitalization is important, because the
 * initalization of the pmalloc physical pages db will need the
 * availability of the page tables physical pages db, so
 * *** DO NOT CHANGE THE ORDER OF INITIALIZATION OF THE PHYS DBs ***
 */
static void __init prmem_phys_dbs_init(void)
{
	prmem_pgtable_pages_phys_db.chunks_nr = PGTABLE_CHUNKS_NR;
	prmem_pgtable_pages_phys_db.db_pages_per_chunk = PGTABLE_PAGES_PER_CHUNK;
	prmem_phys_db_init(&prmem_pgtable_pages_phys_db);

	prmem_pmalloc_phys_db.chunks_nr = PMALLOC_CHUNKS_NR;
	prmem_pmalloc_phys_db.db_pages_per_chunk = PMALLOC_PAGES_PER_CHUNK;
	prmem_phys_db_init(&prmem_pmalloc_phys_db);
}

/* --------------------   runtime allocator --------------------------- */

static enum prmem_protection_states protected_status __ro_after_init;
static enum prmem_states prmem_status __ro_after_init;

static const char *prmem_states_names[] = {
	[PRMEM_NO_HHEE] = "no_hhee",
	[PRMEM_BYPASSED] = "bypassed",
	[PRMEM_ENABLED] = "enabled",
};

const char inline *get_prmem_status(void)
{
	if (prmem_status > PRMEM_ENABLED)
		return "error status";
	return prmem_states_names[prmem_status];
}

bool inline prmem_active(void)
{
	return prmem_status == PRMEM_ENABLED;
}

#define MAX_ALIGN_ORDER (ilog2(sizeof(void *)))

/* Calculation of pool type from struct pool address */
extern char __end_data_ro_no_recl_prmem_pools[];
extern char __end_data_wr_no_recl_prmem_pools[];
extern char __end_data_start_wr_no_recl_prmem_pools[];
extern char __end_data_start_wr_recl_prmem_pools[];
extern char __end_data_wr_recl_prmem_pools[];
extern char __end_data_ro_recl_prmem_pools[];
extern char __end_data_rw_recl_prmem_pools[];

#define pool_range_entry(index, type)					\
	[index] = (uintptr_t)__end_data_##type##_prmem_pools

unsigned long g_prmem_pools_ranges[] = {
	pool_range_entry(PRMEM_RO_NO_RECL, ro_no_recl),
	pool_range_entry(PRMEM_WR_NO_RECL, wr_no_recl),
	pool_range_entry(PRMEM_START_WR_NO_RECL, start_wr_no_recl),
	pool_range_entry(PRMEM_START_WR_RECL, start_wr_recl),
	pool_range_entry(PRMEM_WR_RECL, wr_recl),
	pool_range_entry(PRMEM_RO_RECL, ro_recl),
	pool_range_entry(PRMEM_RW_RECL, rw_recl),
};

static inline void lock_range(int (*fn)(const void *base, size_t size),
			      unsigned long base, size_t size)
{
	if (!prmem_active())
		return;
	set_prmem_ro(base, DIV_ROUND_UP(size, PAGE_SIZE));
	(*fn)((void *)(uintptr_t)base, size);
}

static inline void unlock_range(int (*fn)(const void *base, size_t size),
				unsigned long base, size_t size)
{
	if (!prmem_active())
		return;
	(*fn)((void *)(uintptr_t)base, size);
	set_prmem_rw(base, DIV_ROUND_UP(size, PAGE_SIZE));
}

static int protect_range(const unsigned long start, const unsigned long size)
{
	enum prmem_pool_type type = prmem_get_type((void *)(uintptr_t)start, size);

	switch (type) {
	case PRMEM_POOL_RO_NO_RECL:
		lock_range(hkip_register_ro, start, size);
		break;
	case PRMEM_POOL_START_WR_NO_RECL:
	case PRMEM_POOL_WR_NO_RECL:
		lock_range(hkip_register_rowm, start, size);
		break;
	case PRMEM_POOL_START_WR_RECL:
	case PRMEM_POOL_WR_RECL:
		lock_range(hkip_register_rowm_mod, start, size);
		break;
	case PRMEM_POOL_RO_RECL:
		lock_range(hkip_register_ro_mod, start, size);
		break;
	case PRMEM_POOL_RW_RECL:
		break;
	default:
		prmem_err(PRMEM_POOL_UNKNOWN_TYPE);
		return -PRMEM_POOL_UNKNOWN_TYPE;
	}
	return 0;
}

static void unprotect_range(const unsigned long start, const unsigned long size)
{
	enum prmem_pool_type type = prmem_get_type((void *)(uintptr_t)start, size);

	if (is_pool_type_wr_recl(type))
		unlock_range(hkip_unregister_rowm_mod, start, size);
	else if (type == PRMEM_POOL_RO_RECL)
		unlock_range(hkip_unregister_ro_mod, start, size);
	else
		return;
	memzero_explicit((void *)(uintptr_t)start, size);
}

static inline bool empty(struct prmem_pool *pool)
{
	return unlikely(!pool->pmalloc_list);
}

static inline size_t get_alignment(struct prmem_pool *pool, size_t size,
				   int alignment)
{
	if (alignment < -1)
		return __roundup_pow_of_two(size);
	else if (alignment == -1)
		return pool->alignment;
	else
		return alignment;
}

static inline bool exhausted(struct prmem_pool *pool, size_t size)
{
	/*
	 * Note: the test is done by offset vs size, without accounting for
	 * alignment, because the offset grows downward, in the direction
	 * of the beginning of the page(s) allocated, which is already page
	 * aligned. Page alignment is a superset of any reasonable
	 * alignment that might be required,up to PAGE_SIZE, included.
	 */
	return unlikely(pool->offset < size);
}

static inline bool space_needed(struct prmem_pool *pool, size_t size)
{
	return empty(pool) || exhausted(pool, size);
}

static inline bool can_grow(struct prmem_pool *pool, size_t size)
{
	return ((pool->cap == PRMEM_NO_CAP) ||
		(pool->allocated + size <= pool->cap));
}

/* ----------------------- Support for prmem_node --------------------- */
#define __roundup(a, b) ((((a) + (b) - 1) / (b)) * (b))

#define PRMEM_NODE_SIZE							\
	__roundup(sizeof(struct prmem_node), sizeof(void *))

/* Worst case scenario: each page mapped individually to a node */
#define MAX_NODES_NR							\
	DIV_ROUND_UP(PRMEM_PHYS_MEMORY_SIZE_MB * SZ_1M, PAGE_SIZE)

#define PRMEM_NODES_SIZE						\
	__roundup(MAX_NODES_NR * PRMEM_NODE_SIZE, PAGE_SIZE)

#define VMAP_AREA_SIZE							\
	__roundup(sizeof(struct vmap_area), sizeof(void *))

#define VMAP_AREAS_SIZE							\
	__roundup(MAX_NODES_NR * VMAP_AREA_SIZE, PAGE_SIZE)

static unsigned long prmem_nodes_block __wr;

static inline bool item_in_array(unsigned long start, unsigned long end,
				 unsigned long size, unsigned long item)
{
	return ((start <= item) && (item < end) &&
		(((item - start) % size) == 0));
}

static inline bool is_prmem_node(struct prmem_node *node)
{
	unsigned long start = prmem_nodes_block;
	unsigned long end = (start + MAX_NODES_NR * PRMEM_NODE_SIZE);

	return item_in_array(start, end, PRMEM_NODE_SIZE,
			     (uintptr_t)node);
}

static inline bool is_vmap_area_in_range(struct vmap_area *va)
{
	return item_in_array(vmap_areas_block,
			     vmap_areas_block + VMAP_AREAS_SIZE,
			     VMAP_AREA_SIZE, (uintptr_t)va);
}

static inline bool is_vmap_area_valid(struct vmap_area *va)
{
	return likely(va && is_vmap_area_in_range(va) &&
		      is_prmem_node(va->node) && (va->node->va == va));
}

PRMEM_CACHE(prmem_nodes, PRMEM_CACHE_NO_POOL, sizeof(struct prmem_node),
	    sizeof(void *));

static inline struct prmem_node *get_prmem_node(void)
{
	struct prmem_node *node = NULL;

	node = prmem_cache_alloc(&prmem_nodes, PRMEM_NO_FLAGS);
	if (unlikely(!node)) {
		prmem_err(PRMEM_NO_NODE);
		BUG();
	}
	return node;
}

static inline void put_prmem_node(struct prmem_node *node)
{
	prmem_cache_free(&prmem_nodes, node);
}

static void prmem_activate_node(struct prmem_node *node,
				       struct vmap_area *va,
				       struct prmem_pool *pool,
				       struct prmem_node **list)
{
	DECLARE_WR_PTR_TWIN(node);
	WR_PTR_SYNC_FROM_WR(node);

	va->node = node;
	WR_TWIN(node).pool = pool;
	WR_TWIN(node).va = va;
	WR_TWIN(node).next = (*list);
	WR_TWIN(node).start = va->va_start;
	WR_TWIN(node).size = va->va_end - va->va_start;
	WR_PTR_SYNC_TO_WR(node);
	memset((void *)(uintptr_t)node->start, 0, node->size);
	if (pmalloc_is_start_wr((void *)(uintptr_t)node->start, node->size))
		protect_range(node->start, node->size);
	wr_assign((*list), node);
}

/* ------------------- Protected ranges management -------------------- */

static inline bool is_prmem_node_req(enum prmem_flags flags)
{
	return !!((unsigned int)flags & (PRMEM_NODE | PRMEM_FREEABLE_NODE));
}

static inline bool is_prmem_node_freeable(enum prmem_flags flags)
{
	return !!((unsigned int)flags & PRMEM_FREEABLE_NODE);
}

static struct prmem_node *pool_add_node(struct prmem_pool *pool,
					size_t size,
					enum prmem_flags flags)
{
	struct vmap_area *va = NULL;
	struct prmem_node **list = NULL;
	int type;
	struct prmem_node *node = NULL;

	type = pool_to_prmem_range_type(pool);
	if (type < 0) {
		prmem_err(PRMEM_POOL_UNKNOWN_TYPE);
		return NULL;
	}
	if (is_prmem_node_req(flags)) {
		if (is_prmem_type_recl((enum prmem_type)type) &&
		    is_prmem_node_freeable(flags))
			list = &(pool->recl_list);
		else
			list = &(pool->no_recl_list);
	} else {
		size = (size > pool->refill) ? size : pool->refill;
		list = &(pool->pmalloc_list);
	}
	size = PAGE_ALIGN(size);
	if (unlikely(!can_grow(pool, size))) {
		prmem_err(PRMEM_NO_GROW);
		return NULL;
	}
	node = get_prmem_node();
	if (unlikely(!node)) {
		prmem_err(PRMEM_NO_NODE);
		return NULL;
	}
	va = get_vmap_area(&prmem_pmalloc_phys_db, size, (enum prmem_type)type);
	if (unlikely(!va)) {
		put_prmem_node(node);
		prmem_err(PRMEM_NO_VA);
		return NULL;
	}
	prmem_activate_node(node, va, pool, list);
	if (*list != pool->pmalloc_list)
		goto out;
	if (is_prmem_type_start_wr((enum prmem_type)type)) {
		if (!prmem_pool_pmalloc_protected(pool))
			wr_assign(pool->protection_status, protected_status);
	} else {
		if (prmem_pool_pmalloc_protected(pool))
			wr_assign(pool->protection_status, PMALLOC_UNPROTECTED);
	}
out:
	wr_add(pool->allocated, size);
	if (pool->allocated > pool->peak_allocated)
		wr_assign(pool->peak_allocated, pool->allocated);
	return node;
}

static inline bool is_pool_valid(struct prmem_pool *pool)
{
	unsigned long p = (uintptr_t)pool;
	unsigned long start = (uintptr_t)__start_data_prmem_pools;
	unsigned long end = (uintptr_t)__end_data_prmem_pools;
	unsigned long size = __roundup(sizeof(struct prmem_pool),
				       sizeof(void *));

	return item_in_array(start, end, size, p);
}

static inline bool is_vmap_area_in_list(struct vmap_area *va,
					struct prmem_node *node)
{
	while (node) {
		if (node->va == va)
			return true;
		node = node->next;
	}
	return false;
}

/*
 * prmem_protect_addr() - protect a standalone chunk of pages
 * @addr: the page-aligned address at the beginning of the chunk to protect
 * Returns 0 on success, or -PRMEM_VA_NOT_FOUND / -PRMEM_VA_NOT_VALID
 * Tests that the chunk belongs to the PRMEM range and protects it
 */
int prmem_protect_addr(void *addr)
{
	struct vmap_area *va = NULL;
	struct prmem_node *n = NULL;

	if (unlikely(!addr || (((uintptr_t)addr) & ~PAGE_MASK))) {
		prmem_err(PRMEM_VA_NOT_VALID);
		return -PRMEM_VA_NOT_VALID;
	}

	get_master_lock();
	va = __prmem_find_vmap_area((uintptr_t)addr);
	if (unlikely(!va)) {
		put_master_lock();
		prmem_err(PRMEM_VA_NOT_FOUND);
		return -PRMEM_VA_NOT_FOUND;
	}
	n = va->node;
	demote_master_lock(n->pool);
	protect_range(n->start, n->size);
	put_prmem_obj_lock(n->pool);
	return 0;
}
EXPORT_SYMBOL(prmem_protect_addr);

static void pool_free_node(struct prmem_node **n)
{
	struct prmem_node *node = *n;

	wr_assign(*n, (*n)->next);
	wr_sub(node->pool->allocated, node->size); /*lint !e501*/
	unprotect_range(node->start, node->size);
	prmem_put_vmap_area(&prmem_pmalloc_phys_db, node);
	put_prmem_node(node);
}

int __pfree(const void *addr, const char *filename, const char *func,
	    const int line_nr)
{
	struct vmap_area *va = NULL;
	struct prmem_pool *pool = NULL;
	struct prmem_node *node = NULL;
	struct prmem_node **n = NULL;
	bool found = false;

	if (unlikely(!addr || !filename || !func)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return -PRMEM_INVALID_INPUT_PARAMETERS;
	}

	get_master_lock();
	if (unlikely(!is_pmalloc(addr, 1))) {
		put_master_lock();
		prmem_err(PRMEM_INVALID_ADDRESS);
		return -PRMEM_INVALID_ADDRESS;
	}

	va = __prmem_find_vmap_area((uintptr_t)addr);
	if (unlikely(!va)) {
		put_master_lock();
		prmem_err(PRMEM_VA_NOT_FOUND);
		return -PRMEM_VA_NOT_FOUND;
	}
	node = va->node;
	pool = node->pool;
	demote_master_lock(pool);
	if (unlikely(!pmalloc_is_recl((void *)(uintptr_t)node->start, node->size))) {
		put_prmem_obj_lock(pool);
		prmem_err(PRMEM_VA_NOT_RECLAIMABLE);
		return -PRMEM_VA_NOT_RECLAIMABLE;
	}
	for (n = &pool->recl_list; *n; n = &((*n)->next)) {
		if ((*n)->va == va) {
			pool_free_node(n);
			found = true;
			break;
		}
	}

	put_prmem_obj_lock(pool);
	if (unlikely(!found)) {
		prmem_err(PRMEM_NODE_NOT_FOUND);
		return -PRMEM_NODE_NOT_FOUND;
	}
	return 0;
}
EXPORT_SYMBOL(__pfree);

static inline bool grow(struct prmem_pool *pool, size_t size)
{
	struct prmem_node *node = NULL;

	node = pool_add_node(pool, size, PRMEM_NO_FLAGS);
	if (unlikely(!node))
		return false;
	wr_assign(pool->offset, node->size);
	return true;
}

static inline bool space_available(struct prmem_pool *pool, size_t size,
				   enum prmem_flags flags)
{
	if (space_needed(pool, size))
		return grow(pool, size);
	return true;
}

/*
 * prmem_pool_preload() - pre-allocate memory in a pool
 * @pool: the pool to associate memory to
 * @size: the min size ofthe vmap_area to allocate
 *
 * Returns 0: success, -PRMEM_NO_SPACE otherwise
 */
int prmem_pool_preload(struct prmem_pool *pool, size_t size)
{
	int ret = 0;

	if (unlikely(!pool || !size))
		return -PRMEM_INVALID_INPUT_PARAMETERS;

	get_prmem_obj_lock(pool);
	if (!space_available(pool, size, PRMEM_NO_FLAGS)) {
		prmem_err(PRMEM_NO_SPACE);
		ret = -PRMEM_NO_SPACE;
	}
	put_prmem_obj_lock(pool);
	return ret;
}
EXPORT_SYMBOL(prmem_pool_preload);

/*
 * pmalloc_aligned() - allocate protectable memory from a pool
 * @pool: handle to the pool to be used for memory allocation
 * @size: amount of memory (in bytes) requested
 * @alignment: alignment required.
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Return pointer to memory on success, NULL otherwise
 *
 * Allocates memory from a pool.
 * If needed, the pool will automatically allocate enough memory to
 * either satisfy the request or meet the "refill" parameter received
 * upon creation.
 * New allocation can happen also if the current memory in the pool is
 * already write protected.
 * In case of concurrent allocations, competing users must take care of
 * ensuring serialization of the resource.
 *
 * Special values for the alignemtn required:
 * @alignment == -1 : default value from pool settings
 * @alignemtn < -1 : align to size, rounded up to power of 2
 */
void *pmalloc_aligned(struct prmem_pool *pool, size_t size,
		      int alignment, enum prmem_flags flags)
{
	size_t new_offset;
	void *ret = NULL;

	if (unlikely(!pool || !size)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return NULL;
	}

	alignment = get_alignment(pool, size, alignment);
	get_prmem_obj_lock(pool);
	if ((unsigned int)flags & (PRMEM_NODE | PRMEM_FREEABLE_NODE)) {
		struct prmem_node *node = NULL;

		node = pool_add_node(pool, size, flags);
		if (likely(node))
			ret = (void *)(uintptr_t)node->start;
		else
			prmem_err(PRMEM_NO_NODE);

		goto out;
	}
	if (!space_available(pool, size, flags)) {
		prmem_err(PRMEM_NO_SPACE);
		goto out;
	}
	new_offset = (size_t)round_down(pool->offset - size, alignment);
	wr_assign(pool->offset, new_offset);
	ret = (void *)(uintptr_t)(pool->pmalloc_list->start + new_offset);
out:
	put_prmem_obj_lock(pool);
	return ret;
}
EXPORT_SYMBOL(pmalloc_aligned);

/*
 * pcalloc() - allocates memory for an array of n elements of given size
 * @pool: the pool to use for the allocation
 * @n: the number of elements in the array
 * @size: the minimum amount of memory to allocate for one element
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns on success the required amount of memory, zeroed, NULL on fail
 *
 * The alignment used will be the one defined by the pool
 * The memory is pre-zeroed
 */
void *pcalloc(struct prmem_pool *pool, size_t n, size_t size,
	      enum prmem_flags flags)
{
	size_t element_size;

	element_size = round_up(size, get_alignment(pool, 0, -1));
	return pmalloc_aligned(pool, element_size * n, -1, flags);
}
EXPORT_SYMBOL(pcalloc);

/*
 * prmem_protect_pool() - write-protect the pmalloced memory in the pool
 * @pool: the pool with the pmalloced memory to write-protect
 *
 * Write-protects all the memory areas from the pool which are associated
 * to pmalloc.
 * This does not prevent further allocation of additional memory, that
 * can be initialized and protected.
 * The catch is that whatever spare memory might still be available in the
 * current va is forfaited.
 * This does not apply if the pool is of type "start wr".
 * Successive allocations will use a new va.
 */
void prmem_protect_pool(struct prmem_pool *pool)
{
	struct prmem_node *node = NULL;

	if (unlikely(!pool)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return;
	}

	if (unlikely(is_pool_start_wr(pool))) {
		prmem_err(PRMEM_POOL_IS_START_WR);
		return;
	}
	get_prmem_obj_lock(pool);
	for (node = pool->pmalloc_list; node; node = node->next)
		protect_range(node->start, node->size);
	wr_assign(pool->protection_status, protected_status);
	wr_assign(pool->offset, 0);
	put_prmem_obj_lock(pool);
}
EXPORT_SYMBOL(prmem_protect_pool);

static inline void free_prmem_nodes_list(struct prmem_node **node)
{
	while (*node)
		pool_free_node(node);
}

/*
 * prmem_free_pool() - Releases all the memory associated to a pool
 * @pool: the pool whose memory must be reclaimed
 *
 * All the memory associated to the pool will be freed, if the pool
 * supports it. It has no effect on pools that prevent reclaiming memory.
 */
void prmem_free_pool(struct prmem_pool *pool)
{
	if (unlikely(!pool)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return;
	}

	if (unlikely(!is_pool_recl(pool))) {
		prmem_err(PRMEM_POOL_NOT_RECLAIMABLE);
		return;
	}
	get_prmem_obj_lock(pool);
	free_prmem_nodes_list(&(pool->pmalloc_list));
	free_prmem_nodes_list(&(pool->recl_list));
	free_prmem_nodes_list(&(pool->no_recl_list));
	wr_assign(pool->offset, 0);
	if (!is_pool_type_start_wr(pool_to_prmem_range_type(pool)))
		wr_assign(pool->protection_status, PMALLOC_UNPROTECTED);
	put_prmem_obj_lock(pool);
}
EXPORT_SYMBOL(prmem_free_pool);

char *pstrdup(struct prmem_pool *pool, const char *s,
	      enum prmem_flags flags)
{
	size_t len;
	char *buf = NULL;

	if (unlikely(!pool || !s)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return NULL;
	}

	len = strlen(s) + 1;
	buf = pmalloc(pool, len, flags);
	if (unlikely(!buf))
		return NULL;
	if (pmalloc_is_start_wr(buf, 1))
		wr_memcpy(buf, s, len);
	else
		strncpy(buf, s, len);
	return buf;
}
EXPORT_SYMBOL(pstrdup);

/* ---------------------- Support for object caches ------------------- */

static inline void inc_obj_cache_count(struct prmem_cache *cache)
{
	wr_inc(cache->count);
	if (cache->count > cache->peak_count)
		wr_assign(cache->peak_count, cache->count);
}

static inline void dec_obj_cache_count(struct prmem_cache *cache)
{
	wr_dec(cache->count);
}

/**
 * prmem_cache_alloc() - allocates an object from cache
 * @cache: the cache to allocate the object from
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns the requested object upon success, NULL otherwise
 */
void *prmem_cache_alloc(struct prmem_cache *cache, enum prmem_flags flags)
{
	struct stack_node *node = NULL;

	if (unlikely(!cache)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return NULL;
	}

	get_prmem_obj_lock(cache);
	node = cache->node;
	if (likely(node)) {
		wr_assign(cache->node, cache->node->node);
		dec_obj_cache_count(cache);
		if ((unsigned int)flags & PRMEM_ZERO)
			wr_memset(node, 0, cache->size);
	} else if (cache->pool) {
		node = pmalloc_aligned(cache->pool, cache->size,
				       cache->alignment, flags);
	}
	put_prmem_obj_lock(cache);
	return (void *)node;
}
EXPORT_SYMBOL(prmem_cache_alloc);

static inline void prmem_cache_push(struct prmem_cache *cache, void *obj)
{
	struct stack_node *node = (struct stack_node *)obj;

	if(!cache || !node) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return;
	}

	wr_assign(node->node, cache->node);
	wr_assign(cache->node, node);
	inc_obj_cache_count(cache);
}

/**
 * prmem_cache_free() - returns an object to the object cache
 * @cache: the cache into which the object must be pushed
 * @obj: the object to return
 */
void prmem_cache_free(struct prmem_cache *cache, void *obj)
{
	if (unlikely(!cache || !obj)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return;
	}

	get_prmem_obj_lock(cache);
	prmem_cache_push(cache, obj);
	put_prmem_obj_lock(cache);
}
EXPORT_SYMBOL(prmem_cache_free);

/**
 * prmem_cache_preload() - pre-populates an object cache
 * @cache: the cache to allocate the object from
 * @units: how many objects to push into the cache
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns the number of objects effectively pushed
 */
unsigned int prmem_cache_preload(struct prmem_cache *cache,
				 unsigned int units, enum prmem_flags flags)
{
	unsigned int i = 0;

	if (unlikely(!cache)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return 0;
	}

	get_prmem_obj_lock(cache);
	if (!cache->pool) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		goto out;
	}
	for (i = 0; i < units; i++) {
		void *p = pmalloc_aligned(cache->pool, cache->size,
					  cache->alignment, flags);

		if (unlikely(!p)) {
			prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
			goto out;
		}
		prmem_cache_push(cache, p);
	}
out:
	put_prmem_obj_lock(cache);
	return i;
}
EXPORT_SYMBOL(prmem_cache_preload);

char *pstrdup_cache(struct prmem_cache *cache, const char *s,
		    enum prmem_flags flags)
{
	size_t len;
	char *buf = NULL;

	if (unlikely(!cache || !s)) {
		prmem_err(PRMEM_INVALID_INPUT_PARAMETERS);
		return NULL;
	}

	len = strlen(s) + 1;
	if (unlikely(len > cache->size)) {
		prmem_err(PRMEM_STRING_TOO_LONG);
		return NULL;
	}

	buf = prmem_cache_alloc(cache, flags);
	if (unlikely(!buf))
		return NULL;
	wr_memcpy(buf, s, len); /* A pmalloc object cache is always wr */
	return buf;
}
EXPORT_SYMBOL(pstrdup_cache);

/* -------------------------- Prmem Pgtable -------------------------- */

unsigned long pmalloc_get_page_table_page(void)
{
	struct page *page = NULL;
	void *addr = NULL;

	page = get_phys_page(&prmem_pgtable_pages_phys_db);
	if (unlikely(!page))
		return (unsigned long)NULL;
	addr = page_address(page);
	memset(addr, 0, PAGE_SIZE);
	return (uintptr_t)addr;
}

int pmalloc_put_page_table_page(unsigned long addr)
{
	struct page *p = NULL;

	p = virt_to_page((uintptr_t)addr);
	if (unlikely(p == NULL)) {
		prmem_err(PRMEM_INVALID_ADDRESS);
		return -PRMEM_INVALID_ADDRESS;
	}
	return put_phys_page(&prmem_pgtable_pages_phys_db, p);
}

bool is_pmalloc_page_table_page(unsigned long addr)
{
	struct page *p = NULL;

	p = virt_to_page((uintptr_t)addr);
	if (unlikely(p == NULL))
		return false;
	return is_phys_page(&prmem_pgtable_pages_phys_db, p);
}

/* ------------------------- Hardened Usercopy ------------------------ */

/* ----------------------------- init --------------------------------- */

void __init init_status(void)
{
	if (!hhee_is_present()) {
		prmem_status = PRMEM_NO_HHEE;
		protected_status = PMALLOC_FAKE_NO_HHEE;
	} else {
		prmem_status = PRMEM_ENABLED;
		protected_status = PMALLOC_PROTECTED;
	}
	pr_info("status initializaed to %s", get_prmem_status());
}

static __init int init_vmap_areas_cache(void)
{
	unsigned long bl;
	struct vmap_area *va = NULL;
	struct vmap_area **pva = NULL;
	unsigned int i;

	bl = (unsigned long)(uintptr_t)vmalloc(VMAP_AREAS_SIZE);
	if (unlikely(!bl))
		return -1;
	wr_assign(vmap_areas_block, bl);
	for (i = 0; i < MAX_NODES_NR; i++) {
		pva = (struct vmap_area **)(uintptr_t)(bl + i * VMAP_AREA_SIZE);
		*pva = vmap_areas_cache;
		va = (struct vmap_area *)(uintptr_t)(bl + i * VMAP_AREA_SIZE);
		wr_assign(vmap_areas_cache, va);
	}
	return 0;
}

static __init void init_prmem_nodes_cache(void)
{
	unsigned int i;
	struct vmap_area *va = NULL;
	struct prmem_node *prmem_node = NULL;

	va = get_vmap_area(&prmem_pmalloc_phys_db, PRMEM_NODES_SIZE,
			   PRMEM_START_WR_NO_RECL);
	if (!va) {
		prmem_err(PRMEM_NODES_BATCH_FAIL);
		return;
	}

	protect_range(va->va_start, va->va_end - va->va_start);
	wr_assign(prmem_nodes_block, va->va_start);
	prmem_node = (struct prmem_node *)(uintptr_t)(va->va_start);
	for (i = 0; i < MAX_NODES_NR; i++)
		prmem_cache_push(&prmem_nodes, prmem_node + i);
	pr_info("preloaded %lu prmem_nodes", MAX_NODES_NR);
}

void mark_wr_data_wr(void);
void init_status(void);
void __init prmem_init(void)
{
 /* Do not change order of invocation of any of these functions */
	prepare_aux_va(PMALLOC_PAGES_PER_CHUNK * PMALLOC_CHUNKS_NR);
	prmem_phys_dbs_init();
	init_status();
	mark_wr_data_wr();
	init_vmap_areas_cache();
	init_prmem_nodes_cache();
}
