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
#ifndef DKMD_FENCE_H
#define DKMD_FENCE_H

#include <linux/types.h>
#include <linux/dma-fence.h>
#include <linux/spinlock.h>

#include "dkmd_timeline.h"

struct dkmd_release_fence {
	struct dma_fence base;
	struct dkmd_timeline *timeline;

	spinlock_t spin_lock_pt; // lock for current_pt_value
};

static inline struct dkmd_timeline *release_fence_get_timeline(struct dma_fence *fence)
{
	struct dkmd_release_fence *dkmd_fence = container_of(fence, struct dkmd_release_fence, base);
	return dkmd_fence->timeline;
}

// TODO:
/**
 * @brief
 *  return fence fd, if create fence succ, will return >= 0,
 *  else return < 0.
 */
int dkmd_release_fence_create(struct dkmd_timeline *timeline);


#endif