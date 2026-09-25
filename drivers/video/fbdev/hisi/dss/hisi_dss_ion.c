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
#include "hisi_dss_ion.h"
#include "hisi_fb.h"

/*
 * this function allocate physical memory,
 * and make them to scatter lista.
 * table is global .
 */
 /*lint -e574 -e737 -e570  -e613 -e647*/
static struct iommu_page_info *__hisifb_dma_create_node(void)
{
	/* alloc 8kb each time */
	unsigned int order = 1;
	struct iommu_page_info *info = NULL;
	struct page *page = NULL ;
	info = kzalloc(sizeof(struct iommu_page_info), GFP_KERNEL);
	if (!info) {
		HISI_FB_INFO("kzalloc info failed\n");
		return NULL;
	}
	page = alloc_pages(GFP_KERNEL, order);
	if (!page) {
		HISI_FB_INFO("alloc page error\n");
		kfree(info);
		return NULL;
	}
	info->page = page;
	info->order = order;
	INIT_LIST_HEAD(&info->list);
	return info;
}

static struct sg_table *__hisifb_dma_alloc_memory(unsigned int size)
{
	unsigned int map_size;
	unsigned int sum = 0;
	struct list_head pages;
	struct iommu_page_info *info = NULL;
	struct iommu_page_info *tmp_info = NULL;
	unsigned int i = 0, ret = 0;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;

	if ((size > SZ_512M) || (size == 0)) {
		return NULL;
	}
	map_size = size;

	INIT_LIST_HEAD(&pages);
	do {
		info = __hisifb_dma_create_node();
		if (!info)
			goto error;
		list_add_tail(&info->list, &pages);
		sum += (1 << info->order) * PAGE_SIZE;
		i++;
	} while (sum < map_size);

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table) {
		goto error;
	}

	ret = sg_alloc_table(table, i, GFP_KERNEL);
	if (ret) {
		kfree(table);
		goto error;
	}

	sg = table->sgl;
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		struct page *page = info->page;
		sg_set_page(sg, page, (1 << info->order) * PAGE_SIZE, 0);
		sg = sg_next(sg);
		list_del(&info->list);
		kfree(info);
	}

	HISI_FB_INFO("alloc total memory size 0x%x\n", sum);
	return table;

error:
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		__free_pages(info->page, info->order);
		list_del(&info->list);
		kfree(info);
	}
	return NULL;
}

static int __hisifb_dma_free_memory(struct sg_table *table)
{
	int i;
	struct scatterlist *sg = NULL;
	unsigned int mem_size = 0;
	if (table) {
		for_each_sg(table->sgl, sg, table->nents, i) {
			__free_pages(sg_page(sg), get_order(sg->length));
			mem_size += sg->length;
		}
		sg_free_table(table);
		kfree(table);
	}
	HISI_FB_INFO("free total memory size 0x%x\n", mem_size);
	table = NULL;
	return 0;
}

unsigned long hisifb_alloc_fb_buffer(struct hisi_fb_data_type *hisifd)
{
	size_t buf_len = 0;
	unsigned long buf_addr = 0;
	unsigned long buf_size = 0;
	struct sg_table *sg = NULL;
	struct fb_info *fbi = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return 0;
	}

	fbi = hisifd->fbi;
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL\n");
		return 0;
	}

	buf_len = fbi->fix.smem_len; // align to PAGE_SIZE
	sg = __hisifb_dma_alloc_memory(buf_len);
	if (!sg) {
		HISI_FB_ERR("__hdss_dma_alloc_memory failed!\n");
		return 0;
	}

	buf_addr = hisi_iommu_map_sg(__hdss_get_dev(), sg->sgl, 0, &buf_size);
	if (!buf_addr) {
		HISI_FB_ERR("hisi_iommu_map_sg failed!\n");
		__hisifb_dma_free_memory(sg);
		return 0;
	}
	HISI_FB_INFO("fb%d alloc framebuffer map sg 0x%zxB succuss\n",
		 hisifd->index, buf_size);

	fbi->screen_base = hisifb_iommu_map_kernel(sg, buf_len);
	if (!fbi->screen_base) {
		HISI_FB_ERR("hisifb_iommu_map_kernel failed!\n");
		hisi_iommu_unmap_sg(__hdss_get_dev(), sg->sgl, buf_addr);
		__hisifb_dma_free_memory(sg);
		return 0;
	}

	fbi->fix.smem_start = buf_addr;
	fbi->screen_size = buf_len;
	hisifd->fb_sg_table = sg;

	return buf_addr;
}

void hisifb_free_fb_buffer(struct hisi_fb_data_type *hisifd)
{
	struct fb_info *fbi = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}
	fbi = hisifd->fbi;
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL\n");
		return;
	}

	if ((hisifd->fb_sg_table) && (fbi->screen_base != 0)) {
		hisifb_iommu_unmap_kernel(fbi->screen_base);
		hisi_iommu_unmap_sg(__hdss_get_dev(), hisifd->fb_sg_table->sgl, fbi->fix.smem_start);
		__hisifb_dma_free_memory(hisifd->fb_sg_table);

		hisifd->fb_sg_table = NULL;
		fbi->screen_base = 0;
		fbi->fix.smem_start = 0;
	}
}

void *hisifb_iommu_map_kernel(struct sg_table *sg_table, size_t size)
{
	int i, j;
	void *vaddr = NULL;
	pgprot_t pgprot;
	struct scatterlist *sg = NULL;
	struct sg_table *table = sg_table;
	int npages = PAGE_ALIGN(size) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;

	if (IS_ERR_OR_NULL(pages)) {
		pr_err("%s: vmalloc failed. \n", __func__);
		return NULL;
	}

	if (table == NULL) {
		pr_err("%s: table is NULL \n", __func__);
		vfree(pages);
		return NULL;
	}
	pgprot = pgprot_writecombine(PAGE_KERNEL);

	for_each_sg(table->sgl, sg, table->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		BUG_ON(i >= npages);
		for (j = 0; j < npages_this_entry; j++) {
			*(tmp++) = page++;
		}
	}
	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);
	if (vaddr == NULL) {
		pr_err("%s: vmap failed.\n", __func__);
		return NULL;
	}

	return vaddr;
}

void hisifb_iommu_unmap_kernel(const void *vaddr)
{
	vunmap(vaddr);
}

int hisifb_create_buffer_client(struct hisi_fb_data_type *hisifd)
{
	return 0;
}

void hisifb_destroy_buffer_client(struct hisi_fb_data_type *hisifd)
{
}

int hisi_fb_mmap(struct fb_info *info, struct vm_area_struct * vma)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long remainder = 0;
	unsigned long len = 0;
	unsigned long addr = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	int i = 0;
	int ret = 0;

	if (!info || !vma)
		return -EINVAL;

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (!hisifd || !hisifd->pdev)
		return -EINVAL;

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_INFO("fb%u, no fb buffer!\n", hisifd->index);
		return -EFAULT;
	}

	if (!lock_fb_info(info))
		return -ENODEV;

	if (hisifd->fb_mem_free_flag) {
		if (!hisifb_alloc_fb_buffer(hisifd)) {
			HISI_FB_ERR("fb%d, hisifb_alloc_buffer failed!\n", hisifd->index);
			ret = -ENOMEM;
			goto return_unlock;
		}
		hisifd->fb_mem_free_flag = false;
	}

	table = hisifd->fb_sg_table;
	if (!table) {
		HISI_FB_ERR("fb%d, table is NULL!\n", hisifd->index);
		ret = -EFAULT;
		goto return_unlock;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	addr = vma->vm_start;
	offset = vma->vm_pgoff * PAGE_SIZE;
	size = vma->vm_end - vma->vm_start;

	if (size > info->fix.smem_len) {
		HISI_FB_ERR("fb%d, size=%lu is out of range(%u)!\n",
			hisifd->index, size, info->fix.smem_len);
		ret = -EFAULT;
		goto return_unlock;
	}

	for_each_sg(table->sgl, sg, table->nents, i) {/* lint !e574 */
		page = sg_page(sg);
		remainder = vma->vm_end - addr;
		len = sg->length;

		if (offset >= sg->length) {
			offset -= sg->length;
			continue;
		}

		if (offset) {
			page += offset / PAGE_SIZE;
			len = sg->length - offset;
			offset = 0;
		}

		len = min(len, remainder);
		ret = remap_pfn_range(vma, addr, page_to_pfn(page), len, vma->vm_page_prot);
		if (ret != 0) {
			HISI_FB_ERR("fb%d, failed to remap_pfn_range! ret=%d\n",
				hisifd->index, ret);
			goto return_unlock;
		}

		addr += len;
		if (addr >= vma->vm_end) {
			ret = 0;
			break;
		}
	}

return_unlock:
	unlock_fb_info(info);
	return ret;
}

void hisifb_free_logo_buffer(struct hisi_fb_data_type *hisifd)
{
	uint32_t i;
	struct fb_info *fbi = NULL;
	uint32_t logo_buffer_base_temp = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return;
	}
	fbi = hisifd->fbi;//lint !e838
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL\n");
		return;
	}

	logo_buffer_base_temp = g_logo_buffer_base;
	for (i = 0; i < (g_logo_buffer_size / PAGE_SIZE); i++) {
		free_reserved_page(phys_to_page(logo_buffer_base_temp));
		logo_buffer_base_temp += PAGE_SIZE;
	}
	memblock_free(g_logo_buffer_base, g_logo_buffer_size);

	g_logo_buffer_size = 0;
	g_logo_buffer_base = 0;
}
/*lint +e574 +e737 -e570  -e613 -e647*/
