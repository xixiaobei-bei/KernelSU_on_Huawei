/*
 * fs/proc/hisi/lmkd_dbg_trigger.c
 *
 * Copyright(C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
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

#define pr_fmt(fmt) "hisi_lowmem: " fmt

#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/oom.h>
#include <linux/swap.h>
#include <linux/hisi/lowmem_killer.h>

#define CREATE_TRACE_POINTS
#include "lowmem_trace.h"

#define CMA_TUNE 1

atomic_t swap_no_space;
static int nzones;

int hisi_lowmem_tune(int *other_free, const int *other_file,
		     struct shrink_control *sc)
{
	if (!(sc->gfp_mask & ___GFP_CMA)) {
		int nr_free_cma;
		nr_free_cma = (int)global_zone_page_state(NR_FREE_CMA_PAGES);
		trace_lowmem_tune(nzones, sc->gfp_mask, *other_free,
				  *other_file, -nr_free_cma, 0);
		*other_free -= nr_free_cma;

		return CMA_TUNE;
	}

	return 0;
}


static int __init hisi_lowmem_init(void)
{
	struct zone *zone = NULL;

	for_each_populated_zone(zone)
		nzones++;


	return 0;
}
device_initcall(hisi_lowmem_init);
