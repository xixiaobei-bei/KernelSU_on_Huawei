/*
 * npu_hwts.c
 *
 * about npu hwts
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
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <securec.h>

#include "npu_user_common.h"
#include "npu_common.h"
#include "npu_log.h"
#include "npu_devinit.h"
#include "npu_proc_ctx.h"
#include "npu_shm.h"
#include "npu_hwts.h"

#include "npu_pool.h"

int npu_dev_ctx_sub_init(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_dev_ctx_sub *cur_dev_ctx_sub = NULL;
	int ret;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return -1;
	}

	cur_dev_ctx_sub = kzalloc(sizeof(struct npu_dev_ctx_sub), GFP_KERNEL);
	if (cur_dev_ctx_sub == NULL) {
		npu_drv_err("kzalloc cur_dev_ctx_sub failed, devid:%u\n", dev_id);
		return -ENOMEM;
	}

	// inherit key info from dev_ctx
	cur_dev_ctx_sub->devid = dev_id;

	(void)memset_s(cur_dev_ctx_sub->resv, sizeof(cur_dev_ctx_sub->resv), 0,
		sizeof(cur_dev_ctx_sub->resv));

	ret = npu_hwts_sq_init(dev_id); // npu_hwts_sq_manager_init(dev_id);
	if (ret != 0) {
		kfree(cur_dev_ctx_sub);
		cur_dev_ctx_sub = NULL;
		npu_drv_err("sub devdrv hwts sq init failed, devid:%u, ret:%d\n",
			dev_id, ret);
		return ret;
	}

	ret = npu_hwts_cq_init(dev_id);
	if (ret != 0) {
		kfree(cur_dev_ctx_sub);
		cur_dev_ctx_sub = NULL;
		npu_drv_err("sub devdrv hwts cq init failed, devid:%u, ret:%d\n",
			dev_id, ret);
		return ret;
	}
	mutex_init(&cur_dev_ctx_sub->hwts_cq_mutex_t);
	cur_dev_ctx->dev_ctx_sub = cur_dev_ctx_sub;

	return 0;
}

void npu_dev_ctx_sub_destroy(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_dev_ctx_sub *cur_dev_ctx_sub = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id\n");
		return;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx is null\n");
		return;
	}

	cur_dev_ctx_sub = (struct npu_dev_ctx_sub *)cur_dev_ctx->dev_ctx_sub;
	if (cur_dev_ctx_sub == NULL) {
		npu_drv_err("cur_dev_ctx_sub is NULL, devid:%u\n", dev_id);
		return;
	}

	(void)npu_hwts_sq_destroy(dev_id);
	(void)npu_hwts_cq_destroy(dev_id);

	mutex_destroy(&cur_dev_ctx_sub->hwts_cq_mutex_t);

	kfree(cur_dev_ctx_sub);
	cur_dev_ctx->dev_ctx_sub = NULL;
}

int npu_proc_ctx_sub_init(struct npu_proc_ctx *proc_ctx)
{
	struct npu_proc_ctx_sub *proc_ctx_sub = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -EINVAL, "npu_plat_get_info\n");
	cond_return_error(proc_ctx == NULL, -EINVAL, "npu_plat_get_info\n");

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0) {
		proc_ctx->proc_ctx_sub = NULL;
		return 0;
	}

	proc_ctx_sub = kzalloc(sizeof(struct npu_proc_ctx_sub), GFP_KERNEL);
	if (proc_ctx_sub == NULL) {
		npu_drv_err("alloc memory for proc_ctx_sub failed\n");
		return -ENOMEM;
	}

	// inherit key info from proc_ctx
	proc_ctx_sub->pid = proc_ctx->pid;
	proc_ctx_sub->devid = proc_ctx->devid;
	proc_ctx_sub->hwts_cq_num = 0;

	(void)memset_s(proc_ctx_sub->resv, sizeof(proc_ctx_sub->resv), 0,
		sizeof(proc_ctx_sub->resv));

	INIT_LIST_HEAD(&proc_ctx_sub->hwts_cq_list);
	proc_ctx->proc_ctx_sub = proc_ctx_sub;
	return 0;
}

void npu_proc_ctx_sub_destroy(struct npu_proc_ctx *proc_ctx)
{
	if ((proc_ctx == NULL) || (proc_ctx->proc_ctx_sub == NULL)) {
		npu_drv_err("proc_ctx or proc_ctx_sub is NULL\n");
		return;
	}
	kfree(proc_ctx->proc_ctx_sub);
	proc_ctx->proc_ctx_sub = NULL;
}

int npu_proc_alloc_proc_ctx_sub(struct npu_proc_ctx *proc_ctx)
{
	struct npu_hwts_cq_info *hwts_cq_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_dev_ctx_sub *cur_dev_ctx_sub = NULL;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is NULL\n");
		return -1;
	}
	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx of dev: %d is null\n", proc_ctx->devid);
		return -1;
	}
	cur_dev_ctx_sub = cur_dev_ctx->dev_ctx_sub;
	if (cur_dev_ctx_sub == NULL) {
		npu_drv_err("cur_dev_ctx_sub of dev: %d is null\n", proc_ctx->devid);
		return -1;
	}

	// alloc hwts_cq for current process
	mutex_lock(&cur_dev_ctx_sub->hwts_cq_mutex_t);
	hwts_cq_info = npu_proc_alloc_hwts_cq(proc_ctx);
	if (hwts_cq_info == NULL) {
		mutex_unlock(&cur_dev_ctx_sub->hwts_cq_mutex_t);
		npu_drv_err("alloc hwts_cq for proc_context failed\n");
		return -1;
	}
	mutex_unlock(&cur_dev_ctx_sub->hwts_cq_mutex_t);
	npu_drv_debug("alloc proc_ctx_sub for proc_context success\n");

	return 0;
}

int npu_proc_free_proc_ctx_sub(struct npu_proc_ctx *proc_ctx)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_dev_ctx_sub *cur_dev_ctx_sub = NULL;
	int ret;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is NULL\n");
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx of dev: %d is null\n", proc_ctx->devid);
		return -1;
	}

	cur_dev_ctx_sub = cur_dev_ctx->dev_ctx_sub;
	if (cur_dev_ctx_sub == NULL) {
		npu_drv_err("cur_dev_ctx_sub of dev: %d is null\n", proc_ctx->devid);
		return -1;
	}

	mutex_lock(&cur_dev_ctx_sub->hwts_cq_mutex_t);
	ret = npu_proc_free_hwts_cq(proc_ctx);
	if (ret != 0) {
		mutex_unlock(&cur_dev_ctx_sub->hwts_cq_mutex_t);
		npu_drv_err("proc free hwts_cq failed, devid:%d\n", proc_ctx->devid);
		return ret;
	}
	mutex_unlock(&cur_dev_ctx_sub->hwts_cq_mutex_t);

	return 0;
}

void npu_recycle_proc_ctx_sub(struct npu_proc_ctx *proc_ctx)
{
	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return;
	}

	(void)npu_proc_free_proc_ctx_sub(proc_ctx);
}

int npu_proc_get_hwts_cq_id(struct npu_proc_ctx *proc_ctx,
	u32 *hwts_cq_id)
{
	struct npu_entity_info *hwts_cq_sub = NULL;
	struct npu_proc_ctx_sub *proc_ctx_sub = NULL;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is NULL\n");
		return -1;
	}

	proc_ctx_sub = proc_ctx->proc_ctx_sub;
	if (proc_ctx_sub == NULL) {
		npu_drv_err("cur proc_ctx_sub of dev:%d is null\n", proc_ctx->devid);
		return -1;
	}
	if (list_empty_careful(&proc_ctx_sub->hwts_cq_list)) {
		npu_drv_err("cur proc_ctx_sub hwts_cq_list null\n");
		return -1;
	}

	hwts_cq_sub = list_first_entry(&proc_ctx_sub->hwts_cq_list,
		struct npu_entity_info, list);

	*hwts_cq_id = hwts_cq_sub->index;
	return 0;
}

struct npu_hwts_cq_info *npu_proc_alloc_hwts_cq(
	struct npu_proc_ctx *proc_ctx)
{
	struct npu_proc_ctx_sub *proc_ctx_sub = NULL;
	struct npu_hwts_cq_info *hwts_cq_info = NULL;
	struct npu_entity_info *hwts_cq_sub_info = NULL;
	int hwts_cq_id;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return NULL;
	}
	proc_ctx_sub = proc_ctx->proc_ctx_sub;
	if (proc_ctx_sub == NULL) {
		npu_drv_err("proc_ctx_sub is null\n");
		return NULL;
	}
	hwts_cq_id = npu_alloc_hwts_cq(proc_ctx->devid);
	npu_drv_debug("alloc hwts_cq, hwts_cq_id = %d\n", hwts_cq_id);

	hwts_cq_info = npu_calc_hwts_cq_info(proc_ctx->devid, hwts_cq_id);
	if (hwts_cq_info == NULL) {
		npu_drv_err("npu dev %d hwts_cq_info is null\n", proc_ctx->devid);
		return NULL;
	}

	hwts_cq_sub_info = (struct npu_entity_info *)(uintptr_t)(
		hwts_cq_info->hwts_cq_sub);
	list_add(&hwts_cq_sub_info->list, &proc_ctx_sub->hwts_cq_list);
	proc_ctx_sub->hwts_cq_num++;

	return hwts_cq_info;
}

int npu_proc_free_hwts_cq(struct npu_proc_ctx *proc_ctx)
{
	struct npu_proc_ctx_sub *proc_ctx_sub = NULL;
	struct npu_entity_info *hwts_cq_sub = NULL;
	u32 hwts_cq_id;

	if (proc_ctx == NULL) {
		npu_drv_err("proc_ctx is null\n");
		return -1;
	}

	proc_ctx_sub = proc_ctx->proc_ctx_sub;
	if (proc_ctx_sub == NULL) {
		npu_drv_err("proc_ctx_sub is null\n");
		return -1;
	}
	if (list_empty_careful(&proc_ctx_sub->hwts_cq_list)) {
		npu_drv_err("cur process %d hwts_cq_list empty, left cq_num = %d !!!\n",
			proc_ctx->pid, proc_ctx_sub->hwts_cq_num);
		return -1;
	}

	hwts_cq_sub = list_first_entry(&proc_ctx_sub->hwts_cq_list,
		struct npu_entity_info, list);
	hwts_cq_id = hwts_cq_sub->index;

	if (proc_ctx_sub->hwts_cq_num == 0) {
		npu_drv_err("cq_num is 0\n");
		return -1;
	}

	list_del(&hwts_cq_sub->list);
	proc_ctx_sub->hwts_cq_num--;

	npu_free_hwts_cq(proc_ctx->devid, hwts_cq_id);

	return 0;
}

int npu_bind_stream_with_hwts_sq(u8 dev_id, u32 stream_id, u32 hwts_sq_id)
{
	struct npu_stream_info *stream_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id:%d\n", dev_id);
		return -1;
	}

	if (stream_id < NPU_MAX_NON_SINK_STREAM_ID) {
		npu_drv_err("illegal npu stream id:%d\n", stream_id);
		return -1;
	}

	if (hwts_sq_id >= NPU_MAX_HWTS_SQ_NUM) {
		npu_drv_err("illegal hwts_sq id %u\n", hwts_sq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}

	spin_lock(&cur_dev_ctx->spinlock);
	stream_info = npu_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("stream_info is null, stream_id :%u\n", stream_id);
		return -1;
	}
	stream_info->sq_index = hwts_sq_id;
	spin_unlock(&cur_dev_ctx->spinlock);

	return 0;
}

int npu_bind_stream_with_hwts_cq(u8 dev_id, u32 stream_id, u32 hwts_cq_id)
{
	struct npu_stream_info *stream_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	if (dev_id >= NPU_DEV_NUM) {
		npu_drv_err("illegal npu dev id:%d\n", dev_id);
		return -1;
	}

	if (stream_id < NPU_MAX_NON_SINK_STREAM_ID) {
		npu_drv_err("illegal npu stream id:%d\n", stream_id);
		return -1;
	}

	if (hwts_cq_id >= NPU_MAX_CQ_NUM) {
		npu_drv_err("illegal hwts_cq id %u\n", hwts_cq_id);
		return -1;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		npu_drv_err("cur_dev_ctx %d is null\n", dev_id);
		return -1;
	}
	spin_lock(&cur_dev_ctx->spinlock);
	stream_info = npu_calc_stream_info(dev_id, stream_id);
	if (stream_info == NULL) {
		spin_unlock(&cur_dev_ctx->spinlock);
		npu_drv_err("stream_info is null, stream_id :%u\n", stream_id);
		return -1;
	}
	stream_info->cq_index = hwts_cq_id;
	spin_unlock(&cur_dev_ctx->spinlock);
	return 0;
}
