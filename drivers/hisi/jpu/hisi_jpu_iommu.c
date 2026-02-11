/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2024. All rights reserved.
 * Description: jpeg jpu iommu
 * Author: Huawei Hisilicon
 * Create: 2018
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#include "hisi_jpu.h"
#include "hisi_jpu_iommu.h"
#include "hisi_dss_ion.h"

#define MAX_INPUT_DATA_LEN (8192 * 8192 * 4)

int hisijpu_create_buffer_client(struct hisi_jpu_data_type *hisijd)
{
	return 0;
}

void hisijpu_destroy_buffer_client(struct hisi_jpu_data_type *hisijd)
{

}

int hisijpu_enable_iommu(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->pdev == NULL) {
		HISI_JPU_ERR("pdev is NULL");
		return -EINVAL;
	}

	phys_addr_t ttbr = hisi_domain_get_ttbr(&(hisijd->pdev->dev));

	HISI_JPU_INFO("jpu get iommu (ttbr %pa) success\n", &ttbr);

	dma_set_mask_and_coherent(&(hisijd->pdev->dev), DMA_BIT_MASK(64)); //lint !e598 !e648

	return 0;
}

phys_addr_t hisi_jpu_domain_get_ttbr(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return 0;
	}

	if (hisijd->pdev == NULL) {
		HISI_JPU_ERR("pdev is NULL");
		return 0;
	}

	return hisi_domain_get_ttbr(&(hisijd->pdev->dev));
}

/*
 * this function allocate physical memory,
 * and make them to scatter lista.
 * table is global .
 */
static struct jpu_iommu_page_info *hisi_jpu_dma_create_node(void)
{
	/* alloc 8kb each time */
	unsigned int order = 1;
	struct jpu_iommu_page_info *info = NULL;
	struct page *page = NULL;

	info = kzalloc(sizeof(struct jpu_iommu_page_info), GFP_KERNEL);
	if (info == NULL) {
		HISI_JPU_INFO("kzalloc info failed!\n");
		return NULL;
	}

	/* alloc 8kb each time */
	page = alloc_pages(GFP_KERNEL, order);
	if (page == NULL) {
		HISI_JPU_INFO("alloc page error\n");
		kfree(info);
		return NULL;
	}

	info->page = page;
	info->order = order;
	INIT_LIST_HEAD(&info->list);

	return info;
}

static struct sg_table *__hisi_jpu_dma_alloc_memory(unsigned int size)
{
	unsigned int map_size;
	unsigned int sum = 0;
	struct list_head pages;
	struct jpu_iommu_page_info *info = NULL;
	struct jpu_iommu_page_info *tmp_info = NULL;
	unsigned int i = 0;
	unsigned int ret;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;

	if ((size > SZ_512M) || (size == 0))
		return NULL;

	map_size = size;

	INIT_LIST_HEAD(&pages);
	do {
		info = hisi_jpu_dma_create_node();
		if (info == NULL)
			goto error;

		list_add_tail(&info->list, &pages);
		sum += (1 << info->order) * PAGE_SIZE; //lint !e647
		i++;
	} while (sum < map_size);

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (table == NULL)
		goto error;

	ret = sg_alloc_table(table, i, GFP_KERNEL);
	if (ret != 0) {
		kfree(table);
		goto error;
	}

	sg = table->sgl;
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		struct page *page = info->page;

		sg_set_page(sg, page, (1 << info->order) * PAGE_SIZE, 0); //lint !e647
		sg = sg_next(sg);
		list_del(&info->list);
		kfree(info);
	}

	HISI_JPU_INFO("alloc total memory 0x%x\n", sum);

	return table;
error:
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		__free_pages(info->page, info->order);
		list_del(&info->list);
		kfree(info);
	}
	return NULL;
}

static int __hisi_jpu_dma_free_memory(struct sg_table *table)
{
	int i;
	struct scatterlist *sg = NULL;
	unsigned int mem_size = 0;

	if (table != NULL) {
		for_each_sg(table->sgl, sg, table->nents, i) { //lint !e574
			__free_pages(sg_page(sg), get_order(sg->length));
			mem_size += sg->length;
		}
		sg_free_table(table);
		kfree(table);
	}
	HISI_JPU_INFO("free total memory 0x%x\n", mem_size);
	table = NULL;
	return 0;
}

int hisi_jpu_lb_alloc(struct hisi_jpu_data_type *hisijd)
{
	size_t lb_size;
	unsigned long buf_addr;
	unsigned long buf_size = 0;
	struct sg_table *sg = NULL;

	if (hisijd == NULL) {
		HISI_JPU_ERR("hisijd is NULL");
		return -EINVAL;
	}

	hisijd->lb_sg_table = NULL;

	lb_size = JPU_LB_SIZE; // align to PAGE_SIZE
	sg = __hisi_jpu_dma_alloc_memory(lb_size);
	if (sg == NULL) {
		HISI_JPU_ERR("__hisi_jpu_dma_alloc_memory failed!\n");
		return -ENOMEM;
	}

	buf_addr = hisi_iommu_map_sg(&(hisijd->pdev->dev), sg->sgl, 0, &buf_size);
	if (buf_addr == 0) {
		HISI_JPU_ERR("hisi_iommu_map_sg failed!\n");
		__hisi_jpu_dma_free_memory(sg);
		return -ENOMEM;
	}
	HISI_JPU_INFO("jpu%d alloc lb map sg 0x%zxB succuss\n",
			hisijd->index, buf_size);

	hisijd->lb_buf_base = buf_addr;
	hisijd->lb_addr = (uint32_t)(buf_addr >> 4);

	/* start address for line buffer, unit is 16 byte,
	 * must align to 128 byte
	 */
	if (hisijd->lb_addr & (JPU_LB_ADDR_ALIGN - 1)) {
		HISI_JPU_ERR("lb_addr 0x%x is not %d bytes aligned!\n",
				hisijd->lb_addr, JPU_LB_ADDR_ALIGN - 1);

		hisi_iommu_unmap_sg(&(hisijd->pdev->dev), sg->sgl, buf_addr);
		__hisi_jpu_dma_free_memory(sg);

		return -EINVAL;
	}
	hisijd->lb_sg_table = sg;

	HISI_JPU_INFO("lb_size = %zu, hisijd->lb_addr 0x%x\n", lb_size,
			hisijd->lb_addr);
	return 0;
}

void hisi_jpu_lb_free(struct hisi_jpu_data_type *hisijd)
{
	bool cond = false;

	if ((hisijd == NULL) || (hisijd->pdev == NULL)) {
		HISI_JPU_ERR("hisijd or hisijd->pdev is NULL!\n");
		return;
	}
	cond = ((hisijd->lb_addr != 0) && (hisijd->lb_sg_table != NULL));
	if (cond) {
		hisi_iommu_unmap_sg(&(hisijd->pdev->dev),
					hisijd->lb_sg_table->sgl,
					hisijd->lb_buf_base);

		__hisi_jpu_dma_free_memory(hisijd->lb_sg_table);

		hisijd->lb_buf_base = 0;
		hisijd->lb_addr = 0;
		hisijd->lb_sg_table = NULL;
	}
}

static bool hisi_jpu_check_buffer_validate(int fd)
{
	struct dma_buf *buf = NULL;

	// dim layer share fd -1
	if (fd < 0)
		return false;

	buf = dma_buf_get(fd);
	if (IS_ERR(buf)) {
		HISI_JPU_ERR("Invalid file fd %d", fd);
		return false;
	}

	dma_buf_put(buf);
	return true;
}

/*lint -e559 -e613*/
int hisi_jpu_check_inbuff_addr(struct hisi_jpu_data_type *hisijd,
				struct jpu_data_t *jpu_req)
{
	unsigned long buf_size = 0;
	uint64_t inbuffer_addr;
	struct dma_buf *buf = NULL;
	bool cond = (hisijd == NULL || (hisijd->pdev == NULL));

	if (cond) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req == NULL) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (!hisi_jpu_check_buffer_validate(jpu_req->in_sharefd)) {
		HISI_JPU_ERR("Invalid file fd %d\n", jpu_req->in_sharefd);
		return -EINVAL;
	}

	buf = dma_buf_get(jpu_req->in_sharefd);
	if (IS_ERR(buf)) {
		HISI_JPU_ERR("Invalid file shared_fd %d", jpu_req->in_sharefd);
		return -EINVAL;
	}

	inbuffer_addr = hisi_iommu_map_dmabuf(&(hisijd->pdev->dev),
						buf, 0, &buf_size);
	if (inbuffer_addr == 0) {
		HISI_JPU_ERR("get iova_size 0x%x failed\n", buf_size);
		return -EFAULT;
	}

	HISI_JPU_DEBUG("get iova success iova_size 0x%x\n", buf_size);

	jpu_req->start_addr = jpu_req->start_addr + inbuffer_addr;
	jpu_req->end_addr = jpu_req->end_addr + inbuffer_addr;

	if (jpu_req->end_addr <= jpu_req->start_addr) {
		(void)hisi_iommu_unmap_dmabuf(&(hisijd->pdev->dev), buf, inbuffer_addr);
		dma_buf_put(buf);

		HISI_JPU_ERR("end_addr invalid!\n");
		return -EINVAL;
	}

	cond = ((jpu_req->addr_offset > jpu_req->start_addr) ||
			(jpu_req->addr_offset > MAX_INPUT_DATA_LEN));
	if (cond) {
		(void)hisi_iommu_unmap_dmabuf(&(hisijd->pdev->dev), buf, inbuffer_addr);
		dma_buf_put(buf);

		HISI_JPU_ERR("addr offset invalid!\n");
		return -EINVAL;
	}

	dma_buf_put(buf);
	return 0;
}

int hisi_jpu_check_outbuff_addr(struct hisi_jpu_data_type *hisijd,
					struct jpu_data_t *jpu_req)
{
	unsigned long buf_size = 0;
	uint64_t outbuffer_addr;
	struct dma_buf *buf = NULL;
	bool cond = (hisijd == NULL || (hisijd->pdev == NULL));

	if (cond) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req == NULL) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (!hisi_jpu_check_buffer_validate(jpu_req->out_sharefd)) {
		HISI_JPU_ERR("Invalid file fd %d\n", jpu_req->out_sharefd);
		return -EINVAL;
	}

	buf = dma_buf_get(jpu_req->out_sharefd);
	if (IS_ERR(buf)) {
		HISI_JPU_ERR("Invalid file shared_fd %d",
				jpu_req->out_sharefd);
		return -EINVAL;
	}

	outbuffer_addr = hisi_iommu_map_dmabuf(&(hisijd->pdev->dev),
						buf, 0, &buf_size);
	if (outbuffer_addr == 0) {
		HISI_JPU_ERR("get iova_size 0x%x failed\n", buf_size);
		return -EFAULT;
	}
	HISI_JPU_INFO("get iova success iova_size 0x%x\n", buf_size);

	jpu_req->start_addr_y = outbuffer_addr / 16;

	cond =  (jpu_req->out_color_format >= HISI_JPEG_DECODE_OUT_RGBA4444) &&
			(jpu_req->out_color_format <= HISI_JPEG_DECODE_OUT_BGRA8888);
	if (cond) {
		jpu_req->last_page_y += jpu_req->start_addr_y / JPU_BLOCK_BUF_MAX_SIZE;
	} else {
		if (jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV400)
			jpu_req->last_page_y += jpu_req->start_addr_y / JPU_BLOCK_BUF_MAX_SIZE;

		cond = (jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV420) ||
			(jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV444) ||
			(jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV422_H1V2) ||
			(jpu_req->out_color_format == HISI_JPEG_DECODE_OUT_YUV422_H2V1);
		if (cond) {
			jpu_req->last_page_y += jpu_req->start_addr_y / JPU_BLOCK_BUF_MAX_SIZE;
			jpu_req->start_addr_c += jpu_req->start_addr_y;
			jpu_req->last_page_c += jpu_req->start_addr_y / JPU_BLOCK_BUF_MAX_SIZE;

			HISI_JPU_INFO("outbuf: stride_y 0x%x, stride_c 0x%x\n",
					jpu_req->stride_y, jpu_req->stride_c);
		}
	}

	/* start_addr unit is 16 byte, page unit is 32KB,
	 * so start_addr need to divide 2048
	 */
	cond = (jpu_req->last_page_y < (jpu_req->start_addr_y / JPU_BLOCK_BUF_MAX_SIZE)) ||
		(jpu_req->last_page_c < (jpu_req->start_addr_c / JPU_BLOCK_BUF_MAX_SIZE));
	if (cond) {
		HISI_JPU_ERR("last_page_y invalid!\n");
		(void)hisi_iommu_unmap_dmabuf(&(hisijd->pdev->dev), buf, outbuffer_addr);
		dma_buf_put(buf);
		return -EINVAL;
	}

	dma_buf_put(buf);
	return 0;
}

/*lint +e559 +e613*/

#pragma GCC diagnostic pop
