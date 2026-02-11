/* Copyright (c) 2014-2017, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e502 -e737 -e570 -e651 -e666 -e712 -e713 -e775 -e778 -e413*/

#include "hisi_mmbuf_manager.h"

uint32_t mmbuf_max_size = 0;
dss_mmbuf_t dss_mmbuf_reserved_info[RESERVED_SERVICE_MAX] = {{0}};
struct dss_comm_mmbuf_info g_primary_online_mmbuf[DSS_CHN_MAX_DEFINE] = {{0}};
struct dss_comm_mmbuf_info g_external_online_mmbuf[DSS_CHN_MAX_DEFINE] = {{0}};
struct list_head *g_mmbuf_list = NULL;
/* mmbuf gen pool */
struct gen_pool *g_mmbuf_gen_pool = NULL;

static int mmbuf_refcount = 0;

DEFINE_SEMAPHORE(hisi_dss_mmbuf_sem);

// 128 bytes
#define SMMU_RW_ERR_ADDR_SIZE	(128)
#define IS_MMBUF_RESERVED_SERVICE(owner)	(owner < RESERVED_SERVICE_MAX)

struct hisifb_mmbuf {
	struct list_head list_node;
	uint32_t addr;
	uint32_t size;
};

extern void hisifb_dss_off(struct hisi_fb_data_type *hisifd, bool is_lp);

static bool mmbuf_reserved_ignore(struct hisifb_mmbuf *node)
{
	int index;
	bool ret = false;

	if (node == NULL) {
		HISI_FB_ERR("node is NULL");
		return ret;
	}

	for (index = 0; index < RESERVED_SERVICE_MAX; index++) {
		if ((node->addr == dss_mmbuf_reserved_info[index].addr) && node->size == dss_mmbuf_reserved_info[index].size) {
			ret = true;
			break;
		}
	}

	return ret;
}

static bool mmbuf_reserved_alloc(struct hisi_fb_data_type *hisifd, dss_mmbuf_t *mmbuf_info)
{
	bool ret = true;

	if (dss_mmbuf_reserved_info[mmbuf_info->owner].addr != 0) {
		mmbuf_info->addr = dss_mmbuf_reserved_info[mmbuf_info->owner].addr;
		HISI_FB_DEBUG("fb%d, service %x realloc addr 0x%x, reserved size 0x%x",
			hisifd->index, mmbuf_info->owner, mmbuf_info->addr,mmbuf_info->size);
	} else {
		mmbuf_info->addr = hisi_dss_mmbuf_alloc(hisifd->mmbuf_gen_pool, mmbuf_info->size);
		if (mmbuf_info->addr < MMBUF_BASE)
			ret = false;

		dss_mmbuf_reserved_info[mmbuf_info->owner].addr = mmbuf_info->addr;
		HISI_FB_DEBUG("fb%d, service %x first alloc addr 0x%x, reserved size 0x%x",
			hisifd->index, mmbuf_info->owner, mmbuf_info->addr,mmbuf_info->size);
	}

	return ret;
}

static void mmbuf_reserved_info_clear(uint32_t addr, uint32_t size)
{
	int index;

	for (index = 0; index < RESERVED_SERVICE_MAX; index++) {
		if ((addr == dss_mmbuf_reserved_info[index].addr) && (size == dss_mmbuf_reserved_info[index].size)) {
			dss_mmbuf_reserved_info[index].addr = 0;
			HISI_FB_DEBUG("service %x release addr 0x%x, reserved size 0x%x", index, addr, size);

			return;
		}
	}
}

static void mmbuf_test(uint32_t mmbuf_size_max, dss_mmbuf_t mmbuf_info)
{
	if (g_mmbuf_addr_test >= (mmbuf_size_max + 0x40)) {
		HISI_FB_ERR("g_mmbuf_addr_test(0x%x) is overflow max mmbuf size + 0x40(0x%x)\n",
			g_mmbuf_addr_test, mmbuf_size_max + 0x40);
		HISI_FB_ERR("remain buff size if %d\n", (mmbuf_size_max + 0x40) - (g_mmbuf_addr_test - mmbuf_info.size));
		g_mmbuf_addr_test = 0;
	} else {
		mmbuf_info.addr = g_mmbuf_addr_test;
		g_mmbuf_addr_test += mmbuf_info.size;
	}

	HISI_FB_INFO("addr = 0x%x, size =%d,g_mmbuf_addr_test = 0x%x, MAX_SIZE= 0x%x\n",
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

void* hisi_dss_mmbuf_init(struct hisi_fb_data_type *hisifd)
{
	struct gen_pool *pool = NULL;
	int order = 3;
	uint32_t addr = MMBUF_BASE;
	int prev_refcount;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return NULL;
	}

	HISI_FB_DEBUG("fb%d, +\n", hisifd->index);

	hisi_mmbuf_sem_pend();

	prev_refcount = mmbuf_refcount++;
	if (prev_refcount == 0) {
		// mmbuf pool
		pool = gen_pool_create(order, 0);
		if (pool == NULL) {
			HISI_FB_ERR("fb%d, gen_pool_create failed", hisifd->index);
			goto err_out;
		}

		if (gen_pool_add(pool, addr, (size_t)mmbuf_max_size, 0) != 0) {
			gen_pool_destroy(pool);
			pool = NULL;
			HISI_FB_ERR("fb%d, gen_pool_add failed", hisifd->index);
			goto err_out;
		}

		g_mmbuf_gen_pool = pool;

		// mmbuf list
		if (g_mmbuf_list == NULL) {
			g_mmbuf_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
			if (g_mmbuf_list == NULL) {
				HISI_FB_ERR("g_mmbuf_list is NULL");
				hisi_mmbuf_sem_post();
				return NULL;
			}
			INIT_LIST_HEAD(g_mmbuf_list);
		}

		// smmu
		if (g_smmu_rwerraddr_virt == NULL) {
			g_smmu_rwerraddr_virt = kmalloc(SMMU_RW_ERR_ADDR_SIZE, GFP_KERNEL|__GFP_DMA);
			if (g_smmu_rwerraddr_virt != NULL) {
				memset(g_smmu_rwerraddr_virt, 0, SMMU_RW_ERR_ADDR_SIZE);
			} else {
				HISI_FB_ERR("kmalloc g_smmu_rwerraddr_virt fail\n");
			}
		}
	}

	hisifd->mmbuf_gen_pool = g_mmbuf_gen_pool;
	hisifd->mmbuf_list = g_mmbuf_list;

err_out:
	hisi_mmbuf_sem_post();

	HISI_FB_DEBUG("fb%d, -, mmbuf_refcount=%d\n", hisifd->index, mmbuf_refcount);

	return pool;
}

void hisi_dss_mmbuf_deinit(struct hisi_fb_data_type *hisifd)
{
	int new_refcount;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	HISI_FB_DEBUG("fb%d, +\n", hisifd->index);

	hisifb_dss_off(hisifd, false);

	hisi_mmbuf_sem_pend();
	new_refcount = --mmbuf_refcount;
	if (new_refcount < 0)
		HISI_FB_ERR("dss new_refcount err");

	if (new_refcount == 0) {
		// mmbuf pool
		if (g_mmbuf_gen_pool != NULL) {
			gen_pool_destroy(g_mmbuf_gen_pool);
			g_mmbuf_gen_pool = NULL;
		}

		// mmbuf list
		if (g_mmbuf_list != NULL) {
			kfree(g_mmbuf_list);
			g_mmbuf_list = NULL;
		}

		// smmu
		if (g_smmu_rwerraddr_virt != NULL) {
			kfree(g_smmu_rwerraddr_virt);
			g_smmu_rwerraddr_virt = NULL;
		}
	}

	hisifd->mmbuf_gen_pool = NULL;
	hisifd->mmbuf_list = NULL;
	hisi_mmbuf_sem_post();

	HISI_FB_DEBUG("fb%d, -, mmbuf_refcount=%d\n", hisifd->index, mmbuf_refcount);
}

uint32_t hisi_dss_mmbuf_alloc(void *handle, uint32_t size)
{
	uint32_t addr = 0;
	struct hisifb_mmbuf *node = NULL;
	struct hisifb_mmbuf *mmbuf_node = NULL;
	struct hisifb_mmbuf *_node_ = NULL;

	if (handle == NULL) {
		HISI_FB_ERR("handle is NULL\n");
		return addr;
	}

	if (g_mmbuf_list == NULL) {
		HISI_FB_ERR("g_mmbuf_list is NULL\n");
		return addr;
	}

	if (size == 0) {
		HISI_FB_ERR("mmbuf size is invalid, size=%d\n", size);
		return addr;
	}

	hisi_mmbuf_sem_pend();

	addr = gen_pool_alloc(handle, size);

	if (addr <= 0) {
		list_for_each_entry_safe(mmbuf_node, _node_, g_mmbuf_list, list_node) {
			HISI_FB_DEBUG("mmbuf_node_addr(0x%x), mmbuf_node_size(0x%x)\n", mmbuf_node->addr,
					mmbuf_node->size);
		}
		HISI_FB_INFO("note: mmbuf not enough,addr=0x%x\n", addr);
	} else {
		// node
		node = kzalloc(sizeof(struct hisifb_mmbuf), GFP_KERNEL);
		if (node != NULL) {
			node->addr = addr;
			node->size = size;
			list_add_tail(&node->list_node, g_mmbuf_list);
		} else {
			HISI_FB_ERR("kzalloc struct hisifb_mmbuf fail\n");
		}

		if ((addr & (MMBUF_ADDR_ALIGN - 1)) || (size & (MMBUF_ADDR_ALIGN - 1))) {
			HISI_FB_ERR("addr(0x%x) is not %d bytes aligned, or size(0x%x) is not %d bytes"
				"aligned\n", addr, MMBUF_ADDR_ALIGN, size, MMBUF_ADDR_ALIGN);

			list_for_each_entry_safe(mmbuf_node, _node_, g_mmbuf_list, list_node) {
				HISI_FB_ERR("mmbuf_node_addr(0x%x), mmbuf_node_size(0x%x)\n", mmbuf_node->addr,
					mmbuf_node->size);
			}
		}
	}

	hisi_mmbuf_sem_post();

	if (g_enable_mmbuf_debug != 0)
		HISI_FB_DEBUG("addr=0x%x, size=%d\n", addr, size);

	return addr; //lint !e429
} //lint !e429

void hisi_dss_mmbuf_free(void *handle, uint32_t addr, uint32_t size)
{
	struct hisifb_mmbuf *node = NULL;
	struct hisifb_mmbuf *_node_ = NULL;

	if (handle == NULL) {
		HISI_FB_ERR("handle is NULL\n");
		return ;
	}

	if (g_mmbuf_list == NULL) {
		HISI_FB_ERR("g_mmbuf_list is NULL\n");
		return ;
	}

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
		HISI_FB_DEBUG("addr=0x%x, size=%d\n", addr, size);
}

void hisi_dss_mmbuf_free_all(struct hisi_fb_data_type *hisifd)
{
	struct hisifb_mmbuf *node = NULL;
	struct hisifb_mmbuf *_node_ = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (g_mmbuf_list == NULL) {
		HISI_FB_ERR("g_mmbuf_list is NULL\n");
		return ;
	}

	list_for_each_entry_safe(node, _node_, g_mmbuf_list, list_node) {
		if ((node->addr > 0) && (node->size > 0)) {
			if (mmbuf_reserved_ignore(node) == true) {
				HISI_FB_INFO("fb%d, ignore reserved node addr=0x%x, size=%d\n", hisifd->index, node->addr, node->size);
				continue;
			}
			gen_pool_free(hisifd->mmbuf_gen_pool, node->addr, node->size);
			HISI_FB_DEBUG("hisi_dss_mmbuf_free, addr=0x%x, size=%d\n", node->addr, node->size);
		}

		list_del(&node->list_node);
		kfree(node);
		node = NULL;
	}
}

dss_mmbuf_info_t* hisi_dss_mmbuf_info_get(struct hisi_fb_data_type *hisifd, int idx)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return NULL;
	}
	if ((idx < 0) || (idx >= HISI_DSS_CMDLIST_DATA_MAX)) {
		HISI_FB_ERR("idx is invalid");
		return NULL;
	}

	return &(hisifd->mmbuf_infos[idx]);
}

void hisi_dss_mmbuf_info_clear(struct hisi_fb_data_type *hisifd, int idx)
{
	int i;
	dss_mmbuf_info_t *mmbuf_info = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	if ((idx < 0) || (idx >= HISI_DSS_CMDLIST_DATA_MAX)) {
		HISI_FB_ERR("idx is invalid");
		return;
	}

	// mmbuf
	mmbuf_info = &(hisifd->mmbuf_infos[idx]);
	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (mmbuf_info->mm_used[i] == 1) {
			hisi_dss_mmbuf_free(g_mmbuf_gen_pool, mmbuf_info->mm_base[i], mmbuf_info->mm_size[i]);

			if (g_debug_ovl_online_composer)
				HISI_FB_INFO("fb%d, mm_base(0x%x, %d)\n",
					hisifd->index, mmbuf_info->mm_base[i], mmbuf_info->mm_size[i]);

			mmbuf_info->mm_base[i] = 0;
			mmbuf_info->mm_size[i] = 0;
			mmbuf_info->mm_used[i] = 0;
		}
	}
}

void hisi_dss_mmbuf_on(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (g_dss_version_tag == FB_ACCEL_DSSV501)
		return;

	if (g_dss_version_tag == FB_ACCEL_DSSV510)
		return;

	if (g_dss_version_tag == FB_ACCEL_DSSV350)
		return;

	if (g_fpga_flag == 1) {
	// config according to set_pu_mmbuf
		// step 1: module mtcmos on
		outp32(hisifd->sctrl_base + 0x60, 0x8);  // mmbuf regulator enable
		udelay(200);     // at least 100us
		// step 2: module clk enable
		outp32(hisifd->sctrl_base + 0x258, 0x00c000c0);  // crg clock enable
		outp32(hisifd->sctrl_base + 0x170, 0x03c00000);
		udelay(2);     // at least 1us
		// step 3: module clk disable
		outp32(hisifd->sctrl_base + 0x174, 0x03c00000);
		outp32(hisifd->sctrl_base + 0x258, 0x00c00000);

		// step 4: module iso disable
		outp32(hisifd->sctrl_base + 0x44, 0x00000008);
		// step 5: memory repair
		// step 6: module unrst
		outp32(hisifd->sctrl_base + 0x210, 0x00001800);

		// step 7: module clk enable
		outp32(hisifd->sctrl_base + 0x258, 0x00c000c0);
		outp32(hisifd->sctrl_base + 0x170, 0x03c00000);
		// step 8: bus idle clear
	}
}

static void get_use_comm_mmbuf(int *use_comm_mmbuf,
	dss_mmbuf_t *offline_mmbuf, struct dss_comm_mmbuf_info *online_mmbuf)
{
	int i;
	int j;

	if (g_debug_dump_mmbuf) {
		for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
			HISI_FB_INFO("g_online_mmbuf[%d].addr=0x%x, size=0x%x\n",
				i, online_mmbuf[i].mmbuf.addr, online_mmbuf[i].mmbuf.size);
		}
	}

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		for (j = 0; j < DSS_CHN_MAX_DEFINE; j++) {
			if ((((offline_mmbuf[i].addr < online_mmbuf[j].mmbuf.addr + online_mmbuf[j].mmbuf.size) &&
				(offline_mmbuf[i].addr >= online_mmbuf[j].mmbuf.addr))
				|| ((online_mmbuf[j].mmbuf.addr < offline_mmbuf[i].addr + offline_mmbuf[i].size) &&
				(online_mmbuf[j].mmbuf.addr >= offline_mmbuf[i].addr)))
				&& offline_mmbuf[i].size) {
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


/*lint -e574 -e737*/
void hisi_dss_check_use_comm_mmbuf(uint32_t display_id,
	int *use_comm_mmbuf, dss_mmbuf_t *offline_mmbuf, bool has_rot)
{
	int i;

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

	if (g_debug_dump_mmbuf) {
		for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
			HISI_FB_INFO("offline_mmbuf[%d].addr=0x%x, size=0x%x, *use_comm_mmbuf=%d\n",
				i, offline_mmbuf[i].addr, offline_mmbuf[i].size, *use_comm_mmbuf);
		}
	}
}

int hisi_dss_rdma_set_mmbuf_base_and_size(struct hisi_fb_data_type *hisifd,
	dss_layer_t *layer,int ovl_idx,dss_rect_ltrb_t *afbc_rect,uint32_t *mm_base_0,uint32_t *mm_base_1)
{
	bool mm_alloc_needed = false;
	int chn_idx;
	dss_rect_t new_src_rect = {0,0,0,0};

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifb is NULL");
		return -EINVAL;
	}

	if (afbc_rect == NULL) {
		HISI_FB_ERR("afbc_rect is NULL");
		return -EINVAL;
	}

	if (mm_base_0 == NULL) {
		HISI_FB_ERR("mm_base_0 is NULL");
		return -EINVAL;
	}

	if (mm_base_1 == NULL) {
		HISI_FB_ERR("mm_base_0 is NULL");
		return -EINVAL;
	}

	chn_idx = layer->chn_idx;
	new_src_rect = layer->src_rect;
	if ((layer->img.mmbuf_base > 0) && (layer->img.mmbuf_size > 0)) {
		*mm_base_0 = layer->img.mmbuf_base;
		*mm_base_1 = layer->img.mmbuf_base + layer->img.mmbuf_size / 2;
	} else {
		if (hisifd->mmbuf_info == NULL) {
			HISI_FB_ERR("hisifd->mmbuf_info is NULL");
			return -EINVAL;
		}

		if (ovl_idx <= DSS_OVL1) {
			mm_alloc_needed = true;
		} else {
			if (hisifd->mmbuf_info->mm_used[chn_idx] == 1) {
				mm_alloc_needed = false;
			}
			else {
				mm_alloc_needed = true;
			}
		}

		if (mm_alloc_needed) {
			afbc_rect->left = ALIGN_DOWN((uint32_t)new_src_rect.x, MMBUF_ADDR_ALIGN);
			afbc_rect->right = ALIGN_UP((uint32_t)(new_src_rect.x - afbc_rect->left + new_src_rect.w), MMBUF_ADDR_ALIGN);

			hisifd->mmbuf_info->mm_size[chn_idx] = afbc_rect->right * layer->img.bpp * MMBUF_LINE_NUM;
			hisifd->mmbuf_info->mm_base[chn_idx] = hisi_dss_mmbuf_alloc(g_mmbuf_gen_pool,
				hisifd->mmbuf_info->mm_size[chn_idx]);
			if (hisifd->mmbuf_info->mm_base[chn_idx] < MMBUF_BASE) {
				HISI_FB_ERR("fb%d, chn%d failed to alloc mmbuf, mm_base=0x%x\n",
					hisifd->index, chn_idx, hisifd->mmbuf_info->mm_base[chn_idx]);
				return -EINVAL;
			}
		}

		*mm_base_0 = hisifd->mmbuf_info->mm_base[chn_idx];
		*mm_base_1 = hisifd->mmbuf_info->mm_base[chn_idx] +
			hisifd->mmbuf_info->mm_size[chn_idx] / 2;
		hisifd->mmbuf_info->mm_used[chn_idx] = 1;
	}

	*mm_base_0 -= MMBUF_BASE;
	*mm_base_1 -= MMBUF_BASE;

	return 0;
}
/*lint +e574 +e737*/

void hisi_mmbuf_info_get_online(struct hisi_fb_data_type *hisifd)
{
	int tmp;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	tmp = (hisifd->frame_count + 1) % HISI_DSS_CMDLIST_DATA_MAX;
	hisi_dss_mmbuf_info_clear(hisifd, tmp);

	tmp = hisifd->frame_count % HISI_DSS_CMDLIST_DATA_MAX;
	hisifd->mmbuf_info = &(hisifd->mmbuf_infos[tmp]);
}

int hisi_mmbuf_reserved_size_query(struct fb_info *info, void __user *argp)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_mmbuf_t reserved_mmbuf_info;

	if (info == NULL) {
		HISI_FB_ERR("reserved mmbuf size query info NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par; //lint !e838
	if (hisifd == NULL) {
		HISI_FB_ERR("reserved mmbuf size query hisifd NULL Pointer\n");
		return -EINVAL;
	}

	if (argp == NULL) {
		HISI_FB_ERR("reserved mmbuf size query argp NULL Pointer\n");
		return -EINVAL;
	}

	ret = copy_from_user(&reserved_mmbuf_info, argp, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		HISI_FB_ERR("fb%d, copy for user failed!ret=%d\n", hisifd->index, ret);
		return -EINVAL;
	}

	if (reserved_mmbuf_info.owner < 0 || reserved_mmbuf_info.owner >= RESERVED_SERVICE_MAX)
	{
		HISI_FB_ERR("reserved mmbuf owner invalid\n");
		return -EINVAL;
	}

	reserved_mmbuf_info.size = dss_mmbuf_reserved_info[reserved_mmbuf_info.owner].size;

	ret = copy_to_user(argp, &reserved_mmbuf_info, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		HISI_FB_ERR("fb%d, copy to user failed!ret=%d", hisifd->index, ret);
		return -EFAULT;
	}

	HISI_FB_DEBUG("fb%d, service %d get reserved mmbuf size 0x%x\n", hisifd->index, reserved_mmbuf_info.owner, reserved_mmbuf_info.size);

	return 0;
}

int hisi_mmbuf_request(struct fb_info *info, void __user *argp)
{
	int ret;
	uint32_t mmbuf_size_max;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_mmbuf_t mmbuf_info;

	if (info == NULL) {
		HISI_FB_ERR("dss mmbuf alloc info NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par; //lint !e838
	if (hisifd == NULL) {
		HISI_FB_ERR("dss mmbuf alloc hisifd NULL Pointer\n");
		return -EINVAL;
	}

	if (argp == NULL) {
		HISI_FB_ERR("dss mmbuf alloc argp NULL Pointer\n");
		return -EINVAL;
	}

	ret = copy_from_user(&mmbuf_info, argp, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		HISI_FB_ERR("fb%d, copy for user failed!ret=%d\n", hisifd->index, ret);
		ret = -EINVAL;
		goto err_out;
	}

	mmbuf_size_max = mmbuf_max_size;

	if (mmbuf_info.owner < 0 || mmbuf_info.owner >= SERVICE_MAX) {
		HISI_FB_ERR("mmbuf owner %d is invalid\n", mmbuf_info.owner);
		return -EINVAL;
	}

	/*lint -e574 -e737*/
	if ((mmbuf_info.size <= 0) || (mmbuf_info.size > mmbuf_size_max) || (mmbuf_info.size & (MMBUF_ADDR_ALIGN - 1))) {
		HISI_FB_ERR("fb%d, mmbuf size is invalid, size=%d\n", hisifd->index, mmbuf_info.size);
		ret = -EINVAL;
		goto err_out;
	}
	/*lint +e574 +e737*/

	if (g_mmbuf_addr_test > 0)
		mmbuf_test(mmbuf_size_max, mmbuf_info);

	if (g_mmbuf_addr_test == 0) {
		if (IS_MMBUF_RESERVED_SERVICE(mmbuf_info.owner)) { // alloc reserved mmbuf
			if (mmbuf_reserved_alloc(hisifd, &mmbuf_info) == false) {
				ret = -EINVAL;
				goto err_out;
			}
		} else {
			mmbuf_info.addr = hisi_dss_mmbuf_alloc(hisifd->mmbuf_gen_pool, mmbuf_info.size);
			if (mmbuf_info.addr < MMBUF_BASE) {
				ret = -EINVAL;
				goto err_out;
			}
		}
	}

	ret = copy_to_user(argp, &mmbuf_info, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		HISI_FB_ERR("fb%d, copy to user failed!ret=%d", hisifd->index, ret);
		hisi_dss_mmbuf_free(hisifd->mmbuf_gen_pool, mmbuf_info.addr, mmbuf_info.size);
		ret = -EFAULT;
		goto err_out;
	}

	return 0;

err_out:
	return ret;
}

int hisi_mmbuf_release(struct fb_info *info, const void __user *argp)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_mmbuf_t mmbuf_info;

	if (info == NULL) {
		HISI_FB_ERR("dss mmbuf free info NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par; //lint !e838
	if (hisifd == NULL) {
		HISI_FB_ERR("dss mmbuf free  hisifd NULL Pointer\n");
		return -EINVAL;
	}

	if (argp == NULL) {
		HISI_FB_ERR("dss mmbuf free  argp NULL Pointer\n");
		return -EINVAL;
	}

	//lint -save -e775 -e732
	ret = copy_from_user(&mmbuf_info, argp, sizeof(dss_mmbuf_t));
	if (ret != 0) {
		HISI_FB_ERR("fb%d, copy for user failed!ret=%d", hisifd->index, ret);
		ret = -EINVAL;
		goto err_out;
	}

	if ((mmbuf_info.addr <= 0) || (mmbuf_info.size <= 0)) {
		HISI_FB_ERR("fb%d, addr=0x%x, size=%d is invalid\n",
			hisifd->index, mmbuf_info.addr, mmbuf_info.size);
		ret = -EINVAL;
		goto err_out;
	}

	hisi_dss_mmbuf_free(hisifd->mmbuf_gen_pool, mmbuf_info.addr, mmbuf_info.size);
	//lint -restore

	return 0;

err_out:
	return ret;
}

int hisi_mmbuf_free_all(struct fb_info *info, const void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	if (info == NULL) {
		HISI_FB_ERR("fb info is NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par; //lint !e838
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Pointer\n");
		return -EINVAL;
	}

	/* argp can be set to mmbuf owner, but now it isn't useful */
	(void)argp;

	hisi_dss_mmbuf_free_all(hisifd);
	return 0;
}

/*lint +e502 +e737 +e570 +e651 +e666 +e712 +e713 +e775 +e778 +e413*/
