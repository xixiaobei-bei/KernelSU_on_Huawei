/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU related data structs
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */
#ifndef MALI_KBASE_HISILICON_H
#define MALI_KBASE_HISILICON_H

#include "mali_kbase_hisi_callback.h"
#include "hisi_ipa/mali_kbase_ipa_ctx.h"
#include "linux/gmc.h"

/**
 * struct kbase_hisi_device_data - all hisi platform data in device level.
 *
 * @callbacks: The callbacks hisi implements.
 * @mgm_dev: The memory_group_manager_device used to alloc/free memory, etc.
 *           We can use this dev to alloc normal memory or last buffer memory.
 * @mgm_ops: The operation interfaces of @mgm_dev.
 * @lb_pools: The info related with device's last_buffer memory pools.
 * @cache_policy_info: The list of cache policy info object.
 * @nr_cache_policy: Number of entries in the list of cache_policy_info.
 */
struct kbase_hisi_device_data {
	struct kbase_hisi_callbacks *callbacks;

	/* Add other device data here */

#ifdef CONFIG_MALI_LAST_BUFFER
	struct kbase_policy_manager *policy_manager;
	struct kbase_quota_manager *quota_manager;
#endif

	/* Data about hisi dynamic IPA */
	struct kbase_ipa_context *ipa_ctx;
	struct work_struct bound_detect_work;
	unsigned long bound_detect_freq;
	unsigned long bound_detect_btime;
#ifdef CONFIG_GPU_GMC_GENERIC
	struct gmc_device kbase_gmc_device;
#endif
};

/**
 * struct kbase_hisi_ctx_data - all hisi platform data in context level.
 *
 * @lb_pools: The info related with context's last_buffer memory pools.
 *            Contains the last buffer's memory pools with different policy id
 *            and corresponding lock.
 */
struct kbase_hisi_ctx_data {

/* Add other context data here */

#ifdef CONFIG_GPU_GMC_GENERIC
	bool set_pt_flag;
#endif
};

#endif /* MALI_KBASE_HISILICON_H */
