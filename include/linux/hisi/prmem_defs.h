/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Definitions for memory protection library
 * Date: 2020/04/15
 */

#ifndef _LINUX_PRMEM_DEFS_H
#define _LINUX_PRMEM_DEFS_H

#include <asm/sizes.h>
/*
 * Optimizations on the order of the vmalloc regions:
 * - First the 3 regions which cannot be reclaimed, to have a simple test
 *   on free_vmap_area, thus making contiguous the reclaimable ranges
 * - The 3 zones which are write-rare compatible must be contiguous, to
 *   have one single test encompassing them all.
 *
 *   The resulting layout is:
 *   - READ-ONLY NOT RECLAIMABLE
 *   - WRITE-RARE NOT RECLAIMABLE
 *   - PRE-PROTECTED WRITE-RARE NOT RECLAIMABLE
 *   - PRE-PROTECTED WRITE-RARE RECLAIMABLE
 *   - WRITE-RARE RECLAIMABLE
 *   - READ-ONLY RECLAIMABLE
 *   - READ-WRITE RECLAIMABLE	<---- Not for use in production code!
 *
 *   NB: The purpose of the last region is to have prmem-like code which
 *   doesn't actually write-protect the associated memory.
 *   This can be useful both when in the process of converting non-prmem
 *   code to prmem and if in need to temporarily disable prmem protection
 *   on a specific pool.
 */

#define kB(x) ((x) * 1024UL)
#define MB_UL(x) ((x) * 1024UL * 1024UL)

#ifndef PRMEM_PHYS_MEMORY_SIZE_MB
#define PRMEM_PHYS_MEMORY_SIZE_MB CONFIG_PRMEM_MAX_MEMORY
#endif

#define PRMEM_RO_NO_RECL_SIZE		SZ_128M
#define PRMEM_WR_NO_RECL_SIZE		SZ_128M
#define PRMEM_START_WR_NO_RECL_SIZE	SZ_128M
#define PRMEM_START_WR_RECL_SIZE	SZ_128M
#define PRMEM_WR_RECL_SIZE		SZ_128M
#define PRMEM_RO_RECL_SIZE		SZ_128M
#define PRMEM_RW_RECL_SIZE		SZ_128M

#define PRMEM_SIZE							\
	(PRMEM_RO_NO_RECL_SIZE + PRMEM_WR_NO_RECL_SIZE +		\
	 PRMEM_START_WR_NO_RECL_SIZE + PRMEM_START_WR_RECL_SIZE +	\
	 PRMEM_WR_RECL_SIZE + PRMEM_RO_RECL_SIZE + PRMEM_RW_RECL_SIZE)

/*
 * The whole region reserved for PMALLOC addresses is located after the
 * VMALLOC address space.
 *
 * The 6 basic protectable regions + the regular non protectable one: */
/* 1st region: Read-Only Non-Reclaimable */
#define PRMEM_RO_NO_RECL_START PRMEM_START
#define PRMEM_RO_NO_RECL_END \
	(PRMEM_RO_NO_RECL_START + PRMEM_RO_NO_RECL_SIZE)

/* 2rd region: Write-rare Non-Reclaimable */
#define PRMEM_WR_NO_RECL_START PRMEM_RO_NO_RECL_END
#define PRMEM_WR_NO_RECL_END \
	(PRMEM_WR_NO_RECL_START + PRMEM_WR_NO_RECL_SIZE)

/* 3nd region: Pre-protected Write-rare Non-Reclaimable */
#define PRMEM_START_WR_NO_RECL_START PRMEM_WR_NO_RECL_END
#define PRMEM_START_WR_NO_RECL_END \
	(PRMEM_START_WR_NO_RECL_START + PRMEM_START_WR_NO_RECL_SIZE)

/* 4th region: Pre-protected Write-rare Reclaimable */
#define PRMEM_START_WR_RECL_START PRMEM_START_WR_NO_RECL_END
#define PRMEM_START_WR_RECL_END \
	(PRMEM_START_WR_RECL_START + PRMEM_START_WR_RECL_SIZE)

/* 5th region: Write-rare Reclaimable */
#define PRMEM_WR_RECL_START PRMEM_START_WR_RECL_END
#define PRMEM_WR_RECL_END \
	(PRMEM_WR_RECL_START + PRMEM_WR_RECL_SIZE)

/* 6th region: Read-Only Reclaimable */
#define PRMEM_RO_RECL_START PRMEM_WR_RECL_END
#define PRMEM_RO_RECL_END \
	(PRMEM_RO_RECL_START + PRMEM_RO_RECL_SIZE)

/* 7th region: Read-Write, Reclaimable */
#define PRMEM_RW_RECL_START PRMEM_RO_RECL_END
#define PRMEM_RW_RECL_END  \
	(PRMEM_RW_RECL_START + PRMEM_RW_RECL_SIZE)


/* The 4 combined regions (they partially overlap): */
/*
 * 1st combined region: Reclaimable
 * (PRMEM_START_WR_RECL + PRMEM_WR_RECL +
 *  PRMEM_RO_RECL + PRMEM_RW_RECL)
 */
#define PRMEM_RECL_START PRMEM_START_WR_RECL_START
#define PRMEM_RECL_END PRMEM_RW_RECL_END

/*
 * 2nd combined region: Non Reclaimable
 * (PRMEM_RO_NO_RECL + PRMEM_WR_NO_RECL + PRMEM_START_WR_NO_RECL)
 */
#define PRMEM_NO_RECL_START PRMEM_RO_NO_RECL_START
#define PRMEM_NO_RECL_END PRMEM_START_WR_NO_RECL_END

/*
 * 3rd combined region: Write-Rare
 * (PRMEM_WR_NO_RECL + PRMEM_START_WR_NO_RECL +
 *  PRMEM_START_WR_RECL + PRMEM_WR_RECL)
 */
#define PRMEM_WR_START PRMEM_WR_NO_RECL_START
#define PRMEM_WR_END PRMEM_WR_RECL_END

/*
 * 4th combined region: Start-Write-Rare
 * (PRMEM_START_WR_NO_RECL + PRMEM_START_WR_RECL)
 */
#define PRMEM_START_WR_START PRMEM_START_WR_NO_RECL_START
#define PRMEM_START_WR_END PRMEM_START_WR_RECL_END

/*
 * 5th combined region: all the protectable regions
 * PRMEM_START is defined by arch-specific code
 */

#define PRMEM_FULL_START PRMEM_START
#define PRMEM_FULL_END PRMEM_END

#endif
