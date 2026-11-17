/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU callback headers
 * Author: Huawei Technologies Co., Ltd.
 * Create: 2014-2-24
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef HISI_CALLBACK_H
#define HISI_CALLBACK_H
#ifdef CONFIG_MALI_LAST_BUFFER
#include <base/lb/mali_kbase_hisi_lb.h>
#endif

#include <mali_kbase.h>

#define KBASE_PM_TIME_SHIFT 8

/* This struct holds all of the callbacks of hisi platform. */
struct kbase_hisi_callbacks {
	void (*cl_boost_init)(void *dev);
	void (*cl_boost_update_utilization)(void *dev, void *atom,
		u64 microseconds_spent);

#ifdef CONFIG_MALI_LAST_BUFFER
	struct kbase_hisi_lb_callbacks *lb_cbs;
#endif
};

uintptr_t gpu_get_callbacks(void);

#endif
