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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#include "hisi_dss_iommu.h"
#include "hisi_fb.h"

static struct timeval g_last_map_fail_time;
#define TIME_INTERVAL_FOR_DUMP_MEM_STATUS 10

struct dss_iova_info {
	struct list_head list_node;
	struct dma_buf *dmabuf;
	iova_info_t iova_info;
};

struct platform_device *g_hdss_platform_device;

int hisi_dss_alloc_cma_buffer(size_t size, dma_addr_t *dma_handle, void **cpu_addr)
{
	*cpu_addr = dma_alloc_coherent(__hdss_get_dev(), size, dma_handle, GFP_KERNEL);
	if (!(*cpu_addr)) {
		DPU_FB_ERR("dma alloc coherent failed!\n");
		return -ENOMEM;
	}

	return 0;
}

void hisi_dss_free_cma_buffer(size_t size, dma_addr_t dma_handle, void *cpu_addr)
{
	if ((size > 0) && cpu_addr)
		dma_free_coherent(__hdss_get_dev(), size, cpu_addr, dma_handle);
}

static struct dss_mm_info g_mm_list;
static void hisi_dss_buffer_iova_dump(void)
{
	struct dss_iova_info *node = NULL;
	struct dss_iova_info *_node_ = NULL;
	struct dss_mm_info *mm_list = &g_mm_list;

	spin_lock(&mm_list->map_lock);
	list_for_each_entry_safe(node, _node_, &mm_list->mm_list, list_node)
		DPU_FB_INFO("Display dump: buf(%pK) iova: 0x%llx size: 0x%llx calling_pid: %d\n",
			node->dmabuf, node->iova_info.iova, node->iova_info.size, node->iova_info.calling_pid);

	spin_unlock(&mm_list->map_lock);
}

struct dma_buf *hisi_dss_get_dmabuf(int sharefd)
{
	struct dma_buf *buf = NULL;

	/* dim layer share fd -1 */
	if (sharefd < 0) {
		DPU_FB_ERR("Invalid file sharefd = %d\n", sharefd);
		return NULL;
	}

	buf = dma_buf_get(sharefd);
	if (IS_ERR_OR_NULL(buf)) {
		DPU_FB_ERR("Invalid file buf(%pK), sharefd = %d\n", buf, sharefd);
		return NULL;
	}

	return buf;
}

void hisi_dss_put_dmabuf(struct dma_buf *buf)
{
	if (IS_ERR_OR_NULL(buf)) {
		DPU_FB_ERR("Invalid dmabuf(%pK)\n", buf);
		return;
	}

	dma_buf_put(buf);
}

static void hisi_dump_memory_usage_status(void)
{
	struct timeval curtime;

	dpufb_get_timestamp(&curtime);
	if (curtime.tv_sec - g_last_map_fail_time.tv_sec > TIME_INTERVAL_FOR_DUMP_MEM_STATUS) {
		mm_ion_process_summary_info();
		mm_ion_proecss_info();
		g_last_map_fail_time = curtime;
	}
}

struct dma_buf *hisi_dss_get_buffer_by_sharefd(uint64_t *iova, int fd, uint32_t size)
{
	struct dma_buf *buf = NULL;
	unsigned long buf_size = 0;

	buf = hisi_dss_get_dmabuf(fd);
	if (IS_ERR_OR_NULL(buf)) {
		DPU_FB_ERR("Invalid file shared_fd[%d]\n", fd);
		return NULL;
	}

	*iova = hisi_iommu_map_dmabuf(__hdss_get_dev(), buf, 0, &buf_size);
	if ((*iova == 0) || (buf_size < size)) {
		hisi_dump_memory_usage_status();
		DPU_FB_ERR("get iova_size(0x%llx) smaller than size(0x%x)\n",
			buf_size, size);
		if (*iova != 0) {
			(void)hisi_iommu_unmap_dmabuf(__hdss_get_dev(), buf, *iova);
			*iova = 0;
		}
		hisi_dss_put_dmabuf(buf);
		return NULL;
	}

	return buf;
}

void hisi_dss_put_buffer_by_dmabuf(uint64_t iova, struct dma_buf *dmabuf)
{
	if (IS_ERR_OR_NULL(dmabuf)) {
		DPU_FB_ERR("Invalid dmabuf(%pK)\n", dmabuf);
		return;
	}
	(void)hisi_iommu_unmap_dmabuf(__hdss_get_dev(), dmabuf, iova);

	hisi_dss_put_dmabuf(dmabuf);
}

bool hisi_dss_check_addr_validate(dss_img_t *img)
{
	struct dma_buf *buf = NULL;
	uint64_t iova = 0;

	if (!img) {
		DPU_FB_ERR("img is null\n");
		return false;
	}

	if (img->secure_mode == 1) {
		if (img->vir_addr == 0) {
			DPU_FB_ERR("error buffer iova\n");
			return false;
		}
		return true;
	}

	buf = hisi_dss_get_buffer_by_sharefd(&iova, img->shared_fd, img->buf_size);
	if (IS_ERR_OR_NULL(buf) || (iova == 0)) {
		DPU_FB_ERR("buf or iova is error\n");
		return false;
	}

	if (img->vir_addr != iova) {
		img->vir_addr = iova;
		img->afbc_header_addr = iova + img->afbc_header_offset;
		img->afbc_payload_addr = iova + img->afbc_payload_offset;

		img->hfbc_header_addr0 = iova + img->hfbc_header_offset0;
		img->hfbc_payload_addr0 = iova + img->hfbc_payload_offset0;
		img->hfbc_header_addr1 = iova + img->hfbc_header_offset1;
		img->hfbc_payload_addr1 = iova + img->hfbc_payload_offset1;

		img->hebc_header_addr0 = iova + img->hebc_header_offset0;
		img->hebc_payload_addr0 = iova + img->hebc_payload_offset0;
		img->hebc_header_addr1 = iova + img->hebc_header_offset1;
		img->hebc_payload_addr1 = iova + img->hebc_payload_offset1;
	}

	/* don't unmap iova, dangerous!!!
	 * iova would be unmapped by dmabuf put.
	 */
	hisi_dss_put_dmabuf(buf);

	return true;
}

static bool hisi_dss_check_cld_addr(const dss_layer_t *layer)
{
	uint64_t max_size;
	uint64_t cld_size;

	if (layer->is_cld_layer != 1)
		return true;

	if (layer->cld_data_offset > layer->img.buf_size) {
		DPU_FB_ERR("cld_data_offset exceed img.buf_size\n");
		return false;
	}

	max_size = (uint64_t)(layer->img.buf_size - layer->cld_data_offset);
	cld_size = (uint64_t)(layer->img.stride) * (uint64_t)(layer->cld_height);
	if (cld_size > max_size) {
		DPU_FB_ERR("cld_data_offset exceed max_size\n");
		return false;
	}

	return true;
}

bool hisi_dss_check_layer_addr_validate(dss_layer_t *layer)
{
	if (!layer) {
		DPU_FB_ERR("layer is null\n");
		return false;
	}

	if (!hisi_dss_check_cld_addr(layer))
		return false;

	return hisi_dss_check_addr_validate(&layer->img);
}

int hisi_dss_iommu_enable(struct platform_device *pdev)
{
	struct dss_mm_info *mm_list = &g_mm_list;
	int ret;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}
	hisi_domain_get_ttbr(&pdev->dev);

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));  /*lint !e598*/
	if (ret != 0)
		DPU_FB_ERR("dma set mask and coherent failed\n");

	g_hdss_platform_device = pdev;

	spin_lock_init(&mm_list->map_lock);
	INIT_LIST_HEAD(&mm_list->mm_list);

	return 0;
}

phys_addr_t hisi_dss_domain_get_ttbr(void)
{
	return hisi_domain_get_ttbr(__hdss_get_dev());
}

int hisi_dss_alloc_cmdlist_buffer(struct dpu_fb_data_type *dpufd)
{
	void *cpu_addr = NULL;
	size_t buf_len;
	dma_addr_t dma_handle;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	memset(&dma_handle, 0, sizeof(dma_handle));
	buf_len = dpufd->sum_cmdlist_pool_size;
	cpu_addr = dma_alloc_coherent(__hdss_get_dev(), buf_len, &dma_handle, GFP_KERNEL);
	if (!cpu_addr) {
		DPU_FB_ERR("fb%d dma alloc 0x%zxB coherent failed!\n", dpufd->index, buf_len);
		return -ENOMEM;
	}
	dpufd->cmdlist_pool_vir_addr = cpu_addr;
	dpufd->cmdlist_pool_phy_addr = dma_handle;

	memset(dpufd->cmdlist_pool_vir_addr, 0, buf_len);

	return 0;
}

void hisi_dss_free_cmdlist_buffer(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->cmdlist_pool_vir_addr != 0) {
		dma_free_coherent(__hdss_get_dev(), dpufd->sum_cmdlist_pool_size,
			dpufd->cmdlist_pool_vir_addr, dpufd->cmdlist_pool_phy_addr);
		dpufd->cmdlist_pool_vir_addr = 0;
	}
}

int hisi_dss_buffer_map_iova(struct fb_info *info, void __user *arg)
{
	struct dss_iova_info *node = NULL;
	struct dss_mm_info *mm_list = &g_mm_list;
	iova_info_t map_data;

	if (!arg) {
		DPU_FB_ERR("arg is NULL\n");
		return -EFAULT;
	}
	node = kzalloc(sizeof(struct dss_iova_info), GFP_KERNEL);
	if (!node) {
		DPU_FB_ERR("alloc display meminfo failed\n");
		goto error;
	}

	if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
		DPU_FB_ERR("copy_from_user failed\n");
		goto error;
	}

	node->iova_info.share_fd = map_data.share_fd;
	node->iova_info.calling_pid = map_data.calling_pid;
	node->iova_info.size = map_data.size;
	node->dmabuf = hisi_dss_get_buffer_by_sharefd(&map_data.iova,
		map_data.share_fd, map_data.size);
	if (!node->dmabuf) {
		DPU_FB_ERR("dma buf map share_fd(%d) failed\n", map_data.share_fd);
		goto error;
	}
	node->iova_info.iova = map_data.iova;

	if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
		DPU_FB_ERR("copy_to_user failed\n");
		goto error;
	}

	/* save map list */
	spin_lock(&mm_list->map_lock);
	list_add_tail(&node->list_node, &mm_list->mm_list);
	spin_unlock(&mm_list->map_lock);

	DDTF(g_debug_dump_iova, hisi_dss_buffer_iova_dump);

	return 0;

error:
	if (node) {
		if (node->dmabuf)
			hisi_dss_put_buffer_by_dmabuf(node->iova_info.iova, node->dmabuf);
		kfree(node);
	}
	return -EFAULT;
}

int hisi_dss_buffer_unmap_iova(struct fb_info *info, const void __user *arg)
{
	struct dma_buf *dmabuf = NULL;
	struct dss_iova_info *node = NULL;
	struct dss_iova_info *_node_ = NULL;
	struct dss_mm_info *mm_list = &g_mm_list;
	iova_info_t umap_data;

	if (!arg) {
		DPU_FB_ERR("arg is NULL\n");
		return -EFAULT;
	}

	if (copy_from_user(&umap_data, (void __user *)arg, sizeof(umap_data))) {
		DPU_FB_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	dmabuf = hisi_dss_get_dmabuf(umap_data.share_fd);

	spin_lock(&mm_list->map_lock);
	list_for_each_entry_safe(node, _node_, &mm_list->mm_list, list_node) {
		if (node->dmabuf == dmabuf) {
			list_del(&node->list_node);
			/* already map, need put twice. */
			hisi_dss_put_dmabuf(node->dmabuf);
			/* iova would be unmapped by dmabuf put. */
			kfree(node);
		}
	}
	spin_unlock(&mm_list->map_lock);

	hisi_dss_put_dmabuf(dmabuf);

	DDTF(g_debug_dump_iova, hisi_dss_buffer_iova_dump);

	return 0;
}

#pragma GCC diagnostic pop

