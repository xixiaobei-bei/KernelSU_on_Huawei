/* Copyright (c) 2018-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
static struct iommu_page_info *__dpufb_dma_create_node(void)
{
	/* alloc 8kb each time */
	unsigned int order = 1;
	struct iommu_page_info *info = NULL;
	struct page *page = NULL;

	info = kzalloc(sizeof(struct iommu_page_info), GFP_KERNEL);
	if (!info) {
		DPU_FB_INFO("kzalloc info failed\n");
		return NULL;
	}
	page = alloc_pages(GFP_KERNEL, order);
	if (!page) {
		DPU_FB_INFO("alloc page error\n");
		kfree(info);
		return NULL;
	}
	info->page = page;
	info->order = order;
	INIT_LIST_HEAD(&info->list);

	return info;
}

static struct sg_table *__dpufb_dma_alloc_memory(unsigned int size)
{
	unsigned int i = 0;
	unsigned int sum = 0;
	struct iommu_page_info *info = NULL;
	struct iommu_page_info *tmp_info = NULL;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct list_head pages;
	unsigned int ret;

	if ((size > SZ_512M) || (size == 0))
		return NULL;

	INIT_LIST_HEAD(&pages);
	do {
		info = __dpufb_dma_create_node();
		if (!info)
			goto error;
		list_add_tail(&info->list, &pages);
		sum += (1 << info->order) * PAGE_SIZE;  /*lint !e647*/
		i++;
	} while (sum < size);

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table)
		goto error;

	ret = sg_alloc_table(table, i, GFP_KERNEL);
	if (ret) {
		kfree(table);
		goto error;
	}

	sg = table->sgl;
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		page = info->page;
		sg_set_page(sg, page, (1 << info->order) * PAGE_SIZE, 0);  /*lint !e647*/
		sg = sg_next(sg);
		list_del(&info->list);
		kfree(info);
	}

	DPU_FB_INFO("alloc total memory size 0x%x\n", sum);
	return table;

error:
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		__free_pages(info->page, info->order);
		list_del(&info->list);
		kfree(info);
	}
	return NULL;
}

static int __dpufb_dma_free_memory(struct sg_table *table)
{
	struct scatterlist *sg = NULL;
	unsigned int mem_size = 0;
	int i;

	if (table) {
		for_each_sg(table->sgl, sg, table->nents, i) {  /*lint !e574*/
			__free_pages(sg_page(sg), get_order(sg->length));
			mem_size += sg->length;
		}
		sg_free_table(table);
		kfree(table);
	}
	DPU_FB_INFO("free total memory size 0x%x\n", mem_size);
	table = NULL;

	return 0;
}

unsigned long dpufb_alloc_fb_buffer(struct dpu_fb_data_type *dpufd)
{
	struct sg_table *sg = NULL;
	struct fb_info *fbi = NULL;
	unsigned long buf_addr;
	unsigned long buf_size;
	size_t buf_len;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return 0;
	}

	fbi = dpufd->fbi;
	if (!fbi) {
		DPU_FB_ERR("fbi is NULL\n");
		return 0;
	}

	buf_len = fbi->fix.smem_len;  /* align to PAGE_SIZE */
	sg = __dpufb_dma_alloc_memory(buf_len);
	if (!sg) {
		DPU_FB_ERR("__hdss_dma_alloc_memory failed!\n");
		return 0;
	}

	buf_addr = hisi_iommu_map_sg(__hdss_get_dev(), sg->sgl, 0, &buf_size);
	if (!buf_addr) {
		DPU_FB_ERR("hisi_iommu_map_sg failed!\n");
		__dpufb_dma_free_memory(sg);
		return 0;
	}
	DPU_FB_INFO("fb%d alloc framebuffer map sg 0x%zxB succuss\n", dpufd->index, buf_size);

	fbi->screen_base = dpufb_iommu_map_kernel(sg, buf_len);
	if (!fbi->screen_base) {
		DPU_FB_ERR("dpufb_iommu_map_kernel failed!\n");
		hisi_iommu_unmap_sg(__hdss_get_dev(), sg->sgl, buf_addr);
		__dpufb_dma_free_memory(sg);
		return 0;
	}

	fbi->fix.smem_start = buf_addr;
	fbi->screen_size = buf_len;
	dpufd->fb_sg_table = sg;

	return buf_addr;
}

void dpufb_free_fb_buffer(struct dpu_fb_data_type *dpufd)
{
	struct fb_info *fbi = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	fbi = dpufd->fbi;
	if (!fbi) {
		DPU_FB_ERR("fbi is NULL\n");
		return;
	}

	if ((dpufd->fb_sg_table) && (fbi->screen_base != 0)) {
		dpufb_iommu_unmap_kernel(fbi->screen_base);
		hisi_iommu_unmap_sg(__hdss_get_dev(), dpufd->fb_sg_table->sgl, fbi->fix.smem_start);
		__dpufb_dma_free_memory(dpufd->fb_sg_table);

		dpufd->fb_sg_table = NULL;
		fbi->screen_base = 0;
		fbi->fix.smem_start = 0;
	}
}

void *dpufb_iommu_map_kernel(struct sg_table *sg_table, size_t size)
{
	void *vaddr = NULL;
	struct scatterlist *sg = NULL;
	struct sg_table *table = sg_table;
	int npages = PAGE_ALIGN(size) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	pgprot_t pgprot;
	int i;
	int j;

	if (IS_ERR_OR_NULL(pages)) {
		pr_err("%s: vmalloc failed.\n", __func__);
		return NULL;
	}

	if (!table) {
		pr_err("%s: table is NULL\n", __func__);
		vfree(pages);
		return NULL;
	}
	pgprot = pgprot_writecombine(PAGE_KERNEL); /*lint !e446*/

	for_each_sg(table->sgl, sg, table->nents, i) {  /*lint !e574*/
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		for (j = 0; j < npages_this_entry; j++)
			*(tmp++) = page++;
	}
	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);
	if (!vaddr) {
		pr_err("%s: vmap failed.\n", __func__);
		return NULL;
	}

	return vaddr;
}

void dpufb_iommu_unmap_kernel(const void *vaddr)
{
	vunmap(vaddr);
}

int dpufb_create_buffer_client(struct dpu_fb_data_type *dpufd)
{
	return 0;
}

void dpufb_destroy_buffer_client(struct dpu_fb_data_type *dpufd)
{
}

static int hisi_fb_mmap_table(struct fb_info *info, struct dpu_fb_data_type *dpufd, struct vm_area_struct *vma)
{
	int i = 0;
	int ret = 0;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long size;
	unsigned long addr;
	unsigned long offset;
	unsigned long len;
	unsigned long remainder;

	table = dpufd->fb_sg_table;
	dpu_check_and_return((!table), -EFAULT, ERR, "fb%d, table is NULL!\n", dpufd->index);

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	addr = vma->vm_start;
	offset = vma->vm_pgoff * PAGE_SIZE;
	size = vma->vm_end - vma->vm_start;

	dpu_check_and_return((size > info->fix.smem_len), -EFAULT, ERR,
		"fb%d, size=%lu is out of range(%u)!\n", dpufd->index, size, info->fix.smem_len);

	for_each_sg(table->sgl, sg, table->nents, i) {  /*lint !e574*/
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
		dpu_check_and_return((ret != 0), ret, ERR, "fb%d, failed to remap_pfn_range! ret=%d\n",
				dpufd->index, ret);

		addr += len;
		if (addr >= vma->vm_end) {
			ret = 0;
			break;
		}
	}

	return ret;
}

int hisi_fb_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int ret;

	if (!info || !vma)
		return -EINVAL;

	dpufd = (struct dpu_fb_data_type *)info->par;
	if (!dpufd || !dpufd->pdev)
		return -EINVAL;

	if (dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_INFO("fb%u, no fb buffer!\n", dpufd->index);
		return -EFAULT;
	}

	if (!lock_fb_info(info))
		return -ENODEV;

	if (dpufd->fb_mem_free_flag) {
		if (!dpufb_alloc_fb_buffer(dpufd)) {
			DPU_FB_ERR("fb%d, dpufb_alloc_buffer failed!\n", dpufd->index);
			ret = -ENOMEM;
			goto return_unlock;
		}
		dpufd->fb_mem_free_flag = false;
	}

	ret = hisi_fb_mmap_table(info, dpufd, vma);

return_unlock:
	unlock_fb_info(info);
	return ret;
}

void dpufb_free_logo_buffer(struct dpu_fb_data_type *dpufd)
{
	struct fb_info *fbi = NULL;
	uint32_t logo_buffer_base_temp;
	uint32_t i;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	fbi = dpufd->fbi;
	if (!fbi) {
		DPU_FB_ERR("fbi is NULL\n");
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

