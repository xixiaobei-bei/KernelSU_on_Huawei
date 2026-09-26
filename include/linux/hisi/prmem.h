/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: HKIP prmem support
 * Date: 2020/04/15
 */

#ifndef _LINUX_PRMEM_H
#define _LINUX_PRMEM_H

#include <linux/string.h>
#include <linux/hisi/hhee_prmem.h>
#include <linux/mutex.h>
#include <linux/log2.h>
#include <linux/stringify.h>
#include <linux/hisi/prmem_defs.h>
#include <asm/sections.h>
#include <linux/mmzone.h>
#include <linux/spinlock_types.h>

/* ---------------------------- Data Types ---------------------------- */
/**
 * enum prmem_states - operating modes of prmem
 * @PRMEM_NO_HHEE: hhee disabled at boot
 * @PRMEM_BYPASS: prmem is compiled in bypass mode
 * @PRMEM_ENABLED: prmem is fully active
 */
enum prmem_states {
	PRMEM_NO_HHEE,
	PRMEM_BYPASSED,
	PRMEM_ENABLED
};

/**
 * enum prmem_protection_states - possible protections states for a pool
 * @PMALLOC_UNPROTECTED: pmalloc list has at least some unprotected momory
 * @PMALLOC_FAKE_NO_HHEE: pmalloc list would be protected, but hhee is off
 * @PMALLOC_FAKE_BYPASSED: pmalloc list would be protected, but in bypass
 * @PMALLOC_PROTECTED: pmalloc list is protected
 */
enum prmem_protection_states {
	PMALLOC_UNPROTECTED,
	PMALLOC_FAKE_NO_HHEE,
	PMALLOC_FAKE_BYPASSED,
	PMALLOC_PROTECTED,
};

/**
 * struct prmem_range - description of one of prmem protected subranges
 * @start: the address at which the associated range starts
 * @end: the address at which the associated range ends
 * @range_name: string (max 31 characters) associated with the range
 */
struct prmem_range {
	unsigned long start;
	unsigned long end;
	char range_name[32];
};

/**
 * struct prmem_lock - data type used for locking in prmem
 * @irq_flags: variable used to hold irqs masked when holding the spinlock
 * @spinlock: spinlock part of the prmem_lock
 * @busy: the actualy status-tracking variable
 */
struct prmem_lock {
	unsigned long irq_flags;
	spinlock_t spinlock;
};

/**
 * enum prmem_type - selects sub-ranges of VMALLOC addresses
 * @PRMEM_RO_NO_RECL: R/O allocations, non reclaimable
 * @PRMEM_WR_NO_RECL: W/R allocations, non reclaimable
 * @PRMEM_START_WR_NO_RECL: pre-protected W/R allocations, non reclaimable
 * @PRMEM_START_WR_RECL: pre-protected W/R allocations, reclaimable
 * @PRMEM_WR_RECL: W/R allocations, reclaimable
 * @PRMEM_RO_RECL: R/O allocations, reclaimable
 * @PRMEM_LAST_PROT_REGION: index of the last write-protected range
 * @PRMEM_RW_RECL: R/W allocations, reclaimable
 * @PRMEM_LAST_REGION: index of the last PMALLOC range
 * @PRMEM_NO_RECL: combined range of non reclaimable ranges
 * @PRMEM_RECL: combined range of reclaimable ranges
 * @PRMEM_WR: combined range of W/R ranges
 * @PRMEM_START_WR: combined range of start W/R ranges
 * @PRMEM_FULL: combined range of protected ranges
 * @PRMEM_INDEX_NUM: number of usable indexes
 *
 * The first 6 indexes refer to 6 memory areas used for allocating
 * protected memory
 * WARNING: this *MUST* be kept in sync with "prmem_ranges"
 */
enum prmem_type {
	PRMEM_RO_NO_RECL,
	PRMEM_WR_NO_RECL,
	PRMEM_START_WR_NO_RECL,
	PRMEM_START_WR_RECL,
	PRMEM_WR_RECL,
	PRMEM_RO_RECL,
	PRMEM_LAST_PROTECTED_REGION = PRMEM_RO_RECL,
	PRMEM_RW_RECL,
	PRMEM_LAST_REGION = PRMEM_RW_RECL,
	/* The following indexes refer to unions of the above. */
	PRMEM_NO_RECL,
	PRMEM_RECL,
	PRMEM_WR,
	PRMEM_START_WR,
	PRMEM_FULL,
	PRMEM_INDEX_NUM,
};

/**
 * enum prmem_pool_type - select type of allocation associated to a pool
 * @prmem_pool_ro_no_recl: non-rewritable, non-reclaimable
 * @prmem_pool_wr_no_recl: rewritable, non-reclaimable
 * @prmem_pool_start_wr_no_recl: start protected, rewritable, non-recl
 * @prmem_pool_start_wr_recl: start protected, rewritable, reclaimable
 * @prmem_pool_wr_recl: rewritable, reclaimable
 * @prmem_pool_ro_recl: non-rewritable, reclaimable
 * @prmem_pool_rw_recl: read-write, reclaimable
 * @prmem_pool_type_num: cap value, useful for iterating over the range
 *
 * These enums define in which segment to allocate a specific instance of
 * a pool structure. Depending on the segment chosen, certain properties
 * will be atuomatically associated to the memory allocated in relation to
 * the specific pool structure.
 * The initialization ensures that the enum indexes are matching the values
 * from the corresponding indexes declared in the enum prmem_type, in
 * the file prmem_vmalloc.h
 */
enum prmem_pool_type {
	PRMEM_POOL_RO_NO_RECL = PRMEM_RO_NO_RECL,
	PRMEM_POOL_WR_NO_RECL = PRMEM_WR_NO_RECL,
	PRMEM_POOL_START_WR_NO_RECL = PRMEM_START_WR_NO_RECL,
	PRMEM_POOL_START_WR_RECL = PRMEM_START_WR_RECL,
	PRMEM_POOL_WR_RECL = PRMEM_WR_RECL,
	PRMEM_POOL_RO_RECL = PRMEM_RO_RECL,
	PRMEM_POOL_RW_RECL = PRMEM_RW_RECL,
	PRMEM_POOL_TYPE_NUM,
};

/**
 * struct prmem_node - hardened node for tracking pvamap allocations
 * @pool: pointer to the pool owning the node
 * @next: pointer to the next hardened node
 * @start: address of the beginning of the area
 * @size: size of the area
 * @va: pointer to the va the node is tracking
 */
struct prmem_node {
	struct prmem_pool *pool;
	struct prmem_node *next;
	size_t start;
	size_t size;
	struct vmap_area *va;
};

/**
 * struct prmem_pool - Object tracking allocations
 * @name: pointer to a string with the name of the pool
 * @pmalloc_list: pointer to the first node of a list of va allocations
 * @recl_list: list of reclaimable va in the pool (on-mode)
 * @no_recl_list: list of non-reclaimable va in the pool (on-mode)
 * @offset: amount of free memory available from the current va
 * @alignment: the minimum alignment required for allocations from this pool
 * @refill: the minimum amount of memory to allocate in a new vmap_area
 * @cap: maximum amount of memory that can be allocated - 0 disables it
 * @allocated: memory currently associated with the pool
 * @lock: pointer to spinlock used to serialize access to the pool
 * @protection_status: protection status of the pmalloc memory
 * @sysfs_data: pointer to associated sysfs data structure
 * @pool_type: classification of the pool for prmem-off mode
 *
 * The pool defines - implicitly, see PRMEM_POOL() - the properties of
 * the allocations associated with it.
 * Once a pool is created, it can provide only one specific type of
 * memory. This limitation is intended to limit the attack surface against
 * ROP/JOP attempts to hijack a pool in a different way than the intended
 * one.
 * The field @pool_type is used only for prmem-off mode, when address of
 * the pool structure is not enforced by prmem.
 * Instances of this type of structure should be created only through
 * PRMEM_POOL(), rather than directly.
 */
struct prmem_pool {
	const char *name;
	struct prmem_node *pmalloc_list;
	struct prmem_node *recl_list;
	struct prmem_node *no_recl_list;
	size_t offset;
	size_t alignment;
	size_t refill;
	size_t cap;
	size_t allocated;
	size_t peak_allocated;
	struct prmem_lock *lock;
	enum prmem_protection_states protection_status;
	struct prmem_pool_sysfs_data *sysfs_data;
} __aligned(sizeof(void *));

/**
 * enum prmem_flags - set of options to use when allocating from prmem
 * @PRMEM_NO_FLAGS: alias for a 0 bitmap
 * @PRMEM_ZERO: memory must be zeroed
 * @PRMEM_NODE: provide a separate new node, freed with the pool
 * @PRMEM_FREEABLE_NODE: independently freeable node (if the pool allows)
 */
enum prmem_flags {
	PRMEM_NO_FLAGS = (1 << 0),
	PRMEM_ZERO = (1 << 1),
	PRMEM_NODE = (1 << 2),
	PRMEM_FREEABLE_NODE = (1 << 3),
};

/**
 * struct stack_node - structure used to stack items in an object cache
 * @node: the pointer to thenext object in the cache
 */
struct stack_node {
	struct stack_node *node;
};

/**
 * struct prmem_cache - cache of objects of wr memory
 * @name: the name of the object cache
 * @node: pointer to the first object in the stack implementing the cache
 * @count: number of objects currently in the cache
 * @pool: optional pool from which to allocate additional memory, as needed
 * @size: the size of one of the objects in the cache
 * @alignment: the minimum alignemnt order required
 * @lock: lock used when pushing/popping objects
 * @sysfs_data: pointer to associated sysfs data structure
 */
struct prmem_cache {
	const char *name;
	struct stack_node *node __aligned(sizeof(void *));
	size_t count;
	size_t peak_count;
	struct prmem_pool *pool __aligned(sizeof(void *));
	u16 size;
	u16 alignment;
	struct prmem_lock *lock;
	struct prmem_cache_sysfs_data *sysfs_data;
} __aligned(sizeof(void *));

#define __PRMEM_LOCK_INITIALIZER(name)					\
{									\
	.spinlock = __SPIN_LOCK_UNLOCKED(name.spinlock),		\
}

#define PRMEM_LOCK(name)						\
	struct prmem_lock name = __PRMEM_LOCK_INITIALIZER(name)

/* -------------------------- phys data base -------------------------- */

#define PMALLOC_PAGES_PER_CHUNK 512

#define PMALLOC_CHUNK_SIZE						\
	(PMALLOC_PAGES_PER_CHUNK * PAGE_SIZE)

#define PRMEM_MAP_ROW_BITS						\
	(8 * sizeof(unsigned long))

#define PRMEM_MAP_SIZE							\
	DIV_ROUND_UP(PMALLOC_PAGES_PER_CHUNK, PRMEM_MAP_ROW_BITS)

#define PRMEM_PHYS_MAX_CHUNKS_NR					\
	DIV_ROUND_UP(MB_UL(PRMEM_PHYS_MEMORY_SIZE_MB), PMALLOC_CHUNK_SIZE)

/**
 * struct prmem_phys_chunk - chunk of physical contiguous memory pages
 * @base_pfn: the first pfn of the chunk
 * @free_pages_nr: the amount of physical pages still available
 * @map: bitmap of free/busy pages in the chunk
 */
struct prmem_phys_chunk {
	unsigned long base_pfn;
	unsigned long free_pages_nr;
	unsigned long pages_per_chunk;
	unsigned long map[PRMEM_MAP_SIZE];
};

/**
 * struct prmem_phys_db - collection of chunks of physical contiguous mem
 * @total_free_pages_nr: number of free pages across all the chunks
 * @total_used_pages_nr: number of used pages across all the chunks
 * @peak_total_used_pages_nr: peak total number of used pages
 * @chunks_nr: number of chunks of memory in the db
 * @chunks: array of prmem_phys_chunks data structures - the db
 */
struct prmem_phys_db {
	const char *name;
	bool premap;
	unsigned long total_free_pages_nr;
	unsigned long total_used_pages_nr;
	unsigned long peak_total_used_pages_nr;
	unsigned int chunks_nr;
	unsigned long db_pages_per_chunk;
	struct prmem_cache_sysfs_data *sysfs_data;
	struct prmem_phys_chunk chunks[PRMEM_PHYS_MAX_CHUNKS_NR];
};

#define DO_PREMAP     true
#define DO_NOT_PREMAP false

#define PRMEM_PHYS_DB(_name, _premap)					\
	;								\
	static const char _name##_phys_db_name[] = __stringify(_name);	\
	struct prmem_phys_db _name##_phys_db __wr = {			\
		.name = _name##_phys_db_name,				\
		.premap = _premap,					\
		.total_free_pages_nr = 0,				\
		.total_used_pages_nr = 0,				\
		.peak_total_used_pages_nr = 0,				\
		.chunks_nr = 0,						\
	}


/* ------------------------------ Status ------------------------------ */

const char *get_prmem_status(void);

/**
 * prmem_active() - tells if data will be really write protected
 *
 * Returns true when HW-backed write protection is available
 */
bool prmem_active(void);

/* -------------------------- wr operations --------------------------- */
static inline bool pmalloc_is_rw(const void *start, size_t size);
static inline bool pmalloc_is_start_wr(const void *start, size_t size);
static inline bool is_rw(const void *p, size_t n);
static inline bool is_wr(const void *p, size_t n);

/**
 * wr_memset() - sets n bytes of the destination p to the c value
 * @p: beginning of the memory to write to
 * @c: byte to replicate
 * @n: amount of bytes to copy
 *
 * Returns pointer to the destination
 *
 * The function is forced to be inlined to prevent it from becoming a
 * gadget in code flow attacks.
 */
static __always_inline void *wr_memset(void *p, int c, size_t n)
{
	unsigned long dst = (unsigned long)(uintptr_t)p;
	/* Replicate the LSB of c across the 8 bytes in v */
	uint64_t v = 0x0101010101010101 * (0xFF & (unsigned int)c);

	if (WARN(!p, "Invalid wr memset param"))
		return p;

	if (unlikely(!prmem_active()) || unlikely(is_rw(p, n))) {
		memset(p, c, n);
		return p;
	}
	if (WARN(!is_wr(p, n), "Unsupported memory write"))
		return p;
	if (likely(n == 8) && likely(!(dst & 7)))
		hkip_write_rowm_64(p, v);
	else if (likely((n == 4) && likely(!(dst & 3))))
		hkip_write_rowm_32(p, (uint32_t)v);
	else if (likely((n == 2) && likely(!(dst & 1))))
		hkip_write_rowm_16(p, (uint16_t)v);
	else if (likely(n == 1))
		hkip_write_rowm_8(p, (uint8_t)v);
	else
		hkip_set_rowm(p, c, n);
	return p;
}

/**
 * wr_memcpy() - copies n bytes from q to p
 * @p: beginning of the memory to write to
 * @q: beginning of the memory to read from
 * @n: amount of bytes to copy
 *
 * Returns pointer to the destination
 *
 * The function is forced to be inlined to prevent it from becoming a
 * gadget in code flow attacks.
 */
static __always_inline void *wr_memcpy(void *p, const void *q, size_t n)
{
	unsigned long dst = (unsigned long)(uintptr_t)p;

	if (WARN((!p || !q), "Invalid wr memcpy param"))
		return p;

	if (unlikely(!prmem_active()) || unlikely(is_rw(p, n))) {
		memcpy(p, q, n);
		return p;
	}
	if (WARN(!is_wr(p, n), "Unsupported memory write"))
		return p;
	if (likely(n == 8) && likely(!(dst & 7)))
		hkip_write_rowm_64(p, *(uint64_t *)q);
	else if (likely((n == 4) && likely(!(dst & 3))))
		hkip_write_rowm_32(p, *(uint32_t *)q);
	else if (likely((n == 2) && likely(!(dst & 1))))
		hkip_write_rowm_16(p, *(uint16_t *)q);
	else if (likely(n == 1))
		hkip_write_rowm_8(p, *(uint8_t *)q);
	else
		hkip_write_rowm(p, q, n);
	return p;
}

/**
 * wr_add() - increments a write-rare variable with a given value
 * @dst: the variable to add to
 * @val: the value to add
 *
 * Returns: the updated value of the variable
 */
#define wr_add(dst, val)						\
do {									\
	typeof((dst)) *dst_ptr = &(dst);				\
	typeof(*dst_ptr) tmp = (*dst_ptr) + (typeof(tmp))(val);		\
									\
	wr_memcpy(dst_ptr, &tmp, sizeof(tmp));				\
	*dst_ptr;							\
} while (0)

/**
 * wr_inc() - increase a write-rare variable by one
 * @dst: the variable to decrement
 *
 * Returns: the updated value of the variable
 */
#define wr_inc(dst) wr_add(dst, 1)

/**
 * wr_sub() - decrements a write-rare variable with a given value
 * @dst: the variable to subtract from
 * @val: the value to subtract
 *
 * Returns: the updated value of the variable
 */
#define wr_sub(dst, val) wr_add(dst, -(val))

/**
 * wr_dec() - decrements a write-rare variable by one
 * @dst: the variable to decrement
 *
 * Returns: the updated value of the variable
 */
#define wr_dec(dst) wr_sub(dst, 1)

/**
 * wr_assign() - sets a write-rare variable to a specified value
 * @dst: the variable to set
 * @val: the new value
 *
 * Returns: the updated value of the variable
 */
#define wr_assign(dst, val)						\
do {									\
	typeof((dst)) *dst_ptr = &(dst);				\
	typeof(*dst_ptr) tmp = (typeof(tmp))(val);			\
									\
	wr_memcpy(dst_ptr, &tmp, sizeof(tmp));				\
	*dst_ptr;							\
} while (0)

/**
 * wr_rcu_assign_pointer() - initialize a pointer in rcu mode
 * @p: the rcu pointer - it MUST be aligned to a machine word
 * @v: the new value
 *
 * Returns the value assigned to the rcu pointer.
 *
 * It is provided as macro, to match rcu_assign_pointer()
 * The rcu_assign_pointer() is implemented as equivalent of:
 *
 * smp_mb();
 * WRITE_ONCE();
 */
#define wr_rcu_assign_pointer(p, v)					\
do {									\
	smp_mb();							\
	wr_assign(p, (v));						\
	p;								\
} while (0)

/**
 * DECLARE_WR_TWIN() - declares a replica of a write-rare variable
 * @wr_var: the wr variable to replicate
 */
#define DECLARE_WR_TWIN(wr_var)	typeof(wr_var) twin_##wr_var

/**
 * DECLARE_WR_PTR_TWIN() - replica of a wr variable from its ptr
 * @ptr_wr_var: the pointer to the wr variable to replicate
 */
#define DECLARE_WR_PTR_TWIN(ptr_wr_var)					\
	typeof(*ptr_wr_var) twin_##ptr_wr_var

/**
 * WR_TWIN() - references the twin of a wr variable or pointer to wr
 * @wr_var: the original wr variable or pointer to it
 *
 * Since the twin is always a variable in itself, rather than a pointer,
 * it doesn't matter that the original is a pointer, as long as the twin
 * was created with DECLARE_WR_PTR_TWIN()
 */
#define WR_TWIN(wr_var)	(twin_##wr_var)

/**
 * WR_SYNC_FROM_WR() - write the original to its replica
 * @wr_var: the original wr variable
 */
#define WR_SYNC_FROM_WR(wr_var)	(twin_##wr_var = wr_var)

/**
 * WR_PTR_SYNC_FROM_WR() - write the pointer to the original to its replica
 * @ptr_wr_var: the original wr variable
 */
#define WR_PTR_SYNC_FROM_WR(ptr_wr_var)					\
	(twin_##ptr_wr_var = *ptr_wr_var)

/**
 * WR_SYNC_TO_WR() - write the replica back to the original
 * @wr_var: the original wr variable
 */
#define WR_SYNC_TO_WR(wr_var)						\
	wr_memcpy(&(wr_var), &(twin_##wr_var), sizeof(twin_##wr_var))

/**
 * WR_PTR_SYNC_TO_WR() - write the replica to the pointer to the original
 * @ptr_wr_var: the original wr variable
 */
#define WR_PTR_SYNC_TO_WR(ptr_wr_var)					\
	wr_memcpy(ptr_wr_var, &(twin_##ptr_wr_var),			\
		  sizeof(twin_##ptr_wr_var))

/*
 * ------------------------- Validation Macros ------------------------
 *
 * It can be daunting to debug errors in parameters passed to macros that
 * declare complex data structures. The best way to catch such bugs is to
 * introduce a mechanism which works like ASSERT() and generates a
 * compile-time error, exactly on the spot of the offending parameter.
 * This also has the additional advantage of not introducing any overhead
 * during runtime.
 */

/*
 * VALIDATE() - Generate a compile-time error if the condition is not met
 * @val: the expression to be evaluated - it must resolve at compile-time
 *       and it must not have side effects, if evaluated twice
 * @cond: macro to use for testing @val - it must take exactly 1 parameter
 *
 * Returns @val if the test is successful, otherwise it will halt the build
 */
#define VALIDATE(val, cond) ((val) / (!!cond(val)))

/* true if the parameter is a power of 2 */
#define is_power_of_two(n) (n && ((n & (n - 1)) == 0))

/* Checks if the alignment is a power of 2 */
#define test_pool_alignment(alignment) is_power_of_two(alignment)

#define validate_pool_alignment(alignment)				\
	VALIDATE(alignment, test_pool_alignment)

/*
 * Check if the object can contain a pointer
 * The cache stores objects in the form of a linked list. Each object must
 * be large enough to container a pointer to the next.
 */
#define test_object_size(size) ((size) >= sizeof(void *))

#define validate_object_size(size) VALIDATE(size, test_object_size)

/* -------------------- Runtime Allocator -------------------- */

#define PRMEM_DEFAULT_REFILL PAGE_SIZE
#define PRMEM_DEFAULT_ALIGN_ORD ilog2(ARCH_KMALLOC_MINALIGN)

#define PRMEM_NO_CAP 0

/**
 * PRMEM_POOL() - create an empty protected memory pool
 * @_name: the name of the pool to be created
 * @_type: any of the prmem_pool_type values
 * @_al: alignemnt to use when allocating memory
 * @_refill: min memory for new allocations, roundedup by PAGE_SIZE
 * @_cap: max memory allocated by the pool, roundedup by PAGE_SIZE
 *
 * NOTES:
 * - alignment must be a power of 2, if positive; negative or zero
 * values will default the alignment to ARCH_KMALLOC_MINALIGN
 * - refill_val set to 0 selects PRMEM_DEFAULT_REFILL
 * - cap_val set to PRMEM_NO_CAP disables the memory capping for the pool
 * - union fields used in off-modyye are implicitly initialized by on-mode
 */
#define PRMEM_POOL(_name, _type, _al, _refill, _cap)			\
	;								\
	static PRMEM_LOCK(_name##_pool_lock);				\
	static const char _name##_pool_name[] = __stringify(_name);	\
	struct prmem_pool _name __prmem_pool_type(_type) = {		\
		.name = _name##_pool_name,				\
		.pmalloc_list = NULL,					\
		.recl_list = NULL,					\
		.no_recl_list = NULL,					\
		.offset = 0,						\
		.alignment = (_al) <= 0 ? PRMEM_DEFAULT_ALIGN_ORD :	\
			     validate_pool_alignment(_al),		\
		.refill = (!(_refill)) ? PRMEM_DEFAULT_REFILL :		\
					 round_up((_refill), PAGE_SIZE),\
		.cap = round_up(_cap, PAGE_SIZE),			\
		.allocated = 0,						\
		.peak_allocated = 0,					\
		.lock = &(_name##_pool_lock),				\
		.protection_status = PMALLOC_UNPROTECTED,		\
		.sysfs_data = NULL,					\
	}

int prmem_pool_preload(struct prmem_pool *pool, size_t size);
int prmem_protect_addr(void *addr);

void *pmalloc_aligned(struct prmem_pool *pool, size_t size,
		      int alignment, enum prmem_flags flags);

/**
 * pzalloc_aligned() - allocates and zeroes memory aligned as requested
 * @pool: the pool to use for the allocation
 * @size: the minimum amount of memory to allocate
 * @alignment: minimum alignment for the allocation
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns on success the required amount of memory, zeroed, NULL on fail
 *
 * pzalloc_aligned is an alias for pmalloc_aligned because pmalloc already
 * zeroes the memory provided, by default.
 */
static inline void *pzalloc_aligned(struct prmem_pool *pool, size_t size,
				    int alignment, enum prmem_flags flags)
{
	return pmalloc_aligned(pool, size, alignment, flags);
}

/**
 * pmalloc() - allocates memory as requested
 * @pool: the pool to use for the allocation
 * @size: the minimum amount of memory to allocate
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns on success the required amount of memory, NULL on fail
 *
 * The alignment used will be the default one, taken from the pool setting
 * unless PRMEM_VA is specified as flag. IN that case, the area will be
 * page aligned.
 * The memory is automatically zeroed by the pool, to avoid leaking data.
 */
static inline void *pmalloc(struct prmem_pool *pool, size_t size,
			    enum prmem_flags flags)
{
	return pmalloc_aligned(pool, size, -1, flags);
}

/**
 * pzalloc() - allocates and zeroes memory
 * @pool: the pool to use for the allocation
 * @size: the minimum amount of memory to allocate
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns on success the required amount of memory, zeroed, NULL on fail
 *
 * pzalloc is an alias for pmalloc because pmalloc already zeroes the
 * memory provided, by default.
 *
 * The alignment used will be the default one, taken from the pool setting
 * The memory is automatically zeroed by the pool, to avoid leaking data.
 */
static inline void *pzalloc(struct prmem_pool *pool, size_t size,
			    enum prmem_flags flags)
{
	return pmalloc(pool, size, flags);
}

void *pcalloc(struct prmem_pool *pool, size_t n, size_t size,
	      enum prmem_flags flags);

void prmem_protect_pool(struct prmem_pool *pool);

/**
 * prmem_pool_pmalloc_protected() - are the pmalloc allocations protected?
 * @pool: the pool to test
 *
 * Returns true if all the pmalloc memory is protected, false otherwise
 *
 */
static inline bool prmem_pool_pmalloc_protected(struct prmem_pool *pool)
{
	return pool->protection_status != PMALLOC_UNPROTECTED;
}

void prmem_free_pool(struct prmem_pool *pool);

/**
 * pstrdup() - duplicate a string, using pmalloc as allocator
 * @pool: handler to the pool to be used for memory allocation
 * @s: string to duplicate
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns a pointer to the replica, NULL in case of error.
 *
 * Generates a copy of the given string, allocating sufficient memory
 * from the given pmalloc pool.
 */
char *pstrdup(struct prmem_pool *pool, const char *s,
	      enum prmem_flags flags);

/* ---------------------------- Object Cache -------------------------- */

/* Validates size and alignemnt for a cached object */
#define test_cache_alignment(alignment)					\
	(is_power_of_two(alignment) && test_object_size(alignment))
#define validate_cache_alignment(alignment)				\
	VALIDATE(alignment, test_cache_alignment)

#define PRMEM_CACHE_ALIGN_AS_POOL (-1)

#define PRMEM_CACHE_NO_POOL NULL

/**
 * PRMEM_CACHE() - Declare a cache
 * @_name: name of the cache
 * @_pool: the optional pool to allocate new objects from
 * @_size: the size, in bytes, of the objects to allocate
 * @_al: the alignment, in bytes, of the objects to allocate
 *
 * The macro declares 2 objects:
 * - a private (static) spinlock, referred only by the pointer inside the
 *   cache
 * - the cache itself
 *
 * Because of the semantics, it is not possible to use modifiers with the
 * cache. The lone semicolon at the beginning of the macro is meant to
 * trigger warnings, should any modifier be prepended to PRMEM_CACHE()
 * The warning triggered is:
 * warning: declaration does not declare anything [-Wmissing-declarations]
 *
 * If the cache is pre-loaded explicitly, the pool is optional.
 */
#define PRMEM_CACHE(_name, _pool, _size, _al)				\
	;								\
	static PRMEM_LOCK(_name##_cache_lock);				\
	static const char _name##_cache_name[] = __stringify(_name);	\
	struct prmem_cache _name __prmem_object_cache = {		\
		.name = _name##_cache_name,				\
		.node = NULL,						\
		.count = 0,						\
		.peak_count = 0,					\
		.pool = (_pool),					\
		.size = ((_al) < 0) ? roundup_pow_of_two(_size) :	\
			validate_object_size(round_up(_size, _al)),	\
		.alignment = ((_al) <= 0) ? -1 :			\
			      validate_cache_alignment(_al),		\
		.lock = &(_name##_cache_lock),				\
		.sysfs_data = NULL,					\
	}

void *prmem_cache_alloc(struct prmem_cache *cache, enum prmem_flags flags);

/**
 * prmem_cache_zalloc() - allocate object from the cache, zeroed
 * @cache: the object cache to allocate from
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Returns on success pointer to the object from the cache, NULL otherwise
 */
static inline void *prmem_cache_zalloc(struct prmem_cache *cache,
				       enum prmem_flags flags)
{
	return prmem_cache_alloc(cache, flags | PRMEM_ZERO);
}

void prmem_cache_free(struct prmem_cache *cache, void *obj);

unsigned int prmem_cache_preload(struct prmem_cache *cache, unsigned int units,
				 enum prmem_flags flags);

/**
 * pstrdup_cache() - duplicate a string, using an object cache as allocator
 * @cache: handler to the cache to be used for memory allocation
 * @s: string to duplicate
 * @flags: enum prmem_flags variable affecting the allocation
 *
 * Generates a copy of the given string, allocating sufficient memory
 * from the given prmem pool.
 *
 * Returns a pointer to the replica, NULL in case of error.
 */
char *pstrdup_cache(struct prmem_cache *cache, const char *s,
		    enum prmem_flags flags);

int __pfree(const void *addr, const char *filename, const char *func,
	    const int line_nr);

/**
 * pfree() - free a protected area, by address
 * @addr: the address at the beginning of the area to free
 *
 * Returns 0: success, or a negative error number
 */
#define pfree(addr)	__pfree(addr, __FILE__, __func__, __LINE__)

/* --------------- Link-time Range Validation Helpers ----------------- */

/* Link-time prmem sections */
extern char __start_data_wr[], __end_data_wr[];
extern char __start_data_prmem_pools[], __end_data_prmem_pools[];
extern char __start_data_prmem_object_caches[],
		__end_data_prmem_object_caches[];
extern char __start_data_wr_after_init[], __end_data_wr_after_init[];
extern char __start_data_wr[], __end_data_wr[];
extern char __start_data_rw[], __end_data_rw[];
extern char __start_data_prmem[], __end_data_prmem[];

/**
 * is_static_wr() - is the buffer within static wr memory?
 * @p: beginning of the buffer
 * @n: size of the buffer
 *
 * Returns true if the buffer belongs to static wr memory, false otherwise
 */
static __always_inline bool is_static_wr(const void *p, size_t n)
{
	unsigned long low = (uintptr_t)p;
	unsigned long high = low + n;
	unsigned long start = (uintptr_t)__start_data_wr;
	unsigned long end = (uintptr_t)__end_data_wr;

	return likely(start <= low && high <= end);
}

/**
 * is_static_wr_after_init() - is the buffer within static wr after init?
 * @p: beginning of the buffer
 * @n: size of the buffer
 *
 * Returns true if the buffer is static wr after init, false otherwise
 */
static __always_inline bool is_static_wr_after_init(const void *p, size_t n)
{
	unsigned long low = (uintptr_t)p;
	unsigned long high = low + n;
	unsigned long start = (uintptr_t)__start_data_wr_after_init;
	unsigned long end = (uintptr_t)__end_data_wr_after_init;

	return likely(start <= low && high <= end);
}

static __always_inline bool is_static_rw(const void *p, size_t n)
{
	unsigned long low = (uintptr_t)p;
	unsigned long high = low + n;
	unsigned long start = (uintptr_t)__start_data_rw;
	unsigned long end = (uintptr_t)__end_data_rw;

	return likely(start <= low && high <= end);
}

/* --------------- Run-time Range Validation Helpers ------------------ */

/**
 * is_pool_type_recl() - check if memory from the pool can be reclaimed
 * @type: the pool type
 *
 * Returns true if the memory type is reclaimable, false otherwise
 */
static inline bool is_pool_type_recl(enum prmem_pool_type type)
{
	return (type == PRMEM_POOL_START_WR_RECL ||
		type == PRMEM_POOL_WR_RECL ||
		type == PRMEM_POOL_RO_RECL ||
		type == PRMEM_POOL_RW_RECL);
}

/**
 * is_pool_type_no_recl() - is memory from the pool not reclaimable
 * @type: the pool type
 *
 * Returns false if the memory type is reclaimable, true otherwise
 */
static inline bool is_pool_type_no_recl(enum prmem_pool_type type)
{
	return (type == PRMEM_POOL_START_WR_NO_RECL ||
		type == PRMEM_POOL_WR_NO_RECL ||
		type == PRMEM_POOL_RO_NO_RECL);
}

/**
 * is_pool_type_wr_recl() - is memory from the pool both rw and reclaimable
 * @type: the pool type
 *
 * Returns true if the memory type is both wr and reclaimable, else true
 */
static inline bool is_pool_type_wr_recl(enum prmem_pool_type type)
{
	return (type == PRMEM_POOL_START_WR_RECL ||
		type == PRMEM_POOL_WR_RECL);
}

/**
 * is_pool_type_start_wr() - is memory from the pool pre protected
 * @type: the pool type
 *
 * Returns true if the memory type is pre protected
 */
static inline bool is_pool_type_start_wr(enum prmem_pool_type type)
{
	return (type == PRMEM_POOL_START_WR_NO_RECL ||
		type == PRMEM_POOL_START_WR_RECL);
}

extern unsigned long g_prmem_pools_ranges[];

/**
 * pool_to_prmem_range_type() - Address range for a given pool
 * @pool: the pool for which to identify the vmalloc address range
 *
 * Returns the allocation type associated with the pool.
 *
 * Since some regions can have 0 size and collapse, the first one ending
 * at a certain address is the correct one, and the vector must be scanned
 * from its beginning.
 */
static inline int pool_to_prmem_range_type(struct prmem_pool *pool)
{
	unsigned long p = (uintptr_t)pool;
	unsigned long start = (uintptr_t)__start_data_prmem_pools;
	unsigned long end = (uintptr_t)__end_data_prmem_pools;
	unsigned long i;

	if (unlikely(p < start || end <= p))
		return -EINVAL;

	for (i = 0; i <= PRMEM_LAST_REGION; i++)
		if (p < g_prmem_pools_ranges[i])
			return i;
	return -EINVAL;
}

extern const struct prmem_range g_prmem_ranges[PRMEM_INDEX_NUM];

static bool prmem_in_range(const void *start, unsigned long size,
				  enum prmem_type type)
{
	unsigned long _start = (uintptr_t)start;
	unsigned long end = _start + size;

	if (unlikely(type < 0 || type >= PRMEM_INDEX_NUM))
		return false;

	return ((g_prmem_ranges[type].start <= _start) &&
		(_start < end) && (end <= g_prmem_ranges[type].end));
}

/**
 * is_pool_recl() - is the memory associated to the pool reclaimable?
 * @pool: the pool to analyse
 *
 * Returns true if the memory associated to the pool can be released.
 */
static inline bool is_pool_recl(struct prmem_pool *pool)
{
	enum prmem_pool_type type = pool_to_prmem_range_type(pool);

	return (type == PRMEM_POOL_RO_RECL ||
		type == PRMEM_POOL_WR_RECL ||
		type == PRMEM_POOL_START_WR_RECL);
}

/**
 * is_pool_wr() - is the pool associated to protected rewritable memory?
 * @pool: the pool to analyse
 *
 * Returns true if the memory associated to the pool can be rewritten.
 */
static inline bool is_pool_wr(struct prmem_pool *pool)
{
	enum prmem_pool_type type = pool_to_prmem_range_type(pool);

	return (type == PRMEM_POOL_START_WR_NO_RECL ||
		type == PRMEM_POOL_START_WR_RECL ||
		type == PRMEM_POOL_WR_RECL ||
		type == PRMEM_POOL_WR_NO_RECL);
}

/**
 * is_pool_start_wr() - are pool allocations pre-protected and rewritable?
 * @pool: the pool to analyse
 *
 * Returns true if allocations are pre-protected and can be rewritten.
 */
static inline bool is_pool_start_wr(struct prmem_pool *pool)
{
	enum prmem_pool_type type = pool_to_prmem_range_type(pool);

	return (type == PRMEM_POOL_START_WR_NO_RECL ||
		type == PRMEM_POOL_START_WR_RECL);
}

/**
 * is_prmem_type_recl - true if the memory type is reclaimable
 * @type - one of the possible memory types from enum prmem_types
 *
 * Returns true if the type provided is reclaimable
 */
static inline bool is_prmem_type_recl(enum prmem_type type)
{
	return (type == PRMEM_START_WR_RECL || type == PRMEM_WR_RECL ||
		type == PRMEM_RO_RECL || type == PRMEM_RECL);
}

/**
 * is_prmem_type_start_wr - true if the memory type starts as write-rare
 * @type - one of the possible memory types from enum prmem_types
 *
 * Returns true if the type provided is start_wr
 */
static inline bool is_prmem_type_start_wr(enum prmem_type type)
{
	return (type == PRMEM_START_WR_NO_RECL ||
		type == PRMEM_START_WR_RECL || type == PRMEM_START_WR);
}

/**
 * is_prmem_type_wr - true if the memory type supports write-rare
 * @type - one of the possible memory types from enum prmem_types
 *
 * Returns true if the type provided is write-rare
 */
static inline bool is_prmem_type_wr(enum prmem_type type)
{
	return (type == PRMEM_WR_NO_RECL ||
		type == PRMEM_START_WR_NO_RECL ||
		type == PRMEM_START_WR_RECL || type == PRMEM_WR_RECL ||
		type == PRMEM_WR);
}

static inline int prmem_get_type(void *start, unsigned long size)
{
	int i;

	for (i = 0; i <= PRMEM_LAST_REGION; i++)
		if (prmem_in_range(start, size, i))
			return i;
	return -EINVAL;
}

/**
 * pmalloc_is_recl() - can the memory buffer be freed?
 * @start: address of the buffer
 * @size: dimension of the buffer
 *
 * Returns true if the memory can be released, false otherwise
 */
static inline bool pmalloc_is_recl(const void *start, size_t size)
{
	return prmem_in_range(start, size, PRMEM_RECL);
}

/**
 * prmem_is_wr() - is the memory buffer wr?
 * @start: address of the buffer
 * @size: dimension of the buffer
 *
 * Returns true if the memory is wr, false otherwise
 */
static inline bool prmem_is_wr(const void *start, size_t size)
{
	return prmem_in_range(start, size, PRMEM_WR);
}

/**
 * pmalloc_is_start_wr() - is the buffer wr since its allocation?
 * @start: address of the buffer
 * @size: dimension of the buffer
 *
 * Returns true if the memory was created as wr, false otherwise
 */
static inline bool pmalloc_is_start_wr(const void *start, size_t size)
{
	return prmem_in_range(start, size, PRMEM_START_WR);
}

/**
 * pmalloc_is_rw() - is the memory buffer rw?
 * @start: address of the buffer
 * @size: dimension of the buffer
 *
 * Returns true if the memory is rw, false otherwise
 */
static inline bool pmalloc_is_rw(const void *start, size_t size)
{
	return prmem_in_range(start, size, PRMEM_RW_RECL);
}

/**
 * is_pmalloc() - can the memory buffer be protected?
 * @start: address of the buffer
 * @size: dimension of the buffer
 *
 * Returns true if the memory can be protected, false otherwise
 */
static inline bool is_pmalloc(const void *start, size_t size)
{
	return prmem_in_range(start, size, PRMEM_FULL);
}

/**
 * is_wr() - tests if the buffer is write_rare
 * @p: beginning of the buffer
 * @n: size of the buffer
 *
 * The buffer can be either a statically allocated variable, or it can
 * belong to a protected allocation that is re-writable.
 *
 * Returns truetrue if the buffer is wr, false otherwise
 */
static inline bool is_wr(const void *p, size_t n)
{
	return (prmem_is_wr(p, n) || is_static_wr(p, n) ||
		is_static_wr_after_init(p, n));
}

/**
 * is_rw() - tests if the buffer is in the special read/write area
 * @p: beginning of the buffer
 * @n: size of the buffer
 *
 * The buffer can be either a statically allocated variable, or it can
 * belong to a protected allocation that is a special read/write
 * allocation
 *
 * Returns true/false
 */
static inline bool is_rw(const void *p, size_t n)
{
	return (pmalloc_is_rw(p, n) || is_static_rw(p, n));
}

/* ------------------------- Prmem Pgtable ------------------------ */

unsigned long pmalloc_get_page_table_page(void);
int pmalloc_put_page_table_page(unsigned long tmp);
bool is_pmalloc_page_table_page(unsigned long addr);

/* ----------------------- Hardened Usercopy -------------------------- */

char *check_prmem_object(const void *ptr, unsigned long n);

#endif
