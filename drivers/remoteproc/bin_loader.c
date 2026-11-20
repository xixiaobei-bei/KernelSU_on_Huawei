/*
 * Remote Processor Framework Bin loader
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)    "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/remoteproc.h>
#include <linux/elf.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include "remoteproc_internal.h"

static int rproc_bin_sanity_check(struct rproc *rproc,
	const struct firmware *fw)
{
	return 0;
}

static u32 rproc_bin_get_boot_addr(struct rproc *rproc,
	const struct firmware *fw)
{
	return 0;
}

static int rproc_bin_load_segments(struct rproc *rproc,
	const struct firmware *fw)
{
	if (use_sec_isp())
		return 0;

	return hisp_bin_load_segments(rproc);
}

static struct resource_table *rproc_bin_find_rsc_table(
	struct rproc *rproc, const struct firmware *fw, int *tablesz)
{
	struct resource_table *table = NULL;
	int tablesize;

	table = (struct resource_table *)hisp_get_rsctable(&tablesize);
	if (table == NULL)
		pr_err("[%s] Failed : get_rsctable.%pK\n", __func__, table);

	*tablesz = tablesize;
	return table;
}

static struct resource_table *rproc_bin_find_loaded_rsc_table(
	struct rproc *rproc, const struct firmware *fw)
{
	int tablez;

	return rproc_bin_find_rsc_table(rproc, fw, &tablez);
}

const struct rproc_fw_ops rproc_bin_fw_ops = {
	.load                   = rproc_bin_load_segments,
	.find_rsc_table         = rproc_bin_find_rsc_table,
	.find_loaded_rsc_table  = rproc_bin_find_loaded_rsc_table,
	.sanity_check           = rproc_bin_sanity_check,
	.get_boot_addr          = rproc_bin_get_boot_addr
};
