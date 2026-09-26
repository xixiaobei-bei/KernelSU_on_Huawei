/*
 * npu_sink_stream.c
 *
 * about npu sink stream
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

#include "npu_user_common.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_log.h"
#include "npu_platform.h"

int npu_sink_stream_list_init(struct npu_dev_ctx *dev_ctx)
{
	int ret;
	u32 i;
	struct npu_stream_info *sink_stream_info = NULL;
	struct npu_id_allocator *id_allocator_short = NULL;
	struct npu_id_allocator *id_allocator_long = NULL;
	struct npu_platform_info *plat_info = NULL;
	u32 sink_stream_num = NPU_MAX_SINK_SHORT_STREAM_ID +
		NPU_MAX_SINK_LONG_STREAM_ID;

	cond_return_error(dev_ctx == NULL, -ENODATA, "dev_ctx is null\n");

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info failed\n");
		return -EINVAL;
	}

	id_allocator_short = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_STREAM]);
	ret = npu_id_allocator_create(
		id_allocator_short,
		NPU_MAX_NON_SINK_STREAM_ID + NPU_MAX_SINK_LONG_STREAM_ID,
		NPU_MAX_SINK_SHORT_STREAM_ID,
		0
	);
	if (ret != 0) {
		npu_drv_err("sink stream id regist failed\n");
		return -1;
	}

	if (NPU_MAX_SINK_LONG_STREAM_ID > 0) {
		id_allocator_long =
			&(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_LONG_STREAM]);
		ret = npu_id_allocator_create(
			id_allocator_long,
			NPU_MAX_NON_SINK_STREAM_ID,
			NPU_MAX_SINK_LONG_STREAM_ID,
			0
		);
		if (ret != 0) {
			npu_id_allocator_destroy(id_allocator_short);
			npu_drv_err("sink long stream id regist failed\n");
			return -1;
		}
	}

	for (i = 0; i < sink_stream_num; i++) {
		sink_stream_info = npu_calc_stream_info(dev_ctx->devid,
			i + NPU_MAX_NON_SINK_STREAM_ID);
		cond_return_error(sink_stream_info == NULL, -EINVAL,
			"npu_plat_get_sink_stream_info is null\n");

		sink_stream_info->id = i + NPU_MAX_NON_SINK_STREAM_ID;
		sink_stream_info->devid = dev_ctx->devid;
		sink_stream_info->cq_index = (u32) NPU_CQSQ_INVALID_INDEX;
		sink_stream_info->sq_index = (u32) NPU_CQSQ_INVALID_INDEX;
		sink_stream_info->smmu_substream_id = 0;
		sink_stream_info->priority = 0;
	}

	return 0;
}

struct npu_id_entity *npu_get_sink_stream_sub_addr(
	struct npu_dev_ctx *dev_ctx, u32 stream_id)
{
	struct npu_id_entity *stream_sub = NULL;
	struct npu_id_allocator *id_allocator = NULL;
	struct npu_id_allocator *id_allocator_short = NULL;
	struct npu_id_allocator *id_allocator_long = NULL;
	u32 entity_idx;
	u32 sink_idx;

	if ((stream_id >= NPU_MAX_STREAM_ID) ||
		(stream_id < NPU_MAX_NON_SINK_STREAM_ID)) {
		npu_drv_err("sink stream id %u is invalid\n", stream_id);
		return NULL;
	}

	id_allocator_short = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_STREAM]);
	if (NPU_MAX_SINK_LONG_STREAM_ID > 0) {
		npu_drv_info("stream_id %u max stream_id %d\n",
			stream_id, NPU_MAX_SINK_LONG_STREAM_ID);
		id_allocator_long = &(dev_ctx->id_allocator[NPU_ID_TYPE_SINK_LONG_STREAM]);
	}

	sink_idx = stream_id - NPU_MAX_NON_SINK_STREAM_ID;
	id_allocator = sink_idx < NPU_MAX_SINK_LONG_STREAM_ID ?
		id_allocator_long : id_allocator_short;
	entity_idx = sink_idx < NPU_MAX_SINK_LONG_STREAM_ID ? sink_idx :
		(sink_idx - NPU_MAX_SINK_LONG_STREAM_ID);
	if (id_allocator != NULL && id_allocator->id_entity_base_addr != NULL)
		stream_sub = (struct npu_id_entity *)(
			(uintptr_t)id_allocator->id_entity_base_addr +
			id_allocator->entity_size * entity_idx);
	if (stream_sub == NULL) {
		npu_drv_err("stream_sub %u is null, entity_idx = %u\n",
			stream_id, entity_idx);
		return NULL;
	}

	if (stream_sub->id != stream_id) {
		npu_drv_err("stream_sub id %u stream_id %u is not match\n",
			stream_sub->id, stream_id);
		return NULL;
	}

	return stream_sub;
}

int npu_alloc_sink_stream_id(u8 dev_id, u8 is_long)
{
	struct npu_id_entity *stream_sub_info = NULL;
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_id_allocator *id_allocator = NULL;

	cond_return_error(NPU_MAX_SINK_LONG_STREAM_ID == 0 && is_long,
		-1, "invalid param\n");
	cond_return_error(dev_id >= NPU_DEV_NUM, -1,
		"invalid device id, dev_id = %u\n", dev_id);
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -1, "cur_dev_ctx %u is null\n",
		dev_id);
	id_allocator = &(cur_dev_ctx->id_allocator[is_long ?
		NPU_ID_TYPE_SINK_LONG_STREAM : NPU_ID_TYPE_SINK_STREAM]);

	stream_sub_info = npu_id_allocator_alloc(id_allocator);
	if (stream_sub_info == NULL) {
		npu_drv_err("alloc sink stream id failed\n");
		return -1;
	}

	return stream_sub_info->id;
}

int npu_free_sink_stream_id(u8 dev_id, u32 stream_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_id_allocator *id_allocator = NULL;

	cond_return_error(dev_id >= NPU_DEV_NUM, -EINVAL,
		"invalid device id, dev_id = %u\n", dev_id);
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODATA,
		"cur_dev_ctx %u is null\n", dev_id);
	if (stream_id >=
		NPU_MAX_NON_SINK_STREAM_ID + NPU_MAX_SINK_LONG_STREAM_ID)
		id_allocator = &(cur_dev_ctx->id_allocator[NPU_ID_TYPE_SINK_STREAM]);
	else
		id_allocator =
			&(cur_dev_ctx->id_allocator[NPU_ID_TYPE_SINK_LONG_STREAM]);

	return npu_id_allocator_free(id_allocator, stream_id);
}
