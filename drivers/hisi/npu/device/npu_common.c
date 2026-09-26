/*
 * npu_common.c
 *
 * about npu common
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_common.h"

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

#include "npu_log.h"

static struct npu_dev_ctx *g_dev_ctxs[NPU_DEV_NUM];

void dev_ctx_array_init(void)
{
	int i;

	for (i = 0; i < NPU_DEV_NUM; i++)
		g_dev_ctxs[i] = NULL;
}

void set_dev_ctx_with_dev_id(struct npu_dev_ctx *dev_ctx, u8 dev_id)
{
	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %u\n", dev_id);
		return;
	}

	g_dev_ctxs[dev_id] = dev_ctx;
}

struct npu_dev_ctx *get_dev_ctx_by_id(u8 dev_id)
{
	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return NULL;
	}

	return g_dev_ctxs[dev_id];
}

int npu_add_proc_ctx(struct list_head *proc_ctx, u8 dev_id)
{
	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (g_dev_ctxs[dev_id] == NULL) {
		npu_drv_err(" npu dev %d context is null\n", dev_id);
		return -1;
	}

	spin_lock(&g_dev_ctxs[dev_id]->spinlock);
	list_add(proc_ctx, &g_dev_ctxs[dev_id]->proc_ctx_list);
	spin_unlock(&g_dev_ctxs[dev_id]->spinlock);

	return 0;
}

int npu_remove_proc_ctx(const struct list_head *proc_ctx, u8 dev_id)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return -1;
	}

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %d\n", dev_id);
		return -1;
	}

	if (g_dev_ctxs[dev_id] == NULL) {
		npu_drv_err(" npu dev %d context is null\n", dev_id);
		return -1;
	}

	if (list_empty_careful(&g_dev_ctxs[dev_id]->proc_ctx_list)) {
		npu_drv_debug("g_dev_ctxs npu dev id %d pro_ctx_list is"
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
	npu_drv_debug("remove g_dev_ctxs npu dev id %d pro_ctx_list\n", dev_id);

	return 0;
}

void npu_set_sec_stat(u8 dev_id, u32 state)
{
	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %u\n", dev_id);
		return;
	}
	npu_drv_warn("set npu dev %u secure state = %u", dev_id, state);
	g_dev_ctxs[dev_id]->pm.work_mode = state;
}

u32 npu_get_sec_stat(u8 dev_id)
{
	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id %u\n", dev_id);
		return NPU_WORKMODE_MAX;
	}

	return g_dev_ctxs[dev_id]->pm.work_mode;
}

int copy_from_user_safe(void *to, const void __user *from, unsigned long n)
{
	if (from == NULL || n == 0) {
		npu_drv_err("user pointer is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(to, (void *)from, n))
		return -ENODEV;
	return 0;
}

int copy_to_user_safe(void __user *to, const void *from, unsigned long len)
{
	if (to == NULL || len == 0) {
		npu_drv_err("user pointer is NULL or len(%lu) is 0\n", len);
		return -EINVAL;
	}

	if (copy_to_user(to, (void *)from, len))
		return -ENODEV;
	return 0;
}

