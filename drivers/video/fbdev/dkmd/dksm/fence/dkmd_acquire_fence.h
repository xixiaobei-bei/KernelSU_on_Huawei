/**
 * @file dkmd_acquire_fence.h
 * @brief To provide an interface timing synchronization function
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

#ifndef DKMD_ACQUIRE_FENCE_H
#define DKMD_ACQUIRE_FENCE_H

#include <linux/types.h>
#include <linux/spinlock.h>


#define ACQUIRE_FENCE_TIMEOUT_MSEC (4000) // 4s

/**
 * @brief Create fence object from the given value
 *
 * @param fence
 * @param lock
 * @param value
 * @return int fence fd created on given value
 */
int dkmd_acquire_fence_create_fd(struct dma_fence *fence, spinlock_t *lock, int value);

/**
 * @brief sleep until the fence gets signaled or until timeout elapses
 *
 * @param fence_fd given fence object
 * @param timeout timeout value in ms, or MAX_SCHEDULE_TIMEOUT
 * @return int
 */
int dkmd_acquire_fence_wait(int fence_fd, long timeout);

/**
 * @brief signal completion of a fence
 *
 * @param fence_fd the fence to signal
 * @return int
 */
int dkmd_acquire_fence_signal(int fence_fd);

/**
 * @brief destroy given fence object
 *
 * @param fence_fd the fence to release
 * @return void
 */
void dkmd_acquire_fence_release(int fence_fd);

#endif