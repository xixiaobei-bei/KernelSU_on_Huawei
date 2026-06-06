/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#include "npu_common.h"
#include "drv_log.h"

static struct devdrv_dev_ctx *g_dev_ctxs[NPU_DEV_NUM];

void dev_ctx_array_init(void)
{
	int i = 0;
	for (i = 0; i < NPU_DEV_NUM; i++) {
		g_dev_ctxs[i] = NULL;
	}
}

void set_dev_ctx_with_dev_id(struct devdrv_dev_ctx *dev_ctx, u8 dev_id)
{
	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %u\n", dev_id);
		return;
	}

	g_dev_ctxs[dev_id] = dev_ctx;
}

struct devdrv_dev_ctx *get_dev_ctx_by_id(u8 dev_id)
{
	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return NULL;
	}

	return g_dev_ctxs[dev_id];
}

int devdrv_add_proc_ctx(struct list_head *proc_ctx, u8 dev_id)
{
	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null \n");
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (g_dev_ctxs[dev_id] == NULL) {
		devdrv_drv_err(" npu dev %d context is null\n", dev_id);
		return -1;
	}

	spin_lock(&g_dev_ctxs[dev_id]->spinlock);
	list_add(proc_ctx, &g_dev_ctxs[dev_id]->proc_ctx_list);
	spin_unlock(&g_dev_ctxs[dev_id]->spinlock);

	return 0;
}

int devdrv_add_proc_ctx_to_rubbish_ctx_list(struct list_head *proc_ctx,
					    u8 dev_id)
{
	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null \n");
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (g_dev_ctxs[dev_id] == NULL) {
		devdrv_drv_err(" npu dev %d context is null\n", dev_id);
		return -1;
	}

	spin_lock(&g_dev_ctxs[dev_id]->spinlock);
	list_add(proc_ctx, &g_dev_ctxs[dev_id]->rubbish_context_list);
	spin_unlock(&g_dev_ctxs[dev_id]->spinlock);

	return 0;
}

int devdrv_remove_proc_ctx(struct list_head *proc_ctx, u8 dev_id)
{
	struct list_head *pos = NULL, *n = NULL;

	if (proc_ctx == NULL) {
		devdrv_drv_err("proc_ctx is null \n");
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		devdrv_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (g_dev_ctxs[dev_id] == NULL) {
		devdrv_drv_err(" npu dev %d context is null\n", dev_id);
		return -1;
	}

	if (list_empty_careful(&g_dev_ctxs[dev_id]->proc_ctx_list)) {
		devdrv_drv_debug("g_dev_ctxs npu dev id %d pro_ctx_list is"
				 " null ,no need to remove any more\n", dev_id);
		return 0;
	}

	spin_lock(&g_dev_ctxs[dev_id]->spinlock);
	list_for_each_safe(pos, n, &g_dev_ctxs[dev_id]->proc_ctx_list) {

		if (proc_ctx == pos) {
			pos->prev->next = n;
			n->prev = pos->prev;
			list_del(pos);
			break;
		}

	}
	spin_unlock(&g_dev_ctxs[dev_id]->spinlock);

	devdrv_drv_debug("remove g_dev_ctxs npu dev id %d pro_ctx_list\n",
			 dev_id);

	return 0;
}
