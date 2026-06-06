/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/swap.h>
#include <asm/uaccess.h>
#include <linux/gfp.h>
#include  <linux/fs.h>
#include  <linux/mm.h>

#include "devdrv_user_common.h"
#include "npu_shm.h"
#include "drv_log.h"
#include "npu_platform.h"
#include "hisi_svm.h"

struct devdrv_mem_desc *g_sq_desc = NULL;

static struct devdrv_mem_info g_shm_desc[NPU_DEV_NUM][DEVDRV_MAX_MEM];

static struct devdrv_continuous_mem g_continuous_mem[NPU_DEV_NUM];

static int devdrv_continuous_mem_init(u8 dev_id)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	void* cpu_addr = NULL;
	struct devdrv_platform_info* plat_info = NULL;
	dma_addr_t dma_handle = 0;
	u64 size = MEM_INIT_SIZE;

	spin_lock_init(&g_continuous_mem[dev_id].cm_spinlock);
	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	g_continuous_mem[dev_id].total_size = 0;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		devdrv_drv_err("npu dev %d `s dev_ctx is null\n", dev_id);
		return -1;
	}
	dev_ctx->cm = &g_continuous_mem[dev_id];

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed.\n");
	}
	else {
		cpu_addr = dma_alloc_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size,
				&dma_handle, GFP_KERNEL);
		if (cpu_addr == NULL)
		{
			devdrv_drv_err("fail to malloc cma mem size 0x%llx\n", size);
		}
		else
		{
			devdrv_drv_warn("succ to malloc cma mem size 0x%llx\n", size);
			spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].dma_handle = dma_handle;
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].cpu_addr = cpu_addr;
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].size = size;
			g_continuous_mem[dev_id].mem_info[SHARE_NUM_OCCUPIED].valid = 0;
			spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);
		}
	}

	return 0;
}

struct devdrv_ts_sq_info *devdrv_calc_sq_info(u8 devid, u32 index)
{
	struct devdrv_ts_sq_info *sq = NULL;
	u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	sq = (struct devdrv_ts_sq_info *)(uintptr_t) (addr +
						      (long)(unsigned)sizeof(struct devdrv_ts_sq_info) * (index));
	return sq;
}

struct devdrv_ts_cq_info *devdrv_calc_cq_info(u8 devid, u32 index)
{
	struct devdrv_ts_cq_info *cq = NULL;
	u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	cq = (struct devdrv_ts_cq_info *)(uintptr_t) (addr +
						      DEVDRV_SQ_INFO_OCCUPY_SIZE +
						      (long)(unsigned)sizeof(struct devdrv_ts_cq_info) * (index));
	return cq;
}

struct devdrv_stream_info *devdrv_calc_stream_info(u8 devid, u32 index)
{
	struct devdrv_stream_info *stream_info = NULL;

	u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
	stream_info = (struct devdrv_stream_info *)(uintptr_t) (addr +
								DEVDRV_SQ_INFO_OCCUPY_SIZE +
								DEVDRV_CQ_INFO_OCCUPY_SIZE +
								(long)(unsigned)sizeof(struct devdrv_stream_info) * (index));
	return stream_info;
}

u32 *devdrv_get_ts_work_status(u8 devid, u32 index)
{
    u32 *ts_status = NULL;

    u64 addr = g_shm_desc[devid][DEVDRV_INFO_MEM].virt_addr;
    ts_status = (u32 *)(uintptr_t) (addr +
                                DEVDRV_SQ_INFO_OCCUPY_SIZE +
                                DEVDRV_CQ_INFO_OCCUPY_SIZE +
                                DEVDRV_STREAM_INFO_OCCUPY_SIZE +
                                (long)(unsigned)sizeof(u32) * (index));
    return ts_status;
}

int devdrv_shm_init(u8 dev_id)
{
	struct devdrv_platform_info *plat_info = NULL;
	gfp_t gfp_flags = GFP_KERNEL | __GFP_COMP | __GFP_ZERO;
	u32 order;
	char *tmp = NULL;
	phys_addr_t doorbell_base;
	u32 doorbell_size;
	struct devdrv_mem_desc *persistent_task_buf_desc = NULL;
    u32 *ts_status = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %d\n", dev_id);
		return -1;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed\n");
		return -1;
	}

	order = DEVDRV_MAX_INFO_ORDER;
	doorbell_base = DEVDRV_PLAT_GET_REG_DESC(plat_info,
						 DEVDRV_REG_TS_DOORBELL).base;
	doorbell_size = DEVDRV_PLAT_GET_REG_DESC(plat_info,
						 DEVDRV_REG_TS_DOORBELL).len;
	persistent_task_buf_desc = DEVDRV_PLAT_GET_PERSISTENT_TASK_BUF(plat_info);
	g_sq_desc = DEVDRV_PLAT_GET_SQCQ_BUF(plat_info);

	tmp = (char *)(uintptr_t) __get_free_pages(gfp_flags, order);
	if (tmp == NULL) {
		devdrv_drv_err("alloc share mem descriptor memory failed !\n");
		return -ENOMEM;
	}

	g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr = g_sq_desc->base +
	    CHIP_BASEADDR_PA_OFFSET * dev_id;
	g_shm_desc[dev_id][DEVDRV_SQ_MEM].size = g_sq_desc->len;

	g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr = virt_to_phys(tmp);
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr =
	    (vir_addr_t) (uintptr_t) tmp;
	g_shm_desc[dev_id][DEVDRV_INFO_MEM].size =
	    (long)(unsigned)(1 << order) * PAGE_SIZE;

	g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr = doorbell_base;
	g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size = doorbell_size;

	g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].phy_addr =
							persistent_task_buf_desc->base;
	g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].size =
							persistent_task_buf_desc->len;

    ts_status = devdrv_get_ts_work_status(dev_id, 0);
    *ts_status = DEVDRV_TS_DOWN;

	gfp_flags = GFP_KERNEL | __GFP_ZERO;
	tmp = (char *)(uintptr_t) __get_free_pages(gfp_flags, 0);
	if (tmp == NULL) {
		devdrv_drv_err("alloc share mem pad memory failed.n");
		return -ENOMEM;
	}
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr = virt_to_phys(tmp);
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].virt_addr =
		(vir_addr_t) (uintptr_t) tmp;
	g_shm_desc[dev_id][DEVDRV_PAD_MEM].size = PAGE_SIZE;

	devdrv_drv_debug("sq mem: "
			 "phy_addr = 0x%llx, "
			 "size = %lu\n",
			 g_shm_desc[dev_id][DEVDRV_SQ_MEM].phy_addr,
			 g_shm_desc[dev_id][DEVDRV_SQ_MEM].size);

	devdrv_drv_debug("info mem: virt_addr = 0x%llx, "
			 "order = %u, size = %lu\n",
			 g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr,
			 order, g_shm_desc[dev_id][DEVDRV_INFO_MEM].size);

	devdrv_drv_debug("doorbell mem: "
			 "phy_addr = 0x%llx, "
			 "size = %lu\n",
			 g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr,
			 g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size);

	devdrv_drv_debug("persistent task buf mem: "
			"phy_addr = 0x%llx, "
			"size = %lu\n",
			g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].phy_addr,
			g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].size);

	//init continuous memory resource
	return devdrv_continuous_mem_init(dev_id);
}

int devdrv_save_cm_info(struct devdrv_dev_ctx *dev_ctx,
						struct devdrv_cm_info *info,
						u32 *share_num)
{
	int i = 0;
	u8 dev_id = 0;

	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	if (info == NULL) {
		devdrv_drv_err("cm info is null\n");
		return -EINVAL;
	}

	if (share_num == NULL) {
		devdrv_drv_err("share_num addr is null\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	for (i = 0; i < MAX_MEM_INFO_NUM; i++) {
		if (dev_ctx->cm->mem_info[i].valid != MEM_INFO_VALID) {
			spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
			dev_ctx->cm->mem_info[i].dma_handle = info->dma_handle;
			dev_ctx->cm->mem_info[i].cpu_addr = info->cpu_addr;
			dev_ctx->cm->mem_info[i].size = info->size;
			dev_ctx->cm->mem_info[i].valid = MEM_INFO_VALID;
			*share_num = i;
			spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);
			return 0;
		} else if (info->dma_handle == dev_ctx->cm->mem_info[i].dma_handle) {
			devdrv_drv_err("repeat insert npu cm l2 ctrl addr.\n");
			return -EINVAL;
		}
	}

	return -EINVAL;

}

int devdrv_save_cm_info_occupied(struct devdrv_dev_ctx *dev_ctx,
							    u32 *share_num)
{
	int i = 0;
	u8 dev_id = 0;

	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	if (share_num == NULL) {
		devdrv_drv_err("share_num addr is null\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid = MEM_INFO_VALID;
	*share_num = SHARE_NUM_OCCUPIED;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);
	return 0;
}

int devdrv_delete_cm_info(struct devdrv_dev_ctx *dev_ctx, u32 share_num)
{
	u8 dev_id = 0;

	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	if (share_num >= MAX_MEM_INFO_NUM) {
		devdrv_drv_err("invalid share_num %d\n", share_num);
		return -EINVAL;
	}

	if (dev_ctx->cm->mem_info[share_num].valid != MEM_INFO_VALID) {
		devdrv_drv_err("invalid info ,no need to delete %d\n",
			       dev_ctx->cm->mem_info[share_num].valid);
		return -EINVAL;
	}

	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	dev_ctx->cm->mem_info[share_num].dma_handle = 0;
	dev_ctx->cm->mem_info[share_num].cpu_addr = 0;
	dev_ctx->cm->mem_info[share_num].size = 0;
	dev_ctx->cm->mem_info[share_num].valid = 0;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	return 0;
}

int devdrv_delete_cm_info_occupied(struct devdrv_dev_ctx *dev_ctx)
{
	u8 dev_id = 0;

	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	dev_id = dev_ctx->devid;
	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id = %d\n", dev_id);
		return -EINVAL;
	}

	if (dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid != MEM_INFO_VALID) {
		devdrv_drv_err("invalid info ,no need to delete %d\n",
			       dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid);
		return -EINVAL;
	}

	spin_lock(&g_continuous_mem[dev_id].cm_spinlock);
	dev_ctx->cm->mem_info[SHARE_NUM_OCCUPIED].valid = 0;
	spin_unlock(&g_continuous_mem[dev_id].cm_spinlock);

	return 0;
}

int devdrv_inc_cm_total_size(struct devdrv_dev_ctx *dev_ctx, u64 size)
{
	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}
	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	spin_lock(&dev_ctx->cm->cm_spinlock);
	dev_ctx->cm->total_size += size;
	spin_unlock(&dev_ctx->cm->cm_spinlock);

	devdrv_drv_debug("total ocuppied cm size = %llx after request now\n",
			 dev_ctx->cm->total_size);

	return 0;
}

static int devdrv_get_cm_info(u8 dev_id,
			      u32 share_num, struct devdrv_cm_info *info)
{

	info->dma_handle =
	    g_continuous_mem[dev_id].mem_info[share_num].dma_handle;
	info->cpu_addr = g_continuous_mem[dev_id].mem_info[share_num].cpu_addr;
	info->size = g_continuous_mem[dev_id].mem_info[share_num].size;

	return 0;
}

int devdrv_dec_cm_total_size(struct devdrv_dev_ctx *dev_ctx, u64 size)
{
	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}
	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	spin_lock(&dev_ctx->cm->cm_spinlock);
	dev_ctx->cm->total_size -= size;
	spin_unlock(&dev_ctx->cm->cm_spinlock);

	devdrv_drv_debug("total ocuppied cm size = 0x%llx after free now\n",
			 dev_ctx->cm->total_size);

	return 0;
}

int devdrv_is_cm_available(struct devdrv_dev_ctx *dev_ctx, u64 req_size)
{
	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	devdrv_drv_debug("current cm occupied total size = 0x%llx, "
			 "request_size = 0x%llx\n dev mem total size = 0x%x \n",
			 dev_ctx->cm->total_size, req_size, MEM_TOTAL_SIZE);

	if (req_size > MEM_TOTAL_SIZE) {
	    devdrv_drv_err("req_size = 0x%llx is oversize\n");
	    return -EINVAL;
	}

	if(dev_ctx->cm->total_size > (MEM_TOTAL_SIZE - req_size)) {
	    devdrv_drv_err("left cm size is no enough now\n");
	    return -EINVAL;
	}

	return 0;
}

int devdrv_is_cm_valid(struct devdrv_dev_ctx *dev_ctx, u32 share_num)
{
	u64 total_size = 0;
	u64 size = 0;

	if (dev_ctx == NULL) {
		devdrv_drv_err("dev_ctx is null\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	if (share_num >= MAX_MEM_INFO_NUM) {
		devdrv_drv_err("invalid share_num %d\n", share_num);
		return -EINVAL;
	}

	if (dev_ctx->cm->mem_info[share_num].valid != MEM_INFO_VALID) {
		devdrv_drv_err("invalid cm info valid_val =  %d share_num = %d\n",
		     dev_ctx->cm->mem_info[share_num].valid, share_num);
		return -EINVAL;
	}

	size = dev_ctx->cm->mem_info[share_num].size;
	total_size = dev_ctx->cm->total_size;
	if (size > total_size) {
		devdrv_drv_err("share_num = %d invalid free_size 0x%llx "
			       "total size 0x%llx\n", share_num, size,
			       total_size);
		return -EINVAL;
	}

	return 0;
}

int devdrv_map_l2_buff(const struct file *filp, struct vm_area_struct *vma, u8 dev_id)
{

	unsigned long size = 0;
	struct devdrv_mem_desc *l2_desc = NULL;
	unsigned long l2_base, l2_mask, l2_len;
	unsigned long l2_start, l2_end;
	unsigned long pad_base, pad_start;
	if ((vma == NULL) || (filp == NULL) ||(dev_id >= NPU_DEV_NUM)) {
		devdrv_drv_err("invalid para.\n");
		return -EFAULT;
	}

	l2_desc = devdrv_plat_get_reg_desc(DEVDRV_REG_L2BUF_BASE);
	if (l2_desc == NULL) {
		devdrv_drv_err("devdrv_plat_get_reg_desc failed.\n");
		return -EFAULT;
	}

	l2_base = l2_desc->base;
	l2_len = l2_desc->len + 1;	// becasue of dts will minus 1
	l2_mask = (~(l2_len - 1));

	devdrv_drv_debug("vma start %lx end %lx\n", vma->vm_start, vma->vm_end);
	size = vma->vm_end - vma->vm_start;
	if((size > (l2_len * L2_MAP_ALIGN)) || (l2_len <= PAGE_SIZE)){
		devdrv_drv_err("invalid l2 map size 0x%lx len 0x%lx.\n", size, l2_len);
		return -EFAULT;
	}

	devdrv_drv_debug("l2_base %lx l2_len %lx l2_mask %lx.\n",
			 l2_base, l2_len, l2_mask);

	l2_start = (vma->vm_start & l2_mask) + l2_len;
	l2_end = l2_start + l2_len;
	devdrv_drv_debug("start %lx end %lx\n", l2_start, l2_end);

	/* we do not want to have this area swapped out, lock it */
	vma->vm_flags |= VM_LOCKED;

	pad_base = g_shm_desc[dev_id][DEVDRV_PAD_MEM].phy_addr;
	if((pad_base == 0) || (l2_base == 0)){
		devdrv_drv_err("invalid mem base.\n");
		return -EFAULT;
	}

	/**
	 * map head with the pad page
	 */
	for(pad_start = vma->vm_start; pad_start < l2_start; pad_start += PAGE_SIZE) {
		if (remap_pfn_range(vma, pad_start, __phys_to_pfn(pad_base),
				    PAGE_SIZE, vma->vm_page_prot)) {
			devdrv_drv_err("fail to map pad mem.\n");
			return -ENXIO;
		}
	}

	/**
	 * map l2 body with the pad page
	 */
	if (remap_pfn_range(vma, l2_start, __phys_to_pfn(l2_base), l2_len, vma->vm_page_prot))
		return -ENXIO;

	/**
	 * map tail with the pad page
	 */
	for(pad_start = l2_end; pad_start < vma->vm_end; pad_start += PAGE_SIZE)
	{
		if (remap_pfn_range(vma, pad_start, __phys_to_pfn(pad_base),
				    PAGE_SIZE, vma->vm_page_prot)) {
			devdrv_drv_err("fail to map pad mem.\n");
			return -ENXIO;
		}
	}

	return 0;
}

// for l2ctrl using currently
int devdrv_map_cm(const struct file *filp, struct vm_area_struct *vma,
		  u32 share_num, u8 dev_id)
{
	struct devdrv_cm_info info = { 0 };
	unsigned long pfn = 0;
	unsigned long size = 0;
	int ret = 0;
	struct mm_struct *mm = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -EFAULT;
	}

	if (share_num >= MAX_MEM_INFO_NUM) {
		devdrv_drv_err("illegal share_num %d\n", share_num);
		return -EFAULT;
	}

	size = vma->vm_end - vma->vm_start;

	ret = devdrv_get_cm_info(dev_id, share_num, &info);
	if (ret != 0) {
		devdrv_drv_err("get continue mem info fail ret %d\n", ret);
		return -EFAULT;
	}

	pfn = info.dma_handle >> PAGE_SHIFT;
	/* we do not want to have this area swapped out, lock it */
	vma->vm_flags |= VM_LOCKED;

	if (size > MEM_TOTAL_SIZE) {
		devdrv_drv_err("invalid cm size = %lu\n", size);
		return -ENOMEM;
	}

	vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot, 0, PTE_DIRTY);
	ret = remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot);
	if (ret != 0) {
		devdrv_drv_err("remap_pfn_range cm failed ret = %d\n", ret);
		return -ENXIO;
	}

	mm = current->mm;
	if (mm == NULL)
	{
		return -ENXIO;
	}

	if(hisi_svm_flush_cache(mm, vma->vm_start, size))
	{
		return -ENXIO;
	}
	return ret;
}

int devdrv_cm_resource_recycle(struct devdrv_dev_ctx *dev_ctx)
{
	struct devdrv_platform_info *plat_info = NULL;
	int i = 0;
	u64 size = 0;
	if (dev_ctx == NULL) {
		devdrv_drv_err("invalid dev_ctx value\n");
		return -EINVAL;
	}

	if (dev_ctx->cm == NULL) {
		devdrv_drv_err("invalid cm value\n");
		return -EINVAL;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed.\n");
		return -EINVAL;
	}

	for (i = 0; i < MAX_MEM_INFO_NUM; i++) {
		if (dev_ctx->cm->mem_info[i].valid != MEM_INFO_VALID) {
			continue;
		}
		size = dev_ctx->cm->mem_info[i].size;
		if ((dev_ctx->cm->total_size) < size) {
			devdrv_drv_err("invalid size 0x%llx total size 0x%llx\n",
			     size, dev_ctx->cm->total_size);
			return -EINVAL;
		}
		devdrv_drv_err("recycle cm mem %d size 0x%llx \n", i, size);
		dma_free_coherent(DEVDRV_PLAT_GET_PDEV(plat_info), size,
				  dev_ctx->cm->mem_info[i].cpu_addr,
				  dev_ctx->cm->mem_info[i].dma_handle);

		(void)devdrv_delete_cm_info(dev_ctx, i);
		spin_lock(&dev_ctx->cm->cm_spinlock);
		dev_ctx->cm->total_size -= size;
		spin_unlock(&dev_ctx->cm->cm_spinlock);
		devdrv_drv_err("recycle cm mem %d size 0x%llx "
				"cm->total_size(occupied) = 0x%llx \n",
				i, size, dev_ctx->cm->total_size);
	}

	return 0;

}

// map dev_id npu`s info 、sq and cq to user space
int devdrv_info_sq_cq_mmap(u8 dev_id,
			   const struct file *filep, struct vm_area_struct *vma)
{
	unsigned long start_addr;
	phys_addr_t phy_addr;
	size_t size;
	int err;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (vma == NULL) {
		devdrv_drv_err("vma is null.\n");
		return -1;
	}

	devdrv_drv_debug("vma->vm_end = %pK  vma->vm_start = %pK \n",
			 (void *)(uintptr_t) vma->vm_end,
			 (void *)(uintptr_t) vma->vm_start);

	if (vma->vm_end < vma->vm_start) {
		devdrv_drv_err("vma->vm_end < vma->vm_start, invalid vma para\n");
		return -EINVAL;
	}

	vma->vm_flags |= VM_LOCKED;
	vma->vm_flags |= VM_DONTEXPAND;
	vma->vm_flags |= VM_PFNMAP;
	vma->vm_flags |= VM_WRITE;

	/*
	   |___SQ(32MB)___|____INFO(32MB)_____|__DOORBELL(32MB)___|___CQ(32MB)___|(32M vitural address space respectively)
	 */

	start_addr = vma->vm_start;
	phy_addr = g_sq_desc->base;
	devdrv_drv_debug("npu dev %d sq mem: user_virt_addr = 0x%lx,"
			 " phys_addr = 0x%llx\n",
			 dev_id, start_addr, phy_addr);
	/* remap sq cq pfn range for user space */
	err = remap_pfn_range(vma, start_addr,
			      phy_addr >> PAGE_SHIFT,
			      DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM,
			      vma->vm_page_prot);
	if (err) {
		devdrv_drv_err("npu dev_id %d sq memory mmap failed\n", dev_id);
		return -EINVAL;
	}

	/* remap info pfn range for user space */
	phy_addr = g_shm_desc[dev_id][DEVDRV_INFO_MEM].phy_addr;
	size = g_shm_desc[dev_id][DEVDRV_INFO_MEM].size;
	start_addr += DEVDRV_VM_BLOCK_OFFSET;	// gap to reduce memory override probability
	if (size <= 0) {
		devdrv_drv_err("npu dev %d illegal info mem size = %lu\n", dev_id, size);
		return -ENOMEM;
	}
	devdrv_drv_debug("npu dev %d info mem:user_virt_addr = 0x%lx, "
			 "phys_addr = 0x%llx, size = %lu\n",
			 dev_id, start_addr, phy_addr, size);
	err = remap_pfn_range(vma, start_addr,
			      phy_addr >> PAGE_SHIFT, size, vma->vm_page_prot);
	if (err) {
		devdrv_drv_err("npu dev_id %d info memory mmap failed\n", dev_id);
		return -EINVAL;
	}

	/* remap doorbell pfn range for user space temporarily,will delete later */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);	//lint !e446
	start_addr += DEVDRV_VM_BLOCK_OFFSET;
	phy_addr = g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].phy_addr;
	size = g_shm_desc[dev_id][DEVDRV_DOORBELL_MEM].size;
	if (size <= 0) {
		devdrv_drv_err("npu dev %d illegal doorbell cfg size = %lu\n", dev_id, size);
		return -ENOMEM;
	}
	devdrv_drv_debug("npu dev %d doorbell mem:user_virt_addr = 0x%lx, "
			 "phys_addr = 0x%llx, size = %lu\n",
			 dev_id, start_addr, phy_addr, size);
	err = remap_pfn_range(vma, start_addr,
			      phy_addr >> PAGE_SHIFT, size, vma->vm_page_prot);
	if (err) {
		devdrv_drv_err("npu dev_id %d doobell register mmap failed\n", dev_id);
		return -EINVAL;
	}
	// cq non cache solution
	start_addr += DEVDRV_VM_BLOCK_OFFSET;
	phy_addr = (unsigned long long)(g_sq_desc->base +
					DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE * DEVDRV_MAX_SQ_NUM);	//lint !e647
	devdrv_drv_debug("npu dev %d cq mem:user_virt_addr = 0x%lx, "
			 "phys_addr = 0x%llx\n",
			 dev_id, start_addr, phy_addr);
	err = remap_pfn_range(vma, start_addr,
			      phy_addr >> PAGE_SHIFT,
			      DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE,
			      vma->vm_page_prot);
	if (err) {
		devdrv_drv_err("npu dev_id %d cq mem mmap failed\n", dev_id);
		return -EINVAL;
	}

	//persistent task buff
	start_addr += DEVDRV_VM_BLOCK_OFFSET;
	phy_addr =  g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].phy_addr;
	size =  g_shm_desc[dev_id][DEVDRV_PERSISTENT_TASK_BUFF].size;
	devdrv_drv_debug("npu dev %d persistent task buff mem:user_virt_addr = 0x%lx, "
						"phys_addr = 0x%llx, size = %lu\n",
						dev_id, start_addr, phy_addr,size);
	err = remap_pfn_range(vma, start_addr,
		phy_addr >> PAGE_SHIFT, size, pgprot_writecombine(PAGE_SHARED));
	if (err) {
		devdrv_drv_err("npu dev_id %d persistent task buff mem mmap failed\n",dev_id);
		return -EINVAL;
	}

	return 0;
}

void devdrv_shm_destroy(u8 dev_id)
{
	unsigned long addr;
	u32 order;

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return;
	}

	order = DEVDRV_MAX_INFO_ORDER;
	addr = (unsigned long)g_shm_desc[dev_id][DEVDRV_INFO_MEM].virt_addr;
	free_pages(addr, order);
}
