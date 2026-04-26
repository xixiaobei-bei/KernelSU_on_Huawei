/* Copyright (c) 2014-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_mmbuf_manager.h"

uint32_t mmbuf_max_size;
dss_mmbuf_t dss_mmbuf_reserved_info[RESERVED_SERVICE_MAX] = { {0} };
struct dss_comm_mmbuf_info g_primary_online_mmbuf[DSS_CHN_MAX_DEFINE] = { {0} };
struct dss_comm_mmbuf_info g_external_online_mmbuf[DSS_CHN_MAX_DEFINE] = { {0} };
struct list_head *g_mmbuf_list;
/* mmbuf gen pool */
struct gen_pool *g_mmbuf_gen_pool;

static int g_mmbuf_refcount;

DEFINE_SEMAPHORE(hisi_dss_mmbuf_sem);

/* 128 bytes */
#define SMMU_RW_ERR_ADDR_SIZE 128
#define is_mmbuf_reserved_service(owner) (owner < RESERVED_SERVICE_MAX)

struct dpufb_mmbuf {
	struct list_head list_node;
	uint32_t addr;
	uint32_t size;
};

extern void dpufb_dss_off(struct dpu_fb_data_type *dpufd, bool is_lp);

static bool mmbuf_reserved_ignore(struct dpufb_mmbuf *node)
{
	int index;
	bool ret = false;

	if (!node) {
		DPU_FB_ERR("node is NULL\n");
		return ret;
	}

	for (index = 0; index < RESERVED_SERVICE_MAX; index++) {
		if ((node->addr == dss_mmbuf_reserved_info[index].addr) &&
			node->size == dss_mmbuf_reserved_info[index].size) {
			ret = true;
			break;
		}
	}

	return ret;
}

static bool mmbuf_reserved_alloc(struct dpu_fb_data_type *dpufd, dss_mmbuf_t *mmbuf_info)
{
	bool ret = true;

	if (dss_mmbuf_reserved_info[mmbuf_info->owner].addr != 0) {
		mmbuf_info->addr = dss_mmbuf_reserved_info[mmbuf_info->owner].addr;
		DPU_FB_DEBUG("fb%d, service 0x%x realloc addr 0x%x, reserved size 0x%x\n",
			dpufd->index, mmbuf_info->owner, mmbuf_info->addr, mmbuf_info->size);
	} else {
		mmbuf_info->addr = hisi_dss_mmbuf_alloc(dpufd->mmbuf_gen_pool, mmbuf_info->size);
		if (mmbuf_info->addr < MMBUF_BASE)
			ret = false;

		dss_mmbuf_reserved_info[mmbuf_info->owner].addr = mmbuf_info->addr;
		DPU_FB_DEBUG("fb%d, service 0x%x first alloc addr 0x%x, reserved size 0x%x\n",
			dpufd->index, mmbuf_info->owner, mmbuf_info->addr, mmbuf_info->size);
	}

	return ret;
}

static void mmbuf_reserved_info_clear(uint32_t addr, uint32_t size)
{
	int index;

	for (index = 0; index < RESERVED_SERVICE_MAX; index++) {
		if ((addr == dss_mmbuf_reserved_info[index].addr) && (size == dss_mmbuf_reserved_info[index].size)) {
			dss_mmbuf_reserved_info[index].addr = 0;
			DPU_FB_DEBUG("service 0x%x release addr 0x%x, reserved size 0x%x\n", index, addr, size);

			return;
		}
	}
}

static void mmbuf_test(uint32_t mmbuf_size_max, dss_mmbuf_t mmbuf_info)
{
	if (g_mmbuf_addr_test >= (mmbuf_size_max + 0x40)) {  /* offset addr */
		DPU_FB_ERR("g_mmbuf_addr_test[0x%x] is overflow max mmbuf size + 0x40[0x%x]\n",
			g_mmbuf_addr_test, mmbuf_size_max + 0x40);
		DPU_FB_ERR("remain buff size if %d\n",
			(mmbuf_size_max + 0x40) - (g_mmbuf_addr_test - mmbuf_info.size));
		g_mmbuf_addr_test = 0;
	} else {
		mmbuf_info.addr = g_mmbuf_addr_test;
		g_mmbuf_addr_test += mmbuf_info.size;
	}

	DPU_FB_INFO("addr = 0x%x, size =%d,g_mmbuf_addr_test = 0x%x, MAX_SIZE= 0x%x\n",
		mmbuf_info.addr, mmbuf_info.size, g_mmbuf_addr_test, mmbuf_size_max + 0x40);
}

void hisi_mmbuf_sem_pend(void)
{
	down(&hisi_dss_mmbuf_sem);
}

void hisi_mmbuf_sem_post(void)
{
	up(&hisi_dss_mmbuf_sem);
}

void hisi_mmbuf_info_init(void)
{
	int reserved_service_id;

	for (reserved_service_id = 0; reserved_service_id < RESERVED_SERVICE_MAX; reserved_service_id++) {
		dss_mmbuf_reserved_info[reserved_service_id].owner = reserved_service_id;
		dss_mmbuf_reserved_info[reserved_service_id].addr = 0;
		dss_mmbuf_reserved_info[reserved_service_id].size = 0;
	}
}

void *hisi_dss_mmbuf_init(struct dpu_fb_data_type *dpufd)
{
	struct gen_pool *pool = NULL;
	int order = 3;  /* min alloc order */
	uint32_t addr = MMBUF_BASE;
	int prev_refcount;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return NULL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	hisi_mmbuf_sem_pend();

	prev_refcount = g_mmbuf_refcount++;
	if (prev_refcount == 0) {
		/* mmbuf pool */
		pool = gen_pool_create(order, 0);
		if (!pool) {
			DPU_FB_ERR("fb%d, gen_pool_create failed\n", dpufd->index);
			goto err_out;
		}

		if (gen_pool_add(pool, addr, (size_t)mmbuf_max_size, 0) != 0) {
			gen_pool_destroy(pool);
			pool = NULL;
			DPU_FB_ERR("fb%d, gen_pool_add failed\n", dpufd->index);
			goto err_out;
		}

		g_mmbuf_gen_pool = pool;

		/* mmbuf list */
		if (!g_mmbuf_list) {
			g_mmbuf_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
			if (!g_mmbuf_list) {
				DPU_FB_ERR("g_mmbuf_list is NULL\n");
				hisi_mmbuf_sem_post();
				return NULL;
			}
			INIT_LIST_HEAD(g_mmbuf_list);
		}

		/* smmu */
		if (!g_smmu_rwerraddr_virt) {
			g_smmu_rwerraddr_virt = kmalloc(SMMU_RW_ERR_ADDR_SIZE, GFP_KERNEL | __GFP_DMA);
			if (g_smmu_rwerraddr_virt != NULL)
				memset(g_smmu_rwerraddr_virt, 0, SMMU_RW_ERR_ADDR_SIZE);
			else
				DPU_FB_ERR("kmalloc g_smmu_rwerraddr_virt fail\n");
		}
	}

	dpufd->mmbuf_gen_pool = g_mmbuf_gen_pool;
	dpufd->mmbuf_list = g_mmbuf_list;

err_out:
	hisi_mmbuf_sem_post();

	DPU_FB_DEBUG("fb%d, -, g_mmbuf_refcount=%d\n", dpufd->index, g_mmbuf_refcount);

	return pool;
}

void hisi_dss_mmbuf_deinit(struct dpu_fb_data_type *dpufd)
{
	int new_refcount;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	dpufb_dss_off(dpufd, false);

	hisi_mmbuf_sem_pend();
	new_refcount = --g_mmbuf_refcount;
	if (new_refcount < 0)
		DPU_FB_ERR("dss new_refcount err\n");

	if (new_refcount == 0) {
		/* mmbuf pool */
		if (g_mmbuf_gen_pool != NULL) {
			gen_pool_destroy(g_mmbuf_gen_pool);
			g_mmbuf_gen_pool = NULL;
		}

		/* mmbuf list */
		if (g_mmbuf_list != NULL) {
			kfree(g_mmbuf_list);
			g_mmbuf_list = NULL;
		}

		/* smmu */
		if (g_smmu_rwerraddr_virt != NULL) {
			kfree(g_smmu_rwerraddr_virt);
			g_smmu_rwerraddr_virt = NULL;
		}
	}

	dpufd->mmbuf_gen_pool = NULL;
	dpufd->mmbuf_list = NULL;
	hisi_mmbuf_sem_post();

	DPU_FB_DEBUG("fb%d, -, g_mmbuf_refcount=%d\n", dpufd->index, g_mmbuf_refcount);
}

uint32_t hisi_dss_mmbuf_alloc(void *handle, uint32_t size)
{
	uint32_t addr = 0;
	struct dpufb_mmbuf *node = NULL;
	struct dpufb_mmbuf *mmbuf_node = NULL;
	struct dpufb_mmbuf *_node_ = NULL;

	dpu_check_and_return(!handle, addr, ERR, "handle is NULL\n");
	dpu_check_and_return(!g_mmbuf_list, addr, ERR, "g_mmbuf_list is NULL\n");
	dpu_check_and_return((size == 0), addr, ERR, "mmbuf size is invalid, size=%d\n", size);

	hisi_mmbuf_sem_pend();

	addr = gen_pool_alloc(handle, size);

	if (addr <= 0) {
		list_for_each_entry_safe(mmbuf_node, _node_, g_mmbuf_list, list_node) {
			DPU_FB_DEBUG("mmbuf_node_addr[0x%x], mmbuf_node_size[0x%x]\n",
				mmbuf_node->addr, mmbuf_node->size);
		}
		DPU_FB_INFO("note: mmbuf not enough,addr=0x%x\n", addr);
	} else {
		/* node */
		node = kzalloc(sizeof(struct dpufb_mmbuf), GFP_KERNEL);
		if (node != NULL) {
			node->addr = addr;
			node->size = size;
			list_add_tail(&node->list_node, g_mmbuf_list);
		} else {
			DPU_FB_ERR("kzalloc struct dpufb_mmbuf fail\n");
		}

		if ((addr & (MMBUF_ADDR_ALIGN - 1)) || (size & (MMBUF_ADDR_ALIGN - 1))) {
			DPU_FB_ERR("addr[0x%x] is not %d bytes aligned, or size[0x%x] is not %d bytes aligned\n",
				addr, MMBUF_ADDR_ALIGN, size, MMBUF_ADDR_ALIGN);

			list_for_each_entry_safe(mmbuf_node, _node_, g_mmbuf_list, list_node) {
				DPU_FB_ERR("mmbuf_node_addr[0x%x], mmbuf_node_size[0x%x]\n", mmbuf_node->addr,
					mmbuf_node->size);
			}
		}
	}

	hisi_mmbuf_sem_post();

	if (g_enable_mmbuf_debug != 0)
		DPU_FB_DEBUG("addr=0x%x, size=%d\n", addr, size);

	return addr;
}

void hisi_dss_mmbuf_free(void *handle, uint32_t addr, uint32_t size)
{
	struct dpufb_mmbuf *node = NULL;
	struct dpufb_mmbuf *_node_ = NULL;

	dpu_check_and_no_retval(!handle, ERR, "handle is NULL\n");
	dpu_check_and_no_retval(!g_mmbuf_list, ERR, "g_mmbuf_list is NULL\n");

	hisi_mmbuf_sem_pend();

	list_for_each_entry_safe(node, _node_, g_mmbuf_list, list_node) {
		if ((node->addr == addr) && (node->size == size)) {
			gen_pool_free(handle, addr, size);
			list_del(&node->list_node);
			kfree(node);
			node = NULL;

			mmbuf_reserved_info_clear(addr, size);
		}
	}

	hisi_mmbuf_sem_post();

	if (g_enable_mmbuf_debug != 0)
		DPU_FB_DEBUG("addr=0x%x, size=%d\n", addr, size);
}

void hisi_dss_mmbuf_free_all(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_mmbuf *node = NULL;
	struct dpufb_mmbuf *_node_ = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval(!g_mmbuf_list, ERR, "g_mmbuf_list is NULL\n");

	list_for_each_entry_safe(node, _node_, g_mmbuf_list, list_node) {
		if ((node->addr > 0) && (node->size > 0)) {
			if (mmbuf_reserved_ignore(node) == true) {
				DPU_FB_INFO("fb%d, ignore reserved node addr=0x%x, size=%d\n",
					dpufd->index, node->addr, node->size);
				continue;
			}
			gen_pool_free(dpufd->mmbuf_gen_pool, node->addr, node->size);
			DPU_FB_DEBUG("hisi_dss_mmbuf_free, addr=0x%x, size=%d\n", node->addr, node->size);
		}

		list_del(&node->list_node);
		kfree(node);
		node = NULL;
	}
}

dss_mmbuf_info_t *hisi_dss_mmbuf_info_get(struct dpu_fb_data_type *dpufd, int idx)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return NULL;
	}
	if ((idx < 0) || (idx >= HISI_DSS_CMDLIST_DATA_MAX)) {
		DPU_FB_ERR("idx is invalid\n");
		return NULL;
	}

	return &(dpufd->mmbuf_infos[idx]);
}

void hisi_dss_mmbuf_info_clear(struct dpu_fb_data_type *dpufd, int idx)
{
	int i;
	dss_mmbuf_info_t *mmbuf_info = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if ((idx < 0) || (idx >= HISI_DSS_CMDLIST_DATA_MAX)) {
		DPU_FB_ERR("idx is invalid\n");
		return;
	}

	/* mmbuf */
	mmbuf_info = &(dpufd->mmbuf_infos[idx]);
	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (mmbuf_info->mm_used[i] == 1) {
			hisi_dss_mmbuf_free(g_mmbuf_gen_pool, mmbuf_info->mm_base[i], mmbuf_info->mm_size[i]);

			if (g_debug_ovl_online_composer)
				DPU_FB_INFO("fb%d, mm_base[0x%x, %d]\n",
					dpufd->index, mmbuf_info->mm_base[i], mmbuf_info->mm_size[i]);

			mmbuf_info->mm_base[i] = 0;
			mmbuf_info->mm_size[i] = 0;
			mmbuf_info->mm_used[i] = 0;
		}
	}
}

static void get_use_comm_mmbuf(int *use_comm_mmbuf,
	dss_mmbuf_t *offline_mmbuf, struct dss_comm_mmbuf_info *online_mmbuf)
{
	int i;
	int j;

	if (g_debug_dump_mmbuf) {
		for (i = 0; i < DSS_CHN_MAX_DEFINE; i++)
			DPU_FB_INFO("g_online_mmbuf[%d].addr=0x%x, size=0x%x\n",
				i, online_mmbuf[i].mmbuf.addr, online_mmbuf[i].mmbuf.size);
	}

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		for (j = 0; j < DSS_CHN_MAX_DEFINE; j++) {
			if ((((offline_mmbuf[i].addr < online_mmbuf[j].mmbuf.addr + online_mmbuf[j].mmbuf.size) &&
				(offline_mmbuf[i].addr >= online_mmbuf[j].mmbuf.addr)) ||
				((online_mmbuf[j].mmbuf.addr < offline_mmbuf[i].addr + offline_mmbuf[i].size) &&
				(online_mmbuf[j].mmbuf.addr >= offline_mmbuf[i].addr))) && offline_mmbuf[i].size) {
				if (use_comm_mmbuf) {
					*((uint32_t *)use_comm_mmbuf) |= 1 << (uint32_t)(online_mmbuf[j].ov_idx);
					online_mmbuf[j].mmbuf.addr = 0;
					online_mmbuf[j].mmbuf.size = 0;
					break;
				}
			}
		}
	}
}

void hisi_dss_check_use_comm_mmbuf(uint32_t display_id,
	int *use_comm_mmbuf, dss_mmbuf_t *offline_mmbuf, bool has_rot)
{
	int i;

	dpu_check_and_no_retval(!offline_mmbuf, ERR, "offline_mmbuf is null pointer\n");
	if (display_id == PRIMARY_PANEL_IDX) {
		get_use_comm_mmbuf(use_comm_mmbuf, offline_mmbuf, g_primary_online_mmbuf);
		get_use_comm_mmbuf(use_comm_mmbuf, offline_mmbuf, g_external_online_mmbuf);

		if (!has_rot)
			memset(g_primary_online_mmbuf, 0x0, sizeof(g_primary_online_mmbuf));
	}

	if (display_id == EXTERNAL_PANEL_IDX) {
		get_use_comm_mmbuf(use_comm_mmbuf, offline_mmbuf, g_external_online_mmbuf);
		get_use_comm_mmbuf(use_comm_mmbuf, offline_mmbuf, g_primary_online_mmbuf);

		if (!has_rot)
			memset(g_external_online_mmbuf, 0x0, sizeof(g_external_online_mmbuf));
	}

	if (g_debug_dump_mmbuf && use_comm_mmbuf) {
		for (i = 0; i < DSS_CHN_MAX_DEFINE; i++)
			DPU_FB_INFO("offline_mmbuf[%d].addr=0x%x, size=0x%x, *use_comm_mmbuf=%d\n",
				i, offline_mmbuf[i].addr, offline_mmbuf[i].size, *use_comm_mmbuf);
	}
}

int hisi_dss_rdma_set_mmbuf_base_and_size(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_rect_ltrb_t *afbc_rect, uint32_t *mm_base_0, uint32_t *mm_base_1)
{
	bool mm_alloc_needed = false;
	int chn_idx;
	dss_rect_t new_src_rect = { 0, 0, 0, 0 };
	int ovl_idx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufb is NULL\n");
	dpu_check_and_return(!afbc_rect, -EINVAL, ERR, "afbc_rect is NULL\n");
	dpu_check_and_return(!mm_base_0, -EINVAL, ERR, "mm_base_0 is NULL\n");
	dpu_check_and_return(!mm_base_1, -EINVAL, ERR, "mm_base_1 is NULL\n");

	ovl_idx = dpufd->ov_req.ovl_idx;
	chn_idx = layer->chn_idx;
	new_src_rect = layer->src_rect;
	if ((layer->img.mmbuf_base > 0) && (layer->img.mmbuf_size > 0)) {
		*mm_base_0 = layer->img.mmbuf_base;
		*mm_base_1 = layer->img.mmbuf_base + layer->img.mmbuf_size / 2;
	} else {
		if (dpufd->mmbuf_info == NULL) {
			DPU_FB_ERR("dpufd->mmbuf_info is NULL\n");
			return -EINVAL;
		}

		if (ovl_idx <= DSS_OVL1) {
			mm_alloc_needed = true;
		} else {
			if (dpufd->mmbuf_info->mm_used[chn_idx] == 1)
				mm_alloc_needed = false;
			else
				mm_alloc_needed = true;
		}

		if (mm_alloc_needed) {
			afbc_rect->left = ALIGN_DOWN((uint32_t)new_src_rect.x, MMBUF_ADDR_ALIGN);
			afbc_rect->right = ALIGN_UP((uint32_t)(new_src_rect.x - afbc_rect->left + new_src_rect.w),
				MMBUF_ADDR_ALIGN);
			dpufd->mmbuf_info->mm_size[chn_idx] = afbc_rect->right * layer->img.bpp * MMBUF_LINE_NUM;
			dpufd->mmbuf_info->mm_base[chn_idx] = hisi_dss_mmbuf_alloc(g_mmbuf_gen_pool,
				dpufd->mmbuf_info->mm_size[chn_idx]);
			if (dpufd->mmbuf_info->mm_base[chn_idx] < MMBUF_BASE) {
				DPU_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base=0x%x\n",
					dpufd->index, chn_idx, dpufd->mmbuf_info->mm_base[chn_idx]);
				return -EINVAL;
			}
		}

		*mm_base_0 = dpufd->mmbuf_info->mm_base[chn_idx];
		*mm_base_1 = dpufd->mmbuf_info->mm_base[chn_idx] +
			dpufd->mmbuf_info->mm_size[chn_idx] / 2;  /* half size */
		dpufd->mmbuf_info->mm_used[chn_idx] = 1;  /* enable flag */
	}

	*mm_base_0 -= MMBUF_BASE;
	*mm_base_1 -= MMBUF_BASE;

	return 0;
}

void hisi_mmbuf_info_get_online(struct dpu_fb_data_type *dpufd)
{
	int tmp;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	tmp = (dpufd->frame_count + 1) % HISI_DSS_CMDLIST_DATA_MAX;
	hisi_dss_mmbuf_info_clear(dpufd, tmp);

	tmp = dpufd->frame_count % HISI_DSS_CMDLIST_DATA_MAX;
	dpufd->mmbuf_info = &(dpufd->mmbuf_infos[tmp]);
}

int hisi_mmbuf_reserved_size_query(struct fb_info *info, void __user *argp)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	dss_mmbuf_t reserved_mmbuf_info;

	dpu_check_and_return(!info, -EINVAL, ERR, "reserved mmbuf size query info NULL Pointer\n");
	dpufd = (struct dpu_fb_data_type *)info->par;
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "reserved mmbuf size query dpufd NULL Pointer\n");
	dpu_check_and_return(!argp, -EINVAL, ERR, "reserved mmbuf size query argp NULL Pointer\n");

	ret = copy_from_user(&reserved_mmbuf_info, argp, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		DPU_FB_ERR("fb%d, copy for user failed!, ret=%d\n", dpufd->index, ret);
		return -EINVAL;
	}

	if (reserved_mmbuf_info.owner < 0 || reserved_mmbuf_info.owner >= RESERVED_SERVICE_MAX) {
		DPU_FB_ERR("reserved mmbuf owner invalid\n");
		return -EINVAL;
	}

	reserved_mmbuf_info.size = dss_mmbuf_reserved_info[reserved_mmbuf_info.owner].size;

	ret = copy_to_user(argp, &reserved_mmbuf_info, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		DPU_FB_ERR("fb%d, copy to user failed!ret=%d\n", dpufd->index, ret);
		return -EFAULT;
	}

	DPU_FB_DEBUG("fb%d, service %d get reserved mmbuf size 0x%x\n",
		dpufd->index, reserved_mmbuf_info.owner, reserved_mmbuf_info.size);

	return 0;
}

int hisi_mmbuf_request(struct fb_info *info, void __user *argp)
{
	int ret;
	uint32_t mmbuf_size_max;
	struct dpu_fb_data_type *dpufd = NULL;
	dss_mmbuf_t mmbuf_info;

	dpu_check_and_return(!info, -EINVAL, ERR, "dss mmbuf alloc info NULL Pointer\n");
	dpufd = (struct dpu_fb_data_type *)info->par;
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL Pointer\n");

	dpu_check_and_return(!argp, -EINVAL, ERR, "dss mmbuf alloc argp NULL Pointer\n");

	ret = copy_from_user(&mmbuf_info, argp, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		DPU_FB_ERR("fb%d, copy for user failed!ret=%d\n", dpufd->index, ret);
		return -EINVAL;
	}

	mmbuf_size_max = mmbuf_max_size;

	if (mmbuf_info.owner < 0 || mmbuf_info.owner >= SERVICE_MAX) {
		DPU_FB_ERR("mmbuf owner %d is invalid\n", mmbuf_info.owner);
		return -EINVAL;
	}

	if ((mmbuf_info.size <= 0) || (mmbuf_info.size > mmbuf_size_max) ||
		(mmbuf_info.size & (MMBUF_ADDR_ALIGN - 1))) {
		DPU_FB_ERR("fb%d, mmbuf size is invalid, size=%d\n", dpufd->index, mmbuf_info.size);
		return -EINVAL;
	}

	if (g_mmbuf_addr_test > 0)
		mmbuf_test(mmbuf_size_max, mmbuf_info);

	if (g_mmbuf_addr_test == 0) {
		if (is_mmbuf_reserved_service(mmbuf_info.owner)) { /* alloc reserved mmbuf */
			if (mmbuf_reserved_alloc(dpufd, &mmbuf_info) == false)
				return -EINVAL;
		} else {
			mmbuf_info.addr = hisi_dss_mmbuf_alloc(dpufd->mmbuf_gen_pool, mmbuf_info.size);
			if (mmbuf_info.addr < MMBUF_BASE)
				return -EINVAL;
		}
	}

	ret = copy_to_user(argp, &mmbuf_info, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		DPU_FB_ERR("fb%d, copy to user failed!ret=%d\n", dpufd->index, ret);
		hisi_dss_mmbuf_free(dpufd->mmbuf_gen_pool, mmbuf_info.addr, mmbuf_info.size);
		return -EINVAL;
	}

	return 0;
}

int hisi_mmbuf_release(struct fb_info *info, const void __user *argp)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	dss_mmbuf_t mmbuf_info;

	dpu_check_and_return(!info, -EINVAL, ERR, "dss mmbuf free info NULL Pointer\n");
	dpufd = (struct dpu_fb_data_type *)info->par;
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dss mmbuf free dpufd NULL Pointer\n");
	dpu_check_and_return(!argp, -EINVAL, ERR, "dss mmbuf free argp NULL Pointer\n");

	ret = copy_from_user(&mmbuf_info, argp, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		DPU_FB_ERR("fb%d, copy for user failed!ret=%d\n", dpufd->index, ret);
		ret = -EINVAL;
		goto err_out;
	}

	if ((mmbuf_info.addr <= 0) || (mmbuf_info.size <= 0)) {
		DPU_FB_ERR("fb%d, addr=0x%x, size=%d is invalid\n",
			dpufd->index, mmbuf_info.addr, mmbuf_info.size);
		ret = -EINVAL;
		goto err_out;
	}

	hisi_dss_mmbuf_free(dpufd->mmbuf_gen_pool, mmbuf_info.addr, mmbuf_info.size);

	return 0;

err_out:
	return ret;
}

int hisi_mmbuf_free_all(struct fb_info *info, const void __user *argp)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return(!info, -EINVAL, ERR, "fb info is NULL Pointer\n");
	dpufd = (struct dpu_fb_data_type *)info->par;
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL Pointer\n");

	/* argp can be set to mmbuf owner, but now it isn't useful */
	(void)argp;

	hisi_mmbuf_sem_pend();
	hisi_dss_mmbuf_free_all(dpufd);
	hisi_mmbuf_sem_post();

	return 0;
}

