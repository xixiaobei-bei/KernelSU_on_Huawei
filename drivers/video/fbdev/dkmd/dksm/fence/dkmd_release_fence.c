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
#include <linux/types.h>
#include <linux/dma-fence.h>
#include <linux/file.h>
#include <linux/sync_file.h>
#include <linux/spinlock.h>

#include "dkmd_release_fence.h"
#include "dkmd_fence_utils.h"
#include "dkmd_utils.h"

#define DKMD_RELEASE_FENCE_DRIVER_NAME "dkmd_release_fence"
static const char *release_fence_get_driver_name(struct dma_fence *fence)
{
	return DKMD_RELEASE_FENCE_DRIVER_NAME;
}

static const char *release_fence_get_timeline_name(struct dma_fence *fence)
{
	struct dkmd_timeline *timeline = release_fence_get_timeline(fence);

	if (timeline)
		return timeline->name;

	return NULL;
}

static bool release_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

// gt: greater than
static bool release_fence_is_signaled_gt(struct dma_fence *fence)
{
	struct dkmd_timeline *timeline = release_fence_get_timeline(fence);

	if (!timeline)
		return false;

	return ((s32)(timeline->pt_value - fence->seqno)) > 0;
}

static void release_fence_release(struct dma_fence *fence)
{
	kfree_rcu(fence, rcu);
}

static void release_fence_value_str(struct dma_fence *fence, char *str, int size)
{
	snprintf(str, size, "%u", fence->seqno);
}

static void release_fence_timeline_value_str(struct dma_fence *fence, char *str, int size)
{
	struct dkmd_timeline *timeline = release_fence_get_timeline(fence);

	if (timeline)
		snprintf(str, size, "%u", timeline->pt_value);
}

static struct dma_fence_ops g_release_fence_ops = {
	.get_driver_name = release_fence_get_driver_name,
	.get_timeline_name = release_fence_get_timeline_name,
	.enable_signaling = release_fence_enable_signaling,
	.signaled = release_fence_is_signaled_gt,
	.wait = dma_fence_default_wait,
	.release = release_fence_release,
	.fence_value_str = release_fence_value_str,
	.timeline_value_str = release_fence_timeline_value_str,
};

static const char* _fence_listener_get_name(struct dkmd_timeline_listener *listener)
{
	struct dkmd_release_fence *fence = (struct dkmd_release_fence *)listener->listener_data;

	if (!fence || !fence->base.ops || !fence->base.ops->get_driver_name)
		return NULL;

	return fence->base.ops->get_driver_name(&fence->base);
}

static bool _fence_listener_is_signaled(struct dkmd_timeline_listener *listener, uint32_t tl_pt_val)
{
	struct dkmd_release_fence *fence = (struct dkmd_release_fence *)listener->listener_data;

	if (!fence)
		return false;

	return dma_fence_is_signaled_locked(&fence->base);
}

static void _fence_listener_release(struct dkmd_timeline_listener *listener)
{
	struct dkmd_release_fence *fence = (struct dkmd_release_fence *)listener->listener_data;

	if (fence)
		dma_fence_put(&fence->base);
}

static struct dkmd_timeline_listener_ops g_fence_listener_ops = {
	.get_listener_name = _fence_listener_get_name,
	.enable_signaling = NULL,
	.disable_signaling = NULL,
	.is_signaled = _fence_listener_is_signaled,
	.handle_signal = NULL,
	.release = _fence_listener_release,
};

static void release_fence_init(struct dkmd_release_fence *fence, struct dkmd_timeline *timeline, uint64_t pt_value)
{
	static u64 fence_context = 0;

	if (fence_context == 0)
		fence_context = dma_fence_context_alloc(1);

	fence->timeline = timeline;
	spin_lock_init(&fence->spin_lock_pt);

	dma_fence_init(&fence->base, &g_release_fence_ops, &fence->spin_lock_pt, fence_context, pt_value);
}

int dkmd_release_fence_create(struct dkmd_timeline *timeline)
{
	struct dkmd_release_fence *fence = NULL;
	struct dkmd_timeline_listener *listener = NULL;
	int fd;
	uint64_t pt_value;

	fence = kzalloc(sizeof(*fence), GFP_KERNEL);
	if (!fence) {
		dpu_pr_err("kzalloc fence fail");
		return -1;
	}

	pt_value = dkmd_timeline_get_pt_value(timeline);
	release_fence_init(fence, timeline, pt_value + 1);
	fd = dkmd_fence_get_fence_fd(&fence->base);
	if (fd < 0)
		goto alloc_fence_fail;

	/* add fence to timeline's listener, and listening at pt_value point */
	listener = dkmd_timeline_alloc_listener(&g_fence_listener_ops, fence, pt_value);
	if (!listener)
		goto alloc_fence_listener_fail;

	dkmd_timeline_add_listener(timeline, listener);

	return fd;

alloc_fence_listener_fail:
	put_unused_fd(fd);
alloc_fence_fail:
	kfree(fence);
	return -1;
}
