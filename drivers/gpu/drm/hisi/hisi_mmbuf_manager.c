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

#include <drm/drmP.h>

#include "hisi_mmbuf_manager.h"
#include "hisi_drm_dpe_utils.h"

static uint32_t g_mmbuf_max_size = 0x5a000; /* 360k */
static struct list_head *g_mmbuf_list;
/* mmbuf gen pool */
static struct gen_pool *g_mmbuf_gen_pool;
static bool g_mmbuf_intialized;

DEFINE_SEMAPHORE(hisi_dss_mmbuf_sem);

struct hisifb_mmbuf {
	struct list_head list_node;
	uint32_t addr;
	uint32_t size;
};

int hisi_dss_mmbuf_init(void)
{
	struct gen_pool *pool = NULL;
	const int order = 3;
	const uint32_t addr = MMBUF_BASE;

	HISI_DRM_DEBUG("+");

	down(&hisi_dss_mmbuf_sem);
	if (g_mmbuf_intialized == true) {
		HISI_DRM_ERR("mmbuffer already inited");
		goto err_out;
	}

	// mmbuf pool
	pool = gen_pool_create(order, 0);
	if (pool == NULL) {
		HISI_DRM_ERR("gen_pool_create failed");
		goto err_out;
	}

	if (gen_pool_add(pool, addr, (size_t)g_mmbuf_max_size, 0) != 0) {
		HISI_DRM_ERR("gen_pool_add failed");
		goto err_deinit_out;
	}

	// mmbuf list
	if (g_mmbuf_list == NULL) {
		g_mmbuf_list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
		if (g_mmbuf_list == NULL) {
			HISI_DRM_ERR("g_mmbuf_list is NULL");
			goto err_deinit_out;
		}
		INIT_LIST_HEAD(g_mmbuf_list);
	}

	g_mmbuf_gen_pool = pool;
	g_mmbuf_intialized = true;

	HISI_DRM_DEBUG("-");

	up(&hisi_dss_mmbuf_sem);
	return 0;

err_deinit_out:
	gen_pool_destroy(pool);
err_out:
	up(&hisi_dss_mmbuf_sem);
	return -EINVAL;
}

void hisi_dss_mmbuf_deinit(void)
{
	HISI_DRM_DEBUG("mmbuf_deinit, +\n");

	down(&hisi_dss_mmbuf_sem);
	if (g_mmbuf_intialized == false) {
		up(&hisi_dss_mmbuf_sem);
		HISI_DRM_ERR("mmbuffer already deinited");
		return;
	}

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

	g_mmbuf_intialized = false;
	g_mmbuf_gen_pool = NULL;
	up(&hisi_dss_mmbuf_sem);

	HISI_DRM_DEBUG("mmbuf_deinit, -\n");
}

uint32_t hisi_dss_mmbuf_alloc(uint32_t size)
{
	uint32_t addr = 0;
	struct hisifb_mmbuf *node = NULL;
	struct hisifb_mmbuf *mmbuf_node = NULL;
	struct hisifb_mmbuf *_node_ = NULL;

	if (size == 0) {
		HISI_DRM_ERR("mmbuf size is invalid, size=%d\n", size);
		return addr;
	}

	down(&hisi_dss_mmbuf_sem);
	if (g_mmbuf_intialized == false) {
		HISI_DRM_ERR("mmbuffer is not inited");
		goto err_out;
	}

	addr = gen_pool_alloc(g_mmbuf_gen_pool, size);
	if (addr == 0) {
		list_for_each_entry_safe(mmbuf_node, _node_, g_mmbuf_list, list_node) {
			HISI_DRM_DEBUG("mmbuf_node_addr(0x%x), mmbuf_node_size(0x%x)\n",
				mmbuf_node->addr, mmbuf_node->size);
		}
		HISI_DRM_INFO("note: mmbuf not enough,addr=0x%x\n", addr);
			goto err_out;
	}

	// node
	node = kzalloc(sizeof(struct hisifb_mmbuf), GFP_KERNEL);
	if (!node) {
		gen_pool_free(g_mmbuf_gen_pool, addr, size);
		addr = 0;
		HISI_DRM_ERR("kzalloc struct hisifb_mmbuf fail\n");
	} else {
		node->addr = addr;
		node->size = size;
		list_add_tail(&node->list_node, g_mmbuf_list);
	}

	if ((addr & (MMBUF_ADDR_ALIGN - 1)) || (size & (MMBUF_ADDR_ALIGN - 1))) {
		HISI_DRM_INFO("addr(0x%x) is not %d bytes aligned, or size(0x%x) is not %d bytes aligned\n",
			addr, MMBUF_ADDR_ALIGN, size, MMBUF_ADDR_ALIGN);

		list_for_each_entry_safe(mmbuf_node, _node_, g_mmbuf_list, list_node) {
			HISI_DRM_INFO("mmbuf_node_addr(0x%x), mmbuf_node_size(0x%x)\n",
				mmbuf_node->addr, mmbuf_node->size);
		}
	}

err_out:
	up(&hisi_dss_mmbuf_sem);

	return addr;
}

void hisi_dss_mmbuf_free(uint32_t addr)
{
	struct hisifb_mmbuf *node = NULL;
	struct hisifb_mmbuf *_node_ = NULL;

	down(&hisi_dss_mmbuf_sem);
	if (g_mmbuf_intialized == false) {
		HISI_DRM_ERR("mmbuffer is not inited");
		up(&hisi_dss_mmbuf_sem);
		return;
	}

	list_for_each_entry_safe(node, _node_, g_mmbuf_list, list_node) {
		if (node->addr == addr) {
			gen_pool_free(g_mmbuf_gen_pool, addr, node->size);
			list_del(&node->list_node);
			kfree(node);
			node = NULL;
			up(&hisi_dss_mmbuf_sem);
			return;
		}
	}

	up(&hisi_dss_mmbuf_sem);

	HISI_DRM_ERR("mmbuffer free failed");
}
