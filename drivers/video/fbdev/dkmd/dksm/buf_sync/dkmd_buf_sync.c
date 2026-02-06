/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/dma-buf.h>
#include <linux/module.h>
#include "dkmd_buf_sync.h"
#include "dkmd_utils.h"

static bool buf_sync_is_signaled(struct dkmd_timeline_listener *listener, uint32_t tl_val)
{
	dpu_pr_info("tl_val=%u, listener->pt_value = %u", tl_val, listener->pt_value);
	return tl_val > listener->pt_value;
}

static int buf_sync_handle_signal(struct dkmd_timeline_listener *listener)
{
	struct dkmd_dma_buf *layer_buf = (struct dkmd_dma_buf *)listener->listener_data;

	dpu_pr_info(" ++++ shard_fd = %d", layer_buf->shared_fd);

	dma_buf_put(layer_buf->buf_handle);
	layer_buf->buf_handle = NULL;
	layer_buf->shared_fd = -1;
	return 0;
}

static struct dkmd_timeline_listener_ops g_layer_buf_listener_ops = {
	.get_listener_name = NULL,
	.enable_signaling = NULL,
	.disable_signaling = NULL,
	.is_signaled = buf_sync_is_signaled,
	.handle_signal = buf_sync_handle_signal,
	.release = NULL,
};

int dkmd_buf_sync_lock_dma_buf(struct dkmd_timeline *timeline, struct dkmd_dma_buf *layer_dma_buf, int shared_fd)
{
	struct dma_buf *buf_handle = NULL;
	struct dkmd_timeline_listener *listener = NULL;

	dpu_assert_if_cond(timeline == NULL);
	dpu_assert_if_cond(layer_dma_buf == NULL);

	if (shared_fd < 0)
		return 0;

	buf_handle = dma_buf_get(shared_fd);
	if (IS_ERR_OR_NULL(buf_handle)) {
		dpu_pr_err("get dma buf fail, shard_fd=%d", shared_fd);
		return -1;
	}

	layer_dma_buf->shared_fd = shared_fd;
	layer_dma_buf->buf_handle = buf_handle;

	listener = dkmd_timeline_alloc_listener(&g_layer_buf_listener_ops, layer_dma_buf, dkmd_timeline_get_pt_value(timeline) + 1);
	if (!listener) {
		dpu_pr_err("alloc layer buf listener fail, shared_fd = %d", shared_fd);
		return -1;
	}

	dkmd_timeline_add_listener(timeline, listener);

	return 0;
}
EXPORT_SYMBOL(dkmd_buf_sync_lock_dma_buf);

MODULE_LICENSE("GPL");