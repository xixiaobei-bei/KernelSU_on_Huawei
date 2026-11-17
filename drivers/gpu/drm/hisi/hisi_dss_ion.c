/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*lint -e446 -e570 -e574 -e647*/

#include "hisi_dss_ion.h"
#include "hisi_defs.h"

#include <drm/drmP.h>

struct iommu_info {
	int npages;
	struct page **pages;
	struct page **tmp;
};

void *hisifb_iommu_map_kernel(
	struct sg_table *sg_table,
	size_t size)
{
	int i;
	int j;
	void *vaddr = NULL;
	pgprot_t pgprot;
	struct scatterlist *sg = NULL;
	struct sg_table *table = NULL;
	struct iommu_info page_info;

	page_info.pages = NULL;
	page_info.tmp = NULL;

	if (!sg_table) {
		HISI_DRM_ERR("sg_table is nullptr!");
		return NULL;
	}

	page_info.npages = PAGE_ALIGN(size) / PAGE_SIZE;
	page_info.pages = vmalloc(sizeof(struct page *) * page_info.npages);
	page_info.tmp = page_info.pages;
	if (!page_info.pages) {
		HISI_DRM_ERR("pages is nullptr!");
		return ERR_PTR(-ENOMEM);
	}

	pgprot = pgprot_writecombine(PAGE_KERNEL);

	table = sg_table;
	for_each_sg(table->sgl, sg, table->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		if (i >= page_info.npages) {
			HISI_DRM_ERR("page[%d] is out of range %d!",
				i, page_info.npages);
			goto err_out;
		}

		for (j = 0; j < npages_this_entry; j++)
			*(page_info.tmp++) = page++;
	}

	vaddr = vmap(page_info.pages, page_info.npages, VM_MAP, pgprot);
	vfree(page_info.pages);

	if (!vaddr) {
		HISI_DRM_ERR("vaddr is nullptr!");
		return ERR_PTR(-ENOMEM);
	}

	return vaddr;

err_out:
	if (page_info.pages) {
		vfree(page_info.pages);
		page_info.pages = NULL;
	}

	return NULL;
}
/*lint +e446 +e570 +e574 +e647*/
