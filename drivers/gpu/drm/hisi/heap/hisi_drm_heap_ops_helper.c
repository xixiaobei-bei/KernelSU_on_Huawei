/*
 * hisi_drm_heap_ops_helper.c
 *
 * Provice kmap/kunmap/mmap comment implememnt to heaps
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "hisi_drm_heap_ops_helper.h"

#include <linux/scatterlist.h>

#include "hisi_drm_heaps_defs.h"
#include "hisi_drm.h"

struct mmap_buffer_info {
	unsigned long remainder;
	unsigned long len;
	unsigned long addr;
	unsigned long offset;
	unsigned long size;
};

static int __init_map_buffer_info(struct hisi_drm_gem *hisi_gem,
				   struct vm_area_struct *vma,
				   struct mmap_buffer_info *info)
{
	info->addr = vma->vm_start;
	info->offset = vma->vm_pgoff * PAGE_SIZE;
	info->size = vma->vm_end - vma->vm_start;

	if (info->size > hisi_gem->size) {
		pr_heaps_err("size=%#lx is out of range(%#lx)!\n",
			     info->size, hisi_gem->size);
		return -EINVAL;
	}

	return 0;
}

static int __map_vma(struct hisi_drm_gem *hisi_gem, struct vm_area_struct *vma)
{
	struct sg_table *table = hisi_gem->sgt;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct mmap_buffer_info info;
	unsigned int i;
	int ret;

	ret = __init_map_buffer_info(hisi_gem, vma, &info);
	if (ret) {
		return ret;
	}

	if (!table) {
		pr_heaps_err("table is nullptr!\n");
		return -EINVAL;
	}

	for_each_sg (table->sgl, sg, table->nents, i) {
		page = sg_page(sg);
		info.remainder =
			vma->vm_end - info.addr;
		info.len = sg->length;

		if (info.offset >= sg->length) {
			info.offset -= sg->length;
			continue;
		} else if (info.offset) {
			page += info.offset / PAGE_SIZE;
			info.len =
				sg->length - info.offset;
			info.offset = 0;
		}

		info.len =
			min(info.len, info.remainder);
		ret = remap_pfn_range(vma, info.addr,
				      page_to_pfn(page), info.len,
				      vma->vm_page_prot);
		if (ret != 0) {
			pr_heaps_err("failed to remap_pfn_range! ret=%d\n",
				     ret);
			return ret;
		}

		info.addr += info.len;
		if (info.addr >= vma->vm_end)
			return 0;
	}

	return 0;
}

struct kmap_page_info {
	int npages;
	struct page **pages;
	struct page **tmp;
};

void *__map_kernel(struct kmap_page_info *page_info,
		   struct sg_table *table, pgprot_t pgprot)
{
	struct scatterlist *sg = NULL;
	void *vaddr = NULL;
	unsigned int i, j;

	for_each_sg (table->sgl, sg, table->nents, i) {
		unsigned int npages_this_entry =
			PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		for (j = 0; j < npages_this_entry; j++)
			*(page_info->tmp++) = page++;
	}
	vaddr = vmap(page_info->pages, page_info->npages, VM_MAP, pgprot);
	vfree(page_info->pages);

	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	return vaddr;
}

void *hisi_drm_map_kernel_helper(struct hisi_drm_heap *heap,
				 struct hisi_drm_heap_context *ctx)
{
	void *vaddr = NULL;
	pgprot_t pgprot;
	struct hisi_drm_gem *hisi_gem = NULL;
	struct sg_table *table = NULL;
	struct kmap_page_info page_info;

	if (!ctx || !ctx->priv)
		return ERR_PTR(-EINVAL);

	hisi_gem = ctx->priv;

	table = hisi_gem->sgt;
	page_info.npages = PAGE_ALIGN(hisi_gem->size) >> PAGE_SHIFT;
	page_info.pages = vmalloc(sizeof(struct page *) * page_info.npages);
	if (!page_info.pages)
		return ERR_PTR(-ENOMEM);

	page_info.tmp = page_info.pages;

	if (hisi_gem->flags & HISI_BO_CACHABLE)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	vaddr = __map_kernel(&page_info, table, pgprot);
	if (!IS_ERR(vaddr))
		hisi_gem->vaddr = vaddr;
	else
		pr_heaps_err("failed to map addr into kernel\n");

	return vaddr;
}

void hisi_drm_unmap_kernel_helper(struct hisi_drm_heap *heap,
				  struct hisi_drm_heap_context *ctx,
				  const void *vaddr)
{
	vunmap(vaddr);
}

int hisi_drm_map_user_helper(struct hisi_drm_heap *heap,
			     struct hisi_drm_heap_context *ctx,
			     struct vm_area_struct *vma)
{
	struct hisi_drm_gem *hisi_gem = NULL;
	int ret;
	if (!ctx || !ctx->priv)
		return -EINVAL;

	hisi_gem = ctx->priv;

	if (!vma) {
		pr_heaps_err("vma is nullptr!\n");
		return -EINVAL;
	}

	ret = __map_vma(hisi_gem, vma);
	if (ret) {
		pr_heaps_err("failed to map memory\n");
		return ret;
	}

	return 0;
}
