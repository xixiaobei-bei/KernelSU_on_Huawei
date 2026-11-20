/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_CACHE_H
#define __LINUX_CACHE_H

#include <uapi/linux/kernel.h>
#include <asm/cache.h>

#ifndef L1_CACHE_ALIGN
#define L1_CACHE_ALIGN(x) __ALIGN_KERNEL(x, L1_CACHE_BYTES)
#endif

#ifndef SMP_CACHE_BYTES
#define SMP_CACHE_BYTES L1_CACHE_BYTES
#endif

/*
 * __read_mostly is used to keep rarely changing variables out of frequently
 * updated cachelines. If an architecture doesn't support it, ignore the
 * hint.
 */
#ifndef __read_mostly
#define __read_mostly
#endif

/*
 * __ro_after_init is used to mark things that are read-only after init (i.e.
 * after mark_rodata_ro() has been called). These are effectively read-only,
 * but may get written to during init, so can't live in .rodata (via "const").
 */
#ifndef __ro_after_init
#define __ro_after_init __attribute__((__section__(".data..ro_after_init")))
#endif

#ifdef CONFIG_HKIP_PRMEM
/*
 * __wr is used to mark objects that cannot be modified directly from the
 * beginning of the init phase (i.e. after prmem_init() has been called).
 * These objects become effectively read-only, from the perspective of
 * performing a direct write, like a variable assignment.
 * However, they can be altered through a dedicated function.
 * It is intended for those objects which are modified, ao seldomly, that
 * the extra cost from the indirect modification is either negligible or
 * worth paying, for the sake of the protection gained.
 */
#ifndef __wr
#define __wr  __attribute__((__section__(".data..prmem_wr")))
#endif

/*
 * __wr_after_init is used to mark objects that cannot be modified
 * directly after init (i.e. after mark_rodata_ro() has been called).
 * These objects become effectively read-only, from the perspective of
 * performing a direct write, like a variable assignment.
 * However, they can be altered through a dedicated function.
 * It is intended for those objects which are occasionally modified after
 * init, however they are modified so seldomly, that the extra cost from
 * the indirect modification is either negligible or worth paying, for the
 * sake of the protection gained.
 */
#ifndef __wr_after_init
#define __wr_after_init \
		__attribute__((__section__(".data..prmem_wr_after_init")))
#endif

/*
 * __rw is used as temporary storage for data which eventually will be
 * stored either in __wr or in __wr_after_init. During development it can
 * be convenient to formally adhere to prmem interface, while not
 * enforcing the actual write prtection, until all the code is ready to
 * cope with the data not being writable by direct assignment.
 */
#ifndef __rw
#define __rw  __attribute__((__section__(".data..prmem_rw")))
#endif

#ifndef __prmem_pool_type
#define __prmem_pool_type(type)						\
		__attribute__((__section__(".data.." #type "_prmem_pools")))
#define __prmem_object_cache						\
		__attribute__((__section__(".data..prmem_object_caches")))
#endif

#else /* !defined(CONFIG_HKIP_PRMEM) */

#define __wr
#define __wr_after_init
#define __rw
#define __prmem_pool_type(type)
#define __prmem_object_cache

#endif /* !defined(CONFIG_HKIP_PRMEM) */

#ifndef ____cacheline_aligned
#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))
#endif

#ifndef ____cacheline_aligned_in_smp
#ifdef CONFIG_SMP
#define ____cacheline_aligned_in_smp ____cacheline_aligned
#else
#define ____cacheline_aligned_in_smp
#endif /* CONFIG_SMP */
#endif

#ifndef __cacheline_aligned
#define __cacheline_aligned					\
  __attribute__((__aligned__(SMP_CACHE_BYTES),			\
		 __section__(".data..cacheline_aligned")))
#endif /* __cacheline_aligned */

#ifndef __cacheline_aligned_in_smp
#ifdef CONFIG_SMP
#define __cacheline_aligned_in_smp __cacheline_aligned
#else
#define __cacheline_aligned_in_smp
#endif /* CONFIG_SMP */
#endif

/*
 * The maximum alignment needed for some critical structures
 * These could be inter-node cacheline sizes/L3 cacheline
 * size etc.  Define this in asm/cache.h for your arch
 */
#ifndef INTERNODE_CACHE_SHIFT
#define INTERNODE_CACHE_SHIFT L1_CACHE_SHIFT
#endif

#if !defined(____cacheline_internodealigned_in_smp)
#if defined(CONFIG_SMP)
#define ____cacheline_internodealigned_in_smp \
	__attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))
#else
#define ____cacheline_internodealigned_in_smp
#endif
#endif

#ifndef CONFIG_ARCH_HAS_CACHE_LINE_SIZE
#define cache_line_size()	L1_CACHE_BYTES
#endif

#endif /* __LINUX_CACHE_H */
