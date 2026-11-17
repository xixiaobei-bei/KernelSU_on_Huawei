/**
 * @file fence_acquire_sync.c
 * @brief Provide fence synchronization mechanism
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sync_file.h>
#include <linux/dma-fence.h>

#include "dkmd_acquire_fence.h"
#include "dkmd_fence_utils.h"


#define ACQUIRE_SYNC_FENCE_DRIVER_NAME "acquire_sync"

static const char *acquire_sync_fence_get_driver_name(struct dma_fence *fence)
{
	return ACQUIRE_SYNC_FENCE_DRIVER_NAME;
}

static bool acquire_sync_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

static bool acquire_sync_fence_signaled(struct dma_fence *fence)
{
	return true;
}

static void acquire_sync_fence_value_str(struct dma_fence *fence, char *str, int size)
{
	snprintf(str, size, "%u", fence->seqno);
}

static struct dma_fence_ops acquire_sync_ops = {
	.get_driver_name = acquire_sync_fence_get_driver_name,
	.get_timeline_name = acquire_sync_fence_get_driver_name,
	.enable_signaling = acquire_sync_fence_enable_signaling,
	.signaled = acquire_sync_fence_signaled,
	.wait = dma_fence_default_wait,
	.release = NULL,
	.fence_value_str = acquire_sync_fence_value_str,
	.timeline_value_str = acquire_sync_fence_value_str,
};

int dkmd_acquire_fence_create_fd(struct dma_fence *fence, spinlock_t *lock, int value)
{
	int fence_fd = -1;

	if (!fence)
		return -1;

	dma_fence_init(fence, &acquire_sync_ops, lock, dma_fence_context_alloc(1), value);

	fence_fd = dkmd_fence_get_fence_fd(fence);
	if (fence_fd < 0) {
		pr_err("%s: get_unused_fd_flags failed error:0x%x\n", __func__, fence_fd);
		dma_fence_put(fence);
		return -1;
	}

	return fence_fd;
}
EXPORT_SYMBOL(dkmd_acquire_fence_create_fd);

int dkmd_acquire_fence_wait(int fence_fd, long timeout)
{
	int rc = 0;
	struct dma_fence *fence = NULL;

	if (fence_fd < 0)
		return 0;

	fence = sync_file_get_fence(fence_fd);
	if (!fence) {
		pr_err("fence_fd=%d, sync_file_get_fence failed!\n", fence_fd);
		return -EINVAL;
	}

	rc = dma_fence_wait_timeout(fence, false, msecs_to_jiffies(timeout));
	if (rc > 0) {
		pr_info("signaled drv:%s timeline:%s seqno:%d\n",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);
		rc = 0;
	} else if (rc == 0) {
		pr_err("timeout drv:%s timeline:%s seqno:%d.\n",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);
		rc = -ETIMEDOUT;
	}

	return rc;
}
EXPORT_SYMBOL(dkmd_acquire_fence_wait);

int dkmd_acquire_fence_signal(int fence_fd)
{
	int rc = 0;
	struct dma_fence *fence = NULL;

	if (fence_fd < 0)
		return 0;

	fence = sync_file_get_fence(fence_fd);
	if (!fence) {
		pr_err("fence_fd=%d, sync_file_get_fence failed!\n", fence_fd);
		return -EINVAL;
	}

	if (dma_fence_is_signaled_locked(fence)) {
		pr_info("drv:%s timeline:%s seqno:%d signaled\n",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);
		dma_fence_put(fence);
	} else {
		pr_info("signal err drv:%s timeline:%s seqno:%d\n",
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence),
			fence->seqno);
		rc = -1;
	}

	return rc;
}
EXPORT_SYMBOL(dkmd_acquire_fence_signal);

void dkmd_acquire_fence_release(int fence_fd)
{
	struct dma_fence *fence = NULL;

	if (fence_fd < 0)
		return;

	fence = sync_file_get_fence(fence_fd);
	if (!fence) {
		pr_err("fence_fd=%d, sync_file_get_fence failed!\n", fence_fd);
		return;
	}

	dma_fence_put(fence);
}
EXPORT_SYMBOL(dkmd_acquire_fence_release);

MODULE_LICENSE("GPL");